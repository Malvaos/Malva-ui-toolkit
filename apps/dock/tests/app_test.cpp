// The window and the stand-in backend together, driven the way a person drives them: a pointer that
// moves, hovers, clicks with either button and drags, through the window's own event handling (no
// window appears; time is stepped by hand, so animations and timers run when the test says).
// It needs a Slint build that includes the testing backend (the SDK's does).
//
// What this can see is what the window reports: the rows it shows, the menu, the dock's length
// (which grows as the icons magnify), whether the pointer is inside. What it cannot see is what is
// drawn: that is for the screenshots and recordings.
#include "../backend/app.h"

#include <private/slint_tests_helpers.h>

#include <cmath>
#include <cstdio>
#include <string>

using namespace dockui;
namespace st = slint::private_api::testing;

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond);          \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

static DockWindow* ui = nullptr;

static std::string text(const slint::SharedString& s) { return std::string(std::string_view(s)); }
static std::vector<DockApp> rows()
{
    std::vector<DockApp> out;
    auto model = ui->get_apps();
    for (std::size_t i = 0; i < model->row_count(); ++i)
        out.push_back(*model->row_data(i));
    return out;
}
static std::string ids()
{
    std::string out;
    for (const DockApp& r : rows())
        out += (out.empty() ? "" : ",") + text(r.id);
    return out;
}
static int indexOf(const std::string& id)
{
    int i = 0;
    for (const DockApp& r : rows()) {
        if (text(r.id) == id)
            return i;
        ++i;
    }
    return -1;
}
static DockApp row(const std::string& id)
{
    const int i = indexOf(id);
    return i >= 0 ? rows()[i] : DockApp {};
}
static std::vector<MenuEntry> menu()
{
    std::vector<MenuEntry> out;
    auto model = ui->get_menu_entries();
    for (std::size_t i = 0; i < model->row_count(); ++i)
        out.push_back(*model->row_data(i));
    return out;
}
static int menuRow(const std::string& label)
{
    int i = 0;
    for (const MenuEntry& e : menu()) {
        if (text(e.text) == label)
            return i;
        ++i;
    }
    return -1;
}

// ---- the pointer
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
static void press(slint::PointerEventButton b = slint::PointerEventButton::Left)
{
    ui->window().dispatch_pointer_press_event(slint::LogicalPosition({px, py}), b);
}
static void release(slint::PointerEventButton b = slint::PointerEventButton::Left)
{
    ui->window().dispatch_pointer_release_event(slint::LogicalPosition({px, py}), b);
}
static void click(float x, float y, slint::PointerEventButton b = slint::PointerEventButton::Left)
{
    moveTo(x, y);
    wait(20);
    press(b);
    wait(20);
    release(b);
    wait(20);
}
// To the desk, away from the dock, and let everything settle.
static void leave()
{
    moveTo(60, 60);
    wait(700);
}
static float iconY() { return ui->invoke_icon_y(); }
static void clickApp(int i, slint::PointerEventButton b = slint::PointerEventButton::Left)
{
    leave();
    click(ui->invoke_app_x(i), iconY(), b);
}
static void rightClickApp(int i) { clickApp(i, slint::PointerEventButton::Right); }
// Takes hold of an icon, carries it by (dx, dy) in small steps, and lets go.
static void drag(int app, float dx, float dy)
{
    leave();
    moveTo(ui->invoke_app_x(app), iconY());
    wait(400); // the dock grows under the pointer
    const float x0 = px, y0 = py;
    press();
    wait(20);
    const int steps = 24;
    for (int k = 1; k <= steps; ++k) {
        moveTo(x0 + dx * k / steps, y0 + dy * k / steps);
        wait(16);
    }
    wait(100);
    release();
    wait(60);
}

