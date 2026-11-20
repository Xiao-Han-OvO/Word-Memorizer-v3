#include "MainWindow.h"
#include "AboutDialog.h"
#include "UsageDialog.h"
#include "GlassTheme.h"
#include "IconHelper.h"
#include "TestSupport.h"
#include <algorithm>
#include <cmath>
#include <glib/gstdio.h>
#include <fontconfig/fontconfig.h>

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

Gtk::Button& tooltipButton(Gtk::Widget& root, const std::string& tooltip) {
    for (auto* button : all<Gtk::Button>(root)) {
        if (button->get_tooltip_text() == tooltip) return *button;
    }
    throw std::runtime_error("Missing button tooltip: " + tooltip);
}

Gtk::Label& styledLabel(Gtk::Widget& root, const std::string& style) {
    for (auto* label : all<Gtk::Label>(root)) {
        if (label->get_style_context()->has_class(style)) return *label;
    }
    throw std::runtime_error("Missing styled label: " + style);
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
    const auto* pixels = gdk_pixbuf_read_pixels(pixbuf->gobj());
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

Gdk::RGBA iconColor(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf) {
    require(static_cast<bool>(pixbuf), "Icon has no rendered pixels");
    const auto* pixels = gdk_pixbuf_read_pixels(pixbuf->gobj());
    const int channels = pixbuf->get_n_channels();
    double red = 0, green = 0, blue = 0, weight = 0;
    for (int y = 0; y < pixbuf->get_height(); ++y) {
        for (int x = 0; x < pixbuf->get_width(); ++x) {
            const auto* pixel = pixels + y * pixbuf->get_rowstride() + x * channels;
            const double alpha = channels == 4 ? pixel[3] / 255.0 : 1.0;
            red += alpha * pixel[0];
            green += alpha * pixel[1];
            blue += alpha * pixel[2];
            weight += alpha;
        }
    }
    require(weight > 0, "Icon is entirely transparent");
    Gdk::RGBA color;
    color.set_rgba(red / (255 * weight), green / (255 * weight), blue / (255 * weight), 1);
    return color;
}

Gdk::RGBA backgroundBehind(Gtk::Widget& widget) {
    std::vector<Gtk::Widget*> ancestors;
    for (auto* current = &widget; current; current = current->get_parent()) ancestors.push_back(current);
    Gdk::RGBA result("#ffffff");
    for (auto it = ancestors.rbegin(); it != ancestors.rend(); ++it) {
        const auto color = (*it)->get_style_context()->get_background_color((*it)->get_state_flags());
        const double alpha = color.get_alpha();
        result.set_rgba(color.get_red() * alpha + result.get_red() * (1 - alpha),
                        color.get_green() * alpha + result.get_green() * (1 - alpha),
                        color.get_blue() * alpha + result.get_blue() * (1 - alpha), 1);
    }
    return result;
}

void screenshot(Gtk::Window& window, const std::string& name) {
    std::cout << "CAPTURE: " << name << std::endl;
    window.show();
    settle();
    require(static_cast<bool>(window.get_window()), "Window could not be rendered");
    // GTK 直接绘制到新图像，避免读取 X11 窗口像素时触发系统 Cairo 的 MIME 缓存断言。
    const auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
        window.get_allocated_width(), window.get_allocated_height());
    const auto context = Cairo::Context::create(surface);
    gtk_widget_draw(GTK_WIDGET(window.gobj()), context->cobj());
    require(g_mkdir_with_parents("style-previews", 0700) == 0, "Cannot create preview directory");
    surface->write_to_png("style-previews/" + name + ".png");
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
            if (candidate->get_storage_type() != Gtk::IMAGE_PIXBUF) continue;
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
            auto& settingsButton = tooltipButton(window, "设置");
            auto* image = dynamic_cast<Gtk::Image*>(settingsButton.get_image());
            require(image && image->get_storage_type() == Gtk::IMAGE_PIXBUF,
                    "Primary button icon is missing");
            const auto foreground = settingsButton.get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
            const double expected = 255 * (foreground.get_red() + foreground.get_green() +
                                           foreground.get_blue()) / 3;
            require(std::fabs(iconBrightness(image->get_pixbuf()) - expected) < 5,
                    "Primary button icon does not follow its readable text color");
        }
    });

    tests.run("dark action buttons use light text and SVG strokes for every accent", [] {
        TemporaryDirectory temporary;
        for (const auto* accent : {"blue", "red", "orange", "green", "cyan", "yellow", "brown", "mono", "purple", "pink"}) {
            const auto config = temporary.write(std::string("settings-") + accent,
                std::string("dark_mode=true\ncolor_theme=") + accent + "\n");
            MainWindow window(config);
            window.show();
            auto& button = tooltipButton(window, "设置");
            for (const auto state : {Gtk::STATE_FLAG_NORMAL, Gtk::STATE_FLAG_PRELIGHT, Gtk::STATE_FLAG_ACTIVE}) {
                button.set_state_flags(state, true);
                settle(140);
                const auto context = button.get_style_context();
                const auto foreground = context->get_color(button.get_state_flags());
                const auto background = context->get_background_color(button.get_state_flags());
                require(luminance(foreground) > 0.8 && contrast(foreground, background) >= 4.5,
                        std::string("Dark action text is dark or unreadable for accent ") + accent);
                auto* icon = dynamic_cast<Gtk::Image*>(button.get_image());
                require(icon && luminance(iconColor(icon->get_pixbuf())) > 0.8,
                        std::string("Dark action SVG stroke is still dark for accent ") + accent);
            }
        }
    });

    tests.run("every visible main-window SVG follows its foreground through theme and state changes", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=false\ncolor_theme=blue\n");
        MainWindow window(config);
        window.show();
        for (int mode = 0; mode < 3; ++mode) {
            auto& button = tooltipButton(window, "设置");
            for (bool enabled : {true, false}) {
                button.set_sensitive(enabled);
                settle();
                unsigned checked = 0;
                for (auto* icon : all<Gtk::Image>(window)) {
                    if (!icon->get_is_drawable() || icon->get_storage_type() != Gtk::IMAGE_PIXBUF) continue;
                    const auto actual = iconColor(icon->get_pixbuf());
                    const auto expected = icon->get_style_context()->get_color(icon->get_state_flags());
                    require(std::fabs(actual.get_red() - expected.get_red()) < 0.025 &&
                            std::fabs(actual.get_green() - expected.get_green()) < 0.025 &&
                            std::fabs(actual.get_blue() - expected.get_blue()) < 0.025,
                            "A visible SVG retained stale RGB channels");
                    require(contrast(actual, backgroundBehind(*icon)) >= 3,
                            "A visible SVG has insufficient contrast against its actual background");
                    ++checked;
                }
                require(checked >= 11, "The SVG check missed navigation, statistics or action icons");
            }
            button.set_sensitive(true);
            if (mode < 2) gtk_button_clicked(tooltipButton(window, "切换深色模式").gobj());
        }
    });

    tests.run("bundled SF Pro Display fonts are registered and used for English", [] {
        auto* fonts = FcConfigGetFonts(FcConfigGetCurrent(), FcSetApplication);
        bool registered = false;
        if (fonts) for (int i = 0; i < fonts->nfont; ++i) {
            FcChar8* family = nullptr;
            if (FcPatternGetString(fonts->fonts[i], FC_FAMILY, 0, &family) == FcResultMatch &&
                std::string(reinterpret_cast<const char*>(family)) == "SF Pro Display") registered = true;
        }
        require(registered, "Bundled fonts were not registered with Fontconfig");
        TemporaryDirectory temporary;
        MainWindow window(temporary.write("settings", "dark_mode=true\ncolor_theme=blue\n"));
        require(window.loadWordsFromFile(temporary.write("words", "apple|n.|苹果|An apple a day.\n")),
                "Font fixture was rejected");
        window.show();
        settle();
        Gtk::Label* example = nullptr;
        for (auto* label : all<Gtk::Label>(window)) if (label->get_text().find("An apple") != Glib::ustring::npos) example = label;
        require(example, "English example label is missing");
        auto* layout = example->get_layout()->gobj();
        require(pango_layout_get_unknown_glyphs_count(layout) == 0, "Mixed Chinese/English text has missing glyphs");
        auto* iterator = pango_layout_get_iter(layout);
        bool englishUsesSF = false;
        do {
            const auto* run = pango_layout_iter_get_run_readonly(iterator);
            if (!run || run->item->offset >= static_cast<int>(example->get_text().bytes())) continue;
            const unsigned char first = example->get_text().raw()[run->item->offset];
            if (first < 'A' || first > 'z') continue;
            auto* description = pango_font_describe(run->item->analysis.font);
            englishUsesSF = std::string(pango_font_description_get_family(description)) == "SF Pro Display";
            pango_font_description_free(description);
            if (englishUsesSF) break;
        } while (pango_layout_iter_next_run(iterator));
        pango_layout_iter_free(iterator);
        require(englishUsesSF, "English text still uses a hardcoded Sans family");
    });

    tests.run("title text is centered inside the native title bar", [] {
        TemporaryDirectory temporary;
        MainWindow window(temporary.write("settings", "dark_mode=true\ncolor_theme=blue\n"));
        window.show();
        settle();
        auto* header = dynamic_cast<Gtk::HeaderBar*>(window.get_titlebar());
        require(header, "Native header bar is missing");
        Gtk::Label* title = nullptr;
        for (auto* label : all<Gtk::Label>(*header)) if (label->get_text() == window.get_title()) title = label;
        require(title, "Title text is missing");
        int x = 0, y = 0;
        require(title->translate_coordinates(*header, 0, 0, x, y), "Cannot locate title text");
        require(std::abs(2 * x + title->get_allocated_width() - header->get_allocated_width()) <= 2 &&
                std::abs(2 * y + title->get_allocated_height() - header->get_allocated_height()) <= 2,
                "Title text is offset from the center of the title bar");
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

    tests.run("uncomposited desktops keep window decorations compact", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=false\ncolor_theme=blue\n");
        MainWindow window(config);
        window.show();
        settle();
        if (!window.get_screen()->is_composited()) {
            SettingsManager settings(config);
            SettingsDialog preferences(window, settings);
            AboutDialog about(window);
            UsageDialog help(window);
            WordManagerDialog editor(window);
            for (auto* candidate : std::vector<Gtk::Window*>{&window, &preferences, &about, &help, &editor}) {
                candidate->show();
                settle();
                const auto surface = candidate->get_window();
                auto* header = candidate->get_titlebar();
                require(surface && header && surface->get_width() <= header->get_allocated_width() + 16,
                        "An unpainted shadow adds excessive window padding to " + candidate->get_title().raw());
            }
        }
    });

    tests.run("settings controls follow the dialog's dark theme", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=true\ncolor_theme=blue\n");
        MainWindow window(config);
        SettingsManager settings(config);
        SettingsDialog dialog(window, settings);
        require(dialog.get_style_context()->has_class("dark-glass"),
                "Settings dialog did not inherit the active dark theme");
        window.show();
        dialog.show();
        settle();
        const auto context = namedButton(dialog, "取消").get_style_context();
        const auto background = context->get_background_color(Gtk::STATE_FLAG_NORMAL);
        require(luminance(background) < 0.15, "Dialog button retained a bright native background");
        require(contrast(context->get_color(Gtk::STATE_FLAG_NORMAL), background) >= 4.5,
                "Dialog button text is unreadable");
    });

    tests.run("all custom dialogs inherit the theme and native title bars", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=true\ncolor_theme=blue\n");
        MainWindow window(config);
        SettingsManager settings(config);
        SettingsDialog preferences(window, settings);
        AboutDialog about(window);
        UsageDialog help(window);
        WordManagerDialog editor(window);
        for (auto* dialog : std::vector<Gtk::Window*>{&preferences, &about, &help, &editor}) {
            auto* header = dynamic_cast<Gtk::HeaderBar*>(dialog->get_titlebar());
            require(dialog->get_decorated() && header && header->get_show_close_button(),
                    "A custom dialog is missing its native title bar");
            require(header->get_decoration_layout() == ":close" &&
                    dialog->get_style_context()->has_class("dark-glass"),
                    "A dialog retained the old decorations or light theme");
        }
    });

    tests.run("custom learning colors and warning text remain readable when themes change", [] {
        TemporaryDirectory temporary;
        const auto words = temporary.write("words.txt", "apple|n.|苹果|An apple.\n");
        const auto config = temporary.write("settings",
            "dark_mode=false\ncolor_theme=red\npos_color=#ff8800\n"
            "correct_color=#0000ff\nerror_color=#ffff00\nanswer_color=#ff00ff\n");
        MainWindow window(config);
        window.show();
        for (int mode = 0; mode < 2; ++mode) {
            require(window.loadWordsFromFile(words), "Learning color fixture was rejected");
            auto& entry = *all<Gtk::Entry>(window).front();
            entry.set_text("wrong");
            gtk_button_clicked(namedButton(window, "提交").gobj());
            settle();
            const auto background = window.get_style_context()->get_background_color(Gtk::STATE_FLAG_NORMAL);
            const auto error = styledLabel(window, "feedback-label").get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
            require(std::fabs(error.get_red() - error.get_green()) < 0.03 &&
                    error.get_red() > error.get_blue() + 0.2 && contrast(error, background) >= 4.5,
                    "Custom error color was lost or became unreadable");
            gtk_button_clicked(namedButton(window, "下一个单词").gobj());
            settle();
            require(!styledLabel(window, "status-warning").get_text().empty(),
                    "Previously failed word has no warning text");
            const auto warning = styledLabel(window, "status-warning").get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
            require(contrast(warning, background) >= 4.5, "Wrong-word warning became unreadable");
            auto& pos = styledLabel(window, "word-pos");
            const auto posColor = pos.get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
            const auto card = pos.get_parent()->get_style_context()->get_background_color(Gtk::STATE_FLAG_NORMAL);
            require(posColor.get_red() > posColor.get_blue() + 0.2 && contrast(posColor, card) >= 4.5,
                    "Custom part-of-speech color was lost or became unreadable");
            const auto accent = styledLabel(window, "stat-value").get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
            const auto statCard = styledLabel(window, "stat-value").get_parent()->get_style_context()->get_background_color(Gtk::STATE_FLAG_NORMAL);
            require(contrast(accent, statCard) >= 4.5, "Saved accent is unreadable on a statistic card");

            entry.set_text("apple");
            gtk_button_clicked(namedButton(window, "提交").gobj());
            settle();
            const auto correct = styledLabel(window, "feedback-label").get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
            require(correct.get_blue() > correct.get_green() + 0.2 && contrast(correct, background) >= 4.5,
                    "Custom correct-answer color was lost or became unreadable");
            require(window.loadWordsFromFile(words), "Answer color fixture was rejected");
            gtk_button_clicked(namedButton(window, "查看答案").gobj());
            settle();
            const auto answer = styledLabel(window, "feedback-label").get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
            require(answer.get_red() > answer.get_green() + 0.2 &&
                    answer.get_blue() > answer.get_green() + 0.2 && contrast(answer, background) >= 4.5,
                    "Custom revealed-answer color was lost or became unreadable");
            if (mode == 0) {
                gtk_button_clicked(tooltipButton(window, "切换深色模式").gobj());
                settle();
                const auto live = styledLabel(window, "feedback-label").get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL);
                const auto darkBackground = window.get_style_context()->get_background_color(Gtk::STATE_FLAG_NORMAL);
                require(live.get_red() > live.get_green() + 0.2 &&
                        live.get_blue() > live.get_green() + 0.2 && contrast(live, darkBackground) >= 4.5,
                        "Visible feedback did not recolor during a live theme change");
            }
        }
    });

    tests.run("help markup follows the readable theme colors", [] {
        TemporaryDirectory temporary;
        const auto config = temporary.write("settings", "dark_mode=true\ncolor_theme=blue\n");
        MainWindow window(config);
        UsageDialog help(window);
        help.show();
        settle();
        Gtk::Label* content = nullptr;
        for (auto* label : all<Gtk::Label>(help)) {
            if (label->get_text().find("单词记忆大师使用指南") != Glib::ustring::npos) content = label;
        }
        require(content != nullptr, "Help content is missing");
        const auto background = help.get_style_context()->get_background_color(Gtk::STATE_FLAG_NORMAL);
        require(contrast(content->get_style_context()->get_color(Gtk::STATE_FLAG_NORMAL), background) >= 4.5,
                "Help body text is unreadable");
        auto* attributes = pango_layout_get_attributes(content->get_layout()->gobj());
        if (attributes) {
            auto* iterator = pango_attr_list_get_iterator(attributes);
            bool readable = true;
            do {
                auto* attribute = pango_attr_iterator_get(iterator, PANGO_ATTR_FOREGROUND);
                if (!attribute) continue;
                const auto& color = reinterpret_cast<PangoAttrColor*>(attribute)->color;
                Gdk::RGBA foreground;
                foreground.set_rgba(color.red / 65535.0, color.green / 65535.0, color.blue / 65535.0, 1.0);
                readable = readable && contrast(foreground, background) >= 4.5;
            } while (pango_attr_iterator_next(iterator));
            pango_attr_iterator_destroy(iterator);
            require(readable, "Help markup retained an unreadable hardcoded color");
        }
    });

    tests.run("packaged and built-in palettes render consistently", [] {
        TemporaryDirectory temporary;
        std::vector<Gdk::RGBA> backgrounds;
        for (bool builtin : {false, true}) {
            GlassTheme::init(builtin);
            const auto config = temporary.write(builtin ? "builtin-settings" : "packaged-settings",
                "dark_mode=true\ncolor_theme=yellow\n");
            MainWindow window(config);
            backgrounds.push_back(window.get_style_context()->get_background_color(Gtk::STATE_FLAG_NORMAL));
            const auto context = namedButton(window, "提交").get_style_context();
            require(contrast(context->get_color(Gtk::STATE_FLAG_NORMAL),
                             context->get_background_color(Gtk::STATE_FLAG_NORMAL)) >= 4.5,
                    "Built-in primary button colors are unreadable");
        }
        require(std::fabs(backgrounds[0].get_red() - backgrounds[1].get_red()) < 0.001 &&
                std::fabs(backgrounds[0].get_green() - backgrounds[1].get_green()) < 0.001 &&
                std::fabs(backgrounds[0].get_blue() - backgrounds[1].get_blue()) < 0.001,
                "Built-in background differs from the packaged theme");
        GlassTheme::init();
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
                screenshot(dialog, mode + "-settings");
            }
            {
                AboutDialog dialog(window);
                screenshot(dialog, mode + "-about");
            }
            {
                UsageDialog dialog(window);
                screenshot(dialog, mode + "-help");
            }
            {
                WordManagerDialog dialog(window);
                dialog.loadWordsFromFile(words);
                screenshot(dialog, mode + "-editor");
            }
            auto& entry = *all<Gtk::Entry>(window).front();
            entry.set_text("wrong");
            gtk_button_clicked(namedButton(window, "提交").gobj());
            screenshot(window, mode + "-error");
            entry.set_text("apple");
            gtk_button_clicked(namedButton(window, "提交").gobj());
            settle(1100);
            screenshot(window, mode + "-completion");
        }
    });

    return tests.result();
}
