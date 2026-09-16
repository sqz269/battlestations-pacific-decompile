#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer record append requires MSVC Win32.
#endif

namespace bsp {
struct NativeRendererRecordGuardContext;

// B257B0..B25821: ECX actual 0Ch header, stacked live 28h input, RET4.
// The explicit unused EDX adapts the register ABI to fastcall. Reserve is the
// existing B22A70 provider. DWORD wrapping, eight x87 load/store conversions,
// live aliases, computed-null destination and final current-used reload remain.
// Input/header/output extents must be valid for the native access schedule;
// no private-stack alias, hardware-fault unwind or floating-environment repair.
void __fastcall append_native_renderer_records40_00b257b0(
    void* actual_header, std::uint32_t unused_edx, const void* live_record40);

// B29330..B2942E: native ECX renderer, ten stack DWORDs, RET28h.
// Source interface borrows the same AA0/D5A0 guard context and actual renderer
// +1D18 header. Float arguments are raw bits: the public wrapper uses MOVSS,
// and only the append performs x87 conversions. retained_owner is opaque;
// nonnull requires its real writable interlocked reference count at +04.
// Retain precedes append and is NOT compensated if append throws. Only the
// captured native guard is cleaned up. No manufactured owner or terminal.
// This extra context/source ABI does not reproduce native FH3/private frames.
void append_native_renderer_debug_record40_00b29330(
    void* actual_renderer, const NativeRendererRecordGuardContext& guard_context,
    void* retained_owner, std::uint32_t x_bits, std::uint32_t y_bits,
    std::uint32_t width_bits, std::uint32_t height_bits, std::uint32_t color,
    std::uint32_t u0_bits, std::uint32_t v0_bits,
    std::uint32_t u1_bits, std::uint32_t v1_bits);
} // namespace bsp
