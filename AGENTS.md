# Working on Word Memorizer

## Resource limits

- Always compile with one job: `cmake --build "Word Memorizer/build-local" --parallel 1`.
- Never use `-j` without a limit, `-j2` or higher, or concurrent build commands. The user's computer becomes laggy under parallel compilation.
- Run tests with `--parallel 1` as well. Avoid unnecessary full rebuilds.

## Project layout

- This is a Linux desktop application using C++11, GTKmm 3, Fontconfig and librsvg. It is not a Qt application.
- The CMake source directory is `Word Memorizer`, not the repository root. Quote paths containing spaces.
- `Word Memorizer/src/main.cpp` initializes GTK and starts `MainWindow`.
- `MainWindow` controls the learning pages, answers, wrong-word dialogs and completion timer. `SidePanel` displays session statistics and emits UI actions.
- `WordManager` parses vocabulary files and owns mastery and failure state.
- `SettingsManager` handles preferences; `SettingsDialog` edits them.
- `GlassTheme` loads CSS from `resources` beside or above the executable, with a built-in fallback. `IconHelper` renders SVG icons.
- `Word Memorizer/CMakeLists.txt` lists the compiled sources explicitly. Some older source files are not built; check the source list before changing them.
- `word_memorizer_core` is shared by the application and regression tests so GTK implementations are compiled once per build.

## Build and run

Check the working tree before editing and preserve unrelated existing changes. Use `build-local` instead of an existing `build` directory, which may contain a cache generated at a different path. Do not hand-edit generated build files or commit build artifacts.

From the repository root:

```sh
cmake -S "Word Memorizer" -B "Word Memorizer/build-local" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build "Word Memorizer/build-local" --parallel 1
ctest --test-dir "Word Memorizer/build-local" --output-on-failure --parallel 1
./"Word Memorizer/build-local/bin/word_memorizer"
```

The application needs a graphical desktop. GUI regressions use `xvfb-run` and a virtual display; they are included when Xvfb is available. See `README.md` for dependency installation and a build without tests.

## Behavior to preserve

- Initialize `Gtk::Application` before creating GTK objects, including CSS providers. Apply the saved theme when creating the window.
- Construct managers before widgets that reference them, and ensure those managers outlive the widgets.
- Vocabulary files are UTF-8, with `word|part of speech|meaning|example` per line. The example is optional. Accept UTF-8 BOM and CRLF, trim surrounding field whitespace, and skip rows with empty required fields.
- Load a new library only after parsing succeeds. An unreadable file or a file with no valid entries must preserve the existing library, progress and wrong words.
- Mastery is tracked by spelling. Multiple entries with that spelling all count as mastered; total, remaining and progress must agree, including at completion.
- Ignore empty, unknown or inactive answers. Clearing wrong words must also clear their failure markers.
- Cancel or Reset followed by Cancel in settings must not change active preferences or the settings file. Persist draft preferences only on Save. Reading defaults must not create a file.
- Cancel pending completion callbacks when changing the session or destroying the window. A previous session's callback must not replace the current learning page.
- Keep the existing Chinese UI wording and style consistent when changing user-facing text.

## Verification

- For changes to learning or settings behavior, run the relevant regression tests in `Word Memorizer/tests`. They exercise failed imports, repeated spellings, encoding and line endings, preference cancellation, answer guards and session transitions.
- Tests must use temporary vocabulary and preference files. `SettingsManager(filename)` and `MainWindow(settingsFile)` support isolation; never alter the user's `~/.word_memorizer` for a test.
- `G_DEBUG=fatal-criticals` is enabled for GUI regressions to expose GTK initialization and widget errors.
- Run `git diff --check` before finishing. Review changes against the starting working tree so existing user work remains intact.
- Documentation-only changes do not need compilation. Report which checks actually ran and any remaining limitations.
