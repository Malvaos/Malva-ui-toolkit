#include "fs.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#include <unordered_map>

namespace files {

namespace {

std::string lower(std::string_view text)
{
    std::string out(text);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return out;
}

std::string extensionOf(const fs::path& path)
{
    std::string ext = path.extension().string();
    if (!ext.empty() && ext.front() == '.')
        ext.erase(0, 1);
    return lower(ext);
}

bool contains(std::initializer_list<std::string_view> list, std::string_view value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

} // namespace

std::string_view kindName(Kind kind)
{
    switch (kind) {
    case Kind::Folder: return "folder";
    case Kind::Image: return "image";
    case Kind::Pdf: return "pdf";
    case Kind::Text: return "text";
    case Kind::Archive: return "archive";
    case Kind::Audio: return "audio";
    case Kind::Video: return "video";
    case Kind::File: break;
    }
    return "file";
}

SortColumn sortColumnFromName(std::string_view name)
{
    if (name == "modified")
        return SortColumn::Modified;
    if (name == "size")
        return SortColumn::Size;
    if (name == "kind")
        return SortColumn::Kind;
    return SortColumn::Name;
}

std::string_view sortColumnName(SortColumn column)
{
    switch (column) {
    case SortColumn::Modified: return "modified";
    case SortColumn::Size: return "size";
    case SortColumn::Kind: return "kind";
    case SortColumn::Name: break;
    }
    return "name";
}

Kind classify(const fs::path& path, bool isDir)
{
    if (isDir)
        return Kind::Folder;
    const std::string ext = extensionOf(path);
    if (contains({"jpg", "jpeg", "png", "gif", "bmp", "svg", "webp", "tif", "tiff", "ico", "heic", "avif"}, ext))
        return Kind::Image;
    if (ext == "pdf")
        return Kind::Pdf;
    if (contains({"txt", "md", "log", "csv", "json", "xml", "yml", "yaml", "ini", "conf", "cfg", "toml", "c", "cc", "cpp",
                  "h", "hpp", "py", "js", "ts", "rs", "go", "java", "sh", "html", "css", "qml", "slint"},
                 ext))
        return Kind::Text;
    if (contains({"zip", "tar", "gz", "tgz", "xz", "bz2", "7z", "rar", "zst", "deb", "rpm"}, ext))
        return Kind::Archive;
    if (contains({"mp3", "flac", "wav", "ogg", "opus", "m4a", "aac"}, ext))
        return Kind::Audio;
    if (contains({"mp4", "mkv", "webm", "avi", "mov", "wmv", "m4v"}, ext))
        return Kind::Video;
    return Kind::File;
}

std::string kindLabel(const fs::path& path, Kind kind)
{
    static const std::unordered_map<std::string, std::string> known = {
        {"jpg", "JPEG image"}, {"jpeg", "JPEG image"}, {"png", "PNG image"},   {"gif", "GIF image"},
        {"svg", "SVG image"},  {"webp", "WebP image"}, {"bmp", "Bitmap image"}, {"pdf", "PDF document"},
        {"txt", "Plain text"}, {"md", "Markdown"},     {"zip", "ZIP archive"}, {"gz", "Gzip archive"},
        {"tar", "Tar archive"}, {"7z", "7-Zip archive"}, {"mp3", "MP3 audio"}, {"flac", "FLAC audio"},
        {"wav", "WAV audio"},  {"ogg", "Ogg audio"},   {"m4a", "MPEG-4 audio"}, {"mp4", "MPEG-4 video"},
        {"mkv", "Matroska video"}, {"webm", "WebM video"}, {"mov", "QuickTime video"},
    };
    if (kind == Kind::Folder)
        return "Folder";
    const std::string ext = extensionOf(path);
    if (const auto it = known.find(ext); it != known.end())
        return it->second;
    if (ext.empty())
        return "File";
    std::string upper = ext;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) { return char(std::toupper(c)); });
    return upper + " file";
}

std::vector<Item> listFolder(const fs::path& folder, bool showHidden, std::error_code& error)
{
    std::vector<Item> items;
    fs::directory_iterator it(folder, fs::directory_options::skip_permission_denied, error);
    if (error)
        return items;

    for (const fs::directory_entry& entry : it) {
        Item item;
        item.path = entry.path();
        item.name = item.path.filename().string();
        item.hidden = !item.name.empty() && item.name.front() == '.';
        if (item.hidden && !showHidden)
            continue;

        struct stat st {};
        if (::stat(item.path.c_str(), &st) == 0) {
            item.isDir = S_ISDIR(st.st_mode);
            item.bytes = item.isDir ? 0 : std::uint64_t(st.st_size);
            item.modified = std::int64_t(st.st_mtime);
        }
        item.kind = classify(item.path, item.isDir);
        item.kindLabel = kindLabel(item.path, item.kind);
        if (item.isDir) {
            // Naive on purpose: a real backend counts lazily, off the UI's thread.
            std::error_code ignored;
            for (fs::directory_iterator c(item.path, fs::directory_options::skip_permission_denied, ignored), end;
                 !ignored && c != end; c.increment(ignored)) {
                if (showHidden || c->path().filename().string().rfind('.', 0) != 0)
                    ++item.children;
            }
        }
        items.push_back(std::move(item));
    }
    return items;
}

