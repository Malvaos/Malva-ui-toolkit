#!/usr/bin/env python3
"""Draws the kit's SVG parts and component specimens from the tokens (dist/resolved.json and
tokens/malva-tokens.json), so a designer's file and the code never disagree about a colour or a size.

    python3 tools/make_svg_parts.py          write
    python3 tools/make_svg_parts.py --check  exit 1 if anything on disk differs

Writes
    svg/parts/squircle.svg          the continuous-corner shape (a superellipse, n = 5) that app icons are cut from
    svg/parts/focus-ring.svg        the focus ring, light and dark: 2 px, 2 px off the control
    svg/parts/spinner.svg           an indeterminate spinner (SMIL, one turn in 900 ms; currentColor)
    svg/parts/skeleton.svg          a loading placeholder with a moving sheen (SMIL)
    svg/parts/elevation.svg         the three shadow levels, light and dark
    svg/specimen-light.svg          every basic component on one sheet, drawn at 1x in the Malva light palette
    svg/specimen-dark.svg           the same in the dark palette

Every shape is drawn here from primitives. Nothing is traced from another design system."""
import json, math, os, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
CHECK = "--check" in sys.argv
STALE = []

RES = json.load(open(os.path.join(ROOT, "dist", "resolved.json")))
TOK = json.load(open(os.path.join(ROOT, "tokens", "malva-tokens.json")))
R, S, Z, EL = TOK["radius"], TOK["space"], TOK["size"], TOK["elevation"]
FONT = "Inter, 'Segoe UI', system-ui, sans-serif"
MONO = "'JetBrains Mono', ui-monospace, monospace"


def out(path, text):
    full = os.path.join(ROOT, path)
    if CHECK:
        if not os.path.exists(full) or open(full).read() != text:
            STALE.append(path)
        return
    os.makedirs(os.path.dirname(full), exist_ok=True)
    open(full, "w").write(text)


def f(x):
    """A number as short as it can be written."""
    s = f"{x:.2f}".rstrip("0").rstrip(".")
    return "0" if s in ("-0", "") else s


