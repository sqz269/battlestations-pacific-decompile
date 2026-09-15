#pragma once

namespace bsp {

// Complete original one-word cdecl C0190B[134], ___addlocaleref. This canonical
// native entry preserves EBX/EBP/ESI/EDI and returns with plain RET. It compares
// the original fixed narrow-C sentinel identity 00E161D0; no context argument,
// provider binding, host CRT locale or replacement sentinel is introduced.
//
// The caller owns the actual locale record (readable through D7h), selected
// writable LONG counts and their lifetimes. The original owners hold native
// locale lock 12 around publication/copy and this call. Atomic increments alone
// do not protect pointer loads against release/replacement. This entry neither
// acquires that lock nor implements PTD/locale initialization or destruction.
//
// Cache the real Win32 InterlockedIncrement import once; increment the root,
// reload optional B0h/B8h/B4h/C0h count pointers in order, traverse all six
// category pairs, then reload and unconditionally increment [record+D4h]+B4h.
// Preserve pointer identity, aliasing, repeated increments and every reload.
// No added null guard, deduplication, overflow repair, error translation or EH
// frame. Invalid native inputs may fault after earlier counts have changed.
//
// Fixed sentinel/default objects belong to GameNativeCanonicalDataOwner's
// actual admitted PE pages. Arbitrary nondefault locale/count storage remains
// its real producer's responsibility; this primitive supplies none of it.
// Original code placement and fault/unwind addresses are not reproduced by a
// source build. This entry is build/byte/ABI inspected, not runtime validated.
void __cdecl acquire_native_crt_canonical_locale_references_00c0190b(
    void* actual_locale_record);

} // namespace bsp
