# Icons

Every file here comes from the Malva icon theme (`Malva`). The names are the ones the Rust side and the
`.slint` files already use (`SymbolIcon { name: "home" }`, `KindIcon { kind: "pdf" }`), so swapping an
icon means replacing the file of the same name and nothing else.

There are two kinds:

* **Tinted symbols** (25) are single-colour glyphs on the theme's 24 grid, stroke 2. The app paints them
  with `colorize`, so only the shape matters. They are the theme's `symbolic` glyphs with two mechanical
  changes: `currentColor` became `#000`, and the fixed `width`/`height` on the root element were dropped
  (the `viewBox` stays), which is how the earlier files in this folder were written.
* **Kind icons** (8) are the theme's full-colour file icons, copied unchanged. Their symbol is drawn in
  `currentColor`, with the colour set by a `color` attribute on a parent `<g>`, so they must not be
  tinted by the app and do not need to be.

## Tinted symbols

| File | Theme glyph (`symbolic/…`) |
|---|---|
| `home` | `actions/go-home` |
| `desktop` | `devices/video-display` |
| `documents` | `actions/document-properties` |
| `downloads` | `actions/download` |
| `music` | `devices/audio-headphones` |
| `pictures` | `actions/insert-image` |
| `videos` | `devices/camera-video` |
| `trash` | `actions/edit-delete` |
| `drive` | `devices/drive-harddisk` |
| `usb` | `devices/drive-removable-media-usb` |
| `network` | `places/network-workgroup` |
| `recent` | `actions/document-open-recent` |
| `eject` | `actions/media-eject` |
| `back`, `forward` | `actions/go-previous`, `actions/go-next` |
| `search` | `actions/edit-find` |
| `view-grid`, `view-list` | `actions/view-grid`, `actions/view-list` |
| `chevron-right`, `chevron-up`, `chevron-down` | `ui/pan-end`, `ui/pan-up`, `ui/pan-down` |
| `close`, `minimize`, `maximize` | `ui/window-close`, `ui/window-minimize`, `ui/window-maximize` |
| `clear` | `actions/edit-clear` (the theme's clear is the backspace shape) |

The sidebar places (Desktop, Documents, Downloads, Music, Pictures, Videos) use outline glyphs on purpose.
The theme's own `folder-documents`, `folder-music` and so on are solid folders with a small cut-out, which
at 20 px turn into six near-identical dark blobs, and they would mix solid and outline styles in one list.

## Kind icons

| File | Theme icon (`scalable/…`) |
|---|---|
| `folder` | `places/folder` |
| `image` | `mimetypes/image-x-generic` |
| `pdf` | `mimetypes/application-pdf` |
| `text` | `mimetypes/text-x-generic` |
| `archive` | `mimetypes/package-x-generic` |
| `audio` | `mimetypes/audio-x-generic` |
| `video` | `mimetypes/video-x-generic` |
| `file` | `mimetypes/application-x-generic` |

The theme has more file icons than the app has kinds (document, spreadsheet, presentation, font, script,
executable, disk image, and so on). Using them needs more kinds in `src/format/mime.rs`, which is a
Rust change and was left out of this pass.

These are the same files as in the standalone `malva-files` repository. The kit-wide symbols and application pictures
(`svg/icons`, `svg/app`, `ui/icons`) are handled by `tools/apply_malva_theme.py`; see `svg/malva-theme/README.md`.
