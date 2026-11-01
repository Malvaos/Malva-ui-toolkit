# Using the kit

One source (`tokens/malva-tokens.json`), five ways to read it. Pick the ones your program needs; they agree byte for byte.

## Slint

The library is `slint/malva.slint`. Import it as `@malva/malva.slint` by giving the compiler a library path named `malva`
that points at the `slint/` folder.

```slint
import { Theme, Button, ButtonKind, Toggle, TextField, Card, Heading, Body } from "@malva/malva.slint";

export component Settings inherits Window {
    default-font-family: "Inter";            // the fonts are embedded when the kit is imported
    background: Theme.bg;
    in-out property <bool> dark <=> Theme.dark;   // flip it and every component follows
    Card {
        VerticalLayout {
            padding: 24px;
            spacing: 12px;
            Heading { text: "Appearance"; }
            Toggle { text: "Dark mode"; checked <=> root.dark; }
            Button { text: "Save"; kind: ButtonKind.primary; clicked => { /* ... */ } }
        }
    }
}
```

| Tool | How |
|---|---|
| `slint-viewer` | `slint-viewer -L malva=path/to/malva-ui-kit/slint app.slint` |
| `slint-compiler` | `slint-compiler -L malva=path/to/malva-ui-kit/slint -f cpp app.slint` |
| CMake (C++) | `malva_add_slint(target app.slint)` (from `cpp/`, see below), or `slint_target_sources(target app.slint LIBRARY_PATHS malva=${MALVA_SLINT_DIR})` |
| Rust (`slint-build`) | `CompilerConfiguration::new().with_library_paths(...)` with `malva` pointing at `slint/`. **Not tested here**; the C++ and viewer routes are. |
| Without a library path | import by relative path: `import { Button } from "../malva-ui-kit/slint/malva.slint";` |

* Colours are `Theme.*` (`Theme.accent`, `Theme.text-2`, `Theme.border`), sizes `Size.*`, radii `Radius.*`, gaps `Space.*`,
  type `Typo.*`, timings `Motion.*`. A colour typed by hand in a component is a bug: add a role to the tokens.
* `Theme.accent` is an `out` property: a computed role of the mode, so a program cannot set it. The three apps let a user
  choose an accent by keeping an `in-out` accent in their own `theme.slint` and using it where they would use
  `Theme.accent`; the kit's own components keep the kit's. To re-brand the kit itself, change the token and regenerate.
* Any component can be drawn in a state without a pointer: `Button { force: ForceState.hover; }`.
* [COMPONENTS.md](COMPONENTS.md) lists every component's properties and callbacks.

### Windows

`TitleBar` (44 px, three window controls at the left, the title in the middle), `WindowFrame` and `Sidebar` /
`SidebarSection` draw a window that the system does not decorate (`no-frame: true`). The controls use `Theme.win-close`,
`win-min` and `win-max`. Rounded window corners need a compositor that blends alpha.

## C++

Header-only, standard library only: `cpp/include/malva/`.

```cpp
#include <malva/theme.h>        // Theme, Mode, Colors; pulls in color.h and tokens.h
#include <malva/motion.h>       // ease(), Spring, durations
#include <malva/icons.h>        // malva::icons::symbols / apps / logos: name -> SVG text

malva::Theme theme(malva::Mode::Dark);
malva::Rgba bg = theme.colors().bg;
theme.on_change([](malva::Mode m) { /* push the new colours to your toolkit */ });
auto hover = malva::mix(theme.colors().accent, theme.colors().text, 42);   // the same integer maths as the generator
double k = malva::contrast_ratio(theme.colors().text, theme.colors().bg);
```

With Slint, add `#include <malva/slint_interop.h>` for `malva::to_slint(Rgba)` and `malva::from_slint(slint::Color)`.

CMake: `add_subdirectory(malva-ui-kit/cpp malva)`, then `target_link_libraries(app PRIVATE Malva::UI)` and
`malva_add_slint(app ui/app.slint)`. `cpp/examples/consumer/` is a whole project doing exactly this.

## Rust

Four std-only crates in the workspace `rust/` (add them by path):

```rust
use malva_tokens::{colors, Mode};
let c = colors(Mode::Dark);
let accent = c.accent;                              // malva_color::Rgba { r, g, b, a }
let hex = accent.to_string();                       // "#DC73C2"

let svg = malva_icons::svg_with_color("bell", "#8C3274").unwrap();
let e = malva_motion::ease(malva_motion::curves::OUT, 0.5);
```

`malva-iced` is the theme for [iced](https://iced.rs) 0.14 and is documented in `rust/malva-iced/README.md`, including
what was and was not tested.

## CSS and the web

```html
<link rel="stylesheet" href="malva.css">
<html data-theme="dark">   <!-- optional: otherwise prefers-color-scheme decides -->
```

```css
.card { background: var(--malva-surface); color: var(--malva-text); border: 1px solid var(--malva-border);
        border-radius: var(--malva-radius-l); box-shadow: var(--malva-shadow-2); }
.primary { background: var(--malva-accent); color: var(--malva-on-accent); }
body { font: var(--malva-type-body); }
```

`dist/malva.dtcg.json` is the same tokens in the W3C Design Tokens format, which Figma plugins and Style Dictionary read.
`dist/malva.gpl` is the palette for GIMP, Inkscape and Krita.

## SVG

* `svg/icons/*.svg`: 130 symbols on a 24 grid, `currentColor`. In HTML: inline them, or use them as a CSS mask. In Slint:
  `Icon { name: "bell"; tint: Theme.text-2; }`.
* `svg/app/*.svg`: 20 coloured app pictures (`AppIcon { name: "files"; }`; unknown names show `default`).
* `svg/logo/*.svg`: the brand kit's logos. `Logo { kind: "horizontal"; reverse: false; }` in Slint (`kind`: `mark`, `wordmark`, `horizontal`, `stacked`, `bloom`, `app`).
* `svg/parts/`: the squircle, the focus ring, an animated spinner and skeleton, the three elevations.
* `svg/specimen-light.svg`, `specimen-dark.svg`: every basic component on one sheet, for design tools.
* `svg/wallpaper/malva-light.svg`, `malva-dark.svg`: 1920 x 1200 desktop wallpapers.

To add an icon, put a 24-grid `currentColor` SVG in `svg/icons/` and run `python3 tools/gen_icons.py`.

## The apps

`apps/files`, `apps/dock`, `apps/menubar`: each is a Slint UI plus a deliberately small C++ backend that you are meant to
replace. Their READMEs describe the properties and callbacks a backend implements. Build all three with
`-DMALVA_BUILD_APPS=ON` from the kit's root, or one of them on its own (`cmake -S apps/dock -B build-dock ...`).
