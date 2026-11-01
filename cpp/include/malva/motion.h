// Motion: the cubic-bezier easing curves of the tokens and the spring, for code that animates things itself
// (Slint animates its own properties with the same numbers).
#pragma once
#include "tokens.h"

#include <cmath>

namespace malva::motion {

// The y of a CSS-style cubic bezier (0,0) (x1,y1) (x2,y2) (1,1) at progress x in [0, 1]. Newton's method on
// x(t), with bisection as the fallback, the way browsers do it.
inline double ease(Bezier b, double x)
{
    if (x <= 0)
        return 0;
    if (x >= 1)
        return 1;
    const double cx = 3.0 * b.x1, bx = 3.0 * (b.x2 - b.x1) - cx, ax = 1.0 - cx - bx;
    const double cy = 3.0 * b.y1, by = 3.0 * (b.y2 - b.y1) - cy, ay = 1.0 - cy - by;
    auto sx = [&](double t) { return ((ax * t + bx) * t + cx) * t; };
    auto sy = [&](double t) { return ((ay * t + by) * t + cy) * t; };
    auto dx = [&](double t) { return (3.0 * ax * t + 2.0 * bx) * t + cx; };
    double t = x;
    bool solved = false;
    for (int i = 0; i < 8; ++i) {
        const double err = sx(t) - x;
        if (std::fabs(err) < 1e-7) {
            solved = true;
            break;
        }
        const double d = dx(t);
        if (std::fabs(d) < 1e-6)
            break;
        t -= err / d;
    }
    if (!solved) {
        double lo = 0, hi = 1;
        t = x;
        for (int i = 0; i < 60; ++i) {
            const double v = sx(t);
            if (std::fabs(v - x) < 1e-7)
                break;
            (x > v ? lo : hi) = t;
            t = (lo + hi) / 2;
        }
    }
    return sy(t);
}

// A damped spring pulling a value toward a target, released from rest. value(t) is the closed-form position
// for the underdamped, critically damped and overdamped cases, so it needs no time step and cannot blow up.
struct Spring {
    float stiffness = spring_stiffness;
    float damping = spring_damping;
    float mass = spring_mass;

    // Position at time t (seconds) of a spring that starts at `from` at rest and is pulled to `to`.
    double value(double t, double from, double to) const
    {
        if (t <= 0)
            return from;
        const double w0 = std::sqrt(stiffness / mass);
        const double zeta = damping / (2.0 * std::sqrt(static_cast<double>(stiffness) * mass));
        double k;  // fraction of the distance still to go
        if (zeta < 1.0) {
            const double wd = w0 * std::sqrt(1.0 - zeta * zeta);
            k = std::exp(-zeta * w0 * t) * (std::cos(wd * t) + zeta * w0 / wd * std::sin(wd * t));
        } else if (zeta == 1.0) {
            k = std::exp(-w0 * t) * (1.0 + w0 * t);
        } else {
            const double s = w0 * std::sqrt(zeta * zeta - 1.0);
            const double r1 = -zeta * w0 + s, r2 = -zeta * w0 - s;
            k = (r1 * std::exp(r2 * t) - r2 * std::exp(r1 * t)) / (r1 - r2);
        }
        return to + (from - to) * k;
    }
    // How long until it stays within `tolerance` of the target (as a fraction of the distance), seconds.
    double settle_time(double tolerance = 0.002) const
    {
        const double w0 = std::sqrt(stiffness / mass);
        const double zeta = damping / (2.0 * std::sqrt(static_cast<double>(stiffness) * mass));
        const double rate = zeta < 1.0 ? zeta * w0 : zeta == 1.0 ? w0 : w0 * (zeta - std::sqrt(zeta * zeta - 1.0));
        return std::log(1.0 / tolerance) / rate;
    }
};

}  // namespace malva::motion