# ------------------------------------------------------------------------------------------------ small drawing kit
class Sheet:
    def __init__(self, mode):
        self.c = RES[mode]
        self.mode = mode
        self.parts = []

    def col(self, name):
        return self.c[name]

    def add(self, s):
        self.parts.append(s)

    def rect(self, x, y, w, h, r, fill="none", stroke=None, sw=1, extra=""):
        st = f' stroke="{self.col(stroke) if stroke in self.c else stroke}" stroke-width="{f(sw)}"' if stroke else ""
        fl = self.col(fill) if fill in self.c else fill
        self.add(f'<rect x="{f(x)}" y="{f(y)}" width="{f(w)}" height="{f(h)}" rx="{f(r)}" fill="{fl}"{st}{extra}/>')

    def text(self, x, y, s, role="body", fill="text", anchor="start", family=None, opacity=None):
        t = TOK["type"]["scale"][role]
        fl = self.col(fill) if fill in self.c else fill
        fam = f' font-family="{family}"' if family else ""
        op = f' fill-opacity="{opacity}"' if opacity else ""
        a = f' text-anchor="{anchor}"' if anchor != "start" else ""
        s = s.replace("&", "&amp;").replace("<", "&lt;")
        self.add(f'<text x="{f(x)}" y="{f(y)}" font-size="{t["size"]}" font-weight="{t["weight"]}" fill="{fl}"{op}{a}{fam}>{s}</text>')

    def shadow_filter(self, level):
        e = EL[str(level)]
        a = e["alpha-dark" if self.mode == "dark" else "alpha-light"]
        return (f'<filter id="e{level}" x="-80%" y="-80%" width="260%" height="280%">'
                f'<feDropShadow dx="0" dy="{e["y"]}" stdDeviation="{f(e["blur"] / 2)}" flood-color="{self.col("shadow")}" flood-opacity="{a}"/></filter>')

    def focus_ring(self, x, y, w, h, r):
        o = Z["focus-offset"] + Z["focus-width"] / 2
        self.add(f'<rect x="{f(x - o)}" y="{f(y - o)}" width="{f(w + 2 * o)}" height="{f(h + 2 * o)}" rx="{f(r + o)}" fill="none" stroke="{self.col("focus")}" stroke-width="{Z["focus-width"]}"/>')

    def label(self, x, y, s):
        self.text(x, y, s.upper(), "caption", "muted")

    # ---- components
    def button(self, x, y, w, label, kind="primary", state="rest", h=None):
        h = h or Z["control-m"]
        r = R["s"]
        fill, ink, stroke = {
            "primary": ("accent", "on-accent", None),
            "secondary": ("surface", "text", "border"),
            "ghost": ("none", "accent", None),
            "danger": ("danger-fill", "on-danger", None),
        }[kind]
        if state == "hover":
            fill = {"primary": "accent-hover", "secondary": "secondary-hover", "ghost": "ghost-hover", "danger": "danger-hover"}[kind]
        if state == "press":
            fill = {"primary": "accent-press", "secondary": "secondary-press", "ghost": "ghost-press", "danger": "danger-press"}[kind]
        opacity = ""
        if state == "disabled":
            fill = {"primary": "accent-disabled", "secondary": "surface", "ghost": "none", "danger": "danger-disabled"}[kind]
            ink = {"primary": "on-accent", "secondary": "disabled-text", "ghost": "disabled-text", "danger": "on-danger"}[kind]
            opacity = ""
        self.rect(x, y, w, h, r, fill, stroke)
        if state == "focus":
            self.focus_ring(x, y, w, h, r)
        self.text(x + w / 2, y + h / 2 + 5, label, "label", ink, "middle")

    def toggle(self, x, y, on):
        w, h, k = Z["toggle-width"], Z["toggle-height"], Z["toggle-knob"]
        self.rect(x, y, w, h, h / 2, "accent" if on else "toggle-off")
        kx = x + (w - k - (h - k) / 2) if on else x + (h - k) / 2
        self.add(f'<circle cx="{f(kx + k / 2)}" cy="{f(y + h / 2)}" r="{f(k / 2)}" fill="#FFFFFF" filter="url(#e1)"/>')

    def checkbox(self, x, y, on):
        s = Z["check"]
        if on:
            self.rect(x, y, s, s, R["xs"], "accent")
            self.add(f'<path d="M{f(x + 7)} {f(y + 13.5)} l4 4 l8 -8.5" fill="none" stroke="{self.col("on-accent")}" stroke-width="2.4" stroke-linecap="round" stroke-linejoin="round"/>')
        else:
            self.rect(x + 1, y + 1, s - 2, s - 2, R["xs"] - 1, "field", "field-border", 2)

    def radio(self, x, y, on):
        s = Z["radio"]
        if on:
            self.add(f'<circle cx="{f(x + s / 2)}" cy="{f(y + s / 2)}" r="{f(s / 2 - 1)}" fill="none" stroke="{self.col("accent")}" stroke-width="2"/>')
            self.add(f'<circle cx="{f(x + s / 2)}" cy="{f(y + s / 2)}" r="{f(s / 5)}" fill="{self.col("accent")}"/>')
        else:
            self.add(f'<circle cx="{f(x + s / 2)}" cy="{f(y + s / 2)}" r="{f(s / 2 - 1)}" fill="{self.col("field")}" stroke="{self.col("field-border")}" stroke-width="2"/>')

    def slider(self, x, y, w, v):
        t, k = Z["slider-track"], Z["slider-thumb"]
        self.rect(x, y - t / 2, w, t, t / 2, "track")
        self.rect(x, y - t / 2, w * v, t, t / 2, "accent")
        self.add(f'<circle cx="{f(x + w * v)}" cy="{f(y)}" r="{f(k / 2)}" fill="#FFFFFF" stroke="{self.col("accent")}" stroke-width="2" filter="url(#e1)"/>')

    def progress(self, x, y, w, v):
        h = Z["progress"]
        self.rect(x, y, w, h, h / 2, "track")
        self.rect(x, y, w * v, h, h / 2, "accent")

    def field(self, x, y, w, text, state="rest", placeholder=False):
        h, r = Z["field-height"], R["s"]
        stroke = {"rest": "field-border", "focus": "accent", "error": "danger"}[state]
        self.rect(x + 0.5, y + 0.5, w - 1, h - 1, r, "field", stroke, 1.5 if state != "rest" else 1)
        if state == "focus":
            self.focus_ring(x, y, w, h, r)
        self.text(x + S["s"], y + h / 2 + 5, text, "body", "muted" if placeholder else "text")
        if state == "focus":
            tx = x + S["s"] + len(text) * 7.6 + 1
            self.add(f'<rect x="{f(tx)}" y="{f(y + 11)}" width="1.5" height="20" fill="{self.col("accent")}"/>')

    def chip(self, x, y, label, on=False):
        w = len(label) * 7.4 + 28
        self.rect(x, y, w, 30, 15, "accent-wash" if on else "panel", "accent" if on else "border")
        self.text(x + w / 2, y + 20, label, "label", "accent" if on else "text", "middle")
        return w

    def segmented(self, x, y, labels, sel, seg=84):
        h = 36
        self.rect(x, y, seg * len(labels) + 8, h + 8, R["s"] + 4, "panel")
        for i, s in enumerate(labels):
            sx = x + 4 + i * seg
            if i == sel:
                self.add(f'<rect x="{f(sx)}" y="{f(y + 4)}" width="{seg}" height="{h}" rx="{R["s"]}" fill="{self.col("raised")}" filter="url(#e1)"/>')
            self.text(sx + seg / 2, y + 4 + h / 2 + 5, s, "label", "text" if i == sel else "text-2", "middle")

    def badge(self, x, y, label, kind):
        fill, ink = {"success": ("success-soft", "success-text"), "warning": ("warning-soft", "warning-text"),
                     "danger": ("danger-soft", "danger-text"), "info": ("info-soft", "info-text")}[kind]
        w = len(label) * 7 + 20
        self.rect(x, y, w, 24, 12, fill)
        self.text(x + w / 2, y + 16, label, "caption", ink, "middle")
        return w

    def menu(self, x, y):
        w, it = Z["menu-width"], Z["menu-item"]
        rows = [("New window", "Ctrl N", 0), ("Open recent", "›", 0), None, ("Rename", "F2", 1), ("Move to Bin", "Del", 2)]
        h = 12 + sum(9 if r is None else it for r in rows)
        self.add(f'<g filter="url(#e2)">')
        self.rect(x, y, w, h, R["m"], "raised", "border")
        self.add("</g>")
        cy = y + 6
        for r in rows:
            if r is None:
                self.add(f'<rect x="{f(x + 12)}" y="{f(cy + 4)}" width="{w - 24}" height="1" fill="{self.col("line")}"/>')
                cy += 9
                continue
            label, key, kind = r
            if kind == 1:
                self.rect(x + 6, cy, w - 12, it, R["xs"] + 2, "accent-wash")
            ink = "danger-text" if kind == 2 else "text"
            self.text(x + 16, cy + it / 2 + 5, label, "body", ink)
            self.text(x + w - 16, cy + it / 2 + 4, key, "caption", "muted", "end")
            cy += it
        return h

    def toast(self, x, y):
        w, h = Z["toast-width"], 64
        self.add('<g filter="url(#e2)">')
        self.rect(x, y, w, h, R["m"], "raised", "border")
        self.add("</g>")
        self.add(f'<circle cx="{x + 32}" cy="{y + h / 2}" r="14" fill="{self.col("success-soft")}"/>')
        self.add(f'<path d="M{x + 25.5} {y + h / 2 + .5} l4.5 4.5 l8 -9" fill="none" stroke="{self.col("success")}" stroke-width="2.4" stroke-linecap="round" stroke-linejoin="round"/>')
        self.text(x + 58, y + 28, "Backup finished", "body-strong")
        self.text(x + 58, y + 46, "12 428 files · 3.1 GB · just now", "caption", "text-2")
        self.text(x + w - 16, y + 38, "Undo", "label", "accent", "end")

    def dialog(self, x, y):
        w, h = Z["dialog-width"], 196
        self.add('<g filter="url(#e3)">')
        self.rect(x, y, w, h, R["l"], "raised", "border")
        self.add("</g>")
        self.text(x + 24, y + 44, "Move 3 items to the Bin?", "heading")
        self.text(x + 24, y + 72, "You can restore them from the Bin until it is", "body", "text-2")
        self.text(x + 24, y + 92, "emptied.", "body", "text-2")
        self.button(x + w - 24 - 96 - 8 - 92, y + h - 24 - 40, 92, "Cancel", "secondary")
        self.button(x + w - 24 - 96, y + h - 24 - 40, 96, "Move", "danger")

    def done(self, w, h):
        c = self.c
        defs = "".join(self.shadow_filter(i) for i in (1, 2, 3))
        return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" font-family="{FONT}">\n'
                f'<defs>{defs}</defs>\n<rect width="{w}" height="{h}" fill="{c["bg"]}"/>\n' + "\n".join(self.parts) + "\n</svg>\n")


