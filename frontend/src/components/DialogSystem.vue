<template>
	<!-- 文件选择对话框 -->
	<div 
		v-if="showFileDialog"
		class="dialog-overlay fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50"
		@click="closeFileDialog"
	>
		<div 
			class="dialog-content card p-6 max-w-md w-full mx-4"
			@click.stop
		>
			<h3 class="text-xl font-bold mb-4">选择单词库文件</h3>
			<div class="space-y-4">
				<div class="file-types">
					<h4 class="font-medium mb-2">支持的文件格式:</h4>
					<ul class="list-disc list-inside text-sm text-gray-600 dark:text-gray-400">
						<li>文本文件 (.txt) - 格式: 单词|词性|释义|例句</li>
						<li>WDLST 文件 (.wdlst) - 高级单词库格式</li>
					</ul>
				</div>
				<div class="flex justify-end space-x-3">
					<button
						@click="closeFileDialog"
						class="btn btn-secondary"
					>
						取消
					</button>
					<button
						@click="triggerFileInput"
						class="btn btn-primary"
					>
						选择文件
					</button>
				</div>
			</div>
		</div>
	</div>

	<!-- 关于对话框 -->
	<div 
		v-if="showAboutDialog"
		class="dialog-overlay fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50"
		@click="closeAboutDialog"
	>
		<div 
			class="dialog-content card p-6 max-w-md w-full mx-4"
			@click.stop
		>
			<div class="text-center">
				<div class="text-6xl mb-4">📚</div>
				<h3 class="text-2xl font-bold mb-2">VocabMemster</h3>
				<p class="text-gray-600 dark:text-gray-400 mb-4">
					版本 {{ appConfig.version }}
				</p>
				<p class="text-sm text-gray-500 dark:text-gray-400 mb-6">
					简约高效的单词背诵工具
				</p>
				<div class="space-y-2 text-sm">
					<p>开发者: {{ appConfig.author }}</p>
					<p>许可证: MIT License</p>
				</div>
			</div>
			<div class="flex justify-center mt-6">
				<button
					@click="closeAboutDialog"
					class="btn btn-primary"
				>
					关闭
				</button>
			</div>
		</div>
	</div>

	<!-- 使用说明对话框 -->
	<div 
		v-if="showUsageDialog"
		class="dialog-overlay fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50"
		@click="closeUsageDialog"
	>
		<div 
			class="dialog-content card p-6 max-w-4xl w-full mx-4 max-h-[80vh] overflow-y-auto"
			@click.stop
		>
			<h3 class="text-2xl font-bold mb-6">使用说明</h3>
			
			<div class="space-y-6">
				<!-- 基本操作 -->
				<section>
					<h4 class="text-lg font-semibold mb-3 text-blue-600">🎯 基本操作</h4>
					<ul class="space-y-2 text-sm">
						<li><strong>开始学习</strong>: 通过菜单加载单词库文件</li>
						<li><strong>回答单词</strong>: 在输入框中输入英文单词，按回车或点击提交</li>
						<li><strong>导航控制</strong>: 使用"下一个单词"加载新词，"查看答案"显示正确答案</li>
					</ul>
				</section>

				<!-- 快捷键 -->
				<section>
					<h4 class="text-lg font-semibold mb-3 text-green-600">⌨️ 快捷键</h4>
					<div class="grid grid-cols-1 md:grid-cols-2 gap-2 text-sm">
						<div class="flex justify-between">
							<span>Ctrl + Tab</span>
							<span class="text-gray-600">下一个单词</span>
						</div>
						<div class="flex justify-between">
							<span>Alt + S</span>
							<span class="text-gray-600">显示答案</span>
						</div>
						<div class="flex justify-between">
							<span>Ctrl + Shift + D</span>
							<span class="text-gray-600">切换主题</span>
						</div>
						<div class="flex justify-between">
							<span>Alt + O</span>
							<span class="text-gray-600">打开单词库</span>
						</div>
						<div class="flex justify-between">
							<span>Alt + E</span>
							<span class="text-gray-600">导出错词本</span>
						</div>
						<div class="flex justify-between">
							<span>Enter</span>
							<span class="text-gray-600">提交答案</span>
						</div>
					</div>
				</section>

				<!-- 学习功能 -->
				<section>
					<h4 class="text-lg font-semibold mb-3 text-red-600">📊 学习功能</h4>
					<ul class="space-y-2 text-sm">
						<li><strong>进度跟踪</strong>: 实时显示学习进度和统计信息</li>
						<li><strong>错词本</strong>: 自动记录拼写错误的单词</li>
						<li><strong>派生词检测</strong>: 自动检测并学习单词的派生形式</li>
					</ul>
				</section>

				<!-- 文件格式 -->
				<section>
					<h4 class="text-lg font-semibold mb-3 text-purple-600">📝 文件格式</h4>
					<p class="text-sm mb-2">单词库文件为文本格式，每行一个单词:</p>
					<code class="block bg-gray-100 dark:bg-gray-800 p-3 rounded text-sm mb-2">
						apple|n.|苹果|I eat an apple every day.
					</code>
					<code class="block bg-gray-100 dark:bg-gray-800 p-3 rounded text-sm">
						run|v.|跑步|He runs in the park every morning.
					</code>
				</section>
			</div>

			<div class="flex justify-center mt-6">
				<button
					@click="closeUsageDialog"
					class="btn btn-primary"
				>
					关闭
				</button>
			</div>
		</div>
	</div>

	<!-- 确认对话框 -->
	<div 
		v-if="showConfirmDialog"
		class="dialog-overlay fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50"
		@click="closeConfirmDialog"
	>
		<div 
			class="dialog-content card p-6 max-w-md w-full mx-4"
			@click.stop
		>
			<h3 class="text-xl font-bold mb-4">{{ confirmTitle }}</h3>
			<p class="text-gray-600 dark:text-gray-400 mb-6">
				{{ confirmMessage }}
			</p>
			<div class="flex justify-end space-x-3">
				<button
					@click="confirmCancel"
					class="btn btn-secondary"
				>
					取消
				</button>
				<button
					@click="confirmAction"
					:class="[
						'btn',
						confirmType === 'danger' ? 'btn-danger' : 'btn-primary'
					]"
				>
					{{ confirmButtonText }}
				</button>
			</div>
		</div>
	</div>

	<!-- 文件输入（隐藏） -->
	<input
		type="file"
		ref="fileInput"
		@change="onFileSelected"
		:accept="supportedFormats"
		class="hidden"
	/>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { useWordStore } from '@stores/word'
