//! The Malva icon set as SVG text.
//!
//! * [`symbol`]: one-colour symbols on a 24-unit grid (1.8 stroke, round caps and joins). Stroke and fill are
//!   `currentColor`, so [`svg_with_color`] can tint one.
//! * [`app`]: coloured application pictures.
//! * [`logo`]: the brand lock-ups (mark, wordmark, horizontal, stacked, bloom).
//!
//! ```
//! let star = malva_icons::symbol("star").unwrap();
//! assert!(star.starts_with("<svg"));
//! let plum = malva_icons::svg_with_color("star", "#8C3274").unwrap();
//! assert!(!plum.contains("currentColor") && plum.contains("#8C3274"));
//! ```
#![forbid(unsafe_code)]

mod gen;

pub use gen::{APPS, LOGOS, SYMBOLS};

fn find(table: &'static [(&'static str, &'static str)], name: &str) -> Option<&'static str> {
    table.iter().find(|(n, _)| *n == name).map(|(_, svg)| *svg)
}

/// The SVG of a one-colour symbol, by name (see `docs/ICONS.md`), or `None`.
pub fn symbol(name: &str) -> Option<&'static str> {
    find(&SYMBOLS, name)
}

/// The SVG of a coloured application picture, by name, or `None`.
pub fn app(name: &str) -> Option<&'static str> {
    find(&APPS, name)
}

/// The SVG of a brand lock-up, by name (`malva-mark`, `malva-wordmark`, ...), or `None`.
pub fn logo(name: &str) -> Option<&'static str> {
    find(&LOGOS, name)
}

/// A symbol with its `currentColor` replaced by `color` (any SVG colour: `#8C3274`, `rgb(...)`, `white`).
pub fn svg_with_color(name: &str, color: &str) -> Option<String> {
    symbol(name).map(|svg| svg.replace("currentColor", color))
}

/// The names of all symbols, in order.
pub fn symbol_names() -> impl Iterator<Item = &'static str> {
    SYMBOLS.iter().map(|(n, _)| *n)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn the_counts_of_the_set() {
        assert_eq!(SYMBOLS.len(), 130);
        assert_eq!(APPS.len(), 20);
        assert_eq!(LOGOS.len(), 11);
    }

    #[test]
    fn names_are_unique_and_sorted() {
        for table in [&SYMBOLS[..], &APPS[..], &LOGOS[..]] {
            let mut prev = "";
            for (name, _) in table {
                assert!(*name > prev, "{name} after {prev}");
                prev = name;
            }
        }
    }

    #[test]
    fn every_asset_is_a_complete_svg() {
        for (name, svg) in SYMBOLS.iter().chain(APPS.iter()).chain(LOGOS.iter()) {
            assert!(svg.starts_with("<svg"), "{name}");
            assert!(svg.trim_end().ends_with("</svg>"), "{name}");
            assert!(svg.contains("viewBox"), "{name} has no viewBox");
        }
    }

    /// `#rgb`, `#rrggbb` or `#rrggbbaa` (not an in-file reference such as `href="#petal"`).
    fn has_hex_colour(svg: &str) -> bool {
        svg.match_indices('#').any(|(i, _)| {
            let run = svg[i + 1..].chars().take_while(|c| c.is_ascii_alphanumeric()).collect::<String>();
            matches!(run.len(), 3 | 6 | 8) && run.chars().all(|c| c.is_ascii_hexdigit())
        })
    }

    #[test]
    fn symbols_take_their_colour_from_the_text() {
        for (name, svg) in SYMBOLS.iter() {
            assert!(svg.contains("currentColor"), "{name} does not use currentColor");
            assert!(!has_hex_colour(svg), "{name} has a fixed colour");
        }
    }

    #[test]
    fn lookups() {
        assert!(symbol("star").is_some());
        assert!(symbol("no-such-icon").is_none());
        assert!(app("browser").is_some());
        assert!(logo("malva-mark").is_some());
        assert!(symbol("malva-mark").is_none());
        let tinted = svg_with_color("star", "#8C3274").unwrap();
        assert!(!tinted.contains("currentColor"));
        assert!(tinted.contains("#8C3274"));
        assert!(svg_with_color("nope", "red").is_none());
        assert_eq!(symbol_names().count(), 130);
    }
}
