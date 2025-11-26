<template>
	<div class="word-manager-view container-custom py-6">
		<!-- 页面标题和操作 -->
		<div class="page-header mb-8">
			<div class="flex flex-col md:flex-row md:items-center md:justify-between">
				<div>
					<h1 class="text-3xl font-bold text-gray-900 dark:text-white mb-2">
						单词管理
					</h1>
					<p class="text-gray-600 dark:text-gray-400">
						管理您的单词库，添加、编辑或删除单词
					</p>
				</div>
				<div class="flex space-x-3 mt-4 md:mt-0">
					<button
						@click="loadFile"
						class="btn btn-secondary"
					>
						导入单词库
					</button>
					<button
						@click="addNewWord"
						class="btn btn-primary"
					>
						+ 添加单词
					</button>
				</div>
			</div>
		</div>

		<!-- 统计信息 -->
		<div class="stats-section mb-6">
			<div class="grid grid-cols-1 md:grid-cols-4 gap-4">
				<div class="stat-card card p-4">
					<div class="text-2xl font-bold text-blue-600">{{ words.length }}</div>
					<div class="text-sm text-gray-600">总单词数</div>
				</div>
				<div class="stat-card card p-4">
					<div class="text-2xl font-bold text-green-600">{{ masteredWords.length }}</div>
					<div class="text-sm text-gray-600">已掌握</div>
				</div>
				<div class="stat-card card p-4">
					<div class="text-2xl font-bold text-red-600">{{ wrongWords.length }}</div>
					<div class="text-sm text-gray-600">错词</div>
				</div>
				<div class="stat-card card p-4">
					<div class="text-2xl font-bold text-purple-600">{{ remainingWords.length }}</div>
					<div class="text-sm text-gray-600">待学习</div>
				</div>
			</div>
		</div>

		<!-- 搜索和过滤 -->
		<div class="filters-section mb-6">
			<div class="card p-4">
				<div class="flex flex-col md:flex-row gap-4">
					<!-- 搜索框 -->
					<div class="flex-1">
						<input
							v-model="searchQuery"
							placeholder="搜索单词..."
							class="input w-full"
						/>
					</div>
					<!-- 过滤选项 -->
					<div class="flex space-x-2">
						<select
							v-model="filterType"
							class="input"
						>
							<option value="all">所有单词</option>
							<option value="mastered">已掌握</option>
							<option value="wrong">错词</option>
							<option value="remaining">待学习</option>
						</select>
						<select
							v-model="sortBy"
							class="input"
						>
							<option value="word">按单词排序</option>
							<option value="pos">按词性排序</option>
							<option value="custom">自定义排序</option>
						</select>
					</div>
				</div>
			</div>
		</div>

		<!-- 单词列表 -->
		<div class="words-section">
			<div class="card p-6">
				<!-- 列表头部 -->
				<div class="grid grid-cols-12 gap-4 px-4 py-2 text-sm font-medium text-gray-500 dark:text-gray-400 border-b border-gray-200 dark:border-gray-700">
					<div class="col-span-4">单词</div>
					<div class="col-span-2">词性</div>
					<div class="col-span-4">释义</div>
					<div class="col-span-2 text-center">操作</div>
				</div>

				<!-- 单词项 -->
				<div
					v-for="word in filteredWords"
					:key="word.id || word.word"
					class="word-item grid grid-cols-12 gap-4 px-4 py-3 border-b border-gray-100 dark:border-gray-800 hover:bg-gray-50 dark:hover:bg-gray-800/50 transition-colors"
				>
					<div class="col-span-4 font-medium text-gray-900 dark:text-white">
						{{ word.word }}
					</div>
					<div class="col-span-2">
						<span class="tag tag-pos">{{ word.pos }}</span>
					</div>
					<div class="col-span-4 text-gray-700 dark:text-gray-300">
						{{ word.meaning }}
					</div>
					<div class="col-span-2 flex justify-center space-x-2">
						<button
							@click="editWord(word)"
							class="p-1 text-blue-600 hover:text-blue-800 dark:text-blue-400 dark:hover:text-blue-300"
							title="编辑"
						>
							✏️
						</button>
						<button
							@click="deleteWord(word)"
							class="p-1 text-red-600 hover:text-red-800 dark:text-red-400 dark:hover:text-red-300"
							title="删除"
						>
							🗑️
						</button>
					</div>
				</div>

				<!-- 空状态 -->
				<div
					v-if="filteredWords.length === 0"
					class="empty-state text-center py-12"
				>
					<div class="text-6xl mb-4">📝</div>
					<h3 class="text-xl font-semibold mb-2">没有找到单词</h3>
					<p class="text-gray-600 dark:text-gray-400 mb-6">
						{{ searchQuery ? '尝试调整搜索条件' : '开始添加您的第一个单词' }}
					</p>
					<button
						@click="addNewWord"
						class="btn btn-primary"
					>
						+ 添加单词
					</button>
				</div>
			</div>
		</div>

		<!-- 编辑对话框 -->
		<WordEditDialog
			:show="showEditDialog"
			:word="editingWord"
			@save="saveWord"
			@close="closeEditDialog"
		/>

		<!-- 文件选择输入 -->
		<input
			type="file"
			ref="fileInput"
			@change="onFileSelected"
			accept=".txt,.wdlst"
			class="hidden"
		/>
	</div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useWordStore } from '@stores/word'
