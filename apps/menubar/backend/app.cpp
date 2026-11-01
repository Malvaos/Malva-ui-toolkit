#include "app.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace barui {

namespace {

slint::SharedString str(const std::string& s)
{
    return slint::SharedString(s);
}

slint::Color parseColor(const std::string& text, slint::Color fallback)
{
    if (text.size() == 7 && text[0] == '#') {
        const unsigned long v = std::strtoul(text.c_str() + 1, nullptr, 16);
        return slint::Color::from_rgb_uint8((v >> 16) & 255, (v >> 8) & 255, v & 255);
    }
    return fallback;
}

std::string text(const slint::SharedString& s)
{
    return std::string(std::string_view(s));
}

std::shared_ptr<slint::VectorModel<slint::SharedString>> stringList(const std::vector<std::string>& items)
{
    auto list = std::make_shared<slint::VectorModel<slint::SharedString>>();
    for (const std::string& s : items)
        list->push_back(str(s));
    return list;
}

// ---- model to window shapes
MenuEntry toEntry(const Row& r)
{
    MenuEntry e {};
    e.text = str(r.text);
    e.kind = str(r.kind);
    e.shortcut = str(r.shortcut);
    e.icon = str(r.icon);
    e.checked = r.checked;
    e.enabled = r.enabled;
    e.destructive = r.destructive;
    e.submenu = r.submenu;
    e.parent = r.parent;
    return e;
}

WifiNetwork toRow(const Network& n)
{
    WifiNetwork w {};
    w.name = str(n.name);
    w.strength = n.strength;
    w.secured = n.secured;
    w.connected = n.connected;
    w.connecting = n.connecting;
    w.saved = n.saved;
    return w;
}

BtDevice toRow(const Device& d)
{
    BtDevice b {};
    b.name = str(d.name);
    b.icon = str(d.icon);
    b.connected = d.connected;
    b.connecting = d.connecting;
    b.battery = d.battery;
    return b;
}

AudioDevice toRow(const Output& o)
{
    AudioDevice a {};
    a.name = str(o.name);
    a.icon = str(o.icon);
    a.selected = o.selected;
    return a;
}

CalCell toRow(const Cell& c)
{
    CalCell k {};
    k.day = c.day;
    k.in_month = c.inMonth;
    k.today = c.today;
    k.selected = c.selected;
    k.events = c.events;
    return k;
}

CalEvent toRow(const Event& e)
{
    CalEvent v {};
    v.time = str(e.time);
    v.title = str(e.title);
    v.place = str(e.place);
    v.tone = e.tone;
    return v;
}

QuickTile toRow(const Tile& t)
{
    QuickTile q {};
    q.id = str(t.id);
    q.icon = str(t.icon);
    q.title = str(t.title);
    q.subtitle = str(t.subtitle);
    q.on = t.on;
    q.enabled = t.enabled;
    q.detail = t.detail;
    return q;
}

::Notice toRow(const barui::Notice& n)
{
    ::Notice v {};
    v.id = n.id;
    v.app = str(n.app);
    v.app_icon = str(n.appIcon);
    v.title = str(n.title);
    v.body = str(n.body);
    v.time = str(n.time);
    v.group = str(n.group);
    v.group_index = n.groupIndex;
    v.group_count = n.groupCount;
    v.unread = n.unread;
    v.urgent = n.urgent;
    v.actions = stringList(n.actions);
    return v;
}

bool same(const ::Notice& a, const ::Notice& b)
{
    if (!(a.id == b.id && a.app == b.app && a.app_icon == b.app_icon && a.title == b.title && a.body == b.body
          && a.time == b.time && a.group == b.group && a.group_index == b.group_index && a.group_count == b.group_count
          && a.unread == b.unread && a.urgent == b.urgent))
        return false;
    const std::size_t n = a.actions ? a.actions->row_count() : 0;
    if (n != (b.actions ? b.actions->row_count() : 0))
        return false;
    for (std::size_t i = 0; i < n; ++i) {
        if (*a.actions->row_data(i) != *b.actions->row_data(i))
            return false;
    }
    return true;
}

template<typename T>
bool same(const T& a, const T& b)
{
    return a == b;
}

// The window's rows follow `rows`, changed in place where they differ, so a row that stays keeps whatever
// the window is doing with it (a hover, an animation).
template<typename T>
void assign(const std::shared_ptr<slint::VectorModel<T>>& model, const std::vector<T>& rows)
{
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (i < model->row_count()) {
            if (!same(*model->row_data(i), rows[i]))
                model->set_row_data(i, rows[i]);
        } else {
            model->push_back(rows[i]);
        }
    }
    while (model->row_count() > rows.size())
        model->erase(model->row_count() - 1);
}

