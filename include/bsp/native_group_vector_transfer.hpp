#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual group vector transfer requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete006A6D20..006A6E12, including parent-dependent catch006A6DB0.
// Native ECX first, EDX last; stack destination, owner word, duplicate word,
// tag word; RET10h/EAX final destination. Source adds raw-pool context.
// An actual16B private empty outer header has only4/8/C initialized; opaque0
// remains uninitialized. Each nonnull destination is constructed from that
// header with actual0055B150 before ordered4/8/C swaps; null skips ONLY the
// constructor. Swaps still access the destination. Source/destination+0 are
// untouched. Endpoints/cursors use DWORD equality/wrap, no validation.
// Catch destroys exactly the completed destination prefix through actual
// stacked tidy006A6CE0, then rethrows. Exceptional exits unwind the temporary
// through0055B110. Normal temporary cleanup instead uses cached begin and
// CURRENT end, then frees that same cached begin, with cleanup disarmed.
void* __fastcall transfer_native_group_vector_to_empty_006a6d20(
    void* first, void* last, void* destination, std::uint32_t owner_word,
    std::uint32_t unused_duplicate, std::uint32_t unused_tag,
    NativeStringRawPoolContext& strings);

// Complete006A70E0..006A7104. Native ECX receiver word, EDX unused;
// stacked first,last,destination; RET0Ch. Child EAX passes through. Source
// retains physical inputs and adds context. The child's stack words are
// destination, receiver, destination, receiver&FFFFFF00h. No owner access.
void* __fastcall transfer_native_group_vector_dispatch_006a70e0(
    std::uint32_t receiver_word, std::uint32_t unused_edx, void* first,
    void* last, void* destination, NativeStringRawPoolContext& strings);

// Complete006A6E20..006A6E62. Native cdecl first,last,destination-end and
// three unconsumed stack words; plain RET. Source is void; native EAX is
// cursor residue. Swap backward by16 using the exact source/destination
// field-load order, then ordered4/8/C stores; +0 is not read or written.
// No construction/destruction/context is involved.
void __cdecl swap_native_group_vector_backward_006a6e20(void* first,
    void* last, void* destination_end, std::uint32_t unused0,
    std::uint32_t unused1, std::uint32_t unused2);

// Complete006A7110..006A714F. Native cdecl first,last,destination-end;
// plain RET/EAX independently computed destination-(SAR4(last-first)<<4).
// Native saved ambient ECX has its low byte cleared and is forwarded as the
// sixth DEAD child word; Source normalizes that word to0. It retains the
// original three-stack-word interface rather than inventing semantic ECX.
void* __cdecl move_native_group_vector_backward_006a7110(void* first,
    void* last, void* destination_end);

// Current actual raw-pool/CRT/Source C++ services, no callbacks, host models,
// guards or rollback. RET-only00401130 placement-delete action contributes no
// cleanup. Parent EBP/FH3/catch/unwind frames are represented by Source catch
// scopes, not new callable native-frame adapters. Unconsumed child EDX words
// are normalized0; native register/private-stack residue is not promised.
// Added context/by-value interfaces and Source cleanup exceptions do not bind
// original entry/stack ABI, heap/CRT/handler/FH3/SEH/fault/exception identity.
// Valid reached storage/child ownership is required; private-frame aliases
// are outside this interface. Prior projections/fixtures remain separate.
} // namespace bsp
