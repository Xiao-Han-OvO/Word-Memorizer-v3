import { createRouter, createWebHashHistory } from 'vue-router'

const routes = [
	{
		path: '/',
		name: 'learning',
		component: () => import('@components/LearningView.vue'),
		meta: {
			title: '学习 - VocabMemster'
		}
	},
	{
		path: '/settings',
		name: 'settings',
		component: () => import('@components/SettingsView.vue'),
		meta: {
			title: '设置 - VocabMemster'
		}
	},
	{
		path: '/word-manager',
		name: 'word-manager',
		component: () => import('@components/WordManagerView.vue'),
		meta: {
			title: '单词管理 - VocabMemster'
		}
	},
	{
		path: '/:pathMatch(.*)*',
		redirect: '/'
	}
]

const router = createRouter({
	history: createWebHashHistory(),
	routes
})

// 路由守卫 - 更新页面标题
router.beforeEach((to, from, next) => {
	if (to.meta.title) {
		document.title = to.meta.title
	}
	next()
})

export default router