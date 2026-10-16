#pragma once

namespace bsp {
// Whole AD42C0 normal body: BF681B(18h), conditional DWORD zero stores at
// +00/+04/+08, then BYTE +14=1 and +15=0. All other bytes remain untouched.
// Original takes no arguments, returns the raw allocation in EAX, plain RET.
//
// New Win32 Source ABI over the existing owning singleton_lifetime_allocate
// domain: returns genuine nonnull 18h storage or throws an ordinary Source
// exception. Retire this allocation with the SAME singleton_lifetime_free
// domain when its actual tree lifetime permits; this leaf does not own it.
// Raw field accesses assume that SDK's aligned, writable storage domain.
// Native null/invalid-address paths, private heap/new-handler/exception ABI,
// and fault timing are not reproduced. No Font object, count, registry,
// publication, initializer completion, or whole Text rebuild is established.
void* allocate_native_font_tree_head_00ad42c0();
} // namespace bsp
