// Leaf helpers from the 00600000-0066FFFF band (cc13-sweep60). Reconstructed and build-tested
// only: not ABI-compatible, not fixture- or game-validated. See docs/CC13_BAND_0060_0066.md.

#include "bsp/band_0060_cc13.hpp"

namespace bsp::band_0060_cc13 {

float pulse_alpha_step_006501a0(PulseAlphaState& state, double top) noexcept {
    // FLD [value]; FSUB/FADD double [00D7A2F8]; FSTP [value] (006501A5..006501BB).
    const double stepped = state.falling ? static_cast<double>(state.value) - kPulseStep
                                         : static_cast<double>(state.value) + kPulseStep;
    state.value = static_cast<float>(stepped);
    const float value = state.value;
    // COMISS 0, value / JC (006501C3) jumps on 0 < value or unordered, so only an ordered
    // value <= 0 clears the flag; COMISS value, top / JC (006501E4) leaves it on value < top
    // or unordered.
    if (value <= 0.0F) {
        state.falling = false;  // 006501D0
        return value;
    }
    if (static_cast<double>(value) >= top) {
        state.falling = true;  // 006501ED
    }
    return value;
}

Vec2 world_to_map_00658580(const Vec2& map_size, const MapBounds& bounds, float x, float z) noexcept {
    // 006585BB..006585E3: t is stored to a float slot before the multiply.
    const float tx = static_cast<float>((static_cast<double>(x) - bounds.lo_x) /
                                        (static_cast<double>(bounds.hi_x) - bounds.lo_x));
    const float tz = static_cast<float>((static_cast<double>(z) - bounds.lo_z) /
                                        (static_cast<double>(bounds.hi_z) - bounds.lo_z));
    Vec2 out;
    out.x = static_cast<float>((static_cast<double>(map_size.x) - 0.0) * tx + 0.0);
    out.y = static_cast<float>((static_cast<double>(map_size.y) - 0.0) * tz + 0.0);
    return out;
}

namespace {

float clamp_axis(float value, float lo, float hi) noexcept {
    if (lo > value) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

}  // namespace

void clamp_to_screen_margins_00638b90(float& x, float& y, const ScreenMargins& margins, bool wide) noexcept {
    x = wide ? clamp_axis(x, margins.x_lo_wide, margins.x_hi_wide)  // 00638BA4..00638BC8
             : clamp_axis(x, margins.x_lo, margins.x_hi);           // 00638BD2..00638BF6
    y = clamp_axis(y, margins.y_lo, margins.y_hi);                  // 00638C11..
}

void rewards_step_page_00622e30(void* screen, int direction, RewardsStepPageHost& host) {
    if (direction > 0) {
        // 00622E39..00622E7B: i = page+1 while i < [+58h]+6, the bound re-read each pass.
        for (int i = host.screen_page_58(screen) + 1; i < host.screen_page_58(screen) + 6; ++i) {
            if (host.page_enabled_00e08c88(i % 6)) {
                host.set_screen_page_58(screen, i % 6);  // 00622E9F
                break;
            }
        }
    } else if (direction < 0) {
        // 00622EB0..00622EE5: i = [+58h]+5 while i > 0.
        for (int i = host.screen_page_58(screen) + 5; i > 0; --i) {
            if (host.page_enabled_00e08c88(i % 6)) {
                host.set_screen_page_58(screen, i % 6);  // 00622F09
                break;
            }
        }
    }
    // Every exit reloads +58h (or reuses the value just stored, 00622EA2) and tail-jumps.
    host.rewards_refresh_page_00621c60(screen, host.screen_page_58(screen));
}

int unit_class_icon_index_006440c0(int class_id) noexcept {
    switch (class_id) {
    case 0x07: return 0x00;
    case 0x08: return 0x01;
    case 0x09: return 0x02;
    case 0x0A: return 0x03;
    case 0x0B: return 0x05;
    case 0x0C: return 0x06;
    case 0x0D: return 0x08;
    case 0x0E: return 0x09;
    case 0x10:
    case 0x12: return 0x0A;
    case 0x11: return 0x0B;
    case 0x13:
    case 0x17: return 0x0D;
    case 0x14:
    case 0x15:
    case 0x16: return 0x0E;
    case 0x1B:
    case 0x1C: return 0x0F;
    case 0x45:
    case 0x46: return 0x10;
    default: return 0x12;
    }
}

std::uint8_t payload_icon_frame_00653200(std::uint32_t kind_mask, bool byte_82c, bool byte_808) noexcept {
    const auto is = [kind_mask](unsigned kind) noexcept { return ((kind_mask >> kind) & 1U) != 0U; };
    if (is(0x12)) return 0x0E;
    if (is(0x09)) return byte_82c ? 0x16 : 0x06;
    if (is(0x13) || is(0x17)) return 0x0F;
    if (is(0x10)) return 0x11;
    if (is(0x11)) return 0x13;
    if (is(0x16)) return 0x10;
    if (is(0x0E)) return 0x0D;
    if (is(0x15)) return 0x10;
    if (is(0x07)) return 0x04;
    if (is(0x0D)) return byte_808 ? 0x15 : 0x0C;
    if (is(0x0A)) return byte_808 ? 0x08 : 0x07;
    if (is(0x08)) return 0x05;
    if (is(0x0B)) return 0x09;
    return is(0x0C) ? 0x0B : 0x03;
}

int plane_payload_frame_00604c80(bool has_plane, int plane_970) noexcept {
    if (!has_plane) {
        return -1;
    }
    switch (plane_970) {
    case 0x09: return 0;
    case 0x0A: return 5;
    case 0x0B: return 1;
    case 0x0D: return 2;
    case 0x0F: return 3;
    case 0x12: return 4;
    default: return -1;
    }
}

int sign_float_0064a740(float value) noexcept {
    if (0.0F > value) {
        return -1;
    }
    if (value > 0.0F) {
        return 1;
    }
    return 0;
}

}  // namespace bsp::band_0060_cc13
