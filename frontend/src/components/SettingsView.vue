<template>
	<div class="settings-view container-custom py-6">
		<div class="max-w-4xl mx-auto">
			<!-- 页面标题 -->
			<div class="page-header mb-8">
				<h1 class="text-3xl font-bold text-gray-900 dark:text-white mb-2">
					设置
				</h1>
				<p class="text-gray-600 dark:text-gray-400">
					自定义您的学习体验和界面外观
				</p>
			</div>

			<div class="grid grid-cols-1 lg:grid-cols-3 gap-8">
				<!-- 侧边栏导航 -->
				<div class="lg:col-span-1">
					<nav class="settings-nav card p-4 sticky top-6">
						<ul class="space-y-2">
							<li
								v-for="tab in tabs"
								:key="tab.id"
							>
								<button
									@click="activeTab = tab.id"
									:class="[
										'w-full text-left px-4 py-3 rounded-lg transition-colors',
										activeTab === tab.id
											? 'bg-blue-500 text-white shadow-md'
											: 'text-gray-700 dark:text-gray-300 hover:bg-gray-100 dark:hover:bg-gray-700'
									]"
								>
									<div class="flex items-center space-x-3">
										<span class="text-lg">{{ tab.icon }}</span>
										<span class="font-medium">{{ tab.name }}</span>
									</div>
								</button>
							</li>
						</ul>
					</nav>
				</div>

				<!-- 设置内容 -->
				<div class="lg:col-span-2">
					<div class="settings-content">
						<!-- 外观设置 -->
						<div v-if="activeTab === 'appearance'" class="tab-content">
							<AppearanceSettings />
						</div>

						<!-- 学习设置 -->
						<div v-if="activeTab === 'learning'" class="tab-content">
							<LearningSettings />
						</div>

						<!-- 文件设置 -->
						<div v-if="activeTab === 'files'" class="tab-content">
							<FileSettings />
						</div>

						<!-- 快捷键设置 -->
						<div v-if="activeTab === 'shortcuts'" class="tab-content">
							<ShortcutSettings />
						</div>

						<!-- 关于 -->
						<div v-if="activeTab === 'about'" class="tab-content">
							<AboutSettings />
						</div>
					</div>
				</div>
			</div>
		</div>
	</div>
</template>

<script setup>
import { ref } from 'vue'
import AppearanceSettings from './settings/AppearanceSettings.vue'
import LearningSettings from './settings/LearningSettings.vue'
import FileSettings from './settings/FileSettings.vue'
import ShortcutSettings from './settings/ShortcutSettings.vue'
import AboutSettings from './settings/AboutSettings.vue'

const activeTab = ref('appearance')

const tabs = [
	{ id: 'appearance', name: '外观', icon: '🎨' },
	{ id: 'learning', name: '学习', icon: '📚' },
	{ id: 'files', name: '文件', icon: '📁' },
	{ id: 'shortcuts', name: '快捷键', icon: '⌨️' },
	{ id: 'about', name: '关于', icon: 'ℹ️' }
]
</script>

<style scoped>
.settings-view {
	min-height: calc(100vh - 80px);
}

.settings-nav {
	backdrop-filter: blur(10px);
}

.tab-content {
	animation: fadeIn 0.3s ease-out;
}

@keyframes fadeIn {
	from {
		opacity: 0;
		transform: translateY(10px);
	}
	to {
		opacity: 1;
		transform: translateY(0);
	}
}
</style>