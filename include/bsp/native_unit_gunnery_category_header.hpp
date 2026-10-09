#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native unit gunnery category header construction requires MSVC Win32.
#endif

namespace bsp {

// Actual 0Ch-byte constructor storage. The field names describe offsets;
// 00952640 alone establishes no count, link, ownership or lifetime semantics.
struct NativeUnitGunneryCategoryHeaderStorage {
    std::uint32_t word_00;
    std::uint32_t word_04;
    std::uint32_t word_08;
};

// Complete 00952640..0095264C: entry ECX=actual header, EAX=entry ECX,
// ECX=0, EDX/nonvolatile registers untouched, no stack arguments, RET0.
// Supply valid actual writable storage for all three DWORDs. The naked
// body writes +0, then +4, then +8; it adds no pointer validation, prior
// contents read, allocation, cleanup or exception handler. No noexcept.
// This leaf supplies neither the parent's array iterator/destructor pair
// nor a complete unit receiver, production publication or whole-class ABI.
NativeUnitGunneryCategoryHeaderStorage* __fastcall
construct_native_unit_gunnery_category_header_00952640(
    NativeUnitGunneryCategoryHeaderStorage* actual_header);

} // namespace bsp