def specimen(mode):
    s = Sheet(mode)
    W, H = 1240, 990
    s.text(48, 70, "Malva UI", "title")
    s.text(48, 98, f"Component specimen · {mode} · drawn from tokens/malva-tokens.json", "body", "text-2")
    # a brand strip
    for i, n in enumerate(["petal", "lilac", "orchid", "plum", "leaf", "ink"]):
        s.add(f'<rect x="{900 + i * 48}" y="46" width="40" height="40" rx="{R["xs"]}" fill="{TOK["brand"][n]}" stroke="{s.col("border")}"/>')

    # ---------------------------------------------------------------- buttons
    x0, y0 = 48, 150
    s.label(x0, y0 - 16, "Buttons · rest, hover, press, focus, disabled")
    for row, kind in enumerate(["primary", "secondary", "ghost", "danger"]):
        for col, state in enumerate(["rest", "hover", "press", "focus", "disabled"]):
            s.button(x0 + col * 126, y0 + row * 56, 110, kind.capitalize() if col == 0 else state.capitalize(), kind, state)

    # ---------------------------------------------------------------- selection controls
    x1 = 720
    s.label(x1, y0 - 16, "Selection controls")
    s.toggle(x1, y0, True)
    s.toggle(x1 + 76, y0, False)
    s.checkbox(x1 + 160, y0 + 2, True)
    s.checkbox(x1 + 200, y0 + 2, False)
    s.radio(x1 + 250, y0 + 1, True)
    s.radio(x1 + 292, y0 + 1, False)
    s.label(x1, y0 + 62, "Slider · progress")
    s.slider(x1, y0 + 104, 220, 0.62)
    s.progress(x1 + 260, y0 + 100, 220, 0.45)
    s.label(x1, y0 + 148, "Chips · badges")
    cx = x1
    cx += s.chip(cx, y0 + 164, "All", True) + 8
    cx += s.chip(cx, y0 + 164, "Photos") + 8
    cx += s.chip(cx, y0 + 164, "Documents") + 8
    bx = x1
    for k, t in (("success", "Synced"), ("warning", "Low space"), ("danger", "Failed"), ("info", "New")):
        bx += s.badge(bx, y0 + 212, t, k) + 8

    # ---------------------------------------------------------------- fields and segmented control
    y2 = 410
    s.label(x0, y2 - 16, "Text fields · rest, focus, error")
    s.field(x0, y2, 330, "Search files", "rest", True)
    s.field(x0 + 350, y2, 330, "Trip 2026", "focus")
    s.field(x0 + 700, y2, 330, "Untitled/", "error")
    s.text(x0 + 700, y2 + 64, "A name can't contain a slash.", "caption", "danger-text")
    s.label(x0, y2 + 100, "Segmented control")
    s.segmented(x0, y2 + 116, ["Icons", "List", "Columns"], 0)
    s.label(x0 + 380, y2 + 100, "Type scale")
    ty = y2 + 140
    for role, sample in (("heading", "Heading 20"), ("body-strong", "Body strong 14"), ("body", "Body 14"), ("caption", "Caption 12")):
        s.text(x0 + 380, ty, sample, role)
        ty += {"heading": 26, "body-strong": 22, "body": 20, "caption": 16}[role]
    s.text(x0 + 380 + 230, y2 + 140, "mono 13: #8C3274", "mono", "text-2", family=MONO)

    # ---------------------------------------------------------------- surfaces
    y3 = 640
    s.label(x0, y3 - 16, "Menu")
    s.menu(x0, y3)
    s.label(x0 + 300, y3 - 16, "Toast")
    s.toast(x0 + 300, y3)
    s.label(x0 + 300, y3 + 104, "Dialog")
    s.dialog(x0 + 300 + 0, y3 + 120)
    # a card with the three radii
    s.label(x0 + 740, y3 - 16, "Radii · xs s m l")
    for i, n in enumerate(["xs", "s", "m", "l"]):
        s.rect(x0 + 740 + i * 100, y3, 84, 84, R[n], "panel", "border")
        s.text(x0 + 740 + i * 100 + 42, y3 + 48, f"{R[n]}", "label", "text-2", "middle")
    s.label(x0 + 740, y3 + 120, "Elevation · 1 2 3")
    for i in (1, 2, 3):
        s.add(f'<g filter="url(#e{i})"><rect x="{x0 + 740 + (i - 1) * 100 + 4}" y="{y3 + 140}" width="76" height="60" rx="{R["s"]}" fill="{s.col("raised")}"/></g>')
    return s.done(W, H)


