// Leaf helpers from the 00710000-0078FFFF gameplay band (cc13-gp71).
// Reconstructed and build-tested only: not ABI-compatible, not fixture- or game-validated, and not
// wired into any game host. See docs/CC13_GP_0071_0078.md.

#include "bsp/band_0071_cc13.hpp"

namespace bsp::band_0071_cc13 {

const TypeIdRule kTypeIdRules0071[] = {
    {0x0071C5B0u, 0x00CFD99Cu, 3, {0x59, 0x49, 0x46}},       // BSP_GunBotBase_AcceptsMessageGameUnitMsg (slot +20h)
    {0x00728BB0u, 0x00CFDE1Cu, 3, {0xAB, 0x47, 0x46}},       // MT_GUN_CREATE
    {0x00728DB0u, 0x00CFDE3Cu, 3, {0xAC, 0x49, 0x46}},       // MT_GUN_SET_FIRE_MODE
    {0x00728E90u, 0x00CFDE50u, 3, {0xAF, 0x49, 0x46}},       // MT_R_T_GUN_SET_CONTFIRING
    {0x00729190u, 0x00CFDE8Cu, 3, {0xB0, 0x49, 0x46}},       // MT_S_T_GUN_FIRE
    {0x00744900u, 0x00CFF37Cu, 2, {0xD1, 0x46}},             // MT_LANDFORT_EFFECT
    {0x00749CA0u, 0x00CFF8E8u, 4, {0xA4, 0x8A, 0x47, 0x46}}, // MT_LANDINGSHIP_CREATE
    {0x00749DF0u, 0x00CFF908u, 2, {0xA5, 0x46}},             // MT_LANDINGSHIP_STARTLANDING
    {0x0074CED0u, 0x00CFFD18u, 2, {0x48, 0x46}},             // MT_GRNDVEHICLE_CREATE slot +2Ch: MT_SYNC
    {0x0074CF70u, 0x00CFFD18u, 3, {0xCD, 0x47, 0x46}},       // MT_GRNDVEHICLE_CREATE
    {0x0074CFD0u, 0x00CFFD4Cu, 3, {0xCE, 0x48, 0x46}},       // MT_GRND_VEHICLE_SYNC
    {0x0075A510u, 0x00D02D80u, 2, {0xBA, 0x46}},             // MT_KILL_BULLET
    {0x0075A880u, 0x00D02DBCu, 3, {0xD9, 0x49, 0x46}},       // MT_PATH_SET
    {0x0075A8F0u, 0x00D02DD0u, 3, {0xDA, 0x49, 0x46}},       // MT_PATH_ADD
    {0x0075A960u, 0x00D02DE4u, 3, {0xDB, 0x49, 0x46}},       // MT_PATH_INSERT
    {0x0075A9D0u, 0x00D02DF8u, 3, {0xDC, 0x49, 0x46}},       // MT_PATH_DELETE
    {0x0075B260u, 0x00D02EC0u, 2, {0xBF, 0x46}},             // MT_PLAYERSPAWNPOINT_SETACTIVE
    {0x007590B0u, 0x00D0189Cu, 4, {0x8B, 0x8A, 0x47, 0x46}}, // MT_MOTHERSHIP_CREATE
    {0x00767710u, 0x00D0377Cu, 3, {0xD8, 0x47, 0x46}},       // MT_PATH_CREATE
    {0x007677E0u, 0x00D0379Cu, 3, {0xC0, 0x47, 0x46}},       // MT_PLANE_CREATE
    {0x00767AA0u, 0x00D037BCu, 3, {0xB8, 0x47, 0x46}},       // MT_TORPEDO_CREATE
    {0x00767C00u, 0x00D037E0u, 4, {0xB9, 0xB6, 0x47, 0x46}}, // MT_PARATROOPER_CREATE
    {0x00767D40u, 0x00D03804u, 3, {0xDF, 0x47, 0x46}},       // MT_MNETTABLESYNCENTITY_CREATE
    {0x00749010u, 0x00CFF7CCu, 2, {0x05, 0x04}},             // BSP_VehicleClass_IsKindOf (slot +18h)
};
const std::size_t kTypeIdRule0071Count = sizeof(kTypeIdRules0071) / sizeof(kTypeIdRules0071[0]);

const TypeIdRule* find_type_id_rule_0071(std::uint32_t address) noexcept {
    for (std::size_t i = 0; i < kTypeIdRule0071Count; ++i) {
        if (kTypeIdRules0071[i].address == address) {
            return &kTypeIdRules0071[i];
        }
    }
    return nullptr;
}

bool weapon_director_slot_clearable_0071deb0(std::int32_t command_category, std::int32_t slot) noexcept {
    // 0071DEBB..0071DECA: CMP EAX,1 / JZ; CMP EAX,2 / JNZ false; CMP [ESP+8],0 / JLE false.
    return (command_category == 1 || command_category == 2) && slot > 0;
}

bool plane_state_admits_ai_0074e230(std::int32_t plane_state) noexcept {
    return plane_state == 7 || plane_state == 6 || plane_state == 4 || plane_state == 5;
}

float unit_contact_age_seconds_00779ad0(std::int32_t current_frame, std::int32_t contact_frame) noexcept {
    // SUB in 32-bit integer arithmetic (wraps), FILD, FMUL float, FSTP float.
    const std::int32_t frames = static_cast<std::int32_t>(static_cast<std::uint32_t>(current_frame) -
                                                          static_cast<std::uint32_t>(contact_frame));
    constexpr float kFrameSeconds = 0.05F; // 00D0DE84, bits 3D4CCCCD
    return static_cast<float>(static_cast<double>(frames) * static_cast<double>(kFrameSeconds));
}

bool gun_any_barrel_ready_within_00727d30(const float* timers, std::int32_t count, float t) noexcept {
    for (std::int32_t i = 0; i < count; ++i) {
        if (t >= timers[i]) {
            return true;
        }
    }
    return false;
}

bool gun_any_barrel_timer_below_max_00729a40(const float* timers, std::int32_t count) noexcept {
    constexpr double kThreshold = 3.4028234663852886e38; // 00D7A278
    for (std::int32_t i = 0; i < count; ++i) {
        if (kThreshold > static_cast<double>(timers[i])) {
            return true;
        }
    }
    return false;
}

float leak_manager_flooded_fraction_0074fa80(float water, float settings_404, float capacity_0c) noexcept {
    // FLD [ESI+28h]; FSTP float (no change); FLD [EAX+404h]; FMUL [ESI+0Ch]; FDIVP; FSTP float.
    const double denominator = static_cast<double>(settings_404) * static_cast<double>(capacity_0c);
    return static_cast<float>(static_cast<double>(water) / denominator);
}

std::int32_t mission_narrative_size_00733260(std::int32_t list_count, bool entry_showing) noexcept {
    return (entry_showing ? 1 : 0) + list_count;
}

} // namespace bsp::band_0071_cc13
