<template>
	<div class="progress-container">
		<!-- 进度信息 -->
		<div class="progress-info flex justify-between items-center mb-2">
			<span class="text-sm font-medium text-gray-700 dark:text-gray-300">
				学习进度
			</span>
			<span class="text-sm font-semibold text-blue-600 dark:text-blue-400">
				{{ Math.round(progress * 100) }}%
			</span>
		</div>

		<!-- 进度条 -->
		<div class="progress-bar w-full bg-gray-200 dark:bg-gray-700 rounded-full h-3 overflow-hidden">
			<div 
				class="progress-fill h-3 rounded-full transition-all duration-500 ease-out"
				:style="{
					width: `${progress * 100}%`,
					backgroundColor: progressColor
				}"
			></div>
		</div>

		<!-- 详细统计 -->
		<div class="progress-details flex justify-between items-center mt-2">
			<span class="text-xs text-gray-500 dark:text-gray-400">
				已掌握: {{ masteredCount }} / {{ totalCount }}
			</span>
			<span class="text-xs text-gray-500 dark:text-gray-400">
				剩余: {{ remainingCount }}
			</span>
		</div>

		<!-- 进度阶段指示器 -->
		<div class="progress-stages mt-4" v-if="showStages">
			<div class="flex justify-between text-xs text-gray-500 dark:text-gray-400">
				<span 
					v-for="stage in stages" 
					:key="stage.label"
					:class="[
						'stage-marker',
						stage.completed ? 'text-green-500 font-medium' : '',
						stage.current ? 'text-blue-500 font-bold' : ''
					]"
				>
					{{ stage.label }}
				</span>
			</div>
			<div class="flex justify-between mt-1">
				<div 
					v-for="stage in stages" 
					:key="stage.label"
					class="stage-bar h-1 rounded-full flex-1 mx-1"
					:class="[
						stage.completed ? 'bg-green-500' : 
						stage.current ? 'bg-blue-500' : 'bg-gray-300 dark:bg-gray-600'
					]"
				></div>
			</div>
		</div>
	</div>
</template>

<script setup>
import { computed } from 'vue'
import { useThemeStore } from '@stores/theme'

const themeStore = useThemeStore()

const props = defineProps({
	progress: {
		type: Number,
		default: 0,
		validator: (value) => value >= 0 && value <= 1
	},
	masteredCount: {
		type: Number,
		default: 0
	},
	totalCount: {
		type: Number,
		default: 0
	},
	showStages: {
		type: Boolean,
		default: true
	}
})

// 计算属性
const remainingCount = computed(() => {
	return Math.max(0, props.totalCount - props.masteredCount)
})

const progressColor = computed(() => {
	const progress = props.progress
	if (progress < 0.3) return '#ef4444' // red
	if (progress < 0.7) return '#f59e0b' // amber
	if (progress < 0.9) return '#10b981' // green
	return '#8b5cf6' // violet
})

const stages = computed(() => {
	const progress = props.progress
	return [
		{
			label: '开始',
			completed: progress > 0,
			current: progress <= 0.25
		},
		{
			label: '25%',
			completed: progress > 0.25,
			current: progress > 0.25 && progress <= 0.5
		},
		{
			label: '50%',
			completed: progress > 0.5,
			current: progress > 0.5 && progress <= 0.75
		},
		{
			label: '75%',
			completed: progress > 0.75,
			current: progress > 0.75 && progress < 1
		},
		{
			label: '完成',
			completed: progress >= 1,
			current: progress >= 1
		}
	]
})
</script>

<style scoped>
.progress-container {
	width: 100%;
}

.progress-fill {
	background: linear-gradient(90deg, 
		#ef4444, 
		#f59e0b 25%, 
		#10b981 50%, 
		#3b82f6 75%, 
		#8b5cf6
	);
	background-size: 200% 100%;
	animation: shimmer 2s infinite;
}

@keyframes shimmer {
	0% {
		background-position: -200% 0;
	}
	100% {
		background-position: 200% 0;
	}
}

.stage-bar {
	transition: all 0.3s ease;
}

.stage-bar:hover {
	transform: scaleY(1.5);
}
</style>