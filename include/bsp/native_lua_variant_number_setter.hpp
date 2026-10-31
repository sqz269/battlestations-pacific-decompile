#pragma once

#include "bsp/native_lua_variant_retirement.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua variant number storage requires MSVC Win32.
#endif

namespace bsp {

// Qualified ordinary Source composition of complete 008869D0..008869ED.
// Capture receiver and actual four-byte HOME address, call genuine clear with
// the SAME tuple, propagate actual non-completion before any HOME read/store.
// After completed clear: CURRENT HOME bits, then DWORD tag0, then captured bits
// at receiver+8. No float snapshot/arithmetic/conversion or Native EAX claim.
//
// Caller retains actual initialized/readable HOME representation and genuine
// aligned/live typed receiver plus the child's complete nested/provider/backing
// domain. Legal ordinary HOME/receiver aliases follow the stated epochs; a
// private Native stack-home/address/ABI equivalence is not supplied. No Frame,
// type birth, empty-tag assumption, wrapper cleanup, catch, repair or retry.
NativeLuaVariantRetirementResult
set_native_lua_variant_number_008869d0_cpp_cleanup(
    void* actual_variant, const void* actual_float32_home,
    NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup);

} // namespace bsp
