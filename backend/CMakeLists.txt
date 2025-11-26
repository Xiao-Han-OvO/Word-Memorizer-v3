cmake_minimum_required(VERSION 3.16)
project(VocabMemster)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖包
find_package(PkgConfig REQUIRED)

# GTKMM 3.0
pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)

# WebKit2GTK 4.1
pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.1)

# 包含目录
include_directories(src)
include_directories(${GTKMM3_INCLUDE_DIRS})
include_directories(${WEBKIT2GTK_INCLUDE_DIRS})

# 添加编译器标志
add_compile_options(${GTKMM3_CFLAGS_OTHER})
add_compile_options(${WEBKIT2GTK_CFLAGS_OTHER})

# 源文件
set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/core/WordManager.cpp
    src/core/SettingsManager.cpp
    src/core/WDLSTParser.cpp
    src/bridge/WebViewBridge.cpp
    src/bridge/MessageHandler.cpp
)

# 创建可执行文件
add_executable(vocabmemster ${SOURCES})

# 链接库
target_link_libraries(vocabmemster ${GTKMM3_LIBRARIES} ${WEBKIT2GTK_LIBRARIES})

# 复制前端资源（构建后步骤）
add_custom_command(TARGET vocabmemster POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/resources/web-ui
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/../frontend/dist
        ${CMAKE_BINARY_DIR}/resources/web-ui
    COMMENT "复制前端资源"
)

