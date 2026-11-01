// Colour maths for the Malva tokens, with exactly the integer formulas of tools/gen_tokens.py, so a colour made
// here has the same bytes as the one in the token file (the tests compare them).
#pragma once
#include "tokens.h"

#include <cmath>
#include <optional>
#include <string>
#include <string_view>

namespace malva {

namespace detail {
constexpr int hex_digit(char c)
{
    return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}
constexpr int hex_byte(std::string_view s, std::size_t i)
{
    const int hi = hex_digit(s[i]), lo = hex_digit(s[i + 1]);
    return hi < 0 || lo < 0 ? -1 : hi * 16 + lo;
}
}  // namespace detail

// "#RRGGBB" or "#RRGGBBAA" (the '#' is optional). Returns nullopt for anything else.
constexpr std::optional<Rgba> parse_hex(std::string_view s)
{
    if (!s.empty() && s.front() == '#')
        s.remove_prefix(1);
    if (s.size() != 6 && s.size() != 8)
        return std::nullopt;
    const int r = detail::hex_byte(s, 0), g = detail::hex_byte(s, 2), b = detail::hex_byte(s, 4);
    const int a = s.size() == 8 ? detail::hex_byte(s, 6) : 255;
    if (r < 0 || g < 0 || b < 0 || a < 0)
        return std::nullopt;
    return Rgba{static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b), static_cast<std::uint8_t>(a)};
}

// "#RRGGBB", or "#RRGGBBAA" when the colour is not opaque (or always_alpha).
inline std::string to_hex(Rgba c, bool always_alpha = false)
{
    static const char* d = "0123456789ABCDEF";
    std::string s = "#";
    for (std::uint8_t v : {c.r, c.g, c.b})
        s += {d[v >> 4], d[v & 15]};
    if (c.a != 255 || always_alpha)
        s += {d[c.a >> 4], d[c.a & 15]};
    return s;
}

// Blends a toward b by pct percent (0..100), every channel including alpha.
constexpr Rgba mix(Rgba a, Rgba b, int pct)
{
    auto ch = [&](int x, int y) { return static_cast<std::uint8_t>((x * (100 - pct) + y * pct + 50) / 100); };
    return {ch(a.r, b.r), ch(a.g, b.g), ch(a.b, b.b), ch(a.a, b.a)};
}

// The same colour at pct percent opacity.
constexpr Rgba with_alpha(Rgba a, int pct)
{
    return {a.r, a.g, a.b, static_cast<std::uint8_t>((255 * pct + 50) / 100)};
}

// fg, with its alpha, laid over bg: the opaque colour you see.
constexpr Rgba over(Rgba fg, Rgba bg)
{
    auto ch = [&](int x, int y) { return static_cast<std::uint8_t>((x * fg.a + y * (255 - fg.a) + 127) / 255); };
    return {ch(fg.r, bg.r), ch(fg.g, bg.g), ch(fg.b, bg.b), 255};
}

// WCAG 2.x relative luminance and contrast ratio (1 to 21).
inline double relative_luminance(Rgba c)
{
    auto lin = [](std::uint8_t v) {
        const double x = v / 255.0;
        return x <= 0.03928 ? x / 12.92 : std::pow((x + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * lin(c.r) + 0.7152 * lin(c.g) + 0.0722 * lin(c.b);
}
inline double contrast_ratio(Rgba a, Rgba b)
{
    const double la = relative_luminance(a), lb = relative_luminance(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

// Of two candidates, the one that reads better on `bg`.
inline Rgba readable_on(Rgba bg, Rgba a, Rgba b) { return contrast_ratio(a, bg) >= contrast_ratio(b, bg) ? a : b; }

}  // namespace malva