import { AppConfig } from '@settings/app.config'
import { bridge } from '@utils/bridge'

const wordStore = useWordStore()

// 对话框状态
const showFileDialog = ref(false)
const showAboutDialog = ref(false)
const showUsageDialog = ref(false)
const showConfirmDialog = ref(false)

// 确认对话框数据
const confirmTitle = ref('')
const confirmMessage = ref('')
const confirmButtonText = ref('确认')
const confirmType = ref('primary')
const confirmCallback = ref(null)

// 文件输入引用
const fileInput = ref(null)

// 计算属性
const appConfig = computed(() => AppConfig)
const supportedFormats = computed(() => AppConfig.supportedFileFormats.join(','))

// 对话框控制方法
const openFileDialog = () => {
	showFileDialog.value = true
}

const closeFileDialog = () => {
	showFileDialog.value = false
}

const openAboutDialog = () => {
	showAboutDialog.value = true
}

const closeAboutDialog = () => {
	showAboutDialog.value = false
}

const openUsageDialog = () => {
	showUsageDialog.value = true
}

const closeUsageDialog = () => {
	showUsageDialog.value = false
}

const openConfirmDialog = (title, message, buttonText = '确认', type = 'primary', callback = null) => {
	confirmTitle.value = title
	confirmMessage.value = message
	confirmButtonText.value = buttonText
	confirmType.value = type
	confirmCallback.value = callback
	showConfirmDialog.value = true
}

const closeConfirmDialog = () => {
	showConfirmDialog.value = false
	confirmCallback.value = null
}

const confirmAction = () => {
	if (confirmCallback.value) {
		confirmCallback.value(true)
	}
	closeConfirmDialog()
}

const confirmCancel = () => {
	if (confirmCallback.value) {
		confirmCallback.value(false)
	}
	closeConfirmDialog()
}

// 文件操作
const triggerFileInput = () => {
	fileInput.value?.click()
	closeFileDialog()
}

const onFileSelected = (event) => {
	const file = event.target.files[0]
	if (file) {
		wordStore.loadWordsFile(file.path || file.name)
	}
	event.target.value = ''
}

// 注册全局对话框方法
const registerGlobalMethods = () => {
	// 注册到 window 对象，供其他组件调用
	window.dialogSystem = {
		openFileDialog,
		openAboutDialog,
		openUsageDialog,
		openConfirmDialog
	}
}

// 监听键盘事件关闭对话框
const handleKeyPress = (event) => {
	if (event.key === 'Escape') {
		if (showFileDialog.value) closeFileDialog()
		if (showAboutDialog.value) closeAboutDialog()
		if (showUsageDialog.value) closeUsageDialog()
		if (showConfirmDialog.value) closeConfirmDialog()
	}
}

// 初始化
onMounted(() => {
	registerGlobalMethods()
	document.addEventListener('keydown', handleKeyPress)
})

// 清理
onUnmounted(() => {
	document.removeEventListener('keydown', handleKeyPress)
})

// 暴露方法给父组件
defineExpose({
	openFileDialog,
	openAboutDialog,
	openUsageDialog,
	openConfirmDialog
})
</script>

<style scoped>
.dialog-overlay {
	animation: fadeIn 0.2s ease-out;
}

.dialog-content {
	animation: slideUp 0.3s ease-out;
}

@keyframes fadeIn {
	from {
		opacity: 0;
	}
	to {
		opacity: 1;
	}
}

@keyframes slideUp {
	from {
		opacity: 0;
		transform: translateY(20px) scale(0.95);
	}
	to {
		opacity: 1;
		transform: translateY(0) scale(1);
	}
}
</style>