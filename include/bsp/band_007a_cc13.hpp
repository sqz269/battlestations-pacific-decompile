#pragma once

// Leaf helpers from the 007A0000-007FFFFF gameplay band (cc13-gp7a, docs/CC13_GP_007A_007F.md): the
// plane squadron create message's type predicate (a row for the predicate model of
// bsp/band_006b_cc13.hpp), the plane actuator-channel setters and predicates, plane-state and
// squadron predicates, the plane launch delay, an AABB centre, the map-bounds clamp, the flight
// controller's rate update and the control-surface index from a part name. New C++ interfaces, not
// the native register ABI; nothing here is wired into a game host. Names are hypotheses, not
// recovered symbols; each declaration states the original address and ABI.
//
// x87 model (as in bsp/band_006b_cc13.hpp): 53-bit precision, so an FSTP to a float slot is a float
// cast and unspilled intermediates are doubles. SSE compares (UCOMISS, LAHF, TEST AH,44h, JNP) are
// IEEE equality: a NaN never compares equal.

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "bsp/band_006b_cc13.hpp"

namespace bsp::band_007a_cc13 {

using band_006b_cc13::TypeIdRule;

// 007EE080 BSP_SessionMessage_PLANESQUADRON_CREATE_IsType, slot +0Ch of 00D08740: `MOV EAX,[ESP+4]`,
// CMP/JE against BBh, 47h, 46h, RET 4. Evaluate with band_006b_cc13::type_id_rule_accepts.
extern const TypeIdRule kTypeIdRules007A[];
extern const std::size_t kTypeIdRule007ACount;

// One channel of the plane actuator block ([plane+DECh]): enabled byte +0h, target byte +1h, value
// float +4h, dirty byte +8h. The block holds channel B at +28h (the gear, SQUADRON_LAND_TASK.md),
// channel C at +44h and a channel at +60h, and a block-dirty byte at +11h.
struct ActuatorChannel {
    std::uint8_t enabled;
    std::uint8_t target;
    float value;
    std::uint8_t dirty;
};

// 007B8D10 BSP_Plane_ActuatorCAtOne, __fastcall(ECX plane) -> EAX: 1 unless channel C is enabled and
// its value is not 1.0f (00D7A24C). 007B8D40 BSP_Plane_ActuatorCAtZero: the same against 0.0f
// (00D7A218). Both compare with UCOMISS, so a NaN value answers 0 when enabled.
bool actuator_channel_at_value_007b8d10(const ActuatorChannel& channel, float wanted) noexcept;

// 007B8DA0 BSP_Plane_ActuatorCRetract (target 0) and 007B8DC0 BSP_Plane_ActuatorCExtend (target 1),
// __fastcall(ECX plane) on channel C; 007B8840 BSP_ActuatorBlock_SetChannel60Target,
// __thiscall(ECX block, char target), RET 4, on the +60h channel. Each: when the target byte differs,
// store it and set the channel dirty; then, when the channel is dirty, set the block dirty.
void actuator_channel_set_target_007b8da0(ActuatorChannel& channel, std::uint8_t target,
                                          std::uint8_t& block_dirty) noexcept;

// 007B8DE0 BSP_Plane_ActuatorCResetToZero, __fastcall(ECX plane): when the target is set or the value
// is not 0.0f (a NaN is not equal), value = 0, target = 0, dirty = 1; then the block-dirty rule.
void actuator_channel_reset_to_zero_007b8de0(ActuatorChannel& channel, std::uint8_t& block_dirty) noexcept;

// 007B8860 BSP_ActuatorBlock_SetGearChannelInstant, __thiscall(ECX block, char down), RET 4, on the
// +28h channel: v = down ? 1.0f : 0.0f; when the target differs or the value is not v (FUCOMIP), value
// = v, target = down, dirty = 1; then the block-dirty rule.
void actuator_channel_set_instant_007b8860(ActuatorChannel& channel, std::uint8_t down,
                                           std::uint8_t& block_dirty) noexcept;

// 007B8BD0 BSP_Plane_IsFreeFlightOrParked, __fastcall(ECX plane) -> EAX 0/1: the control mode's
// vtable[38h] answer (free flight, passed in), or flight state +900h == 6, or state 4 with the ground
// contact byte +BF8h. State 5 answers 0 even with contact.
bool plane_free_flight_or_parked_007b8bd0(bool free_flight, std::int32_t flight_state,
                                          bool ground_contact) noexcept;

// 007B95E0 BSP_Plane_IsSquadronLeader, __fastcall(ECX plane) -> EAX 0/1: spawn index +9D8h == 0, a
// squadron at +9D4h, and the squadron's +3BCh (its active-wingmen count, written by 007EE790) > 0.
bool plane_is_leader_with_wingmen_007b95e0(std::int32_t spawn_index, bool has_squadron,
                                           std::int32_t squadron_active_wingmen) noexcept;

// 007B83F0 BSP_Plane_SetLaunchDelayAA0, __thiscall(ECX plane, char flag), RET 4: the value stored at
// +AA0h, 5.0f (00CE3850) when the flag is set, else -1.0f (00D7A260).
float plane_launch_delay_007b83f0(bool flag) noexcept;

// 007B8F20 BSP_Aabb_Center, __thiscall(ECX box[6] = min xyz, max xyz, float* out), RET 4: each lane
// FADD-ed and stored to a float, then multiplied by the double 0.5 (00D7A280) and stored.
void aabb_center_007b8f20(const float box[6], float out[3]) noexcept;

// 007EF8F0 BSP_World_ClampPointToMapBounds, __thiscall(ECX world, float* out, const float* in),
// RET 8, EAX = out. x is clamped to [world+711Ch + 1, world+7128h - 1] and z to
// [world+7130h + 1, world+7124h - 1], each bound stored to a float first; y is copied. The low bound
// is tested first (FCOMI lo, x; JBE keeps x), then the high (x <= hi keeps x), so a NaN survives.
void world_clamp_point_to_map_bounds_007ef8f0(const float in[3], float bound_711c, float bound_7124,
                                              float bound_7128, float bound_7130, float out[3]) noexcept;

// The fields 007D80C0 reads and writes in the plane flight controller.
struct FlightControllerRateFields {
    float f44;
    float f54[3];
    float f60[3];
    float rate_78;
    float previous_7c;
};

// 007D80C0 BSP_PlaneFlightController_UpdateRateOf44, __thiscall(ECX controller, float dt), RET 4.
// dt != 0 (UCOMISS, so a NaN takes this arm): rate = (f44 - previous) / dt with the FSUB/FDIV on
// float operands, previous = f44. dt == 0: rate = 0, previous = f44, f60 = the image's zero vector
// global 00F87574 (passed in) and f54 = f60.
void flight_controller_update_rate_007d80c0(FlightControllerRateFields& fields, float dt,
                                            const float zero_vector_f87574[3]) noexcept;

// 007E1D80 BSP_PlaneControlSurface_IndexFromName, __fastcall(ECX part) -> EAX. Every test is a
// whole-string equality: 004BEB60(pos 0, count = the name's size +14h, literal, literal length),
// directly for "aileron_l" (0), "aileron_r" (1) and "rudder" (3), and through
// BSP_CommandLine_MatchesToken (004C2DF0, the same call with strlen) for "elevator", "elevator_l" and
// "elevator_r" (2 each); anything else answers 5 (NEG/SBB/AND -3/ADD 5).
std::int32_t plane_control_surface_index_007e1d80(std::string_view part_name) noexcept;

} // namespace bsp::band_007a_cc13
