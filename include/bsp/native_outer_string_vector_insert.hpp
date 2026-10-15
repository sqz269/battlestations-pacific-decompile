#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual outer string vector insertion requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete006A5C50 normal body and its006A5DC5/006A5E4E catch scopes.
// Native ECX actual outer16B header; incoming EDX unconsumed; stack
// (unused owner,position,unsigned count,borrowed inner16B value),RET10h.
// Existing inner backing transfers by empty construction and field swaps;
// inserted values deep-copy a private actual557BC0 prototype, even count0.
// No semantic result. Context/private frames are new Source interfaces.
void __fastcall insert_native_outer_string_vector_count_006a5c50(
    void* header, std::uint32_t unused_edx, const void* unused_owner,
    void* position, std::uint32_t count, const void* value,
    NativeStringRawPoolContext& strings);

// Complete00C7F470 owner action: native LEA ECX,[EBP-28]; JMP004324A0.
// Source receives the actual private inner header explicitly, not native EBP.
// Normal006A5EA8 cleanup is separately inlined in the parent and frees its
// captured begin; this unwind action instead uses actual4324A0 current-begin
// free and ordered field clears. No native frame/FH3/SEH adapter is implied.
void __fastcall unwind_native_outer_insert_value_00c7f470(
    void* temporary, NativeStringRawPoolContext& strings);

// Uses admitted actual raw-pool and current CRT services. There is no added
// validation, self guard, callback, allocator model, rollback or restoration of
// transferred source fields. Current C++ cleanup/throw behavior does not prove
// original CRT/FH3/SEH, handler-failure, private-stack or game compatibility.
} // namespace bsp
