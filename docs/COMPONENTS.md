# Component reference

_Generated from the `///` comments in `ui/components/*.slint` by `tools/make-docs.py`. Import everything from `ui/malva.slint`._


### `Label`

Text in one of Malva's roles. Hierarchy comes from size and weight steps: 400 body, the dial's ~500 for UI labels, 650 for titles only.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `role` | TextRole | `TextRole.body` |  |
| `muted` | bool | `false` |  |
| `strong` | bool | `false` |  |

### `Button`

A push button. States: default / hover / pressed (scales to 0.96) / focus (keyboard ring) / disabled (40 %).

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `text` | string |  |  |
| `icon` | string |  | Name of an icon from `Icons` shown before the text. |
| `variant` | ButtonVariant | `ButtonVariant.primary` |  |
| `size` | ControlSize | `ControlSize.md` |  |
| `enabled` | bool | `true` |  |
| `preview-state` | string |  | Documentation/testing aid: "hover", "pressed" or "focus" draws that state without a pointer. |
| `clicked()` | callback |  |  |
| `pressed` | bool | `ta.pressed \|\| preview-state == "pressed"` |  (output) |
| `hovered` | bool | `ta.has-hover \|\| preview-state == "hover"` |  (output) |
| `has-focus` | bool | `fs.has-focus` |  (output) |

### `IconButton`

A square button with only an icon. `selected` shows it as the active one of a group.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `icon` | string | `"plus"` |  |
| `look` | IconButtonStyle | `IconButtonStyle.ghost` |  |
| `size` | ControlSize | `ControlSize.md` |  |
| `selected` | bool | `false` |  |
| `enabled` | bool | `true` |  |
| `label` | string |  | Spoken/tooltip name; also used by accessibility tooling. |
| `preview-state` | string |  | Documentation/testing aid: "hover", "pressed" or "focus" draws that state without a pointer. |
| `clicked()` | callback |  |  |
| `pressed` | bool | `ta.pressed \|\| preview-state == "pressed"` |  (output) |
| `hovered` | bool | `ta.has-hover \|\| preview-state == "hover"` |  (output) |

### `Toggle`

An on/off switch. The knob arrives with the Malva spring (a ~4 % overshoot).

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `checked` | bool | `false` |  |
| `enabled` | bool | `true` |  |
| `label` | string |  |  |
| `preview-state` | string |  | Documentation/testing aid: "hover", "pressed" or "focus" draws that state without a pointer. |
| `toggled(bool)` | callback |  |  |
| `hovered` | bool | `ta.has-hover \|\| preview-state == "hover"` |  (output) |

### `Checkbox`

A checkbox with an optional label. `indeterminate` shows a dash (a "some selected" parent).

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `checked` | bool | `false` |  |
| `indeterminate` | bool | `false` |  |
| `enabled` | bool | `true` |  |
| `text` | string |  |  |
| `preview-state` | string |  | Documentation/testing aid: "hover", "pressed" or "focus" draws that state without a pointer. |
| `toggled(bool)` | callback |  |  |
| `hovered` | bool | `ta.has-hover \|\| preview-state == "hover"` |  (output) |

### `RadioButton`

One radio button. Use `RadioGroup` for a set.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `selected` | bool | `false` |  |
| `enabled` | bool | `true` |  |
| `text` | string |  |  |
| `preview-state` | string |  | Documentation/testing aid: "hover", "pressed" or "focus" draws that state without a pointer. |
| `clicked()` | callback |  |  |
| `hovered` | bool | `ta.has-hover \|\| preview-state == "hover"` |  (output) |

### `RadioGroup`

A vertical set of radio buttons where exactly one is selected.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `options` | [string] |  |  |
| `current-index` | int | `0` |  |
| `enabled` | bool | `true` |  |
| `selected(int)` | callback |  |  |

### `Slider`

A value slider. `value` runs from `minimum` to `maximum`; arrow keys move by `step`.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `value` | float | `0.5` |  |
| `minimum` | float | `0` |  |
| `maximum` | float | `1` |  |
| `step` | float | `0.05` |  |
| `enabled` | bool | `true` |  |
| `label` | string |  |  |
| `preview-state` | string |  | Documentation/testing aid: "hover", "pressed" or "focus" draws that state without a pointer. |
| `changed(float)` | callback |  |  |

### `TextField`

A single-line text field. Validation shows after the person leaves the field, never while typing.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `text` | string |  |  |
| `placeholder` | string |  |  |
| `label` | string |  |  |
| `hint` | string |  |  |
| `error` | string |  |  |
| `leading-icon` | string |  |  |
| `password` | bool | `false` |  |
| `enabled` | bool | `true` |  |
| `read-only` | bool | `false` |  |
| `preview-state` | string |  | Documentation/testing aid: "hover", "pressed" or "focus" draws that state without a pointer. |
| `edited(string)` | callback |  |  |
| `accepted(string)` | callback |  |  |
| `has-focus` | bool | `input.has-focus` |  (output) |

