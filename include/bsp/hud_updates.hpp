#pragma once
#include <cstdint>

#include "bsp/hud_screens.hpp"

// Packet hud_central_updates, docs/HUD_CENTRAL_UPDATES.md.
//
// The per-frame update virtuals (+20h) of the three central in-mission HUD
// screens: the HUD root (registry slot 44h, 00649860), the world markers screen
// (slot 4Dh, 006435D0) and the minimap (slot 35h, 005C0F20). All three are
// __thiscall(this, float seconds) and RET 4.
//
// Everything here is taken from the listing; the pseudocode of all three
// carries register inputs (the HUD root aliases its loop counters to floats,
// the markers screen keeps the squared distance on the x87 stack) so the
// constants and the operation order below come from the disassembly, not from
// the decompiler output. Descriptive names are hypotheses, not symbols.
namespace bsp {

// ---------------------------------------------------------------------------
// Shared value types
// ---------------------------------------------------------------------------

// The three-float argument BSP_GuiWidget_SetResolvedPosition (00AA8240) and
// BSP_GuiWidget_SetLocalPositionAndBounds (00AA7DC0) take by address. The third
// component is a depth, always a negative literal in these three routines.
struct HudGuiPoint {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// The screen-space rectangle 006435D0 publishes into 00E197C4..00E197D8.
struct HudMarkerClipRect {
    float left{0.0f};   // 00E197D0 and 00E197D8 both receive this
    float right{0.0f};  // 00E197CC and 00E197D4 both receive this
    float top{0.0f};    // 00E197C8
    float bottom{0.0f}; // 00E197C4
};

// ---------------------------------------------------------------------------
// Slot 44h, the HUD root update (00649860)
// ---------------------------------------------------------------------------

// Layout constants of the power-up column, all literal loads in 00649860.
inline constexpr float kHudRootColumnAnchor = 0.26f;          // 00CE8190
inline constexpr float kHudRootColumnXOffset = 0.935f;        // 00CF5C00, a double
inline constexpr float kHudRootFirstRowOffset = 0.14166668f;  // 00CF5C08, 17/120
inline constexpr float kHudRootRowGap = 0.013888889f;         // 00CEBDD0, 1/72, a double
inline constexpr float kHudRootCircleYOffset = 0.008333333f;  // 00CF0DE8, 1/120, a double
inline constexpr float kHudRootIconDepth = -100.0f;           // 00CE65D8
inline constexpr float kHudRootCircleDepth = -99.0f;          // 00CF5BFC
// Reload of the ticker timer at +BCh once the queue at +B0h has drained, 00CF5BF8.
inline constexpr float kHudRootTickerInterval = 7.0f;
// Records in the per-team power-up table are 38h bytes; the icon texture is at
// +30h and the UV rectangle at +34h (00649BC5, 00649BF4).
inline constexpr std::size_t kHudRootTeamRecordStride = 0x38;
inline constexpr std::size_t kHudRootTeamRecordTextureOffset = 0x30;
inline constexpr std::size_t kHudRootTeamRecordUvOffset = 0x34;
// The two page widgets 006463E0 binds as clone templates, hidden at bind time.
inline constexpr std::uint32_t kHudRootIconTemplateField = 0x0FCu;   // puptemplate_Icon
inline constexpr std::uint32_t kHudRootCircleTemplateField = 0x100u; // circletemplate_Section
// The input action the update polls at 0064A1B9 (004C43C0).
inline constexpr int kHudRootToggleInputAction = 0xDF;
// The interface ids 0064A0FC pushes to BSP_FrontEndManager_PushInterfaceRequest.
inline constexpr int kHudRootRequestLimbo = 0x34; // no controlled unit
inline constexpr int kHudRootRequestScene = 0x20; // controlled unit present

// y of the first cloned row. The platform singleton byte 0109CF04+0Dh selects
// between a flush start and the 17/120 inset (006499D9..006499FC).
float hud_root_first_row_y(bool platform_inset) noexcept;

// Running y after placing a row whose template is `template_height` tall
// (00649B1C..00649B3E): the accumulator gains the template height plus 1/72
// before the row is positioned, so the returned value is this row's y.
float hud_root_advance_row_y(float previous_y, float template_height) noexcept;

// Position handed to BSP_GuiWidget_SetResolvedPosition for a power-up icon
// clone (00649B42..00649B58).
HudGuiPoint hud_root_icon_position(float row_y) noexcept;

// Position of the circle clone that accompanies a timed power-up
// (00649ED9..00649F08). Its x is the icon column x; its y ignores the row
// accumulator and sits one 1/120 above the column anchor.
HudGuiPoint hud_root_circle_position() noexcept;

// The timed-power-up gate at 00649C6B: the entry runs only while its deadline
// in the float vector at +154h is still ahead of the mission clock 00F876A4.
bool hud_root_timed_entry_active(float deadline, float mission_clock) noexcept;

// The fill the circle clone receives at 00649F8C..00649FA0: remaining time over
// the entry's full duration, read from the record at +7Ch. Not clamped by the
// native code. `duration` of zero reproduces the native division by zero only
// if the caller allows it, so it is rejected here and returns 0.
float hud_root_circle_fill(float deadline, float mission_clock, float duration) noexcept;

// Fields of the slot 44h screen that 00649860 reads or writes.
struct HudRootUpdateState {
    std::int32_t update_countdown{0};  // +F4h, decremented then reloaded with 2
    float ticker_timer{0.0f};          // +BCh, counted down by the frame delta
    std::uint32_t ticker_queue_count{0}; // (+B4h - +B0h) / 10h
    std::uint16_t pending_unit_handle{0}; // +78h, a unit handle or 0
    bool pending_group_request{false}; // +20h, cleared once 006485A0 has run
    std::uint32_t selector_widget{0};  // +40h, shown every frame
    std::uint32_t weapon_info_field{0};// +C2h, written from 00644CC0/00644C20
    bool closed_hud_flag{false};       // +1Ch, read by the toggle test
};

// Integration boundary for 00649860. One method per native call site, in body
// order. Nothing here stands in for behaviour that was not recovered.
struct HudRootUpdateHost {
    virtual ~HudRootUpdateHost() = default;

