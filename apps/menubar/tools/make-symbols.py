#!/usr/bin/env python3
"""Writes the menu bar's SVG symbols into ui/icons: 24-unit, one-colour, round-ended strokes (tinted by
the theme), and the four coloured weather pictures. Every shape is drawn here from primitives; nothing
is traced from an existing icon theme. Run from the project root: python3 tools/make-symbols.py"""
import math, os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ui", "icons")


def write(path, body):
    full = os.path.join(OUT, path)
    os.makedirs(os.path.dirname(full), exist_ok=True)
    with open(full, "w") as f:
        f.write(body)


def sym(name, body, width=1.8):
    write(f"symbols/{name}.svg", f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="#000" '
          f'stroke-width="{width}" stroke-linecap="round" stroke-linejoin="round">{body}</svg>\n')


P = lambda d: f'<path d="{d}"/>'
F = lambda d: f'<path d="{d}" fill="#000" stroke="none"/>'
FE = lambda d: f'<path d="{d}" fill="#000" fill-rule="evenodd" stroke="none"/>'
C = lambda cx, cy, r: f'<circle cx="{cx}" cy="{cy}" r="{r}"/>'
R = lambda x, y, w, h, rx: f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}"/>'
SLASH = P("M4 4l16 16")

WIFI = P("M2.6 9.3a13.6 13.6 0 0 1 18.8 0") + P("M5.7 12.6a9.3 9.3 0 0 1 12.6 0") + P("M8.8 15.9a4.9 4.9 0 0 1 6.4 0") + '<circle cx="12" cy="19.4" r="1.15" fill="#000" stroke="none"/>'
sym("wifi", WIFI)
sym("wifi-off", WIFI + SLASH)
sym("bluetooth", P("M7 7.6l10 8.8-5 4.4V3.2l5 4.4-10 8.8"))
sym("bell", P("M6 10a6 6 0 0 1 12 0c0 5.6 2.4 7 2.4 7H3.6S6 15.6 6 10z") + P("M10 20.4a2.3 2.3 0 0 0 4 0"))
sym("bell-off", P("M8.2 5.3A6 6 0 0 1 18 10c0 3 .7 4.7 1.4 5.8") + P("M6 10c0 5.6-2.4 7-2.4 7H15") + P("M10 20.4a2.3 2.3 0 0 0 4 0") + SLASH)
sym("moon", P("M20 14.6A8.6 8.6 0 1 1 9.4 4a6.9 6.9 0 0 0 10.6 10.6z"))
rays = "".join(P(f"M{12 + 7.4 * math.cos(a):.2f} {12 + 7.4 * math.sin(a):.2f}L{12 + 9.6 * math.cos(a):.2f} {12 + 9.6 * math.sin(a):.2f}") for a in [i * math.pi / 4 for i in range(8)])
sym("sun", C(12, 12, 3.9) + rays)
spk = P("M3.8 9.6h3.6L12 5.8v12.4l-4.6-3.8H3.8z")
sym("volume-0", spk)
sym("volume-1", spk + P("M15.4 9.2a4.2 4.2 0 0 1 0 5.6"))
sym("volume-2", spk + P("M15.4 9.2a4.2 4.2 0 0 1 0 5.6") + P("M18.2 6.6a8 8 0 0 1 0 10.8"))
sym("volume-mute", spk + P("M16 9.5l4.5 5M20.5 9.5l-4.5 5"))
sym("mic", P("M12 3.4a3 3 0 0 1 3 3V12a3 3 0 0 1-6 0V6.4a3 3 0 0 1 3-3z") + P("M6.4 11a5.6 5.6 0 0 0 11.2 0") + P("M12 16.6V20.4"))
sym("power", P("M12 3.2v8") + P("M7.2 6.2a8 8 0 1 0 9.6 0"))
sym("lock", R(5, 10.8, 14, 9.8, 2.6) + P("M8 10.8V8a4 4 0 0 1 8 0v2.8"))
sym("logout", P("M10 4H6.6A2.6 2.6 0 0 0 4 6.6v10.8A2.6 2.6 0 0 0 6.6 20H10") + P("M15.2 8l4 4-4 4") + P("M19.2 12H9.4"))
sym("restart", P("M20 12a8 8 0 1 1-2.6-5.9") + P("M20 4.2v4.6h-4.6"))
sym("airplane", P("M10.4 13.6L3.4 12v-1.7l7-1.6V4.6a1.6 1.6 0 0 1 3.2 0v4.1l7 1.6V12l-7 1.6v4.3l2.4 1.5V21L12 20.1 8 21v-1.6l2.4-1.5z"))
sym("dark-mode", C(12, 12, 8.2) + F("M12 3.8a8.2 8.2 0 0 1 0 16.4z"))
sym("night-light", P("M12 3.4v3.4") + P("M4.9 7.9l2.3 2.3") + P("M19.1 7.9l-2.3 2.3") + P("M3 17h18") + P("M7.4 17a4.6 4.6 0 0 1 9.2 0") + P("M8 20.6h8"))
sym("keyboard", R(2.6, 6.4, 18.8, 11.2, 2.6) + P("M6.6 10.4h.01M10.2 10.4h.01M13.8 10.4h.01M17.4 10.4h.01M7.6 14h8.8"), 2.0)
sym("chevron-right", P("M9.4 5.8l6.2 6.2-6.2 6.2"))
sym("chevron-left", P("M14.6 5.8L8.4 12l6.2 6.2"))
sym("chevron-down", P("M5.8 9.4l6.2 6.2 6.2-6.2"))
sym("chevron-up", P("M5.8 14.6l6.2-6.2 6.2 6.2"))
sym("check", P("M5 12.6l4.6 4.6L19 7.6"))
sym("close", P("M6.2 6.2l11.6 11.6M17.8 6.2L6.2 17.8"))
sym("search", C(10.6, 10.6, 6.4) + P("M15.4 15.4l5 5"))
sym("plus", P("M12 5v14M5 12h14"))
sym("minus", P("M5 12h14"))
sym("calendar", R(3.8, 5.2, 16.4, 15, 3.2) + P("M3.8 10h16.4") + P("M8.4 3.2v4M15.6 3.2v4"))
sym("clock", C(12, 12, 8.6) + P("M12 7.2V12l3.2 2"))
sym("play", F("M8.2 5.6v12.8a.6.6 0 0 0 .9.5l10.4-6.4a.6.6 0 0 0 0-1L9.1 5.1a.6.6 0 0 0-.9.5z"), 1.2)
sym("pause", R(6.2, 5, 3.8, 14, 1.4).replace("<rect", '<rect fill="#000"') + R(14, 5, 3.8, 14, 1.4).replace("<rect", '<rect fill="#000"'), 1.2)
sym("next", F("M5.4 6.2v11.6a.6.6 0 0 0 .9.5L15 12.5a.6.6 0 0 0 0-1L6.3 5.7a.6.6 0 0 0-.9.5z") + P("M18.4 6v12"), 1.6)
sym("previous", F("M18.6 6.2v11.6a.6.6 0 0 1-.9.5L9 12.5a.6.6 0 0 1 0-1l8.7-5.8a.6.6 0 0 1 .9.5z") + P("M5.6 6v12"), 1.6)
sym("plug", P("M9 3v4.6M15 3v4.6") + P("M6.4 7.6h11.2v3.4a5.6 5.6 0 0 1-11.2 0z") + P("M12 16.6V21"))
sym("info", C(12, 12, 8.6) + P("M12 11v5.2") + '<circle cx="12" cy="7.9" r="1.1" fill="#000" stroke="none"/>')
sym("more", P("M5.6 12h.01M12 12h.01M18.4 12h.01"), 3.2)
sym("refresh", P("M19.6 12a7.6 7.6 0 1 1-2.3-5.4") + P("M19.8 4.6v4.2h-4.2"))
sym("trash", P("M4.4 7h15.2M9.6 7V4.8h4.8V7") + P("M6.4 7l.8 12a1.6 1.6 0 0 0 1.6 1.5h6.4a1.6 1.6 0 0 0 1.6-1.5l.8-12") + P("M10 11v5.4M14 11v5.4"))
sym("eye", P("M2.6 12S6 5.8 12 5.8 21.4 12 21.4 12 18 18.2 12 18.2 2.6 12 2.6 12z") + C(12, 12, 2.8))
sym("sliders", P("M4 7h9M17 7h3M4 17h3M11 17h9") + C(15, 7, 2.2) + C(9, 17, 2.2))
sym("lightning", F("M13.4 2.8L5.4 13.2h5.6l-1 8 8.2-10.8h-5.8z"), 1.4)
sym("headphones", P("M4.6 15v-3a7.4 7.4 0 0 1 14.8 0v3") + R(3.6, 14.2, 4, 6, 1.9) + R(16.4, 14.2, 4, 6, 1.9))
sym("mouse", R(6.6, 3.4, 10.8, 17.2, 5.4) + P("M12 3.6v5.6"))
sym("phone", R(7, 2.6, 10, 18.8, 2.8) + P("M10.8 18.2h2.4"))
sym("speaker", R(6.6, 3, 10.8, 18, 3) + C(12, 14.4, 3.3) + C(12, 7.4, 1.1))
sym("globe", C(12, 12, 8.8) + P("M3.4 12h17.2") + P("M12 3.2c2.8 2.6 4 5.6 4 8.8s-1.2 6.2-4 8.8c-2.8-2.6-4-5.6-4-8.8s1.2-6.2 4-8.8z"))
sym("signal", P("M5 18v-3M10 18v-6M15 18V9M20 18V6"), 2.4)
sym("sidebar", R(3.6, 4.6, 16.8, 14.8, 3) + P("M14.6 4.6v14.8"))
sym("window", R(3.4, 5, 17.2, 14, 3) + P("M3.4 9.6h17.2"))
sym("copy", R(8.6, 8.6, 11.4, 11.4, 2.6) + P("M15.4 8.6V6.6a2.6 2.6 0 0 0-2.6-2.6H6.6A2.6 2.6 0 0 0 4 6.6v6.2a2.6 2.6 0 0 0 2.6 2.6h2"))
sym("undo", P("M8.6 6.4L4.4 10.6l4.2 4.2") + P("M4.4 10.6h9.2a5.4 5.4 0 0 1 0 10.8H9"))
sym("redo", P("M15.4 6.4l4.2 4.2-4.2 4.2") + P("M19.6 10.6h-9.2a5.4 5.4 0 0 0 0 10.8H15"))
sym("minimize", P("M5 18h14"))
sym("fullscreen", P("M4.4 9.4V5.4a1 1 0 0 1 1-1h4M14.6 4.4h4a1 1 0 0 1 1 1v4M19.6 14.6v4a1 1 0 0 1-1 1h-4M9.4 19.6h-4a1 1 0 0 1-1-1v-4"))

