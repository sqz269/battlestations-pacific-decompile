#pragma once

#include "bsp/native_lua_variant_retirement.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua variant string setter requires MSVC Win32.
#endif

namespace bsp {

// Whole 00886A10, ordinary completion-only Source API; no Native EAX/RET4 claim.
// Inherit the actual clear_cpp_cleanup live typed20/nested/operation/backing
// domain. The source-pointer HOME is an actual four-byte synchronous borrow,
// read only after nonnull allocation; its value is captured once at that epoch.
// Nonself source/header/data and all reached extents must be genuine and live.
//
// New8 must be fresh aligned storage. A private trivial Header8 receives
// default placement birth WITHOUT an initializer after comparison/before the
// native zero stores, including self. Input identity does not adopt old payload.
// SAME raw cells/canonical feature/heap and real pool/context/code lifetimes
// remain required. No feature pre-read or replacement typed/string model.
//
// Constructor C++ escape frees captured8 ONLY; partial pool/string/root/tag
// debt remains caller-owned. Do not inspect the freed header, retry, retire or
// infer safe shutdown from that free. Clear/allocation escape is not converted
// to completion. Normal completion does not certify final tag/retirability.
// No Frame, root rollback, buffer/pool refund, input cleanup or new owner.
// Ordinary private HOME, aliases/extent/DF/CPU/CRT and lower noexcept/nonlocal/
// FH3/SEH/fault limits remain; no parent/VM/startup/ABI/runtime permission.
NativeLuaVariantRetirementResult
set_native_lua_variant_string_00886a10_cpp_cleanup(
    void* actual_variant, const void* actual_source_pointer_home,
    NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup,
    const volatile std::uint32_t& actual_feature_word_0109eea4);

} // namespace bsp
