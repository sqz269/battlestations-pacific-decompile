#pragma once
// Land units and structures: the six class layouts, the convoy that is the land
// units' motion controller, and the shipyard launch sentinel.
// docs/LAND_AND_STRUCTURES.md.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The scene
// key spellings below ("Rows", "Columns", "RowGap", "ColumnGap", "Speed",
// "Reverse", "Offset", "HP", "Path") are recovered strings read from the image
// at the addresses noted; the C++ names around them are not.
//
// Nothing here is a drop-in binary replacement. The native entities are opaque
// handles behind LandConvoyPlacementHost, and the records are projections of the
// fields this packet read, not full native layouts.
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// The six classes (docs/LAND_AND_STRUCTURES.md section 1)
// ---------------------------------------------------------------------------
// None of these is the 0x1188 ship instance of docs/UNIT_INSTANCE_LAYOUT.md.
// Five of the six stop at the level-4 base (0095CC90, class id 05), whose own
// storage ends at 0x72C; LandConvoy stops at level 2 (0077EED0), which ends at
// 0x310. An offset above the size below does not exist on that class.
struct LandClassLayout
{
    std::uint32_t class_id;      // instance+C4h, written by the leaf constructor
    std::uint32_t parent_class_id;  // the level the leaf constructor calls
    std::uint32_t allocator;      // descriptor->vtable[28h]
    std::uint32_t leaf_constructor;
    std::uint32_t main_vtable;    // instance+0h
    std::uint32_t element_vtable;  // instance+310h
    std::size_t instance_size;    // the operator new argument in the allocator
    std::size_t own_fields_begin;  // first offset the leaf constructor writes
    int extra_vtable_slots;       // slots beyond the base table's count
};

inline constexpr std::array<LandClassLayout, 6> kLandClassLayouts{{
    // MLandVehicle: 0074DF10 operator new(740h) at 0074DF2A, leaf 0074DCC0,
    // +C4h = 19h at 0074DD29.
    {0x19, 0x05, 0x0074DF10, 0x0074DCC0, 0x00CFFDE0, 0x00CFFD9C, 0x740, 0x72C, 0},
    // LandConvoy: 004F2700 operator new(3CCh), leaf 004F2410, +C4h = 1Ah at
    // 004F24A9. Not a unit creator: no 009553D0 and no +354h descriptor write.
    {0x1A, 0x02, 0x004F2700, 0x004F2410, 0x00CEA570, 0x00CEA528, 0x3CC, 0x310, 0},
    // MLandFort: 00747000 operator new(758h), leaf 00745940, +C4h = 1Bh at
    // 007459CB.
    {0x1B, 0x05, 0x00747000, 0x00745940, 0x00CFF3F8, 0x00CFF3B4, 0x758, 0x72C, 3},
    // MCommandBuilding: 006F5C10 operator new(7E8h), leaf 006F5610 which calls
    // the fort leaf 00745940 at 006F5635; +C4h = 1Ch at 006F5723.
    {0x1C, 0x1B, 0x006F5C10, 0x006F5610, 0x00CFB028, 0x00CFAFE0, 0x7E8, 0x758, 6},
    // MAirfield: 006D3110 operator new(8E4h), leaf 006D1C20, +C4h = 45h at
    // 006D1D04.
    {0x45, 0x05, 0x006D3110, 0x006D1C20, 0x00CF8C08, 0x00CF8BC0, 0x8E4, 0x72C, 2},
    // MShipyard: 00848380 operator new(7A4h) at 00848398, leaf 00848080,
    // +C4h = 46h at 00848131.
    {0x46, 0x05, 0x00848380, 0x00848080, 0x00D0B770, 0x00D0B728, 0x7A4, 0x770, 0},
}};

// Returns the layout for a class id, or nullptr when the id is not one of the
// six. Never returns a ship layout.
const LandClassLayout* land_class_layout(std::uint32_t class_id) noexcept;

// True when an offset is inside the class's instance. The reason this exists:
// unit+1130h (the shipyard-launch sentinel, docs/UNIT_MESSAGE_ARMS.md) is a ship
// offset and lies past every layout here, so a land class can never carry it.
bool land_class_offset_exists(std::uint32_t class_id, std::size_t offset) noexcept;

// ---------------------------------------------------------------------------
// The group owner at +738h
// ---------------------------------------------------------------------------
// The same field on MLandVehicle (the convoy) and on MLandFort (the master
// fort). docs/RECON_SLOT_LISTS.md already declares this offset from the consumer
// side as kReconLandVehicleConvoyBackPointerOffset (same value, consumer
// 00805680); this packet supplies the producer, the convoy roster pass 00743450.
inline constexpr std::size_t kLandGroupOwnerOffset = 0x738;
// The slot index inside the owner, written next to it at 00743450.
inline constexpr std::size_t kLandGroupSlotIndexOffset = 0x73C;

