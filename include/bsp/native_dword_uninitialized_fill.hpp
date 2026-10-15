#pragma once

#include <cstdint>

namespace bsp {

// Complete STL_UninitFillN_006970d0 over actual four-byte scalar storage.
// Original ECX=destination, EDX=unsigned count; four stacked DWORDs are
// value pointer, owner, duplicate value pointer, tag; RET10h. Only the first
// stacked word is consumed, and only when count is nonzero. Capture its
// ADDRESS, then reread its current DWORD before each nonnull destination
// store. A null destination skips both accesses but still advances by four.
// Cursor and count use wrapping DWORD arithmetic. No semantic return value.
//
// All physical argument positions are explicit. The last three are dead;
// they introduce no ownership, allocator, validation or callback contract.
// Volatile DWORD accesses retain the native progressive load/store order on
// the MSVC Win32 target. No portable-pointer, original private-stack alias,
// CRT/fault/SEH or binary/gameplay equivalence is claimed. Existing qualified
// checked-vector fragments retain their separate coverage and interfaces.
void __fastcall fill_native_dword_uninitialized_006970d0(
    void* destination, std::uint32_t count, const void* value,
    void* unused_owner, const void* unused_value_duplicate,
    std::uint32_t unused_tag);

// Complete STL_TagDispatch_00697760. Original ECX=owner; EDX unconsumed;
// stack destination/count/value, RET0Ch. Calls the core above with the
// original owner, duplicate value and dead tag (owner bits with low byte0).
// Return the independently captured destination+count*4 with DWORD wrap;
// never use the void core's ambient native EAX. Matching register/stack
// shapes here do not promise the original code generation or private frame.
void* __fastcall fill_native_dword_uninitialized_n_dispatch_00697760(
    void* owner, std::uint32_t unused_edx, void* destination,
    std::uint32_t count, const void* value);

} // namespace bsp
