#pragma once

#include "bsp/native_lua_variant_retirement.hpp"

#include <cstdint>
#include <optional>

namespace bsp {

// Ordinary Source result, not a Native pointer/receiver or register ABI.
// No input value exists on child non-completion. A completed result contains
// the late DWORD captured by B30 before tag/payload stores (Native normal EAX).
struct NativeLuaVariantPointerSetResult {
    NativeLuaVariantRetirementResult completion;
    std::optional<std::uint32_t> actual_normal_input_bits;
};

// Inherit the real clear_cpp_cleanup's entire typed/nested/provider/backing
// domain. HOME is a genuine live DWORD through its late read after completed
// clear; legal receiver-field aliases observe clear's changes. Result/control
// metadata must remain independent. No entry snapshot, birth, type filter,
// Frame, cleanup, rollback or retry is supplied; C++ escape propagates.
//
// F0 is SIGNED DWORD -> FLOAT32, not bool normalization. It uses actual SSE
// CVTSI2SS under CURRENT MXCSR, then tag0/payload. No control-mode reset or
// x87/C++ float cast; instruction status/exception effects remain real. Its
// ordinary completion result does not represent incidental Native child EAX.
NativeLuaVariantRetirementResult set_native_lua_variant_signed_number_008869f0(
    void* actual_variant, const volatile std::uint32_t& actual_input_home,
    NativeStringRawPoolContext&, NativeLuaVariantRetirementOperation&,
    const NativeLuaVariantCppCleanupBacking&);

// B30 reads CURRENT full HOME before tag4/payload, returning that captured
// opaque word only after normal stores. Never return receiver or reread HOME.
// No direct HOME/partial-BYTE writes, profile/name stores or post-payload reads.
NativeLuaVariantPointerSetResult set_native_lua_variant_pointer_word_00886b30(
    void* actual_variant, const volatile std::uint32_t& actual_input_home,
    NativeStringRawPoolContext&, NativeLuaVariantRetirementOperation&,
    const NativeLuaVariantCppCleanupBacking&);

// Ordinary live-HOME/Source metadata interfaces: no physical private-stack,
// upper-XMM, Native ABI/FH3/SEH/fault, caller/whole-builder/VM/startup or runtime
// permission. Failed child debt and actual record/HOME/graph custody stay caller-owned.
} // namespace bsp
