#!/usr/bin/env python3
"""Films the dock under a virtual X server while a real (XTest) pointer uses it, and writes the
frames and when each phase of the script began. The dock records itself (its --record mode takes a
snapshot of its own window every frame), so what is filmed is what the renderer drew.

    tools/record-demo.py build/dock out-dir [--dark] [--edge=bottom]

Needs Xvfb. The script is timed in real time; a slow machine drops frames, never events."""
import json
import math
import os
import re
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from xinput import LEFT, RIGHT, Pointer  # noqa: E402

dock = sys.argv[1]
out = sys.argv[2]
extra = [a for a in sys.argv[3:] if a.startswith("--")]
dark = "--dark" in extra
W, H = 1100, 440
ICON, GAP = 56, 7
UNIT = ICON + GAP
N_APPS, N_MIN = 9, 1
SEP = ICON * 0.3
PAD_ALONG = ICON * 0.25
LAUNCHER = "--no-launcher" not in extra
REST = 2 * PAD_ALONG + (UNIT if LAUNCHER else 0) + (SEP if LAUNCHER else 0) + N_APPS * UNIT + SEP + N_MIN * UNIT + UNIT
APPS_START = W / 2 - REST / 2 + PAD_ALONG + ((UNIT + SEP) if LAUNCHER else 0)
ICON_Y = H - 10 - ICON * 0.2 - ICON / 2


def app_x(i):
    return APPS_START + i * UNIT + UNIT / 2


def trash_x():
    return APPS_START + N_APPS * UNIT + SEP + N_MIN * UNIT + UNIT / 2


os.makedirs(out, exist_ok=True)
display = ":%d" % (60 + os.getpid() % 30)
xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "1440x900x24", "-nolisten", "tcp"],
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
time.sleep(1.0)
env = dict(os.environ, DISPLAY=display, SLINT_BACKEND="winit-femtovg")
frames = os.path.join(out, "frames")
args = [dock, "--record=" + frames, "--fps=25", "--seconds=17", "--size=%dx%d" % (W, H), "--launch-ms=1400",
        "--desk=#7b7bc4"] + extra
app = subprocess.Popen(args, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
phases = {}
try:
    # Frame 0 is the recording's zero.
    deadline = time.time() + 20
    while not (os.path.isdir(frames) and any(f.startswith("frame-00000") for f in os.listdir(frames))):
        if time.time() > deadline:
            raise RuntimeError("the dock never started recording")
        time.sleep(0.01)
    t0 = time.monotonic()
    # The recording's clock started a little before frame 0 was written: its name says when.
    first = sorted(f for f in os.listdir(frames) if f.startswith("frame-00000"))[0]
    offset = int(re.search(r"-(\d+)ms", first).group(1))

    def mark(name):
        # In the recording's own milliseconds, as in the frames' names.
        phases[name] = int((time.monotonic() - t0) * 1000) + offset

    def at(seconds):
        wait = t0 + seconds - time.monotonic()
        if wait > 0:
            time.sleep(wait)

    p = Pointer(display)
    p.move(W / 2, 90)

    mark("entrance")                      # the icons arrive one after another
    at(1.9)
    mark("approach")                      # the pointer comes down to the dock from above
    p.glide(app_x(0), ICON_Y, 0.5)
    mark("sweep")                         # along the dock and back: the icons grow and part
    p.glide(app_x(8), ICON_Y, 2.2)
    p.glide(app_x(4), ICON_Y, 0.9)
    at(5.7)
    mark("leave")                         # off the dock: it settles
    p.glide(W / 2, 120, 0.5)
    at(6.7)
    mark("launch")                        # a click on an app that is not running: it hops, then runs
    p.glide(app_x(3), ICON_Y, 0.45)
    time.sleep(0.25)
    p.click(LEFT)
    p.glide(app_x(3), ICON_Y - 4, 1.4, hz=20)
    at(9.0)
    mark("menu")                          # a right click: the menu grows out of the icon
    p.glide(app_x(1), ICON_Y, 0.4)
    time.sleep(0.2)
    p.click(RIGHT)
    time.sleep(1.1)
    at(10.8)
    mark("dismiss")                       # a click on the desk closes it
    p.glide(W / 2 - 300, 120, 0.4)
    p.click(LEFT)
    time.sleep(0.4)
    at(11.8)
    mark("reorder")                       # take Notes and carry it three places left: the others make way
    p.glide(app_x(6), ICON_Y, 0.4)
    time.sleep(0.3)
    p.button(LEFT, True)
    time.sleep(0.12)
    p.glide(app_x(3) - 6, ICON_Y - 10, 1.0)
    time.sleep(0.35)
    p.button(LEFT, False)
    at(14.0)
    mark("remove")                        # take Settings off the dock
    p.glide(app_x(8), ICON_Y, 0.4)
    time.sleep(0.2)
    p.button(LEFT, True)
    time.sleep(0.12)
    p.glide(app_x(8) + 20, ICON_Y - 190, 0.7)
    time.sleep(0.2)
    p.button(LEFT, False)
    at(15.8)
    mark("end")
    app.wait(timeout=30)
finally:
    for proc in (app, xvfb):
        try:
            proc.terminate()
        except Exception:
            pass
with open(os.path.join(out, "phases.json"), "w") as f:
    json.dump(phases, f, indent=1)
print(json.dumps(phases))
