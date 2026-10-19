#include "bsp/entity_orders_oct18b_nB_low.hpp"

namespace bsp {

// Original: ECX receiver unused, RET, return in EAX (category/property) or AL
// (requires-target). Upper EAX bits are not part of the boolean result contract.
// Vtables 00CFB61C (closetoship), 00CFB63C (moveonpath), 00CFB660 (Leave),
// 00CFB67C (disband); slots +0Ch, +08h and +10h respectively.
std::uint32_t __fastcall CommandCloseToShip_Category_006f8e60(const void*) { return 3; }
std::uint8_t __fastcall CommandCloseToShip_RequiresTarget_006f8e70(const void*) { return 1; }
std::uint32_t __fastcall CommandCloseToShip_Property10_006f8e80(const void*) { return 1; }
std::uint32_t __fastcall CommandMoveOnPath_Category_006f8f00(const void*) { return 3; }
std::uint8_t __fastcall CommandMoveOnPath_RequiresTarget_006f8f10(const void*) { return 0; }
std::uint32_t __fastcall CommandMoveOnPath_Property10_006f8f20(const void*) { return 0; }

// Original 006F8F30..006F8F97: ECX unused; [ESP+4]=out, RET 4, EAX=out.
// Lazy flag 00E19A8C is tested as a byte but ORed as a dword. All copies are
// integer MOVs, preserving the four words. This deliberately uses the observed
// original image storage shared with 006F8960, rather than a second local static.
std::uint32_t* __fastcall CommandMoveOnPath_CopyDefaultParameters_006f8f30(
    const void*, void*, std::uint32_t* out) {
    auto* flags = reinterpret_cast<volatile std::uint32_t*>(0x00e19a8c);
    auto* defaults = reinterpret_cast<volatile std::uint32_t*>(0x00e19a7c);
    if ((*reinterpret_cast<volatile std::uint8_t*>(flags) & 1u) == 0) {
        *flags |= 1u;
        defaults[0] = 0;
        defaults[1] = 0;
        const auto one = *reinterpret_cast<const volatile std::uint32_t*>(0x00d7a24c);
        defaults[2] = one;
        defaults[3] = one;
    }
    const auto first = defaults[0];
    const auto second = defaults[1];
    out[0] = first;
    const auto third = defaults[2];
    out[1] = second;
    const auto fourth = defaults[3];
    out[2] = third;
    out[3] = fourth;
    return out;
}

std::uint32_t __fastcall CommandLeave_Category_006f9010(const void*) { return 0; }
std::uint8_t __fastcall CommandLeave_RequiresTarget_006f9020(const void*) { return 0; }
std::uint32_t __fastcall CommandLeave_Property10_006f9030(const void*) { return 0; }
std::uint32_t __fastcall CommandDisband_Category_006f90b0(const void*) { return 0; }
std::uint8_t __fastcall CommandDisband_RequiresTarget_006f90c0(const void*) { return 0; }
std::uint32_t __fastcall CommandDisband_Property10_006f90d0(const void*) { return 0; }

}  // namespace bsp
