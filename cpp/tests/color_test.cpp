// The colour maths: parsing, the integer blend formulas, compositing, contrast.
#include "check.h"
#include "malva/color.h"

using namespace malva;

int main()
{
    // parsing
    CHECK(parse_hex("#8C3274") == Rgba({0x8C, 0x32, 0x74, 0xFF}));
    CHECK(parse_hex("8c327466") == Rgba({0x8C, 0x32, 0x74, 0x66}));
    CHECK(!parse_hex("#12345"));
    CHECK(!parse_hex("#GGGGGG"));
    CHECK(!parse_hex(""));
    static_assert(parse_hex("#FFFFFF").has_value());
    static_assert(*parse_hex("#102030") == Rgba({0x10, 0x20, 0x30, 0xFF}));

    // formatting round-trips
    CHECK(to_hex(brand::plum) == "#8C3274");
    CHECK(to_hex(with_alpha(brand::plum, 40)) == "#8C327466");
    CHECK(to_hex(brand::plum, true) == "#8C3274FF");
    for (const Role& r : roles) {
        for (Mode m : {Mode::Light, Mode::Dark}) {
            const Rgba c = colors(m).*(r.member);
            CHECK(parse_hex(to_hex(c, true)) == c);
        }
    }

    // mix: the two ends, the middle, the rounding (50 % of 0 and 1 rounds up), alpha carried along
    constexpr Rgba black{0, 0, 0, 255}, white{255, 255, 255, 255};
    static_assert(mix(black, white, 0) == black);
    static_assert(mix(black, white, 100) == white);
    static_assert(mix(black, white, 50) == Rgba({128, 128, 128, 255}));
    static_assert(mix(Rgba({0, 0, 0, 255}), Rgba({1, 1, 1, 255}), 50) == Rgba({1, 1, 1, 255}));
    CHECK(mix(black, Rgba({255, 255, 255, 0}), 50).a == 128);

    // alpha: 40 % of 255 is 102 (0x66), and 100 % is opaque
    static_assert(with_alpha(white, 40).a == 0x66);
    static_assert(with_alpha(white, 100).a == 255);
    static_assert(with_alpha(white, 0).a == 0);

    // over: opaque on top hides, transparent on top shows, half mixes
    static_assert(over(white, black) == white);
    static_assert(over(with_alpha(white, 0), black) == black);
    CHECK(over(with_alpha(white, 50), black) == Rgba({128, 128, 128, 255}));

    // contrast: white on black is 21:1, a colour on itself 1:1, and it is symmetric
    CHECK_NEAR(contrast_ratio(white, black), 21.0, 1e-9);
    CHECK_NEAR(contrast_ratio(brand::plum, brand::plum), 1.0, 1e-9);
    CHECK_NEAR(contrast_ratio(brand::plum, white), contrast_ratio(white, brand::plum), 1e-12);
    CHECK(readable_on(brand::plum, white, black) == white);
    CHECK(readable_on(brand::petal, white, brand::ink) == brand::ink);

    // the same formulas as the generator, on the values the tokens are built from
    CHECK(to_hex(mix(light.accent, light.text, 42)) == "#5D2950");  // accent-hover in light
    CHECK(to_hex(mix(light.accent, light.text, 62)) == "#47253F");  // accent-press in light
    return finish("color_test");
}
