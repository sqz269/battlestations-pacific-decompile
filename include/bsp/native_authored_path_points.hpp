#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native authored path point entries require MSVC Win32.
#endif

namespace bsp {

// Complete007B10A0..007B10B8,25B. Original ECX actual0Ch owner, stacked
// output/index, RET8/EAX captured output; ESI preserved. Capture actual index
// BEFORE loading owner+8 path, then capture output. Call the genuine complete
// raw scene-point entry and return the saved output regardless of child EAX.
// Source EDX is an unused binding placeholder, not an Original parameter.
void* __fastcall get_native_authored_path_point_007b10a0(
    void* actual_owner, void* unused_source_edx, void* actual_output_float3,
    std::uint32_t index);

// Complete007B10C0..007B10FF,64B. Original ECX actual owner, stacked output,
// RET4/EAX captured output; ESI preserved. Capture owner+8 path and path+8 first.
// Null first uses FFFFFFFF; otherwise SAR32(uint32(end-first),2)-1 moduloDWORD.
// Both actual call sites and their output-capture/return schedules remain.
// No empty/index/range repair or replacement virtual facade is introduced.
void* __fastcall get_last_native_authored_path_point_007b10c0(
    void* actual_owner, void* unused_source_edx, void* actual_output_float3);

// Only owned CALL operands change to accepted transform_native_scene_path_point.
// It consumes ECX path and the same stacked output/index; its Source EDX is
// also unused. Actual raw owner/path/pose/point/output/stack/control accesses
// need backing. Returning current SDK policy can repair actual path storage;
// the already captured index remains unchanged. Canonical pose/matrix/x87,
// current CRT policy and Source stack/frame identities retain their declared
// boundaries; no original class/vtable installation, Native runtime or game
// validation is asserted. Older semantic/host interfaces remain distinct.
// Evidence: docs/GOAL15H_AUTHORED_PATH_POINTS.md.

} // namespace bsp
