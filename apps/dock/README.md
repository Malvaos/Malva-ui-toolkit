# Dock: the Slint frontend

The redesigned Malva OS dock as a **Slint** user interface with **SVG** icons, and a deliberately small
**C++ backend** that you are meant to replace. It is the Files redesign's sibling: the same Malva tokens. The UI owns all drawing and all motion; the backend says what is in the dock and hears what the
person did.

![Light](../../docs/screenshots/apps/dock-light.jpg)

| | |
|---|---|
| `../../docs/screenshots/apps/dock-*.jpg` | Seven scenes on a desktop (`preview/preview.slint`): light, dark, context menu (both), a dock on the left and on the right, the bin with its lid open. The glass shows a blurred copy of what is behind it. |
| `tools/record-demo.py`, `tools/make-filmstrips.py` | Record the compiled app under a real (X11) pointer and cut the run into filmstrips. The recordings themselves are not part of the kit. |

**Built on the Malva kit.** `ui/theme.slint` maps the dock's names onto the kit's `Theme`, `Radius` and `Typo`
(`@malva/malva.slint`), so the dock follows `tokens/malva-tokens.json`: neutral off-white and graphite surfaces, with
the logo's plum / orchid as the only strong colour (the running dot, the selection plate and focus).

## What is here

```
ui/
  dock.slint             the one file a backend compiles; exports DockWindow, DockApp, MinimizedWindow, MenuEntry, Theme, Motion
  dock-window.slint      the Window: every property and callback a backend uses, in one flat list
  theme.slint            Theme (colours, glass, radii, light/dark) and Motion (durations) globals
  types.slint            DockApp, MinimizedWindow, MenuEntry
  components/            DockPanel (the glass, the strip, the pointer, the label, the menu's place, dragging),
                         DockSlot (one icon and everything that happens to it), DockMenu, DockTooltip,
                         surfaces (glass plate, soft shadow), icon (name to SVG)
  icons/app/             18 SVGs: 15 app tiles, the launcher among them, and the bin's three parts
  icons/symbols/         13 one-colour symbols for the menu, tinted by the theme
preview/                 design scenes with sample data and wallpapers (nothing here ships)
backend/                 the stand-in backend (replace it)
  dock_model.*           what the dock holds and what each choice does. No Slint.
  app.*                  wires the window's callbacks to the model
  main.cpp               options, run loop, --snapshot, --record
tests/                   model_test (pure logic), app_test (the window and backend, driven by a moving pointer)
tools/                   make-icons.py, make-wallpapers.py, render-previews.sh, record-demo.py, make-filmstrips.py, xinput.py
```

## Build and run

You need the Slint C++ SDK 1.18 (built and tested against 1.18.1; other versions are untested), CMake 3.21 and a C++20 compiler.

```sh
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/path/to/Slint-cpp-1.18.1-Linux-x86_64 -DCMAKE_BUILD_TYPE=Release -DDOCK_APP_TEST=ON
cmake --build build
ctest --test-dir build
./build/dock [--dark] [--edge=bottom|left|right] [--accent=#8c3274] [--icon-size=56] [--magnification=1.65]
             [--no-launcher] [--autohide] [--desk=#8e8e93] [--size=1100x440]
```

Use a GPU renderer (`SLINT_BACKEND=winit-femtovg`, or Skia if your Slint has it). Slint's software renderer draws no blurred shadows and bands long soft gradients (found with the Files app;
not retried for the dock, whose label, menu and icon shadows would lose them).

The demo draws the dock in an ordinary window on a flat "desk" colour. A real dock is a transparent window along a
screen edge (a layer-shell surface on Wayland); see *What the window needs* below.

To look at the UI without a backend: `slint-viewer preview/preview.slint --component PreviewDark`. To redraw every
picture here: `tools/render-previews.sh`, `tools/record-demo.py build/dock out-dir` (needs Xvfb) and
`tools/make-filmstrips.py out-dir pictures-dir`.

