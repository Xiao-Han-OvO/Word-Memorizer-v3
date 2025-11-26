#include "SettingsManager.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>

// 构造函数：初始化配置
SettingsManager::SettingsManager() {
	// 确定配置文件路径
	const char* homeDir = std::getenv("HOME");
	if (homeDir) {
		configFile = std::string(homeDir) + "/.vocabmemster_config";
	} else {
		configFile = ".vocabmemster_config";
	}
	
	// 加载设置
	loadSettings();
	
	// 初始化深色模式
	std::string darkModeSetting = getSetting("dark_mode", "");
	if (darkModeSetting.empty()) {
		darkModeEnabled = detectSystemDarkMode();
	} else {
		darkModeEnabled = (darkModeSetting == "true");
	}
	
	// 初始化颜色主题
	colorTheme = getSetting("color_theme", "blue");
}

// 获取设置值
std::string SettingsManager::getSetting(const std::string& key, const std::string& defaultValue) const {
	auto it = settings.find(key);
	if (it != settings.end()) {
		return it->second;
	}
	return defaultValue;
}

// 设置值
void SettingsManager::setSetting(const std::string& key, const std::string& value) {
	settings[key] = value;
	saveSettings();
}

// 获取深色模式状态
bool SettingsManager::isDarkModeEnabled() const {
	return darkModeEnabled;
}

// 设置深色模式
void SettingsManager::setDarkModeEnabled(bool enabled) {
	darkModeEnabled = enabled;
	setSetting("dark_mode", enabled ? "true" : "false");
}

// 获取颜色主题
std::string SettingsManager::getColorTheme() const {
	return colorTheme;
}

// 设置颜色主题
void SettingsManager::setColorTheme(const std::string& theme) {
	colorTheme = theme;
	setSetting("color_theme", theme);
}

// 检测系统深色模式 - 简化版本，前端会处理真正的检测
bool SettingsManager::detectSystemDarkMode() {
	// 简化实现，返回默认值
	// 在实际使用中，前端会检测系统主题并通知后端
	return false;
}

// 加载设置
void SettingsManager::loadSettings() {
	settings.clear();
	
	std::ifstream file(configFile);
	if (!file.is_open()) {
		// 如果文件不存在，使用默认设置
		applyDefaults();
		return;
	}
	
	std::string line;
	while (std::getline(file, line)) {
		size_t pos = line.find('=');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			settings[key] = value;
		}
	}
	file.close();
}

// 保存设置
void SettingsManager::saveSettings() {
	std::ofstream file(configFile);
	if (!file.is_open()) {
		std::cerr << "无法保存设置到文件: " << configFile << std::endl;
		return;
	}
	
	for (const auto& pair : settings) {
		file << pair.first << "=" << pair.second << std::endl;
	}
	file.close();
}

// 颜色获取方法 - 修复：添加 const
std::string SettingsManager::getPosColor() const {
	return getSetting("pos_color", DEFAULT_POS_COLOR);
}

std::string SettingsManager::getCorrectColor() const {
	return getSetting("correct_color", DEFAULT_CORRECT_COLOR);
}

std::string SettingsManager::getErrorColor() const {
	return getSetting("error_color", DEFAULT_ERROR_COLOR);
}

std::string SettingsManager::getAnswerColor() const {
	return getSetting("answer_color", DEFAULT_ANSWER_COLOR);
}

// 颜色设置方法
void SettingsManager::setPosColor(const std::string& color) {
	setSetting("pos_color", color);
}

void SettingsManager::setCorrectColor(const std::string& color) {
	setSetting("correct_color", color);
}

void SettingsManager::setErrorColor(const std::string& color) {
	setSetting("error_color", color);
}

void SettingsManager::setAnswerColor(const std::string& color) {
	setSetting("answer_color", color);
}

