//! The Malva typefaces: Inter for the interface, JetBrains Mono for code.
//!
//! Load the bytes with `.font(...)` on the application and make Inter the default:
//! ```ignore
//! iced::application(...)
//!     .font(malva_iced::fonts::INTER_REGULAR)
//!     .font(malva_iced::fonts::INTER_MEDIUM)
//!     .font(malva_iced::fonts::INTER_SEMIBOLD)
//!     .font(malva_iced::fonts::INTER_BOLD)
//!     .default_font(malva_iced::fonts::UI)
//! ```
//! Both are open-source fonts; their licences are in slint/fonts.
use iced::font::{Family, Weight};
use iced::Font;

pub const INTER_REGULAR: &[u8] = include_bytes!("../../../slint/fonts/Inter-Regular.otf");
pub const INTER_MEDIUM: &[u8] = include_bytes!("../../../slint/fonts/Inter-Medium.otf");
pub const INTER_SEMIBOLD: &[u8] = include_bytes!("../../../slint/fonts/Inter-SemiBold.otf");
pub const INTER_BOLD: &[u8] = include_bytes!("../../../slint/fonts/Inter-Bold.otf");
pub const MONO_REGULAR: &[u8] = include_bytes!("../../../slint/fonts/JetBrainsMono-Regular.ttf");
pub const MONO_MEDIUM: &[u8] = include_bytes!("../../../slint/fonts/JetBrainsMono-Medium.ttf");
pub const MONO_BOLD: &[u8] = include_bytes!("../../../slint/fonts/JetBrainsMono-Bold.ttf");

const fn inter(weight: Weight) -> Font {
    Font { family: Family::Name("Inter"), weight, ..Font::DEFAULT }
}

/// Inter, regular: body text.
pub const UI: Font = inter(Weight::Normal);
/// Inter, medium.
pub const UI_MEDIUM: Font = inter(Weight::Medium);
/// Inter, semibold: headings, labels, button text.
pub const UI_SEMIBOLD: Font = inter(Weight::Semibold);
/// Inter, bold: titles.
pub const UI_BOLD: Font = inter(Weight::Bold);
/// JetBrains Mono, regular.
pub const MONO: Font = Font { family: Family::Name("JetBrains Mono"), ..Font::DEFAULT };

/// A step of the type scale: size and line height in logical pixels, and the font to draw it in.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Type {
    pub size: f32,
    pub line_height: f32,
    pub font: Font,
}

pub const TITLE: Type = Type { size: 28.0, line_height: 34.0, font: UI_BOLD };
pub const HEADING: Type = Type { size: 20.0, line_height: 26.0, font: UI_SEMIBOLD };
pub const BODY: Type = Type { size: 14.0, line_height: 20.0, font: UI };
pub const BODY_STRONG: Type = Type { size: 14.0, line_height: 20.0, font: UI_SEMIBOLD };
pub const LABEL: Type = Type { size: 13.0, line_height: 18.0, font: UI_SEMIBOLD };
pub const CAPTION: Type = Type { size: 12.0, line_height: 16.0, font: UI };
pub const CODE: Type = Type { size: 13.0, line_height: 18.0, font: MONO };

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn scale_matches_the_tokens() {
        use malva_tokens::typography as t;
        for (mine, want) in [(TITLE, t::TITLE), (HEADING, t::HEADING), (BODY, t::BODY), (BODY_STRONG, t::BODY_STRONG), (LABEL, t::LABEL), (CAPTION, t::CAPTION), (CODE, t::MONO)] {
            assert_eq!((mine.size, mine.line_height), (want.size, want.line));
        }
    }

    #[test]
    fn the_font_files_are_fonts() {
        for bytes in [INTER_REGULAR, INTER_MEDIUM, INTER_SEMIBOLD, INTER_BOLD, MONO_REGULAR, MONO_MEDIUM, MONO_BOLD] {
            assert!(bytes.len() > 50_000);
            // OpenType with CFF outlines starts "OTTO"; TrueType starts 0x00010000
            assert!(bytes.starts_with(b"OTTO") || bytes.starts_with(&[0, 1, 0, 0]));
        }
    }
}
