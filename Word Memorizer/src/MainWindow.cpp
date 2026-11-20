#include "MainWindow.h"
#include "SettingsDialog.h"
#include "UsageDialog.h"
#include "AboutDialog.h"
#include "IconHelper.h"
#include "GlassTheme.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace {
std::string normalize_answer(std::string answer) {
    const auto first = answer.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = answer.find_last_not_of(" \t\r\n");
    answer = answer.substr(first, last - first + 1);
    std::transform(answer.begin(), answer.end(), answer.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return answer;
}
}

MainWindow::MainWindow(const std::string& settingsFile)
    : settingsManager(settingsFile),
      mainContainer(Gtk::ORIENTATION_VERTICAL),
      contentArea(Gtk::ORIENTATION_HORIZONTAL),
      leftBox(Gtk::ORIENTATION_VERTICAL),
      sidePanel(wordManager, settingsManager),
      learningPage(Gtk::ORIENTATION_VERTICAL),
      completionPage(Gtk::ORIENTATION_VERTICAL),
      placeholderPage(Gtk::ORIENTATION_VERTICAL),
      attemptCount(0),
      wordBox(Gtk::ORIENTATION_VERTICAL),
      inputBox(Gtk::ORIENTATION_HORIZONTAL),
      controlBox(Gtk::ORIENTATION_HORIZONTAL) {

    set_default_size(900, 600);
    GlassTheme::configure_window(*this, "VocabMemster", true);

    // ========== 左侧导航栏（始终可见，不随页面切换隐藏） ==========
    auto* navBar = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
    navBar->get_style_context()->add_class("side-navbar");
    navBar->set_size_request(60, -1);

    auto make_nav_button = [](const std::string& icon, const std::string& label) -> Gtk::Button* {
        auto* btn = Gtk::manage(new Gtk::Button());
        btn->get_style_context()->add_class("nav-button");
        auto* box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 2));
        box->pack_start(*IconHelper::create_icon(icon, 20), Gtk::PACK_SHRINK);
        auto* lbl = Gtk::manage(new Gtk::Label(label));
        lbl->get_style_context()->add_class("nav-button-label");
        box->pack_start(*lbl, Gtk::PACK_SHRINK);
        btn->add(*box);
        btn->show_all();
        return btn;
    };

    auto* openBtn = make_nav_button(IconHelper::FOLDER_OPEN, "打开");
    auto* helpBtn = make_nav_button(IconHelper::INFO, "帮助");
    auto* aboutBtn = make_nav_button(IconHelper::INFO, "关于");

    navBar->pack_start(*openBtn, Gtk::PACK_SHRINK);
    navBar->pack_start(*helpBtn, Gtk::PACK_SHRINK);
    navBar->pack_start(*aboutBtn, Gtk::PACK_SHRINK);

    openBtn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_open_file));
    helpBtn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_usage));
    aboutBtn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_about));

    // ========== 学习页 ==========
    leftBox.set_orientation(Gtk::ORIENTATION_VERTICAL);

    wordBox.set_spacing(15);
    wordBox.set_border_width(20);
    wordBox.set_halign(Gtk::ALIGN_CENTER);
    wordBox.get_style_context()->add_class("glass-card");

    posLabel.set_justify(Gtk::JUSTIFY_CENTER);
    posLabel.get_style_context()->add_class("word-pos");
    posLabel.override_font(Pango::FontDescription("Sans Bold 14"));
    posLabel.set_margin_bottom(10);
    meaningLabel.set_justify(Gtk::JUSTIFY_CENTER);
    meaningLabel.override_font(Pango::FontDescription("Sans Bold 20"));
    meaningLabel.set_margin_bottom(15);
    exampleLabel.set_justify(Gtk::JUSTIFY_CENTER);
    exampleLabel.override_font(Pango::FontDescription("Sans 12"));
    exampleLabel.set_line_wrap(true);
    exampleLabel.set_max_width_chars(40);

    wordBox.pack_start(posLabel, Gtk::PACK_SHRINK);
    wordBox.pack_start(meaningLabel, Gtk::PACK_SHRINK);
    wordBox.pack_start(exampleLabel, Gtk::PACK_SHRINK);
    leftBox.pack_start(wordBox, Gtk::PACK_SHRINK);

    inputBox.set_spacing(10);
    inputBox.set_border_width(15);
    inputBox.set_halign(Gtk::ALIGN_CENTER);
    inputHintLabel.set_label("请输入英文单词:");
    answerEntry.get_style_context()->add_class("glass-entry");
    answerEntry.set_placeholder_text("在这里输入答案...");
    answerEntry.set_size_request(200, 36);
    submitButton.get_style_context()->add_class("glass-btn");
    submitButton.set_label("提交");
    inputBox.pack_start(inputHintLabel, Gtk::PACK_SHRINK);
    inputBox.pack_start(answerEntry, Gtk::PACK_SHRINK);
    inputBox.pack_start(submitButton, Gtk::PACK_SHRINK);
    leftBox.pack_start(inputBox, Gtk::PACK_SHRINK);

    feedbackLabel.get_style_context()->add_class("feedback-label");
    feedbackLabel.set_justify(Gtk::JUSTIFY_CENTER);
    attemptLabel.set_justify(Gtk::JUSTIFY_CENTER);
    leftBox.pack_start(feedbackLabel, Gtk::PACK_SHRINK);
    leftBox.pack_start(attemptLabel, Gtk::PACK_SHRINK);

    controlBox.set_spacing(10);
    controlBox.set_border_width(10);
    controlBox.set_halign(Gtk::ALIGN_CENTER);
    showAnswerButton.get_style_context()->add_class("glass-btn-secondary");
    showAnswerButton.set_label("查看答案");
    nextWordButton.get_style_context()->add_class("glass-btn");
    nextWordButton.set_label("下一个单词");
    controlBox.pack_start(showAnswerButton, Gtk::PACK_SHRINK);
    controlBox.pack_start(nextWordButton, Gtk::PACK_SHRINK);
    leftBox.pack_start(controlBox, Gtk::PACK_SHRINK);

    learningPage.pack_start(leftBox, Gtk::PACK_EXPAND_WIDGET);

    // ========== 占位页 ==========
    auto* placeholderBox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
    placeholderBox->set_halign(Gtk::ALIGN_CENTER);
    placeholderBox->set_valign(Gtk::ALIGN_CENTER);
    placeholderBox->set_vexpand(true);
    placeholderBox->get_style_context()->add_class("placeholder-page");
    auto* alertIcon = IconHelper::create_icon(IconHelper::ALERT_TRIANGLE, 64);
    alertIcon->get_style_context()->add_class("placeholder-icon");
    placeholderBox->pack_start(*alertIcon, Gtk::PACK_SHRINK);
    auto* placeholderLabel = Gtk::manage(new Gtk::Label("暂未打开单词本\n通过左侧「打开」按钮选择词库开始学习"));
    placeholderLabel->set_justify(Gtk::JUSTIFY_CENTER);
    placeholderLabel->override_font(Pango::FontDescription("Sans 14"));
    placeholderLabel->get_style_context()->add_class("placeholder-text");
    placeholderBox->pack_start(*placeholderLabel, Gtk::PACK_SHRINK);
    placeholderPage.pack_start(*placeholderBox, Gtk::PACK_EXPAND_WIDGET);

    // ========== 完成页 ==========
    auto* completionContent = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
    completionContent->set_halign(Gtk::ALIGN_CENTER);
    completionContent->set_valign(Gtk::ALIGN_CENTER);
    completionContent->set_vexpand(true);
    completionContent->get_style_context()->add_class("glass-card");
    completionTitle.set_label("恭喜你！");
    completionTitle.override_font(Pango::FontDescription("Sans Bold 28"));
    completionStats.set_justify(Gtk::JUSTIFY_CENTER);
    completionStats.override_font(Pango::FontDescription("Sans 16"));
    restartButton.get_style_context()->add_class("glass-btn");
    restartButton.set_label("重新开始");
    reviewWrongWordsButton.get_style_context()->add_class("glass-btn-secondary");
    reviewWrongWordsButton.set_label("复习错词本");
    completionContent->pack_start(completionTitle, Gtk::PACK_SHRINK);
    completionContent->pack_start(completionStats, Gtk::PACK_SHRINK);
    completionContent->pack_start(restartButton, Gtk::PACK_SHRINK);
    completionContent->pack_start(reviewWrongWordsButton, Gtk::PACK_SHRINK);
    completionPage.pack_start(*completionContent, Gtk::PACK_EXPAND_WIDGET);

    pageStack.add(placeholderPage, "placeholder");
    pageStack.add(learningPage, "learning");
    pageStack.add(completionPage, "completion");
    pageStack.set_visible_child("placeholder");

    // ========== 内容区：导航栏 + 页面栈 + 右侧面板（全局固定布局） ==========
    contentArea.pack_start(*navBar, Gtk::PACK_SHRINK);
    contentArea.pack_start(pageStack, Gtk::PACK_EXPAND_WIDGET);
    contentArea.pack_start(sidePanel, Gtk::PACK_SHRINK);
    mainContainer.pack_start(contentArea, Gtk::PACK_EXPAND_WIDGET);

    add(mainContainer);

    // 信号连接
    submitButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_submit_answer));
    showAnswerButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_show_answer));
    nextWordButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_next_word));
    answerEntry.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::on_entry_activate));
    restartButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_restart));
    reviewWrongWordsButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_review_wrong_words));

    sidePanel.signal_toggle_dark_mode().connect(sigc::mem_fun(*this, &MainWindow::toggle_dark_mode));
    sidePanel.signal_open_settings().connect(sigc::mem_fun(*this, &MainWindow::on_settings));
    sidePanel.signal_open_wrong_words().connect(sigc::mem_fun(*this, &MainWindow::on_review_wrong_words));

    apply_theme();
    reset_attempt();
    show_all_children();
    sidePanel.update_stats();
}

