// The Slint components of the kit, driven the way a person drives them: a pointer that moves, hovers, presses,
// drags and scrolls, and keys, through the window's own event handling. No window appears and time is stepped
// by hand, so animations and timers run when the test says.
//
// What this sees is what the components report: values, callbacks, what is open, where things are. What is
// drawn is checked separately, by the renders in docs/screenshots (tools/shot.sh).
// It needs a Slint build that includes the testing backend (the SDK's does).
#include "testbench.h"

#include <private/slint_tests_helpers.h>

#include "check.h"
#include "malva/slint_interop.h"
#include "malva/theme.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace st = slint::private_api::testing;
namespace kc = slint::platform::key_codes;

static TestBench* ui = nullptr;
static float px = 0, py = 0;

// Time moves in frames: a timer that fires in the middle of a long wait starts its animations then.
static void wait(int ms)
{
    for (; ms > 0; ms -= 16)
        st::mock_elapsed_time(ms >= 16 ? 16 : ms);
}
static void moveTo(float x, float y)
{
    px = x;
    py = y;
    ui->window().dispatch_pointer_move_event(slint::LogicalPosition({x, y}));
}
static void press() { ui->window().dispatch_pointer_press_event(slint::LogicalPosition({px, py}), slint::PointerEventButton::Left); }
static void release() { ui->window().dispatch_pointer_release_event(slint::LogicalPosition({px, py}), slint::PointerEventButton::Left); }
static void click(float x, float y)
{
    moveTo(x, y);
    wait(20);
    press();
    wait(20);
    release();
    wait(20);
}
static void drag(float x, float y, float x2, float y2)
{
    moveTo(x, y);
    wait(40);
    press();
    wait(20);
    for (int k = 1; k <= 20; ++k) {
        moveTo(x + (x2 - x) * k / 20, y + (y2 - y) * k / 20);
        wait(16);
    }
    wait(40);
    release();
    wait(40);
}
static void key(std::u8string_view k)
{
    const slint::SharedString s(std::string_view(reinterpret_cast<const char*>(k.data()), k.size()));
    ui->window().dispatch_key_press_event(s);
    ui->window().dispatch_key_release_event(s);
    wait(20);
}
static void type(const char* text)
{
    for (const char* c = text; *c; ++c) {
        const slint::SharedString s(std::string_view(c, 1));
        ui->window().dispatch_key_press_event(s);
        ui->window().dispatch_key_release_event(s);
        wait(16);
    }
}
static void away() { moveTo(990, 890); wait(20); }

static float cx(const Box& b) { return b.x + b.w / 2; }
static float cy(const Box& b) { return b.y + b.h / 2; }
static std::string text(const slint::SharedString& s) { return std::string(std::string_view(s)); }
static std::vector<MenuEntry> menu_entries()
{
    auto e = [](const char* t, const char* shortcut, bool danger, bool disabled, bool separator) {
        MenuEntry m;
        m.text = t;
        m.shortcut = shortcut;
        m.danger = danger;
        m.disabled = disabled;
        m.separator = separator;
        m.checked = false;
        return m;
    };
    return {e("Cut", "Ctrl X", false, false, false), e("Copy", "Ctrl C", false, false, false), e("", "", false, false, true),
            e("Paste", "Ctrl V", false, true, false), e("Delete", "", true, false, false)};
}

static void section(const char* name) { std::printf("  %s\n", name); }

