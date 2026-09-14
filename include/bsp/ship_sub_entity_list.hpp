#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/airfield_taxi.hpp"   // AirfieldHangarCandidate, AirfieldHangarRecord
#include "bsp/plane_squadron.hpp"  // PlaneSquadronOffsets

// Projection of entity virtual slot `0FCh`, the "sub-entity list" the unit gunnery
// tick `00864FE0` calls at `00865521` and `00865680`. The slot takes one argument,
// the caller's pointer vector at `this+0BCh`, and appends entity pointers to it.
//
// The binary has exactly three implementations behind that slot, and the whole
// point of this header is which one a class gets:
//
//   00432480  92 of the 94 entity classes, every ship among them. It appends
//             `this`. The list of a ship is the ship itself.
//   006D4DD0  MAirfield only (primary vtable 00CF8C08). It appends the hangar
//             buildings of the 0Ch-stride vector at airfield+830h whose
//             `object+370h` is above zero.
//   007F44E0  the plane squadron only (primary vtable 00D087C0). It appends the
//             `+3CCh` live planes of the inline array at squadron+3D0h.
//
// docs/SHIP_SUB_ENTITY_LIST.md carries the evidence, the vtable census and the
// correction it forces on docs/GAME_EXECUTABLE.md, which records the opposite:
// that step 8.7 "appends sub-entities and never the target itself" and that a
// ship "answers vtable[0FCh] with no sub-entity". Both are wrong.
//
// Every name here is a hypothesis, not a recovered symbol. Nothing here is a
// binary-compatible layout; the `k*` values are the native offsets.
//
// Contracts named but not reconstructed: the visibility test `00864D90` and the
// pose refresh `00414DB0` the gunnery tick applies to each element it gets back;
// the vector growth path `004322D0`; the bounds-check trap `00BF6713`.

namespace bsp {

// ---------------------------------------------------------------------------
// Native offsets.
// ---------------------------------------------------------------------------

// The virtual slot itself. Confirmed twice over: the two call sites load
// `[vtable + 0FCh]`, and the only two data references to the two overrides are
// `00CF8D04 = 00CF8C08 + 0FCh` and `00D088BC = 00D087C0 + 0FCh`.
inline constexpr std::size_t kSubEntitySlot = 0x0FC;

// The pointer vector the slot fills, from 004323D0 BSP_PointerVector_PushBack
// and from the clear 0063BCD0 the tick runs before each call. Element size 4.
inline constexpr std::size_t kSubEntityVectorFirst = 0x04;
inline constexpr std::size_t kSubEntityVectorLast = 0x08;
inline constexpr std::size_t kSubEntityVectorCapacityEnd = 0x0C;

// 006D4DD0's gate reads this float off the hangar building and requires it above
// 0.0f (the constant at 00D7A218 is 0.0f). docs/AIRFIELD_TAXI.md already calls the
// same field the hangar-failure rule for 006D2640.
inline constexpr std::size_t kSubEntityHangarCondition = 0x370;

// ---------------------------------------------------------------------------
// Which implementation a class carries.
// ---------------------------------------------------------------------------
enum class SubEntityProvider {
    Self,             // 00432480, the base: append `this`
    AirFieldHangars,  // 006D4DD0, MAirfield
    SquadronPlanes,   // 007F44E0, the plane squadron
};

// Everything the three bodies read, named where the native code reads it. A
// caller supplies only the arm its `provider` selects.
struct SubEntityListInputs {
    SubEntityProvider provider = SubEntityProvider::Self;

    // Self: the entity whose slot was called.
    const void* self = nullptr;

    // AirFieldHangars: airfield+830h base, airfield+834h count, stride 0Ch.
    // `record.object` is the pointer 006D4DD0 appends and `condition` is its
    // `+370h`; `AirfieldHangarCandidate::object_present` and `local_z` are not
    // read here, 006D4DD0 tests the pointer itself and ignores geometry.
    const AirfieldHangarCandidate* hangars = nullptr;
    std::size_t hangar_count = 0;

    // SquadronPlanes: squadron+3D0h, five inline slots, squadron+3CCh live.
    const void* const* squadron_planes = nullptr;
    std::int32_t squadron_plane_count = 0;
};

// ---------------------------------------------------------------------------
// Pure rules.
// ---------------------------------------------------------------------------

// 006D4DD0's per-record gate, 006D4DF1-006D4E06: a non-null object whose
// condition is strictly above zero. `COMISS` against 0.0f with `JBE` to skip
// means an unordered compare skips too, so a NaN condition contributes nothing.
bool hangar_contributes_006d4dd0(const AirfieldHangarCandidate& hangar) noexcept;

// The slot itself. Writes at most `out_capacity` pointers into `out` in the
// order the native body appends them, and returns the number it would have
// appended; a caller that sizes `out` for `hangar_count` (or for
// `squadron_plane_count`, or 1) never loses an element. `out` may be null only
// when `out_capacity` is zero, which is how a caller asks for the count alone.
//
// The native body appends to a vector the caller cleared first (0063BCD0 at
// 00865516 and 00865679), so this function always starts from empty as well.
std::size_t sub_entity_list_slot0fc(const SubEntityListInputs& inputs,
                                    const void** out,
                                    std::size_t out_capacity) noexcept;

}  // namespace bsp
