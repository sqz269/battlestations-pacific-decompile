#pragma once

#include <cstdint>

namespace bsp {

// Nine DISTINCT complete original ship-profile +5Ch targets. Each naked Win32
// entry takes ECX=the SAME actual receiver, stack=raw class DWORD, returns full
// EAX0/1, RET4. The fastcall spelling makes the unused EDX explicit.
// Each exact owning-profile fixed list (including zero) precedes ONE fresh
// actual DWORD+C4 comparison. There is no generic classifier, base substitution,
// eager field snapshot, census, copied cache, callback, default or routing.
// Ordinary caller: stable nonnull actual receiver, genuine live aligned DWORD
// exactly at+C4, readable backing>=C8. Fixed paths do not read the receiver;
// that establishes no constructor/profile/ownership/lifetime domain.
// Original tables remain uncallable DATA here. Whole class/native virtual
// routing, constructors, group admission/speed/detach, world/arena/observer
// lifetime, faults, concurrency, private EH and gameplay remain unbound.
// COMPLETE006DFE50..006DFE86: fixed6/5/4/2/1/0 then actual+C4.
std::uint32_t __fastcall native_ship_base_is_kind_006dfe50(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

// COMPLETE006FE530..006FE56B: fixed7/6/5/4/2/1/0 then actual+C4.
std::uint32_t __fastcall native_destroyer_is_kind_006fe530(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

// COMPLETE00853050..0085308B: fixed8/6/5/4/2/1/0 then actual+C4.
std::uint32_t __fastcall native_submarine_is_kind_00853050(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

// COMPLETE00758510..0075854B: fixed9/6/5/4/2/1/0 then actual+C4.
std::uint32_t __fastcall native_mothership_is_kind_00758510(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

// COMPLETE006FB3D0..006FB40B: fixed10/6/5/4/2/1/0 then actual+C4.
std::uint32_t __fastcall native_cruiser_is_kind_006fb3d0(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

// COMPLETE006EB230..006EB26B: fixed11/6/5/4/2/1/0 then actual+C4.
std::uint32_t __fastcall native_cargo_is_kind_006eb230(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

// COMPLETE0074BC60..0074BC9B: fixed12/6/5/4/2/1/0 then actual+C4.
std::uint32_t __fastcall native_landing_ship_is_kind_0074bc60(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

// COMPLETE006DFE90..006DFECB: fixed13/6/5/4/2/1/0 then actual+C4.
std::uint32_t __fastcall native_battleship_is_kind_006dfe90(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

// COMPLETE00857DC0..00857DFB: fixed14/6/5/4/2/1/0 then actual+C4.
std::uint32_t __fastcall native_torpedo_boat_is_kind_00857dc0(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

} // namespace bsp