## Writing your backend

Compile `ui/dock.slint`, then `#include "dock.h"` and create a `DockWindow`. `backend/app.cpp` is a complete working
example of everything below.

**Give it** (properties on `DockWindow`, all with `set_` setters):

| property | meaning |
|---|---|
| `apps` `[DockApp]` | the apps, in order: `id` (stable), `name`, `icon` (a built-in name, see `ui/components/icon.slint`; unknown shows `default`) or `icon-image` + `has-icon-image`, `pinned`, `running`, `active` (has the focused window: its mark is the long pill), `launching` (the icon hops), `badge` (red count, 0 for none), `windows` (for the label), `removing` |
| `minimized-windows` `[MinimizedWindow]` | the tray after the apps: `title`, `icon`, and a `preview` picture (without one, a small window is drawn) |
| `show-launcher`, `trash-full` | the launcher at the start, the bin's contents |
| `edge` | `"bottom"`, `"left"` or `"right"` |
| `icon-size`, `magnification` | logical pixels at rest (default 56); the icon under the pointer grows to this many times (default 1.65, 1 for none) |
| `hidden` | slid away off the edge; false brings it back |
| `animate-entrance` | icons arrive one after another when the dock appears |
| `menu-entries` `[MenuEntry]`, `menu-open` | the context menu; each entry has `text`, `kind` (`item`, `header`, `separator`), `icon` (a symbol name), `checked`, `destructive`, `enabled` |
| `backdrop`, `has-backdrop`, `backdrop-x/y/width/height` | a blurred copy of the wallpaper for the glass to show, when you have one |
| `dark`, `accent`, `desk` | the look; set from the desktop's settings. `desk` is what shows where the dock is not, for a demo window; leave it clear |

**Hear it** (`on_...` callbacks): `app_clicked(i)`, `app_context(i)`, `launcher_clicked`, `launcher_context`,
`trash_clicked`, `trash_context`, `window_clicked(j)`, `window_context(j)`, `menu_chosen(row)`, `menu_dismissed`,
`reordered(from, to)`, `app_dragged_off(i)`, `reveal_requested`. Indexes are into `apps` or `minimized-windows`.

**Read it**: `pointer_inside` (for auto-hide), `footprint` (how far the glass reaches from the edge, margin
included: the exclusive zone), `dock_length`, and `app_x(i)`, `window_x(j)`, `launcher_x()`, `trash_x()`,
`icon_y()` for where the icons are with nothing magnified (a bottom dock; for input regions and tests).

Five rules that are easy to miss:

1. **Rows change in place.** Keep `id` stable. When an app goes, set `removing` and drop the row about 300 ms later:
   the icon shrinks away and the others close up. When two rows swap or one is erased, the slots stay and their
   contents change; the UI shows the new occupant without an arrival, so a reorder is a clean swap.
2. **Fill the dock after the window is on the screen** if you want its entrance seen. Icons that arrive in a window
   that is not yet showing have finished arriving by the time it shows. (`DockWindow` also keeps `animate-entrance`
   for a quiet start.)
3. **The menu is yours to fill.** A right click on an icon emits `*_context`; build the entries, set `menu-open`.
   Clear `menu-open` on `menu_chosen` and `menu_dismissed`. A click anywhere else, or on another icon, dismisses it.
4. **Clicks and drags are the UI's to tell apart.** A press that moves more than 6 px becomes a drag and never a
   click. Along the dock it reorders (`reordered`); pulled away from the edge by more than about 1.3 icons, or dropped
   on the bin, it removes (`app_dragged_off`). The model changes are yours; the UI has already drawn them.
5. **Auto-hide is two halves.** The UI slides the dock away when `hidden` and calls `reveal_requested` when the
   pointer touches the edge where it waits; the margin between the glass and the screen's edge counts as the dock, so
   a pointer resting on the edge does not make it flicker. When to hide (the pointer has left for a while and no menu
   is open) is yours: see `App::checkAutoHide`.

