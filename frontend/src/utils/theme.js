import { ThemeConfig } from '@settings/theme.config'

// 主题工具函数
export const themeUtils = {
	// 获取主题颜色
	getThemeColor(themeName, colorType = 'primary') {
		const theme = ThemeConfig.colorThemes[themeName] || ThemeConfig.colorThemes.blue
		return theme[colorType] || theme.primary
	},

	// 获取背景颜色
	getBackgroundColor(themeName, isDarkMode = false) {
		const backgrounds = isDarkMode ? 
			ThemeConfig.darkModeBackgrounds : 
			ThemeConfig.lightModeBackgrounds
		return backgrounds[themeName] || backgrounds.blue
	},

	// 生成颜色变体
	generateColorVariants(baseColor) {
		// 简单的颜色变体生成
		return {
			light: this.lightenColor(baseColor, 20),
			dark: this.darkenColor(baseColor, 20),
			lighter: this.lightenColor(baseColor, 40),
			darker: this.darkenColor(baseColor, 40)
		}
	},

	// 变亮颜色
	lightenColor(hex, percent) {
		const num = parseInt(hex.replace('#', ''), 16)
		const amt = Math.round(2.55 * percent)
		const R = (num >> 16) + amt
		const G = (num >> 8 & 0x00FF) + amt
		const B = (num & 0x0000FF) + amt
		
		return '#' + (
			0x1000000 +
			(R < 255 ? R < 1 ? 0 : R : 255) * 0x10000 +
			(G < 255 ? G < 1 ? 0 : G : 255) * 0x100 +
			(B < 255 ? B < 1 ? 0 : B : 255)
		).toString(16).slice(1)
	},

	// 变暗颜色
	darkenColor(hex, percent) {
		const num = parseInt(hex.replace('#', ''), 16)
		const amt = Math.round(2.55 * percent)
		const R = (num >> 16) - amt
		const G = (num >> 8 & 0x00FF) - amt
		const B = (num & 0x0000FF) - amt
		
		return '#' + (
			0x1000000 +
			(R > 0 ? R : 0) * 0x10000 +
			(G > 0 ? G : 0) * 0x100 +
			(B > 0 ? B : 0)
		).toString(16).slice(1)
	},

	// 检查颜色对比度
	getContrastColor(backgroundColor) {
		// 简单的亮度计算
		const hex = backgroundColor.replace('#', '')
		const r = parseInt(hex.substr(0, 2), 16)
		const g = parseInt(hex.substr(2, 2), 16)
		const b = parseInt(hex.substr(4, 2), 16)
		
		const brightness = (r * 299 + g * 587 + b * 114) / 1000
		return brightness > 128 ? '#000000' : '#ffffff'
	},

	// 应用主题到元素
	applyThemeToElement(element, themeName, isDarkMode = false) {
		const theme = ThemeConfig.colorThemes[themeName] || ThemeConfig.colorThemes.blue
		const bgColor = this.getBackgroundColor(themeName, isDarkMode)
		
		element.style.setProperty('--theme-primary', theme.primary)
		element.style.setProperty('--theme-secondary', theme.secondary)
		element.style.setProperty('--theme-accent', theme.accent)
		element.style.setProperty('--theme-bg', bgColor)
	}
}

// CSS 工具函数
export const cssUtils = {
	// 生成渐变背景
	generateGradient(colors, direction = 'to right') {
		return `linear-gradient(${direction}, ${colors.join(', ')})`
	},

	// 生成阴影
	generateShadow(color, opacity = 0.1) {
		return `0 4px 6px -1px rgba(${this.hexToRgb(color)}, ${opacity}), 0 2px 4px -1px rgba(${this.hexToRgb(color)}, ${opacity * 0.6})`
	},

	// 十六进制颜色转 RGB
	hexToRgb(hex) {
		const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex)
		return result ? 
			`${parseInt(result[1], 16)}, ${parseInt(result[2], 16)}, ${parseInt(result[3], 16)}` : 
			'0, 0, 0'
	}
}

export default {
	themeUtils,
	cssUtils
}