#include "DerivativeQuizDialog.h"
#include <iostream>
#include <algorithm>

DerivativeQuizDialog::DerivativeQuizDialog(Gtk::Window& parent, const std::vector<Word>& derivatives)
    : Dialog("派生词检测", parent, true),
      derivatives(derivatives),
      currentDerivativeIndex(0),
      correctCount(0),
      totalCount(derivatives.size()),
      mainBox(Gtk::ORIENTATION_VERTICAL),
      titleBox(Gtk::ORIENTATION_HORIZONTAL),
      contentBox(Gtk::ORIENTATION_VERTICAL),
      inputBox(Gtk::ORIENTATION_HORIZONTAL),
      buttonBox(Gtk::ORIENTATION_HORIZONTAL) {
    
    // 设置对话框属性
    set_default_size(500, 400);
    set_border_width(20);
    set_resizable(false);
    
    initializeUI();
    showCurrentDerivative();
}

DerivativeQuizDialog::~DerivativeQuizDialog() {}

void DerivativeQuizDialog::initializeUI() {
    // 标题区域
    titleLabel.set_label("派生词检测");
    titleLabel.override_font(Pango::FontDescription("Sans Bold 18"));
    titleLabel.set_margin_bottom(15);
    
    progressLabel.set_label("进度: 0/" + std::to_string(totalCount));
    progressLabel.set_margin_bottom(10);
    
    titleBox.pack_start(titleLabel, Gtk::PACK_SHRINK);
    titleBox.pack_end(progressLabel, Gtk::PACK_SHRINK);
    
    // 内容区域
    contentBox.set_spacing(12);
    contentBox.set_margin_bottom(15);
    
    posLabel.set_justify(Gtk::JUSTIFY_CENTER);
    posLabel.override_font(Pango::FontDescription("Sans Bold 14"));
    
    meaningLabel.set_justify(Gtk::JUSTIFY_CENTER);
    meaningLabel.override_font(Pango::FontDescription("Sans Bold 16"));
    meaningLabel.set_margin_bottom(10);
    
    exampleLabel.set_justify(Gtk::JUSTIFY_CENTER);
    exampleLabel.override_font(Pango::FontDescription("Sans 12"));
    exampleLabel.set_line_wrap(true);
    exampleLabel.set_max_width_chars(50);
    
    feedbackLabel.set_justify(Gtk::JUSTIFY_CENTER);
    feedbackLabel.override_font(Pango::FontDescription("Sans Bold 14"));
    feedbackLabel.set_margin_top(10);
    feedbackLabel.set_margin_bottom(10);
    
    contentBox.pack_start(posLabel, Gtk::PACK_SHRINK);
    contentBox.pack_start(meaningLabel, Gtk::PACK_SHRINK);
    contentBox.pack_start(exampleLabel, Gtk::PACK_SHRINK);
    contentBox.pack_start(feedbackLabel, Gtk::PACK_SHRINK);
    
    // 输入区域
    inputBox.set_spacing(10);
    inputBox.set_halign(Gtk::ALIGN_CENTER);
    
    answerEntry.set_placeholder_text("请输入派生词的英文拼写");
    answerEntry.set_size_request(250, 35);
    answerEntry.override_font(Pango::FontDescription("Sans 14"));
    
    submitButton.set_label("提交");
    submitButton.set_size_request(80, 35);
    submitButton.get_style_context()->add_class("primary");
    
    inputBox.pack_start(answerEntry, Gtk::PACK_SHRINK);
    inputBox.pack_start(submitButton, Gtk::PACK_SHRINK);
    
    // 进度条
    progressBar.set_fraction(0.0);
    progressBar.set_show_text(true);
    progressBar.set_size_request(300, 20);
    progressBar.set_halign(Gtk::ALIGN_CENTER);
    progressBar.set_margin_bottom(10);
    
    // 按钮区域
    buttonBox.set_spacing(10);
    buttonBox.set_halign(Gtk::ALIGN_CENTER);
    
    skipButton.set_label("跳过");
    skipButton.set_size_request(80, 35);
    
    closeButton.set_label("关闭");
    closeButton.set_size_request(80, 35);
    
    buttonBox.pack_start(skipButton, Gtk::PACK_SHRINK);
    buttonBox.pack_start(closeButton, Gtk::PACK_SHRINK);
    
    // 主布局
    mainBox.set_spacing(15);
    mainBox.pack_start(titleBox, Gtk::PACK_SHRINK);
    mainBox.pack_start(progressBar, Gtk::PACK_SHRINK);
    mainBox.pack_start(contentBox, Gtk::PACK_SHRINK);
    mainBox.pack_start(inputBox, Gtk::PACK_SHRINK);
    mainBox.pack_start(buttonBox, Gtk::PACK_SHRINK);
    
    get_content_area()->pack_start(mainBox);
    
    // 连接信号
    submitButton.signal_clicked().connect(sigc::mem_fun(*this, &DerivativeQuizDialog::on_submit_clicked));
    skipButton.signal_clicked().connect(sigc::mem_fun(*this, &DerivativeQuizDialog::on_skip_clicked));
    closeButton.signal_clicked().connect(sigc::mem_fun(*this, &DerivativeQuizDialog::on_close_clicked));
    answerEntry.signal_activate().connect(sigc::mem_fun(*this, &DerivativeQuizDialog::on_entry_activate));
    
    show_all_children();
}

