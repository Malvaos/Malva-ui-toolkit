#!/usr/bin/env python3
"""Writes the dock's SVG icon set into ui/icons. Every icon is drawn here from primitives; nothing is
traced from any existing icon theme. Run from the project root: python3 tools/make-icons.py"""
import math, os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ui", "icons")
RX = 29  # corner radius of a 128 tile, a little under a quarter of its side

def write(path, body):
    full = os.path.join(OUT, path)
    os.makedirs(os.path.dirname(full), exist_ok=True)
    with open(full, "w") as f:
        f.write(body)

def tile(name, top, bottom, glyph, defs="", gloss=0.5):
    """A squircle tile: vertical gradient, a gloss over its upper half, a light inner edge and a dark outer hairline."""
    svg = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 128 128">
<defs>
<linearGradient id="g" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="{top}"/><stop offset="1" stop-color="{bottom}"/></linearGradient>
<linearGradient id="gl" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#fff" stop-opacity="{gloss}"/><stop offset="0.55" stop-color="#fff" stop-opacity="0"/></linearGradient>
<clipPath id="c"><rect width="128" height="128" rx="{RX}"/></clipPath>
{defs}
</defs>
<rect width="128" height="128" rx="{RX}" fill="url(#g)"/>
<g clip-path="url(#c)">
{glyph}
<rect width="128" height="76" fill="url(#gl)"/>
</g>
<rect x="1" y="1" width="126" height="126" rx="{RX-1}" fill="none" stroke="#fff" stroke-opacity=".38" stroke-width="2"/>
<rect x=".5" y=".5" width="127" height="127" rx="{RX-.5}" fill="none" stroke="#000" stroke-opacity=".22"/>
</svg>
'''
    write(f"app/{name}.svg", svg)

# ---------------------------------------------------------------- launcher: nine coloured dots on dark glass
dots = ""
colors = ["#ff6b6b", "#ffb347", "#ffe066", "#51cf66", "#3bc9db", "#4dabf7", "#9775fa", "#dc73c2", "#ffffff"]
for i, c in enumerate(colors):
    x = 36 + (i % 3) * 28
    y = 36 + (i // 3) * 28
    dots += f'<circle cx="{x}" cy="{y}" r="9.5" fill="{c}"/>\n'
tile("launcher", "#4a4a50", "#1b1b1e", dots)

# ---------------------------------------------------------------- files: an orchid folder on a light tile
folder = '''<g transform="translate(14 17) scale(1.5)">
<defs><linearGradient id="fb" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#b84f9f"/><stop offset="1" stop-color="#8c3274"/></linearGradient>
<linearGradient id="ff" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#e58bd0"/><stop offset="1" stop-color="#c95fae"/></linearGradient></defs>
<path d="M5 14a5 5 0 0 1 5-5h14l6 6h24a5 5 0 0 1 5 5v31a5 5 0 0 1-5 5H10a5 5 0 0 1-5-5z" fill="url(#fb)"/>
<rect x="5" y="22" width="54" height="33" rx="5" fill="url(#ff)"/>
<rect x="5.5" y="22.5" width="53" height="1" fill="#fff" opacity=".55"/></g>'''
tile("files", "#ffffff", "#e4e4ea", folder, gloss=0.0)

# ---------------------------------------------------------------- browser: a globe
globe = '''<g fill="none" stroke="#fff" stroke-width="6" stroke-linecap="round">
<circle cx="64" cy="64" r="34"/>
<ellipse cx="64" cy="64" rx="14" ry="34"/>
<path d="M32 52 Q64 62 96 52 M32 76 Q64 66 96 76 M30 64 H98" stroke-width="5"/></g>'''
tile("browser", "#52b0ff", "#1d5be0", globe)

# ---------------------------------------------------------------- terminal
term = '''<path d="M38 46 L64 64 L38 82" fill="none" stroke="#6dffb0" stroke-width="10" stroke-linecap="round" stroke-linejoin="round"/>
<path d="M74 86 H98" stroke="#fff" stroke-width="10" stroke-linecap="round"/>'''
tile("terminal", "#3e3e43", "#121214", term, gloss=0.28)

# ---------------------------------------------------------------- settings: a gear
teeth = "".join(f'<rect x="-7" y="-41" width="14" height="17" rx="3.5" transform="rotate({k*45})"/>' for k in range(8))
gear = f'<g transform="translate(64 64)" fill="#fff"><circle r="23" fill="none" stroke="#fff" stroke-width="15"/>{teeth}</g>'
tile("settings", "#aaaab0", "#5c5c62", gear)

# ---------------------------------------------------------------- music: two beamed notes
music = '''<g fill="#fff"><path d="M58 36 L100 26 V84 H88 V44 L70 48 V94 H58 Z" />
<ellipse cx="50" cy="94" rx="15" ry="11.5" transform="rotate(-18 50 94)"/>
<ellipse cx="92" cy="84" rx="15" ry="11.5" transform="rotate(-18 92 84)"/></g>'''
tile("music", "#ff7a98", "#dc1f5c", music)

# ---------------------------------------------------------------- photos: a landscape
photos = '''<circle cx="92" cy="44" r="13" fill="#ffe27a"/>
<path d="M0 96 L38 52 L62 80 L82 62 L128 104 V128 H0 Z" fill="#2f4ea8"/>
<path d="M0 108 Q40 84 76 100 T128 98 V128 H0 Z" fill="#2fb27a"/>'''
tile("photos", "#8fd3ff", "#2f7be6", photos)

# ---------------------------------------------------------------- mail: an envelope
mail = '''<rect x="25" y="38" width="78" height="54" rx="10" fill="#fff"/>
<path d="M30 47 L64 72 L98 47" fill="none" stroke="#2c6be6" stroke-width="6.5" stroke-linecap="round" stroke-linejoin="round"/>'''
tile("mail", "#5cbcff", "#2363e8", mail)

# ---------------------------------------------------------------- calendar: a red header and the date
def seg_digit(d, x0, y0, w=22, h=40):
    mid = h / 2
    paths = {
        "2": f"M{x0} {y0} h{w} v{mid} h-{w} v{mid} h{w}",
        "7": f"M{x0} {y0} h{w} v{h}",
    }
    return f'<path d="{paths[d]}" fill="none" stroke="#1d1d1f" stroke-width="9" stroke-linecap="round" stroke-linejoin="round"/>'
cal = f'''<rect width="128" height="40" fill="url(#rd)"/>
<rect x="30" y="14" width="22" height="7" rx="3.5" fill="#fff" opacity=".9"/>
<rect x="76" y="14" width="22" height="7" rx="3.5" fill="#fff" opacity=".9"/>
{seg_digit("2", 36, 62)}{seg_digit("7", 70, 62)}'''
tile("calendar", "#ffffff", "#ececf1", cal,
     defs='<linearGradient id="rd" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#ff6459"/><stop offset="1" stop-color="#e5352b"/></linearGradient>',
     gloss=0.0)

# ---------------------------------------------------------------- notes: ruled paper
notes = '''<rect width="128" height="30" fill="#f7b92d" opacity=".65"/>
<g stroke="#a9780f" stroke-opacity=".5" stroke-width="5" stroke-linecap="round">
<path d="M30 56 H98 M30 74 H98 M30 92 H72"/></g>'''
tile("notes", "#ffe88a", "#ffc83d", notes)

# ---------------------------------------------------------------- video: a play button
video = '''<circle cx="64" cy="64" r="35" fill="#fff" fill-opacity=".96"/>
<path d="M55 46 L55 82 L86 64 Z" fill="#6a38e8" stroke="#6a38e8" stroke-width="7" stroke-linejoin="round"/>'''
tile("video", "#9a6bff", "#4e1fb5", video)

# ---------------------------------------------------------------- calculator: display and keys
keys = ""
for r in range(3):
    for c in range(4):
        col = "#ff9f0a" if c == 3 else "#6a6a70"
        keys += f'<rect x="{27 + c*19.5}" y="{58 + r*19}" width="15" height="15" rx="4.5" fill="{col}"/>\n'
calc = f'''<rect x="27" y="24" width="74" height="26" rx="7" fill="#0f0f11"/>
<rect x="68" y="33" width="26" height="8" rx="4" fill="#6dffb0" opacity=".9"/>
{keys}'''
tile("calculator", "#4c4c51", "#1d1d20", calc, gloss=0.3)

# ---------------------------------------------------------------- store: a shopping bag
store = '''<path d="M34 54 H94 L99 100 a7 7 0 0 1 -7 7 H36 a7 7 0 0 1 -7 -7 Z" fill="#fff"/>
<path d="M48 58 V48 a16 16 0 0 1 32 0 V58" fill="none" stroke="#fff" stroke-width="7" stroke-linecap="round"/>
<path d="M54 74 L64 84 L76 68" fill="none" stroke="#0aa573" stroke-width="7" stroke-linecap="round" stroke-linejoin="round"/>'''
tile("store", "#4be3a8", "#07935f", store)

# ---------------------------------------------------------------- editor: a pencil
editor = '''<g transform="rotate(45 64 64)">
<rect x="54" y="20" width="20" height="62" rx="4" fill="#fff"/>
<rect x="54" y="26" width="20" height="9" fill="#ffb59a"/>
<path d="M54 82 H74 L64 104 Z" fill="#ffe3cf"/>
<path d="M60 96 H68 L64 104 Z" fill="#3a2a22"/></g>'''
tile("editor", "#ffb067", "#ea4f1f", editor)

# ---------------------------------------------------------------- the stand-in for an app with no icon
tile("default", "#a6a6ac", "#5c5c62",
     '<rect x="34" y="34" width="60" height="60" rx="16" fill="none" stroke="#fff" stroke-width="8"/><circle cx="64" cy="64" r="8" fill="#fff"/>')

# ---------------------------------------------------------------- the bin: body, lid and paper are separate so the lid can swing
write("app/trash-body.svg", '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 128 128">
<defs>
<linearGradient id="b" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#f0f0f4" stop-opacity=".96"/><stop offset="1" stop-color="#a9a9b2" stop-opacity=".92"/></linearGradient>
<linearGradient id="s" x1="0" y1="0" x2="1" y2="0"><stop offset="0" stop-color="#fff" stop-opacity=".0"/><stop offset=".3" stop-color="#fff" stop-opacity=".55"/><stop offset=".5" stop-color="#fff" stop-opacity="0"/></linearGradient>
</defs>
<path d="M30 46 H98 L92 108 a9 9 0 0 1 -9 8 H45 a9 9 0 0 1 -9 -8 Z" fill="url(#b)" stroke="#6c6c74" stroke-opacity=".9" stroke-width="2.5" stroke-linejoin="round"/>
<path d="M30 46 H98 L92 108 a9 9 0 0 1 -9 8 H45 a9 9 0 0 1 -9 -8 Z" fill="url(#s)"/>
<g stroke="#6c6c74" stroke-opacity=".6" stroke-width="3.5" stroke-linecap="round"><path d="M52 60 V100 M64 60 V100 M76 60 V100"/></g>
</svg>
''')
write("app/trash-lid.svg", '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 128 128">
<defs><linearGradient id="l" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#f7f7fa"/><stop offset="1" stop-color="#b2b2ba"/></linearGradient></defs>
<rect x="50" y="22" width="28" height="12" rx="5" fill="url(#l)" stroke="#6c6c74" stroke-opacity=".9" stroke-width="2.5"/>
<rect x="24" y="32" width="80" height="14" rx="7" fill="url(#l)" stroke="#6c6c74" stroke-opacity=".9" stroke-width="2.5"/>
</svg>
''')
write("app/trash-paper.svg", '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 128 128">
<defs><linearGradient id="p" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#ffffff"/><stop offset="1" stop-color="#dcdce2"/></linearGradient></defs>
<g fill="url(#p)" stroke="#9a9aa4" stroke-width="2" stroke-linejoin="round">
<path d="M38 52 L50 30 L66 40 L72 56 Z"/><path d="M62 54 L74 26 L92 36 L94 56 Z"/><path d="M44 56 L52 42 L70 50 L72 60 Z"/></g>
<g fill="none" stroke="#9a9aa4" stroke-width="1.6" stroke-linecap="round"><path d="M52 32 L58 46 M76 30 L82 46 M48 54 L60 48"/></g>
</svg>
''')