int main()
{
    slint::cbindgen_private::slint_testing_init_backend();

    Options options;
    options.animateEntrance = false;
    options.launchMs = 1000;
    App app(options);
    auto handle = app.window();
    ui = &*handle;
    ui->window().set_size(slint::LogicalSize({1100, 440}));
    ui->show();

    // ---- What the dock shows.
    CHECK(ids() == "files,browser,mail,music,photos,calendar,notes,terminal,settings");
    CHECK(ui->get_minimized_windows()->row_count() == 1);
    CHECK(ui->get_trash_full());
    CHECK(!ui->get_menu_open() && !ui->get_pointer_inside() && !ui->get_hidden());
    const float restLength = ui->get_dock_length();
    CHECK(restLength > 500 && ui->get_footprint() > 60);
    // The icons are laid out left to right, one unit (icon and gap) apart; the dock is centred.
    const float unit = ui->invoke_app_x(1) - ui->invoke_app_x(0);
    CHECK(unit > 56 && std::abs((ui->invoke_app_x(5) - ui->invoke_app_x(4)) - unit) < 0.01f);
    CHECK(std::abs((ui->invoke_app_x(0) + ui->invoke_app_x(8)) / 2 - 550) < 80);

    // ---- Hover: the pointer over the dock magnifies the icons under it, and the dock grows.
    moveTo(ui->invoke_app_x(3), iconY());
    wait(50);
    CHECK(ui->get_pointer_inside());
    wait(400);
    const float grown = ui->get_dock_length();
    CHECK(grown > restLength + 30); // the icon under the pointer is 1.65x, its neighbours part-way
    // Moving along the dock keeps it (a gap between two icons is not a leave) ...
    for (float x = ui->invoke_app_x(3); x < ui->invoke_app_x(6); x += 4) {
        moveTo(x, iconY());
        wait(8);
    }
    wait(400);
    CHECK(ui->get_pointer_inside() && std::abs(ui->get_dock_length() - grown) < 12);
    // ... and leaving lets it settle back.
    moveTo(60, 60);
    wait(30);
    CHECK(ui->get_pointer_inside()); // not at once: the dock waits a moment, so a sweep never flickers
    wait(700);
    CHECK(!ui->get_pointer_inside());
    CHECK(std::abs(ui->get_dock_length() - restLength) < 0.5f);

    // ---- A click on an app that is not running starts it: it hops, then it runs and is in front.
    CHECK(!row("music").running);
    clickApp(3);
    CHECK(row("music").launching && !row("music").running);
    CHECK(row("files").active);
    wait(500);
    CHECK(row("music").launching);
    wait(600);
    CHECK(!row("music").launching && row("music").running && row("music").active);
    CHECK(!row("files").active);

    // ---- A click on one that runs brings it forward, and starts nothing.
    clickApp(1);
    CHECK(row("browser").active && !row("music").active && !row("browser").launching);

    // ---- A right click opens the menu of the icon under the pointer.
    rightClickApp(0);
    CHECK(ui->get_menu_open());
    CHECK(text(menu().front().kind) == "header" && text(menu().front().text) == "FILES");
    CHECK(menuRow("New Window") == 1 && menuRow("Quit") > 0);
    CHECK(menu()[menuRow("Keep in Dock")].checked);
    CHECK(menu()[menuRow("Quit")].destructive);
    // The row is chosen through the window, as a click would.
    const int windowsBefore = row("files").windows;
    ui->invoke_menu_chosen(menuRow("New Window"));
    CHECK(!ui->get_menu_open());
    CHECK(row("files").windows == windowsBefore + 1 && row("files").active);

    // A header chosen does nothing, and the menu stays.
    rightClickApp(7);
    CHECK(ui->get_menu_open() && text(menu().front().text) == "TERMINAL");
    ui->invoke_menu_chosen(0);
    CHECK(ui->get_menu_open());
    // A click on the desk closes it without choosing anything.
    click(80, 60);
    CHECK(!ui->get_menu_open());
    CHECK(row("terminal").running);

    // A click on an icon closes it too, and does what the click does.
    rightClickApp(7);
    CHECK(ui->get_menu_open());
    leave();
    click(ui->invoke_app_x(2), iconY());
    CHECK(!ui->get_menu_open());
    CHECK(row("mail").active);

    // Quit: the app stops running; being pinned it stays; its count is gone.
    rightClickApp(indexOf("mail"));
    ui->invoke_menu_chosen(menuRow("Quit"));
    CHECK(!row("mail").running && !row("mail").active && row("mail").badge == 0);
    CHECK(indexOf("mail") >= 0);
    // An app that does not run has no Quit and nothing to hide.
    rightClickApp(indexOf("mail"));
    CHECK(menuRow("Quit") == -1 && menuRow("Hide") == -1 && menuRow("New Window") > 0);
    ui->invoke_menu_dismissed();
    CHECK(!ui->get_menu_open());

    // ---- Hide puts a window in the tray; clicking it there brings it back.
    rightClickApp(indexOf("browser"));
    const int browserWindows = row("browser").windows;
    ui->invoke_menu_chosen(menuRow("Hide"));
    CHECK(ui->get_minimized_windows()->row_count() == 2);
    CHECK(row("browser").windows == browserWindows - 1);
    leave();
    click(ui->invoke_window_x(1), iconY());
    CHECK(ui->get_minimized_windows()->row_count() == 1);
    CHECK(row("browser").windows == browserWindows && row("browser").active);

    // ---- Dragging an icon to a new place: the icons make way and the model follows.
    CHECK(ids() == "files,browser,mail,music,photos,calendar,notes,terminal,settings");
    drag(0, 2 * unit, 0);
    CHECK(ids() == "browser,mail,files,music,photos,calendar,notes,terminal,settings");
    drag(8, -8 * unit, 0); // all the way back
    CHECK(ids() == "settings,browser,mail,files,music,photos,calendar,notes,terminal");
    drag(3, 0.3f * unit, 0); // not far enough to matter
    CHECK(ids() == "settings,browser,mail,files,music,photos,calendar,notes,terminal");
    CHECK(!ui->get_menu_open()); // and a drag is not a click
    CHECK(row("files").active || row("browser").active);
    const bool browserWasActive = row("browser").active;
    drag(1, 0.2f * unit, 0);
    CHECK(row("browser").active == browserWasActive); // let go where it was picked up: no click either

    // ---- Dragging one off the dock: it shrinks away, then its place is gone. One that runs stays.
    const std::size_t count = rows().size();
    drag(indexOf("photos"), 0, -170);
    CHECK(row("photos").removing && rows().size() == count);
    wait(200);
    CHECK(rows().size() == count);
    wait(300);
    CHECK(rows().size() == count - 1 && indexOf("photos") == -1);
    drag(indexOf("terminal"), 0, -170);
    wait(600);
    CHECK(indexOf("terminal") >= 0 && !row("terminal").removing && !row("terminal").pinned);

    // ---- The bin: its menu, and emptying it.
    CHECK(ui->get_trash_full());
    leave();
    click(ui->invoke_trash_x(), iconY(), slint::PointerEventButton::Right);
    CHECK(ui->get_menu_open() && text(menu().front().text) == "TRASH" && menuRow("Empty Trash") > 0);
    ui->invoke_menu_chosen(menuRow("Empty Trash"));
    CHECK(!ui->get_trash_full() && !ui->get_menu_open());
    leave();
    click(ui->invoke_trash_x(), iconY(), slint::PointerEventButton::Right);
    CHECK(!menu()[menuRow("Empty Trash")].enabled);
    ui->invoke_menu_chosen(menuRow("Empty Trash"));
    CHECK(ui->get_menu_open()); // nothing to empty: the disabled row does nothing
    ui->invoke_menu_dismissed();

    // ---- The launcher and the bin answer a click without harm.
    leave();
    click(ui->invoke_launcher_x(), iconY());
    leave();
    click(ui->invoke_trash_x(), iconY());
    CHECK(!ui->get_menu_open());

    // ---- Looks follow the window's properties; the dock follows the icon size.
    ui->set_dark(true);
    ui->set_icon_size(40);
    wait(50);
    CHECK(ui->get_icon_size() == 40);
    CHECK(ui->get_dock_length() < restLength * 0.85f);

    if (failures == 0)
        std::puts("app_test: all checks passed");
    return failures ? 1 : 0;
}