// 主题颜色获取方法 - 修复：添加 const
std::string SettingsManager::getPrimaryColor() const {
	// 根据颜色主题返回对应的主色调
	if (colorTheme == "red") return "#e74c3c";
	else if (colorTheme == "orange") return "#e67e22";
	else if (colorTheme == "green") return "#2ecc71";
	else if (colorTheme == "cyan") return "#1abc9c";
	else if (colorTheme == "yellow") return "#f1c40f";
	else if (colorTheme == "brown") return "#a0522d";
	else if (colorTheme == "mono") return "#7f8c8d";
	else if (colorTheme == "purple") return "#9b59b6";
	else if (colorTheme == "pink") return "#e91e63";
	else return "#3498db"; // 默认蓝色
}

std::string SettingsManager::getSecondaryColor() const {
	// 根据颜色主题返回对应的辅助色调
	if (colorTheme == "red") return "#c0392b";
	else if (colorTheme == "orange") return "#d35400";
	else if (colorTheme == "green") return "#27ae60";
	else if (colorTheme == "cyan") return "#16a085";
	else if (colorTheme == "yellow") return "#f39c12";
	else if (colorTheme == "brown") return "#8b4513";
	else if (colorTheme == "mono") return "#34495e";
	else if (colorTheme == "purple") return "#8e44ad";
	else if (colorTheme == "pink") return "#ad1457";
	else return "#2980b9"; // 默认蓝色
}

std::string SettingsManager::getAccentColor() const {
	// 根据颜色主题返回对应的强调色调
	if (colorTheme == "red") return "#ff6b6b";
	else if (colorTheme == "orange") return "#ffa726";
	else if (colorTheme == "green") return "#4cd964";
	else if (colorTheme == "cyan") return "#4ecdc4";
	else if (colorTheme == "yellow") return "#ffeaa7";
	else if (colorTheme == "brown") return "#d2691e";
	else if (colorTheme == "mono") return "#95a5a6";
	else if (colorTheme == "purple") return "#a29bfe";
	else if (colorTheme == "pink") return "#fd79a8";
	else return "#74b9ff"; // 默认蓝色
}

std::string SettingsManager::getBackgroundColor() const {
	return getThemeBackgroundColor();
}

std::string SettingsManager::getTextColor() const {
	return darkModeEnabled ? "#f7fafc" : "#1a202c";
}

std::string SettingsManager::getCardBackgroundColor() const {
	return getThemeCardBackgroundColor();
}

std::string SettingsManager::getBorderColor() const {
	return getThemeBorderColor();
}

// 主题背景颜色获取 - 修复：添加 const
std::string SettingsManager::getThemeBackgroundColor() const {
	// 根据颜色主题和明暗模式返回对应的背景色
	if (darkModeEnabled) {
		if (colorTheme == "red") return "#161111";
		else if (colorTheme == "orange") return "#1a140f";
		else if (colorTheme == "green") return "#0f160f";
		else if (colorTheme == "cyan") return "#0f1616";
		else if (colorTheme == "yellow") return "#16160f";
		else if (colorTheme == "brown") return "#161310";
		else if (colorTheme == "mono") return "#0a0a0a";
		else if (colorTheme == "purple") return "#161016";
		else if (colorTheme == "pink") return "#160f13";
		else return "#0f141a"; // 默认蓝色深色背景
	} else {
		if (colorTheme == "red") return "#fae6e6";
		else if (colorTheme == "orange") return "#fff4e6";
		else if (colorTheme == "green") return "#e6f7e6";
		else if (colorTheme == "cyan") return "#e6f7f7";
		else if (colorTheme == "yellow") return "#fffde6";
		else if (colorTheme == "brown") return "#f7f3e6";
		else if (colorTheme == "mono") return "#f5f5f5";
		else if (colorTheme == "purple") return "#f3e6f7";
		else if (colorTheme == "pink") return "#fce6f5";
		else return "#f0f8ff"; // 默认蓝色浅色背景
	}
}

