#include "app.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <spawn.h>
#include <unistd.h>

extern char** environ;

namespace files {

namespace {

slint::SharedString str(const std::string& s)
{
    return slint::SharedString(std::string_view(s));
}

std::string lowered(std::string_view text)
{
    std::string out(text);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return out;
}

std::string trimmed(std::string_view text)
{
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
        text.remove_prefix(1);
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
        text.remove_suffix(1);
    return std::string(text);
}

// Open a file with whatever the desktop has for it, without waiting for it.
void openWithDesktop(const fs::path& path)
{
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_addopen(&actions, 1, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_addopen(&actions, 2, "/dev/null", O_WRONLY, 0);
    const std::string target = path.string();
    char* const argv[] = {const_cast<char*>("xdg-open"), const_cast<char*>(target.c_str()), nullptr};
    pid_t pid = 0;
    if (posix_spawnp(&pid, "xdg-open", &actions, nullptr, argv, environ) != 0)
        std::fprintf(stderr, "files: could not start xdg-open for %s\n", target.c_str());
    posix_spawn_file_actions_destroy(&actions);
}

bool within(const fs::path& folder, const fs::path& place)
{
    const fs::path relative = folder.lexically_relative(place);
    return !relative.empty() && *relative.begin() != "..";
}

} // namespace

App::App(const Options& options)
    : m_ui(FilesWindow::create())
    , m_entries(std::make_shared<slint::VectorModel<FileEntry>>())
    , m_placeModel(std::make_shared<slint::VectorModel<Place>>())
    , m_crumbModel(std::make_shared<slint::VectorModel<slint::SharedString>>())
{
    m_ui->set_dark(options.dark);
    m_ui->set_frameless(options.frameless);
    m_ui->set_view_mode(options.list ? 1 : 0);
    m_ui->set_icon_size(options.iconSize);
    m_ui->set_entries(m_entries);
    m_ui->set_crumbs(m_crumbModel);
    m_ui->set_places(m_placeModel);
    if (options.accent.size() == 7 && options.accent[0] == '#') {
        const unsigned long rgb = std::strtoul(options.accent.c_str() + 1, nullptr, 16);
        m_ui->set_accent(slint::Color::from_rgb_uint8((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff));
    }

    m_places = places();
    std::vector<Place> rows;
    for (const PlaceDef& p : m_places)
        rows.push_back(Place {str(p.name), str(p.icon), p.header, p.ejectable});
    m_placeModel = std::make_shared<slint::VectorModel<Place>>(std::move(rows));
    m_ui->set_places(m_placeModel);

    wire();

    fs::path start = options.start.empty() ? homeFolder() : options.start;
    if (!go(start))
        go(homeFolder());
}

// ---- Navigation -------------------------------------------------------------------------------

bool App::go(const fs::path& target, bool remember)
{
    std::error_code ec;
    fs::path folder = fs::weakly_canonical(target, ec);
    if (ec)
        folder = target.lexically_normal();
    if (!fs::is_directory(folder, ec))
        return false;

    std::vector<Item> items = listFolder(folder, m_showHidden, ec);
    m_emptyText = "This folder is empty";
    if (ec)
        m_emptyText = ec == std::errc::permission_denied ? "You don't have permission to open this folder" : ec.message();

    m_folder = folder;
    if (remember) {
        m_history.resize(m_history.empty() ? 0 : m_historyPos + 1);
        if (m_history.empty() || m_history.back() != folder)
            m_history.push_back(folder);
        m_historyPos = m_history.size() - 1;
    }

    m_items = std::move(items);
    sortItems(m_items, m_sort, m_ascending);
    m_selected.clear();
    m_anchor = m_cursor = -1;
    m_filter.clear();
    m_ui->set_editor_text("");
    m_thumbnails.startFolder();

    // Before the entries change, for the new items to arrive one after another.
    m_ui->invoke_folder_opening();
    rebuildView();
    publishNavigation();
    return true;
}

void App::goBack()
{
    if (m_historyPos > 0) {
        --m_historyPos;
        go(m_history[m_historyPos], false);
    }
}

void App::goForward()
{
    if (m_historyPos + 1 < m_history.size()) {
        ++m_historyPos;
        go(m_history[m_historyPos], false);
    }
}

// Read the same folder again, as a new file shows up or hidden files are asked for. The items
// do not arrive again and what was selected stays selected.
void App::reload()
{
    std::error_code ec;
    m_items = listFolder(m_folder, m_showHidden, ec);
    sortItems(m_items, m_sort, m_ascending);
    std::unordered_set<std::string> present;
    for (const Item& item : m_items)
        present.insert(item.path.string());
    for (auto it = m_selected.begin(); it != m_selected.end();)
        it = present.count(*it) ? std::next(it) : m_selected.erase(it);
    m_thumbnails.startFolder();
    rebuildView();
}

void App::open(const Item& item)
{
    if (item.isDir)
        go(item.path);
    else
        openWithDesktop(item.path);
}

// ---- What is showing --------------------------------------------------------------------------

void App::rebuildView()
{
    const std::string needle = lowered(m_filter);
    m_showing.clear();
    for (int i = 0; i < int(m_items.size()); ++i) {
        if (needle.empty() || lowered(m_items[i].name).find(needle) != std::string::npos)
            m_showing.push_back(i);
    }
    m_anchor = m_cursor = -1;
    publishEntries();
}

void App::publishEntries()
{
    const std::int64_t now = std::time(nullptr);
    m_rows.clear();
    m_rows.reserve(m_showing.size());
    for (int index : m_showing) {
        const Item& item = m_items[index];
        FileEntry row {}; // value-initialised: the generated struct leaves its bools unset
        row.name = str(item.name);
        row.kind = str(std::string(kindName(item.kind)));
        row.hidden = item.hidden;
        row.selected = m_selected.count(item.path.string()) > 0;
        row.size = str(sizeText(item));
        row.modified = str(humanDate(item.modified, now));
        row.kind_label = str(item.kindLabel);
        if (auto image = m_thumbnails.get(item)) {
            row.thumbnail = *image;
            row.has_thumbnail = true;
        }
        m_rows.push_back(std::move(row));
    }
    m_entries = std::make_shared<slint::VectorModel<FileEntry>>(m_rows);
    m_ui->set_entries(m_entries);
    publishStatus();
}

// A selection changes a few rows, not the folder: change those, and the cards animate.
void App::publishSelection()
{
    for (std::size_t i = 0; i < m_rows.size(); ++i) {
        const bool selected = isSelected(int(i));
        if (m_rows[i].selected != selected) {
            m_rows[i].selected = selected;
            m_entries->set_row_data(i, m_rows[i]);
        }
    }
    publishStatus();
}

void App::publishNavigation()
{
    // The crumbs.
    m_crumbPaths.clear();
    std::vector<slint::SharedString> names;
    for (const Crumb& crumb : crumbs(m_folder)) {
        names.push_back(str(crumb.name));
        m_crumbPaths.push_back(crumb.path);
    }
    m_crumbModel = std::make_shared<slint::VectorModel<slint::SharedString>>(std::move(names));
    m_ui->set_crumbs(m_crumbModel);

    m_ui->set_folder_name(str(m_crumbPaths.size() <= 1 || m_folder.filename().empty() ? crumbs(m_folder).back().name
                                                                                      : m_folder.filename().string()));
    m_ui->set_can_back(m_historyPos > 0);
    m_ui->set_can_forward(m_historyPos + 1 < m_history.size());

    // The place that holds this folder: the deepest one.
    int best = -1;
    std::size_t bestLength = 0;
    for (int i = 0; i < int(m_places.size()); ++i) {
        const PlaceDef& p = m_places[i];
        if (p.header || p.path.empty() || !within(m_folder, p.path))
            continue;
        const std::size_t length = p.path.string().size();
        if (best < 0 || length > bestLength) {
            best = i;
            bestLength = length;
        }
    }
    m_ui->set_selected_place(best);
    m_ui->set_sort_column(str(std::string(sortColumnName(m_sort))));
    m_ui->set_sort_ascending(m_ascending);
}

void App::publishStatus()
{
    const std::size_t total = m_items.size();
    const std::size_t shown = m_showing.size();
    std::size_t selected = 0;
    std::uint64_t bytes = 0;
    for (int index : m_showing) {
        const Item& item = m_items[index];
        if (m_selected.count(item.path.string())) {
            ++selected;
            bytes += item.bytes;
        }
    }

    std::string status;
    if (!m_filter.empty()) {
        status = std::to_string(shown) + " of " + std::to_string(total) + " matching \xE2\x80\x9C" + m_filter + "\xE2\x80\x9D";
        m_ui->set_empty_text(str("No results for \xE2\x80\x9C" + m_filter + "\xE2\x80\x9D"));
    } else {
        status = std::to_string(shown) + (shown == 1 ? " item" : " items");
        m_ui->set_empty_text(str(m_emptyText));
    }
    if (selected > 0) {
        status += ", " + std::to_string(selected) + " selected";
        if (bytes > 0)
            status += " (" + humanSize(bytes) + ")";
    }
    if (shown == 0)
        status.clear();
    m_ui->set_status(str(status));
}

// ---- Selection --------------------------------------------------------------------------------

bool App::isSelected(int index) const
{
    return index >= 0 && index < int(m_showing.size()) && m_selected.count(m_items[m_showing[index]].path.string()) > 0;
}

void App::selectOnly(int index)
{
    m_selected.clear();
    if (index >= 0 && index < int(m_showing.size()))
        m_selected.insert(m_items[m_showing[index]].path.string());
    m_anchor = m_cursor = index;
}

void App::selectRange(int from, int to)
{
    if (from < 0)
        from = to;
    const int low = std::min(from, to), high = std::max(from, to);
    for (int i = std::max(0, low); i <= high && i < int(m_showing.size()); ++i)
        m_selected.insert(m_items[m_showing[i]].path.string());
    m_cursor = to;
}

void App::selectAll()
{
    for (int index : m_showing)
        m_selected.insert(m_items[index].path.string());
}

void App::clearSelection()
{
    m_selected.clear();
    m_anchor = m_cursor = -1;
}

void App::press(int index, bool ctrl, bool shift)
{
    if (index < 0 || index >= int(m_showing.size()))
        return;
    if (shift && m_anchor >= 0) {
        if (!ctrl)
            m_selected.clear();
        selectRange(m_anchor, index);
    } else if (ctrl) {
        const std::string path = m_items[m_showing[index]].path.string();
        if (!m_selected.erase(path))
            m_selected.insert(path);
        m_anchor = m_cursor = index;
    } else {
        selectOnly(index);
    }
    publishSelection();
}

void App::moveSelection(int to, bool extend)
{
    if (m_showing.empty())
        return;
    to = std::clamp(to, 0, int(m_showing.size()) - 1);
    if (extend && m_anchor >= 0) {
        m_selected.clear();
        selectRange(m_anchor, to);
    } else {
        selectOnly(to);
    }
    publishSelection();
    m_ui->invoke_reveal(to);
}

void App::keyCommand(const std::string& command, bool ctrl, bool shift)
{
    (void)ctrl;
    const bool grid = m_ui->get_view_mode() == 0;
    const int columns = grid ? std::max(1, m_ui->get_columns()) : 1;
    const int count = int(m_showing.size());
    const int from = m_cursor >= 0 ? m_cursor : -1;

    if (command == "up") {
        moveSelection(from < 0 ? count - 1 : from - columns, shift);
    } else if (command == "down") {
        moveSelection(from < 0 ? 0 : from + columns, shift);
    } else if (command == "left" && grid) {
        moveSelection(from < 0 ? count - 1 : from - 1, shift);
    } else if (command == "right" && grid) {
        moveSelection(from < 0 ? 0 : from + 1, shift);
    } else if (command == "home") {
        moveSelection(0, shift);
    } else if (command == "end") {
        moveSelection(count - 1, shift);
    } else if (command == "open") {
        // Folders open one at a time; files all open.
        std::vector<const Item*> chosen;
        for (int index : m_showing) {
            if (m_selected.count(m_items[index].path.string()))
                chosen.push_back(&m_items[index]);
        }
        for (const Item* item : chosen) {
            if (item->isDir) {
                go(item->path);
                break;
            }
            open(*item);
        }
    } else if (command == "parent") {
        if (m_folder.has_parent_path() && m_folder.parent_path() != m_folder)
            go(m_folder.parent_path());
    } else if (command == "back") {
        goBack();
    } else if (command == "forward") {
        goForward();
    } else if (command == "select-all") {
        selectAll();
        publishSelection();
    } else if (command == "clear-selection") {
        clearSelection();
        publishSelection();
    } else if (command == "toggle-hidden") {
        m_showHidden = !m_showHidden;
        reload();
    } else if (command == "delete") {
        // Left for the real backend: moving files to the trash is its business.
        std::fprintf(stderr, "files: delete is not implemented in this stand-in backend\n");
    }
}

// ---- The window's callbacks -------------------------------------------------------------------

void App::wire()
{
    m_ui->on_place_clicked([this](int i) {
        if (i >= 0 && i < int(m_places.size()) && !m_places[i].header)
            go(m_places[i].path);
    });
    m_ui->on_place_context([](int i, float x, float y) {
        // A real backend shows its menu for place `i` here: open, open in a new window, eject.
        std::fprintf(stderr, "files: context menu for place %d at %.0f,%.0f\n", i, x, y);
    });
    m_ui->on_eject_clicked([this](int i) {
        // A real backend unmounts the device here.
        std::fprintf(stderr, "files: eject %s (not implemented in this stand-in backend)\n",
                     i >= 0 && i < int(m_places.size()) ? m_places[i].name.c_str() : "?");
    });

    m_ui->on_back([this] { goBack(); });
    m_ui->on_forward([this] { goForward(); });
    m_ui->on_crumb_clicked([this](int i) {
        if (i >= 0 && i < int(m_crumbPaths.size()))
            go(m_crumbPaths[i]);
    });
    m_ui->on_path_accepted([this](slint::SharedString text) {
        const std::string typed = trimmed(std::string_view(text));
        if (typed.empty())
            return;
        const fs::path path = expandUser(typed);
        std::error_code ec;
        if (fs::is_directory(path, ec))
            go(path);
        else if (fs::exists(path, ec))
            openWithDesktop(path);
        // Anything else was a search, and the filter stays.
    });
    m_ui->on_search_edited([this](slint::SharedString text) {
        const std::string typed = trimmed(std::string_view(text));
        // A path being typed is not a search.
        const std::string filter = (!typed.empty() && (typed[0] == '/' || typed[0] == '~')) ? std::string() : typed;
        if (filter == m_filter)
            return;
        m_filter = filter;
        rebuildView();
    });

    m_ui->on_view_mode_selected([this](int mode) { m_ui->set_view_mode(mode); });
    m_ui->on_sort_requested([this](slint::SharedString column) {
        const SortColumn requested = sortColumnFromName(std::string_view(column));
        if (requested == m_sort)
            m_ascending = !m_ascending;
        else {
            m_sort = requested;
            m_ascending = true;
        }
        sortItems(m_items, m_sort, m_ascending);
        m_ui->set_sort_column(str(std::string(sortColumnName(m_sort))));
        m_ui->set_sort_ascending(m_ascending);
        rebuildView();
    });

    m_ui->on_item_pressed([this](int i, bool ctrl, bool shift) { press(i, ctrl, shift); });
    m_ui->on_item_activated([this](int i) {
        if (i >= 0 && i < int(m_showing.size()))
            open(m_items[m_showing[i]]);
    });
    m_ui->on_item_context([this](int i, float x, float y) {
        if (!isSelected(i)) {
            selectOnly(i);
            publishSelection();
        }
        // A real backend shows its menu for the selection here: open, copy, rename, trash.
        std::fprintf(stderr, "files: context menu for %zu selected at %.0f,%.0f\n", m_selected.size(), x, y);
    });
    m_ui->on_background_pressed([this] {
        clearSelection();
        publishSelection();
    });
    m_ui->on_background_context([](float x, float y) {
        // A real backend shows its menu for the folder here: new folder, paste, sort by.
        std::fprintf(stderr, "files: context menu for the folder at %.0f,%.0f\n", x, y);
    });
    m_ui->on_key_command([this](slint::SharedString command, bool ctrl, bool shift) {
        keyCommand(std::string(std::string_view(command)), ctrl, shift);
    });

    // The buttons and the dragging of a window the app draws itself.
    m_ui->on_window_control([this](slint::SharedString command) {
        slint::Window& window = m_ui->window();
        if (command == "close")
            m_ui->hide();
        else if (command == "minimize")
            window.set_minimized(true);
        else if (command == "maximize")
            window.set_maximized(!window.is_maximized());
    });
    m_ui->on_window_drag([this](float dx, float dy) {
        // Not every platform lets a program place its windows (Wayland does not).
        slint::Window& window = m_ui->window();
        const float scale = window.scale_factor();
        slint::PhysicalPosition position = window.position();
        position.x += int(std::lround(dx * scale));
        position.y += int(std::lround(dy * scale));
        window.set_position(position);
    });
}

} // namespace files