import WordEditDialog from './WordEditDialog.vue'

const wordStore = useWordStore()

// 响应式数据
const searchQuery = ref('')
const filterType = ref('all')
const sortBy = ref('word')
const showEditDialog = ref(false)
const editingWord = ref(null)
const fileInput = ref(null)

// 从 store 获取数据
const { allWords, stats, getAllWords } = wordStore

// 计算属性
const words = computed(() => allWords.value || [])
const masteredWords = computed(() => words.value.filter(word => word.mastered))
const wrongWords = computed(() => words.value.filter(word => word.wrong))
const remainingWords = computed(() => words.value.filter(word => !word.mastered))

const filteredWords = computed(() => {
	let filtered = words.value

	// 应用搜索过滤
	if (searchQuery.value) {
		const query = searchQuery.value.toLowerCase()
		filtered = filtered.filter(word => 
			word.word.toLowerCase().includes(query) ||
			word.meaning.toLowerCase().includes(query) ||
			word.pos.toLowerCase().includes(query)
		)
	}

	// 应用类型过滤
	if (filterType.value === 'mastered') {
		filtered = filtered.filter(word => word.mastered)
	} else if (filterType.value === 'wrong') {
		filtered = filtered.filter(word => word.wrong)
	} else if (filterType.value === 'remaining') {
		filtered = filtered.filter(word => !word.mastered)
	}

	// 应用排序
	if (sortBy.value === 'word') {
		filtered.sort((a, b) => a.word.localeCompare(b.word))
	} else if (sortBy.value === 'pos') {
		filtered.sort((a, b) => a.pos.localeCompare(b.pos))
	}

	return filtered
})

// 方法
const addNewWord = () => {
	editingWord.value = null
	showEditDialog.value = true
}

const editWord = (word) => {
	editingWord.value = { ...word }
	showEditDialog.value = true
}

const deleteWord = (word) => {
	if (confirm(`确定要删除单词 "${word.word}" 吗？`)) {
		// 实现删除逻辑
		console.log('删除单词:', word)
	}
}

const saveWord = (wordData) => {
	if (editingWord.value) {
		// 更新现有单词
		console.log('更新单词:', wordData)
	} else {
		// 添加新单词
		console.log('添加新单词:', wordData)
	}
	closeEditDialog()
}

const closeEditDialog = () => {
	showEditDialog.value = false
	editingWord.value = null
}

const loadFile = () => {
	fileInput.value?.click()
}

const onFileSelected = (event) => {
	const file = event.target.files[0]
	if (file) {
		wordStore.loadWordsFile(file.path || file.name)
	}
	event.target.value = ''
}

// 初始化
onMounted(() => {
	getAllWords()
})
</script>

<style scoped>
.word-manager-view {
	min-height: calc(100vh - 80px);
}

.stat-card {
	transition: all 0.3s ease;
}

.stat-card:hover {
	transform: translateY(-2px);
}

.word-item {
	animation: slideUp 0.3s ease-out;
}

@keyframes slideUp {
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