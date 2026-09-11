#pragma once

#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {

// Complete B34020..B34067: ECX actual owner, stack borrowed COM/flags,
// EAX same owner, RET8. Actual writable24h base, not a separate host object.
// Preserve the native profile/field order, name0, additional word+14=0,
// borrowed COM+10, flags+1C, and serial+20. Word+18 remains untouched.
// Borrow the same serial DWORD0108D6E8 as all named/unnamed texture bases;
// reread it for wrapped increment after the owner store, including aliases.
// Existing name/COM values are overwritten without release or AddRef.
void* construct_native_logical_texture_unnamed_base_00b34020(
    void* actual_owner, void* borrowed_com, std::uint32_t flags,
    std::uint32_t& actual_shared_serial_0108d6e8) noexcept;

// Complete five-byte B34090 JMP to actual B33F50; native ECX owner, RET.
// Run the existing complete named-base destruction over this actual owner:
// current name release and base action. Used by original cube FH3 cleanup.
// Borrow the genuine current NativeStringStorage; exceptions may propagate.
void unwind_native_logical_texture_unnamed_base_00b34090(
    void* actual_owner, NativeStringStorage& actual_strings);

// New MSVC Win32 source interfaces; descriptive names are hypotheses.
// No independent serial domain, owner allocation, native ABI or game proof.
} // namespace bsp
