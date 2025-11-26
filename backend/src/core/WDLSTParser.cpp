#include "WDLSTParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cctype>

// 检查文件是否为WDLST格式
bool WDLSTParser::isWDLSTFile(const std::string& filename) {
	if (filename.length() < 6) return false; // 至少要有 ".wdlst"
	std::string ext = filename.substr(filename.length() - 6);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	return ext == ".wdlst";
}

// 去除字符串两端空白字符
std::string WDLSTParser::trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\n\r");
	if (start == std::string::npos) return "";
	size_t end = str.find_last_not_of(" \t\n\r");
	return str.substr(start, end - start + 1);
}

// 解析WDLST文件
bool WDLSTParser::parseFile(const std::string& filename, std::vector<Word>& words) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "无法打开WDLST文件: " << filename << std::endl;
		return false;
	}
	
	// 读取整个文件内容
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	file.close();
	
	// 查找 words 数组
	size_t words_start = content.find("\"words\"");
	if (words_start == std::string::npos) {
		std::cerr << "无效的WDLST格式: 未找到words字段" << std::endl;
		return false;
	}
	
	size_t array_start = content.find(':', words_start);
	array_start = content.find('[', array_start);
	if (array_start == std::string::npos) {
		std::cerr << "无效的WDLST格式: 未找到words数组" << std::endl;
		return false;
	}
	
	size_t pos = array_start + 1;
	words.clear();
	int word_count = 0;
	
	// 解析每个单词对象
	while (pos < content.length()) {
		// 跳过空白字符
		while (pos < content.length() && std::isspace(content[pos])) pos++;
		if (pos >= content.length()) break;
		
		// 检查是否到达数组结束
		if (content[pos] == ']') break;
		
		// 跳过逗号和空白
		if (content[pos] == ',') pos++;
		while (pos < content.length() && std::isspace(content[pos])) pos++;
		
		// 查找数字键和对象开始
		size_t key_start = content.find('"', pos);
		if (key_start == std::string::npos) break;
		
		size_t key_end = content.find('"', key_start + 1);
		if (key_end == std::string::npos) break;
		
		// 验证键是否为数字
		std::string key = content.substr(key_start + 1, key_end - key_start - 1);
		if (!std::all_of(key.begin(), key.end(), ::isdigit)) {
			std::cerr << "警告: 非数字键: " << key << std::endl;
		}
		
		size_t obj_start = content.find('[', key_end);
		if (obj_start == std::string::npos) break;
		
		// 解析单词对象
		Word word;
		if (parseWordObject(content, obj_start, word)) {
			words.push_back(word);
			word_count++;
		}
		
		pos = obj_start + 1;
	}
	
	std::cout << "成功解析 " << word_count << " 个单词" << std::endl;
	return !words.empty();
}

// 解析单个单词对象
bool WDLSTParser::parseWordObject(const std::string& content, size_t& pos, Word& word) {
	// 跳过空白字符
	while (pos < content.length() && std::isspace(content[pos])) pos++;
	if (pos >= content.length() || content[pos] != '[') return false;
	
	pos++; // 跳过 '['
	
	// 解析单词字段
	word.word = trim(unescapeString(findValue(content, "word", pos)));
	word.pos = trim(unescapeString(findValue(content, "pos", pos)));
	word.meaning = trim(unescapeString(findValue(content, "meaning", pos)));
	
	// 注意：新格式中使用 "sentence" 而不是 "example"
	std::string sentence = trim(unescapeString(findValue(content, "sentence", pos)));
	word.example = sentence; // 保持兼容性，将sentence存储到example字段
	
	// 解析派生词
	size_t extend_start = content.find("\"extend\"", pos);
	if (extend_start != std::string::npos) {
		size_t extend_array_start = content.find('[', extend_start);
		if (extend_array_start != std::string::npos) {
			parseDerivatives(content, extend_array_start, word.derivatives);
		}
	}
	
	// 查找对象结束
	pos = content.find(']', pos);
	if (pos != std::string::npos) pos++;
	
	// 验证必要字段
	if (word.word.empty() || word.meaning.empty()) {
		std::cerr << "警告: 单词或释义为空，跳过此单词" << std::endl;
		return false;
	}
	
	return true;
}

