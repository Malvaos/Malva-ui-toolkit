#!/usr/bin/env python3
"""Writes the two Malva desktop wallpapers (svg/wallpaper/malva-light.svg, malva-dark.svg): a silver (light) or graphite (dark) base with one soft bloom of the logo's colours, 1920x1200. Used by the app previews and as shipped artwork.

    python3 tools/make_wallpapers.py"""
import os

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

SCENES = {
    # Silver and graphite, with one soft bloom of the logo's colours low in the frame: the neutrals stay in charge.
    "light": dict(base=("#F7F7F9", "#ECECF0", "#E3E3E9"),
                  blobs=[("a", 0.16, 0.10, 0.55, "#FFFFFF", 0.95), ("b", 0.84, 1.02, 0.60, "#B84F9F", 0.22), ("c", 0.50, 1.05, 0.55, "#C763B0", 0.14), ("d", 0.96, 0.10, 0.40, "#D6D6DE", 0.70)]),
    "dark": dict(base=("#1B1B1D", "#141415", "#0F0F10"),
                 blobs=[("a", 0.16, 0.08, 0.52, "#3A3A3F", 0.60), ("b", 0.84, 1.02, 0.62, "#8C3274", 0.34), ("c", 0.50, 1.06, 0.52, "#B84F9F", 0.18), ("d", 0.96, 0.10, 0.36, "#2A2A2E", 0.70)]),
}


def svg(name, s):
    W, H = 1920, 1200
    defs = [f'<linearGradient id="base" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="{s["base"][0]}"/><stop offset="0.55" stop-color="{s["base"][1]}"/><stop offset="1" stop-color="{s["base"][2]}"/></linearGradient>']
    rects = ['<rect width="1920" height="1200" fill="url(#base)"/>']
    for ident, cx, cy, r, col, op in s["blobs"]:
        defs.append(f'<radialGradient id="{ident}" cx="{cx}" cy="{cy}" r="{r}"><stop offset="0" stop-color="{col}" stop-opacity="{op}"/><stop offset="1" stop-color="{col}" stop-opacity="0"/></radialGradient>')
        rects.append(f'<rect width="1920" height="1200" fill="url(#{ident})"/>')
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">\n  <defs>\n    ' + "\n    ".join(defs)
            + "\n  </defs>\n  " + "\n  ".join(rects) + "\n</svg>\n")


os.makedirs(os.path.join(ROOT, "svg", "wallpaper"), exist_ok=True)
for name, s in SCENES.items():
    open(os.path.join(ROOT, "svg", "wallpaper", f"malva-{name}.svg"), "w").write(svg(name, s))
print("wallpapers written")
