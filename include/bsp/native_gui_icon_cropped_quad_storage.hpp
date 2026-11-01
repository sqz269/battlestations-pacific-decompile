#pragma once
#include <cstdint>

namespace bsp {
// Six ordinary caller-retained Native spill words; no initialization, status,
// owner, callback or cleanup. Prepare distinct live backing for each active call.
struct NativeGuiIconCroppedQuadStorageFrame {
    std::uint32_t native_spill_words[6];
};
static_assert(sizeof(NativeGuiIconCroppedQuadStorageFrame) == 24);

// Whole AB1860..AB1ED0 NORMAL leaf,1649B/472ops, no callees or local EH.
// Native ECX Icon/seven stack DWORDs/RET1Ch becomes this ordinary Source ABI.
// Actual argument homes initially hold: stream identity, UV float4 identity,
// leftU, topV, rightU, bottomV, and size float2 identity (pointer words on Win32).
// They are MUTABLE homes: the leaf overwrites homes1/2/3/6 at its reached
// Native spill epochs. Do not pass snapshots, copied scalars or a typed vector.
//
// Actual Icon+114 ratio is ordered-clamped with the genuine live one cell;
// unordered ratios remain unchanged. Read CURRENT+110 after the first UV
// calculations, then preserve every mode0..4/default path, SSE/x87 operation,
// single-precision store boundary, dynamic stream load and 0,1,3,2 write order.
// Outside0..4 still executes the common prefix/clamp then pops and returns.
// E12FD0 is the SAME genuine live Icon vertical-scale cell, distinct from the
// Text E12FD4 cell; no captured value, default0.75 or substitute bank is read.
//
// Caller proves real/live raw Icon, logical stream, current+08 mapping, stride
// +0C/position+10/UV+28, actual UV/size/vertex subobjects and canonical constants.
// All reached offsets/uint32 addresses and storage must be valid in that genuine
// C++ domain; no count, stride, mode, finite, null or bounds guard is added.
// Argument homes/Frame/Source controls and active provider/private extents are
// separate from actual payload/write extents; retain inputs and all real partial
// stores through UNKNOWN escape. Distinct nested calls require distinct backing.
// No T[n], stream Lock, owner/registry/custody, GPU publication or provider birth
// is created. Existing fragment/semantic writer and all callers stay unchanged.
//
// One asm region preserves the payload/FP schedule. EBX/EDI carry real retained
// spill/home addresses; five local Source label addresses replace the Native
// switch table, and private carriers reach the two genuine constant references.
// Native stack/register/RET/EAX/FLAGS/fault/SEH/unmaskedFP/Origin/NoThrow/runtime
// identity is not claimed. No catch, rollback/retry/reset or cleanup is added.
void write_native_gui_icon_cropped_quad_storage_00ab1860(void* actual_icon,
    std::uint32_t (&actual_argument_homes)[7], NativeGuiIconCroppedQuadStorageFrame&,
    const volatile float& actual_one_00d7a24c,
    const volatile float& actual_vertical_scale_00e12fd0);
} // namespace bsp
