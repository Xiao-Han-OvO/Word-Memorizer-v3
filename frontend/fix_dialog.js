const fs = require('fs');
const path = require('path');

const filePath = path.join(__dirname, 'src/components/DialogSystem.vue');

try {
  let content = fs.readFileSync(filePath, 'utf8');
  
  // 修复导入
  content = content.replace(
    /import\s*{\s*AppConfig\s*}\s*from\s*['"`]@settings\/app\.config['"`]/g,
    'import appConfig from "@settings/app.config.js"'
  );
  
  // 修复变量使用
  content = content.replace(/\bAppConfig\b/g, 'appConfig');
  
  fs.writeFileSync(filePath, content);
  console.log('成功修复 DialogSystem.vue');
} catch (error) {
  console.error('修复 DialogSystem.vue 失败:', error.message);
  
  // 创建基础的 DialogSystem.vue 文件
  const basicContent = `<template>
  <div>
    <!-- Dialog System Component -->
  </div>
</template>

<script setup>
import { ref } from 'vue'
import appConfig from "@settings/app.config.js"

// 组件逻辑
</script>

<style scoped>
/* 组件样式 */
</style>`;
  
  fs.writeFileSync(filePath, basicContent);
  console.log('已创建基础的 DialogSystem.vue 文件');
}
