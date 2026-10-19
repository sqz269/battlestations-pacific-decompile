#pragma once
// Packet cc13-ver19: the anti-aircraft target pick, image against host.
// Evidence: docs/CC13_VER_19.md. The pick itself (00864FE0 and its scoring,
// ordering and walk) is already reconstructed in src/unit_gunnery_pass.cpp;
// this file holds only the callees that had no body there. Every routine is
// pure and takes the fields the image reads; nothing here is wired into a
// host. The names are hypotheses, not recovered symbols.
//
// - 008636A0: which fire-target provider (this+5Ch) and category gate
//   (this+60h) the unit gunnery AI installs.
// - 00863640: the director-backed provider's slot 4, with the formation
//   leader fallback.
// - 008627A0: the submarine category gate (slot 4 of 00D0D32C).
// - 00729BC0 up to its dispatch (00729BC0..00729C8A): the ammunition index,
//   the null and side-2 refusals and the range test. The dispatch onto the six
//   bot slots is bot_slot_for_projectile_kind_00729bc0 (unit_gunnery_pass.hpp).
#include <cstdint>

namespace bsp {

// D0D314 slot1: XOR EAX,EAX / RET. D0D31C slot1: MOV AL,1 / RET4.
std::uint32_t aa_pick_no_target_00861b90() noexcept;
bool aa_pick_accept_category_00861be0(void* self, std::uint32_t category) noexcept;
// Exact gunnery empty methods; stack cleanups 4, C, 0. These are complete native no-ops.
void gunnery_empty_one_00864520(void*, std::uint32_t) noexcept;
void gunnery_empty_three_00864530(void*, std::uint32_t, std::uint32_t, std::uint32_t) noexcept;
void gunnery_empty_zero_00864540(void*) noexcept;

inline constexpr std::uint32_t kAaPickInstallPoliciesAddr = 0x008636A0;
inline constexpr std::uint32_t kAaPickDirectorFireTargetAddr = 0x00863640;
inline constexpr std::uint32_t kAaPickSubmarineGateAddr = 0x008627A0;
inline constexpr std::uint32_t kAaPickGunSlotAcceptAddr = 0x00729BC0;

// The vtables 008636A0 stores. 00D0D314 slot 4 is 00861B90 (xor eax,eax) and
// 00D0D31C slot 4 is 00861BE0 (mov al,1); docs/UNIT_GUNNERY_PASS.md.
enum class AaPickFireTargetProvider { kNull_00D0D314, kDirector_00D0D324 };
enum class AaPickCategoryGate { kAlways_00D0D31C, kSubmarine_00D0D32C };

struct AaPickPolicyInputs {
    bool unit_is_kind_2 = false;            // unit->vtable[5Ch](2), 008636AD
    bool unit_has_director = false;         // unit->vtable[114h]() != 0, 008636C2
    bool director_answers_48h_2 = false;    // director->vtable[48h](2), 008636CF
    bool unit_is_kind_8 = false;            // unit->vtable[5Ch](8), 0086371E
};

struct AaPickPolicies {
    AaPickFireTargetProvider fire_target = AaPickFireTargetProvider::kNull_00D0D314;
    AaPickCategoryGate gate = AaPickCategoryGate::kAlways_00D0D31C;
};

// 008636A0, __thiscall(ai)(unit), the two installs only (allocation failures
// are not modelled).
AaPickPolicies aa_pick_install_policies_008636a0(const AaPickPolicyInputs& in) noexcept;

// 00863640, __thiscall(provider)() -> entity*. `own` is director->vtable[2Ch]()
// (008364E0, director+238h). On null it follows [director+34h] through
// 007788D0 ([[unit+284h]+14h], the formation leader) and, when that leader
// answers IsKindOf(6), returns [leader+738h]->vtable[2Ch](). Handles are
// opaque; null is "no target".
struct AaPickDirectorFireTargetInputs {
    const void* own = nullptr;               // 0086364C..00863654
    bool director_has_unit = false;          // [director+34h] != 0, 0086365D
    const void* formation_leader = nullptr;  // 007788D0, 00863664
    bool leader_is_ship = false;             // leader->vtable[5Ch](6), 00863674
    const void* leader_fire_target = nullptr;  // [leader+738h]->vtable[2Ch], 0086367E
};
const void* aa_pick_director_fire_target_00863640(
    const AaPickDirectorFireTargetInputs& in) noexcept;

// 008627A0, __thiscall(gate)(int category) -> AL, gate+4 = the submarine.
// Category 7 asks 008527E0 (shallow enough to launch); 1..6 ask 008527A0
// (above deck-gun depth); every other category answers false.
bool aa_pick_submarine_category_gate_008627a0(int category,
                                              bool above_deck_gun_depth_008527a0,
                                              bool shallow_enough_008527e0) noexcept;

struct AaPickGunSlotInputs {
    int gun_weapon_kind = 0;           // [[gun+3F4h]+80h], the device Function id
    bool target_present = false;       // target != 0, 00729BF1
    bool target_is_plane = false;      // target->vtable[5Ch](0Fh), 00729BE2
    bool target_is_kind_5 = false;     // target->vtable[5Ch](5), 00729C05
    int target_side = 0;               // [target+54h], 00729C0F
    float gun_to_target_distance = 0;  // |gun+FCh - target+FCh|, 00729C35..00729C7A
    float ammo_max_range[2] = {0, 0};  // [ammo record i]+60h, i = 0 / 1
};
struct AaPickGunSlotResult {
    bool reaches_dispatch = false;     // true: 00729C8C runs the bot-slot dispatch
    int ammo_index = 0;                // 00729BD2..00729BEC, the record dispatched on
};
// 00729BC0..00729C8A. The ammunition record is [[gun+3F4h]+74h] + index*48h +
// 34h; index 1 only for a kind-6 gun against a plane. The range test is
// FCOMI + JA on (distance, range): distance > range refuses, NaN passes.
AaPickGunSlotResult aa_pick_gun_slot_pre_dispatch_00729bc0(
    const AaPickGunSlotInputs& in) noexcept;

}  // namespace bsp