    // [00E188A8 + 61Fh] or [00E188A8 + 620h]: either set suppresses the body.
    virtual bool game_hud_suppressed() = 0;
    // 0109CF04 + 0Dh, the platform singleton flag that insets the first row.
    virtual bool platform_row_inset() = 0;
    // 00F876A4, the mission clock the timed-entry gate and the circle fill
    // both subtract from the entry deadline.
    virtual float mission_clock() = 0;
    // 00E188D8. The whole clone pass is skipped when it is null.
    virtual bool controlled_unit_present() = 0;

    // Destroy every widget in the vector at +104h (untimed clones) and at +114h
    // (circle clones) through widget virtual +4h(1), then erase both vectors.
    virtual void release_icon_clones() = 0;
    virtual void release_circle_clones() = 0;

    // 008E9AF0 on the power-up manager 00F88C30, filling the screen's five
    // vectors: +164h (untimed entries), +124h (timed entries), +134h, +144h and
    // +154h (the timed deadlines). The last two arguments are literal 0 and 1.
    virtual void collect_powerups() = 0;
    virtual std::size_t untimed_entry_count() = 0;   // (+16Ch - +168h) / 4
    virtual std::size_t timed_entry_count() = 0;     // (+12Ch - +128h) / 4
    virtual float timed_entry_deadline(std::size_t index) = 0; // +158h[index]

    // 008E62A0(entry, 00E188D8): does this power-up apply to the controlled
    // unit. Scope 4 compares unit+180h, scope 5 compares unit+54h.
    virtual bool untimed_entry_applies(std::size_t index) = 0;
    virtual bool timed_entry_applies(std::size_t index) = 0;

    // BSP_GuiWidget_CloneSubtree (00AAB4C0) on +FCh or +100h, then virtual
    // +34h(1) on the clone and a push_back into the matching vector.
    virtual std::uint32_t clone_icon_template() = 0;
    virtual std::uint32_t clone_circle_template() = 0;
    // BSP_GuiWidget_GetSize (00AA6740) on +FCh; only the height is used.
    virtual float icon_template_height() = 0;
    // BSP_GuiWidget_SetResolvedPosition (00AA8240).
    virtual void widget_set_resolved_position(std::uint32_t widget, const HudGuiPoint& p) = 0;
    // [00E188A8 + 18ECh], the local team index used to pick the team record.
    virtual int local_team_index() = 0;
    // 00AB64C0(clone, texture, uv): appends a state to the icon's state vector
    // at +F4h and returns its index. Texture and UV come from the entry's
    // record at +30h and +34h, strided by 38h with the team record id.
    virtual void icon_add_state_from_entry(std::uint32_t clone, std::size_t index,
                                           bool timed) = 0;
    // Clone virtual +88h(1, 0, 1.0f), the state select that follows every add.
    virtual void widget_select_state(std::uint32_t widget) = 0;
    // 00ABE6E0(clone, fill, 0, 0, 1.0f): four floats into +F4h, +F8h, +100h and
    // +104h, then virtual +7Ch to re-emit.
    virtual void circle_set_fill(std::uint32_t widget, float fill) = 0;
    // The entry's full duration at record+7Ch, the divisor of the circle fill.
    virtual float timed_entry_duration(std::size_t index) = 0;

