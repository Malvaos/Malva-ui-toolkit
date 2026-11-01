//! Scrollable: an overlay scrollbar with a 6 px thumb that is quiet at rest and darkens when it is grabbed.
use iced::widget::container;
use iced::widget::scrollable::{AutoScroll, Rail, Scroller, Status, Style};
use iced::{Background, Border, Color, Theme};

use crate::{background, color, colors, elevation, outline};
use malva_tokens::radius;

fn rail(theme: &Theme, thumb: malva_color::Rgba) -> Rail {
    let _ = theme;
    Rail {
        background: None,
        border: Border { color: Color::TRANSPARENT, width: 0.0, radius: 0.0.into() },
        scroller: Scroller {
            background: background(thumb),
            border: Border { color: Color::TRANSPARENT, width: 0.0, radius: 3.0.into() },
        },
    }
}

/// `scrollable(content).style(malva_iced::scroll::default)`; give the scrollbar `Scrollbar::new().width(6).scroller_width(6)`.
pub fn default(theme: &Theme, status: Status) -> Style {
    let c = colors(theme);
    let (v, h) = match status {
        Status::Active { .. } => (c.scroll_thumb, c.scroll_thumb),
        Status::Hovered { is_vertical_scrollbar_hovered, is_horizontal_scrollbar_hovered, .. } => (
            if is_vertical_scrollbar_hovered { c.text_2 } else { c.scroll_thumb },
            if is_horizontal_scrollbar_hovered { c.text_2 } else { c.scroll_thumb },
        ),
        Status::Dragged { is_vertical_scrollbar_dragged, is_horizontal_scrollbar_dragged, .. } => (
            if is_vertical_scrollbar_dragged { c.accent } else { c.scroll_thumb },
            if is_horizontal_scrollbar_dragged { c.accent } else { c.scroll_thumb },
        ),
    };
    Style {
        container: container::Style::default(),
        vertical_rail: rail(theme, v),
        horizontal_rail: rail(theme, h),
        gap: None,
        auto_scroll: AutoScroll {
            background: Background::Color(color(c.raised)),
            border: outline(color(c.border), 1.0, radius::PILL),
            shadow: elevation(theme, 1),
            icon: color(c.text_2),
        },
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::light;

    fn thumb(r: &Rail) -> Background {
        r.scroller.background
    }

    #[test]
    fn thumb_darkens_when_grabbed() {
        let t = light();
        let c = &malva_tokens::LIGHT;
        let rest = default(&t, Status::Active { is_horizontal_scrollbar_disabled: false, is_vertical_scrollbar_disabled: false });
        assert_eq!(thumb(&rest.vertical_rail), Background::Color(color(c.scroll_thumb)));
        let hover = default(
            &t,
            Status::Hovered {
                is_horizontal_scrollbar_hovered: false,
                is_vertical_scrollbar_hovered: true,
                is_horizontal_scrollbar_disabled: false,
                is_vertical_scrollbar_disabled: false,
            },
        );
        assert_eq!(thumb(&hover.vertical_rail), Background::Color(color(c.text_2)));
        assert_eq!(thumb(&hover.horizontal_rail), Background::Color(color(c.scroll_thumb)));
        let drag = default(
            &t,
            Status::Dragged {
                is_horizontal_scrollbar_dragged: true,
                is_vertical_scrollbar_dragged: false,
                is_horizontal_scrollbar_disabled: false,
                is_vertical_scrollbar_disabled: false,
            },
        );
        assert_eq!(thumb(&drag.horizontal_rail), Background::Color(color(c.accent)));
    }

    #[test]
    fn no_track_is_drawn() {
        let s = default(&light(), Status::Active { is_horizontal_scrollbar_disabled: false, is_vertical_scrollbar_disabled: false });
        assert!(s.vertical_rail.background.is_none() && s.gap.is_none());
    }
}
