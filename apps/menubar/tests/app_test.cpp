// The window and the stand-in backend together, driven the way a person drives them: a pointer that
// moves, hovers and clicks, and keys, through the window's own event handling (no window appears; time
// is stepped by hand, so animations and timers run when the test says).
// It needs a Slint build that includes the testing backend (the SDK's does).
//
// What this can see is what the window reports: its rows and properties, what is open, where the open
// menu is, whether the volume display is up. What it cannot see is what is drawn: that is for the
// screenshots and recordings.
#include "../backend/app.h"

#include <private/slint_tests_helpers.h>

#include <cmath>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

using namespace barui;
namespace st = slint::private_api::testing;

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond);          \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

static MenubarWindow* ui = nullptr;
static App* app = nullptr;
static const float W = 1440, H = 900;

static std::string text(const slint::SharedString& s) { return std::string(std::string_view(s)); }
template<typename T>
static std::vector<T> rowsOf(const std::shared_ptr<slint::Model<T>>& model)
{
    std::vector<T> out;
    for (std::size_t i = 0; i < model->row_count(); ++i)
        out.push_back(*model->row_data(i));
    return out;
}
static std::string popup() { return text(ui->get_popup()); }
static std::vector<MenuEntry> entries() { return rowsOf(ui->get_menu_entries()); }
static int entryRow(const std::string& label)
{
    int i = 0;
    for (const MenuEntry& e : entries()) {
        if (text(e.text) == label)
            return i;
        ++i;
    }
    return -1;
}
static std::size_t requests() { return app->requests().size(); }
static std::string lastRequest() { return app->requests().empty() ? "" : app->requests().back(); }
static bool asked(const std::string& what)
{
    for (const std::string& r : app->requests()) {
        if (r == what)
            return true;
    }
    return false;
}

// ---- time and the pointer
static float px = 0, py = 0;
// Time moves in frames: a timer that fires in the middle of a long wait starts its animations then,
// not at the end of the wait.
static void wait(int ms)
{
    for (; ms > 0; ms -= 16)
        st::mock_elapsed_time(ms >= 16 ? 16 : ms);
}
static void moveTo(float x, float y)
{
    px = x;
    py = y;
    ui->window().dispatch_pointer_move_event(slint::LogicalPosition({x, y}));
}
static void press(slint::PointerEventButton b = slint::PointerEventButton::Left) { ui->window().dispatch_pointer_press_event(slint::LogicalPosition({px, py}), b); }
static void release(slint::PointerEventButton b = slint::PointerEventButton::Left) { ui->window().dispatch_pointer_release_event(slint::LogicalPosition({px, py}), b); }
static void click(float x, float y)
{
    moveTo(x, y);
    wait(20);
    press();
    wait(20);
    release();
    wait(20);
}
// Takes hold at (x, y), carries to (x2, y2) in small steps, lets go.
static void drag(float x, float y, float x2, float y2)
{
    moveTo(x, y);
    wait(40);
    press();
    wait(20);
    const int steps = 20;
    for (int k = 1; k <= steps; ++k) {
        moveTo(x + (x2 - x) * k / steps, y + (y2 - y) * k / steps);
        wait(16);
    }
    wait(40);
    release();
    wait(40);
}
static void key(std::u8string_view k)
{
    const slint::SharedString s(std::string_view(reinterpret_cast<const char*>(k.data()), k.size()));
    ui->window().dispatch_key_press_event(s);
    ui->window().dispatch_key_release_event(s);
    wait(20);
}
namespace kc = slint::platform::key_codes;
static void desk() { moveTo(700, 500); wait(20); }

// ---- where things are on the bar, found the way a person finds them: by sweeping along it with something
// open, which opens whatever the pointer arrives on. An item is where its name first appears.
static std::map<std::string, std::pair<float, float>> bar; // name -> [first x, next item's first x)
static std::vector<std::string> barOrder;
static void measureBar()
{
    ui->invoke_show_popup("clock");
    wait(300);
    std::string current = popup();
    std::string previous;
    float from = 0;
    barOrder.clear();
    for (float x = 0; x < W; x += 2) {
        moveTo(x, 15);
        wait(16);
        if (popup() != current) {
            if (!previous.empty() || !barOrder.empty())
                bar[current].second = x;
            current = popup();
            bar[current].first = x;
            barOrder.push_back(current);
            previous = current;
        }
        from = x;
    }
    bar[current].second = from;
    ui->invoke_close_popup();
    desk();
    wait(300);
}
static float cx(const std::string& name)
{
    const auto it = bar.find(name);
    // (An item is no wider than about 70 px; after the last menu there is a gap to the tray.)
    return it == bar.end() ? -1000 : (it->second.first + std::fmin(it->second.second, it->second.first + 70)) / 2 - 1;
}
static void clickBar(const std::string& name) { click(cx(name), 15); }

