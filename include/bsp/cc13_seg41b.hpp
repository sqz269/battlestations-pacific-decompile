#pragma once

// cc13_seg41b: the segment 41 / 33 functions the cc13-seg41 and cc13-seg33b packets skipped
// (docs/CC13_SEG41B.md). New C++ interfaces over the image's Win32 layouts; nothing here is
// wired into the game. Names are hypotheses, not recovered symbols. Every call the image makes
// is either a direct call to an existing reconstruction or a named host hook invoked at the
// same point with the same arguments.
//
// x87 model (as bsp/band_0067_cc13.hpp): the image runs with 53-bit precision, so an FSTP to a
// float slot is a float cast and unspilled x87 intermediates are doubles.

#include <cstddef>
#include <cstdint>

#include "bsp/band_0067_cc13.hpp"  // Vec3, Plane, 0073E780, 0073E7C0
#include "bsp/cc13_seg33b.hpp"     // TaggedValue
#include "bsp/cc13_seg41.hpp"      // RefCounted, RefCountedHost
#include "bsp/cc13_seg41_land.hpp" // NativeStringHost

namespace bsp::cc13_seg41b {

using Object = std::uint8_t*;
using band_0067_cc13::Vec3;
using cc13_seg33b::TaggedValue;
using cc13_seg41::RefCounted;

// A 14h-byte LuaObject / LuaReader as the image keeps it on the stack; opaque here.
struct LuaSlot {
    alignas(4) std::uint8_t bytes[0x14];
};

// ---------------------------------------------------------------------------
// 00742C10 BSP_LandConvoy_CacheParentAndPlacementLaw, __thiscall(ECX convoy), RET 0 (the
// convoy's post-load init; body 00742C10-0074305C).
// ---------------------------------------------------------------------------
struct LandConvoyInitHost : cc13_seg41::NativeStringHost {
    // 0077FAD0 BSP_NetEntity_ReadReplicationState(ECX = convoy).
    virtual void net_entity_read_replication_state_0077fad0(Object convoy) = 0;
    // 007ACA30 BSP_PathObject_GetEntity(ECX = path object) -> EAX (+14h, 0 for a null object).
    virtual Object path_object_get_entity_007aca30(Object path_object) = 0;
    // 00927B40 BSP_MissionEntity_LuaSelfObject(ECX = entity)(LuaObject* out), RET 4 -> EAX = out.
    virtual Object entity_lua_self_object_00927b40(Object entity, LuaSlot* out) = 0;
    // 00B67580 (ECX = object)(NativeStr* key), RET 4: object[key] = {}.
    virtual void lua_set_new_table_00b67580(Object object, const std::int32_t* key) = 0;
    // 00B67800 (ECX = object)(LuaObject* out, const char* name), RET 8 -> EAX = out.
    virtual Object lua_get_by_name_00b67800(Object object, LuaSlot* out, const char* name) = 0;
    // 00B664E0 (ECX = table)(int index), RET 4: table[index] = nil.
    virtual void lua_set_nil_at_index_00b664e0(Object table, std::int32_t index) = 0;
    // 00B666C0 (ECX = table)(int index, LuaObject* value), RET 8: table[index] = value.
    virtual void lua_set_object_at_index_00b666c0(Object table, std::int32_t index, Object value) = 0;
    // 00B66790 (ECX = object)(const char* key, const char* value), RET 8.
    virtual void lua_set_cstring_00b66790(Object object, const char* key, const char* value) = 0;
    // 00B675D0 (ECX = object)(NativeStr* key, LuaObject* value), RET 8.
    virtual void lua_set_object_00b675d0(Object object, const std::int32_t* key, Object value) = 0;
    // 00B67700, the LuaObject destructor (ECX = object).
    virtual void lua_object_destruct_00b67700(Object object) = 0;
    // 00BF6713, the checked-iterator failure.
    virtual void invalid_iterator_00bf6713() = 0;
    // [parent+3D0h]->vtable[40h]() -> EAX, ECX = [parent+3D0h], no stack arguments.
    virtual std::uint32_t parent_object_vfunc_40(Object object) = 0;
    // 00B6E680 BSP_Node_SetParent(ECX = node)(parent), RET 4.
    virtual void node_set_parent_00b6e680(Object node, std::uint32_t parent) = 0;
    // path entity vtable[5Ch](1), ECX = entity, RET 4 -> AL.
    virtual bool path_entity_vfunc_5c(Object entity, std::uint32_t argument) = 0;
    // 00742400 BSP_LandConvoy_PlaceMembers(ECX = convoy).
    virtual void land_convoy_place_members_00742400(Object convoy) = 0;
    // (convoy+310h)->vtable[0Ch](), ECX = convoy+310h.
    virtual void sub_object_vfunc_0c(Object sub_object) = 0;
    // convoy vtable[148h](a, b), RET 8.
    virtual void entity_vfunc_148(Object convoy, std::uint32_t a, std::uint32_t b) = 0;
};
void land_convoy_post_load_init_00742c10(Object convoy, LandConvoyInitHost& host);

// ---------------------------------------------------------------------------
// 00748CC0 BSP_LandFort_LoadSavedState, __thiscall(ECX fort), RET 0. The read side of
// 007455F0 (cc13_seg41_land.hpp): "_landFort" { "gazdi" { "ID" -> +738h }, "section" -> +73Ch
// as the ADDRESS of element [section] of the master's +344h pointer vector (0 for -1) },
// then "gameUnit" visited by the weapon director +74Ch, then the repair-zone slot +72Ch.
// ---------------------------------------------------------------------------
// read_10 (from SaveReaderHost) is the reader's own vtable[10h], reached through
// cc13_seg41's save_reader_read_observer_ref_007453c0.
struct LandFortLoadHost : cc13_seg41::SaveReaderHost {
    // 0095E5B0 BSP_Unit_ReadCommonLuaProperties(ECX = fort).
    virtual void unit_read_common_lua_properties_0095e5b0(Object fort) = 0;
    // 009238A0 SEntity_GetSavedEntityLuaData(ECX = entity)(LuaObject* out), RET 4.
    virtual void saved_entity_lua_data_009238a0(Object entity, LuaSlot* out) = 0;
    // 004425C0 BSP_LuaReader_Construct(ECX = reader)(LuaObject by value, 14h), RET 14h.
    virtual void lua_reader_construct_004425c0(LuaSlot* reader, LuaSlot* lua_object_by_value) = 0;
    // 00BD8E20 EnterKey(key) RET 8; 00BD5EB0 HasKey(key) RET 8 -> AL; 00BD6830 ReadField(key,
    // value) RET 10h; 00BD7A20 LeaveKey(); 00441A20 the reader destructor. ECX = reader.
    virtual void lua_reader_enter_00bd8e20(LuaSlot* reader, TaggedValue key) = 0;
    virtual bool lua_reader_has_key_00bd5eb0(LuaSlot* reader, TaggedValue key) = 0;
    virtual void lua_reader_read_00bd6830(LuaSlot* reader, TaggedValue key, TaggedValue value) = 0;
    virtual void lua_reader_leave_00bd7a20(LuaSlot* reader) = 0;
    virtual void lua_reader_destruct_00441a20(LuaSlot* reader) = 0;
    // The reader's own vtable[4](key) RET 8 and vtable[8](), ECX = reader.
    virtual void reader_vfunc_4(LuaSlot* reader, TaggedValue key) = 0;
    virtual void reader_vfunc_8(LuaSlot* reader) = 0;
    // 00836510 BSP_WeaponDirector_VisitEnables(ECX = director)(reader), RET 4.
    virtual void weapon_director_visit_enables_00836510(Object director, LuaSlot* reader) = 0;
    // 0043F8E0 std::vector<T*>::operator[] (ECX = vector)(index), RET 4 -> EAX = &begin[index],
    // 00BF6713 when the vector is empty or index >= size (vendor STL, bound not rebuilt).
    virtual std::uint32_t* pointer_vector_at_0043f8e0(Object vector, std::uint32_t index) = 0;
    // 00809BC0 BSP_UnitRepairZoneSlot_Load(ECX = slot)(unit, a, b), RET 0Ch.
    virtual void repair_zone_slot_load_00809bc0(Object slot, Object unit, std::uint32_t a, std::uint32_t b) = 0;
};
void land_fort_load_saved_state_00748cc0(Object fort, LandFortLoadHost& host);

// ---------------------------------------------------------------------------
// 0074D800 BSP_LandVehicle_InitAll, __thiscall(ECX vehicle), RET 0 (MLandVehicle vtable
// 00CFFDE0 slot +A0h).
// ---------------------------------------------------------------------------
struct LandVehicleInitHost : cc13_seg41::RefCountedHost {
    // 00955420 BSP_UnitInstance_InitializeSceneBindings(ECX = vehicle).
    virtual void unit_instance_init_scene_bindings_00955420(Object vehicle) = 0;
    // [00E188A8], the game-session object (+1FE4h is read).
    virtual Object game_session_00e188a8() = 0;
    // 00BF681B operator new(bytes), cdecl.
    virtual Object operator_new_00bf681b(std::uint32_t bytes) = 0;
    // 00864580 BSP_UnitGunneryAi_Construct(ECX = memory) -> EAX.
    virtual Object gunnery_ai_construct_00864580(Object memory) = 0;
    // gunnery vtable[4](vehicle+310h), RET 4, ECX = gunnery.
    virtual void gunnery_ai_vfunc_4(Object gunnery, Object vehicle_310) = 0;
    // [00CE221C] InterlockedIncrement(&object->refs_04).
    virtual long interlocked_increment(volatile long* counter) = 0;
    // 008687C0 BSP_PointEffect_CreateWithParentMatrix, __fastcall(ECX out slot, EDX parent,
    // definition by value (callee owns the reference), matrix, a, b, c), RET 14h -> EAX = out.
    virtual RefCounted** effect_create_matrix_008687c0(RefCounted** out, Object parent, RefCounted* definition,
                                                       const float* matrix, std::uint32_t a, std::uint32_t b,
                                                       std::uint32_t c) = 0;
    // 0074D780 append to the live-effect reference array (ECX = vehicle+72Ch)(cell), RET 4;
    // reconstructed as bsp::append_live_effect_reference_0074d780 (live_effect_insertion.hpp).
    virtual void live_effect_append_0074d780(Object array_72c, RefCounted* const* source_cell) = 0;
};
void land_vehicle_init_all_0074d800(Object vehicle, LandVehicleInitHost& host);

// ---------------------------------------------------------------------------
// Mesh-decal triangle collection (0073F6E0, 0073EBA0): unreferenced in this image.
// ---------------------------------------------------------------------------
struct MeshDecalHost {
    virtual ~MeshDecalHost() = default;
    // 004F9B30 BSP_Vector3f_Cross, __fastcall(ECX out, EDX a)(b), RET 4: out = a x b.
    virtual Vec3 cross_004f9b30(const Vec3& a, const Vec3& b) = 0;
    // 00419440 BSP_Vector3f_Length(ECX v) -> ST0 (every caller here FSTPs it to a float).
    virtual float length_00419440(const Vec3& v) = 0;
    // 00419510 BSP_Vector3f_Normalize, __fastcall(ECX out, EDX in) -> EAX = out.
    virtual Vec3 normalize_00419510(const Vec3& v) = 0;
    // 004142E0 BSP_Vector3f_TransformAffinePoint(ECX point)(out, matrix), RET 8.
    virtual Vec3 transform_point_004142e0(const Vec3& point, const void* matrix) = 0;
    // 004768D0 / 0070FDB0 (ECX = logical vertex stream)(out float3, index), RET 8 -> EAX = out.
    virtual Vec3 vertex_stream_read_position_004768d0(Object stream, std::uint32_t index) = 0;
    virtual Vec3 vertex_stream_read_normal_0070fdb0(Object stream, std::uint32_t index) = 0;
};

// The 7Ch triangle record: p0 +0, p1 +0Ch, p2 +18h, vertex normals +24h/+30h/+3Ch, face
// normal +70h (written by 0073EBA0). Only these fields are touched here.
struct DecalTriangle {
    Vec3 position[3];      // +00h
    Vec3 normal[3];        // +24h
    std::uint8_t gap_48[0x28];
    Vec3 face_normal;      // +70h
};
static_assert(sizeof(DecalTriangle) == 0x7C, "7Ch record");

// 0073EBA0 BSP_MeshDecal_ClipTriangleToBox(ECX unused)(tri, matrix, float max_distance,
// float* out_normal), RET 10h -> AL.
bool mesh_decal_clip_triangle_to_box_0073eba0(DecalTriangle* tri, const void* matrix, float max_distance,
                                              float* out_normal, MeshDecalHost& host);

// 0073F6E0 BSP_MeshDecal_CollectTreeTriangles, __thiscall(ECX ctx)(node, query), RET 8. ctx is
// only passed on as 0073EBA0's (unused) ECX and to the recursive calls.
void mesh_decal_collect_tree_triangles_0073f6e0(void* ctx, Object node, Object query, MeshDecalHost& host);

// 0073F1B0 BSP_Triangle_SphereContactTest, __stdcall(p0, p1, p2, centre, float radius, float*
// out_normal), RET 18h -> AL; ECX is not read. Unreferenced in this image. Note the image writes
// out_normal (the face normal) after the edge tests and BEFORE the corner tests, so a triangle
// rejected by a corner plane still leaves its normal in out_normal.
bool triangle_sphere_contact_0073f1b0(const Vec3* p0, const Vec3* p1, const Vec3* p2, const Vec3* centre,
                                      float radius, float* out_normal, MeshDecalHost& host);

// ---------------------------------------------------------------------------
// 006F3CC0 BSP_CommandBuilding_DebugDrawId, __thiscall(ECX building), RET 0. Debug overlay:
// within 3500 units of the camera, a wire sphere and a 50-unit flat-forward arrow per garrison
// member (+794h vector), then "ID: %d" (+174h) at the building's screen position.
// ---------------------------------------------------------------------------
// The function-local statics 00E19A38..00E19A5F (zero-initialised .bss in the image).
struct DebugIdColourStatics {
    float arrow_rgba_00e19a38[4];        // {1, 1, 0, 1} on first use
    std::uint32_t arrow_guard_00e19a48;  // bit 0
    float sphere_rgba_00e19a4c[4];       // {1, 0.5, 0, 1} on first use
    std::uint32_t sphere_guard_00e19a5c; // bit 0
};
struct CommandBuildingDebugIdHost {
    virtual ~CommandBuildingDebugIdHost() = default;
    // 0051F460 BSP_DebugFeatureOwner_GetSingleton() -> EAX (+2Ch is the overlay flag byte).
    virtual Object debug_feature_owner_0051f460() = 0;
    // [00E188A8], the game-session object (+19FCh is the camera node).
    virtual Object game_session_00e188a8() = 0;
    // 00B6DB70 BSP_Transform_RefreshWorldMatrix(ECX = node).
    virtual void transform_refresh_world_matrix_00b6db70(Object node) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld(ECX = entity).
    virtual void entity_pose_refresh_world_00414db0(Object entity) = 0;
    // 006AC260 (ECX = entity)(float3* out), RET 4: the normalised (forward.x, 0, forward.z).
    virtual Vec3 entity_flat_forward_006ac260(Object entity) = 0;
    // 00860BE0 BSP_DebugDraw_WireSphereMarker, __fastcall(ECX centre, EDX rgba)(float radius,
    // rgba2, detailed), RET 0Ch.
    virtual void debug_wire_sphere_00860be0(const Vec3* centre, const float* rgba, float radius,
                                            const float* rgba2, std::uint32_t detailed) = 0;
    // 0085FFF0 BSP_DebugDraw_ArrowRgbaFloatsOneColour, __fastcall(ECX from, EDX to)(rgba by
    // value, 10h), RET 10h.
    virtual void debug_arrow_0085fff0(const Vec3* from, const Vec3* to, const float rgba[4]) = 0;
    // 0043A660 BSP_Camera_ProjectWorldToScreen, __fastcall(ECX world, EDX out)(mode, clamp),
    // RET 8 -> EAX (reconstructed as camera_project_world_to_screen_0043a660).
    virtual std::int32_t camera_project_world_to_screen_0043a660(const float* world, float* out, std::int32_t mode,
                                                                 std::int32_t clamp) = 0;
    // 004C7070 BSP_DiagnosticPrintf, cdecl(session, float x, float y, format, ...).
    virtual void diagnostic_printf_004c7070(Object session, float x, float y, const char* format,
                                            std::uint32_t value) = 0;
};
void command_building_debug_draw_id_006f3cc0(Object building, DebugIdColourStatics& statics,
                                             CommandBuildingDebugIdHost& host);

}  // namespace bsp::cc13_seg41b
