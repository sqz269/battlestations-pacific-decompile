#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/hud_updates.hpp"

// Packet cc_hud_minimap, docs/HUD_MARKERS_RUNTIME.md.
//
// The marker pool under the slot 4Dh markers screen update (006435D0, covered by
// bsp/hud_updates.hpp): the projection chain 0043A660 / 00638E50, the four pools
// 00640620 releases, the classification and dispatch in 006430C0
// (BSP_InGameHudMarkers_AddUnitMarker) and the screen-bounds rule 0063A6C0
// (BSP_HudMarker_ProjectUnitScreenBounds).
//
// The float work in 0043A660, 00638E50 and 0063A6C0 is x87 or SSE scalar with
// register inputs the decompiler loses, so every rule below is transcribed from
// the disassembly. Descriptive names are hypotheses, not recovered symbols.
namespace bsp {

// ---------------------------------------------------------------------------
// 0043A660 BSP_Camera_ProjectWorldToScreen
// ---------------------------------------------------------------------------

// Clip mask bits the projection returns. All three are cleared when the
// reciprocal of clip.w is not above zero (00D7A218), which is the behind-camera
// case. 0043A6F5, 0043A742, 0043A756, 0043A76A.
inline constexpr unsigned kHudMarkerProjectClipX = 1u;
inline constexpr unsigned kHudMarkerProjectClipY = 2u;
inline constexpr unsigned kHudMarkerProjectClipZ = 4u;
inline constexpr unsigned kHudMarkerProjectClipAll = 7u;

// The four output conventions selected by the third argument. Slot 4Dh only
// ever uses kHudMarkerProjectViewportAspect through 00638E50 at 00638E52.
enum class HudMarkerProjectMode : int {
    Viewport = 0,       // (ndc + 1) * 0.5, y flipped
    ViewportAspect = 1, // y first multiplied by the GUI extent height
    Pixels = 2,         // Viewport times 1024 by 768
    ViewportWide = 3,   // ViewportAspect with x scaled by [renderer+1C8h]
};

// The two pixel divisors of mode 2, doubles at 00CE42B8 and 00CE42B0.
inline constexpr float kHudMarkerProjectPixelWidth = 1024.0f;
inline constexpr float kHudMarkerProjectPixelHeight = 768.0f;

// Result of the projection: NDC in `ndc`, the mode-mapped point in `screen`
// (whose z stays the NDC z), and the clip mask.
struct HudMarkerScreenProjection {
    float ndc_x{0.0f};
    float ndc_y{0.0f};
    float ndc_z{0.0f};
    float screen_x{0.0f};
    float screen_y{0.0f};
    unsigned clip_mask{0};
};

// 0043A660, __fastcall(ECX=const float3* world, EDX=float3* out, int mode,
// char clampDepth), RET 8, body 0043A660..0043A869. Pure given the clip-space
// point the caller's camera produced, so the view-projection multiply itself
// (00B70490 then 00B62D10 at 0043A6A0 and 0043A6AF) stays with the host.
// `gui_extent_height` is [00AA1FE0()+4h] and is read only for modes 1 and 3;
// `renderer_aspect` is [renderer+1C8h] and only for mode 3.
// Uncertainty: the clip-mask z rule uses `clamp_depth` exactly as the listing
// does, but no caller in this packet passes a non-zero value, so that branch is
// transcribed and not exercised.
HudMarkerScreenProjection camera_project_world_to_screen_0043a660(
    float clip_x, float clip_y, float clip_z, float clip_w, HudMarkerProjectMode mode,
    bool clamp_depth, float gui_extent_height, float renderer_aspect) noexcept;

// ---------------------------------------------------------------------------
// 00638E50 BSP_HudMarker_ProjectAndClip
// ---------------------------------------------------------------------------

// The horizontal rescale about 0.5 applied when [0109CF04+0Dh] is set.
// 00CF5750, a double, 1.3333334f as a float. 00638E80.
inline constexpr float kHudMarkerWideAspect = 1.3333334f;

// 00638E50 after the projection: the widescreen rescale of x, then the clip
// tests against the rectangle 006435D0 publishes. `wide_aspect_active` is the
// byte [0109CF04+0Dh] read at 00638E6C.
//
// The rectangle's two x pairs hold the same values (006435D0 writes both), so
// HudMarkerClipRect from bsp/hud_updates.hpp carries one left and one right and
// this rule is the reader that settles docs/HUD_CENTRAL_UPDATES.md's open
// question. 00638E9F, 00638EAD, 00638ED0, 00638EE0, 00638F08, 00638F18.
struct HudMarkerClipResult {
    float screen_x{0.0f}; // after the widescreen rescale
    float screen_y{0.0f};
    bool visible{false};
};
HudMarkerClipResult hud_marker_project_and_clip_00638e50(
    float screen_x, float screen_y, unsigned clip_mask, bool wide_aspect_active,
    const HudMarkerClipRect& clip) noexcept;

// ---------------------------------------------------------------------------
// 006430C0 BSP_InGameHudMarkers_AddUnitMarker
// ---------------------------------------------------------------------------

// IsKindOf class ids (vtable slot +5Ch, 006FE530) the dispatch probes, in the
// order 006430C0 tests them. These are raw ids, not named unit categories.
inline constexpr int kHudMarkerKindShipLeaf = 0x06;   // 006430D8, with the parts-object gate
inline constexpr int kHudMarkerKindExcludedA = 0x46;  // 006430F8
inline constexpr int kHudMarkerKindExcludedB = 0x45;  // 0064310B
inline constexpr int kHudMarkerKindGroupOwner = 0x18; // 006431BB
inline constexpr int kHudMarkerKindSubUnits = 0x1A;   // 006431E8
inline constexpr int kHudMarkerKindDelegating = 0x1B; // 00643215
inline constexpr int kHudMarkerKindClose = 0x1C;      // 0064327A
inline constexpr int kHudMarkerKindShip = 0x05;       // 006432B5
inline constexpr int kHudMarkerKindTracked = 0x01;    // 006432E7
inline constexpr int kHudMarkerKindLockA = 0x1E;      // 0064331B
inline constexpr int kHudMarkerKindLockB = 0x1F;      // 0064332A

// Which builder 006430C0 hands the unit to. `None` is the fall-through when no
// probe answers, in which case only the 1Eh/1Fh tail can still run.
enum class HudMarkerBuilder : int {
    None = 0,
    GroupOwner = 1,  // 00642960 at 006431D9
    SubUnits = 2,    // 00642B80 at 00643206
    Delegating = 3,  // 00642040(unit, ..., 1, 2) at 0064326B
    Close = 4,       // 00641910 at 006432A9
    Ship = 5,        // 00642040(unit, ..., kindA, kindB) at 006432DB
    TrackedSet = 6,  // 0063F1E0 set insert at 00643311
};

// The three flags 006430C0 derives at 00643136, 0064315B and 0064316A, plus the
// objective reclassification at 00643183..006431A1.
struct HudMarkerFlags {
    bool is_self{false};
    bool is_friendly{false};
    bool is_target{false};
};

// 00643136..006431A1. `unit_team_id` is [unit+54h]; `local_team_record_id` is
// [[00E188A8+18CCh+team*4]+28h]; `in_objective_container` is the result of
// 008DDF90 at 00643183; `force_objective_colour` is the routine's second stack
// argument. Pure.
HudMarkerFlags hud_marker_flags_006430c0(const void* unit, const void* self_unit,
                                         const void* target_unit, int unit_team_id,
                                         int local_team_record_id, bool in_objective_container,
                                         bool force_objective_colour) noexcept;

// 006431B6..00643311. `probe` answers IsKindOf for the unit; the caller supplies
// it because the class test is a native virtual. `delegate_is_45_or_46` is the
// 1Bh branch's extra test on the [140h] subobject at 00643232 and 0064324D.
// Uncertainty: the 1Bh branch only reaches 00642040 when that test passes;
// when it fails control falls into the 1Ch probe, which the listing shows as
// `JZ 00643275` at 00643253 and which Ghidra's decompiler marks unreachable.
struct HudMarkerDispatch {
    HudMarkerBuilder builder{HudMarkerBuilder::None};
    int kind_a{0}; // fifth argument to 00642040
    int kind_b{0}; // sixth argument to 00642040
    bool close_highlight{false}; // fourth argument to 00641910
    bool run_lock_tail{false};   // the 1Eh/1Fh tail at 00643316
};

// A minimal class-test callback: returns whether the unit answers IsKindOf(id).
struct HudMarkerKindProbe {
    virtual ~HudMarkerKindProbe() = default;
    virtual bool is_kind_of(const void* unit, int class_id) const = 0;
};

HudMarkerDispatch hud_marker_dispatch_006430c0(const void* unit, const HudMarkerKindProbe& probe,
                                               const HudMarkerFlags& flags,
                                               bool force_objective_colour,
                                               bool delegate_is_45_or_46, int caller_kind_a,
                                               int caller_kind_b, int unit_team_id,
                                               int local_team_record_id) noexcept;

// ---------------------------------------------------------------------------
// 00640620 BSP_InGameHudMarkers_ResetMarkerPools
// ---------------------------------------------------------------------------

// Offsets of the four pools on the markers screen, all from 00640620's listing.
inline constexpr std::size_t kHudMarkerPoolAFirst = 0x50;
inline constexpr std::size_t kHudMarkerPoolALast = 0x54;
inline constexpr std::size_t kHudMarkerPoolACursor = 0x7C;
inline constexpr std::size_t kHudMarkerPoolBFirst = 0x60;
inline constexpr std::size_t kHudMarkerPoolBLast = 0x64;
inline constexpr std::size_t kHudMarkerPoolBCursor = 0x80;
inline constexpr std::size_t kHudMarkerPoolCFirst = 0x70;
inline constexpr std::size_t kHudMarkerPoolCLast = 0x74;
inline constexpr std::size_t kHudMarkerPoolCCursor = 0x84;
inline constexpr std::size_t kHudMarkerPoolDFirst = 0x94;
inline constexpr std::size_t kHudMarkerPoolDLast = 0x98;
inline constexpr std::size_t kHudMarkerPoolDCursor = 0xA0;

// The five widget slots of a pool B entry, hidden at 00640690..006406DE.
inline constexpr std::size_t kHudMarkerPoolBWidgetOffsets[5] = {0x04, 0x08, 0x0C, 0x14, 0x10};

// 006374B0 hides 24 widgets from entry+8h in six rounds of four (006374C1 and
// 006374C6 load 6 and 4) plus the one at entry+68h.
inline constexpr int kHudMarkerEntryWidgetRounds = 6;
inline constexpr int kHudMarkerEntryWidgetsPerRound = 4;

// Per-frame cursors the reset compares against. 006435D0 sets `pool_d` to 0
// before the opening call, so the opening call tears pool D down completely and
// the closing call at 00643D41 keeps exactly what the frame used.
struct HudMarkerPoolCursors {
    std::uint32_t pool_a{0};
    std::uint32_t pool_b{0};
    std::uint32_t pool_c{0};
    std::uint32_t pool_d{0};
};

// ---------------------------------------------------------------------------
// 0063A6C0 BSP_HudMarker_ProjectUnitScreenBounds
// ---------------------------------------------------------------------------

// The bound seeds at 00CE4970 and 00CE4ADC, loaded at 0063A6CD and 0063A710.
inline constexpr float kHudMarkerBoundsMin = 1.0e10f;
inline constexpr float kHudMarkerBoundsMax = -1.0e10f;

// The minimum-size rule: this[88h] divided by 00CE3D78 (1.5), then the widening
// half-step 00CEC9E0 (-0.5) and the y-axis factor 00CF5780 (1.33).
inline constexpr float kHudMarkerMinSizeDivisor = 1.5f;
inline constexpr float kHudMarkerWidenHalfStep = -0.5f;
inline constexpr float kHudMarkerMinSizeYFactor = 1.33f;

// The up-axis doubling applied to IsKindOf(6) units that are not IsKindOf(8).
// 00D7A308, a double. 0063A831.
inline constexpr float kHudMarkerUpAxisScale = 2.0f;

// Class-record extent offsets on [unit+538h], used at 0063A7C3, 0063A8D5 and
// 0063A911. The pairing of each extent with a matrix row is the loop's, not a
// guess: +A0h multiplies the forward row, +A4h the right row, +A8h the up row.
inline constexpr std::size_t kHudMarkerExtentForward = 0xA0;
inline constexpr std::size_t kHudMarkerExtentRight = 0xA4;
inline constexpr std::size_t kHudMarkerExtentUp = 0xA8;

// Screen-space rectangle 0063A6C0 produces.
struct HudMarkerScreenBounds {
    float min_x{kHudMarkerBoundsMin};
    float min_y{kHudMarkerBoundsMin};
    float max_x{kHudMarkerBoundsMax};
    float max_y{kHudMarkerBoundsMax};
    bool collapsed{false}; // the char* out argument, set at 0063ABAC
};

// One projected corner as 00638E50 reports it.
struct HudMarkerProjectedCorner {
    float x{0.0f};
    float y{0.0f};
    bool visible{false};
};

// 0063A6C0's accumulate step, applied once per corner. Only the min/max is
// taken; `visible` is not consulted at 0063A9DB..0063AA8B, which is why a
// marker can keep a bound from a clipped corner.
void hud_marker_accumulate_corner_0063a6c0(HudMarkerScreenBounds& bounds,
                                           const HudMarkerProjectedCorner& corner) noexcept;

// 0063AAB9..0063AB73, the x widening, and 0063AB25..0063AB53, the y widening.
// Returns true when both axes were widened, which is the condition under which
// 0063AB75 reprojects the anchor and collapses the rectangle to a point.
bool hud_marker_apply_minimum_size_0063a6c0(HudMarkerScreenBounds& bounds,
                                            float screen_field_88) noexcept;

// The eight corner signs of the triple loop at 0063A860, 0063A870 and 0063A890,
// in the order the loops produce them.
struct HudMarkerCornerSigns {
    float right{0.0f};
    float up{0.0f};
    float forward{0.0f};
};
HudMarkerCornerSigns hud_marker_corner_signs_0063a6c0(int index) noexcept;
inline constexpr int kHudMarkerCornerCount = 8;

// ---------------------------------------------------------------------------
// Host boundary
// ---------------------------------------------------------------------------

// The per-marker widget names 0063D1E0 binds, with the string address and the
// length it pushes to BSP_NativeString_Resize. They are children of
// sidemarker_Group and commandbuilding_Group in interface/gui_markers.lua.
inline constexpr const char* kHudMarkerWidgetUnitName = "Unit_name_Text";       // 00CF58CC, 0Eh
inline constexpr const char* kHudMarkerWidgetDistance = "Distance_Text";        // 00CF58BC, 0Dh
inline constexpr const char* kHudMarkerWidgetHealth = "HP_Icon";                // 00CF58B4, 7
inline constexpr const char* kHudMarkerWidgetHealthBack = "HP_BG_Icon";         // 00CF58A8, 0Ah
inline constexpr const char* kHudMarkerWidgetType = "type_Icon";                // 00CF57B4, 9
inline constexpr const char* kHudMarkerWidgetCommand = "commandbuilding_Icon";  // 00CF5818, 14h

// The three section-callout localisation keys of 00640D70, chosen by the part
// record's kind at [entry+4h].
inline constexpr const char* kHudMarkerSectionMagazine = "ingame.sections_magazine"; // kind 8
inline constexpr const char* kHudMarkerSectionEngine = "ingame.sections_engine";     // kind 5
inline constexpr const char* kHudMarkerSectionFuel = "ingame.sections_fuel";         // kind 6

// One native call site per method, in the order the markers screen reaches
// them. Nothing here has a default body; a host that cannot answer a step must
// say so rather than return a stand-in value.
struct HudMarkersRuntimeHost {
    virtual ~HudMarkersRuntimeHost() = default;

