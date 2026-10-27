// Leaf helpers from the 007A0000-007FFFFF gameplay band (cc13-gp7a).
// Reconstructed and build-tested only: not ABI-compatible, not fixture- or game-validated, and not
// wired into any game host. See docs/CC13_GP_007A_007F.md.

#include "bsp/band_007a_cc13.hpp"

namespace bsp::band_007a_cc13 {

const TypeIdRule kTypeIdRules007A[] = {
    {0x007EE080u, 0x00D08740u, 3, {0xBB, 0x47, 0x46}}, // MT_PLANESQUADRON_CREATE
};
const std::size_t kTypeIdRule007ACount = sizeof(kTypeIdRules007A) / sizeof(kTypeIdRules007A[0]);

namespace {

// The tail every channel setter shares: CMP byte [channel+dirty],0 / JE / MOV byte [block+11h],1.
void propagate_dirty(const ActuatorChannel& channel, std::uint8_t& block_dirty) noexcept {
    if (channel.dirty != 0) {
        block_dirty = 1;
    }
}

} // namespace

// Documented deviation (cc13-D3): one body for 007B8D10 (wanted = 1.0f, 00D7A24C) and
// 007B8D40 (0.0f); the image hardwires the constant and reads the channel from
// [plane+DECh]+44h itself. For the address's own constant the result is identical,
// NaN included (UCOMISS unordered -> PF set -> false; C++ == is false).
bool actuator_channel_at_value_007b8d10(const ActuatorChannel& channel, float wanted) noexcept {
    // 007B8D19 CMP byte [EAX],0 / JE true; 007B8D1E UCOMISS value, wanted; LAHF; TEST AH,44h; JNP true.
    if (channel.enabled == 0) {
        return true;
    }
    return channel.value == wanted;
}

// Documented deviation (cc13-D3): one body for 007B8DA0 (target 0: CMP [+45h],0 / MOV
// [+45h],0 / MOV [+4Ch],1 / CMP [+4Ch],0 / MOV [+11h],1) and 007B8DC0 (target 1); the
// image hardwires the target. With target 0 the stores and their order are the image's.
void actuator_channel_set_target_007b8da0(ActuatorChannel& channel, std::uint8_t target,
                                          std::uint8_t& block_dirty) noexcept {
    // 007B8DA9 CMP byte [EAX+45h],0 / JE (retract); 007B8DC9 CMP [EAX+45h],1 / JE (extend).
    if (channel.target != target) {
        channel.target = target;
        channel.dirty = 1;
    }
    propagate_dirty(channel, block_dirty);
}

void actuator_channel_reset_to_zero_007b8de0(ActuatorChannel& channel, std::uint8_t& block_dirty) noexcept {
    // 007B8DE6 CMP byte [ECX+45h],0 / JNE store; else UCOMISS value, 0 / TEST AH,44h / JNP skip.
    if (channel.target != 0 || !(channel.value == 0.0F)) {
        channel.value = 0.0F;
        channel.target = 0;
        channel.dirty = 1;
    }
    propagate_dirty(channel, block_dirty);
}

void actuator_channel_set_instant_007b8860(ActuatorChannel& channel, std::uint8_t down,
                                           std::uint8_t& block_dirty) noexcept {
    // 007B8864 TEST DL,DL selects 1.0f (00D7A24C) or XORPS 0; 007B8878 CMP [ECX+29h],DL / JNE store;
    // FLD v, FLD value, FUCOMIP, LAHF, TEST AH,44h, JNP skip.
    const float v = down != 0 ? 1.0F : 0.0F;
    if (channel.target != down || !(channel.value == v)) {
        channel.value = v;
        channel.target = down;
        channel.dirty = 1;
    }
    propagate_dirty(channel, block_dirty);
}

bool plane_free_flight_or_parked_007b8bd0(bool free_flight, std::int32_t flight_state,
                                          bool ground_contact) noexcept {
    // 007B8BE1 TEST AL,AL / JNE true; CMP state,6 / JE true; state 4 or 5 reach 007B8BFD, where a clear
    // +BF8h answers 0 and only state 4 answers 1 (CMP EAX,5 / JNE true).
    if (free_flight || flight_state == 6) {
        return true;
    }
    return flight_state == 4 && ground_contact;
}

bool plane_is_leader_with_wingmen_007b95e0(std::int32_t spawn_index, bool has_squadron,
                                           std::int32_t squadron_active_wingmen) noexcept {
    // CMP [ECX+9D8h],0 / JNE; TEST squadron / JE; CMP [EAX+3BCh],0 / JLE.
    return spawn_index == 0 && has_squadron && squadron_active_wingmen > 0;
}

// Documented deviation (cc13-D3): 007B83F0 stores the value to plane+AA0h itself
// (007B83FF / 007B8412, MOVSS [ECX+0AA0h]) and returns nothing; this returns the same
// float for the caller to store (host-ABI glue, no plane object here).
float plane_launch_delay_007b83f0(bool flag) noexcept {
    constexpr float kLaunchDelay = 5.0F;   // 00CE3850, bits 40A00000
    constexpr float kNoLaunchDelay = -1.0F; // 00D7A260, bits BF800000
    return flag ? kLaunchDelay : kNoLaunchDelay;
}

void aabb_center_007b8f20(const float box[6], float out[3]) noexcept {
    constexpr double kHalf = 0.5; // 00D7A280, a double
    for (int lane = 0; lane < 3; ++lane) {
        // FLD [ECX+0Ch+4*lane] / FADD [ECX+4*lane] / FSTP float, then FMUL by the double and FSTP.
        const float sum = static_cast<float>(static_cast<double>(box[lane + 3]) + static_cast<double>(box[lane]));
        out[lane] = static_cast<float>(static_cast<double>(sum) * kHalf);
    }
}

namespace {

// 007EF92A..007EF958 (x) and 007EF975..007EF9BE (z): FCOMI lo, v / JBE keeps v for the high test,
// else lo; FCOMI v, hi / JBE keeps v, else hi. Unordered compares take JBE.
float clamp_lane(float v, float lo, float hi) noexcept {
    if (lo > v) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

} // namespace

void world_clamp_point_to_map_bounds_007ef8f0(const float in[3], float bound_711c, float bound_7124,
                                              float bound_7128, float bound_7130, float out[3]) noexcept {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    // FLD [7128h], FLD1, FSUB st(1),st; FSTP float (hi); FLD [711Ch], FADD st(1); FSTP float (lo).
    const float hi_x = static_cast<float>(static_cast<double>(bound_7128) - 1.0);
    const float lo_x = static_cast<float>(static_cast<double>(bound_711c) + 1.0);
    out[0] = clamp_lane(out[0], lo_x, hi_x);
    // The 1.0 left on the stack: FLD [7124h], FSUB st,st(1) (hi); FADD [7130h] (lo).
    const float hi_z = static_cast<float>(static_cast<double>(bound_7124) - 1.0);
    const float lo_z = static_cast<float>(1.0 + static_cast<double>(bound_7130));
    out[2] = clamp_lane(out[2], lo_z, hi_z);
}

void flight_controller_update_rate_007d80c0(FlightControllerRateFields& fields, float dt,
                                            const float zero_vector_f87574[3]) noexcept {
    // 007D80C4 UCOMISS dt, 0 / LAHF / TEST AH,44h / JNP reset.
    if (!(dt == 0.0F)) {
        // FLD previous; FSTP float; FLD f44 (via a float slot); FSUB float; FDIV float dt; FSTP float.
        const float previous = fields.previous_7c;
        fields.previous_7c = fields.f44;
        // Modelled at the main thread's 24-bit PC (each x87 op rounds to a float);
        // at 53 bits the image can differ by a double rounding of the quotient.
        const float diff = static_cast<float>(static_cast<double>(fields.f44) - static_cast<double>(previous));
        fields.rate_78 = static_cast<float>(static_cast<double>(diff) / static_cast<double>(dt));
        return;
    }
    fields.rate_78 = 0.0F;
    fields.previous_7c = fields.f44;
    for (int i = 0; i < 3; ++i) {
        fields.f60[i] = zero_vector_f87574[i];
    }
    for (int i = 0; i < 3; ++i) {
        fields.f54[i] = fields.f60[i];
    }
}

std::int32_t plane_control_surface_index_007e1d80(std::string_view part_name) noexcept {
    if (part_name == "aileron_l") { // 00D06184
        return 0;
    }
    if (part_name == "aileron_r") { // 00D06178
        return 1;
    }
    if (part_name == "rudder") { // 00D06170
        return 3;
    }
    if (part_name == "elevator" || part_name == "elevator_l" || part_name == "elevator_r") { // 00CF8AB0, 00D06C7C, 00D06C70
        return 2;
    }
    return 5;
}

} // namespace bsp::band_007a_cc13
