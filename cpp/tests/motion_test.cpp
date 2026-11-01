// The easing solver and the spring against samples computed by an independent Python implementation.
#include "check.h"
#include "data.h"
#include "malva/motion.h"

using namespace malva;

int main()
{
    const auto g = testdata::golden();
    const auto& xs = g.at("xs");
    CHECK(xs.size() == 21);

    struct Named { const char* name; motion::Bezier b; };
    const Named curves[] = {{"out", motion::curve_out}, {"standard", motion::curve_standard}, {"soft", motion::curve_soft},
                            {"spring", motion::curve_spring}, {"exit", motion::curve_exit}};
    for (const Named& c : curves) {
        const auto& want = g.at(c.name);
        CHECK(want.size() == xs.size());
        for (std::size_t i = 0; i < xs.size() && i < want.size(); ++i)
            CHECK_NEAR(motion::ease(c.b, xs[i]), want[i], 1e-4);
        // every curve starts at 0 and ends at 1
        CHECK_NEAR(motion::ease(c.b, 0), 0, 1e-12);
        CHECK_NEAR(motion::ease(c.b, 1), 1, 1e-12);
    }
    // the "spring" curve overshoots, the others do not
    double peak = 0;
    for (double x = 0; x <= 1; x += 0.01)
        peak = std::max(peak, motion::ease(motion::curve_spring, x));
    CHECK(peak > 1.03 && peak < 1.06);
    for (const Named& c : curves) {
        if (std::string(c.name) == "spring")
            continue;
        for (double x = 0; x <= 1; x += 0.01)
            CHECK(motion::ease(c.b, x) <= 1.0 + 1e-9);
    }
    // monotonic where it should be
    for (const Named& c : curves) {
        if (std::string(c.name) == "spring")
            continue;
        double last = -1;
        for (double x = 0; x <= 1; x += 0.01) {
            const double y = motion::ease(c.b, x);
            CHECK(y >= last - 1e-9);
            last = y;
        }
    }

    // the spring
    const auto& ts = g.at("spring-ts");
    const auto& want = g.at("spring-from-0-to-1");
    motion::Spring sp;
    for (std::size_t i = 0; i < ts.size() && i < want.size(); ++i)
        CHECK_NEAR(sp.value(ts[i], 0.0, 1.0), want[i], 1e-4);
    CHECK_NEAR(sp.value(0, 0.0, 1.0), 0.0, 1e-12);
    CHECK_NEAR(sp.value(10, 0.0, 1.0), 1.0, 1e-6);
    CHECK(sp.settle_time() > 0.1 && sp.settle_time() < 1.0);
    // it overshoots a little (underdamped) and comes back
    double top = 0;
    for (double t = 0; t < 1; t += 0.005)
        top = std::max(top, sp.value(t, 0.0, 1.0));
    CHECK(top > 1.0 && top < 1.05);
    // critically damped and overdamped never overshoot
    motion::Spring crit{400, 40, 1};
    motion::Spring over{400, 80, 1};
    for (double t = 0; t < 2; t += 0.01) {
        CHECK(crit.value(t, 0.0, 1.0) <= 1.0 + 1e-9);
        CHECK(over.value(t, 0.0, 1.0) <= 1.0 + 1e-9);
    }
    // moving the other way is the mirror image
    CHECK_NEAR(sp.value(0.2, 1.0, 0.0), 1.0 - sp.value(0.2, 0.0, 1.0), 1e-12);
    return finish("motion_test");
}
