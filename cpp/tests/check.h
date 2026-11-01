// A tiny test harness: CHECK(cond) counts failures and keeps going; main returns the number of failures.
#pragma once
#include <cstdio>

inline int g_failures = 0;
#define CHECK(cond)                                                                       \
    do {                                                                                  \
        if (!(cond)) {                                                                    \
            std::fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                                                 \
        }                                                                                 \
    } while (0)
#define CHECK_NEAR(a, b, eps)                                                                                       \
    do {                                                                                                            \
        const double va = (a), vb = (b);                                                                            \
        if (!(va - vb <= (eps) && vb - va <= (eps))) {                                                              \
            std::fprintf(stderr, "%s:%d: %s = %.6f, expected %s = %.6f (+-%g)\n", __FILE__, __LINE__, #a, va, #b, vb, (double)(eps)); \
            ++g_failures;                                                                                           \
        }                                                                                                           \
    } while (0)
inline int finish(const char* name)
{
    if (g_failures == 0)
        std::printf("%s: all checks passed\n", name);
    else
        std::printf("%s: %d check(s) failed\n", name, g_failures);
    return g_failures == 0 ? 0 : 1;
}
