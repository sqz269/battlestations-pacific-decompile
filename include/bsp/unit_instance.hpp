#pragma once
#include "bsp/pose_refresh.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

// Per-frame update of a unit instance, docs/UNIT_INSTANCE_UPDATE.md.
//
// The native object is the 0x1188-byte instance that a vehicle-class descriptor
// allocates through its vtable +28h (docs/SCENE_UNIT_CREATORS.md); its constructor
// is 006FE460 and its primary vftable 00CFC3D0. The world update 00904BF0 walks the
// world node's child chain once per frame and calls slot 0DCh on every child whose
// activation byte is set; for this class that slot is 008255B0.
//
// Nothing here is a drop-in binary replacement. The layout constants are offsets
// read out of the listing, not a reconstructed struct: the object is only partly
// mapped and the C++ types below carry just the fields the update touches.

namespace bsp {

// ---------------------------------------------------------------------------
// Layout, from the listing sites named in docs/UNIT_INSTANCE_UPDATE.md.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitInstanceSize = 0x1188; // operator new in 006FE590

// The eight vptr slots 006FE460 writes, in the order it writes them.
inline constexpr std::size_t kUnitVptrOffsets[8] = {
    0x000, 0x010, 0x024, 0x170, 0x1E4, 0x310, 0x38C, 0x72C};
inline constexpr std::uint32_t kUnitVftables[8] = {
    0x00CFC3D0, 0x00CFC3B8, 0x00CFC3B0, 0x00CFC3AC,
    0x00CFC3A4, 0x00CFC38C, 0x00CFC388, 0x00CFC384};

// Field offsets touched by the update. Names are descriptions, not symbols.
inline constexpr std::size_t kUnitOffParentNode = 0x030;    // 006FE628
inline constexpr std::size_t kUnitOffSiblingPrev = 0x034;   // 009041DC
inline constexpr std::size_t kUnitOffSiblingNext = 0x038;   // 00904C1A
inline constexpr std::size_t kUnitOffActiveByte = 0x05C;    // 00904C00, the tick gate
inline constexpr std::size_t kUnitOffSimulateByte = 0x05D;  // 0082568F
inline constexpr std::size_t kUnitOffDeadByte = 0x05E;      // 00904CA4
inline constexpr std::size_t kUnitOffClassId = 0x0C4;       // 006FE4B3, 7 for MDestroyer
inline constexpr std::size_t kUnitOffPoseValid = 0x0C8;     // 00825A64
inline constexpr std::size_t kUnitOffPoseBlock = 0x0CC;     // 00825A91
inline constexpr std::size_t kUnitOffPoseLateral = 0x0F0;   // [pose+24h]
inline constexpr std::size_t kUnitOffPoseBase = 0x100;      // 00825AB4
inline constexpr std::size_t kUnitOffNameLength = 0x154;    // docs/SCENE_UNIT_CREATORS.md
inline constexpr std::size_t kUnitOffIntensityOverride = 0x2F0; // 006FF27A
inline constexpr std::size_t kUnitOffIntensityScale = 0x2F4;    // 006FF295
inline constexpr std::size_t kUnitOffDescriptor = 0x354;    // 006FE590, 00956696
inline constexpr std::size_t kUnitOffDrivenSubUnit = 0x3D0; // 004C08C8
inline constexpr std::size_t kUnitOffSceneNode = 0x4A4;     // 008255FA
inline constexpr std::size_t kUnitOffAge = 0x524;           // 00956626
inline constexpr std::size_t kUnitOffClassBlock = 0x538;    // 00825728
inline constexpr std::size_t kUnitOffFireCountdown = 0x6D8; // 0095666F
inline constexpr std::size_t kUnitOffClampedCountdown = 0x728; // 0095662C
inline constexpr std::size_t kUnitOffEffectOffsets = 0x74C; // 00825613
inline constexpr std::size_t kUnitOffEffectHandles = 0x758; // 008255E2
inline constexpr std::size_t kUnitOffEffectCount = 0x75C;   // 008255CD
inline constexpr std::size_t kUnitOffBowAnchorSink = 0x9F0; // 00825870
inline constexpr std::size_t kUnitOffSternAnchorSink = 0x9F4; // 00825886
inline constexpr std::size_t kUnitOffPartsBegin = 0xA14;    // 00825D83
inline constexpr std::size_t kUnitOffPartsCount = 0xA18;    // 00825D89
inline constexpr std::size_t kUnitOffAttachSlots = 0xB54;   // 00825C2D
inline constexpr std::size_t kUnitOffAttachPoints = 0xB68;  // 00825C33
inline constexpr std::size_t kUnitOffBubbleTimer = 0xBC8;   // 008256D3
inline constexpr std::size_t kUnitOffHitLatchCurrent = 0x1010; // 00825824
inline constexpr std::size_t kUnitOffHitLatchPrevious = 0x1011; // 00825838
inline constexpr std::size_t kUnitOffController = 0x1018;   // 00825832
inline constexpr std::size_t kUnitOffIntensityProduct = 0x10A4; // 00825A44
inline constexpr std::size_t kUnitOffPropWash = 0x1170;     // 00825BD4

// Scene node fields the update reads through kUnitOffSceneNode.
inline constexpr std::size_t kSceneNodeFlags = 0x05C;      // bit 1 = world matrix clean
inline constexpr std::size_t kSceneNodeWorldMatrix = 0x0F0;
inline constexpr std::size_t kSceneNodeWorldY = 0x124;     // row 3, column 1
inline constexpr std::uint8_t kSceneNodeMatrixCleanBit = 0x02; // 00825600

// Vtable slots established for the primary vftable 00CFC3D0.
inline constexpr std::size_t kUnitVtableDestructor = 0x000;
inline constexpr std::size_t kUnitVtableSelfAccessor = 0x004;
inline constexpr std::size_t kUnitVtableControlAnchor = 0x018;
inline constexpr std::size_t kUnitVtableIsKindOf = 0x05C;
inline constexpr std::size_t kUnitVtableSetParent = 0x098;
inline constexpr std::size_t kUnitVtableUpdate = 0x0DC;   // the per-frame update
inline constexpr std::size_t kUnitVtableAttachWorld = 0x130;
inline constexpr std::size_t kUnitVtableDetachWorld = 0x134;

// Constants read from the image by 008255B0 and its immediate callees.
inline constexpr float kUnitIntensityFull = 1.0f;          // 00D7A24C
inline constexpr float kUnitIntensityControlled = 0.5f;    // 00CE3800
inline constexpr float kUnitWaterSurfaceThreshold = -5.0f; // 00CFBC84
inline constexpr double kUnitSubmergedCullDepth = -0.5;    // 00CEC9E0
inline constexpr double kUnitAnchorSurfaceLift = 0.5;      // 00D7A280
inline constexpr double kUnitWakeHalfWidthScale = 0.5;     // 00D7A280, 00825A77

// The class ids 006FE530 answers true to, in listing order. Id 3 is absent.
inline constexpr int kUnitDestroyerClassId = 7;
inline constexpr int kUnitDestroyerAncestry[7] = {7, 6, 5, 4, 2, 1, 0};

// ---------------------------------------------------------------------------
// Integration rules as pure functions. Each reproduces one native expression in
// the native operation order; none of them touches global state.
// ---------------------------------------------------------------------------

// 006FE530, the IsKindOf predicate. own_class_id is the instance field at +0C4h.
bool unit_is_kind_of_006fe530(int class_id, int own_class_id) noexcept;

// 006FF270 and the inlined copy at 008259FD. global_override is DAT_00F87152.
float unit_intensity_scale_006ff270(bool global_override, bool local_override,
                                    float local_scale) noexcept;

// 008259EB..00825A44: the product written back to +10A4h.
float unit_intensity_product_008259eb(bool global_override, bool local_override,
                                      float local_scale, bool is_controlled_unit) noexcept;

// 00825621: an effect slot is released once its anchor sits below the cull depth.
bool unit_effect_submerged_00825621(float local_offset_y, float node_world_y) noexcept;

// 00956626: the unbounded age accumulator at +524h.
float unit_advance_age_00956626(float age, float scaled_delta) noexcept;

// 0095662C: countdown at +728h, only stepped while positive, clamped at zero.
float unit_advance_clamped_countdown_0095662c(float remaining, float scaled_delta) noexcept;

// 0095666F: countdown at +6D8h, stepped unconditionally and allowed to go negative.
float unit_advance_countdown_0095666f(float remaining, float scaled_delta) noexcept;

// 008256D3: the bubble timer at +BC8h. Returns true when it expired this frame,
// in which case the caller reloads it from the random draw of step 2.
struct UnitBubbleTimerStep {
    float remaining{0.0f};
    bool expired{false};
};
UnitBubbleTimerStep unit_step_bubble_timer_008256d3(float remaining, float scaled_delta) noexcept;

// 00825824: the one-frame edge latch pair at +1010h/+1011h.
struct UnitHitLatch {
    bool current{false};
    bool previous{false};
};
void unit_rotate_hit_latch_00825824(UnitHitLatch& latch) noexcept;

// A world-space point produced by the anchor transforms of step 5 and step 10.
struct UnitAnchorPoint {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// 00825946..008259AA: when the bow anchor is above the water threshold both
// anchors are snapped to the ocean surface plus the half-metre lift. The test
// reads the bow anchor only; the stern follows it.
bool unit_anchors_snap_to_surface_00825946(float bow_y) noexcept;
float unit_anchor_surface_height_00825977(float sampled_ocean_height) noexcept;

// 00825A64..00825AF0: four interleaved validity checks/refreshes and the two
// wake test points of step 8. The pose belongs to the native unit (ECX/ESI),
// not its +4A4h scene node. Width is borrowed so its read follows the first check.
struct UnitWakeSpan {
    float left{0.0f};
    float right{0.0f};
};
UnitWakeSpan unit_wake_span_00825a6b(PoseRefreshView& unit_pose,
                                   const float& descriptor_width);
bool unit_wake_spawns_00825b19(const UnitWakeSpan& span) noexcept;

// 0082583E: the argument 00815AA0 receives is a 0/1 scalar, not the delta.
float unit_gated_sub_update_scalar_0082583e(float controller_value) noexcept;

// ---------------------------------------------------------------------------
// State the update reads and writes.
// ---------------------------------------------------------------------------

struct UnitEffectSlot {
    bool live{false};       // the pointer at [+758h] + index*4
    float local_offset_y{0.0f}; // [+74Ch] + index*0Ch, the y at +4
};

struct UnitAttachSlot {
    bool live{false};             // [+B54h] + index*4
    UnitAnchorPoint local_point{}; // [+B68h] + index*0Ch
};

inline constexpr std::size_t kUnitAttachSlotCount = 5; // 00825C39

struct UnitInstanceState {
    // Required canonical field projection for this same native unit owner:
    // parent +3Ch, local +74h, valid +C8h, world +CCh, derived-valid +10Ch.
    // No copied pose values or default pose; the binding must outlive the state.
    PoseRefreshView& pose;
    int class_id{kUnitDestroyerClassId}; // +0C4h
    bool active{true};                   // +05Ch, the world tick gate
    bool simulate{true};                 // +05Dh
    bool intensity_override{false};      // +2F0h
    float intensity_scale{1.0f};         // +2F4h
    float intensity_product{0.0f};       // +10A4h
    float age{0.0f};                     // +524h
    float fire_countdown{0.0f};          // +6D8h
    float clamped_countdown{0.0f};       // +728h
    float bubble_timer{0.0f};            // +BC8h
    UnitHitLatch hit_latch{};            // +1010h / +1011h
    bool has_bow_anchor_sink{false};     // +9F0h
    bool has_stern_anchor_sink{false};   // +9F4h
    bool has_scene_node{true};           // +4A4h
    bool has_prop_wash{false};           // +1170h
    bool controller_one_shot{false};     // [+1018h]+88h
    std::vector<UnitEffectSlot> effect_slots{};            // +758h / +75Ch
    UnitAttachSlot attach_slots[kUnitAttachSlotCount]{};   // +B54h / +B68h
    std::size_t part_count{0};                             // +A18h
};

// The per-class constants the update reads through +538h and +354h.
struct UnitClassBlock {
    bool has_bubble_template{false};  // [+538h]+0F0h
    bool has_attachments{false};      // [+538h]+6A8h
    float wake_width{0.0f};           // [+538h]+0A0h
};

// ---------------------------------------------------------------------------
// Host: one method per native call site of 008255B0, in frame order. There are
// no default implementations; nothing here stands in for unrecovered behaviour.
// ---------------------------------------------------------------------------

struct UnitInstanceHost {
    virtual ~UnitInstanceHost() = default;

