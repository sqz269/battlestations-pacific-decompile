#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native invalid-parameter call requires MSVC Win32.
#endif

namespace bsp {

// Qualified Source entry for the complete 00BF6713..00BF6722 schedule:
// XOR EAX; five PUSH EAX; call the actual Source CRT zero-argument API;
// ADD ESP, 14h; RET. The five zero DWORDs are unused Source callee padding,
// not invented API arguments or recovered Native child field types.
//
// No receiver, context, stack arguments or defined EAX result. The owned
// instructions do not write EBX/ESI/EDI/EBP. The caller supplies a valid return
// slot and writable stack through entry ESP-24, plus the provider's stack needs.
// A compatible normal provider return leaves the five words for this entry to
// discard; ADD supplies the returned arithmetic flags, then plain RET returns
// through the CURRENT caller return slot. Provider nonvolatile preservation,
// return-slot backing, handler state/lifetime and failure behavior remain external.
//
// The current SDK _invalid_parameter_noinfo import supplies Source policy.
// It does not reproduce Native 00BF66EF's encoded-global selection, debugger
// hook, dynamic-handler/Watson tail delivery or original fault/return addresses.
// No noexcept or noreturn promise, handler installation, exception conversion,
// Native ABI equivalence or production-member binding is supplied by this entry.
void __cdecl invoke_native_invalid_parameter_00bf6713();

} // namespace bsp