    // 00648060, the message ticker step once +BCh has expired.
    virtual void advance_ticker() = 0;
    // The handle table at 00F89A54/00F89AA8 resolved from +78h; false when the
    // entry is missing or fails the +5Ch/+5Dh/+60h/+5Eh byte filter.
    virtual bool pending_unit_selectable(std::uint16_t handle) = 0;
    // 00645060 on the resolved unit with the local team index in EDX and a
    // literal 1 on the stack.
    virtual bool pending_unit_allowed() = 0;
    virtual void commit_pending_unit() = 0; // 00645600(this, unit)
    // The two-branch interface request at 0064A0BF/0064A0FA through
    // BSP_FrontEndManager_PushInterfaceRequest (004CC460). The payload of the
    // scene request is the unit's virtual +140h.
    virtual bool interface_manager_idle() = 0; // +4h == +20h and +1Ch == +38h
    virtual void notify_camera_and_input() = 0; // 0059DA80 then 005251C0
    virtual int controlled_unit_scene_payload() = 0; // 00E188D8 virtual +140h
    virtual void push_interface_request(int interface_id, int payload) = 0;

    virtual void widget_set_shown(std::uint32_t widget, bool shown) = 0; // +34h
    // 00644CC0 then, when the word at +2h of its result is FFFFh, 00644C20.
    virtual std::uint32_t resolve_weapon_info(bool& needs_fallback) = 0;
    virtual std::uint32_t resolve_weapon_info_fallback() = 0;
    virtual void update_unit_rows(float seconds) = 0; // 00644DB0
    virtual void update_medals() = 0;                 // 00648C20

    // The three unit-kind probes of the toggle test, virtual +5Ch with 9, 45h
    // and 46h, plus the 18h probe and the +379h byte of the second test.
    virtual bool controlled_unit_is_kind(int kind) = 0;
    virtual bool game_blocks_toggle() = 0;       // [00E188A8 + 19C4h]
    virtual bool input_action_pressed(int action) = 0; // 004C43C0
    virtual bool controlled_unit_flag_379() = 0;
    virtual void toggle_closed_hud() = 0; // 00647080

