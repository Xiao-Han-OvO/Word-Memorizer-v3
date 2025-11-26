#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <string>
#include <webkit2/webkit2.h>

class WebViewBridge;

class MessageHandler {
private:
	WebViewBridge& bridge;

public:
	MessageHandler(WebViewBridge& bridge);
	~MessageHandler();
	
	// 处理来自 WebKit 的消息
	static void onScriptMessageReceived(WebKitUserContentManager* manager, WebKitJavascriptResult* result, gpointer userData);
	
	// 注册消息处理器
	void registerHandler(WebKitWebView* webView);
};

#endif