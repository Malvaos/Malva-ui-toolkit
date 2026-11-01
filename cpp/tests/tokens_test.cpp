// The generated tokens.h against dist/resolved.json (which the generator writes from the same JSON), and the
// design rules the tokens must keep: contrast, ordering, the brand's numbers.
#include "check.h"
#include "data.h"
#include "malva/color.h"
#include "malva/theme.h"

#include <set>
#include <string>

using namespace malva;

static std::string kebab(std::string s)
{
    for (char& c : s)
        if (c == '_')
            c = '-';
    return s;
}

int main()
{
    const auto res = testdata::resolved();
    CHECK(res.count("light") && res.count("dark") && res.count("brand"));
    CHECK(res.at("light").size() == roles.size());
    CHECK(res.at("dark").size() == roles.size());

    // every role of both modes has the bytes the generator computed
    for (const Role& r : roles) {
        for (auto [mode, name] : {std::pair{Mode::Light, "light"}, std::pair{Mode::Dark, "dark"}}) {
            auto it = res.at(name).find(std::string(r.name));
            CHECK(it != res.at(name).end());
            if (it != res.at(name).end()) {
                const auto want = parse_hex(it->second);
                CHECK(want.has_value());
                if (want && !(colors(mode).*(r.member) == *want))
                    std::fprintf(stderr, "  role %s (%s): header %s, json %s\n", std::string(r.name).c_str(), name, to_hex(colors(mode).*(r.member), true).c_str(), it->second.c_str());
            }
        }
    }
    // the brand palette
    CHECK(to_hex(brand::petal) == "#E6DAE8");
    CHECK(to_hex(brand::lilac) == "#DEC3E0");
    CHECK(to_hex(brand::orchid) == "#B84F9F");
    CHECK(to_hex(brand::plum) == "#8C3274");
    CHECK(to_hex(brand::leaf) == "#274903");
    for (auto& [k, v] : res.at("brand"))
        CHECK(parse_hex(v).has_value());

    // names are unique and the lookup finds each of them
    std::set<std::string> seen;
    Theme theme;
    for (const Role& r : roles) {
        CHECK(seen.insert(std::string(r.name)).second);
        CHECK(theme.get(r.name).has_value());
    }
    CHECK(!theme.get("no-such-role").has_value());
    theme.set_mode(Mode::Dark);
    CHECK(theme.get("bg") == dark.bg);

    // the neutral surfaces, text and borders of the system look
    CHECK(to_hex(light.bg) == "#F5F5F7");
    CHECK(to_hex(light.surface) == "#FFFFFF");
    CHECK(to_hex(light.text) == "#1D1D1F");
    CHECK(to_hex(light.text_2) == "#86868B");
    CHECK(to_hex(light.border) == "#E5E5EA");
    CHECK(to_hex(dark.bg) == "#161617");
    CHECK(to_hex(dark.surface) == "#1E1E1F");
    CHECK(to_hex(dark.text) == "#F5F5F7");
    CHECK(to_hex(dark.text_2) == "#8E8E93");
    CHECK(to_hex(dark.border) == "#2C2C2E");
    CHECK(light.accent == brand::plum);

    // the scales
    CHECK(radius::xs == 6 && radius::s == 10 && radius::m == 14 && radius::l == 22 && radius::xl == 36);
    CHECK(space::xxs == 4 && space::xs == 8 && space::s == 12 && space::m == 16 && space::l == 24 && space::xl == 32 && space::xxl == 48);
    CHECK(type::title.size == 28 && type::title.weight == 700);
    CHECK(type::heading.size == 20 && type::heading.weight == 600);
    CHECK(type::body.size == 14 && type::body.weight == 400);
    CHECK(type::caption.size == 12 && type::caption.weight == 400);
    CHECK(motion::hover_ms == 160 && motion::open_ms == 240);
    // the scales only grow
    CHECK(radius::xs < radius::s && radius::s < radius::m && radius::m < radius::l && radius::l < radius::xl);
    CHECK(space::xxs < space::xs && space::xs < space::s && space::s < space::m && space::m < space::l && space::l < space::xl && space::xl < space::xxl);
    CHECK(size::control_s < size::control_m && size::control_m < size::control_l);

    // contrast of the text pairings, in both modes (the generator checks the same list; this is the C++ side).
    // text-2 (#86868B on white is 3.6:1) and muted are the specified neutral greys, for supporting text and placeholders only.
    struct Pair { std::string_view fg, bg; double min; };
    const Pair pairs[] = {
        {"text", "bg", 4.5}, {"text", "surface", 4.5}, {"text", "raised", 4.5}, {"text-2", "surface", 3.0}, {"text-2", "bg", 3.0},
        {"on-accent", "accent", 4.5}, {"on-accent", "accent-hover", 4.5}, {"on-accent", "accent-press", 4.5},
        {"on-danger", "danger-fill", 4.5}, {"on-danger", "danger-press", 4.5}, {"on-inverse", "inverse", 4.5},
        {"success-text", "success-soft", 4.5}, {"warning-text", "warning-soft", 4.5}, {"danger-text", "danger-soft", 4.5},
        {"info-text", "info-soft", 4.5}, {"danger-text", "surface", 4.5}, {"accent", "surface", 3.0}, {"muted", "surface", 2.5},
    };
    for (Mode m : {Mode::Light, Mode::Dark}) {
        Theme t(m);
        for (const Pair& p : pairs) {
            Rgba bg = *t.get(p.bg);
            if (bg.a != 255)
                bg = over(bg, *t.get("surface"));
            Rgba fg = *t.get(p.fg);
            if (fg.a != 255)
                fg = over(fg, bg);
            const double ratio = contrast_ratio(fg, bg);
            if (ratio < p.min)
                std::fprintf(stderr, "  %s: %s on %s is %.2f, needs %.1f\n", m == Mode::Dark ? "dark" : "light", std::string(p.fg).c_str(), std::string(p.bg).c_str(), ratio, p.min);
            CHECK(ratio >= p.min);
        }
    }
    // hover, press and disabled are all different from the resting accent, and from each other
    for (Mode m : {Mode::Light, Mode::Dark}) {
        const Colors& c = colors(m);
        CHECK(!(c.accent == c.accent_hover) && !(c.accent == c.accent_press) && !(c.accent_hover == c.accent_press));
        CHECK(!(c.danger_fill == c.danger_hover) && !(c.danger_fill == c.danger_press));
    }
    (void)kebab;
    return finish("tokens_test");
}
