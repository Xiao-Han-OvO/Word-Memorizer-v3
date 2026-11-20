#include "MainWindow.h"
#include "AboutDialog.h"
#include "UsageDialog.h"
#include "GlassTheme.h"
#include "IconHelper.h"
#include "TestSupport.h"
#include <algorithm>
#include <cmath>
#include <glib/gstdio.h>

template<class T>
void collect(Gtk::Widget& root, std::vector<T*>& found) {
    if (auto* widget = dynamic_cast<T*>(&root)) found.push_back(widget);
    if (auto* container = dynamic_cast<Gtk::Container*>(&root)) {
        const auto children = container->get_children();
        for (auto* child : children) collect(*child, found);
        if (auto* window = dynamic_cast<Gtk::Window*>(&root)) {
            auto* titlebar = window->get_titlebar();
            if (titlebar && std::find(children.begin(), children.end(), titlebar) == children.end()) {
                collect(*titlebar, found);
            }
        }
    }
}

template<class T>
std::vector<T*> all(Gtk::Widget& root) {
    std::vector<T*> found;
    collect(root, found);
    return found;
}

void settle(unsigned milliseconds = 220) {
    auto loop = Glib::MainLoop::create();
    Glib::signal_timeout().connect_once([loop]() { loop->quit(); }, milliseconds);
    loop->run();
}

Gtk::Button& namedButton(Gtk::Widget& root, const std::string& label) {
    for (auto* button : all<Gtk::Button>(root)) {
        if (button->get_label() == label) return *button;
    }
    throw std::runtime_error("Missing button: " + label);
}

