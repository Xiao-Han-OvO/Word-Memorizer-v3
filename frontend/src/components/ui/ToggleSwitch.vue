<template>
	<button
		:class="[
			'toggle-switch relative inline-flex h-6 w-11 items-center rounded-full transition-colors focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2',
			modelValue 
				? 'bg-blue-600' 
				: 'bg-gray-200 dark:bg-gray-700'
		]"
		@click="$emit('update:modelValue', !modelValue)"
		role="switch"
		:aria-checked="modelValue"
	>
		<span
			:class="[
				'inline-block h-4 w-4 transform rounded-full bg-white transition-transform',
				modelValue ? 'translate-x-6' : 'translate-x-1'
			]"
		/>
	</button>
</template>

<script setup>
defineProps({
	modelValue: {
		type: Boolean,
		default: false
	}
})

defineEmits(['update:modelValue'])
</script>

<style scoped>
.toggle-switch {
	transition: all 0.2s ease;
}

.toggle-switch:hover {
	transform: scale(1.05);
}
</style>