// The glue: holds the bar's model and answers the window's callbacks by changing it. All the look and all
// the motion are in the .slint files; nothing here draws.
#pragma once

#include "bar_model.h"

#include "menubar.h" // generated from ui/menubar.slint

#include <slint.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace barui {

struct Options {
    bool dark = false;
    std::string accent; // "#rrggbb", or empty for the default
    // What shows where the window draws nothing (the bar is 30 px of a screen-sized window): "#rrggbb",
    // or empty for clear, which a compositor that blends alpha shows as the desktop.
    std::string desk;
    // A blurred copy of the wallpaper, the size of the screen, for the glass to show. Empty: plain glass.
    std::string backdrop;
    bool showBatteryPercent = false;
    // How long a Wi-Fi network or a Bluetooth device takes to connect. Real ones take as long as they take.
    int connectMs = 1100;
    // The countdown of the power dialog, in seconds.
    int powerSeconds = 30;
    // Follow the system's clock. Off, the clock stays at the sample machine's time, so a picture is repeatable.
    bool liveClock = false;
    // Leave the bar empty until `sync()` is called (a bar filled after its window is up gets its arrival seen).
    bool deferFill = false;
};

class App
{
public:
    explicit App(const Options& options);

    slint::ComponentHandle<MenubarWindow> window() const { return m_ui; }
    Model& model() { return m_model; }
    // The window's rows and properties follow the model. Call after changing it from outside.
    void sync();

    // ---- what the system does around the bar (a real backend gets these from its services) ----
    // A notification arrives: it joins the list, and comes up as a banner unless Do Not Disturb is on.
    int notify(const std::string& app, const std::string& icon, const std::string& title, const std::string& body,
               const std::string& group, const std::vector<std::string>& actions = {}, bool urgent = false);
    // The volume, brightness and mute keys: the value changes and the display shows it.
    void volumeKey(float delta);
    void muteKey();
    void brightnessKey(float delta);
    // The clock moves on (a real backend ticks it).
    void advance(int minutes);

    // ---- what the bar asks of the system ----
    // Everything the bar asks for that is not the bar's own business ("power:shutdown", "open:settings",
    // "file:new-folder", "notice:open:Mail") goes here, in order. A real backend acts on them; this one keeps them.
    const std::vector<std::string>& requests() const { return m_requests; }
    std::function<void(const std::string&)> onRequest;
    // What the bar has open changed: the name ("" for nothing), as the window's `popup` says. For a backend that
    // wants to know (to take the keyboard while a menu is open, say); the bar needs nothing from it.
    std::function<void(const std::string&)> onPopup;

private:
    void request(const std::string& what);
    // Menus, power
    void openMenu(const std::string& name);
    void chooseMenu(const std::string& name, int row);
    void power(const std::string& action);
    // Syncs by area
    void syncBar();
    void syncWifi();
    void syncBluetooth();
    void syncBattery();
    void syncSound();
    void syncTiles();
    void syncCalendar();
    void syncMedia();
    void syncWeather();
    void syncNotices();
    void syncToasts();
    void popupChanged(const std::string& name);
    void tick();

    slint::ComponentHandle<MenubarWindow> m_ui;
    Model m_model;
    Options m_options;

    std::shared_ptr<slint::VectorModel<slint::SharedString>> m_menus, m_powerModes, m_weekdays;
    std::shared_ptr<slint::VectorModel<TrayItem>> m_tray;
    std::shared_ptr<slint::VectorModel<MenuEntry>> m_menu;
    std::shared_ptr<slint::VectorModel<WifiNetwork>> m_networks;
    std::shared_ptr<slint::VectorModel<BtDevice>> m_devices;
    std::shared_ptr<slint::VectorModel<AudioDevice>> m_outputs;
    std::shared_ptr<slint::VectorModel<Fact>> m_facts;
    std::shared_ptr<slint::VectorModel<CalCell>> m_cells;
    std::shared_ptr<slint::VectorModel<CalEvent>> m_events;
    std::shared_ptr<slint::VectorModel<QuickTile>> m_tiles;
    std::shared_ptr<slint::VectorModel<::Notice>> m_notices;
    std::shared_ptr<slint::VectorModel<::Notice>> m_toasts;

    std::string m_popup;
    std::vector<std::string> m_requests;
    slint::Timer m_clock;
    // The centre counts as looked at once it has been open a moment (a pointer passing over the bell does not read them).
    slint::Timer m_seenTimer;
    bool m_seen = false;
};

} // namespace barui
