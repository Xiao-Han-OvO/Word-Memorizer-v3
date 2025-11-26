#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow() 
	: mainBox(Gtk::ORIENTATION_VERTICAL) {
	
	setupWindow();
	setupHeaderBar();
	setupMenu();
	setupWebView();
	
	show_all_children();
}

MainWindow::~MainWindow() {}

void MainWindow::setupWindow() {
	set_title("VocabMemster");
	set_default_size(1000, 700);
	set_border_width(0);
	
	// 添加主容器
	add(mainBox);
}

void MainWindow::setupHeaderBar() {
	// 设置标题栏
	headerBar.set_title("VocabMemster");
	headerBar.set_subtitle("单词记忆大师");
	headerBar.set_show_close_button(true);
	
	// 创建菜单按钮
	menuButton.set_image_from_icon_name("open-menu-symbolic");
	menuButton.set_tooltip_text("菜单");
	
	headerBar.pack_end(menuButton);
	set_titlebar(headerBar);
}

void MainWindow::setupMenu() {
	// 创建动作组
	actionGroup = Gio::SimpleActionGroup::create();
	
	// 添加动作
	actionGroup->add_action("open", sigc::mem_fun(*this, &MainWindow::on_action_open));
	actionGroup->add_action("settings", sigc::mem_fun(*this, &MainWindow::on_action_settings));
	actionGroup->add_action("about", sigc::mem_fun(*this, &MainWindow::on_action_about));
	actionGroup->add_action("usage", sigc::mem_fun(*this, &MainWindow::on_action_usage));
	actionGroup->add_action("quit", sigc::mem_fun(*this, &MainWindow::on_action_quit));
	
	insert_action_group("app", actionGroup);
	
	// 创建菜单模型
	menuModel = Gio::Menu::create();
	
	// 文件菜单
	auto fileSection = Gio::Menu::create();
	fileSection->append("打开单词库", "app.open");
	fileSection->append("设置", "app.settings");
	fileSection->append("退出", "app.quit");
	menuModel->append_section(fileSection);
	
	// 帮助菜单
	auto helpSection = Gio::Menu::create();
	helpSection->append("使用说明", "app.usage");
	helpSection->append("关于", "app.about");
	menuModel->append_section(helpSection);
	
	// 设置弹出菜单
	appMenuButton.set_menu_model(menuModel);
	appMenuButton.set_image_from_icon_name("open-menu-symbolic");
	appMenuButton.set_direction(Gtk::ARROW_DOWN);
	
	headerBar.pack_end(appMenuButton);
}

void MainWindow::setupWebView() {
	// 创建 WebView 桥接
	webViewBridge = std::make_unique<WebViewBridge>(wordManager, settingsManager, *this);
	
	// 初始化 WebView
	webViewBridge->initializeWebView(&mainBox);
}

bool MainWindow::on_key_press_event(GdkEventKey* event) {
	// 处理全局快捷键
	if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_Tab) {
		// Ctrl+Tab - 下一个单词
		webViewBridge->sendToFrontend("nextWord", "{}");
		return true;
	}
	else if ((event->state & GDK_MOD1_MASK) && event->keyval == GDK_KEY_s) {
		// Alt+S - 显示答案
		webViewBridge->sendToFrontend("showAnswer", "{}");
		return true;
	}
	else if ((event->state & GDK_CONTROL_MASK) && (event->state & GDK_SHIFT_MASK) && 
			(event->keyval == GDK_KEY_d || event->keyval == GDK_KEY_D)) {
		// Ctrl+Shift+D - 切换主题
		webViewBridge->sendToFrontend("toggleTheme", "{}");
		return true;
	}
	
	return Gtk::Window::on_key_press_event(event);
}

void MainWindow::on_action_open() {
	// 创建文件选择对话框
	Gtk::FileChooserDialog dialog("选择单词库文件", Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_transient_for(*this);
	
	// 添加按钮
	dialog.add_button("取消", Gtk::RESPONSE_CANCEL);
	dialog.add_button("打开", Gtk::RESPONSE_OK);
	
	// 添加文件过滤器
	auto filterText = Gtk::FileFilter::create();
	filterText->set_name("文本文件 (*.txt)");
	filterText->add_pattern("*.txt");
	dialog.add_filter(filterText);
	
	auto filterWdlst = Gtk::FileFilter::create();
	filterWdlst->set_name("WDLST 文件 (*.wdlst)");
	filterWdlst->add_pattern("*.wdlst");
	dialog.add_filter(filterWdlst);
	
	auto filterAll = Gtk::FileFilter::create();
	filterAll->set_name("所有文件");
	filterAll->add_pattern("*");
	dialog.add_filter(filterAll);
	
	// 显示对话框
	int result = dialog.run();
	
	if (result == Gtk::RESPONSE_OK) {
		std::string filename = dialog.get_filename();
		webViewBridge->sendToFrontend("loadWordsFile", "{\"filename\":\"" + filename + "\"}");
	}
}

void MainWindow::on_action_settings() {
	webViewBridge->sendToFrontend("openSettings", "{}");
}

void MainWindow::on_action_about() {
	webViewBridge->sendToFrontend("openAbout", "{}");
}

void MainWindow::on_action_usage() {
	webViewBridge->sendToFrontend("openUsage", "{}");
}

void MainWindow::on_action_quit() {
	hide();
}

// 原有的信号处理函数（保持兼容性）
void MainWindow::on_open_file() {
	on_action_open();
}

void MainWindow::on_edit_words() {
	webViewBridge->sendToFrontend("openWordManager", "{}");
}

void MainWindow::on_settings() {
	on_action_settings();
}

void MainWindow::on_about() {
	on_action_about();
}

void MainWindow::on_usage() {
	on_action_usage();
}

void MainWindow::on_export_wrong_words() {
	Gtk::FileChooserDialog dialog("导出错词本", Gtk::FILE_CHOOSER_ACTION_SAVE);
	dialog.set_transient_for(*this);
	
	dialog.add_button("取消", Gtk::RESPONSE_CANCEL);
	dialog.add_button("保存", Gtk::RESPONSE_OK);
	
	dialog.set_current_name("wrong_words.txt");
	
	int result = dialog.run();
	
	if (result == Gtk::RESPONSE_OK) {
		std::string filename = dialog.get_filename();
		webViewBridge->sendToFrontend("exportWrongWords", "{\"filename\":\"" + filename + "\"}");
	}
}

void MainWindow::on_reset_progress() {
	webViewBridge->sendToFrontend("resetProgress", "{}");
}

void MainWindow::on_quit() {
	on_action_quit();
}