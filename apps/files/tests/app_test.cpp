// The window and the stand-in backend together, driven through the UI as a person would: clicks
// that land on real items, keys that go through the window's own key handling, double clicks,
// the search field's callbacks. It needs a Slint build that includes the testing backend (the
// SDK's does); no window appears.
#include "../backend/app.h"

#include <private/slint_tests_helpers.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>

using namespace files;
namespace st = slint::private_api::testing;

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond);          \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

static FilesWindow* ui = nullptr;

static std::vector<FileEntry> rows()
{
    std::vector<FileEntry> out;
    auto model = ui->get_entries();
    for (std::size_t i = 0; i < model->row_count(); ++i)
        out.push_back(*model->row_data(i));
    return out;
}
static std::string name(int i) { return std::string(std::string_view(rows().at(i).name)); }
static std::string status() { return std::string(std::string_view(ui->get_status())); }
static std::string folder() { return std::string(std::string_view(ui->get_folder_name())); }
static std::string selection()
{
    std::string out;
    for (const FileEntry& row : rows()) {
        if (row.selected)
            out += (out.empty() ? "" : ",") + std::string(std::string_view(row.name));
    }
    return out;
}
static std::string names()
{
    std::string out;
    for (const FileEntry& row : rows())
        out += (out.empty() ? "" : ",") + std::string(std::string_view(row.name));
    return out;
}

// Where item `i` of the first row is, in window coordinates, from the layout of a 980 px window:
// a 190 px sidebar, then columns 112 px apart, centred in the page, under a 56 px header.
static std::pair<float, float> itemPoint(int i)
{
    const int columns = ui->get_columns();
    const float page = 980 - 190;
    const float left = (page - (columns * 112 - 16)) / 2;
    return {190 + left + (i % columns) * 112 + 8 + 48, 56 + 8 + 32.0f};
}
static void click(int i)
{
    const auto [x, y] = itemPoint(i);
    st::send_mouse_click(ui, x, y);
}
static void keys(std::u8string_view sequence)
{
    st::send_keyboard_string_sequence(ui, slint::SharedString(std::string_view(reinterpret_cast<const char*>(sequence.data()), sequence.size())));
}

