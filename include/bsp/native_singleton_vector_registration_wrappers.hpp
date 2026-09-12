#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native singleton vector registration wrappers require MSVC Win32.
#endif

namespace bsp {

// Complete BD08D0[137]. ECX original container, EDX unconsumed; four stack
// arguments in this order, EAX result_iterator, RET10h. The result is raw
// eight-byte storage: owner at+0 and position at+4. Position is written first.
// Null begin or zero signed-shift size selects index0 and skips iterator-owner
// validation. Otherwise a returning null/foreign-owner validation continues
// insertion in the original container with the original iterator owner.
void* __fastcall insert_one_native_singleton_slots_checked_00bd08d0(
    void* owner, void* unused_edx, void* result_iterator,
    const void* iterator_owner, void* position, const void* value_slot);

// Complete BD0BC0[105]. ECX actual container, EDX unconsumed, one stack
// value-slot pointer, RET4. The capacity path loads the current DWORD value
// into captured end, then publishes that end+4. Growth passes captured end
// and an actual eight-byte stack result buffer to the checked insertion.
void __fastcall append_native_singleton_slot_00bd0bc0(
    void* owner, void* unused_edx, const void* value_slot);

// Complete BD0C30[39]. ECX actual manager/container, EDX unconsumed, one stack
// object pointer, RET4. Validate bounds before testing the current argument
// for null. A nonnull argument is passed by its actual stack-slot address.
// No lock, duplicate check, retain operation or exception frame is installed.
void __fastcall register_native_singleton_object_00bd0c30(
    void* owner, void* unused_edx, void* object);

// All entries preserve raw32 arithmetic, actual argument/result slots and the
// captured/current read schedule. Native BF6713 calls use a fixed real SDK
// _invalid_parameter_noinfo shim and its current source-CRT handler domain.
// It can return; no noop/fatal/throw callback is substituted. Original encoded
// global109DD64, Watson internals and service register/exception identities
// remain explicit boundaries. Source BD0700 and its allocation/exception
// providers are required; these wrappers do not create another owner domain.
// Append/register have no semantic EAX result. No native lock or cleanup frame
// exists in these three bodies, and none is invented in the source entries.
} // namespace bsp
