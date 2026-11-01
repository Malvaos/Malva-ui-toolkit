//! The curves and the spring against numbers computed independently in Python (tools/gen_tokens.py writes
//! dist/motion-golden.json): the same file the C++ test reads.
use malva_motion::{curves, ease, Spring};

const GOLDEN: &str = include_str!("../../../dist/motion-golden.json");

/// The numbers of `"key": [ ... ]`, starting the search at `from`.
fn array(json: &str, key: &str, from: usize) -> Vec<f64> {
    let k = format!("\"{key}\"");
    let at = json[from..].find(&k).unwrap_or_else(|| panic!("no key {key}")) + from;
    let open = json[at..].find('[').unwrap() + at;
    let close = json[open..].find(']').unwrap() + open;
    json[open + 1..close].split(',').map(|s| s.trim().parse::<f64>().unwrap()).collect()
}

#[test]
fn easing_curves_match_the_reference() {
    let xs = array(GOLDEN, "xs", 0);
    let base = GOLDEN.find("\"curves\"").unwrap();
    for (name, curve) in [("out", curves::OUT), ("standard", curves::STANDARD), ("soft", curves::SOFT), ("spring", curves::SPRING), ("exit", curves::EXIT)] {
        let ys = array(GOLDEN, name, base);
        assert_eq!(xs.len(), ys.len());
        for (x, want) in xs.iter().zip(&ys) {
            let got = ease(curve, *x);
            assert!((got - want).abs() < 1e-5, "{name} at {x}: {got} vs {want}");
        }
    }
}

#[test]
fn spring_matches_the_reference() {
    let ts = array(GOLDEN, "spring-ts", 0);
    let ys = array(GOLDEN, "spring-from-0-to-1", 0);
    assert_eq!(ts.len(), ys.len());
    let s = Spring::default();
    for (t, want) in ts.iter().zip(&ys) {
        let got = s.value(*t, 0.0, 1.0);
        assert!((got - want).abs() < 1e-5, "spring at {t}: {got} vs {want}");
    }
}
