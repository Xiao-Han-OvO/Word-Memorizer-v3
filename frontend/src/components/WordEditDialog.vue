<template>
	<div 
		v-if="show"
		class="dialog-overlay fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50"
		@click="$emit('close')"
	>
		<div 
			class="dialog-content card p-6 max-w-md w-full mx-4 max-h-[80vh] overflow-y-auto"
			@click.stop
		>
			<h3 class="text-xl font-bold mb-4">
				{{ editingWord ? '编辑单词' : '添加新单词' }}
			</h3>

			<form @submit.prevent="saveWord" class="space-y-4">
				<!-- 单词输入 -->
				<div>
					<label class="block text-sm font-medium mb-2">英文单词 *</label>
					<input
						v-model="form.word"
						type="text"
						required
						class="input"
						placeholder="输入英文单词"
					>
				</div>

				<!-- 词性输入 -->
				<div>
					<label class="block text-sm font-medium mb-2">词性 *</label>
					<select
						v-model="form.pos"
						required
						class="input"
					>
						<option value="">选择词性</option>
						<option value="n.">名词 (n.)</option>
						<option value="v.">动词 (v.)</option>
						<option value="adj.">形容词 (adj.)</option>
						<option value="adv.">副词 (adv.)</option>
						<option value="prep.">介词 (prep.)</option>
						<option value="conj.">连词 (conj.)</option>
						<option value="pron.">代词 (pron.)</option>
						<option value="interj.">感叹词 (interj.)</option>
					</select>
				</div>

				<!-- 释义输入 -->
				<div>
					<label class="block text-sm font-medium mb-2">中文释义 *</label>
					<input
						v-model="form.meaning"
						type="text"
						required
						class="input"
						placeholder="输入中文释义"
					>
				</div>

				<!-- 例句输入 -->
				<div>
					<label class="block text-sm font-medium mb-2">例句</label>
					<textarea
						v-model="form.example"
						class="input resize-none"
						rows="3"
						placeholder="输入包含该单词的例句（可选）"
					></textarea>
				</div>

				<!-- 派生词区域 -->
				<div>
					<label class="block text-sm font-medium mb-2">派生词</label>
					<div
						v-for="(derivative, index) in form.derivatives"
						:key="index"
						class="derivative-item flex items-center space-x-2 mb-2"
					>
						<input
							v-model="derivative.word"
							type="text"
							class="input flex-1"
							placeholder="派生词"
						>
						<select
							v-model="derivative.pos"
							class="input w-24"
						>
							<option value="n.">n.</option>
							<option value="v.">v.</option>
							<option value="adj.">adj.</option>
							<option value="adv.">adv.</option>
						</select>
						<button
							type="button"
							@click="removeDerivative(index)"
							class="p-2 text-red-600 hover:text-red-800"
						>
							🗑️
						</button>
					</div>
					<button
						type="button"
						@click="addDerivative"
						class="btn btn-secondary text-sm"
					>
						+ 添加派生词
					</button>
				</div>

				<!-- 操作按钮 -->
				<div class="flex justify-end space-x-3 pt-4">
					<button
						type="button"
						@click="$emit('close')"
						class="btn btn-secondary"
					>
						取消
					</button>
					<button
						type="submit"
						class="btn btn-primary"
						:disabled="!isFormValid"
					>
						{{ editingWord ? '更新' : '添加' }}
					</button>
				</div>
			</form>
		</div>
	</div>
</template>

<script setup>
import { ref, watch, computed } from 'vue'

const props = defineProps({
	show: {
		type: Boolean,
		default: false
	},
	word: {
		type: Object,
		default: null
	}
})

const emit = defineEmits(['save', 'close'])

// 表单数据
const form = ref({
	word: '',
	pos: '',
	meaning: '',
	example: '',
	derivatives: []
})

// 计算属性
const isFormValid = computed(() => {
	return form.value.word.trim() && form.value.pos && form.value.meaning.trim()
})

// 方法
const resetForm = () => {
	form.value = {
		word: '',
		pos: '',
		meaning: '',
		example: '',
		derivatives: []
	}
}

const addDerivative = () => {
	form.value.derivatives.push({
		word: '',
		pos: 'n.',
		meaning: ''
	})
}

const removeDerivative = (index) => {
	form.value.derivatives.splice(index, 1)
}

const saveWord = () => {
	if (!isFormValid.value) return
	
	// 过滤空的派生词
	const derivatives = form.value.derivatives.filter(d => d.word.trim())
	
	emit('save', {
		...form.value,
		derivatives
	})
	resetForm()
}

// 监听编辑单词的变化
watch(() => props.word, (newWord) => {
	if (newWord) {
		form.value = {
			word: newWord.word || '',
			pos: newWord.pos || '',
			meaning: newWord.meaning || '',
			example: newWord.example || '',
			derivatives: newWord.derivatives ? [...newWord.derivatives] : []
		}
	} else {
		resetForm()
	}
}, { immediate: true })
</script>

<style scoped>
.dialog-overlay {
	animation: fadeIn 0.2s ease-out;
}

.dialog-content {
	animation: slideUp 0.3s ease-out;
}

.derivative-item {
	animation: slideUp 0.2s ease-out;
}

@keyframes fadeIn {
	from {
		opacity: 0;
	}
	to {
		opacity: 1;
	}
}

@keyframes slideUp {
	from {
		opacity: 0;
		transform: translateY(20px) scale(0.95);
	}
	to {
		opacity: 1;
		transform: translateY(0) scale(1);
	}
}
</style>