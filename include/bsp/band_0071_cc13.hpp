#pragma once

// Leaf helpers from the 00710000-0078FFFF gameplay band (cc13-gp71, docs/CC13_GP_0071_0078.md):
// the band's session-message and class-kind type predicates (as rows for the predicate model of
// bsp/band_006b_cc13.hpp), weapon-director and plane-state predicates, gun barrel-timer scans, the
// contact age, the leak manager's flooded fraction and the mission narrative size. New C++
// interfaces, not the native register ABI; nothing here is wired into a game host. Names are
// hypotheses, not recovered symbols; each declaration states the original address and ABI.
//
// x87 model (as in bsp/band_006b_cc13.hpp): 53-bit precision, so an FSTP to a float slot is a float
// cast and unspilled intermediates are doubles.

#include <cstddef>
#include <cstdint>

#include "bsp/band_006b_cc13.hpp"

namespace bsp::band_0071_cc13 {

using band_006b_cc13::TypeIdRule;

// Type predicates of this band (each `MOV EAX,[ESP+4]`, then `CMP EAX,id / JE` per id, RET 4; ids read
// from the bytes with capstone). Session messages at slot +0Ch of their vtables, plus three other
// predicates of the same shape (the gun-bot base's message test, the create message's MT_SYNC test
// at +2Ch, and the vehicle-class IsKindOf). Evaluate with band_006b_cc13::type_id_rule_accepts.
extern const TypeIdRule kTypeIdRules0071[];
extern const std::size_t kTypeIdRule0071Count;

// The rule whose predicate starts at `address`, or null.
const TypeIdRule* find_type_id_rule_0071(std::uint32_t address) noexcept;

// 0071DEB0 BSP_WeaponDirector_SlotClearable, __thiscall(ECX ignored, command*, int slot), RET 8:
// command->vtable[0Ch]() (the category) is 1 or 2 and slot > 0. The category is passed in.
bool weapon_director_slot_clearable_0071deb0(std::int32_t command_category, std::int32_t slot) noexcept;

// 0074E230 BSP_Plane_StateAdmitsAi, __fastcall(ECX unit) -> AL: the plane state (+900h) is 7, 6, 4
// or 5.
bool plane_state_admits_ai_0074e230(std::int32_t plane_state) noexcept;

// 00779AD0 BSP_Unit_ContactAgeSeconds, __fastcall(ECX unit) -> ST0: FILD (frame 00F876B0 - unit+294h),
// FMUL float [00D0DE84] (0.05f), FSTP float.
float unit_contact_age_seconds_00779ad0(std::int32_t current_frame, std::int32_t contact_frame) noexcept;

// 00727D30 BSP_Gun_AnyBarrelReadyWithin, __thiscall(ECX gun, float t), RET 4: true when some barrel
// timer (+414h, count +448h) satisfies t >= timer (FCOMI t, timer; JNC), so a NaN never matches.
bool gun_any_barrel_ready_within_00727d30(const float* timers, std::int32_t count, float t) noexcept;

// 00729A40 BSP_Gun_AnyBarrelTimerBelowThreshold, __fastcall(ECX gun) -> AL: true when some barrel
// timer is below the double at 00D7A278 (3.4028234663852886e38, FLT_MAX widened): FCOMI threshold,
// timer; JA.
bool gun_any_barrel_timer_below_max_00729a40(const float* timers, std::int32_t count) noexcept;

// 0074FA80 BSP_LeakManager_FloodedFraction, __fastcall(ECX leaks) -> ST0: leaks+28h (stored to a
// float and reloaded) divided by the unrounded product settings+404h x leaks+0Ch; FSTP float.
float leak_manager_flooded_fraction_0074fa80(float water, float settings_404, float capacity_0c) noexcept;

// 00733260 BSP_MissionNarrative_Size, __fastcall(ECX narrative) -> EAX: the list count (+0Ch) plus
// 1 while an entry shows (+14h).
std::int32_t mission_narrative_size_00733260(std::int32_t list_count, bool entry_showing) noexcept;

} // namespace bsp::band_0071_cc13
