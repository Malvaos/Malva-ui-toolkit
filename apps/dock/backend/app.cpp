#include "app.h"

#include <algorithm>
#include <cstdlib>

namespace dockui {

static slint::SharedString str(const std::string& s)
{
    return slint::SharedString(s);
}

static DockApp toRow(const AppState& a)
{
    DockApp row {};
    row.id = str(a.id);
    row.name = str(a.name);
    row.icon = str(a.icon);
    row.pinned = a.pinned;
    row.running = a.running;
    row.active = a.active;
    row.launching = a.launching;
    row.badge = a.badge;
    row.windows = a.windows;
    row.removing = a.removing;
    return row;
}

static MenuEntry toEntry(const MenuItem& m)
{
    MenuEntry e {};
    e.text = str(m.text);
    e.kind = str(m.kind);
    e.icon = str(m.icon);
    e.checked = m.checked;
    e.destructive = m.destructive;
    e.enabled = m.enabled;
    return e;
}

static bool sameRow(const DockApp& a, const DockApp& b)
{
    return a.id == b.id && a.name == b.name && a.icon == b.icon && a.pinned == b.pinned && a.running == b.running
            && a.active == b.active && a.launching == b.launching && a.badge == b.badge && a.windows == b.windows
            && a.removing == b.removing;
}

static slint::Color parseColor(const std::string& text, slint::Color fallback)
{
    if (text.size() == 7 && text[0] == '#') {
        const unsigned long v = std::strtoul(text.c_str() + 1, nullptr, 16);
        return slint::Color::from_rgb_uint8((v >> 16) & 255, (v >> 8) & 255, v & 255);
    }
    return fallback;
}

App::App(const Options& options)
    : m_ui(DockWindow::create())
    , m_model(Model::sample())
    , m_options(options)
    , m_apps(std::make_shared<slint::VectorModel<DockApp>>())
    , m_minimized(std::make_shared<slint::VectorModel<MinimizedWindow>>())
    , m_menu(std::make_shared<slint::VectorModel<MenuEntry>>())
{
    auto ui = m_ui;
    ui->set_dark(options.dark);
    if (!options.accent.empty())
        ui->set_accent(parseColor(options.accent, slint::Color::from_rgb_uint8(0x33, 0x85, 0xFF)));
    ui->set_edge(str(options.edge));
    ui->set_icon_size(options.iconSize);
    ui->set_magnification(options.magnification);
    ui->set_show_launcher(options.launcher);
    ui->set_animate_entrance(options.animateEntrance);
    if (!options.desk.empty())
        ui->set_desk(slint::Brush(parseColor(options.desk, slint::Color::from_rgb_uint8(0, 0, 0))));
    ui->set_apps(m_apps);
    ui->set_minimized_windows(m_minimized);
    ui->set_menu_entries(m_menu);
    if (!options.deferFill)
        sync();

    ui->on_app_clicked([this](int i) {
        closeMenu();
        launch(i);
    });
    ui->on_launcher_clicked([this] { closeMenu(); });
    ui->on_trash_clicked([this] { closeMenu(); });
    ui->on_window_clicked([this](int j) {
        closeMenu();
        if (m_model.restoreWindow(j))
            sync();
    });
    ui->on_app_context([this](int i) { openMenu("app", i); });
    ui->on_window_context([this](int j) { openMenu("window", j); });
    ui->on_trash_context([this] { openMenu("trash", 0); });
    ui->on_launcher_context([this] { closeMenu(); });
    ui->on_menu_chosen([this](int row) { chooseMenu(row); });
    ui->on_menu_dismissed([this] { closeMenu(); });
    ui->on_reordered([this](int from, int to) {
        if (m_model.reorder(from, to))
            sync();
    });
    ui->on_app_dragged_off([this](int i) {
        closeMenu();
        if (m_model.dragOff(i)) {
            sync();
            scheduleRemoval();
        }
    });
    ui->on_reveal_requested([this] {
        m_outsideTicks = 0;
        m_ui->set_hidden(false);
    });

    if (options.autoHide) {
        // Come back when the pointer touches the edge (the window says so); go away a moment after
        // it has left the dock and nothing is open.
        m_hideTimer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(150), [this] { checkAutoHide(); });
    }
}

