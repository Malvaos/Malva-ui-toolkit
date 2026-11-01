#!/usr/bin/env python3
"""Turns a recording (tools/record-demo.py) into what can be looked at: a filmstrip for each thing
the script did (frames with the time since that thing began), a video and a GIF of the whole run.

    tools/make-filmstrips.py rec-dir out-dir"""
import json
import os
import re
import shutil
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFont

rec, out = sys.argv[1], sys.argv[2]
os.makedirs(out, exist_ok=True)
frames_dir = os.path.join(rec, "frames")
phases = json.load(open(os.path.join(rec, "phases.json")))
files = sorted(os.listdir(frames_dir))
times = [int(re.search(r"-(\d+)ms", f).group(1)) for f in files]

try:
    FONT = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 15)
    BIG = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20)
except OSError:
    FONT = BIG = ImageFont.load_default()


def frame_at(ms):
    """The frame on screen at ms: the last one taken by then."""
    best = 0
    for i, t in enumerate(times):
        if t <= ms:
            best = i
        else:
            break
    return Image.open(os.path.join(frames_dir, files[best])).convert("RGB"), times[best]


def first_change(start_ms, region, threshold=1.5, reference_ms=None):
    """The time of the first frame after start_ms whose region differs from the one at reference_ms."""
    ref, _ = frame_at(start_ms if reference_ms is None else reference_ms)
    ref = ref.crop(region).convert("L")
    for t, f in zip(times, files):
        if t < start_ms:
            continue
        im = Image.open(os.path.join(frames_dir, f)).convert("L").crop(region)
        diff = sum(abs(a - b) for a, b in zip(ref.getdata(), im.getdata())) / (im.width * im.height)
        if diff > threshold:
            return t
    return start_ms


def strip(name, title, phase, offsets, crop, columns=2, scale=0.8, base=None):
    cw, ch = crop[2] - crop[0], crop[3] - crop[1]
    w, h = int(cw * scale), int(ch * scale)
    rows = (len(offsets) + columns - 1) // columns
    pad, head = 10, 44
    sheet = Image.new("RGB", (columns * (w + pad) + pad, head + rows * (h + 26 + pad) + pad), (24, 24, 28))
    d = ImageDraw.Draw(sheet)
    d.text((pad, 10), title, font=BIG, fill=(255, 255, 255))
    for k, off in enumerate(offsets):
        im, taken = frame_at((phases[phase] if base is None else base) + off)
        im = im.crop(crop).resize((w, h), Image.LANCZOS)
        x = pad + (k % columns) * (w + pad)
        y = head + (k // columns) * (h + 26 + pad)
        sheet.paste(im, (x, y + 22))
        d.text((x, y), "+%d ms" % off, font=FONT, fill=(190, 190, 200))
    sheet.save(os.path.join(out, name))
    print("wrote", name)


DOCK = (130, 335, 970, 440)
strip("film-entrance.png", "The dock arrives: icons rise out of the glass one after another, 28 ms apart", "entrance",
      [0, 60, 120, 180, 240, 300, 380, 520], DOCK)
strip("film-hover.png", "The pointer sweeps along the dock: icons grow by a cosine of the distance, the label glides", "sweep",
      [150, 550, 950, 1350, 1750, 2150], (100, 250, 1000, 440), columns=2, scale=0.62)
strip("film-launch.png", "A click on an app that is not running: it hops until it is up, then its mark appears", "launch",
      [450, 700, 900, 1100, 1400, 1800, 2300, 2900], (260, 240, 860, 440), columns=2, scale=0.78)
opened = first_change(phases["menu"] + 300, (200, 100, 440, 180), 0.5)
strip("film-menu.png", "A right click: the menu grows out of its icon, rows arriving 16 ms apart (times from the click)", "menu",
      [-80, 0, 40, 80, 120, 200, 400, 800], (100, 90, 700, 440), columns=4, scale=0.5, base=opened - 40)
strip("film-reorder.png", "Dragging Notes three places left: the icons make way, it settles where it lands", "reorder",
      [450, 700, 900, 1100, 1300, 1550, 1900, 2300], DOCK)
strip("film-remove.png", "Dragging Settings off the dock: it fades as it is pulled away, the others close up", "remove",
      [500, 850, 1050, 1250, 1450, 1700, 2000, 2400], (130, 200, 970, 440), columns=2, scale=0.75)

# ---- video and GIF of the whole run, at a steady rate (the recording's frames are about 40 ms apart
# but not exactly).
tmp = os.path.join(out, "_seq")
shutil.rmtree(tmp, ignore_errors=True)
os.makedirs(tmp)
end = times[-1]
n = 0
t = 0
while t <= end:
    im, _ = frame_at(t)
    im.save(os.path.join(tmp, "%05d.png" % n))
    n += 1
    t += 40
subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-framerate", "25", "-i", os.path.join(tmp, "%05d.png"), "-c:v", "libx264",
                "-preset", "veryfast", "-crf", "20", "-pix_fmt", "yuv420p", os.path.join(out, "dock-demo.mp4")], check=True)
print("wrote dock-demo.mp4")
palette = os.path.join(tmp, "palette.png")
filters = "fps=12.5,scale=720:-1:flags=lanczos"
subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-framerate", "25", "-i", os.path.join(tmp, "%05d.png"), "-vf",
                filters + ",palettegen=max_colors=160:stats_mode=diff", palette], check=True)
subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-framerate", "25", "-i", os.path.join(tmp, "%05d.png"), "-i", palette,
                "-lavfi", filters + "[x];[x][1:v]paletteuse=dither=bayer:bayer_scale=4:diff_mode=rectangle",
                os.path.join(out, "dock-demo.gif")], check=True)
print("wrote dock-demo.gif")
shutil.rmtree(tmp)
