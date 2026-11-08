// Focused differential probe for the 00855A90 adapter. Link against bsp_core.
#include "bsp/bullet_engagement_range.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {
struct NativeLeaf { float range, terminal; };

// Independent reference: the original listing's x87 operations and stores,
// including the CRT sqrt worker's PC53 control word (00BF7058..00BF706C).
// All fixtures are finite/nonnegative, so the CRT's error arms are excluded.
__declspec(noinline) NativeLeaf native_leaf(float speed, float time, float max_fall) {
    const double factor = 0.60000002384185791015625; // 00CEFF98 bytes
    const double gravity = 9.81000041961669921875;   // 00CF9058 bytes
    NativeLeaf out{};
    float range, fall, terminal;
    unsigned short saved, pc53;
    __asm { fnstcw saved }
    pc53 = static_cast<unsigned short>((saved & ~0x0300u) | 0x0200u);
    __asm {
        fld speed
        fmul time
        fmul factor
        fstp range
        fld max_fall
        fadd st(0), st(0)
        fmul gravity
        fstp fall
        fld fall
        fldcw pc53
        fsqrt
        fldcw saved
        fstp terminal
    }
    out.range = range;
    out.terminal = terminal;
    return out;
}

std::uint32_t bits(float value) {
    std::uint32_t out;
    std::memcpy(&out, &value, sizeof out);
    return out;
}
}

int main() {
    unsigned short saved, pc24;
    __asm { fnstcw saved }
    pc24 = static_cast<unsigned short>(saved & ~0x0300u);
    __asm { fldcw pc24 }
    // ArcadeTable[4], installed arcade/bulletclasses.lua: Fido's authored
    // WaterTravelSpeed/FlyTime/MaxFall. The second fixture changes FlyTime to
    // expose per-op rounding, and MaxFall to exercise the strict-positive gate.
    const float fixtures[][3] = {{30.8664f, 60.0f, 50.0f}, {30.8664f, 180.0f, 0.0f}};
    unsigned mismatches = 0;
    for (const auto& fixture : fixtures) {
        bsp::WeaponClassFinaliseInput in;
        in.sub_type = bsp::kProjectileSubTypeTorpedo;
        in.water_travel_speed = fixture[0];
        in.fly_time = fixture[1];
        in.max_fall = fixture[2];
        const auto expected = native_leaf(fixture[0], fixture[1], fixture[2]);
        for (const bool latched : {false, true}) {
            const auto actual = bsp::weapon_class_derive_engagement_range(in, latched);
            if (bits(actual.engagement_range) != bits(expected.range)
                || bits(actual.terminal_fall_speed) != bits(expected.terminal)
                || actual.accepted != (expected.terminal > 0.0f)
                || actual.sub_type != bsp::kProjectileSubTypeTorpedo) {
                ++mismatches;
                std::printf("mismatch latch=%d range=%08x/%08x terminal=%08x/%08x\n",
                    latched, bits(actual.engagement_range), bits(expected.range),
                    bits(actual.terminal_fall_speed), bits(expected.terminal));
            }
        }
    }
    __asm { fldcw saved }
    std::printf("00855A90 differential: 2 fixtures x 2 latch states, %u mismatches\n", mismatches);
    return mismatches ? 1 : 0;
}