# ------------------------------------------------------------------------------------------------ parts
def superellipse(cx, cy, a, b, n=5.0, steps=160):
    pts = []
    for i in range(steps):
        t = 2 * math.pi * i / steps
        c, s_ = math.cos(t), math.sin(t)
        x = cx + a * math.copysign(abs(c) ** (2 / n), c)
        y = cy + b * math.copysign(abs(s_) ** (2 / n), s_)
        pts.append((x, y))
    return "M" + " L".join(f"{f(x)} {f(y)}" for x, y in pts) + " Z"


def part_squircle():
    d = superellipse(64, 64, 62, 62)
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 128 128">\n'
            f'<!-- The squircle: |x|^5 + |y|^5 = 1. App icons are cut from it; a plain rounded rectangle is the fallback. -->\n'
            f'<path d="{d}" fill="{TOK["brand"]["orchid"]}"/>\n</svg>\n')


def part_focus():
    # two controls, light and dark; the ring is 2 px, 2 px off the control
    L, D = RES["light"], RES["dark"]
    o = Z["focus-offset"] + Z["focus-width"] / 2
    def tile(x, c):
        return (f'<rect x="{x}" y="0" width="200" height="120" fill="{c["bg"]}"/>'
                f'<rect x="{x + 40}" y="38" width="120" height="{Z["control-m"]}" rx="{R["s"]}" fill="{c["accent"]}"/>'
                f'<rect x="{f(x + 40 - o)}" y="{f(38 - o)}" width="{f(120 + 2 * o)}" height="{f(Z["control-m"] + 2 * o)}" rx="{f(R["s"] + o)}" fill="none" stroke="{c["focus"]}" stroke-width="{Z["focus-width"]}"/>')
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="400" height="120" viewBox="0 0 400 120">\n'
            f'<!-- Focus ring: {Z["focus-width"]} px wide, {Z["focus-offset"]} px off the control, in the focus colour of each mode. -->\n'
            f'{tile(0, L)}\n{tile(200, D)}\n</svg>\n')


