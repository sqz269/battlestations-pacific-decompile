#pragma once

// cc13_x87_model: shared models of x87 arithmetic for cc13 reconstructions (cc13-fixture2
// follow-up, docs/CC13_FIXTURE_PASS_2.md). Header-only. These are modelling helpers, not
// recovered routines.
//
// Precision follows the calling thread (docs/CC13_FPU_PRECISION.md): the main thread runs
// with precision control 24-bit after device creation, the job-pool and Dyn workers with
// 53-bit. Under 24-bit control FADD/FSUB/FMUL/FDIV/FSQRT round each result once to a 24-bit
// significand, but the exponent keeps the extended range, so an intermediate near 1e30
// squared does not overflow on the stack. The x87_*24 helpers model exactly that: each
// result is the exact operation rounded once (to nearest, ties to even) to 24 significand
// bits, carried in a double. FSTP DWORD then rounds that value to float, which is
// static_cast<float> (the value already has 24 bits, so only overflow, underflow and
// denormals can change it, as on the x87). The exact error of the double operation decides
// ties, so there is no double rounding. Operands are assumed to be in double range, which
// holds for float inputs and short chains.

#include <float.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace bsp::cc13x87 {

// The x87 CRT transcendental helpers (log2, expm1, sin, cos) run on the x87 and so lose
// precision when the calling thread runs with 24-bit precision control, as the game's main
// thread does. The models below call them under this guard, which sets 53-bit precision
// for the call and restores the caller's word.
class X87Precision53 {
public:
    X87Precision53() noexcept {
        _controlfp_s(&old_, 0, 0);
        unsigned int ignored;
        _controlfp_s(&ignored, _PC_53, _MCW_PC);
    }
    ~X87Precision53() {
        unsigned int ignored;
        _controlfp_s(&ignored, old_ & _MCW_PC, _MCW_PC);
    }
    X87Precision53(const X87Precision53&) = delete;
    X87Precision53& operator=(const X87Precision53&) = delete;

private:
    unsigned int old_ = 0;
};

// FYL2X: k * log2(x) with the extended result read as a double (FYL2X ignores precision
// control).
inline double x87_fyl2x(double k, double x) noexcept {
    const X87Precision53 guard;
    return k * std::log2(x);
}

// Rounds hi + lo to a 24-bit significand, where hi is the double result of an operation and
// lo carries the sign of its exact rounding error (0 when hi is exact).
inline double round24(double hi, double lo) noexcept {
    std::uint64_t bits;
    std::memcpy(&bits, &hi, sizeof bits);
    if (((bits >> 52) & 0x7ff) == 0x7ff || hi == 0.0) return hi;
    constexpr std::uint64_t kLow = (std::uint64_t{1} << 29) - 1;
    constexpr std::uint64_t kHalf = std::uint64_t{1} << 28;
    const std::uint64_t low = bits & kLow;
    bits &= ~kLow;
    bool up = low > kHalf;
    if (low == kHalf) {
        // A tie in the double result: the exact error says which side the true value is on
        // (toward larger magnitude when lo has the sign of hi); an exact tie goes to even.
        if (lo == 0.0) up = (bits >> 29) & 1;
        else up = (lo > 0.0) == (hi > 0.0);
    }
    if (up) bits += kLow + 1;
    double out;
    std::memcpy(&out, &bits, sizeof out);
    return out;
}

inline double x87_add24(double a, double b) noexcept {
    const double s = a + b;
    if (!std::isfinite(s)) return s;
    const double bb = s - a;
    return round24(s, (a - (s - bb)) + (b - bb));
}
inline double x87_sub24(double a, double b) noexcept {
    const double s = a - b;  // a NaN keeps its own sign, as FSUB does
    if (!std::isfinite(s)) return s;
    return x87_add24(a, -b);
}
inline double x87_mul24(double a, double b) noexcept {
    const double p = a * b;
    if (!std::isfinite(p)) return p;
    return round24(p, std::fma(a, b, -p));
}
inline double x87_div24(double a, double b) noexcept {
    const double q = a / b;
    if (!std::isfinite(q) || q == 0.0) return q;
    const double r = std::fma(-q, b, a);
    return round24(q, r == 0.0 ? 0.0 : ((r > 0.0) == (b > 0.0) ? 1.0 : -1.0));
}
inline double x87_sqrt24(double a) noexcept {
    const double q = std::sqrt(a);
    if (!std::isfinite(q) || q == 0.0) return q;
    return round24(q, std::fma(-q, q, a));
}

// The power tail FLD ST0 / FRNDINT / FSUB ST1,ST0 / F2XM1 / FLD1 / FADDP / FSCALE on y (the
// FYL2X result, here a double), under 24-bit precision control. FYL2X, FRNDINT, F2XM1 and
// FSCALE ignore precision control; the FSUB (y - n) and the FADDP (1 + 2^f - 1) round to 24
// significand bits. An infinite y gives inf - inf, so the default NaN; a NaN passes through.
inline double x87_pow2_tail24(double y) noexcept {
    if (std::isnan(y)) return y;
    if (std::isinf(y)) return -std::numeric_limits<double>::quiet_NaN();
    const double n = std::nearbyint(y);
    const double f = x87_sub24(y, n);
    double e;
    {
        const X87Precision53 guard;
        e = std::expm1(f * 0.69314718055994530942);
    }
    const double m = x87_add24(1.0, e);
    const double clamped = n > 4096.0 ? 4096.0 : (n < -4096.0 ? -4096.0 : n);
    return std::ldexp(m, static_cast<int>(clamped));
}

// FSIN / FCOS. The instruction leaves an operand of magnitude 2^63 or more unchanged (C2
// set, no reduction), turns an infinity into the default NaN and passes a NaN through. The
// transcendental instructions ignore precision control. Inside the range the CRT value,
// rounded to float by the image's FSTP, matched for the fixture's inputs (|x| <= 7 and the
// edge values); FSIN/FCOS reduce with a 66-bit pi, so a large finite operand below 2^63
// (for example 1e5 or 1e15) can differ from the CRT and is not claimed here.
inline double x87_trig_unreduced(double x) noexcept {
    if (std::isinf(x)) return -std::numeric_limits<double>::quiet_NaN();
    return x;
}
inline double x87_sin(double x) noexcept {
    if (!(std::fabs(x) < 0x1p63)) return x87_trig_unreduced(x);
    const X87Precision53 guard;
    return std::sin(x);
}
inline double x87_cos(double x) noexcept {
    if (!(std::fabs(x) < 0x1p63)) return x87_trig_unreduced(x);
    const X87Precision53 guard;
    return std::cos(x);
}

}  // namespace bsp::cc13x87
