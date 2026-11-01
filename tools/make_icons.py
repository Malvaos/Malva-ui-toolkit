#!/usr/bin/env python3
"""Draws the icons that the kit adds to the 73 menu-bar symbols, into svg/icons/.

Every icon is on a 24-unit grid, one colour (currentColor), 1.8 stroke, round caps and joins. Every shape is
drawn here from primitives; nothing is traced from an existing icon theme. The files in svg/icons/ are the
source of truth: this script only (re)creates the ones listed here, and fixes the brand mark so that it
takes the colour it is given.

    python3 tools/make_icons.py"""
import math, os, re

OUT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "svg", "icons"))
os.makedirs(OUT, exist_ok=True)


def sym(name, body, width=1.8):
    with open(os.path.join(OUT, name + ".svg"), "w") as f:
        f.write(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" '
                f'stroke-width="{width}" stroke-linecap="round" stroke-linejoin="round">{body}</svg>\n')


P = lambda d, extra="": f'<path d="{d}"{extra}/>'
FILL = ' fill="currentColor" stroke="none"'
C = lambda cx, cy, r, extra="": f'<circle cx="{cx}" cy="{cy}" r="{r}"{extra}/>'
R = lambda x, y, w, h, rx, extra="": f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}"{extra}/>'
DOT = lambda x, y: P(f"M{x} {y}h.01")


def star_points(cx, cy, ro, ri, n=5):
    pts = []
    for i in range(n * 2):
        a = -math.pi / 2 + i * math.pi / n
        r = ro if i % 2 == 0 else ri
        pts.append(f"{cx + r * math.cos(a):.2f} {cy + r * math.sin(a):.2f}")
    return "M" + "L".join(pts) + "Z"


