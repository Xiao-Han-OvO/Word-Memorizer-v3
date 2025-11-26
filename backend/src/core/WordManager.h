#ifndef WORDMANAGER_H
#define WORDMANAGER_H

#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <nlohmann/json.hpp>

// 单词数据结构
struct Word {
    std::string word;        // 英文单词
    std::string pos;         // 词性
    std::string meaning;     // 中文意思
    std::string example;     // 例句
    std::vector<Word> derivatives; // 派生词列表
    
    // 重载==运算符，用于比较两个Word对象
    bool operator==(const Word& other) const {
        return word == other.word && pos == other.pos && meaning == other.meaning;
    }
    
    // 检查是否有派生词
    bool hasDerivatives() const {
        return !derivatives.empty();
    }
    
    // 转换为 JSON
    nlohmann::json toJson() const {
        nlohmann::json j;
        j["word"] = word;
        j["pos"] = pos;
        j["meaning"] = meaning;
        j["example"] = example;
        j["hasDerivatives"] = hasDerivatives();
        
        // 转换派生词
        if (hasDerivatives()) {
            nlohmann::json derivativesJson = nlohmann::json::array();
            for (const auto& derivative : derivatives) {
                derivativesJson.push_back(derivative.toJson());
            }
            j["derivatives"] = derivativesJson;
        }
        
        return j;
    }
    
    // 从 JSON 创建 Word
    static Word fromJson(const nlohmann::json& j) {
        Word w;
        w.word = j.value("word", "");
        w.pos = j.value("pos", "");
        w.meaning = j.value("meaning", "");
        w.example = j.value("example", "");
        
        // 解析派生词
        if (j.contains("derivatives") && j["derivatives"].is_array()) {
            for (const auto& derivativeJson : j["derivatives"]) {
                w.derivatives.push_back(Word::fromJson(derivativeJson));
            }
        }
        
        return w;
    }
};

// 为Word结构体提供哈希函数，用于unordered_set
namespace std {
    template<>
    struct hash<Word> {
        size_t operator()(const Word& w) const {
            return std::hash<std::string>()(w.word + "|" + w.pos);
        }
    };
}

// 学习统计数据结构
struct LearningStats {
    size_t totalWords;
    size_t masteredWords;
    size_t wrongWords;
    size_t remainingWords;
    float progress;
    
    nlohmann::json toJson() const {
        return {
            {"totalWords", totalWords},
            {"masteredWords", masteredWords},
            {"wrongWords", wrongWords},
            {"remainingWords", remainingWords},
            {"progress", progress}
        };
    }
};

// 单词管理器类
class WordManager {
private:
    std::vector<Word> words;                           // 所有单词
    std::unordered_set<Word> wrongWords;               // 错词集合
    std::unordered_set<std::string> masteredWords;     // 已掌握单词
    std::unordered_set<std::string> failedWords;       // 曾经拼错过的单词
    std::mt19937 rng;                                  // 随机数生成器

public:
    WordManager();
    
    // 文件操作
    bool loadWordsFromFile(const std::string& filename);
    bool saveWordsToFile(const std::string& filename, const std::vector<Word>& customWords = {});
    
    // 单词操作
    Word getRandomWord();
    bool addWrongWord(const Word& word);
    void addFailedWord(const std::string& word);
    void addMasteredWord(const std::string& word);
    
    // 获取数据
    std::vector<Word> getWrongWords() const;
    std::vector<Word> getAllWords() const;
    LearningStats getStats() const;
    
    // 管理操作
    void clearWrongWords();
    void clearMasteredWords();
    void resetProgress();
    
    // 状态检查
    size_t getTotalWords() const;
    size_t getWrongWordsCount() const;
    size_t getMasteredWordsCount() const;
    size_t getRemainingWordsCount() const;
    bool allWordsMastered() const;
    float getProgress() const;
    bool hasFailed(const std::string& word) const;
    
    // JSON 序列化
    nlohmann::json toJson() const;
    bool fromJson(const nlohmann::json& j);
};

#endif