# Design

How Malva looks and why. Every number here is in `tokens/malva-tokens.json`; this page explains it, the file defines it.

## 1. Black and white, with the logo's colour

The interface is neutral. Warm-free greys for surfaces, charcoal and silver for text, a razor line for borders. One
colour is allowed to be strong, and it is the logo's: Plum `#8C3274` (the mark's gradient runs Plum to Orchid
`#C763B0`), lightened to `#DC73C2` in dark so it keeps its contrast.

The same idea as a phone maker whose logo is blue and whose interface is black and white: the brand colour is a signal,
not a wallpaper. It marks **what you can act on** and **what is selected**.

| Accent appears on | Accent does not appear on |
|---|---|
| Primary buttons, links, ghost-button text | Window, card, sidebar and menu backgrounds |
| The on state of a switch, checked boxes, radio dots | Body text, headings, borders, dividers |
| The slider fill and progress fill | Icons at rest (they are `text-2`) |
| Selection (a faint wash of it over the neutral), the focus ring | Shadows (they are black) |
| The running dot in the dock, today in the calendar | Status (success is green, warning amber, danger red, info blue) |

Status colours are their own hues on purpose: information is blue, not plum, so a notice is never mistaken for the brand.

### Surfaces

| Role | Light | Dark | Used for |
|---|---|---|---|
| `bg` | `#F5F5F7` | `#161617` | The canvas behind everything |
| `panel` | `#EDEDF0` | `#1A1A1B` | Sidebars and title bars |
| `surface` | `#FFFFFF` | `#1E1E1F` | Windows, cards, content |
| `raised` | `#FFFFFF` | `#29292B` | Menus, popovers, dialogs, toasts (one step above the window) |
| `border` | `#E5E5EA` | `#2C2C2E` | Hairlines round cards and secondary buttons |
| `text` | `#1D1D1F` | `#F5F5F7` | Charcoal, never stark black; silver, never glaring white |
| `text-2` | `#86868B` | `#8E8E93` | Secondary text, hierarchy |
| `muted` | `#A1A1A6` | `#6E6E73` | Placeholders, hints |

Elevation in light mode is shadow; in dark mode it is a lighter surface plus a stronger shadow (`surface` to `raised`).

`text-2` on white is 3.6:1, below the 4.5:1 that WCAG AA asks of small text. It is the grey that was specified, and it is
meant for supporting text; anything a person must read to act (labels, button text, errors, values) is `text` or a
status text colour, all of which pass 4.5:1. The tests encode this: see the comment above `CONTRAST_PAIRS` in
`tools/gen_tokens.py`.

### The accent in use

| Role | Light | Dark |
|---|---|---|
| `accent` | `#8C3274` | `#DC73C2` |
| `accent-hover` | `#5D2950` (accent mixed 42% toward text) | `#F093D7` |
| `accent-press` | `#47253F` | `#C75EAE` |
| `on-accent` | `#FFFFFF` | `#1D1D1F` |
| `accent-wash` | `#EBE2EA` (selected row) | `#422A3D` |
| `focus` | `#8C3274` | `#F093D7` |

The brand palette itself (petal `#E6DAE8`, lilac `#DEC3E0`, orchid `#B84F9F`, plum `#8C3274`, leaf `#274903`, ink
`#2A1226`) stays in the tokens for logos and illustration. Interface roles never use petal, lilac or ink.

## 2. Shape

| Token | Values | Used for |
|---|---|---|
| Radius | `xs` 6, `s` 10, `m` 14, `l` 22, `xl` 36, `pill` 999 | Chips and checkboxes (xs); buttons and fields (s); menus (m); cards and dialogs (l); the app-icon squircle (xl) |
| Space | 4, 8, 12, 16, 24, 32, 48 | One 4-point scale for every gap and padding |
| Control height | 32 / 40 / 48 (small, medium, large); fields 42 | A control is never shorter than 32 |
| Touch | switch 52 x 30, checkbox 26, radio 28, slider thumb 22 | |

