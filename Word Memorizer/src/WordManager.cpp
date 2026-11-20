#include "WordManager.h"
#include <random>
#include <iostream>

namespace {
std::string trim(const std::string& text) {
	const auto first = text.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) return "";
	const auto last = text.find_last_not_of(" \t\r\n");
	return text.substr(first, last - first + 1);
}
}

// 构造函数：初始化随机数生成器
WordManager::WordManager() : rng(std::random_device{}()) {}

// 从文件加载单词
bool WordManager::loadWordsFromFile(const std::string& filename) {
	std::ifstream file(filename);
	
	if (!file.is_open()) {
		return false;
	}

	std::vector<Word> loadedWords;
	std::string line;
	bool firstLine = true;
	while (std::getline(file, line)) {
		if (firstLine && line.compare(0, 3, "\xef\xbb\xbf") == 0) {
			line.erase(0, 3);
		}
		firstLine = false;
		if (trim(line).empty()) continue;
		
		std::stringstream ss(line);
		std::string word, pos, meaning, example;
		
		// 解析单词数据格式：单词|词性|释义|例句
		if (std::getline(ss, word, '|') && 
			std::getline(ss, pos, '|') &&
			std::getline(ss, meaning, '|')) {
			
			std::getline(ss, example);
			word = trim(word);
			pos = trim(pos);
			meaning = trim(meaning);
			example = trim(example);
			if (word.empty() || pos.empty() || meaning.empty()) continue;
			
			Word w;
			w.word = word;
			w.pos = pos;
			w.meaning = meaning;
			w.example = example;
			loadedWords.push_back(w);
		}
	}
	
	if (file.bad() || loadedWords.empty()) return false;
	words.swap(loadedWords);
	wrongWords.clear();
	masteredWords.clear();
	failedWords.clear();
	return true;
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
	if (word.word.empty() || std::find(words.begin(), words.end(), word) == words.end()) {
		return false;
	}
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
	if (std::any_of(words.begin(), words.end(), [&word](const Word& entry) {
		return entry.word == word;
	})) {
		failedWords.insert(word);
	}
}

// 添加已掌握单词
void WordManager::addMasteredWord(const std::string& word) {
	if (std::none_of(words.begin(), words.end(), [&word](const Word& entry) {
		return entry.word == word;
	})) return;
	masteredWords.insert(word);
	// 如果单词被掌握，就从失败列表中移除
	failedWords.erase(word);
}

// 获取所有错词
std::vector<Word> WordManager::getWrongWords() const {
	std::cout << "getWrongWords() 被调用，返回 " << wrongWords.size() << " 个错词" << std::endl;
	
	std::vector<Word> result;
	result.reserve(wrongWords.size());
	
	for (const auto& word : wrongWords) {
		result.push_back(word);
		std::cout << "  返回错词: " << word.word << std::endl;
	}
	
	return result;
}

// 清空错词本
void WordManager::clearWrongWords() {
	wrongWords.clear();
	failedWords.clear();
}

// 清空已掌握单词
void WordManager::clearMasteredWords() {
	masteredWords.clear();
}

// 获取统计信息
size_t WordManager::getTotalWords() const {
	return words.size();
}

size_t WordManager::getWrongWordsCount() const {
	return wrongWords.size();
}

size_t WordManager::getMasteredWordsCount() const {
	// 同一拼写可能有多个词性或释义；掌握该拼写后，这些条目都已完成。
	return std::count_if(words.begin(), words.end(), [this](const Word& word) {
		return masteredWords.find(word.word) != masteredWords.end();
	});
}

size_t WordManager::getRemainingWordsCount() const {
	return words.size() - getMasteredWordsCount();
}

// 检查是否所有单词都已掌握
bool WordManager::allWordsMastered() const {
	return !words.empty() && getRemainingWordsCount() == 0;
}

// 获取学习进度
float WordManager::getProgress() const {
	if (words.empty()) return 0.0f;
	return static_cast<float>(getMasteredWordsCount()) / words.size();
}

// 检查单词是否曾经失败
bool WordManager::hasFailed(const std::string& word) const {
	return failedWords.find(word) != failedWords.end();
}
