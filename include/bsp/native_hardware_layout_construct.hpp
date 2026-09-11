#pragma once

#include "bsp/native_hardware_layout_fields.hpp"
#include "bsp/native_string.hpp"

namespace bsp {

// Borrow the same actual renderer slot, support/lifetime services, tree and
// initialized declaration/hardware pools used by the hardware owner routines.
// NativeStringStorage is the existing explicit native sized-string boundary.
struct NativeHardwareLayoutConstructContext {
    NativeHardwareLayoutOwnerContext& actual_owner;
    NativeStringStorage& actual_string_storage;
};

// B60790: ECX actual 44h owner, stack stream-list pointer, RET4. The list has
// four actual declaration pointer words then signed count at+10h. Append each
// reached stream before reading its current raw data/count at+0Ch/+10h; make
// both diagnostic strings and access actual support once per reached stream.
// Read the current renderer, its device+1A10h and current COM slot+158h; pass
// owner+40h directly as output. Ignore HRESULT and recompute current stride.
// Existing owner+40h is neither released nor precleared by this function.
//
// Supported valid-storage domain: reached input stream indices 0..3, every
// append target valid, full raw usage DWORD 0..13, at most15 total raw elements
// across reached streams, plus END in the original fixed16-element scratch.
// Counts are reread at native points; zero/negative initial counts are valid.
// These are preconditions, not added native runtime checks. Inputs that make
// native stores overwrite the adjacent live count/exception frame are outside
// the source interface. No unbounded vector or initialized unused slots.
void create_native_hardware_layout_from_streams_00b60790(
    void* actual_owner, const void* actual_stream_list,
    NativeHardwareLayoutConstructContext&);

// B60CB0: ECX actual44h owner, stack actual stream list, EAX same owner, RET4.
// Complete B48C00, arm base B48960 cleanup, clear+40h, install D62AF4, create.
// Failure invokes the base cleanup only; no derived COM release or pool return.
void* construct_native_hardware_layout_00b60cb0(
    void* actual_owner, const void* actual_stream_list,
    NativeHardwareLayoutConstructContext&);

// New MSVC Win32 C++ interfaces; no original binary ABI or game validation.
} // namespace bsp
