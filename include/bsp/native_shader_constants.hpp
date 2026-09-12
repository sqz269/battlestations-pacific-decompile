#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {
// Complete 00B21820 and 00B218C0, with a new EDX synchronization binding.
// ECX is borrowed renderer storage; the three stack arguments retain their
// native order and RET 0Ch. Count is captured before optional guard entry;
// data and start are read from the actual callee slots after entry.
// Nonzero uploads use the current device at +1A10 and current COM table,
// ignore HRESULT, then update the actual wrapping DWORD counters. No device
// or renderer validation is added. Zero count skips all renderer/global use.
void __fastcall set_native_vertex_shader_constants_f_00b21820(
    void* actual_renderer, NativeRendererSynchronizationGlobals*,
    std::uint32_t start_register, const float* data, std::uint32_t float4_count);
void __fastcall set_native_pixel_shader_constants_f_00b218c0(
    void* actual_renderer, NativeRendererSynchronizationGlobals*,
    std::uint32_t start_register, const float* data, std::uint32_t float4_count);

// The globals are borrowed from the same actual synchronization domain as
// the renderer. Source C++ cleanup preserves guard lifetime, including an
// exception from the device call. Original FH3/private frame and hardware
// fault/SEH identity are outside this source interface. Cleanup must not
// become enabled after an entry skipped with an uninitialized guard record.
// This is not a drop-in original-caller ABI or a game-validation claim.
} // namespace bsp
