#ifndef WDLSTPARSER_H
#define WDLSTPARSER_H

#include <string>
#include <vector>
#include "WordManager.h"

class WDLSTParser {
public:
	// 解析WDLST文件
	static bool parseFile(const std::string& filename, std::vector<Word>& words);
	
	// 保存为WDLST文件
	static bool saveFile(const std::string& filename, const std::vector<Word>& words);
	
	// 检查文件是否为WDLST格式
	static bool isWDLSTFile(const std::string& filename);
	
private:
	// 工具函数
	static std::string trim(const std::string& str);
	static std::string unescapeString(const std::string& str);
	static std::string escapeString(const std::string& str);
	
	// 解析函数
	static bool parseWordObject(const std::string& content, size_t& pos, Word& word);
	static bool parseDerivatives(const std::string& content, size_t& pos, std::vector<Word>& derivatives);
	static std::string findValue(const std::string& content, const std::string& key, size_t start_pos = 0);
	
	// 生成函数
	static std::string generateWordObject(const Word& word, int index);
	static std::string generateDerivatives(const std::vector<Word>& derivatives);
};

#endif