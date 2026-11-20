#ifndef GLASSTHEME_H
#define GLASSTHEME_H

#include <gtkmm.h>
#include <string>

class GlassTheme {
public:
    static void init(bool use_builtin = false);
    static void apply_theme(bool dark, const std::string& accent_color);
    static void set_screen(const Glib::RefPtr<Gdk::Screen>& screen);
    static void add_provider(const std::string& css_data);
    static void configure_window(Gtk::Window& window, const Glib::ustring& title, bool main_window = false);
    static void style_window(Gtk::Window& window);
    static Gdk::RGBA readable_color(const std::string& color);
    
private:
    static Glib::RefPtr<Gtk::CssProvider> css_provider;
    static Glib::RefPtr<Gdk::Screen> screen;
    static sigc::connection compositor_connection;
    static std::string base_css;
    static std::string dark_css;
    static bool dark_mode;
    
    static std::string load_file(const std::string& path);
};

#endif