std::string SettingsManager::getThemeCardBackgroundColor() const {
	// 卡片背景色比主背景色稍深/稍浅
	if (darkModeEnabled) {
		if (colorTheme == "red") return "#1e1919";
		else if (colorTheme == "orange") return "#221c15";
		else if (colorTheme == "green") return "#151e15";
		else if (colorTheme == "cyan") return "#151e1e";
		else if (colorTheme == "yellow") return "#1e1e15";
		else if (colorTheme == "brown") return "#1e1b16";
		else if (colorTheme == "mono") return "#121212";
		else if (colorTheme == "purple") return "#1e151e";
		else if (colorTheme == "pink") return "#1e1519";
		else return "#151e25"; // 默认蓝色深色卡片背景
	} else {
		return "#ffffff"; // 默认浅色卡片背景保持白色
	}
}

std::string SettingsManager::getThemeBorderColor() const {
	// 边框颜色
	if (darkModeEnabled) {
		if (colorTheme == "red") return "#3a2e2e";
		else if (colorTheme == "orange") return "#3a3225";
		else if (colorTheme == "green") return "#253a25";
		else if (colorTheme == "cyan") return "#253a3a";
		else if (colorTheme == "yellow") return "#3a3a25";
		else if (colorTheme == "brown") return "#3a3525";
		else if (colorTheme == "mono") return "#2a2a2a";
		else if (colorTheme == "purple") return "#3a253a";
		else if (colorTheme == "pink") return "#3a2532";
		else return "#25303a"; // 默认蓝色深色边框
	} else {
		if (colorTheme == "red") return "#e6d1d1";
		else if (colorTheme == "orange") return "#e6d9c6";
		else if (colorTheme == "green") return "#c6e6c6";
		else if (colorTheme == "cyan") return "#c6e6e6";
		else if (colorTheme == "yellow") return "#e6e6c6";
		else if (colorTheme == "brown") return "#e6e0c6";
		else if (colorTheme == "mono") return "#d5d5d5";
		else if (colorTheme == "purple") return "#e0c6e6";
		else if (colorTheme == "pink") return "#e6c6da";
		else return "#c6d9e6"; // 默认蓝色浅色边框
	}
}

// 转换为 JSON - 修复：现在可以调用 const 方法了
nlohmann::json SettingsManager::toJson() const {
	nlohmann::json j;
	
	j["darkMode"] = darkModeEnabled;
	j["colorTheme"] = colorTheme;
	j["posColor"] = getPosColor();
	j["correctColor"] = getCorrectColor();
	j["errorColor"] = getErrorColor();
	j["answerColor"] = getAnswerColor();
	
	// 主题颜色
	j["primaryColor"] = getPrimaryColor();
	j["secondaryColor"] = getSecondaryColor();
	j["accentColor"] = getAccentColor();
	j["backgroundColor"] = getBackgroundColor();
	j["textColor"] = getTextColor();
	j["cardBackgroundColor"] = getCardBackgroundColor();
	j["borderColor"] = getBorderColor();
	
	return j;
}

// 从 JSON 加载
bool SettingsManager::fromJson(const nlohmann::json& j) {
	try {
		if (j.contains("darkMode") && j["darkMode"].is_boolean()) {
			darkModeEnabled = j["darkMode"];
		}
		
		if (j.contains("colorTheme") && j["colorTheme"].is_string()) {
			colorTheme = j["colorTheme"];
		}
		
		if (j.contains("posColor") && j["posColor"].is_string()) {
			setPosColor(j["posColor"]);
		}
		
		if (j.contains("correctColor") && j["correctColor"].is_string()) {
			setCorrectColor(j["correctColor"]);
		}
		
		if (j.contains("errorColor") && j["errorColor"].is_string()) {
			setErrorColor(j["errorColor"]);
		}
		
		if (j.contains("answerColor") && j["answerColor"].is_string()) {
			setAnswerColor(j["answerColor"]);
		}
		
		return true;
	} catch (const std::exception& e) {
		std::cerr << "从JSON加载SettingsManager失败: " << e.what() << std::endl;
		return false;
	}
}

// 应用默认设置
void SettingsManager::applyDefaults() {
	setPosColor(DEFAULT_POS_COLOR);
	setCorrectColor(DEFAULT_CORRECT_COLOR);
	setErrorColor(DEFAULT_ERROR_COLOR);
	setAnswerColor(DEFAULT_ANSWER_COLOR);
	setColorTheme("blue");
	setDarkModeEnabled(false);
}