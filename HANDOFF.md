# CLI handoff

The user requested fixes for incomplete dark mode and unattractive title bars, then asked to stop mid-task for migration.

- Follow `AGENTS.md`. This is GTKmm 3 / C++11 / CMake. Always build and test with `--parallel 1`; preserve the existing, extensively modified working tree.
- Earlier functional fixes are complete: startup and saved themes, safe imports, duplicate-word progress, BOM/CRLF parsing, settings Cancel, and answer/completion guards. All three original CTest checks passed before this styling task.
- Styling groundwork is written but **has not been built or tested**: `resources/style.css` now covers native controls and header bars; `style-dark.css` provides a shared dark palette. `GlassTheme.cpp/.h` now set GTK's dark preference, calculate readable colors, update window classes, and expose `configure_window()` for native header bars. CMake generates the built-in CSS from these same files using the new `src/ThemeResources.h.in`.
- Still unfinished: connect the header-bar helper to MainWindow and custom dialogs, remove manual dragging/title-bar rows, make existing SVG icons recolor on theme changes, and fix hardcoded help/status colors. The icon helper and window/dialog implementations are still unchanged from the start of this styling task.
- Added `tests/StyleRegressionTests.cpp` and its CMake target. Against the original implementation, five style checks failed and the visual-capture phase timed out at 30 seconds. Investigate that timeout. When native header icons are introduced, guard `get_pixbuf()` calls with `get_storage_type() == Gtk::IMAGE_PIXBUF` to avoid querying symbolic images incorrectly.
- The compiled executables currently predate the new theme edits. Xvfb tests needed an unsandboxed CTest run because the sandbox could not access its display.
- Starting-point backup for this styling task: `/tmp/word-memorizer-style-2rvb9qz6/original`. Compare against it to distinguish styling changes from previous work.

Resume with:

```sh
cmake -S "Word Memorizer" -B "Word Memorizer/build-local" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build "Word Memorizer/build-local" --parallel 1
ctest --test-dir "Word Memorizer/build-local" --output-on-failure --parallel 1
```

Review the rendered light/dark screens and run `git diff --check` before considering the styling work complete.
