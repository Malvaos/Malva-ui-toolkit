//! Button styles: `.style(malva_iced::button::primary)`.
//!
//! Radius 10, 1 px outline only on the secondary kind; pressed buttons are a shade darker, disabled ones fade.
use iced::widget::button::{Status, Style};
use iced::{Background, Border, Color, Shadow, Theme};

use crate::{background, color, colors, outline};
use malva_tokens::radius;

fn base(text: Color, bg: Option<Background>, border: Border) -> Style {
    Style { background: bg, text_color: text, border, shadow: Shadow::default(), snap: false }
}

fn flat(text: Color, bg: Option<Background>) -> Style {
    base(text, bg, outline(Color::TRANSPARENT, 0.0, radius::S))
}

/// The main action: Plum (light) / Orchid-pink (dark), white or charcoal text.
pub fn primary(theme: &Theme, status: Status) -> Style {
    let c = colors(theme);
    let (bg, text) = match status {
        Status::Active => (c.accent, c.on_accent),
        Status::Hovered => (c.accent_hover, c.on_accent),
        Status::Pressed => (c.accent_press, c.on_accent),
        Status::Disabled => (c.accent_disabled, c.on_accent),
    };
    flat(color(text), Some(background(bg)))
}

/// A quiet button on a surface, outlined.
pub fn secondary(theme: &Theme, status: Status) -> Style {
    let c = colors(theme);
    let (bg, text, line) = match status {
        Status::Active => (c.surface, c.text, c.border),
        Status::Hovered => (c.secondary_hover, c.text, c.border),
        Status::Pressed => (c.secondary_press, c.text, c.border),
        Status::Disabled => (c.surface, c.disabled_text, c.line),
    };
    base(color(text), Some(background(bg)), outline(color(line), 1.0, radius::S))
}

/// Text in the accent colour on nothing; a wash appears under the pointer.
pub fn ghost(theme: &Theme, status: Status) -> Style {
    let c = colors(theme);
    match status {
        Status::Active => flat(color(c.accent), None),
        Status::Hovered => flat(color(c.accent), Some(background(c.ghost_hover))),
        Status::Pressed => flat(color(c.accent), Some(background(c.ghost_press))),
        Status::Disabled => flat(color(c.accent.with_alpha(40)), None),
    }
}

/// Destructive action: the red fill, white text (ink in dark).
pub fn danger(theme: &Theme, status: Status) -> Style {
    let c = colors(theme);
    let bg = match status {
        Status::Active => c.danger_fill,
        Status::Hovered => c.danger_hover,
        Status::Pressed => c.danger_press,
        Status::Disabled => c.danger_disabled,
    };
    flat(color(c.on_danger), Some(background(bg)))
}

/// A square button with one icon in it (toolbars, window controls): nothing until hovered.
pub fn icon(theme: &Theme, status: Status) -> Style {
    let c = colors(theme);
    match status {
        Status::Active => flat(color(c.text), None),
        Status::Hovered => flat(color(c.text), Some(background(c.icon_hover))),
        Status::Pressed => flat(color(c.text), Some(background(c.icon_press))),
        Status::Disabled => flat(color(c.disabled_text), None),
    }
}

/// An icon button that is toggled on (a toolbar toggle): the accent wash and accent icon.
pub fn icon_selected(theme: &Theme, status: Status) -> Style {
    let c = colors(theme);
    let bg = match status {
        Status::Pressed => c.icon_press,
        _ => c.accent_wash,
    };
    let text = if status == Status::Disabled { c.disabled_text } else { c.accent };
    flat(color(text), Some(background(bg)))
}

/// A row of a sidebar or list: the selected one is washed in the accent.
pub fn sidebar_item(selected: bool) -> impl Fn(&Theme, Status) -> Style {
    move |theme, status| {
        let c = colors(theme);
        let text = if selected { c.accent } else { c.text };
        let bg = match (selected, status) {
            (_, Status::Pressed) => Some(c.ghost_press),
            (true, _) => Some(c.accent_wash),
            (false, Status::Hovered) => Some(c.ghost_hover),
            _ => None,
        };
        let text = if status == Status::Disabled { c.disabled_text } else { text };
        flat(color(text), bg.map(background))
    }
}

