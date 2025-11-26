<template>
	<div class="word-card-container">
		<!-- 主要单词卡片 -->
		<div 
			class="word-card card p-8 text-center transition-all duration-300"
			:class="cardClasses"
		>
			<!-- 词性标签 -->
			<div 
				v-if="word?.pos"
				class="pos-tag tag tag-pos inline-block mb-4"
			>
				{{ word.pos }}
			</div>

			<!-- 中文释义 -->
			<h2 
				v-if="word?.meaning"
				class="meaning text-3xl md:text-4xl font-bold mb-6 text-gray-900 dark:text-white"
			>
				{{ word.meaning }}
			</h2>

			<!-- 例句 -->
			<div 
				v-if="word?.example && showExample"
				class="example text-lg text-gray-600 dark:text-gray-300 mb-6 italic"
			>
				"{{ word.example }}"
			</div>

			<!-- 反馈信息 -->
			<div 
				v-if="feedback"
				class="feedback mt-6 p-4 rounded-lg transition-all duration-300"
				:class="feedbackClasses"
			>
				<div class="flex items-center justify-center space-x-2">
					<span class="text-2xl">{{ feedbackIcon }}</span>
					<span class="text-lg font-medium">{{ feedback }}</span>
				</div>
			</div>

			<!-- 控制按钮 -->
			<div 
				v-if="showControls"
				class="controls mt-8 flex justify-center space-x-4"
			>
				<button
					@click="$emit('next-word')"
					class="btn btn-primary px-6 py-3"
				>
					下一个单词
				</button>
				<button
					v-if="showAnswerButton"
					@click="$emit('show-answer')"
					class="btn btn-secondary px-6 py-3"
				>
					查看答案
				</button>
			</div>

			<!-- 加载状态 -->
			<div 
				v-if="learningState === 'loading'"
				class="loading-state mt-6"
			>
				<div class="flex items-center justify-center space-x-2 text-gray-500">
					<div class="w-4 h-4 border-2 border-blue-500 border-t-transparent rounded-full animate-spin"></div>
					<span>加载中...</span>
				</div>
			</div>

			<!-- 派生词提示 -->
			<div 
				v-if="word?.hasDerivatives"
				class="derivatives-hint mt-6 p-3 bg-blue-50 dark:bg-blue-900/20 rounded-lg border border-blue-200 dark:border-blue-800"
			>
				<div class="flex items-center justify-center space-x-2 text-blue-700 dark:text-blue-300">
					<span>📚</span>
					<span class="text-sm">这个单词有派生词需要学习</span>
				</div>
			</div>
		</div>

		<!-- 单词信息面板 -->
		<div 
			v-if="word?.word && showWordInfo"
			class="word-info-panel card mt-4 p-4"
		>
			<div class="grid grid-cols-1 md:grid-cols-2 gap-4 text-sm">
				<div class="info-item">
					<span class="font-medium text-gray-500 dark:text-gray-400">英文:</span>
					<span class="ml-2 font-semibold text-gray-900 dark:text-white">{{ word.word }}</span>
				</div>
				<div class="info-item">
					<span class="font-medium text-gray-500 dark:text-gray-400">词性:</span>
					<span class="ml-2 text-gray-700 dark:text-gray-300">{{ word.pos }}</span>
				</div>
				<div class="info-item md:col-span-2">
					<span class="font-medium text-gray-500 dark:text-gray-400">释义:</span>
					<span class="ml-2 text-gray-700 dark:text-gray-300">{{ word.meaning }}</span>
				</div>
				<div 
					v-if="word.example"
					class="info-item md:col-span-2"
				>
					<span class="font-medium text-gray-500 dark:text-gray-400">例句:</span>
					<span class="ml-2 text-gray-700 dark:text-gray-300 italic">"{{ word.example }}"</span>
				</div>
			</div>
		</div>
	</div>
</template>

<script setup>
import { computed } from 'vue'

const props = defineProps({
	word: {
		type: Object,
		default: null
	},
	learningState: {
		type: String,
		default: 'idle'
	},
	feedback: {
		type: String,
		default: ''
	},
	feedbackType: {
		type: String,
		default: ''
	},
	showExample: {
		type: Boolean,
		default: true
	},
	showWordInfo: {
		type: Boolean,
		default: false
	}
})

const emit = defineEmits(['next-word', 'show-answer'])

// 计算属性
const showControls = computed(() => {
	return props.learningState === 'answered' || props.learningState === 'reviewing'
})

const showAnswerButton = computed(() => {
	return props.learningState === 'idle' && props.word
})

const cardClasses = computed(() => {
	const classes = []
	
	// 根据反馈类型添加样式
	if (props.feedbackType === 'correct') {
		classes.push('border-green-200', 'bg-green-50', 'dark:border-green-800', 'dark:bg-green-900/20')
	} else if (props.feedbackType === 'incorrect') {
		classes.push('border-red-200', 'bg-red-50', 'dark:border-red-800', 'dark:bg-red-900/20')
	} else if (props.feedbackType === 'show_answer') {
		classes.push('border-blue-200', 'bg-blue-50', 'dark:border-blue-800', 'dark:bg-blue-900/20')
	}
	
	// 悬停效果
	classes.push('hover:shadow-card-hover')
	
	return classes
})

const feedbackClasses = computed(() => {
	if (props.feedbackType === 'correct') {
		return 'bg-green-100 text-green-800 dark:bg-green-800/30 dark:text-green-300'
	} else if (props.feedbackType === 'incorrect') {
		return 'bg-red-100 text-red-800 dark:bg-red-800/30 dark:text-red-300'
	} else if (props.feedbackType === 'show_answer') {
		return 'bg-blue-100 text-blue-800 dark:bg-blue-800/30 dark:text-blue-300'
	}
	return 'bg-gray-100 text-gray-800 dark:bg-gray-800/30 dark:text-gray-300'
})

const feedbackIcon = computed(() => {
	if (props.feedbackType === 'correct') {
		return '✅'
	} else if (props.feedbackType === 'incorrect') {
		return '❌'
	} else if (props.feedbackType === 'show_answer') {
		return '💡'
	}
	return 'ℹ️'
})
</script>

<style scoped>
.word-card {
	min-height: 300px;
	display: flex;
	flex-direction: column;
	justify-content: center;
	align-items: center;
}

.meaning {
	line-height: 1.2;
	word-break: break-word;
}

.example {
	max-width: 600px;
	line-height: 1.5;
}

.feedback {
	animation: bounceIn 0.5s ease-out;
}

.derivatives-hint {
	animation: pulse 2s infinite;
}

@keyframes bounceIn {
	0% {
		transform: scale(0.3);
		opacity: 0;
	}
	50% {
		transform: scale(1.05);
	}
	70% {
		transform: scale(0.9);
	}
	100% {
		transform: scale(1);
		opacity: 1;
	}
}

.info-item {
	display: flex;
	align-items: flex-start;
}
</style>