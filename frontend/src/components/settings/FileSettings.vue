<template>
	<div class="file-settings space-y-6">
		<!-- 文件格式设置 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">文件格式</h3>
			
			<div class="space-y-4">
				<!-- 默认文件格式 -->
				<div class="setting-item">
					<label class="block text-sm font-medium mb-2">默认保存格式</label>
					<select
						:value="appSettings.defaultFileFormat"
						@change="updateSetting('defaultFileFormat', $event.target.value)"
						class="input"
					>
						<option value=".txt">文本文件 (.txt)</option>
						<option value=".wdlst">WDLST 文件 (.wdlst)</option>
					</select>
					<p class="text-sm text-gray-500 dark:text-gray-400 mt-1">
						{{ formatDescription }}
					</p>
				</div>

				<!-- 记住上次文件 -->
				<div class="setting-item flex items-center justify-between">
					<div>
						<label class="block text-sm font-medium mb-1">记住上次文件</label>
						<p class="text-sm text-gray-500 dark:text-gray-400">
							启动时自动加载上次使用的单词库
						</p>
					</div>
					<toggle-switch
						:value="appSettings.rememberLastFile"
						@change="updateSetting('rememberLastFile', $event)"
					/>
				</div>

				<!-- 自动备份 -->
				<div class="setting-item flex items-center justify-between">
					<div>
						<label class="block text-sm font-medium mb-1">自动备份</label>
						<p class="text-sm text-gray-500 dark:text-gray-400">
							定期自动备份学习进度
						</p>
					</div>
					<toggle-switch
						:value="appSettings.autoBackup"
						@change="updateSetting('autoBackup', $event)"
					/>
				</div>
			</div>
		</div>

		<!-- 导入/导出设置 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">导入/导出</h3>
			
			<div class="space-y-4">
				<!-- 导出设置 -->
				<div class="setting-item">
					<label class="block text-sm font-medium mb-2">导出包含内容</label>
					<div class="space-y-2">
						<label class="flex items-center">
							<input
								type="checkbox"
								:checked="appSettings.exportIncludeMastered"
								@change="updateSetting('exportIncludeMastered', $event.target.checked)"
								class="rounded border-gray-300 text-blue-600 focus:ring-blue-500"
							>
							<span class="ml-2 text-sm">已掌握单词</span>
						</label>
						<label class="flex items-center">
							<input
								type="checkbox"
								:checked="appSettings.exportIncludeWrong"
								@change="updateSetting('exportIncludeWrong', $event.target.checked)"
								class="rounded border-gray-300 text-blue-600 focus:ring-blue-500"
							>
							<span class="ml-2 text-sm">错词本</span>
						</label>
						<label class="flex items-center">
							<input
								type="checkbox"
								:checked="appSettings.exportIncludeProgress"
								@change="updateSetting('exportIncludeProgress', $event.target.checked)"
								class="rounded border-gray-300 text-blue-600 focus:ring-blue-500"
							>
							<span class="ml-2 text-sm">学习进度</span>
						</label>
					</div>
				</div>

				<!-- 导入设置 -->
				<div class="setting-item">
					<label class="block text-sm font-medium mb-2">导入行为</label>
					<select
						:value="appSettings.importBehavior"
						@change="updateSetting('importBehavior', $event.target.value)"
						class="input"
					>
						<option value="merge">合并到现有单词库</option>
						<option value="replace">替换现有单词库</option>
						<option value="ask">每次询问</option>
					</select>
				</div>
			</div>
		</div>

		<!-- 文件操作 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">文件操作</h3>
			
			<div class="space-y-3">
				<!-- 导出单词库 -->
				<button
					@click="exportWordLibrary"
					class="btn btn-secondary w-full justify-center"
				>
					📤 导出单词库
				</button>

				<!-- 导出错词本 -->
				<button
					@click="exportWrongWords"
					class="btn btn-secondary w-full justify-center"
				>
					📋 导出错词本
				</button>

				<!-- 备份学习进度 -->
				<button
					@click="backupProgress"
					class="btn btn-secondary w-full justify-center"
				>
					💾 备份学习进度
				</button>

				<!-- 恢复备份 -->
				<button
					@click="restoreBackup"
					class="btn btn-secondary w-full justify-center"
				>
					🔄 恢复备份
				</button>
			</div>
		</div>

		<!-- 文件信息 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">文件信息</h3>
			
			<div class="card p-4 space-y-3">
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">当前单词库:</span>
					<span class="text-sm font-medium">{{ currentFile || '未加载' }}</span>
				</div>
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">文件大小:</span>
					<span class="text-sm font-medium">{{ fileSize }}</span>
				</div>
				<div class="flex justify-between">
					<span class="text-sm text-gray-600 dark:text-gray-400">最后修改:</span>
					<span class="text-sm font-medium">{{ lastModified }}</span>
				</div>
			</div>
		</div>
	</div>
</template>

<script setup>
import { computed } from 'vue'
import { useSettingsStore } from '@stores/settings'
import { useWordStore } from '@stores/word'
import { bridge } from '@utils/bridge'

const settingsStore = useSettingsStore()
const wordStore = useWordStore()

// 从 store 获取状态和方法
const { appSettings, updateSetting } = settingsStore

// 计算属性
const safeAppSettings = computed(() => ({
	defaultFileFormat: appSettings.value.defaultFileFormat ?? '.wdlst',
	rememberLastFile: appSettings.value.rememberLastFile ?? true,
	autoBackup: appSettings.value.autoBackup ?? false,
	exportIncludeMastered: appSettings.value.exportIncludeMastered ?? true,
	exportIncludeWrong: appSettings.value.exportIncludeWrong ?? true,
	exportIncludeProgress: appSettings.value.exportIncludeProgress ?? true,
	importBehavior: appSettings.value.importBehavior ?? 'merge',
	...appSettings.value
}))

const formatDescription = computed(() => {
	return safeAppSettings.value.defaultFileFormat === '.wdlst' 
		? 'WDLST 格式支持派生词和高级功能'
		: '文本格式简单通用，易于编辑'
})

const currentFile = computed(() => {
	// 实现获取当前文件名的逻辑
	return 'my_words.wdlst'
})

const fileSize = computed(() => {
	// 实现获取文件大小的逻辑
	return '2.1 KB'
})

const lastModified = computed(() => {
	// 实现获取最后修改时间的逻辑
	return '2024-01-15 14:30'
})

// 方法
const exportWordLibrary = () => {
	bridge.send('exportWordLibrary', {
		includeMastered: safeAppSettings.value.exportIncludeMastered,
		includeWrong: safeAppSettings.value.exportIncludeWrong,
		includeProgress: safeAppSettings.value.exportIncludeProgress,
		format: safeAppSettings.value.defaultFileFormat
	})
}

const exportWrongWords = () => {
	bridge.send('exportWrongWords', {})
}

const backupProgress = () => {
	bridge.send('backupProgress', {})
}

const restoreBackup = () => {
	bridge.send('restoreBackup', {})
}
</script>

<style scoped>
.file-settings .btn {
	display: flex;
	align-items: center;
}
</style>