STAR = star_points(12, 12.8, 9.2, 4.2)
sym("star", P(STAR))
sym("star-fill", P(STAR, FILL.replace('stroke="none"', 'stroke="currentColor"')))
sym("heart", P("M12 20s-7.4-4.4-7.4-10.2a4.4 4.4 0 0 1 7.4-3.2 4.4 4.4 0 0 1 7.4 3.2C19.4 15.6 12 20 12 20z"))
sym("alert", P("M12 4.2l8.7 15.1H3.3z") + P("M12 10v4.2") + DOT(12, 16.9))
sym("error", C(12, 12, 8.6) + P("M9.1 9.1l5.8 5.8M14.9 9.1l-5.8 5.8"))
sym("success", C(12, 12, 8.6) + P("M8.2 12.4l2.6 2.6 5-5.4"))
sym("plus-circle", C(12, 12, 8.6) + P("M12 8.4v7.2M8.4 12h7.2"))
sym("edit", P("M4.4 19.6l.9-4 10.1-10.1a2.1 2.1 0 0 1 3 3L8.3 18.6z") + P("M13.8 7.2l3 3"))
sym("share", P("M12 15V4M8 7.6l4-4 4 4") + P("M6.4 11H5.6A1.6 1.6 0 0 0 4 12.6v5.8A1.6 1.6 0 0 0 5.6 20h12.8a1.6 1.6 0 0 0 1.6-1.6v-5.8a1.6 1.6 0 0 0-1.6-1.6h-.8"))
sym("link", P("M10 14a4 4 0 0 0 5.7 0l3-3a4 4 0 0 0-5.7-5.7l-1 1") + P("M14 10a4 4 0 0 0-5.7 0l-3 3a4 4 0 0 0 5.7 5.7l1-1"))
sym("home", P("M3.8 11.2L12 4.4l8.2 6.8") + P("M6 9.6V19a1 1 0 0 0 1 1h3.4v-5.4h3.2V20H17a1 1 0 0 0 1-1V9.6"))
sym("menu", P("M4 7h16M4 12h16M4 17h16"))
sym("grid", R(4, 4, 6.4, 6.4, 1.8) + R(13.6, 4, 6.4, 6.4, 1.8) + R(4, 13.6, 6.4, 6.4, 1.8) + R(13.6, 13.6, 6.4, 6.4, 1.8))
sym("list", P("M9.4 7H20M9.4 12H20M9.4 17H20") + P("M4.6 7h.01M4.6 12h.01M4.6 17h.01", ' stroke-width="2.6"'))
sym("filter", P("M4 5.6h16l-6.2 7.4v5.4l-3.6 1.8V13z"))
sym("sort", P("M7 4.6v14.8M3.8 16.2L7 19.4l3.2-3.2") + P("M17 19.4V4.6M13.8 7.8L17 4.6l3.2 3.2"))
sym("tag", P("M3.6 12.4V4.8a1.2 1.2 0 0 1 1.2-1.2h7.6l8 8a1.2 1.2 0 0 1 0 1.7l-6.7 6.7a1.2 1.2 0 0 1-1.7 0z") + DOT(8, 8))
sym("arrow-left", P("M19 12H5M11 6l-6 6 6 6"))
sym("arrow-right", P("M5 12h14M13 6l6 6-6 6"))
sym("arrow-up", P("M12 19V5M6 11l6-6 6 6"))
sym("arrow-down", P("M12 5v14M6 13l6 6 6-6"))
sym("external", P("M13.4 4H20v6.6") + P("M20 4l-9 9") + P("M18 14v4.4a1.6 1.6 0 0 1-1.6 1.6H5.6A1.6 1.6 0 0 1 4 18.4V7.6A1.6 1.6 0 0 1 5.6 6H10"))
sym("cloud", P("M7.2 18.6a4.4 4.4 0 0 1-.8-8.7 5.8 5.8 0 0 1 11.3 1.3 3.7 3.7 0 0 1-.9 7.4z"))
sym("more-vertical", P("M12 5.6v.01M12 12v.01M12 18.4v.01"), 3.2)
sym("folder-open", P("M4 19V6.4A1.4 1.4 0 0 1 5.4 5h3.8l2 2.2h6.4A1.4 1.4 0 0 1 19 8.6V11") + P("M4 19l2.3-6.6a1.4 1.4 0 0 1 1.3-1h13a1 1 0 0 1 1 1.3L19.4 18a1.4 1.4 0 0 1-1.3 1z"))
sym("file", P("M6.4 3.6h7.2l4.8 4.8v11.2a1 1 0 0 1-1 1H6.4a1 1 0 0 1-1-1V4.6a1 1 0 0 1 1-1z") + P("M13.6 3.6v4.8h4.8"))
sym("file-text", P("M6.4 3.6h7.2l4.8 4.8v11.2a1 1 0 0 1-1 1H6.4a1 1 0 0 1-1-1V4.6a1 1 0 0 1 1-1z") + P("M13.6 3.6v4.8h4.8") + P("M8.8 13h6.4M8.8 16.4h4.2"))
sym("image", R(3.6, 4.6, 16.8, 14.8, 2.6) + C(9, 10, 1.7) + P("M3.8 16.6l4.8-4.6 3.8 3.6 3.2-3 4.8 4.4"))
sym("music", P("M9 18V6.4l10-2V16") + C(6.6, 18, 2.4) + C(16.6, 16, 2.4))
sym("video", R(3.4, 6, 13.2, 12, 2.6) + P("M16.6 10.4l4-2.4v8l-4-2.4"))
sym("archive", R(4, 4.8, 16, 4, 1.2) + P("M5.2 8.8V18a1.2 1.2 0 0 0 1.2 1.2h11.2a1.2 1.2 0 0 0 1.2-1.2V8.8") + P("M10 12.6h4"))
sym("drive", P("M4 14.6V6.8A1.8 1.8 0 0 1 5.8 5h12.4A1.8 1.8 0 0 1 20 6.8v7.8") + R(4, 14.6, 16, 4.6, 1.8) + DOT(16.6, 16.9) + DOT(13.4, 16.9))
sym("usb", P("M8 3.6h8v5.4H8z") + P("M10.6 6.2h.01M13.4 6.2h.01") + P("M6.8 9h10.4v8a3 3 0 0 1-3 3h-4.4a3 3 0 0 1-3-3z"))
sym("eject", P("M12 4.6l7.6 9H4.4z") + P("M5 18.8h14"))
sym("network", R(9.2, 3.6, 5.6, 5, 1.4) + R(3.4, 15.4, 5.6, 5, 1.4) + R(15, 15.4, 5.6, 5, 1.4) + P("M12 8.6v3.4M6.2 15.4V12h11.6v3.4"))
sym("desktop", R(3.6, 4.4, 16.8, 11.4, 2.2) + P("M9 20h6M12 15.8V20"))
sym("zoom-in", C(10.6, 10.6, 6.4) + P("M15.4 15.4l5 5") + P("M8 10.6h5.2M10.6 8v5.2"))
sym("zoom-out", C(10.6, 10.6, 6.4) + P("M15.4 15.4l5 5") + P("M8 10.6h5.2"))
sym("save", P("M5.4 3.8h10.2l4.4 4.4v10.6a1.4 1.4 0 0 1-1.4 1.4H5.4A1.4 1.4 0 0 1 4 18.8V5.2a1.4 1.4 0 0 1 1.4-1.4z") + P("M7.6 3.8v5h7.2V3.8") + P("M7.6 20.2v-6h8.8v6"))
sym("upload", P("M12 16V4M7.6 8.4L12 4l4.4 4.4") + P("M4.8 15.6v2.8A1.6 1.6 0 0 0 6.4 20h11.2a1.6 1.6 0 0 0 1.6-1.6v-2.8"))
sym("key", C(8, 15.6, 3.8) + P("M10.7 12.9L20 3.6M16.4 7.2l2.6 2.6"))
sym("bookmark", P("M6.6 3.8h10.8a.8.8 0 0 1 .8.8V20l-6.2-4.4L5.8 20V4.6a.8.8 0 0 1 .8-.8z"))
sym("camera", P("M3.8 8.2a1.6 1.6 0 0 1 1.6-1.6h2.2l1.4-2.2h6l1.4 2.2h2.2a1.6 1.6 0 0 1 1.6 1.6v9.6a1.6 1.6 0 0 1-1.6 1.6H5.4a1.6 1.6 0 0 1-1.6-1.6z") + C(12, 12.8, 3.6))
sym("printer", P("M7 8.4V3.6h10v4.8") + P("M7 17H5.4a1.6 1.6 0 0 1-1.6-1.6V10a1.6 1.6 0 0 1 1.6-1.6h13.2a1.6 1.6 0 0 1 1.6 1.6v5.4a1.6 1.6 0 0 1-1.6 1.6H17") + R(7, 13.6, 10, 6.8, 1))
sym("mail", R(3.4, 5.4, 17.2, 13.2, 2.6) + P("M4 7.6l8 6 8-6"))
sym("chat", P("M20 12.4a7.4 7.4 0 0 1-10.6 6.7L4.6 20l1.2-4.4A7.4 7.4 0 1 1 20 12.4z"))
sym("users", C(9, 8.6, 3.4) + P("M2.8 19.6a6.2 6.2 0 0 1 12.4 0") + P("M15.8 5.4a3.4 3.4 0 0 1 0 6.4") + P("M18 14a6.2 6.2 0 0 1 3.2 5.6"))
sym("code", P("M8.4 7.4L3.8 12l4.6 4.6M15.6 7.4l4.6 4.6-4.6 4.6M13.6 5.2l-3.2 13.6"))
sym("cpu", R(6.6, 6.6, 10.8, 10.8, 2) + R(9.6, 9.6, 4.8, 4.8, 1) + P("M9.6 3.6v3M14.4 3.6v3M9.6 17.4v3M14.4 17.4v3M3.6 9.6h3M3.6 14.4h3M17.4 9.6h3M17.4 14.4h3"))
sym("memory", R(3.4, 7.2, 17.2, 9, 1.6) + P("M7 16.2v2.6M10.4 16.2v2.6M13.6 16.2v2.6M17 16.2v2.6") + P("M7.4 10.6h.01M10.8 10.6h.01M14.2 10.6h.01M17.6 10.6h.01"))
sym("server", R(3.6, 4.6, 16.8, 6.4, 1.8) + R(3.6, 13, 16.8, 6.4, 1.8) + DOT(7.4, 7.8) + DOT(7.4, 16.2) + P("M12 7.8h5M12 16.2h5"))
sym("palette", P("M12 3.6a8.4 8.4 0 1 0 0 16.8c1.3 0 1.9-.9 1.9-1.8 0-1.4-1.4-1.3-1.4-2.6 0-.9.8-1.6 1.8-1.6H17a3.6 3.6 0 0 0 3.6-3.6C20.6 6.8 16.8 3.6 12 3.6z") + DOT(7.6, 11.4) + DOT(10, 7.6) + DOT(14.4, 7.6))
sym("sparkle", P("M12 3.6c.6 4.8 2.8 7 8.4 8.4-5.6 1.4-7.8 3.6-8.4 8.4-.6-4.8-2.8-7-8.4-8.4 5.6-1.4 7.8-3.6 8.4-8.4z"))
sym("eye-off", P("M3 12s3.4-6.2 9-6.2c1.6 0 3 .5 4.2 1.2M21 12s-3.4 6.2-9 6.2c-1.6 0-3-.5-4.2-1.2") + P("M9.6 9.8a3.2 3.2 0 0 0 4.6 4.4") + P("M4 4l16 16"))
sym("location", P("M12 20.6s6.4-5.6 6.4-10.8a6.4 6.4 0 0 0-12.8 0c0 5.2 6.4 10.8 6.4 10.8z") + C(12, 9.8, 2.4))
sym("battery", R(3.2, 8, 16, 8.4, 2.2) + P("M21.4 11v2.4") + P("M6.2 10.8v2.8"))
sym("grip", P("M9 6h.01M15 6h.01M9 12h.01M15 12h.01M9 18h.01M15 18h.01"), 2.6)

# The brand mark, as a symbol: it takes the colour it is given.
mark = os.path.join(OUT, "malva.svg")
if os.path.exists(mark):
    s = open(mark).read().replace('"#2A1226"', '"currentColor"')
    open(mark, "w").write(s)
print("icons:", len([f for f in os.listdir(OUT) if f.endswith('.svg')]))
