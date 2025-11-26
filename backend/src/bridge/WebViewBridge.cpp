#include "WebViewBridge.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <ctime>
#include <cstdio>

WebViewBridge::WebViewBridge(WordManager& wm, SettingsManager& sm, Gtk::Window& window)
    : wordManager(wm), settingsManager(sm), mainWindow(window), webView(nullptr) {

    messageHandler = std::make_unique<MessageHandler>(*this);
}

WebViewBridge::~WebViewBridge() {
    // 在析构时小心释放 WebKitWebView：在某些情况下窗口或容器
    // 已经释放了该对象，直接 unref 会导致 g_object_unref 断言失败。
    // 使用 G_IS_OBJECT 来确保对象仍然有效，释放后将指针置为 nullptr。
    if (webView) {
        if (G_IS_OBJECT(webView)) {
            g_object_unref(webView);
        }
        webView = nullptr;
    }
}

void WebViewBridge::initializeWebView(Gtk::Container* container) {
    // 创建 WebView
    webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // 启用 JavaScript
    WebKitSettings* settings = webkit_web_view_get_settings(webView);
    webkit_settings_set_enable_javascript(settings, TRUE);
    webkit_settings_set_enable_developer_extras(settings, TRUE);
										
    // 注册 MessageHandler（会向 content manager 注入脚本并注册 handler）
    if (messageHandler) {
        messageHandler->registerHandler(webView);
    }

    // 连接信号
    g_signal_connect(webView, "load-changed", G_CALLBACK(onLoadChanged), this);
    g_signal_connect(webView, "script-dialog", G_CALLBACK(onScriptDialog), this);
    g_signal_connect(webView, "decide-policy", G_CALLBACK(onDecidePolicy), this);

    // 添加到容器并确保可见
    Gtk::Widget* webviewWidget = Glib::wrap(GTK_WIDGET(webView));
    container->add(*webviewWidget);
    // 调试时强制给一个最小尺寸并显示
    try {
        webviewWidget->set_size_request(900, 600);
    } catch (...) {
        // 忽略在某些构建中可能抛出的异常
    }
    webviewWidget->show();
    container->show_all_children();

    // 加载前端
    loadFrontend();
}

void WebViewBridge::loadFrontend() {
    // 搜索可能的前端位置（本地开发与构建产物）
    std::vector<std::string> candidates;
    auto cwd = std::filesystem::current_path().string();
    candidates.push_back(cwd + "/resources/web-ui");
    candidates.push_back(cwd + "/backend/build/resources/web-ui");
    candidates.push_back(cwd + "/backend/resources/web-ui");
    candidates.push_back(cwd + "/frontend/dist");

    std::string foundDir;
    std::string fallbackDir;
    for (const auto& p : candidates) {
        std::string indexPath = p + "/index.html";
        std::string assetsPath = p + "/assets";
        if (std::filesystem::exists(indexPath)) {
            // 优先选择同时存在 assets 目录的前端目录
            if (std::filesystem::exists(assetsPath) && std::filesystem::is_directory(assetsPath)) {
                foundDir = p;
                std::cout << "找到前端目录(含 assets): " << foundDir << std::endl;
                break;
            }
            // 记录一个 fallback，如果没有包含 assets 的目录，再使用它
            if (fallbackDir.empty()) fallbackDir = p;
        }
    }
    if (foundDir.empty() && !fallbackDir.empty()) {
        foundDir = fallbackDir;
        std::cout << "找到前端目录（fallback，assets 缺失）: " << foundDir << std::endl;
    }

    if (foundDir.empty()) {
        std::cerr << "前端目录在候选路径中未找到。候选路径如下：\n";
        for (const auto& p : candidates) std::cerr << "  " << p << "\n";

        std::string errorHtml = R"(
            <html>
                <body style="font-family: Arial, sans-serif; text-align: center; padding: 50px;">
                    <h1>VocabMemster</h1>
                    <p style="color: red;">前端资源未找到，请构建前端项目。</p>
                    <p>请将前端构建输出复制到 backend/resources/web-ui 或运行应用时设置工作目录。</p>
                </body>
            </html>
        )";

        webkit_web_view_load_html(webView, errorHtml.c_str(), nullptr);
        return;
    }

    // 启动本地 HTTP 服务器（如果尚未启动）
    static bool serverStarted = false;
    if (!serverStarted) {
        // 先尝试 kill 任何现有的进程（防止端口占用）
        system("pkill -f 'python3 -m http.server 8888' > /dev/null 2>&1");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 使用 Python 启动一个简单的 HTTP 服务器
        // 输出到日志文件便于诊断
        std::string logFile = "/tmp/vocabmemster_server.log";
        std::string command = "cd '" + foundDir + "' && python3 -m http.server 8888 > " + logFile + " 2>&1 &";
        std::cout << "执行命令: " << command << std::endl;
        int result = system(command.c_str());
        
        if (result == 0) {
            std::cout << "HTTP 服务器启动命令已执行（端口 8888）" << std::endl;
            serverStarted = true;
            // 给服务器更多时间启动
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } else {
            std::cerr << "HTTP 服务器启动失败，返回码: " << result << std::endl;
        }
    }

    // 尝试从 HTTP 服务器加载
    std::string httpUrl = "http://localhost:8888/index.html";
    std::cout << "尝试加载前端: " << httpUrl << std::endl;
    webkit_web_view_load_uri(webView, httpUrl.c_str());
}

