#!/usr/bin/env python3
"""Films the menu bar under a virtual X server while a real (XTest) pointer uses it, and writes the
frames and when each phase of the script began. The app records itself (its --record mode takes a
snapshot of its own window every frame and puts it over the desktop picture, as a compositor would), so
what is filmed is what the renderer drew. Things the system does on its own (a notification coming in, the
volume keys) are scheduled in the app (--at) and written to events.txt with the time they happened.

    tools/record-demo.py build/menubar out-dir [--dark]

Needs Xvfb, python3 with pillow. The script is timed in real time; a slow machine drops frames, never events.

Where things are on the bar is not assumed: before filming, the script sweeps the pointer along it with a
menu open (which opens whatever it arrives on) and reads what the app says it opened ("--log-popups")."""
import json
import os
import re
import subprocess
import sys
import threading
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from xinput import LEFT, Pointer  # noqa: E402
from PIL import Image  # noqa: E402

app_path = os.path.abspath(sys.argv[1])
out = os.path.abspath(sys.argv[2])
dark = "--dark" in sys.argv[3:]
root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
W, H = 1440, 900
BAR_Y = 15
mode = "dark" if dark else "light"

os.makedirs(out, exist_ok=True)
under_ppm = os.path.join(out, "desktop-%s.ppm" % mode)
Image.open(os.path.join(root, "preview/assets/desktop-%s.png" % mode)).convert("RGB").save(under_ppm)
common = ["--size=%dx%d" % (W, H), "--under=" + under_ppm, "--backdrop=" + os.path.join(root, "preview/assets/backdrop-%s.png" % mode),
          "--connect-ms=1100", "--power-seconds=8"] + (["--dark"] if dark else [])

display = ":%d" % (60 + os.getpid() % 30)
xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", "%dx%dx24" % (W, H), "-nolisten", "tcp"],
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
time.sleep(1.0)
env = dict(os.environ, DISPLAY=display, SLINT_BACKEND="winit-femtovg")


def clamp(v, lo, hi):
    return max(lo, min(hi, v))


# ---------------------------------------------------------------------------------------- the bar's places
def measure_bar():
    """Sweeps the pointer along the bar with the calendar open; each item opens as the pointer arrives. An
    item's centre is half way between where it opened and where the next did."""
    app = subprocess.Popen([app_path, "--popup=clock", "--log-popups"] + common, env=env, stdout=subprocess.PIPE,
                           stderr=subprocess.DEVNULL, text=True)
    lines = []

    def reader():
        for line in app.stdout:
            lines.append((time.monotonic(), line.strip()))

    threading.Thread(target=reader, daemon=True).start()
    deadline = time.time() + 20
    while not any(l == "popup: clock" for _, l in lines):
        if time.time() > deadline:
            raise RuntimeError("the app never opened its calendar")
        time.sleep(0.05)
    time.sleep(1.0)
    p = Pointer(display)
    steps = []
    p.move(0, BAR_Y)
    time.sleep(0.3)
    for x in range(0, W, 3):
        p.move(x, BAR_Y)
        steps.append((time.monotonic(), x))
        time.sleep(0.035)
    time.sleep(0.3)
    p.close()
    app.terminate()
    app.wait(timeout=10)
    opened = []
    for t, line in lines:
        m = re.match(r"popup: (.+)$", line)
        if not m or t < steps[0][0]:
            continue
        before = [x for ts, x in steps if ts <= t - 0.02]
        opened.append((m.group(1), before[-1] if before else 0))
    # The first thing opened in the sweep is the first item; the clock (open at the start) opens again at its own place.
    first = {}
    order = []
    for name, x in opened:
        if name not in first:
            first[name] = x
            order.append(name)
    centres = {}
    for i, name in enumerate(order):
        # (An item is no wider than about 70 px: after the last of the menus there is a gap to the tray.)
        end = min(first[order[i + 1]], first[name] + 70) if i + 1 < len(order) else first[name] + 70
        centres[name] = (first[name] + end) / 2 - 1
    lefts = dict(first)
    return order, centres, lefts


order, C, L = measure_bar()
expect = ["system", "menu-0", "menu-1", "menu-2", "menu-3", "menu-4", "menu-5", "menu-6", "tray-0", "tray-1", "layout", "bluetooth", "wifi",
          "volume", "battery", "control", "bell", "clock"]
if order != expect:
    raise RuntimeError("the bar's items were not found as expected: %s" % order)
print("bar:", {k: round(v) for k, v in C.items()})


# ---------------------------------------------------------------------------------------- geometry of what opens
KIND = {  # the rows of each menu that are in the card itself (not in a flyout): i item, s separator, h header
    "system": "isiiisisiiisii",
    "menu-1": "iiisisiisii",
    "menu-2": "iisiiisii",
    "menu-3": "iiisiisi",
}


def row_y(menu, i):
    y = 34 + 6
    for k in KIND[menu][:i]:
        y += {"i": 28, "s": 11, "h": 26}[k]
    return y + {"i": 28, "s": 11, "h": 26}[KIND[menu][i]] / 2


def menu_x(name):
    return max(L[name] - 4, 6)


# ---------------------------------------------------------------------------------------- the film
LENGTH = 40.0
frames_root = os.path.join(out, "frames")
for stale in (frames_root, os.path.join(out, "events.txt")):
    if os.path.isdir(stale):
        subprocess.run(["rm", "-rf", stale])
