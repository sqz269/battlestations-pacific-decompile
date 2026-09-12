#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native gameplay effect construction requires MSVC Win32.
#endif

namespace bsp {

// 869C50[17]: original ECX=raw owner, RET. New EDX reference identifies the
// actual volatile F87664 publication cell. Clear it unconditionally before
// storing CE3818 to the captured owner. No allocation free or unregister.
void __fastcall reset_native_gameplay_effect_manager_00869c50(
    void* owner, void* volatile& actual_publication_00f87664);

// 870370[104]: original ECX=raw10h owner, EAX=same owner, RET. New EDX
// reference supplies the actual mutable F87664 cell for constructor cleanup.
// Install D0DA64, allocate raw18h sentinel through86AC00, publish owner+8,
// set nil15, reread head before parent/left/right links, clear count+0C.
// Owner+4, node payload+0C/+10 and padding+16/+17 stay untouched.
// Source C++ failure directly runs869C50 and rethrows; no sentinel/owner free.
void* __fastcall construct_native_gameplay_effect_manager_00870370(
    void* owner, void* volatile& actual_publication_00f87664);

// 4C1650[189]: original no consumed input, EAX=current/captured owner, RET.
// Borrow distinct actual native publication cells, with stable references.
// Complete raw manager lookup/section capture, entered-depth accounting,
// registry recheck, raw10h allocation and constructor, publication, second
// manager lookup/current owner reread/registration, captured section release,
// and post-Leave publication reread. The fast path returns its first capture.
// Constructor failure frees captured owner storage after its own reset;
// registration failure retains publication/storage and destroys the guard.
void* get_native_gameplay_effect_manager_004c1650(
    void* volatile& actual_manager_publication_01090aa0,
    void* volatile& actual_effect_publication_00f87664);

// Uses actual raw manager/construction/registration and source CRT/Win32
// providers, without private globals or a typed-container projection. Source
// C++ exception boundaries do not reproduce native FH3/SEH stack identity,
// private binding/EH-spill aliases, hardware-fault cleanup or provider throw
// and register identities. Original no-input caller ABI is not this ABI.
// Native profile DWORDs are identity data, not callable rebuilt vtables.
// Raw erasure/destruction and canonical executable migration remain separate.
} // namespace bsp