void WebViewBridge::handleFrontendMessage(const std::string& message) {
    std::cout << "收到前端消息: " << message << std::endl;

    try {
        // 解析为 JSON
        auto j = nlohmann::json::parse(message);
        if (!j.contains("method")) {
            std::cerr << "无效消息（缺少 method字段）" << std::endl;
            return;
        }

        std::string method = j["method"].get<std::string>();

        if (method == "console") {
            // console 转发：打印详细信息
            try {
                auto data = j["data"];
                std::string level = data.value("level", "log");
                auto args = data.value("args", nlohmann::json::array());
                std::cout << "[前端 console][" << level << "] ";
                for (const auto& a : args) std::cout << a.get<std::string>() << " ";
                std::cout << std::endl;
            } catch (...) {
                std::cout << "[前端 console] " << message << std::endl;
            }
            return;
        }

        if (method == "getRandomWord") {
            handleGetRandomWord();
            return;
        }

        if (method == "submitAnswer") {
            auto data = j["data"];
            std::string answer = data.value("answer", "");
            std::string currentWord = data.value("currentWord", "");
            handleSubmitAnswer(answer, currentWord);
            return;
        }

        if (method == "showAnswer") {
            auto data = j["data"];
            std::string currentWord = data.value("currentWord", "");
            handleShowAnswer(currentWord);
            return;
        }

        if (method == "loadWordsFile") {
            auto data = j["data"];
            std::string filename = data.value("filename", "");
            handleLoadWordsFile(filename);
            return;
        }

        if (method == "loadWordsContent" || method == "loadWordsContent" ) {
            auto data = j["data"];
            std::string filename = data.value("filename", "upload.txt");
            std::string content = data.value("content", "");
            handleLoadWordsFileContent(filename, content);
            return;
        }

        if (method == "getStats") {
            handleGetStats();
            return;
        }

        if (method == "getAllWords") {
            handleGetAllWords();
            return;
        }

        if (method == "resetProgress") {
            handleResetProgress();
            return;
        }

    } catch (const std::exception& e) {
        std::cerr << "处理前端消息错误: " << e.what() << std::endl;
    }
}

// 简化的 JavaScript 执行函数，不处理回调
void WebViewBridge::sendToFrontend(const std::string& method, const std::string& data) {
    std::string script = "if (window.receiveFromBackend) { window.receiveFromBackend('" + method + "', '" + data + "'); }";

    // 使用旧版 API 避免复杂的回调处理
    webkit_web_view_run_javascript(webView, script.c_str(), nullptr, nullptr, nullptr);
}