### `Card`

A surface for grouped content. `elevation` 0 (flat) .. 4 (modal); `interactive` adds hover/press.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `elevation` | int | `1` |  |
| `content-padding` | length | `Theme.space-md` |  |
| `interactive` | bool | `false` |  |
| `selected` | bool | `false` |  |
| `fill` | color | `Theme.surface` |  |
| `clicked()` | callback |  |  |
| `hovered` | bool | `area.has-hover` |  (output) |

### `Divider`

A 1 px hairline.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `vertical` | bool | `false` |  |
| `strong` | bool | `false` |  |

### `Spacer`

Empty space: a fixed `size`, or `flexible` to push siblings apart.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `size` | length | `0px` |  |
| `flexible` | bool | `false` |  |

### `VStack`

A vertical stack with the Malva default gap (12).


### `HStack`

A horizontal stack with the Malva default gap (12).


### `ProgressBar`

A determinate (0..1) or indeterminate progress bar.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `value` | float | `0` |  |
| `indeterminate` | bool | `false` |  |
| `tone` | Tone | `Tone.accent` |  |
| `thickness` | length | `8px` |  |

### `Spinner`

A spinning arc for work of unknown length.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `size` | length | `24px` |  |
| `tint` | color | `Theme.accent` |  |

### `Badge`

A small pill that labels a status or a count. Pair the colour with the text (or an icon).

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `text` | string |  |  |
| `icon` | string |  |  |
| `tone` | Tone | `Tone.neutral` |  |

### `Picture`

An image clipped to a shape, cropped to fill.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `source` | image |  |  |
| `shape` | PictureShape | `PictureShape.squircle` |  |
| `corner` | length | `Theme.radius-control` |  |

### `ScrollView`

A vertical scroll area with a thin overlay bar that brightens near the pointer. Children are stacked top to bottom inside the scrolling content.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `content-padding` | length | `0px` |  |
| `offset` | length | `0px` |  |
| `content-height` | length | `fl.content-height` |  (output) |

### `Segmented`

A pill-shaped segmented control: one of N options, the highlight glides between them.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `options` | [string] |  |  |
| `current-index` | int | `0` |  |
| `enabled` | bool | `true` |  |
| `selected(int)` | callback |  |  |

### `TabBar`

A row of text tabs with an accent underline that glides to the active one.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `options` | [string] |  |  |
| `current-index` | int | `0` |  |
| `selected(int)` | callback |  |  |

### `SidebarItem`

One destination in a Sidebar. `count` shows a small number on the right when it is above 0.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `icon` | string |  |  |
| `text` | string |  |  |
| `active` | bool | `false` |  |
| `count` | int | `0` |  |
| `clicked()` | callback |  |  |

### `SidebarHeading`

A small caption that groups sidebar items.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `text` | string |  |  |

### `Sidebar`

The navigation column of a window: a slightly raised surface with a hairline on its right edge.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `content-width` | length | `240px` |  |

### `ListRow`

A list row: icon, title with an optional second line, a trailing value and an optional chevron.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `icon` | string |  |  |
| `title` | string |  |  |
| `subtitle` | string |  |  |
| `trailing` | string |  |  |
| `chevron` | bool | `false` |  |
| `selected` | bool | `false` |  |
| `enabled` | bool | `true` |  |
| `icon-tint` | color | `Theme.accent` |  |
| `clicked()` | callback |  |  |

### `Toolbar`

A glass toolbar strip along the top of a window or a pane. Children are laid out left to right.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `content-padding` | length | `12px` |  |
| `tint` | color |  |  |
| `glyph` | string |  |  |
| `group-hover` | bool |  |  |
| `clicked()` | callback |  |  |

### `WindowFrame`

A window: rounded glass body with a title bar, three window dots and room for content.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `title` | string |  |  |
| `active` | bool | `true` |  |
| `close()` | callback |  |  |
| `minimize()` | callback |  |  |
| `maximize()` | callback |  |  |

### `MenuItem`

One row of a Menu. `destructive` rows turn rose; `checked` shows a check; `shortcut` is right-aligned.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `text` | string |  |  |
| `icon` | string |  |  |
| `shortcut` | string |  |  |
| `checked` | bool | `false` |  |
| `destructive` | bool | `false` |  |
| `enabled` | bool | `true` |  |
| `submenu` | bool | `false` |  |
| `clicked()` | callback |  |  |

### `MenuSeparator`


### `MenuHeading`

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `text` | string |  |  |

### `Menu`

The card that holds menu rows (use under a popup or at the pointer).


### `Scrim`

A full-window dimmer that sits behind a Dialog. Clicking it asks to dismiss.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `dismissed()` | callback |  |  |

### `Dialog`

