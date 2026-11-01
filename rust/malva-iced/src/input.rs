//! Text input and text editor: a 42 px field, radius 10, a hairline that darkens on hover and turns into a 2 px
//! accent ring when focused.
use iced::widget::{text_editor, text_input};
use iced::{Color, Theme};

use crate::{background, color, colors, outline};
use malva_color::Rgba;
use malva_tokens::radius;

struct Look {
    fill: Rgba,
    line: Color,
    width: f32,
    value: Rgba,
    placeholder: Rgba,
    icon: Rgba,
}

fn look(theme: &Theme, state: State) -> Look {
    let c = colors(theme);
    match state {
        State::Active => Look { fill: c.field, line: color(c.field_border), width: 1.0, value: c.text, placeholder: c.muted, icon: c.text_2 },
        State::Hovered => Look { fill: c.field, line: color(c.text_2.with_alpha(55)), width: 1.0, value: c.text, placeholder: c.muted, icon: c.text_2 },
        State::Focused => Look { fill: c.field, line: color(c.accent), width: 2.0, value: c.text, placeholder: c.muted, icon: c.accent },
        State::Disabled => Look { fill: c.field, line: color(c.line), width: 1.0, value: c.disabled_text, placeholder: c.disabled_text, icon: c.disabled_text },
    }
}

#[derive(Clone, Copy)]
enum State {
    Active,
    Hovered,
    Focused,
    Disabled,
}

/// `text_input(..).style(malva_iced::input::text_input)`
pub fn text_input(theme: &Theme, status: text_input::Status) -> text_input::Style {
    let c = colors(theme);
    let state = match status {
        text_input::Status::Active => State::Active,
        text_input::Status::Hovered => State::Hovered,
        text_input::Status::Focused { .. } => State::Focused,
        text_input::Status::Disabled => State::Disabled,
    };
    let l = look(theme, state);
    text_input::Style {
        background: background(l.fill),
        border: outline(l.line, l.width, radius::S),
        icon: color(l.icon),
        placeholder: color(l.placeholder),
        value: color(l.value),
        selection: color(c.accent.with_alpha(30)),
    }
}

/// A search field: the same, with the tinted search-fill and no hairline until focused.
pub fn search(theme: &Theme, status: text_input::Status) -> text_input::Style {
    let c = colors(theme);
    let mut s = text_input(theme, status);
    if matches!(status, text_input::Status::Active | text_input::Status::Hovered) {
        s.background = background(c.search_fill);
        s.border = outline(Color::TRANSPARENT, 0.0, radius::S);
    }
    s
}

/// A text input that shows an error: a danger outline in every state.
pub fn text_input_error(theme: &Theme, status: text_input::Status) -> text_input::Style {
    let c = colors(theme);
    let mut s = text_input(theme, status);
    if !matches!(status, text_input::Status::Disabled) {
        s.border = outline(color(c.danger), 2.0, radius::S);
    }
    s
}

/// `text_editor(..).style(malva_iced::input::text_editor)`
pub fn text_editor(theme: &Theme, status: text_editor::Status) -> text_editor::Style {
    let c = colors(theme);
    let state = match status {
        text_editor::Status::Active => State::Active,
        text_editor::Status::Hovered => State::Hovered,
        text_editor::Status::Focused { .. } => State::Focused,
        text_editor::Status::Disabled => State::Disabled,
    };
    let l = look(theme, state);
    text_editor::Style {
        background: background(l.fill),
        border: outline(l.line, l.width, radius::S),
        placeholder: color(l.placeholder),
        value: color(l.value),
        selection: color(c.accent.with_alpha(30)),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{dark, light};
    use iced::Background;

    #[test]
    fn states_of_the_border() {
        let t = light();
        let c = &malva_tokens::LIGHT;
        let active = text_input(&t, text_input::Status::Active);
        let hovered = text_input(&t, text_input::Status::Hovered);
        let focused = text_input(&t, text_input::Status::Focused { is_hovered: false });
        let disabled = text_input(&t, text_input::Status::Disabled);
        assert_eq!(active.border.color, color(c.field_border));
        assert_eq!(active.border.width, 1.0);
        assert_ne!(hovered.border.color, active.border.color);
        assert_eq!(focused.border.color, color(c.accent));
        assert_eq!(focused.border.width, 2.0);
        assert_eq!(disabled.border.color, color(c.line));
        assert_eq!(disabled.value, color(c.disabled_text));
    }

    #[test]
    fn dark_has_its_own_fill() {
        let s = text_input(&dark(), text_input::Status::Active);
        assert_eq!(s.background, Background::Color(color(malva_tokens::DARK.field)));
    }

    #[test]
    fn error_is_red_in_every_live_state() {
        let t = light();
        for st in [text_input::Status::Active, text_input::Status::Hovered, text_input::Status::Focused { is_hovered: true }] {
            assert_eq!(text_input_error(&t, st).border.color, color(malva_tokens::LIGHT.danger));
        }
        assert_ne!(text_input_error(&t, text_input::Status::Disabled).border.color, color(malva_tokens::LIGHT.danger));
    }

    #[test]
    fn search_is_borderless_at_rest() {
        let s = search(&light(), text_input::Status::Active);
        assert_eq!(s.border.width, 0.0);
        assert_eq!(search(&light(), text_input::Status::Focused { is_hovered: false }).border.width, 2.0);
    }

    #[test]
    fn editor_matches_input() {
        let t = dark();
        let e = text_editor(&t, text_editor::Status::Focused { is_hovered: false });
        let i = text_input(&t, text_input::Status::Focused { is_hovered: false });
        assert_eq!((e.border, e.background, e.value), (i.border, i.background, i.value));
    }
}