// ---- where a row of the open menu is (items 28 px, headers 26, separators 11, from 6 px inside its card)
static float rowHeight(const MenuEntry& e) { return text(e.kind) == "separator" ? 11 : text(e.kind) == "header" ? 26 : 28; }
static float menuRowY(int row)
{
    float y = ui->get_menu_y() + 6;
    const auto rows = entries();
    for (int j = 0; j < row; ++j) {
        if (rows[j].parent == -1)
            y += rowHeight(rows[j]);
    }
    return y + rowHeight(rows[row]) / 2;
}
static void clickMenuRow(int row) { click(ui->get_menu_x() + 40, menuRowY(row)); }

static bool open(const std::string& name)
{
    desk();
    if (popup() != "") {
        ui->invoke_close_popup();
        wait(300);
    }
    clickBar(name);
    wait(300);
    return popup() == name;
}
static void closeAll()
{
    ui->invoke_close_popup();
    desk();
    wait(400);
}

int main()
{
    slint::cbindgen_private::slint_testing_init_backend();

    Options options;
    options.connectMs = 1000;
    options.powerSeconds = 5;
    App theApp(options);
    app = &theApp;
    auto handle = app->window();
    ui = &*handle;
    ui->window().set_size(slint::LogicalSize({W, H}));
    ui->show();
    wait(100);
    std::vector<std::string> popupsSeen;
    app->onPopup = [&](const std::string& name) { popupsSeen.push_back(name); };

    // ---- What the bar shows to begin with.
    {
        const auto menus = rowsOf(ui->get_menus());
        CHECK(menus.size() == 7 && text(menus[0]) == "Files" && text(menus[1]) == "File");
        CHECK(text(ui->get_clock_text()) == "Wed 28 Oct  5:01 AM");
        CHECK(text(ui->get_layout_text()) == "EN");
        CHECK(ui->get_unread() == 2);
        CHECK(ui->get_battery() == 82 && ui->get_charging() && ui->get_has_battery());
        CHECK(ui->get_wifi_connected() && ui->get_wifi_strength() == 3 && ui->get_wifi_enabled() && ui->get_bluetooth_enabled());
        const auto tray = rowsOf(ui->get_tray());
        CHECK(tray.size() == 2 && text(tray[0].icon) == "record" && tray[0].active && !tray[1].active);
        CHECK(ui->get_wifi_networks()->row_count() == 5 && ui->get_bluetooth_devices()->row_count() == 4);
        CHECK(ui->get_audio_outputs()->row_count() == 3 && ui->get_quick_tiles()->row_count() == 6);
        CHECK(ui->get_cal_cells()->row_count() == 42 && ui->get_cal_events()->row_count() == 4);
        CHECK(text(ui->get_cal_title()) == "October 2026" && text(ui->get_cal_day_title()) == "Wednesday, 28 October");
        CHECK(ui->get_notices()->row_count() == 5 && ui->get_toasts()->row_count() == 0);
        CHECK(ui->get_weather().has_weather && text(ui->get_weather().city) == "Hyderabad" && ui->get_weather().hours->row_count() == 5);
        CHECK(ui->get_media().has_media && text(ui->get_media().title) == "Golden Hour" && ui->get_media().playing);
        CHECK(popup() == "" && !ui->get_something_open() && !ui->get_power_open() && !ui->get_osd_showing());
        CHECK(ui->get_toasts_height() == 0);
        CHECK(text(ui->get_power_user()) == "Aria");
    }

    // ---- The bar's items, in order, found by sweeping the pointer along it with something open.
    measureBar();
    {
        const std::vector<std::string> expect = {"system", "menu-0", "menu-1", "menu-2", "menu-3", "menu-4", "menu-5", "menu-6", "tray-0", "tray-1",
                                                 "layout", "bluetooth", "wifi", "volume", "battery", "control", "bell", "clock"};
        CHECK(barOrder == expect);
        for (std::size_t i = 1; i < barOrder.size(); ++i)
            CHECK(bar[barOrder[i]].first > bar[barOrder[i - 1]].first);
        CHECK(cx("system") < 40 && cx("clock") > W - 150);
        CHECK(popup() == "");
    }
    popupsSeen.clear();

    // ---- Click a menu's title: it opens, and holds that menu's rows, built when it opened.
    CHECK(open("menu-1"));
    CHECK(ui->get_something_open() && popupsSeen.back() == "menu-1");
    CHECK(entries().size() == app->model().menuFor("menu-1").size());
    CHECK(entryRow("New Window") == 0 && entryRow("Get Info") > 0 && entryRow("Move to Bin") > 0);
    CHECK(!entries()[entryRow("Get Info")].enabled && entries()[entryRow("Move to Bin")].destructive);
    CHECK(text(entries()[entryRow("New Window")].shortcut) == "Ctrl+N");
    CHECK(ui->get_menu_x() > 40 && ui->get_menu_x() < 120 && ui->get_menu_y() > 30 && ui->get_menu_width() >= 210);
    // A row that cannot be chosen does nothing, and the menu stays.
    const std::size_t before = requests();
    clickMenuRow(entryRow("Get Info"));
    wait(300);
    CHECK(popup() == "menu-1" && requests() == before);
    // Nor does a click on a line between rows, or on a header: the menu is not the desk behind it.
    {
        const int sep = [] { int i = 0; for (const MenuEntry& e : entries()) { if (text(e.kind) == "separator") return i; ++i; } return -1; }();
        CHECK(sep > 0);
        clickMenuRow(sep);
        wait(300);
        CHECK(popup() == "menu-1" && requests() == before);
    }
    // A click on the title again closes it.
    clickBar("menu-1");
    wait(300);
    CHECK(popup() == "");
    // The menu is built afresh each time: with something selected, the same row can be chosen.
    app->model().selection = 3;
    CHECK(open("menu-1"));
    CHECK(entries()[entryRow("Get Info")].enabled);
    clickMenuRow(entryRow("Get Info"));
    wait(300);
    CHECK(popup() == "" && lastRequest() == "file:info");
    // A row chosen: the menu closes and the system is asked.
    CHECK(open("menu-1"));
    clickMenuRow(entryRow("New Folder"));
    wait(300);
    CHECK(popup() == "" && lastRequest() == "file:new-folder");

    // ---- A click on the desk closes it, and chooses nothing.
    CHECK(open("menu-1"));
    const std::size_t asks = requests();
    click(700, 500);
    wait(300);
    CHECK(popup() == "" && requests() == asks);

    // ---- With a menu open, the pointer arriving on another title opens that one (and a sweep walks through).
    CHECK(open("menu-1"));
    moveTo(cx("menu-2"), 15);
    wait(300);
    CHECK(popup() == "menu-2" && text(entries()[0].text) == "Undo");
    moveTo(cx("menu-3"), 15);
    wait(300);
    CHECK(popup() == "menu-3" && entryRow("as Icons") == 0 && entries()[0].checked);
    moveTo(cx("wifi"), 15);
    wait(300);
    CHECK(popup() == "wifi");
    moveTo(cx("menu-4"), 15);
    wait(300);
    CHECK(popup() == "menu-4" && text(entries()[0].text) == "Back");
    // The pointer going down to the desk leaves it open.
    moveTo(700, 500);
    wait(300);
    CHECK(popup() == "menu-4");
    click(700, 500);
    wait(300);
    CHECK(popup() == "");
    // Without anything open, the pointer on a title only lights it.
    moveTo(cx("menu-2"), 15);
    wait(300);
    CHECK(popup() == "");

    // ---- Keys: down through the rows (past what cannot be chosen), return to choose, escape to close.
    CHECK(open("menu-2"));
    CHECK(ui->get_menu_cursor() == -1);
    key(kc::DownArrow);
    CHECK(ui->get_menu_cursor() == 0); // Undo
    key(kc::DownArrow);
    CHECK(ui->get_menu_cursor() == entryRow("Cut")); // Redo is not available, nor is the line between
    key(kc::UpArrow);
    CHECK(ui->get_menu_cursor() == 0);
    key(kc::UpArrow);
    CHECK(ui->get_menu_cursor() == entryRow("Find…")); // round to the last
    key(kc::DownArrow);
    CHECK(ui->get_menu_cursor() == 0);
    key(kc::Return);
    wait(300);
    CHECK(popup() == "" && lastRequest() == "edit:undo");
    // Undone, so Redo is there and Undo is not.
    CHECK(open("menu-2"));
    CHECK(!entries()[entryRow("Undo")].enabled && entries()[entryRow("Redo")].enabled);
    key(kc::Escape);
    wait(300);
    CHECK(popup() == "");

    // ---- Left and right walk along the bar, round at the ends.
    CHECK(open("menu-2"));
    key(kc::RightArrow);
    wait(200);
    CHECK(popup() == "menu-3");
    key(kc::LeftArrow);
    key(kc::LeftArrow);
    wait(200);
    CHECK(popup() == "menu-1");
    CHECK(text(entries()[0].text) == "New Window");
    CHECK(open("system"));
    key(kc::LeftArrow);
    wait(200);
    CHECK(popup() == "clock");
    key(kc::RightArrow);
    wait(200);
    CHECK(popup() == "system");
    key(kc::Escape);
    wait(300);
    CHECK(popup() == "");

    // ---- A row with a flyout, by keys: right goes in, left comes out, return chooses.
    CHECK(open("menu-1"));
    const int recent = entryRow("Open Recent");
    CHECK(recent > 0 && entries()[recent].submenu && entries()[recent + 1].parent == recent);
    for (int i = 0; i < 4 && ui->get_menu_cursor() != recent; ++i)
        key(kc::DownArrow);
    CHECK(ui->get_menu_cursor() == recent && ui->get_menu_sub() == -1);
    key(kc::RightArrow);
    CHECK(ui->get_menu_sub() == recent && ui->get_menu_sub_cursor() == recent + 1);
    key(kc::DownArrow);
    CHECK(ui->get_menu_sub_cursor() == recent + 2);
    key(kc::LeftArrow);
    CHECK(ui->get_menu_sub() == -1 && popup() == "menu-1");
    key(kc::RightArrow);
    key(kc::DownArrow);
    key(kc::Return);
    wait(300);
    CHECK(popup() == "" && lastRequest() == "file:open-recent:Downloads");

    // ---- The same by pointer: rest on the row, the flyout comes, move into it, click its second row.
    CHECK(open("menu-1"));
    moveTo(ui->get_menu_x() + 40, menuRowY(recent));
    wait(100);
    CHECK(ui->get_menu_cursor() == recent);
    wait(500);
    CHECK(ui->get_menu_sub() == recent);
    CHECK(ui->get_flyout_x() > ui->get_menu_x() + 150);
    moveTo(ui->get_flyout_x() + 40, menuRowY(recent));
    wait(60);
    moveTo(ui->get_flyout_x() + 40, menuRowY(recent) + 28);
    wait(400);
    CHECK(ui->get_menu_sub() == recent && ui->get_menu_sub_cursor() == recent + 2); // still open: the pointer is in it
    press();
    wait(20);
    release();
    wait(300);
    CHECK(popup() == "" && lastRequest() == "file:open-recent:Downloads");
    // Passing over the row without stopping opens nothing, and moving to another row closes it.
    CHECK(open("menu-1"));
    moveTo(ui->get_menu_x() + 40, menuRowY(recent));
    wait(60);
    moveTo(ui->get_menu_x() + 40, menuRowY(0));
    wait(500);
    CHECK(ui->get_menu_sub() == -1);
    closeAll();

    // ---- What a choice changes shows when the menu is next opened: a ticked view mode, a layout.
    CHECK(open("menu-3"));
    clickMenuRow(entryRow("as List"));
    wait(300);
    CHECK(lastRequest() == "view:mode:1" && app->model().viewMode == 1);
    CHECK(open("menu-3"));
    CHECK(entries()[entryRow("as List")].checked && !entries()[entryRow("as Icons")].checked);
    closeAll();
    CHECK(open("layout"));
    CHECK(text(entries()[0].kind) == "header" && entries()[entryRow("English (US)")].checked);
    clickMenuRow(entryRow("हिन्दी (Hindi)"));
    wait(300);
    CHECK(popup() == "" && text(ui->get_layout_text()) == "HI" && lastRequest() == "layout:2");
    CHECK(open("layout"));
    CHECK(entries()[entryRow("हिन्दी (Hindi)")].checked);
    closeAll();
    app->model().setLayout(0);
    app->sync();

    // ---- The system menu and the power rows.
    CHECK(open("system"));
    CHECK(entryRow("Sleep") > 0 && entryRow("Shut Down…") > 0);
    clickMenuRow(entryRow("Sleep"));
    wait(300);
    CHECK(popup() == "" && lastRequest() == "power:sleep" && !ui->get_power_open());
    CHECK(open("system"));
    clickMenuRow(entryRow("Lock Screen"));
    wait(300);
    CHECK(lastRequest() == "power:lock" && !ui->get_power_open());
    // Restart asks first, and counts down, then it is done.
    CHECK(open("system"));
    clickMenuRow(entryRow("Restart…"));
    wait(400);
    CHECK(popup() == "" && ui->get_power_open() && text(ui->get_power_pending()) == "restart");
    CHECK(ui->get_power_remaining() == 5);
    const std::size_t beforePower = requests();
    wait(2000);
    CHECK(ui->get_power_open() && ui->get_power_remaining() == 3 && requests() == beforePower);
    wait(3100);
    CHECK(!ui->get_power_open() && lastRequest() == "power:restart");
    // Escape in the middle of the count cancels it, and nothing is done.
    CHECK(open("system"));
    clickMenuRow(entryRow("Shut Down…"));
    wait(400);
    CHECK(ui->get_power_open() && text(ui->get_power_pending()) == "shutdown");
    wait(1500);
    key(kc::Escape);
    // (The countdown is over at once, but it fades as a countdown, not as the list of choices.)
    CHECK(!ui->get_power_open() && text(ui->get_power_pending()).empty());
    wait(100);
    CHECK(text(ui->get_power_view()) == "shutdown");
    wait(400);
    CHECK(!ui->get_power_open() && !asked("power:shutdown") && text(ui->get_power_view()).empty());
    // The dialog opened without a choice waits for one, and a click on the glass outside the card cancels it.
    ui->invoke_open_power();
    wait(400);
    CHECK(ui->get_power_open() && text(ui->get_power_pending()).empty());
    wait(3000);
    CHECK(ui->get_power_open()); // nothing counts down until something is chosen
    click(60, 600);
    wait(400);
    CHECK(!ui->get_power_open());
    desk();

    // ---- Wi-Fi: click the icon; click a network; the switch.
    CHECK(open("wifi"));
    const float wifiX = cx("wifi");
    const float cardX = std::fmax(8.0f, std::fmin(wifiX - 165, W - 330 - 8));
    const auto rowY = [](int i) { return 36 + 61 + 46.0f * i + 23; };
    click(cardX + 120, rowY(1)); // Cafe Guest: open
    wait(100);
    {
        const auto nets = rowsOf(ui->get_wifi_networks());
        CHECK(nets[1].connecting && nets[0].connected);
    }
    CHECK(popup() == "wifi"); // a popover stays while something in it is done
    wait(1100);
    {
        const auto nets = rowsOf(ui->get_wifi_networks());
        CHECK(nets[1].connected && !nets[1].connecting && !nets[0].connected && nets[1].saved);
        CHECK(ui->get_wifi_strength() == 2 && ui->get_wifi_connected());
        CHECK(text(rowsOf(ui->get_quick_tiles())[0].subtitle) == "Cafe Guest");
    }
    // A secured network that is not saved needs a password: the bar says so and changes nothing.
    click(cardX + 120, rowY(3));
    wait(200);
    CHECK(lastRequest() == "wifi:password:Printer-Direct" && !rowsOf(ui->get_wifi_networks())[3].connecting);
    // A saved one just connects.
    click(cardX + 120, rowY(2));
    wait(1200);
    CHECK(rowsOf(ui->get_wifi_networks())[2].connected && !rowsOf(ui->get_wifi_networks())[1].connected);
    // The switch turns it off, and on.
    click(cardX + 330 - 35, 36 + 10 + 22);
    wait(200);
    CHECK(!ui->get_wifi_enabled() && !app->model().wifiEnabled() && !ui->get_wifi_connected());
    CHECK(!rowsOf(ui->get_quick_tiles())[0].on);
    click(cardX + 330 - 35, 36 + 10 + 22);
    wait(200);
    CHECK(ui->get_wifi_enabled() && app->model().wifiEnabled());
    closeAll();

    // ---- Bluetooth: a device connects after a moment, and the tile says so.
    CHECK(open("bluetooth"));
    {
        const float btX = std::fmax(8.0f, std::fmin(cx("bluetooth") - 165, W - 330 - 8));
        click(btX + 120, rowY(2)); // Pixel 8
        wait(100);
        CHECK(rowsOf(ui->get_bluetooth_devices())[2].connecting);
        wait(1100);
        const auto devices = rowsOf(ui->get_bluetooth_devices());
        CHECK(devices[2].connected && !devices[2].connecting && devices[2].battery >= 0);
        CHECK(text(rowsOf(ui->get_quick_tiles())[1].subtitle) == "3 devices");
        click(btX + 120, rowY(0)); // Studio Buds, connected: it lets go
        wait(200);
        CHECK(!rowsOf(ui->get_bluetooth_devices())[0].connected);
        click(btX + 330 - 35, 36 + 10 + 22); // the switch
        wait(200);
        CHECK(!ui->get_bluetooth_enabled() && !app->model().bluetoothEnabled());
        CHECK(app->model().devicesConnected() == 0);
        click(btX + 330 - 35, 36 + 10 + 22);
        wait(200);
        CHECK(ui->get_bluetooth_enabled());
    }
    closeAll();

    // ---- Battery and sound: what they say, and what choosing does.
    CHECK(open("battery"));
    CHECK(text(ui->get_battery_status()) == "Charging" && text(ui->get_battery_detail()) == "36 min until full" && text(ui->get_battery_source()) == "Power adapter");
    ui->invoke_power_mode_picked(2);
    CHECK(app->model().powerMode() == 2 && ui->get_power_mode() == 2);
    closeAll();
    app->advance(20);
    CHECK(ui->get_battery() == 92 && text(ui->get_clock_text()) == "Wed 28 Oct  5:21 AM");
    app->model().setBattery(12, false);
    app->sync();
    CHECK(open("battery"));
    CHECK(text(ui->get_battery_status()) == "Low battery" && ui->get_battery() == 12 && !ui->get_charging());
    closeAll();
    app->model().setBattery(82, true);
    app->model().setTime(5, 1);
    app->sync();

    CHECK(open("volume"));
    ui->invoke_output_clicked(1);
    CHECK(rowsOf(ui->get_audio_outputs())[1].selected && !rowsOf(ui->get_audio_outputs())[0].selected);
    ui->invoke_mute_toggled(true);
    CHECK(ui->get_muted() && app->model().muted());
    ui->invoke_volume_edited(0.3f);
    CHECK(!ui->get_muted() && std::fabs(ui->get_volume() - 0.3f) < 0.001f); // moving the slider unmutes
    closeAll();

    // ---- The calendar: months, days, today.
    CHECK(open("clock"));
    {
        const float calX = std::fmax(8.0f, std::fmin(cx("clock") - 176, W - 352 - 8));
        const auto cell = [&](int i) { return std::pair<float, float>{calX + 12 + (i % 7) * (328.0f / 7) + 23, 36 + 56 + 24 + (i / 7) * 36 + 18}; };
        click(calX + 40, 36 + 30); // on the card's own surface (beside its title): nothing, and it stays
        wait(100);
        CHECK(popup() == "clock");
        click(calX + 352 - 26, 68); // the arrow to the next month
        wait(100);
        CHECK(text(ui->get_cal_title()) == "November 2026");
        click(cell(20).first, cell(20).second); // 15 November
        wait(100);
        CHECK(text(ui->get_cal_day_title()) == "Sunday, 15 November" && rowsOf(ui->get_cal_cells())[20].selected);
        CHECK(ui->get_cal_events()->row_count() == 2); // the 15th: a third and a fifth
        click(calX + 352 - 12 - 28 - 4 - 26, 68); // Today
        wait(100);
        CHECK(text(ui->get_cal_title()) == "October 2026" && text(ui->get_cal_day_title()) == "Wednesday, 28 October");
        click(calX + 352 - 12 - 28 - 4 - 52 - 4 - 14, 68); // the arrow back
        wait(100);
        CHECK(text(ui->get_cal_title()) == "September 2026");
        // Having wandered off, closing and opening again shows today.
        closeAll();
        CHECK(open("clock"));
        CHECK(text(ui->get_cal_title()) == "October 2026" && text(ui->get_cal_day_title()) == "Wednesday, 28 October");
    }
    closeAll();

    // ---- The Control Center: tiles, sliders, the player, the buttons below.
    CHECK(open("control"));
    {
        const float ccX = std::fmax(8.0f, std::fmin(cx("control") - 180, W - 360 - 8));
        const auto tile = [&](int i) { return std::pair<float, float>{ccX + 12 + (i % 2) * 172 + 60, 36 + 12 + (i / 2) * 68 + 30}; };
        CHECK(!ui->get_dark());
        click(tile(4).first, tile(4).second); // Dark Mode
        wait(100);
        CHECK(ui->get_dark() && app->model().dark() && rowsOf(ui->get_quick_tiles())[4].on);
        click(tile(4).first, tile(4).second);
        wait(100);
        CHECK(!ui->get_dark() && !app->model().dark());
        click(tile(2).first, tile(2).second); // Do Not Disturb
        wait(100);
        CHECK(ui->get_dnd() && app->model().dnd() && text(rowsOf(ui->get_quick_tiles())[2].subtitle) == "On");
        click(tile(2).first, tile(2).second);
        wait(100);
        CHECK(!ui->get_dnd());
        // Airplane mode: both radios go off and their tiles cannot be pressed.
        click(tile(3).first, tile(3).second);
        wait(100);
        CHECK(!ui->get_wifi_enabled() && !ui->get_bluetooth_enabled() && !rowsOf(ui->get_quick_tiles())[0].enabled);
        click(tile(0).first, tile(0).second); // Wi-Fi: disabled, so nothing
        wait(100);
        CHECK(!ui->get_wifi_enabled());
        click(tile(3).first, tile(3).second);
        wait(100);
        CHECK(rowsOf(ui->get_quick_tiles())[0].enabled);
        click(tile(0).first, tile(0).second);
        click(tile(1).first, tile(1).second);
        wait(100);
        CHECK(ui->get_wifi_enabled() && ui->get_bluetooth_enabled());
        // The chevron of the Wi-Fi tile goes to its popover.
        click(tile(0).first + 86, tile(0).second);
        wait(300);
        CHECK(popup() == "wifi");
        CHECK(open("control"));
        // The brightness slider: a click a quarter of the way along.
        click(ccX + 12 + 0.25f * 336, 36 + 12 + 196 + 10 + 24 + 19);
        wait(100);
        CHECK(std::fabs(ui->get_brightness() - 0.25f) < 0.12f && std::fabs(app->model().brightness() - ui->get_brightness()) < 0.001f);
        click(ccX + 12 + 0.9f * 336, 36 + 12 + 196 + 10 + 24 + 19);
        wait(100);
        CHECK(ui->get_brightness() > 0.75f);
        // The sound slider below it.
        click(ccX + 12 + 0.5f * 336, 36 + 12 + 196 + 10 + 24 + 38 + 2 + 24 + 19);
        wait(100);
        CHECK(std::fabs(ui->get_volume() - 0.5f) < 0.12f && std::fabs(app->model().volume() - ui->get_volume()) < 0.001f);
        // The player.
        ui->invoke_media_play_pause();
        CHECK(!ui->get_media().playing);
        ui->invoke_media_play_pause();
        ui->invoke_media_next();
        CHECK(text(ui->get_media().title) == "Open Water" && ui->get_media().progress == 0.0f);
        ui->invoke_media_previous();
        CHECK(text(ui->get_media().title) == "Golden Hour"); // back to the one before, at its start
        // The Lock button, at the bottom: asks, and closes.
        const float lockX = ccX + 12 + 1 * ((360 - 24 - 16) / 3.0f + 8) + 53;
        click(lockX, 36 + 12 + 196 + 10 + 24 + 38 + 2 + 24 + 38 + 10 + 86 + 28);
        wait(300);
        CHECK(popup() == "" && lastRequest() == "power:lock");
    }
    closeAll();

    // ---- The player keeps time while it plays, and not while it is paused.
    {
        const auto seconds = [](const std::string& t) { return std::atoi(t.c_str()) * 60 + std::atoi(t.c_str() + t.find(':') + 1); };
        const int start = seconds(text(ui->get_media().position));
        const float before0 = ui->get_media().progress;
        wait(3000);
        CHECK(seconds(text(ui->get_media().position)) == start + 3 && ui->get_media().progress > before0);
        ui->invoke_media_play_pause();
        wait(3000);
        CHECK(seconds(text(ui->get_media().position)) == start + 3); // paused: it does not move
        ui->invoke_media_play_pause();
    }

    // ---- The notification centre: opening it, what it holds; closing it reads them.
    CHECK(ui->get_unread() == 2);
    CHECK(open("bell"));
    {
        const auto list = rowsOf(ui->get_notices());
        CHECK(list.size() == 5 && text(list[0].app) == "Calendar" && list[0].unread && text(list[1].title) == "Aria Patel");
        CHECK(list[1].group_index == 0 && list[1].group_count == 3 && list[3].group_index == 2);
        CHECK(text(ui->get_date_text()) == "Wednesday 28 Oct");
        CHECK(ui->get_unread() == 2); // while it is open they still show as new
    }
    clickBar("bell"); // closing it before it has been looked at: they stay new
    wait(400);
    CHECK(popup() == "" && ui->get_unread() == 2);
    CHECK(open("bell"));
    wait(800);
    clickBar("bell"); // closing it after a look: they are read
    wait(400);
    CHECK(popup() == "" && ui->get_unread() == 0 && !rowsOf(ui->get_notices())[0].unread);
    // Pull a card sideways: it slides away, the rows close up, and then it is gone.
    CHECK(open("bell"));
    {
        const std::size_t count = app->model().count();
        const std::size_t swipeAsks = requests();
        drag(W - 408 + 150, 610, W - 408 + 340, 610); // the Calendar one, at the top
        wait(600);
        CHECK(popup() == "bell" && requests() == swipeAsks); // a pull is not a click
        CHECK(app->model().count() == count - 1 && ui->get_notices()->row_count() == count - 1);
        CHECK(text(rowsOf(ui->get_notices())[0].app) == "Mail");
        // A short pull only moves it, and it springs back.
        drag(W - 408 + 150, 610, W - 408 + 190, 610);
        wait(400);
        CHECK(popup() == "bell" && requests() == swipeAsks);
        CHECK(app->model().count() == count - 1);
    }
    // Pulling the first card of a group leaves the group's header on the next one, with the count one less.
    {
        const auto before = rowsOf(ui->get_notices());
        CHECK(text(before[0].app) == "Mail" && before[0].group_count == 3);
        drag(W - 408 + 150, 610, W - 408 + 340, 610);
        wait(600);
        const auto after = rowsOf(ui->get_notices());
        CHECK(after.size() == before.size() - 1 && text(after[0].app) == "Mail");
        CHECK(after[0].group_index == 0 && after[0].group_count == 2 && after[1].group_index == 1 && after[1].group_count == 2);
    }
    // "Clear all" empties the list.
    click(W - 68, 500);
    wait(500);
    CHECK(ui->get_notices()->row_count() == 0 && app->model().count() == 0);
    closeAll();

    // ---- A notification arrives: a banner comes up, waits, and goes by itself; the notice stays.
    {
        const int id = app->notify("Mail", "mail", "Aria Patel", "Are we still on for lunch?", "mail", {"Reply", "Archive"});
        CHECK(id > 0 && ui->get_toasts()->row_count() == 1 && ui->get_unread() == 1 && app->model().count() == 1);
        wait(500);
        CHECK(ui->get_toasts_height() > 80);
        wait(4000);
        CHECK(ui->get_toasts()->row_count() == 1); // its time is not up (5.2 s)
        wait(1200);
        wait(600);
        CHECK(ui->get_toasts()->row_count() == 0 && ui->get_toasts_height() == 0);
        CHECK(app->model().toastIds().empty() && app->model().count() == 1 && ui->get_notices()->row_count() == 1);
    }
    // The pointer on a banner holds it.
    {
        app->notify("Store", "store", "Files 2.4 is ready", "A faster list view.", "store", {"Update"});
        wait(500);
        moveTo(W - 380 + 184, 40 + 40);
        wait(200);
        wait(9000);
        CHECK(ui->get_toasts()->row_count() == 1);
        moveTo(700, 500);
        wait(5000);
        CHECK(ui->get_toasts()->row_count() == 1);
        wait(1200);
        CHECK(ui->get_toasts()->row_count() == 0);
    }
    // A button on a banner: it does that, the banner leaves, and the notice is answered.
    {
        const std::size_t count = app->model().count();
        app->notify("Mail", "mail", "Sam", "Photos from the weekend", "mail", {"Reply", "Archive"});
        wait(600);
        CHECK(ui->get_toasts()->row_count() == 1 && app->model().count() == count + 1);
        click(W - 380 + 56, 40 + (ui->get_toasts_height() - 10) - 28); // "Reply"
        wait(700);
        CHECK(lastRequest() == "notice:action:Mail:Reply");
        CHECK(ui->get_toasts()->row_count() == 0 && app->model().count() == count);
    }
    // A click on the banner opens its app, and the banner and the notice both go.
    {
        const std::size_t count = app->model().count();
        app->notify("Music", "music", "New release", "Lumen has a new single out today.", "music");
        wait(600);
        click(W - 380 + 150, 40 + 40);
        wait(700);
        CHECK(lastRequest() == "notice:open:Music");
        CHECK(ui->get_toasts()->row_count() == 0 && app->model().count() == count);
    }
    // Do Not Disturb: kept, but quiet. The switch is in the centre.
    CHECK(open("bell"));
    click(W - 80, 69);
    wait(200);
    CHECK(ui->get_dnd() && app->model().dnd());
    closeAll();
    {
        const std::size_t count = app->model().count();
        app->notify("Mail", "mail", "Quiet", "Not a sound", "mail");
        wait(600);
        CHECK(ui->get_toasts()->row_count() == 0 && app->model().count() == count + 1);
    }
    CHECK(open("bell"));
    click(W - 80, 69);
    wait(200);
    CHECK(!ui->get_dnd());
    closeAll();

    // ---- The volume display: it comes up for the keys, stays while they are used, goes by itself.
    {
        CHECK(!ui->get_osd_showing());
        const float v = ui->get_volume();
        app->volumeKey(0.1f);
        wait(50);
        CHECK(ui->get_osd_showing() && text(ui->get_osd_kind()) == "volume" && std::fabs(ui->get_osd_value() - (v + 0.1f)) < 0.001f);
        CHECK(std::fabs(ui->get_volume() - (v + 0.1f)) < 0.001f);
        wait(1000);
        app->volumeKey(0.1f); // used again: it stays
        wait(1000);
        CHECK(ui->get_osd_showing());
        wait(700);
        CHECK(!ui->get_osd_showing());
        app->brightnessKey(-0.2f);
        wait(50);
        CHECK(ui->get_osd_showing() && text(ui->get_osd_kind()) == "brightness");
        wait(2000);
        app->muteKey();
        wait(50);
        CHECK(ui->get_muted() && text(ui->get_osd_kind()) == "mute" && ui->get_osd_value() == 0.0f && ui->get_osd_showing());
        app->muteKey();
        CHECK(!ui->get_muted());
        wait(2000);
        CHECK(!ui->get_osd_showing());
    }

    // ---- Looks follow the window's properties.
    ui->set_dark(true);
    wait(50);
    CHECK(ui->get_dark());
    ui->set_dark(false);

    if (failures == 0)
        std::puts("app_test: all checks passed");
    return failures ? 1 : 0;
}
