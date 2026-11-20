#ifndef TESTSUPPORT_H
#define TESTSUPPORT_H

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

inline void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        std::string pattern = "/tmp/word-memorizer-tests-XXXXXX";
        std::vector<char> buffer(pattern.begin(), pattern.end());
        buffer.push_back('\0');
        const char* created = mkdtemp(buffer.data());
        if (!created) throw std::runtime_error("Cannot create test directory");
        directory = created;
    }

    ~TemporaryDirectory() {
        for (const auto& filename : files) std::remove(filename.c_str());
        rmdir(directory.c_str());
    }

    std::string file(const std::string& name) {
        const std::string filename = directory + "/" + name;
        files.push_back(filename);
        return filename;
    }

    std::string write(const std::string& name, const std::string& content) {
        const std::string filename = file(name);
        std::ofstream output(filename, std::ios::binary);
        output << content;
        require(static_cast<bool>(output), "Cannot write test fixture");
        return filename;
    }

private:
    std::string directory;
    std::vector<std::string> files;
};

inline std::string readFile(const std::string& filename) {
    std::ifstream input(filename, std::ios::binary);
    require(static_cast<bool>(input), "Cannot read test file");
    std::ostringstream content;
    content << input.rdbuf();
    return content.str();
}

class TestRunner {
public:
    void run(const std::string& name, const std::function<void()>& test) {
        try {
            test();
            std::cout << "PASS: " << name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "FAIL: " << name << ": " << error.what() << '\n';
        }
    }

    int result() const { return failures == 0 ? 0 : 1; }

private:
    int failures = 0;
};

#endif
