#include "WordManager.h"
#include "WDLSTParser.h"
#include <iostream>
#include <filesystem>

// 构造函数：初始化随机数生成器
WordManager::WordManager() : rng(std::random_device{}()) {}

// 从文件加载单词
bool WordManager::loadWordsFromFile(const std::string& filename) {
	words.clear();
	wrongWords.clear();
	masteredWords.clear();
	failedWords.clear();
	
	bool success = false;
	
	// 检查文件是否存在
	if (!std::filesystem::exists(filename)) {
		std::cout << "文件不存在: " << filename << std::endl;
		return false;
	}
	
	// 根据文件扩展名选择解析器
	if (WDLSTParser::isWDLSTFile(filename)) {
		std::cout << "检测到WDLST文件格式，使用WDLST解析器" << std::endl;
		success = WDLSTParser::parseFile(filename, words);
	} else {
		std::cout << "使用传统TXT文件解析器" << std::endl;
		// 原有的TXT文件解析逻辑
		std::ifstream file(filename);
		if (!file.is_open()) {
			return false;
		}

		std::string line;
		while (std::getline(file, line)) {
			if (line.empty()) continue;
			
			std::stringstream ss(line);
			std::string word, pos, meaning, example;
			
			// 解析单词数据格式：单词|词性|释义|例句
			if (std::getline(ss, word, '|') && 
					std::getline(ss, pos, '|') &&
					std::getline(ss, meaning, '|')) {
					
					std::getline(ss, example);
					
					Word w;
					w.word = word;
					w.pos = pos;
					w.meaning = meaning;
					w.example = example;
					words.push_back(w);
			}
		}
		
		file.close();
		success = !words.empty();
	}
	
	std::cout << "成功加载 " << words.size() << " 个单词" << std::endl;
	return success;
}

// 保存单词到文件
bool WordManager::saveWordsToFile(const std::string& filename, const std::vector<Word>& customWords) {
	const std::vector<Word>& wordsToSave = customWords.empty() ? words : customWords;
	
	if (WDLSTParser::isWDLSTFile(filename)) {
		return WDLSTParser::saveFile(filename, wordsToSave);
	} else {
		std::ofstream file(filename);
		if (!file.is_open()) {
			return false;
		}
		
		for (const auto& word : wordsToSave) {
			file << word.word << "|" << word.pos << "|" << word.meaning << "|" << word.example << std::endl;
		}
		
		file.close();
		return true;
	}
}

// 获取随机单词（从未掌握的单词中）
Word WordManager::getRandomWord() {
	if (words.empty() || allWordsMastered()) {
		return Word{"", "", "", ""};
	}
	
	// 只从未掌握的单词中随机选择
	std::vector<Word> remainingWords;
	for (const auto& word : words) {
		if (masteredWords.find(word.word) == masteredWords.end()) {
			remainingWords.push_back(word);
		}
	}
	
	if (remainingWords.empty()) {
		return Word{"", "", "", ""};
	}
	
	std::uniform_int_distribution<size_t> dist(0, remainingWords.size() - 1);
	return remainingWords[dist(rng)];
}

// 添加错词
bool WordManager::addWrongWord(const Word& word) {
	// 检查是否已经存在该错词
	if (wrongWords.find(word) != wrongWords.end()) {
		std::cout << "错词已存在: " << word.word << std::endl;
		return false;
	}
	
	wrongWords.insert(word);
	std::cout << "添加错词: " << word.word << "，当前错词数量: " << wrongWords.size() << std::endl;
	return true;
}

// 添加失败单词
void WordManager::addFailedWord(const std::string& word) {
	failedWords.insert(word);
}

// 添加已掌握单词
void WordManager::addMasteredWord(const std::string& word) {
	masteredWords.insert(word);
	// 如果单词被掌握，就从失败列表中移除
	failedWords.erase(word);
}

// 获取所有错词
std::vector<Word> WordManager::getWrongWords() const {
	std::vector<Word> result;
	result.reserve(wrongWords.size());
	
	for (const auto& word : wrongWords) {
		result.push_back(word);
	}
	
	return result;
}

