//! Slider: a 6 px rail (accent filled, neutral empty), a 22 px ring thumb that grows by 2 px under the pointer.
use iced::widget::slider::{Handle, HandleShape, Rail, Status, Style};
use iced::{Border, Color, Theme};

use crate::{background, color, colors};
use malva_tokens::size;

/// `slider(0.0..=100.0, value, Message::Changed).style(malva_iced::slider::default)`; also for `vertical_slider`.
pub fn default(theme: &Theme, status: Status) -> Style {
    let c = colors(theme);
    let (ring, grow) = match status {
        Status::Active => (c.accent, 0.0),
        Status::Hovered => (c.accent, 1.0),
        Status::Dragged => (c.accent_press, 1.0),
    };
    Style {
        rail: Rail {
            backgrounds: (background(c.accent), background(c.track)),
            width: size::SLIDER_TRACK,
            border: Border { color: Color::TRANSPARENT, width: 0.0, radius: (size::SLIDER_TRACK / 2.0).into() },
        },
        handle: Handle {
            shape: HandleShape::Circle { radius: size::SLIDER_THUMB / 2.0 + grow },
            background: background(c.surface),
            border_width: 3.0,
            border_color: color(ring),
        },
    }
}

/// A slider that cannot be moved is drawn with `default`; iced has no disabled status for it, so for a disabled
/// one use this style explicitly.
pub fn disabled(theme: &Theme, _status: Status) -> Style {
    let c = colors(theme);
    let mut s = default(theme, Status::Active);
    s.rail.backgrounds = (background(c.accent_disabled), background(c.line));
    s.handle.border_color = color(c.disabled_text);
    s
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{dark, light};
    use iced::Background;

    #[test]
    fn sizes_come_from_the_tokens() {
        let s = default(&light(), Status::Active);
        assert_eq!(s.rail.width, 6.0);
        assert_eq!(s.handle.shape, HandleShape::Circle { radius: 11.0 });
        assert_eq!(s.handle.border_width, 3.0);
        let hot = default(&light(), Status::Hovered);
        assert_eq!(hot.handle.shape, HandleShape::Circle { radius: 12.0 });
    }

    #[test]
    fn rail_colours() {
        let s = default(&dark(), Status::Active);
        assert_eq!(s.rail.backgrounds.0, Background::Color(color(malva_tokens::DARK.accent)));
        assert_eq!(s.rail.backgrounds.1, Background::Color(color(malva_tokens::DARK.track)));
    }

    #[test]
    fn disabled_is_muted() {
        let s = disabled(&light(), Status::Active);
        assert_eq!(s.handle.border_color, color(malva_tokens::LIGHT.disabled_text));
    }
}
