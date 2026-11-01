// The glue: holds what the window is showing (the folder, its history, the selection, the
// filter, the sort) and answers the window's callbacks by changing it. All the look and all the
// motion are in the .slint files; nothing here draws.
#pragma once

#include "fs.h"
#include "thumbnails.h"

#include "files.h" // generated from ui/files.slint

#include <slint.h>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace files {

struct Options {
    fs::path start;
    bool dark = false;
    bool frameless = false;
    bool list = false;
    std::string accent; // "#rrggbb", or empty for the default
    float iconSize = 64;
};

class App
{
public:
    explicit App(const Options& options);

    slint::ComponentHandle<FilesWindow> window() const { return m_ui; }

private:
    // Navigation.
    bool go(const fs::path& folder, bool remember = true);
    void goBack();
    void goForward();
    void reload();
    void open(const Item& item);

    // What is showing, pushed to the window.
    void rebuildView();
    void publishEntries();
    void publishNavigation();
    void publishStatus();
    void publishSelection();

    // Selection, by index into the entries showing.
    void press(int index, bool ctrl, bool shift);
    void selectOnly(int index);
    void selectRange(int from, int to);
    void selectAll();
    void clearSelection();
    bool isSelected(int index) const;
    void moveSelection(int to, bool extend);
    void keyCommand(const std::string& command, bool ctrl, bool shift);

    void wire();

    slint::ComponentHandle<FilesWindow> m_ui;
    Thumbnails m_thumbnails;

    // The folder, and where it has been.
    fs::path m_folder;
    std::vector<fs::path> m_history;
    std::size_t m_historyPos = 0;
    std::string m_emptyText = "This folder is empty";

    // Everything in the folder, sorted; and the part of it showing, after the filter.
    std::vector<Item> m_items;
    std::vector<int> m_showing;
    std::string m_filter;
    bool m_showHidden = false;
    SortColumn m_sort = SortColumn::Name;
    bool m_ascending = true;

    std::unordered_set<std::string> m_selected; // paths
    int m_anchor = -1;
    int m_cursor = -1;

    std::vector<PlaceDef> m_places;
    std::vector<fs::path> m_crumbPaths;

    std::shared_ptr<slint::VectorModel<FileEntry>> m_entries;
    std::vector<FileEntry> m_rows; // what the model holds, to change one row at a time
    std::shared_ptr<slint::VectorModel<Place>> m_placeModel;
    std::shared_ptr<slint::VectorModel<slint::SharedString>> m_crumbModel;
};

} // namespace files