### What the window needs

`DockWindow` is the dock's whole area: the glass sits on the screen's edge in the middle of it, and the rest is clear,
which is room for the icons to grow, for the label and for the menu. For a bottom dock make it the screen's width and
about `icon-size * 2.2 + 300 px` tall (more if your menus are long), transparent, and pass pointer events through
everywhere but the glass, the grown icons, the label and the menu. Frameless transparent windows need a compositor
that blends alpha. None of the window management is in this project.

## Motion and colour

Everything tunable is in `ui/theme.slint`. `Motion`: magnify in 90 ms and out 260 ms (out is slower so a sweep never
flickers), hover 140/220, a hop 300 ms up and 300 ms down, arrival 360 ms staggered 28 ms, running mark 240 ms, label
waits 320 ms then fades 140 and glides 170 between icons, menu 200 ms with rows 16 ms apart, neighbours part 180 ms,
bin lid 280 ms, slide off the edge 320 ms. `Theme`: the palette, glass tint, rim, hairline and sheen in light and dark,
the icons' shadow, radii. Icon size follows everything: gaps, padding, radius, the mark, the badge.

The magnification is `1 + (m - 1) * (1 + cos(pi * d / 130 px)) / 2` for a distance `d` from the pointer, measured from
resting positions so the effect stays steady as icons move. The glass's shadow is drawn as fading 1 px rings, not a blur,
so it never tints the translucent glass it surrounds.

## Tested, and not

Verified here: the UI compiles; the scenes and sheets render in light and dark; the C++ backend builds against the
SDK; `ctest` runs `model_test` and `app_test` (`-DDOCK_APP_TEST=ON`, which needs Slint's testing backend, in the SDK).
`app_test` moves a pointer, hovers, clicks with both buttons and drags, through the window's own event handling, with
time stepped by hand. It checks that hovering grows the dock and leaving settles it back, that a click launches (the
hop lasts until the app is up) or focuses; that a right click opens the right menu and each
choice does its work; that a header, a disabled row and a click on the desk behave; that dragging reorders, that a
short drag is not a click, that dragging away removes (and keeps an app that is running); the bin; the tray.

Verified by recording: the compiled app was run under Xvfb with Slint's `winit-femtovg` renderer and driven by a real
X pointer (XTest); the app snapshots its own window about every 40 ms. The filmstrips and videos are those frames.
Auto-hide was run the same way: it slides away, comes back when the pointer reaches the edge, stays while the pointer
is on the dock or its margin, and goes again after it leaves.

Running the tests and the recordings found real bugs, all fixed: icons that never animated in because arrival began
before the first frame; every icon lifting when one was dragged away; the running mark staying behind when its icon
made way; the dragged icon drifting from the pointer as the dock shrank under it; a drag that ended inside its own
icon firing a click; the dock flickering on the screen's edge when auto-hidden.

**Not verified:** the animations were watched as frames, not on a display at the refresh rate, so timings and curves
are specified and sampled but their feel is untested. Nor: a GPU or a Wayland compositor, a layer-shell surface (the
exclusive zone, input regions), high-DPI scales, real compositor blur behind the glass (the previews use a
pre-blurred wallpaper through `backdrop`; without one the glass is just translucent), dragging and hover on the left
and right docks (they are drawn in the previews; the code is the same with the axes swapped but was not driven),
system icon themes (`icon-image` is wired but was not tried), and many apps.

**Not included:** shrinking icons to fit when there are too many for the screen (the dock grows past the edge), keyboard
navigation and accessibility, touch and long press, drag and drop of files onto an icon, a real launcher, real process
and window tracking, and the window management above.
The sample apps, wallpapers and the mock desktop in `preview/` are stand-ins.

## Licence

No licence is applied to these files; choose one that suits where they go. Slint itself is offered under GPL-3.0, a
royalty-free licence, or a commercial one: check which applies to you.
