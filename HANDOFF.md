# CLI handoff

The dark-theme and title-bar task is complete. Follow `AGENTS.md` for further work.

## Recovery status (2026-11-20)

- The previous Codex window stopped after updating this file, before committing the final styling changes. The resumed session reviewed the inherited changes, reran the checks and visually inspected all 16 previews.
- A live GitHub history check confirmed that `dev` already contained both earlier checkpoints, `780faf8` (10:39 CST) and `a560952` (10:46 CST), on 2026-11-20. Those commits had been published; the later styling changes were still uncommitted at takeover.
- The user had requested publishing to `Xiao-Han-OvO/Word-Memorizer-v3`, branch `dev`. The final styling checkpoint is identified by the commit message `Complete GTK dark theme and native title bars`; it includes the code, regression tests, README and this handoff.
- Check `git status --short --branch` and `git log -1 --oneline` when resuming. If the styling checkpoint is still ahead of `origin/dev`, finish the normal push to `dev` before starting another task.
- No implementation work remains for the dark-theme/title-bar task. A subsequent feature or fix has not been specified. Native desktop/Wayland interaction remains unverified.
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

- The resumed incremental build passed with `cmake --build "Word Memorizer/build-local" --parallel 1`; all targets were already up to date.
- On 2026-11-20, the single-threaded whole-suite run passed vocabulary (0.04 s), application startup (6.68 s) and styling (19.55 s). GUI behavior hit the 30-second timeout before producing any test output, then passed on a focused rerun (10.15 s). All four targets have passing results, but the whole-suite run was not fully green. Similar GUI/startup timeouts occurred in the previous session; their cause is still unconfirmed.
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