# ---------------------------------------------------------------- symbols: one colour, stroked, tinted by the theme
def symbol(name, body):
    write(f"symbols/{name}.svg", f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="#000" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">{body}</svg>\n')

symbol("new-window", '<rect x="3.5" y="5" width="17" height="14" rx="3"/><path d="M12 9v6M9 12h6"/>')
symbol("pin", '<path d="M9 4h6l-1 5 3 3v1.5H7V12l3-3z"/><path d="M12 13.5V20"/>')
symbol("unpin", '<path d="M9 4h6l-1 5 3 3v1.5H7V12l3-3z"/><path d="M12 13.5V20M4 4l16 16"/>')
symbol("close", '<path d="M6 6l12 12M18 6L6 18"/>')
symbol("quit", '<path d="M12 3.5v8"/><path d="M7 6.8a7.5 7.5 0 1 0 10 0"/>')
symbol("windows", '<rect x="3.5" y="6.5" width="12" height="10" rx="2.5"/><path d="M8.5 6.5V5.5A2 2 0 0 1 10.5 3.5h7a2 2 0 0 1 2 2v7a2 2 0 0 1-2 2h-1"/>')
symbol("options", '<path d="M4 8h9M17 8h3M4 16h3M11 16h9"/><circle cx="15" cy="8" r="2"/><circle cx="9" cy="16" r="2"/>')
symbol("check", '<path d="M5 12.5l4.5 4.5L19 7.5"/>')
symbol("hide", '<path d="M3 12s3.5-6 9-6 9 6 9 6-3.5 6-9 6-9-6-9-6z"/><circle cx="12" cy="12" r="2.6"/><path d="M4 4l16 16"/>')
symbol("trash", '<path d="M5 7h14M10 7V5h4v2M7 7l1 12h8l1-12"/>')
symbol("empty", '<path d="M5 7h14M10 7V5h4v2M7 7l1 12h8l1-12"/><path d="M10 11v5M14 11v5"/>')
symbol("folder", '<path d="M3.5 7.5a2 2 0 0 1 2-2H10l2 2h6.5a2 2 0 0 1 2 2V17a2 2 0 0 1-2 2h-13a2 2 0 0 1-2-2z"/>')
symbol("add", '<path d="M12 5v14M5 12h14"/>')

print("icons written to", os.path.abspath(OUT))
