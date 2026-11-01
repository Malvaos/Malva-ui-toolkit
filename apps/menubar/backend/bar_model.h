// What the menu bar shows and what each choice does, with no Slint in it: Wi-Fi, Bluetooth, the battery,
// sound and light, the calendar, notifications, the media player, the menus. A real backend replaces
// this with the system's services (NetworkManager, BlueZ, UPower, PulseAudio, a notification daemon);
// the window only ever sees the shapes below.
#pragma once

#include <string>
#include <vector>

namespace barui {

struct Network {
    std::string name;
    int strength = 3; // 0 to 3
    bool secured = false;
    bool connected = false;
    bool connecting = false;
    bool saved = false;
};

struct Device {
    std::string name;
    std::string icon;
    bool connected = false;
    bool connecting = false;
    int battery = -1;
};

struct Output {
    std::string name;
    std::string icon;
    bool selected = false;
};

struct Cell {
    int day = 1;
    bool inMonth = true;
    bool today = false;
    bool selected = false;
    int events = 0;
};

struct Event {
    std::string time;
    std::string title;
    std::string place;
    int tone = 0;
};

struct Tile {
    std::string id;
    std::string icon;
    std::string title;
    std::string subtitle;
    bool on = false;
    bool enabled = true;
    bool detail = false;
};

struct Notice {
    int id = 0;
    std::string app;
    std::string appIcon;
    std::string title;
    std::string body;
    std::string time; // "now", "5 m", "2 h", "Yesterday"
    std::string group;
    int groupIndex = 0;
    int groupCount = 1;
    bool unread = true;
    bool urgent = false;
    std::vector<std::string> actions;
};

struct Row {
    std::string text;
    std::string kind = "item"; // "item", "header", "separator"
    std::string shortcut;
    std::string icon;
    // What choosing it does ("power:shutdown", "file:new-window"); the model applies the state changes
    // it owns (a tick moving, a layout chosen) and reports the rest.
    std::string action;
    bool checked = false;
    bool enabled = true;
    bool destructive = false;
    bool submenu = false;
    int parent = -1;
};

struct Hour {
    std::string label;
    std::string icon;
    std::string temp;
};

struct Weather {
    std::string city, icon, temp, condition, range;
    std::vector<Hour> hours;
};

struct Media {
    bool has = true;
    std::string title, artist, app;
    bool playing = true;
    float progress = 0;
    std::string position, length;
};

// The date and time the model believes it is (the backend sets it; tests set it by hand).
struct Clock {
    int year = 2026, month = 10, day = 28, hour = 5, minute = 1;
};

class Model
{
public:
    // A sample machine: a few networks and devices, a battery, some notifications, a track.
    static Model sample();

    // ---- time
    const Clock& now() const { return m_now; }
    void setNow(const Clock& c);
    // Sets the time of day only, leaving the calendar where the person has it.
    void setTime(int hour, int minute);
    // "Wed 28 Oct  5:01 AM"
    std::string clockText() const;
    std::string dateText() const;   // "Wednesday 28 Oct"
    // Moves the minute hand: the battery changes, notifications age, a playing track advances.
    void advance(int minutes);

    // ---- layout
    int layout() const { return m_layout; }
    std::string layoutText() const; // "EN"
    void setLayout(int i);

    // ---- Wi-Fi
    bool wifiEnabled() const { return m_wifi; }
    void setWifi(bool on);
    const std::vector<Network>& networks() const { return m_networks; }
    // Starts connecting (a saved or open network) or disconnects (a connected one); a secured network that
    // is not saved needs a password, which is not here: nothing changes and false comes back.
    bool clickNetwork(int i);
    void finishConnect(int i);
    bool wifiConnected() const;
    int wifiStrength() const;
    std::string wifiName() const;

    // ---- Bluetooth
    bool bluetoothEnabled() const { return m_bt; }
    void setBluetooth(bool on);
    const std::vector<Device>& devices() const { return m_devices; }
    void clickDevice(int i);
    void finishDevice(int i);
    int devicesConnected() const;

    // ---- airplane mode, dark mode, night light, do not disturb
    bool airplane() const { return m_airplane; }
    void setAirplane(bool on);
    bool dark() const { return m_dark; }
    void setDark(bool on) { m_dark = on; }
    bool nightLight() const { return m_night; }
    bool dnd() const { return m_dnd; }
    void setDnd(bool on) { m_dnd = on; }

