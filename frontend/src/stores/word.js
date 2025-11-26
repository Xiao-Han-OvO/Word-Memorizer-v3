import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { Constants } from '@settings/constants'
import { bridge } from '@utils/bridge'

export const useWordStore = defineStore('word', () => {
	// 状态
	const currentWord = ref(null)
	const learningState = ref(Constants.LEARNING_STATES.IDLE)
	const userAnswer = ref('')
	const feedback = ref('')
	const feedbackType = ref('')
	const stats = ref({
		totalWords: 0,
		masteredWords: 0,
		wrongWords: 0,
		remainingWords: 0,
		progress: 0
	})
	const allWords = ref([])
	const wrongWords = ref([])

	// 计算属性
	const isInputEnabled = computed(() => {
		return learningState.value === Constants.LEARNING_STATES.IDLE
	})

	const isAnswerCorrect = computed(() => {
		return feedbackType.value === Constants.FEEDBACK_TYPES.CORRECT
	})

	const hasCurrentWord = computed(() => {
		return currentWord.value && currentWord.value.word
	})

	// 动作
	const loadRandomWord = async () => {
		learningState.value = Constants.LEARNING_STATES.LOADING
		userAnswer.value = ''
		feedback.value = ''
		feedbackType.value = ''
		
		bridge.send(Constants.MESSAGE_TYPES.GET_RANDOM_WORD, {})
	}

	const submitAnswer = async () => {
		if (!userAnswer.value.trim() || !currentWord.value) return
		
		learningState.value = Constants.LEARNING_STATES.ANSWERED
		bridge.send(Constants.MESSAGE_TYPES.SUBMIT_ANSWER, {
			answer: userAnswer.value,
			currentWord: currentWord.value.word
		})
	}

	const showAnswer = () => {
		if (!currentWord.value) return
		
		learningState.value = Constants.LEARNING_STATES.REVIEWING
		bridge.send(Constants.MESSAGE_TYPES.SHOW_ANSWER, {
			currentWord: currentWord.value.word
		})
	}

	const loadWordsFile = async (filename) => {
		bridge.send(Constants.MESSAGE_TYPES.LOAD_WORDS_FILE, { filename })
	}

	const loadWordsFileContent = async (content, filename) => {
		bridge.send(Constants.MESSAGE_TYPES.LOAD_WORDS_CONTENT, { filename, content })
	}

	const resetProgress = () => {
		bridge.send(Constants.MESSAGE_TYPES.RESET_PROGRESS, {})
	}

	const getAllWords = () => {
		bridge.send(Constants.MESSAGE_TYPES.GET_ALL_WORDS, {})
	}

	// 更新状态的方法
	const updateCurrentWord = (wordData) => {
		currentWord.value = wordData
		learningState.value = Constants.LEARNING_STATES.IDLE
		userAnswer.value = ''
		feedback.value = ''
		feedbackType.value = ''
	}

	const updateAnswerResult = (result) => {
		feedback.value = result.message
		feedbackType.value = result.correct ? 
			Constants.FEEDBACK_TYPES.CORRECT : 
			Constants.FEEDBACK_TYPES.INCORRECT
	}

	const updateStats = (newStats) => {
		stats.value = { ...stats.value, ...newStats }
	}

	const updateAllWords = (words) => {
		allWords.value = words
	}

	// 初始化
	const initialize = () => {
		// 注册消息处理器
		bridge.on(Constants.MESSAGE_TYPES.WORD_LOADED, (data) => {
			updateCurrentWord(data)
		})

		bridge.on(Constants.MESSAGE_TYPES.ANSWER_RESULT, (data) => {
			updateAnswerResult(data)
		})

		bridge.on(Constants.MESSAGE_TYPES.ANSWER_SHOWN, (data) => {
			feedback.value = `正确答案: ${data.correctAnswer}`
			feedbackType.value = Constants.FEEDBACK_TYPES.SHOW_ANSWER
		})

		bridge.on(Constants.MESSAGE_TYPES.STATS_UPDATED, (data) => {
			updateStats(data)
		})

		bridge.on(Constants.MESSAGE_TYPES.ALL_WORDS, (data) => {
			updateAllWords(data)
		})

		bridge.on(Constants.MESSAGE_TYPES.PROGRESS_RESET, () => {
			loadRandomWord()
		})

		// 初始加载统计信息
		bridge.send(Constants.MESSAGE_TYPES.GET_STATS, {})
	}

	return {
		// 状态
		currentWord,
		learningState,
		userAnswer,
		feedback,
		feedbackType,
		stats,
		allWords,
		wrongWords,
		
		// 计算属性
		isInputEnabled,
		isAnswerCorrect,
		hasCurrentWord,
		
		// 动作
		loadRandomWord,
		submitAnswer,
		showAnswer,
		loadWordsFile,
		loadWordsFileContent,
		resetProgress,
		getAllWords,
		
		// 状态更新方法
		updateCurrentWord,
		updateAnswerResult,
		updateStats,
		updateAllWords,
		
		// 初始化
		initialize
	}
})