/// A pill chip, outlined; `selected` washes it in the accent.
pub fn chip(selected: bool) -> impl Fn(&Theme, Status) -> Style {
    move |theme, status| {
        let c = colors(theme);
        let bg = if selected {
            c.accent_wash
        } else if status == Status::Hovered {
            c.secondary_hover
        } else {
            c.surface
        };
        let line = if selected { c.accent.with_alpha(55) } else { c.border };
        let text = if selected { c.accent } else { c.text };
        let opacity = if status == Status::Disabled { 0.5 } else { 1.0 };
        let fade = |mut col: Color| {
            col.a *= opacity;
            col
        };
        base(fade(color(text)), Some(Background::Color(fade(color(bg)))), outline(fade(color(line)), 1.0, radius::PILL))
    }
}

/// A link: accent text, underline-free; hover darkens it.
pub fn link(theme: &Theme, status: Status) -> Style {
    let c = colors(theme);
    let text = match status {
        Status::Active => c.accent,
        Status::Hovered | Status::Pressed => c.accent_hover,
        Status::Disabled => c.disabled_text,
    };
    flat(color(text), None)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{dark, light};

    fn bg(s: &Style) -> Color {
        match s.background {
            Some(Background::Color(c)) => c,
            _ => panic!("no colour background"),
        }
    }

    #[test]
    fn primary_walks_through_the_state_colours() {
        for (theme, c) in [(light(), &malva_tokens::LIGHT), (dark(), &malva_tokens::DARK)] {
            assert_eq!(bg(&primary(&theme, Status::Active)), color(c.accent));
            assert_eq!(bg(&primary(&theme, Status::Hovered)), color(c.accent_hover));
            assert_eq!(bg(&primary(&theme, Status::Pressed)), color(c.accent_press));
            assert_eq!(bg(&primary(&theme, Status::Disabled)), color(c.accent_disabled));
            assert_eq!(primary(&theme, Status::Active).text_color, color(c.on_accent));
        }
    }

    #[test]
    fn light_and_dark_differ() {
        assert_ne!(bg(&primary(&light(), Status::Active)), bg(&primary(&dark(), Status::Active)));
    }

    #[test]
    fn secondary_has_an_outline_and_the_others_do_not() {
        let t = light();
        assert_eq!(secondary(&t, Status::Active).border.width, 1.0);
        for s in [primary(&t, Status::Active), ghost(&t, Status::Active), danger(&t, Status::Active), icon(&t, Status::Active)] {
            assert_eq!(s.border.width, 0.0);
        }
    }

    #[test]
    fn ghost_and_icon_are_empty_until_hovered() {
        let t = dark();
        assert!(ghost(&t, Status::Active).background.is_none());
        assert!(ghost(&t, Status::Hovered).background.is_some());
        assert!(icon(&t, Status::Active).background.is_none());
        assert!(icon(&t, Status::Pressed).background.is_some());
    }

    #[test]
    fn sidebar_selection() {
        let t = light();
        let c = &malva_tokens::LIGHT;
        let sel = sidebar_item(true)(&t, Status::Active);
        assert_eq!(bg(&sel), color(c.accent_wash));
        assert_eq!(sel.text_color, color(c.accent));
        assert!(sidebar_item(false)(&t, Status::Active).background.is_none());
        assert_eq!(bg(&sidebar_item(false)(&t, Status::Hovered)), color(c.ghost_hover));
    }

    #[test]
    fn disabled_chip_is_faded() {
        let t = light();
        let on = chip(false)(&t, Status::Active);
        let off = chip(false)(&t, Status::Disabled);
        assert!(off.text_color.a < on.text_color.a);
    }
}
