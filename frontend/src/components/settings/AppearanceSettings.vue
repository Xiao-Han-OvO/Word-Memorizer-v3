<template>
	<div class="appearance-settings space-y-6">
		<!-- 主题选择 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">主题设置</h3>
			
			<!-- 颜色主题 -->
			<div class="mb-6">
				<label class="block text-sm font-medium mb-3">颜色主题</label>
				<div class="grid grid-cols-2 md:grid-cols-5 gap-3">
					<div
						v-for="theme in colorThemes"
						:key="theme.id"
						@click="setTheme(theme.id)"
						:class="[
							'theme-option p-4 rounded-lg cursor-pointer transition-all border-2',
							currentTheme === theme.id
								? 'border-blue-500 ring-2 ring-blue-200 dark:ring-blue-800'
								: 'border-gray-200 dark:border-gray-700 hover:border-gray-300 dark:hover:border-gray-600'
						]"
						:style="{
							backgroundColor: theme.colors.primary + '20',
							borderLeftColor: theme.colors.primary
						}"
					>
						<div class="flex items-center space-x-2">
							<div 
								class="w-4 h-4 rounded-full"
								:style="{ backgroundColor: theme.colors.primary }"
							></div>
							<span class="text-sm font-medium">{{ theme.name }}</span>
						</div>
					</div>
				</div>
			</div>

			<!-- 明暗模式 -->
			<div class="mb-6">
				<label class="block text-sm font-medium mb-3">明暗模式</label>
				<div class="flex space-x-4">
					<button
						@click="setDarkMode(false)"
						:class="[
							'theme-mode-btn p-4 rounded-lg border-2 flex-1 text-center transition-all',
							!isDarkMode
								? 'border-blue-500 bg-blue-50 dark:bg-blue-900/20 text-blue-700 dark:text-blue-300'
								: 'border-gray-200 dark:border-gray-700 text-gray-600 dark:text-gray-400 hover:border-gray-300 dark:hover:border-gray-600'
						]"
					>
						<div class="text-2xl mb-2">☀️</div>
						<div class="text-sm font-medium">浅色模式</div>
					</button>
					<button
						@click="setDarkMode(true)"
						:class="[
							'theme-mode-btn p-4 rounded-lg border-2 flex-1 text-center transition-all',
							isDarkMode
								? 'border-blue-500 bg-blue-50 dark:bg-blue-900/20 text-blue-700 dark:text-blue-300'
								: 'border-gray-200 dark:border-gray-700 text-gray-600 dark:text-gray-400 hover:border-gray-300 dark:hover:border-gray-600'
						]"
					>
						<div class="text-2xl mb-2">🌙</div>
						<div class="text-sm font-medium">深色模式</div>
					</button>
				</div>
			</div>
		</div>

		<!-- 自定义颜色 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">自定义颜色</h3>
			
			<div class="space-y-4">
				<!-- 词性颜色 -->
				<div class="color-setting">
					<label class="block text-sm font-medium mb-2">词性标签颜色</label>
					<div class="flex items-center space-x-3">
						<input
							type="color"
							:value="customColors.posColor"
							@input="updateColor('posColor', $event.target.value)"
							class="w-12 h-12 rounded cursor-pointer"
						>
						<div class="flex-1">
							<input
								type="text"
								:value="customColors.posColor"
								@input="updateColor('posColor', $event.target.value)"
								class="input text-sm"
								placeholder="#3498db"
							>
						</div>
						<div class="tag tag-pos px-3 py-1">n.</div>
					</div>
				</div>

				<!-- 正确颜色 -->
				<div class="color-setting">
					<label class="block text-sm font-medium mb-2">正确反馈颜色</label>
					<div class="flex items-center space-x-3">
						<input
							type="color"
							:value="customColors.correctColor"
							@input="updateColor('correctColor', $event.target.value)"
							class="w-12 h-12 rounded cursor-pointer"
						>
						<div class="flex-1">
							<input
								type="text"
								:value="customColors.correctColor"
								@input="updateColor('correctColor', $event.target.value)"
								class="input text-sm"
								placeholder="#27ae60"
							>
						</div>
						<div class="px-3 py-1 rounded bg-green-100 text-green-800 dark:bg-green-900 dark:text-green-200 text-sm">正确</div>
					</div>
				</div>

				<!-- 错误颜色 -->
				<div class="color-setting">
					<label class="block text-sm font-medium mb-2">错误反馈颜色</label>
					<div class="flex items-center space-x-3">
						<input
							type="color"
							:value="customColors.errorColor"
							@input="updateColor('errorColor', $event.target.value)"
							class="w-12 h-12 rounded cursor-pointer"
						>
						<div class="flex-1">
							<input
								type="text"
								:value="customColors.errorColor"
								@input="updateColor('errorColor', $event.target.value)"
								class="input text-sm"
								placeholder="#e74c3c"
							>
						</div>
						<div class="px-3 py-1 rounded bg-red-100 text-red-800 dark:bg-red-900 dark:text-red-200 text-sm">错误</div>
					</div>
				</div>

				<!-- 重置按钮 -->
				<div class="pt-4">
					<button
						@click="resetColors"
						class="btn btn-secondary"
					>
						重置为默认颜色
					</button>
				</div>
			</div>
		</div>

		<!-- 预览区域 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">预览</h3>
			<div class="preview-area card p-6">
				<div class="space-y-4">
					<!-- 单词卡片预览 -->
					<div class="preview-card card p-4 border-2">
						<div class="flex items-center justify-between mb-3">
							<div class="tag tag-pos">n.</div>
							<div class="text-sm text-gray-500">预览</div>
						</div>
						<h4 class="text-xl font-bold mb-2">苹果</h4>
						<p class="text-gray-600 dark:text-gray-400 italic">
							"I eat an apple every day."
						</p>
					</div>

					<!-- 按钮预览 -->
					<div class="flex space-x-3">
						<button class="btn btn-primary">主要按钮</button>
						<button class="btn btn-secondary">次要按钮</button>
						<button class="btn btn-success">成功按钮</button>
						<button class="btn btn-danger">危险按钮</button>
					</div>

					<!-- 进度条预览 -->
					<div class="progress-bar w-full bg-gray-200 dark:bg-gray-700 rounded-full h-2">
						<div 
							class="progress-fill h-2 rounded-full"
							:style="{
								width: '60%',
								backgroundColor: themeColors.primary
							}"
						></div>
					</div>
				</div>
			</div>
		</div>
	</div>
