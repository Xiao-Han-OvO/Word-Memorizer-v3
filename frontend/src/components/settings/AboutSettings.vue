<template>
	<div class="about-settings space-y-6">
		<!-- 应用信息 -->
		<div class="setting-section text-center">
			<div class="card p-8">
				<div class="text-6xl mb-4">📚</div>
				<h2 class="text-3xl font-bold mb-2">VocabMemster</h2>
				<p class="text-gray-600 dark:text-gray-400 mb-4">
					版本 {{ appConfig.version }}
				</p>
				<p class="text-sm text-gray-500 dark:text-gray-400 max-w-md mx-auto">
					{{ appConfig.appName }} 是一个简约高效的单词背诵工具，帮助您轻松记忆英语单词。
				</p>
			</div>
		</div>

		<!-- 版本信息 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">版本信息</h3>
			
			<div class="card p-4 space-y-3">
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">应用版本:</span>
					<span class="text-sm font-medium">{{ appConfig.version }}</span>
				</div>
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">构建日期:</span>
					<span class="text-sm font-medium">{{ buildDate }}</span>
				</div>
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">前端框架:</span>
					<span class="text-sm font-medium">Vue 3 + Vite</span>
				</div>
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">后端框架:</span>
					<span class="text-sm font-medium">C++ + GTK + WebKit</span>
				</div>
			</div>
		</div>

		<!-- 开发者信息 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">开发者信息</h3>
			
			<div class="card p-4 space-y-3">
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">开发者:</span>
					<span class="text-sm font-medium">{{ appConfig.author }}</span>
				</div>
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">许可证:</span>
					<span class="text-sm font-medium">MIT License</span>
				</div>
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">源代码:</span>
					<a 
						href="#" 
						class="text-sm font-medium text-blue-600 hover:text-blue-800 dark:text-blue-400 dark:hover:text-blue-300"
						@click="openRepository"
					>
						GitHub Repository
					</a>
				</div>
			</div>
		</div>

		<!-- 系统信息 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">系统信息</h3>
			
			<div class="card p-4 space-y-3">
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">用户代理:</span>
					<span class="text-sm font-medium">{{ userAgent }}</span>
				</div>
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">语言:</span>
					<span class="text-sm font-medium">{{ language }}</span>
				</div>
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">时区:</span>
					<span class="text-sm font-medium">{{ timezone }}</span>
				</div>
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">屏幕分辨率:</span>
					<span class="text-sm font-medium">{{ screenResolution }}</span>
				</div>
			</div>
		</div>

		<!-- 操作按钮 -->
		<div class="setting-section">
			<div class="grid grid-cols-1 md:grid-cols-2 gap-3">
				<button
					@click="checkForUpdates"
					class="btn btn-primary"
				>
					🔄 检查更新
				</button>
				<button
					@click="showLicense"
					class="btn btn-secondary"
				>
					📄 查看许可证
				</button>
				<button
					@click="showChangelog"
					class="btn btn-secondary"
				>
					📋 更新日志
				</button>
				<button
					@click="showSystemInfo"
					class="btn btn-secondary"
				>
					💻 系统信息
				</button>
			</div>
		</div>

		<!-- 致谢 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">致谢</h3>
			
			<div class="card p-4">
				<p class="text-sm text-gray-600 dark:text-gray-400 mb-3">
					感谢以下开源项目和技术：
				</p>
				<ul class="text-sm space-y-1 text-gray-500 dark:text-gray-400">
					<li>• Vue.js - 渐进式 JavaScript 框架</li>
					<li>• Vite - 下一代前端工具</li>
					<li>• Tailwind CSS - 实用优先的 CSS 框架</li>
					<li>• GTK - 跨平台图形工具包</li>
					<li>• WebKit - Web 浏览器引擎</li>
					<li>• nlohmann/json - C++ JSON 库</li>
				</ul>
			</div>
		</div>
	</div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { AppConfig } from '@settings/app.config'

const appConfig = AppConfig

// 响应式数据
const buildDate = ref('2024-01-15')
const userAgent = ref('')
const language = ref('')
const timezone = ref('')
const screenResolution = ref('')

// 方法
const openRepository = () => {
	// 打开 GitHub 仓库
	window.open('https://github.com/username/vocabmemster', '_blank')
}

const checkForUpdates = () => {
	bridge.send('checkForUpdates', {})
}

const showLicense = () => {
	bridge.send('showLicense', {})
}

const showChangelog = () => {
	bridge.send('showChangelog', {})
}

const showSystemInfo = () => {
	bridge.send('showSystemInfo', {})
}

// 初始化
onMounted(() => {
	// 获取浏览器信息
	userAgent.value = navigator.userAgent
	language.value = navigator.language
	timezone.value = Intl.DateTimeFormat().resolvedOptions().timeZone
	screenResolution.value = `${window.screen.width}x${window.screen.height}`
	
	// 获取构建日期（从环境变量或其他来源）
	if (import.meta.env?.VITE_BUILD_DATE) {
		buildDate.value = import.meta.env.VITE_BUILD_DATE
	}
})
</script>

<style scoped>
.about-settings {
	text-align: left;
}
</style>