bool naturalLess(std::string_view a, std::string_view b)
{
    std::size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        const unsigned char x = static_cast<unsigned char>(a[i]);
        const unsigned char y = static_cast<unsigned char>(b[j]);
        if (std::isdigit(x) && std::isdigit(y)) {
            // Compare the runs of digits as numbers: skip zeros, then longer is larger.
            std::size_t ie = i, je = j;
            while (ie < a.size() && std::isdigit(static_cast<unsigned char>(a[ie])))
                ++ie;
            while (je < b.size() && std::isdigit(static_cast<unsigned char>(b[je])))
                ++je;
            std::string_view na = a.substr(i, ie - i);
            std::string_view nb = b.substr(j, je - j);
            while (na.size() > 1 && na.front() == '0')
                na.remove_prefix(1);
            while (nb.size() > 1 && nb.front() == '0')
                nb.remove_prefix(1);
            if (na.size() != nb.size())
                return na.size() < nb.size();
            if (const int c = na.compare(nb); c != 0)
                return c < 0;
            i = ie;
            j = je;
            continue;
        }
        const int lx = std::tolower(x);
        const int ly = std::tolower(y);
        if (lx != ly)
            return lx < ly;
        ++i;
        ++j;
    }
    return (a.size() - i) < (b.size() - j);
}

void sortItems(std::vector<Item>& items, SortColumn column, bool ascending)
{
    std::stable_sort(items.begin(), items.end(), [&](const Item& a, const Item& b) {
        if (a.isDir != b.isDir)
            return a.isDir; // folders stay first whichever way the column runs
        int order = 0; // <0: a first
        switch (column) {
        case SortColumn::Name:
            order = naturalLess(a.name, b.name) ? -1 : (naturalLess(b.name, a.name) ? 1 : 0);
            break;
        case SortColumn::Modified:
            order = a.modified < b.modified ? -1 : (a.modified > b.modified ? 1 : 0);
            break;
        case SortColumn::Size: {
            const std::uint64_t x = a.isDir ? a.children : a.bytes;
            const std::uint64_t y = b.isDir ? b.children : b.bytes;
            order = x < y ? -1 : (x > y ? 1 : 0);
            break;
        }
        case SortColumn::Kind:
            order = a.kindLabel.compare(b.kindLabel);
            break;
        }
        if (order == 0 && column != SortColumn::Name)
            return naturalLess(a.name, b.name); // ties read in name order, whichever way it runs
        return ascending ? order < 0 : order > 0;
    });
}

std::string humanSize(std::uint64_t bytes)
{
    if (bytes < 1000)
        return std::to_string(bytes) + (bytes == 1 ? " byte" : " bytes");
    static const std::array<const char*, 5> units = {"KB", "MB", "GB", "TB", "PB"};
    double value = double(bytes);
    std::size_t unit = 0;
    do {
        value /= 1000.0;
    } while (value >= 1000.0 && ++unit + 1 < units.size());
    char buffer[32];
    const bool whole = value >= 100.0 || value == double(std::uint64_t(value));
    std::snprintf(buffer, sizeof buffer, whole ? "%.0f %s" : "%.1f %s", value, units[unit]);
    return buffer;
}

std::string humanDate(std::int64_t when, std::int64_t now)
{
    if (when <= 0)
        return "";
    std::time_t t = std::time_t(when), n = std::time_t(now);
    std::tm tm {}, tn {};
    localtime_r(&t, &tm);
    localtime_r(&n, &tn);

    char buffer[48];
    const int days = (tn.tm_year - tm.tm_year) * 366 + (tn.tm_yday - tm.tm_yday);
    const bool sameYear = tn.tm_year == tm.tm_year;
    if (sameYear && days == 0) {
        std::strftime(buffer, sizeof buffer, "Today at %H:%M", &tm);
    } else if ((sameYear && days == 1) || (!sameYear && tn.tm_yday == 0 && tm.tm_mon == 11 && tm.tm_mday == 31)) {
        std::strftime(buffer, sizeof buffer, "Yesterday at %H:%M", &tm);
    } else {
        std::strftime(buffer, sizeof buffer, "%b %-d, %Y", &tm);
    }
    return buffer;
}