def part_spinner():
    return ('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48" width="48" height="48" color="#8C3274">\n'
            '<!-- Indeterminate spinner: a 270 degree arc, one turn in 900 ms. It takes the colour of its text (currentColor). -->\n'
            '<g fill="none" stroke="currentColor" stroke-width="4.5" stroke-linecap="round">\n'
            '<circle cx="24" cy="24" r="17" stroke-opacity=".18"/>\n'
            '<path d="M24 7 A17 17 0 0 1 41 24">\n'
            '<animateTransform attributeName="transform" type="rotate" from="0 24 24" to="360 24 24" dur="0.9s" repeatCount="indefinite"/>\n'
            '</path></g>\n</svg>\n')


def part_skeleton():
    L = RES["light"]
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="320" height="96" viewBox="0 0 320 96">\n'
            f'<!-- A loading placeholder: the sheen crosses it in 1.4 s. -->\n'
            f'<defs><linearGradient id="sheen" x1="0" x2="1" y1="0" y2="0"><stop offset="0" stop-color="#fff" stop-opacity="0"/>'
            f'<stop offset=".5" stop-color="#fff" stop-opacity=".65"/><stop offset="1" stop-color="#fff" stop-opacity="0"/>'
            f'<animateTransform attributeName="gradientTransform" type="translate" from="-1 0" to="1 0" dur="1.4s" repeatCount="indefinite"/></linearGradient>'
            f'<clipPath id="k"><rect x="0" y="0" width="56" height="56" rx="{R["s"]}"/><rect x="72" y="6" width="200" height="14" rx="7"/>'
            f'<rect x="72" y="34" width="140" height="12" rx="6"/><rect x="0" y="72" width="320" height="12" rx="6"/></clipPath></defs>\n'
            f'<g clip-path="url(#k)"><rect width="320" height="96" fill="{L["track"]}"/><rect width="320" height="96" fill="url(#sheen)"/></g>\n</svg>\n')


