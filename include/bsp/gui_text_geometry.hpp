#pragma once
#include "bsp/font_geometry.hpp"
#include "bsp/font_layout.hpp"
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"

namespace bsp {
enum class GuiTextGlyphWriteResult {
    complete,
    // Vertex/index stores and native matching-glyph UV blanking are complete.
    // The original continues atAB9D33 with actual Text child allocation and
    // construction; callers must stop or provide that real continuation.
    needs_glyph_child
};

// Actual mapped-storage projection of AB98F0..AB9D32 and its ordinary returns.
// Original ECX Text, ten stack arguments, RET28h. This interface consumes the
// SAME canonical Text lifetime; it reads that text's current+1D8 width scale,
// +1B0/+1AC/+1A4 substitution fields and SAME owner's+DC branch gate.
// Placement supplies original code-unit/pen; height, quad index and first
// vertex are separate native arguments. Two stack slots not consumed by the
// covered body are omitted. Actual stream+08 is mapped data, +0C/+10/+28/+34..44
// are producer-established writer fields; +64 is its current vertex capacity.
// indices points directly into the actual logical index Lock result.
// There is no vertex/index snapshot, buffer allocation or implicit mapping.
// Caller holds valid nonaliasing mappings of sufficient size, produced with
// zero requested offset as both Text builders do. Unsupported capacity or
// layout rejects before stores; native invalid-input faults are not emulated.
// Reuses the existing x87 quad kernel. Index stores retain native low16 wrap
// and order. Optional child tailAB9D33..AB9FAD is explicitly not executed.
GuiTextGlyphWriteResult write_gui_text_quad_00ab98f0_fragment(
    GuiTextLifetime&, const FontGlyphData&, const FontGlyphPlacement&,
    std::uint32_t quad_index, std::uint16_t height, std::uint32_t first_vertex,
    void* actual_logical_vertex_stream, void* actual_six_indices,
    const volatile float& vertical_scale_00e12fd4);

// AB9FD0 and ABA270 are inspected consumers, not implemented by this packet.
// Their complete material/font state needs an actual font-resource association
// (native glyph+18/+1C), not FontData's scalar-only glyphs or semantic textures.
} // namespace bsp