App icons are cut from a squircle (`svg/parts/squircle.svg`, |x|^5 + |y|^5 = 1) with a vertical gradient, a soft gloss
over the upper half, a light inner edge and a dark outer hairline. Symbols are a 24-unit grid, 1.8 stroke, round caps and
joins, one colour (`currentColor`).

## 3. Type

Inter for the interface, JetBrains Mono for code. Both ship in `slint/fonts` and register themselves when the kit is
imported.

| Role | Size / line | Weight |
|---|---|---|
| Title | 28 / 34 | 700 |
| Heading | 20 / 26 | 600 |
| Body | 14 / 20 | 400 |
| Body strong | 14 / 20 | 600 |
| Label | 13 / 18 | 600 |
| Caption | 12 / 16 | 400 |
| Mono | 13 / 18 | 400 |

## 4. Motion

Motion explains; it never decorates. Press is instant, hover is quick, things that arrive take a little longer than
things that leave.

| Duration | ms | | Easing | Curve |
|---|---|---|---|---|
| `press` | 90 | | `out` | cubic-bezier(0, 0, 0.2, 1): decelerates into place |
| `hover` | 160 | | `standard` | cubic-bezier(0.2, 0, 0, 1): quick start, long soft landing |
| `fade` | 200 | | `soft` | cubic-bezier(0.4, 0, 0.2, 1): fades in and out |
| `open` | 240 | | `spring` | cubic-bezier(0.34, 1.35, 0.64, 1): rises about 4% past, settles |
| `close` | 160 | | `exit` | cubic-bezier(0.4, 0, 1, 1): accelerates away |
| `slow` | 380 | | | |

In the components, hover and press fades and the 0.98 press scale use `out`; menus and dialogs open with `open` +
`spring`; a toast arriving and a progress bar filling use `slow`. The durations and curves are separate tokens (`Motion.hover`, `Motion.curve-out`),
so a component picks one of each.

A spring (stiffness 420, damping 32, mass 1) is available in C++ and Rust (`malva::motion::Spring`, `malva_motion::Spring`)
for things that follow a pointer, such as the dock's magnification.

## 5. States

Every interactive control has the same five: rest, hover, pressed, focus, disabled.

* **Hover** and **pressed** shift the fill one step (`accent-hover`, `accent-press`; `secondary-hover`, `secondary-press`;
  `ghost-hover`, `ghost-press`). A pressed control scales to 0.98 in 90 ms.
* **Focus** is a 2 px ring, 2 px outside the control, in `focus`. It shows for keyboard focus only.
* **Disabled** drops the fill to `accent-disabled` (40%) or the text to `disabled-text`; it never hides the control.
* Destructive actions are `danger-fill` and are never the default button. Colour is always paired with text or an icon.

Every Slint component takes `force: ForceState.hover | pressed | focus` so a design sheet can draw a state without a
pointer (the screenshots do).

## 6. The three apps

The Files app, the dock and the menu bar use the same tokens through a small `theme.slint` each, which maps their own
names onto the kit's `Theme`, `Radius` and `Typo`. What belongs to the app (the dock's glass, the bar's hairline, the
selection card of Files) is built from kit colours with alpha, so it follows light and dark with everything else.

The glass of the dock and the bar is a translucent plate, `surface` or `bg` at 60 to 85% opacity depending on mode, with a
1 px sheen along its top edge and a hairline outside. Slint cannot blur what is behind a window; the compositor does, or
the app is handed a blurred copy of the wallpaper (`backdrop`), which is what the screenshots show.

Wallpapers (`svg/wallpaper`) follow the same rule as the interface: silver or graphite, with one soft bloom of the logo's
colours low in the frame.

## 7. Rules of thumb

1. If a surface needs emphasis, give it more elevation or a hairline, not a colour.
2. If something is actionable, it may carry the accent. If it is not, it may not.
3. Text is `text` unless it is supporting (`text-2`), a hint (`muted`) or disabled.
4. Use a token. A hex value in a component is a bug; add a role to `malva-tokens.json` instead.
5. Never tint neutrals toward the brand. Greys stay grey.
