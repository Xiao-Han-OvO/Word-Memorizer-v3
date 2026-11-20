# Word Memorizer

这是一个使用 C++11 和 GTKmm 3 的 Linux 桌面单词记忆程序。通过左侧「打开」按钮导入词库，输入英文单词并按回车或「提交」。右侧面板显示学习进度、错词本和主题设置。

## 从源码构建和运行

需要 C++ 编译器、CMake、pkg-config、GTKmm 3、Fontconfig 和 librsvg。Ubuntu / Debian 可安装：

```sh
sudo apt install build-essential cmake pkg-config libgtkmm-3.0-dev libfontconfig1-dev librsvg2-dev
```

在项目根目录执行以下命令。`--parallel 1` 限制为单个编译任务，减少电脑负载；`build-local` 避免复用其他路径下生成的旧构建缓存。

```sh
cmake -S "Word Memorizer" -B "Word Memorizer/build-local" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build "Word Memorizer/build-local" --parallel 1
./"Word Memorizer/build-local/bin/word_memorizer"
```

运行需要图形桌面环境。主题文件在构建时复制到 `build-local/resources`，程序也提供内置主题。设置保存在 `~/.word_memorizer`，学习进度和错词本保留在当前会话中，可从错词本窗口导出。

## 词库格式

使用 UTF-8 文本，每行一个条目：

```text
单词|词性|释义|例句
apple|n.|苹果|I eat an apple every day.
run|v.|跑步|He runs in the park.
```

例句可省略。支持 UTF-8 BOM 和 Windows 换行；忽略空行，以及缺少单词、词性或释义的行。没有有效条目的文件会被拒绝，当前学习会话保持不变。同一拼写的多个词性或释义在答对该拼写后一起计入已掌握。

快捷键：回车提交、`Ctrl+Tab` 切换单词、`Alt+S` 查看答案、`Ctrl+Shift+D` 切换明暗主题。

词汇表：[Word-List-For-Word-Memorizer](https://github.com/Xiao-Han-OvO/Word-List-For-Word-Memorizer/tree/main)。

## 回归检查

GUI 检查使用 Xvfb，在虚拟显示中运行，不会在桌面上打开测试窗口。Ubuntu / Debian 可通过 `sudo apt install xvfb xauth` 安装；未找到 `xvfb-run` 时仅构建词库检查。

```sh
cmake -S "Word Memorizer" -B "Word Memorizer/build-local" -DBUILD_TESTING=ON
cmake --build "Word Memorizer/build-local" --parallel 1
ctest --test-dir "Word Memorizer/build-local" --output-on-failure --parallel 1
```

检查覆盖词库导入失败、重复拼写、文件编码和换行、设置保存与取消、空答案，以及完成提示在重新开始或更换词库后的行为。测试使用临时设置文件，不修改个人设置。
