import { defineStore } from 'pinia'
import { ref, computed, watch } from 'vue'
import { ThemeConfig } from '@settings/theme.config'
import { bridge } from '@utils/bridge'

export const useThemeStore = defineStore('theme', () => {
	// 状态
	const isDarkMode = ref(false)
	const currentTheme = ref('blue')
	const customColors = ref({
		posColor: ThemeConfig.defaults.posColor,
		correctColor: ThemeConfig.defaults.correctColor,
		errorColor: ThemeConfig.defaults.errorColor,
		answerColor: ThemeConfig.defaults.answerColor
	})

	// 计算属性
	const themeColors = computed(() => {
		return ThemeConfig.colorThemes[currentTheme.value] || ThemeConfig.colorThemes.blue
	})

	const cssVariables = computed(() => {
		const theme = themeColors.value
		return {
			'--color-primary': theme.primary,
			'--color-secondary': theme.secondary,
			'--color-accent': theme.accent,
			'--color-pos': customColors.value.posColor,
			'--color-correct': customColors.value.correctColor,
			'--color-error': customColors.value.errorColor,
			'--color-answer': customColors.value.answerColor,
			'--app-bg-color': isDarkMode.value ? 
				ThemeConfig.darkModeBackgrounds[currentTheme.value] : 
				ThemeConfig.lightModeBackgrounds[currentTheme.value],
			'--main-bg-color': isDarkMode.value ? 
				ThemeConfig.darkModeBackgrounds[currentTheme.value] : 
				'#ffffff',
			'--text-color': isDarkMode.value ? '#f7fafc' : '#1a202c',
			'--card-bg-color': isDarkMode.value ? 
				ThemeConfig.darkModeBackgrounds[currentTheme.value].replace('0f', '1e') : 
				'#ffffff',
			'--border-color': isDarkMode.value ? 
				ThemeConfig.darkModeBackgrounds[currentTheme.value].replace('0f', '3a') : 
				'#e2e8f0'
		}
	})

	// 动作
	const toggleDarkMode = () => {
		isDarkMode.value = !isDarkMode.value
		updateCssVariables()
		saveToBackend()
	}

	const setTheme = (themeName) => {
		if (ThemeConfig.colorThemes[themeName]) {
			currentTheme.value = themeName
			updateCssVariables()
			saveToBackend()
		}
	}

	const setCustomColor = (colorType, colorValue) => {
		if (customColors.value[colorType] !== undefined) {
			customColors.value[colorType] = colorValue
			updateCssVariables()
			saveToBackend()
		}
	}

	// 工具方法
	const updateCssVariables = () => {
		const root = document.documentElement
		Object.entries(cssVariables.value).forEach(([key, value]) => {
			root.style.setProperty(key, value)
		})
	}

	const saveToBackend = () => {
		const settings = {
			darkMode: isDarkMode.value,
			colorTheme: currentTheme.value,
			posColor: customColors.value.posColor,
			correctColor: customColors.value.correctColor,
			errorColor: customColors.value.errorColor,
			answerColor: customColors.value.answerColor
		}
		bridge.send('saveSettings', settings)
	}

	const loadFromBackend = (settings) => {
		if (settings.darkMode !== undefined) {
			isDarkMode.value = settings.darkMode
		}
		if (settings.colorTheme) {
			currentTheme.value = settings.colorTheme
		}
		if (settings.posColor) {
			customColors.value.posColor = settings.posColor
		}
		if (settings.correctColor) {
			customColors.value.correctColor = settings.correctColor
		}
		if (settings.errorColor) {
			customColors.value.errorColor = settings.errorColor
		}
		if (settings.answerColor) {
			customColors.value.answerColor = settings.answerColor
		}
		updateCssVariables()
	}

	// 初始化
	const initializeTheme = () => {
		// 检测系统主题偏好
		if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
			isDarkMode.value = true
		}
		
		// 应用CSS变量
		updateCssVariables()
		
		// 监听系统主题变化
		window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', (e) => {
			isDarkMode.value = e.matches
			updateCssVariables()
		})
	}

	return {
		// 状态
		isDarkMode,
		currentTheme,
		customColors,
		
		// 计算属性
		themeColors,
		cssVariables,
		
		// 动作
		toggleDarkMode,
		setTheme,
		setCustomColor,
		
		// 方法
		updateCssVariables,
		saveToBackend,
		loadFromBackend,
		initializeTheme
	}
})