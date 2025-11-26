export const AppConfig = {
  appName: "Word Memorizer",
  version: "2.1.1",
  apiBaseUrl: "http://localhost:8080",
  theme: {
    default: "light",
    available: ["light", "dark", "blue"]
  },
  // 添加缺失的属性
  defaultFileFormat: "json",
  supportedFileFormats: ["json", "csv", "txt"]
}

// 保持默认导出以兼容可能使用默认导入的文件
export default AppConfig
