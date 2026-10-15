#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pool storage cleanup requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete 0041F820, including both null-buffer branches. Original ECX is
// an existing record and the routine ends in plain RET. Its owner identity
// and complete class layout are unknown; no replacement record is created.
// Conditional CRT free of +10h precedes DWORD clears at +10h/+14h/+18h.
// Then return the captured +8h buffer with wrapped DWORD(+4h)+1 through
// the actual string pool. +4h/+8h and all other record bytes stay untouched.
//
// This source interface adds an explicit raw-pool context in EDX; it is not
// a native binary replacement. Supply a valid record extent of at least 1Ch
// and the application's existing publication cells/lifetime domain. Getter
// failures propagate after the three clears; no retry or rollback is added.
// Evidence: docs/GOAL15_POOL_STORAGE_CLEANUP.md.
void __fastcall cleanup_native_pool_storage_0041f820(
    void* actual_record, NativeStringRawPoolContext& context);
} // namespace bsp
