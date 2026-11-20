#include "MainWindow.h"
#include "GlassTheme.h"
#include <fstream>
#include <iostream>
#include <sstream>

// 单词卡片类
class WordCard : public Gtk::Frame {
private:
	Gtk::Box mainBox;
	Gtk::Box contentBox;
	Gtk::Box buttonBox;
	
	// 显示模式组件
	Gtk::Box displayBox;
	Gtk::Label wordLabel;
	Gtk::Label posLabel;
	Gtk::Label meaningLabel;
	Gtk::Label exampleLabel;
	Gtk::Button editButton;
	
	// 编辑模式组件
	Gtk::Box editBox;
	Gtk::Entry wordEntry;
	Gtk::Entry posEntry;
	Gtk::Entry meaningEntry;
	Gtk::Entry exampleEntry;
	Gtk::Button saveButton;
	Gtk::Button cancelButton;
	Gtk::Button deleteButton;
	
	bool isEditing;
	WordManagerDialog* parentDialog;
    
public:
	WordCard(WordManagerDialog* parent, 
				const std::string& word = "", 
				const std::string& pos = "", 
				const std::string& meaning = "", 
				const std::string& example = "");
	
	std::string getWord() const;
	std::string getPos() const;
	std::string getMeaning() const;
	std::string getExample() const;
	bool getIsEditing() const;
    
private:
	void switchToEditMode();
	void switchToDisplayMode();
	void onEditClicked();
	void onSaveClicked();
	void onCancelClicked();
	void onDeleteClicked();
};

WordCard::WordCard(WordManagerDialog* parent, 
						const std::string& word, 
						const std::string& pos, 
						const std::string& meaning, 
						const std::string& example)
	: isEditing(false), parentDialog(parent) {
	
	set_shadow_type(Gtk::SHADOW_NONE);
	set_border_width(15);
	get_style_context()->add_class("card");
	
	// 主布局
	mainBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
	mainBox.set_spacing(12);
	
	// 内容区域
	contentBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
	contentBox.set_spacing(8);
	
	// 显示模式
	displayBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
	displayBox.set_spacing(6);
	
	// 单词标题 - 使用CSS类而不是硬编码颜色
	wordLabel.set_label(word);
	wordLabel.set_halign(Gtk::ALIGN_START);
	wordLabel.override_font(Pango::FontDescription("Bold 16"));
	wordLabel.get_style_context()->add_class("title");
	
	// 词性标签 - 使用muted类
	posLabel.set_label("[" + pos + "]");
	posLabel.set_halign(Gtk::ALIGN_START);
	posLabel.override_font(Pango::FontDescription("12"));
	posLabel.get_style_context()->add_class("muted");
	
	// 释义
	meaningLabel.set_label(meaning);
	meaningLabel.set_halign(Gtk::ALIGN_START);
	meaningLabel.override_font(Pango::FontDescription("14"));
	meaningLabel.get_style_context()->add_class("subtitle");
	
	// 例句 - 使用muted类
	exampleLabel.set_label("例句: " + example);
	exampleLabel.set_halign(Gtk::ALIGN_START);
	exampleLabel.override_font(Pango::FontDescription("12"));
	exampleLabel.set_line_wrap(true);
	exampleLabel.set_max_width_chars(50);
	exampleLabel.get_style_context()->add_class("muted");
	
	displayBox.pack_start(wordLabel, Gtk::PACK_SHRINK);
	displayBox.pack_start(posLabel, Gtk::PACK_SHRINK);
	displayBox.pack_start(meaningLabel, Gtk::PACK_SHRINK);
	displayBox.pack_start(exampleLabel, Gtk::PACK_SHRINK);
	
	// 编辑模式
	editBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
	editBox.set_spacing(8);
	
	wordEntry.set_placeholder_text("英文单词");
	wordEntry.set_text(word);
	
	posEntry.set_placeholder_text("词性 (n., v., adj., etc.)");
	posEntry.set_text(pos);
	
	meaningEntry.set_placeholder_text("中文释义");
	meaningEntry.set_text(meaning);
	
	exampleEntry.set_placeholder_text("例句");
	exampleEntry.set_text(example);
	
	editBox.pack_start(wordEntry, Gtk::PACK_SHRINK);
	editBox.pack_start(posEntry, Gtk::PACK_SHRINK);
	editBox.pack_start(meaningEntry, Gtk::PACK_SHRINK);
	editBox.pack_start(exampleEntry, Gtk::PACK_SHRINK);
	
	// 按钮区域
	buttonBox.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	buttonBox.set_spacing(8);
	buttonBox.set_halign(Gtk::ALIGN_END);
	
	editButton.set_label("编辑");
	saveButton.set_label("保存");
	cancelButton.set_label("取消");
	deleteButton.set_label("删除");
	
	buttonBox.pack_start(deleteButton, Gtk::PACK_SHRINK);
	buttonBox.pack_start(cancelButton, Gtk::PACK_SHRINK);
	buttonBox.pack_start(saveButton, Gtk::PACK_SHRINK);
	buttonBox.pack_start(editButton, Gtk::PACK_SHRINK);
	
	// 初始布局
	contentBox.pack_start(displayBox, Gtk::PACK_SHRINK);
	mainBox.pack_start(contentBox, Gtk::PACK_SHRINK);
	mainBox.pack_start(buttonBox, Gtk::PACK_SHRINK);
	
	add(mainBox);
	
	// 连接信号
	editButton.signal_clicked().connect(sigc::mem_fun(*this, &WordCard::onEditClicked));
	saveButton.signal_clicked().connect(sigc::mem_fun(*this, &WordCard::onSaveClicked));
	cancelButton.signal_clicked().connect(sigc::mem_fun(*this, &WordCard::onCancelClicked));
	deleteButton.signal_clicked().connect(sigc::mem_fun(*this, &WordCard::onDeleteClicked));
	
	// 初始状态
	switchToDisplayMode();
}

