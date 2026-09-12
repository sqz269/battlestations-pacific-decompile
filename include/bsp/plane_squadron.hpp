// The plane squadron entity (scene class 18h, PlaneSquadronGen).
//
// Addresses: 007F2C60 (constructor), 007EF9D0 (destructor), 007F4580 (the
// spawn/attach override that creates the planes), 007F3820 (rename), 007F31A0
// (a plane left the map), 007F3A60 (control handover), 007F3500 (clone),
// 007F1D90 (avoid-zone layers), 007EFA70 (the vtable +98h thunk), 00D087C0
// (the vtable).
//
// Evidence: docs/PLANE_SQUADRON.md. Descriptive names are hypotheses, not
// recovered symbols.
//
// Contracts owned elsewhere and NOT redeclared here:
//   - the wave-3 tick 007F3BA0: include/bsp/tick_element_overrides.hpp
//     (SquadronTickState / SquadronTickHost / squadron_tick_advance_sim_007f3ba0).
//   - the plane array removal rule 007F3970 and the landing walk 008A20E0:
//     include/bsp/air_operations.hpp (kSquadronMaxPlanes,
//     kSquadronDirtyByteOffset, kSquadronLastPlaneFlagOffset,
//     kPlaneSquadronBackPointerOffset, squadron_remove_plane_007f3970).
//   - the scene creator 004F0AD0 and kPlaneSquadronInstanceSize:
//     include/bsp/scene_unit_creators.hpp.
//   - the class id 18h: kUnitClassPlaneSquadron in
//     include/bsp/local_player_unit_lists.hpp.
//   - the pilot bots that fly each plane (0099D300, 009998A0, 009F3F80), the
//     bot task vector (docs/BOT_TASKS.md), the session message router
//     (0077C7B0 / BSP_Session_RouteMessage) and the front-end manager: all
//     external, named at the call site only.
#ifndef BSP_PLANE_SQUADRON_HPP
#define BSP_PLANE_SQUADRON_HPP

#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// The 0x414-byte squadron object
// ---------------------------------------------------------------------------
// Every offset below is a site in 007F2C60 (the constructor), 007F4580 (the
// spawn override) or 007F3BA0 (the tick); docs/PLANE_SQUADRON.md carries the
// writer for each. Offsets below +310h belong to the shared entity base
// 0077EED0 and are not restated here.
struct PlaneSquadronOffsets {
    // Base fields this packet reads directly.
    static constexpr std::size_t kPrimaryVtable = 0x000;   // 00D087C0
    static constexpr std::size_t kWorldNode = 0x030;       // read at 007F4828
    static constexpr std::size_t kLocalMatrix = 0x074;     // read at 007F48C9
    static constexpr std::size_t kSpawnDescriptor = 0x0C0; // the 0Ch-byte record
    static constexpr std::size_t kClassId = 0x0C4;         // 18h, 007F2DEC
    static constexpr std::size_t kNameLength = 0x154;      // 004F0AD0, 007F3500
    static constexpr std::size_t kNamePointer = 0x158;

    // The tick registration node, group 3, vtable 00D0877C (00875890).
    static constexpr std::size_t kTickNode = 0x310;

