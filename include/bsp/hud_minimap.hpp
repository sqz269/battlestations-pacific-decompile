#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/hud_updates.hpp"

// Packet cc_hud_minimap, docs/HUD_MINIMAP.md.
//
// The part of the slot 35h minimap update (005C0F20) that places unit icons:
// the two global-config radii, the camera-relative rotated transform at
// 005C1A3C..005C1C99, and the widget slots 005BEC50 and 005BE240 bind. The
// animation block and the pulse already live in bsp/hud_updates.hpp; nothing
// here redeclares them.
//
// The transform is x87 throughout with values carried across basic blocks on
// the x87 stack, so it is transcribed from the disassembly. Descriptive names
// are hypotheses, not recovered symbols.
namespace bsp {

// ---------------------------------------------------------------------------
// Range, from the global config
// ---------------------------------------------------------------------------

// Offsets on the object 00432650 returns. 0087D7B0 fills them from
// Globals["Minimap"] in scripts/datatables/globals.lua: +6Ch from MinimapRange
// and +70h from VisibilityRange. Both are 4000 in the shipped data.
inline constexpr std::size_t kHudMinimapRangeOffset = 0x6C;
inline constexpr std::size_t kHudMinimapVisibilityRangeOffset = 0x70;
inline constexpr float kHudMinimapInstalledRange = 4000.0f;
inline constexpr float kHudMinimapInstalledVisibilityRange = 4000.0f;

// 00CF1440, a double, divided into the range at 005C1799 to give the scale.
// kHudMinimapCompassDivisor in bsp/hud_updates.hpp is the same literal, named
// there from the reading this packet corrects; this alias states its role.
inline constexpr float kHudMinimapRadiusUnits = 80.0f;

// ---------------------------------------------------------------------------
// The terrain layer
// ---------------------------------------------------------------------------

// Constants of shaderfx/gui/minimap_terrain.shfx, the shader
// minimap_islandmap_Icon binds through ShaderName. They are asset values, not
// code, and are recorded so a host can reproduce the same mapping.
inline constexpr float kHudMinimapTerrainWorldSize = 30000.0f;
inline constexpr float kHudMinimapTerrainWorldHalf = 15000.0f;
inline constexpr float kHudMinimapTerrainMaskRadiusSq = 0.018f;
inline constexpr float kHudMinimapTerrainBorderTile = 2.0f;

// uv = (world + 15000) / 30000 with z inverted, as the vertex shader's
// eyeOffs and limits both assume.
struct HudMinimapTerrainUv {
    float u{0.0f};
    float v{0.0f};
};
HudMinimapTerrainUv hud_minimap_terrain_uv(float world_x, float world_z) noexcept;

// The circular mask: alpha is zeroed outside sqrt(0.018) of the quad centre,
// which is 4025 world units and so agrees with MinimapRange.
bool hud_minimap_terrain_inside_mask(float quad_u, float quad_v) noexcept;

// ---------------------------------------------------------------------------
// The icon transform
// ---------------------------------------------------------------------------

// World position of an icon and of the camera unit, as the pass reads them from
// the entity world matrix translation at +FCh.
struct HudMinimapWorldPoint {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// 005C1A3C..005C1B5A. A point farther than `range` from the camera is pulled
// onto the circle of that radius; nearer points are returned unchanged. The
// distance test is on the squared length and the clamp then takes one sqrt.
HudMinimapWorldPoint hud_minimap_clamp_to_rim_005c1a3c(const HudMinimapWorldPoint& point,
                                                       const HudMinimapWorldPoint& camera,
                                                       float range) noexcept;

// 005C1B62..005C1BFF then 005C1C61..005C1C99. `icon_heading` is the negated
// atan2 of the renderer basis, `range` is MinimapRange and `depth` one of the
// five literals. The result is the widget's local position; its x divides by
// 1024 and its y by 768, so the space is anisotropic 4:3.
HudGuiPoint hud_minimap_icon_position_005c1b62(const HudMinimapWorldPoint& point,
                                               const HudMinimapWorldPoint& camera, float range,
                                               float icon_heading, float depth) noexcept;

// 005C1CA1..005C1CD0: pi/2 minus the sum of the unit's own heading and the
// icon heading. kHudMinimapHeadingBias in bsp/hud_updates.hpp is the pi/2.
float hud_minimap_icon_rotation_005c1ca1(float unit_heading, float icon_heading) noexcept;

// 005C1788..005C1812. `heading_raw` is atan2([renderer+110h], [renderer+118h]);
// the icons use its negation and the map layers its positive value.
struct HudMinimapHeadings {
    float icon{0.0f}; // -heading_raw, 005C17F2
    float map{0.0f};  // +heading_raw, 005C1812
    float scale{0.0f}; // 80 / MinimapRange, 005C1799
};
HudMinimapHeadings hud_minimap_headings_005c1788(float heading_raw, float range) noexcept;

// 005C16D3 and 005C19B1: an icon is dropped when its squared distance from the
// camera exceeds VisibilityRange squared. The self icon skips the first test.
bool hud_minimap_within_visibility(const HudMinimapWorldPoint& point,
                                   const HudMinimapWorldPoint& camera,
                                   float visibility_range) noexcept;

// ---------------------------------------------------------------------------
// Widget slots and page nodes
// ---------------------------------------------------------------------------

inline constexpr std::size_t kHudMinimapPageRootSlot = 0x1C;   // 005BECCC
inline constexpr std::size_t kHudMinimapDirIconSlot = 0x48;    // 005BEB6D
inline constexpr std::size_t kHudMinimapCompassSlot = 0x4C;    // 005BE2AC
inline constexpr std::size_t kHudMinimapIslandMapSlot = 0x50;  // 005BED46
inline constexpr std::size_t kHudMinimapUnitGroupSlot = 0x6C;  // 005BE397, first of six
inline constexpr std::size_t kHudMinimapMarkerGroupSlot = 0xF0; // 005BEBDD
inline constexpr std::size_t kHudMinimapIconMapSlot = 0xF8;    // 005BE110's container
inline constexpr std::size_t kHudMinimapCapturePointSlot = 0xF4; // 005BEC76

inline constexpr const char* kHudMinimapPageName = "GUI_minimap";
inline constexpr const char* kHudMinimapDirIcon = "minimap_dir_Icon";           // 00CF1170
inline constexpr const char* kHudMinimapCompassIcon = "minimap_compass_Icon";   // 00CF1280
inline constexpr const char* kHudMinimapIslandMapIcon = "minimap_islandmap_Icon"; // 00CF13C8
inline constexpr const char* kHudMinimapUnitItemIcon = "item_ship_Icon";        // 00CF1254
inline constexpr const char* kHudMinimapMarkerGroup = "unit_marker_Group";      // 00CF115C
inline constexpr const char* kHudMinimapTerrainShader = "minimap_terrain.mshd";

// The six colour groups, in the order 005BE240 binds their item icons. The same
// six values 00639990 returns index them.
inline constexpr const char* kHudMinimapUnitGroups[6] = {
    "minimap_units_white_Group",  "minimap_units_red_Group",
    "minimap_units_blue_Group",   "minimap_units_grey_Group",
    "minimap_units_yellow_Group", "minimap_units_silver_Group",
};

// Byte gates of the unit walk at 005C1628..005C164A, the same four bytes
// 0043F080 tests.
inline constexpr std::size_t kHudMinimapUnitAliveByte = 0x5C;
inline constexpr std::size_t kHudMinimapUnitDeadByte = 0x5D;
inline constexpr std::size_t kHudMinimapUnitHiddenByteA = 0x60;
inline constexpr std::size_t kHudMinimapUnitHiddenByteB = 0x5E;

// The linked-list shape of the team unit list at
// [[[00E188A8 + 18CCh + team*4] + 30h] + E0Ch]: next at +4h, the unit at
// [node+8h]+4h. 005C1610..005C1625 and 005C1774.
inline constexpr std::size_t kHudMinimapUnitListOffset = 0xE0C;
inline constexpr std::size_t kHudMinimapUnitListNext = 0x04;
inline constexpr std::size_t kHudMinimapUnitListPayload = 0x08;

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------

// One native call site per method, in the order 005C0F20 reaches them.
struct HudMinimapHost {
    virtual ~HudMinimapHost() = default;

