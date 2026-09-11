#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera projection requires MSVC Win32 x87 and MOVSS.
#endif

namespace bsp {

// Full 412E20..412E33. Stack DWORD contains the original float argument bits;
// FSINCOS, divide, float spill and reload return ST0; RET4. This is the full
// native hardware helper, not a CRT tangent or C++ floating conversion.
float __stdcall native_projection_tangent_00412e20(std::uint32_t argument_bits);

// Full B642F0..B643A0. Original ECX destination; four stack float DWORDs
// fov/aspect/near/far, EAX destination, RET10h. New fastcall takes ECX raw
// 64-byte destination and EDX writable 16-byte CALLEE argument-slot storage,
// returns the destination in EAX, and uses plain RET.
//
// These slots represent copies prepared by the caller, NOT camera-owner
// fields. Fov slot+0 is overwritten by half-angle then reciprocal scale;
// far slot+0C receives rounded far/(far-near). A future native getter must
// perform its original ordered argument stores into temporary slots first.
// Output/argument-slot overlap retains ordered current reads and stores.
void* __fastcall build_native_camera_projection_00b642f0(
    void* actual_destination_float16, void* actual_callee_argument_slots_float4);

// No owner mutation policy, checks, angle conversion, clipping repair,
// floating-control changes or exception rollback are added. Reconstructed
// readonly constants exactly match immutable D7A280 double0.5/D7A24C float1.
// Native caller ABI integration and gameplay are separate claims.

} // namespace bsp
