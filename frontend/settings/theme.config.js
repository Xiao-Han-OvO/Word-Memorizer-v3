// 主题配置
export const ThemeConfig = {
	// 颜色主题映射
	colorThemes: {
		blue: {
			primary: '#3498db',
			secondary: '#2980b9',
			accent: '#74b9ff',
			name: '蓝色主题'
		},
		red: {
			primary: '#e74c3c',
			secondary: '#c0392b',
			accent: '#ff6b6b',
			name: '红色主题'
		},
		orange: {
			primary: '#e67e22',
			secondary: '#d35400',
			accent: '#ffa726',
			name: '橙色主题'
		},
		green: {
			primary: '#2ecc71',
			secondary: '#27ae60',
			accent: '#4cd964',
			name: '绿色主题'
		},
		cyan: {
			primary: '#1abc9c',
			secondary: '#16a085',
			accent: '#4ecdc4',
			name: '青色主题'
		},
		yellow: {
			primary: '#f1c40f',
			secondary: '#f39c12',
			accent: '#ffeaa7',
			name: '黄色主题'
		},
		brown: {
			primary: '#a0522d',
			secondary: '#8b4513',
			accent: '#d2691e',
			name: '棕色主题'
		},
		mono: {
			primary: '#7f8c8d',
			secondary: '#34495e',
			accent: '#95a5a6',
			name: '黑白主题'
		},
		purple: {
			primary: '#9b59b6',
			secondary: '#8e44ad',
			accent: '#a29bfe',
			name: '紫色主题'
		},
		pink: {
			primary: '#e91e63',
			secondary: '#ad1457',
			accent: '#fd79a8',
			name: '粉色主题'
		}
	},

	// 深色模式背景色配置
	darkModeBackgrounds: {
		blue: '#0f141a',
		red: '#161111',
		orange: '#1a140f',
		green: '#0f160f',
		cyan: '#0f1616',
		yellow: '#16160f',
		brown: '#161310',
		mono: '#0a0a0a',
		purple: '#161016',
		pink: '#160f13'
	},

	// 浅色模式背景色配置
	lightModeBackgrounds: {
		blue: '#f0f8ff',
		red: '#fae6e6',
		orange: '#fff4e6',
		green: '#e6f7e6',
		cyan: '#e6f7f7',
		yellow: '#fffde6',
		brown: '#f7f3e6',
		mono: '#f5f5f5',
		purple: '#f3e6f7',
		pink: '#fce6f5'
	},

	// 默认颜色
	defaults: {
		posColor: 'rgb(52,152,219)',
		correctColor: 'rgb(46,204,113)',
		errorColor: 'rgb(231,76,60)',
		answerColor: 'rgb(155,89,182)'
	}
}