#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Full B1F740 (4 bytes): ECX viewport, EAX viewport+10, RET. Computes an
// address only; does not dereference the viewport or read its dimensions.
void* __fastcall native_viewport_size_address_00b1f740(const void*) noexcept;

// Full B48CD0 (4 bytes): ECX logical stream, EAX DWORD at+64, RET.
std::uint32_t __fastcall native_logical_vertex_count_00b48cd0(const void*) noexcept;

struct NativeRendererIndexedDrawContext {
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    // Borrow the original D61D6C profile; current slot20 is full B48CD0.
    // This operation only queries a borrowed stream, without lifetime work.
    const volatile std::uint32_t* actual_logical_vertex_profile_00d61d6c;
};

// Complete B24010..B2415B. Original ECX renderer; stack primitive type,
// minimum vertex, vertex count, start index, primitive count; RET14h.
// Inhibit1D90/lost1D8A gates precede viewport1904 read and optional guard.
// Zero-count paths use the mode captured after guard entry. Nonzero paths
// query current stream0+20; excess count is rejected only for CURRENT
// stream0 tag54==40000001. Device/table/base vertex remain current reads.
// DrawIndexedPrimitive uses table148, ignores HRESULT and changes no counter.
void draw_native_renderer_indexed_00b24010(void* actual_renderer,
    std::uint32_t primitive_type, std::uint32_t minimum_vertex,
    std::uint32_t vertex_count, std::uint32_t start_index,
    std::uint32_t primitive_count, NativeRendererIndexedDrawContext&);

// New explicit-context interface, concrete logical profile, borrowed storage.
// Skipped native guard fields are uninitialized; mode changes requiring those
// fields are outside its valid domain. This does not implement stream binding,
// stream ownership, original caller ABI or gameplay.

} // namespace bsp