    // Squadron fields proper.
    static constexpr std::size_t kAvoidZoneLayerSecondary = 0x34C; // 007F1D90
    static constexpr std::size_t kAvoidZoneLayerPrimary = 0x350;   // 007F1D90
    static constexpr std::size_t kConstantTwentyMinusOne = 0x354;  // 13h, 007F2DF6
    static constexpr std::size_t kRenameFlag = 0x358;              // 007F3820
    static constexpr std::size_t kPlaneClassDescriptor = 0x35C;    // 007F4783
    static constexpr std::size_t kFlag360 = 0x360;
    static constexpr std::size_t kExitZoneHandled = 0x361;         // 007F31A0
    static constexpr std::size_t kBehaviour = 0x364;               // 007F47D9
    static constexpr std::size_t kFlag368 = 0x368;
    static constexpr std::size_t kFlag369 = 0x369;                 // 007F3500 copies it
    static constexpr std::size_t kLastPlaneFlag = 0x36A;           // 007F3970
    static constexpr std::size_t kFloat36C = 0x36C;
    static constexpr std::size_t kCounter370 = 0x370;              // 1, cleared by 007F31A0
    static constexpr std::size_t kFlag378 = 0x378;                 // 1
    static constexpr std::size_t kFlag379 = 0x379;
    static constexpr std::size_t kTimerBlock = 0x37C;              // 007F2BD0 seeds it
    static constexpr std::size_t kTimerFrozenBytes = 0x38C;
    static constexpr std::size_t kAnyPlaneReady = 0x3B0;           // tick scratch
    static constexpr std::size_t kReadyPlaneValue = 0x3B4;
    static constexpr std::size_t kAnyPlaneReadyPrev = 0x3B8;
    static constexpr std::size_t kField3BC = 0x3BC;
    static constexpr std::size_t kPeriodicPeriod = 0x3C0;          // 1.0f
    static constexpr std::size_t kPeriodicCountdown = 0x3C4;       // -1.0f
    static constexpr std::size_t kWingCount = 0x3C8;               // 007F4778
    static constexpr std::size_t kPlaneCount = 0x3CC;              // 007F4B60
    static constexpr std::size_t kPlaneArray = 0x3D0;              // five pointers
    static constexpr std::size_t kField3E4 = 0x3E4;                // 1
    static constexpr std::size_t kMorale = 0x3E8;                  // 1.0f
    static constexpr std::size_t kDirtyByte = 0x3EC;               // 007F4B6E
    static constexpr std::size_t kEmbeddedSubobject = 0x3F0;       // vtable 00CF938C
    static constexpr std::size_t kSubobjectFlag400 = 0x400;        // 1
    static constexpr std::size_t kInstanceEnd = 0x414;

    // The plane side of the binding, written by 007F4580 only.
    static constexpr std::size_t kPlaneBackPointer = 0x9D4;  // 007F4B49
    static constexpr std::size_t kPlaneSpawnIndex = 0x9D8;   // 007F4B43
    static constexpr std::size_t kPlaneSpawnDescriptor = 0x0C0; // 007F491A
};

// The squadron vtable slots this packet identified in 00D087C0.
struct PlaneSquadronVtableSlots {
    static constexpr std::size_t kScalarDeletingDtor = 0x00; // 007F1140
    static constexpr std::size_t kSetName = 0x0C;            // 007F3820
    static constexpr std::size_t kIsKindOf = 0x5C;           // 007EFB00
    static constexpr std::size_t kControlHandover = 0x7C;    // 007F3A60
    static constexpr std::size_t kOnDestroyed = 0x80;        // 007F3B10
    static constexpr std::size_t kPlaceInWorld = 0x98;       // 007EFA70 -> 00928860
    static constexpr std::size_t kAttachLuaSelf = 0x9C;      // 007F4580
};

// 007F2C60 seeds these; everything else in the 0x414 bytes stays at the
// memset-0 the allocator site leaves. `kPlaneSquadronInstanceSize` in
// include/bsp/scene_unit_creators.hpp is the size.
struct PlaneSquadronConstructedState {
    std::int32_t class_id{0x18};          // +C4h, 007F2DEC
    std::int32_t constant_354h{0x13};     // +354h, 007F2DF6
    std::int32_t behaviour{-1};           // +364h, 007F2CF9
    std::int32_t counter_370h{1};         // +370h
    std::int32_t field_3e4h{1};           // +3E4h
    std::uint8_t flag_369h{1};            // +369h
    std::uint8_t flag_378h{1};            // +378h
    std::uint8_t subobject_flag_400h{1};  // +400h
    float periodic_period{1.0f};          // +3C0h, DAT_00D7A24C
    float periodic_countdown{-1.0f};      // +3C4h, FCHS at 007F2D60
    float morale{1.0f};                   // +3E8h, DAT_00D7A24C
    std::int32_t plane_count{0};          // +3CCh
    std::int32_t wing_count{0};           // +3C8h
    std::int32_t tick_group{3};           // 00875890(node, 3) at 007F2C9A
};
PlaneSquadronConstructedState plane_squadron_construct_007f2c60() noexcept;

