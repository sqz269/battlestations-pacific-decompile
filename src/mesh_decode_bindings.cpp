#include "bsp/mesh_decode_bindings.hpp"
#include "bsp/d3d9_states.hpp"
#include <algorithm>
#include <cstring>

namespace bsp {
namespace {
std::uint32_t read_word_le(const std::uint8_t* source) {
    return std::uint32_t(source[0]) | (std::uint32_t(source[1]) << 8)
        | (std::uint32_t(source[2]) << 16) | (std::uint32_t(source[3]) << 24);
}

void write_word(float& destination, std::uint32_t bits) {
    static_assert(sizeof(destination) == sizeof(bits), "32-bit mesh float required");
    std::memcpy(&destination, &bits, sizeof(bits));
}

void copy_decode_word_x87(float* destination, const std::uint8_t* source) {
    //00B429D4..00B42A12 loads and stores EACH word before reading the next.
    // C++ float assignment may preserve signaling bits using MOVSS instead.
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}

std::int64_t signed_dword(std::uint32_t word) {
    return word <= 0x7fffffffu ? std::int64_t(word)
        : std::int64_t(word) - 0x100000000ll;
}

bool has_records(const MeshVertexStreamPayload& stream) {
    return stream.has_compressed_data;
}
bool has_records(const LogicalVertexStream& stream) {
    return stream.compressed_format_bytes_50.has_value();
}
const std::vector<std::uint8_t>* record_bytes(const MeshVertexStreamPayload& stream) {
    return &stream.compressed_format_bytes;
}
const std::vector<std::uint8_t>* record_bytes(const LogicalVertexStream& stream) {
    return stream.compressed_format_bytes_50 ? &*stream.compressed_format_bytes_50 : nullptr;
}
bool element_count(const MeshVertexStreamPayload& stream, std::uint32_t& count,
    std::string&) {
    count = stream.layout.element_count;
    return true;
}
bool element_count(const LogicalVertexStream& stream, std::uint32_t& count,
    std::string& error) {
    if (!stream.declaration) {
        error = "Mesh decode logical stream has no retained declaration";
        return false;
    }
    static_assert(sizeof(std::size_t) == sizeof(std::uint32_t), "Native Win32 counts");
    count = static_cast<std::uint32_t>(stream.declaration->elements().size());
    return true;
}

template<class Stream>
bool pack_decode(const std::vector<const Stream*>& ordered_streams,
    const ShaderConstantBindings& vertex_bindings,
    const MeshDecodeDescriptorLimits& descriptor, std::uint32_t selector,
    std::vector<float>& vertex_words, MeshDecodeBindingStats& stats,
    std::string& error) {
    stats = {};
    error.clear();
    std::size_t scale_cursor = vertex_bindings.registers[24];
    if (scale_cursor == 0xff) {
        stats.scale_register_absent = true;
        return true;
    }
    std::size_t offset_cursor = vertex_bindings.registers[25];
    if (selector == 2 && !descriptor.element_limit24) {
        error = "Mesh decode selector2 requires the explicit descriptor+24 limit";
        return false;
    }
    const auto gap = std::int64_t(offset_cursor) - std::int64_t(scale_cursor);
    stats.remaining_gap = static_cast<std::uint32_t>(gap);
    stats.remaining_descriptor_limit = selector == 2
        ? *descriptor.element_limit24 : descriptor.element_limit20;
    for (std::size_t stream_index = 0; stream_index < ordered_streams.size(); ++stream_index) {
        if (!stats.remaining_descriptor_limit || !stats.remaining_gap) break;
        const auto* stream = ordered_streams[stream_index];
        if (!stream) {
            error = "Mesh decode draw section contains a null stream";
            return false;
        }
        ++stats.streams_visited;
        //00B4294E/5B captures+50 presence BEFORE the concrete declaration getter.
        const bool compressed = has_records(*stream);
        std::uint32_t declaration_count;
        if (!element_count(*stream, declaration_count, error)) return false;
        const auto count = (std::min)(signed_dword(declaration_count), gap);
        if (count <= 0) continue;
        const auto elements = static_cast<std::uint32_t>(count);
        for (std::uint32_t element = 0; element < elements; ++element) {
            const auto capacity = vertex_words.size() / 4;
            if (scale_cursor >= capacity || offset_cursor >= capacity) {
                error = "Mesh decode constants exceed the VS destination word capacity";
                return false;
            }
            const auto scale_word = scale_cursor * 4;
            const auto offset_word = offset_cursor * 4;
            if (compressed) {
                //00B61E10 rereads stream+50 for EACH record. Do not cache all
                // source values, nor pass them through a float-return helper.
                const auto* bytes = record_bytes(*stream);
                if (!bytes || element >= bytes->size() / 0x20) {
                    error = "Mesh decode record exceeds current compressed metadata bounds";
                    return false;
                }
                const auto* source = bytes->data() + static_cast<std::size_t>(element) * 0x20;
                for (std::size_t lane = 0; lane < 4; ++lane)
                    copy_decode_word_x87(&vertex_words[scale_word + lane], source + lane * 4);
                for (std::size_t lane = 0; lane < 4; ++lane)
                    copy_decode_word_x87(&vertex_words[offset_word + lane], source + 0x10 + lane * 4);
                ++stats.records_from_metadata;
            } else {
                //00D7A24C is raw3F800000; native identity branch uses MOVSS.
                for (std::size_t lane = 0; lane < 4; ++lane)
                    write_word(vertex_words[scale_word + lane], 0x3f800000u);
                for (std::size_t lane = 0; lane < 4; ++lane)
                    write_word(vertex_words[offset_word + lane], 0u);
            }
            ++scale_cursor;
            ++offset_cursor;
            ++stats.records_written;
            --stats.remaining_gap;
            --stats.remaining_descriptor_limit;
        }
    }
    return true;
}
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
    for (std::size_t lane = 0; lane < 4; ++lane) {
        write_word(record.scale[lane], read_word_le(source + lane * 4));
        write_word(record.offset[lane], read_word_le(source + 0x10 + lane * 4));
    }
    std::memcpy(&output, &record, sizeof(record));
    error.clear();
    return true;
}

bool pack_mesh_vertex_decode_constants_00b428c0(
    const std::vector<const MeshVertexStreamPayload*>& ordered_streams,
    const ShaderConstantBindings& vertex_bindings,
    const MeshDecodeDescriptorLimits& descriptor, std::uint32_t selector,
    std::vector<float>& vertex_words, MeshDecodeBindingStats& stats,
    std::string& error) {
    return pack_decode(ordered_streams, vertex_bindings, descriptor, selector,
        vertex_words, stats, error);
}

bool pack_mesh_vertex_decode_constants_00b428c0(
    const std::vector<const LogicalVertexStream*>& ordered_streams,
    const ShaderConstantBindings& vertex_bindings,
    const MeshDecodeDescriptorLimits& descriptor, std::uint32_t selector,
    std::vector<float>& vertex_words, MeshDecodeBindingStats& stats,
    std::string& error) {
    return pack_decode(ordered_streams, vertex_bindings, descriptor, selector,
        vertex_words, stats, error);
}
}
