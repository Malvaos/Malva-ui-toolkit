// The file system, as the UI wants to see it. Nothing here knows about Slint: this is the layer
// to replace with a real backend (a proper icon theme, thumbnails, trash, mounts, file operations).
// What it does now is the least that makes the window a working browser.
#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace files {

namespace fs = std::filesystem;

// The icons the UI has: it draws a coloured icon for each of these.
enum class Kind { Folder, Image, Pdf, Text, Archive, Audio, Video, File };

// The name the UI knows the kind by ("folder", "image", ...).
std::string_view kindName(Kind kind);

struct Item {
    fs::path path;
    std::string name;
    Kind kind = Kind::File;
    // What the Kind column says: "JPEG image", "Folder".
    std::string kindLabel;
    bool isDir = false;
    bool hidden = false;
    std::uint64_t bytes = 0;
    // Entries of a folder.
    std::uint64_t children = 0;
    std::int64_t modified = 0; // seconds since the epoch
};

enum class SortColumn { Name, Modified, Size, Kind };

// "name", "modified", "size" or "kind", as the UI says it.
SortColumn sortColumnFromName(std::string_view name);
std::string_view sortColumnName(SortColumn column);

// The entries of a folder, not yet sorted. On failure the error says why and the list is empty.
std::vector<Item> listFolder(const fs::path& folder, bool showHidden, std::error_code& error);

// Folders first, then by the column; names compare as a person would read them (file2 < file10).
void sortItems(std::vector<Item>& items, SortColumn column, bool ascending);

Kind classify(const fs::path& path, bool isDir);
std::string kindLabel(const fs::path& path, Kind kind);

// "4.8 MB", "412 KB", "512 bytes".
std::string humanSize(std::uint64_t bytes);
// "Today at 14:02", "Yesterday at 21:15", "Oct 20, 2026".
std::string humanDate(std::int64_t when, std::int64_t now);
// The text of the Size column for an item: its size, or how many entries a folder holds.
std::string sizeText(const Item& item);

// Natural order, ignoring case: file2 < file10.
bool naturalLess(std::string_view a, std::string_view b);

// A place in the sidebar. A header has no path.
struct PlaceDef {
    std::string name;
    std::string icon;
    fs::path path;
    bool header = false;
    bool ejectable = false;
};

// Favorites from the user's folders, then the devices that are mounted.
std::vector<PlaceDef> places();

// One crumb of the path bar.
struct Crumb {
    std::string name;
    fs::path path;
};
// "Home", "Pictures", "Trip 2026" for ~/Pictures/Trip 2026; "File System", "usr", "share" otherwise.
std::vector<Crumb> crumbs(const fs::path& folder);

// "~" and "~/x" become the home folder.
fs::path expandUser(std::string_view text);
fs::path homeFolder();

} // namespace files
