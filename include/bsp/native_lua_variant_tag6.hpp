#pragma once

#include "bsp/native_lua_variant_retirement.hpp"

namespace bsp {

// Qualified ordinary Source composition of 0074FB30, not its ECX/plain-RET ABI
// or incidental child EAX. Capture the genuine receiver identity, call the
// existing clear_cpp_cleanup with the SAME tuple, and store full DWORD6 at +4
// only after its actual completed result. Return any non-completion unchanged;
// C++ escape propagates with no wrapper action, tag store or retry.
//
// Inherit the child's genuine aligned/live typed fields, nested payload/map,
// matched allocator/raw-cell and independently retained operation/cleanup
// backing contracts, including its alias/reentry exclusions. No arbitrary-byte
// slab admission, C++ object birth, field repair or new cleanup is supplied.
NativeLuaVariantRetirementResult initialize_native_lua_variant_tag6_0074fb30(
    void* actual_variant, NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup);

} // namespace bsp
