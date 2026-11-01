#!/usr/bin/env python3
"""Draws vector previews of the kit's screens as SVG, from the tokens (dist/resolved.json), the kit's own icons and
wallpapers, so they stay in step with the design and scale without loss. They are drawings, not captures of the running
UI (Slint cannot export a window to SVG); the PNG/JPG screenshots in docs/screenshots are the captures.

    python3 tools/make_svg_screens.py          write
    python3 tools/make_svg_screens.py --check  exit 1 if anything on disk differs

Writes docs/svg/
    desktop-{light,dark}.svg      menu bar, a Files window, the dock, on the wallpaper (1440 x 900)
    files-{light,dark}.svg        the Files window on the wallpaper (1100 x 700)
    dock-{light,dark}.svg         the dock with one icon magnified and its label (1040 x 220)
    menubar-{light,dark}.svg      the menu bar with the Control Center open (1440 x 560)

Text is set in Inter; a viewer without Inter falls back to the system UI font."""
import json, os, re, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
CHECK = "--check" in sys.argv
STALE = []
RES = json.load(open(os.path.join(ROOT, "dist", "resolved.json")))
TOK = json.load(open(os.path.join(ROOT, "tokens", "malva-tokens.json")))
R, EL = TOK["radius"], TOK["elevation"]
FONT = "Inter, 'Segoe UI', system-ui, sans-serif"


def out(path, text):
    full = os.path.join(ROOT, path)
    if CHECK:
        if not os.path.exists(full) or open(full).read() != text:
            STALE.append(path)
        return
    os.makedirs(os.path.dirname(full), exist_ok=True)
    open(full, "w").write(text)


def read(rel):
    return open(os.path.join(ROOT, rel)).read().strip()


def num(x):
    s = f"{x:.2f}".rstrip("0").rstrip(".")
    return "0" if s in ("-0", "") else s


class Scene:
    def __init__(self, mode, w, h):
        self.mode, self.w, self.h = mode, w, h
        self.c = RES[mode]
        self.parts, self.n = [], 0

    # ---- colours: "#RRGGBBAA" becomes a fill and an opacity
    def paint(self, name, opacity=None):
        v = self.c.get(name, name)
        a = int(v[7:9], 16) / 255 if len(v) == 9 else 1.0
        if opacity is not None:
            a *= opacity
        s = f' fill="{v[:7]}"'
        return s + (f' fill-opacity="{num(a)}"' if a < 0.999 else "")

    def stroke(self, name, w=1, opacity=None):
        v = self.c.get(name, name)
        a = int(v[7:9], 16) / 255 if len(v) == 9 else 1.0
        if opacity is not None:
            a *= opacity
        s = f' stroke="{v[:7]}" stroke-width="{num(w)}"'
        return s + (f' stroke-opacity="{num(a)}"' if a < 0.999 else "")

    def add(self, s):
        self.parts.append(s)

    def rect(self, x, y, w, h, r=0, fill="none", stroke=None, sw=1, op=None, sop=None, extra=""):
        f = self.paint(fill, op) if fill != "none" else ' fill="none"'
        st = self.stroke(stroke, sw, sop) if stroke else ""
        self.add(f'<rect x="{num(x)}" y="{num(y)}" width="{num(w)}" height="{num(h)}" rx="{num(r)}"{f}{st}{extra}/>')

    def text(self, x, y, s, size=13, weight=400, fill="text", anchor="start", op=None):
        s = s.replace("&", "&amp;").replace("<", "&lt;")
        a = f' text-anchor="{anchor}"' if anchor != "start" else ""
        self.add(f'<text x="{num(x)}" y="{num(y)}" font-size="{size}" font-weight="{weight}"{self.paint(fill, op)}{a}>{s}</text>')

    def line(self, x1, y1, x2, y2, name="border", w=1, op=None):
        self.add(f'<line x1="{num(x1)}" y1="{num(y1)}" x2="{num(x2)}" y2="{num(y2)}"{self.stroke(name, w, op)}/>')

    def circle(self, cx, cy, r, fill, op=None):
        self.add(f'<circle cx="{num(cx)}" cy="{num(cy)}" r="{num(r)}"{self.paint(fill, op)}/>')

    def shadow(self, level):
        e = EL[str(level)]
        a = e["alpha-dark" if self.mode == "dark" else "alpha-light"]
        return (f'<filter id="e{level}" x="-40%" y="-40%" width="180%" height="190%"><feDropShadow dx="0" dy="{e["y"]}" '
                f'stdDeviation="{num(e["blur"] / 2)}" flood-color="{self.c["shadow"][:7]}" flood-opacity="{a}"/></filter>')

    # ---- embedded artwork: ids are made unique so many icons can share one document
    def embed(self, svg, x, y, w, h, color=None, extra=""):
        self.n += 1
        tag = f"i{self.n}"
        m = re.match(r"\s*<svg([^>]*)>(.*)</svg>\s*$", svg, re.S)
        attrs, body = m.group(1), m.group(2)
        attrs = re.sub(r'\s+(xmlns(:\w+)?|width|height|role|aria-label|x|y)="[^"]*"', "", attrs)
        body = re.sub(r'\bid="([^"]+)"', lambda k: f'id="{tag}{k.group(1)}"', body)
        body = re.sub(r'url\(#([^)]+)\)', lambda k: f'url(#{tag}{k.group(1)})', body)
        body = re.sub(r'(xlink:)?href="#([^"]+)"', lambda k: f'{k.group(1) or ""}href="#{tag}{k.group(2)}"', body)
        col = f' color="{self.c.get(color, color)[:7]}"' if color else ""
        self.add(f'<svg x="{num(x)}" y="{num(y)}" width="{num(w)}" height="{num(h)}"{attrs}{col}{extra}>{body}</svg>')

    def symbol(self, name, x, y, size=16, color="text"):
        self.embed(read(f"svg/icons/{name}.svg"), x, y, size, size, color)

    def app(self, name, x, y, size):
        self.embed(read(f"svg/app/{name}.svg"), x, y, size, size)

    def wallpaper(self, x, y, w, h):
        self.embed(read(f"svg/wallpaper/malva-{self.mode}.svg"), x, y, w, h, extra=' preserveAspectRatio="xMidYMid slice"')

    def done(self):
        defs = "".join(self.shadow(i) for i in (1, 2, 3))
        return (f'<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="{self.w}" height="{self.h}" '
                f'viewBox="0 0 {self.w} {self.h}" font-family="{FONT}">\n<defs>{defs}<clipPath id="all"><rect width="{self.w}" height="{self.h}" rx="0"/></clipPath></defs>\n'
                + "\n".join(self.parts) + "\n</svg>\n")


