#pragma once

// Segment 46 (007A49A0-007B2D80) reconstructions: Path / PathCursor / PathSource /
// ScenePath / CameraPath helpers, MovCam / MovieCam helpers, and MParatrooper
// residue. Packet argon-seg46; docs/ARGON_SEG46_PATH_MOVCAM.md.
//
// Reconstructed and build-tested C++ models read from Ghidra pseudocode and
// verified against x86/x87 assembly. Field names and C++ interfaces are
// hypotheses, not recovered symbols or drop-in binary ABI replacements.

#include <cstdint>
#include <vector>

namespace bsp::path_movcam_seg46_argon {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// ============================================================================
// 1. Vector3 & Point3PtrArray / Point3Vector Helpers
// ============================================================================

// 007AE330 BSP_Vector3_ComponentMin_007ae330
// __fastcall(out in ECX, a in EDX, const float* b on stack), RET 4.
// Lane-wise minimum (z, y, x order via FCOMIP + JBE -> b on <= or unordered).
Vec3& vector3_component_min_007ae330(Vec3& out, const Vec3& a, const Vec3& b) noexcept;

// 007AE3E0 BSP_Vector3_ComponentMax_007ae3e0
// __fastcall(out in ECX, a in EDX, const float* b on stack), RET 4.
// Lane-wise maximum (z, y, x order via FCOMIP + JBE -> b on >= or unordered).
Vec3& vector3_component_max_007ae3e0(Vec3& out, const Vec3& a, const Vec3& b) noexcept;

// Host interface for archive save/load of path points and sources.
struct PathArchiveHost {
    virtual ~PathArchiveHost() = default;
    // Archive vslot +04h(kind, section_name): open section.
    virtual void begin_section(int kind, const char* name) = 0;
    // Archive vslot +08h(): close section.
    virtual void end_section() = 0;
    // Archive vslot +0Ch: write tagged value (index/name key, type tag, payload).
    virtual void write_indexed_vec3(int key_kind, std::uint32_t index, int val_type, const Vec3& pt) = 0;
    virtual void write_indexed_bool(int key_kind, std::uint32_t index, int val_type, bool present) = 0;
    virtual void write_named_string(int key_kind, const char* key, int val_type, const char* val) = 0;
    virtual void write_named_u32(int key_kind, const char* key, int val_type, std::uint32_t val) = 0;
    // Archive vslot +10h: read tagged value.
    virtual void read_indexed_vec3(int key_kind, std::uint32_t index, int val_type, Vec3& out) = 0;
    virtual void read_named_u32(int key_kind, const char* key, int val_type, std::uint32_t& out) = 0;
    // Archive vslot +14h(1, index): returns true while element `index` exists.
    virtual bool has_indexed_entry(int key_kind, std::uint32_t index) = 0;
    // 00BF6713 CRT invalid parameter handler.
    virtual void on_invalid_parameter() = 0;
    // Entity save base hooks (00928450 for Path/PathEntity, 0077C160 for PathOwner).
    virtual void save_entity_base_00928450() = 0;
    virtual void save_net_entity_replication_0077c160() = 0;
    // 007AC9D0 BSP_Entity_PathInterfaceForKind(entity_ptr).
    virtual void* resolve_path_interface_007ac9d0(std::uint32_t entity_id) = 0;
};

// 007AE650 BSP_Point3Vector_SaveToArchive
// __thiscall(ECX = vector_owner, archive*), RET 4.
// Writes each 12-byte point at +04h..+08h via archive vslot +0Ch((1, i), (5, &pt[i])).
void point3_vector_save_to_archive_007ae650(const std::vector<Vec3>& points, PathArchiveHost& archive);

// 007AEEC0 BSP_Point3PtrArray_NearestPointToLine
// __thiscall(ECX = array, line_origin*, line_dir*, float* out_dist), RET 0Ch.
// Normalizes `line_dir` (0 if length <= 0), measures each point's distance to
// the line via 0046F440, and returns the index of the strictly smallest distance.
struct PointLineDistanceHost {
    virtual ~PointLineDistanceHost() = default;
    // 00419440 BSP_Vector3f_Length(ECX = v).
    virtual float vector3_length_00419440(const Vec3& v) = 0;
    // 0046F440 BSP_Geometry_PointLineDistance(ECX = point*, EDX = line_origin*, stack = unit_dir*).
    virtual float point_line_distance_0046f440(const Vec3& point, const Vec3& line_origin, const Vec3& unit_dir) = 0;
    // 00BF6713 CRT invalid parameter handler when array is empty.
    virtual void on_invalid_parameter() = 0;
};
std::uint32_t point3_ptr_array_nearest_point_to_line_007aeec0(
    const std::vector<const Vec3*>& points,
    const Vec3& line_origin,
    const Vec3& line_dir,
    float* out_distance,
    PointLineDistanceHost& host);

// 007AF020 BSP_Point3PtrArray_LoadFromArchive
// __thiscall(ECX = array, archive*), RET 4.
// While archive vslot +14h(1, i) returns non-zero, reads point i (type 5) via
// archive vslot +10h and appends it via array vslot +08h.
struct Point3PtrArrayAppendHost {
    virtual ~Point3PtrArrayAppendHost() = default;
    virtual void append_point_vslot08(const Vec3& pt) = 0;
};
void point3_ptr_array_load_from_archive_007af020(
    Point3PtrArrayAppendHost& array_self,
    PathArchiveHost& archive);

// 007AF090 BSP_Point3PtrArray_SaveToArchive
// __thiscall(ECX = array, archive*), RET 4.
// Note from assembly (007AF0CB-007AF0F8): tests `points[i] != nullptr` (`SETNZ CL`)
// and passes type 3 (bool) with `CL` to archive vslot +0Ch((1, i), (3, non_null)).
void point3_ptr_array_save_to_archive_007af090(
    const std::vector<const Vec3*>& points,
    PathArchiveHost& archive);

// 007AF110 BSP_PathComponent_SavePoints
// __thiscall(ECX = path_component, archive*), RET 4.
// Opens section "_pathBaseEntity" (archive vslot +04h), calls 007AF090, closes section (+08h).
void path_component_save_points_007af110(
    const std::vector<const Vec3*>& points,
    PathArchiveHost& archive);

// 007B0B00 BSP_PathComponent_SavePoints_00CE84E8
// __thiscall(ECX = path_component, archive*), RET 4.
// Byte-identical twin of 007AF110 for vtables 00CE84E8 and 00D0540C.
void path_component_save_points_007b0b00(
    const std::vector<const Vec3*>& points,
    PathArchiveHost& archive);

// 007B0B40 BSP_Path_SaveEntityAndPoints
// __thiscall(ECX = path_entity, archive*), RET 4.
// Calls 00928450(archive), then saves component at `entity+1E4h` under "_pathBaseEntity".
void path_save_entity_and_points_007b0b40(
    const std::vector<const Vec3*>& points_at_1e4,
    PathArchiveHost& archive);

// 007B0BA0 BSP_PathOwner_SaveEntityAndPoints_0077c160
// __thiscall(ECX = path_owner, archive*), RET 4.
// Calls 0077C160(archive), then saves component at `owner+310h` under "_pathBaseEntity".
void path_owner_save_entity_and_points_007b0ba0(
    const std::vector<const Vec3*>& points_at_310,
    PathArchiveHost& archive);

// 007B0D30 BSP_PathEntity_SaveEntityAndPoints_004e6660
// __thiscall(ECX = path_entity, archive*), RET 4.
// Calls 00928450(archive), then saves component at `entity+1E4h` under "_pathBaseEntity".
void path_entity_save_entity_and_points_007b0d30(
    const std::vector<const Vec3*>& points_at_1e4,
    PathArchiveHost& archive);

// 007B1B10 BSP_Point3PtrArray_AllocPoint
// __thiscall / cdecl(), RET 0: allocates a 12-byte Vec3 and zeroes x, y, z.
Vec3* point3_ptr_array_alloc_point_007b1b10();

// 007B1FB0 BSP_Point3Vector_PruneFromIndex
// __thiscall(ECX = point_vector, int start_index), RET 4.
// Clamps `start_index` to >= 0, then walks backward from `count - 1` down to
// `start_index + 1`. For each adjacent pair `(i - 1, i)`, computes 3D distance
// (squared sum <= 1e-10 -> 0.0f, else float(sqrt)). If distance < threshold
// (`[00CE7D78]`), erases element `i - 1` by shifting `[i .. end)` left by 12 bytes.
void point3_vector_prune_from_index_007b1fb0(
    std::vector<Vec3>& points,
    int start_index,
    float min_distance_threshold_ce7d78);

// ============================================================================
// 2. PathObject, PathPoints, ScenePath, and CameraPath World Transforms
// ============================================================================

struct PathComponentState {
    std::uint32_t vtable_00{0};
    std::vector<Vec3*> points_08{};
    std::uint32_t entity_14{0};
    bool knots_dirty_18{true};
    float cached_length_1c{0.0f};
    std::uint32_t extra_20{0};
};

// 007ACA30 BSP_PathObject_GetEntity
// __fastcall(ECX = path_component), RET 0.
// Returns `[ECX + 14h]`, or 0 when `ECX == 0`.
std::uint32_t path_object_get_entity_007aca30(const PathComponentState* path_obj) noexcept;

// 007B28C0 BSP_PathComponent_Construct
// __thiscall(ECX = path_component, entity), RET 4.
// Initializes vtable to 00D053D8, zeroes vector (+08h..+10h), sets entity (+14h),
// byte +18h = 1, float +1Ch = 0.0f, dword +20h = 0.
void path_component_construct_007b28c0(PathComponentState& comp, std::uint32_t entity) noexcept;

// 007B28F0 BSP_CameraPathComponent_Construct
// __thiscall(ECX = camera_path_component, entity), RET 4.
// Same initialization as 007B28C0 with vtable 00D0540C.
void camera_path_component_construct_007b28f0(PathComponentState& comp, std::uint32_t entity) noexcept;

// Host interface for entity pose and local/world transforms on ScenePath / CameraPath.
struct ScenePathTransformHost {
    virtual ~ScenePathTransformHost() = default;
    // Checks `entity+10Ch`; if 0, calls 00414DB0 (`BSP_EntityPose_RefreshWorld`),
    // sets `entity+10Ch = 1`, and calls 00B63D50 (`BSP_Matrix_BuildOrthogonalScaledAffineInverse`)
    // from `entity+CCh` into `entity+110h`.
    virtual void ensure_inverse_world_matrix_10c(std::uint32_t entity) = 0;
    // Checks `entity+C8h`; if 0, calls 00414DB0 (`BSP_EntityPose_RefreshWorld`).
    virtual void ensure_world_matrix_c8(std::uint32_t entity) = 0;
    // 00B62D10 BSP_Vector4f_Transform(out, vec4(pt, 1.0f), matrix).
    virtual void transform_vec4_inverse_110(std::uint32_t entity, const Vec3& world_pt, float out_h[4]) = 0;
    virtual void transform_vec4_world_cc(std::uint32_t entity, const Vec3& local_pt, float out_h[4]) = 0;
    virtual void transform_vec4_cached_74(std::uint32_t entity, const Vec3& local_pt, float out_h[4]) = 0;
    // 0042D0D0 BSP_Vector3f_TransformDirectionOptionalNormalize(out, dir, matrix, normalize=0).
    virtual Vec3 transform_direction_world_cc(std::uint32_t entity, const Vec3& local_dir) = 0;
    virtual Vec3 transform_direction_cached_74(std::uint32_t entity, const Vec3& local_dir) = 0;
    // PathComponent virtual slots (+04h set, +08h add, +0Ch insert).
    virtual void vslot04_set_point(int index, const Vec3& local_pt) = 0;
    virtual void vslot08_add_point(const Vec3& local_pt) = 0;
    virtual void vslot0c_insert_point(int index, const Vec3& local_pt) = 0;
    // Delegated local geometry / sampling calls.
    virtual Vec3 centroid_local_007ae6d0() = 0;
    virtual float distance_to_polyline_local_007ae7b0(const Vec3& local_query) = 0;
    virtual void sample_local_linear_007afac0(float u, Vec3& out_pos, Vec3* out_tan) = 0;
    virtual void sample_local_cubic_007afe80(float u, Vec3& out_pos, Vec3* out_tan, std::uint32_t mode) = 0;
    virtual void on_invalid_parameter() = 0;
};

// 007AD180 BSP_PathPoints_SetPointLocal
// __thiscall(ECX = path_component, int index, const Vec3* world_pt), RET 8.
// Refreshes inverse world matrix (+10Ch / +110h) if needed, transforms `world_pt`
// into local space with homogeneous divide `1.0f / w`, and calls vslot +04h(index, &local).
void path_points_set_point_local_007ad180(
    const PathComponentState& comp,
    int index,
    const Vec3& world_pt,
    ScenePathTransformHost& host);

// 007AD330 BSP_PathPoints_AddPointLocal
// __thiscall(ECX = path_component, const Vec3* world_pt), RET 4.
// Transforms `world_pt` into local space (+110h + homogeneous divide) and calls vslot +08h(&local).
void path_points_add_point_local_007ad330(
    const PathComponentState& comp,
    const Vec3& world_pt,
    ScenePathTransformHost& host);

// 007AD4E0 BSP_PathPoints_InsertPointLocal
// __thiscall(ECX = path_component, int index, const Vec3* world_pt), RET 8.
// Transforms `world_pt` into local space (+110h + homogeneous divide) and calls vslot +0Ch(index, &local).
void path_points_insert_point_local_007ad4e0(
    const PathComponentState& comp,
    int index,
    const Vec3& world_pt,
    ScenePathTransformHost& host);

// 007AF740 BSP_ScenePath_PointToWorldCached
// __thiscall(ECX = scene_path, Vec3* out, uint index), RET 8.
// Bounds-checks `index` against `points_08` (calls 00BF6713 on out-of-range),
// transforms `*points_08[index]` by `entity+74h` without pose refresh, divides by `w`.
void scene_path_point_to_world_cached_007af740(
    const PathComponentState& comp,
    Vec3& out_world,
    std::uint32_t index,
    ScenePathTransformHost& host);

// 007AFC60 BSP_CameraPath_SampleWorldLinearCached
// __thiscall(ECX = camera_path, float u, Vec3* out_pos, Vec3* out_tan), RET 0Ch.
// Calls 007AFAC0 (`BSP_CameraPath_SampleLocalLinear`), transforms `out_pos` by
// `entity+74h` with homogeneous divide, and if `out_tan != nullptr` transforms
// `out_tan` via 0042D0D0 (`entity+74h`, normalize=0).
void camera_path_sample_world_linear_cached_007afc60(
    const PathComponentState& comp,
    float u,
    Vec3& out_pos,
    Vec3* out_tan,
    ScenePathTransformHost& host);

// 007AFD60 BSP_CameraPath_SampleWorldLinear
// __thiscall(ECX = camera_path, float u, Vec3* out_pos, Vec3* out_tan), RET 0Ch.
// Calls 007AFAC0, refreshes `entity+C8h` if 0, transforms `out_pos` by `entity+CCh`
// with homogeneous divide, and if `out_tan != nullptr` refreshes `entity+C8h` again
// and transforms `out_tan` via 0042D0D0 (`entity+CCh`, normalize=0).
void camera_path_sample_world_linear_007afd60(
    const PathComponentState& comp,
    float u,
    Vec3& out_pos,
    Vec3* out_tan,
    ScenePathTransformHost& host);

// 007B03C0 BSP_CameraPath_SampleWorld
// __thiscall(ECX = camera_path, float u, Vec3* out_pos, Vec3* out_tan, uint32_t mode), RET 10h.
// Calls 007AFE80 (`BSP_CameraPath_SampleLocalCubic`), transforms `out_pos` by
// `entity+74h` with homogeneous divide, and if `out_tan != nullptr` transforms
// `out_tan` via 0042D0D0 (`entity+74h`, normalize=0).
void camera_path_sample_world_cached_cubic_007b03c0(
    const PathComponentState& comp,
    float u,
    Vec3& out_pos,
    Vec3* out_tan,
    std::uint32_t mode,
    ScenePathTransformHost& host);

// 007B0650 BSP_ScenePath_CentroidWorld
// __thiscall(ECX = scene_path, Vec3* out_world), RET 4.
// Refreshes `entity+C8h` if 0, calls 007AE6D0 (`BSP_Point3PtrArray_Centroid`),
// and transforms the local centroid by `entity+CCh` with homogeneous divide.
void scene_path_centroid_world_007b0650(
    const PathComponentState& comp,
    Vec3& out_world,
    ScenePathTransformHost& host);

// 007B0710 BSP_ScenePath_DistanceToPolylineWorld
// __thiscall(ECX = scene_path, const Vec3* world_query), RET 4, returns float in ST0.
// Refreshes inverse world matrix (`entity+10Ch` / `entity+110h`), transforms
// `world_query` into local space with homogeneous divide, and calls 007AE7B0.
float scene_path_distance_to_polyline_world_007b0710(
    const PathComponentState& comp,
    const Vec3& world_query,
    ScenePathTransformHost& host);

// ============================================================================
// 3. PathReplicator & SessionMessage_PATH_CREATE
// ============================================================================

struct PathReplicatorHost {
    virtual ~PathReplicatorHost() = default;
    // Routes a constructed session message (`0075B430` + vtable + payload) via
    // `0077C2A0 BSP_Session_RouteMessage(ECX = this - 310h, &msg, 0, 0)`.
    virtual void route_path_set_d9(int index, const Vec3& pt) = 0;
    virtual void route_path_add_da(const Vec3& pt) = 0;
    virtual void route_path_insert_db(int index, const Vec3& pt) = 0;
    virtual void route_path_delete_dc(int index) = 0;
};

// 007AD5C0 BSP_PathReplicator_SendPathSet (slot +04h of 00D0546C, msg D9h, vtable 00D02DBC, RET 8)
void path_replicator_send_path_set_007ad5c0(int index, const Vec3& pt, PathReplicatorHost& host);

// 007AD660 BSP_PathReplicator_SendPathAdd (slot +08h of 00D0546C, msg DAh, vtable 00D02DD0, RET 4)
void path_replicator_send_path_add_007ad660(const Vec3& pt, PathReplicatorHost& host);

// 007AD700 BSP_PathReplicator_SendPathInsert (slot +0Ch of 00D0546C, msg DBh, vtable 00D02DE4, RET 8)
void path_replicator_send_path_insert_007ad700(int index, const Vec3& pt, PathReplicatorHost& host);

// 007AD7A0 BSP_PathReplicator_SendPathDelete (slot +10h of 00D0546C, msg DCh, vtable 00D02DF8, RET 4)
void path_replicator_send_path_delete_007ad7a0(int index, PathReplicatorHost& host);

struct SessionMessagePathCreate {
    std::uint32_t vtable_00{0};
    std::vector<Vec3> points_114{};
};

struct PathCreateBitStreamHost {
    virtual ~PathCreateBitStreamHost() = default;
    // 00766FC0 BSP_NetEntityCreateMessage_Construct(D8h, 0, entity, 0).
    virtual void construct_net_entity_create_base_00766fc0(std::uint32_t msg_type, const void* entity) = 0;
    // 00765000 BSP_NetEntityCreateMessage_Write(bit_cursor).
    virtual void write_net_entity_create_base_00765000(void* bit_cursor) = 0;
    // 00429790 BSP_BitCursor_WriteNumericFloatArray(ECX = bit_cursor, ptr, count=3, 0, 1, FLT_MAX, 32).
    virtual void write_float_array_00429790(void* bit_cursor, const Vec3& pt, int count, int a, int b, float max_val, int bits) = 0;
    virtual void on_invalid_parameter() = 0;
};

// 007B2950 BSP_SessionMessage_PATH_CREATE_Construct
// __thiscall(ECX = msg, entity), RET 4.
// Calls 00766FC0(D8h), sets vtable 00D0377C, resizes `msg+110h` vector to match
// the pointer array at `entity+318h..+31Ch`, and copies each dereferenced Vec3.
void session_message_path_create_construct_007b2950(
    SessionMessagePathCreate& msg,
    const void* entity_opaque,
    const std::vector<const Vec3*>& entity_points_318,
    PathCreateBitStreamHost& host);

// 007B0BF0 BSP_SessionMessage_PATH_CREATE_Write
// __thiscall(ECX = msg, void* bit_cursor), RET 4.
// Calls 00765000(bit_cursor), then writes each 12-byte point in `msg+114h` via 00429790.
void session_message_path_create_write_007b0bf0(
    const SessionMessagePathCreate& msg,
    void* bit_cursor,
    PathCreateBitStreamHost& host);

// ============================================================================
// 4. PathSource Hierarchy & PathCursor Operations
// ============================================================================

struct PathSourceBase {
    std::uint32_t vtable_00{0x00D052DCu};
    std::int32_t ref_count_04{1};
    virtual ~PathSourceBase() = default;
    virtual Vec3 get_point_vslot08(int index) = 0;
    virtual int point_count_vslot0c() = 0;
    virtual int start_index_for_pos_vslot10(const Vec3& pos, float unused_radius, bool forward) = 0;
};

struct SimplePathSource {
    std::uint32_t vtable_00{0x00D05318u};
    std::int32_t ref_count_04{1};
    const std::vector<Vec3>* slot_points_08{nullptr};
};

struct AuthoredPathSource {
    std::uint32_t vtable_00{0x00D0534Cu};
    std::int32_t ref_count_04{1};
    PathComponentState* scene_path_08{nullptr};
};

struct OwnedPointsPathSource {
    std::uint32_t vtable_00{0x00D05440u};
    std::int32_t ref_count_04{1};
    std::uint32_t alloc_proxy_08{0};
    std::vector<Vec3> points_0c{};
};

// 007B0E30 BSP_SimplePathSource_PointCount (slot +0Ch of 00D05318, RET 0)
int simple_path_source_point_count_007b0e30(const SimplePathSource& src) noexcept;

// 007B0F00 BSP_SimplePathSource_Save (slot +20h of 00D05318, RET 4)
void simple_path_source_save_007b0f00(const SimplePathSource& src, PathArchiveHost& archive);

// 007B1030 BSP_AuthoredPathSource_PointCount (slot +0Ch of 00D0534C, RET 0)
int authored_path_source_point_count_007b1030(const AuthoredPathSource& src) noexcept;

struct AuthoredPathWorldHost {
    virtual ~AuthoredPathWorldHost() = default;
    // 007AF800 BSP_ScenePath_TransformPointToWorld(ECX = scene_path, out*, index).
    virtual Vec3 transform_point_to_world_007af800(const PathComponentState* scene_path, int index) = 0;
    // 007B0900 BSP_ScenePath_NearestPointIndexWorld(ECX = scene_path, pos*, float* out_dist).
    virtual int nearest_point_index_world_007b0900(const PathComponentState* scene_path, const Vec3& pos, float* out_dist) = 0;
};

// 007B10A0 BSP_AuthoredPathSource_GetPoint (slot +08h of 00D0534C, RET 8)
Vec3& authored_path_source_get_point_007b10a0(
    const AuthoredPathSource& src,
    Vec3& out,
    int index,
    AuthoredPathWorldHost& host);

// 007B10C0 BSP_AuthoredPathSource_LastPoint (slot +18h of 00D0534C, RET 4)
// Passes `count - 1` (or `-1` when `points_08` buffer is null) to 007AF800.
Vec3& authored_path_source_last_point_007b10c0(
    const AuthoredPathSource& src,
    Vec3& out,
    AuthoredPathWorldHost& host);

// 007B1100 BSP_AuthoredPathSource_NearestIndex (slot +14h of 00D0534C, RET 4)
int authored_path_source_nearest_index_007b1100(
    const AuthoredPathSource& src,
    const Vec3& pos,
    AuthoredPathWorldHost& host);

// 007B1120 BSP_AuthoredPathSource_Load (slot +1Ch of 00D0534C, RET 4)
void authored_path_source_load_007b1120(
    AuthoredPathSource& src,
    PathArchiveHost& archive);

// 007B1170 BSP_AuthoredPathSource_Save (slot +20h of 00D0534C, RET 4)
void authored_path_source_save_007b1170(
    const AuthoredPathSource& src,
    PathArchiveHost& archive);

// 007B2250 BSP_EntityPathSource_CreateForSlot
// __fastcall(ECX = SimplePathSource** out, EDX = const std::vector<Vec3>* slot_points), RET 0.
SimplePathSource** entity_path_source_create_for_slot_007b2250(
    SimplePathSource*& out_ptr,
    const std::vector<Vec3>* slot_points);

// 007B22A0 BSP_EntityPathSource_CreateForEntity
// __fastcall(ECX = AuthoredPathSource** out, EDX = PathComponentState* scene_path), RET 0.
AuthoredPathSource** entity_path_source_create_for_entity_007b22a0(
    AuthoredPathSource*& out_ptr,
    PathComponentState* scene_path);

// 007B2AB0 BSP_OwnedPointsPathSource_Construct
// __thiscall(ECX = source, const std::vector<Vec3>& src_points), RET 4.
void owned_points_path_source_construct_007b2ab0(
    OwnedPointsPathSource& src,
    const std::vector<Vec3>& points);

// 007B2C00 BSP_OwnedPointsPathSource_Save (slot +20h of 00D05440, RET 4)
void owned_points_path_source_save_007b2c00(
    const OwnedPointsPathSource& src,
    PathArchiveHost& archive);

// 007B2CF0 BSP_OwnedPointsPathSource_FreePoints (destructor body of 00D05440, RET 0)
void owned_points_path_source_free_points_007b2cf0(OwnedPointsPathSource& src) noexcept;

// 007B2D80 BSP_OwnedPointsPathSource_ConstructEmpty
// __fastcall(ECX = source), RET 0.
void owned_points_path_source_construct_empty_007b2d80(OwnedPointsPathSource& src) noexcept;

// PathCursor state (+00h vtable, +04h source*, +08h index, +0Ch follow_mode, +10h forward).
struct PathCursorState {
    PathSourceBase* source_04{nullptr};
    int index_08{0};
    int follow_mode_0c{1};
    bool forward_10{true};
};

struct FollowModeLastPointResult {
    Vec3 point{};
    bool valid_0c{false};
};

struct PathCursorHost {
    virtual ~PathCursorHost() = default;
    // Cursor vslot +04h(out*, index).
    virtual Vec3 cursor_get_point_vslot04(const PathCursorState& cursor, int index) = 0;
    // Cursor vslot +08h(new_index).
    virtual void cursor_set_index_vslot08(PathCursorState& cursor, int new_index) = 0;
    // 00BD2F10 BSP_Random_UniformFloatRange(ECX = stream, lo, hi).
    virtual float uniform_float_00bd2f10(int stream, float lo, float hi) = 0;
};

// 007ADB90 BSP_PathCursor_PointAtIndex
// __thiscall(ECX = cursor, Vec3* out, int index), RET 8.
// Without a source at +04h, writes the zero vector (`00F87574..00F8757C`).
// Otherwise replaces negative `index` with `cursor.index_08` and calls source vslot +08h.
Vec3& path_cursor_point_at_index_007adb90(
    const PathCursorState& cursor,
    Vec3& out,
    int index) noexcept;

// 007ADC60 BSP_EntityCommand_IsOnFinalLeg
// __fastcall(ECX = cursor), RET 0.
// Returns true when `source_04 == nullptr` or `count <= 0`; returns false when
// `follow_mode_0c != 1`; in mode 1 returns true at `count - 1` when `forward_10`
// is set, or at `0` when `forward_10` is clear.
bool entity_command_is_on_final_leg_007adc60(const PathCursorState& cursor) noexcept;

// 007ADCC0 BSP_EntityCommand_PathCursorNextIndex
// __fastcall(ECX = cursor), RET 0.
// Mode 1 (SIMPLE): `index + (forward ? +1 : -1)`.
// Mode 2 (PINGPONG): at 0 sets `forward = true` and returns 1; at `count - 1`
// sets `forward = false` and returns `index - 1`; otherwise steps by `forward`.
// Mode 3 (CIRCLE): wraps `count - 1 -> 0` forward and `0 -> count - 1` backward.
// Any other mode returns `index_08` unchanged.
int entity_command_path_cursor_next_index_007adcc0(PathCursorState& cursor) noexcept;

// 007ADD50 BSP_PathCursor_AdvanceToNextIndex
// __fastcall(ECX = cursor), RET 0.
// Calls `007ADCC0` and passes the result to cursor vslot +08h.
void path_cursor_advance_to_next_index_007add50(
    PathCursorState& cursor,
    PathCursorHost& host);

// 007AE060 BSP_PathCursor_SetIndexFromMessage
// __thiscall(ECX = cursor, int new_index), RET 4.
// When `source_04 != nullptr`, `count > 0`, and `new_index <= count - 1` (signed JG),
// stores `cursor.index_08 = new_index`.
void path_cursor_set_index_from_message_007ae060(
    PathCursorState& cursor,
    int new_index) noexcept;

// 007B1320 BSP_PathCursor_NextPoint
// __thiscall(ECX = cursor, Vec3* out), RET 4.
// Calls `007ADCC0` and queries cursor vslot +04h(out, next_idx).
Vec3& path_cursor_next_point_007b1320(
    PathCursorState& cursor,
    Vec3& out,
    PathCursorHost& host);

// 007B1350 BSP_PathCursor_FollowModeLastPoint
// __thiscall(ECX = cursor, FollowModeLastPointResult* out), RET 4.
// Only when `follow_mode_0c == 1`, `source_04 != nullptr`, and `count > 0`:
// queries cursor vslot +04h(count - 1), stores x/y/z and sets byte +0Ch = 1;
// otherwise sets byte +0Ch = 0.
void path_cursor_follow_mode_last_point_007b1350(
    const PathCursorState& cursor,
    FollowModeLastPointResult& out,
    PathCursorHost& host);

// 007B19B0 BSP_PathSourceRef_Assign
// __thiscall(ECX = PathSourceBase** ref_slot, PathSourceBase* new_src), RET 4.
// Decrements old refcount (deletes if 0) and stores `new_src` without incrementing.
PathSourceBase** path_source_ref_assign_007b19b0(
    PathSourceBase*& ref_slot,
    PathSourceBase* new_src) noexcept;

// 007B1BF0 BSP_PathSourceRef_CopyFrom
// __thiscall(ECX = dst_cursor, const PathCursorState* src_cursor), RET 4.
// Increments `src.source_04`, decrements/deletes old `dst.source_04`, and copies
// `index_08`, `follow_mode_0c`, and `forward_10`.
void path_source_ref_copy_from_007b1bf0(
    PathCursorState& dst,
    const PathCursorState& src) noexcept;

// 007B1D90 BSP_PathSourceRef_Release
// __fastcall(ECX = cursor), RET 0.
// Decrements/deletes `source_04`, then resets `source_04 = nullptr`, `index_08 = 0`,
// `follow_mode_0c = 1`, `forward_10 = true`.
void path_source_ref_release_007b1d90(PathCursorState& cursor) noexcept;

// 007B1C50 BSP_EntityCommand_PathCursorStart
// __thiscall(ECX = cursor, PathSourceBase* const* holder, const Vec3* pos, float radius, int follow_mode, int start_mode), RET 14h.
// Binds `*holder` with refcount update, sets `follow_mode_0c = follow_mode` and
// `forward_10 = true`. Start mode 6 (`PATH_SM_BEGIN`) sets `index_08 = 0`;
// mode 8 (`PATH_SM_JOIN_BACKWARDS`) sets `forward_10 = false`; mode 7
// (`PATH_SM_JOIN_RANDOM_DIR`) draws `U(0, 1)` on stream 1 (`ECX = 7 - 6 = 1`)
// and sets `forward_10 = (draw > 0.5f)`; mode 5 (`PATH_SM_JOIN`) keeps `forward_10 = true`;
// modes 5/7/8 then set `index_08 = source->start_index_for_pos_vslot10(pos, radius, forward_10)`.
void entity_command_path_cursor_start_007b1c50(
    PathCursorState& cursor,
    PathSourceBase* holder_source,
    const Vec3& pos,
    float radius,
    int follow_mode,
    int start_mode,
    PathCursorHost& host);

// 007B1D30 BSP_EntityCommand_PathCursorStartThunk
// __thiscall(ECX = cursor, PathSourceBase* const* holder, const Vec3* pos, float radius, const int params[2]), RET 10h.
// Unpacks `follow_mode = params[0]` and `start_mode = params[1]` and calls 007B1C50.
void entity_command_path_cursor_start_thunk_007b1d30(
    PathCursorState& cursor,
    PathSourceBase* holder_source,
    const Vec3& pos,
    float radius,
    const int params[2],
    PathCursorHost& host);

// ============================================================================
// 5. MovCam & MovieCam Helpers
// ============================================================================

struct CameraRefState {
    std::uint32_t target_404{0};
    Vec3 offset_408{};
    float radius_414{0.0f};
};

struct MovCamState {
    int mode_384{0};
    int submode_388{0};
    std::uint32_t target_38c{0};
    std::uint32_t look_target_390{0};
    Vec3 current_pos_394{};
    std::uint32_t extra_target_3e4{0};
    CameraRefState cam_ref_3f0{};
    Vec3 viktor_angles_488{};
    std::vector<char> callback_name_49c{};
    std::uint32_t callback_args_4ac{0};
    bool tompi_flag_4b4{false};
    Vec3 tompi_look_offset_4b8{};
    Vec3 tompi_pos_offset_4c4{};
};

struct MovCamHost {
    virtual ~MovCamHost() = default;
    virtual void inherit_race_and_activate_0042a910() = 0;
    virtual void enter_mode_007a5110(MovCamState& cam, int transition_flag) = 0;
    virtual void step_subobject_vslot0c_170(MovCamState& cam) = 0;
    virtual void set_look_target_00797070(MovCamState& cam, std::uint32_t target, int flag) = 0;
    virtual bool entity_is_kind_of(std::uint32_t entity, std::uint32_t kind) = 0;
    virtual std::uint32_t entity_parent_3d0(std::uint32_t entity) = 0;
    virtual bool observer_is_pair_registered_00694af0(std::uint32_t target) = 0;
    virtual void observer_register_pair_00694a60(std::uint32_t target) = 0;
    virtual void observer_unregister_pair_006952a0(std::uint32_t target) = 0;
    virtual int lookup_saved_deck_index_007a0ac0(std::uint32_t target) = 0;
    virtual int pick_random_deck_camera_007a49a0(std::uint32_t target, std::uint32_t look_target, int group) = 0;
    virtual CameraRefState construct_camera_ref_point_00793920(std::uint32_t target, const Vec3& pt) = 0;
    virtual CameraRefState construct_camera_ref_radius_007939b0(std::uint32_t target, float radius) = 0;
    virtual Vec3 resolve_camera_ref_00793ac0(const CameraRefState& ref, int flag) = 0;
    virtual float vector3_length_threshold_0042b2f0(const Vec3& diff) = 0;
    virtual void set_linear_speed_limit_005cb9f0(float speed) = 0;
    virtual void free_callback_args_006b88e0(std::uint32_t args_ptr) = 0;
    virtual Vec3 transform_target_local_point_004142e0(std::uint32_t target, const Vec3& local_pt) = 0;
    virtual Vec3 transform_target_world_via_scratch_00414d10(std::uint32_t target, const Vec3& local_pt) = 0;
};

// 007A6270 BSP_MovCam_SetMode1FlagBit1
// __thiscall(ECX = MovCam*, char flag), RET 4.
void movcam_set_mode1_flag_bit1_007a6270(MovCamState& cam, bool flag, MovCamHost& host);

// 007A6310 BSP_MovCam_SetMode1FlagBit0
// __thiscall(ECX = MovCam*, char flag), RET 4.
void movcam_set_mode1_flag_bit0_007a6310(MovCamState& cam, bool flag, MovCamHost& host);

// 007A63B0 BSP_MovCam_SetRefPosPointImpl
// __thiscall(ECX = MovCam*, float x, float y, float z, char keep_mode, int transition_flag), RET 14h.
void movcam_set_ref_pos_point_impl_007a63b0(
    MovCamState& cam,
    const Vec3& pt,
    bool keep_mode,
    int transition_flag,
    MovCamHost& host);

// 007A68F0 BSP_MovCam_OnActivate
// __fastcall(ECX = MovCam*), RET 0.
void movcam_on_activate_007a68f0(MovCamState& cam, MovCamHost& host);

// 007A6910 BSP_MovCam_SetTarget
// __thiscall(ECX = MovCam*, uint32_t entity), RET 4.
void movcam_set_target_007a6910(MovCamState& cam, std::uint32_t entity, MovCamHost& host);

// 007A6A30 BSP_MovCam_OnObservedReleased
// __thiscall(ECX = MovCam + 10h, uint32_t entity), RET 4.
void movcam_on_observed_released_007a6a30(MovCamState& cam, std::uint32_t entity, MovCamHost& host);

// 007A6B40 BSP_MovCam_SetHandModeTargeted
// __thiscall(ECX = MovCam*, uint32_t target, char flag_a, char flag_b), RET 0Ch.
void movcam_set_hand_mode_targeted_007a6b40(
    MovCamState& cam,
    std::uint32_t target,
    bool flag_a,
    bool flag_b,
    MovCamHost& host);

// 007A6B90 BSP_MovCam_SetHandModeDeck
// __thiscall(ECX = MovCam*, uint32_t target, int deck_index), RET 8.
void movcam_set_hand_mode_deck_007a6b90(
    MovCamState& cam,
    std::uint32_t target,
    int deck_index,
    MovCamHost& host);

// 007A6BC0 BSP_MovCam_SetHandModeOverview
// __thiscall(ECX = MovCam*, uint32_t target, int transition_flag, float duration, const string* cb_name, uint32_t cb_args), RET 14h.
void movcam_set_hand_mode_overview_007a6bc0(
    MovCamState& cam,
    std::uint32_t target,
    int transition_flag,
    float duration,
    const char* cb_name,
    std::size_t cb_name_len,
    std::uint32_t cb_args,
    MovCamHost& host);

// 007A6CD0 BSP_MovCam_SetRefPos
// __thiscall(ECX = MovCam*, uint32_t target, const Vec3* offset, int transition_flag, float duration, const string* cb_name, uint32_t cb_args), RET 18h.
void movcam_set_ref_pos_007a6cd0(
    MovCamState& cam,
    std::uint32_t target,
    const Vec3& offset,
    int transition_flag,
    float duration,
    const char* cb_name,
    std::size_t cb_name_len,
    std::uint32_t cb_args,
    MovCamHost& host);

// 007A6E60 BSP_MovCam_SetRefPosPolar
// __thiscall(ECX = MovCam*, uint32_t target, float r, float a, float b, int transition_flag, float duration, const string* cb_name, uint32_t cb_args), RET 20h.
void movcam_set_ref_pos_polar_007a6e60(
    MovCamState& cam,
    std::uint32_t target,
    float r,
    float angle_a,
    float angle_b,
    int transition_flag,
    float duration,
    const char* cb_name,
    std::size_t cb_name_len,
    std::uint32_t cb_args,
    MovCamHost& host);

// 007A7070 BSP_MovCam_SetRefPosRadius
// __thiscall(ECX = MovCam*, uint32_t target, float radius, int transition_flag, float duration, const string* cb_name, uint32_t cb_args), RET 18h.
void movcam_set_ref_pos_radius_007a7070(
    MovCamState& cam,
    std::uint32_t target,
    float radius,
    int transition_flag,
    float duration,
    const char* cb_name,
    std::size_t cb_name_len,
    std::uint32_t cb_args,
    MovCamHost& host);

// 007A7DE0 BSP_MovCam_SetViktorMode
// __thiscall(ECX = MovCam*, uint32_t target, uint32_t look_target, float angle_a, float angle_b, int transition_flag, float duration, const string* cb_name, uint32_t cb_args), RET 20h.
void movcam_set_viktor_mode_007a7de0(
    MovCamState& cam,
    std::uint32_t target,
    std::uint32_t look_target,
    float angle_a,
    float angle_b,
    int transition_flag,
    float duration,
    const char* cb_name,
    std::size_t cb_name_len,
    std::uint32_t cb_args,
    MovCamHost& host);

// 007A7F30 BSP_MovCam_SetTompiModeWeak
// __thiscall(ECX = MovCam*, uint32_t target, const Vec3* pos_offset, uint32_t look_target, const Vec3* look_offset, int transition_flag, float duration, const string* cb_name, uint32_t cb_args), RET 20h.
void movcam_set_tompi_mode_weak_007a7f30(
    MovCamState& cam,
    std::uint32_t target,
    const Vec3& pos_offset,
    std::uint32_t look_target,
    const Vec3& look_offset,
    int transition_flag,
    float duration,
    const char* cb_name,
    std::size_t cb_name_len,
    std::uint32_t cb_args,
    MovCamHost& host);

// 007A8100 BSP_MovCam_SetTompiMode
// __thiscall(ECX = MovCam*, uint32_t target, const Vec3* pos_offset, uint32_t look_target, const Vec3* look_offset), RET 10h.
void movcam_set_tompi_mode_007a8100(
    MovCamState& cam,
    std::uint32_t target,
    const Vec3& pos_offset,
    std::uint32_t look_target,
    const Vec3& look_offset,
    MovCamHost& host);

// 007A8180 BSP_MovCam_PickDeckPosition
// __thiscall(ECX = MovCam*, int group, uint32_t target, uint32_t look_target), RET 0Ch.
void movcam_pick_deck_position_007a8180(
    MovCamState& cam,
    int group,
    std::uint32_t target,
    std::uint32_t look_target,
    MovCamHost& host);

// 007A82C0 BSP_MovCam_SetRefPosPoint
// __thiscall(ECX = MovCam*, uint32_t target, Vec3 pt_by_val, char keep_mode, int transition_flag), RET 18h.
void movcam_set_ref_pos_point_007a82c0(
    MovCamState& cam,
    std::uint32_t target,
    const Vec3& pt,
    bool keep_mode,
    int transition_flag,
    MovCamHost& host);

// 007A8420 BSP_MovCam_SetRefPosSpherical
// __thiscall(ECX = MovCam*, uint32_t target, float r, float a, float b, char keep_mode, int transition_flag), RET 18h.
void movcam_set_ref_pos_spherical_007a8420(
    MovCamState& cam,
    std::uint32_t target,
    float r,
    float angle_a,
    float angle_b,
    bool keep_mode,
    int transition_flag,
    MovCamHost& host);

struct MovieCamEntityState {
    std::uint32_t field_58{1};
    virtual ~MovieCamEntityState() = default;
    virtual void inherit_race_and_activate_thunk_009277e0() = 0;
};

// 007AADB0 BSP_MovieCamLookat_Activate (slot +A0h of 00CE8860, RET 0)
void movie_cam_lookat_activate_007aadb0(MovieCamEntityState& entity);

// 007AADD0 BSP_MovieCamPos_Activate (slot +A0h of 00CE86D8, RET 0)
void movie_cam_pos_activate_007aadd0(MovieCamEntityState& entity);

// ============================================================================
// 6. MParatrooper Residue & Lua Float Field Readers
// ============================================================================

struct LuaTableReaderHost {
    virtual ~LuaTableReaderHost() = default;
    // 00B67800 BSP_LuaObject_GetByName + 00B66270 BSP_LuaObject_GetNumber + 00B67700 Destruct.
    virtual double get_number_by_name(const char* field_name) = 0;
    virtual const char* get_string_by_name(const char* field_name) = 0;
    virtual void read_bomb_class_lua_fields_006e1be0() = 0;
    virtual void read_number_field_007003b0(const char* field_name, float& out) = 0;
    virtual void read_string_field_004fe080(const char* field_name, std::uint32_t& out_handle) = 0;
    virtual std::uint32_t get_or_load_soldier_class_004b1400(const char* class_name) = 0;
};

// 007AAE50, 007AAED0, 007AAF50, 007AAFD0 BSP_LuaObject_GetFloatField_*
// __fastcall(ECX = const char* field_name, EDX = float* out, LuaObject by-val on stack), RET 14h.
void lua_object_get_float_field_007aae50(const char* field_name, float& out, LuaTableReaderHost& host);
void lua_object_get_float_field_007aaed0(const char* field_name, float& out, LuaTableReaderHost& host);
void lua_object_get_float_field_007aaf50(const char* field_name, float& out, LuaTableReaderHost& host);
void lua_object_get_float_field_007aafd0(const char* field_name, float& out, LuaTableReaderHost& host);

struct ParatrooperClassResidue {
    float initial_speed_50{0.0f};
    float capture_power_d8{0.0f};
    float slow_factor_closed_dc{0.0f};
    float slow_factor_opened_e0{0.0f};
    float open_duration_e4{0.0f};
    float close_duration_e8{0.0f};
    std::uint32_t soldier_class_ec{0};
    std::uint32_t soldier_anim_f0{0};
    float capture_duration_f8{0.0f};
    float damage_fc{0.0f};
};

// 007AC780 BSP_MParatrooperClass_ReadLuaFields
// __thiscall(ECX = ParatrooperClass*, const LuaObject* table), RET 4.
void mparatrooper_class_read_lua_fields_007ac780(
    ParatrooperClassResidue& cls,
    LuaTableReaderHost& host);

struct ParatrooperResidueHost {
    virtual ~ParatrooperResidueHost() = default;
    // 006E0D20 BSP_SessionMessage_BOMB_CREATE_Construct(B9h, class_id_0c, entity) +
    // 00779FC0 BSP_Entity_SendMessageToPeer(peer_id, &msg) + 007668E0 Destruct.
    virtual void send_paratrooper_create_msg_b9(std::uint32_t peer_id, std::uint32_t class_id_0c, const Vec3& target_47c) = 0;
    // 007AB230 BSP_MParatrooper_SetLandingPos(ECX = entity, const Vec3* pt).
    virtual void set_landing_pos_007ab230(std::uint32_t entity, const Vec3& pt) = 0;
    // 00928560 BSP_GameEntity_RegisterInParentEntityList(ECX = entity).
    virtual void register_in_parent_entity_list_00928560() = 0;
    // 006E29C0 BSP_BombFamily_DetachFromWorldLists(ECX = entity).
    virtual void detach_bomb_family_from_world_lists_006e29c0() = 0;
    // 00484540 BSP_UnitList_PushBack(ECX = world + list_offset, entity).
    virtual void world_list_push_back_00484540(std::uint32_t list_offset) = 0;
    // 004837D0 BSP_UnitList_EraseNode(ECX = world + 264h, node).
    virtual void world_list_264_erase_self_004837d0() = 0;
    // 007AB8C0 BSP_MParatrooper_Construct(ECX = raw_mem, param).
    virtual void construct_paratrooper_007ab8c0(std::uint32_t spawn_param) = 0;
    virtual void attach_to_world_scene_vslot98() = 0;
    virtual void orthonormalize_basis_rows_0085dc80(const Vec3& forward_row_94) = 0;
    virtual void invalidate_subtree_pose_0042ed50() = 0;
};

// 007AB7D0 BSP_MParatrooper_SendCreateMessage (slot +17Ch of 00D05060, RET 4)
void mparatrooper_send_create_message_007ab7d0(
    std::uint32_t peer_id,
    std::uint32_t class_id_0c,
    const Vec3& target_47c,
    ParatrooperResidueHost& host);

// 007AB8A0 BSP_MParatrooper_SetLandingPosAdjust120 (slot +20h of 00D037E0, RET 4)
// Adds 120h to `ECX` (message payload landing target) and calls `007AB230(entity, msg + 120h)`.
void mparatrooper_set_landing_pos_adjust_120_007ab8a0(
    const Vec3& msg_landing_pos_120,
    std::uint32_t entity,
    ParatrooperResidueHost& host);

// 007AB9D0 BSP_MParatrooper_GetFlagPairFalse (slot +1A4h of 00D05060, RET 8)
void mparatrooper_get_flag_pair_false_007ab9d0(bool& out_a, bool& out_b) noexcept;

// 007ABAB0 BSP_MParatrooper_RegisterInWorldLists (slot +130h of 00D05060, RET 0)
// Calls 00928560, then pushes self into `[world+30h]`, `[world+210h]`, and `[world+264h]`.
void mparatrooper_register_in_world_lists_007abab0(ParatrooperResidueHost& host);

// 007ABAF0 BSP_MParatrooper_UnregisterFromWorldLists (slot +134h of 00D05060, RET 0)
// Calls 006E29C0, then walks `[world+268h]` (`node+8 == this`, `node = node+4`) and erases via 004837D0.
void mparatrooper_unregister_from_world_lists_007abaf0(ParatrooperResidueHost& host);

struct ParatrooperCreatedEntity {
    std::uint32_t side_54{0};
    Vec3 forward_94{};
    Vec3 pos_a4{};
    bool pose_valid_c8{false};
    bool inv_pose_valid_10c{false};
    const ParatrooperClassResidue* cls_314{nullptr};
    Vec3 velocity_318{};
    Vec3 bullet_pos_370{};
    Vec3 bullet_prev_pos_37c{};
};

// 007AC4C0 BSP_MParatrooperClass_Create (slot +24h of 00CFA604, RET 4)
void mparatrooper_class_create_007ac4c0(
    const ParatrooperClassResidue& cls,
    std::uint32_t spawn_param,
    ParatrooperCreatedEntity& out_entity,
    ParatrooperResidueHost& host);

// 007AC2C0 BSP_MParatrooperClass_CreateWithPose (slot +20h of 00CFA604, RET 1Ch)
// Allocates and constructs `MParatrooper`, sets `side_54`, `cls_314`, attaches to
// world scene (`vslot +98h`), sets `velocity_318 = cls->initial_speed_50 * dir`,
// orthonormalizes pose (`0085DC80`), copies `pos` into `+A4h`, `+370h`, and `+37Ch`,
// clears `+C8h` and `+10Ch`, invalidates child subtree poses (`0042ED50`), and
// returns the `+310h` bullet sub-object.
void mparatrooper_class_create_with_pose_007ac2c0(
    const ParatrooperClassResidue& cls,
    std::uint32_t spawn_param,
    std::uint32_t side,
    const Vec3& pos,
    const Vec3& dir,
    ParatrooperCreatedEntity& out_entity,
    ParatrooperResidueHost& host);

}  // namespace bsp::path_movcam_seg46_argon
