#pragma once
// The gun class's muzzle list from its device model: the "fire" Points items.
// Packet cc9_gun_barrel_count, docs/GUN_BARREL_COUNT.md.
//
// Chain in the image, per weapon class at load:
//   [class+50h] is the device row's `Mesh` model (classtables/*/deviceclasses.lua).
//   Its `Resource` section holds `Aux` entries; the Aux factory 0071B5C0 builds a
//   54h-byte item (0071ABB0) and 0071B3E0 reads it: `Identifier` = counted
//   string into item+8 and U32 into item+24h, `Category` = string into item+28h,
//   `Points` = float triples pushed onto the Vector12 at item+44h, anything else
//   skipped. 007325A0 then asks 00718870 for ("fire", 0) and, failing that,
//   ("fire", 1), ("fire", 2), ... and fills class+98h; 0072AB80 turns the
//   list size into gun+448h at 0072E71A.
//
// Host representation, not the native layout or ABI. The reader walks the
// model through the recovered StructuredReader (00BEA700/00BEA680 and the
// node reads); it skips every Resource entry other than `Aux` rather than
// running their parsers, which is sufficient because 00718870 only consults
// the classified Aux items.
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

struct GunFirePointItem {
    std::string name;                         // item+8, counted, case-sensitive
    std::uint32_t index{0xffffffffu};         // item+24h, 0071ABB0 seeds FFFFFFFFh
    std::string category;                     // item+28h
    std::vector<std::array<float, 3>> points; // item+44h..4Ch
};

// Parse an .mmod byte image: root `MMOD`, the version control word, then the
// root children; inside `Resource`, each `Aux` entry is read as 0071B3E0 reads
// it and appended in file order. False on a malformed stream (error set);
// items read before the failure are kept.
bool read_mmod_aux_point_items_0071b3e0(const std::vector<std::uint8_t>& bytes,
    std::vector<GunFirePointItem>& items, std::string& error);

// 00718870 (00717F20 contains, 00718000 find): the LAST item whose counted name
// equals `name` byte for byte and whose index equals `index`; null when none.
const GunFirePointItem* find_named_point_group_00718870(
    const std::vector<GunFirePointItem>& items, const std::string& name,
    std::uint32_t index) noexcept;

// 007325A0 on the items: ("fire", 0) contributes its whole point list
// (00732689-007326BF); otherwise each consecutive ("fire", k), k = 1, 2, ...,
// contributes its FIRST point (007326DA-00732788). An item with no points in
// the second walk is the native's invalid-parameter path (the begin<end check
// before 004215D0); the host stops the walk there and reports it.
struct GunFireMuzzleList {
    std::vector<std::array<float, 3>> offsets; // class+98h..A0h, mount-node local
    bool whole_index0{};                       // took the ("fire", 0) branch
    bool empty_item_stopped{};                 // hit the invalid-parameter case
};
GunFireMuzzleList gun_fire_muzzle_offsets_007325a0(
    const std::vector<GunFirePointItem>& items);

// 0095F500's slot pass (0095FA33-0095FEA9), for the platform whose Lua key is
// `key` (the platform vector is indexed by the key, 00961B69). It takes
// ("slot", key) through 00718000, skips a group with fewer than three points,
// and builds the frame it copies to platform+4Ch: translation = p0
// (0095FBF3-0095FC13), row 2 = p2 - p0 (0095FC67-0095FC98), row 1 =
// (p2 - p0) x (p1 - p0) (0095FD4E-0095FDD8), then 0085DC80. Rows are left
// unnormalised here; only the translation is consumed by this host.
// docs/SHIP_PLATFORM_ATTACHMENT.md.
struct GunPlatformSlotFrame {
    std::array<float, 3> origin{};    // row 3, ship-model space
    std::array<float, 3> forward{};   // row 2 before 0085DC80
    std::array<float, 3> up{};        // row 1 before 0085DC80
};
// The model's top-level `BoundingBox` (6 floats, min xyz then max xyz), which
// 00B7F430 stores at model+24h+28h. False when the model has none.
bool read_mmod_bounding_box(const std::vector<std::uint8_t>& bytes,
    std::array<float, 6>& box);

// Every `GeomMesh` entry of the model's Resource section, decoded by the
// recovered parser 00727310 (include/bsp/geom_mesh_resource.hpp), in file
// order. False on a malformed stream; payloads decoded before it are kept.
bool read_mmod_geom_meshes(const std::vector<std::uint8_t>& bytes,
    std::vector<struct GeomMeshResourcePayload>& meshes, std::string& error);

bool gun_platform_slot_frame_0095f500(const std::vector<GunFirePointItem>& items,
    int key, GunPlatformSlotFrame& frame);

} // namespace bsp