MainWindow::~MainWindow() {
    completionTimeout.disconnect();
}

bool MainWindow::loadWordsFromFile(const std::string& filename) {
    if (!wordManager.loadWordsFromFile(filename)) return false;
    currentWord = Word{"", "", "", ""};
    show_learning_page();
    on_next_word();
    return true;
}

void MainWindow::on_open_file() {
    Gtk::FileChooserDialog dialog("选择单词库文件", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);
    dialog.add_button("取消", Gtk::RESPONSE_CANCEL);
    dialog.add_button("打开", Gtk::RESPONSE_OK);
    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("文本文件");
    filter_text->add_pattern("*.txt");
    dialog.add_filter(filter_text);
    apply_dialog_theme(dialog);
    if (dialog.run() == Gtk::RESPONSE_OK) {
        std::string filename = dialog.get_filename();
        dialog.hide();
        if (!loadWordsFromFile(filename)) {
            Gtk::MessageDialog error(*this, "无法加载单词库", false,
                                     Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            error.set_secondary_text("请检查文件是否可读，并使用 单词|词性|释义|例句 的格式。");
            apply_dialog_theme(error);
            error.run();
        }
    }
}

void MainWindow::on_edit_words() {
    WordManagerDialog dialog(*this);
    apply_dialog_theme(dialog);
    dialog.run();
}

void MainWindow::on_next_word() {
    completionTimeout.disconnect();
    if (wordManager.getTotalWords() == 0) return;
    if (!currentWord.word.empty() && wordManager.hasFailed(currentWord.word) && answerEntry.get_sensitive()) {
        wordManager.addWrongWord(currentWord);
    }
    if (wordManager.allWordsMastered()) {
        show_completion_page();
        return;
    }
    currentWord = wordManager.getRandomWord();
    if (!currentWord.word.empty()) {
        reset_attempt();
        update_display();
        update_progress();
        sidePanel.update_stats();
    }
}

void MainWindow::on_submit_answer() {
    if (currentWord.word.empty() || is_word_completed() ||
        pageStack.get_visible_child_name() != "learning") return;
    const std::string answerLower = normalize_answer(answerEntry.get_text());
    if (answerLower.empty()) return;
    const std::string correctLower = normalize_answer(currentWord.word);

    if (answerLower == correctLower) {
        wordManager.addMasteredWord(currentWord.word);
        feedbackLabel.set_label("✓ 正确！答案: " + currentWord.word);
        set_feedback_style("feedback-correct");
        answerEntry.set_sensitive(false);
        submitButton.set_sensitive(false);
        showAnswerButton.set_sensitive(false);
        update_progress();
        sidePanel.update_stats();
        if (wordManager.allWordsMastered()) {
            completionTimeout = Glib::signal_timeout().connect([this]() {
                show_completion_page();
                return false;
            }, 1000);
        }
    } else {
        wordManager.addWrongWord(currentWord);
        wordManager.addFailedWord(currentWord.word);
        attemptCount++;
        feedbackLabel.set_label("✗ 错误！请再试一次");
        set_feedback_style("feedback-error");
        attemptLabel.set_label("尝试次数: " + std::to_string(attemptCount));
        answerEntry.set_text("");
        answerEntry.grab_focus();
        sidePanel.update_stats();
    }
}

void MainWindow::on_show_answer() {
    if (currentWord.word.empty() || is_word_completed() ||
        pageStack.get_visible_child_name() != "learning") return;
    feedbackLabel.set_label("正确答案: " + currentWord.word);
    set_feedback_style("feedback-answer");
    answerEntry.set_sensitive(false);
    submitButton.set_sensitive(false);
    showAnswerButton.set_sensitive(false);
    wordManager.addWrongWord(currentWord);
    wordManager.addFailedWord(currentWord.word);
    sidePanel.update_stats();
}

void MainWindow::on_entry_activate() { on_submit_answer(); }

void MainWindow::on_restart() {
    if (wordManager.getTotalWords() == 0) return;
    currentWord = Word{"", "", "", ""};
    wordManager.clearMasteredWords();
    wordManager.clearWrongWords();
    show_learning_page();
    on_next_word();
    sidePanel.update_stats();
}

void MainWindow::on_export_wrong_words() {
    auto wrongWords = wordManager.getWrongWords();
    if (wrongWords.empty()) return;
    Gtk::FileChooserDialog dialog("导出错词本", Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);
    dialog.add_button("取消", Gtk::RESPONSE_CANCEL);
    dialog.add_button("保存", Gtk::RESPONSE_OK);
    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("文本文件");
    filter_text->add_pattern("*.txt");
    dialog.add_filter(filter_text);
    dialog.set_current_name("wrong_words.txt");
    apply_dialog_theme(dialog);
    if (dialog.run() == Gtk::RESPONSE_OK) {
        std::string filename = dialog.get_filename();
        if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") filename += ".txt";
        std::ofstream file(filename);
        for (const auto& word : wrongWords)
            file << word.word << "|" << word.pos << "|" << word.meaning << "|" << word.example << std::endl;
    }
}

void MainWindow::on_review_wrong_words() {
    auto wrongWords = wordManager.getWrongWords();
    if (wrongWords.empty()) {
        Gtk::MessageDialog d(*this, "错词本为空", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        apply_dialog_theme(d);
        d.run();
        return;
    }
    Gtk::Dialog dialog("错词本", *this, true);
    GlassTheme::configure_window(dialog, "错词本");
    dialog.set_default_size(600, 450);
    dialog.get_style_context()->add_class("dialog-box");

    Gtk::Box content(Gtk::ORIENTATION_VERTICAL, 10);
    content.set_border_width(16);

    std::stringstream header;
    header << "共 " << wrongWords.size() << " 个错词";
    auto* headerLabel = Gtk::manage(new Gtk::Label(header.str()));
    headerLabel->override_font(Pango::FontDescription("Sans Bold 16"));
    content.pack_start(*headerLabel, Gtk::PACK_SHRINK);

    Gtk::ScrolledWindow scrolled;
    scrolled.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    Gtk::Box wordsBox(Gtk::ORIENTATION_VERTICAL, 8);

    for (const auto& w : wrongWords) {
        auto* card = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 4));
        card->get_style_context()->add_class("glass-card");
        card->set_border_width(12);

        auto* wordLabel = Gtk::manage(new Gtk::Label(w.word + "  [" + w.pos + "]"));
        wordLabel->override_font(Pango::FontDescription("Sans Bold 14"));
        wordLabel->set_halign(Gtk::ALIGN_START);

        auto* meaningLabel = Gtk::manage(new Gtk::Label("释义: " + w.meaning));
        meaningLabel->set_halign(Gtk::ALIGN_START);

        auto* exampleLabel = Gtk::manage(new Gtk::Label("例句: " + w.example));
        exampleLabel->set_halign(Gtk::ALIGN_START);
        exampleLabel->set_line_wrap(true);

        card->pack_start(*wordLabel, Gtk::PACK_SHRINK);
        card->pack_start(*meaningLabel, Gtk::PACK_SHRINK);
        card->pack_start(*exampleLabel, Gtk::PACK_SHRINK);
        wordsBox.pack_start(*card, Gtk::PACK_SHRINK);
    }
    scrolled.add(wordsBox);
    content.pack_start(scrolled, Gtk::PACK_EXPAND_WIDGET);

    // 底部按钮
    Gtk::Box buttonBox(Gtk::ORIENTATION_HORIZONTAL, 10);
    buttonBox.set_halign(Gtk::ALIGN_END);
    auto* exportBtn = Gtk::manage(new Gtk::Button("导出"));
    exportBtn->get_style_context()->add_class("glass-btn-secondary");
    auto* clearBtn = Gtk::manage(new Gtk::Button("清空"));
    clearBtn->get_style_context()->add_class("glass-btn");
    auto* closeBtn = Gtk::manage(new Gtk::Button("关闭"));
    closeBtn->get_style_context()->add_class("glass-btn");

    buttonBox.pack_start(*exportBtn, Gtk::PACK_SHRINK);
    buttonBox.pack_start(*clearBtn, Gtk::PACK_SHRINK);
    buttonBox.pack_start(*closeBtn, Gtk::PACK_SHRINK);
    content.pack_start(buttonBox, Gtk::PACK_SHRINK);

    dialog.get_content_area()->pack_start(content);
    dialog.show_all();
    apply_dialog_theme(dialog);

    exportBtn->signal_clicked().connect([this, &dialog]() {
        dialog.hide();
        on_export_wrong_words();
    });
    clearBtn->signal_clicked().connect([this, &dialog]() {
        wordManager.clearWrongWords();
        sidePanel.update_stats();
        dialog.response(Gtk::RESPONSE_OK);
    });
    closeBtn->signal_clicked().connect([&dialog]() {
        dialog.response(Gtk::RESPONSE_OK);
    });

    dialog.run();
}


