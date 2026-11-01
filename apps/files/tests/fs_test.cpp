// The file system layer's pure parts: ordering, sizes, dates, crumbs. No framework: a failed
// check prints and the program exits non-zero.
#include "../backend/fs.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond);          \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

using namespace files;

int main()
{
    // Natural order.
    CHECK(naturalLess("file2", "file10"));
    CHECK(!naturalLess("file10", "file2"));
    CHECK(naturalLess("Apple", "banana"));
    CHECK(naturalLess("a", "ab"));
    CHECK(!naturalLess("same", "SAME"));
    CHECK(naturalLess("img001", "img2") == true); // 1 < 2 as numbers

    // Sizes.
    CHECK(humanSize(0) == "0 bytes");
    CHECK(humanSize(1) == "1 byte");
    CHECK(humanSize(512) == "512 bytes");
    CHECK(humanSize(4000) == "4 KB");
    CHECK(humanSize(412000) == "412 KB");
    CHECK(humanSize(4800000) == "4.8 MB");
    CHECK(humanSize(88000000) == "88 MB");
    CHECK(humanSize(2500000000ull) == "2.5 GB");

    // Dates, in the local zone: build "now" and its neighbours with mktime.
    std::tm tm {};
    tm.tm_year = 126; tm.tm_mon = 9; tm.tm_mday = 26; tm.tm_hour = 15; tm.tm_min = 49; tm.tm_isdst = -1;
    const std::int64_t now = std::mktime(&tm);
    tm.tm_hour = 14; tm.tm_min = 2;
    CHECK(humanDate(std::mktime(&tm), now) == "Today at 14:02");
    tm.tm_mday = 25; tm.tm_hour = 21; tm.tm_min = 15;
    CHECK(humanDate(std::mktime(&tm), now) == "Yesterday at 21:15");
    tm.tm_mday = 20;
    CHECK(humanDate(std::mktime(&tm), now) == "Oct 20, 2026");
    tm.tm_year = 125; tm.tm_mon = 11; tm.tm_mday = 31; tm.tm_hour = 23; tm.tm_min = 0;
    CHECK(humanDate(std::mktime(&tm), now) == "Dec 31, 2025");
    CHECK(humanDate(0, now).empty());

    // Kinds.
    CHECK(classify("a/b.JPG", false) == Kind::Image);
    CHECK(classify("a/b.pdf", false) == Kind::Pdf);
    CHECK(classify("a/b.tar.gz", false) == Kind::Archive);
    CHECK(classify("a/noext", false) == Kind::File);
    CHECK(classify("a/dir.jpg", true) == Kind::Folder);
    CHECK(kindLabel("x.png", Kind::Image) == "PNG image");
    CHECK(kindLabel("x.foo", Kind::File) == "FOO file");
    CHECK(kindLabel("x", Kind::File) == "File");

    // Sorting: folders first, whichever way; ties by name.
    auto make = [](const char* name, bool dir, std::uint64_t bytes, std::int64_t mod) {
        Item i;
        i.name = name; i.path = name; i.isDir = dir; i.bytes = bytes; i.modified = mod;
        i.kind = dir ? Kind::Folder : Kind::File; i.kindLabel = dir ? "Folder" : "File";
        return i;
    };
    std::vector<Item> items = {make("b10", false, 5, 1), make("b2", false, 9, 3), make("dir", true, 0, 2), make("a", false, 5, 2)};
    sortItems(items, SortColumn::Name, true);
    CHECK(items[0].name == "dir" && items[1].name == "a" && items[2].name == "b2" && items[3].name == "b10");
    sortItems(items, SortColumn::Name, false);
    CHECK(items[0].name == "dir" && items[1].name == "b10" && items[3].name == "a");
    sortItems(items, SortColumn::Size, true);
    CHECK(items[0].name == "dir" && items[1].name == "a" && items[2].name == "b10" && items[3].name == "b2");
    sortItems(items, SortColumn::Modified, false);
    CHECK(items[0].name == "dir" && items[1].name == "b2");

    // Crumbs and ~.
    setenv("HOME", "/home/someone", 1);
    CHECK(expandUser("~") == fs::path("/home/someone"));
    CHECK(expandUser("~/Pictures") == fs::path("/home/someone/Pictures"));
    CHECK(expandUser("/usr") == fs::path("/usr"));
    auto c = crumbs("/home/someone/Pictures/Trip 2026");
    CHECK(c.size() == 3 && c[0].name == "Home" && c[1].name == "Pictures" && c[2].name == "Trip 2026");
    CHECK(c[2].path == fs::path("/home/someone/Pictures/Trip 2026"));
    c = crumbs("/home/someone");
    CHECK(c.size() == 1 && c[0].name == "Home");
    c = crumbs("/usr/share");
    CHECK(c.size() == 3 && c[0].name == "File System" && c[1].path == fs::path("/usr") && c[2].path == fs::path("/usr/share"));
    c = crumbs("/");
    CHECK(c.size() == 1 && c[0].name == "File System");

    if (failures == 0)
        std::puts("fs_test: all checks passed");
    return failures == 0 ? 0 : 1;
}
