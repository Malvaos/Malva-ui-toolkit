//! Colour maths for the Malva tokens.
//!
//! The blend formulas are integer formulas, the same ones `tools/gen_tokens.py` and `malva/color.h` use, so a colour
//! made here has the same bytes as the one in the token file. The tests of `malva-tokens` compare them.
//!
//! ```
//! use malva_color::Rgba;
//! let plum = Rgba::parse_hex("#8C3274").unwrap();
//! let hover = plum.mix(Rgba::new(0x2A, 0x12, 0x26, 255), 42);
//! assert_eq!(hover.to_hex(), "#632553");
//! assert!(plum.contrast_ratio(Rgba::WHITE) > 7.0);
//! ```
#![forbid(unsafe_code)]

use core::fmt;
use core::str::FromStr;

/// A colour: 8 bits per channel, straight (not premultiplied) alpha.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct Rgba {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
}

/// Why a colour string did not parse.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ParseError {
    /// Not 6 or 8 hex digits (after an optional `#`).
    Length(usize),
    /// A character that is not a hex digit.
    Digit(char),
}

impl fmt::Display for ParseError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ParseError::Length(n) => write!(f, "expected 6 or 8 hex digits, found {n}"),
            ParseError::Digit(c) => write!(f, "{c:?} is not a hex digit"),
        }
    }
}

impl std::error::Error for ParseError {}

const fn digit(c: u8) -> Option<u8> {
    match c {
        b'0'..=b'9' => Some(c - b'0'),
        b'a'..=b'f' => Some(c - b'a' + 10),
        b'A'..=b'F' => Some(c - b'A' + 10),
        _ => None,
    }
}

impl Rgba {
    pub const TRANSPARENT: Rgba = Rgba::new(0, 0, 0, 0);
    pub const BLACK: Rgba = Rgba::new(0, 0, 0, 255);
    pub const WHITE: Rgba = Rgba::new(255, 255, 255, 255);

    pub const fn new(r: u8, g: u8, b: u8, a: u8) -> Rgba {
        Rgba { r, g, b, a }
    }

    /// `0xRRGGBB`, opaque.
    pub const fn from_rgb_u32(rgb: u32) -> Rgba {
        Rgba::new((rgb >> 16) as u8, (rgb >> 8) as u8, rgb as u8, 255)
    }

    /// `"#RRGGBB"` or `"#RRGGBBAA"`; the `#` is optional and case does not matter.
    pub fn parse_hex(s: &str) -> Result<Rgba, ParseError> {
        let s = s.strip_prefix('#').unwrap_or(s);
        let bytes = s.as_bytes();
        if bytes.len() != 6 && bytes.len() != 8 {
            return Err(ParseError::Length(s.chars().count()));
        }
        let mut v = [0u8; 4];
        v[3] = 255;
        for (i, pair) in bytes.chunks(2).enumerate() {
            let hi = digit(pair[0]).ok_or_else(|| ParseError::Digit(s[2 * i..].chars().next().unwrap_or('?')))?;
            let lo = digit(pair[1]).ok_or_else(|| ParseError::Digit(s[2 * i + 1..].chars().next().unwrap_or('?')))?;
            v[i] = hi * 16 + lo;
        }
        Ok(Rgba::new(v[0], v[1], v[2], v[3]))
    }

    /// `"#RRGGBB"`, or `"#RRGGBBAA"` when the colour is not opaque.
    pub fn to_hex(self) -> String {
        if self.a == 255 {
            format!("#{:02X}{:02X}{:02X}", self.r, self.g, self.b)
        } else {
            self.to_hex_alpha()
        }
    }

    /// `"#RRGGBBAA"`, always.
    pub fn to_hex_alpha(self) -> String {
        format!("#{:02X}{:02X}{:02X}{:02X}", self.r, self.g, self.b, self.a)
    }

