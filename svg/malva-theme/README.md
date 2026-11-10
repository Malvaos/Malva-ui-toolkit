# Malva icon theme (vendored)

The artwork of the Malva icon theme (`Malva`) that the kit's icons come from. `tools/apply_malva_theme.py` copies it
into `svg/icons`, `svg/app`, `ui/icons` and the icon folders of `apps/dock`, `apps/menubar` and `shell/menubar`
(`python3 tools/apply_malva_theme.py --check` is part of CI). Only the files the kit uses are here, under the theme's own
paths:

| Folder | What | Used for |
|---|---|---|
| `symbolic/<context>/<name>-symbolic.svg` | 105 one-colour glyphs, 24 grid, stroke 2, `currentColor` | the symbol packs |
| `scalable/apps`, `scalable/categories` | 11 coloured tiles, 1024 x 1024 | the application pictures |
| `generated/` | 4 coloured tiles made with the theme's own tools (below) | calculator, calendar, mail, notes |

## Symbols

The tool replaces a kit symbol with its theme glyph (the table is `SYMBOLS` in `tools/apply_malva_theme.py`; 108 names
map to the 105 glyphs). The glyph's shapes are copied unchanged. Only the root tag is adapted to the pack: `width` and
`height` are dropped, and `currentColor` is written as `#000` in the packs that Slint tints with `colorize`
(`ui/icons`, the menu bar's and the dock's `symbols`); `svg/icons` keeps `currentColor`.

50 names keep the kit's own drawing, and every symbol file is set to the theme's stroke 2 (it was 1.8, and 1.75 in
`ui/icons`) so a pack has one weight. The dot-only symbols (`more-vertical`, `grip`) keep their thick stroke, which is
the dot size. The kept names:

* The theme draws these as solid shapes, which would mix filled and outline styles in one pack: `archive`, `calendar`,
  `cloud`, `code`, `drive`, `file`, `folder`, `music`, `server`, `terminal`, `usb`.
* The theme has no suitable equivalent: no glyph with that meaning, or its nearest one is a different symbol (for
  `mail` the theme has only an open envelope and a badged one, for `chat` only a "new message" bubble): `activity`,
  `chat`, `check`, `check-circle`, `clock`, `command`, `cpu`, `database`, `drop`, `empty`, `error`, `file-text`,
  `folder-open`, `globe`, `hard-drive`, `heart`, `hide`, `key`, `leaf`, `lightning`, `logo`, `mail`, `malva`,
  `memory`, `notes`, `options`, `plus-circle`, `quit`, `rocket`, `settings`, `sparkle`, `success`, `thermometer`,
  `unlock`, `unpin`, `volume-0`, `window`, `x-circle`, `zap`. `logo` and `malva` are the brand mark.

## Application pictures

| Kit name | Theme file |
|---|---|
| `browser` | `scalable/categories/applications-internet` (the theme has no browser tile; the Internet tile stands in) |
| `default` | `scalable/categories/applications-other` |
| `editor` | `scalable/apps/text-editor` |
| `files`, `launcher`, `music`, `photos`, `settings`, `terminal`, `video` | `scalable/apps/<same name>` |
| `store` | `scalable/apps/app-store` |
| `calculator`, `calendar`, `mail`, `notes` | `generated/` (the theme has no tile for them) |

The weather pictures and the dock's three trash parts (`trash-body`, `trash-lid`, `trash-paper`) are not application
tiles and keep the kit's drawings.

### Generated tiles

Made with the theme's `tools/icons/appfmt.py` and `tiles.py`, so they follow the shipped recipe (squircle, two-stop
gradient, highlight, lilac symbol; the symbol sized to the theme's rule and centred by its optical rule).

| Tile | Theme glyph | Hue | Scale | Shift |
|---|---|---|---|---|
| `calculator` | `accessories-calculator` | 28 | 1.092 | (0, 0.8) |
| `calendar` | `calendar` | 355 | 1.151 | (0.0, -0.1) |
| `mail` | `mail-read` | 225 | 1.151 | (0.0, -0.6) |
| `notes` | `document-properties` | 94 | 1.151 | (0.2, -0.7) |

The tools read `CORNER_EXPONENT` from `crates/malva-tokens/src/balance.rs` of the main Malva repository, which was not at
hand. 4.0 was used because regenerating the theme's `preferences-desktop-keyboard` tile with it reproduces the shipped
file byte for byte (3.0, 3.5, 4.5 and 5.0 do not). If the theme gains tiles for these four, copy them over `generated/`
and point `APPS` at the new files.
