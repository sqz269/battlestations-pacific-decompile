#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native singleton removal and reorder require MSVC Win32.
#endif

namespace bsp {

// Complete BCFCA0[115]. ECX raw owner, EDX unconsumed; one stack object
// pointer, RET4, no semantic result. Null object touches no owner fields.
// Recheck current count/bounds while scanning; clear only the first matching
// slot through the current begin. Length, capacity and later duplicates stay.
void __fastcall unregister_native_singleton_object_00bcfca0(
    void* owner, void* unused_edx, void* object);

// Complete BD0D70[474]. ECX raw owner, EDX unconsumed; stack object, anchor,
// RET8, no semantic result. Remove the first object by shifting the tail and
// decrementing current end, then insert it after the first remaining anchor.
// Both searches are required: missing matches are not converted into no-ops.
// Preserve returning validations, actual argument/result slots, captures and
// reloads, inline append and both calls to the full checked insert provider.
void __fastcall move_native_singleton_object_after_00bd0d70(
    void* owner, void* unused_edx, void* object, void* anchor);

// Actual raw fields: untouched DWORD+0, begin+4, end+8, capacity-end+0C.
// No typed projection, synchronization or rollback is added. Fixed providers
// are the completed raw count/checked-insert entries and actual SDK memmove_s
// and _invalid_parameter_noinfo. Source CRT handler, heap, errno and C++
// exception domains differ from the original static CRT. Native service
// register/flag, hardware-fault and SEH identity are not claimed. These entries
// do not perform or complete registered-owner destruction dispatch.
} // namespace bsp
