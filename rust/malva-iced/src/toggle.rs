//! Checkbox, radio and toggler (switch).
//!
//! Checkbox: 2 px outline, radius 7, fills with the accent when checked. Radio: 2 px ring and an accent dot.
//! Toggler: pill, accent when on and the neutral toggle-off colour when off, with a white knob.
use iced::widget::{checkbox, radio, toggler};
use iced::{Background, Color, Theme};

use crate::{background, color, colors, outline};
use malva_tokens::radius;

/// `checkbox(checked).style(malva_iced::toggle::checkbox)`
pub fn checkbox(theme: &Theme, status: checkbox::Status) -> checkbox::Style {
    let c = colors(theme);
    let (checked, hovered, disabled) = match status {
        checkbox::Status::Active { is_checked } => (is_checked, false, false),
        checkbox::Status::Hovered { is_checked } => (is_checked, true, false),
        checkbox::Status::Disabled { is_checked } => (is_checked, false, true),
    };
    let fill = match (checked, disabled, hovered) {
        (true, true, _) => c.accent_disabled,
        (true, false, true) => c.accent_hover,
        (true, false, false) => c.accent,
        (false, ..) => malva_color::Rgba::TRANSPARENT,
    };
    let line = match (checked, disabled, hovered) {
        (_, true, _) => if checked { c.accent_disabled } else { c.disabled_text },
        (true, false, true) => c.accent_hover,
        (true, false, false) => c.accent,
        (false, false, true) => c.text,
        (false, false, false) => c.text_2,
    };
    checkbox::Style {
        background: background(fill),
        icon_color: color(c.on_accent),
        border: outline(color(line), 2.0, radius::XS + 1.0),
        text_color: Some(color(if disabled { c.disabled_text } else { c.text })),
    }
}

/// `radio(..).style(malva_iced::toggle::radio)`
pub fn radio(theme: &Theme, status: radio::Status) -> radio::Style {
    let c = colors(theme);
    let (selected, hovered) = match status {
        radio::Status::Active { is_selected } => (is_selected, false),
        radio::Status::Hovered { is_selected } => (is_selected, true),
    };
    let ring = match (selected, hovered) {
        (true, true) => c.accent_hover,
        (true, false) => c.accent,
        (false, true) => c.text,
        (false, false) => c.text_2,
    };
    radio::Style {
        background: Background::Color(Color::TRANSPARENT),
        dot_color: color(if hovered && selected { c.accent_hover } else { c.accent }),
        border_width: 2.0,
        border_color: color(ring),
        text_color: Some(color(c.text)),
    }
}

/// `toggler(on).style(malva_iced::toggle::toggler)`
pub fn toggler(theme: &Theme, status: toggler::Status) -> toggler::Style {
    let c = colors(theme);
    let (on, hovered, disabled) = match status {
        toggler::Status::Active { is_toggled } => (is_toggled, false, false),
        toggler::Status::Hovered { is_toggled } => (is_toggled, true, false),
        toggler::Status::Disabled { is_toggled } => (is_toggled, false, true),
    };
    let track = match (on, disabled, hovered) {
        (true, true, _) => c.accent_disabled,
        (true, false, true) => c.accent_hover,
        (true, false, false) => c.accent,
        (false, true, _) => c.toggle_off.with_alpha(50),
        (false, false, true) => c.toggle_off.mix(c.text, 12),
        (false, false, false) => c.toggle_off,
    };
    // the knob is white; a disabled knob in dark is the text colour at half strength, as in the Slint Toggle
    let knob = if disabled && colors_are_dark(theme) { c.text.with_alpha(50) } else { malva_color::Rgba::WHITE };
    toggler::Style {
        background: background(track),
        background_border_width: 0.0,
        background_border_color: Color::TRANSPARENT,
        foreground: background(knob),
        foreground_border_width: 0.0,
        foreground_border_color: Color::TRANSPARENT,
        text_color: Some(color(if disabled { c.disabled_text } else { c.text })),
        border_radius: None, // perfectly round
        // 30 px track, 24 px knob: 3 px of room all round
        padding_ratio: 0.1,
    }
}

fn colors_are_dark(theme: &Theme) -> bool {
    crate::mode(theme) == malva_tokens::Mode::Dark
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{dark, light};

    fn fill(b: Background) -> Color {
        match b {
            Background::Color(c) => c,
            _ => panic!(),
        }
    }

    #[test]
    fn checkbox_fills_when_checked() {
        let t = light();
        let c = &malva_tokens::LIGHT;
        let off = checkbox(&t, checkbox::Status::Active { is_checked: false });
        let on = checkbox(&t, checkbox::Status::Active { is_checked: true });
        assert_eq!(fill(off.background).a, 0.0);
        assert_eq!(fill(on.background), color(c.accent));
        assert_eq!(on.border.color, color(c.accent));
        assert_eq!(off.border.color, color(c.text_2));
        assert_eq!(on.border.width, 2.0);
        let hot = checkbox(&t, checkbox::Status::Hovered { is_checked: false });
        assert_eq!(hot.border.color, color(c.text));
        let gone = checkbox(&t, checkbox::Status::Disabled { is_checked: true });
        assert_eq!(fill(gone.background), color(c.accent_disabled));
        assert_eq!(gone.text_color, Some(color(c.disabled_text)));
    }

    #[test]
    fn radio_ring_and_dot() {
        let t = dark();
        let c = &malva_tokens::DARK;
        let sel = radio(&t, radio::Status::Active { is_selected: true });
        assert_eq!(sel.border_color, color(c.accent));
        assert_eq!(sel.dot_color, color(c.accent));
        assert_eq!(sel.border_width, 2.0);
        let unsel = radio(&t, radio::Status::Hovered { is_selected: false });
        assert_eq!(unsel.border_color, color(c.text));
    }

    #[test]
    fn toggler_tracks() {
        for (t, c) in [(light(), &malva_tokens::LIGHT), (dark(), &malva_tokens::DARK)] {
            let on = toggler(&t, toggler::Status::Active { is_toggled: true });
            let off = toggler(&t, toggler::Status::Active { is_toggled: false });
            assert_eq!(fill(on.background), color(c.accent));
            assert_eq!(fill(off.background), color(c.toggle_off));
            assert_eq!(fill(on.foreground), Color::WHITE);
            assert!(on.border_radius.is_none());
            assert_eq!(on.padding_ratio, 0.1);
            let dim = toggler(&t, toggler::Status::Disabled { is_toggled: false });
            assert!(fill(dim.background).a < 1.0);
        }
    }
}
