<template>
	<div class="shortcut-settings space-y-6">
		<!-- 快捷键列表 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">快捷键设置</h3>
			
			<div class="space-y-3">
				<div
					v-for="shortcut in shortcuts"
					:key="shortcut.id"
					class="shortcut-item flex items-center justify-between p-3 rounded-lg border border-gray-200 dark:border-gray-700"
				>
					<div>
						<div class="font-medium text-sm">{{ shortcut.name }}</div>
						<div class="text-xs text-gray-500 dark:text-gray-400">
							{{ shortcut.description }}
						</div>
					</div>
					<div class="flex items-center space-x-2">
						<kbd class="px-2 py-1 text-xs bg-gray-100 dark:bg-gray-800 border border-gray-300 dark:border-gray-600 rounded">
							{{ shortcut.key }}
						</kbd>
						<button
							v-if="shortcut.customizable"
							@click="editShortcut(shortcut)"
							class="text-blue-600 hover:text-blue-800 dark:text-blue-400 dark:hover:text-blue-300 text-sm"
						>
							编辑
						</button>
					</div>
				</div>
			</div>
		</div>

		<!-- 快捷键说明 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">使用说明</h3>
			
			<div class="card p-4 space-y-3">
				<div class="text-sm text-gray-600 dark:text-gray-400">
					<p class="mb-2">快捷键可以帮助您更高效地使用应用：</p>
					<ul class="list-disc list-inside space-y-1">
						<li>在输入单词时，按 <kbd class="px-1 py-0.5 text-xs bg-gray-100 dark:bg-gray-800 rounded">Enter</kbd> 提交答案</li>
						<li>使用 <kbd class="px-1 py-0.5 text-xs bg-gray-100 dark:bg-gray-800 rounded">Ctrl+Tab</kbd> 快速切换到下一个单词</li>
						<li>按 <kbd class="px-1 py-0.5 text-xs bg-gray-100 dark:bg-gray-800 rounded">Alt+S</kbd> 直接显示当前单词的答案</li>
						<li>使用 <kbd class="px-1 py-0.5 text-xs bg-gray-100 dark:bg-gray-800 rounded">Ctrl+Shift+D</kbd> 切换明暗主题</li>
					</ul>
				</div>
			</div>
		</div>

		<!-- 自定义快捷键 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">自定义快捷键</h3>
			
			<div class="card p-4 space-y-4">
				<div class="text-sm text-gray-600 dark:text-gray-400 mb-4">
					点击上方的"编辑"按钮来自定义快捷键。支持组合键如 Ctrl、Alt、Shift。
				</div>

				<!-- 快捷键录制界面 -->
				<div 
					v-if="editingShortcut"
					class="shortcut-recorder p-4 border-2 border-dashed border-gray-300 dark:border-gray-600 rounded-lg text-center"
				>
					<div class="text-lg font-medium mb-2">
						为 "{{ editingShortcut.name }}" 设置新快捷键
					</div>
					<div class="text-sm text-gray-500 dark:text-gray-400 mb-4">
						请按下新的组合键...
					</div>
					<div class="recorded-keys text-xl font-mono mb-4 min-h-8">
						{{ recordedKeys }}
					</div>
					<div class="flex justify-center space-x-3">
						<button
							@click="cancelRecording"
							class="btn btn-secondary"
						>
							取消
						</button>
						<button
							@click="saveShortcut"
							:disabled="!recordedKeys"
							class="btn btn-primary"
						>
							保存
						</button>
					</div>
				</div>

				<!-- 重置按钮 -->
				<div class="pt-4">
					<button
						@click="resetShortcuts"
						class="btn btn-secondary"
					>
						重置所有快捷键
					</button>
				</div>
			</div>
		</div>
	</div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { AppConfig } from '@settings/app.config'

