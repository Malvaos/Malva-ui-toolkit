# Malva UI for Slint

The Malva OS design system as a pure-Slint toolkit: one `Theme` global, 58 components, an 87-icon pack, a gallery that shows every state in dark and light, and your menu bar re-skinned onto the same tokens. No Rust, C++ or scripts are needed to use it - it is `.slint` files (Slint **1.18.1**).

```
slint-viewer gallery/gallery.slint              # every component, every state, dark + light
slint-viewer shell/menubar/preview/preview.slint --component PreviewControlDark
```
(On Windows: `slint-viewer.exe gallery\gallery.slint`.)

## Use it in your app

```slint
import { Theme, Button, ButtonVariant, TextField, Card, Toggle } from "@malva";

export component MyApp inherits Window {
    background: Theme.bg;
    default-font-family: Theme.font-family;   // "Inter"
    Card { Button { text: "Save"; clicked => { /* ... */ } } }
}
```
Point `@malva` at the barrel file:

| Where | How |
|---|---|
| `slint-viewer` / `slint-compiler` | `-L malva=path/to/malva-slint/ui/malva.slint` |
| Rust `build.rs` | `CompilerConfiguration::new().with_library_paths(HashMap::from([("malva".into(), "path/to/malva-slint/ui/malva.slint".into())]))` |
| C++ / CMake | pass `-L malva=...` to the compiler, or import by relative path: `import { Button } from "malva-slint/ui/malva.slint";` |

Or import by relative path and skip libraries entirely. Install **Inter** and **JetBrains Mono** for the intended type (Slint falls back to any installed font).

## Theming
Everything reads `Theme` (`ui/tokens.slint`). Flip it at runtime and the whole UI follows:

```slint
Theme.dark = false;           // light polarity (plum accent #7E376A); dark is orchid #D47ABC
Theme.balance = 70;           // 0 Soft ... 100 Crisp; 50 is the tuned default (radii, weights, shadows, contrast move together)
Theme.reduce-motion = true;   // every duration becomes 0 ms
Theme.high-contrast = true;   // opaque surfaces, solid borders, pure ink
Theme.reduce-transparency = true;
```
The values are Malva's balanced defaults from `docs/BALANCE.md` (surfaces `#1D101C/#2A1929` dark, `#F6F0F6/#FBF8FB` light; controls 12 px, windows 20 px; UI weight 500 at balance 50; feedback 120 ms, overlays 240 ms with the 1.35 spring; hover wash 8 %, pressed 14 %, press scale 0.96, disabled 40 %; touch target 44 px; 2 px accent focus ring with a 2 px halo). `docs/TOKENS.md` lists every token.

## What is in the box

| Group | Components |
|---|---|
| Foundations | `Theme`, `Icon` + `Icons` (87 icons), `Label` (8 text roles), `FocusRing`, `Tone`/`ToneStyle` |
| Controls | `Button` (primary / secondary / ghost / danger, 3 sizes, icon), `IconButton` (ghost / tonal / filled), `Toggle`, `Checkbox` (incl. mixed), `RadioButton`/`RadioGroup`, `Slider`, `TextField` (label, hint, error-after-blur, search, password), `Select` |
| Containers | `Card` (elevation 0-4), `Divider`, `Spacer`, `VStack`/`HStack`, `ScrollView`, `Picture` |
| Feedback | `ProgressBar`, `Spinner`, `Badge`, `Tooltip`, `Toast`, `Banner`, `Chip`, `Avatar`, `Stepper`, `ResultView` |
| Navigation | `Segmented`, `TabBar`, `DocumentTabs`, `Sidebar`/`SidebarItem`/`SidebarHeading`, `ListRow`, `ChoiceRow`, `SettingRow`, `ColumnHeader`, `PropertyRow`, `Kbd` |
| Overlays | `Menu`/`MenuItem`/`MenuSeparator`/`MenuHeading`, `Dialog` + `Scrim`, `CommandPalette` |
| Windows | `WindowFrame`, `Toolbar` |
| Data widgets | `AreaChart`, `RingGauge`, `StackedBar`, `StatTile`, `WidgetCard`, `MetricRow` |
| Shell | `Dock` (cosine magnification, badges, running marks, labels), **`MenubarWindow`** and its model structs (menus, tray, Wi-Fi, Bluetooth, battery, sound, calendar, control center, notifications, OSD, power) |

Every interactive control has the same five states (default, hover, pressed, focus, disabled), keyboard operation (Space/Enter; arrows on sliders), accessible roles/labels, and a `preview-state` property that draws a state without a pointer (used by the gallery and for visual tests). `docs/COMPONENTS.md` documents every property and callback.

## The menu bar
`shell/menubar/` is your Cutefish menu bar, unchanged except its `ui/theme.slint`, which now maps each of its tokens onto `Theme` (so it follows dark/light/balance/reduce-motion with everything else). The 23 preview scenes and the elements sheet work as before; their wallpapers are the Malva Dusk/Dawn images. Backend wiring is described in `shell/menubar/ORIGINAL-README.md`. The bar's `Screen` and `Motion` globals are exported from the barrel as `ShellScreen` and `ShellMotion`; its `Theme` is simply Malva's `Theme`.

## Layout
```
ui/malva.slint        the one import     ui/tokens.slint   Theme       ui/icons.slint   Icon + Icons (generated)
ui/components/*.slint one file per family
gallery/              the gallery app (10 pages)        tests/all-components.slint   compile + smoke test
shell/menubar/        menu bar module + preview scenes   docs/   generated reference     screenshots/   renders
tools/                optional generators (icons, chart paths, docs) - their output is committed; you never need to run them
```

## Design rules baked in
Accent is dusty orchid/plum, never neon; surfaces tint toward plum, never grey. Destructive actions are rose and never the default button. Colour is always paired with text or an icon. Fields validate after you leave them, not while typing. Focus rings show for keyboard focus only.

## Known limits (read these)
- **No backdrop blur in Slint.** `Theme.glass` is a translucent plate; the compositor supplies the blur behind it. The menu bar's preview fakes it with a pre-blurred backdrop image, as the original did.
- **One drop shadow per element**, so Malva's two-layer elevation is approximated by a single tuned shadow plus the 1 px hairline.
- **Charts take at most 64 points.** Slint cannot loop inside a `Path`, so `ChartPath` builds the path string with a generated, unrolled function.
- **`Dock` magnification** uses the resting-position cosine and displaces neighbours with its integral; it is exact for equal-size icons, approximate otherwise.
- The shell's own `Symbol`/app-icon art is yours and is not re-drawn in Malva style.

## Verification
All files compile with the Slint 1.18.1 compiler (Python interpreter) **and** the 1.18.1 native `slint-compiler` from the C++ SDK. The screenshots in `screenshots/` were rendered by the 1.18.1 interpreter (femtovg renderer on Mesa software GL) from these exact files. Not verified here: running the Windows `slint-viewer.exe` (same version, not run), and Rust `build.rs` / CMake integration (no Rust toolchain or CMake in the build environment) - the `-L` form above was verified with the native compiler.