# ------------------------------------------------------------------------------------------------ pieces
def menu_bar(s, w, width_of_items=None):
    dark = s.mode == "dark"
    s.rect(0, 0, w, 30, 0, "panel" if dark else "surface", op=0.84 if dark else 0.72)
    s.line(0, 29.5, w, 29.5, "shadow", 1, 0.5 if dark else 0.14)
    s.symbol("malva", 12, 7, 16, "text")
    x = 40
    for i, t in enumerate(["Files", "File", "Edit", "View", "Go", "Window", "Help"]):
        s.text(x, 20, t, 13, 700 if i == 0 else 400)
        x += 6.9 * len(t) + (3 if i == 0 else 0) + 20
    # status items, right to left
    s.text(w - 14, 20, "Wed 28 Oct  5:01 AM", 13, 400, anchor="end")
    x = w - 14 - 142
    for name in ("bell", "sliders", "battery", "volume-2", "wifi", "bluetooth"):
        x -= 24
        s.symbol(name, x, 7, 16, "text")


def files_window(s, x, y, w, h, selected=1):
    s.add(f'<g filter="url(#e3)">')
    s.rect(x, y, w, h, R["l"], "surface", "border")
    s.add("</g>")
    sb = 190
    # sidebar: a panel on the left, squared on its inner side
    s.add(f'<clipPath id="win{int(x)}{int(y)}"><rect x="{num(x)}" y="{num(y)}" width="{num(w)}" height="{num(h)}" rx="{R["l"]}"/></clipPath>')
    s.add(f'<g clip-path="url(#win{int(x)}{int(y)})">')
    s.rect(x, y, sb, h, 0, "panel")
    s.line(x + sb + 0.5, y, x + sb + 0.5, y + h, "border")
    s.add("</g>")
    for i, c in enumerate(("win-close", "win-min", "win-max")):
        s.circle(x + 20 + i * 20, y + 22, 6, c)
    s.text(x + 18, y + 62, "Favorites", 11, 600, "muted")
    items = [("home", "Home"), ("desktop", "Desktop"), ("file-text", "Documents"), ("download", "Downloads"),
             ("music", "Music"), ("image", "Pictures"), ("video", "Videos"), ("trash", "Trash")]
    for i, (ic, label) in enumerate(items):
        iy = y + 74 + i * 36
        sel = i == 5
        if sel:
            s.rect(x + 8, iy, sb - 16, 32, R["s"], "accent-wash")
            s.rect(x + 8, iy + 8, 3, 16, 1.5, "accent")
        s.symbol(ic, x + 20, iy + 8, 16, "accent" if sel else "text-2")
        s.text(x + 46, iy + 21, label, 13, 600 if sel else 400)
    s.text(x + 18, y + 74 + 8 * 36 + 24, "Devices", 11, 600, "muted")
    s.symbol("drive", x + 20, y + 74 + 8 * 36 + 38, 16, "text-2")
    s.text(x + 46, y + 74 + 8 * 36 + 51, "Backup Drive", 13)
    # toolbar
    cx = x + sb
    s.symbol("chevron-left", cx + 18, y + 14, 16, "text")
    s.symbol("chevron-right", cx + 44, y + 14, 16, "muted")
    s.text(cx + 76, y + 27, "Home  ›  Pictures  ›", 13, 400, "text-2")
    s.text(cx + 76 + 146, y + 27, "Trip 2026", 13, 700)
    s.symbol("search", x + w - 92, y + 14, 16, "text")
    s.rect(x + w - 62, y + 10, 28, 24, R["xs"] + 2, "accent-wash")
    s.symbol("grid", x + w - 58, y + 14, 16, "accent")
    s.symbol("list", x + w - 28, y + 14, 16, "text-2")
    s.line(cx, y + 44.5, x + w, y + 44.5, "border")
    # the grid of items
    gw = (w - sb - 40) / 4
    names = ["Trip 2026", "Screenshots", "Edits", "RAW", "Beach.jpg", "Mountains.jpg", "City lights.png", "Forest.jpg",
             "Itinerary.pdf", "Notes.txt", "Backup.zip", "Voice memo.m4a"]
    for i, n in enumerate(names):
        col, row = i % 4, i // 4
        ix = cx + 20 + col * gw
        iy = y + 64 + row * 118
        sel = i in (5, 6)
        if sel:
            s.rect(ix + 6, iy - 6, gw - 12, 106, R["m"], "accent-wash", "accent", 1, sop=0.55)
        if i < 4:
            s.embed(read("apps/files/ui/icons/folder.svg"), ix + gw / 2 - 28, iy, 56, 56)
        elif i < 8:
            hue = [("#FF9A6B", "#5B4BA8"), ("#7BD39A", "#1F6B4A"), ("#5CA8F5", "#1E3A8A"), ("#D9A6FF", "#4B2B7A")][i - 4]
            gid = f"ph{int(x)}{int(y)}{i}"
            s.add(f'<linearGradient id="{gid}" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="{hue[0]}"/><stop offset="1" stop-color="{hue[1]}"/></linearGradient>')
            s.add(f'<rect x="{num(ix + gw / 2 - 36)}" y="{num(iy + 6)}" width="72" height="50" rx="6" fill="url(#{gid})"/>')
            s.add(f'<circle cx="{num(ix + gw / 2 + 14)}" cy="{num(iy + 20)}" r="5" fill="#FFFFFF" fill-opacity=".85"/>')
        else:
            s.embed(read("svg/icons/file.svg"), ix + gw / 2 - 20, iy + 6, 40, 40, "text-2")
        s.text(ix + gw / 2, iy + 78, n, 12, 600 if sel else 400, anchor="middle")
    s.rect(x + sb + (w - sb) / 2 - 62, y + h - 40, 124, 26, 13, "raised", "border")
    s.text(x + sb + (w - sb) / 2, y + h - 22, "24 items, 2 selected", 11, 400, "text-2", "middle")