double luminance(const Gdk::RGBA& color) {
    const auto linear = [](double value) {
        return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * linear(color.get_red()) + 0.7152 * linear(color.get_green()) +
           0.0722 * linear(color.get_blue());
}

double contrast(const Gdk::RGBA& first, const Gdk::RGBA& second) {
    const double a = luminance(first), b = luminance(second);
    return (std::max(a, b) + 0.05) / (std::min(a, b) + 0.05);
}

double iconBrightness(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf) {
    require(static_cast<bool>(pixbuf), "Icon has no rendered pixels");
    const auto* pixels = pixbuf->get_pixels();
    const int channels = pixbuf->get_n_channels();
    double brightness = 0.0, weight = 0.0;
    for (int y = 0; y < pixbuf->get_height(); ++y) {
        for (int x = 0; x < pixbuf->get_width(); ++x) {
            const auto* pixel = pixels + y * pixbuf->get_rowstride() + x * channels;
            const double alpha = channels == 4 ? pixel[3] / 255.0 : 1.0;
            brightness += alpha * (pixel[0] + pixel[1] + pixel[2]) / 3.0;
            weight += alpha;
        }
    }
    require(weight > 0.0, "Icon is entirely transparent");
    return brightness / weight;
}

void screenshot(Gtk::Window& window, const std::string& name) {
    window.show();
    settle();
    const auto surface = window.get_window();
    require(static_cast<bool>(surface), "Window could not be rendered");
    auto pixels = Gdk::Pixbuf::create(surface, 0, 0, surface->get_width(), surface->get_height());
    require(static_cast<bool>(pixels), "Window capture failed");
    require(g_mkdir_with_parents("style-previews", 0700) == 0, "Cannot create preview directory");
    pixels->save("style-previews/" + name + ".png", "png");
}

int main() {
    auto app = Gtk::Application::create("com.example.wordmemorizer.styleregressions");
    IconHelper::init_icons();
    GlassTheme::init();
    TestRunner tests;

    tests.run("dark mode selects the native GTK dark variant", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=true\ncolor_theme=blue\n");
        auto gtkSettings = Gtk::Settings::get_default();
        gtkSettings->property_gtk_application_prefer_dark_theme() = false;
        MainWindow window(config);
        require(gtkSettings->property_gtk_application_prefer_dark_theme().get_value(),
                "Native controls are still using the light theme");
    });

    tests.run("existing navigation icons recolor in both directions", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=false\ncolor_theme=blue\n");
        MainWindow window(config);
        window.show();
        settle();
        Gtk::Image* icon = nullptr;
        for (auto* candidate : all<Gtk::Image>(window)) {
            const auto pixels = candidate->get_pixbuf();
            if (pixels && pixels->get_width() == 20) { icon = candidate; break; }
        }
        require(icon != nullptr, "Navigation icon was not found");
        Gtk::Button* toggle = nullptr;
        for (auto* candidate : all<Gtk::Button>(window)) {
            if (candidate->get_tooltip_text() == "切换深色模式") toggle = candidate;
        }
        require(toggle != nullptr, "Theme toggle was not found");
        const double light = iconBrightness(icon->get_pixbuf());
        gtk_button_clicked(toggle->gobj());
        settle();
        const double dark = iconBrightness(icon->get_pixbuf());
        require(dark > light + 80, "Existing SVG pixels did not change in dark mode");
        gtk_button_clicked(toggle->gobj());
        settle();
        require(std::fabs(iconBrightness(icon->get_pixbuf()) - light) < 5,
                "Icon did not return to its light-mode color");
    });

    tests.run("bright accent colors keep primary buttons readable", [] {
        TemporaryDirectory temporary;
        for (const auto& mode : {std::string("false"), std::string("true")}) {
            const auto config = temporary.write("settings-" + mode,
                "dark_mode=" + mode + "\ncolor_theme=yellow\n");
            MainWindow window(config);
            window.show();
            settle();
            const auto context = namedButton(window, "提交").get_style_context();
            require(contrast(context->get_color(Gtk::STATE_FLAG_NORMAL),
                             context->get_background_color(Gtk::STATE_FLAG_NORMAL)) >= 4.5,
                    "Yellow buttons have insufficient text contrast");
        }
    });

    tests.run("the main window uses a complete GTK header bar", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=false\ncolor_theme=blue\n");
        MainWindow window(config);
        auto* header = dynamic_cast<Gtk::HeaderBar*>(window.get_titlebar());
        require(window.get_decorated() && header, "Title bar is still a manually dragged content row");
        require(header->get_show_close_button(), "Native window controls are hidden");
        require(header->get_decoration_layout() == ":minimize,maximize,close",
                "Minimize, maximize and close controls are incomplete");
        require(!header->get_has_subtitle(), "Empty subtitle adds unused title-bar space");
    });

    tests.run("settings controls follow the dialog's dark theme", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=true\ncolor_theme=blue\n");
        MainWindow window(config);
        SettingsManager settings(config);
        SettingsDialog dialog(window, settings);
        dialog.get_style_context()->add_class("dark-glass");
        window.show();
        dialog.show();
        settle();
        const auto context = namedButton(dialog, "取消").get_style_context();
        const auto background = context->get_background_color(Gtk::STATE_FLAG_NORMAL);
        require(luminance(background) < 0.15, "Dialog button retained a bright native background");
        require(contrast(context->get_color(Gtk::STATE_FLAG_NORMAL), background) >= 4.5,
                "Dialog button text is unreadable");
    });

    tests.run("light and dark pages and dialogs render for visual review", [] {
        TemporaryDirectory temporary;
        const auto words = temporary.write("words.txt", "apple|n.|苹果|An apple a day.\n");
        for (const auto& mode : {std::string("light"), std::string("dark")}) {
            const bool dark = mode == "dark";
            const auto config = temporary.write("settings-" + mode,
                std::string("dark_mode=") + (dark ? "true" : "false") + "\ncolor_theme=blue\n");
            MainWindow window(config);
            screenshot(window, mode + "-placeholder");
            require(window.loadWordsFromFile(words), "Visual fixture could not be loaded");
            screenshot(window, mode + "-learning");
            SettingsManager settings(config);
            {
                SettingsDialog dialog(window, settings);
                dialog.get_style_context()->add_class(dark ? "dark-glass" : "light-glass");
                screenshot(dialog, mode + "-settings");
            }
            {
                AboutDialog dialog(window);
                dialog.get_style_context()->add_class(dark ? "dark-glass" : "light-glass");
                screenshot(dialog, mode + "-about");
            }
            {
                UsageDialog dialog(window);
                dialog.get_style_context()->add_class(dark ? "dark-glass" : "light-glass");
                screenshot(dialog, mode + "-help");
            }
        }
    });

    return tests.result();
}