template<typename Src, typename Dst, typename F>
std::vector<Dst> convert(const std::vector<Src>& in, F f)
{
    std::vector<Dst> out;
    out.reserve(in.size());
    for (const Src& s : in)
        out.push_back(f(s));
    return out;
}

} // namespace

App::App(const Options& options)
    : m_ui(MenubarWindow::create())
    , m_model(Model::sample())
    , m_options(options)
    , m_menus(std::make_shared<slint::VectorModel<slint::SharedString>>())
    , m_powerModes(stringList({"Saver", "Balanced", "Performance"}))
    , m_weekdays(stringList({"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"}))
    , m_tray(std::make_shared<slint::VectorModel<TrayItem>>())
    , m_menu(std::make_shared<slint::VectorModel<MenuEntry>>())
    , m_networks(std::make_shared<slint::VectorModel<WifiNetwork>>())
    , m_devices(std::make_shared<slint::VectorModel<BtDevice>>())
    , m_outputs(std::make_shared<slint::VectorModel<AudioDevice>>())
    , m_facts(std::make_shared<slint::VectorModel<Fact>>())
    , m_cells(std::make_shared<slint::VectorModel<CalCell>>())
    , m_events(std::make_shared<slint::VectorModel<CalEvent>>())
    , m_tiles(std::make_shared<slint::VectorModel<QuickTile>>())
    , m_notices(std::make_shared<slint::VectorModel<::Notice>>())
    , m_toasts(std::make_shared<slint::VectorModel<::Notice>>())
{
    auto ui = m_ui;
    ui->set_dark(options.dark);
    m_model.setDark(options.dark);
    if (!options.accent.empty())
        ui->set_accent(parseColor(options.accent, slint::Color::from_rgb_uint8(0x33, 0x85, 0xFF)));
    if (!options.desk.empty())
        ui->set_desk(slint::Brush(parseColor(options.desk, slint::Color::from_rgb_uint8(0, 0, 0))));
    if (!options.backdrop.empty()) {
        const slint::Image image = slint::Image::load_from_path(str(options.backdrop));
        const slint::Size<uint32_t> size = image.size();
        if (size.width > 0 && size.height > 0) {
            ui->set_backdrop(image);
            ui->set_backdrop_width(float(size.width));
            ui->set_backdrop_height(float(size.height));
            ui->set_has_backdrop(true);
        }
    }
    ui->set_show_battery_percent(options.showBatteryPercent);
    ui->set_has_battery(true);
    ui->set_power_user(str(m_model.user));
    ui->set_power_countdown(options.powerSeconds);

    ui->set_menus(m_menus);
    ui->set_tray(m_tray);
    ui->set_menu_entries(m_menu);
    ui->set_wifi_networks(m_networks);
    ui->set_bluetooth_devices(m_devices);
    ui->set_power_modes(m_powerModes);
    ui->set_battery_facts(m_facts);
    ui->set_audio_outputs(m_outputs);
    ui->set_cal_weekdays(m_weekdays);
    ui->set_cal_weekdays_short(stringList({"M", "T", "W", "T", "F", "S", "S"}));
    ui->set_cal_cells(m_cells);
    ui->set_cal_events(m_events);
    ui->set_quick_tiles(m_tiles);
    ui->set_notices(m_notices);
    ui->set_toasts(m_toasts);
    if (!options.deferFill)
        sync();

    // ---- the bar and its menus
    ui->on_popup_changed([this](slint::SharedString name) { popupChanged(text(name)); });
    ui->on_menu_requested([this](slint::SharedString name) { openMenu(text(name)); });
    ui->on_menu_chosen([this](slint::SharedString name, int row) { chooseMenu(text(name), row); });

    // ---- Wi-Fi
    ui->on_wifi_toggled([this](bool on) {
        m_model.setWifi(on);
        syncWifi();
        syncTiles();
    });
    ui->on_wifi_network_clicked([this](int i) {
        if (i < 0 || i >= int(m_model.networks().size()))
            return;
        const std::string name = m_model.networks()[i].name;
        if (!m_model.clickNetwork(i)) {
            // Needs a password, which a window of its own asks for: the bar only says so.
            request("wifi:password:" + name);
            return;
        }
        syncWifi();
        syncTiles();
        if (m_model.networks()[i].connecting) {
            slint::Timer::single_shot(std::chrono::milliseconds(m_options.connectMs), [this, i] {
                m_model.finishConnect(i);
                syncWifi();
                syncTiles();
            });
        }
    });
    ui->on_wifi_settings([this] { request("settings:wifi"); });

    // ---- Bluetooth
    ui->on_bluetooth_toggled([this](bool on) {
        m_model.setBluetooth(on);
        syncBluetooth();
        syncTiles();
    });
    ui->on_bluetooth_device_clicked([this](int i) {
        if (i < 0 || i >= int(m_model.devices().size()))
            return;
        m_model.clickDevice(i);
        syncBluetooth();
        syncTiles();
        if (m_model.devices()[i].connecting) {
            slint::Timer::single_shot(std::chrono::milliseconds(m_options.connectMs), [this, i] {
                m_model.finishDevice(i);
                syncBluetooth();
                syncTiles();
            });
        }
    });
    ui->on_bluetooth_settings([this] { request("settings:bluetooth"); });

    // ---- battery
    ui->on_power_mode_picked([this](int i) {
        m_model.setPowerMode(i);
        syncBattery();
    });
    ui->on_battery_settings([this] { request("settings:battery"); });

    // ---- sound and light
    ui->on_volume_edited([this](float v) {
        m_model.setVolume(v);
        syncSound();
    });
    ui->on_mute_toggled([this](bool muted) {
        m_model.setMuted(muted);
        syncSound();
    });
    ui->on_output_clicked([this](int i) {
        m_model.pickOutput(i);
        syncSound();
    });
    ui->on_sound_settings([this] { request("settings:sound"); });
    ui->on_brightness_edited([this](float v) {
        m_model.setBrightness(v);
        syncSound();
    });

    // ---- the calendar
    ui->on_calendar_previous([this] {
        m_model.previousMonth();
        syncCalendar();
    });
    ui->on_calendar_next([this] {
        m_model.nextMonth();
        syncCalendar();
    });
    ui->on_calendar_today([this] {
        m_model.goToday();
        syncCalendar();
    });
    ui->on_calendar_picked([this](int i) {
        m_model.pickCell(i);
        syncCalendar();
    });
    ui->on_calendar_open([this] { request("open:calendar"); });

    // ---- Control Center
    ui->on_tile_toggled([this](int i) {
        const auto tiles = m_model.tiles();
        if (i < 0 || i >= int(tiles.size()) || !tiles[i].enabled)
            return;
        m_model.toggleTile(tiles[i].id);
        sync();
        m_ui->set_dark(m_model.dark());
    });
    ui->on_tile_detail([this](int i) {
        const auto tiles = m_model.tiles();
        if (i >= 0 && i < int(tiles.size()))
            request("tile-detail:" + tiles[i].id);
    });
    ui->on_control_action([this](slint::SharedString a) {
        const std::string action = text(a);
        request(action == "lock" ? "power:lock" : action == "settings" ? "open:settings" : action);
    });
    ui->on_media_previous([this] {
        m_model.previousTrack();
        syncMedia();
    });
    ui->on_media_play_pause([this] {
        m_model.playPause();
        syncMedia();
    });
    ui->on_media_next([this] {
        m_model.nextTrack();
        syncMedia();
    });

    // ---- notifications
    ui->on_notice_clicked([this](int i) {
        const auto row = m_notices->row_data(i);
        if (!row)
            return;
        request("notice:open:" + text(row->app));
        m_model.dismiss(row->id);
        m_ui->invoke_close_popup();
        sync();
    });
    ui->on_notice_action([this](int i, int a) {
        const auto row = m_notices->row_data(i);
        if (!row || a < 0 || std::size_t(a) >= row->actions->row_count())
            return;
        // The card is sliding away; the window says `removed` when it has gone, and the row goes then.
        request("notice:action:" + text(row->app) + ":" + text(*row->actions->row_data(a)));
    });
    ui->on_notice_removed([this](int i) {
        const auto row = m_notices->row_data(i);
        if (!row)
            return;
        m_model.dismiss(row->id);
        sync();
    });
    ui->on_group_cleared([this](slint::SharedString group) {
        m_model.clearGroup(text(group));
        sync();
    });
    ui->on_notices_cleared([this] {
        m_model.clearAll();
        sync();
    });
    ui->on_dnd_toggled([this](bool on) {
        m_model.setDnd(on);
        syncTiles();
    });
    ui->on_toast_clicked([this](int i) {
        const auto row = m_toasts->row_data(i);
        if (!row)
            return;
        // The banner is leaving by itself; the notice is answered, so it goes from the list too.
        request("notice:open:" + text(row->app));
        m_model.dismiss(row->id);
        syncNotices();
    });
    ui->on_toast_action([this](int i, int a) {
        const auto row = m_toasts->row_data(i);
        if (!row || a < 0 || std::size_t(a) >= row->actions->row_count())
            return;
        request("notice:action:" + text(row->app) + ":" + text(*row->actions->row_data(a)));
        m_model.dismiss(row->id);
        syncNotices();
    });
    ui->on_toast_closed([](int) {
        // Dismissing a banner leaves the notice in the list.
    });
    ui->on_toast_expired([this](int i) {
        const auto row = m_toasts->row_data(i);
        if (!row)
            return;
        m_model.expireToast(row->id);
        m_toasts->erase(i);
    });

    // ---- power
    ui->on_power_chosen([this](slint::SharedString a) { power(text(a)); });
    ui->on_power_cancelled([this] {});

    // The clock: a tick a second, for the playing track and (if asked) the time of day.
    m_clock.start(slint::TimerMode::Repeated, std::chrono::seconds(1), [this] { tick(); });
}