# the gear: eight teeth, a round hub
pts = []
for i in range(8):
    a = i * 45
    for r, d in ((7.0, -17), (9.4, -9.5), (9.4, 9.5), (7.0, 17)):
        t = math.radians(a + d - 90)
        pts.append((12 + r * math.cos(t), 12 + r * math.sin(t)))
gear = "M" + " L".join(f"{x:.2f} {y:.2f}" for x, y in pts) + "Z"
sym("gear", P(gear) + C(12, 12, 2.9))

# the fish: the system menu's mark (a hole for the eye)
# The bar's logo is the Malva mark, the brand kit's single-colour small mark (assets/logo/malva-mark-small-black.svg),
# unchanged: five petals and a centre, which the bar tints with the theme's text colour.
MALVA_MARK = '<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" viewBox="-1.2 -1.2 2.4 2.4" role="img" aria-label="Malva OS"><defs><path id="mp" d="M0 0C-0.200 -.10 -0.500 -.30 -0.500 -.62C-0.500 -0.860 -0.430 -1.000 -0.260 -1.000C-0.117 -1.000 -0.030 -0.800 0 -0.700C.03 -0.800 0.117 -1.000 0.260 -1.000C0.430 -1.000 0.500 -0.860 0.500 -.62C0.500 -.30 0.200 -.10 0 0Z"/></defs><g fill="#2A1226"><use xlink:href="#mp" transform="rotate(0) translate(0 -0.24) scale(0.86)"/><use xlink:href="#mp" transform="rotate(72) translate(0 -0.24) scale(0.86)"/><use xlink:href="#mp" transform="rotate(144) translate(0 -0.24) scale(0.86)"/><use xlink:href="#mp" transform="rotate(216) translate(0 -0.24) scale(0.86)"/><use xlink:href="#mp" transform="rotate(288) translate(0 -0.24) scale(0.86)"/></g><circle r="0.16" fill="#2A1226"/></svg>'
write("symbols/logo.svg", MALVA_MARK)

