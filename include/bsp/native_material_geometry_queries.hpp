#pragma once

#include <cstdint>

namespace bsp {

// The original entries take the actual borrowed native object in ECX and RET.
// They read storage directly; the caller owns object validity and lifetime.
// Names describe observed use and are not recovered source symbols.
std::uint32_t __fastcall get_native_logical_vertex_base_00b48d50(
    const void* actual_logical_vertex_stream);
std::uint32_t __fastcall get_native_logical_index_base_00b48de0(
    const void* actual_logical_index_stream);
std::uint32_t __fastcall get_native_draw_section_instance_count_00b855a0(
    const void* actual_draw_section);

// Original 00B855F0 returns the byte in AL alone; it does not clear EAX[31:8].
std::uint8_t __fastcall get_native_draw_section_indexed_byte_00b855f0(
    const void* actual_draw_section);

} // namespace bsp
