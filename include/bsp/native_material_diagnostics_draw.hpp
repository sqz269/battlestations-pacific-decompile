#pragma once

#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {

// Complete B16F80 over actual diagnostics storage: byte+681 enable,
// header+684 current grouping name, record vector+68C (stride12Ch).
// Original thiscall ECX diagnostics; six stack words below; RET18h.
// This raw fastcall entry adds the concrete pool context in EDX and keeps
// the original six public argument slots, including current effect/counter
// reads after record construction and returning invalid-parameter handlers.
void __fastcall record_native_material_diagnostics_draw_00b16f80(
    void* actual_diagnostics, NativeStringRawPoolContext& strings,
    const void* effect, std::uint32_t mode, std::uint32_t primitives,
    std::uint32_t vertices, std::uint32_t vertex_shader_registers,
    std::uint32_t pixel_shader_registers);

// Matching traverses current vector bounds and keeps the last matching
// effect/group-name record. Absent records are appended before their names
// are populated. Counter updates wrap at32 bits in the native order, with
// no mode clamp, range repair, rollback or private FH3/frame identity claim.
} // namespace bsp