# ---------------------------------------------------------------- coloured weather pictures (64 units)
def weather(name, body):
    write(f"app/{name}.svg", f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64">
<defs>
<linearGradient id="sun" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#ffd84a"/><stop offset="1" stop-color="#ff9f1a"/></linearGradient>
<linearGradient id="cl" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#ffffff"/><stop offset="1" stop-color="#d9d9df"/></linearGradient>
<linearGradient id="cd" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#bcbcc4"/><stop offset="1" stop-color="#8e8e98"/></linearGradient>
<filter id="sh" x="-20%" y="-20%" width="140%" height="150%"><feDropShadow dx="0" dy="2" stdDeviation="2" flood-color="#1d1d1f" flood-opacity=".25"/></filter>
</defs>
{body}
</svg>
''')

def sunbody(cx, cy, r):
    rays = "".join(f'<line x1="{cx + (r + 4) * math.cos(a):.1f}" y1="{cy + (r + 4) * math.sin(a):.1f}" x2="{cx + (r + 8) * math.cos(a):.1f}" y2="{cy + (r + 8) * math.sin(a):.1f}" stroke="#ffb82e" stroke-width="3.4" stroke-linecap="round"/>' for a in [i * math.pi / 4 for i in range(8)])
    return rays + f'<circle cx="{cx}" cy="{cy}" r="{r}" fill="url(#sun)"/><ellipse cx="{cx - r * .25}" cy="{cy - r * .4}" rx="{r * .55}" ry="{r * .3}" fill="#fff" opacity=".4"/>'

def cloud(x, y, s, fill="url(#cl)"):
    return (f'<g transform="translate({x} {y}) scale({s})" filter="url(#sh)" fill="{fill}">'
            '<circle cx="18" cy="30" r="11"/><circle cx="31" cy="24" r="14"/><circle cx="44" cy="31" r="10"/>'
            '<rect x="8" y="30" width="46" height="12" rx="6"/></g>')

weather("weather-clear", sunbody(32, 32, 13))
weather("weather-partly", sunbody(24, 24, 10) + cloud(4, 12, 1.0))
weather("weather-cloudy", cloud(8, 0, 0.7, "url(#cd)") + cloud(2, 14, 1.0))
drops = "".join(f'<path d="M{x} {y}l-3 7a3 3 0 1 0 6 0z" fill="#4aa3ff"/>' for x, y in ((20, 46), (32, 50), (44, 46)))
weather("weather-rain", cloud(2, 2, 1.0, "url(#cd)") + drops)

# ---------------------------------------------------------------- more symbols
ARC1 = P("M8.8 15.9a4.9 4.9 0 0 1 6.4 0")
ARC2 = P("M5.7 12.6a9.3 9.3 0 0 1 12.6 0")
ARC3 = P("M2.6 9.3a13.6 13.6 0 0 1 18.8 0")
DOT = '<circle cx="12" cy="19.4" r="1.15" fill="#000" stroke="none"/>'
dim = lambda d: f'<g opacity=".28">{d}</g>'
sym("wifi-1", dim(ARC3) + dim(ARC2) + ARC1 + DOT)
sym("wifi-2", dim(ARC3) + ARC2 + ARC1 + DOT)
sym("wifi-3", ARC3 + ARC2 + ARC1 + DOT)
sym("folder", P("M3.6 7.4A2.4 2.4 0 0 1 6 5h3.4l2 2.4H18A2.4 2.4 0 0 1 20.4 9.8V16.6A2.4 2.4 0 0 1 18 19H6a2.4 2.4 0 0 1-2.4-2.4z"))
sym("user", C(12, 8.4, 3.8) + P("M4.4 20.2a7.6 7.6 0 0 1 15.2 0"))
sym("help", C(12, 12, 8.6) + P("M9.6 9.6a2.5 2.5 0 1 1 3.6 2.2c-.8.4-1.2 1-1.2 1.9") + '<circle cx="12" cy="16.9" r="1.05" fill="#000" stroke="none"/>')
sym("clipboard", R(5.4, 4.6, 13.2, 16, 2.8) + R(8.8, 2.8, 6.4, 3.8, 1.6) + P("M9 11.4h6M9 15h4"))
sym("shield", P("M12 3.2l7 2.6v5.6c0 4.4-2.8 7.6-7 9.4-4.2-1.8-7-5-7-9.4V5.8z") + P("M9 12l2.2 2.2L15.2 10"))
sym("record", C(12, 12, 8.4) + '<circle cx="12" cy="12" r="3.6" fill="#000" stroke="none"/>')
sym("download", P("M12 4v11") + P("M7.4 10.6l4.6 4.6 4.6-4.6") + P("M4.8 19.6h14.4"))
sym("drop", P("M12 3.4c3.6 4.2 5.8 7 5.8 10a5.8 5.8 0 0 1-11.6 0c0-3 2.2-5.8 5.8-10z"))
sym("wind", P("M3.6 9.4h10.2a2.5 2.5 0 1 0-2.4-3.2") + P("M3.6 13.4h14.4a2.6 2.6 0 1 1-2.5 3.3") + P("M3.6 17.4h5.4"))
sym("thermometer", P("M10 14.4V5.6a2 2 0 0 1 4 0v8.8a4 4 0 1 1-4 0z") + P("M12 9v6.8"))
sym("screenshot", P("M4 8.4V6.6A2.6 2.6 0 0 1 6.6 4h1.8M15.6 4h1.8A2.6 2.6 0 0 1 20 6.6v1.8M20 15.6v1.8a2.6 2.6 0 0 1-2.6 2.6h-1.8M8.4 20H6.6A2.6 2.6 0 0 1 4 17.4v-1.8") + C(12, 12, 3))
sym("keyboard-small", R(3, 7, 18, 10, 2.4) + P("M7 11h.01M11 11h.01M15 11h.01M7.6 14h8.8"), 2.0)

weather("weather-night", f'''<g filter="url(#sh)"><path d="M46 38a17 17 0 1 1-20-27 13.6 13.6 0 0 0 20 27z" fill="#eef0f8"/><path d="M40 20l1.4 3.4 3.4 1.4-3.4 1.4L40 29.6l-1.4-3.4-3.4-1.4 3.4-1.4z" fill="#ffe27a"/></g>''')

# ---------------------------------------------------------------- the Slint icon table: always matches the files
def emit_icon_slint():
    syms = sorted(f[:-4] for f in os.listdir(os.path.join(OUT, "symbols")))
    apps = sorted(f[:-4] for f in os.listdir(os.path.join(OUT, "app")))
    L = ['import { Theme } from "../theme.slint";', "",
         "// Generated by tools/make-symbols.py from the files in ui/icons: do not edit by hand.", "",
         "// A coloured picture by name: an app's tile, or a weather picture. Unknown names show \"default\".",
         "export component AppIcon inherits Image {",
         "    in property <string> name;",
         "    in property <image> system-image;",
         "    in property <bool> has-system-image: false;",
         "    image-fit: contain;",
         "    source: has-system-image ? system-image"]
    for a in apps:
        L.append(f'        : name == "{a}" ? @image-url("../icons/app/{a}.svg")')
    L += ['        : @image-url("../icons/app/default.svg");', "}", "",
          "// A one-colour symbol, tinted with the colour it is given.",
          "export component Symbol inherits Image {",
          "    in property <string> name;",
          "    in property <color> tint: Theme.text;",
          "    colorize: tint;",
          "    image-fit: contain;",
          "    source: name == \"\" ? @image-url(\"../icons/symbols/more.svg\")"]
    for a in syms:
        L.append(f'        : name == "{a}" ? @image-url("../icons/symbols/{a}.svg")')
    L += ['        : @image-url("../icons/symbols/info.svg");', "}", ""]
    with open(os.path.join(OUT, "..", "components", "icon.slint"), "w") as f:
        f.write("\n".join(L))
    return len(syms), len(apps)

n_s, n_a = emit_icon_slint()
print("icon.slint:", n_s, "symbols,", n_a, "pictures")

print("symbols:", len(os.listdir(os.path.join(OUT, "symbols"))), "pictures:", len([f for f in os.listdir(os.path.join(OUT, "app")) if f.startswith("weather")]))