int main()
{
    slint::cbindgen_private::slint_testing_init_backend();

    // A folder to browse, and HOME pointing at it so "Home" is a place in it.
    char tmpl[] = "/tmp/files-ui-test-XXXXXX";
    const fs::path root = mkdtemp(tmpl);
    fs::create_directories(root / "alpha");
    fs::create_directories(root / "beta");
    for (const char* n : {"a.txt", "b.txt", "c2.txt", "c10.txt", ".hidden"})
        std::ofstream(root / n) << "x";
    std::ofstream(root / "alpha" / "inner.txt") << "x";
    setenv("HOME", root.c_str(), 1);

    Options options;
    options.start = root;
    App app(options);
    auto handle = app.window();
    ui = &*handle;
    ui->window().set_size(slint::LogicalSize({980, 640}));
    ui->show();

    // ---- What the folder shows.
    CHECK(names() == "alpha,beta,a.txt,b.txt,c2.txt,c10.txt"); // folders first; c2 before c10
    CHECK(rows()[0].kind == "folder" && rows()[2].kind == "text");
    CHECK(rows()[0].size == "1 item" && rows()[1].size == "0 items");
    CHECK(status() == "6 items");
    CHECK(ui->get_columns() == 6);
    CHECK(!ui->get_can_back() && !ui->get_can_forward());
    CHECK(ui->get_crumbs()->row_count() == 1); // Home
    CHECK(ui->get_selected_place() >= 0);

    // ---- A click on an item selects it; on the page, clears.
    click(3);
    CHECK(selection() == "b.txt");
    CHECK(status().find("6 items, 1 selected") == 0);
    st::send_mouse_click(ui, 900, 560);
    CHECK(selection().empty());

    // ---- Shift extends from the anchor, Ctrl toggles.
    click(2);
    ui->invoke_item_pressed(4, false, true);
    CHECK(selection() == "a.txt,b.txt,c2.txt");
    ui->invoke_item_pressed(1, true, false);
    CHECK(selection() == "beta,a.txt,b.txt,c2.txt");
    ui->invoke_item_pressed(3, true, false);
    CHECK(selection() == "beta,a.txt,c2.txt");
    ui->invoke_background_pressed();
    CHECK(selection().empty());

    // ---- Keys through the window's own handling.
    keys(slint::platform::key_codes::DownArrow);
    CHECK(selection() == "alpha");
    keys(slint::platform::key_codes::RightArrow);
    CHECK(selection() == "beta");
    keys(slint::platform::key_codes::DownArrow); // a row down: past the end, so the last
    CHECK(selection() == "c10.txt");
    keys(slint::platform::key_codes::Home);
    CHECK(selection() == "alpha");
    keys(slint::platform::key_codes::End);
    CHECK(selection() == "c10.txt");
    keys(slint::platform::key_codes::Escape);
    CHECK(selection().empty());

    // ---- Enter opens a folder; Backspace goes up; back and forward keep the history.
    keys(slint::platform::key_codes::Home);
    keys(slint::platform::key_codes::Return);
    CHECK(folder() == "alpha");
    CHECK(names() == "inner.txt");
    CHECK(ui->get_can_back() && !ui->get_can_forward());
    CHECK(ui->get_crumbs()->row_count() == 2);
    keys(slint::platform::key_codes::Backspace);
    CHECK(folder() == std::string("Home"));
    CHECK(names().rfind("alpha,beta", 0) == 0);
    ui->invoke_back();
    CHECK(folder() == "alpha");
    ui->invoke_back();
    CHECK(folder() == std::string("Home") && !ui->get_can_back());
    ui->invoke_forward();
    CHECK(folder() == "alpha" && ui->get_can_forward());
    ui->invoke_crumb_clicked(0); // "Home"
    CHECK(folder() == std::string("Home") && !ui->get_can_forward());

    // ---- A double click on a folder opens it.
    st::mock_elapsed_time(2000);
    click(1);
    st::mock_elapsed_time(100);
    click(1);
    CHECK(folder() == "beta");
    CHECK(status().empty()); // an empty folder says nothing under it
    ui->invoke_back();

    // ---- Search narrows what shows, and ending it brings it back.
    ui->invoke_search_edited("C");
    CHECK(names() == "c2.txt,c10.txt");
    CHECK(status() == "2 of 6 matching \xE2\x80\x9C" "C\xE2\x80\x9D");
    ui->invoke_search_edited("zzz");
    CHECK(names().empty());
    CHECK(std::string(std::string_view(ui->get_empty_text())).find("No results") == 0);
    ui->invoke_search_edited("/usr"); // a path being typed is not a search
    CHECK(rows().size() == 6);
    ui->invoke_search_edited("");

    // ---- Sorting: the same column again reverses it; folders stay first.
    ui->invoke_sort_requested("name");
    CHECK(!ui->get_sort_ascending());
    CHECK(names() == "beta,alpha,c10.txt,c2.txt,b.txt,a.txt");
    ui->invoke_sort_requested("name");
    CHECK(names() == "alpha,beta,a.txt,b.txt,c2.txt,c10.txt");
    ui->invoke_sort_requested("kind");
    CHECK(std::string(std::string_view(ui->get_sort_column())) == "kind" && ui->get_sort_ascending());

    // ---- Hidden files.
    ui->invoke_key_command("toggle-hidden", false, false);
    CHECK(rows().size() == 7);
    bool sawHidden = false;
    for (const FileEntry& row : rows())
        sawHidden |= row.hidden && row.name == ".hidden";
    CHECK(sawHidden);
    ui->invoke_key_command("toggle-hidden", false, false);
    CHECK(rows().size() == 6);

    // ---- Select all, and what the status says of it.
    ui->invoke_key_command("select-all", false, false);
    CHECK(rows().size() == 6 && selection().find("alpha") == 0);
    CHECK(status().find("6 selected") != std::string::npos);
    ui->invoke_key_command("clear-selection", false, false);

    // ---- A typed path goes there.
    ui->invoke_path_accepted((root / "alpha").c_str());
    CHECK(folder() == "alpha");
    ui->invoke_path_accepted("~");
    CHECK(folder() == std::string("Home"));

    // ---- The sidebar: a place opens its folder.
    ui->invoke_place_clicked(ui->get_selected_place());
    CHECK(folder() == std::string("Home"));

    // ---- The chrome, clicked where it is: the header's buttons, a column heading, a sidebar row.
    // Header buttons, from the right of a 980 px window: list 951, grid 915, search 879; back 219,
    // forward 255; all at y 26.
    st::send_mouse_click(ui, 951, 26);
    CHECK(ui->get_view_mode() == 1);
    st::mock_elapsed_time(600); // the list fades in over 300 ms; time only moves when told to
    // The list's "Size" heading: the sort column changes to size.
    st::send_mouse_click(ui, 767, 70);
    CHECK(std::string(std::string_view(ui->get_sort_column())) == "size");
    st::send_mouse_click(ui, 915, 26);
    CHECK(ui->get_view_mode() == 0);
    st::mock_elapsed_time(600);
    ui->invoke_path_accepted((root / "alpha").c_str());
    CHECK(folder() == "alpha" && ui->get_can_back());
    st::send_mouse_click(ui, 219, 26); // back
    CHECK(folder() == "Home" && ui->get_can_forward());
    st::send_mouse_click(ui, 255, 26); // forward
    CHECK(folder() == "alpha");
    st::send_mouse_click(ui, 100, 104); // the sidebar's "Home"
    CHECK(folder() == "Home");
    // A crumb: into alpha, then the "Home" crumb at the left of the path bar.
    ui->invoke_path_accepted((root / "alpha").c_str());
    st::send_mouse_click(ui, 190 + 14 + 30 + 6 + 30 + 6 + 30, 26);
    CHECK(folder() == "Home");

    fs::remove_all(root);
    if (failures == 0)
        std::puts("app_test: all checks passed");
    return failures == 0 ? 0 : 1;
}