// ------------------------------------------------------------------------------------------ requests
void App::request(const std::string& what)
{
    m_requests.push_back(what);
    if (onRequest)
        onRequest(what);
}

void App::power(const std::string& action)
{
    // The dialog has done its counting; this is the choice, made.
    request("power:" + action);
}

// ------------------------------------------------------------------------------------------ menus
void App::openMenu(const std::string& name)
{
    std::vector<MenuEntry> entries;
    for (const Row& r : m_model.menuFor(name))
        entries.push_back(toEntry(r));
    // The window reads the rows as soon as this handler returns.
    assign(m_menu, entries);
}

void App::chooseMenu(const std::string& name, int row)
{
    const std::string action = m_model.perform(name, row);
    if (action.empty())
        return;
    if (action == "power:restart" || action == "power:shutdown" || action == "power:logout") {
        // These ask first, with a count-down to the real thing.
        m_ui->invoke_ask_power(str(action.substr(6)));
    } else {
        request(action);
    }
    // What the choice changed in the model shows (a layout chosen, a view mode ticked).
    syncBar();
}

void App::popupChanged(const std::string& name)
{
    const std::string before = m_popup;
    m_popup = name;
    if (onPopup)
        onPopup(name);
    if (name == "bell") {
        m_seen = false;
        m_seenTimer.start(slint::TimerMode::SingleShot, std::chrono::milliseconds(600), [this] { m_seen = true; });
    }
    if (name == "clock" || name == "bell") {
        // The calendar opens on today.
        m_model.goToday();
        syncCalendar();
    }
    if (before == "bell" && name != "bell") {
        m_seenTimer.stop();
        // Having looked at them, they are read.
        if (m_seen) {
            m_model.markAllRead();
            syncNotices();
            syncBar();
        }
    }
}

