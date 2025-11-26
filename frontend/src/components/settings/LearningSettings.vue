<template>
	<div class="learning-settings space-y-6">
		<!-- 学习行为设置 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">学习行为</h3>
			
			<div class="space-y-4">
				<!-- 自动前进 -->
				<div class="setting-item flex items-center justify-between">
					<div>
						<label class="block text-sm font-medium mb-1">自动前进</label>
						<p class="text-sm text-gray-500 dark:text-gray-400">
							回答正确后自动加载下一个单词
						</p>
					</div>
					<toggle-switch
						:value="appSettings.autoAdvance"
						@change="updateSetting('autoAdvance', $event)"
					/>
				</div>

				<!-- 自动前进延迟 -->
				<div class="setting-item" v-if="appSettings.autoAdvance">
					<label class="block text-sm font-medium mb-2">自动前进延迟</label>
					<div class="flex items-center space-x-3">
						<input
							type="range"
							:value="appSettings.autoAdvanceDelay"
							@input="updateSetting('autoAdvanceDelay', parseInt($event.target.value))"
							min="500"
							max="5000"
							step="100"
							class="flex-1"
						>
						<span class="text-sm text-gray-600 dark:text-gray-400 w-16">
							{{ appSettings.autoAdvanceDelay }}ms
						</span>
					</div>
				</div>

				<!-- 区分大小写 -->
				<div class="setting-item flex items-center justify-between">
					<div>
						<label class="block text-sm font-medium mb-1">区分大小写</label>
						<p class="text-sm text-gray-500 dark:text-gray-400">
							检查答案时是否区分大小写
						</p>
					</div>
					<toggle-switch
						:value="appSettings.caseSensitive"
						@change="updateSetting('caseSensitive', $event)"
					/>
				</div>

				<!-- 启用提示 -->
				<div class="setting-item flex items-center justify-between">
					<div>
						<label class="block text-sm font-medium mb-1">启用提示</label>
						<p class="text-sm text-gray-500 dark:text-gray-400">
							在学习过程中显示提示信息
						</p>
					</div>
					<toggle-switch
						:value="appSettings.enableHints"
						@change="updateSetting('enableHints', $event)"
					/>
				</div>
			</div>
		</div>

		<!-- 界面显示设置 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">界面显示</h3>
			
			<div class="space-y-4">
				<!-- 显示例句 -->
				<div class="setting-item flex items-center justify-between">
					<div>
						<label class="block text-sm font-medium mb-1">显示例句</label>
						<p class="text-sm text-gray-500 dark:text-gray-400">
							在单词卡片中显示例句
						</p>
					</div>
					<toggle-switch
						:value="appSettings.showExamples"
						@change="updateSetting('showExamples', $event)"
					/>
				</div>

				<!-- 显示派生词 -->
				<div class="setting-item flex items-center justify-between">
					<div>
						<label class="block text-sm font-medium mb-1">显示派生词</label>
						<p class="text-sm text-gray-500 dark:text-gray-400">
							自动检测并显示单词的派生形式
						</p>
					</div>
					<toggle-switch
						:value="appSettings.showDerivatives"
						@change="updateSetting('showDerivatives', $event)"
					/>
				</div>

				<!-- 启用动画 -->
				<div class="setting-item flex items-center justify-between">
					<div>
						<label class="block text-sm font-medium mb-1">启用动画</label>
						<p class="text-sm text-gray-500 dark:text-gray-400">
							在界面交互中启用动画效果
						</p>
					</div>
					<toggle-switch
						:value="appSettings.animationEnabled"
						@change="updateSetting('animationEnabled', $event)"
					/>
				</div>
			</div>
		</div>

		<!-- 复习设置 -->
		<div class="setting-section">
			<h3 class="text-lg font-semibold mb-4">复习设置</h3>
			
			<div class="space-y-4">
				<!-- 错词复习 -->
				<div class="setting-item">
					<label class="block text-sm font-medium mb-2">错词复习频率</label>
					<select
						:value="appSettings.wrongWordReview"
						@change="updateSetting('wrongWordReview', $event.target.value)"
						class="input"
					>
						<option value="immediate">立即复习</option>
						<option value="daily">每天复习</option>
						<option value="weekly">每周复习</option>
						<option value="monthly">每月复习</option>
					</select>
				</div>

				<!-- 间隔重复 -->
				<div class="setting-item flex items-center justify-between">
					<div>
						<label class="block text-sm font-medium mb-1">启用间隔重复</label>
						<p class="text-sm text-gray-500 dark:text-gray-400">
							基于记忆曲线安排复习时间
						</p>
					</div>
					<toggle-switch
						:value="appSettings.spacedRepetition"
						@change="updateSetting('spacedRepetition', $event)"
					/>
				</div>

				<!-- 每日目标 -->
				<div class="setting-item">
					<label class="block text-sm font-medium mb-2">每日学习目标</label>
					<input
						type="number"
						:value="appSettings.dailyGoal"
						@input="updateSetting('dailyGoal', parseInt($event.target.value))"
						min="1"
						max="100"
						class="input"
						placeholder="20"
					>
				</div>
			</div>
		</div>

		<!-- 重置按钮 -->
		<div class="setting-section">
			<button
				@click="resetToDefaults"
				class="btn btn-secondary"
			>
				恢复默认设置
			</button>
		</div>
	</div>
</template>

<script setup>
import { computed } from 'vue'
import { useSettingsStore } from '@stores/settings'
import ToggleSwitch from '../ui/ToggleSwitch.vue'

const settingsStore = useSettingsStore()

// 从 store 获取状态和方法
const { appSettings, updateSetting, resetToDefaults } = settingsStore

// 计算属性 - 确保设置项存在
const safeAppSettings = computed(() => ({
	autoAdvance: appSettings.value.autoAdvance ?? true,
	autoAdvanceDelay: appSettings.value.autoAdvanceDelay ?? 1500,
	caseSensitive: appSettings.value.caseSensitive ?? false,
	enableHints: appSettings.value.enableHints ?? true,
	showExamples: appSettings.value.showExamples ?? true,
	showDerivatives: appSettings.value.showDerivatives ?? true,
	animationEnabled: appSettings.value.animationEnabled ?? true,
	wrongWordReview: appSettings.value.wrongWordReview ?? 'immediate',
	spacedRepetition: appSettings.value.spacedRepetition ?? false,
	dailyGoal: appSettings.value.dailyGoal ?? 20,
	...appSettings.value
}))
</script>

<style scoped>
.setting-item {
	padding: 1rem 0;
	border-bottom: 1px solid var(--border-color);
}

.setting-item:last-child {
	border-bottom: none;
}

input[type="range"] {
	-webkit-appearance: none;
	height: 6px;
	background: var(--border-color);
	border-radius: 3px;
	outline: none;
}

input[type="range"]::-webkit-slider-thumb {
	-webkit-appearance: none;
	width: 18px;
	height: 18px;
	background: var(--color-primary);
	border-radius: 50%;
	cursor: pointer;
}

input[type="range"]::-moz-range-thumb {
	width: 18px;
	height: 18px;
	background: var(--color-primary);
	border-radius: 50%;
	cursor: pointer;
	border: none;
}
</style>