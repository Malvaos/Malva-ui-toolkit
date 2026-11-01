//! Text and icon colours: `text("Caption").style(malva_iced::text::muted)`, `svg(icon).style(malva_iced::text::icon)`.
use iced::widget::{svg, text};
use iced::Theme;

use crate::{color, colors};

fn tint(theme: &Theme, pick: impl Fn(&malva_tokens::Colors) -> malva_color::Rgba) -> text::Style {
    text::Style { color: Some(color(pick(colors(theme)))) }
}

/// Primary text (the default; useful to reset an inherited colour).
pub fn primary(theme: &Theme) -> text::Style {
    tint(theme, |c| c.text)
}
/// Secondary text: descriptions, subtitles.
pub fn secondary(theme: &Theme) -> text::Style {
    tint(theme, |c| c.text_2)
}
/// Placeholders, captions, hints.
pub fn muted(theme: &Theme) -> text::Style {
    tint(theme, |c| c.muted)
}
pub fn disabled(theme: &Theme) -> text::Style {
    tint(theme, |c| c.disabled_text)
}
pub fn accent(theme: &Theme) -> text::Style {
    tint(theme, |c| c.accent)
}
pub fn on_accent(theme: &Theme) -> text::Style {
    tint(theme, |c| c.on_accent)
}
/// Error text.
pub fn danger(theme: &Theme) -> text::Style {
    tint(theme, |c| c.danger_text)
}
pub fn success(theme: &Theme) -> text::Style {
    tint(theme, |c| c.success_text)
}
pub fn warning(theme: &Theme) -> text::Style {
    tint(theme, |c| c.warning_text)
}
pub fn info(theme: &Theme) -> text::Style {
    tint(theme, |c| c.info_text)
}

/// A one-colour icon (from `malva-icons`) drawn in the secondary text colour; hovering brings it to full strength.
pub fn icon(theme: &Theme, status: svg::Status) -> svg::Style {
    let c = colors(theme);
    svg::Style { color: Some(color(if status == svg::Status::Hovered { c.text } else { c.text_2 })) }
}

/// An icon in the accent colour.
pub fn icon_accent(theme: &Theme, _status: svg::Status) -> svg::Style {
    svg::Style { color: Some(color(colors(theme).accent)) }
}

/// An icon on an accent fill.
pub fn icon_on_accent(theme: &Theme, _status: svg::Status) -> svg::Style {
    svg::Style { color: Some(color(colors(theme).on_accent)) }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{dark, light};

    #[test]
    fn text_colours() {
        assert_eq!(secondary(&light()).color, Some(color(malva_tokens::LIGHT.text_2)));
        assert_eq!(danger(&dark()).color, Some(color(malva_tokens::DARK.danger_text)));
        assert_eq!(muted(&dark()).color, Some(color(malva_tokens::DARK.muted)));
    }

    #[test]
    fn icons_brighten_on_hover() {
        let idle = icon(&light(), svg::Status::Idle).color.unwrap();
        let hot = icon(&light(), svg::Status::Hovered).color.unwrap();
        assert_ne!(idle, hot);
        assert_eq!(hot, color(malva_tokens::LIGHT.text));
    }
}
