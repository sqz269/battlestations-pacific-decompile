// Installed-file traversal through the actual mounted VFS and recovered reader.
// Resource payloads without a reconstructed parser are explicitly skipped here.
#include "asset_stream_probe.hpp"
#include "bsp/structured_resource.hpp"
#include "bsp/structured_hierarchy.hpp"
#include "bsp/mesh_resource.hpp"
#include "bsp/vertex_format.hpp"
#include "bsp/structured_resource_registry.hpp"
#include "installed_model_probe.hpp"
#include <array>
#include <cstdio>
#include <cstring>

bool probe_model_metadata(AssetStreamProbe& assets, InstalledModelProbe* output) {
    constexpr const char* name = "models/misc/repulogepdarabok_004.mmod";
    std::shared_ptr<bsp::MemoryStream> source;
    std::string error;
    if (!assets.read(name, source, error)) return false;
    bsp::StructuredReader reader(source);
    auto root = reader.read_root_00bea700();
    if (!root || root->tag() != "MMOD" || !root->read_control_00be9a40()
        || reader.control_word() != 14) return false;
    std::string note;
    bsp::HierarchyItem hierarchy;
    bsp::MeshResourcePayload mesh;
    float group_params = 0;
    std::size_t note_count = 0, skipped_items = 0, hierarchy_count = 0;
    std::size_t mesh_count = 0, group_count = 0;
    bsp::MeshStructuredResourceParser mesh_parser(bsp::resolve_mesh_vertex_format_layout_00b2dbd0);
    bsp::NoteStructuredResourceParser note_parser;
    bsp::GroupParamsStructuredResourceParser group_parser;
    bsp::StructuredResourceRegistry registry;
    if (!registry.register_parser(mesh_parser) || !registry.register_parser(note_parser)
        || !registry.register_parser(group_parser) || registry.register_parser(mesh_parser)
        || registry.size() != 3 || registry.find_parser("mEsH") != &mesh_parser) return false;
    while (root->has_remaining_00715bf0()) {
        auto container = root->read_child_00bea680();
        if (!container) return false;
        if (container->tag() == "Resource") {
            std::vector<bsp::DecodedStructuredResource> items;
            if (!registry.dispatch_items_00b7e970(*container, items, error)) {
                std::printf("Resource dispatch: %s\n", error.c_str()); return false;
            }
            for (auto& item : items) {
                if (!item.payload) ++skipped_items;
                else if (auto* value = std::get_if<bsp::NoteResourcePayload>(&*item.payload)) {
                    note = std::move(value->text); ++note_count;
                } else if (auto* mesh_value = std::get_if<bsp::MeshResourcePayload>(&*item.payload)) {
                    mesh = std::move(*mesh_value); ++mesh_count;
                } else if (auto* group_value = std::get_if<bsp::GroupParamsResourcePayload>(&*item.payload)) {
                    group_params = group_value->value; ++group_count;
                }
            }
        } else if (container->tag() == "Hierarchy") {
            while (container->has_remaining_00715bf0()) {
                auto item = container->read_child_00bea680();
                if (!item || item->tag() != "Item"
                    || !bsp::parse_hierarchy_item_00b7eb90(*item, hierarchy, error)) {
                    std::printf("Hierarchy reader: %s\n", error.c_str());
                    return false;
                }
                if (!item->close()) return false;
                ++hierarchy_count;
            }
        } else if (!container->skip_00be9c40()) return false;
        if (!container->close()) return false;
    }
    if (reader.error() != bsp::StructuredReaderError::none || !root->close()
        || !hierarchy.matrix) return false;
    bool matrix_matches = true;
    for (std::size_t index = 0; index != 16; ++index) {
        std::uint32_t bits;
        std::memcpy(&bits, &(*hierarchy.matrix)[index], sizeof(bits));
        const std::uint32_t expected = index % 5 == 0 ? 0x3f800000u
            : index == 9 ? 0x80000000u : 0u;
        matrix_matches = matrix_matches && bits == expected;
    }
    constexpr std::array<std::uint32_t, 6> box_bits{
        0xbea01636u, 0xbf011209u, 0xc01ef14au,
        0x3ea01636u, 0x3f011209u, 0x401ef14au};
    bool box_matches = true;
    for (std::size_t index = 0; index != box_bits.size(); ++index) {
        std::uint32_t bits;
        std::memcpy(&bits, &hierarchy.box[index], sizeof(bits));
        box_matches = box_matches && bits == box_bits[index];
    }
    // Independently inspected installed-file offsets. Compare copied bytes
    // directly with the retained VFS source; this also checks that metadata
    // preceding Subset did not shift the shared reader cursor.
    if (source->size_00bef600() != 1303 || !source->fully_initialized()
        || mesh.vertex_streams.size() != 1 || !mesh.indices
        || mesh.subsets.size() != 1 || mesh.lod_phases.size() != 1) return false;
    const auto* original = source->data_00bef610();
    const auto& stream = mesh.vertex_streams.front();
    const auto& indices = *mesh.indices;
    const auto& subset = mesh.subsets.front();
    const bool vertex_bytes = stream.bytes.size() == 192
        && std::memcmp(stream.bytes.data(), original + 177, 192) == 0;
    const bool compressed_bytes = stream.compressed_format_bytes.size() == 96
        && std::memcmp(stream.compressed_format_bytes.data(), original + 403, 96) == 0;
    const bool index_bytes = indices.bytes.size() == 48
        && std::memcmp(indices.bytes.data(), original + 522, 48) == 0;
    const auto* selected = subset.events.size() == 3
        ? std::get_if<bsp::MeshVertexStreamReference>(&subset.events[0]) : nullptr;
    const auto* texture = subset.events.size() == 3
        ? std::get_if<bsp::MeshTextureRequest>(&subset.events[1]) : nullptr;
    const auto* lighting = subset.events.size() == 3
        ? std::get_if<bsp::MeshLightingRecord>(&subset.events[2]) : nullptr;
    const bool subset_matches = subset.serialized_primitive == 5
        && subset.native_primitive == 4
        && subset.range_words == std::array<std::uint32_t,4>{0,12,0,8}
        && subset.effect_name == "textured.mshd"
        && selected && selected->index == 0 && texture && texture->slot == 0
        && texture->name == "repulodestroyed.tga" && lighting
        && lighting->ignored_slot == 0
        && std::memcmp(lighting->values.data(), original + 805, 68) == 0;
    const auto& phase = mesh.lod_phases.front();
    const bool lod_matches = mesh.lod_value == 1.0f && phase.value0 == 0.0f
        && std::memcmp(&phase.value1, original + 936, 4) == 0
        && phase.word0 == 0 && phase.word1 == 0;
    const bool mesh_checked = mesh_count == 1 && mesh.prefix_word == 0
        && stream.count == 12 && stream.format_name == "pssn4nubn4ussn2.mvfm"
        && stream.layout.stride == 16 && stream.layout.element_count == 3
        && stream.has_compressed_data && vertex_bytes && compressed_bytes
        && indices.count == 24 && indices.format == 0x65 && indices.index_width == 2
        && index_bytes && subset_matches && lod_matches
        && mesh.weight_map_names.empty() && mesh.unknown_tags.empty()
        && mesh.field_order == std::vector<std::string>{"VertexStream",
            "CompressedVertexFormatData", "Indices", "BoundingSphere",
            "BoundingBox", "Subset", "LODPhases"};
    const bool group_checked = group_count == 1
        && std::memcmp(&group_params, original + 998, 4) == 0;
    const bool checked = note_count == 1 && note == "visp100-visp1.5"
        && skipped_items == 0 && hierarchy_count == 1 && mesh_checked && group_checked
        && hierarchy.name == "repulogepdarabok_004"
        && hierarchy.parent == 0xffffffffu && hierarchy.flags == 0
        && hierarchy.resources == std::vector<std::uint32_t>{0, 1, 2}
        && matrix_matches && box_matches
        && hierarchy.sphere[0] == 0 && hierarchy.sphere[1] == 0
        && hierarchy.sphere[2] == 0 && hierarchy.sphere[3] > hierarchy.box[5]
        && reader.active_node_count() == 0
        && source->position_00bef580() == 1303 && source->size_00bef600() == 1303;
    std::printf("Installed model metadata: name=%s control=%u notes=%zu note=%s hierarchy_items=%zu hierarchy_name=%s resources=%zu matrix_bits_match=%d explicit_box_over_sphere=%d cursor=%lld skipped_resource_parsers=%zu checked=%d\n",
        name, reader.control_word(), note_count, note.c_str(), hierarchy_count,
        hierarchy.name.c_str(), hierarchy.resources.size(), matrix_matches,
        box_matches, static_cast<long long>(source->position_00bef580()),
        skipped_items, checked);
    std::printf("Installed mesh payload: vertices=%u stride=%u elements=%u indices=%u index_width=%u vertex_bytes_match=%d compressed_bytes_match=%d index_bytes_match=%d subsets=%zu subset_fields_match=%d lod_match=%d group_params_match=%d field_order_match=%d checked=%d\n",
        stream.count, stream.layout.stride, stream.layout.element_count,
        indices.count, indices.index_width, vertex_bytes, compressed_bytes,
        index_bytes, mesh.subsets.size(), subset_matches, lod_matches,
        group_checked, mesh.field_order.size() == 7, mesh_checked && group_checked);
    registry.clear();
    if (registry.size() != 0 || registry.find_parser("Mesh")) return false;
    std::printf("Installed resource registry: parsers=3 duplicate_rejected=1 case_insensitive_lookup=1 ordered_dispatch=1 clear=1\n");
    if (checked && output) *output = {std::move(mesh), std::move(hierarchy)};
    return checked;
}

