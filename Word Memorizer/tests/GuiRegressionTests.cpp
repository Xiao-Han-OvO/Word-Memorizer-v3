#include "MainWindow.h"
#include "GlassTheme.h"
#include "IconHelper.h"
#include "TestSupport.h"

template<class T>
void collectWidgets(Gtk::Widget& widget, std::vector<T*>& matches) {
    if (auto* match = dynamic_cast<T*>(&widget)) matches.push_back(match);
    if (auto* container = dynamic_cast<Gtk::Container*>(&widget)) {
        for (auto* child : container->get_children()) collectWidgets(*child, matches);
    }
}

template<class T>
std::vector<T*> widgets(Gtk::Widget& root) {
    std::vector<T*> matches;
    collectWidgets(root, matches);
    return matches;
}

template<class T>
T& onlyWidget(Gtk::Widget& root) {
    const auto matches = widgets<T>(root);
    require(matches.size() == 1, "Expected exactly one matching widget");
    return *matches.front();
}

Gtk::Button& button(Gtk::Widget& root, const std::string& text) {
    for (auto* candidate : widgets<Gtk::Button>(root)) {
        if (candidate->get_label() == text) return *candidate;
    }
    throw std::runtime_error("Button missing: " + text);
}

void click(Gtk::Widget& root, const std::string& text) {
    gtk_button_clicked(button(root, text).gobj());
}

Gtk::Label& statValue(MainWindow& window, const std::string& name) {
    for (auto* box : widgets<Gtk::Box>(window)) {
        if (!box->get_style_context()->has_class("stat-card")) continue;
        Gtk::Label* value = nullptr;
        bool matchingName = false;
        for (auto* label : widgets<Gtk::Label>(*box)) {
            if (label->get_text() == name) matchingName = true;
            if (label->get_style_context()->has_class("stat-value")) value = label;
        }
        if (matchingName && value) return *value;
    }
    throw std::runtime_error("Statistic missing: " + name);
}

Gtk::Label& feedback(MainWindow& window) {
    for (auto* label : widgets<Gtk::Label>(window)) {
        if (label->get_style_context()->has_class("feedback-label")) return *label;
    }
    throw std::runtime_error("Feedback label missing");
}

void processEvents(unsigned int milliseconds) {
    auto loop = Glib::MainLoop::create();
    Glib::signal_timeout().connect_once([loop]() { loop->quit(); }, milliseconds);
    loop->run();
}

