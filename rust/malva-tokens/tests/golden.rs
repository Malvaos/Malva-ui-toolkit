//! The generated constants against dist/resolved.json, which tools/gen_tokens.py resolves on its own: every
//! role of both modes. Also: the contrast the design promises for text on its fills.
use malva_color::Rgba;
use malva_tokens::{brand, colors, Colors, Mode};

const RESOLVED: &str = include_str!("../../../dist/resolved.json");

/// `"name": "#RRGGBBAA"` pairs of the object called `section`.
fn section(section: &str) -> Vec<(String, Rgba)> {
    let at = RESOLVED.find(&format!("\"{section}\"")).unwrap_or_else(|| panic!("no section {section}"));
    let open = RESOLVED[at..].find('{').unwrap() + at;
    let close = RESOLVED[open..].find('}').unwrap() + open;
    RESOLVED[open + 1..close]
        .lines()
        .filter_map(|l| {
            let mut parts = l.trim().trim_end_matches(',').splitn(2, ':');
            let name = parts.next()?.trim().trim_matches('"').to_string();
            let value = parts.next()?.trim().trim_matches('"').to_string();
            let color = Rgba::parse_hex(&value).unwrap_or_else(|e| panic!("{name}: {e}"));
            Some((name, color))
        })
        .collect()
}

#[test]
fn brand_colours() {
    for (name, want) in section("brand") {
        let got = match name.as_str() {
            "petal" => brand::PETAL,
            "lilac" => brand::LILAC,
            "orchid" => brand::ORCHID,
            "plum" => brand::PLUM,
            "leaf" => brand::LEAF,
            "ink" => brand::INK,
            other => panic!("unknown brand colour {other}"),
        };
        assert_eq!(got, want, "{name}");
    }
}

#[test]
fn every_role_of_both_modes_equals_the_resolved_file() {
    for (mode, name) in [(Mode::Light, "light"), (Mode::Dark, "dark")] {
        let want = section(name);
        assert_eq!(want.len(), Colors::NAMES.len(), "{name}: the file and the crate list different roles");
        for (role, value) in want {
            assert_eq!(colors(mode).get(&role), Some(value), "{name} {role}");
        }
    }
}

#[test]
fn names_are_all_known_and_unknown_is_none() {
    for n in Colors::NAMES {
        assert!(colors(Mode::Light).get(n).is_some(), "{n}");
    }
    assert_eq!(colors(Mode::Dark).get("no-such-role"), None);
}

#[test]
fn text_is_readable_on_its_fills() {
    for mode in [Mode::Light, Mode::Dark] {
        let c = colors(mode);
        // (text, fill, minimum ratio): AA for normal text is 4.5
        let pairs = [
            (c.text, c.bg, 7.0),
            (c.text, c.surface, 7.0),
            (c.text, c.panel, 7.0),
            (c.text_2, c.bg, 3.0), // the specified neutral grey (#86868B is 3.6:1 on white): supporting text only
            (c.on_accent, c.accent, 4.5),
            (c.on_accent, c.accent_hover, 4.5),
            (c.on_accent, c.accent_press, 4.5),
            (c.on_danger, c.danger_fill, 4.5),
            (c.on_danger, c.danger_hover, 3.0), // a hover state that lasts a moment; the generator allows 3.0 too
            (c.on_danger, c.danger_press, 4.5),
            (c.on_inverse, c.inverse, 7.0),
            (c.success_text, c.bg, 4.5),
            (c.warning_text, c.bg, 4.5),
            (c.danger_text, c.bg, 4.5),
            (c.info_text, c.bg, 4.5),
        ];
        for (fg, bg, min) in pairs {
            let ratio = fg.contrast_ratio(bg);
            assert!(ratio >= min, "{mode:?}: {fg} on {bg} is {ratio:.2}, needs {min}");
        }
    }
}

#[test]
fn scales() {
    use malva_tokens::{radius, size, space};
    assert_eq!((radius::XS, radius::S, radius::M, radius::L, radius::XL), (6.0, 10.0, 14.0, 22.0, 36.0));
    assert_eq!((space::XXS, space::XS, space::S, space::M, space::L, space::XL, space::XXL), (4.0, 8.0, 12.0, 16.0, 24.0, 32.0, 48.0));
    assert_eq!((size::CONTROL_S, size::CONTROL_M, size::CONTROL_L, size::FIELD_HEIGHT), (32.0, 40.0, 48.0, 42.0));
    assert_eq!((size::TOGGLE_WIDTH, size::TOGGLE_HEIGHT, size::TOGGLE_KNOB), (52.0, 30.0, 24.0));
}
