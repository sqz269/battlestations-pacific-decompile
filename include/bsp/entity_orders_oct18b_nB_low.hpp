#pragma once

#include <cstdint>

namespace bsp {

// Additional bodies of EntityOrderCommandClass (entity_orders.hpp). The original
// pair is leased by oct18b-nA; this extension owns no second command-class model.
// Names are hypotheses; docs/OCT18B_NB_LOW.md records table/slot and ABI evidence.
// __fastcall supplies the original ECX receiver; an unused EDX parameter bridges
// __thiscall methods that also pop one stack argument on MSVC Win32.
std::uint32_t __fastcall CommandCloseToShip_Category_006f8e60(const void* self);
std::uint8_t __fastcall CommandCloseToShip_RequiresTarget_006f8e70(const void* self);
std::uint32_t __fastcall CommandCloseToShip_Property10_006f8e80(const void* self);
std::uint32_t __fastcall CommandMoveOnPath_Category_006f8f00(const void* self);
std::uint8_t __fastcall CommandMoveOnPath_RequiresTarget_006f8f10(const void* self);
std::uint32_t __fastcall CommandMoveOnPath_Property10_006f8f20(const void* self);
std::uint32_t* __fastcall CommandMoveOnPath_CopyDefaultParameters_006f8f30(
    const void* self, void* unused_edx, std::uint32_t* out);
std::uint32_t __fastcall CommandLeave_Category_006f9010(const void* self);
std::uint8_t __fastcall CommandLeave_RequiresTarget_006f9020(const void* self);
std::uint32_t __fastcall CommandLeave_Property10_006f9030(const void* self);
std::uint32_t __fastcall CommandDisband_Category_006f90b0(const void* self);
std::uint8_t __fastcall CommandDisband_RequiresTarget_006f90c0(const void* self);
std::uint32_t __fastcall CommandDisband_Property10_006f90d0(const void* self);

}  // namespace bsp
