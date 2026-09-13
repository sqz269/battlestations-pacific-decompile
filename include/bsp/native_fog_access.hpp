#pragma once
#include <cstdint>

namespace bsp {
// Actual fog-owner accessors. The existing SystemFogOwner is 94h with
// SystemFogState at+8; pass its owner address, not the fields view. Borrow
// this same live owner, including valid caller-initialized directional bytes.
// Names are descriptive hypotheses; scalar semantic units are unestablished.
// Scalar leaves return one FLD value directly in x87 ST0, with no local
// spill, conversion, exception masking or NaN normalization. Original ECX
// receiver/no stack args/RET. The declared fastcall interface also uses ECX.
const void* __fastcall get_native_fog_underwater_color_00b84c90(const void* actual_owner);
float __fastcall load_native_fog_scalar_68_00b84ca0(const void* actual_owner);
float __fastcall load_native_fog_scalar_6c_00b84cb0(const void* actual_owner);
float __fastcall load_native_fog_scalar_70_00b84cc0(const void* actual_owner);
float __fastcall load_native_fog_scalar_74_00b84cd0(const void* actual_owner);
float __fastcall load_native_fog_scalar_78_00b84ce0(const void* actual_owner);
float __fastcall load_native_fog_scalar_7c_00b84cf0(const void* actual_owner);
float __fastcall load_native_fog_scalar_80_00b84da0(const void* actual_owner);
float __fastcall load_native_fog_scalar_84_00b84db0(const void* actual_owner);
float __fastcall load_native_fog_scalar_88_00b84e20(const void* actual_owner);
float __fastcall load_native_fog_scalar_8c_00b84e30(const void* actual_owner);
float __fastcall load_native_fog_scalar_90_00b84e40(const void* actual_owner);

// Native index is one stack DWORD, RET4, EAX=owner+28+(index<<4), wrapping.
// This source interface places index in EDX and uses RET; no range check,
// memory read, retain or null substitution. Valid address domain is required.
const void* __fastcall get_native_fog_directional_color_00b84fd0(const void* actual_owner, std::uint32_t index);

// Existing native_ambient_color_address_00b84c60 supplies owner+8; reused,
// not reconstructed again. No complete system-gatherer, caller FP spill,
// original whole-application ABI, arbitrary concurrency or gameplay claim.
} // namespace bsp
