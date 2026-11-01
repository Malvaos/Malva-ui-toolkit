#!/usr/bin/env python3
"""Draws the two wallpapers the previews sit on (soft light, ridges, grain) and, for each, the
copy of the finished desktop blurred behind the dock's glass (written by render-previews.sh,
which calls blur() below). Nothing here ships."""
import sys, numpy as np
from PIL import Image, ImageFilter

W, H = 1440, 900
rng = np.random.default_rng(7)

def lerp(a, b, t):
    return a + (b - a) * t[..., None]

def blob(img, cx, cy, r, color, strength):
    y, x = np.mgrid[0:H, 0:W]
    d = np.sqrt(((x - cx * W) / (r * W)) ** 2 + ((y - cy * H) / (r * W)) ** 2)
    a = np.clip(1 - d, 0, 1) ** 2 * strength
    return img * (1 - a[..., None]) + np.array(color, float) * a[..., None]

def ridge(img, base, amp, freq, phase, color, alpha, haze=0.0):
    y, x = np.mgrid[0:H, 0:W]
    xs = x / W
    h = base * H + amp * H * (np.sin(xs * freq + phase) * 0.55 + np.sin(xs * freq * 2.3 + phase * 1.7) * 0.3 + np.sin(xs * freq * 4.1 + phase * 0.4) * 0.15)
    m = np.clip((y - h) / 1.6 + 0.5, 0, 1)
    fade = 1 - haze * np.clip((y - h) / (0.3 * H), 0, 1)
    a = (m * alpha * fade)[..., None]
    return img * (1 - a) + np.array(color, float) * a

def make(name, dark):
    yy = np.linspace(0, 1, H)[:, None] * np.ones((1, W))
    xx = np.ones((H, 1)) * np.linspace(0, 1, W)[None, :]
    t = np.clip(yy * 0.8 + xx * 0.25, 0, 1)
    if not dark:
        top, mid, bot = np.array([247, 247, 249.]), np.array([236, 236, 240.]), np.array([226, 226, 233.])
    else:
        top, mid, bot = np.array([29, 29, 31.]), np.array([20, 20, 21.]), np.array([15, 15, 16.])
    img = np.where(t[..., None] < 0.5, lerp(top, mid, np.clip(t * 2, 0, 1)), lerp(mid, bot, np.clip(t * 2 - 1, 0, 1)))
    # Silver and graphite, with one soft bloom of the logo's colours low in the frame: the neutrals stay in charge.
    if not dark:
        img = blob(img, 0.16, 0.10, 0.55, (255, 255, 255), 0.85)
        img = blob(img, 0.86, 1.02, 0.60, (184, 79, 159), 0.24)
        img = blob(img, 0.50, 1.06, 0.60, (199, 99, 176), 0.14)
        img = ridge(img, 0.66, 0.05, 6.5, 0.4, (222, 222, 230), 0.70, 0.25)
        img = ridge(img, 0.76, 0.05, 5.0, 2.1, (226, 208, 224), 0.62, 0.20)
        img = ridge(img, 0.88, 0.04, 7.5, 4.0, (206, 200, 212), 0.60, 0.0)
    else:
        img = blob(img, 0.16, 0.08, 0.52, (62, 62, 68), 0.60)
        img = blob(img, 0.86, 1.02, 0.62, (140, 50, 116), 0.34)
        img = blob(img, 0.50, 1.06, 0.55, (184, 79, 159), 0.18)
        img = ridge(img, 0.66, 0.05, 6.5, 0.4, (36, 36, 40), 0.75, 0.3)
        img = ridge(img, 0.76, 0.05, 5.0, 2.1, (29, 26, 33), 0.80, 0.2)
        img = ridge(img, 0.88, 0.04, 7.5, 4.0, (21, 21, 23), 0.88, 0.0)
    img += rng.normal(0, 1.4, img.shape)
    Image.fromarray(np.clip(img, 0, 255).astype(np.uint8)).save(f"{name}.png")

def blur(src, dst, dark, radius=26):
    """What the glass shows behind itself: the desktop, blurred, a touch more saturated."""
    im = Image.open(src).convert("RGB").filter(ImageFilter.GaussianBlur(radius))
    a = np.asarray(im, float)
    g = a.mean(axis=2, keepdims=True)
    a = g + (a - g) * 1.25
    a += rng.normal(0, 2.0, a.shape)
    Image.fromarray(np.clip(a, 0, 255).astype(np.uint8)).save(dst)

if __name__ == "__main__":
    if len(sys.argv) >= 2 and sys.argv[1] == "blur":
        blur(sys.argv[2], sys.argv[3], sys.argv[4] == "dark")
    else:
        out = sys.argv[1] if len(sys.argv) > 1 else "."
        make(f"{out}/wallpaper-light", False)
        make(f"{out}/wallpaper-dark", True)
