#pragma once

// cc13_x87c: three hard leftovers (packet cc13-x87c, docs/CC13_X87C.md). Every
// name is a hypothesis, not a recovered symbol. These are new C++ interfaces
// over the native records, not drop-in replacements of the original ABI.
// Nothing here is wired into the game.

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_seg30.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// 00421CE0 BSP_AvoidZone_TraceGridContours
// ---------------------------------------------------------------------------

// The 1Ch-byte object 00421CE0 allocates and 00421C10 constructs: the owner,
// a NativeString name ("unnamed"; length at +4h, data at +8h, released through
// the sized pool with length + 1) and a vector of 0Ch-byte points (proxy word at
// +0Ch, first/last/end at +10h/+14h/+18h). It is the same object
// Cc13AvoidZonePathNative (cc13_avoid1.hpp) views at +10h/+14h.
struct Cc13X87cContourPathNative {
    void* owner;                           // +00h, stored by 00421D7C
    std::uint32_t name_length;             // +04h
    char* name_data;                       // +08h
    std::uint32_t vector_proxy;            // +0Ch, ECX of 004215D0 points here
    float* first;                          // +10h
    float* last;                           // +14h
    float* end;                            // +18h
};
static_assert(sizeof(Cc13X87cContourPathNative) == 0x1C);

// The owner fields 00421CE0 touches: the entity pose block (local 4x4 at +74h,
// cached world at +CCh with valid byte +C8h, cached inverse at +110h with valid
// byte +10Ch, parent at +3Ch) and the grid source at +3D0h. Only these words
// are read or written; the gaps are unread here.
struct Cc13X87cContourOwnerNative {
    std::uint8_t unread_00[0x3C];
    void* parent;                          // +3Ch, an entity with its world at +CCh
    std::uint8_t unread_40[0x34];
    float local[16];                       // +74h
    std::uint8_t unread_b4[0x14];
    std::uint8_t world_valid;              // +C8h
    std::uint8_t unread_c9[3];
    float world[16];                       // +CCh
    std::uint8_t inverse_valid;            // +10Ch
    std::uint8_t unread_10d[3];
    float inverse[16];                     // +110h
    std::uint8_t unread_150[0x280];
    void* grid_source;                     // +3D0h, vtable slots +10h/+14h give the extents
};
static_assert(offsetof(Cc13X87cContourOwnerNative, parent) == 0x3C);
static_assert(offsetof(Cc13X87cContourOwnerNative, local) == 0x74);
static_assert(offsetof(Cc13X87cContourOwnerNative, world_valid) == 0xC8);
static_assert(offsetof(Cc13X87cContourOwnerNative, world) == 0xCC);
static_assert(offsetof(Cc13X87cContourOwnerNative, inverse_valid) == 0x10C);
static_assert(offsetof(Cc13X87cContourOwnerNative, inverse) == 0x110);
static_assert(offsetof(Cc13X87cContourOwnerNative, grid_source) == 0x3D0);

class Cc13X87cContourHost {
public:
    virtual ~Cc13X87cContourHost() = default;
    // [[owner+3D0h]]+10h / +14h, __thiscall(ECX = source), no stack arguments.
    virtual std::int32_t grid_source_extent_x_vslot10(void* source) = 0;
    virtual std::int32_t grid_source_extent_y_vslot14(void* source) = 0;
    // 00BF681B operator new(1Ch).
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    // 00421C10, __thiscall(ECX = block): zeroes the words and names it "unnamed".
    virtual Cc13X87cContourPathNative* construct_path_00421c10(void* block) = 0;
    // 00BF65AC free: the unwind funclet 00C5E570 (state 0) and the two frees of
    // the empty-result path.
    virtual void free_00bf65ac(void* block) noexcept = 0;
    // 0087FC30 BSP_Landscape_ForwardLocalGridToWorldXZ, __thiscall(ECX = owner,
    // out, a, b), RET 0Ch; returns out.
    virtual float* forward_local_grid_to_world_xz_0087fc30(Cc13X87cContourOwnerNative& owner,
                                                           float out[2], float a, float b) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld, __thiscall(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00413920 BSP_Matrix_Multiply4x4, __thiscall(ECX = left, out, right), RET 8;
    // returns out.
    virtual float* matrix_multiply4x4_00413920(const float* left, float out[16],
                                               const float* right) = 0;
    // 00B63D50 BSP_Matrix_BuildOrthogonalScaledAffineInverse, ECX = out, EDX = source.
    virtual void matrix_affine_inverse_00b63d50(float* out, const float* source) = 0;
    // 004215D0 MSVC_Vector12_PushBack, __thiscall(ECX = &path+0Ch, const point*), RET 4.
    virtual void vector12_push_back_004215d0(void* vector, const float point[3]) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton, no arguments.
    virtual void* sized_pool_singleton_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(ECX = pool, block, size, 1), RET 0Ch.
    virtual void sized_pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size,
                                                  std::int32_t flag) = 0;
};

