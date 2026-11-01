#!/usr/bin/env python3
"""Turns a recording (tools/record-demo.py) into what can be looked at: a filmstrip for each thing the script
did (frames with the time since that thing began), and a video and a GIF of the whole run.

    tools/make-filmstrips.py rec-dir out-dir [name] [--no-video]

`name` is put in front of the files ("light" gives film-menu-light.png and menubar-demo-light.mp4)."""
import json
import os
import re
import shutil
import subprocess
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFont, ImageStat

video = "--no-video" not in sys.argv  # (the strips alone take a few seconds; the video and the GIF a few minutes)
args = [a for a in sys.argv[1:] if a != "--no-video"]
rec, out = args[0], args[1]
name = args[2] if len(args) > 2 else ""
suffix = "-" + name if name else ""
os.makedirs(out, exist_ok=True)
frames_dir = os.path.join(rec, "frames")
data = json.load(open(os.path.join(rec, "phases.json")))
phases = data["phases"]
events = []
for line in open(os.path.join(rec, "events.txt")):
    ms, what = line.strip().split(" ", 1)
    events.append((int(ms), what))
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
        if ImageStat.Stat(ImageChops.difference(ref, im)).mean[0] > threshold:
            return t
    return start_ms


def event(what, nth=0):
    return [ms for ms, w in events if w == what][nth]


def rel(base, offsets, label=""):
    """Frames at base + each offset, labelled with the offset."""
    return [(base + o, "%s%+d ms" % (label, o)) for o in offsets]


def wrap(text, font, width, draw):
    lines, line = [], ""
    for word in text.split(" "):
        trial = (line + " " + word).strip()
        if line and draw.textlength(trial, font=font) > width:
            lines.append(line)
            line = word
        else:
            line = trial
    lines.append(line)
    return lines