// The gun holder the gunnery pass reads through unit->vtable[140h]
// (docs/UNIT_GUNNERY_PASS.md, base 0047F320 = MOV EAX,ECX; RET).
//
// 0074CDA0, MLandVehicle: EAX = [ECX+738h]; RET. No fallback, so a land vehicle
// outside a convoy hands the gunnery pass a null holder.
constexpr std::uintptr_t land_vehicle_gun_holder_0074cda0(std::uintptr_t convoy) noexcept
{
    return convoy;
}

// 006F57A0, MLandFort and MCommandBuilding: EAX = [ECX+738h]; if (!EAX) EAX = ECX.
constexpr std::uintptr_t land_fort_gun_holder_006f57a0(std::uintptr_t master_fort,
                                                       std::uintptr_t self) noexcept
{
    return master_fort != 0 ? master_fort : self;
}

// ---------------------------------------------------------------------------
// The tick element's slot 10h (docs/TICK_ELEMENT_OVERRIDES.md correction)
// ---------------------------------------------------------------------------
// 00874DE0 BSP_FixedStepCallbackList_Run invokes node->vtable[10h](0.05f), so
// +10h is the fixed-step-callback entry of the element interface and not a dead
// slot. MCommandBuilding is the only class in the image that overrides it
// (00CFAFE0 slot +10h = 006F7360); every other element table carries the base
// no-op 0042BBA0. The registration is 00875A80(this+310h, repeating = 1) at
// 006F5767.
inline constexpr std::size_t kLandElementSlotFixedStepCallback = 0x10;
inline constexpr std::uint32_t kLandElementFixedStepNoOp = 0x0042BBA0;
inline constexpr std::uint32_t kLandCommandBuildingFixedStep = 0x006F7360;

// ---------------------------------------------------------------------------
// The shipyard launch sentinel (docs/LAND_AND_STRUCTURES.md section 3)
// ---------------------------------------------------------------------------
// 0082226F, inside BSP_UnitInstance_HandleMessage: NEG CL; SBB ECX,ECX;
// AND ECX,5; MOV [EDI+1130h],ECX. The field is ship-only and the reflection
// tables name it shipYardLaunch (00818653). Its readers -- 0081DE31, which
// returns from the ship motion body when it is positive, and the HUD control
// passes 0064B97D and 0067C736, which skip when it is non-zero -- make 5 a
// sentinel and not a countdown: nothing decrements it.
inline constexpr std::size_t kLandShipYardLaunchOffset = 0x1130;  // on the SHIP
inline constexpr int kLandShipYardLaunchActive = 5;
inline constexpr int kLandShipYardLaunchIdle = 0;

constexpr int ship_yard_launch_state_0082226f(bool launching) noexcept
{
    return launching ? kLandShipYardLaunchActive : kLandShipYardLaunchIdle;
}

// True when the ship suppresses player motion and HUD controls, i.e. the gate
// 0081DE31 / 0064B97D / 0067C736 agree on.
constexpr bool ship_yard_launch_suppresses_control(int state) noexcept
{
    return state > 0;
}

// ---------------------------------------------------------------------------
// The convoy: the land units' motion controller
// ---------------------------------------------------------------------------
// Field offsets on the 0x3CC LandConvoy instance. The scene keys are the
// recovered strings 00743450 looks up.
inline constexpr std::size_t kLandConvoyRowsOffset = 0x354;        // "Rows", 00CFF344
inline constexpr std::size_t kLandConvoyColumnsOffset = 0x358;     // "Columns"
inline constexpr std::size_t kLandConvoyRowGapOffset = 0x35C;      // "RowGap"
inline constexpr std::size_t kLandConvoyColumnGapOffset = 0x360;   // "ColumnGap"
inline constexpr std::size_t kLandConvoyAuthoredHpOffset = 0x364;  // "HP", 00CE6750
inline constexpr std::size_t kLandConvoySpeedOffset = 0x368;       // "Speed"
inline constexpr std::size_t kLandConvoyPathOffset = 0x344;        // "Path", 00CEA738
inline constexpr std::size_t kLandConvoyPathLengthOffset = 0x28;   // inside the path object
inline constexpr std::size_t kLandConvoyMembersBeginOffset = 0x398;
inline constexpr std::size_t kLandConvoyMembersEndOffset = 0x39C;
inline constexpr std::size_t kLandConvoyParentOffset = 0x3A4;      // cached at 00742C81
inline constexpr std::size_t kLandConvoyStoppedOffset = 0x3A8;     // byte
inline constexpr std::size_t kLandConvoyReverseOffset = 0x3A9;     // byte, "Reverse"
inline constexpr std::size_t kLandConvoyLiveArcOffset = 0x3AC;     // "Offset" seeds it
inline constexpr std::size_t kLandConvoyCommittedArcOffset = 0x3B0;
inline constexpr std::size_t kLandConvoyOdometerOffset = 0x3B4;
inline constexpr std::size_t kLandConvoyLiveMemberCountOffset = 0x3BC;
inline constexpr std::size_t kLandConvoyWorldPoseLawOffset = 0x3C8;  // byte