// ------------------------------------------------------------------------------------------ the system's side
int App::notify(const std::string& app, const std::string& icon, const std::string& title, const std::string& body,
                const std::string& group, const std::vector<std::string>& actions, bool urgent)
{
    const int id = m_model.post(app, icon, title, body, group, actions, urgent);
    syncNotices();
    syncToasts();
    syncBar();
    return id;
}

void App::volumeKey(float delta)
{
    m_model.setVolume(m_model.volume() + delta);
    syncSound();
    const int percent = int(m_model.volume() * 100 + 0.5f);
    m_ui->invoke_show_osd(str(m_model.volume() <= 0.001f ? "mute" : "volume"), m_model.volume(), str("Volume " + std::to_string(percent) + "%"));
}

void App::muteKey()
{
    m_model.setMuted(!m_model.muted());
    syncSound();
    m_ui->invoke_show_osd(str(m_model.muted() ? "mute" : "volume"), m_model.muted() ? 0.0f : m_model.volume(),
                          str(m_model.muted() ? "Muted" : "Volume " + std::to_string(int(m_model.volume() * 100 + 0.5f)) + "%"));
}

void App::brightnessKey(float delta)
{
    m_model.setBrightness(m_model.brightness() + delta);
    syncSound();
    m_ui->invoke_show_osd(str("brightness"), m_model.brightness(), str("Brightness " + std::to_string(int(m_model.brightness() * 100 + 0.5f)) + "%"));
}

