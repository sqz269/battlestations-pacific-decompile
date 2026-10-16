#pragma once

#include "bsp/native_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual preset temporary-key release requires MSVC Win32.
#endif

namespace bsp {

// Whole0069E090..0069E0AC:29B/12 instructions. Preserve the existing descriptive
// BSP_NativeString_ReleaseStorage_0069e090 identity; the Source name is a
// hypothesis. Actual pooled header: LENGTH DWORD0, DATA DWORD4. Earlier name
// evidence reversed those fields; the older generic Source already reads them
// correctly and retains its NativeStringStorage/noexcept contract.
//
// Native ECX header, incoming EDX unconsumed, no stack inputs/plainRET. Null data
// leaves ambient EAX; nonnull return leaves provider residue. Source exposes no
// EAX result and adds a borrowed real RawCtx stack word/RET4, a new ABI.
// Directly reuse the published REAL RawCtx overload of whole0041DD20: capture
// data4 first; if nonnull capture CURRENT length0+1 with DWORD wrap, resolve the
// actual getter before EVERY return, then return the captured data/size. Null
// data skips length/provider/context-field accesses. Never reset the header.
void __fastcall release_native_input_preset_temp_key_0069e090(
    void* completed_actual_key_header, std::uint32_t unused_edx,
    NativeStringRawPoolContext& strings);

// Intentionally not noexcept: getter C++ failure propagates with its established
// progress. Caller owns any secondary-cleanup exception policy. Matching actual
// publication/gate/raw-manager cells and valid reached owned storage are required.
// No new adapter, context, global, cache, guard, retry, free, rollback or catch.
// Source private frames/current CRT/CPP do not establish Original heap, register,
// flags, FH3/SEH/RTTI/fault, binary ABI or game identity. C7F100 is readonly origin
// evidence only; no public EBP helper, logical action or parent promotion.
} // namespace bsp
