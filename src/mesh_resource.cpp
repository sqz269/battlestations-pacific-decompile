#include "bsp/mesh_resource.hpp"
#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp {
bool parse_mesh_resource_00b944e0(StructuredNode& node,
    const MeshVertexFormatResolver& resolve_format, MeshResourcePayload& output,
    std::string& error) {
    error.clear();
    if (!node.ready()) {
        error = "Mesh reader is not ready.";
        return false;
    }
    try {
        MeshResourcePayload parsed;
        if (!node.read_u32(parsed.prefix_word)) {
            error = "Could not read Mesh prefix DWORD.";
            return false;
        }
        while (node.has_remaining_00715bf0()) {
            auto child = node.read_child_00bea680();
            if (!child) {
                error = "Could not read Mesh child header.";
                return false;
            }
            parsed.field_order.push_back(child->tag());
            const char* tag = child->tag().c_str();
            bool success;
            if (_stricmp(tag, "LODValue") == 0) {
                success = read_mesh_lod_value_00b944e0_fragment(*child,
                    parsed.lod_value);
            } else if (_stricmp(tag, "LODPhases") == 0) {
                success = append_mesh_lod_phases_00b93710(*child,
                    parsed.lod_phases, error);
            } else if (_stricmp(tag, "Subset") == 0) {
                MeshSubsetFields subset;
                success = parse_mesh_subset_00b941d0(*child, subset, error);
                if (success) parsed.subsets.push_back(std::move(subset));
            } else if (_stricmp(tag, "Indices") == 0) {
                MeshIndexPayload indices;
                success = read_mesh_indices_00b93aa0(*child, indices, error);
                if (success) parsed.indices = std::move(indices);
            } else if (_stricmp(tag, "VertexStream") == 0) {
                MeshVertexStreamPayload stream;
                success = read_mesh_vertex_stream_00b93e60(*child,
                    resolve_format, stream, error);
                if (success) parsed.vertex_streams.push_back(std::move(stream));
            } else if (_stricmp(tag, "CompressedVertexFormatData") == 0) {
                if (parsed.vertex_streams.empty()) {
                    error = "Mesh compressed format data has no preceding VertexStream.";
                    return false;
                }
                success = read_mesh_compressed_vertex_format_data_00b93800(
                    *child, parsed.vertex_streams.back(), error);
            } else if (_stricmp(tag, "BoundingSphere") == 0) {
                success = consume_mesh_sphere_00b93590(*child);
            } else if (_stricmp(tag, "BoundingBox") == 0) {
                success = consume_mesh_box_00b935c0(*child);
            } else if (_stricmp(tag, "WeightMapNames") == 0) {
                success = append_mesh_weight_map_names_00b93f90(*child,
                    parsed.weight_map_names, error);
            } else {
                parsed.unknown_tags.push_back(child->tag());
                success = child->skip_00be9c40();
            }
            if (!success) {
                if (error.empty()) {
                    error = "Could not read Mesh field: ";
                    error += tag;
                }
                return false;
            }
            // rope.mvfm and unknown fields may already be detached by their
            // explicit skip. close() is harmless for those; other recognized
            // children do not acquire an implicit trailing-payload seek.
            if (!child->close()) {
                error = "Could not close Mesh field.";
                return false;
            }
        }
        if (!node.ready()) {
            error = "Mesh reader failed before completion.";
            return false;
        }
        output = std::move(parsed);
        return true;
    } catch (const std::bad_alloc&) {
        error = "Could not allocate Mesh wire values.";
        return false;
    } catch (const std::length_error&) {
        error = "Mesh wire values exceed host container capacity.";
        return false;
    }
}
}