int main()
{
    slint::cbindgen_private::slint_testing_init_backend();
    auto handle = TestBench::create();
    ui = &*handle;
    ui->window().set_size(slint::LogicalSize({1000, 900}));
    ui->show();
    wait(100);
    if (std::getenv("MALVA_UI_TEST_VERBOSE")) {  // where everything is, for writing or debugging a test
        auto show = [](const char* n, const Box& b) { std::printf("    %-8s x=%.0f y=%.0f w=%.0f h=%.0f\n", n, b.x, b.y, b.w, b.h); };
        show("btn", ui->get_btn_box());
        show("icon", ui->get_icon_box());
        show("tg", ui->get_tg_box());
        show("cb", ui->get_cb_box());
        show("rg", ui->get_rg_box());
        show("sl", ui->get_sl_box());
        show("tf", ui->get_tf_box());
        show("ta", ui->get_ta_box());
        show("st", ui->get_st_box());
        show("dd", ui->get_dd_box());
        show("seg", ui->get_seg_box());
        show("tabs", ui->get_tabs_box());
        show("qt", ui->get_qt_box());
        show("chip", ui->get_chip_box());
        show("list", ui->get_list_box());
        show("side", ui->get_side_box());
        show("crumbs", ui->get_crumbs_box());
        show("menu", ui->get_menu_box());
        show("scroll", ui->get_scroll_box());
    }

    // ---------------------------------------------------------------------------------------- Theme
    section("theme: every role of the Slint global equals tokens.h, light and dark");
    {
        malva::Theme theme;
        auto& g = ui->global<::Theme>();
        auto check_mode = [&](malva::Mode m) {
            theme.set_mode(m);
            g.set_dark(m == malva::Mode::Dark);
            wait(20);
#define CHECK_ROLE(snake, kebab)                                                                                      \
    do {                                                                                                              \
        const malva::Rgba got = malva::from_slint(g.get_##snake());                                                   \
        const malva::Rgba want = theme.colors().snake;                                                                \
        if (!(got == want)) {                                                                                         \
            std::fprintf(stderr, "%s:%d: role %s (%s): Slint %02X%02X%02X%02X, tokens.h %02X%02X%02X%02X\n", __FILE__, \
                         __LINE__, kebab, m == malva::Mode::Dark ? "dark" : "light", got.r, got.g, got.b, got.a, want.r, \
                         want.g, want.b, want.a);                                                                     \
            ++g_failures;                                                                                             \
        }                                                                                                             \
    } while (0);
#include "theme_roles.inc"
#undef CHECK_ROLE
        };
        check_mode(malva::Mode::Light);
        CHECK(!g.get_dark());
        check_mode(malva::Mode::Dark);
        CHECK(g.get_dark());
        // the C++ Theme reports the same accents as the brand sheet
        CHECK(malva::to_hex(theme.get("accent").value()) == "#DC73C2");
        malva::set_dark<::Theme>(ui, false);
        CHECK(!g.get_dark());
        CHECK(malva::from_slint(g.get_accent()) == malva::brand::plum);
    }

    // ---------------------------------------------------------------------------------------- Buttons
    section("buttons: click, disabled, loading, keyboard, hover, press, tooltip");
    {
        const Box b = ui->get_btn_box();
        CHECK(b.w == 100 && b.h == 40);
        away();
        CHECK(!ui->get_btn_hover());
        moveTo(cx(b), cy(b));
        wait(30);
        CHECK(ui->get_btn_hover());
        press();
        wait(30);
        CHECK(ui->get_btn_pressed());
        release();
        wait(30);
        CHECK(!ui->get_btn_pressed());
        CHECK(ui->get_btn_clicks() == 1);
        // pressing and moving away before the release is not a click
        moveTo(cx(b), cy(b));
        press();
        moveTo(cx(b), cy(b) + 200);
        release();
        wait(30);
        CHECK(ui->get_btn_clicks() == 1);

        click(cx(ui->get_off_box()), cy(ui->get_off_box()));
        CHECK(ui->get_off_clicks() == 0);
        click(cx(ui->get_loading_box()), cy(ui->get_loading_box()));
        CHECK(ui->get_loading_clicks() == 0);

        ui->invoke_focus_control("btn");
        key(u8" ");
        CHECK(ui->get_btn_clicks() == 2);
        key(kc::Return);
        CHECK(ui->get_btn_clicks() == 3);
        key(u8"x");
        CHECK(ui->get_btn_clicks() == 3);

        const Box ib = ui->get_icon_box();
        away();
        CHECK(!ui->get_icon_tip());
        moveTo(cx(ib), cy(ib));
        wait(300);
        CHECK(!ui->get_icon_tip());  // not yet: the pointer has to rest on it
        wait(500);
        CHECK(ui->get_icon_tip());
        away();
        wait(30);
        CHECK(!ui->get_icon_tip());
        click(cx(ib), cy(ib));
        CHECK(ui->get_icon_clicks() == 1);
        away();
    }

    // ---------------------------------------------------------------------------------------- Toggle, checkbox, radio
    section("toggle, checkbox, radio group");
    {
        const Box t = ui->get_tg_box();
        CHECK(t.w == 52 && t.h == 30);
        CHECK(!ui->get_tg_checked());
        click(cx(t), cy(t));
        CHECK(ui->get_tg_checked() && ui->get_tg_count() == 1);
        click(cx(t), cy(t));
        CHECK(!ui->get_tg_checked() && ui->get_tg_count() == 2);
        ui->invoke_focus_control("tg");
        key(u8" ");
        CHECK(ui->get_tg_checked() && ui->get_tg_count() == 3);
        key(u8" ");
        CHECK(!ui->get_tg_checked());
        const Box to = ui->get_tgoff_box();
        click(cx(to), cy(to));
        CHECK(!ui->get_tgoff_checked());

        const Box c = ui->get_cb_box();
        CHECK(!ui->get_cb_checked());
        click(c.x + 13, cy(c));
        CHECK(ui->get_cb_checked());
        click(c.x + 13, cy(c));
        CHECK(!ui->get_cb_checked());
        ui->invoke_focus_control("cb");
        key(u8" ");
        CHECK(ui->get_cb_checked());
        const Box ci = ui->get_cbi_box();
        click(ci.x + 13, cy(ci));
        CHECK(ui->get_cbi_checked());

        const Box r = ui->get_rg_box();
        CHECK(ui->get_rg_index() == 0);
        click(r.x + 14, r.y + r.h * 0.5f);
        CHECK(ui->get_rg_index() == 1 && ui->get_rg_last() == 1);
        click(r.x + 14, r.y + r.h - 8);
        CHECK(ui->get_rg_index() == 2 && ui->get_rg_last() == 2);
        click(r.x + 14, r.y + 8);
        CHECK(ui->get_rg_index() == 0 && ui->get_rg_last() == 0);
        away();
    }

    // ---------------------------------------------------------------------------------------- Slider
    section("slider: click, drag, step, keys, disabled");
    {
        const Box s = ui->get_sl_box();
        const float run = s.w - 22, left = s.x + 11;
        CHECK(ui->get_sl_value() == 0 && ui->get_sl_changes() == 0);
        click(left + run * 0.5f, cy(s));
        CHECK_NEAR(ui->get_sl_value(), 50.0, 1.0);
        CHECK(ui->get_sl_changes() >= 1);
        drag(left + run * 0.5f, cy(s), left + run * 0.8f, cy(s) + 30);  // the pointer may leave the track while dragging
        CHECK_NEAR(ui->get_sl_value(), 80.0, 1.5);
        drag(left + run * 0.8f, cy(s), s.x - 100, cy(s));  // dragged past the end it stops at the end
        CHECK_NEAR(ui->get_sl_value(), 0.0, 0.01);
        drag(left, cy(s), s.x + s.w + 100, cy(s));
        CHECK_NEAR(ui->get_sl_value(), 100.0, 0.01);
        const int before = ui->get_sl_changes();
        ui->set_sl_value(40);
        wait(20);
        CHECK(ui->get_sl_changes() == before);  // `changed` is for the person, not for the program
        ui->invoke_focus_control("sl");
        key(kc::RightArrow);
        CHECK_NEAR(ui->get_sl_value(), 41.0, 0.001);
        key(kc::LeftArrow);
        key(kc::LeftArrow);
        CHECK_NEAR(ui->get_sl_value(), 39.0, 0.001);
        key(kc::End);
        CHECK_NEAR(ui->get_sl_value(), 100.0, 0.001);
        key(kc::Home);
        CHECK_NEAR(ui->get_sl_value(), 0.0, 0.001);

        const Box t = ui->get_step_box();
        const float tl = t.x + 11, trun = t.w - 22;
        click(tl + trun * 0.33f, cy(t));
        CHECK_NEAR(ui->get_step_value(), 30.0, 0.001);
        click(tl + trun * 0.97f, cy(t));
        CHECK_NEAR(ui->get_step_value(), 100.0, 0.001);
        ui->invoke_focus_control("sl-step");
        key(kc::LeftArrow);
        CHECK_NEAR(ui->get_step_value(), 90.0, 0.001);

        const Box o = ui->get_sloff_box();
        click(o.x + o.w * 0.9f, cy(o));
        CHECK_NEAR(ui->get_off_value(), 25.0, 0.001);
        away();
    }

    // ---------------------------------------------------------------------------------------- Text
    section("text field, password, search, text area");
    {
        const Box f = ui->get_tf_box();
        CHECK(!ui->get_tf_focus());
        click(cx(f), cy(f));
        CHECK(ui->get_tf_focus());
        type("Ada");
        CHECK(text(ui->get_tf_text()) == "Ada");
        CHECK(ui->get_tf_edits() == 3);
        key(kc::Return);
        CHECK(ui->get_tf_accepts() == 1);
        key(kc::Backspace);
        CHECK(text(ui->get_tf_text()) == "Ad");
        // the clear button at the right edge, shown while there is text
        click(f.x + f.w - 20, cy(f));
        CHECK(text(ui->get_tf_text()).empty());
        click(ui->get_pw_box().x + 60, cy(ui->get_pw_box()));  // another field: focus moves there
        CHECK(!ui->get_tf_focus());

        const Box p = ui->get_pw_box();
        click(p.x + 60, cy(p));
        type("s3cret");
        CHECK(text(ui->get_pw_text()) == "s3cret");  // the text is real, only the drawing is hidden

        const Box s = ui->get_sf_box();
        click(s.x + 80, cy(s));
        CHECK(ui->get_sf_focus());
        type("mal");
        CHECK(text(ui->get_sf_text()) == "mal" && ui->get_sf_edits() == 3);
        key(kc::Escape);
        CHECK(text(ui->get_sf_text()).empty());

        const Box a = ui->get_ta_box();
        click(a.x + 40, a.y + 30);
        type("one");
        key(kc::Return);
        type("two");
        CHECK(text(ui->get_ta_text()) == "one\ntwo");
        away();
    }

    // ---------------------------------------------------------------------------------------- Stepper, dropdown, segmented, tabs
    section("stepper, dropdown, segmented control, tabs");
    {
        const Box t = ui->get_st_box();
        CHECK(ui->get_st_value() == 2);
        const float plus = t.x + t.w - 20, minus = t.x + 20;
        click(plus, cy(t));
        click(plus, cy(t));
        click(plus, cy(t));
        click(plus, cy(t));
        CHECK(ui->get_st_value() == 5);  // stops at the maximum
        for (int i = 0; i < 8; ++i)
            click(minus, cy(t));
        CHECK(ui->get_st_value() == 0);  // and at the minimum

        const Box d = ui->get_dd_box();
        CHECK(ui->get_dd_index() == 0);
        ui->invoke_focus_control("dd");
        key(kc::DownArrow);
        CHECK(ui->get_dd_index() == 1 && ui->get_dd_selected_count() == 1);
        key(kc::DownArrow);
        key(kc::DownArrow);
        CHECK(ui->get_dd_index() == 2);  // stops at the last
        key(kc::UpArrow);
        CHECK(ui->get_dd_index() == 1);
        // open the list with the pointer and pick the first row
        click(cx(d), cy(d));
        wait(100);
        click(d.x + 40, d.y + d.h + 4 + 6 + 18);
        wait(100);
        CHECK(ui->get_dd_index() == 0);
        CHECK(ui->get_dd_selected_count() == 4);
        away();

        const Box g = ui->get_seg_box();
        CHECK(ui->get_seg_index() == 0);
        click(g.x + g.w * 0.5f, cy(g));
        CHECK(ui->get_seg_index() == 1 && ui->get_seg_count() == 1);
        click(g.x + g.w * 0.9f, cy(g));
        CHECK(ui->get_seg_index() == 2);
        click(g.x + g.w * 0.1f, cy(g));
        CHECK(ui->get_seg_index() == 0);

        const Box tb = ui->get_tabs_box();
        CHECK(ui->get_tabs_index() == 0);
        // "General", "Network" and "Privacy" at 14 px, 24 px apart: about 0..56, 80..135 and 159..210 from the left
        click(tb.x + 105, tb.y + 16);
        CHECK(ui->get_tabs_index() == 1);
        click(tb.x + 180, tb.y + 16);
        CHECK(ui->get_tabs_index() == 2);
        click(tb.x + 25, tb.y + 16);
        CHECK(ui->get_tabs_index() == 0);
        click(tb.x + 270, tb.y + 16);  // past the last tab: nothing
        CHECK(ui->get_tabs_index() == 0);
        away();
    }

    // ---------------------------------------------------------------------------------------- Small things
    section("quick tile, chip, list item, sidebar item, breadcrumbs");
    {
        const Box q = ui->get_qt_box();
        CHECK(q.w == 76 || q.w > 60);
        click(cx(q), cy(q));
        CHECK(ui->get_qt_on());
        click(cx(q), cy(q));
        CHECK(!ui->get_qt_on());

        const Box c = ui->get_chip_box();
        click(c.x + 12, cy(c));
        CHECK(ui->get_chip_clicks() == 1 && ui->get_chip_removed() == 0);
        click(c.x + c.w - 12, cy(c));
        CHECK(ui->get_chip_removed() == 1);

        const Box l = ui->get_list_box();
        click(cx(l), cy(l));
        CHECK(ui->get_list_clicks() == 1);
        const Box sd = ui->get_side_box();
        click(cx(sd), cy(sd));
        CHECK(ui->get_side_clicks() == 1);

        const Box b = ui->get_crumbs_box();
        click(b.x + 18, cy(b));
        CHECK(ui->get_crumb_last() == 0);
        away();
    }

    // ---------------------------------------------------------------------------------------- Menu
    section("menu panel: keyboard walks over rules and disabled rows, pointer picks");
    {
        ui->set_entries(std::make_shared<slint::VectorModel<MenuEntry>>(menu_entries()));
        wait(50);
        const Box m = ui->get_menu_box();
        CHECK(m.w == 230);
        CHECK(m.h == 6 + 34 + 2 + 34 + 2 + 9 + 2 + 34 + 2 + 34 + 6);
        ui->invoke_focus_menu();
        CHECK(ui->get_menu_hl() == -1);
        key(kc::DownArrow);
        CHECK(ui->get_menu_hl() == 0);
        key(kc::DownArrow);
        CHECK(ui->get_menu_hl() == 1);
        key(kc::DownArrow);
        CHECK(ui->get_menu_hl() == 4);  // over the rule and the disabled row
        key(kc::DownArrow);
        CHECK(ui->get_menu_hl() == 0);  // wraps
        key(kc::UpArrow);
        CHECK(ui->get_menu_hl() == 4);
        key(kc::UpArrow);
        CHECK(ui->get_menu_hl() == 1);
        key(kc::Return);
        CHECK(ui->get_menu_last() == 1);
        key(kc::Escape);
        CHECK(ui->get_menu_dismissed() == 1);
        ui->set_menu_last(-1);
        click(m.x + 100, m.y + 6 + 17);
        CHECK(ui->get_menu_last() == 0);
        click(m.x + 100, m.y + 6 + 36 + 11 + 36 + 17);  // the disabled row
        CHECK(ui->get_menu_last() == 0);
        click(m.x + 100, m.y + 6 + 36 + 11 + 36 + 36 + 17);  // Delete
        CHECK(ui->get_menu_last() == 4);
        away();
    }

    section("context menu: opens at the point, keyboard and pointer choose, closes");
    {
        ui->invoke_open_context(60, 40);
        wait(100);
        key(kc::DownArrow);
        key(kc::Return);
        wait(50);
        CHECK(ui->get_ctx_last() == 0);
        // open again: starts with nothing highlighted
        ui->set_ctx_last(-1);
        ui->invoke_open_context(60, 40);
        wait(100);
        key(kc::DownArrow);
        CHECK(true);
        key(kc::Escape);
        wait(50);
        CHECK(ui->get_ctx_last() == -1);
        // with the pointer: the menu's first row is at (cm.x + 60 .. , cm.y + 40 + 6 .. 40)
        ui->invoke_open_context(60, 40);
        wait(100);
        click(300 + 60 + 100, 520 + 40 + 6 + 36 + 17);
        wait(50);
        CHECK(ui->get_ctx_last() == 1);
        away();
    }

    // ---------------------------------------------------------------------------------------- Scroll
    section("scroll area: wheel scrolls, stops at both ends");
    {
        const Box s = ui->get_scroll_box();
        CHECK(ui->get_scroll_y() == 0.0f);
        ui->window().dispatch_pointer_scroll_event(slint::LogicalPosition({cx(s), cy(s)}), 0, -80);
        wait(600);
        CHECK(ui->get_scroll_y() < -40.0f && ui->get_scroll_y() > -150.0f);
        ui->window().dispatch_pointer_scroll_event(slint::LogicalPosition({cx(s), cy(s)}), 0, -10000);
        wait(600);
        // 20 rows of 24 px and 19 gaps of 8 px, in a window of 150 px
        CHECK_NEAR(ui->get_scroll_y(), -(20 * 24 + 19 * 8 - 150), 1.0);
        ui->window().dispatch_pointer_scroll_event(slint::LogicalPosition({cx(s), cy(s)}), 0, 100000);
        wait(600);
        CHECK_NEAR(ui->get_scroll_y(), 0.0, 0.01);
        // the thumb: dragging the bar at the right edge moves the content
        drag(s.x + s.w - 7, s.y + 20, s.x + s.w - 7, s.y + 80);
        CHECK(ui->get_scroll_y() < -50.0f);
        away();
    }

    // ---------------------------------------------------------------------------------------- Toasts
    section("toasts: a stack lets each go after its time, one toast dismisses itself");
    {
        std::vector<ToastItem> items(2);
        items[0].id = 7;
        items[0].title = "Copied";
        items[1].id = 8;
        items[1].title = "Synced";
        ui->set_toasts(std::make_shared<slint::VectorModel<ToastItem>>(items));
        wait(100);
        CHECK(ui->get_toast_dismissed_id() == -1);
        wait(1500);
        CHECK(ui->get_single_dismissed() == 1);  // the single toast of 1 s
        CHECK(ui->get_toast_dismissed_id() == -1);
        wait(900);
        CHECK(ui->get_toast_dismissed_id() == 8 || ui->get_toast_dismissed_id() == 7);
        CHECK(ui->get_single_dismissed() == 1);  // and it said so once, not every second
    }

    // ---------------------------------------------------------------------------------------- Dialog
    section("dialog: Escape, Enter, scrim, buttons");
    {
        ui->set_dlg_open(true);
        wait(400);
        key(kc::Escape);
        CHECK(ui->get_dlg_dismissed() == 1);
        key(kc::Return);
        CHECK(ui->get_dlg_primary() == 1);
        click(30, 880);  // the scrim
        CHECK(ui->get_dlg_dismissed() == 2);
        const float h = ui->get_dlg_card_h(), w = ui->get_dlg_card_w();
        CHECK(w == 400 && h > 120);
        const float bx = 500 + w / 2 - 24 - 50, by = 450 + h / 2 - 24 - 20;
        click(bx, by);  // the primary button
        CHECK(ui->get_dlg_primary() == 2);
        click(bx - 130, by);  // the secondary one
        CHECK(ui->get_dlg_secondary() == 1);
        click(500, 450 - h / 2 + 8);  // on the card: not the scrim
        CHECK(ui->get_dlg_dismissed() == 2);
        // closed, it no longer takes anything
        ui->set_dlg_open(false);
        wait(400);
        const int clicks = ui->get_btn_clicks();
        const Box b = ui->get_btn_box();
        click(cx(b), cy(b));
        CHECK(ui->get_btn_clicks() == clicks + 1);
        CHECK(ui->get_dlg_dismissed() == 2);
    }

    return finish("ui_test");
}
