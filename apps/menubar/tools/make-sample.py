#!/usr/bin/env python3
"""Writes preview/sample.slint: the sample data (menus, networks, notifications, a month of days) that the
design previews show. Nothing here ships; a backend supplies its own. Run from the project root."""
import calendar, datetime, os

def q(s): return '"' + s.replace('"', '\\"') + '"'

def entry(text="", kind="item", shortcut="", icon="", checked=False, enabled=True, destructive=False, submenu=False, parent=-1):
    return "{text: %s, kind: %s, shortcut: %s, icon: %s, checked: %s, enabled: %s, destructive: %s, submenu: %s, parent: %d}" % (
        q(text), q(kind), q(shortcut), q(icon), str(checked).lower(), str(enabled).lower(), str(destructive).lower(), str(submenu).lower(), parent)

def sep(): return entry(kind="separator")
def head(t): return entry(t, kind="header")

system_menu = [
    entry("About This Computer", icon="info"), sep(),
    entry("System Settings…", icon="gear", shortcut="Ctrl+,"),
    entry("App Store…", icon="download"),
    entry("Recent Items", icon="clock", submenu=True),
    entry("Documents", parent=4), entry("Budget 2026.ods", parent=4), entry("Photos", parent=4), entry("Trip notes.md", parent=4),
    sep(),
    entry("Force Quit…", icon="close", shortcut="Ctrl+Alt+Esc"), sep(),
    entry("Sleep", icon="moon"), entry("Restart…", icon="restart"), entry("Shut Down…", icon="power"), sep(),
    entry("Lock Screen", icon="lock", shortcut="Ctrl+Alt+L"), entry("Log Out Aria…", icon="logout", shortcut="Ctrl+Shift+Q"),
]
file_menu = [
    entry("New Window", icon="window", shortcut="Ctrl+N"), entry("New Tab", shortcut="Ctrl+T"), entry("New Folder", icon="folder", shortcut="Ctrl+Shift+N"), sep(),
    entry("Open Recent", icon="clock", submenu=True),
    entry("Documents", parent=4), entry("Downloads", parent=4), entry("Pictures", checked=True, parent=4), entry("Projects", parent=4),
    sep(),
    entry("Get Info", icon="info", shortcut="Ctrl+I"), entry("Compress…", icon="download"), sep(),
    entry("Close Window", shortcut="Ctrl+W", enabled=False), entry("Move to Bin", icon="trash", shortcut="Del", destructive=True),
]
tray_menu = [head("RECORDING"), entry("Stop Recording", icon="record", shortcut="Ctrl+Alt+R"), entry("Pause", icon="pause"), sep(), entry("Show Recordings", icon="folder"), entry("Recording Settings…", icon="gear")]
layout_menu = [head("KEYBOARD LAYOUT"), entry("English (US)", checked=True), entry("English (UK)"), entry("हिन्दी (Hindi)"), entry("Deutsch"), sep(), entry("Show Keyboard Viewer", icon="keyboard-small"), entry("Keyboard Settings…", icon="gear")]

def net(name, strength, secured, connected=False, connecting=False, saved=False):
    return "{name: %s, strength: %d, secured: %s, connected: %s, connecting: %s, saved: %s}" % (q(name), strength, str(secured).lower(), str(connected).lower(), str(connecting).lower(), str(saved).lower())
networks = [net("Mangrove 5G", 3, True, connected=True, saved=True), net("Cafe Guest", 2, False), net("Neighbour_2.4", 1, True, connecting=True, saved=True), net("Printer-Direct", 1, True), net("AndroidAP 4821", 2, True, saved=True)]

def dev(name, icon, connected=False, connecting=False, battery=-1):
    return "{name: %s, icon: %s, connected: %s, connecting: %s, battery: %d}" % (q(name), q(icon), str(connected).lower(), str(connecting).lower(), battery)
devices = [dev("Studio Buds", "headphones", True, battery=82), dev("MX Master 3", "mouse", True, battery=45), dev("Pixel 8", "phone"), dev("Kitchen Speaker", "speaker", connecting=True)]

today = datetime.date(2026, 10, 28)
first = today.replace(day=1)
start = first - datetime.timedelta(days=first.weekday())  # Monday first
event_days = {3: 2, 7: 1, 9: 2, 14: 1, 21: 1, 22: 3, 28: 3, 30: 1}
cells = []
for i in range(42):
    d = start + datetime.timedelta(days=i)
    inm = d.month == today.month
    ev = event_days.get(d.day, 0) if inm else 0
    cells.append("{day: %d, in-month: %s, today: %s, selected: %s, events: %d}" % (d.day, str(inm).lower(), str(d == today).lower(), str(d == today).lower(), ev))

def ev(time, title, place, tone): return "{time: %s, title: %s, place: %s, tone: %d}" % (q(time), q(title), q(place), tone)
events = [ev("09:30 – 10:00", "Design review", "Room 4", 0), ev("13:00 – 14:00", "Lunch with Aria", "Corner Cafe", 1), ev("17:30", "Gym", "", 2), ev("20:00", "Film night", "Home", 3)]

