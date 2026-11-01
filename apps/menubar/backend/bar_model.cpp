#include "bar_model.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <map>

namespace barui {

namespace {

const char* kMonths[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
const char* kDays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

bool leap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }
int daysIn(int y, int m)
{
    static const int d[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return m == 2 && leap(y) ? 29 : d[m - 1];
}
// 0 = Sunday (Sakamoto's method).
int weekday(int y, int m, int d)
{
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3)
        y -= 1;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}
std::string fmt(const char* f, ...) __attribute__((format(printf, 1, 2)));
std::string fmt(const char* f, ...)
{
    char buf[256];
    va_list args;
    va_start(args, f);
    std::vsnprintf(buf, sizeof buf, f, args);
    va_end(args);
    return buf;
}

struct Track {
    const char* title;
    const char* artist;
    int seconds;
};
const Track kTracks[] = {{"Golden Hour", "Kacey Musgraves", 233}, {"Open Water", "Lumen", 198}, {"Late Bloom", "Aster & Co.", 271}};

std::string clockOf(int seconds)
{
    return fmt("%d:%02d", seconds / 60, seconds % 60);
}

Row item(const std::string& text, const std::string& action, const std::string& shortcut = "", const std::string& icon = "")
{
    Row r;
    r.text = text;
    r.action = action;
    r.shortcut = shortcut;
    r.icon = icon;
    return r;
}
Row sep()
{
    Row r;
    r.kind = "separator";
    return r;
}
Row head(const std::string& text)
{
    Row r;
    r.kind = "header";
    r.text = text;
    return r;
}
Row sub(const std::string& text, const std::string& icon = "")
{
    Row r;
    r.text = text;
    r.icon = icon;
    r.submenu = true;
    return r;
}

} // namespace

// -------------------------------------------------------------------------------- the sample machine
Model Model::sample()
{
    Model m;
    m.m_networks = {{"Mangrove 5G", 3, true, true, false, true}, {"Cafe Guest", 2, false, false, false, false},
                    {"Neighbour_2.4", 1, true, false, false, true}, {"Printer-Direct", 1, true, false, false, false},
                    {"AndroidAP 4821", 2, true, false, false, true}};
    m.m_devices = {{"Studio Buds", "headphones", true, false, 82}, {"MX Master 3", "mouse", true, false, 45},
                   {"Pixel 8", "phone", false, false, -1}, {"Kitchen Speaker", "speaker", false, false, -1}};
    m.m_outputs = {{"Speakers", "speaker", true}, {"Studio Buds", "headphones", false}, {"HDMI Monitor", "globe", false}};
    m.m_weather = {"Hyderabad", "weather-partly", "27°", "Partly Cloudy", "H 31°   L 22°",
                   {{"Now", "weather-partly", "27°"}, {"6 AM", "weather-clear", "28°"}, {"7 AM", "weather-clear", "29°"},
                    {"8 AM", "weather-cloudy", "29°"}, {"9 AM", "weather-rain", "26°"}}};
    m.m_media.app = "Music";
    m.nextTrack(); // sets up the first track's text
    m.previousTrack();
    m.m_trackSeconds = 94;
    m.tickMedia(0);
    // Four notifications, oldest first, so the newest is on top.
    m.m_minutes = 400;
    m.post("Store", "store", "3 updates available", "Files, Terminal and Music have new versions.", "store", {"Update All"});
    m.m_notes.back().postedAt = 400 - 24 * 60;
    m.m_notes.back().unread = false;
    m.post("Mail", "mail", "Invoice 1042", "Your invoice is ready to download.", "mail");
    m.m_notes.back().postedAt = 400 - 180;
    m.m_notes.back().unread = false;
    m.post("Mail", "mail", "Design weekly", "The notes from Monday's review are up, with the new bar mock-ups.", "mail");
    m.m_notes.back().postedAt = 400 - 60;
    m.m_notes.back().unread = false;
    m.post("Mail", "mail", "Aria Patel", "Lunch at 1? The corner cafe has the good tables on Wednesdays, and they are holding one for us.", "mail", {"Reply", "Archive"});
    m.m_notes.back().postedAt = 400 - 5;
    m.post("Calendar", "calendar", "Design review in 10 minutes", "Room 4 · 09:30", "calendar", {"Join", "Snooze"}, true);
    m.m_notes.back().postedAt = 400;
    m.m_toasts.clear(); // they were already there before the bar started
    return m;
}

// -------------------------------------------------------------------------------- time
void Model::setNow(const Clock& c)
{
    m_now = c;
    m_viewYear = m_selYear = c.year;
    m_viewMonth = m_selMonth = c.month;
    m_selDay = c.day;
}

void Model::setTime(int hour, int minute)
{
    m_now.hour = hour;
    m_now.minute = minute;
}

std::string Model::clockText() const
{
    const int wd = weekday(m_now.year, m_now.month, m_now.day);
    const int h12 = m_now.hour % 12 == 0 ? 12 : m_now.hour % 12;
    return fmt("%.3s %d %.3s  %d:%02d %s", kDays[wd], m_now.day, kMonths[m_now.month - 1], h12, m_now.minute, m_now.hour < 12 ? "AM" : "PM");
}

std::string Model::dateText() const
{
    const int wd = weekday(m_now.year, m_now.month, m_now.day);
    return fmt("%s %d %.3s", kDays[wd], m_now.day, kMonths[m_now.month - 1]);
}

void Model::advance(int minutes)
{
    m_minutes += minutes;
    int total = m_now.hour * 60 + m_now.minute + minutes;
    while (total >= 24 * 60) {
        total -= 24 * 60;
        if (++m_now.day > daysIn(m_now.year, m_now.month)) {
            m_now.day = 1;
            if (++m_now.month > 12) {
                m_now.month = 1;
                ++m_now.year;
            }
        }
    }
    m_now.hour = total / 60;
    m_now.minute = total % 60;
    // The battery: 1% every 3 minutes on battery, 1% every 2 while charging, never past either end.
    if (m_charging)
        m_battery = std::min(100, m_battery + minutes / 2);
    else
        m_battery = std::max(0, m_battery - minutes / 3);
}

// -------------------------------------------------------------------------------- layout
std::string Model::layoutText() const
{
    static const char* t[] = {"EN", "UK", "HI", "DE"};
    return t[m_layout % 4];
}
void Model::setLayout(int i) { m_layout = i < 0 ? 0 : i > 3 ? 3 : i; }

// -------------------------------------------------------------------------------- Wi-Fi
void Model::setWifi(bool on)
{
    m_wifi = on;
    if (on) {
        m_airplane = false;
    } else {
        for (Network& n : m_networks)
            n.connecting = false;
    }
}
bool Model::clickNetwork(int i)
{
    if (i < 0 || i >= int(m_networks.size()) || !m_wifi)
        return false;
    Network& n = m_networks[i];
    if (n.connecting)
        return true;
    if (n.connected) {
        n.connected = false;
        return true;
    }
    if (n.secured && !n.saved)
        return false;
    n.connecting = true;
    return true;
}
void Model::finishConnect(int i)
{
    if (i < 0 || i >= int(m_networks.size()) || !m_networks[i].connecting)
        return;
    for (Network& n : m_networks)
        n.connected = false;
    m_networks[i].connecting = false;
    m_networks[i].connected = true;
    m_networks[i].saved = true;
}
bool Model::wifiConnected() const
{
    return m_wifi && std::any_of(m_networks.begin(), m_networks.end(), [](const Network& n) { return n.connected; });
}
int Model::wifiStrength() const
{
    for (const Network& n : m_networks) {
        if (n.connected)
            return n.strength;
    }
    return 0;
}
std::string Model::wifiName() const
{
    for (const Network& n : m_networks) {
        if (n.connected)
            return n.name;
    }
    return "";
}

// -------------------------------------------------------------------------------- Bluetooth
void Model::setBluetooth(bool on)
{
    m_bt = on;
    if (on) {
        m_airplane = false;
    } else {
        for (Device& d : m_devices) {
            d.connected = false;
            d.connecting = false;
        }
    }
}
void Model::clickDevice(int i)
{
    if (i < 0 || i >= int(m_devices.size()) || !m_bt)
        return;
    Device& d = m_devices[i];
    if (d.connecting)
        return;
    if (d.connected) {
        d.connected = false;
        d.battery = d.battery; // it keeps what it last said
    } else {
        d.connecting = true;
    }
}
void Model::finishDevice(int i)
{
    if (i < 0 || i >= int(m_devices.size()) || !m_devices[i].connecting)
        return;
    m_devices[i].connecting = false;
    m_devices[i].connected = true;
    if (m_devices[i].battery < 0)
        m_devices[i].battery = 64;
}
int Model::devicesConnected() const
{
    return int(std::count_if(m_devices.begin(), m_devices.end(), [](const Device& d) { return d.connected; }));
}

void Model::setAirplane(bool on)
{
    m_airplane = on;
    if (on) {
        setWifi(false);
        setBluetooth(false);
        m_airplane = true;
    }
}

// -------------------------------------------------------------------------------- battery
void Model::setBattery(int percent, bool charging)
{
    m_battery = percent < 0 ? 0 : percent > 100 ? 100 : percent;
    m_charging = charging;
}
std::string Model::durationText(int minutes) const
{
    if (minutes >= 60)
        return fmt("%d h %d min", minutes / 60, minutes % 60);
    return fmt("%d min", minutes);
}
std::string Model::batteryStatus() const
{
    if (m_charging)
        return m_battery >= 100 ? "Fully charged" : "Charging";
    return m_battery <= 15 ? "Low battery" : "On battery";
}
std::string Model::batteryDetail() const
{
    if (m_charging)
        return m_battery >= 100 ? "Charged" : durationText((100 - m_battery) * 2) + " until full";
    return durationText(m_battery * 3) + " remaining";
}
std::string Model::batterySource() const { return m_charging ? "Power adapter" : "Battery"; }

// -------------------------------------------------------------------------------- sound and light
void Model::setVolume(float v)
{
    m_volume = v < 0 ? 0 : v > 1 ? 1 : v;
    if (m_volume > 0)
        m_muted = false;
}
void Model::setBrightness(float b) { m_brightness = b < 0.02f ? 0.02f : b > 1 ? 1 : b; }
void Model::pickOutput(int i)
{
    if (i < 0 || i >= int(m_outputs.size()))
        return;
    for (std::size_t k = 0; k < m_outputs.size(); ++k)
        m_outputs[k].selected = int(k) == i;
}

// -------------------------------------------------------------------------------- tiles
std::vector<Tile> Model::tiles() const
{
    std::vector<Tile> t;
    const bool radios = !m_airplane;
    t.push_back({"wifi", wifiConnected() ? "wifi-3" : m_wifi ? "wifi-1" : "wifi-off", "Wi-Fi",
                 !m_wifi ? "Off" : wifiConnected() ? wifiName() : "Not connected", m_wifi, radios, true});
    const int n = devicesConnected();
    t.push_back({"bluetooth", "bluetooth", "Bluetooth", !m_bt ? "Off" : n == 0 ? "On" : n == 1 ? "1 device" : fmt("%d devices", n), m_bt, radios, true});
    t.push_back({"dnd", "moon", "Do Not Disturb", m_dnd ? "On" : "Off", m_dnd, true, false});
    t.push_back({"airplane", "airplane", "Airplane Mode", "", m_airplane, true, false});
    t.push_back({"dark", "dark-mode", "Dark Mode", m_dark ? "On" : "Off", m_dark, true, false});
    t.push_back({"night", "night-light", "Night Light", m_night ? "Until 7:00" : "Off", m_night, true, false});
    return t;
}
void Model::toggleTile(const std::string& id)
{
    if (id == "wifi")
        setWifi(!m_wifi);
    else if (id == "bluetooth")
        setBluetooth(!m_bt);
    else if (id == "dnd")
        m_dnd = !m_dnd;
    else if (id == "airplane")
        setAirplane(!m_airplane);
    else if (id == "dark")
        m_dark = !m_dark;
    else if (id == "night")
        m_night = !m_night;
}

// -------------------------------------------------------------------------------- calendar
std::string Model::monthTitle() const { return fmt("%s %d", kMonths[m_viewMonth - 1], m_viewYear); }

std::vector<Cell> Model::cells() const
{
    std::vector<Cell> out;
    // The first cell is the Monday on or before the 1st.
    const int first = (weekday(m_viewYear, m_viewMonth, 1) + 6) % 7; // 0 = Monday
    int y = m_viewYear, m = m_viewMonth - 1;
    if (m < 1) {
        m = 12;
        --y;
    }
    int day = daysIn(y, m) - first + 1;
    bool inMonth = first == 0;
    if (first == 0) {
        y = m_viewYear;
        m = m_viewMonth;
        day = 1;
    }
    for (int i = 0; i < 42; ++i) {
        Cell c;
        c.day = day;
        c.inMonth = inMonth;
        c.today = y == m_now.year && m == m_now.month && day == m_now.day;
        c.selected = y == m_selYear && m == m_selMonth && day == m_selDay;
        // Every third day has something on, every fifth another thing; the 28th is the busy one (see events()).
        c.events = day == 28 ? 4 : (day % 3 == 0 ? 1 : 0) + (day % 5 == 0 ? 1 : 0);
        out.push_back(c);
        if (++day > daysIn(y, m)) {
            day = 1;
            if (++m > 12) {
                m = 1;
                ++y;
            }
            inMonth = (y == m_viewYear && m == m_viewMonth);
        }
    }
    return out;
}

std::string Model::selectedTitle() const
{
    return fmt("%s, %d %s", kDays[weekday(m_selYear, m_selMonth, m_selDay)], m_selDay, kMonths[m_selMonth - 1]);
}

std::vector<Event> Model::events() const
{
    std::vector<Event> out;
    const int d = m_selDay;
    if (d % 3 == 0)
        out.push_back({"09:30 – 10:00", "Design review", "Room 4", 0});
    if (d % 5 == 0)
        out.push_back({"13:00 – 14:00", "Lunch with Aria", "Corner Cafe", 1});
    if (d == 28) {
        out.clear();
        out.push_back({"09:30 – 10:00", "Design review", "Room 4", 0});
        out.push_back({"13:00 – 14:00", "Lunch with Aria", "Corner Cafe", 1});
        out.push_back({"17:30", "Gym", "", 2});
        out.push_back({"20:00", "Film night", "Home", 3});
    }
    return out;
}

void Model::previousMonth()
{
    if (--m_viewMonth < 1) {
        m_viewMonth = 12;
        --m_viewYear;
    }
}
void Model::nextMonth()
{
    if (++m_viewMonth > 12) {
        m_viewMonth = 1;
        ++m_viewYear;
    }
}
void Model::goToday()
{
    m_viewYear = m_selYear = m_now.year;
    m_viewMonth = m_selMonth = m_now.month;
    m_selDay = m_now.day;
}
void Model::pickCell(int i)
{
    const std::vector<Cell> c = cells();
    if (i < 0 || i >= int(c.size()))
        return;
    int y = m_viewYear, m = m_viewMonth;
    if (!c[i].inMonth) {
        // Before the month if it is among the first week and above 20; otherwise after.
        if (i < 7 && c[i].day > 20) {
            if (--m < 1) {
                m = 12;
                --y;
            }
        } else if (++m > 12) {
            m = 1;
            ++y;
        }
    }
    m_selYear = y;
    m_selMonth = m;
    m_selDay = c[i].day;
    m_viewYear = y;
    m_viewMonth = m;
}

// -------------------------------------------------------------------------------- notifications
std::string Model::ago(int minutes)
{
    if (minutes < 1)
        return "now";
    if (minutes < 60)
        return fmt("%d m", minutes);
    if (minutes < 24 * 60)
        return fmt("%d h", minutes / 60);
    return minutes < 48 * 60 ? "Yesterday" : fmt("%d d", minutes / (24 * 60));
}

int Model::post(const std::string& app, const std::string& icon, const std::string& title, const std::string& body,
                const std::string& group, const std::vector<std::string>& actions, bool urgent)
{
    Note n{m_nextId++, app, icon, title, body, group, m_minutes, true, urgent, actions};
    m_notes.push_back(n);
    if (!m_dnd) {
        m_toasts.push_back(n.id);
        // At most three banners at once: the oldest goes.
        while (m_toasts.size() > 3)
            m_toasts.erase(m_toasts.begin());
    }
    return n.id;
}

std::vector<Notice> Model::notices() const
{
    // Groups ordered by their newest member; inside a group, newest first.
    std::vector<const Note*> sorted;
    for (const Note& n : m_notes)
        sorted.push_back(&n);
    std::map<std::string, int> newest;
    for (const Note* n : sorted)
        newest[n->group] = std::max(newest.count(n->group) ? newest[n->group] : -1, n->postedAt * 100000 + n->id);
    std::stable_sort(sorted.begin(), sorted.end(), [&](const Note* a, const Note* b) {
        if (a->group != b->group)
            return newest[a->group] > newest[b->group];
        return a->postedAt != b->postedAt ? a->postedAt > b->postedAt : a->id > b->id;
    });
    std::map<std::string, int> size, seen;
    for (const Note* n : sorted)
        ++size[n->group];
    std::vector<Notice> out;
    for (const Note* n : sorted) {
        Notice v;
        v.id = n->id;
        v.app = n->app;
        v.appIcon = n->icon;
        v.title = n->title;
        v.body = n->body;
        v.time = ago(m_minutes - n->postedAt);
        v.group = n->group;
        v.groupIndex = seen[n->group]++;
        v.groupCount = size[n->group];
        v.unread = n->unread;
        v.urgent = n->urgent;
        v.actions = n->actions;
        out.push_back(v);
    }
    return out;
}

Notice Model::toast(int id) const
{
    for (const Notice& n : notices()) {
        if (n.id == id) {
            Notice t = n;
            t.groupIndex = 0;
            t.groupCount = 1;
            return t;
        }
    }
    return Notice {};
}

int Model::unread() const
{
    return int(std::count_if(m_notes.begin(), m_notes.end(), [](const Note& n) { return n.unread; }));
}
void Model::dismiss(int id)
{
    m_notes.erase(std::remove_if(m_notes.begin(), m_notes.end(), [&](const Note& n) { return n.id == id; }), m_notes.end());
    m_toasts.erase(std::remove(m_toasts.begin(), m_toasts.end(), id), m_toasts.end());
}
void Model::clearGroup(const std::string& group)
{
    std::vector<int> ids;
    for (const Note& n : m_notes) {
        if (n.group == group)
            ids.push_back(n.id);
    }
    for (int id : ids)
        dismiss(id);
}
void Model::clearAll()
{
    m_notes.clear();
    m_toasts.clear();
}
void Model::expireToast(int id) { m_toasts.erase(std::remove(m_toasts.begin(), m_toasts.end(), id), m_toasts.end()); }
void Model::markAllRead()
{
    for (Note& n : m_notes)
        n.unread = false;
}

// -------------------------------------------------------------------------------- media
void Model::playPause()
{
    m_media.playing = !m_media.playing;
}
void Model::nextTrack()
{
    m_track = (m_track + 1) % 3;
    m_trackSeconds = 0;
    m_media.title = kTracks[m_track].title;
    m_media.artist = kTracks[m_track].artist;
    m_media.progress = 0;
    m_media.position = clockOf(0);
    m_media.length = clockOf(kTracks[m_track].seconds);
}
void Model::previousTrack()
{
    // Past the first seconds, previous goes back to the start of this one; otherwise to the one before.
    if (m_trackSeconds > 3) {
        m_trackSeconds = 0;
    } else {
        m_track = (m_track + 2) % 3;
        m_trackSeconds = 0;
    }
    m_media.title = kTracks[m_track].title;
    m_media.artist = kTracks[m_track].artist;
    m_media.progress = 0;
    m_media.position = clockOf(0);
    m_media.length = clockOf(kTracks[m_track].seconds);
}

void Model::tickMedia(int seconds)
{
    if (!m_media.has || !m_media.playing)
        return;
    m_trackSeconds += seconds;
    const int length = kTracks[m_track].seconds;
    if (m_trackSeconds >= length) {
        nextTrack();
        return;
    }
    m_media.progress = float(m_trackSeconds) / float(length);
    m_media.position = clockOf(m_trackSeconds);
}

// -------------------------------------------------------------------------------- menus
std::vector<std::string> Model::menus() const { return {"Files", "File", "Edit", "View", "Go", "Window", "Help"}; }

std::vector<Row> Model::menuFor(const std::string& name) const
{
    std::vector<Row> r;
    if (name == "system") {
        r.push_back(item("About This Computer", "about", "", "info"));
        r.push_back(sep());
        r.push_back(item("System Settings…", "open:settings", "Ctrl+,", "gear"));
        r.push_back(item("App Store…", "open:store", "", "download"));
        r.push_back(sub("Recent Items", "clock"));
        const int parent = int(r.size()) - 1;
        for (const char* t : {"Documents", "Budget 2026.ods", "Photos", "Trip notes.md"}) {
            Row x = item(t, std::string("open-recent:") + t);
            x.parent = parent;
            r.push_back(x);
        }
        r.push_back(sep());
        r.push_back(item("Force Quit…", "force-quit", "Ctrl+Alt+Esc", "close"));
        r.push_back(sep());
        r.push_back(item("Sleep", "power:sleep", "", "moon"));
        r.push_back(item("Restart…", "power:restart", "", "restart"));
        r.push_back(item("Shut Down…", "power:shutdown", "", "power"));
        r.push_back(sep());
        r.push_back(item("Lock Screen", "power:lock", "Ctrl+Alt+L", "lock"));
        r.push_back(item("Log Out " + user + "…", "power:logout", "Ctrl+Shift+Q", "logout"));
    } else if (name == "menu-0") {
        r.push_back(item("About Files", "app:about", "", "info"));
        r.push_back(sep());
        r.push_back(item("Preferences…", "app:preferences", "Ctrl+,", "gear"));
        r.push_back(sep());
        r.push_back(item("Hide Files", "app:hide", "Ctrl+H"));
        r.push_back(item("Quit Files", "app:quit", "Ctrl+Q"));
    } else if (name == "menu-1") {
        r.push_back(item("New Window", "file:new-window", "Ctrl+N", "window"));
        r.push_back(item("New Tab", "file:new-tab", "Ctrl+T"));
        r.push_back(item("New Folder", "file:new-folder", "Ctrl+Shift+N", "folder"));
        r.push_back(sep());
        r.push_back(sub("Open Recent", "clock"));
        const int parent = int(r.size()) - 1;
        for (const char* t : {"Documents", "Downloads", "Pictures", "Projects"}) {
            Row x = item(t, std::string("file:open-recent:") + t);
            x.parent = parent;
            r.push_back(x);
        }
        r.push_back(sep());
        Row info = item("Get Info", "file:info", "Ctrl+I", "info");
        info.enabled = selection > 0;
        r.push_back(info);
        Row zip = item("Compress…", "file:compress", "", "download");
        zip.enabled = selection > 0;
        r.push_back(zip);
        r.push_back(sep());
        Row close = item("Close Window", "file:close", "Ctrl+W");
        close.enabled = windows > 0;
        r.push_back(close);
        Row bin = item("Move to Bin", "file:trash", "Del", "trash");
        bin.destructive = true;
        bin.enabled = selection > 0;
        r.push_back(bin);
    } else if (name == "menu-2") {
        Row undo = item("Undo", "edit:undo", "Ctrl+Z", "undo");
        undo.enabled = canUndo;
        Row redo = item("Redo", "edit:redo", "Ctrl+Shift+Z", "redo");
        redo.enabled = canRedo;
        r.push_back(undo);
        r.push_back(redo);
        r.push_back(sep());
        Row cut = item("Cut", "edit:cut", "Ctrl+X");
        Row copy = item("Copy", "edit:copy", "Ctrl+C", "copy");
        Row paste = item("Paste", "edit:paste", "Ctrl+V");
        cut.enabled = copy.enabled = selection > 0;
        r.push_back(cut);
        r.push_back(copy);
        r.push_back(paste);
        r.push_back(sep());
        r.push_back(item("Select All", "edit:select-all", "Ctrl+A"));
        r.push_back(item("Find…", "edit:find", "Ctrl+F", "search"));
    } else if (name == "menu-3") {
        const char* modes[] = {"as Icons", "as List", "as Columns"};
        for (int i = 0; i < 3; ++i) {
            Row x = item(modes[i], std::string("view:mode:") + char('0' + i), std::string("Ctrl+") + char('1' + i));
            x.checked = viewMode == i;
            r.push_back(x);
        }
        r.push_back(sep());
        r.push_back(sub("Sort By"));
        const int parent = int(r.size()) - 1;
        const char* sorts[] = {"Name", "Date Modified", "Size"};
        for (int i = 0; i < 3; ++i) {
            Row x = item(sorts[i], std::string("view:sort:") + char('0' + i));
            x.checked = sortBy == i;
            x.parent = parent;
            r.push_back(x);
        }
        Row hidden = item("Show Hidden Files", "view:hidden", "Ctrl+H");
        hidden.checked = showHidden;
        r.push_back(hidden);
        r.push_back(sep());
        r.push_back(item("Enter Full Screen", "view:fullscreen", "F11", "fullscreen"));
    } else if (name == "menu-4") {
        r.push_back(item("Back", "go:back", "Alt+Left", "chevron-left"));
        r.push_back(item("Forward", "go:forward", "Alt+Right", "chevron-right"));
        r.push_back(item("Enclosing Folder", "go:up", "Alt+Up", "chevron-up"));
        r.push_back(sep());
        r.push_back(item("Home", "go:home", "Ctrl+Shift+H", "folder"));
        r.push_back(item("Documents", "go:documents", "Ctrl+Shift+O"));
        r.push_back(item("Downloads", "go:downloads", "Ctrl+Shift+L"));
        r.push_back(item("Pictures", "go:pictures"));
        r.push_back(sep());
        r.push_back(item("Go to Folder…", "go:to", "Ctrl+L"));
    } else if (name == "menu-5") {
        r.push_back(item("Minimise", "window:minimize", "Ctrl+M", "minimize"));
        r.push_back(item("Zoom", "window:zoom", "", "fullscreen"));
        r.push_back(sep());
        for (int i = 0; i < windows; ++i) {
            Row x = item("Pictures" + std::string(i ? " " + std::to_string(i + 1) : ""), "window:focus:" + std::to_string(i));
            x.checked = i == 0;
            r.push_back(x);
        }
        if (windows > 0)
            r.push_back(sep());
        r.push_back(item("Bring All to Front", "window:front"));
    } else if (name == "menu-6") {
        r.push_back(item("Files Help", "help:files", "F1", "help"));
        r.push_back(item("Keyboard Shortcuts", "help:shortcuts", "Ctrl+/", "keyboard-small"));
        r.push_back(sep());
        r.push_back(item("Report a Problem…", "help:report"));
    } else if (name == "tray-0") {
        r.push_back(head("RECORDING"));
        r.push_back(item("Stop Recording", "tray:record-stop", "Ctrl+Alt+R", "record"));
        r.push_back(item("Pause", "tray:record-pause", "", "pause"));
        r.push_back(sep());
        r.push_back(item("Show Recordings", "tray:record-show", "", "folder"));
        r.push_back(item("Recording Settings…", "tray:record-settings", "", "gear"));
    } else if (name == "tray-1") {
        r.push_back(head("CLIPBOARD"));
        r.push_back(item("Rent invoice, October", "tray:clip:0", "", "copy"));
        r.push_back(item("https://malva.example", "tray:clip:1", "", "copy"));
        r.push_back(item("Meeting at 13:00", "tray:clip:2", "", "copy"));
        r.push_back(sep());
        r.push_back(item("Clear History", "tray:clip-clear", "", "trash"));
    } else if (name == "layout") {
        r.push_back(head("KEYBOARD LAYOUT"));
        const char* names[] = {"English (US)", "English (UK)", "हिन्दी (Hindi)", "Deutsch"};
        for (int i = 0; i < 4; ++i) {
            Row x = item(names[i], "layout:" + std::to_string(i));
            x.checked = m_layout == i;
            r.push_back(x);
        }
        r.push_back(sep());
        r.push_back(item("Show Keyboard Viewer", "layout:viewer", "", "keyboard-small"));
        r.push_back(item("Keyboard Settings…", "layout:settings", "", "gear"));
    }
    return r;
}

std::string Model::perform(const std::string& name, int row)
{
    const std::vector<Row> rows = menuFor(name);
    if (row < 0 || row >= int(rows.size()) || rows[row].kind != "item" || !rows[row].enabled)
        return "";
    const std::string a = rows[row].action;
    // The ones this model owns.
    if (a.rfind("layout:", 0) == 0 && a.size() == 8 && a[7] >= '0' && a[7] <= '3') {
        setLayout(a[7] - '0');
    } else if (a == "view:hidden") {
        showHidden = !showHidden;
    } else if (a.rfind("view:mode:", 0) == 0) {
        viewMode = a.back() - '0';
    } else if (a.rfind("view:sort:", 0) == 0) {
        sortBy = a.back() - '0';
    } else if (a == "file:new-window") {
        ++windows;
    } else if (a == "file:close") {
        windows = windows > 0 ? windows - 1 : 0;
    } else if (a == "edit:undo") {
        canUndo = false;
        canRedo = true;
    } else if (a == "edit:redo") {
        canRedo = false;
        canUndo = true;
    } else if (a == "edit:select-all") {
        selection = 12;
    }
    return a;
}

} // namespace barui