void App::advance(int minutes)
{
    m_model.advance(minutes);
    sync();
}

void App::tick()
{
    m_model.tickMedia(1);
    syncMedia();
    if (m_options.liveClock) {
        const std::time_t t = std::time(nullptr);
        std::tm local {};
        localtime_r(&t, &local);
        if (local.tm_hour != m_model.now().hour || local.tm_min != m_model.now().minute) {
            m_model.setTime(local.tm_hour, local.tm_min);
            syncBar();
        }
    }
}

// ------------------------------------------------------------------------------------------ syncing
void App::sync()
{
    syncBar();
    syncWifi();
    syncBluetooth();
    syncBattery();
    syncSound();
    syncTiles();
    syncCalendar();
    syncMedia();
    syncWeather();
    syncNotices();
    syncToasts();
}

void App::syncBar()
{
    std::vector<slint::SharedString> menus;
    for (const std::string& m : m_model.menus())
        menus.push_back(str(m));
    assign(m_menus, menus);
    assign(m_tray, std::vector<TrayItem> {TrayItem {str("record"), str("Recording"), true}, TrayItem {str("clipboard"), str("Clipboard"), false}});
    m_ui->set_layout_text(str(m_model.layoutText()));
    m_ui->set_clock_text(str(m_model.clockText()));
    m_ui->set_unread(m_model.unread());
    m_ui->set_date_text(str(m_model.dateText()));
    m_ui->set_dark(m_model.dark());
}

void App::syncWifi()
{
    m_ui->set_wifi_enabled(m_model.wifiEnabled());
    m_ui->set_wifi_connected(m_model.wifiConnected());
    m_ui->set_wifi_strength(m_model.wifiStrength());
    assign(m_networks, convert<Network, WifiNetwork>(m_model.networks(), [](const Network& n) { return toRow(n); }));
}

void App::syncBluetooth()
{
    m_ui->set_bluetooth_enabled(m_model.bluetoothEnabled());
    assign(m_devices, convert<Device, BtDevice>(m_model.devices(), [](const Device& d) { return toRow(d); }));
}

void App::syncBattery()
{
    m_ui->set_battery(m_model.battery());
    m_ui->set_charging(m_model.charging());
    m_ui->set_battery_status(str(m_model.batteryStatus()));
    m_ui->set_battery_detail(str(m_model.batteryDetail()));
    m_ui->set_battery_source(str(m_model.batterySource()));
    m_ui->set_power_mode(m_model.powerMode());
    assign(m_facts, std::vector<Fact> {Fact {str("Health"), str("94 %")}, Fact {str("Cycles"), str("212")}});
}

