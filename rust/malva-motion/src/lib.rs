//! Motion: the easing curves of the Malva tokens and the spring, for code that animates things itself (the Slint
//! components animate their own properties with the same numbers).
//!
//! ```
//! use malva_motion::{ease, curves, Spring};
//! assert_eq!(ease(curves::OUT, 0.0), 0.0);
//! assert_eq!(ease(curves::OUT, 1.0), 1.0);
//! let overshoot = (1..100).map(|i| ease(curves::SPRING, i as f64 / 100.0)).fold(0.0_f64, f64::max);
//! assert!(overshoot > 1.0); // the spring curve goes past the target and comes back
//! let s = Spring::default();
//! assert!((s.value(s.settle_time(0.002), 0.0, 1.0) - 1.0).abs() < 0.003);
//! ```
#![forbid(unsafe_code)]

use malva_tokens::motion as t;

/// A CSS-style cubic bezier from (0,0) to (1,1) with control points (x1,y1) and (x2,y2).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Bezier {
    pub x1: f64,
    pub y1: f64,
    pub x2: f64,
    pub y2: f64,
}

impl Bezier {
    pub const fn new(x1: f64, y1: f64, x2: f64, y2: f64) -> Bezier {
        Bezier { x1, y1, x2, y2 }
    }
}

impl From<t::Bezier> for Bezier {
    fn from(b: t::Bezier) -> Self {
        // the tokens are f32; going through the shortest decimal keeps 0.34 as 0.34 and not 0.3400000035
        let d = |v: f32| format!("{v}").parse::<f64>().unwrap_or(v as f64);
        Bezier::new(d(b[0]), d(b[1]), d(b[2]), d(b[3]))
    }
}

/// The five curves of the tokens.
pub mod curves {
    use super::Bezier;
    pub const OUT: Bezier = Bezier::new(0.0, 0.0, 0.2, 1.0);
    pub const STANDARD: Bezier = Bezier::new(0.2, 0.0, 0.0, 1.0);
    pub const SOFT: Bezier = Bezier::new(0.4, 0.0, 0.2, 1.0);
    pub const SPRING: Bezier = Bezier::new(0.34, 1.35, 0.64, 1.0);
    pub const EXIT: Bezier = Bezier::new(0.4, 0.0, 1.0, 1.0);
}

/// The y of the curve at progress `x` in 0..=1 (clamped outside). Newton's method on x(t), with bisection as the
/// fallback, the way browsers do it.
pub fn ease(b: Bezier, x: f64) -> f64 {
    if x <= 0.0 {
        return 0.0;
    }
    if x >= 1.0 {
        return 1.0;
    }
    let cx = 3.0 * b.x1;
    let bx = 3.0 * (b.x2 - b.x1) - cx;
    let ax = 1.0 - cx - bx;
    let cy = 3.0 * b.y1;
    let by = 3.0 * (b.y2 - b.y1) - cy;
    let ay = 1.0 - cy - by;
    let sx = |t: f64| ((ax * t + bx) * t + cx) * t;
    let sy = |t: f64| ((ay * t + by) * t + cy) * t;
    let dx = |t: f64| (3.0 * ax * t + 2.0 * bx) * t + cx;
    let mut t = x;
    let mut solved = false;
    for _ in 0..8 {
        let err = sx(t) - x;
        if err.abs() < 1e-7 {
            solved = true;
            break;
        }
        let d = dx(t);
        if d.abs() < 1e-6 {
            break;
        }
        t -= err / d;
    }
    if !solved {
        let (mut lo, mut hi) = (0.0, 1.0);
        t = x;
        for _ in 0..60 {
            let v = sx(t);
            if (v - x).abs() < 1e-7 {
                break;
            }
            if x > v {
                lo = t;
            } else {
                hi = t;
            }
            t = (lo + hi) / 2.0;
        }
    }
    sy(t)
}

/// Linear interpolation of `a` to `b` along a curve.
pub fn lerp(b: Bezier, from: f64, to: f64, x: f64) -> f64 {
    from + (to - from) * ease(b, x)
}

/// A damped spring pulling a value toward a target, released from rest. `value` is the closed form for the
/// underdamped, critically damped and overdamped cases, so it needs no time step and cannot blow up.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Spring {
    pub stiffness: f64,
    pub damping: f64,
    pub mass: f64,
}

impl Default for Spring {
    fn default() -> Self {
        Spring { stiffness: t::SPRING_STIFFNESS as f64, damping: t::SPRING_DAMPING as f64, mass: t::SPRING_MASS as f64 }
    }
}

