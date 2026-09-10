#include "bsp/mesh_fields.hpp"
#include <cstring>
#include <new>
#include <utility>

namespace bsp {
namespace {
bool discard_floats(StructuredNode& node, unsigned count) noexcept {
    float discarded;
    for (unsigned i = 0; i < count; ++i) {
        if (!node.read_float(discarded)) return false;
    }
    return true;
}

std::uint32_t native_primitive(std::uint32_t serialized) noexcept {
    switch (serialized) {
    case 1: return 3;
    case 2: return 2;
    case 3: return 5;
    case 4: return 6;
    case 5: return 4;
    default: return 1;
    }
}

bool parse_texture(StructuredNode& node, MeshTextureRequest& output) {
    if (!node.read_string(output.name) || !node.read_u32(output.slot)) return false;
    while (node.has_remaining_00715bf0()) {
        auto child = node.read_child_00bea680();
        if (!child) return false;
        if (_stricmp(child->tag().c_str(), "TextureAddress") == 0) {
            std::uint32_t discarded;
            for (unsigned i = 0; i < 3; ++i) {
                if (!child->read_u32(discarded)) return false;
            }
        } else if (!child->skip_00be9c40()) {
            return false;
        }
        if (!child->close()) return false;
    }
    return node.ready();
}

bool parse_lighting(StructuredNode& node, std::vector<MeshSubsetEvent>& events) {
    std::uint32_t count;
    if (!node.read_u32(count)) return false;
    for (std::uint32_t i = 0; i < count; ++i) {
        MeshLightingRecord record;
        if (!node.read_u32(record.ignored_slot)) return false;
        for (float& value : record.values) {
            if (!node.read_float(value)) return false;
        }
        events.emplace_back(std::move(record));
    }
    return true;
}
}

bool parse_mesh_subset_00b941d0(StructuredNode& node,
    MeshSubsetFields& output, std::string& error) {
    error.clear();
    if (!node.ready()) {
        error = "Subset reader is not ready.";
        return false;
    }
    try {
        MeshSubsetFields parsed;
        if (!node.read_u32(parsed.serialized_primitive)) {
            error = "Could not read Subset primitive.";
            return false;
        }
        parsed.native_primitive = native_primitive(parsed.serialized_primitive);
        for (std::uint32_t& word : parsed.range_words) {
            if (!node.read_u32(word)) {
                error = "Could not read Subset range words.";
                return false;
            }
        }
        if (!node.read_string(parsed.effect_name)) {
            error = "Could not read Subset effect name.";
            return false;
        }
        // Native CRT stricmp sees only the C-string prefix. Resize to12 bytes
        // and copy the12-byte literal, even if the original had bytes after NUL.
        if (_stricmp(parsed.effect_name.c_str(), "soldiers.mshd") == 0)
            parsed.effect_name = "soldier.mshd";

        while (node.has_remaining_00715bf0()) {
            auto child = node.read_child_00bea680();
            if (!child) {
                error = "Could not read Subset child header.";
                return false;
            }
            const char* tag = child->tag().c_str();
            bool success;
            if (_stricmp(tag, "Texture") == 0) {
                MeshTextureRequest request;
                success = parse_texture(*child, request);
                if (success) parsed.events.emplace_back(std::move(request));
            } else if (_stricmp(tag, "LightingSettings") == 0) {
                success = parse_lighting(*child, parsed.events);
            } else if (_stricmp(tag, "BoundingSphere") == 0) {
                success = consume_mesh_sphere_00b93590(*child);
            } else if (_stricmp(tag, "VertexStreamIndex") == 0) {
                MeshVertexStreamReference reference;
                success = child->read_u32(reference.index);
                if (success) parsed.events.emplace_back(reference);
            } else {
                success = child->skip_00be9c40();
            }
            if (!success) {
                error = "Could not read Subset field: ";
                error += tag;
                return false;
            }
            if (!child->close()) {
                error = "Could not close Subset field.";
                return false;
            }
        }
        if (!node.ready()) {
            error = "Subset reader failed before completion.";
            return false;
        }
        output = std::move(parsed);
        return true;
    } catch (const std::bad_alloc&) {
        error = "Could not allocate Subset fields.";
        return false;
    }
}

bool append_mesh_lod_phases_00b93710(StructuredNode& node,
    std::vector<MeshLodPhase>& output, std::string& error) {
    error.clear();
    std::uint32_t count;
    if (!node.read_u32(count)) {
        error = "Could not read LODPhases count.";
        return false;
    }
    if (output.size() > 4 || count > 4 - output.size()) {
        error = "LODPhases exceeds the native four-record storage domain.";
        return false;
    }
    try {
        std::vector<MeshLodPhase> parsed(output);
        for (std::uint32_t i = 0; i < count; ++i) {
            MeshLodPhase phase;
            if (!node.read_float(phase.value0) || !node.read_float(phase.value1) ||
                !node.read_u32(phase.word0) || !node.read_u32(phase.word1)) {
                error = "Could not read LODPhases record.";
                return false;
            }
            parsed.push_back(phase);
        }
        output = std::move(parsed);
        return true;
    } catch (const std::bad_alloc&) {
        error = "Could not allocate LODPhases records.";
        return false;
    }
}

bool read_mesh_lod_value_00b944e0_fragment(StructuredNode& node,
    float& output) noexcept {
    return node.read_float(output);
}

bool consume_mesh_sphere_00b93590(StructuredNode& node) noexcept {
    return discard_floats(node, 4);
}

bool consume_mesh_box_00b935c0(StructuredNode& node) noexcept {
    return discard_floats(node, 6);
}

bool append_mesh_weight_map_names_00b93f90(StructuredNode& node,
    std::vector<std::string>& output, std::string& error) {
    error.clear();
    if (!node.ready()) {
        error = "WeightMapNames reader is not ready.";
        return false;
    }
    try {
        std::vector<std::string> parsed(output);
        while (node.has_remaining_00715bf0()) {
            std::string name;
            if (!node.read_string(name)) {
                error = "Could not read WeightMapNames string.";
                return false;
            }
            parsed.push_back(std::move(name));
        }
        if (!node.ready()) {
            error = "WeightMapNames reader failed before completion.";
            return false;
        }
        output = std::move(parsed);
        return true;
    } catch (const std::bad_alloc&) {
        error = "Could not allocate WeightMapNames strings.";
        return false;
    }
}
}
