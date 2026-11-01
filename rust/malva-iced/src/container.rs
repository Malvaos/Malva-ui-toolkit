//! Container styles: `container(x).style(malva_iced::container::card)`.
//!
//! Surfaces follow the three layers of the design: `background_layer` (the window), `panel` (sidebars, title bars)
//! and `surface` (cards and content); floating things (`raised`, `tooltip`, `toast`, `dialog`) carry a shadow.
use iced::widget::container::Style;
use iced::{Border, Color, Shadow, Theme};

use crate::{background, color, colors, elevation, mode, outline, Tone};
use malva_tokens::radius;

fn style(text: Option<Color>, bg: Option<iced::Background>, border: Border, shadow: Shadow) -> Style {
    Style { text_color: text, background: bg, border, shadow, snap: false }
}

fn none() -> Border {
    outline(Color::TRANSPARENT, 0.0, 0.0)
}

/// The window background.
pub fn background_layer(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.bg)), none(), Shadow::default())
}

/// Sidebars and title bars.
pub fn panel(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.panel)), none(), Shadow::default())
}

/// Content areas.
pub fn surface(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.surface)), none(), Shadow::default())
}

/// A card: surface, 1 px hairline, radius 14, the lightest shadow.
pub fn card(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.surface)), outline(color(c.border), 1.0, radius::M), elevation(theme, 1))
}

/// A card with elevation 1, 2 or 3 (`level` 0 is flat, with only the hairline).
pub fn card_level(level: usize) -> impl Fn(&Theme) -> Style {
    move |theme| {
        let c = colors(theme);
        style(Some(color(c.text)), Some(background(c.surface)), outline(color(c.border), 1.0, radius::M), elevation(theme, level))
    }
}

/// Menus, popovers: raised, hairline, radius 14, elevation 2.
pub fn raised(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.raised)), outline(color(c.border), 1.0, radius::M), elevation(theme, 2))
}

/// A dialog card: raised, radius 22, elevation 3.
pub fn dialog(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.raised)), outline(color(c.border), 1.0, radius::L), elevation(theme, 3))
}

/// A toast: raised, radius 18, elevation 2.
pub fn toast(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.raised)), outline(color(c.border), 1.0, radius::M + 4.0), elevation(theme, 2))
}

/// The dark bubble of a tooltip (inverse colours: dark on light, light on dark).
pub fn tooltip(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.on_inverse)), Some(background(c.inverse)), outline(Color::TRANSPARENT, 0.0, radius::XS + 2.0), elevation(theme, 2))
}

/// The dim layer behind a modal dialog.
pub fn scrim(theme: &Theme) -> Style {
    let c = colors(theme);
    style(None, Some(background(c.scrim)), none(), Shadow::default())
}

/// Translucent bars and docks over a wallpaper: the glass fill and its inner rim.
pub fn glass(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.glass)), outline(color(c.glass_border), 1.0, radius::L), elevation(theme, 2))
}

/// A field-like box (the fill of text fields, for a custom control).
pub fn field(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.field)), outline(color(c.field_border), 1.0, radius::S), Shadow::default())
}

/// The rail of a segmented control.
pub fn segmented_rail(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.search_fill)), outline(Color::TRANSPARENT, 0.0, radius::S + 2.0), Shadow::default())
}

/// The selected segment of a segmented control: raised, lifted by a small shadow.
pub fn segmented_thumb(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.raised)), outline(Color::TRANSPARENT, 0.0, radius::S), elevation(theme, 1))
}

/// A small status pill: Synced, Low battery, Offline, New.
pub fn badge(tone: Tone) -> impl Fn(&Theme) -> Style {
    move |theme| {
        let c = colors(theme);
        let (soft, text, _) = tone.colors(c, mode(theme));
        style(Some(color(text)), Some(background(soft)), outline(Color::TRANSPARENT, 0.0, radius::PILL), Shadow::default())
    }
}

/// A banner across the top of a view: soft fill, a hairline in the strong colour.
pub fn banner(tone: Tone) -> impl Fn(&Theme) -> Style {
    move |theme| {
        let c = colors(theme);
        let (soft, text, strong) = tone.colors(c, mode(theme));
        style(Some(color(text)), Some(background(soft)), outline(color(strong.with_alpha(45)), 1.0, radius::S + 2.0), Shadow::default())
    }
}

/// The selected row of a list.
pub fn selected_row(theme: &Theme) -> Style {
    let c = colors(theme);
    style(Some(color(c.text)), Some(background(c.accent_wash)), outline(Color::TRANSPARENT, 0.0, radius::S), Shadow::default())
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{dark, light};
    use iced::Background;

    fn fill(s: &Style) -> Color {
        match s.background {
            Some(Background::Color(c)) => c,
            _ => panic!("no fill"),
        }
    }

    #[test]
    fn layers_use_their_tokens() {
        for (t, c) in [(light(), &malva_tokens::LIGHT), (dark(), &malva_tokens::DARK)] {
            assert_eq!(fill(&background_layer(&t)), color(c.bg));
            assert_eq!(fill(&panel(&t)), color(c.panel));
            assert_eq!(fill(&surface(&t)), color(c.surface));
            assert_eq!(fill(&raised(&t)), color(c.raised));
            assert_eq!(fill(&scrim(&t)), color(c.scrim));
        }
    }

    #[test]
    fn floating_things_carry_a_shadow_flat_ones_do_not() {
        let t = light();
        assert_eq!(surface(&t).shadow.blur_radius, 0.0);
        assert_eq!(card(&t).shadow.blur_radius, 3.0);
        assert_eq!(raised(&t).shadow.blur_radius, 18.0);
        assert_eq!(dialog(&t).shadow.blur_radius, 40.0);
        assert_eq!(dialog(&t).border.radius, iced::border::Radius::from(22.0));
        assert_eq!(card_level(0)(&t).shadow, Shadow::default());
    }

    #[test]
    fn shadows_are_stronger_in_dark() {
        assert!(raised(&dark()).shadow.color.a > raised(&light()).shadow.color.a);
    }

    #[test]
    fn tooltip_is_inverse() {
        assert_eq!(fill(&tooltip(&light())), color(malva_tokens::LIGHT.inverse));
        assert_eq!(tooltip(&dark()).text_color, Some(color(malva_tokens::DARK.on_inverse)));
    }

    #[test]
    fn badge_tones() {
        let t = light();
        assert_eq!(fill(&badge(Tone::Success)(&t)), color(malva_tokens::LIGHT.success_soft));
        assert_eq!(badge(Tone::Danger)(&t).text_color, Some(color(malva_tokens::LIGHT.danger_text)));
    }
}