std::string sizeText(const Item& item)
{
    if (item.isDir)
        return std::to_string(item.children) + (item.children == 1 ? " item" : " items");
    return humanSize(item.bytes);
}

fs::path homeFolder()
{
    if (const char* home = std::getenv("HOME"); home && *home)
        return fs::path(home);
    return fs::path("/");
}

fs::path expandUser(std::string_view text)
{
    if (text == "~")
        return homeFolder();
    if (text.rfind("~/", 0) == 0)
        return homeFolder() / std::string(text.substr(2));
    return fs::path(std::string(text));
}

namespace {

// ~/.config/user-dirs.dirs: XDG_PICTURES_DIR="$HOME/Pictures"
std::unordered_map<std::string, fs::path> userDirs()
{
    std::unordered_map<std::string, fs::path> dirs;
    const char* config = std::getenv("XDG_CONFIG_HOME");
    const fs::path file = (config && *config ? fs::path(config) : homeFolder() / ".config") / "user-dirs.dirs";
    std::ifstream in(file);
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("XDG_", 0) != 0)
            continue;
        const auto eq = line.find('=');
        const auto q1 = line.find('"', eq);
        const auto q2 = line.rfind('"');
        if (eq == std::string::npos || q1 == std::string::npos || q2 <= q1)
            continue;
        std::string value = line.substr(q1 + 1, q2 - q1 - 1);
        if (value.rfind("$HOME", 0) == 0)
            value = homeFolder().string() + value.substr(5);
        dirs[line.substr(0, eq)] = value;
    }
    return dirs;
}

} // namespace

std::vector<PlaceDef> places()
{
    const auto dirs = userDirs();
    const fs::path home = homeFolder();
    auto userDir = [&](const char* key, const char* fallback) {
        const auto it = dirs.find(key);
        return it != dirs.end() ? it->second : home / fallback;
    };

    std::vector<PlaceDef> out;
    out.push_back({"Favorites", "", {}, true, false});
    out.push_back({"Home", "home", home, false, false});
    const struct {
        const char* name;
        const char* icon;
        const char* key;
        const char* fallback;
    } favorites[] = {
        {"Desktop", "desktop", "XDG_DESKTOP_DIR", "Desktop"},
        {"Documents", "documents", "XDG_DOCUMENTS_DIR", "Documents"},
        {"Downloads", "downloads", "XDG_DOWNLOAD_DIR", "Downloads"},
        {"Music", "music", "XDG_MUSIC_DIR", "Music"},
        {"Pictures", "pictures", "XDG_PICTURES_DIR", "Pictures"},
        {"Videos", "videos", "XDG_VIDEOS_DIR", "Videos"},
    };
    std::error_code ec;
    for (const auto& f : favorites) {
        const fs::path path = userDir(f.key, f.fallback);
        if (fs::is_directory(path, ec) && path != home)
            out.push_back({f.name, f.icon, path, false, false});
    }
    const fs::path trash = home / ".local/share/Trash/files";
    if (fs::is_directory(trash, ec))
        out.push_back({"Trash", "trash", trash, false, false});

    out.push_back({"Devices", "", {}, true, false});
    out.push_back({"File System", "drive", "/", false, false});

    // Removable media the desktop has mounted for this user.
    const char* user = std::getenv("USER");
    for (const fs::path& base : {fs::path("/run/media") / (user ? user : ""), fs::path("/media") / (user ? user : "")}) {
        if (!fs::is_directory(base, ec))
            continue;
        for (const auto& entry : fs::directory_iterator(base, fs::directory_options::skip_permission_denied, ec)) {
            if (entry.is_directory(ec))
                out.push_back({entry.path().filename().string(), "usb", entry.path(), false, true});
        }
    }
    return out;
}

std::vector<Crumb> crumbs(const fs::path& folder)
{
    std::vector<Crumb> out;
    const fs::path home = homeFolder();
    fs::path base;
    fs::path rest;
    const fs::path relative = folder.lexically_relative(home);
    if (!relative.empty() && *relative.begin() != "..") {
        out.push_back({"Home", home});
        base = home;
        rest = relative == "." ? fs::path() : relative;
    } else {
        out.push_back({"File System", "/"});
        base = "/";
        rest = folder.relative_path();
    }
    for (const fs::path& part : rest) {
        if (part.empty() || part == ".")
            continue;
        base /= part;
        out.push_back({part.string(), base});
    }
    return out;
}

} // namespace files