int main(int argc, char* argv[]) {
    auto application = Gtk::Application::create(argc, argv,
        "com.example.wordmemorizer.regressiontests");
    IconHelper::init_icons();
    GlassTheme::init();
    TestRunner tests;

    tests.run("opening settings, resetting and cancelling do not write preferences", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=true\ncolor_theme=red\n");
        const auto original = readFile(config);
        SettingsManager manager(config);
        Gtk::Window parent;
        SettingsDialog dialog(parent, manager);
        require(readFile(config) == original, "Opening the dialog rewrote settings");
        onlyWidget<Gtk::Switch>(dialog).set_active(false);
        onlyWidget<Gtk::ComboBoxText>(dialog).set_active_id("green");
        require(manager.isDarkModeEnabled() && manager.getColorTheme() == "red",
                "Draft controls changed the active preferences");
        click(dialog, "重置默认");
        click(dialog, "取消");
        require(manager.isDarkModeEnabled() && manager.getColorTheme() == "red",
                "Cancel retained reset preferences");
        require(readFile(config) == original, "Cancel changed preferences on disk");
    });

    tests.run("Save persists theme choices and defaults do not create a file on startup", [] {
        TemporaryDirectory temporary;
        const auto absentConfig = temporary.file("absent-settings");
        SettingsManager defaults(absentConfig);
        require(!std::ifstream(absentConfig), "Reading defaults created a settings file");

        const auto config = temporary.write("settings", "dark_mode=false\ncolor_theme=blue\n");
        SettingsManager manager(config);
        Gtk::Window parent;
        SettingsDialog dialog(parent, manager);
        onlyWidget<Gtk::Switch>(dialog).set_active(true);
        onlyWidget<Gtk::ComboBoxText>(dialog).set_active_id("purple");
        click(dialog, "保存");
        SettingsManager restored(config);
        require(restored.isDarkModeEnabled() && restored.getColorTheme() == "purple",
                "Saved theme did not survive reload");
    });

    tests.run("closing the settings title bar cancels draft preferences", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=true\ncolor_theme=red\n");
        const auto original = readFile(config);
        SettingsManager manager(config);
        Gtk::Window parent;
        SettingsDialog dialog(parent, manager);
        onlyWidget<Gtk::Switch>(dialog).set_active(false);
        onlyWidget<Gtk::ComboBoxText>(dialog).set_active_id("green");
        click(dialog, "重置默认");
        Glib::signal_timeout().connect_once([&dialog]() { dialog.close(); }, 40);
        const auto response = dialog.run();
        require(response == Gtk::RESPONSE_DELETE_EVENT || response == Gtk::RESPONSE_CANCEL,
                "Closing the title bar accepted settings");
        require(manager.isDarkModeEnabled() && manager.getColorTheme() == "red" &&
                readFile(config) == original, "Closing the title bar saved the draft");
    });

    tests.run("saved dark mode and accent are applied when the window is created", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=true\ncolor_theme=red\n");
        MainWindow window(config);
        require(window.get_style_context()->has_class("dark-glass"),
                "Saved dark mode was not selected");
        const auto background = window.get_style_context()->get_background_color(Gtk::STATE_FLAG_NORMAL);
        require(background.get_red() < 0.3 && background.get_green() < 0.3 &&
                background.get_blue() < 0.3, "Saved dark background was not applied");
        const auto accent = statValue(window, "总单词").get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
        require(accent.get_red() > 0.75 && accent.get_red() > accent.get_green() + 0.25 &&
                accent.get_red() > accent.get_blue() + 0.25,
                "Saved red accent was replaced with the default theme");
    });

    tests.run("the word editor imports cards and cancels edits without changing display controls", [] {
        TemporaryDirectory temporary;
        const auto words = temporary.write("words.txt", "apple|n.|苹果|An apple.\n");
        Gtk::Window parent;
        WordManagerDialog dialog(parent);
        dialog.loadWordsFromFile(words);
        dialog.show_all();
        require(button(dialog, "编辑").get_visible() &&
                !button(dialog, "保存").get_visible() && !button(dialog, "取消").get_visible(),
                "Display mode exposed editing controls");
        click(dialog, "编辑");
        const auto entries = widgets<Gtk::Entry>(dialog);
        require(entries.size() == 4 && entries[0]->get_text() == "apple", "Imported card cannot be edited");
        entries[0]->set_text("pear");
        dialog.show_all();
        require(!button(dialog, "编辑").get_visible() &&
                button(dialog, "保存").get_visible() && button(dialog, "取消").get_visible(),
                "Editing mode exposed display controls");
        click(dialog, "取消");
        dialog.show_all();
        require(button(dialog, "编辑").get_visible() && !button(dialog, "保存").get_visible() &&
                !button(dialog, "取消").get_visible(), "Cancel did not restore display controls");
        click(dialog, "编辑");
        require(widgets<Gtk::Entry>(dialog)[0]->get_text() == "apple", "Cancel retained the edited spelling");
        click(dialog, "取消");
        require(readFile(words) == "apple|n.|苹果|An apple.\n", "Editing wrote the vocabulary fixture");
    });

    tests.run("inactive and empty answers cannot create phantom wrong words", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=false\ncolor_theme=blue\n");
        const auto apple = temporary.write("apple.txt", "apple|n.|苹果|An apple.\n");
        const auto pear = temporary.write("pear.txt", "pear|n.|梨|A pear.\n");
        MainWindow window(config);
        auto& stack = onlyWidget<Gtk::Stack>(window);
        auto& entry = onlyWidget<Gtk::Entry>(window);
        click(window, "提交");
        click(window, "查看答案");
        click(window, "下一个单词");
        require(stack.get_visible_child_name() == "placeholder", "Empty session changed pages");
        require(statValue(window, "错词").get_text() == "0" &&
                statValue(window, "已掌握").get_text() == "0", "Empty session recorded answers");

        require(window.loadWordsFromFile(apple), "Apple fixture was rejected");
        entry.set_text("   ");
        click(window, "提交");
        require(statValue(window, "错词").get_text() == "0", "Blank input counted as an error");
        entry.set_text("wrong");
        click(window, "提交");
        require(statValue(window, "错词").get_text() == "1", "Incorrect answer was not recorded");
        entry.set_text("  APPLE\t");
        click(window, "提交");
        require(statValue(window, "已掌握").get_text() == "1", "Trimmed case-insensitive answer failed");
        const auto correctFeedback = feedback(window).get_text();
        entry.set_text("wrong");
        click(window, "提交");
        require(feedback(window).get_text() == correctFeedback, "Completed question accepted another answer");
        require(!window.loadWordsFromFile(temporary.file("missing.txt")), "Missing import succeeded");
        require(statValue(window, "已掌握").get_text() == "1", "Failed import changed the visible progress");
        require(window.loadWordsFromFile(pear), "Pear fixture was rejected");
        require(entry.get_sensitive() && feedback(window).get_text().empty(), "New library retained old answer state");
        require(statValue(window, "错词").get_text() == "0", "New library retained old wrong words");
    });

    tests.run("completion timers respect restarts and replacement libraries", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=false\ncolor_theme=blue\n");
        const auto apple = temporary.write("apple.txt", "apple|n.|苹果\n");
        const auto pear = temporary.write("pear.txt", "pear|n.|梨\n");
        MainWindow window(config);
        auto& stack = onlyWidget<Gtk::Stack>(window);
        auto& entry = onlyWidget<Gtk::Entry>(window);
        require(window.loadWordsFromFile(apple), "Apple fixture was rejected");
        entry.set_text("apple");
        click(window, "提交");
        click(window, "重新开始");
        processEvents(1100);
        require(stack.get_visible_child_name() == "learning" &&
                statValue(window, "已掌握").get_text() == "0", "Old timer completed a restarted session");
        entry.set_text("apple");
        click(window, "提交");
        require(window.loadWordsFromFile(pear), "Pear fixture was rejected");
        processEvents(1100);
        require(stack.get_visible_child_name() == "learning" &&
                statValue(window, "已掌握").get_text() == "0", "Old timer completed a new library");
        entry.set_text("PEAR");
        click(window, "提交");
        processEvents(1100);
        require(stack.get_visible_child_name() == "completion", "Completed session never showed completion");
        click(window, "重新开始");
        require(stack.get_visible_child_name() == "learning" && entry.get_sensitive(),
                "Restart did not restore an answerable question");
    });

    return tests.result();
}