def tile(id, icon, title, subtitle, on, detail=False): return "{id: %s, icon: %s, title: %s, subtitle: %s, on: %s, enabled: true, detail: %s}" % (q(id), q(icon), q(title), q(subtitle), str(on).lower(), str(detail).lower())
tiles = [tile("wifi", "wifi-3", "Wi-Fi", "Mangrove 5G", True, True), tile("bluetooth", "bluetooth", "Bluetooth", "2 devices", True, True), tile("dnd", "moon", "Do Not Disturb", "Off", False), tile("airplane", "airplane", "Airplane Mode", "", False), tile("dark", "dark-mode", "Dark Mode", "Off", False), tile("night", "night-light", "Night Light", "Until 7:00", True)]
tiles_dark = [tile("wifi", "wifi-3", "Wi-Fi", "Mangrove 5G", True, True), tile("bluetooth", "bluetooth", "Bluetooth", "2 devices", True, True), tile("dnd", "moon", "Do Not Disturb", "Off", False), tile("airplane", "airplane", "Airplane Mode", "", False), tile("dark", "dark-mode", "Dark Mode", "On", True), tile("night", "night-light", "Night Light", "Until 7:00", True)]

def note(id, app, icon, title, body, time, group, gi, gc, unread=False, urgent=False, actions=()):
    return "{id: %d, app: %s, app-icon: %s, title: %s, body: %s, time: %s, group: %s, group-index: %d, group-count: %d, unread: %s, urgent: %s, actions: [%s]}" % (
        id, q(app), q(icon), q(title), q(body), q(time), q(group), gi, gc, str(unread).lower(), str(urgent).lower(), ", ".join(q(a) for a in actions))
notices = [
    note(1, "Calendar", "calendar", "Design review in 10 minutes", "Room 4 · 09:30", "now", "calendar", 0, 1, True, True, ["Join", "Snooze"]),
    note(2, "Mail", "mail", "Aria Patel", "Lunch at 1? The corner cafe has the good tables on Wednesdays, and they are holding one for us.", "5 m", "mail", 0, 3, True, False, ["Reply", "Archive"]),
    note(3, "Mail", "mail", "Design weekly", "The notes from Monday's review are up, with the new bar mock-ups.", "1 h", "mail", 1, 3),
    note(4, "Mail", "mail", "Invoice 1042", "Your invoice is ready to download.", "3 h", "mail", 2, 3),
    note(5, "Store", "store", "3 updates available", "Files, Terminal and Music have new versions.", "Yesterday", "store", 0, 1, False, False, ["Update All"]),
]
toasts = [
    note(11, "Messages", "notes", "Aria", "Running five minutes late, can you save me a seat?", "now", "msg", 0, 1, True, False, ["Reply", "Mute"]),
    note(12, "Downloads", "files", "Download finished", "malva-wallpapers.zip · 48 MB", "now", "dl", 0, 1),
]

weather = "{has-weather: true, city: \"Hyderabad\", icon: \"weather-partly\", temp: \"27°\", condition: \"Partly Cloudy\", range: \"H 31°   L 22°\", hours: [" + ", ".join(
    "{label: %s, icon: %s, temp: %s}" % (q(l), q(i), q(t)) for l, i, t in [("Now", "weather-partly", "27°"), ("6 AM", "weather-clear", "28°"), ("7 AM", "weather-clear", "29°"), ("8 AM", "weather-cloudy", "29°"), ("9 AM", "weather-rain", "26°")]) + "]}"
media = "{has-media: true, title: \"Golden Hour\", artist: \"Kacey Musgraves\", app: \"Music\", playing: true, progress: 0.42, position: \"1:34\", length: \"3:53\", has-art: false}"

def lst(items, indent=8): return "[\n" + ",\n".join(" " * indent + "    " + i for i in items) + "\n" + " " * indent + "]"

out = '''import { MenuEntry, TrayItem, WifiNetwork, BtDevice, AudioDevice, CalCell, CalEvent, QuickTile, Notice, Weather, Media, Fact } from "../ui/types.slint";

// Generated by tools/make-sample.py: the sample data the design previews show.
export global Sample {
    out property <[string]> menus: ["Files", "File", "Edit", "View", "Go", "Window", "Help"];
    out property <[TrayItem]> tray: [{icon: "record", tip: "Recording", active: true}, {icon: "clipboard", tip: "Clipboard", active: false}];
    out property <[MenuEntry]> system-menu: %s;
    out property <[MenuEntry]> file-menu: %s;
    out property <[MenuEntry]> tray-menu: %s;
    out property <[MenuEntry]> layout-menu: %s;
    out property <[WifiNetwork]> networks: %s;
    out property <[BtDevice]> devices: %s;
    out property <[AudioDevice]> outputs: [{name: "Speakers", icon: "speaker", selected: true}, {name: "Studio Buds", icon: "headphones", selected: false}, {name: "HDMI Monitor", icon: "globe", selected: false}];
    out property <[string]> weekdays: ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"];
    out property <[string]> weekdays-short: ["M", "T", "W", "T", "F", "S", "S"];
    out property <[CalCell]> cells: %s;
    out property <[CalEvent]> events: %s;
    out property <[QuickTile]> tiles: %s;
    out property <[QuickTile]> tiles-dark: %s;
    out property <[Notice]> notices: %s;
    out property <[Notice]> toasts: %s;
    out property <[Fact]> battery-facts: [{label: "Health", value: "94 %%"}, {label: "Cycles", value: "212"}];
    out property <Weather> weather: %s;
    out property <Media> media: %s;
}
''' % (lst(system_menu), lst(file_menu), lst(tray_menu), lst(layout_menu), lst(networks), lst(devices), lst(cells), lst(events), lst(tiles), lst(tiles_dark), lst(notices), lst(toasts), weather, media)
os.makedirs("preview", exist_ok=True)
open(os.path.join("preview", "sample.slint"), "w").write(out)
print("preview/sample.slint written")
