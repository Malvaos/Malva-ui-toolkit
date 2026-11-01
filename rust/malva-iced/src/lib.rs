//! The Malva design language for [iced](https://iced.rs) 0.14.
//!
//! ```ignore
//! use iced::widget::{button, container, text};
//! use malva_iced as malva;
//!
//! iced::application(State::default, State::update, State::view)
//!     .theme(|s: &State| malva::theme(s.mode))   // a Theme for Malva light or dark
//!     .style(|_, theme| malva::app_style(theme)) // window background and default text colour
//!     .font(malva::fonts::INTER_REGULAR)
//!     .default_font(malva::fonts::UI)
//!     .run()
//! // ... and in view():
//! button("Save").style(malva::button::primary).padding([10, 18]);
//! container(content).style(malva::container::card);
//! ```
//!
//! Every function here is a plain `fn(&Theme[, Status]) -> Style`, the shape iced's `.style(...)` takes, so it
//! also works as a building block (`|t, s| Style { border: ..., ..malva::button::primary(t, s) }`).
//!
//! The colours come from [`malva_tokens`] for the mode of the `Theme` the widget is drawn with ([`mode`]), so one
//! function serves light and dark, and any iced theme that is not a Malva one is mapped by its background.
//!
//! # What has and has not been tested
//! Written against the 0.14.0 source of iced's style structs; type-checked against those structs (stub/README.md).
//! Not run: drawing, hover and focus behaviour of real widgets is not tested here.
#![forbid(unsafe_code)]

use iced::{border, Background, Border, Color, Shadow, Theme, Vector};
use malva_color::Rgba;
use malva_tokens::{self as tokens, Colors, Mode};

pub mod button;
pub mod container;
pub mod fonts;
pub mod input;
pub mod pick_list;
pub mod progress;
pub mod rule;
pub mod scroll;
pub mod slider;
pub mod text;
pub mod toggle;

/// A token colour as an iced colour.
pub fn color(c: Rgba) -> Color {
    Color::from_rgba8(c.r, c.g, c.b, c.a as f32 / 255.0)
}

/// A token colour as an iced background.
pub fn background(c: Rgba) -> Background {
    Background::Color(color(c))
}

/// The iced [`Theme`] of Malva in one mode.
pub fn theme(mode: Mode) -> Theme {
    let c = tokens::colors(mode);
    Theme::custom(
        match mode {
            Mode::Light => "Malva Light",
            Mode::Dark => "Malva Dark",
        },
        iced::theme::Palette {
            background: color(c.bg),
            text: color(c.text),
            primary: color(c.accent),
            success: color(c.success),
            warning: color(c.warning),
            danger: color(c.danger),
        },
    )
}

pub fn light() -> Theme {
    theme(Mode::Light)
}

pub fn dark() -> Theme {
    theme(Mode::Dark)
}

/// Which Malva mode a theme is drawn in: dark when its background is darker than middle grey. Malva's own themes
/// answer exactly; iced's other themes are put into the nearest mode.
pub fn mode(theme: &Theme) -> Mode {
    let bg = theme.palette().background;
    // relative luminance of the (sRGB) background, close enough to the WCAG one for a light/dark decision
    let l = 0.2126 * bg.r + 0.7152 * bg.g + 0.0722 * bg.b;
    if l < 0.5 {
        Mode::Dark
    } else {
        Mode::Light
    }
}

/// The colour roles for the mode of `theme`.
pub fn colors(theme: &Theme) -> &'static Colors {
    tokens::colors(mode(theme))
}

/// The window's own style: the background behind everything and the default text colour.
pub fn app_style(theme: &Theme) -> iced::theme::Style {
    let c = colors(theme);
    iced::theme::Style { background_color: color(c.bg), text_color: color(c.text) }
}

/// A drop shadow of elevation 1, 2 or 3 (0 gives none), in the colour and strength the tokens give for the mode.
pub fn elevation(theme: &Theme, level: usize) -> Shadow {
    if level == 0 || level > tokens::ELEVATION.len() {
        return Shadow::default();
    }
    let c = colors(theme);
    let e = tokens::ELEVATION[level - 1];
    let alpha = match mode(theme) {
        Mode::Light => e.alpha_light,
        Mode::Dark => e.alpha_dark,
    };
    Shadow {
        color: Color { a: alpha, ..color(c.shadow) },
        offset: Vector::new(0.0, e.y),
        blur_radius: e.blur,
    }
}

/// A border of `width` with rounded corners.
pub(crate) fn outline(color: Color, width: f32, radius: f32) -> Border {
    Border { color, width, radius: border::Radius::from(radius) }
}

/// A status colour family: the strong colour, its soft fill and the text on that fill.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Tone {
    Neutral,
    Accent,
    Success,
    Warning,
    Danger,
    Info,
}

impl Tone {
    /// (soft fill, text on it, strong colour) in `c`'s mode. Accent uses 14 % / 24 % of the accent like the Slint
    /// `ToneColors`.
    pub fn colors(self, c: &Colors, mode: Mode) -> (Rgba, Rgba, Rgba) {
        match self {
            Tone::Neutral => (c.line, c.text_2, c.muted),
            Tone::Accent => (c.accent.with_alpha(if mode == Mode::Dark { 24 } else { 14 }), c.accent, c.accent),
            Tone::Success => (c.success_soft, c.success_text, c.success),
            Tone::Warning => (c.warning_soft, c.warning_text, c.warning),
            Tone::Danger => (c.danger_soft, c.danger_text, c.danger),
            Tone::Info => (c.info_soft, c.info_text, c.info),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn modes_round_trip() {
        assert_eq!(mode(&theme(Mode::Light)), Mode::Light);
        assert_eq!(mode(&theme(Mode::Dark)), Mode::Dark);
        assert_eq!(mode(&Theme::Dark), Mode::Dark);
        assert_eq!(mode(&Theme::Light), Mode::Light);
    }

    #[test]
    fn palette_is_the_tokens() {
        let p = theme(Mode::Dark).palette();
        assert_eq!(p.background, color(tokens::DARK.bg));
        assert_eq!(p.primary, color(tokens::DARK.accent));
    }
}
