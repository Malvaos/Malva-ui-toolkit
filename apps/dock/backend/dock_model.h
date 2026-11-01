// What the dock holds and what happens to it: which apps are pinned, running, active, which
// windows are minimised, what each menu offers and what each choice does. No Slint, no drawing, no
// real processes: a stand-in to replace with the desktop's own idea of an app.
#pragma once

#include <string>
#include <vector>

namespace dockui {

struct AppState {
    std::string id;
    std::string name;
    std::string icon; // one of the dock's built-in icon names
    bool pinned = false;
    bool running = false;
    bool active = false;
    bool launching = false;
    bool removing = false; // leaving: the UI shrinks it away, then `finishRemoval` drops it
    int badge = 0;
    int windows = 0;
};

struct MinimizedState {
    std::string title;
    std::string icon;
};

struct MenuItem {
    std::string text;
    std::string kind = "item"; // "item", "header" or "separator"
    std::string icon;          // one of the dock's symbol names, or empty
    bool checked = false;
    bool destructive = false;
    bool enabled = true;
    std::string action; // what choosing it does; empty for headers and separators
};

class Model
{
public:
    // A dock with a few apps pinned, some of them running.
    static Model sample();

    const std::vector<AppState>& apps() const { return m_apps; }
    const std::vector<MinimizedState>& minimized() const { return m_minimized; }
    bool trashFull() const { return m_trashFull; }

    int find(const std::string& id) const;

    // A click on an app. One that is not running starts (`launching`, until `finishLaunch`);
    // one that is running comes to the front. Returns true if something changed.
    bool activate(int index);
    // The app has started: it runs, with a window, in front.
    void finishLaunch(int index);

    // The menu of an app, and doing what one of its entries says (by `action`).
    std::vector<MenuItem> menuFor(int index) const;
    std::vector<MenuItem> trashMenu() const;
    std::vector<MenuItem> windowMenu(int index) const;
    bool perform(int index, const std::string& action);
    bool performTrash(const std::string& action);

    // Dragging.
    bool reorder(int from, int to);
    // Taken off the dock: an app that is running stays while it runs; any other leaves.
    bool dragOff(int index);
    // Drops an app that is leaving for good (after its leaving animation).
    void finishRemoval();

    // A minimised window comes back.
    bool restoreWindow(int index);

private:
    std::vector<AppState> m_apps;
    std::vector<MinimizedState> m_minimized;
    bool m_trashFull = true;
    void focus(int index);
};

} // namespace dockui