// ---------------------------------------------------------------------------
// The spawn descriptor at squadron+C0h
// ---------------------------------------------------------------------------
// A 0Ch-byte record: +0h a vtable, +4h the kind, +8h the payload. 00922E20
// builds the kind-1 record from a property bag (the catapult path); 00922DE0
// builds the per-plane copy stored at plane+C0h. 007F4580 switches on +4h and
// 00928A00 treats kind 3 as "script owns the Lua table".
enum class SquadronSpawnKind : std::int32_t {
    kPropertyBag = 1,  // 007F471A: +8h is the bag; the only kind that spawns planes
    kSourceRecord = 2, // 007F46D1: +8h is a record with class/slope/behaviour
    kLuaTable = 3,     // 007F45D0: fields come from a _planeSquadron Lua table
};

// Property-bag keys 007F4580 reads, all recovered strings.
struct SquadronSpawnProperties {
    std::int32_t type_class_id{0};   // "Type" 00CE4780, site 007F471D
    bool wing_count_present{false};  // "WingCount" 00CF8840, site 007F4735
    std::int32_t wing_count_raw{0};  // site 007F4754
    bool parent_present{false};      // "PlaneParentID" 00CFACC8, site 007F4794
    std::int32_t parent_id{0};
    bool behaviour_present{false};   // "Behaviour" 00CFACBC, site 007F47C2
    std::int32_t behaviour{0};
};

// 007F4735..007F4772. The default when the key is absent is 3, not the " 1"
// the plane.props descriptor declares; a present value below 1 is raised to 1.
// The authored enum PlaneWingCount admits 1..5 and the array holds exactly
// five, so a value above 5 would run past kPlaneArray: the routine has no
// upper clamp.
inline constexpr std::int32_t kSquadronDefaultWingCount = 3;
std::int32_t squadron_resolve_wing_count_007f4747(const SquadronSpawnProperties& props) noexcept;
bool squadron_wing_count_fits_array_007f4b55(std::int32_t wing_count) noexcept;

// ---------------------------------------------------------------------------
// Plane naming, 007F4926 and 007F49EF
// ---------------------------------------------------------------------------
// A single-wing squadron names its one plane "<squadron>|"; a multi-wing
// squadron names wing i (0-based) "<squadron>|.-<i+1>". 007F3820 splits a
// plane name at the first '|' (_strcspn at 007F38A0) and re-prefixes it, which
// is what makes the separator load-bearing.
inline constexpr char kSquadronNameSeparator = '|';
extern const char kSquadronSingleWingSuffix[];  // 00CF100C, "|"
extern const char kSquadronMultiWingSuffix[];   // 00CF8038, "|.-"
// Writes the suffix (not the squadron name) for wing `index` of `wing_count`.
// Returns the number of characters written, or -1 when the buffer is too small.
int squadron_plane_name_suffix_007f4926(std::int32_t wing_count, std::int32_t index,
                                        char* out, std::size_t capacity) noexcept;

// ---------------------------------------------------------------------------
// Formation and leader rules
// ---------------------------------------------------------------------------
// The leader is the plane at array slot 0: 007F31A0's body tests `plane ==
// *(squadron+3D0h)` before it walks the tail from +3D4h. 007F3970 compacts the
// array towards 0, so the leader is whatever survives at slot 0 and the spawn
// index stamped at plane+9D8h is NOT rewritten by a compaction.
inline constexpr std::int32_t kSquadronLeaderSlot = 0;
bool squadron_plane_is_leader_007f31a0(std::int32_t slot) noexcept;

// 007F4813: a plane spawned under a parent entity (PlaneParentID resolved) is
// placed with an identity matrix relative to that parent; a plane spawned
// without one is placed under the squadron's own world node with the
// squadron's local matrix at +74h. There is no per-wing offset at spawn: the
// formation spacing is the pilot bot's, not the squadron's.
struct SquadronPlanePlacement {
    bool under_parent{false};       // 007F4817 TEST EBP,EBP
    bool identity_matrix{false};    // 007F481D..007F48AE
    bool use_squadron_local{false}; // 007F48C9 LEA ECX,[ESI+74h]
};
SquadronPlanePlacement squadron_plane_placement_007f4813(bool parent_resolved) noexcept;

