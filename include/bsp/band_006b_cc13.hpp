#pragma once

// Leaf helpers from the 006B0000-0070FFFF gameplay band (cc13-gp6b, docs/CC13_GP_006B_0070.md):
// session-message and projectile-class type predicates, air-ops slot counters and resets, the
// runway approach setback, two float/int clamps, a 4x4 in-place transpose and the part lookup by
// type. New C++ interfaces, not the native register ABI; nothing here is wired into a game host.
// Names are hypotheses, not recovered symbols; each declaration states the original address and
// ABI.
//
// x87 model (as in bsp/band_0060_cc13.hpp and bsp/band_0067_cc13.hpp): the image runs with
// 53-bit precision, so an FSTP to a float slot is a float cast and unspilled intermediates are
// doubles.

#include <cstddef>
#include <cstdint>

#include "bsp/air_operations.hpp"

namespace bsp::band_006b_cc13 {

// ---------------------------------------------------------------------------------------------
// Type predicates
// ---------------------------------------------------------------------------------------------
// Every session message class answers slot +0Ch (the "is it this kind" test) with a short chain
// of full-DWORD compares: its own id, then the ids of the bases it derives from (62h for the
// airfield messages, 47h for the network entity-create messages, 49h for ordinary unit messages,
// 46h for all). The projectile class records answer slot +8h the same way with entity class ids.
// Each body is `MOV EAX,[ESP+4]` then `CMP EAX,id / JZ` per id, `XOR EAX,EAX; RET 4` on a miss and
// `MOV EAX,1; RET 4` on a hit (read at 006BC410 and 006E3B80; the others from the pseudocode); the
// receiver (ECX) is ignored. The tables keep the ids in the order the code tests them.
struct TypeIdRule {
    std::uint32_t address;   // the predicate's entry point
    std::uint32_t vtable;    // the table whose slot holds it
    std::uint8_t count;      // number of ids accepted
    std::uint8_t ids[5];     // accepted ids, in test order
};

// The session-message predicates of this band (slot +0Ch of each message vtable).
extern const TypeIdRule kSessionMessageTypeRules[];
extern const std::size_t kSessionMessageTypeRuleCount;

// The projectile-class record predicates of this band (slot +8h of each class-record vtable).
extern const TypeIdRule kProjectileClassKindRules[];
extern const std::size_t kProjectileClassKindRuleCount;

// True when `type` equals one of the rule's ids (the native full-DWORD compare, so a type above
// FFh never matches).
bool type_id_rule_accepts(const TypeIdRule& rule, std::int32_t type) noexcept;

// The rule whose predicate starts at `address`, or null.
const TypeIdRule* find_type_id_rule(std::uint32_t address) noexcept;

// ---------------------------------------------------------------------------------------------
// Air-ops slots (bsp::AirOpsSlot, the 58h records at block+4Ch, count at block+50h)
// ---------------------------------------------------------------------------------------------

// 006BDC30 BSP_AirOps_CountIdleSlots, __fastcall(ECX block) -> EAX: slots whose state (+2Ch) is
// 1 or 5, the two "planes on the deck" states. No lock is taken.
int air_ops_count_idle_slots_006bdc30(const AirOpsSlot* slots, int slot_count) noexcept;

// 006BF4C0 BSP_AirOps_CountPlanesAirborne, __fastcall(ECX block) -> EAX, unreferenced. A slot with
// a squadron (+28h) in state 3 or 4 contributes the squadron's live count (squadron+3CCh, passed
// in `launched_plane_counts`); otherwise a slot in state 6 contributes its count (+8h).
int air_ops_count_planes_airborne_006bf4c0(const AirOpsSlot* slots,
                                           const std::int32_t* launched_plane_counts,
                                           int slot_count) noexcept;

// 006BF540 BSP_AirOps_CountPlanesAirborneOrQueued: as 006BF4C0, but state 2 (queued) slots also
// contribute their count.
int air_ops_count_planes_airborne_or_queued_006bf540(const AirOpsSlot* slots,
                                                     const std::int32_t* launched_plane_counts,
                                                     int slot_count) noexcept;

// 006BD360 BSP_AirOps_ResetSlotsToDeck, __fastcall(ECX block), unreferenced, under the block+0Ch
// critical section (not modelled). Per slot: state 1, timer 0, but timer 5.0 (00CE3850) and the
// request byte cleared when +34h was set; class (+4h) and its +10h copy cleared when a class was
// set; count (+8h) cleared.
void air_ops_reset_slots_to_deck_006bd360(AirOpsSlot* slots, int slot_count) noexcept;

// 006C4F30 BSP_AirOpsSlot_Clear, __fastcall(ECX slot), unreferenced: unregisters the squadron
// observer at +28h and clears it (the observer pair is not modelled), count 0, state 1, timer 5.0
// (00CE3850), request byte 0.
void air_ops_slot_clear_006c4f30(AirOpsSlot& slot) noexcept;

// 006BA620 BSP_AirOpsHolder_ApproachSetback, __fastcall(ECX holder) -> ST0: RunwayLength
// (holder+B4h) times the double 0.30000001192092896 (00CE3DC8) when the owner answers class 9
// (MMothership, through (holder+4)->+7Ch vtable[5Ch](9)), else times 0.4000000059604645
// (00CE65D0); FSTP to a float before the return.
float air_ops_approach_setback_006ba620(float runway_length, bool owner_is_mothership) noexcept;

// ---------------------------------------------------------------------------------------------
// Small math
// ---------------------------------------------------------------------------------------------

// 006F22B0 BSP_Math_MinFloatInt, __thiscall(ECX bound, float v) -> ST0, RET 4. bound is FILD then
// FSTP to a float; FCOMIP bound, v: returns v when bound > v, otherwise bound (also for NaN).
float math_min_float_int_006f22b0(std::int32_t bound, float v) noexcept;

// 006F22F0 BSP_Math_MaxFloatInt, same ABI: returns v when v > float(bound), otherwise bound (also
// for NaN).
float math_max_float_int_006f22f0(std::int32_t bound, float v) noexcept;

// 006E0540 BSP_Vector3f_AddScaled_006e0540, __thiscall(ECX v, d*, float s), RET 8, unreferenced.
// The three products d*s are stored to float temporaries first, then v += product per component
// (FMUL bytes D8 C9 / DE CA read at 006E0559 / 006E054F). Same operation as 00709B60, which
// bsp/band_0067_cc13.hpp already reconstructs; this entry delegates to it.
void vec3_add_scaled_in_place_006e0540(float v[3], const float d[3], float s) noexcept;

// 006FDD00 BSP_Matrix4x4_TransposeInPlace_006fdd00, __fastcall(ECX m) -> EAX m. The six
// off-diagonal swaps; each pair moves one value through MOVSS and the other through FLD/FSTP
// float, which quiets a signalling NaN in the second element of a pair (not modelled: the C++
// swap copies bits).
float* matrix4x4_transpose_in_place_006fdd00(float m[16]) noexcept;

// 006D2120 BSP_Entity_NthPartOfType0C, __thiscall(ECX entity, n) -> EAX, RET 4: walks the part
// pointer vector at entity+348h/+34Ch and returns the address of the n-th entry whose part has
// type 0Ch at +4, else 0. Modelled on the part types: the index of that entry, or -1.
int entity_nth_part_of_type_0c_006d2120(const std::int32_t* part_types, std::size_t part_count,
                                        int n) noexcept;

} // namespace bsp::band_006b_cc13