def strip(file, title, frames, crop, columns=2, scale=0.8):
    """frames: [(time in ms, label)]."""
    cw, ch = crop[2] - crop[0], crop[3] - crop[1]
    w, h = int(cw * scale), int(ch * scale)
    rows = (len(frames) + columns - 1) // columns
    pad = 10
    width = columns * (w + pad) + pad
    lines = wrap(title, BIG, width - 2 * pad, ImageDraw.Draw(Image.new("RGB", (1, 1))))
    head = 14 + 26 * len(lines) + 8
    sheet = Image.new("RGB", (width, head + rows * (h + 26 + pad) + pad), (24, 24, 28))
    d = ImageDraw.Draw(sheet)
    for n, line in enumerate(lines):
        d.text((pad, 12 + 26 * n), line, font=BIG, fill=(255, 255, 255))
    for k, (ms, label) in enumerate(frames):
        im, _ = frame_at(ms)
        im = im.crop(crop).resize((w, h), Image.LANCZOS)
        x = pad + (k % columns) * (w + pad)
        y = head + (k // columns) * (h + 26 + pad)
        sheet.paste(im, (x, y + 22))
        d.text((x, y), label, font=FONT, fill=(190, 190, 200))
    path = os.path.join(out, file.replace(".png", suffix + ".png"))
    sheet.save(path)
    print("wrote", os.path.basename(path))


# ---- each thing the script did
BAR_LEFT = (0, 0, 560, 340)
opened = first_change(phases["menu"] + 300, (0, 30, 400, 330), 0.5)
strip("film-menu.png", "A click on File: the menu grows out of its title, rows arriving 14 ms apart (times from the first change)",
      rel(opened - 40, [-40, 0, 40, 80, 120, 200, 400, 800]), BAR_LEFT, columns=4, scale=0.5)

# Two switches with a menu open: File to Edit, then Edit to View.
MENUS = (0, 30, 500, 330)
sw1 = first_change(phases["sweep"] + 100, MENUS, 1.0)
sw2 = first_change(phases["sweep"] + 1000, MENUS, 1.0, reference_ms=phases["sweep"] + 950)
SW = [-40, 0, 40, 80, 120, 240]
strip("film-sweep.png", "Along the bar with a menu open: each menu gives way to the next, gliding to the title it hangs from",
      rel(sw1, SW, "to Edit ") + rel(sw2, SW, "to View "), (0, 0, 560, 240), columns=3, scale=0.55)

strip("film-flyout.png", "A row with a flyout: rest on it, the flyout comes, move in, choose; both close",
      rel(phases["flyout"] + 300, [0, 500, 800, 1100, 1400, 1700, 2000, 2400, 2800, 3200, 3600, 4000]), (0, 0, 600, 260), columns=3, scale=0.55)

wifi_open = first_change(phases["wifi"] + 400, (900, 30, 1440, 340), 1.0)
strip("film-wifi.png", "The Wi-Fi popover grows out of its icon; a network is chosen, spins, and connects",
      rel(wifi_open - 40, [0, 40, 80, 120, 200, 300, 500, 1000, 1700, 2100, 2900, 3600]), (880, 0, 1440, 400), columns=3, scale=0.6)

hover1 = first_change(phases["hover"] + 350, (880, 30, 1440, 340), 1.0)
strip("film-hover.png", "Along the bar from Wi-Fi to the Control Center: each popover gives way to the next, sliding between",
      rel(hover1 - 60, [0, 80, 160, 240, 320, 400, 500, 600, 750, 900]), (880, 0, 1440, 560), columns=5, scale=0.45)

strip("film-control.png", "The Control Center: a slider dragged, a tile pressed",
      rel(phases["control"] + 400, [0, 300, 600, 900, 1200, 1500, 1800, 2100]), (1020, 0, 1440, 560), columns=4, scale=0.55)

toast = event("notify:mail")
strip("film-toast.png", "A notification arrives: the banner springs in from the right, waits, and goes by itself (5.2 s)",
      rel(toast - 20, [0, 60, 120, 200, 300, 500, 1500, 5300, 5450, 5600, 5750, 6000]), (1000, 0, 1440, 260), columns=4, scale=0.55)

vol = event("volume:+0.1")
strip("film-osd.png", "The volume keys: the display comes up, steps with each key, then mute, and goes by itself",
      rel(vol - 20, [0, 100, 250, 450, 700, 1000, 1400, 1700, 1900, 2200, 2800, 3300]), (520, 600, 920, 860), columns=4, scale=0.55)

CENTRE = (1000, 30, 1440, 900)
cen = first_change(phases["centre"] + 300, CENTRE, 2.0)
strip("film-centre.png", "The notification centre slides in from the right: the weather, the calendar and the player, then the notifications",
      rel(cen - 40, [0, 50, 100, 160, 250, 450]), (1000, 0, 1440, 900), columns=3, scale=0.4)

# A card pulled sideways (the drag begins at `pull`, and is let go about 0.6 s later): it slides off and fades, the
# rows close up, and the groups sort again (the group whose newest card went now has an older newest).
pull = first_change(phases["centre"] + 3000, (1000, 560, 1440, 900), 3.0, reference_ms=phases["centre"] + 3000)
strip("film-dismiss.png", "A notification pulled to the side: it slides away and fades, the rows close up, the group counts one less and the groups sort again",
      rel(pull, [-80, 300, 600, 750, 900, 1050, 1250, 1600]), (1032, 440, 1440, 780), columns=4, scale=0.6)

strip("film-power.png", "Restart…: the screen goes to frosted glass and counts down; a click on the glass cancels",
      rel(phases["power"] + 900, [0, 400, 800, 1200, 1600, 2400, 3600, 4400, 4800, 5000, 5200, 5400]), (0, 0, 1440, 900), columns=4, scale=0.24)

if not video:
    sys.exit(0)

# ---- video and GIF of the whole run, at a steady rate (the recording's frames are about 40 ms apart
# but not exactly).
tmp = os.path.join(out, "_seq" + suffix)
shutil.rmtree(tmp, ignore_errors=True)
os.makedirs(tmp)
end = times[-1]
n = 0
t = 0
while t <= end:
    im, _ = frame_at(t)
    im.save(os.path.join(tmp, "%05d.png" % n), compress_level=1)
    n += 1
    t += 40
subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-framerate", "25", "-i", os.path.join(tmp, "%05d.png"), "-c:v", "libx264",
                "-preset", "veryfast", "-crf", "20", "-pix_fmt", "yuv420p", os.path.join(out, "menubar-demo%s.mp4" % suffix)], check=True)
print("wrote menubar-demo%s.mp4" % suffix)
palette = os.path.join(tmp, "palette.png")
filters = "fps=10,scale=640:-1:flags=lanczos"
subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-framerate", "25", "-i", os.path.join(tmp, "%05d.png"), "-vf",
                filters + ",palettegen=max_colors=128:stats_mode=diff", palette], check=True)
subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-framerate", "25", "-i", os.path.join(tmp, "%05d.png"), "-i", palette,
                "-lavfi", filters + "[x];[x][1:v]paletteuse=dither=bayer:bayer_scale=4:diff_mode=rectangle",
                os.path.join(out, "menubar-demo%s.gif" % suffix)], check=True)
print("wrote menubar-demo%s.gif" % suffix)
shutil.rmtree(tmp)