// The scene property the convoy resolves to its path, and the scene parent the
// terrain law samples through. 007420D6 reads "Path"; 00742C81 caches the parent.
inline constexpr char kLandConvoyPathKey[] = "Path";

// One convoy's authored shape. Rows ride the path; columns spread across it.
struct LandConvoyFormation
{
    int rows = 0;            // +354h
    int columns = 0;         // +358h
    float row_gap = 0.0f;    // +35Ch, along the path
    float column_gap = 0.0f;  // +360h, across it
    float speed = 0.0f;      // +368h, path units per second
    float path_length = 0.0f;  // [[convoy+344h]+28h]
    bool reverse = false;    // +3A9h
    bool stopped = false;    // +3A8h
};

// The direction scale 00743060 and 007410C0 pick: 00D7A260 = -1.0f when
// "Reverse" is set, 00D7A24C = +1.0f otherwise.
constexpr float land_convoy_direction_scale(bool reverse) noexcept
{
    return reverse ? -1.0f : 1.0f;
}

// The loop bound 00742400 computes with the IMUL at 00742458.
constexpr int land_convoy_slot_count(const LandConvoyFormation& f) noexcept
{
    return f.rows * f.columns;
}

// Element slot +4h, 00743060 (body 00743060-00743154, RET 4, this = convoy+310h).
// Advances the live arc and the odometer. Returns the values unchanged when the
// convoy is stopped, which is exactly what the +3A8h gate at 00743063 does.
struct LandConvoyArcStep
{
    float live_arc = 0.0f;
    float odometer = 0.0f;
};

LandConvoyArcStep land_convoy_advance_arc_00743060(const LandConvoyFormation& formation,
                                                   float live_arc,
                                                   float odometer,
                                                   float t) noexcept;

// Element slot +8h, 007410C0 (no Ghidra function, 007410C0..00741103, RET 4):
// re-derive the live arc from the committed one. This is the slot that discards
// the interpolation drift; note that for the convoy it is +8h and not +4h.
float land_convoy_restore_arc_007410c0(const LandConvoyFormation& formation,
                                       float committed_arc,
                                       float dt) noexcept;

// Element slot +0Ch, 007410B0 (no Ghidra function, 007410B0..007410BC, plain
// RET): commit the live arc. Included as a named rule because the pair only
// makes sense together.
constexpr float land_convoy_commit_arc_007410b0(float live_arc) noexcept
{
    return live_arc;
}

// 00742400's per-group arc. Unlike 00743060 this wraps with a single conditional
// (00742581-007425A9), so a row gap larger than the path length leaves the arc
// outside [0, length).
float land_convoy_group_arc_00742400(const LandConvoyFormation& formation,
                                     float live_arc,
                                     int group) noexcept;

// 00742400's lateral offset, 00742622-00742644: centred on the file count with
// the doubles 1.0 (00D7A210) and 0.5 (00D7A280).
float land_convoy_lateral_offset_00742400(const LandConvoyFormation& formation,
                                          int lane) noexcept;

// The flat member index split, 00742400: group = i / columns, lane = i % columns.
struct LandConvoySlot
{
    int group = 0;
    int lane = 0;
};

LandConvoySlot land_convoy_slot_00742400(const LandConvoyFormation& formation, int index) noexcept;

// The placement law 00742C10 chooses at 00742C70: true (the +3C8h byte set) when
// the path entity and the convoy have different scene parents, which forces the
// world-pose law because the path-space pose cannot be written into a local
// matrix under a different parent.
constexpr bool land_convoy_uses_world_pose_law_00742c70(std::uintptr_t path_entity_parent,
                                                        std::uintptr_t convoy_parent) noexcept
{
    return path_entity_parent != convoy_parent;
}

// 007422A0, reached from a member's vtable[080h] 0074CD10 with
// ECX = [member+738h]: the convoy is killed once its last member is gone.
constexpr bool land_convoy_dies_on_member_death_007422a0(int live_member_count_after) noexcept
{
    return live_member_count_after == 0;
}
inline constexpr int kLandConvoyKillReason = 2;  // BSP_MissionEntity_Kill(2)

