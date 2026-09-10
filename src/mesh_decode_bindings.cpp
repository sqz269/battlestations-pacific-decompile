#include "bsp/mesh_decode_bindings.hpp"
#include <algorithm>
#include <cstring>

namespace bsp {
namespace {
float read_float_le(const std::uint8_t* source) {
    const std::uint32_t bits = std::uint32_t(source[0])
        | (std::uint32_t(source[1]) << 8)
        | (std::uint32_t(source[2]) << 16)
        | (std::uint32_t(source[3]) << 24);
    float value;
    static_assert(sizeof(value) == sizeof(bits), "32-bit mesh float required");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::int64_t signed_dword(std::uint32_t word) {
    return word <= 0x7fffffffu ? std::int64_t(word)
        : std::int64_t(word) - 0x100000000ll;
}

struct PendingDecodeWrite {
    std::size_t scale_word{}, offset_word{};
    MeshVertexDecodeRecord record;
};
}

bool read_mesh_vertex_decode_record_00b61e10(const MeshVertexStreamPayload& stream,
    std::uint32_t element_index, MeshVertexDecodeRecord& output,
    std::string& error) {
    if (!stream.has_compressed_data) {
        error = "Mesh decode record metadata is absent";
        return false;
    }
    if (element_index >= stream.layout.element_count
        || element_index >= stream.compressed_format_bytes.size() / 0x20) {
        error = "Mesh decode record index exceeds declaration or metadata bounds";
        return false;
    }
    const auto* source = stream.compressed_format_bytes.data()
        + static_cast<std::size_t>(element_index) * 0x20;
    MeshVertexDecodeRecord record;
    for (std::size_t i = 0; i < 4; ++i) {
        record.scale[i] = read_float_le(source + i * 4);
        record.offset[i] = read_float_le(source + 0x10 + i * 4);
    }
    output = record;
    error.clear();
    return true;
}

bool pack_mesh_vertex_decode_constants_00b428c0(
    const std::vector<const MeshVertexStreamPayload*>& ordered_streams,
    const ShaderConstantBindings& vertex_bindings,
    const MeshDecodeDescriptorLimits& descriptor, std::uint32_t selector,
    std::vector<float>& vertex_words, MeshDecodeBindingStats& stats,
    std::string& error) {
    MeshDecodeBindingStats result;
    std::size_t scale_cursor = vertex_bindings.registers[24];
    std::size_t offset_cursor = vertex_bindings.registers[25];
    if (scale_cursor == 0xff) {
        result.scale_register_absent = true;
        stats = result;
        error.clear();
        return true;
    }
    if (selector == 2 && !descriptor.element_limit24) {
        error = "Mesh decode selector2 requires the explicit descriptor+24 limit";
        return false;
    }
    const auto gap = std::int64_t(offset_cursor) - std::int64_t(scale_cursor);
    result.remaining_gap = static_cast<std::uint32_t>(gap);
    result.remaining_descriptor_limit = selector == 2
        ? *descriptor.element_limit24 : descriptor.element_limit20;

    // Stage values as well as addresses so a late host bounds failure cannot
    // partially alter the shared VS register vector. This is a host safeguard,
    // not a reconstructed native error path or a different loop clamp.
    std::vector<PendingDecodeWrite> writes;
    for (const auto* stream : ordered_streams) {
        if (!result.remaining_descriptor_limit || !result.remaining_gap) break;
        if (!stream) {
            error = "Mesh decode draw section contains a null stream";
            return false;
        }
        ++result.streams_visited;
        const auto count = (std::min)(signed_dword(stream->layout.element_count), gap);
        if (count <= 0) continue;
        const auto elements = static_cast<std::size_t>(count);
        const auto capacity = vertex_words.size() / 4;
        if (scale_cursor > capacity || elements > capacity - scale_cursor
            || offset_cursor > capacity || elements > capacity - offset_cursor) {
            error = "Mesh decode constants exceed the VS destination word capacity";
            return false;
        }
        for (std::uint32_t element = 0; element < elements; ++element) {
            PendingDecodeWrite write;
            write.scale_word = (scale_cursor + element) * 4;
            write.offset_word = (offset_cursor + element) * 4;
            if (stream->has_compressed_data) {
                if (!read_mesh_vertex_decode_record_00b61e10(*stream, element,
                    write.record, error)) return false;
                ++result.records_from_metadata;
            }
            writes.push_back(write);
        }
        scale_cursor += elements;
        offset_cursor += elements;
        result.records_written += elements;
        result.remaining_gap -= static_cast<std::uint32_t>(elements);
        result.remaining_descriptor_limit -= static_cast<std::uint32_t>(elements);
    }
    for (const auto& write : writes) {
        for (std::size_t i = 0; i < 4; ++i)
            vertex_words[write.scale_word + i] = write.record.scale[i];
        for (std::size_t i = 0; i < 4; ++i)
            vertex_words[write.offset_word + i] = write.record.offset[i];
    }
    stats = result;
    error.clear();
    return true;
}
}
