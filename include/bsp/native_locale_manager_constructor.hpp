#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native locale manager construction requires MSVC Win32.
#endif

namespace bsp {

// Borrow the application's actual cells. Reference identities remain fixed;
// their volatile pointer values are observed at the native load/store epochs.
// This context creates no publication, manager, callback or semantic owner.
struct NativeLocaleManagerConstructorContext {
    void* volatile& actual_manager_01090aa0;
    void* volatile& actual_locale_00f8bc4c;
};

// Complete normal 736540[145]: native ECX receiver, EAX original receiver, RET.
// Capture the first manager's actual +10 section, Enter/increment its physical
// +18 DWORD, then publish. Resolve the SECOND manager before loading CURRENT
// locale for BD0C30. Release the original captured section and return receiver.
void* construct_native_locale_manager_base_00736540(
    void* actual_receiver, const NativeLocaleManagerConstructorContext&);

// Complete normal 73C960[141]: same native input/result, plain RET. Only after
// base success write CFF140 and the exact zero fields/range of the 4040h image.
// Words +4/+4020/+4030 remain inherited. The old gui_startup field-only API is
// separate and unchanged. This adds no allocation, owner or loaded locale.
void* construct_native_locale_manager_0073c960(
    void* actual_receiver, const NativeLocaleManagerConstructorContext&);

// Caller supplies valid four-byte-aligned 4040h receiver storage with live
// uint32_t accesses, disjoint stable context/cells, actual raw managers/vector
// owners and captured Win32 section+18 storage through all calls and cleanup.
// Install genuine profile/lifetime handling before any registration. Borrow
// the real application domains; no concurrent conflicting access, retargeted
// context or provider-induced storage invalidation is supported.
//
// Saved base state0 resets the original receiver via genuine 412430. Only
// AFTER the optional Enter/depth (including the null-section skip), state1
// arms genuine 411EE0 on its actual local eight-byte guard; C++ escape then
// runs guard cleanup before root reset.
// No unarmed unlock, publication clear, unregister, free, rollback or retry is
// added. Normal release remains armed through Leave. A second escaping C++
// cleanup terminates; failed calls may retain publication/registration effects.
// The derived body arms no cleanup states; its saved state remains -1.
//
// Ordinary Source APIs add explicit arguments: Original register/FH3/SEH,
// mutable private EH spills, fault timing, cookies and provider throw identity
// are not reproduced. Profile DWORDs are identities, not callable C++ vtables.
// No 7365E0/73C9F0/scalar deletion, canonical cell, startup, loading, uppercase,
// Text, ABI, runtime or source-ready activation is supplied by this library.

} // namespace bsp
