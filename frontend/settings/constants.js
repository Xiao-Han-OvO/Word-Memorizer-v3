// 应用常量定义
export const Constants = {
	// 文件相关
	FILE_FORMATS: {
		TXT: '.txt',
		WDLST: '.wdlst'
	},
	
	// 学习状态
	LEARNING_STATES: {
		IDLE: 'idle',
		LOADING: 'loading',
		ANSWERED: 'answered',
		REVIEWING: 'reviewing',
		COMPLETED: 'completed'
	},
	
	// 反馈类型
	FEEDBACK_TYPES: {
		CORRECT: 'correct',
		INCORRECT: 'incorrect',
		SHOW_ANSWER: 'show_answer',
		HINT: 'hint'
	},
	
	// 消息类型（前后端通信）
	MESSAGE_TYPES: {
		// 从前端到后端
		GET_RANDOM_WORD: 'getRandomWord',
		SUBMIT_ANSWER: 'submitAnswer',
		SHOW_ANSWER: 'showAnswer',
		LOAD_WORDS_FILE: 'loadWordsFile',
		SAVE_SETTINGS: 'saveSettings',
		EXPORT_WRONG_WORDS: 'exportWrongWords',
		RESET_PROGRESS: 'resetProgress',
		GET_STATS: 'getStats',
		GET_ALL_WORDS: 'getAllWords',
		
		// 从后端到前端
		WORD_LOADED: 'wordLoaded',
		ANSWER_RESULT: 'answerResult',
		ANSWER_SHOWN: 'answerShown',
		WORDS_LOADED: 'wordsLoaded',
		STATS_UPDATED: 'statsUpdated',
		ALL_WORDS: 'allWords',
		PROGRESS_RESET: 'progressReset',
		ERROR: 'error'
	},
	
	// 错误代码
	ERROR_CODES: {
		FILE_NOT_FOUND: 1001,
		FILE_PARSE_ERROR: 1002,
		NETWORK_ERROR: 1003,
		UNKNOWN_ERROR: 9999
	}
}

// 本地存储键名
export const StorageKeys = {
	THEME_PREFERENCES: 'vocabmemster_theme_prefs',
	LEARNING_PROGRESS: 'vocabmemster_progress',
	USER_SETTINGS: 'vocabmemster_settings'
}