// 解析派生词数组
bool WDLSTParser::parseDerivatives(const std::string& content, size_t& pos, std::vector<Word>& derivatives) {
	// 跳过空白字符
	while (pos < content.length() && std::isspace(content[pos])) pos++;
	if (pos >= content.length() || content[pos] != '[') return false;
	
	pos++; // 跳过 '['
	derivatives.clear();
	int derivative_count = 0;
	
	while (pos < content.length()) {
		// 跳过空白字符
		while (pos < content.length() && std::isspace(content[pos])) pos++;
		if (pos >= content.length()) break;
		
		// 检查是否到达数组结束
		if (content[pos] == ']') break;
		
		// 跳过逗号和空白
		if (content[pos] == ',') pos++;
		while (pos < content.length() && std::isspace(content[pos])) pos++;
		
		// 查找数字键和对象开始
		size_t key_start = content.find('"', pos);
		if (key_start == std::string::npos) break;
		
		size_t key_end = content.find('"', key_start + 1);
		if (key_end == std::string::npos) break;
		
		size_t obj_start = content.find('[', key_end);
		if (obj_start == std::string::npos) break;
		
		// 解析派生词对象
		Word derivative;
		if (parseWordObject(content, obj_start, derivative)) {
			derivatives.push_back(derivative);
			derivative_count++;
		}
		
		pos = obj_start + 1;
	}
	
	// 跳过结束的 ']'
	if (pos < content.length() && content[pos] == ']') pos++;
	
	std::cout << "解析到 " << derivative_count << " 个派生词" << std::endl;
	return true;
}

// 在内容中查找键对应的值
std::string WDLSTParser::findValue(const std::string& content, const std::string& key, size_t start_pos) {
	std::string search_key = "\"" + key + "\"";
	size_t key_pos = content.find(search_key, start_pos);
	if (key_pos == std::string::npos) return "";
	
	size_t colon_pos = content.find(':', key_pos);
	if (colon_pos == std::string::npos) return "";
	
	// 查找值的开始（第一个引号）
	size_t value_start = content.find('"', colon_pos);
	if (value_start == std::string::npos) return "";
	
	// 查找值的结束（下一个引号）
	size_t value_end = content.find('"', value_start + 1);
	if (value_end == std::string::npos) return "";
	
	return content.substr(value_start + 1, value_end - value_start - 1);
}

// 去除字符串转义
std::string WDLSTParser::unescapeString(const std::string& str) {
	std::string result;
	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] == '\\' && i + 1 < str.length()) {
			switch (str[i + 1]) {
					case 'n': result += '\n'; break;
					case 't': result += '\t'; break;
					case 'r': result += '\r'; break;
					case '\\': result += '\\'; break;
					case '"': result += '"'; break;
					default: result += str[i + 1]; break;
			}
			i++; // 跳过下一个字符
		} else {
			result += str[i];
		}
	}
	return result;
}

// 转义字符串
std::string WDLSTParser::escapeString(const std::string& str) {
	std::string result;
	for (char c : str) {
		switch (c) {
			case '\n': result += "\\n"; break;
			case '\t': result += "\\t"; break;
			case '\r': result += "\\r"; break;
			case '\\': result += "\\\\"; break;
			case '"': result += "\\\""; break;
			default: result += c; break;
		}
	}
	return result;
}

// 保存为WDLST文件
bool WDLSTParser::saveFile(const std::string& filename, const std::vector<Word>& words) {
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "无法创建WDLST文件: " << filename << std::endl;
		return false;
	}
	
	file << "{\n";
	file << "\t\"words\" : [\n";
	
	for (size_t i = 0; i < words.size(); i++) {
		file << generateWordObject(words[i], i + 1);
		if (i < words.size() - 1) {
			file << ",";
		}
		file << "\n";
	}
	
	file << "\t]\n";
	file << "}\n";
	
	file.close();
	std::cout << "成功保存 " << words.size() << " 个单词到 " << filename << std::endl;
	return true;
}

// 生成单词对象字符串
std::string WDLSTParser::generateWordObject(const Word& word, int index) {
	std::stringstream ss;
	ss << "\t\t\"" << index << "\" :[\n";
	ss << "\t\t\t\"word\" : \"" << escapeString(word.word) << "\",\n";
	ss << "\t\t\t\"pos\" : \"" << escapeString(word.pos) << "\",\n";
	ss << "\t\t\t\"meaning\" : \"" << escapeString(word.meaning) << "\",\n";
	ss << "\t\t\t\"sentence\" : \"" << escapeString(word.example) << "\"";
	
	if (!word.derivatives.empty()) {
		ss << ",\n";
		ss << "\t\t\t\"extend\" : " << generateDerivatives(word.derivatives);
	} else {
		ss << "\n";
	}
	
	ss << "\t\t]";
	return ss.str();
}

// 生成派生词数组字符串
std::string WDLSTParser::generateDerivatives(const std::vector<Word>& derivatives) {
	std::stringstream ss;
	ss << "[\n";
	
	for (size_t i = 0; i < derivatives.size(); i++) {
		ss << "\t\t\t\t\"" << (i + 1) << "\" : [\n";
		ss << "\t\t\t\t\t\"word\" : \"" << escapeString(derivatives[i].word) << "\",\n";
		ss << "\t\t\t\t\t\"pos\" : \"" << escapeString(derivatives[i].pos) << "\",\n";
		ss << "\t\t\t\t\t\"meaning\" : \"" << escapeString(derivatives[i].meaning) << "\"\n";
		ss << "\t\t\t\t]";
		
		if (i < derivatives.size() - 1) {
			ss << ",";
		}
		ss << "\n";
	}
	
	ss << "\t\t\t]";
	return ss.str();
}