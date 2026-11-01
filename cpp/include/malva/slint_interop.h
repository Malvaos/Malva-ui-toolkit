// Conversions between the token types and Slint's colours.
#pragma once
#include "color.h"

#include <slint.h>

namespace malva {

inline slint::Color to_slint(Rgba c) { return slint::Color::from_argb_uint8(c.a, c.r, c.g, c.b); }
inline Rgba from_slint(const slint::Color& c)
{
    const auto v = c.as_argb_encoded();
    return {static_cast<std::uint8_t>((v >> 16) & 0xFF), static_cast<std::uint8_t>((v >> 8) & 0xFF), static_cast<std::uint8_t>(v & 0xFF),
            static_cast<std::uint8_t>((v >> 24) & 0xFF)};
}

}  // namespace malva
