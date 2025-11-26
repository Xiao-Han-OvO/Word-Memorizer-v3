// 前后端通信桥梁
class WebViewBridge {
	constructor() {
		this.handlers = new Map()
		this.initialize()
	}

	initialize() {
		// 定义接收后端消息的函数
		window.onBackendMessage = (method, data) => {
			this.handleBackendMessage(method, data)
		}

		// 定义发送消息到后端的函数
		window.sendToBackend = (message) => {
			this.send(message)
		}

		console.log('WebViewBridge 初始化完成')
	}

	// 发送消息到后端
	send(method, data = {}) {
		const message = JSON.stringify({ method, data })
		
		if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.bridge) {
			// WebKit 环境 (GTK WebView)
			window.webkit.messageHandlers.bridge.postMessage(message)
		} else if (window.chrome && window.chrome.webview) {
			// Chromium WebView 环境
			window.chrome.webview.postMessage(message)
		} else {
			// 开发环境 - 模拟后端响应
			console.log('发送到后端:', method, data)
			this.simulateBackendResponse(method, data)
		}
	}

	// 注册消息处理器
	on(method, handler) {
		this.handlers.set(method, handler)
	}

	// 移除消息处理器
	off(method) {
		this.handlers.delete(method)
	}

	// 处理后端消息
	handleBackendMessage(method, data) {
		console.log('收到后端消息:', method, data)
		
		const handler = this.handlers.get(method)
		if (handler) {
			try {
				// 解析数据
				const parsedData = typeof data === 'string' ? JSON.parse(data) : data
				handler(parsedData)
			} catch (error) {
				console.error('解析后端数据错误:', error)
			}
		} else {
			console.warn(`未注册的消息处理器: ${method}`)
		}
	}

	// 开发环境模拟后端响应
	simulateBackendResponse(method, data) {
		setTimeout(() => {
			switch (method) {
				case 'getRandomWord':
					const mockWord = {
						word: 'example',
						pos: 'n.',
						meaning: '例子，实例',
						example: 'This is an example sentence.',
						hasDerivatives: false
					}
					this.handleBackendMessage('wordLoaded', JSON.stringify(mockWord))
					break
					
				case 'submitAnswer':
					const isCorrect = data.answer.toLowerCase() === 'example'
					const result = {
						correct: isCorrect,
						message: isCorrect ? '答案正确！' : '答案错误，正确答案: example'
					}
					this.handleBackendMessage('answerResult', JSON.stringify(result))
					break
					
				case 'getStats':
					const mockStats = {
						totalWords: 100,
						masteredWords: 25,
						wrongWords: 5,
						remainingWords: 75,
						progress: 0.25
					}
					this.handleBackendMessage('statsUpdated', JSON.stringify(mockStats))
					break
					
				default:
					console.log(`模拟后端响应: ${method}`)
			}
		}, 500)
	}
}

// 创建单例实例
export const bridge = new WebViewBridge()