    // The group-change tail: 006485A0 when the game has a group manager at
    // +1FE4h and the controlled unit's team at +188h is below 8 and differs
    // from the local team index.
    virtual bool game_has_group_manager() = 0;
    virtual int controlled_unit_team() = 0;
    virtual void rebuild_group_rows() = 0;
};

// The whole of 00649860. `seconds` is the native float argument.
void hud_root_screen_update(HudRootUpdateState& state, HudRootUpdateHost& host,
                            float seconds);

// ---------------------------------------------------------------------------
// Slot 4Dh, the world markers update (006435D0)
// ---------------------------------------------------------------------------

// 006435D0 multiplies the GUI extent by this before building the clip rect.
inline constexpr float kHudMarkerClipHalfExtent = 0.4925f; // 00CF5990, a double
inline constexpr float kHudMarkerClipCentre = 0.5f;        // 00D7A280, a double
// The screen fraction the crosshair pick uses; the enter virtual writes both.
inline constexpr float kHudMarkerCrosshairFraction = 0.5f; // +44h and +48h
// 00643C9C: the objective sweep runs only while 00F88A00 exceeds this.
inline constexpr float kHudMarkerObjectiveThreshold = 0.03f; // 00CEB690, a double
// Marker kinds, the trailing three arguments of 006430C0.
inline constexpr int kHudMarkerKindSelf = 2;   // controlled unit and squad mates
inline constexpr int kHudMarkerKindTarget = 1; // the interface manager's target
// Group objects hold their member count at +3CCh and up to five members from
// +3D0h (00643C28, 00643D00).
inline constexpr std::size_t kHudMarkerGroupCountOffset = 0x3CC;
inline constexpr std::size_t kHudMarkerGroupMemberOffset = 0x3D0;
inline constexpr int kHudMarkerGroupMemberLimit = 5;

// The rectangle published into 00E197C4..00E197D8 from the GUI extent
// (0064361B..0064367F). The two x bounds are each stored twice.
HudMarkerClipRect hud_marker_clip_rect(float gui_width, float gui_height) noexcept;

// The squad-mate radius test at 006437DD..00643853: a marker is added only when
// the squared distance is strictly below the squared radius held as an integer
// at unit+7C4h. Computed in the native order dx*dx + dy*dy + dz*dz.
bool hud_marker_within_radius(float dx, float dy, float dz, int radius) noexcept;

// Fields of the slot 4Dh screen that 006435D0 reads or writes.
struct HudMarkersUpdateState {
    float crosshair_x{0.5f};      // +44h
    float crosshair_y{0.5f};      // +48h
    std::uint32_t camera_unit{0};  // +24h, from 004B4B00
    std::uint32_t self_marker_unit{0}; // +1Ch, from 00927880 on 00E188D8
    std::uint32_t target_unit{0};  // +20h
    bool markers_disabled{false};  // +28h, skips the whole marker pass
    std::uint32_t pool_flag{0};    // +A0h, cleared at the top
};

// Integration boundary for 006435D0, in body order.
struct HudMarkersUpdateHost {
    virtual ~HudMarkersUpdateHost() = default;

    virtual int local_team_index() = 0;    // [00E188A8 + 18ECh], must be 0..7
    virtual bool game_hud_suppressed() = 0; // +61Fh or +620h
    // 00AA1FE0, the GUI extent the clip rectangle is built from.
    virtual void gui_extent(float& width, float& height) = 0;
    virtual void publish_clip_rect(const HudMarkerClipRect& rect) = 0;

    virtual std::uint32_t camera_unit() = 0;        // 004B4B00
    virtual void reset_marker_pool() = 0;           // 00640620
    virtual void clear_marker_set() = 0;            // 0063BCD0 on +34h
    virtual bool controlled_unit_present() = 0;     // 00E188D8
    virtual std::uint32_t self_marker_unit() = 0;   // 00927880 on 00E188D8
    virtual void refresh_screen_state() = 0;        // 006394B0
    virtual void refresh_marker_layout() = 0;       // 0063B5E0 on +A4h

    // 006430C0(unit, a, b, kind) then 004323D0 into the set at +34h.
    virtual void add_marker(std::uint32_t unit, int a, int b, int kind) = 0;
    virtual void note_marker(std::uint32_t unit) = 0;

    // [[00E198C4 + CCh] + 4Ch], the interface manager's current target.
    virtual std::uint32_t interface_target_unit() = 0;
    virtual int unit_team_id(std::uint32_t unit) = 0;      // unit+54h
    virtual int local_team_record_id() = 0;                // team record +28h
    virtual bool unit_passes_target_filter(std::uint32_t unit) = 0; // 00804350(_,5)
    virtual bool target_is_selectable(std::uint32_t unit) = 0;      // 005220C0

    // The squad list at [[00E188A8 + 19CCh] + 16Ch], payload at node+8h.
    virtual std::size_t squad_member_count() = 0;
    virtual std::uint32_t squad_member(std::size_t index) = 0;
    virtual void refresh_unit_pose(std::uint32_t unit) = 0; // 00414DB0 when +C8h is 0
    virtual void unit_position(std::uint32_t unit, float& x, float& y, float& z) = 0;
    virtual int squad_marker_radius(std::uint32_t unit) = 0; // (int)unit+7C4h

    // The screen-centre world pick at 0043A290 with the viewport size from
    // [[00E188A8 + 19FCh]] +10h and +14h; the result and its validity byte go
    // to the interface manager at +F0h and +FCh.
    virtual bool pick_world_point(float screen_x, float screen_y,
                                  float& wx, float& wy, float& wz) = 0;
    virtual void publish_crosshair_point(bool valid, float wx, float wy, float wz) = 0;
    virtual void viewport_size(int& width, int& height) = 0;