void WebViewBridge::handleGetRandomWord() {
    Word word = wordManager.getRandomWord();
    if (!word.word.empty()) {
        sendToFrontend("wordLoaded", word.toJson().dump());
    } else {
        sendToFrontend("error", "{\"message\":\"没有可用单词或所有单词已掌握\"}");
    }
}

void WebViewBridge::handleSubmitAnswer(const std::string& userAnswer, const std::string& currentWord) {
    // 查找当前单词
    Word currentWordObj;
    for (const auto& word : wordManager.getAllWords()) {
        if (word.word == currentWord) {
            currentWordObj = word;
            break;
        }
    }

    if (currentWordObj.word.empty()) {
        sendToFrontend("error", "{\"message\":\"找不到当前单词\"}");
        return;
    }

    // 比较答案（不区分大小写）
    std::string userAnswerLower = userAnswer;
    std::transform(userAnswerLower.begin(), userAnswerLower.end(), userAnswerLower.begin(), ::tolower);

    std::string correctAnswerLower = currentWordObj.word;
    std::transform(correctAnswerLower.begin(), correctAnswerLower.end(), correctAnswerLower.begin(), ::tolower);

    if (userAnswerLower == correctAnswerLower) {
        // 答案正确
        wordManager.addMasteredWord(currentWordObj.word);
        sendToFrontend("answerResult", "{\"correct\":true,\"message\":\"答案正确！\"}");

        // 发送更新后的统计信息
        handleGetStats();
    } else {
        // 答案错误
        wordManager.addWrongWord(currentWordObj);
        wordManager.addFailedWord(currentWordObj.word);
        sendToFrontend("answerResult", "{\"correct\":false,\"message\":\"答案错误，正确答案: " + currentWordObj.word + "\"}");

        // 发送更新后的统计信息
        handleGetStats();
    }
}

void WebViewBridge::handleShowAnswer(const std::string& currentWord) {
    // 查找当前单词
    Word currentWordObj;
    for (const auto& word : wordManager.getAllWords()) {
        if (word.word == currentWord) {
            currentWordObj = word;
            break;
        }
    }

    if (!currentWordObj.word.empty()) {
        wordManager.addWrongWord(currentWordObj);
        wordManager.addFailedWord(currentWordObj.word);
        sendToFrontend("answerShown", "{\"correctAnswer\":\"" + currentWordObj.word + "\"}");

        // 发送更新后的统计信息
        handleGetStats();
    }
}

void WebViewBridge::handleLoadWordsFile(const std::string& filename) {
    if (wordManager.loadWordsFromFile(filename)) {
        sendToFrontend("wordsLoaded", "{\"message\":\"单词库加载成功\", \"count\":" + std::to_string(wordManager.getTotalWords()) + "}");
        handleGetStats();
    } else {
        sendToFrontend("error", "{\"message\":\"无法加载单词库文件\"}");
    }
}

void WebViewBridge::handleLoadWordsFileContent(const std::string& filename, const std::string& content) {
    // 写入到临时文件并调用现有加载逻辑
    std::time_t t = std::time(nullptr);
    std::string tmpPath = "/tmp/vocabmemster_upload_" + std::to_string(t) + "_" + filename;

    std::ofstream ofs(tmpPath, std::ios::binary);
    if (!ofs.is_open()) {
        sendToFrontend("error", "{\"message\":\"无法创建临时文件\"}");
        return;
    }
    ofs << content;
    ofs.close();

    bool ok = wordManager.loadWordsFromFile(tmpPath);

    // 尝试删除临时文件（忽略错误）
    std::remove(tmpPath.c_str());

    if (ok) {
        sendToFrontend("wordsLoaded", "{\"message\":\"单词库加载成功\", \"count\":" + std::to_string(wordManager.getTotalWords()) + "}");
        handleGetStats();
    } else {
        sendToFrontend("error", "{\"message\":\"无法加载上传的单词库\"}");
    }
}

