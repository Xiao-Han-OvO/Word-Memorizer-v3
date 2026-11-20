#include "AboutDialog.h"
#include "GlassTheme.h"
#include <iostream>

// 构造函数：初始化关于对话框
AboutDialog::AboutDialog(Gtk::Window& parent)
   : Dialog("关于", parent, true),
   mainBox(Gtk::ORIENTATION_VERTICAL) {
   
	GlassTheme::configure_window(*this, "关于");
	set_default_size(400, 300);
	mainBox.set_border_width(20);
	set_resizable(false);
	
	// 设置标题
	titleLabel.set_label("VocabMemorizer");
	titleLabel.override_font(Pango::FontDescription("Sans Bold 20"));
	titleLabel.set_margin_bottom(10);
	titleLabel.get_style_context()->add_class("title");
	
	// 版本信息
	versionLabel.set_label("Ver 3.0.1 Alpha RC 2");
	versionLabel.override_font(Pango::FontDescription("Sans 12"));
	versionLabel.set_margin_bottom(10);
	versionLabel.get_style_context()->add_class("subtitle");
	
	// 描述
	descriptionLabel.set_label("简约高效的单词背诵工具");
	descriptionLabel.set_justify(Gtk::JUSTIFY_CENTER);
	descriptionLabel.set_line_wrap(true);
	descriptionLabel.set_max_width_chars(40);
	descriptionLabel.set_margin_bottom(15);
	descriptionLabel.get_style_context()->add_class("muted");
	
	// 作者信息
	authorLabel.set_label("开发者: Federico Prask");
	authorLabel.override_font(Pango::FontDescription("Sans 10"));
	authorLabel.set_margin_bottom(10);
	authorLabel.get_style_context()->add_class("muted");
	
	// 许可证信息
	licenseLabel.set_label("许可证: MIT License");
	licenseLabel.override_font(Pango::FontDescription("Sans 10"));
	licenseLabel.set_margin_bottom(20);
	licenseLabel.get_style_context()->add_class("muted");
	
	// 关闭按钮
	closeButton.set_label("关闭");
	closeButton.set_size_request(100, 35);
	closeButton.get_style_context()->add_class("primary");
	
	// 布局
	mainBox.set_spacing(5);
	mainBox.pack_start(titleLabel, Gtk::PACK_SHRINK);
	mainBox.pack_start(versionLabel, Gtk::PACK_SHRINK);
	mainBox.pack_start(descriptionLabel, Gtk::PACK_SHRINK);
	mainBox.pack_start(authorLabel, Gtk::PACK_SHRINK);
	mainBox.pack_start(licenseLabel, Gtk::PACK_SHRINK);
	mainBox.pack_start(closeButton, Gtk::PACK_SHRINK);
	
	// 为主盒子添加卡片样式
	mainBox.get_style_context()->add_class("card");
	
	get_content_area()->pack_start(mainBox);
	
	// 连接信号
	closeButton.signal_clicked().connect([this]() {
		response(Gtk::RESPONSE_OK);
	});
	
	show_all_children();
}

// 析构函数
AboutDialog::~AboutDialog() {}
