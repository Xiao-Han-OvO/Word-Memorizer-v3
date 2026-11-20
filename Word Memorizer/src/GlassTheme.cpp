#include "GlassTheme.h"
#include "ThemeResources.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <linux/limits.h>
#include <vector>
#include <fontconfig/fontconfig.h>
#include <pango/pangocairo.h>

Glib::RefPtr<Gtk::CssProvider> GlassTheme::css_provider;
Glib::RefPtr<Gdk::Screen> GlassTheme::screen;
sigc::connection GlassTheme::compositor_connection;
std::string GlassTheme::base_css;
std::string GlassTheme::dark_css;
bool GlassTheme::dark_mode = false;

namespace {
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

Gdk::RGBA mix(const Gdk::RGBA& color, const Gdk::RGBA& target, double amount) {
    Gdk::RGBA mixed;
    mixed.set_rgba(color.get_red() * (1 - amount) + target.get_red() * amount,
                   color.get_green() * (1 - amount) + target.get_green() * amount,
                   color.get_blue() * (1 - amount) + target.get_blue() * amount, 1.0);
    return mixed;
}

Gdk::RGBA darken_for_text(const Gdk::RGBA& background, const Gdk::RGBA& text, double minimum) {
    if (contrast(background, text) >= minimum) return background;
    double low = 0, high = 1;
    const Gdk::RGBA black("#000000");
    for (int i = 0; i < 20; ++i) {
        const double amount = (low + high) / 2;
        if (contrast(mix(background, black, amount), text) < minimum) low = amount;
        else high = amount;
    }
    return mix(background, black, high);
}

void register_fonts(const std::vector<std::string>& directories) {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    bool loaded = false;
    for (const auto& directory : directories) {
        bool found = false;
        for (const auto* style : {"Regular", "Semibold", "Bold", "RegularItalic"}) {
            const std::string filename = directory + "/fonts/SF-Pro-Display-" + style + ".otf";
            if (access(filename.c_str(), R_OK) == 0 &&
                FcConfigAppFontAddFile(FcConfigGetCurrent(), reinterpret_cast<const FcChar8*>(filename.c_str()))) {
                found = loaded = true;
            }
        }
        if (found) break;
    }
    // 更新当前进程的字体映射；不安装字体，也不写入用户的字体缓存。
    if (loaded) pango_font_map_changed(pango_cairo_font_map_get_default());
}
}

std::string GlassTheme::load_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    std::stringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