// 00421CE0, __stdcall(owner, grid, step), RET 0Ch; EH frame (handler 00C5E57E).
// `grid` is an array of column pointers indexed grid[x][y] with signed bytes.
// Finds the first set cell (y outer, x inner), traces its eight-neighbour
// boundary appending one world point per direction change, marks each visited
// cell with 40h + 8 * entry direction + exit direction, then clears the traced
// region scanline by scanline. Returns the path, or null (path freed) when it
// has fewer than three points.
Cc13X87cContourPathNative* cc13_x87c_avoid_zone_trace_grid_contour_00421ce0(
    Cc13X87cContourOwnerNative& owner, std::int8_t* const* grid, std::int32_t step,
    Cc13X87cContourHost& host);

// ---------------------------------------------------------------------------
// 006C19D0 BSP_AirOpsBlock_DeclareLuaProperties
// ---------------------------------------------------------------------------

// Hooks for 006C19D0. The archive is the property visitor of cc13_seg30.hpp
// (vtable +4h open section, +8h close section, +0Ch write key/value; ECX =
// archive, each ArchiveArg passed by value as {kind, value}).
class Cc13X87cAirOpsDeclareHost {
public:
    virtual ~Cc13X87cAirOpsDeclareHost() = default;
    virtual void archive_open_04(void* archive, cc13_seg30::ArchiveArg key) = 0;
    virtual void archive_close_08(void* archive) = 0;
    virtual void archive_write_0c(void* archive, cc13_seg30::ArchiveArg key,
                                  cc13_seg30::ArchiveArg value) = 0;
    // The lock object behind block+4h/+8h/+0Ch: a CRITICAL_SECTION with a depth
    // counter at +18h. Enter = EnterCriticalSection ([00CE2218]) then ++[+18h];
    // leave = --[+18h] then LeaveCriticalSection ([00CE2210]). The leave is the
    // same sequence the guard destructor 00411F00 (vtable 00CE37FC) runs on unwind.
    virtual void lock_enter(void* lock) = 0;
    virtual void lock_leave(void* lock) = 0;
    // 00BF6713, the checked-iterator report; must not return.
    virtual void invalid_parameter_00bf6713() = 0;
    // 0095B9C0 BSP_VehicleClass_GetRegistryId, __thiscall(ECX = class).
    virtual std::uint32_t vehicle_class_registry_id_0095b9c0(void* vehicle_class) = 0;
    // 00BD5680 BSP_TaggedValue_Equals, __thiscall(ECX = &left, &right), RET 4.
    virtual bool tagged_value_equals_00bd5680(const cc13_seg30::ArchiveArg& left,
                                              const cc13_seg30::ArchiveArg& right) = 0;
    // [[block+3Ch]]+10h, __thiscall(ECX = traffic object, archive), RET 4.
    virtual void traffic_visit_vslot10(void* traffic, void* archive) = 0;
};

// 006C19D0, __thiscall(ECX = air-ops block, archive), RET 4; EH frame
// (handler 00C81178, three lock-guard states). Emits sections readySlots,
// landingSquadrons, landingPlanes, the scalar properties, slots, stock and
// traffic in the image's order.
void cc13_x87c_air_ops_declare_properties_006c19d0(void* block, void* archive,
                                                   Cc13X87cAirOpsDeclareHost& host);

// ---------------------------------------------------------------------------
// 00819CC0 BSP_ShipUnit_SetDamagedGfxLevel
// ---------------------------------------------------------------------------