    // ---- battery
    int battery() const { return m_battery; }
    bool charging() const { return m_charging; }
    void setBattery(int percent, bool charging);
    std::string batteryStatus() const;
    std::string batteryDetail() const;
    std::string batterySource() const;
    int powerMode() const { return m_powerMode; }
    void setPowerMode(int i) { m_powerMode = i < 0 ? 0 : i > 2 ? 2 : i; }

    // ---- sound and light
    float volume() const { return m_volume; }
    bool muted() const { return m_muted; }
    void setVolume(float v);
    void setMuted(bool m) { m_muted = m; }
    float brightness() const { return m_brightness; }
    void setBrightness(float b);
    const std::vector<Output>& outputs() const { return m_outputs; }
    void pickOutput(int i);

    // ---- the Control Center's tiles
    std::vector<Tile> tiles() const;
    // Toggles by id ("wifi", "bluetooth", "dnd", "airplane", "dark", "night").
    void toggleTile(const std::string& id);

    // ---- the calendar
    int viewYear() const { return m_viewYear; }
    int viewMonth() const { return m_viewMonth; }
    std::string monthTitle() const;   // "October 2026"
    std::vector<Cell> cells() const;  // 42, Monday first
    std::string selectedTitle() const; // "Wednesday, 28 October"
    std::vector<Event> events() const; // the selected day's
    void previousMonth();
    void nextMonth();
    void goToday();
    // Picks the day in cell i of `cells()`; a day of another month shows that month.
    void pickCell(int i);
    int selectedDay() const { return m_selDay; }

    // ---- notifications
    // Adds one (newest). Returns its id. `toast` says whether it should also come up as a banner (not
    // while Do Not Disturb is on).
    int post(const std::string& app, const std::string& icon, const std::string& title, const std::string& body,
             const std::string& group, const std::vector<std::string>& actions = {}, bool urgent = false);
    std::vector<Notice> notices() const;
    std::vector<int> toastIds() const { return m_toasts; }
    Notice toast(int id) const;
    int unread() const;
    void dismiss(int id);
    void clearGroup(const std::string& group);
    void clearAll();
    void expireToast(int id);
    void markAllRead();
    int count() const { return int(m_notes.size()); }

    // ---- media
    const Media& media() const { return m_media; }
    void playPause();
    void nextTrack();
    void previousTrack();
    // A playing track moves on by this many seconds (at its end, the next begins).
    void tickMedia(int seconds);
    std::string weatherCity() const { return m_weather.city; }
    const Weather& weather() const { return m_weather; }

    // ---- menus: the rows of "system", "menu-0" ... "menu-6", "tray-0", "tray-1", "layout"
    std::vector<std::string> menus() const;
    std::vector<Row> menuFor(const std::string& name) const;
    // Does what row `row` of that menu does to the model, and says what else it asks for ("power:restart", ...).
    std::string perform(const std::string& name, int row);

    // ---- the Files window's state the menus reflect
    int viewMode = 0;     // 0 icons, 1 list, 2 columns
    int sortBy = 0;       // 0 name, 1 date, 2 size
    bool showHidden = false;
    int selection = 0;    // items selected
    int windows = 1;      // Files windows open
    bool canUndo = true;
    bool canRedo = false;
    std::string user = "Aria";

private:
    struct Note {
        int id;
        std::string app, icon, title, body, group;
        int postedAt; // minutes on the model's own count
        bool unread, urgent;
        std::vector<std::string> actions;
    };
    static std::string ago(int minutes);
    int minuteCount() const { return m_minutes; }
    std::string durationText(int minutes) const;

    Clock m_now;
    int m_minutes = 0;
    int m_layout = 0;
    bool m_wifi = true, m_bt = true, m_airplane = false, m_dark = false, m_night = true, m_dnd = false;
    std::vector<Network> m_networks;
    std::vector<Device> m_devices;
    int m_battery = 82;
    bool m_charging = true;
    int m_powerMode = 1;
    float m_volume = 0.62f;
    bool m_muted = false;
    float m_brightness = 0.72f;
    std::vector<Output> m_outputs;
    int m_viewYear = 2026, m_viewMonth = 10;
    int m_selYear = 2026, m_selMonth = 10, m_selDay = 28;
    std::vector<Note> m_notes;
    std::vector<int> m_toasts;
    int m_nextId = 1;
    Media m_media;
    int m_track = 0;
    int m_trackSeconds = 94;
    Weather m_weather;
};

} // namespace barui