An alert/confirm dialog. For a destructive choice set `destructive` (the main button turns rose and the safe button is the default). Buttons: `confirm-text` and an optional `cancel-text`.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `title` | string |  |  |
| `message` | string |  |  |
| `icon` | string | `"info"` |  |
| `tone` | Tone | `Tone.accent` |  |
| `confirm-text` | string | `"OK"` |  |
| `cancel-text` | string |  |  |
| `destructive` | bool | `false` |  |
| `confirmed()` | callback |  |  |
| `cancelled()` | callback |  |  |

### `Tooltip`

A small label that appears over something on hover. Place it yourself; it only draws.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `text` | string |  |  |

### `Toast`

A transient message with an optional action ("Undo").

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `text` | string |  |  |
| `icon` | string | `"check-circle"` |  |
| `tone` | Tone | `Tone.success` |  |
| `action` | string |  |  |
| `action-clicked()` | callback |  |  |
| `dismissed()` | callback |  |  |

### `Banner`

A full-width notice inside a page, for something that needs attention without blocking.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `title` | string |  |  |
| `text` | string |  |  |
| `icon` | string | `"alert-triangle"` |  |
| `tone` | Tone | `Tone.warning` |  |
| `closable` | bool | `true` |  |
| `closed()` | callback |  |  |

### `Chip`

A compact pill for filters and tags. `selected` fills it; `removable` adds an x.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `text` | string |  |  |
| `icon` | string |  |  |
| `selected` | bool | `false` |  |
| `removable` | bool | `false` |  |
| `clicked()` | callback |  |  |
| `removed()` | callback |  |  |

### `Avatar`

A person or thing: a photo, or initials on an accent tint, with an optional presence dot.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `initials` | string |  |  |
| `source` | image |  |  |
| `size` | length | `40px` |  |
| `online` | bool | `false` |  |

### `Stepper`

Progress through a wizard: numbered dots joined by a line; done steps show a check.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `steps` | [string] |  |  |
| `current` | int | `0` |  |

### `AreaChart`

A filled line chart. `values` run 0..1 (bottom..top, up to 64 points); `reference` is an optional dashed second series.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `values` | [float] |  |  |
| `reference` | [float] |  |  |
| `tint` | color | `Theme.accent` |  |
| `grid` | bool | `true` |  |
| `fill-area` | bool | `true` |  |

### `RingGauge`

A circular gauge: `value` runs 0..1 clockwise from the top; the centre shows `label` and `caption`.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `value` | float | `0` |  |
| `tint` | color | `Theme.accent` |  |
| `thickness` | length | `9px` |  |
| `label` | string |  |  |
| `caption` | string |  |  |

### `StackedBar`

A bar split into proportional segments (memory in use / cached / free, storage by kind...).

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `segments` | [Segment] |  |  |
| `thickness` | length | `10px` |  |

### `WidgetCard`

The standard widget plate: tinted icon, title, an optional trailing badge, then your content.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `title` | string |  |  |
| `icon` | string |  |  |
| `tint` | color | `Theme.accent` |  |
| `badge` | string |  |  |
| `badge-tone` | Tone | `Tone.neutral` |  |

### `StatTile`

A headline number with a sparkline: CPU 31 %, Network 143 Mb/s...

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `title` | string |  |  |
| `icon` | string |  |  |
| `value` | string |  |  |
| `unit` | string |  |  |
| `caption` | string |  |  |
| `tint` | color | `Theme.accent` |  |
| `history` | [float] |  |  |

### `MetricRow`

A line in a list of metrics: icon, name, value and a thin bar ("Top processes").

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `icon` | string |  |  |
| `name` | string |  |  |
| `value` | string |  |  |
| `fraction` | float | `0` |  |
| `tint` | color | `Theme.accent` |  |

### `Dock`

The dock: a floating glass shelf of app tiles. Icons magnify toward the pointer (a cosine of the distance, up to 1.65x within 130 px) and push their neighbours aside; the label floats above. Everything follows `icon-size` (32, 48, 64 or 80).

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `items` | [DockItem] |  |  |
| `icon-size` | length | `52px` |  |
| `magnify` | bool | `true` |  |
| `preview-x` | length | `-1px` | Documentation/testing aid: a pointer position (from the dock's left edge) to draw the magnified state without a mouse; -1px = off. |
| `activated(int)` | callback |  |  |

### `FocusRing`

The Malva focus indicator: a 2 px accent ring with a 2 px surface halo inside it, so it reads on any background. Put it as the LAST child of the control and give it the control's size. Controls show it for keyboard focus only (see `kb` in each control).

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `active` | bool | `false` |  |
| `corner` | length | `Theme.radius-control` |  |

### `Icon`

A one-colour icon, tinted with `tint` (default: the theme's ink). `name` is any icon of the `Icons` global.

| Property / callback | Type | Default | Notes |
|---|---|---|---|
| `name` | string | `"info"` |  |
| `tint` | color | `Theme.ink` |  |
| `size` | length | `20px` |  |