// 响应式数据
const shortcuts = ref([
	{ id: 'next-word', name: '下一个单词', key: 'Ctrl+Tab', description: '加载并显示下一个单词', customizable: true },
	{ id: 'show-answer', name: '显示答案', key: 'Alt+S', description: '显示当前单词的正确答案', customizable: true },
	{ id: 'toggle-theme', name: '切换主题', key: 'Ctrl+Shift+D', description: '在明暗主题之间切换', customizable: true },
	{ id: 'open-words', name: '打开单词库', key: 'Alt+O', description: '打开单词库文件选择对话框', customizable: true },
	{ id: 'edit-words', name: '编辑单词', key: 'Alt+Shift+O', description: '打开单词管理界面', customizable: true },
	{ id: 'export-wrong', name: '导出错词本', key: 'Alt+E', description: '导出错词本到文件', customizable: true },
	{ id: 'submit-answer', name: '提交答案', key: 'Enter', description: '提交当前输入的答案', customizable: false }
])

const editingShortcut = ref(null)
const recordedKeys = ref('')
const isRecording = ref(false)

// 方法
const editShortcut = (shortcut) => {
	editingShortcut.value = shortcut
	recordedKeys.value = ''
	isRecording.value = true
}

const cancelRecording = () => {
	editingShortcut.value = null
	recordedKeys.value = ''
	isRecording.value = false
}

const saveShortcut = () => {
	if (editingShortcut.value && recordedKeys.value) {
		// 更新快捷键
		const shortcut = shortcuts.value.find(s => s.id === editingShortcut.value.id)
		if (shortcut) {
			shortcut.key = recordedKeys.value
		}
		
		// 保存到后端
		bridge.send('saveShortcuts', {
			[editingShortcut.value.id]: recordedKeys.value
		})
	}
	
	cancelRecording()
}

const resetShortcuts = () => {
	// 重置为默认快捷键
	shortcuts.value = [
		{ id: 'next-word', name: '下一个单词', key: 'Ctrl+Tab', description: '加载并显示下一个单词', customizable: true },
		{ id: 'show-answer', name: '显示答案', key: 'Alt+S', description: '显示当前单词的正确答案', customizable: true },
		{ id: 'toggle-theme', name: '切换主题', key: 'Ctrl+Shift+D', description: '在明暗主题之间切换', customizable: true },
		{ id: 'open-words', name: '打开单词库', key: 'Alt+O', description: '打开单词库文件选择对话框', customizable: true },
		{ id: 'edit-words', name: '编辑单词', key: 'Alt+Shift+O', description: '打开单词管理界面', customizable: true },
		{ id: 'export-wrong', name: '导出错词本', key: 'Alt+E', description: '导出错词本到文件', customizable: true },
		{ id: 'submit-answer', name: '提交答案', key: 'Enter', description: '提交当前输入的答案', customizable: false }
	]
	
	// 保存到后端
	bridge.send('resetShortcuts', {})
}

// 键盘事件处理
const handleKeyDown = (event) => {
	if (!isRecording.value) return
	
	event.preventDefault()
	event.stopPropagation()
	
	const keys = []
	
	// 添加修饰键
	if (event.ctrlKey) keys.push('Ctrl')
	if (event.altKey) keys.push('Alt')
	if (event.shiftKey) keys.push('Shift')
	if (event.metaKey) keys.push('Cmd') // Mac
	
	// 添加主键（排除修饰键）
	if (!['Control', 'Alt', 'Shift', 'Meta'].includes(event.key)) {
		let key = event.key
		
		// 特殊键处理
		if (key === ' ') key = 'Space'
		else if (key === 'Tab') key = 'Tab'
		else if (key === 'Enter') key = 'Enter'
		else if (key === 'Escape') key = 'Esc'
		else if (key === 'ArrowUp') key = '↑'
		else if (key === 'ArrowDown') key = '↓'
		else if (key === 'ArrowLeft') key = '←'
		else if (key === 'ArrowRight') key = '→'
		else if (key.length === 1) key = key.toUpperCase()
		
		keys.push(key)
	}
	
	recordedKeys.value = keys.join('+')
}

// 生命周期
onMounted(() => {
	document.addEventListener('keydown', handleKeyDown)
})

onUnmounted(() => {
	document.removeEventListener('keydown', handleKeyDown)
})
</script>

<style scoped>
.shortcut-item {
	transition: all 0.2s ease;
}

.shortcut-item:hover {
	background-color: var(--card-bg-color);
	transform: translateX(4px);
}

.shortcut-recorder {
	background-color: var(--card-bg-color);
}

.recorded-keys {
	font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
}

kbd {
	font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
}
</style>