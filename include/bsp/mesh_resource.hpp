#pragma once
#include "bsp/mesh_buffers.hpp"
#include "bsp/mesh_fields.hpp"
#include <optional>

namespace bsp {
// Owning decoded wire values. No native mesh/item/material/GPU object or
// retained draw-section binding is constructed by this representation.
struct MeshResourcePayload {
    //00b944e0 writes this leading DWORD to native output pair+4; meaning unknown.
    std::uint32_t prefix_word{};
    std::vector<MeshVertexStreamPayload> vertex_streams;
    std::optional<MeshIndexPayload> indices;
    std::vector<MeshSubsetFields> subsets;
    //00b73d70 initializes mesh+0Ch to1.0; counts and index pointer start zero.
    float lod_value{1.0f};
    std::vector<MeshLodPhase> lod_phases;
    std::vector<std::string> weight_map_names;
    std::vector<std::string> unknown_tags;
    // Original counted tag bytes, in encounter order, including unknown and
    // consume-only fields. This trace does not retain overwritten resources.
    std::vector<std::string> field_order;
};

//00b944e0 native ABI: ECX output pair, stack node-handle pointer, RET4.
// Calls field parsers in encounter order; repeated Indices/LODValue replace,
// streams/subsets/phases/names append, and compression targets the last stream
// already appended. Compression without a preceding stream is rejected by the
// host. Recognized children close without skipping an unread tail; unknown
// children explicitly skip. Input remains attached for the enclosing parser.
// Output commits only on success; consumed input is not rolled back on error.
// Does not synthesize stream-zero selections or resolve effect/texture names.
// Evidence: docs/MESH_SUBSET_LOD_FIELDS.md and the referenced buffer audit.
bool parse_mesh_resource_00b944e0(StructuredNode& node,
    const MeshVertexFormatResolver& resolve_format, MeshResourcePayload& output,
    std::string& error);
}
