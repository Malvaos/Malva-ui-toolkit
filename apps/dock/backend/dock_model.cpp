#include "dock_model.h"

#include <algorithm>
#include <cctype>

namespace dockui {

static AppState make(const char* id, const char* name, const char* icon, bool running = false, int windows = 0,
                     int badge = 0)
{
    AppState app;
    app.id = id;
    app.name = name;
    app.icon = icon;
    app.pinned = true;
    app.running = running;
    app.windows = windows;
    app.badge = badge;
    return app;
}

Model Model::sample()
{
    Model m;
    m.m_apps = {make("files", "Files", "files", true, 2),
                make("browser", "Browser", "browser", true, 3),
                make("mail", "Mail", "mail", true, 1, 4),
                make("music", "Music", "music"),
                make("photos", "Photos", "photos"),
                make("calendar", "Calendar", "calendar"),
                make("notes", "Notes", "notes", true, 1),
                make("terminal", "Terminal", "terminal", true, 1),
                make("settings", "Settings", "settings")};
    m.m_apps[0].active = true;
    m.m_minimized = {{"Report.pdf", "files"}};
    return m;
}

int Model::find(const std::string& id) const
{
    for (std::size_t i = 0; i < m_apps.size(); ++i) {
        if (m_apps[i].id == id)
            return int(i);
    }
    return -1;
}

void Model::focus(int index)
{
    for (std::size_t i = 0; i < m_apps.size(); ++i)
        m_apps[i].active = int(i) == index;
}

bool Model::activate(int index)
{
    if (index < 0 || index >= int(m_apps.size()))
        return false;
    AppState& app = m_apps[index];
    if (app.removing || app.launching)
        return false;
    if (!app.running) {
        app.launching = true;
        return true;
    }
    focus(index);
    return true;
}

void Model::finishLaunch(int index)
{
    if (index < 0 || index >= int(m_apps.size()))
        return;
    AppState& app = m_apps[index];
    app.launching = false;
    app.running = true;
    app.windows = std::max(app.windows, 1);
    focus(index);
}

std::vector<MenuItem> Model::menuFor(int index) const
{
    std::vector<MenuItem> menu;
    if (index < 0 || index >= int(m_apps.size()))
        return menu;
    const AppState& app = m_apps[index];
    std::string upper = app.name;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) { return std::toupper(c); });
    menu.push_back({upper, "header", "", false, false, true, ""});
    menu.push_back({"New Window", "item", "new-window", false, false, true, "new-window"});
    if (app.running) {
        menu.push_back({"Show All Windows", "item", "windows", false, false, app.windows > 1, "show-windows"});
    }
    menu.push_back({"", "separator", "", false, false, true, ""});
    menu.push_back({"Keep in Dock", "item", "", app.pinned, false, true, "keep"});
    if (app.running) {
        menu.push_back({"Hide", "item", "hide", false, false, app.windows > 0, "hide"});
        menu.push_back({"", "separator", "", false, false, true, ""});
        menu.push_back({"Quit", "item", "quit", false, true, true, "quit"});
    } else if (!app.pinned) {
        menu.push_back({"Remove from Dock", "item", "close", false, true, true, "remove"});
    }
    return menu;
}

std::vector<MenuItem> Model::trashMenu() const
{
    return {{"TRASH", "header", "", false, false, true, ""},
            {"Open Trash", "item", "folder", false, false, true, "open"},
            {"", "separator", "", false, false, true, ""},
            {"Empty Trash", "item", "empty", false, true, m_trashFull, "empty"}};
}

std::vector<MenuItem> Model::windowMenu(int index) const
{
    if (index < 0 || index >= int(m_minimized.size()))
        return {};
    return {{m_minimized[index].title, "header", "", false, false, true, ""},
            {"Show Window", "item", "windows", false, false, true, "restore"},
            {"Close Window", "item", "close", false, true, true, "close"}};
}

bool Model::perform(int index, const std::string& action)
{
    if (index < 0 || index >= int(m_apps.size()))
        return false;
    AppState& app = m_apps[index];
    if (action == "new-window") {
        if (!app.running) {
            app.launching = true;
        } else {
            app.windows += 1;
            focus(index);
        }
        return true;
    }
    if (action == "show-windows") {
        focus(index);
        return true;
    }
    if (action == "keep") {
        app.pinned = !app.pinned;
        // An app that is neither pinned nor running has no business in the dock.
        if (!app.pinned && !app.running)
            app.removing = true;
        return true;
    }
    if (action == "hide") {
        if (app.windows <= 0)
            return false;
        app.windows -= 1;
        app.active = false;
        m_minimized.push_back({app.name + " window", app.icon});
        return true;
    }
    if (action == "quit") {
        app.running = false;
        app.active = false;
        app.windows = 0;
        app.badge = 0;
        if (!app.pinned)
            app.removing = true;
        return true;
    }
    if (action == "remove") {
        app.removing = true;
        return true;
    }
    return false;
}

bool Model::performTrash(const std::string& action)
{
    if (action == "empty" && m_trashFull) {
        m_trashFull = false;
        return true;
    }
    return false;
}

bool Model::reorder(int from, int to)
{
    const int n = int(m_apps.size());
    if (from < 0 || from >= n || to < 0 || to >= n || from == to)
        return false;
    AppState moved = m_apps[from];
    m_apps.erase(m_apps.begin() + from);
    m_apps.insert(m_apps.begin() + to, moved);
    return true;
}

bool Model::dragOff(int index)
{
    if (index < 0 || index >= int(m_apps.size()))
        return false;
    AppState& app = m_apps[index];
    app.pinned = false;
    if (!app.running)
        app.removing = true;
    return true;
}

void Model::finishRemoval()
{
    m_apps.erase(std::remove_if(m_apps.begin(), m_apps.end(), [](const AppState& a) { return a.removing; }),
                 m_apps.end());
}

bool Model::restoreWindow(int index)
{
    if (index < 0 || index >= int(m_minimized.size()))
        return false;
    const std::string icon = m_minimized[index].icon;
    m_minimized.erase(m_minimized.begin() + index);
    for (std::size_t i = 0; i < m_apps.size(); ++i) {
        if (m_apps[i].icon == icon) {
            m_apps[i].windows += 1;
            focus(int(i));
            break;
        }
    }
    return true;
}

} // namespace dockui
