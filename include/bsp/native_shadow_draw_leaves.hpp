#pragma once
#include <cstdint>

namespace bsp {
struct NativePlaneRecord;

// Complete B48D00[4]: ECX actual logical stream, EAX current DWORD+58, RET.
// No public stack argument, retain or validation. At AE492F/AE4A6F the
// already-pushed byte offset belongs to the later B4A9B0 constructor.
void* __fastcall native_logical_vertex_physical_00b48d00(const void* actual_stream);

// Complete B6FDC0[11]: add2F4 to ECX, tail-jump full B656F0. Its RET4
// consumes only the original public index; lower caller stack words survive.
// EDX is unused. Wrapped DWORD address arithmetic, no count clamp, frustum
// update or result dereference. Names are hypotheses; these new C++ entries
// do not establish the missing AE47E0 caller or original game integration.
const NativePlaneRecord* __fastcall native_camera_plane_00b6fdc0(
    const void* actual_camera, void* unused_edx, std::uint32_t raw_index);
} // namespace bsp