bool probe_structured_resource(AssetStreamProbe& assets) {
    constexpr const char* name = "models/clouds/cloud_10.mmod";
    std::shared_ptr<bsp::MemoryStream> source;
    std::string error;
    if (!assets.read(name, source, error)) {
        std::printf("Structured reader: %s\n", error.c_str());
        return false;
    }
    std::weak_ptr<bsp::MemoryStream> lifetime = source;
    bool traversed = false;
    {
        bsp::StructuredReader reader(source);
        auto* observed = source.get();
        source.reset();
        auto root = reader.read_root_00bea700();
        if (!root || root->tag() != "MMOD" || root->declared_payload() != 253
            || !root->read_control_00be9a40() || reader.control_word() != 12)
            return false;
        const std::array<const char*, 4> expected{
            "BoundingSphere", "BoundingBox", "Resource", "Hierarchy"};
        std::array<float, 6> bounds{};
        std::size_t record_count = 0;
        std::size_t resource_records = 0;
        for (const char* tag : expected) {
            auto child = root->read_child_00bea680();
            if (!child || child->tag() != tag) return false;
            ++record_count;
            if (child->tag() == "BoundingBox") {
                if (child->declared_payload() != 24
                    || !bsp::read_bounding_box_00b93310(*child, bounds)
                    || child->remaining() != 0 || !child->close()) return false;
            } else if (child->tag() == "Resource") {
                auto item = child->read_child_00bea680();
                if (!item || item->tag() != "CloudSystem"
                    || item->declared_payload() != 116) return false;
                ++resource_records;
                // This probe checks record boundaries. It does not substitute
                // an empty object for a concrete CloudSystem parser.
                if (!item->skip_00be9c40() || child->remaining() != 0
                    || !child->close()) return false;
            } else if (!child->skip_00be9c40()) return false;
        }
        bool box_matches = true;
        for (std::size_t index = 0; index != bounds.size(); ++index) {
            std::uint32_t bits;
            std::memcpy(&bits, &bounds[index], sizeof(bits));
            // Observed installed-file values, independent of reader arithmetic.
            box_matches = box_matches && bits ==
                (index < 3 ? 0xc2a44306u : 0x42a44306u);
        }
        traversed = reader.error() == bsp::StructuredReaderError::none
            && !root->has_remaining_00715bf0() && root->close()
            && reader.active_node_count() == 0 && box_matches
            && observed->position_00bef580() == 265
            && observed->size_00bef600() == 265 && !lifetime.expired();
        std::printf("Installed structured reader: name=%s control=%u root_records=%zu resource_records=%zu cursor=%lld bytes=%lld box_bits_match=%d retained_source=%d checked=%d\n",
            name, reader.control_word(), record_count, resource_records,
            static_cast<long long>(observed->position_00bef580()),
            static_cast<long long>(observed->size_00bef600()), box_matches,
            !lifetime.expired(), traversed);
    }
    if (!traversed || !lifetime.expired()) return false;
    std::printf("Structured reader source lifetime: released_after_reader=%d\n", lifetime.expired());

    // One focused lifetime case: releasing a partially read child charges its
    // full declared payload to the parent but must not seek its unread bytes.
    if (!assets.read(name, source, error)) return false;
    bsp::StructuredReader reader(source);
    auto root = reader.read_root_00bea700();
    if (!root || !root->read_control_00be9a40()) return false;
    auto child = root->read_child_00bea680();
    float center_x;
    if (!child || child->tag() != "BoundingSphere"
        || !child->read_float(center_x)) return false;
    const auto cursor = source->position_00bef580();
    const auto parent_before = root->remaining();
    const auto declared = child->declared_payload();
    const auto remaining = child->remaining();
    child.reset();
    const bool released_without_seek = source->position_00bef580() == cursor
        && remaining == 12 && root->remaining() == parent_before - declared
        && reader.active_node_count() == 1 && root->close()
        && source->position_00bef580() == cursor
        && reader.error() == bsp::StructuredReaderError::none;
    std::printf("Structured reader release: unread=%u cursor=%lld no_seek=%d parent_charged_declared=%d\n",
        remaining, static_cast<long long>(cursor), released_without_seek,
        root->remaining() == parent_before - declared);
    return released_without_seek && probe_model_metadata(assets);
}