impl Spring {
    fn w0(&self) -> f64 {
        (self.stiffness / self.mass).sqrt()
    }
    fn zeta(&self) -> f64 {
        self.damping / (2.0 * (self.stiffness * self.mass).sqrt())
    }

    /// Position after `t` seconds of a spring that starts at `from` at rest and is pulled to `to`.
    pub fn value(&self, t: f64, from: f64, to: f64) -> f64 {
        if t <= 0.0 {
            return from;
        }
        let (w0, zeta) = (self.w0(), self.zeta());
        let k = if zeta < 1.0 {
            let wd = w0 * (1.0 - zeta * zeta).sqrt();
            (-zeta * w0 * t).exp() * ((wd * t).cos() + zeta * w0 / wd * (wd * t).sin())
        } else if zeta == 1.0 {
            (-w0 * t).exp() * (1.0 + w0 * t)
        } else {
            let s = w0 * (zeta * zeta - 1.0).sqrt();
            let (r1, r2) = (-zeta * w0 + s, -zeta * w0 - s);
            (r1 * (r2 * t).exp() - r2 * (r1 * t).exp()) / (r1 - r2)
        };
        to + (from - to) * k
    }

    /// Seconds until it stays within `tolerance` (a fraction of the distance) of the target.
    pub fn settle_time(&self, tolerance: f64) -> f64 {
        let (w0, zeta) = (self.w0(), self.zeta());
        let rate = if zeta < 1.0 {
            zeta * w0
        } else if zeta == 1.0 {
            w0
        } else {
            w0 * (zeta - (zeta * zeta - 1.0).sqrt())
        };
        (1.0 / tolerance).ln() / rate
    }
}

/// The durations of the tokens, in milliseconds.
pub mod duration {
    pub use malva_tokens::motion::{CLOSE_MS, FADE_MS, HOVER_MS, OPEN_MS, PRESS_MS, SLOW_MS};
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn curves_equal_the_tokens() {
        assert_eq!(Bezier::from(t::CURVE_OUT), curves::OUT);
        assert_eq!(Bezier::from(t::CURVE_STANDARD), curves::STANDARD);
        assert_eq!(Bezier::from(t::CURVE_SOFT), curves::SOFT);
        assert_eq!(Bezier::from(t::CURVE_SPRING), curves::SPRING);
        assert_eq!(Bezier::from(t::CURVE_EXIT), curves::EXIT);
    }

    #[test]
    fn ends_and_monotonic() {
        for c in [curves::OUT, curves::STANDARD, curves::SOFT, curves::EXIT] {
            assert_eq!(ease(c, 0.0), 0.0);
            assert_eq!(ease(c, 1.0), 1.0);
            assert_eq!(ease(c, -3.0), 0.0);
            assert_eq!(ease(c, 7.0), 1.0);
            let mut prev = 0.0;
            for i in 1..=200 {
                let y = ease(c, i as f64 / 200.0);
                assert!(y >= prev - 1e-9, "{c:?} goes backwards at {i}");
                prev = y;
            }
        }
    }

    #[test]
    fn linear_bezier_is_the_identity() {
        let linear = Bezier::new(1.0 / 3.0, 1.0 / 3.0, 2.0 / 3.0, 2.0 / 3.0);
        for i in 0..=20 {
            let x = i as f64 / 20.0;
            assert!((ease(linear, x) - x).abs() < 1e-6);
        }
    }

    #[test]
    fn spring_defaults_are_underdamped_and_settle() {
        let s = Spring::default();
        assert_eq!(s.value(0.0, 5.0, 9.0), 5.0);
        let zeta = s.damping / (2.0 * (s.stiffness * s.mass).sqrt());
        assert!(zeta < 1.0 && zeta > 0.5);
        let peak = (1..600).map(|i| s.value(i as f64 / 1000.0, 0.0, 1.0)).fold(0.0_f64, f64::max);
        assert!(peak > 1.0 && peak < 1.2, "overshoot {peak}");
        let end = s.settle_time(0.002);
        assert!(end > 0.1 && end < 1.0, "settle {end}");
        for dt in [0.0, 0.01, 0.5] {
            assert!((s.value(end + dt, 0.0, 1.0) - 1.0).abs() < 0.0025);
        }
    }

    #[test]
    fn critically_and_over_damped_never_overshoot() {
        let critical = Spring { stiffness: 100.0, damping: 20.0, mass: 1.0 };
        let over = Spring { stiffness: 100.0, damping: 40.0, mass: 1.0 };
        for s in [critical, over] {
            for i in 0..400 {
                let v = s.value(i as f64 / 100.0, 0.0, 1.0);
                assert!((-1e-9..=1.0 + 1e-9).contains(&v), "{s:?} at {i}: {v}");
            }
        }
    }
}