std::string WordCard::getWord() const {
	return isEditing ? wordEntry.get_text() : wordLabel.get_text().substr(4);
}

std::string WordCard::getPos() const {
	return isEditing ? posEntry.get_text() : posLabel.get_text().substr(4);
}

std::string WordCard::getMeaning() const {
	return isEditing ? meaningEntry.get_text() : meaningLabel.get_text().substr(4);
}

std::string WordCard::getExample() const {
	return isEditing ? exampleEntry.get_text() : exampleLabel.get_text().substr(4);
}

bool WordCard::getIsEditing() const {
	return isEditing;
}

void WordCard::switchToEditMode() {
	isEditing = true;
	
	// 移除显示组件，添加编辑组件
	if (displayBox.get_parent() == &contentBox) contentBox.remove(displayBox);
	if (editBox.get_parent() != &contentBox) contentBox.pack_start(editBox, Gtk::PACK_SHRINK);
	
	// 显示保存和取消按钮，隐藏编辑按钮
	editButton.set_no_show_all(true);
	editButton.hide();
	saveButton.set_no_show_all(false);
	saveButton.show();
	cancelButton.set_no_show_all(false);
	cancelButton.show();
	
	show_all_children();
}

void WordCard::switchToDisplayMode() {
	isEditing = false;
	
	// 更新显示标签
	wordLabel.set_label("单词: " + wordEntry.get_text());
	posLabel.set_label("词性: " + posEntry.get_text());
	meaningLabel.set_label("释义: " + meaningEntry.get_text());
	exampleLabel.set_label("例句: " + exampleEntry.get_text());
	
	// 移除编辑组件，添加显示组件
	if (editBox.get_parent() == &contentBox) contentBox.remove(editBox);
	if (displayBox.get_parent() != &contentBox) contentBox.pack_start(displayBox, Gtk::PACK_SHRINK);
	
	// 显示编辑按钮，隐藏保存和取消按钮
	editButton.set_no_show_all(false);
	editButton.show();
	saveButton.set_no_show_all(true);
	saveButton.hide();
	cancelButton.set_no_show_all(true);
	cancelButton.hide();
	
	show_all_children();
}

void WordCard::onEditClicked() {
	switchToEditMode();
}