</template>

<script setup>
import { computed } from 'vue'
import { useThemeStore } from '@stores/theme'
import { ThemeConfig } from '@settings/theme.config'

const themeStore = useThemeStore()

// 从 store 获取状态和方法
const { 
	isDarkMode, 
	currentTheme, 
	customColors, 
	setTheme, 
	setDarkMode, 
	setCustomColor 
} = themeStore

// 计算属性
const themeColors = computed(() => {
	return ThemeConfig.colorThemes[currentTheme.value] || ThemeConfig.colorThemes.blue
})

const colorThemes = computed(() => {
	return Object.entries(ThemeConfig.colorThemes).map(([id, theme]) => ({
		id,
		name: theme.name,
		colors: theme
	}))
})

// 方法
const updateColor = (colorType, value) => {
	setCustomColor(colorType, value)
}

const resetColors = () => {
	setCustomColor('posColor', ThemeConfig.defaults.posColor)
	setCustomColor('correctColor', ThemeConfig.defaults.correctColor)
	setCustomColor('errorColor', ThemeConfig.defaults.errorColor)
	setCustomColor('answerColor', ThemeConfig.defaults.answerColor)
}
</script>

<style scoped>
.theme-option {
	border-left-width: 4px;
	transition: all 0.2s ease;
}

.theme-option:hover {
	transform: translateY(-2px);
}

.theme-mode-btn {
	transition: all 0.2s ease;
}

.theme-mode-btn:hover {
	transform: translateY(-1px);
}

.color-setting input[type="color"] {
	border: none;
	padding: 0;
	background: none;
}

.preview-card {
	background-color: var(--card-bg-color);
	border-color: var(--border-color);
}
</style>