    // The objective sweep over [00E188A8 + team*4 + 21A4h] and the per-objective
    // sub-list; each surviving objective is placed by 00643360.
    virtual void sweep_objectives() = 0;
    // The command-unit sweep over [[00E188A8 + 19CCh] + 58h] guarded by +1FE4h.
    virtual void sweep_command_units() = 0;
    // The group sweep of the current target, 006434E0(member, 1) per member and
    // 006434E0(target, 0) for the target itself.
    virtual void add_group_marker(std::uint32_t unit, int flag) = 0;
    virtual std::uint32_t query_target_unit() = 0;  // 00523020
    virtual bool unit_is_group_leader(std::uint32_t unit) = 0; // virtual +5Ch(0Fh)
    virtual std::size_t group_member_count(std::uint32_t unit) = 0;
    virtual std::uint32_t group_member(std::uint32_t unit, std::size_t index) = 0;
    virtual void flush_markers() = 0; // 00640D70 then 00640620
    // 00F88A00 against 00CEB690, the gate of the reinforcement sweep.
    virtual float objective_detail_level() = 0;
    virtual void sweep_reinforcements() = 0;
};

// The whole of 006435D0. Returns false when the top gate rejected the frame.
bool hud_markers_screen_update(HudMarkersUpdateState& state,
                               HudMarkersUpdateHost& host, float seconds);

// ---------------------------------------------------------------------------
// Slot 35h, the minimap update (005C0F20)
// ---------------------------------------------------------------------------

// The capture-marker pulse, 005C0F9E..005C0FEA and again at 005C12C6.
inline constexpr float kHudMinimapPulseRate = 4.0f;      // 00D7A328, a double
inline constexpr float kHudMinimapPulseCentre = 0.5f;    // 00D7A280, a double
inline constexpr float kHudMinimapPulseScale = 0.1f;     // 00D7A3A0, a double
inline constexpr float kHudMinimapPulseBase = 1.0f;      // 00D7A210, a double
inline constexpr float kHudMinimapRampRate = 4.0f;       // 00D7A328 again
// World-to-minimap conversion of every icon placement (005C1C68, 005C1C8C).
inline constexpr float kHudMinimapXDivisor = 1024.0f;    // 00CEDAE8 is 1/1024
inline constexpr float kHudMinimapYDivisor = 768.0f;     // 00CE42B0
inline constexpr float kHudMinimapHeadingBias = 1.5707964f; // 00CE3830, pi/2
// The depth literals the three placement sites choose between.
inline constexpr float kHudMinimapDepthSelf = -1.0f;     // 00D7A260
inline constexpr float kHudMinimapDepthControlled = -4.0f; // 00CF1430
inline constexpr float kHudMinimapDepthHighlight = -2.0f;  // 00CE7D7C
inline constexpr float kHudMinimapDepthCapture = -8.0f;    // 00CE3CC8
inline constexpr float kHudMinimapDepthMarker = -10.0f;    // 00CE6848
inline constexpr float kHudMinimapCompassDivisor = 80.0f;  // 00CF1440

// sin(mission_clock * 4), the shared pulse of both animation branches.
float hud_minimap_pulse(float mission_clock) noexcept;
// Alpha of the pulsing capture widget: 0.5 * (1 + pulse). The colour handed to
// widget virtual +50h is (1, 1, 1, alpha).
float hud_minimap_pulse_alpha(float pulse) noexcept;
// The 0..1 ramp of the animation clock at +16Ch, saturating at 1 (005C103D).
float hud_minimap_intro_ramp(float animation_clock) noexcept;
// The uniform scale handed to widget virtual +48h: ramp * (0.1 * pulse + 1).
float hud_minimap_pulse_scale(float ramp, float pulse) noexcept;
// The local position of a minimap icon, 005C1C61..005C1C99 and its two twins.
// The world-relative offset is divided anisotropically and z is a depth layer.
HudGuiPoint hud_minimap_icon_position(float world_dx, float world_dz,
                                      float depth) noexcept;
// The rotation handed to widget virtual +44h: pi/2 minus the icon heading.
float hud_minimap_icon_rotation(float heading) noexcept;
// The map-extent cull at 005C16D3: an icon is dropped once the squared distance
// exceeds the squared half-extent read from the world descriptor at +70h.
bool hud_minimap_within_map(float squared_distance, float half_extent) noexcept;

// Fields of the slot 35h screen that 005C0F20 reads or writes.
struct HudMinimapUpdateState {
    std::uint32_t selected_capture{0}; // +F4h, the selected capture point or 0
    bool fade_latch{false};            // +169h
    float animation_clock{0.0f};       // +16Ch
    std::uint32_t capture_widget{0};   // +54h
    std::uint32_t capture_ring_a{0};   // +58h
    std::uint32_t capture_ring_b{0};   // +5Ch
    std::uint32_t capture_group{0};    // +60h
    std::uint32_t compass_widget{0};   // +4Ch
    std::uint32_t marker_widget{0};    // +1ACh
};

// Integration boundary for the part of 005C0F20 this packet recovered.
struct HudMinimapUpdateHost {
    virtual ~HudMinimapUpdateHost() = default;