def part_elevation():
    out_ = []
    defs = ""
    for mi, mode in enumerate(("light", "dark")):
        c = RES[mode]
        x = mi * 360
        out_.append(f'<rect x="{x}" y="0" width="360" height="200" fill="{c["bg"]}"/>')
        for i in (1, 2, 3):
            e = EL[str(i)]
            a = e["alpha-dark" if mode == "dark" else "alpha-light"]
            fid = f"e{mode[0]}{i}"
            defs += (f'<filter id="{fid}" x="-80%" y="-80%" width="260%" height="280%"><feDropShadow dx="0" dy="{e["y"]}" '
                     f'stdDeviation="{f(e["blur"] / 2)}" flood-color="{c["shadow"]}" flood-opacity="{a}"/></filter>')
            out_.append(f'<g filter="url(#{fid})"><rect x="{x + 28 + (i - 1) * 108}" y="56" width="92" height="88" rx="{R["m"]}" fill="{c["raised"]}"/></g>')
            out_.append(f'<text x="{x + 28 + (i - 1) * 108 + 46}" y="106" font-size="13" font-weight="600" text-anchor="middle" fill="{c["text-2"]}" font-family="{FONT}">{i}</text>')
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="720" height="200" viewBox="0 0 720 200">\n'
            f'<!-- Elevation 1, 2, 3: y offset {"/".join(str(EL[k]["y"]) for k in "123")} px, blur {"/".join(str(EL[k]["blur"]) for k in "123")} px; the shadow is Ink in light and black in dark. -->\n'
            f'<defs>{defs}</defs>\n' + "\n".join(out_) + "\n</svg>\n")


out("svg/parts/squircle.svg", part_squircle())
out("svg/parts/focus-ring.svg", part_focus())
out("svg/parts/spinner.svg", part_spinner())
out("svg/parts/skeleton.svg", part_skeleton())
out("svg/parts/elevation.svg", part_elevation())
out("svg/specimen-light.svg", specimen("light"))
out("svg/specimen-dark.svg", specimen("dark"))

if CHECK:
    if STALE:
        print("make_svg_parts: out of date:", ", ".join(STALE))
        sys.exit(1)
    print("make_svg_parts: all outputs up to date")
else:
    print("svg parts and specimens written")
