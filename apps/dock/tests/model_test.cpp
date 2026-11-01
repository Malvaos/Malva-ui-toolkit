// The dock's model on its own: launching, focus, menus and what they do, reordering, removal.
#include "dock_model.h"

#include <cstdio>

using namespace dockui;

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond);          \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

static std::string order(const Model& m)
{
    std::string out;
    for (const AppState& a : m.apps())
        out += (out.empty() ? "" : ",") + a.id;
    return out;
}

int main()
{
    Model m = Model::sample();
    CHECK(m.apps().size() == 9);
    CHECK(m.find("music") == 3);
    CHECK(m.find("nothing") == -1);
    CHECK(m.apps()[0].active && !m.apps()[1].active);

    // A running app comes to the front; the one that was there is not any more.
    CHECK(m.activate(1));
    CHECK(m.apps()[1].active && !m.apps()[0].active);

    // One that is not running starts; a second click while it starts does nothing.
    const int music = m.find("music");
    CHECK(!m.apps()[music].running);
    CHECK(m.activate(music));
    CHECK(m.apps()[music].launching && !m.apps()[music].running);
    CHECK(!m.activate(music));
    m.finishLaunch(music);
    CHECK(!m.apps()[music].launching && m.apps()[music].running && m.apps()[music].active);
    CHECK(m.apps()[music].windows == 1);
    CHECK(!m.apps()[1].active);
    CHECK(!m.activate(99) && !m.activate(-1));

    // Menus: a running app can quit and hide, one that is not cannot; both can be kept or not.
    auto hasAction = [](const std::vector<MenuItem>& menu, const std::string& action) {
        for (const MenuItem& item : menu) {
            if (item.action == action)
                return true;
        }
        return false;
    };
    const int photos = m.find("photos");
    CHECK(hasAction(m.menuFor(music), "quit") && hasAction(m.menuFor(music), "hide"));
    CHECK(!hasAction(m.menuFor(photos), "quit") && !hasAction(m.menuFor(photos), "hide"));
    CHECK(m.menuFor(photos).front().kind == "header" && m.menuFor(photos).front().text == "PHOTOS");
    CHECK(m.menuFor(99).empty());
    for (const MenuItem& item : m.menuFor(music)) {
        // Headers and separators say nothing; items always do.
        CHECK((item.kind == "item") == !item.action.empty());
    }

    // Hide puts a window in the tray, restoring puts it back and brings its app forward.
    const std::size_t before = m.minimized().size();
    const int browser = m.find("browser");
    const int windows = m.apps()[browser].windows;
    CHECK(m.perform(browser, "hide"));
    CHECK(m.minimized().size() == before + 1 && m.apps()[browser].windows == windows - 1);
    CHECK(m.restoreWindow(int(m.minimized().size()) - 1));
    CHECK(m.minimized().size() == before && m.apps()[browser].windows == windows && m.apps()[browser].active);

    // Quitting stops an app and keeps it if it is pinned; an app that is not pinned leaves.
    CHECK(m.perform(music, "quit"));
    CHECK(!m.apps()[music].running && !m.apps()[music].removing && m.apps()[music].badge == 0);
    CHECK(m.perform(music, "keep"));
    CHECK(!m.apps()[music].pinned && m.apps()[music].removing);
    m.finishRemoval();
    CHECK(m.find("music") == -1 && m.apps().size() == 8);

    // Reordering moves one and keeps the rest in order.
    CHECK(order(m) == "files,browser,mail,photos,calendar,notes,terminal,settings");
    CHECK(m.reorder(0, 2));
    CHECK(order(m) == "browser,mail,files,photos,calendar,notes,terminal,settings");
    CHECK(m.reorder(5, 1));
    CHECK(order(m) == "browser,notes,mail,files,photos,calendar,terminal,settings");
    CHECK(!m.reorder(3, 3) && !m.reorder(-1, 2) && !m.reorder(2, 8));

    // Dragging off: a running app stays (unpinned), one that is not leaves.
    const int terminal = m.find("terminal");
    CHECK(m.dragOff(terminal));
    CHECK(!m.apps()[terminal].pinned && !m.apps()[terminal].removing);
    const int settings = m.find("settings");
    CHECK(m.dragOff(settings));
    CHECK(m.apps()[settings].removing);
    m.finishRemoval();
    CHECK(m.find("settings") == -1 && m.find("terminal") >= 0);

    // The bin.
    CHECK(m.trashFull());
    CHECK(m.performTrash("empty") && !m.trashFull() && !m.performTrash("empty"));

    if (failures == 0)
        std::puts("model_test: all checks passed");
    return failures ? 1 : 0;
}
