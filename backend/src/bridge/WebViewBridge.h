#ifndef WEBVIEWBRIDGE_H
#define WEBVIEWBRIDGE_H

#include <gtkmm.h>
#include <webkit2/webkit2.h>
#include "core/WordManager.h"
#include "core/SettingsManager.h"
#include "MessageHandler.h"

class WebViewBridge {
private:
	WordManager& wordManager;
	SettingsManager& settingsManager;
	Gtk::Window& mainWindow;
	WebKitWebView* webView;
	std::unique_ptr<MessageHandler> messageHandler;
	
public:
	WebViewBridge(WordManager& wm, SettingsManager& sm, Gtk::Window& window);
	~WebViewBridge();
	
	// 初始化 WebView
	void initializeWebView(Gtk::Container* container);
	
	// 处理来自前端的消息
	void handleFrontendMessage(const std::string& message);
	
	// 发送消息到前端
	void sendToFrontend(const std::string& method, const std::string& data);
	
	// 加载前端页面
	void loadFrontend();
	
	// 获取 WebView
	WebKitWebView* getWebView() const { return webView; }

private:
	// 静态回调函数
	static void onLoadChanged(WebKitWebView* webView, WebKitLoadEvent event, gpointer userData);
	static gboolean onScriptDialog(WebKitWebView* webView, WebKitScriptDialog* dialog, gpointer userData);
	static gboolean onDecidePolicy(WebKitWebView* webView, WebKitPolicyDecision* decision, WebKitPolicyDecisionType decisionType, gpointer userData);
	
	// 处理具体的前端消息
	void handleGetRandomWord();
	void handleSubmitAnswer(const std::string& userAnswer, const std::string& currentWord);
	void handleShowAnswer(const std::string& currentWord);
	void handleLoadWordsFile(const std::string& filename);
	void handleLoadWordsFileContent(const std::string& filename, const std::string& content);
	void handleSaveSettings(const std::string& settingsJson);
	void handleExportWrongWords(const std::string& filename);
	void handleResetProgress();
	void handleGetStats();
	void handleGetAllWords();
};

#endif