def dock(s, cx, y, size=56, magnify=None, label=None):
    apps = ["launcher", "files", "browser", "terminal", "settings", "music", "photos", "mail", "calendar"]
    gap = 10
    scales = [1.0] * len(apps)
    if magnify is not None:
        for i in range(len(apps)):
            d = abs(i - magnify)
            scales[i] = 1.0 + 0.65 * max(0, 1 - d / 2.2) ** 2
    total = sum(size * k for k in scales) + gap * (len(apps) - 1)
    pad = 14
    w = total + 2 * pad
    x0 = cx - w / 2
    dark = s.mode == "dark"
    s.add('<g filter="url(#e3)">')
    s.rect(x0, y, w, size + 2 * pad, R["l"], "panel" if dark else "surface", op=0.80 if dark else 0.65)
    s.add("</g>")
    s.rect(x0 + 0.5, y + 0.5, w - 1, size + 2 * pad - 1, R["l"], "none", "shadow", 1, sop=0.5 if dark else 0.14)
    s.rect(x0 + 1.5, y + 1.5, w - 3, size + 2 * pad - 3, R["l"] - 1, "none", "#FFFFFF", 1, sop=0.7 if not dark else 0.14)
    x = x0 + pad
    base = y + pad + size
    for i, a in enumerate(apps):
        k = scales[i]
        sz = size * k
        s.app(a, x, base - sz, sz)
        if a in ("files", "browser", "terminal"):
            s.circle(x + sz / 2, y + size + 2 * pad - 5, 2.2, "text", 0.8)
        if label and i == magnify:
            lw = len(label) * 7.4 + 24
            s.add('<g filter="url(#e2)">')
            s.rect(x + sz / 2 - lw / 2, base - sz - 40, lw, 28, R["s"], "raised", "border")
            s.add("</g>")
            s.text(x + sz / 2, base - sz - 21, label, 12, 600, anchor="middle")
        x += sz + gap


