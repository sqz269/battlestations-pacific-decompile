#include "bsp/mesh_buffers.hpp"
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
bool read_payload(StructuredNode& node, std::uint32_t count, std::uint32_t width,
    std::vector<std::uint8_t>& output, std::string& error) {
    const std::uint64_t length = static_cast<std::uint64_t>(count) * width;
    if (length > std::numeric_limits<std::uint32_t>::max()) {
        error = "Mesh payload size exceeds the host DWORD extent";
        return false;
    }
    if (length > node.remaining()) {
        error = "Mesh payload exceeds its remaining node bytes";
        return false;
    }
    try {
        std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
        if (!node.read_bytes(bytes.data(), static_cast<std::uint32_t>(length))) {
            error = "Mesh raw payload read failed";
            return false;
        }
        output = std::move(bytes);
        return true;
    } catch (const std::bad_alloc&) {
        error = "Mesh payload allocation failed";
        return false;
    } catch (const std::length_error&) {
        error = "Mesh payload exceeds the host byte vector capacity";
        return false;
    }
}

bool is_rope_format(const std::string& name) {
    constexpr char expected[] = "rope.mvfm";
    for (std::size_t index = 0; index != sizeof(expected); ++index) {
        // Native stricmp observes the first NUL; stored format names retain all
        // bytes, including any bytes beyond a serialized embedded NUL.
        const unsigned char value = index < name.size()
            ? static_cast<unsigned char>(name[index]) : 0;
        const unsigned char folded = value >= 'A' && value <= 'Z'
            ? static_cast<unsigned char>(value + ('a' - 'A')) : value;
        if (folded != static_cast<unsigned char>(expected[index])) return false;
    }
    return true;
}
}

bool read_mesh_indices_00b93aa0(StructuredNode& node, MeshIndexPayload& output,
    std::string& error) {
    MeshIndexPayload value;
    if (!node.read_u32(value.count) || !node.read_u32(value.format)) {
        error = "Mesh Indices header read failed";
        return false;
    }
    if (value.format == 0x65) value.index_width = 2;
    else if (value.format == 0x66) value.index_width = 4;
    else {
        error = "Unsupported Mesh index format";
        return false;
    }
    if (!read_payload(node, value.count, value.index_width, value.bytes, error))
        return false;
    output = std::move(value);
    error.clear();
    return true;
}

bool read_mesh_vertex_stream_00b93e60(StructuredNode& node,
    const MeshVertexFormatResolver& resolve_format, MeshVertexStreamPayload& output,
    std::string& error) {
    MeshVertexStreamPayload value;
    if (!node.read_u32(value.count) || !node.read_string(value.format_name)) {
        error = "Mesh VertexStream header read failed";
        return false;
    }
    if (!resolve_format) {
        error = "Mesh VertexStream requires a named declaration resolver";
        return false;
    }
    if (!resolve_format(value.format_name, value.layout, error)) {
        if (error.empty()) error = "Mesh VertexStream declaration resolution failed";
        return false;
    }
    if (!read_payload(node, value.count, value.layout.stride, value.bytes, error))
        return false;
    if (is_rope_format(value.format_name) && !node.skip_00be9c40()) {
        error = "Mesh rope VertexStream tail skip failed";
        return false;
    }
    output = std::move(value);
    error.clear();
    return true;
}

bool read_mesh_compressed_vertex_format_data_00b93800(StructuredNode& node,
    MeshVertexStreamPayload& last_stream, std::string& error) {
    std::vector<std::uint8_t> bytes;
    if (!read_payload(node, last_stream.layout.element_count, 0x20, bytes, error))
        return false;
    last_stream.compressed_format_bytes = std::move(bytes);
    last_stream.has_compressed_data = true;
    error.clear();
    return true;
}
}
