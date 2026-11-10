#!/usr/bin/env python3
"""Puts the Malva icon theme into the kit's icon packs.

    python3 tools/apply_malva_theme.py            write every pack
    python3 tools/apply_malva_theme.py --check    exit 1 if a file differs from what this tool would write

The theme's artwork is vendored in svg/malva-theme (see its README.md). This tool copies it into the places
the kit reads icons from:

    symbols   svg/icons, ui/icons, apps/{menubar,dock}/ui/icons/symbols, shell/menubar/ui/icons/symbols
              A kit symbol that has a theme glyph (SYMBOLS below) is replaced by it: the glyph's shapes are copied
              unchanged; only the root tag is adapted to the pack (no width/height; `currentColor` kept in
              svg/icons and written as #000 in the packs that Slint tints with `colorize`).
              A symbol the theme has no good equivalent for keeps the kit's own drawing; its root stroke width is
              set to the theme's 2 (from 1.8 or 1.75) so the whole pack has one weight.
    apps      svg/app, apps/{dock,menubar}/ui/icons/app, shell/menubar/ui/icons/app
              The kit's application pictures that the theme covers (APPS below) are replaced by the theme's tiles;
              the weather pictures and the dock's trash parts are not tiles and are left alone.

The older generators (make_icons.py, make-icons.py, make-symbols.py) still draw the symbols the theme does not
have, and call this tool when they finish, so running one of them does not undo the theme.
"""
import os, re, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
THEME = os.path.join(ROOT, "svg", "malva-theme")
STROKE = "2"  # the theme's symbol weight

# kit symbol name -> theme glyph (symbolic/<this>-symbolic.svg). A name that is not here keeps the kit's own drawing
# (svg/malva-theme/README.md lists them and says why).
SYMBOLS = {
    "add": "actions/list-add",
    "airplane": "status/airplane-mode",
    "alert": "status/dialog-warning",
    "alert-circle": "status/dialog-error",
    "alert-triangle": "status/dialog-warning",
    "arrow-down": "actions/arrow-down",
    "arrow-left": "actions/arrow-left",
    "arrow-right": "actions/arrow-right",
    "arrow-up": "actions/arrow-up",
    "battery": "status/battery-level-100",
    "bell": "status/notification",
    "bell-off": "status/notifications-disabled",
    "bluetooth": "status/bluetooth-active",
    "bookmark": "actions/bookmark-new",
    "calculator": "apps/accessories-calculator",
    "camera": "devices/camera-photo",
    "chevron-down": "ui/pan-down",
    "chevron-left": "ui/pan-start",
    "chevron-right": "ui/pan-end",
    "chevron-up": "ui/pan-up",
    "clipboard": "actions/edit-paste",
    "close": "ui/window-close",
    "copy": "actions/edit-copy",
    "dark-mode": "apps/preferences-desktop-appearance",
    "desktop": "devices/video-display",
    "download": "actions/download",
    "edit": "actions/document-edit",
    "eject": "actions/media-eject",
    "external": "actions/external-link",
    "eye": "actions/view-reveal",
    "eye-off": "actions/view-conceal",
    "filter": "actions/filter",
    "fullscreen": "actions/view-fullscreen",
    "gear": "categories/preferences-system",
    "grid": "actions/view-grid",
    "grip": "ui/list-drag-handle",
    "headphones": "devices/audio-headphones",
    "help": "status/dialog-question",
    "home": "actions/go-home",
    "image": "actions/insert-image",
    "info": "status/dialog-information",
    "keyboard": "devices/input-keyboard",
    "keyboard-small": "devices/input-keyboard",
    "link": "actions/insert-link",
    "list": "actions/view-list-bullet",
    "location": "actions/mark-location",
    "lock": "status/system-lock-screen",
    "logout": "status/system-log-out",
    "maximize": "ui/window-maximize",
    "menu": "actions/open-menu",
    "mic": "devices/audio-input-microphone",
    "minimize": "ui/window-minimize",
    "minus": "actions/list-remove",
    "moon": "status/weather-clear-night",
    "more": "actions/view-more-horizontal",
    "more-vertical": "actions/view-more",
    "mouse": "devices/input-mouse",
    "network": "places/network-workgroup",
    "new-window": "ui/window-new",
    "next": "actions/media-skip-forward",
    "night-light": "status/night-light",
    "palette": "apps/preferences-color",
    "pause": "actions/media-playback-pause",
    "phone": "devices/phone",
    "pin": "actions/view-pin",
    "play": "actions/media-playback-start",
    "plug": "devices/ac-adapter",
    "plus": "actions/list-add",
    "power": "status/system-shutdown",
    "previous": "actions/media-skip-backward",
    "printer": "devices/printer",
    "record": "actions/media-record",
    "redo": "actions/edit-redo",
    "refresh": "actions/view-refresh",
    "restart": "status/system-reboot",
    "save": "actions/document-save",
    "screenshot": "apps/screenshot",
    "search": "actions/edit-find",
    "share": "actions/send-to",
    "shield": "status/security-high",
    "sidebar": "actions/sidebar-show",
    "signal": "status/network-cellular-signal-excellent",
    "sliders": "actions/sliders",
    "sort": "ui/chevrons-up-down",
    "speaker": "devices/audio-speakers",
    "star": "status/non-starred",
    "star-fill": "status/starred",
    "sun": "status/display-brightness",
    "tag": "actions/tag",
    "trash": "actions/edit-delete",
    "undo": "actions/edit-undo",
    "upload": "actions/upload",
    "user": "status/avatar-default",
    "users": "apps/system-users",
    "video": "devices/camera-video",
    "volume": "status/audio-volume-high",
    "volume-1": "status/audio-volume-low",
    "volume-2": "status/audio-volume-medium",
    "volume-mute": "status/audio-volume-muted",
    "wifi": "status/network-wireless-signal-excellent",
    "wifi-1": "status/network-wireless-signal-weak",
    "wifi-2": "status/network-wireless-signal-ok",
    "wifi-3": "status/network-wireless-signal-good",
    "wifi-off": "status/network-wireless-disabled",
    "wind": "status/weather-windy",
    "windows": "ui/window-restore",
    "zoom-in": "actions/zoom-in",
    "zoom-out": "actions/zoom-out",
}