void DerivativeQuizDialog::showCurrentDerivative() {
    if (currentDerivativeIndex >= derivatives.size()) {
        showResults();
        return;
    }
    
    const Word& current = derivatives[currentDerivativeIndex];
    
    // 更新显示
    posLabel.set_label("[" + current.pos + "]");
    meaningLabel.set_label(current.meaning);
    
    // 如果有例句就显示，否则隐藏
    if (!current.example.empty()) {
        exampleLabel.set_label("例句: " + current.example);
        exampleLabel.show();
    } else {
        exampleLabel.hide();
    }
    
    // 重置反馈和输入
    feedbackLabel.set_label("");
    answerEntry.set_text("");
    answerEntry.set_sensitive(true);
    submitButton.set_sensitive(true);
    skipButton.set_sensitive(true);
    
    // 更新进度
    updateProgress();
    
    // 聚焦到输入框
    answerEntry.grab_focus();
}

void DerivativeQuizDialog::updateProgress() {
    float progress = (float)currentDerivativeIndex / totalCount;
    progressBar.set_fraction(progress);
    progressBar.set_text(std::to_string((int)(progress * 100)) + "%");
    progressLabel.set_label("进度: " + std::to_string(currentDerivativeIndex) + "/" + std::to_string(totalCount));
}

void DerivativeQuizDialog::on_submit_clicked() {
    std::string userAnswer = answerEntry.get_text();
    const Word& current = derivatives[currentDerivativeIndex];
    
    // 转换为小写进行比较（不区分大小写）
    std::string userAnswerLower = userAnswer;
    std::transform(userAnswerLower.begin(), userAnswerLower.end(), userAnswerLower.begin(), ::tolower);
    
    std::string correctAnswerLower = current.word;
    std::transform(correctAnswerLower.begin(), correctAnswerLower.end(), correctAnswerLower.begin(), ::tolower);
    
    if (userAnswerLower == correctAnswerLower) {
        // 答案正确
        correctCount++;
        feedbackLabel.set_label("✓ 正确！");
        feedbackLabel.override_color(Gdk::RGBA("green"));
    } else {
        // 答案错误
        feedbackLabel.set_label("✗ 错误！正确答案: " + current.word);
        feedbackLabel.override_color(Gdk::RGBA("red"));
    }
    
    // 禁用输入和按钮
    answerEntry.set_sensitive(false);
    submitButton.set_sensitive(false);
    skipButton.set_sensitive(false);
    
    // 延迟切换到下一个单词
    Glib::signal_timeout().connect_once(
        [this]() {
            currentDerivativeIndex++;
            showCurrentDerivative();
        }, 1500);
}

void DerivativeQuizDialog::on_skip_clicked() {
    // 跳过当前单词
    currentDerivativeIndex++;
    showCurrentDerivative();
}

void DerivativeQuizDialog::on_close_clicked() {
    response(Gtk::RESPONSE_CANCEL);
}

void DerivativeQuizDialog::on_entry_activate() {
    on_submit_clicked();
}

void DerivativeQuizDialog::showResults() {
    // 隐藏输入区域
    inputBox.hide();
    skipButton.hide();
    
    // 显示结果
    std::stringstream resultText;
    resultText << "检测完成！\n\n";
    resultText << "正确: " << correctCount << "/" << totalCount << "\n";
    resultText << "准确率: " << (int)(getAccuracy() * 100) << "%";
    
    feedbackLabel.set_label(resultText.str());
    feedbackLabel.override_color(Gdk::RGBA("blue"));
    
    // 更新按钮文本
    closeButton.set_label("完成");
    
    // 更新进度条到完成状态
    progressBar.set_fraction(1.0);
    progressBar.set_text("100%");
    progressLabel.set_label("完成: " + std::to_string(totalCount) + "/" + std::to_string(totalCount));
}