void App::syncSound()
{
    m_ui->set_volume(m_model.volume());
    m_ui->set_muted(m_model.muted());
    m_ui->set_brightness(m_model.brightness());
    assign(m_outputs, convert<Output, AudioDevice>(m_model.outputs(), [](const Output& o) { return toRow(o); }));
}

void App::syncTiles()
{
    m_ui->set_dnd(m_model.dnd());
    m_ui->set_wifi_enabled(m_model.wifiEnabled());
    m_ui->set_bluetooth_enabled(m_model.bluetoothEnabled());
    m_ui->set_wifi_connected(m_model.wifiConnected());
    m_ui->set_wifi_strength(m_model.wifiStrength());
    m_ui->set_dark(m_model.dark());
    assign(m_tiles, convert<Tile, QuickTile>(m_model.tiles(), [](const Tile& t) { return toRow(t); }));
}

void App::syncCalendar()
{
    m_ui->set_cal_title(str(m_model.monthTitle()));
    m_ui->set_cal_day_title(str(m_model.selectedTitle()));
    assign(m_cells, convert<Cell, CalCell>(m_model.cells(), [](const Cell& c) { return toRow(c); }));
    assign(m_events, convert<Event, CalEvent>(m_model.events(), [](const Event& e) { return toRow(e); }));
}

void App::syncMedia()
{
    const Media& m = m_model.media();
    ::Media v {};
    v.has_media = m.has;
    v.title = str(m.title);
    v.artist = str(m.artist);
    v.app = str(m.app);
    v.playing = m.playing;
    v.progress = m.progress;
    v.position = str(m.position);
    v.length = str(m.length);
    v.has_art = false;
    if (!(m_ui->get_media() == v))
        m_ui->set_media(v);
}

void App::syncWeather()
{
    const Weather& w = m_model.weather();
    ::Weather v {};
    v.has_weather = !w.city.empty();
    v.city = str(w.city);
    v.icon = str(w.icon);
    v.temp = str(w.temp);
    v.condition = str(w.condition);
    v.range = str(w.range);
    auto hours = std::make_shared<slint::VectorModel<HourForecast>>();
    for (const Hour& h : w.hours)
        hours->push_back(HourForecast {str(h.label), str(h.icon), str(h.temp)});
    v.hours = hours;
    m_ui->set_weather(v);
}

void App::syncNotices()
{
    m_ui->set_unread(m_model.unread());
    const std::vector<barui::Notice> now = m_model.notices();
    std::vector<::Notice> want;
    for (const barui::Notice& n : now)
        want.push_back(toRow(n));

    // Rows are matched by id, so a card that stays is the same card (its slide, its fold), and one that
    // arrives or leaves does only that.
    for (std::size_t i = m_notices->row_count(); i-- > 0;) {
        const int id = m_notices->row_data(i)->id;
        if (std::none_of(want.begin(), want.end(), [id](const ::Notice& n) { return n.id == id; }))
            m_notices->erase(i);
    }
    for (std::size_t i = 0; i < want.size(); ++i) {
        if (i < m_notices->row_count() && m_notices->row_data(i)->id == want[i].id) {
            if (!same(*m_notices->row_data(i), want[i]))
                m_notices->set_row_data(i, want[i]);
        } else {
            m_notices->insert(i, want[i]);
        }
    }
    while (m_notices->row_count() > want.size())
        m_notices->erase(m_notices->row_count() - 1);
}

void App::syncToasts()
{
    // New banners go on top. A banner that has gone from the model stays until the window says it has
    // finished leaving (`toast-expired`).
    for (int id : m_model.toastIds()) {
        bool present = false;
        for (std::size_t i = 0; i < m_toasts->row_count(); ++i)
            present = present || m_toasts->row_data(i)->id == id;
        if (!present)
            m_toasts->insert(0, toRow(m_model.toast(id)));
    }
}

} // namespace barui