# kit application picture name -> file in svg/malva-theme
APPS = {
    "browser": "scalable/categories/applications-internet.svg",
    "calculator": "generated/calculator.svg",
    "calendar": "generated/calendar.svg",
    "default": "scalable/categories/applications-other.svg",
    "editor": "scalable/apps/text-editor.svg",
    "files": "scalable/apps/files.svg",
    "launcher": "scalable/apps/launcher.svg",
    "mail": "generated/mail.svg",
    "music": "scalable/apps/music.svg",
    "notes": "generated/notes.svg",
    "photos": "scalable/apps/photos.svg",
    "settings": "scalable/apps/settings.svg",
    "store": "scalable/apps/app-store.svg",
    "terminal": "scalable/apps/terminal.svg",
    "video": "scalable/apps/video.svg",
}

# pack id -> (directory, kind, colour written for `currentColor`)
PACKS = {
    "svg-icons": ("svg/icons", "symbols", "currentColor"),
    "ui-icons": ("ui/icons", "symbols", "#000"),
    "menubar-symbols": ("apps/menubar/ui/icons/symbols", "symbols", "#000"),
    "shell-menubar-symbols": ("shell/menubar/ui/icons/symbols", "symbols", "#000"),
    "dock-symbols": ("apps/dock/ui/icons/symbols", "symbols", "#000"),
    "svg-app": ("svg/app", "apps", None),
    "dock-app": ("apps/dock/ui/icons/app", "apps", None),
    "menubar-app": ("apps/menubar/ui/icons/app", "apps", None),
    "shell-menubar-app": ("shell/menubar/ui/icons/app", "apps", None),
}

_ROOT_TAG = re.compile(r"<svg\b[^>]*>")


def _read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def theme_symbol(name, colour):
    """The theme glyph for kit symbol `name`, adapted to a pack: no width/height on the root, `colour` for currentColor."""
    s = _read(os.path.join(THEME, "symbolic", SYMBOLS[name] + "-symbolic.svg")).strip()
    root = _ROOT_TAG.search(s).group(0)
    s = s.replace(root, re.sub(r'\s(width|height)="[^"]*"', "", root), 1)
    return s.replace("currentColor", colour) + "\n"


def reweight(svg):
    """Set the root stroke width to the theme's. Only the usual 1.8 and 1.75 are changed: the dot-only symbols
    (more-vertical, grip) use a thick stroke on purpose, because their strokes are zero-length."""
    m = _ROOT_TAG.search(svg)
    root = re.sub(r'stroke-width="(1\.8|1\.75)"', f'stroke-width="{STROKE}"', m.group(0), count=1)
    return svg[:m.start()] + root + svg[m.end():]


def theme_app(name):
    return _read(os.path.join(THEME, APPS[name])).strip() + "\n"


def expected(pack, name, current):
    """What file `name` of `pack` should contain, given what it contains now."""
    _, kind, colour = PACKS[pack]
    if kind == "apps":
        return theme_app(name) if name in APPS else current
    return theme_symbol(name, colour) if name in SYMBOLS else reweight(current)


def apply(packs=None, check=False, quiet=False):
    """Write (or, with check=True, compare) the named packs; returns the list of files that differ / were changed."""
    changed = []
    for pack in packs or PACKS:
        folder = PACKS[pack][0]
        d = os.path.join(ROOT, folder)
        if not os.path.isdir(d):
            continue
        n = 0
        for f in sorted(os.listdir(d)):
            if not f.endswith(".svg"):
                continue
            path = os.path.join(d, f)
            cur = _read(path)
            new = expected(pack, f[:-4], cur)
            if new != cur:
                changed.append(os.path.relpath(path, ROOT))
                n += 1
                if not check:
                    with open(path, "w", encoding="utf-8") as o:
                        o.write(new)
        if not quiet:
            print(f"{folder}: {n} file(s) {'differ' if check else 'written'}")
    return changed


if __name__ == "__main__":
    names = [a for a in sys.argv[1:] if not a.startswith("--")]
    bad = apply(names or None, check="--check" in sys.argv)
    if "--check" in sys.argv and bad:
        print("out of date (run python3 tools/apply_malva_theme.py): " + ", ".join(bad))
        sys.exit(1)
