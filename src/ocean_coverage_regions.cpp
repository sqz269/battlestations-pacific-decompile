// Packet cc9_physics1_sea: the ocean's coverage regions. See the header and
// docs/OCEAN_WAVE_FIELD.md section 10 for the chain and what is labelled.
#include "bsp/ocean_coverage_regions.hpp"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <optional>

#include "bsp/memory_stream.hpp"
#include "bsp/mesh_resource.hpp"
#include "bsp/structured_model.hpp"
#include "bsp/structured_reader.hpp"
#include "bsp/structured_resource_registry.hpp"
#include "bsp/vertex_format.hpp"

namespace bsp {
namespace {

constexpr float kBoxStart = 1.0e10f;          // 00CE4970 (bits 501502F9h)
constexpr unsigned kMaskLevelSize = 0x80;     // 00B9E1B1
constexpr const char* kErrorTexture = "error.tga";  // 00B31BD0

std::uint32_t u32_at(const std::string& b, std::size_t o) {
    std::uint32_t v;
    std::memcpy(&v, b.data() + o, 4);
    return v;
}

// d3dx9_40's colour-to-red rule as fitted (header). Every step rounds to float.
std::uint8_t to_byte(float v) {
    const float scaled = v * 255.0f;
    return static_cast<std::uint8_t>(static_cast<int>(scaled + 0.5f));
}

void decode_red_palette(std::uint16_t c0, std::uint16_t c1, bool four_colour,
                        std::array<std::uint8_t, 4>& red) {
    const float k = 1.0f / 31.0f;
    const float a = static_cast<float>(c0 >> 11) * k;
    const float b = static_cast<float>(c1 >> 11) * k;
    red[0] = to_byte(a);
    red[1] = to_byte(b);
    if (four_colour) {
        const float third = 1.0f / 3.0f;
        red[2] = to_byte(a + (b - a) * third);
        red[3] = to_byte(b + (a - b) * third);
    } else {
        red[2] = to_byte(a + (b - a) * 0.5f);
        red[3] = 0;
    }
}

// D3DXFloat16To32Array as a plain conversion. LABELLED: exponent 31 is taken as
// a normal number (D3DX's half has no infinities), never reached by the
// terrain streams.
float half_to_float(std::uint16_t h) {
    const std::uint32_t sign = (h >> 15) & 1u;
    const std::uint32_t exponent = (h >> 10) & 31u;
    const std::uint32_t mantissa = h & 1023u;
    float value;
    if (exponent == 0) value = static_cast<float>(mantissa) * (1.0f / 16777216.0f);
    else value = std::ldexp(1.0f + static_cast<float>(mantissa) / 1024.0f,
                            static_cast<int>(exponent) - 15);
    return sign ? -value : value;
}

// The position element's D3DDECLTYPE from the .mvfm name, which leads with
// `p` in every terrain stream this installation carries.
std::optional<int> position_type(const std::string& format) {
    if (format.size() < 4 || (format[0] != 'p' && format[0] != 'P')) return std::nullopt;
    const std::string token = format.substr(1, 3);
    if (token == "f43") return 2;
    if (token == "f44") return 3;
    if (token == "f24") return 16;
    return std::nullopt;
}

// 004768D0 for one vertex of a decoded stream: float3 at offset 0 when the
// stream has no compressed format data (+50h zero); otherwise the typed lanes
// times record 0's scale (+0h) plus its bias (+10h).
bool read_position(const MeshVertexStreamPayload& stream, int type, std::uint32_t index,
                   float (&out)[3]) {
    const std::size_t at = static_cast<std::size_t>(stream.layout.stride) * index;
    if (at + 12 > stream.bytes.size()) return false;
    const std::uint8_t* p = stream.bytes.data() + at;
    if (!stream.has_compressed_data) {
        std::memcpy(out, p, 12);
        return true;
    }
    float local[3];
    if (type == 16) {
        std::uint16_t h[3];
        std::memcpy(h, p, 6);
        for (int i = 0; i != 3; ++i) local[i] = half_to_float(h[i]);
    } else {
        std::memcpy(local, p, 12);
    }
    if (stream.compressed_format_bytes.size() < 0x20) return false;
    float record[8];
    std::memcpy(record, stream.compressed_format_bytes.data(), 0x20);
    for (int i = 0; i != 3; ++i) {
        const float scaled = record[i] * local[i];
        out[i] = scaled + record[4 + i];
    }
    return true;
}

// Row-vector 4x4 product a * b.
void multiply(const float a[16], const float b[16], float out[16]) {
    float r[16];
    for (int i = 0; i != 4; ++i)
        for (int j = 0; j != 4; ++j) {
            float s = 0.0f;
            for (int k = 0; k != 4; ++k) s += a[i * 4 + k] * b[k * 4 + j];
            r[i * 4 + j] = s;
        }
    std::memcpy(out, r, sizeof(r));
}

std::string base_name(const std::string& effect) {
    const std::size_t slash = effect.rfind('/');  // 00CE7898 is "/" alone
    return slash == std::string::npos ? effect : effect.substr(slash + 1);
}

}  // namespace

bool ocean_coverage_read_mask_00b9e170(const std::string& dds, int texture_reduction,
    std::vector<std::uint8_t>& bitmap, float& width, float& height, std::string& error) {
    if (dds.size() < 128 || std::memcmp(dds.data(), "DDS ", 4) != 0) {
        error = "not a DDS file";
        return false;
    }
    const std::uint32_t flags = u32_at(dds, 8);
    const std::uint32_t h0 = u32_at(dds, 12);
    const std::uint32_t w0 = u32_at(dds, 16);
    // D3DXGetImageInfo's MipLevels: the count when DDSD_MIPMAPCOUNT is set and
    // nonzero, else 1 (leyte_oceanwavedamp.dds; matches the probe).
    std::uint32_t levels = (flags & 0x20000u) && u32_at(dds, 28) ? u32_at(dds, 28) : 1u;
    const char* fourcc = dds.data() + 84;
    const bool dxt1 = std::memcmp(fourcc, "DXT1", 4) == 0;
    const bool dxt5 = std::memcmp(fourcc, "DXT5", 4) == 0;
    if (!dxt1 && !dxt5) {
        error = "texture format not decoded by this host";
        return false;
    }
    if (levels > 1 && texture_reduction != 0) {
        error = "texture reduction rescales through D3DX; not reproduced";
        return false;
    }
    std::uint32_t size = w0;  // 00B9E1A8: the texture's width
    std::uint32_t level = 0;
    while (size > kMaskLevelSize) {
        size >>= 1;
        ++level;
    }
    if (size == 0) {
        error = "zero-sized texture (00D63960)";
        return false;
    }
    if (level >= levels) {
        error = "mask level past the texture's levels (00D6392C)";
        return false;
    }
    const std::size_t block_bytes = dxt1 ? 8 : 16;
    std::size_t offset = 128;
    std::uint32_t w = w0, h = h0;
    for (std::uint32_t i = 0; i != level; ++i) {
        offset += static_cast<std::size_t>((w + 3) / 4) * ((h + 3) / 4) * block_bytes;
        w = w > 1 ? w / 2 : 1;
        h = h > 1 ? h / 2 : 1;
    }
    const std::uint32_t blocks_wide = (w + 3) / 4;
    const std::uint32_t blocks_high = (h + 3) / 4;
    if (offset + static_cast<std::size_t>(blocks_wide) * blocks_high * block_bytes > dds.size()) {
        error = "truncated DDS level";
        return false;
    }
    bitmap.assign(static_cast<std::size_t>(w) * h, 0);
    for (std::uint32_t by = 0; by != blocks_high; ++by) {
        for (std::uint32_t bx = 0; bx != blocks_wide; ++bx) {
            const std::size_t at = offset + (static_cast<std::size_t>(by) * blocks_wide + bx)
                * block_bytes + (dxt5 ? 8 : 0);
            std::uint16_t c0, c1;
            std::memcpy(&c0, dds.data() + at, 2);
            std::memcpy(&c1, dds.data() + at + 2, 2);
            const std::uint32_t bits = u32_at(dds, at + 4);
            std::array<std::uint8_t, 4> red;
            decode_red_palette(c0, c1, dxt5 || c0 > c1, red);
            for (std::uint32_t py = 0; py != 4; ++py) {
                for (std::uint32_t px = 0; px != 4; ++px) {
                    const std::uint32_t x = bx * 4 + px, y = by * 4 + py;
                    if (x >= w || y >= h) continue;
                    bitmap[static_cast<std::size_t>(y) * w + x] =
                        red[(bits >> (2 * (py * 4 + px))) & 3u];
                }
            }
        }
    }
    width = static_cast<float>(w);   // +28h
    height = static_cast<float>(h);  // +2Ch
    return true;
}

std::string ocean_coverage_add_landscape_00ba1370(OceanCoverageRegions& out,
    const std::string& model_bytes, const float landscape_world[16],
    const OceanCoverageFileReader& read_file, int texture_reduction) {
    if (model_bytes.empty()) return "regions=0 (model not readable)";
    auto stream = std::make_shared<MemoryStream>(
        memory_stream_from_complete_bytes(model_bytes.data(), model_bytes.size()));
    StructuredReader reader(stream);
    auto root = reader.read_root_00bea700();
    if (!root || root->tag() != "MMOD") return "regions=0 (no MMOD root)";
    MeshStructuredResourceParser mesh_parser(resolve_mesh_vertex_format_layout_00b2dbd0);
    NoteStructuredResourceParser note_parser;
    GroupParamsStructuredResourceParser group_parser;
    StructuredResourceRegistry registry;
    registry.register_parser(mesh_parser);
    registry.register_parser(note_parser);
    registry.register_parser(group_parser);
    RendererRootHooks_00d5f0a8 hooks;
    StructuredModel model;
    std::string error;
    if (!dispatch_root_00b7f430(*root, registry, model, hooks, error)) {
        return "regions=0 (model parse: " + error + ")";
    }

    // World frames, parents first (every item's parent index precedes it).
    std::vector<std::array<float, 16>> world(model.hierarchy.size());
    std::vector<bool> placed(model.hierarchy.size(), false);
    std::size_t added = 0, sections = 0, refused = 0;
    std::string notes;
    for (std::size_t i = 0; i != model.hierarchy.size(); ++i) {
        const HierarchyItem& item = model.hierarchy[i];
        if (!item.matrix) continue;  // native leaves it uninitialised: no claim
        const float* parent = landscape_world;
        if (item.parent != 0xffffffffu) {
            if (item.parent >= i || !placed[item.parent]) continue;
            parent = world[item.parent].data();
        }
        multiply(item.matrix->data(), parent, world[i].data());
        placed[i] = true;
    }
    for (std::size_t i = 0; i != model.hierarchy.size(); ++i) {
        const HierarchyItem& item = model.hierarchy[i];
        for (const std::uint32_t resource : item.resources) {
            if (resource >= model.resources.size() || !model.resources[resource].payload) continue;
            const auto* mesh = std::get_if<MeshResourcePayload>(&*model.resources[resource].payload);
            if (mesh == nullptr) continue;
            for (const MeshSubsetFields& subset : mesh->subsets) {
                if (base_name(subset.effect_name).find("oceanwavedamp") == std::string::npos) continue;
                ++sections;
                char line[160];
                if (!placed[i]) {
                    ++refused;
                    std::snprintf(line, sizeof(line), " [%s: no world frame]", item.name.c_str());
                    notes += line;
                    continue;
                }
                std::uint32_t stream_index = 0;
                const MeshTextureRequest* texture = nullptr;
                for (const MeshSubsetEvent& event : subset.events) {
                    if (const auto* s = std::get_if<MeshVertexStreamReference>(&event)) stream_index = s->index;
                    if (const auto* t = std::get_if<MeshTextureRequest>(&event)) {
                        if (texture == nullptr || (t->slot == 0 && texture->slot != 0)) texture = t;
                    }
                }
                if (stream_index >= mesh->vertex_streams.size()) {
                    ++refused;
                    notes += " [" + item.name + ": no vertex stream]";
                    continue;
                }
                const MeshVertexStreamPayload& vertices = mesh->vertex_streams[stream_index];
                const std::optional<int> type = position_type(vertices.format_name);
                if (!type) {
                    ++refused;
                    notes += " [" + item.name + ": position format " + vertices.format_name + "]";
                    continue;
                }
                // 00BA0EB0..00BA0FB6.
                float x0 = kBoxStart, z0 = kBoxStart, x1 = -kBoxStart, z1 = -kBoxStart;
                bool read_all = true;
                for (std::uint32_t v = 0; v != vertices.count; ++v) {
                    float p[3];
                    if (!read_position(vertices, *type, v, p)) { read_all = false; break; }
                    // FCOMI then JBE: below, equal or unordered takes the vertex.
                    z0 = !(p[2] > z0) ? p[2] : z0;
                    x0 = !(p[0] > x0) ? p[0] : x0;
                    z1 = !(z1 > p[2]) ? p[2] : z1;
                    x1 = !(x1 > p[0]) ? p[0] : x1;
                }
                if (!read_all) {
                    ++refused;
                    notes += " [" + item.name + ": vertex bytes short]";
                    continue;
                }
                // 00BA0FD9..00BA1077: + [node+120h] / [node+128h].
                const float node_x = world[i][12];
                const float node_z = world[i][14];
                x1 = node_x + x1;
                z1 = node_z + z1;
                x0 = node_x + x0;
                z0 = node_z + z0;
                OceanCoverageRegion region;
                region.x0 = x0;
                region.z0 = z0;
                region.x1 = x1;
                region.z1 = z1;
                region.scale_x = 1.0f / (x1 - x0);  // 00BA107B..00BA108B
                region.scale_z = 1.0f / (z1 - z0);  // 00BA108F..00BA1099
                // 00BA109D: no texture leaves 00B9E170 a null and no region.
                if (texture == nullptr) {
                    ++refused;
                    notes += " [" + item.name + ": no texture]";
                    continue;
                }
                std::string texture_bytes;
                std::string texture_name = texture->name;
                if (!read_file(texture_name, texture_bytes)) {
                    texture_name = kErrorTexture;  // 00B31C20: TEXTURE NOT FOUND
                    if (!read_file(texture_name, texture_bytes)) {
                        ++refused;
                        notes += " [" + item.name + ": " + texture->name + " and error.tga unreadable]";
                        continue;
                    }
                }
                auto bitmap = std::make_unique<std::vector<std::uint8_t>>();
                std::string mask_error;
                if (!ocean_coverage_read_mask_00b9e170(texture_bytes, texture_reduction, *bitmap,
                        region.width, region.height, mask_error)) {
                    ++refused;
                    notes += " [" + item.name + ": " + texture_name + ": " + mask_error + "]";
                    continue;
                }
                region.bitmap = bitmap->data();
                out.bitmaps.push_back(std::move(bitmap));
                out.regions.push_back(region);
                ++added;
                std::snprintf(line, sizeof(line), " [%s: %s box=(%.3f,%.3f)-(%.3f,%.3f) mask=%gx%g]",
                    item.name.c_str(), texture_name.c_str(), x0, z0, x1, z1, region.width,
                    region.height);
                notes += line;
            }
        }
    }
    char head[96];
    std::snprintf(head, sizeof(head), "regions=%zu damp_sections=%zu refused=%zu total=%zu",
        added, sections, refused, out.regions.size());
    return head + notes;
}

}  // namespace bsp