void GlassTheme::init(bool use_builtin) {
    if (screen && css_provider) {
        Gtk::StyleContext::remove_provider_for_screen(screen, css_provider);
    }

    char buffer[PATH_MAX];
    const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    std::string executable_directory = ".";
    if (length != -1) {
        buffer[length] = '\0';
        executable_directory = buffer;
        executable_directory = executable_directory.substr(0, executable_directory.rfind('/'));
    }

    const std::vector<std::string> candidates = {
        executable_directory + "/resources",
        executable_directory + "/../resources",
        "resources",
        "/usr/local/share/word_memorizer/resources"
    };
    register_fonts(candidates);

    bool loaded = false;
    if (!use_builtin) {
        for (const auto& directory : candidates) {
            base_css = load_file(directory + "/style.css");
            dark_css = load_file(directory + "/style-dark.css");
            if (!base_css.empty() && !dark_css.empty()) {
                std::cerr << "Loaded CSS from: " << directory << '\n';
                loaded = true;
                break;
            }
        }
    }
    if (!loaded) {
        base_css = ThemeResources::base_css;
        dark_css = ThemeResources::dark_css;
        std::cerr << "Using built-in CSS\n";
    }

    css_provider = Gtk::CssProvider::create();
    if (screen) {
        Gtk::StyleContext::add_provider_for_screen(screen, css_provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

Gdk::RGBA GlassTheme::readable_color(const std::string& requested) {
    Gdk::RGBA color;
    if (!color.set(requested)) color.set(dark_mode ? "#f1f5f9" : "#1a202c");
    color.set_alpha(1.0);
    const Gdk::RGBA background(dark_mode ? "#303744" : "#edf1f7");
    if (contrast(color, background) >= 4.6) return color;

    const Gdk::RGBA target(dark_mode ? "#ffffff" : "#000000");
    double low = 0.0, high = 1.0;
    for (int i = 0; i < 20; ++i) {
        const double amount = (low + high) / 2.0;
        if (contrast(mix(color, target, amount), background) < 4.6) low = amount;
        else high = amount;
    }
    return mix(color, target, high);
}

void GlassTheme::apply_theme(bool dark, const std::string& accent_color) {
    if (!css_provider) init();
    dark_mode = dark;

    if (auto settings = Gtk::Settings::get_default()) {
        settings->property_gtk_application_prefer_dark_theme() = dark;
    }

    Gdk::RGBA accent;
    if (!accent.set(accent_color)) accent.set("#3498db");
    accent.set_alpha(1.0);
    const Gdk::RGBA accent_foreground = readable_color(accent.to_string());
    const bool white_text = dark || contrast(accent, Gdk::RGBA("#ffffff")) >= 4.5;
    const Gdk::RGBA text(dark ? "#f1f5f9" : (white_text ? "#ffffff" : "#000000"));
    // 深色页面使用浅色文字和图标，调整按钮底色而不是把前景切换成黑色。
    if (dark) accent = darken_for_text(accent, text, 5.6);
    Gdk::RGBA hover = mix(accent, Gdk::RGBA(dark || !white_text ? "#ffffff" : "#000000"), 0.08);
    Gdk::RGBA active = mix(accent, Gdk::RGBA(dark || !white_text ? "#ffffff" : "#000000"), 0.16);
    if (dark) {
        hover = darken_for_text(hover, text, 5.0);
        active = darken_for_text(active, text, 4.6);
    }
    std::string combined =
        "@define-color wm_accent " + accent.to_string() + ";\n" +
        "@define-color wm_accent_text " + text.to_string() + ";\n" +
        "@define-color wm_accent_fg " + accent_foreground.to_string() + ";\n" +
        "@define-color wm_accent_hover " + hover.to_string() + ";\n" +
        "@define-color wm_accent_active " + active.to_string() + ";\n" + base_css;
    if (dark) combined += "\n" + dark_css;

    try {
        css_provider->load_from_data(combined);
    } catch (const Gtk::CssProviderError& error) {
        std::cerr << "CSS Error: " << error.what() << '\n';
    }

    for (auto* window : Gtk::Window::list_toplevels()) style_window(*window);
}

void GlassTheme::set_screen(const Glib::RefPtr<Gdk::Screen>& new_screen) {
    if (screen == new_screen) return;
    if (screen && css_provider) Gtk::StyleContext::remove_provider_for_screen(screen, css_provider);
    compositor_connection.disconnect();
    screen = new_screen;
    if (screen) {
        compositor_connection = screen->signal_composited_changed().connect([]() {
            for (auto* window : Gtk::Window::list_toplevels()) style_window(*window);
        });
    }
    if (screen && css_provider) {
        Gtk::StyleContext::add_provider_for_screen(screen, css_provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

void GlassTheme::style_window(Gtk::Window& window) {
    auto context = window.get_style_context();
    context->add_class("word-memorizer");
    context->remove_class("light-glass");
    context->remove_class("dark-glass");
    context->add_class(dark_mode ? "dark-glass" : "light-glass");
    const auto window_screen = window.get_screen();
    if (window_screen && !window_screen->is_composited()) context->add_class("uncomposited");
    else context->remove_class("uncomposited");
    if (auto* header = dynamic_cast<Gtk::HeaderBar*>(window.get_titlebar())) {
        header->get_style_context()->add_class("custom-titlebar");
    }
}

void GlassTheme::configure_window(Gtk::Window& window, const Glib::ustring& title, bool main_window) {
    auto* header = Gtk::manage(new Gtk::HeaderBar());
    header->set_title(title);
    header->set_has_subtitle(false);
    header->set_show_close_button(true);
    header->set_decoration_layout(main_window ? ":minimize,maximize,close" : ":close");
    header->get_style_context()->add_class("custom-titlebar");
    auto* label = Gtk::manage(new Gtk::Label(title));
    label->get_style_context()->add_class("window-title");
    label->set_halign(Gtk::ALIGN_CENTER);
    label->set_valign(Gtk::ALIGN_CENTER);
    label->set_single_line_mode(true);
    label->set_ellipsize(Pango::ELLIPSIZE_END);
    header->set_custom_title(*label);
    window.set_title(title);
    window.set_decorated(true);
    window.set_titlebar(*header);
    header->show_all();
    style_window(window);
}

void GlassTheme::add_provider(const std::string& css_data) {
    auto provider = Gtk::CssProvider::create();
    try {
        provider->load_from_data(css_data);
    } catch (const Gtk::CssProviderError& error) {
        std::cerr << "CSS Error: " << error.what() << '\n';
        return;
    }
    if (screen) {
        Gtk::StyleContext::add_provider_for_screen(screen, provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}