    /// Blends `self` toward `other` by `pct` percent (0 to 100), every channel including alpha.
    pub const fn mix(self, other: Rgba, pct: u32) -> Rgba {
        const fn ch(x: u8, y: u8, pct: u32) -> u8 {
            ((x as u32 * (100 - pct) + y as u32 * pct + 50) / 100) as u8
        }
        let pct = if pct > 100 { 100 } else { pct };
        Rgba::new(ch(self.r, other.r, pct), ch(self.g, other.g, pct), ch(self.b, other.b, pct), ch(self.a, other.a, pct))
    }

    /// The same colour at `pct` percent opacity.
    pub const fn with_alpha(self, pct: u32) -> Rgba {
        let pct = if pct > 100 { 100 } else { pct };
        Rgba::new(self.r, self.g, self.b, ((255 * pct + 50) / 100) as u8)
    }

    /// `self`, with its alpha, laid over `bg`: the opaque colour you see.
    pub const fn over(self, bg: Rgba) -> Rgba {
        const fn ch(x: u8, y: u8, a: u32) -> u8 {
            ((x as u32 * a + y as u32 * (255 - a) + 127) / 255) as u8
        }
        let a = self.a as u32;
        Rgba::new(ch(self.r, bg.r, a), ch(self.g, bg.g, a), ch(self.b, bg.b, a), 255)
    }

    /// WCAG 2.x relative luminance, 0 (black) to 1 (white).
    pub fn relative_luminance(self) -> f64 {
        fn lin(v: u8) -> f64 {
            let x = v as f64 / 255.0;
            if x <= 0.03928 {
                x / 12.92
            } else {
                ((x + 0.055) / 1.055).powf(2.4)
            }
        }
        0.2126 * lin(self.r) + 0.7152 * lin(self.g) + 0.0722 * lin(self.b)
    }

    /// WCAG contrast ratio between two colours, 1 to 21. Alpha is ignored: lay a translucent colour over its
    /// background with [`Rgba::over`] first.
    pub fn contrast_ratio(self, other: Rgba) -> f64 {
        let (a, b) = (self.relative_luminance(), other.relative_luminance());
        (a.max(b) + 0.05) / (a.min(b) + 0.05)
    }

    /// Of two candidates, the one that reads better on `self`.
    pub fn readable(self, a: Rgba, b: Rgba) -> Rgba {
        if a.contrast_ratio(self) >= b.contrast_ratio(self) {
            a
        } else {
            b
        }
    }

    /// Channels as floats in 0..=1, the form most GPU toolkits take.
    pub fn to_f32(self) -> [f32; 4] {
        [self.r as f32 / 255.0, self.g as f32 / 255.0, self.b as f32 / 255.0, self.a as f32 / 255.0]
    }

    pub fn to_array(self) -> [u8; 4] {
        [self.r, self.g, self.b, self.a]
    }
}

impl fmt::Display for Rgba {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(&self.to_hex())
    }
}

impl FromStr for Rgba {
    type Err = ParseError;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        Rgba::parse_hex(s)
    }
}

