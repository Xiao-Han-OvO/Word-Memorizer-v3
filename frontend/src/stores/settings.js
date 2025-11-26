import { defineStore } from 'pinia'
import { ref } from 'vue'
import { AppConfig } from '@settings/app.config'
import { bridge } from '@utils/bridge'

export const useSettingsStore = defineStore('settings', () => {
	// 状态
	const appSettings = ref({
		// 学习设置
		autoAdvance: true,
		autoAdvanceDelay: 1500,
		caseSensitive: false,
		enableHints: true,
		
		// 界面设置
		showExamples: true,
		showDerivatives: true,
		animationEnabled: true,
		
		// 文件设置
		defaultFileFormat: AppConfig.defaultFileFormat,
		rememberLastFile: true
	})

	// 动作
	const updateSetting = (key, value) => {
		if (appSettings.value[key] !== undefined) {
			appSettings.value[key] = value
			saveToBackend()
		}
	}

	const resetToDefaults = () => {
		appSettings.value = {
			autoAdvance: true,
			autoAdvanceDelay: 1500,
			caseSensitive: false,
			enableHints: true,
			showExamples: true,
			showDerivatives: true,
			animationEnabled: true,
			defaultFileFormat: AppConfig.defaultFileFormat,
			rememberLastFile: true
		}
		saveToBackend()
	}

	// 工具方法
	const saveToBackend = () => {
		bridge.send('saveAppSettings', appSettings.value)
	}

	const loadFromBackend = (settings) => {
		if (settings) {
			appSettings.value = { ...appSettings.value, ...settings }
		}
	}

	return {
		// 状态
		appSettings,
		
		// 动作
		updateSetting,
		resetToDefaults,
		
		// 方法
		saveToBackend,
		loadFromBackend
	}
})