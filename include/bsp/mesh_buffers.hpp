#pragma once
#include "bsp/structured_reader.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace bsp {
// Native declaration +cch and +10h. Resolve from the actual named format;
// neither value is serialized in VertexStream or inferred from payload length.
struct MeshVertexFormatLayout {
    std::uint32_t stride{};
    std::uint32_t element_count{};
};
using MeshVertexFormatResolver = std::function<bool(const std::string&,
    MeshVertexFormatLayout&, std::string&)>;

struct MeshIndexPayload {
    std::uint32_t count{};
    std::uint32_t format{}; // Native 65h = 16-bit, 66h = 32-bit indices.
    std::uint32_t index_width{};
    std::vector<std::uint8_t> bytes; // Exact little-endian serialized bytes.
};

struct MeshVertexStreamPayload {
    std::uint32_t count{};
    std::string format_name;
    MeshVertexFormatLayout layout;
    std::vector<std::uint8_t> bytes;
    bool has_compressed_data{};
    // Exact 20h bytes per declaration element. The handler only attaches this
    // allocation; it does not decompress it or establish its numeric fields.
    std::vector<std::uint8_t> compressed_format_bytes;
};

// Host complete-transfer projection; not native ABI or graphics allocation.
// Evidence: docs/MESH_VERTEX_INDEX_PAYLOADS.md. Outputs commit on success only.
// Ordinary success consumes the proved fields without closing/skipping a node.
// Native rope.mvfm handling explicitly skips and detaches any remaining tail.
bool read_mesh_indices_00b93aa0(StructuredNode& node, MeshIndexPayload& output,
    std::string& error);
bool read_mesh_vertex_stream_00b93e60(StructuredNode& node,
    const MeshVertexFormatResolver& resolve_format, MeshVertexStreamPayload& output,
    std::string& error);
// Caller supplies the last appended stream, as selected by the native handler.
bool read_mesh_compressed_vertex_format_data_00b93800(StructuredNode& node,
    MeshVertexStreamPayload& last_stream, std::string& error);
}