    // 00B6DB70, BSP_Transform_RefreshWorldMatrix, called whenever bit 1 of the
    // scene node's +5Ch is clear. Returns the node's world Y at +124h.
    virtual float refresh_scene_node_world_matrix() = 0;
    // 00867B10 then handle->byte_9 = 1 then the ref-count release, step 1.
    virtual void release_submerged_effect(std::size_t slot) = 0;
    // The ocean object at [game+19FCh]; step 2 runs only while its world Y < 0.
    virtual bool ocean_exists() = 0;
    virtual float ocean_world_y() = 0;
    // 00BD2F10 between the two settings floats at 00424C40()+64Ch and +650h.
    virtual float draw_bubble_interval() = 0;
    // 008687C0 with the stack identity frame, step 2.
    virtual void spawn_bubble_effect() = 0;
    // 0092D730 on the controller at +1018h; its sign selects the step-4 scalar.
    virtual float controller_time_value() = 0;
    virtual void sub_update_00815aa0(float scalar) = 0;
    // 00413920 twice, against [+538h]+5B0h and +5F0h, step 5.
    virtual UnitAnchorPoint transform_bow_anchor() = 0;
    virtual UnitAnchorPoint transform_stern_anchor() = 0;
    // 0078CF20 on the ocean, step 5.
    virtual float sample_ocean_height(float x, float z) = 0;
    // 004842C0 on the two receivers at +9F0h and +9F4h.
    virtual void publish_bow_anchor(const UnitAnchorPoint& point) = 0;
    virtual void publish_stern_anchor(const UnitAnchorPoint& point) = 0;
    // 0092BE80 on the controller, then 0081C050 on the unit, step 6.
    virtual void sub_update_0092be80(float scaled_delta) = 0;
    virtual void sub_update_0081c050() = 0;
    // 008259EB: whether this instance is DAT_00E188D8.
    virtual bool is_controlled_unit() = 0;
    // DAT_00F87152, the global intensity override byte.
    virtual bool global_intensity_override() = 0;
    // 008227E0 on the smoothing state at +10A0h.
    virtual void smooth_intensity_008227e0(float scaled_delta) = 0;
    // 00424C40()+680h, the settings byte that enables the wake spawn.
    virtual bool wake_enabled() = 0;
    // 00C31F90 / 00C32000 / 00C33650 / 00C31FC0 then 00935540, step 8.
    virtual void spawn_wake() = 0;
    // 00CE69D0 compared against +100h, then 00B6DA70 on +1170h, step 9.
    virtual bool prop_wash_threshold_passed(float pose_base) = 0;
    virtual void stop_prop_wash() = 0;
    // 004142E0 then 008689C0 and the ref-count pair, step 10.
    virtual void update_attachment(std::size_t slot, const UnitAnchorPoint& local_point) = 0;
    // The three timed sub-updates of step 11, in order.
    virtual void sub_update_008252c0(float scaled_delta) = 0;
    virtual void sub_update_00956600(float scaled_delta) = 0;
    virtual void sub_update_00834e90(float scaled_delta) = 0;
    // 00815370 on each part, with the scalar 006FF270 returned, step 12.
    virtual void tick_part(std::size_t index, float intensity) = 0;
};

// 008255B0, __thiscall(this, float), RET 4. Runs the twelve steps of
// docs/UNIT_INSTANCE_UPDATE.md in the native order. The state is a new C++
// interface, not an original-layout object or drop-in ABI replacement.
void update_unit_instance_008255b0(UnitInstanceState& unit, const UnitClassBlock& class_block,
                                   UnitInstanceHost& host, float scaled_delta);

// ---------------------------------------------------------------------------
// The world pass that drives it.
// ---------------------------------------------------------------------------

struct WorldEntityUpdateHost {
    virtual ~WorldEntityUpdateHost() = default;
    // entity->vtable[0DCh](scaledDelta) for one child of the world node.
    virtual void update_entity(std::size_t index, float scaled_delta) = 0;
    // 00904600, the timed-attachment pass that closes the world update.
    virtual void update_timed_attachments_00904600(float scaled_delta) = 0;
};

// 00904BF0, __thiscall(world, float), RET 4. `active` is the byte at entity+5Ch
// for each child, in child-chain order (head [world+4], link entity+38h).
std::size_t update_world_entities_00904bf0(const std::vector<bool>& active,
                                           WorldEntityUpdateHost& host, float scaled_delta);

// ---------------------------------------------------------------------------
// Controlled-unit bind, 004C0890 and its inlined copy at 004E4A80.
// ---------------------------------------------------------------------------

// Trait ids 004C0890 queries. They are IsKindOf arguments, not vtable slots.
inline constexpr int kUnitTraitDirectlyControlled = 0x05;
inline constexpr int kUnitTraitHasDrivenSubUnit = 0x18;
inline constexpr int kUnitTraitPublishesAnchor = 0x0F;

struct ControlledUnitQuery {
    virtual ~ControlledUnitQuery() = default;
    virtual bool unit_is_kind_of(int class_id) = 0;      // vtable +5Ch on the unit
    virtual bool has_driven_sub_unit() = 0;              // [unit+3D0h] != 0
    virtual bool sub_unit_is_kind_of(int class_id) = 0;  // vtable +5Ch on [unit+3D0h]
};

// Which object 004C0890 resolves to and whether it publishes an anchor.
struct ControlledUnitBind {
    bool has_target{false};
    bool target_is_sub_unit{false};
    bool publishes_anchor{false};
};

ControlledUnitBind resolve_controlled_unit_004c0890(bool unit_present, ControlledUnitQuery& query);

} // namespace bsp