    // 005C0F3F -> 005BD420, the screen's own per-frame preparation.
    virtual void prepare_frame() = 0;
    // 005C154E -> 004B4B00.
    virtual void* camera_unit() = 0;
    // 005C157E and 005C158A -> 00432650, then +6Ch and +70h.
    virtual float minimap_range() = 0;
    virtual float visibility_range() = 0;
    // 005C15BB -> 00414DB0 when [unit+C8h] is clear.
    virtual void refresh_pose(void* unit) = 0;
    // 005C15F2 -> 00927880.
    virtual void* displayed_self_unit(void* controlled_unit) = 0;
    // 005C1687 -> 00427EB0, the world-matrix translation at +FCh.
    virtual HudMinimapWorldPoint world_position(void* unit) = 0;
    // The team list head and its walk.
    virtual void* team_unit_list_head(int team_index) = 0;
    virtual void* team_unit_list_next(void* node) = 0;
    virtual void* team_unit_list_unit(void* node) = 0;
    // 005C1628..005C164A, the four bytes.
    virtual bool unit_is_alive_and_visible(void* unit) = 0;
    // 005C165D, vtable +5Ch with class id 5, and 005C1675, vtable +B8h.
    virtual bool is_kind_of(void* unit, int class_id) = 0;
    virtual bool unit_shows_on_minimap(void* unit) = 0;
    // 005C170A -> 005BE110 and the three entry calls that follow a miss.
    virtual void* find_icon_entry(void* unit) = 0;
    virtual void* create_icon_entry(void* unit) = 0;
    // 005C17A7 -> 00B6DB70, then the two renderer basis floats at +110h, +118h.
    virtual void renderer_basis(float& y_component, float& x_component) = 0;
    // 005C1C99 -> 00AA7DC0 and the widget rotation virtual +44h.
    virtual void set_icon_position(void* icon, const HudGuiPoint& position) = 0;
    virtual void set_icon_rotation(void* icon, float radians) = 0;
    // 005C1825, 005C184F and 005C1872: compass, island map and direction wedge.
    virtual void set_map_layer_rotation(void* widget, float radians) = 0;
    virtual void* map_layer(std::size_t slot) = 0;
};

// The per-frame icon pass over one team list, 005C154E..005C1C99. Returns how
// many icons were placed.
//
// Coverage: partial. It covers the gate at 005C1561..005C1578, the two
// distance tests, the icon-entry lookup, the rim clamp, the rotation and the
// placement. It does not cover the capture-point passes at 005C1F90, 005C239E
// and 005C2523, nor the depth selection at 005C1C3E, which needs the target
// group test at 005C1C0B; every icon this routine places uses the default
// depth. `team_index` is [00E188A8+18ECh] and must be 0..7.
int hud_minimap_place_unit_icons_005c154e(int team_index, HudMinimapHost& host,
                                          float depth) noexcept;

} // namespace bsp
