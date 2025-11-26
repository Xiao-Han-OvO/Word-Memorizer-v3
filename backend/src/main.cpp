#include "ui/MainWindow.h"
#include <gtkmm/application.h>
#include <iostream>

int main(int argc, char *argv[]) {
	// 初始化 GTK 应用
	auto app = Gtk::Application::create(argc, argv, "com.vocabmemster.app");
	
	try {
		// 创建并显示主窗口
		MainWindow window;
		
		// 运行应用
		return app->run(window);
		
	} catch (const std::exception& e) {
		std::cerr << "应用程序错误: " << e.what() << std::endl;
		return 1;
	}
}