#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual nested string vector wrappers require MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete00558640 ordinary body and verified no-op placement-delete unwind.
// Native ECX=actual16B slot, EDX=source header; no stacked input, plain RET.
// Source preserves those physical inputs and adds stacked raw-pool context.
// Null slot performs no source/header/pool work. Otherwise forward the same
// slot/source to actual00557BC0. EDX still holds source at the native call.
void __fastcall placement_copy_native_nested_string_vector_00558640(
    void* slot, const void* source, NativeStringRawPoolContext& strings);

// Complete005590C0 second placement wrapper and its no-op unwind action.
// Native entry ECX/EDX are unconsumed; stack=(slot,source), RET8. Source
// retains those physical words and adds stacked context. Incoming EDX is
// forwarded as the accepted constructor's unused EDX word. Same null rule.
void __fastcall placement_copy_native_nested_string_vector_005590c0(
    void* unused_ecx, std::uint32_t unused_edx, void* slot,
    const void* source, NativeStringRawPoolContext& strings);

// Complete00559DC0. Native ECX/EDX are unconsumed; stacked actual16B header,
// RET4, no semantic result. Source adds context after the original stack word.
// Exact00432440/004324A0 equivalence: capture begin; if nonnull, capture end,
// destroy actual8h strings, free CURRENT begin; always clear +4,+8,+C in order.
// Preserve +0. No local catch/retry; exceptions stop later free/clears.
void __fastcall tidy_native_nested_string_vector_00559dc0(void* unused_ecx,
    std::uint32_t unused_edx, void* actual_header,
    NativeStringRawPoolContext& strings);

// No semantic EAX contract is established for either placement wrapper: nonnull
// calls leave constructor EAX=slot, while null skips leave the entry FS:[0]
// exception-chain value in EAX. These void Source interfaces make no EAX
// residue or private-frame promise and do not invent a null pointer result.
// Original unwind actions00C6D930/00C6D9F0 call RET-only00401130: no free,
// header rollback or extra Source catch belongs to either wrapper.
//
// Actual owning headers/pool publications and accepted constructor/destructor
// services are used directly. Added context changes stack cleanup; these are
// current Source C++ exception-domain interfaces, not original binary ABI,
// FH3/SEH/private-stack/register-output or cleanup-failure identity proofs.
// Older qualified host projections remain separate. No native differential
// or game validation is claimed. See OCT15_NESTED_STRING_VECTOR_WRAPPERS.md.
} // namespace bsp