// ---------------------------------------------------------------------------
// The placement pass, 00742400
// ---------------------------------------------------------------------------
// A 4x4 affine, row-major, as the native code lays it out at member+74h:
// row 0 right, row 1 up, row 2 forward, row 3 translation.
struct LandPoseMatrix
{
    std::array<float, 16> m{};

    float* row(int i) noexcept { return m.data() + 4 * i; }
    const float* row(int i) const noexcept { return m.data() + 4 * i; }
};

LandPoseMatrix land_pose_identity() noexcept;

struct LandVec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// One method per native call site in 00742400. Every entity is an opaque handle:
// this packet reconstructed the arithmetic, not the scene graph.
class LandConvoyPlacementHost
{
public:
    virtual ~LandConvoyPlacementHost() = default;

    // The member vector +398h/+39Ch. Handle 0 for an emptied slot, which
    // 007422A0 leaves behind.
    virtual int member_count() = 0;
    virtual std::uintptr_t member_at(int index) = 0;

    // The convoy's cached scene parent, +3A4h (00742C81). The terrain law
    // samples the height field through parent+3D0h.
    virtual std::uintptr_t convoy_parent() = 0;

    // 007B03C0 at 007425B9: sample the path at an arc length. Writes the
    // position into matrix row 3 and the tangent into row 2 natively; here it
    // returns both.
    virtual void sample_path_007b03c0(float arc, LandVec3* position, LandVec3* tangent) = 0;

    // 0085DC80 at 0074260E: complete an orthonormal frame from rows 1 and 2.
    virtual void orthonormalize_frame_0085dc80(LandPoseMatrix* matrix) = 0;

    // The global up-axis triple 00F8758C..00F87594, written elsewhere at run
    // time and therefore a host read, not a constant.
    virtual LandVec3 world_up_00f8758c() = 0;

    // 00414DB0 at 00742734: refresh the parent's world pose before it is read.
    virtual void refresh_world_pose_00414db0(std::uintptr_t entity) = 0;

    // 0087FA20 at 0074278D and 0087FB90 at 007427BC: the parent's height field,
    // both [parent+3D0h] forwarders. The sample is a world height; 00742792
    // subtracts the parent's own origin at parent+100h to land in parent space.
    virtual float terrain_height_0087fa20(std::uintptr_t parent, float x, float z) = 0;
    virtual float parent_height_origin_00742792(std::uintptr_t parent) = 0;
    virtual LandVec3 terrain_normal_0087fb90(std::uintptr_t parent, float x, float z) = 0;

    // 0085DAD0 at 007426BD and 007427E5: both laws write an up vector into
    // matrix row 1 and then rebuild the frame from it (length and cross, no
    // normalise). The world-pose law writes the literal (0, 1, 0) at 00742666-
    // 0074268B; the terrain law writes the sampled surface normal at 007427C1-
    // 007427DF.
    virtual void orthonormalize_from_normal_0085dad0(LandPoseMatrix* matrix,
                                                     const LandVec3& normal) = 0;

    // The terrain law's direct write of member+74h..+B0h (007427F0 onward),
    // followed by clearing +C8h/+10Ch and walking the +48h/+44h child chain with
    // 0042ED50 BSP_SceneNode_InvalidateSubtreePose.
    virtual void write_member_local_matrix(std::uintptr_t member, const LandPoseMatrix& matrix) = 0;
    virtual void invalidate_member_subtree_0042ed50(std::uintptr_t member) = 0;

    // The world-pose law's 00741E90 at 007426C9, the shared setter
    // 00929CB0 BSP_TickableGameEntity_PlaceStepPose also uses.
    virtual void set_member_world_pose_00741e90(std::uintptr_t member,
                                                const LandPoseMatrix& matrix) = 0;

    // [member+4A4h]->vtable[38h](member+74h) at 007426DD and 007428CD, run by
    // both laws.
    virtual void notify_member_scene_node(std::uintptr_t member) = 0;
};

struct LandConvoyPlacementResult
{
    int members_placed = 0;
    int slots_visited = 0;
    int groups_sampled = 0;     // one path sample per occupied group
    int terrain_law_placements = 0;
    int world_pose_law_placements = 0;
};

// 00742400 BSP_LandConvoy_PlaceMembers, __fastcall(convoy), body
// 00742400-00742927. Coverage: complete for the placement arithmetic and both
// laws; the out-of-range trap arm (LIBCRT_unmatched_00BF6713, taken when the
// member vector is shorter than rows * columns) is projected as "skip the slot"
// rather than reproduced, and the group-matrix cache is a local here because the
// native allocation and its free loop (00742900-00742925) carry no behaviour.
LandConvoyPlacementResult land_convoy_place_members_00742400(
    const LandConvoyFormation& formation,
    float live_arc,
    bool world_pose_law,
    LandConvoyPlacementHost& host);

}  // namespace bsp
