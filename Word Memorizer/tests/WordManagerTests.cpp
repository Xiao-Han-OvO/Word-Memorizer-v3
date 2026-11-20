#include "WordManager.h"
#include "TestSupport.h"
#include <cmath>

int main() {
    TestRunner tests;

    tests.run("empty library is not a completed session", [] {
        WordManager manager;
        require(manager.getRandomWord().word.empty(), "Empty library returned a word");
        require(!manager.allWordsMastered(), "Empty library reported completion");
        require(manager.getProgress() == 0.0f, "Empty library has progress");
    });

    tests.run("UTF-8 BOM, CRLF and field whitespace are normalized", [] {
        TemporaryDirectory temporary;
        const auto filename = temporary.write("windows.txt",
            "\xef\xbb\xbf  apple  | n. | 苹果 | An apple | a day. \r\n");
        WordManager manager;
        require(manager.loadWordsFromFile(filename), "Valid Windows file was rejected");
        const Word word = manager.getRandomWord();
        require(word.word == "apple", "BOM or whitespace became part of the answer");
        require(word.pos == "n.", "Part of speech was not trimmed");
        require(word.meaning == "苹果", "Meaning was not trimmed");
        require(word.example == "An apple | a day.", "Example or line ending was corrupted");
    });

    tests.run("optional examples work and empty required fields are skipped", [] {
        TemporaryDirectory temporary;
        const auto filename = temporary.write("mixed.txt",
            "invalid line\n|n.|empty word\nempty||missing pos\nempty|n.|   \n"
            "pear|n.|梨\r\n");
        WordManager manager;
        require(manager.loadWordsFromFile(filename), "Three-field word was rejected");
        require(manager.getTotalWords() == 1, "Invalid rows became learning questions");
        const Word word = manager.getRandomWord();
        require(word.word == "pear" && word.meaning == "梨", "CRLF leaked into a field");
        require(word.example.empty(), "Missing example was not empty");
    });

    tests.run("failed imports preserve the current words and learning state", [] {
        TemporaryDirectory temporary;
        const auto original = temporary.write("original.txt",
            "alpha|n.|first|Example one.\nbeta|n.|second|Example two.\n");
        const auto invalid = temporary.write("invalid.txt", "bad row\n|n.|missing word\n");
        const auto empty = temporary.write("empty.txt", "\n\r\n");
        WordManager manager;
        require(manager.loadWordsFromFile(original), "Fixture could not be loaded");
        manager.addMasteredWord("alpha");
        const Word beta = manager.getRandomWord();
        manager.addWrongWord(beta);
        manager.addFailedWord(beta.word);

        for (const auto& filename : {temporary.file("missing.txt"), invalid, empty}) {
            require(!manager.loadWordsFromFile(filename), "Invalid import succeeded");
            require(manager.getTotalWords() == 2, "Failed import erased the current library");
            require(manager.getMasteredWordsCount() == 1, "Failed import erased progress");
            require(manager.getWrongWordsCount() == 1 && manager.hasFailed("beta"),
                    "Failed import erased the wrong-word history");
            require(manager.getRandomWord().word == "beta", "Current session changed");
        }

        const auto replacement = temporary.write("replacement.txt", "gamma|n.|third\n");
        require(manager.loadWordsFromFile(replacement), "Replacement could not be loaded");
        require(manager.getTotalWords() == 1 && manager.getMasteredWordsCount() == 0,
                "Successful replacement retained old progress");
        require(manager.getWrongWordsCount() == 0 && !manager.hasFailed("beta"),
                "Successful replacement retained old failures");
    });

    tests.run("repeated spellings can reach complete progress", [] {
        TemporaryDirectory temporary;
        const auto filename = temporary.write("duplicates.txt",
            "run|v.|跑步|I run.\nrun|n.|跑步过程|A long run.\nwalk|v.|走路|I walk.\n");
        WordManager manager;
        require(manager.loadWordsFromFile(filename), "Duplicate fixture was rejected");
        manager.addMasteredWord("run");
        require(manager.getMasteredWordsCount() == 2, "Repeated entries were not counted");
        require(manager.getRemainingWordsCount() == 1, "Remaining count is inconsistent");
        require(std::fabs(manager.getProgress() - 2.0f / 3.0f) < 0.0001f,
                "Progress does not match mastered entries");
        require(manager.getRandomWord().word == "walk", "Mastered spelling was repeated");
        manager.addMasteredWord("walk");
        require(manager.allWordsMastered(), "Session got stuck after every answer was mastered");
        require(manager.getProgress() == 1.0f && manager.getRemainingWordsCount() == 0,
                "Completed progress is inconsistent");
        require(manager.getRandomWord().word.empty(), "Completed session returned another word");
    });

    tests.run("unknown words cannot corrupt progress and clearing failures stays cleared", [] {
        TemporaryDirectory temporary;
        const auto filename = temporary.write("single.txt", "apple|n.|苹果\n");
        WordManager manager;
        require(manager.loadWordsFromFile(filename), "Single-word fixture was rejected");
        const Word apple = manager.getRandomWord();
        manager.addMasteredWord("");
        manager.addMasteredWord("missing");
        require(manager.getMasteredWordsCount() == 0 && manager.getRemainingWordsCount() == 1,
                "Unknown spelling changed progress");
        require(!manager.addWrongWord(Word{"", "", "", ""}), "Empty answer became a wrong word");
        manager.addWrongWord(apple);
        manager.addFailedWord(apple.word);
        manager.clearWrongWords();
        require(manager.getWrongWordsCount() == 0 && !manager.hasFailed(apple.word),
                "Clearing wrong words left stale failure markers");
        manager.addMasteredWord(apple.word);
        manager.clearMasteredWords();
        require(!manager.allWordsMastered() && manager.getProgress() == 0.0f,
                "Restart did not reset mastery");
    });

    return tests.result();
}