// Hooks for 00819CC0. Objects are native pointers; the function writes the
// effect byte +9h and the unit pose words directly, as the image does.
class Cc13X87cShipDamageGfxHost {
public:
    virtual ~Cc13X87cShipDamageGfxHost() = default;
    // 00877B00 BSP_UnitTickableEntity_SetDamagedGfxLevel, __thiscall(unit, level), RET 4.
    virtual void set_damaged_gfx_level_base_00877b00(void* unit, std::int32_t level) = 0;
    // Part virtuals (ECX = part): +5Ch(1Eh) -> AL, +70h(1), +20h() -> scene node.
    virtual bool part_vslot5c(void* part, std::int32_t arg) = 0;
    virtual void part_vslot70(void* part, std::int32_t arg) = 0;
    virtual void* part_vslot20(void* part) = 0;
    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor, __thiscall(node, float, char), RET 8.
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float value,
                                                           bool recurse) = 0;
    // 00710B80 BSP_UnitPartInstance_DetachSpatialNode, ECX = the +20h result.
    virtual void unit_part_detach_spatial_node_00710b80(void* node) = 0;
    // 00867B10 BSP_PointEffect_StopChildren, ECX = effect.
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;
    // [00CE221C] InterlockedIncrement / [00CE2220] InterlockedDecrement on object+4h.
    virtual long interlocked_increment_00ce221c(long* value) = 0;
    virtual long interlocked_decrement_00ce2220(long* value) = 0;
    // The refcounted object's vtable slot 0, ECX = object, no stack arguments.
    virtual void refcounted_destroy_vslot00(void* object) = 0;
    // 00B6DFA0 BSP_Node_UnlinkAndRelease, ECX = node.
    virtual void node_unlink_release_00b6dfa0(void* node) = 0;
    // 00BD2FC0 BSP_RandomThreads_NextU32, ECX = 0 at both sites.
    virtual std::uint32_t random_next_u32_00bd2fc0(void* generator) = 0;
    // 008687C0 BSP_PointEffect_CreateWithParentMatrix, __fastcall(ECX = out,
    // EDX = parent node; handle by value (callee owns the reference), matrix, 1,
    // 0, 0), RET 14h; returns out.
    virtual void** point_effect_create_with_parent_matrix_008687c0(
        void** out, void* parent_node, void* handle, const float* matrix, std::int32_t a,
        std::int32_t b, std::int32_t c) = 0;
    // 00745130 BSP_LandFort_SetEffectAnchor, __thiscall(ECX = effect, anchor), RET 4.
    virtual void land_fort_set_effect_anchor_00745130(void* effect, const float* anchor) = 0;
    // The global word [00E188A8] (read twice by the image).
    virtual void* game_global_00e188a8() = 0;
    // 00414DB0 / 00413920, as in Cc13X87cContourHost.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    virtual float* matrix_multiply4x4_00413920(const float* left, float out[16],
                                               const float* right) = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint, __thiscall(ECX = point, out, matrix), RET 8.
    virtual void transform_affine_point_004142e0(const float* point, float out[3],
                                                 const float* matrix) = 0;
    // Unit vtable +34h, __thiscall(ECX = unit, out), RET 4; returns a float[3]
    // pointer (00812090 BSP_UnitInstance_GetVelocityVector in the ship vtables).
    virtual const float* unit_vslot34(void* unit, float out[3]) = 0;
    // 0049C940 BSP_PointAnnouncement_Create_Provisional, __thiscall(ECX = manager,
    // position, velocity, descriptor), RET 0Ch. The result is not used.
    virtual void* point_announcement_create_0049c940(void* manager, const float* position,
                                                     const float* velocity,
                                                     std::int32_t descriptor) = 0;
    // 00BF6713, the checked-iterator report; must not return.
    virtual void invalid_parameter_00bf6713() = 0;
};

// 00819CC0, __thiscall(ECX = unit, level), RET 4; EH frame (handler 00C90E2B,
// one state protecting the created effect). Slot +1B4h of the nine ship
// vtables (00CF90B0 battleship, 00CFA778 cargo, 00CFB738 cruiser, 00CFC3D0
// destroyer, 00CFFA30 landing ship, 00D01630 mothership, 00D09678 ship base,
// 00D0BF80, 00D0C648 torpedo boat), overriding 00877B00.
void cc13_x87c_ship_set_damaged_gfx_level_00819cc0(void* unit, std::int32_t level,
                                                   Cc13X87cShipDamageGfxHost& host);

}  // namespace bsp
