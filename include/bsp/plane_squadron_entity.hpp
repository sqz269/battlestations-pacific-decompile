#pragma once
// The plane squadron as the object the AI commands: the member array at
// +3D0h, the flight leader, the groupable-combatant answer 009FE080 takes for
// class 18h, and the member fan-out the squadron's own vtable performs.
//
// Addresses: 007F2C60 (the constructor that zeroes the five member slots at
// 007F2DA3..007F2DBB), 004F0AD0 (the scene creator), 007F4580 (the slot-39
// attach that fills the array; 007F4B43..007F4B6E is the per-wing tail),
// 007F4735/007F4754/007F4778 (the WingCount rule), 007ED610 (the flight-leader
// rotation, vtable 00D087C0 slot +164h arm BEh), 007ECF80 (vtable slot +128h,
// the member broadcast), 007ECFD0 (vtable slot +114h, the AI command block at
// squadron+348h), 007EDA90 (the exclusion 009FE080 negates), 007F3970 /
// 007F39ED (the removal and its compaction), 007F3500 (the clone).
//
// docs/PLANE_SQUADRON_ENTITY.md carries the listing evidence. The object
// layout itself is docs/PLANE_SQUADRON.md's table and is not re-derived here;
// this header adds only the fields the AI path reads and the four rules that
// move planes in and out of the member array.
//
// Nothing here is ABI-compatible with the native 0x414-byte block. The field
// names carry the native offsets so a reader can follow them back.

#include <cstddef>
#include <cstdint>

#include "bsp/ai_command_lifetime.hpp"

namespace bsp {

// +C4h, stamped by 007F2DEC. docs/ENTITY_CLASS_IDS.md row 18.
inline constexpr int kPlaneSquadronClassId = 0x18;

// MPlaneKamikaze, the id 007EDAA0 pushes through the lead plane's
// vtable[+5Ch]. docs/ENTITY_CLASS_IDS.md row 17, class test 009534A0.
inline constexpr int kPlaneSquadronKamikazeKindId = 0x17;

// The plane base 0Fh, the class every member answers and the class 009FE080
// does NOT admit on its own (009FE0A5 pushes 6, the ship base, not 0Fh).
inline constexpr int kPlaneSquadronMemberKindId = 0x0F;

// 007F2DA3..007F2DBB zero exactly +3D0h, +3D4h, +3D8h, +3DCh and +3E0h, and
// 007F3885 CMP EBX,4 / 007F3888 JA is the second witness: five slots.
inline constexpr std::size_t kPlaneSquadronMaxWings = 5;

// 007F4735: the key is absent -> 3. 007F4754: present -> max(1, authored).
inline constexpr int kPlaneSquadronDefaultWingCount = 3;

// The squadron fields the AI path touches. Offsets are the native ones.
struct PlaneSquadronEntity {
    int class_id{kPlaneSquadronClassId};   // +C4h
    int wing_count{0};                     // +3C8h, WingCount
    int live_count{0};                     // +3CCh, the live member count
    void* members[kPlaneSquadronMaxWings]{};  // +3D0h..+3E0h
    int behaviour{-1};                     // +364h, seeded -1 by 007F2DE0
    int attack_mode{1};                    // +370h, seeded 1, cleared by 007F31A0
    int avoid_layer_secondary{0};          // +34Ch
    int avoid_layer_primary{0};            // +350h
    float morale{1.0f};                    // +3E8h
    bool dirty{false};                     // +3ECh
    void* ai_command_block{nullptr};       // +348h, vtable[+114h] returns it
};

// 007F4735 / 007F4754 / 007F4778. `present` is whether the property bag
// carried `WingCount` (00CF8840); `authored` is the value at prop+0Ch.
int plane_squadron_wing_count_007f4754(bool present, int authored) noexcept;

// 007F4B43..007F4B6E, the per-wing tail of 007F4580's mode-1 loop:
//   plane+9D8h = squadron+3CCh   (the spawn-order id, never rewritten)
//   plane+9D4h = squadron        (the back pointer)
//   squadron+3D0h[count] = plane
//   squadron+3CCh += 1
//   squadron+3ECh = 1
// The native store at 007F4B55 has NO bound test; this returns false instead
// of writing past the fifth slot, which is the only deliberate divergence.
// `spawn_index` receives the value 007F4B43 stamps.
bool plane_squadron_attach_plane_007f4b43(PlaneSquadronEntity& squadron, void* plane,
                                          int* spawn_index) noexcept;

// +3D0h, the flight leader. 007ED610 is what makes slot 0 mean "leader".
void* plane_squadron_flight_leader(const PlaneSquadronEntity& squadron) noexcept;

// 007ED610, vtable 00D087C0 slot +164h arm BEh. Rotates members[index] into
// slot 0 by shifting the prefix up one; 007ED614 rejects index <= 0 and
// 007ED618 rejects index >= +3CCh, both without touching the array.
bool plane_squadron_promote_flight_leader_007ed610(PlaneSquadronEntity& squadron,
                                                   int index) noexcept;

// 007F3970's compaction, 007F39E0..007F3A1D: move the tail down over the
// removed slot, decrement +3CCh, zero the now-unused last slot, set +3ECh.
bool plane_squadron_remove_plane_007f39ed(PlaneSquadronEntity& squadron,
                                          void* plane) noexcept;

// The three reads 007EDA90 takes, all on the FLIGHT LEADER at +3D0h and not on
// a carrier: 007EDA91 loads [squadron+3D0h], 007EDAA0 asks that object
// IsKindOf(17h) = MPlaneKamikaze, 007EDAAA tests the byte at leader+C24h,
// which docs/ATTACK_GATE_TAILS.md establishes is the authored `PilotFires`.
struct PlaneSquadronLeadPlaneFacts {
    bool has_lead_plane{false};        // [squadron+3D0h] != 0
    bool lead_is_kamikaze_17{false};   // leader->vtable[+5Ch](17h)
    bool lead_pilot_fires_0c24{false}; // the byte at leader+C24h
};

// 007EDA90 itself, expressed on the leader facts. True only when all three
// hold in the shape the listing tests them. This is the SAME rule
// ai_squadron_excluded_007eda90 models; this overload exists so a caller that
// holds a squadron can answer it without inventing a "carrier link".
bool plane_squadron_excluded_007eda90(const PlaneSquadronLeadPlaneFacts& lead) noexcept;

// The bridge into 009FE080: a squadron fills the plane-squadron arm, never the
// ship-base arm (009FE0A0 is only reached when IsKindOf(18h) is false).
AiGroupableCombatantFacts plane_squadron_combatant_facts(
    const PlaneSquadronLeadPlaneFacts& lead) noexcept;

// 007ECF80, vtable 00D087C0 slot +128h, __thiscall(squadron, arg), RET 4:
//   for (i = 0; i < squadron->+3CCh; ++i)
//       members[i]->vtable[+128h](arg)
// Slot +128h is the skill-level setter (0089539A in BSP_LuaBinding_SetSkillLevel,
// 0082388E in BSP_UnitInstance_SEntityInit), so on a squadron the slot means
// "apply to every live member". `visit` is called once per live member, in
// array order, and the count of members visited is returned.
std::size_t plane_squadron_broadcast_to_members_007ecf80(
    const PlaneSquadronEntity& squadron,
    void (*visit)(void* member, void* context), void* context) noexcept;

}  // namespace bsp
