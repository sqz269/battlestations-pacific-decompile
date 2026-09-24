// The gun class's muzzle list from its device model. docs/GUN_BARREL_COUNT.md.
#include "bsp/gun_fire_points.hpp"
#include "bsp/geom_mesh_resource.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/structured_reader.hpp"
#include <cstring>
#include <memory>

namespace bsp {
namespace {

// __stricmp on the node tag, as the dispatchers compare it.
bool tag_is(const StructuredNode& node, const char* name) noexcept {
    const std::string& tag = node.tag();
    const std::size_t length = std::strlen(name);
    if (tag.size() != length) return false;
    for (std::size_t i = 0; i != length; ++i) {
        char a = tag[i];
        char b = name[i];
        if (a >= 'A' && a <= 'Z') a = static_cast<char>(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = static_cast<char>(b - 'A' + 'a');
        if (a != b) return false;
    }
    return true;
}

// 0071B3E0 on one `Aux` entry node.
bool read_aux_item(StructuredNode& entry, GunFirePointItem& item) {
    while (entry.has_remaining_00715bf0()) {
        auto child = entry.read_child_00bea680();
        if (!child) return false;
        if (tag_is(*child, "Identifier")) {
            // 71B451 character string into item+8, 71B458 U32 into item+24h.
            if (!child->read_string(item.name) || !child->read_u32(item.index)) return false;
        } else if (tag_is(*child, "Category")) {
            if (!child->read_string(item.category)) return false;
        } else if (tag_is(*child, "Points")) {
            // Three 00BE9A60 float reads per point, pushed by 004215D0 at 71B4F0,
            // while the Points node has payload left.
            while (child->has_remaining_00715bf0()) {
                std::array<float, 3> point{};
                if (!child->read_float(point[0]) || !child->read_float(point[1])
                    || !child->read_float(point[2])) return false;
                item.points.push_back(point);
            }
        } else if (!child->skip_00be9c40()) {
            return false;
        }
        if (!child->close()) return false;
    }
    return true;
}

} // namespace

bool read_mmod_aux_point_items_0071b3e0(const std::vector<std::uint8_t>& bytes,
    std::vector<GunFirePointItem>& items, std::string& error) {
    if (bytes.empty()) { error = "empty model"; return false; }
    auto stream = std::make_shared<MemoryStream>(
        memory_stream_from_complete_bytes(bytes.data(), bytes.size()));
    StructuredReader reader(stream);
    auto root = reader.read_root_00bea700();
    if (!root || !tag_is(*root, "MMOD")) { error = "no MMOD root"; return false; }
    // 00B80720 consumes the version word before 00B7F430 dispatches the children.
    if (!root->read_control_00be9a40()) { error = "no version word"; return false; }
    while (root->has_remaining_00715bf0()) {
        auto section = root->read_child_00bea680();
        if (!section) { error = "bad root child"; return false; }
        if (tag_is(*section, "Resource")) {
            // 00B7E970: one typed entry per child; only Aux is read here.
            while (section->has_remaining_00715bf0()) {
                auto entry = section->read_child_00bea680();
                if (!entry) { error = "bad resource entry"; return false; }
                if (tag_is(*entry, "Aux")) {
                    GunFirePointItem item;
                    if (!read_aux_item(*entry, item)) { error = "bad Aux item"; return false; }
                    items.push_back(std::move(item));
                } else if (!entry->skip_00be9c40()) {
                    error = "cannot skip resource entry"; return false;
                }
                if (!entry->close()) { error = "cannot close resource entry"; return false; }
            }
        } else if (!section->skip_00be9c40()) {
            error = "cannot skip root child"; return false;
        }
        if (!section->close()) { error = "cannot close root child"; return false; }
    }
    root->close();
    if (reader.error() != StructuredReaderError::none) { error = "reader error"; return false; }
    return true;
}

const GunFirePointItem* find_named_point_group_00718870(
    const std::vector<GunFirePointItem>& items, const std::string& name,
    std::uint32_t index) noexcept {
    const GunFirePointItem* found = nullptr;
    for (const GunFirePointItem& item : items) {
        if (item.index == index && item.name.size() == name.size()
            && std::memcmp(item.name.data(), name.data(), name.size()) == 0) {
            found = &item;  // 00718000 keeps the last match
        }
    }
    return found;
}

bool read_mmod_bounding_box(const std::vector<std::uint8_t>& bytes,
    std::array<float, 6>& box) {
    if (bytes.empty()) return false;
    auto stream = std::make_shared<MemoryStream>(
        memory_stream_from_complete_bytes(bytes.data(), bytes.size()));
    StructuredReader reader(stream);
    auto root = reader.read_root_00bea700();
    if (!root || !tag_is(*root, "MMOD") || !root->read_control_00be9a40()) return false;
    bool found = false;
    while (!found && root->has_remaining_00715bf0()) {
        auto section = root->read_child_00bea680();
        if (!section) return false;
        if (tag_is(*section, "BoundingBox")) {
            found = true;
            for (float& v : box) {
                if (!section->read_float(v)) return false;
            }
        } else if (!section->skip_00be9c40()) {
            return false;
        }
        if (!section->close()) return false;
    }
    return found;
}

bool read_mmod_geom_meshes(const std::vector<std::uint8_t>& bytes,
    std::vector<GeomMeshResourcePayload>& meshes, std::string& error) {
    if (bytes.empty()) { error = "empty model"; return false; }
    auto stream = std::make_shared<MemoryStream>(
        memory_stream_from_complete_bytes(bytes.data(), bytes.size()));
    StructuredReader reader(stream);
    auto root = reader.read_root_00bea700();
    if (!root || !tag_is(*root, "MMOD") || !root->read_control_00be9a40()) {
        error = "no MMOD root"; return false;
    }
    while (root->has_remaining_00715bf0()) {
        auto section = root->read_child_00bea680();
        if (!section) { error = "bad root child"; return false; }
        if (tag_is(*section, "Resource")) {
            while (section->has_remaining_00715bf0()) {
                auto entry = section->read_child_00bea680();
                if (!entry) { error = "bad resource entry"; return false; }
                if (tag_is(*entry, "GeomMesh")) {
                    GeomMeshResourcePayload payload;
                    if (!parse_geom_mesh_resource_00727310(*entry, payload, error)) return false;
                    meshes.push_back(std::move(payload));
                } else if (!entry->skip_00be9c40()) {
                    error = "cannot skip resource entry"; return false;
                }
                if (entry->attached() && !entry->close()) {
                    error = "cannot close resource entry"; return false;
                }
            }
        } else if (!section->skip_00be9c40()) {
            error = "cannot skip root child"; return false;
        }
        if (!section->close()) { error = "cannot close root child"; return false; }
    }
    return reader.error() == StructuredReaderError::none;
}

bool gun_platform_slot_frame_0095f500(const std::vector<GunFirePointItem>& items,
    int key, GunPlatformSlotFrame& frame) {
    static const std::string kSlot("slot");  // 00CEB728, length 4
    if (key < 0) return false;
    const GunFirePointItem* item = find_named_point_group_00718870(items, kSlot,
        static_cast<std::uint32_t>(key));
    if (item == nullptr || item->points.size() < 3) return false;
    const auto& p0 = item->points[0];
    const auto& p1 = item->points[1];
    const auto& p2 = item->points[2];
    const float a[3] = {p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};
    const float b[3] = {p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]};
    frame.origin = p0;
    frame.forward = {b[0], b[1], b[2]};
    frame.up = {b[1] * a[2] - b[2] * a[1], b[2] * a[0] - b[0] * a[2], b[0] * a[1] - b[1] * a[0]};
    return true;
}

GunFireMuzzleList gun_fire_muzzle_offsets_007325a0(
    const std::vector<GunFirePointItem>& items) {
    GunFireMuzzleList out;
    static const std::string kFire("fire");  // 00CE6798, length 4
    if (const GunFirePointItem* whole = find_named_point_group_00718870(items, kFire, 0)) {
        out.whole_index0 = true;
        out.offsets = whole->points;
        return out;
    }
    for (std::uint32_t k = 1;; ++k) {
        const GunFirePointItem* item = find_named_point_group_00718870(items, kFire, k);
        if (item == nullptr) break;
        if (item->points.empty()) { out.empty_item_stopped = true; break; }
        out.offsets.push_back(item->points.front());
    }
    return out;
}

} // namespace bsp