void App::checkAutoHide()
{
    if (m_ui->get_pointer_inside() || m_ui->get_menu_open()) {
        m_outsideTicks = 0;
        return;
    }
    if (!m_ui->get_hidden() && ++m_outsideTicks >= 4)
        m_ui->set_hidden(true);
}

void App::sync()
{
    // Rows change in place, so an icon that stays keeps its animation state.
    const auto& apps = m_model.apps();
    for (std::size_t i = 0; i < apps.size(); ++i) {
        const DockApp row = toRow(apps[i]);
        if (i < m_apps->row_count()) {
            if (!sameRow(*m_apps->row_data(i), row))
                m_apps->set_row_data(i, row);
        } else {
            m_apps->push_back(row);
        }
    }
    while (m_apps->row_count() > apps.size())
        m_apps->erase(m_apps->row_count() - 1);

    const auto& mins = m_model.minimized();
    while (m_minimized->row_count() > mins.size())
        m_minimized->erase(m_minimized->row_count() - 1);
    for (std::size_t j = 0; j < mins.size(); ++j) {
        MinimizedWindow row {};
        row.title = str(mins[j].title);
        row.icon = str(mins[j].icon);
        row.has_preview = false;
        if (j < m_minimized->row_count())
            m_minimized->set_row_data(j, row);
        else
            m_minimized->push_back(row);
    }
    m_ui->set_trash_full(m_model.trashFull());
}

void App::launch(int index)
{
    if (!m_model.activate(index))
        return;
    sync();
    if (index >= 0 && index < int(m_model.apps().size()) && m_model.apps()[index].launching) {
        const std::string id = m_model.apps()[index].id;
        // Once the app is up, its mark appears and the hop stops.
        slint::Timer::single_shot(std::chrono::milliseconds(m_options.launchMs), [this, id] {
            const int now = m_model.find(id);
            if (now >= 0 && m_model.apps()[now].launching) {
                m_model.finishLaunch(now);
                sync();
            }
        });
    }
}

void App::openMenu(const std::string& target, int index)
{
    m_menuTarget = target;
    m_menuIndex = index;
    if (target == "app")
        m_menuItems = m_model.menuFor(index);
    else if (target == "window")
        m_menuItems = m_model.windowMenu(index);
    else
        m_menuItems = m_model.trashMenu();
    m_menu->clear();
    for (const MenuItem& item : m_menuItems)
        m_menu->push_back(toEntry(item));
    m_ui->set_menu_open(!m_menuItems.empty());
}

void App::closeMenu()
{
    if (m_ui->get_menu_open())
        m_ui->set_menu_open(false);
}

void App::chooseMenu(int row)
{
    if (row < 0 || row >= int(m_menuItems.size()))
        return;
    const MenuItem item = m_menuItems[row];
    if (item.kind != "item" || !item.enabled)
        return;
    closeMenu();
    bool changed = false;
    if (m_menuTarget == "app") {
        const std::string id = m_model.apps()[m_menuIndex].id;
        changed = m_model.perform(m_menuIndex, item.action);
        const int now = m_model.find(id);
        if (item.action == "new-window" && now >= 0 && m_model.apps()[now].launching) {
            sync();
            changed = false;
            const std::string again = id;
            slint::Timer::single_shot(std::chrono::milliseconds(m_options.launchMs), [this, again] {
                const int i = m_model.find(again);
                if (i >= 0 && m_model.apps()[i].launching) {
                    m_model.finishLaunch(i);
                    sync();
                }
            });
        }
    } else if (m_menuTarget == "window") {
        if (item.action == "restore")
            changed = m_model.restoreWindow(m_menuIndex);
    } else {
        changed = m_model.performTrash(item.action);
    }
    if (changed)
        sync();
    scheduleRemoval();
}

void App::scheduleRemoval()
{
    bool leaving = false;
    for (const AppState& a : m_model.apps())
        leaving = leaving || a.removing;
    if (!leaving)
        return;
    sync();
    // The icon shrinks away over 260 ms; drop its row once it has.
    m_removalTimer.start(slint::TimerMode::SingleShot, std::chrono::milliseconds(320), [this] {
        m_model.finishRemoval();
        sync();
    });
}

} // namespace dockui
