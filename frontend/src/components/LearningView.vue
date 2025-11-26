<template>
	<div class="learning-view container-custom py-6">
		<!-- 学习统计 -->
		<div class="stats-section mb-8">
			<div class="grid grid-cols-2 md:grid-cols-4 gap-4">
				<div class="stat-card card p-4 text-center">
					<div class="text-2xl font-bold text-blue-600 dark:text-blue-400">
						{{ stats.totalWords }}
					</div>
					<div class="text-sm text-gray-600 dark:text-gray-400">总单词</div>
				</div>
				<div class="stat-card card p-4 text-center">
					<div class="text-2xl font-bold text-green-600 dark:text-green-400">
						{{ stats.masteredWords }}
					</div>
					<div class="text-sm text-gray-600 dark:text-gray-400">已掌握</div>
				</div>
				<div class="stat-card card p-4 text-center">
					<div class="text-2xl font-bold text-red-600 dark:text-red-400">
						{{ stats.wrongWords }}
					</div>
					<div class="text-sm text-gray-600 dark:text-gray-400">错词</div>
				</div>
				<div class="stat-card card p-4 text-center">
					<div class="text-2xl font-bold text-purple-600 dark:text-purple-400">
						{{ Math.round(stats.progress * 100) }}%
					</div>
					<div class="text-sm text-gray-600 dark:text-gray-400">进度</div>
				</div>
			</div>
		</div>

		<!-- 进度条 -->
		<div class="progress-section mb-8">
			<ProgressBar :progress="stats.progress" />
		</div>

		<!-- 单词卡片区域 -->
		<div class="word-section mb-8">
			<WordCard 
				:word="currentWord"
				:learning-state="learningState"
				:feedback="feedback"
				:feedback-type="feedbackType"
				@next-word="loadRandomWord"
				@show-answer="showAnswer"
			/>
		</div>

		<!-- 输入区域 -->
		<div class="input-section mb-8" v-if="hasCurrentWord">
			<div class="card p-6">
				<div class="flex flex-col md:flex-row gap-4 items-center">
					<input
						v-model="userAnswer"
						@keyup.enter="submitAnswer"
						:disabled="!isInputEnabled"
						:placeholder="isInputEnabled ? '请输入英文单词...' : '等待下一题...'"
						class="input flex-1 text-lg py-3"
						ref="answerInput"
					/>
					<div class="flex gap-2">
						<button
							@click="submitAnswer"
							:disabled="!isInputEnabled || !userAnswer.trim()"
							class="btn btn-primary px-6 py-3 text-lg"
						>
							提交答案
						</button>
						<button
							@click="showAnswer"
							:disabled="!isInputEnabled"
							class="btn btn-secondary px-6 py-3 text-lg"
						>
							查看答案
						</button>
					</div>
				</div>
			</div>
		</div>

		<!-- 空状态 -->
		<div v-else class="empty-state text-center py-16">
			<div class="card p-8 max-w-md mx-auto">
				<div class="text-6xl mb-4">📚</div>
				<h3 class="text-xl font-semibold mb-2">还没有单词库</h3>
				<p class="text-gray-600 dark:text-gray-400 mb-6">
					请先加载单词库文件开始学习
				</p>
				<button
					@click="openFileDialog"
					class="btn btn-primary px-6 py-3"
				>
					打开单词库
				</button>
			</div>
		</div>

		<!-- 快捷键提示 -->
		<div class="shortcuts-section mt-8">
			<div class="text-center">
				<p class="text-sm text-gray-500 dark:text-gray-400">
					快捷键: 
					<kbd class="px-2 py-1 text-xs bg-gray-200 dark:bg-gray-700 rounded">Ctrl+Tab</kbd> 下一个单词 • 
					<kbd class="px-2 py-1 text-xs bg-gray-200 dark:bg-gray-700 rounded">Alt+S</kbd> 显示答案 • 
					<kbd class="px-2 py-1 text-xs bg-gray-200 dark:bg-gray-700 rounded">Enter</kbd> 提交答案
				</p>
			</div>
		</div>

		<!-- 文件选择对话框 -->
		<input
			type="file"
			ref="fileInput"
			@change="onFileSelected"
			:accept="supportedFormats"
			class="hidden"
		/>
	</div>
</template>

<script setup>
import { ref, computed, onMounted, nextTick, watch } from 'vue'
import { useWordStore } from '@stores/word'
import { AppConfig } from '@settings/app.config'
import { bridge } from '@utils/bridge'
import ProgressBar from './ProgressBar.vue'
import WordCard from './WordCard.vue'

const wordStore = useWordStore()

// 响应式数据
const fileInput = ref(null)

// 从 store 获取状态
const {
	currentWord,
	learningState,
	userAnswer,
	feedback,
	feedbackType,
	stats,
	loadRandomWord,
	submitAnswer,
	showAnswer,
	loadWordsFile
} = wordStore

// 计算属性
const hasCurrentWord = computed(() => wordStore.hasCurrentWord)
const isInputEnabled = computed(() => wordStore.isInputEnabled)
const isAnswerCorrect = computed(() => wordStore.isAnswerCorrect)
const supportedFormats = computed(() => AppConfig.supportedFileFormats.join(','))

// 方法
const openFileDialog = () => {
	fileInput.value?.click()
}

const onFileSelected = (event) => {
	const file = event.target.files[0]
	if (file) {
		loadWordsFile(file.path || file.name)
	}
	// 重置 input
	event.target.value = ''
}

// 自动聚焦到输入框
const answerInput = ref(null)
const focusInput = () => {
	nextTick(() => {
		if (answerInput.value && isInputEnabled.value) {
			answerInput.value.focus()
		}
	})
}

// 监听状态变化
watch(isInputEnabled, focusInput)
watch(learningState, focusInput)

// 初始化
onMounted(() => {
	// 初始化单词存储
	wordStore.initialize()
	// 加载第一个单词
	loadRandomWord()
	// 聚焦输入框
	focusInput()
})
</script>

<style scoped>
.learning-view {
	min-height: calc(100vh - 80px);
	display: flex;
	flex-direction: column;
}

.stat-card {
	transition: all 0.3s ease;
}

.stat-card:hover {
	transform: translateY(-2px);
	box-shadow: 0 8px 25px -8px rgba(0, 0, 0, 0.15);
}

.empty-state {
	flex: 1;
	display: flex;
	align-items: center;
	justify-content: center;
}

.shortcuts-section kbd {
	font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
}
</style>