//! Progress bar: 8 px, a Line-coloured track and an Orchid fill, fully rounded.
use iced::widget::progress_bar::Style;
use iced::{Border, Color, Theme};

use crate::{background, colors};
use malva_tokens::size;

fn bar(theme: &Theme, fill: malva_color::Rgba) -> Style {
    let c = colors(theme);
    Style {
        background: background(c.line),
        bar: background(fill),
        border: Border { color: Color::TRANSPARENT, width: 0.0, radius: (size::PROGRESS / 2.0).into() },
    }
}

/// `progress_bar(0.0..=1.0, value).style(malva_iced::progress::default)`; give the bar `.girth(8)`.
pub fn default(theme: &Theme) -> Style {
    bar(theme, colors(theme).accent_2)
}

/// A fill in the accent instead of the second accent.
pub fn accent(theme: &Theme) -> Style {
    bar(theme, colors(theme).accent)
}

/// Success, warning or danger fill (a finished download, a nearly full disk, a failed upload).
pub fn success(theme: &Theme) -> Style {
    bar(theme, colors(theme).success)
}

pub fn warning(theme: &Theme) -> Style {
    bar(theme, colors(theme).warning)
}

pub fn danger(theme: &Theme) -> Style {
    bar(theme, colors(theme).danger)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{color, light};
    use iced::Background;

    #[test]
    fn track_and_fill() {
        let s = default(&light());
        assert_eq!(s.background, Background::Color(color(malva_tokens::LIGHT.line)));
        assert_eq!(s.bar, Background::Color(color(malva_tokens::LIGHT.accent_2)));
        assert_eq!(s.border.radius, iced::border::Radius::from(4.0));
        assert_eq!(danger(&light()).bar, Background::Color(color(malva_tokens::LIGHT.danger)));
    }
}
