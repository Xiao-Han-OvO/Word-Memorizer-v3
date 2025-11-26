#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtkmm.h>
#include <webkit2/webkit2.h>
#include "core/WordManager.h"
#include "core/SettingsManager.h"
#include "bridge/WebViewBridge.h"

class MainWindow : public Gtk::Window {
private:
	// 核心组件
	WordManager wordManager;
	SettingsManager settingsManager;
	std::unique_ptr<WebViewBridge> webViewBridge;
	
	// UI 组件
	Gtk::Box mainBox;
	Gtk::HeaderBar headerBar;
	Gtk::Button menuButton;
	Gtk::MenuButton appMenuButton;
	Gtk::PopoverMenu appMenu;
	
	// 菜单项
	Glib::RefPtr<Gio::Menu> menuModel;
	Glib::RefPtr<Gio::SimpleActionGroup> actionGroup;
	
public:
	MainWindow();
	virtual ~MainWindow();
	
protected:
	// 键盘事件处理
	virtual bool on_key_press_event(GdkEventKey* event) override;

private:
	// 初始化方法
	void setupWindow();
	void setupHeaderBar();
	void setupMenu();
	void setupWebView();
	
	// 信号处理函数
	void on_open_file();
	void on_edit_words();
	void on_settings();
	void on_about();
	void on_usage();
	void on_export_wrong_words();
	void on_reset_progress();
	void on_quit();
	
	// 动作处理函数
	void on_action_open();
	void on_action_settings();
	void on_action_about();
	void on_action_usage();
	void on_action_quit();
};

#endif