args = [app_path, "--record=" + out, "--fps=25", "--seconds=%g" % (LENGTH + 1.5), "--log-popups"] + common + [
    "--at=20.3:notify:mail", "--at=23.0:volume:+0.1", "--at=23.4:volume:+0.1", "--at=23.8:volume:+0.1", "--at=24.6:mute"]
app = subprocess.Popen(args, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
phases = {}
try:
    deadline = time.time() + 30
    while not (os.path.isdir(frames_root) and any(f.startswith("frame-00000") for f in os.listdir(frames_root))):
        if time.time() > deadline:
            raise RuntimeError("the app never started recording")
        time.sleep(0.01)
    t0 = time.monotonic()
    first = sorted(f for f in os.listdir(frames_root) if f.startswith("frame-00000"))[0]
    offset = int(re.search(r"-(\d+)ms", first).group(1))

    def mark(name):
        phases[name] = int((time.monotonic() - t0) * 1000) + offset

    def at(seconds):
        wait = t0 + seconds - time.monotonic()
        if wait > 0:
            time.sleep(wait)

    p = Pointer(display)
    p.move(700, 520)

    mark("rest")
    at(1.0)
    mark("menu")                          # a click on File: its menu grows out of the title
    p.glide(C["menu-1"], BAR_Y, 0.55)
    time.sleep(0.2)
    p.click(LEFT)
    at(3.1)
    mark("sweep")                         # along the bar: each menu gives way to the next
    p.glide(C["menu-2"], BAR_Y, 0.5)
    time.sleep(0.55)
    p.glide(C["menu-3"], BAR_Y, 0.5)
    time.sleep(0.5)
    at(5.0)
    mark("flyout")                        # rest on Sort By: its flyout comes; move in, choose
    mx = menu_x("menu-3")
    p.glide(mx + 50, row_y("menu-3", 4), 0.6)
    time.sleep(0.9)
    p.glide(mx + 290, row_y("menu-3", 4), 0.45)
    p.glide(mx + 290, row_y("menu-3", 4) + 28, 0.25)
    time.sleep(0.5)
    p.click(LEFT)
    time.sleep(0.4)
    at(8.4)
    mark("wifi")                          # the Wi-Fi popover; a network connects
    p.glide(C["wifi"], BAR_Y, 0.8)
    time.sleep(0.2)
    p.click(LEFT)
    time.sleep(0.9)
    card = clamp(C["wifi"] - 165, 8, W - 330 - 8)
    p.glide(card + 130, 36 + 61 + 46 + 23, 0.5)
    time.sleep(0.3)
    p.click(LEFT)
    time.sleep(2.0)
    at(12.9)
    mark("hover")                         # along the bar to the Control Center: popovers give way to each other
    p.glide(C["wifi"], BAR_Y, 0.4)         # up onto its icon (the popover stays), then along the bar
    p.glide(C["control"], BAR_Y, 0.9)
    time.sleep(0.5)
    at(14.8)
    mark("control")                       # drag the brightness, switch Night Light off
    cc = clamp(C["control"] - 180, 8, W - 360 - 8)
    slider_y = 36 + 12 + 196 + 10 + 24 + 19
    p.glide(cc + 12 + 0.72 * 336, slider_y, 0.5)
    time.sleep(0.25)
    p.button(LEFT, True)
    time.sleep(0.1)
    p.glide(cc + 12 + 0.30 * 336, slider_y, 0.8)
    time.sleep(0.15)
    p.button(LEFT, False)
    time.sleep(0.4)
    p.glide(cc + 12 + 172 + 82, 36 + 12 + 2 * 68 + 30, 0.5)
    time.sleep(0.2)
    p.click(LEFT)
    time.sleep(0.9)
    at(18.4)
    mark("dismiss")                       # a click on the desk closes it
    p.glide(500, 640, 0.5)
    time.sleep(0.15)
    p.click(LEFT)
    # 20.3: a notification comes in by itself (the Control Center has finished closing); 23.0 on: the volume keys, then mute
    at(26.6)
    mark("centre")                        # the notification centre: slides in, a card is pulled away
    p.glide(C["bell"], BAR_Y, 0.6)
    time.sleep(0.15)
    p.click(LEFT)
    time.sleep(1.5)
    p.glide(W - 408 + 150, 610, 0.5)
    time.sleep(0.3)
    p.button(LEFT, True)
    time.sleep(0.1)
    p.glide(W - 408 + 340, 610, 0.5)
    time.sleep(0.1)
    p.button(LEFT, False)
    time.sleep(1.2)
    at(31.8)
    mark("close")
    p.glide(C["bell"], BAR_Y, 0.5)
    time.sleep(0.1)
    p.click(LEFT)
    time.sleep(0.4)
    at(33.2)
    mark("power")                         # the system menu, Restart…: a countdown, which is cancelled
    p.glide(C["system"], BAR_Y, 0.6)
    time.sleep(0.15)
    p.click(LEFT)
    time.sleep(0.8)
    p.glide(menu_x("system") + 60, row_y("system", 9), 0.5)
    time.sleep(0.4)
    p.click(LEFT)
    time.sleep(3.0)
    p.glide(220, 700, 0.5)
    p.click(LEFT)
    at(LENGTH)
    mark("end")
    app.wait(timeout=60)
finally:
    for proc in (app, xvfb):
        try:
            proc.terminate()
        except Exception:
            pass
with open(os.path.join(out, "phases.json"), "w") as f:
    json.dump({"phases": phases, "bar": C}, f, indent=1)
print(json.dumps(phases))
