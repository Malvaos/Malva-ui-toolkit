//! Dividers: a hairline in the Line colour.
use iced::widget::rule::{FillMode, Style};
use iced::Theme;

use crate::{color, colors};

/// `rule::horizontal(1).style(malva_iced::rule::divider)`
pub fn divider(theme: &Theme) -> Style {
    Style { color: color(colors(theme).line), radius: 0.0.into(), fill_mode: FillMode::Full, snap: true }
}

/// The same inset from both ends by 16 px, for dividers inside a list.
pub fn inset(theme: &Theme) -> Style {
    Style { fill_mode: FillMode::Padded(16), ..divider(theme) }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::dark;

    #[test]
    fn hairline_colour() {
        assert_eq!(divider(&dark()).color, color(malva_tokens::DARK.line));
        assert_eq!(inset(&dark()).fill_mode, FillMode::Padded(16));
    }
}
