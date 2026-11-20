# CLI handoff

The latest UI corrections are implemented and verified. Follow `AGENTS.md` for further work.

## Latest UI corrections (2026-11-20)

- After checkpoint `2c05fc4`, the user reported dark foregrounds on main-window icons and text, awkward title-bar rounding/alignment, and requested SF Pro Display from the installed fonts. The correction checkpoint uses the commit message `Fix dark foregrounds and bundle SF Pro Display`.
- Dark action buttons now use light text and SVG strokes. Their backgrounds are darkened while preserving the selected hue; normal, hover and active states retain readable contrast. Accent text and saved custom learning colors continue to use their own readable foreground colors.
- SVG icons refresh their foreground at draw time as well as on style/theme/state changes. The default color argument is now empty rather than a hardcoded dark stroke. Regression checks compare each RGB channel for all visible main-window SVGs, including disabled buttons and both theme-switch directions.
- Native GTK header bars use a single-line centered title label and SF Pro Display at weight 600. Text/icon shadows are removed. Inner title-bar rounding matches the outer border; solid windows without a compositor use square corners throughout, avoiding the previous corner gaps.
- Four user-provided SF Pro Display faces (Regular, Semibold, Bold, Regular Italic) are included in `Word Memorizer/resources/fonts/` and copied by CMake. `GlassTheme::init()` registers them with the current process's Fontconfig and refreshes Pango's font map. Widget font overrides retain size/weight without forcing Sans. Chinese falls back to Noto Sans CJK SC or another available system font.
- Both `build/bin/word_memorizer` and `build-local/bin/word_memorizer` exist locally. The former dates from 2026-11-09 and still has the old theme; the user's launch path was not confirmed. Run the current `build-local/bin/word_memorizer`. Do not overwrite or reuse the old CMake cache.
- The current single-threaded whole-suite run passed all four targets in 37.80 s: vocabulary (0.00 s), GUI behavior (12.72 s), startup (0.64 s), styling (24.39 s). A separate `GTK_THEME=Yaru` style run passed in 23.25 s. New checks cover ten dark accent colors, button states, actual SF Pro Display English glyph runs with Chinese fallback, and title-label geometry.
- Main-window light/dark previews were visually reviewed after the corrections. Styling tests now have a 90-second timeout to accommodate the expanded checks and font-heavy desktops. Xvfb checks remain without a compositor; actual native desktop/Wayland interaction has not been exercised.

## Recovery status (2026-11-20)

- The previous Codex window stopped after updating this file, before committing the final styling changes. The resumed session reviewed the inherited changes, reran the checks and visually inspected all 16 previews.
- A live GitHub history check confirmed that `dev` already contained both earlier checkpoints, `780faf8` (10:39 CST) and `a560952` (10:46 CST), on 2026-11-20. Those commits had been published; the later styling changes were still uncommitted at takeover.
- The user had requested publishing to `Xiao-Han-OvO/Word-Memorizer-v3`, branch `dev`. The final styling checkpoint is identified by the commit message `Complete GTK dark theme and native title bars`; it includes the code, regression tests, README and this handoff.
- Check `git status --short --branch` and `git log -1 --oneline` when resuming. If the styling checkpoint is still ahead of `origin/dev`, finish the normal push to `dev` before starting another task.
- The first styling checkpoint was published as `2c05fc4`. The subsequent user-requested corrections are described above; check the current branch status before continuing or publishing further changes.
- The filename is `HANDOFF.md` (case-sensitive), not `HANDOFF.MD`.

## Repository

- The user selected `Xiao-Han-OvO/Word-Memorizer-v3`, branch `dev` (tracking `origin/dev`).
- At the user's explicit request, the old `dev` contents were replaced with this C++ / GTKmm project. The initial checkpoint is `780faf8`; the `.gitignore` checkpoint is `a560952`.
- GitHub Plugin is authenticated as `Federico-Prask`. Local Git uses the existing SSH authentication for pushes.
- Build directories, generated CMake files, editor caches and GUI previews are covered by `.gitignore`.

## Completed work

- Earlier functional fixes remain intact: GTK initialization, saved themes, safe imports, duplicate-spelling progress, BOM/CRLF parsing, preference cancellation, answer guards and completion-callback cancellation.
- `GlassTheme` applies GTK's native dark preference, updates existing windows, and generates readable text colors. Both external and built-in themes use the same CSS through `ThemeResources.h.in`.
- MainWindow, settings, help, about, word management and the wrong-word dialog use native GTK header bars. Manual dragging and duplicated title-bar rows were removed.
- SVG icons redraw with their parent widget's foreground color when the theme or widget state changes. Theme callbacks disconnect when icons are destroyed.
- Learning feedback honors the saved custom colors with readable brightness. Warning and help text follow the active palette.
- Window shadows are disabled without a compositor, including for GTK dialogs whose CSS node is not named `window`. Screen changes update that styling.
- Word-card construction no longer adds the same child twice. Display/edit controls stay hidden when appropriate, and Cancel restores the draft spelling.

## Verification

- The latest changes built with `cmake --build "Word Memorizer/build-local" --parallel 1`.
- All four targets passed in the latest whole-suite run; the Yaru style rerun also passed. Earlier sessions had intermittent GUI/startup timeouts, but none occurred in this final verification. Sandboxed Xvfb runs can fail to access their display; use the approved unsandboxed CTest command when that happens.
- All 16 light/dark previews were rendered and visually reviewed: placeholder, learning, settings, about, help, editor, incorrect answer and completion.
- GUI tests use temporary settings/vocabulary files and `G_DEBUG=fatal-criticals`; they do not modify personal preferences.
- Preview PNGs are in `Word Memorizer/build-local/style-previews/`, which is ignored by Git.
- Preview capture uses GTK drawing to a fresh Cairo image surface. Reading X11 window pixels through `gdk_pixbuf_get_from_window()` triggered a system Cairo MIME-cache assertion during repeated captures.
- Use `gdk_pixbuf_read_pixels()` for read-only pixel inspection. Check `get_storage_type() == Gtk::IMAGE_PIXBUF` before `get_pixbuf()`; native title-bar icons can be symbolic images.
- GUI verification ran under Xvfb without a desktop compositor. Native desktop/Wayland window-manager interaction was not exercised.
- `git diff --check` passed. Build outputs and preview images remain ignored by Git.

To rerun checks from the repository root:

```sh
cmake -S "Word Memorizer" -B "Word Memorizer/build-local" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build "Word Memorizer/build-local" --parallel 1
ctest --test-dir "Word Memorizer/build-local" --output-on-failure --parallel 1
git diff --check
```
