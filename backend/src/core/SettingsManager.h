#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <string>
#include <map>
#include <nlohmann/json.hpp>

// 设置管理器类
class SettingsManager {
private:
	std::string configFile;
	std::map<std::string, std::string> settings;
	
	// 默认颜色
	const std::string DEFAULT_POS_COLOR = "rgb(52,152,219)";
	const std::string DEFAULT_CORRECT_COLOR = "rgb(46,204,113)";
	const std::string DEFAULT_ERROR_COLOR = "rgb(231,76,60)";
	const std::string DEFAULT_ANSWER_COLOR = "rgb(155,89,182)";
	
	// 主题相关
	bool darkModeEnabled;
	std::string colorTheme;

public:
	SettingsManager();
	
	// 配置文件操作
	void loadSettings();
	void saveSettings();
	std::string getSetting(const std::string& key, const std::string& defaultValue = "") const;  // 添加 const
	void setSetting(const std::string& key, const std::string& value);
	
	// 颜色相关的便捷方法 - 修复：添加 const
	std::string getPosColor() const;
	std::string getCorrectColor() const;
	std::string getErrorColor() const;
	std::string getAnswerColor() const;
	void setPosColor(const std::string& color);
	void setCorrectColor(const std::string& color);
	void setErrorColor(const std::string& color);
	void setAnswerColor(const std::string& color);
	
	// 主题相关方法
	bool isDarkModeEnabled() const;
	void setDarkModeEnabled(bool enabled);
	std::string getColorTheme() const;
	void setColorTheme(const std::string& theme);
	
	// 系统检测
	bool detectSystemDarkMode();
	
	// 主题颜色获取方法 - 修复：添加 const
	std::string getPrimaryColor() const;
	std::string getSecondaryColor() const;
	std::string getAccentColor() const;
	std::string getBackgroundColor() const;
	std::string getTextColor() const;
	std::string getCardBackgroundColor() const;
	std::string getBorderColor() const;

	// 主题背景颜色获取 - 修复：添加 const
	std::string getThemeBackgroundColor() const;
	std::string getThemeCardBackgroundColor() const;
	std::string getThemeBorderColor() const;
	
	// JSON 序列化
	nlohmann::json toJson() const;
	bool fromJson(const nlohmann::json& j);
	
	// 应用默认设置
	void applyDefaults();
};

#endif