// 获取所有单词
std::vector<Word> WordManager::getAllWords() const {
	return words;
}

// 获取学习统计
LearningStats WordManager::getStats() const {
	LearningStats stats;
	stats.totalWords = getTotalWords();
	stats.masteredWords = getMasteredWordsCount();
	stats.wrongWords = getWrongWordsCount();
	stats.remainingWords = getRemainingWordsCount();
	stats.progress = getProgress();
	return stats;
}

// 清空错词本
void WordManager::clearWrongWords() {
	wrongWords.clear();
}

// 清空已掌握单词
void WordManager::clearMasteredWords() {
	masteredWords.clear();
}

// 重置所有进度
void WordManager::resetProgress() {
	clearWrongWords();
	clearMasteredWords();
	failedWords.clear();
}

// 获取统计信息
size_t WordManager::getTotalWords() const {
	return words.size();
}

size_t WordManager::getWrongWordsCount() const {
	return wrongWords.size();
}

size_t WordManager::getMasteredWordsCount() const {
	return masteredWords.size();
}

size_t WordManager::getRemainingWordsCount() const {
	return words.size() - masteredWords.size();
}

// 检查是否所有单词都已掌握
bool WordManager::allWordsMastered() const {
	return !words.empty() && masteredWords.size() >= words.size();
}

// 获取学习进度
float WordManager::getProgress() const {
	if (words.empty()) return 0.0f;
	return static_cast<float>(masteredWords.size()) / words.size();
}

// 检查单词是否曾经失败
bool WordManager::hasFailed(const std::string& word) const {
	return failedWords.find(word) != failedWords.end();
}

// 转换为 JSON
nlohmann::json WordManager::toJson() const {
	nlohmann::json j;
	
	// 序列化单词列表
	nlohmann::json wordsJson = nlohmann::json::array();
	for (const auto& word : words) {
		wordsJson.push_back(word.toJson());
	}
	j["words"] = wordsJson;
	
	// 序列化错词
	nlohmann::json wrongWordsJson = nlohmann::json::array();
	for (const auto& word : wrongWords) {
		wrongWordsJson.push_back(word.toJson());
	}
	j["wrongWords"] = wrongWordsJson;
	
	// 序列化已掌握单词
	nlohmann::json masteredWordsJson = nlohmann::json::array();
	for (const auto& word : masteredWords) {
		masteredWordsJson.push_back(word);
	}
	j["masteredWords"] = masteredWordsJson;
	
	// 序列化失败单词
	nlohmann::json failedWordsJson = nlohmann::json::array();
	for (const auto& word : failedWords) {
		failedWordsJson.push_back(word);
	}
	j["failedWords"] = failedWordsJson;
	
	return j;
}

// 从 JSON 加载
bool WordManager::fromJson(const nlohmann::json& j) {
	try {
		// 清空现有数据
		words.clear();
		wrongWords.clear();
		masteredWords.clear();
		failedWords.clear();
		
		// 解析单词列表
		if (j.contains("words") && j["words"].is_array()) {
			for (const auto& wordJson : j["words"]) {
						words.push_back(Word::fromJson(wordJson));
			}
		}
		
		// 解析错词
		if (j.contains("wrongWords") && j["wrongWords"].is_array()) {
			for (const auto& wordJson : j["wrongWords"]) {
						wrongWords.insert(Word::fromJson(wordJson));
			}
		}
		
		// 解析已掌握单词
		if (j.contains("masteredWords") && j["masteredWords"].is_array()) {
			for (const auto& word : j["masteredWords"]) {
						masteredWords.insert(word.get<std::string>());
			}
		}
		
		// 解析失败单词
		if (j.contains("failedWords") && j["failedWords"].is_array()) {
			for (const auto& word : j["failedWords"]) {
						failedWords.insert(word.get<std::string>());
			}
		}
		
		return true;
	} catch (const std::exception& e) {
		std::cerr << "从JSON加载WordManager失败: " << e.what() << std::endl;
		return false;
	}
}