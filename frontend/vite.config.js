import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import { resolve } from 'path'

export default defineConfig({
	plugins: [vue()],
	resolve: {
		alias: {
			'@': resolve(__dirname, 'src'),
			'@components': resolve(__dirname, 'src/components'),
			'@stores': resolve(__dirname, 'src/stores'),
			'@utils': resolve(__dirname, 'src/utils'),
			'@assets': resolve(__dirname, 'src/assets'),
			'@settings': resolve(__dirname, 'settings')
		}
	},
	build: {
		outDir: 'dist',
		assetsDir: 'assets',
		rollupOptions: {
			output: {
				manualChunks: {
					vendor: ['vue', 'pinia', 'vue-router'],
					utils: ['./src/utils/bridge.js', './src/utils/theme.js']
				}
			}
		}
	},
	server: {
		port: 3000,
		open: false
	}
})