void MainWindow::on_clear_wrong_words() {
    wordManager.clearWrongWords();
    sidePanel.update_stats();
}

void MainWindow::on_reset_progress() {
    on_restart();
}

void MainWindow::on_settings() {
    SettingsDialog dialog(*this, settingsManager);
    apply_dialog_theme(dialog);
    if (dialog.run() == Gtk::RESPONSE_OK) {
        apply_theme();
        sidePanel.update_stats();
    }
}

void MainWindow::on_debug_info() {
    std::stringstream ss;
    ss << "总单词: " << wordManager.getTotalWords() << "\n";
    ss << "已掌握: " << wordManager.getMasteredWordsCount() << "\n";
    ss << "错词: " << wordManager.getWrongWordsCount() << "\n";
    Gtk::MessageDialog d(*this, ss.str(), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    apply_dialog_theme(d);
    d.run();
}

void MainWindow::on_about() {
    AboutDialog aboutDialog(*this);
    apply_dialog_theme(aboutDialog);
    aboutDialog.run();
}

void MainWindow::on_usage() {
    UsageDialog usageDialog(*this);
    apply_dialog_theme(usageDialog);
    usageDialog.run();
}

void MainWindow::update_display() {
    if (!currentWord.word.empty()) {
        posLabel.set_label("[" + currentWord.pos + "]");
        meaningLabel.set_label(currentWord.meaning);
        exampleLabel.set_label("例句: " + currentWord.example);
        sidePanel.set_current_word_status(wordManager.hasFailed(currentWord.word));
        feedbackLabel.set_label("");
        set_feedback_style("");
        attemptLabel.set_label("");
        answerEntry.set_sensitive(true);
        submitButton.set_sensitive(true);
        showAnswerButton.set_sensitive(true);
        answerEntry.set_text("");
        answerEntry.grab_focus();
    }
}

void MainWindow::reset_attempt() {
    attemptCount = 0;
    attemptLabel.set_label("");
}

void MainWindow::show_completion_page() {
    if (!wordManager.allWordsMastered()) return;
    std::stringstream stats;
    stats << "你已掌握所有 " << wordManager.getTotalWords() << " 个单词！\n错词: " << wordManager.getWrongWordsCount();
    completionStats.set_label(stats.str());
    pageStack.set_visible_child("completion");
    sidePanel.update_stats();
}

void MainWindow::show_learning_page() {
    pageStack.set_visible_child("learning");
}

void MainWindow::update_progress() {
    sidePanel.update_stats();
}

void MainWindow::apply_theme() {
    bool dark = settingsManager.isDarkModeEnabled();
    std::string accent = settingsManager.getPrimaryColor();
    GlassTheme::set_screen(get_screen());
    GlassTheme::apply_theme(dark, accent);
    IconHelper::set_dark_mode(dark);
    sidePanel.set_dark_mode_button(dark);
    refresh_text_colors();
}

void MainWindow::set_feedback_style(const std::string& style) {
    const auto context = feedbackLabel.get_style_context();
    for (const auto* name : {"feedback-correct", "feedback-error", "feedback-answer"}) {
        context->remove_class(name);
    }
    if (!style.empty()) context->add_class(style);
    refresh_text_colors();
}

void MainWindow::refresh_text_colors() {
    posLabel.override_color(GlassTheme::readable_color(settingsManager.getPosColor()));
    const auto context = feedbackLabel.get_style_context();
    std::string color;
    if (context->has_class("feedback-correct")) color = settingsManager.getCorrectColor();
    else if (context->has_class("feedback-error")) color = settingsManager.getErrorColor();
    else if (context->has_class("feedback-answer")) color = settingsManager.getAnswerColor();
    if (color.empty()) feedbackLabel.unset_color();
    else feedbackLabel.override_color(GlassTheme::readable_color(color));
}

void MainWindow::toggle_dark_mode() {
    settingsManager.setDarkModeEnabled(!settingsManager.isDarkModeEnabled());
    apply_theme();
}

void MainWindow::apply_dialog_theme(Gtk::Window& dialog) {
    GlassTheme::style_window(dialog);
}

bool MainWindow::on_key_press_event(GdkEventKey* event) {
    if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_Tab) {
        on_next_word(); return true;
    }
    if ((event->state & GDK_MOD1_MASK) && (event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S)) {
        on_show_answer(); return true;
    }
    if ((event->state & GDK_CONTROL_MASK) && (event->state & GDK_SHIFT_MASK) && (event->keyval == GDK_KEY_d || event->keyval == GDK_KEY_D)) {
        toggle_dark_mode(); return true;
    }
    return Gtk::Window::on_key_press_event(event);
}
