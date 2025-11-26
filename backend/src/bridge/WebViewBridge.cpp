#include "WebViewBridge.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

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
    candidates.push_back(cwd + "/resources/web-ui/index.html");
    candidates.push_back(cwd + "/backend/build/resources/web-ui/index.html");
    candidates.push_back(cwd + "/backend/resources/web-ui/index.html");
    candidates.push_back(cwd + "/frontend/dist/index.html");
    candidates.push_back(cwd + "/frontend/index.html");

    std::string foundPath;
    for (const auto& p : candidates) {
        if (std::filesystem::exists(p)) {
            foundPath = p;
            break;
        }
    }

    if (foundPath.empty()) {
        std::cerr << "前端文件在候选路径中未找到。候选路径如下：\n";
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

    // 读取 HTML 文件并注入 base 标签以修正资源路径
    std::ifstream htmlFile(foundPath);
    if (!htmlFile.is_open()) {
        std::cerr << "无法打开前端文件: " << foundPath << std::endl;
        webkit_web_view_load_html(webView, "<html><body><h1>错误</h1><p>无法打开前端文件</p></body></html>", nullptr);
        return;
    }

    std::stringstream htmlBuffer;
    htmlBuffer << htmlFile.rdbuf();
    std::string htmlContent = htmlBuffer.str();
    htmlFile.close();

    // 获取 HTML 文件所在目录作为 base URL
    std::filesystem::path htmlDir = std::filesystem::path(foundPath).parent_path();
    std::string baseUrl = "file://" + htmlDir.string() + "/";

    // 将以 /assets/ 开头的绝对路径转换为相对路径（file:// 加上 base tag 不会影响以 / 开头的绝对路径）
    // 例如将 "/assets/.." -> "assets/.."，以便相对于 baseUrl 正确加载资源
    std::string from = "/assets/";
    std::string to = "assets/";
    size_t pos = 0;
    while ((pos = htmlContent.find(from, pos)) != std::string::npos) {
        htmlContent.replace(pos, from.length(), to);
        pos += to.length();
    }

    // 在 <head> 中注入 <base> 标签以修正其它相对资源路径
    size_t headPos = htmlContent.find("</head>");
    if (headPos != std::string::npos) {
        std::string baseTag = "<base href=\"" + baseUrl + "\">\n";
        htmlContent.insert(headPos, baseTag);
    }

    // 调试输出：检查是否仍存在以 /assets/ 开头的引用（用于排查 file:// 加载问题）
    if (htmlContent.find("/assets/") != std::string::npos) {
        std::cout << "注意：HTML 内容仍包含 '/assets/' 绝对引用，可能导致资源加载失败 via file://" << std::endl;
    } else {
        std::cout << "HTML 内容中的 /assets/ 已被重写为相对路径。" << std::endl;
    }

    std::cout << "加载前端（带 base URL）: " << baseUrl << std::endl;
    webkit_web_view_load_html(webView, htmlContent.c_str(), baseUrl.c_str());
}

void WebViewBridge::handleFrontendMessage(const std::string& message) {
    std::cout << "收到前端消息: " << message << std::endl;

    try {
        // 解析消息
        // 消息格式: {"method": "methodName", "data": "data"}
        // 这里使用简单的字符串解析，实际可以使用更复杂的JSON解析

        if (message.find("\"method\":\"getRandomWord\"") != std::string::npos) {
            handleGetRandomWord();
        }
        else if (message.find("\"method\":\"submitAnswer\"") != std::string::npos) {
            // 提取数据
            size_t dataStart = message.find("\"data\":") + 8;
            size_t dataEnd = message.find("\"", dataStart);
            std::string data = message.substr(dataStart, dataEnd - dataStart);

            // 简单的数据解析
            size_t answerStart = data.find("answer\":\"") + 9;
            size_t answerEnd = data.find("\"", answerStart);
            std::string userAnswer = data.substr(answerStart, answerEnd - answerStart);

            size_t wordStart = data.find("currentWord\":\"") + 14;
            size_t wordEnd = data.find("\"", wordStart);
            std::string currentWord = data.substr(wordStart, wordEnd - wordStart);

            handleSubmitAnswer(userAnswer, currentWord);
        }
        else if (message.find("\"method\":\"showAnswer\"") != std::string::npos) {
            size_t dataStart = message.find("\"data\":\"") + 8;
            size_t dataEnd = message.find("\"", dataStart);
            std::string currentWord = message.substr(dataStart, dataEnd - dataStart);
            handleShowAnswer(currentWord);
        }
        else if (message.find("\"method\":\"loadWordsFile\"") != std::string::npos) {
            size_t dataStart = message.find("\"data\":\"") + 8;
            size_t dataEnd = message.find("\"", dataStart);
            std::string filename = message.substr(dataStart, dataEnd - dataStart);
            handleLoadWordsFile(filename);
        }
        else if (message.find("\"method\":\"getStats\"") != std::string::npos) {
            handleGetStats();
        }
        else if (message.find("\"method\":\"getAllWords\"") != std::string::npos) {
            handleGetAllWords();
        }
        else if (message.find("\"method\":\"resetProgress\"") != std::string::npos) {
            handleResetProgress();
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
