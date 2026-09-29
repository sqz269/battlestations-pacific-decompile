#include "bsp/mmod_hull_convex_box.hpp"

#include <cstring>
#include <map>
#include <memory>

#include "bsp/memory_stream.hpp"
#include "bsp/ship_hull_shapes.hpp"
#include "bsp/structured_reader.hpp"

namespace bsp {
namespace {

// __stricmp on the node tag, as the dispatchers compare it (same rule as gun_fire_points.cpp).
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

struct PointBox {
    float lo[3]{};
    float hi[3]{};
    std::uint32_t points{0};
};

// 006FAD70's front half: the u32 at +8h when at least 0Ch bytes remain (006FAD83), then
// 006FA7F0's record count and records: a float3 (00BE9A60 x3) and three u32 lists, each a u32
// count followed by that many u32 indices. The later arrays (006FA910, 006FACE0, 006FAA20) do
// not feed the box and are skipped with the rest of the payload.
bool read_convex_points(StructuredNode& entry, PointBox& box) {
    std::uint32_t word = 0;
    if (entry.remaining() >= 0x0Cu && !entry.read_u32(word)) return false;
    std::uint32_t count = 0;
    if (!entry.read_u32(count)) return false;
    for (int k = 0; k < 3; ++k) {
        box.lo[k] = 3.402823466e+38f;   // 006F9EE0 seeds at FLT_MAX (00D7A248)
        box.hi[k] = -3.402823466e+38f;  // and -FLT_MAX (00D7A244)
    }
    for (std::uint32_t i = 0; i != count; ++i) {
        float p[3]{};
        for (float& v : p) {
            if (!entry.read_float(v)) return false;
        }
        for (int k = 0; k < 3; ++k) {
            if (p[k] < box.lo[k]) box.lo[k] = p[k];
            if (box.hi[k] < p[k]) box.hi[k] = p[k];
        }
        for (int list = 0; list < 3; ++list) {
            std::uint32_t n = 0;
            if (!entry.read_u32(n)) return false;
            for (std::uint32_t j = 0; j != n; ++j) {
                std::uint32_t index = 0;
                if (!entry.read_u32(index)) return false;
            }
        }
    }
    box.points = count;
    return true;
}

// Every `Item` of `Hierarchy`, in record order; record 0 is the graph root B891A0 publishes at
// instance+0Ch, and B891A0 publishes each record's items with that record's own node. An item's
// `Resource` children are u32 positions in the root's Resource list.
bool read_record_resources(StructuredNode& hierarchy,
                           std::vector<std::vector<std::uint32_t>>& out) {
    while (hierarchy.has_remaining_00715bf0()) {
        auto item = hierarchy.read_child_00bea680();
        if (!item) return false;
        if (tag_is(*item, "Item")) {
            out.emplace_back();
            while (item->has_remaining_00715bf0()) {
                auto field = item->read_child_00bea680();
                if (!field) return false;
                if (tag_is(*field, "Resource")) {
                    std::uint32_t index = 0;
                    if (!field->read_u32(index)) return false;
                    out.back().push_back(index);
                }
                if (!field->skip_00be9c40() || !field->close()) return false;
            }
        }
        if (!item->skip_00be9c40() || !item->close()) return false;
    }
    return true;
}

// The three names 00937C90 looks up with 0071AD50 before its walk: `firstnode` (00D1968C at
// 00938F4E), `front` (00D196A0 at 00938DB9, kept at controller+370h) and `back` (00D19698 at
// 00938DE2, +374h). 0071AD50 is an exact, case-sensitive match on a Note item's name
// (GUNNERY_OPEN_ITEMS 53), and it returns the node the Note was published with.
bool is_owner_note(const std::string& name) noexcept {
    return name == "firstnode" || name == "front" || name == "back";
}

}  // namespace

bool read_mmod_hull_convex_box(const std::vector<std::uint8_t>& mmod_bytes,
                               MmodHullConvexBox& out, std::string& error) {
    out = MmodHullConvexBox{};
    if (mmod_bytes.empty()) { error = "empty model"; return false; }
    auto stream = std::make_shared<MemoryStream>(
        memory_stream_from_complete_bytes(mmod_bytes.data(), mmod_bytes.size()));
    StructuredReader reader(stream);
    auto root = reader.read_root_00bea700();
    if (!root || !tag_is(*root, "MMOD")) { error = "no MMOD root"; return false; }
    // 00B80720 consumes the version word before 00B7F430 dispatches the children.
    if (!root->read_control_00be9a40()) { error = "no version word"; return false; }

    std::map<std::uint32_t, PointBox> convex;  // Resource-list position -> point box
    std::map<std::uint32_t, std::string> notes;  // Resource-list position -> Note name
    std::vector<std::vector<std::uint32_t>> records;
    std::uint32_t resource_position = 0;  // file order across Resource entries
    while (root->has_remaining_00715bf0()) {
        auto section = root->read_child_00bea680();
        if (!section) { error = "bad root child"; return false; }
        if (tag_is(*section, "Resource")) {
            while (section->has_remaining_00715bf0()) {
                auto entry = section->read_child_00bea680();
                if (!entry) { error = "bad resource entry"; return false; }
                const std::uint32_t position = resource_position++;
                if (tag_is(*entry, "ConvexObject")) {
                    PointBox box;
                    if (!read_convex_points(*entry, box)) {
                        error = "bad ConvexObject"; return false;
                    }
                    convex[position] = box;
                } else if (tag_is(*entry, "Note")) {
                    // The Note's payload opens with its name, a u32 length and the bytes.
                    std::string name;
                    if (!entry->read_string(name)) { error = "bad Note"; return false; }
                    notes[position] = name.substr(0, name.find('\0'));
                }
                if (!entry->skip_00be9c40() || !entry->close()) {
                    error = "cannot close resource entry"; return false;
                }
            }
        } else if (tag_is(*section, "Hierarchy")) {
            if (!read_record_resources(*section, records)) {
                error = "bad Hierarchy"; return false;
            }
        }
        if (!section->skip_00be9c40() || !section->close()) {
            error = "cannot close root child"; return false;
        }
    }
    root->close();
    if (reader.error() != StructuredReaderError::none) { error = "reader error"; return false; }
    if (records.empty()) { error = "no Hierarchy item"; return false; }

    // The walk 00938F61..0093918C keeps a {ConvexObject, node} pair when its node is the
    // firstnode, root (instance+0Ch), front or back node, so the owners are record 0 and every
    // record that lists a Note of one of those names. One shape per kept pair, in record order,
    // as 0071B710 appends them; each widened by 0.02 (00C57C40) and merged into the union
    // (00C55FC0). The points are taken in file coordinates: every owner record in this
    // installation's ship models has a composed matrix that leaves the box unchanged
    // (GUNNERY_OPEN_ITEMS 55), and the shape's node transform is not modelled.
    float lo[3] = {3.402823466e+38f, 3.402823466e+38f, 3.402823466e+38f};
    float hi[3] = {-3.402823466e+38f, -3.402823466e+38f, -3.402823466e+38f};
    for (std::size_t record = 0; record != records.size(); ++record) {
        bool owner = record == 0;
        for (const std::uint32_t index : records[record]) {
            const auto note = notes.find(index);
            if (note != notes.end() && is_owner_note(note->second)) owner = true;
        }
        if (!owner) continue;
        if (record != 0) ++out.note_owner_records;
        for (const std::uint32_t index : records[record]) {
            const auto found = convex.find(index);
            if (found == convex.end() || found->second.points == 0) continue;
            const PointBox& box = found->second;
            for (int k = 0; k < 3; ++k) {
                const float shape_lo = box.lo[k] - kDynConvexMeshBoundsEpsilon;
                const float shape_hi = box.hi[k] + kDynConvexMeshBoundsEpsilon;
                if (shape_lo < lo[k]) lo[k] = shape_lo;
                if (hi[k] < shape_hi) hi[k] = shape_hi;
            }
            ++out.shape_count;
            out.point_count += box.points;
        }
    }
    if (out.shape_count != 0) {
        out.min = OceanVec3{lo[0], lo[1], lo[2]};
        out.max = OceanVec3{hi[0], hi[1], hi[2]};
    }
    return true;
}

}  // namespace bsp
