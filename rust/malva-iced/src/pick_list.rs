//! Pick list and its menu: the closed list looks like a text field; the open menu is a raised card.
use iced::overlay::menu;
use iced::widget::pick_list::{Status, Style};
use iced::Theme;

use crate::{background, color, colors, elevation, outline};
use malva_tokens::radius;

/// `pick_list(..).style(malva_iced::pick_list::default).menu_style(malva_iced::pick_list::menu)`
pub fn default(theme: &Theme, status: Status) -> Style {
    let c = colors(theme);
    let line = match status {
        Status::Active => color(c.field_border),
        Status::Hovered => color(c.text_2.with_alpha(55)),
        Status::Opened { .. } => color(c.accent),
    };
    let width = if matches!(status, Status::Opened { .. }) { 2.0 } else { 1.0 };
    Style {
        text_color: color(c.text),
        placeholder_color: color(c.muted),
        handle_color: color(c.text_2),
        background: background(c.field),
        border: outline(line, width, radius::S),
    }
}

/// The open list.
pub fn menu(theme: &Theme) -> menu::Style {
    let c = colors(theme);
    menu::Style {
        background: background(c.raised),
        border: outline(color(c.border), 1.0, radius::M),
        text_color: color(c.text),
        selected_text_color: color(c.accent),
        selected_background: background(c.accent_wash),
        shadow: elevation(theme, 2),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{dark, light};

    #[test]
    fn closed_open_hovered() {
        let t = light();
        let c = &malva_tokens::LIGHT;
        assert_eq!(default(&t, Status::Active).border.color, color(c.field_border));
        assert_eq!(default(&t, Status::Opened { is_hovered: false }).border.color, color(c.accent));
        assert_eq!(default(&t, Status::Opened { is_hovered: false }).border.width, 2.0);
        assert_ne!(default(&t, Status::Hovered).border.color, default(&t, Status::Active).border.color);
    }

    #[test]
    fn menu_is_raised_with_a_shadow() {
        let m = menu(&dark());
        assert_eq!(m.shadow.blur_radius, 18.0);
        assert_eq!(m.selected_text_color, color(malva_tokens::DARK.accent));
    }
}