void WebViewBridge::handleGetStats() {
    LearningStats stats = wordManager.getStats();
    sendToFrontend("statsUpdated", stats.toJson().dump());
}

void WebViewBridge::handleGetAllWords() {
    auto allWords = wordManager.getAllWords();
    nlohmann::json wordsArray = nlohmann::json::array();

    for (const auto& word : allWords) {
        wordsArray.push_back(word.toJson());
    }

    sendToFrontend("allWords", wordsArray.dump());
}

void WebViewBridge::handleResetProgress() {
    wordManager.resetProgress();
    sendToFrontend("progressReset", "{\"message\":\"进度已重置\"}");
    handleGetStats();
}

// 静态回调函数实现
void WebViewBridge::onLoadChanged(WebKitWebView* webView, WebKitLoadEvent event, gpointer userData) {
    WebViewBridge* bridge = static_cast<WebViewBridge*>(userData);

    if (event == WEBKIT_LOAD_FINISHED) {
        std::cout << "前端页面加载完成" << std::endl;

        // 注入通信桥梁
        std::string bridgeScript = R"(
            window.sendToBackend = function(message) {
                window.webkit.messageHandlers.bridge.postMessage(message);
            };

            window.receiveFromBackend = function(method, data) {
                // 这个函数由前端实现
                if (window.onBackendMessage) {
                    window.onBackendMessage(method, data);
                }
            };

            console.log('WebViewBridge 初始化完成');
        )";
        // 额外移除加载遮罩（如果前端脚本因加载顺序问题未能移除），保证界面可见
        bridgeScript += R"(
            (function(){
                try {
                    var loading = document.getElementById('loading');
                    if (loading) {
                        loading.style.opacity = '0';
                        setTimeout(function(){ if (loading && loading.parentNode) loading.parentNode.removeChild(loading); }, 300);
                    }
                } catch(e) { console.error('移除 loading 遮罩失败', e); }
            })();
        )";

        // 使用旧版 API 避免复杂的回调处理
        webkit_web_view_run_javascript(webView, bridgeScript.c_str(), nullptr, nullptr, nullptr);
    }
}

gboolean WebViewBridge::onScriptDialog(WebKitWebView* webView, WebKitScriptDialog* dialog, gpointer userData) {
    WebViewBridge* bridge = static_cast<WebViewBridge*>(userData);

    // 处理来自 JavaScript 的对话框
    if (webkit_script_dialog_get_dialog_type(dialog) == WEBKIT_SCRIPT_DIALOG_ALERT) {
        const gchar* message = webkit_script_dialog_get_message(dialog);
        std::cout << "JavaScript 提示: " << message << std::endl;
        webkit_script_dialog_close(dialog);
        return TRUE;
    }

    return FALSE;
}

gboolean WebViewBridge::onDecidePolicy(WebKitWebView* webView, WebKitPolicyDecision* decision, WebKitPolicyDecisionType decisionType, gpointer userData) {
    if (decisionType == WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION) {
        WebKitNavigationPolicyDecision* navigationDecision = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
        WebKitNavigationAction* action = webkit_navigation_policy_decision_get_navigation_action(navigationDecision);
        WebKitURIRequest* request = webkit_navigation_action_get_request(action);
        const gchar* uri = webkit_uri_request_get_uri(request);

        // 阻止外部链接在 WebView 中打开
        if (g_str_has_prefix(uri, "http://") || g_str_has_prefix(uri, "https://")) {
            // 可以在外部浏览器中打开
            // gtk_show_uri_on_window(nullptr, uri, GDK_CURRENT_TIME, nullptr);
            webkit_policy_decision_ignore(decision);
            return TRUE;
        }
    }

    return FALSE;
}
