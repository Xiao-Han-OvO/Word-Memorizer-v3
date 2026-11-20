cmake_minimum_required(VERSION 3.10)
project(WordMemorizer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 同一份 CSS 用于外部资源和内置主题，并让修改 CSS 自动触发更新。
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/resources" "${CMAKE_CURRENT_BINARY_DIR}/generated")
configure_file(resources/style.css "${CMAKE_CURRENT_BINARY_DIR}/resources/style.css" COPYONLY)
configure_file(resources/style-dark.css "${CMAKE_CURRENT_BINARY_DIR}/resources/style-dark.css" COPYONLY)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/resources/style.css" WORD_MEMORIZER_BASE_CSS)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/resources/style-dark.css" WORD_MEMORIZER_DARK_CSS)
configure_file(src/ThemeResources.h.in "${CMAKE_CURRENT_BINARY_DIR}/generated/ThemeResources.h" @ONLY)

# 查找GTKmm包
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})

# 设置编译器标志
add_definitions(${GTKMM_CFLAGS_OTHER})

# 源文件列表
set(SOURCES
    src/SidePanel.cpp
    src/IconHelper.cpp
    src/GlassTheme.cpp
    src/MainWindow.cpp
    src/WordManager.cpp
    src/SettingsManager.cpp
    src/SettingsDialog.cpp
    src/AboutDialog.cpp
    src/WordManagerDialog.cpp
    src/UsageDialog.cpp
)

# 头文件列表
set(HEADERS
    src/SidePanel.h
    src/IconHelper.h
    src/GlassTheme.h
    src/MainWindow.h
    src/WordManager.h
    src/SettingsManager.h
    src/SettingsDialog.h
    src/AboutDialog.h
    src/UsageDialog.h
)

# 应用和回归测试共享编译后的实现，避免重复编译 GTK 源文件。
add_library(word_memorizer_core STATIC ${SOURCES} ${HEADERS})
target_include_directories(word_memorizer_core PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")

# 链接库
target_link_libraries(word_memorizer_core PUBLIC ${GTKMM_LIBRARIES} ${FONTCONFIG_LIBRARIES} -lrsvg-2 -lgio-2.0 -lgdk_pixbuf-2.0 -lgobject-2.0 -lglib-2.0 -lcairo)

add_executable(word_memorizer src/main.cpp)
target_link_libraries(word_memorizer PRIVATE word_memorizer_core)

# 设置可执行文件的属性
set_target_properties(word_memorizer PROPERTIES
    OUTPUT_NAME "word_memorizer"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
# 在构建后将 resources 目录复制到 build 目录
add_custom_command(TARGET word_memorizer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "${CMAKE_BINARY_DIR}/resources")

include(CTest)
if(BUILD_TESTING)
    add_executable(word_manager_tests tests/WordManagerTests.cpp)
    target_link_libraries(word_manager_tests PRIVATE word_memorizer_core)
    add_test(NAME word_manager_regressions COMMAND word_manager_tests)

    find_program(XVFB_RUN_EXECUTABLE xvfb-run)
    if(XVFB_RUN_EXECUTABLE)
        add_executable(gui_regression_tests tests/GuiRegressionTests.cpp)
        target_link_libraries(gui_regression_tests PRIVATE word_memorizer_core)
        add_test(NAME gui_regressions
            COMMAND "${XVFB_RUN_EXECUTABLE}" -a "$<TARGET_FILE:gui_regression_tests>")
        add_test(NAME application_startup
            COMMAND "${XVFB_RUN_EXECUTABLE}" -a "$<TARGET_FILE:word_memorizer>" --help)
        add_executable(style_regression_tests tests/StyleRegressionTests.cpp)
        target_link_libraries(style_regression_tests PRIVATE word_memorizer_core)
        add_test(NAME style_regressions
            COMMAND "${XVFB_RUN_EXECUTABLE}" -a "$<TARGET_FILE:style_regression_tests>")
        set_tests_properties(gui_regressions application_startup style_regressions PROPERTIES
            ENVIRONMENT "G_DEBUG=fatal-criticals"
            TIMEOUT 30)
    endif()
endif()
