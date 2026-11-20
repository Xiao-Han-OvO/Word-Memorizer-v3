#include "MainWindow.h"
#include "GlassTheme.h"
#include "IconHelper.h"
#include <gtkmm/application.h>
#include <iostream>

int main(int argc, char *argv[]) {
    // 必须先初始化 GTK，才能创建主题提供器和其他 GTK 对象。
    auto app = Gtk::Application::create(argc, argv, "com.example.wordmemorizer");

    // 初始化字体
    IconHelper::init_icons();

    // 初始化主题
    GlassTheme::init();
    MainWindow window;

    return app->run(window);
}