void WordCard::onSaveClicked() {
	// 验证输入
	if (wordEntry.get_text().empty() || posEntry.get_text().empty() || meaningEntry.get_text().empty()) {
		Gtk::MessageDialog dialog(*parentDialog, "请填写单词、词性和释义", false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
		dialog.run();
		return;
	}
	
	switchToDisplayMode();
}

void WordCard::onCancelClicked() {
	// 恢复原始值
	std::string originalWord = wordLabel.get_text().substr(4);
	std::string originalPos = posLabel.get_text().substr(4);
	std::string originalMeaning = meaningLabel.get_text().substr(4);
	std::string originalExample = exampleLabel.get_text().substr(4);
	
	wordEntry.set_text(originalWord);
	posEntry.set_text(originalPos);
	meaningEntry.set_text(originalMeaning);
	exampleEntry.set_text(originalExample);
	
	switchToDisplayMode();
}

// 在 WordCard 类的 onDeleteClicked 方法中，添加正确的删除逻辑：
// void WordCard::onDeleteClicked() {
//     Gtk::MessageDialog dialog(*parentDialog, "确定要删除这个单词吗？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
//     if (dialog.run() == Gtk::RESPONSE_YES) {
//         // 隐藏卡片而不是删除，让父对话框处理删除逻辑
//         hide();
//         // 可以添加一个信号来通知父对话框这个卡片应该被删除
//     }
// }

// 修改删除按钮的处理函数
void WordCard::onDeleteClicked() {
	Gtk::MessageDialog dialog(*parentDialog, "确定要删除这个单词吗？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
	if (dialog.run() == Gtk::RESPONSE_YES) {
		// 通知父对话框删除这个卡片
		parentDialog->removeWordCard(this);
	}
}

// WordManagerDialog 实现
WordManagerDialog::WordManagerDialog(Gtk::Window& parent)
	: Dialog("单词管理", parent, true),
	mainBox(Gtk::ORIENTATION_VERTICAL),
	wordsContainer(Gtk::ORIENTATION_VERTICAL) {
	
	GlassTheme::configure_window(*this, "单词管理");
	set_default_size(700, 550);
	
	// 创建简约工具栏
	Gtk::Box* toolbar = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
	toolbar->set_spacing(10);
	toolbar->set_border_width(10);
	
	// 简约按钮
	Gtk::Button* addButton = Gtk::manage(new Gtk::Button("+ 添加"));
	addButton->set_tooltip_text("添加新单词");
	
	Gtk::Button* saveButton = Gtk::manage(new Gtk::Button("💾 保存"));
	saveButton->set_tooltip_text("保存单词库");
	
	Gtk::Button* closeButton = Gtk::manage(new Gtk::Button("✕ 关闭"));
	closeButton->set_tooltip_text("关闭");
	
	// 将按钮添加到工具栏
	toolbar->pack_start(*addButton, Gtk::PACK_SHRINK);
	toolbar->pack_start(*saveButton, Gtk::PACK_SHRINK);
	toolbar->pack_end(*closeButton, Gtk::PACK_SHRINK);
	
	// 滚动窗口
	scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scrolledWindow.add(wordsContainer);
	
	// 主布局
	mainBox.set_spacing(0);
	mainBox.pack_start(*toolbar, Gtk::PACK_SHRINK);
	mainBox.pack_start(scrolledWindow, Gtk::PACK_EXPAND_WIDGET);
	
	get_content_area()->pack_start(mainBox);
	
	// 连接信号
	addButton->signal_clicked().connect(sigc::mem_fun(*this, &WordManagerDialog::on_add_button_clicked));
	saveButton->signal_clicked().connect(sigc::mem_fun(*this, &WordManagerDialog::on_save_all_clicked));
	closeButton->signal_clicked().connect(sigc::mem_fun(*this, &WordManagerDialog::on_close_clicked));
	
	show_all_children();
}

WordManagerDialog::~WordManagerDialog() {
	// 清理所有单词卡片
	for (Gtk::Widget* card : wordCards) {
		wordsContainer.remove(*card);
		// 注意：由于卡片是 Gtk::manage 的，不需要手动 delete
	}
	wordCards.clear();
}

void WordManagerDialog::loadWordsFromFile(const std::string& filename) {
	currentFilename = filename;
	
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cout << "无法打开文件: " << filename << std::endl;
		return;
	}
	
	std::string line;
	while (std::getline(file, line)) {
		if (line.empty()) continue;
		
		std::stringstream ss(line);
		std::string word, pos, meaning, example;
		
		if (std::getline(ss, word, '|') && 
			std::getline(ss, pos, '|') &&
			std::getline(ss, meaning, '|')) {
			
			std::getline(ss, example);
			addWordCard(word, pos, meaning, example);
		}
	}
	
	file.close();
}

void WordManagerDialog::saveWordsToFile(const std::string& filename) {
	std::ofstream file(filename);
	if (!file.is_open()) {
		Gtk::MessageDialog dialog(*this, "无法保存文件", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
		dialog.run();
		return;
	}
	
	for (Gtk::Widget* widget : wordCards) {
		WordCard* card = dynamic_cast<WordCard*>(widget);
		if (card && !card->getIsEditing()) {
			file << card->getWord() << "|" << card->getPos() << "|" 
				<< card->getMeaning() << "|" << card->getExample() << std::endl;
		}
	}
	
	file.close();
	
	Gtk::MessageDialog dialog(*this, "单词已保存到文件", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
	dialog.run();
}

void WordManagerDialog::on_add_button_clicked() {
	addWordCard(); // 添加空白卡片
}


void WordManagerDialog::on_remove_button_clicked() {
	if (wordCards.empty()) {
		Gtk::MessageDialog dialog(*this, "没有单词可以删除", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
		dialog.run();
		return;
	}
	
	// 删除最后一个单词卡片
	//  Gtk::Widget* lastCard = wordCards.back();
	//  removeWordCard(lastCard);

	Gtk::MessageDialog dialog(*this, "请使用单词卡片上的删除按钮", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
	dialog.run();
}

void WordManagerDialog::on_save_all_clicked() {
	// 首先检查是否有卡片正在编辑
	for (Gtk::Widget* widget : wordCards) {
		WordCard* card = dynamic_cast<WordCard*>(widget);
		if (card && card->getIsEditing()) {
			Gtk::MessageDialog dialog(*this, "请先完成所有编辑操作", false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
			dialog.run();
			return;
		}
	}
	
	// 选择保存文件
	Gtk::FileChooserDialog dialog("保存单词库", Gtk::FILE_CHOOSER_ACTION_SAVE);
	dialog.set_transient_for(*this);
	
	dialog.add_button("取消", Gtk::RESPONSE_CANCEL);
	dialog.add_button("保存", Gtk::RESPONSE_OK);
	
	auto filter_text = Gtk::FileFilter::create();
	filter_text->set_name("文本文件");
	filter_text->add_pattern("*.txt");
	dialog.add_filter(filter_text);
	
	// 设置默认文件名
	if (!currentFilename.empty()) {
		dialog.set_filename(currentFilename);
	} else {
		dialog.set_current_name("my_words.txt");
	}
	
	int result = dialog.run();
	
	if (result == Gtk::RESPONSE_OK) {
		std::string filename = dialog.get_filename();
		if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") {
			filename += ".txt";
		}
		saveWordsToFile(filename);
	}
}

void WordManagerDialog::on_close_clicked() {
	response(Gtk::RESPONSE_OK);
}

void WordManagerDialog::addWordCard(const std::string& word, const std::string& pos, 
											const std::string& meaning, const std::string& example) {
	WordCard* card = Gtk::manage(new WordCard(this, word, pos, meaning, example));
	
	// 设置卡片之间的间距
	card->set_margin_top(3);
	card->set_margin_bottom(3);
	card->set_margin_start(5);
	card->set_margin_end(5);
	
	wordsContainer.pack_start(*card, Gtk::PACK_SHRINK);
	wordCards.push_back(card);
	show_all_children();
}

void WordManagerDialog::removeWordCard(Gtk::Widget* card) {
	// 从容器中移除卡片
	wordsContainer.remove(*card);
	
	// 从向量中删除指针
	auto it = std::find(wordCards.begin(), wordCards.end(), card);
	if (it != wordCards.end()) {
		wordCards.erase(it);
	}
	
	// 由于卡片是 Gtk::manage 的，不需要手动 delete
	// 刷新显示
	show_all_children();
}