    virtual void refresh_transforms() = 0;   // 005BD420, the unconditional head
    virtual float mission_clock() = 0;       // 00F876A4
    virtual void widget_set_shown(std::uint32_t widget, bool shown) = 0; // +34h
    virtual bool widget_query(std::uint32_t widget) = 0;                 // +38h
    virtual void widget_set_color(std::uint32_t widget, float r, float g,
                                  float b, float a) = 0;                 // +50h
    virtual void widget_set_scale(std::uint32_t widget, float sx, float sy) = 0; // +48h
    virtual void widget_set_rotation(std::uint32_t widget, float radians) = 0;   // +44h
    virtual void widget_select_state(std::uint32_t widget, int state) = 0;       // +88h
    virtual void widget_set_local_position(std::uint32_t widget,
                                           const HudGuiPoint& p) = 0; // 00AA7DC0
    // [selected capture + 7ACh], the icon state the capture widget shows.
    virtual int capture_icon_state(std::uint32_t capture) = 0;
    // 006F1F90 on the selected capture, a float the ring scale multiplies.
    virtual float capture_ring_factor(std::uint32_t capture) = 0;

    virtual int local_team_index() = 0;      // [00E188A8 + 18ECh], must be 0..7
    virtual std::uint32_t camera_unit() = 0; // 004B4B00
    // 00432650, the world descriptor; +6Ch and +70h are the map half-extents.
    virtual void map_half_extents(float& x, float& z) = 0;
    virtual void refresh_unit_pose(std::uint32_t unit) = 0; // 00414DB0 when +C8h is 0
    virtual void unit_position(std::uint32_t unit, float& x, float& y, float& z) = 0;
    virtual std::uint32_t self_marker_unit() = 0; // 00927880 on 00E188D8

    // The team unit list at [[00E188A8 + team*4 + 18CCh] + 30h] + E0Ch.
    virtual std::size_t team_unit_count() = 0;
    virtual std::uint32_t team_unit(std::size_t index) = 0;
    // The four byte filters (+5Ch set, +5Dh, +60h and +5Eh clear) and the two
    // virtual probes, +5Ch(5) and +B8h.
    virtual bool unit_drawable(std::uint32_t unit) = 0;
    // 00427EB0, the icon-space position, and 00427E30, its squared length.
    virtual void unit_icon_offset(std::uint32_t unit, float& dx, float& dy, float& dz) = 0;
    virtual float squared_length(float dx, float dy, float dz) = 0;
    // The per-unit icon map at +F8h: 005BE110 looks up, 005BD590 constructs a
    // 0Ch-byte entry and 005C0700 inserts it.
    virtual bool icon_missing(std::uint32_t unit) = 0;
    virtual void create_icon(std::uint32_t unit) = 0;
    virtual void attach_icon(std::uint32_t unit) = 0; // 00694A60

    // The camera heading pair used by the compass: [[00E188A8 + 19FCh] + 110h]
    // and +118h through the CRT atan2 helper 00BF701A, with the refresh at
    // 00B6DB70 when bit 1 of +5Ch is clear.
    virtual float camera_heading() = 0;
};

// The animation prologue of 005C0F20, body 005C0F3F..005C154E. The rest of the
// routine is analyzed only; see docs/HUD_CENTRAL_UPDATES.md.
void hud_minimap_update_animation(HudMinimapUpdateState& state,
                                  HudMinimapUpdateHost& host, float seconds);

// The per-unit icon pass of 005C0F20, body 005C154E..005C1783. Returns the
// number of units that survived the filters and the map-extent cull.
std::size_t hud_minimap_update_unit_icons(HudMinimapUpdateState& state,
                                          HudMinimapUpdateHost& host);

} // namespace bsp