impl From<[u8; 4]> for Rgba {
    fn from(v: [u8; 4]) -> Self {
        Rgba::new(v[0], v[1], v[2], v[3])
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const PLUM: Rgba = Rgba::from_rgb_u32(0x8C3274);

    #[test]
    fn parses_and_formats() {
        assert_eq!(Rgba::parse_hex("#8C3274"), Ok(PLUM));
        assert_eq!(Rgba::parse_hex("8c327466"), Ok(Rgba::new(0x8C, 0x32, 0x74, 0x66)));
        assert_eq!("#8C3274".parse::<Rgba>(), Ok(PLUM));
        assert_eq!(Rgba::parse_hex("#12345"), Err(ParseError::Length(5)));
        assert_eq!(Rgba::parse_hex(""), Err(ParseError::Length(0)));
        assert_eq!(Rgba::parse_hex("#GGGGGG"), Err(ParseError::Digit('G')));
        assert_eq!(Rgba::parse_hex("#8C32G4"), Err(ParseError::Digit('G')));
        assert_eq!(PLUM.to_hex(), "#8C3274");
        assert_eq!(PLUM.with_alpha(40).to_hex(), "#8C327466");
        assert_eq!(PLUM.to_hex_alpha(), "#8C3274FF");
        assert_eq!(format!("{PLUM}"), "#8C3274");
        assert_eq!(Rgba::parse_hex(&PLUM.with_alpha(7).to_hex()).unwrap(), PLUM.with_alpha(7));
    }

    #[test]
    fn mix_matches_the_integer_formula() {
        assert_eq!(Rgba::BLACK.mix(Rgba::WHITE, 0), Rgba::BLACK);
        assert_eq!(Rgba::BLACK.mix(Rgba::WHITE, 100), Rgba::WHITE);
        assert_eq!(Rgba::BLACK.mix(Rgba::WHITE, 50), Rgba::new(128, 128, 128, 255)); // 127.5 rounds up
        assert_eq!(Rgba::new(0, 0, 0, 255).mix(Rgba::new(1, 1, 1, 255), 50), Rgba::new(1, 1, 1, 255));
        assert_eq!(Rgba::BLACK.mix(Rgba::new(255, 255, 255, 0), 50).a, 128); // alpha is mixed too
        assert_eq!(Rgba::BLACK.mix(Rgba::WHITE, 250), Rgba::WHITE); // clamped
        const FROM_CONST: Rgba = Rgba::BLACK.mix(Rgba::WHITE, 50); // usable in const context
        assert_eq!(FROM_CONST.r, 128);
    }

    #[test]
    fn alpha_and_over() {
        assert_eq!(Rgba::WHITE.with_alpha(40).a, 0x66);
        assert_eq!(Rgba::WHITE.with_alpha(100).a, 255);
        assert_eq!(Rgba::WHITE.with_alpha(0).a, 0);
        assert_eq!(Rgba::WHITE.over(Rgba::BLACK), Rgba::WHITE);
        assert_eq!(Rgba::WHITE.with_alpha(0).over(Rgba::BLACK), Rgba::BLACK);
        assert_eq!(Rgba::WHITE.with_alpha(50).over(Rgba::BLACK), Rgba::new(128, 128, 128, 255));
    }

    #[test]
    fn contrast() {
        assert!((Rgba::WHITE.contrast_ratio(Rgba::BLACK) - 21.0).abs() < 1e-9);
        assert!((PLUM.contrast_ratio(PLUM) - 1.0).abs() < 1e-9);
        assert_eq!(PLUM.contrast_ratio(Rgba::WHITE), Rgba::WHITE.contrast_ratio(PLUM));
        assert_eq!(PLUM.readable(Rgba::WHITE, Rgba::BLACK), Rgba::WHITE);
        let petal = Rgba::from_rgb_u32(0xE6DAE8);
        let ink = Rgba::from_rgb_u32(0x2A1226);
        assert_eq!(petal.readable(Rgba::WHITE, ink), ink);
    }

    #[test]
    fn same_bytes_as_the_token_generator() {
        let accent = PLUM;
        let text = Rgba::from_rgb_u32(0x2A1226);
        assert_eq!(accent.mix(text, 42).to_hex(), "#632553"); // accent-hover, light
        assert_eq!(accent.mix(text, 80).to_hex(), "#3E1836"); // accent-press, light
    }

    #[test]
    fn floats() {
        assert_eq!(Rgba::WHITE.to_f32(), [1.0, 1.0, 1.0, 1.0]);
        assert_eq!(Rgba::TRANSPARENT.to_f32(), [0.0; 4]);
        assert_eq!(Rgba::from([1, 2, 3, 4]).to_array(), [1, 2, 3, 4]);
    }
}
