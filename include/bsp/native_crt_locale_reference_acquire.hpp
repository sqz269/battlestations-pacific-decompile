#pragma once

namespace bsp {

// Complete native C0190B[134], ___addlocaleref. The native cdecl entry accepts
// only the locale-record pointer and preserves EBX/EBP/ESI/EDI. This source entry
// adds the actual narrow-C-locale sentinel identity as its second cdecl argument.
// Both arguments are borrowed: no locale, reference count or sentinel is owned
// or initialized here. The original sentinel is the address 00E161D0, not any
// other string with equal contents. Its supplied argument must remain stable.
//
// The actual record must support the original reads through offset D7h; all
// selected count targets must support real Win32 InterlockedIncrement. Count
// pointers may alias each other or subsequently read fields. Their reload order
// is deliberate: root, optional B0h/B8h/B4h/C0h, six category pairs, then the
// unguarded count at [record+D4h]+B4h. Repeated targets are not deduplicated.
// The root and final time-locale pointer have no added null checks. Existing
// synchronization/lifetime requirements belong to the actual locale owner.
//
// Uses the real imported InterlockedIncrement target, cached once as in the
// native body. Adds no allocator, lock, release, overflow repair or error/EH
// policy. The extra stack argument and ECX use qualify the source ABI, register
// and fault continuation: this is not a native entry-point replacement or a
// reconstruction of the original PTD, locale, SEH or runtime owner.
void __cdecl acquire_native_crt_locale_references_00c0190b(
    void* actual_locale_record, const void* actual_narrow_c_sentinel);

} // namespace bsp