    // 00643616 -> 00AA1FE0. The GUI extent, width then height.
    virtual void gui_extent(float& width, float& height) = 0;
    // 0064368B -> 004B4B00. The camera's unit, stored at this[24h].
    virtual void* camera_unit() = 0;
    // 006436B7 -> 00927880. The unit the self marker points at, this[1Ch].
    virtual void* displayed_self_unit(void* controlled_unit) = 0;
    // 006430E4 -> 0080E490, then the byte at +14h tested at 006430E9.
    virtual bool parts_object_flag_is_one(void* unit) = 0;
    // vtable +5Ch, 006FE530, reached indirectly at 006430DC and eight more sites.
    virtual bool is_kind_of(void* unit, int class_id) = 0;
    // 006431A8 -> 0043F080.
    virtual bool is_alive_and_visible(void* unit) = 0;
    // 0064311C -> 0063ABD0 and 00643131 -> 004323D0, the frame's marked set.
    virtual bool already_marked(void* unit) = 0;
    virtual void mark_unit(void* unit) = 0;
    // 00643183 -> 008DDF90 against [00E188A8+21A4h+team*4].
    virtual bool objective_container_contains(void* unit) = 0;
    // 006420D8 -> 00639990, the six-value colour index.
    virtual int marker_colour_index(void* unit) = 0;
    // 006437C2 -> 00414DB0 when [unit+C8h] is clear.
    virtual void refresh_pose(void* unit) = 0;
    // The world matrix rows at unit+CCh, +DCh, +ECh and the translation +FCh.
    virtual void world_matrix_rows(void* unit, float right[3], float up[3], float forward[3],
                                   float translation[3]) = 0;
    // [unit+538h] and its three extents at +A0h, +A4h, +A8h.
    virtual void class_record_extents(void* unit, float& forward, float& right, float& up) = 0;
    // 0043A6A0 -> 00B70490 then 0043A6AF -> 00B62D10: the clip-space point.
    virtual void project_clip_space(const float world[3], float out_clip[4]) = 0;
    // 006422F0 -> 00803DC0, the class name the type icon and label use.
    virtual const char* unit_class_name(void* unit) = 0;
    // 0064240B -> 0077A2F0, the health fraction the HP icon scales by.
    virtual float unit_health(void* unit) = 0;
    // 0063D42A -> 00AA7E00.
    virtual void* find_child(void* widget, const char* name) = 0;
    // 0063D1E0's SetVisible calls, widget vtable +34h.
    virtual void set_visible(void* widget, bool visible) = 0;
    // 0063D3AF -> 00AA7DC0 and 0063D8B9 -> 00AA8240.
    virtual void set_local_position(void* widget, const HudGuiPoint& position) = 0;
    virtual void set_resolved_position(void* widget, const HudGuiPoint& position) = 0;
    // 0064304C -> 00AA7D00, the two-float form 00642C20 uses.
    virtual void set_local_xy(void* widget, float x, float y) = 0;
    // 0063D694 -> 00ABAED0 and 0063D751 -> 00ABBE50.
    virtual void set_localised_text(void* widget, const char* key) = 0;
    // 0063D7FD -> 00AA6740.
    virtual void widget_size(void* widget, float& width, float& height) = 0;
    // 00640620's four pools and 006374B0's per-entry hide.
    virtual void release_pool_tail(int pool, std::uint32_t cursor) = 0;
};

// One unit's pass through 006430C0 and, when it dispatches to 00642040, the
// screen bounds and widget writes 0063A6C0 and 0063D1E0 perform. Returns the
// builder that was selected, or HudMarkerBuilder::None when the unit was
// rejected. Coverage: complete for the gate, the flag derivation, the dispatch
// selection and the bounds rule; the icon-state choice inside 0063D1E0 past
// 0063D700 is not reproduced, so `bounds` is written and the label and health
// are fetched, but no texture state is selected.
HudMarkerBuilder hud_markers_add_unit_marker_006430c0(
    void* unit, const HudMarkerFlags& flags, bool force_objective_colour, int caller_kind_a,
    int caller_kind_b, bool delegate_is_45_or_46, int unit_team_id, int local_team_record_id,
    const HudMarkerClipRect& clip, bool wide_aspect_active, float screen_field_88,
    HudMarkersRuntimeHost& host, HudMarkerScreenBounds& bounds) noexcept;

} // namespace bsp
