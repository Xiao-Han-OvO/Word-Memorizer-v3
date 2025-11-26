import { createApp } from 'vue'
import { createPinia } from 'pinia'
import { createRouter, createWebHistory } from 'vue-router'
import App from './App.vue'
import './assets/css/main.css'

// 创建 Pinia 实例
const pinia = createPinia()

// 创建路由
const router = createRouter({
	history: createWebHistory(),
	routes: [
		{
			path: '/',
			name: 'learning',
			component: () => import('@components/LearningView.vue')
		},
		{
			path: '/settings',
			name: 'settings',
			component: () => import('@components/SettingsView.vue')
		},
		{
			path: '/word-manager',
			name: 'word-manager',
			component: () => import('@components/WordManagerView.vue')
		}
	]
})

// 创建 Vue 应用
const app = createApp(App)

// 使用插件
app.use(pinia)
app.use(router)

// 挂载应用
app.mount('#app')

// 隐藏加载界面
document.addEventListener('DOMContentLoaded', () => {
	setTimeout(() => {
		const loading = document.getElementById('loading')
		if (loading) {
			loading.style.opacity = '0'
			setTimeout(() => loading.remove(), 300)
		}
	}, 500)
})