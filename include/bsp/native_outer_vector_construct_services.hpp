#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual outer vector construction services require MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete00557250..00557298 over an actual16B checked header, +0 untouched.
// Native ECX header, EDX unconsumed, stacked unsigned count, RET4. Source
// reserves ignored EDX and preserves the exact defined DWORD EAX result.
// Capture count, zero+4/+8/+C in order, then test zero. Zero returns0 without
// allocating. Unsigned count>0FFFFFFF throws the actual owning556C90 service.
// Otherwise actual553300(count,0), publish capacity then begin then end, and
// finally replace only AL with1: EAX=(allocation bits & FFFFFF00h)|1.
// No old backing destruction/free, element construction, proxy initialization
// or extra guard. The earlier Fe3InputHost projection remains separate.
std::uint32_t __fastcall initialize_native_outer_vector_storage_00557250(
    void* header, std::uint32_t unused_edx, std::uint32_t count);

// Complete006A3350 ordinary range and006A33AA prefix catch, via the accepted
// exact00559E00 Source. Complete native148B schedules differ only in the
// independently verified EH-handler immediate and three rel32 call fields;
// their complete try/unwind maps and RET-only placement actions also match.
// Native ECX first,EDX last,stack(dest,owner,tag3,tag4),RET10h/EAX dest_end.
// Source retains those words and adds actual context, RET14h. Each iteration
// constructs actual16B inner owning headers through557BC0 if dest!=0, then
// advances dest by16, publishes completed end, then advances source by16.
// Null skip still joins prefix; catch tidies prefix without a null skip and
// excludes failing current child, then rethrows. No outer allocation free.
void* __fastcall copy_native_inner_vectors_uninitialized_006a3350(
    const void* first, const void* last, void* destination,
    std::uint32_t unused_owner, std::uint32_t unused_tag3,
    std::uint32_t unused_tag4, NativeStringRawPoolContext& strings);

// Complete006A4C00..006A4C24. Native ECX owner,EDX unconsumed, THREE stacked
// words(first,last,destination),RET0Ch/EAX helper result. Source adds context,
// RET10h. Forward owner,destination bits,owner&FFFFFF00h as dead metadata.
// Accepted0055A950 instead has FIVE physical words/RET14h; do not alias ABIs.
void* __fastcall copy_native_inner_vectors_dispatch_006a4c00(
    std::uint32_t unused_owner, std::uint32_t unused_edx,
    const void* first, const void* last, void* destination,
    NativeStringRawPoolContext& strings);

// DWORD address arithmetic, actual child pool/ownership/currentCRT and all
// accessed storage/endpoint contracts apply. Fixed captured source endpoints,
// forward overlap effects and+0 preimages are retained; no snapshot/self/null
// repair is added. Safe catch cleanup requires every prefix access to satisfy
// actual Tidy preconditions, including any skipped null slot. Exclude aliases
// to native/Source private frames. Extra context/current C++ cleanup, allocator
// and owning exception transport do not establish native FH3/SEH/RTTI/fault,
// private frame/register/binary ABI, CRT identity or game equivalence.
} // namespace bsp
