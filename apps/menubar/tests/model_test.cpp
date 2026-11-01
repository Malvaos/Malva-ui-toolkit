// The bar's model on its own (no Slint): the clock and calendar, Wi-Fi, Bluetooth, the battery, sound and
// light, the Control Center's tiles, notifications, the player, and the menus with what choosing a row does.
#include "bar_model.h"

#include <cstdio>
#include <string>

using namespace barui;

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond);          \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

static int rowOf(const std::vector<Row>& rows, const std::string& text)
{
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (rows[i].text == text)
            return int(i);
    }
    return -1;
}

static std::string groupOrder(const Model& m)
{
    std::string out;
    for (const Notice& n : m.notices())
        out += (out.empty() ? "" : ",") + n.group + ":" + std::to_string(n.groupIndex) + "/" + std::to_string(n.groupCount);
    return out;
}

int main()
{
    // ---- the clock
    {
        Model m = Model::sample();
        CHECK(m.clockText() == "Wed 28 Oct  5:01 AM");
        CHECK(m.dateText() == "Wednesday 28 Oct");
        m.advance(20 * 60); // past midnight into the next day
        CHECK(m.clockText() == "Thu 29 Oct  1:01 AM");
        m.setNow({2026, 12, 31, 23, 59});
        m.advance(2);
        CHECK(m.now().year == 2027 && m.now().month == 1 && m.now().day == 1 && m.now().hour == 0 && m.now().minute == 1);
        m.setNow({2028, 2, 28, 12, 0});
        m.advance(24 * 60);
        CHECK(m.now().month == 2 && m.now().day == 29); // 2028 is a leap year
        m.advance(24 * 60);
        CHECK(m.now().month == 3 && m.now().day == 1);
        m.setNow({2026, 10, 28, 15, 5});
        CHECK(m.clockText() == "Wed 28 Oct  3:05 PM");
        m.setNow({2026, 10, 28, 0, 7});
        CHECK(m.clockText() == "Wed 28 Oct  12:07 AM");
        m.setTime(12, 30);
        CHECK(m.clockText() == "Wed 28 Oct  12:30 PM");
    }

    // ---- the calendar: 42 cells, Monday first
    {
        Model m = Model::sample();
        auto cells = m.cells();
        CHECK(cells.size() == 42);
        CHECK(m.monthTitle() == "October 2026");
        // 1 October 2026 is a Thursday: three days of September come first.
        CHECK(!cells[0].inMonth && cells[0].day == 28);
        CHECK(!cells[2].inMonth && cells[2].day == 30);
        CHECK(cells[3].inMonth && cells[3].day == 1);
        CHECK(cells[33].inMonth && cells[33].day == 31);
        CHECK(!cells[34].inMonth && cells[34].day == 1);
        CHECK(cells[30].today && cells[30].selected && cells[30].day == 28);
        int todays = 0;
        for (const Cell& c : cells)
            todays += c.today;
        CHECK(todays == 1);
        CHECK(m.selectedTitle() == "Wednesday, 28 October");
        CHECK(m.events().size() == 4);
        CHECK(cells[30].events == int(m.events().size()));

        m.nextMonth();
        cells = m.cells();
        CHECK(m.monthTitle() == "November 2026");
        // 1 November 2026 is a Sunday: six days of October first.
        CHECK(!cells[0].inMonth && cells[0].day == 26);
        CHECK(cells[6].inMonth && cells[6].day == 1);
        CHECK(!cells[30].today); // today is not in this month
        m.nextMonth();
        m.nextMonth();
        CHECK(m.monthTitle() == "January 2027");
        m.previousMonth();
        m.previousMonth();
        m.previousMonth();
        m.previousMonth();
        CHECK(m.monthTitle() == "September 2026");
        m.goToday();
        CHECK(m.monthTitle() == "October 2026" && m.selectedDay() == 28);

        // A day of another month shows that month, with the day picked.
        m.pickCell(0);
        CHECK(m.monthTitle() == "September 2026" && m.selectedDay() == 28);
        CHECK(m.selectedTitle() == "Monday, 28 September");
        m.goToday();
        m.pickCell(34); // the 1st of November, seen from October
        CHECK(m.monthTitle() == "November 2026" && m.selectedDay() == 1);
        CHECK(m.selectedTitle() == "Sunday, 1 November");
        m.pickCell(-1);
        m.pickCell(42);
        CHECK(m.selectedDay() == 1);
        m.goToday();
        m.pickCell(10); // 8 October
        CHECK(m.monthTitle() == "October 2026" && m.selectedDay() == 8);
        CHECK(m.events().empty());
    }

    // ---- Wi-Fi
    {
        Model m = Model::sample();
        CHECK(m.wifiEnabled() && m.wifiConnected() && m.wifiName() == "Mangrove 5G" && m.wifiStrength() == 3);
        // An open network connects; the first lets go when it does.
        CHECK(m.clickNetwork(1));
        CHECK(m.networks()[1].connecting && m.networks()[0].connected);
        m.finishConnect(1);
        CHECK(m.networks()[1].connected && !m.networks()[1].connecting && !m.networks()[0].connected);
        CHECK(m.networks()[1].saved);
        CHECK(m.wifiName() == "Cafe Guest" && m.wifiStrength() == 2);
        // A secured network we have not joined needs a password, which is not here: nothing changes.
        CHECK(!m.clickNetwork(3));
        CHECK(!m.networks()[3].connecting && !m.networks()[3].connected);
        // A saved one does not.
        CHECK(m.clickNetwork(2));
        CHECK(m.networks()[2].connecting);
        m.finishConnect(2);
        CHECK(m.wifiName() == "Neighbour_2.4");
        // Clicking the connected one disconnects it.
        CHECK(m.clickNetwork(2));
        CHECK(!m.wifiConnected() && m.wifiName().empty() && m.wifiStrength() == 0);
        // Finishing what was never started does nothing.
        m.finishConnect(0);
        CHECK(!m.wifiConnected());
        CHECK(!m.clickNetwork(-1) && !m.clickNetwork(99));
        // Off: nothing connects, and what was connecting stops.
        CHECK(m.clickNetwork(0));
        m.setWifi(false);
        CHECK(!m.networks()[0].connecting && !m.wifiConnected() && !m.clickNetwork(0));
        m.finishConnect(0);
        CHECK(!m.wifiConnected());
        m.setWifi(true);
        CHECK(m.clickNetwork(0));
    }

    // ---- Bluetooth
    {
        Model m = Model::sample();
        CHECK(m.bluetoothEnabled() && m.devicesConnected() == 2);
        CHECK(m.devices()[2].battery == -1 && !m.devices()[2].connected);
        m.clickDevice(2);
        CHECK(m.devices()[2].connecting && !m.devices()[2].connected);
        m.clickDevice(2); // while it connects: nothing
        CHECK(m.devices()[2].connecting);
        m.finishDevice(2);
        CHECK(m.devices()[2].connected && !m.devices()[2].connecting && m.devices()[2].battery >= 0);
        CHECK(m.devicesConnected() == 3);
        m.clickDevice(2); // a connected one lets go, and keeps what it said about its battery
        CHECK(!m.devices()[2].connected && m.devices()[2].battery >= 0);
        m.setBluetooth(false);
        CHECK(m.devicesConnected() == 0);
        m.clickDevice(0);
        CHECK(!m.devices()[0].connecting);
        m.finishDevice(0);
        CHECK(m.devicesConnected() == 0);
        m.setBluetooth(true);
        CHECK(m.devicesConnected() == 0); // they do not come back on their own
    }

    // ---- airplane mode and the tiles
    {
        Model m = Model::sample();
        auto tiles = m.tiles();
        CHECK(tiles.size() == 6);
        CHECK(tiles[0].id == "wifi" && tiles[0].on && tiles[0].detail && tiles[0].subtitle == "Mangrove 5G");
        CHECK(tiles[1].id == "bluetooth" && tiles[1].subtitle == "2 devices");
        CHECK(tiles[2].id == "dnd" && !tiles[2].on && !tiles[2].detail);
        m.toggleTile("dnd");
        CHECK(m.dnd() && m.tiles()[2].subtitle == "On");
        m.toggleTile("dark");
        CHECK(m.dark() && m.tiles()[4].on);
        m.toggleTile("night");
        CHECK(!m.nightLight() && m.tiles()[5].subtitle == "Off");
        m.toggleTile("nothing"); // unknown: ignored
        m.toggleTile("airplane");
        CHECK(m.airplane() && !m.wifiEnabled() && !m.bluetoothEnabled());
        tiles = m.tiles();
        CHECK(!tiles[0].enabled && !tiles[1].enabled && tiles[0].subtitle == "Off" && tiles[1].subtitle == "Off");
        CHECK(tiles[3].on && tiles[3].enabled);
        m.toggleTile("airplane");
        CHECK(!m.airplane());
        m.toggleTile("wifi");
        CHECK(m.wifiEnabled() && !m.airplane());
        // Turning a radio on while in airplane mode ends it.
        m.toggleTile("airplane");
        m.toggleTile("bluetooth");
        // (the tile is disabled for the window, but the model itself is allowed)
        CHECK(m.bluetoothEnabled() && !m.airplane());
    }

    // ---- the battery
    {
        Model m = Model::sample();
        CHECK(m.battery() == 82 && m.charging());
        CHECK(m.batteryStatus() == "Charging" && m.batteryDetail() == "36 min until full" && m.batterySource() == "Power adapter");
        m.setBattery(12, false);
        CHECK(m.batteryStatus() == "Low battery" && m.batteryDetail() == "36 min remaining" && m.batterySource() == "Battery");
        m.setBattery(80, false);
        CHECK(m.batteryStatus() == "On battery" && m.batteryDetail() == "4 h 0 min remaining");
        m.setBattery(99, true);
        m.advance(10);
        CHECK(m.battery() == 100 && m.batteryStatus() == "Fully charged" && m.batteryDetail() == "Charged");
        m.setBattery(5, false);
        m.advance(60);
        CHECK(m.battery() == 0);
        m.setBattery(250, true);
        CHECK(m.battery() == 100);
        m.setBattery(-4, true);
        CHECK(m.battery() == 0);
        CHECK(m.powerMode() == 1);
        m.setPowerMode(7);
        CHECK(m.powerMode() == 2);
        m.setPowerMode(-1);
        CHECK(m.powerMode() == 0);
    }

    // ---- sound and light
    {
        Model m = Model::sample();
        m.setVolume(1.7f);
        CHECK(m.volume() == 1.0f);
        m.setVolume(-1);
        CHECK(m.volume() == 0.0f);
        m.setMuted(true);
        m.setVolume(0.3f);
        CHECK(!m.muted() && m.volume() > 0.29f && m.volume() < 0.31f); // moving the slider unmutes
        m.setBrightness(0);
        CHECK(m.brightness() > 0); // never fully dark
        m.setBrightness(2);
        CHECK(m.brightness() == 1.0f);
        CHECK(m.outputs().size() == 3 && m.outputs()[0].selected);
        m.pickOutput(1);
        CHECK(!m.outputs()[0].selected && m.outputs()[1].selected && !m.outputs()[2].selected);
        m.pickOutput(9);
        CHECK(m.outputs()[1].selected);
    }

    // ---- notifications
    {
        Model m = Model::sample();
        CHECK(m.count() == 5);
        CHECK(m.unread() == 2);
        CHECK(m.toastIds().empty());
        // Grouped by app, the group with the newest notice first, newest first inside.
        CHECK(groupOrder(m) == "calendar:0/1,mail:0/3,mail:1/3,mail:2/3,store:0/1");
        auto notices = m.notices();
        CHECK(notices[1].title == "Aria Patel" && notices[2].title == "Design weekly" && notices[3].title == "Invoice 1042");
        CHECK(notices[0].time == "now" && notices[1].time == "5 m" && notices[2].time == "1 h" && notices[3].time == "3 h" && notices[4].time == "Yesterday");
        CHECK(notices[0].urgent && notices[0].actions.size() == 2 && notices[1].actions[0] == "Reply");

        // A new one comes on top of its group, is unread, and is a banner.
        const int id = m.post("Mail", "mail", "New thing", "Hello", "mail");
        CHECK(m.count() == 6 && m.unread() == 3);
        CHECK(m.toastIds().size() == 1 && m.toastIds()[0] == id);
        CHECK(m.toast(id).title == "New thing" && m.toast(id).groupCount == 1);
        CHECK(m.notices()[0].group == "mail" && m.notices()[0].title == "New thing");
        CHECK(m.toast(9999).title.empty());

        // At most three banners; the oldest goes first.
        const int b = m.post("Store", "store", "b", "", "store");
        const int c = m.post("Store", "store", "c", "", "store");
        const int d = m.post("Store", "store", "d", "", "store");
        CHECK(m.toastIds().size() == 3 && m.toastIds()[0] == b && m.toastIds()[1] == c && m.toastIds()[2] == d);
        m.expireToast(c);
        CHECK(m.toastIds().size() == 2 && m.count() == 9); // the banner going does not remove the notice
        m.dismiss(b);
        CHECK(m.toastIds().size() == 1 && m.count() == 8);
        m.dismiss(12345); // unknown: nothing
        CHECK(m.count() == 8);

        // Do Not Disturb: it is kept, but no banner.
        m.setDnd(true);
        const size_t before = m.toastIds().size();
        m.post("Mail", "mail", "Quiet one", "", "mail");
        CHECK(m.toastIds().size() == before && m.unread() > 0);
        m.setDnd(false);

        // Reading, clearing.
        m.markAllRead();
        CHECK(m.unread() == 0);
        m.clearGroup("mail");
        for (const Notice& n : m.notices())
            CHECK(n.group != "mail");
        const int left = m.count();
        CHECK(left > 0);
        m.clearGroup("nothing");
        CHECK(m.count() == left);
        m.clearAll();
        CHECK(m.count() == 0 && m.toastIds().empty() && m.notices().empty());
    }

    // ---- the player
    {
        Model m = Model::sample();
        CHECK(m.media().has && m.media().playing && m.media().title == "Golden Hour" && m.media().length == "3:53");
        CHECK(m.media().position == "1:34" && m.media().progress > 0.40f && m.media().progress < 0.41f);
        m.tickMedia(6);
        CHECK(m.media().position == "1:40");
        m.playPause();
        CHECK(!m.media().playing);
        m.tickMedia(30); // paused: it does not move
        CHECK(m.media().position == "1:40");
        m.playPause();
        m.nextTrack();
        CHECK(m.media().title == "Open Water" && m.media().position == "0:00" && m.media().progress == 0.0f && m.media().length == "3:18");
        m.nextTrack();
        CHECK(m.media().title == "Late Bloom");
        m.nextTrack();
        CHECK(m.media().title == "Golden Hour"); // round again
        // Previous: past the first seconds, back to the start of this one; at the start, the one before.
        m.tickMedia(20);
        m.previousTrack();
        CHECK(m.media().title == "Golden Hour" && m.media().position == "0:00");
        m.previousTrack();
        CHECK(m.media().title == "Late Bloom");
        // A track that ends goes on to the next.
        m.tickMedia(271);
        CHECK(m.media().title == "Golden Hour" && m.media().position == "0:00");
    }

    // ---- menus
    {
        Model m = Model::sample();
        CHECK(m.menus().size() == 7 && m.menus()[0] == "Files" && m.menus()[1] == "File");
        // Every row that says it is in a flyout names a row before it that opens one.
        for (const std::string& name : {"system", "menu-0", "menu-1", "menu-2", "menu-3", "menu-4", "menu-5", "menu-6", "tray-0", "tray-1", "layout"}) {
            const auto rows = m.menuFor(name);
            CHECK(!rows.empty());
            for (std::size_t i = 0; i < rows.size(); ++i) {
                if (rows[i].parent >= 0) {
                    CHECK(rows[i].parent < int(i) && rows[rows[i].parent].submenu);
                    CHECK(!rows[i].submenu);
                }
                if (rows[i].kind == "item")
                    CHECK(!rows[i].text.empty() && (rows[i].submenu || !rows[i].action.empty()));
                CHECK(rows[i].kind == "item" || rows[i].kind == "header" || rows[i].kind == "separator");
            }
        }
        CHECK(m.menuFor("nothing").empty());

        // The system menu: a flyout of recent items, and the power rows.
        auto sys = m.menuFor("system");
        const int recent = rowOf(sys, "Recent Items");
        CHECK(recent >= 0 && sys[recent].submenu);
        int children = 0;
        for (const Row& r : sys)
            children += r.parent == recent;
        CHECK(children == 4);
        CHECK(m.perform("system", rowOf(sys, "Shut Down…")) == "power:shutdown");
        CHECK(m.perform("system", rowOf(sys, "Restart…")) == "power:restart");
        CHECK(m.perform("system", rowOf(sys, "Log Out Aria…")) == "power:logout");
        CHECK(m.perform("system", rowOf(sys, "Lock Screen")) == "power:lock");
        CHECK(m.perform("system", recent).empty()); // a row that opens a flyout chooses nothing itself

        // Rows that are not choices do nothing.
        auto tray = m.menuFor("tray-1");
        CHECK(tray[0].kind == "header" && m.perform("tray-1", 0).empty());
        CHECK(m.perform("system", 1).empty()); // a separator
        CHECK(m.perform("system", -1).empty() && m.perform("system", 99).empty());

        // The File menu follows what is selected.
        auto file = m.menuFor("menu-1");
        CHECK(!file[rowOf(file, "Get Info")].enabled && !file[rowOf(file, "Move to Bin")].enabled);
        CHECK(file[rowOf(file, "Move to Bin")].destructive);
        CHECK(m.perform("menu-1", rowOf(file, "Get Info")).empty()); // disabled: nothing
        m.selection = 3;
        file = m.menuFor("menu-1");
        CHECK(file[rowOf(file, "Get Info")].enabled && file[rowOf(file, "Move to Bin")].enabled);
        CHECK(m.perform("menu-1", rowOf(file, "Get Info")) == "file:info");
        // New Window and Close Window change the count of windows, which the Window menu lists.
        CHECK(m.windows == 1);
        CHECK(m.perform("menu-1", rowOf(file, "New Window")) == "file:new-window" && m.windows == 2);
        auto window = m.menuFor("menu-5");
        CHECK(rowOf(window, "Pictures") >= 0 && rowOf(window, "Pictures 2") >= 0);
        CHECK(m.perform("menu-1", rowOf(m.menuFor("menu-1"), "Close Window")) == "file:close" && m.windows == 1);
        m.perform("menu-1", rowOf(m.menuFor("menu-1"), "Close Window"));
        CHECK(m.windows == 0);
        CHECK(!m.menuFor("menu-1")[rowOf(m.menuFor("menu-1"), "Close Window")].enabled);

        // The Edit menu: undo and redo trade places.
        auto edit = m.menuFor("menu-2");
        CHECK(edit[0].text == "Undo" && edit[0].enabled && !edit[1].enabled);
        CHECK(m.perform("menu-2", 0) == "edit:undo");
        edit = m.menuFor("menu-2");
        CHECK(!edit[0].enabled && edit[1].enabled);
        CHECK(m.perform("menu-2", 0).empty());
        CHECK(m.perform("menu-2", 1) == "edit:redo");
        CHECK(m.menuFor("menu-2")[0].enabled);

        // The View menu keeps its ticks.
        auto view = m.menuFor("menu-3");
        CHECK(view[rowOf(view, "as Icons")].checked && !view[rowOf(view, "as List")].checked);
        m.perform("menu-3", rowOf(view, "as List"));
        view = m.menuFor("menu-3");
        CHECK(m.viewMode == 1 && view[rowOf(view, "as List")].checked && !view[rowOf(view, "as Icons")].checked);
        m.perform("menu-3", rowOf(view, "Date Modified"));
        view = m.menuFor("menu-3");
        CHECK(m.sortBy == 1 && view[rowOf(view, "Date Modified")].checked && !view[rowOf(view, "Name")].checked);
        CHECK(!view[rowOf(view, "Show Hidden Files")].checked);
        m.perform("menu-3", rowOf(view, "Show Hidden Files"));
        CHECK(m.showHidden && m.menuFor("menu-3")[rowOf(view, "Show Hidden Files")].checked);

        // The layout menu: choosing one changes the bar's short name.
        CHECK(m.layoutText() == "EN");
        auto layout = m.menuFor("layout");
        CHECK(layout[rowOf(layout, "English (US)")].checked);
        CHECK(m.perform("layout", rowOf(layout, "हिन्दी (Hindi)")) == "layout:2");
        CHECK(m.layout() == 2 && m.layoutText() == "HI");
        layout = m.menuFor("layout");
        CHECK(layout[rowOf(layout, "हिन्दी (Hindi)")].checked && !layout[rowOf(layout, "English (US)")].checked);
        CHECK(m.perform("layout", rowOf(layout, "Keyboard Settings…")) == "layout:settings");
        CHECK(m.layout() == 2); // not a layout
        m.setLayout(-3);
        CHECK(m.layout() == 0);
        m.setLayout(40);
        CHECK(m.layout() == 3 && m.layoutText() == "DE");
    }

    if (failures) {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    std::puts("model_test: all checks passed");
    return 0;
}
