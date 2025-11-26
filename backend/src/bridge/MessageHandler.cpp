#include "MessageHandler.h"
#include "WebViewBridge.h"
#include <iostream>

MessageHandler::MessageHandler(WebViewBridge& bridge) : bridge(bridge) {}

MessageHandler::~MessageHandler() {}

void MessageHandler::registerHandler(WebKitWebView* webView) {
	WebKitUserContentManager* contentManager = webkit_web_view_get_user_content_manager(webView);
	
	// 注册消息处理器
	webkit_user_content_manager_register_script_message_handler(contentManager, "bridge");
	g_signal_connect(contentManager, "script-message-received::bridge", 
					G_CALLBACK(onScriptMessageReceived), this);
	
	// 注入通信脚本并转发 console 日志到后端，便于调试
	std::string injectionScript = R"(
		(function(){
			// 发送到后端的辅助函数
			function _sendToBridge(obj) {
				try {
					var msg = typeof obj === 'string' ? obj : JSON.stringify(obj);
					if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.bridge) {
						window.webkit.messageHandlers.bridge.postMessage(msg);
					}
				} catch(e) {}
			}

			// 兼容性的 sendToBackend（供前端直接调用）
			window.sendToBackend = function(message) {
				_sendToBridge(message);
			};

			// 转发 console 日志到后端
			try {
				['log','info','warn','error'].forEach(function(level){
					var orig = console[level];
					console[level] = function(){
						try { _sendToBridge({ method: 'console', level: level, args: Array.prototype.slice.call(arguments) }); } catch(e) {}
						try { orig.apply(console, arguments); } catch(e) {}
					};
				});
			} catch(e) {}
		})();
	)";
	
	webkit_user_content_manager_add_script(
		contentManager,
		webkit_user_script_new(
			injectionScript.c_str(),
			WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
			WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
			nullptr,
			nullptr
		)
	);
}

void MessageHandler::onScriptMessageReceived(WebKitUserContentManager* manager, WebKitJavascriptResult* result, gpointer userData) {
	MessageHandler* handler = static_cast<MessageHandler*>(userData);
	
	if (webkit_javascript_result_get_js_value(result)) {
		JSCValue* value = webkit_javascript_result_get_js_value(result);
		gchar* message = jsc_value_to_string(value);
		
		if (message) {
			handler->bridge.handleFrontendMessage(message);
			g_free(message);
		}
	}
	
	webkit_javascript_result_unref(result);
}