def control_center(s, x, y):
    w, h = 360, 480
    s.add('<g filter="url(#e3)">')
    s.rect(x, y, w, h, R["m"] + 2, "raised", "border")
    s.add("</g>")
    tiles = [("wifi", "Wi-Fi", "Mangrove", True), ("bluetooth", "Bluetooth", "2 devices", True), ("moon", "Do Not Disturb", "Off", False),
             ("airplane", "Airplane Mode", "", False), ("dark-mode", "Dark Mode", "On" if s.mode == "dark" else "Off", s.mode == "dark"),
             ("night-light", "Night Light", "Until 7:00", False)]
    for i, (ic, t, sub, on) in enumerate(tiles):
        tx, ty = x + 12 + (i % 2) * 172, y + 12 + (i // 2) * 62
        s.rect(tx, ty, 164, 54, R["s"] + 2, "panel", "border")
        s.circle(tx + 28, ty + 27, 17, "accent" if on else "track")
        s.symbol(ic, tx + 20, ty + 19, 16, "on-accent" if on else "text")
        s.text(tx + 54, ty + (23 if sub else 32), t, 13, 600)
        if sub:
            s.text(tx + 54, ty + 40, sub, 11, 400, "text-2")
    for i, (lab, ic, v) in enumerate((("Display", "sun", 0.72), ("Sound", "volume-2", 0.46))):
        sy = y + 212 + i * 62
        s.text(x + 14, sy, lab, 12, 600, "text-2")
        s.rect(x + 12, sy + 10, w - 24, 34, 17, "track")
        s.add(f'<clipPath id="sl{i}{int(x)}"><rect x="{num(x + 12)}" y="{num(sy + 10)}" width="{num(w - 24)}" height="34" rx="17"/></clipPath>')
        s.add(f'<g clip-path="url(#sl{i}{int(x)})">')
        s.rect(x + 12, sy + 10, (w - 24) * v, 34, 0, "accent")
        s.add("</g>")
        s.symbol(ic, x + 24, sy + 19, 16, "on-accent")
    s.rect(x + 12, y + 346, w - 24, 62, R["s"] + 2, "panel", "border")
    s.rect(x + 22, y + 356, 42, 42, R["xs"] + 2, "accent-wash")
    s.symbol("headphones", x + 33, y + 367, 20, "accent")
    s.text(x + 76, y + 374, "Golden Hour", 13, 600)
    s.text(x + 76, y + 391, "Kacey Musgraves", 11, 400, "text-2")
    s.symbol("pause", x + w - 56, y + 369, 16, "text")
    s.symbol("next", x + w - 32, y + 369, 16, "text")
    for i, (ic, t) in enumerate((("screenshot", "Screenshot"), ("lock", "Lock"), ("gear", "Settings"))):
        bx = x + 12 + i * 116
        s.rect(bx, y + 420, 108, 50, R["s"] + 2, "panel", "border")
        s.symbol(ic, bx + 46, y + 428, 16, "text")
        s.text(bx + 54, y + 459, t, 11, 400, "text-2", "middle")


# ------------------------------------------------------------------------------------------------ screens
def desktop(mode):
    s = Scene(mode, 1440, 900)
    s.wallpaper(0, 0, 1440, 900)
    menu_bar(s, 1440)
    files_window(s, 200, 110, 1040 - 80, 560)
    dock(s, 720, 800, 56)
    return s.done()


def files(mode):
    s = Scene(mode, 1100, 700)
    s.wallpaper(0, 0, 1100, 700)
    files_window(s, 70, 50, 960, 600)
    return s.done()


def dock_only(mode):
    s = Scene(mode, 1040, 220)
    s.wallpaper(0, 0, 1040, 220)
    dock(s, 520, 92, 56, magnify=2, label="Browser")
    return s.done()


def menubar(mode):
    s = Scene(mode, 1440, 560)
    s.wallpaper(0, 0, 1440, 560)
    menu_bar(s, 1440)
    control_center(s, 1440 - 360 - 12, 40)
    return s.done()


for m in ("light", "dark"):
    out(f"docs/svg/desktop-{m}.svg", desktop(m))
    out(f"docs/svg/files-{m}.svg", files(m))
    out(f"docs/svg/dock-{m}.svg", dock_only(m))
    out(f"docs/svg/menubar-{m}.svg", menubar(m))

if CHECK:
    if STALE:
        print("make_svg_screens: out of date:", ", ".join(STALE))
        sys.exit(1)
    print("make_svg_screens: all outputs up to date")
else:
    print("svg screens written to docs/svg")