// ---------------------------------------------------------------------------
// The host: one virtual method per native call site in 007F4580 mode 1
// ---------------------------------------------------------------------------
struct PlaneSquadronSpawnHost {
    virtual ~PlaneSquadronSpawnHost() = default;

    // 007F45A7 -> 0077E830, the base slot-39 attach this override calls first.
    // docs/MISSION_ENTITY_LUA_ATTACH.md owns its body.
    virtual void attach_lua_self_base_0077e830() = 0;

    // 007F4724 / 007F4742 / 007F4759 / 007F479D / 007F47C7 -> 008F2260
    // BSP_ScenePropertyBag_Find. Returns the four keys as one record because a
    // read of an absent key is a null return at every site.
    virtual SquadronSpawnProperties read_spawn_properties_008f2260() = 0;

    // 007F477E and 007F47E1 -> 007B8A80 BSP_VehicleClass_GetOrCreate. Called
    // twice with the same class id: once for the field at +35Ch, once for the
    // factory the loop drives.
    virtual std::uint32_t vehicle_class_for_type_007b8a80(std::int32_t type_class_id) = 0;

    // 007F47AE -> 00521E30: resolve PlaneParentID to a live entity, 0 when the
    // handle is stale.
    virtual std::uint32_t resolve_parent_entity_00521e30(std::int32_t parent_id) = 0;

    // 007F4811 -> vehicleClass->vtable[28h](0): allocate and construct one
    // plane unit instance. The plane's own class body is external.
    virtual std::uint32_t create_plane_instance(std::uint32_t vehicle_class) = 0;

    // 007F48BA and 007F48D2 -> plane->vtable[98h] (BSP_GameEntity_PlaceInWorld
    // 00928860 for the squadron's own class). `parent` is 0 on the no-parent
    // arm, where the matrix is the squadron's local one.
    virtual void place_plane_in_world(std::uint32_t plane, std::uint32_t parent,
                                      std::uint32_t world_node,
                                      const SquadronPlanePlacement& placement) = 0;

    // 007F48D6 operator new(0Ch) then 007F48FD -> 00922DE0: the plane's own
    // spawn descriptor, copied from the squadron's, stored at plane+C0h.
    virtual std::uint32_t clone_spawn_descriptor_00922de0(std::uint32_t squadron_descriptor) = 0;

    // 007F494B / 007F4A32 / 007F4A47 -> 004261A0 and 00742A70: build the plane
    // name from the squadron name and the suffix. Kept as one method because
    // the two arms differ only in the suffix.
    virtual void set_plane_name(std::uint32_t plane, const char* suffix,
                                std::int32_t one_based_index, bool multi_wing) = 0;
};

// One step of the spawn loop, in the order 007F4808..007F4B6E performs it.
struct SquadronSpawnStep {
    std::int32_t index{0};
    std::uint32_t plane{0};
    SquadronPlanePlacement placement{};
    bool multi_wing{false};
    std::int32_t stamped_spawn_index{0}; // plane+9D8h, 007F4B43
    std::int32_t array_slot{0};          // squadron+3D0h + slot*4, 007F4B55
    bool overflowed_array{false};        // slot >= kSquadronMaxPlanes
};

// The result of the whole mode-1 arm of 007F4580.
struct PlaneSquadronSpawnResult {
    SquadronSpawnKind kind{SquadronSpawnKind::kPropertyBag};
    std::int32_t wing_count{0};       // -> +3C8h
    std::int32_t plane_count{0};      // -> +3CCh
    std::uint32_t plane_class{0};     // -> +35Ch
    std::int32_t behaviour{-1};       // -> +364h
    std::uint32_t parent_entity{0};
    bool dirty{false};                // -> +3ECh
    std::int32_t step_count{0};
};

// 007F4580, mode 1 only. `steps` receives up to `max_steps` entries; the other
// two kinds spawn no planes and are reported by `kind` alone.
PlaneSquadronSpawnResult plane_squadron_spawn_planes_007f4580(
        SquadronSpawnKind kind, std::uint32_t squadron, std::uint32_t squadron_world_node,
        std::uint32_t squadron_spawn_descriptor, PlaneSquadronSpawnHost& host,
        SquadronSpawnStep* steps, std::int32_t max_steps);

}  // namespace bsp

#endif  // BSP_PLANE_SQUADRON_HPP
