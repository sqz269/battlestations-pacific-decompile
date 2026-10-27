#include "bsp/path_movcam_seg46_argon.hpp"

namespace bsp::path_movcam_seg46_argon {
const char* movcam_description_0078e450(const MovCamState& c) noexcept {
    return c.mode_384 == 2 ? "MovieCameraDeck" : "MovieCameraOuter";
}
void movcam_set_cinematic_007a66d0(MovCamState& c, bool enabled, int transition, std::uint32_t, MovCamHost& h) {
    if (enabled) c.mode_384 = 3;
    else if (c.mode_384 == 3) c.mode_384 = 1;
    h.enter_mode_007a5110(c, transition);
}
int path_cursor_index_007b1010(const PathCursorState& c) noexcept { return c.index_08; }
std::uint32_t path_source_zero_007ac970() noexcept { return 0; }
void point3_vector_load_from_archive_007b2780(std::vector<Vec3>& points, PathArchiveHost& h) {
    std::uint32_t i = 0;
    while (h.has_indexed_entry(1, i)) {
        // Native resize(i+1) shrinks an existing vector on the first entry;
        // an archive with no entries leaves the original vector untouched.
        points.resize(static_cast<std::size_t>(i) + 1);
        h.read_indexed_vec3(1, i, 5, points[i]);
        ++i;
    }
}
void path_cursor_save_007b1800(const PathCursorState& c, PathCursorSaveHost& h) {
    if (c.source_04) {
        h.begin_section(0, "pathIntf");
        h.save_source_vslot20(*c.source_04);
        h.end_section();
    }
    h.write_int(0, "index", 1, c.index_08);
    h.write_int(0, "mode", 1, c.follow_mode_0c);
    h.write_bool(0, "forward", 3, c.forward_10);
}
}

#include "bsp/cc13_x87_model.hpp"

#include <cmath>
#include <limits>
#include <new>

namespace bsp::path_movcam_seg46_argon {

namespace {

inline float to_f32(double v) noexcept {
    return static_cast<float>(v);
}

inline Vec3 homogeneous_divide_vec4(const float h[4]) noexcept {
    const float inv_w = to_f32(1.0 / static_cast<double>(h[3]));
    return Vec3{
        to_f32(static_cast<double>(h[0]) * static_cast<double>(inv_w)),
        to_f32(static_cast<double>(h[1]) * static_cast<double>(inv_w)),
        to_f32(static_cast<double>(inv_w) * static_cast<double>(h[2])),
    };
}

inline void assign_callback_fields(
    MovCamState& cam,
    const char* cb_name,
    std::size_t cb_name_len,
    std::uint32_t cb_args,
    MovCamHost& host) {
    if (cb_name_len > 0 && cb_name != nullptr) {
        cam.callback_name_49c.assign(cb_name, cb_name + cb_name_len);
        if (cam.callback_args_4ac != 0) {
            host.free_callback_args_006b88e0(cam.callback_args_4ac);
            cam.callback_args_4ac = 0;
        }
        cam.callback_args_4ac = cb_args;
    }
}

inline void apply_camera_ref_speed_limit(
    MovCamState& cam,
    int transition_flag,
    float duration,
    MovCamHost& host) {
    if (static_cast<char>(transition_flag) == 0 && duration > 1.0e-5f) {
        const Vec3 target_pos = host.resolve_camera_ref_00793ac0(cam.cam_ref_3f0, 1);
        const Vec3 diff{
            to_f32(static_cast<double>(target_pos.x) - static_cast<double>(cam.current_pos_394.x)),
            to_f32(static_cast<double>(target_pos.y) - static_cast<double>(cam.current_pos_394.y)),
            to_f32(static_cast<double>(target_pos.z) - static_cast<double>(cam.current_pos_394.z)),
        };
        const float dist = host.vector3_length_threshold_0042b2f0(diff);
        const float speed = to_f32(static_cast<double>(dist) / static_cast<double>(duration));
        host.set_linear_speed_limit_005cb9f0(speed);
    }
}

}  // namespace

// ============================================================================
// 1. Vector3 & Point3PtrArray / Point3Vector Helpers
// ============================================================================

// 007AE330 BSP_Vector3_ComponentMin_007ae330
// Address: 007AE330-007AE3D5
// Evidence: FCOMIP ST0(b.z), ST1(a.z) + JBE selects b when !(b > a) (so b <= a or unordered);
// repeats for y and x, then writes x, y, z to ECX and returns ECX. Each operand goes through
// FLD / FSTP DWORD into a stack slot first, which quiets a signalling NaN (cc13-fixture6).
Vec3& vector3_component_min_007ae330(Vec3& out, const Vec3& a, const Vec3& b) noexcept {
    using cc13x87::x87_fld_fstp;
    const float az = x87_fld_fstp(a.z), bz = x87_fld_fstp(b.z);
    const float z = (bz > az) ? az : bz;
    const float ay = x87_fld_fstp(a.y), by = x87_fld_fstp(b.y);
    const float y = (by > ay) ? ay : by;
    const float ax = x87_fld_fstp(a.x), bx = x87_fld_fstp(b.x);
    const float x = (bx > ax) ? ax : bx;
    out.x = x;
    out.y = y;
    out.z = z;
    return out;
}

// 007AE3E0 BSP_Vector3_ComponentMax_007ae3e0
// Address: 007AE3E0-007AE485
// Evidence: FCOMIP ST0(a.z), ST1(b.z) + JBE selects b when !(a > b) (so a <= b or unordered);
// repeats for y and x, then writes x, y, z to ECX and returns ECX. Operands are quieted by
// FLD / FSTP DWORD as in 007AE330.
Vec3& vector3_component_max_007ae3e0(Vec3& out, const Vec3& a, const Vec3& b) noexcept {
    using cc13x87::x87_fld_fstp;
    const float az = x87_fld_fstp(a.z), bz = x87_fld_fstp(b.z);
    const float z = (az > bz) ? az : bz;
    const float ay = x87_fld_fstp(a.y), by = x87_fld_fstp(b.y);
    const float y = (ay > by) ? ay : by;
    const float ax = x87_fld_fstp(a.x), bx = x87_fld_fstp(b.x);
    const float x = (ax > bx) ? ax : bx;
    out.x = x;
    out.y = y;
    out.z = z;
    return out;
}

// 007AE650 BSP_Point3Vector_SaveToArchive
// Address: 007AE650-007AE6CD
void point3_vector_save_to_archive_007ae650(const std::vector<Vec3>& points, PathArchiveHost& archive) {
    const std::uint32_t count = static_cast<std::uint32_t>(points.size());
    for (std::uint32_t i = 0; i < count; ++i) {
        if (i >= points.size()) {
            archive.on_invalid_parameter();
        }
        archive.write_indexed_vec3(1, i, 5, points[i]);
    }
}

// 007AEEC0 BSP_Point3PtrArray_NearestPointToLine
// Address: 007AEEC0-007AF019
std::uint32_t point3_ptr_array_nearest_point_to_line_007aeec0(
    const std::vector<const Vec3*>& points,
    const Vec3& line_origin,
    const Vec3& line_dir,
    float* out_distance,
    PointLineDistanceHost& host) {
    const float len = host.vector3_length_00419440(line_dir);
    const float inv_len = (len > 0.0f) ? to_f32(1.0 / static_cast<double>(len)) : 0.0f;
    const Vec3 unit_dir{
        to_f32(static_cast<double>(inv_len) * static_cast<double>(line_dir.x)),
        to_f32(static_cast<double>(line_dir.y) * static_cast<double>(inv_len)),
        to_f32(static_cast<double>(inv_len) * static_cast<double>(line_dir.z)),
    };

    if (points.empty()) {
        host.on_invalid_parameter();
        if (out_distance != nullptr) {
            *out_distance = 0.0f;
        }
        return 0;
    }

    std::uint32_t best_index = 0;
    float best_dist = host.point_line_distance_0046f440(*points[0], line_origin, unit_dir);
    float last_dist = best_dist;

    const std::size_t n = points.size();
    for (std::uint32_t i = 1; i < static_cast<std::uint32_t>(n); ++i) {
        if (i >= points.size()) {
            host.on_invalid_parameter();
        }
        last_dist = host.point_line_distance_0046f440(*points[i], line_origin, unit_dir);
        if (best_dist > last_dist) {
            best_dist = last_dist;
            best_index = i;
        }
    }

    if (out_distance != nullptr) {
        // Note: assembly at 007AF005 reads [ESP+2Ch] (last_dist when n > 1, or best_dist when n == 1)
        // except that 007AF005 is after POP EBX (+4 shift), so [ESP+2Ch] after POP EBX is [ESP+30h]
        // before POP EBX, which is best_dist!
        *out_distance = best_dist;
    }
    return best_index;
}

// 007AF020 BSP_Point3PtrArray_LoadFromArchive
// Address: 007AF020-007AF08F
void point3_ptr_array_load_from_archive_007af020(
    Point3PtrArrayAppendHost& array_self,
    PathArchiveHost& archive) {
    std::uint32_t i = 0;
    while (archive.has_indexed_entry(1, i)) {
        Vec3 pt{};
        archive.read_indexed_vec3(1, i, 5, pt);
        array_self.append_point_vslot08(pt);
        ++i;
    }
}

// 007AF090 BSP_Point3PtrArray_SaveToArchive
// Address: 007AF090-007AF104
void point3_ptr_array_save_to_archive_007af090(
    const std::vector<const Vec3*>& points,
    PathArchiveHost& archive) {
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(points.size()); ++i) {
        if (i >= points.size()) {
            archive.on_invalid_parameter();
        }
        archive.write_indexed_bool(1, i, 3, points[i] != nullptr);
    }
}

// 007AF110 BSP_PathComponent_SavePoints
// Address: 007AF110-007AF148
void path_component_save_points_007af110(
    const std::vector<const Vec3*>& points,
    PathArchiveHost& archive) {
    archive.begin_section(0, "_pathBaseEntity");
    point3_ptr_array_save_to_archive_007af090(points, archive);
    archive.end_section();
}

// 007B0B00 BSP_PathComponent_SavePoints_00CE84E8
// Address: 007B0B00-007B0B38
void path_component_save_points_007b0b00(
    const std::vector<const Vec3*>& points,
    PathArchiveHost& archive) {
    archive.begin_section(0, "_pathBaseEntity");
    point3_ptr_array_save_to_archive_007af090(points, archive);
    archive.end_section();
}

// 007B0B40 BSP_Path_SaveEntityAndPoints
// Address: 007B0B40-007B0B82
void path_save_entity_and_points_007b0b40(
    const std::vector<const Vec3*>& points_at_1e4,
    PathArchiveHost& archive) {
    archive.save_entity_base_00928450();
    archive.begin_section(0, "_pathBaseEntity");
    point3_ptr_array_save_to_archive_007af090(points_at_1e4, archive);
    archive.end_section();
}

// 007B0BA0 BSP_PathOwner_SaveEntityAndPoints_0077c160
// Address: 007B0BA0-007B0BE2
void path_owner_save_entity_and_points_007b0ba0(
    const std::vector<const Vec3*>& points_at_310,
    PathArchiveHost& archive) {
    archive.save_net_entity_replication_0077c160();
    archive.begin_section(0, "_pathBaseEntity");
    point3_ptr_array_save_to_archive_007af090(points_at_310, archive);
    archive.end_section();
}

// 007B0D30 BSP_PathEntity_SaveEntityAndPoints_004e6660
// Address: 007B0D30-007B0D72
void path_entity_save_entity_and_points_007b0d30(
    const std::vector<const Vec3*>& points_at_1e4,
    PathArchiveHost& archive) {
    archive.save_entity_base_00928450();
    archive.begin_section(0, "_pathBaseEntity");
    point3_ptr_array_save_to_archive_007af090(points_at_1e4, archive);
    archive.end_section();
}

// 007B1B10 BSP_Point3PtrArray_AllocPoint
// Address: 007B1B10-007B1B32
Vec3* point3_ptr_array_alloc_point_007b1b10() {
    Vec3* pt = new (std::nothrow) Vec3{};
    if (pt != nullptr) {
        pt->x = 0.0f;
        pt->y = 0.0f;
        pt->z = 0.0f;
    }
    return pt;
}

// 007B1FB0 BSP_Point3Vector_PruneFromIndex
// Address: 007B1FB0-007B2170
void point3_vector_prune_from_index_007b1fb0(
    std::vector<Vec3>& points,
    int start_index,
    float min_distance_threshold_ce7d78) {
    if (start_index < 0) {
        start_index = 0;
    }
    int ebp = static_cast<int>(points.size()) - 1;
    while (ebp > start_index) {
        const Vec3 cur = points[static_cast<std::size_t>(ebp)];
        const Vec3 prev = points[static_cast<std::size_t>(ebp - 1)];
        const float dx = to_f32(static_cast<double>(prev.x) - static_cast<double>(cur.x));
        const float dy = to_f32(static_cast<double>(prev.y) - static_cast<double>(cur.y));
        const float dz = to_f32(static_cast<double>(prev.z) - static_cast<double>(cur.z));
        const float sum_sq = to_f32(
            (static_cast<double>(dx) * static_cast<double>(dx) +
             static_cast<double>(dy) * static_cast<double>(dy)) +
            static_cast<double>(dz) * static_cast<double>(dz));
        float dist = 0.0f;
        if (static_cast<double>(sum_sq) > 1.0e-10) {
            dist = to_f32(std::sqrt(static_cast<double>(sum_sq)));
        }
        if (min_distance_threshold_ce7d78 > dist) {
            points.erase(points.begin() + (ebp - 1));
        }
        --ebp;
    }
}

// ============================================================================
// 2. PathObject, PathPoints, ScenePath, and CameraPath World Transforms
// ============================================================================

// 007ACA30 BSP_PathObject_GetEntity
// Address: 007ACA30-007ACA3A
std::uint32_t path_object_get_entity_007aca30(const PathComponentState* path_obj) noexcept {
    if (path_obj == nullptr) {
        return 0;
    }
    return path_obj->entity_14;
}

// 007B28C0 BSP_PathComponent_Construct
// Address: 007B28C0-007B28EB
void path_component_construct_007b28c0(PathComponentState& comp, std::uint32_t entity) noexcept {
    comp.points_08.clear();
    comp.vtable_00 = 0x00D053D8u;
    comp.entity_14 = entity;
    comp.cached_length_1c = 0.0f;
    comp.knots_dirty_18 = true;
    comp.extra_20 = 0;
}

// 007B28F0 BSP_CameraPathComponent_Construct
// Address: 007B28F0-007B291B
void camera_path_component_construct_007b28f0(PathComponentState& comp, std::uint32_t entity) noexcept {
    comp.points_08.clear();
    comp.entity_14 = entity;
    comp.cached_length_1c = 0.0f;
    comp.knots_dirty_18 = true;
    comp.extra_20 = 0;
    comp.vtable_00 = 0x00D0540Cu;
}

// 007AD180 BSP_PathPoints_SetPointLocal
// Address: 007AD180-007AD25E
void path_points_set_point_local_007ad180(
    const PathComponentState& comp,
    int index,
    const Vec3& world_pt,
    ScenePathTransformHost& host) {
    host.ensure_inverse_world_matrix_10c(comp.entity_14);
    float h[4]{0.0f, 0.0f, 0.0f, 1.0f};
    host.transform_vec4_inverse_110(comp.entity_14, world_pt, h);
    const Vec3 local_pt = homogeneous_divide_vec4(h);
    host.vslot04_set_point(index, local_pt);
}

// 007AD330 BSP_PathPoints_AddPointLocal
// Address: 007AD330-007AD409
void path_points_add_point_local_007ad330(
    const PathComponentState& comp,
    const Vec3& world_pt,
    ScenePathTransformHost& host) {
    host.ensure_inverse_world_matrix_10c(comp.entity_14);
    float h[4]{0.0f, 0.0f, 0.0f, 1.0f};
    host.transform_vec4_inverse_110(comp.entity_14, world_pt, h);
    const Vec3 local_pt = homogeneous_divide_vec4(h);
    host.vslot08_add_point(local_pt);
}

// 007AD4E0 BSP_PathPoints_InsertPointLocal
// Address: 007AD4E0-007AD5BE
void path_points_insert_point_local_007ad4e0(
    const PathComponentState& comp,
    int index,
    const Vec3& world_pt,
    ScenePathTransformHost& host) {
    host.ensure_inverse_world_matrix_10c(comp.entity_14);
    float h[4]{0.0f, 0.0f, 0.0f, 1.0f};
    host.transform_vec4_inverse_110(comp.entity_14, world_pt, h);
    const Vec3 local_pt = homogeneous_divide_vec4(h);
    host.vslot0c_insert_point(index, local_pt);
}

// 007AF740 BSP_ScenePath_PointToWorldCached
// Address: 007AF740-007AF7FF
void scene_path_point_to_world_cached_007af740(
    const PathComponentState& comp,
    Vec3& out_world,
    std::uint32_t index,
    ScenePathTransformHost& host) {
    if (comp.points_08.empty() || index >= comp.points_08.size()) {
        host.on_invalid_parameter();
        return;
    }
    const Vec3& local_pt = *comp.points_08[index];
    float h[4]{0.0f, 0.0f, 0.0f, 1.0f};
    host.transform_vec4_cached_74(comp.entity_14, local_pt, h);
    out_world = homogeneous_divide_vec4(h);
}

// 007AFC60 BSP_CameraPath_SampleWorldLinearCached
// Address: 007AFC60-007AFD51
void camera_path_sample_world_linear_cached_007afc60(
    const PathComponentState& comp,
    float u,
    Vec3& out_pos,
    Vec3* out_tan,
    ScenePathTransformHost& host) {
    host.sample_local_linear_007afac0(u, out_pos, out_tan);
    float h[4]{0.0f, 0.0f, 0.0f, 1.0f};
    host.transform_vec4_cached_74(comp.entity_14, out_pos, h);
    out_pos = homogeneous_divide_vec4(h);
    if (out_tan != nullptr) {
        *out_tan = host.transform_direction_cached_74(comp.entity_14, *out_tan);
    }
}

// 007AFD60 BSP_CameraPath_SampleWorldLinear
// Address: 007AFD60-007AFE76
void camera_path_sample_world_linear_007afd60(
    const PathComponentState& comp,
    float u,
    Vec3& out_pos,
    Vec3* out_tan,
    ScenePathTransformHost& host) {
    host.sample_local_linear_007afac0(u, out_pos, out_tan);
    host.ensure_world_matrix_c8(comp.entity_14);
    float h[4]{0.0f, 0.0f, 0.0f, 1.0f};
    host.transform_vec4_world_cc(comp.entity_14, out_pos, h);
    out_pos = homogeneous_divide_vec4(h);
    if (out_tan != nullptr) {
        host.ensure_world_matrix_c8(comp.entity_14);
        *out_tan = host.transform_direction_world_cc(comp.entity_14, *out_tan);
    }
}

// 007B03C0 BSP_CameraPath_SampleWorld
// Address: 007B03C0-007B04B6
void camera_path_sample_world_cached_cubic_007b03c0(
    const PathComponentState& comp,
    float u,
    Vec3& out_pos,
    Vec3* out_tan,
    std::uint32_t mode,
    ScenePathTransformHost& host) {
    host.sample_local_cubic_007afe80(u, out_pos, out_tan, mode);
    float h[4]{0.0f, 0.0f, 0.0f, 1.0f};
    host.transform_vec4_cached_74(comp.entity_14, out_pos, h);
    out_pos = homogeneous_divide_vec4(h);
    if (out_tan != nullptr) {
        *out_tan = host.transform_direction_cached_74(comp.entity_14, *out_tan);
    }
}

// 007B0650 BSP_ScenePath_CentroidWorld
// Address: 007B0650-007B0709
void scene_path_centroid_world_007b0650(
    const PathComponentState& comp,
    Vec3& out_world,
    ScenePathTransformHost& host) {
    host.ensure_world_matrix_c8(comp.entity_14);
    const Vec3 local_centroid = host.centroid_local_007ae6d0();
    float h[4]{0.0f, 0.0f, 0.0f, 1.0f};
    host.transform_vec4_world_cc(comp.entity_14, local_centroid, h);
    out_world = homogeneous_divide_vec4(h);
}

// 007B0710 BSP_ScenePath_DistanceToPolylineWorld
// Address: 007B0710-007B07E7
float scene_path_distance_to_polyline_world_007b0710(
    const PathComponentState& comp,
    const Vec3& world_query,
    ScenePathTransformHost& host) {
    host.ensure_inverse_world_matrix_10c(comp.entity_14);
    float h[4]{0.0f, 0.0f, 0.0f, 1.0f};
    host.transform_vec4_inverse_110(comp.entity_14, world_query, h);
    const Vec3 local_query = homogeneous_divide_vec4(h);
    return host.distance_to_polyline_local_007ae7b0(local_query);
}

// ============================================================================
// 3. PathReplicator & SessionMessage_PATH_CREATE
// ============================================================================

// 007AD5C0 BSP_PathReplicator_SendPathSet
// Address: 007AD5C0-007AD65B
void path_replicator_send_path_set_007ad5c0(int index, const Vec3& pt, PathReplicatorHost& host) {
    host.route_path_set_d9(index, pt);
}

// 007AD660 BSP_PathReplicator_SendPathAdd
// Address: 007AD660-007AD6F3
void path_replicator_send_path_add_007ad660(const Vec3& pt, PathReplicatorHost& host) {
    host.route_path_add_da(pt);
}

// 007AD700 BSP_PathReplicator_SendPathInsert
// Address: 007AD700-007AD79B
void path_replicator_send_path_insert_007ad700(int index, const Vec3& pt, PathReplicatorHost& host) {
    host.route_path_insert_db(index, pt);
}

// 007AD7A0 BSP_PathReplicator_SendPathDelete
// Address: 007AD7A0-007AD817
void path_replicator_send_path_delete_007ad7a0(int index, PathReplicatorHost& host) {
    host.route_path_delete_dc(index);
}

// 007B2950 BSP_SessionMessage_PATH_CREATE_Construct
// Address: 007B2950-007B2AAC
void session_message_path_create_construct_007b2950(
    SessionMessagePathCreate& msg,
    const void* entity_opaque,
    const std::vector<const Vec3*>& entity_points_318,
    PathCreateBitStreamHost& host) {
    host.construct_net_entity_create_base_00766fc0(0xD8u, entity_opaque);
    msg.vtable_00 = 0x00D0377Cu;
    msg.points_114.assign(entity_points_318.size(), Vec3{});
    for (std::size_t i = 0; i < msg.points_114.size(); ++i) {
        if (i >= entity_points_318.size() || entity_points_318[i] == nullptr) {
            host.on_invalid_parameter();
            break;
        }
        msg.points_114[i] = *entity_points_318[i];
    }
}

// 007B0BF0 BSP_SessionMessage_PATH_CREATE_Write
// Address: 007B0BF0-007B0C85
void session_message_path_create_write_007b0bf0(
    const SessionMessagePathCreate& msg,
    void* bit_cursor,
    PathCreateBitStreamHost& host) {
    host.write_net_entity_create_base_00765000(bit_cursor);
    const float max_val = std::numeric_limits<float>::max();
    for (std::size_t i = 0; i < msg.points_114.size(); ++i) {
        host.write_float_array_00429790(bit_cursor, msg.points_114[i], 3, 0, 1, max_val, 32);
    }
}

// ============================================================================
// 4. PathSource Hierarchy & PathCursor Operations
// ============================================================================

// 007B0E30 BSP_SimplePathSource_PointCount
// Address: 007B0E30-007B0E52
int simple_path_source_point_count_007b0e30(const SimplePathSource& src) noexcept {
    if (src.slot_points_08 == nullptr || src.slot_points_08->empty()) {
        return 0;
    }
    return static_cast<int>(src.slot_points_08->size());
}

// 007B0F00 BSP_SimplePathSource_Save
// Address: 007B0F00-007B0FEF
void simple_path_source_save_007b0f00(const SimplePathSource& src, PathArchiveHost& archive) {
    archive.write_named_string(0, "", 0, "simple");
    if (src.slot_points_08 == nullptr) {
        return;
    }
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(src.slot_points_08->size()); ++i) {
        const Vec3 pt = (*src.slot_points_08)[i];
        archive.write_indexed_vec3(1, i, 5, pt);
    }
}

// 007B1030 BSP_AuthoredPathSource_PointCount
// Address: 007B1030-007B1048
int authored_path_source_point_count_007b1030(const AuthoredPathSource& src) noexcept {
    if (src.scene_path_08 == nullptr || src.scene_path_08->points_08.empty()) {
        return 0;
    }
    return static_cast<int>(src.scene_path_08->points_08.size());
}

// 007B10A0 BSP_AuthoredPathSource_GetPoint
// Address: 007B10A0-007B10B8
Vec3& authored_path_source_get_point_007b10a0(
    const AuthoredPathSource& src,
    Vec3& out,
    int index,
    AuthoredPathWorldHost& host) {
    out = host.transform_point_to_world_007af800(src.scene_path_08, index);
    return out;
}

// 007B10C0 BSP_AuthoredPathSource_LastPoint
// Address: 007B10C0-007B10FF
Vec3& authored_path_source_last_point_007b10c0(
    const AuthoredPathSource& src,
    Vec3& out,
    AuthoredPathWorldHost& host) {
    int last_index = -1;
    if (src.scene_path_08 != nullptr && !src.scene_path_08->points_08.empty()) {
        last_index = static_cast<int>(src.scene_path_08->points_08.size()) - 1;
    }
    out = host.transform_point_to_world_007af800(src.scene_path_08, last_index);
    return out;
}

// 007B1100 BSP_AuthoredPathSource_NearestIndex
// Address: 007B1100-007B1111
int authored_path_source_nearest_index_007b1100(
    const AuthoredPathSource& src,
    const Vec3& pos,
    AuthoredPathWorldHost& host) {
    return host.nearest_point_index_world_007b0900(src.scene_path_08, pos, nullptr);
}

// 007B1120 BSP_AuthoredPathSource_Load
// Address: 007B1120-007B1163
void authored_path_source_load_007b1120(
    AuthoredPathSource& src,
    PathArchiveHost& archive) {
    std::uint32_t path_id = 0;
    archive.read_named_u32(0, "pathID", 4, path_id);
    src.scene_path_08 = static_cast<PathComponentState*>(archive.resolve_path_interface_007ac9d0(path_id));
}

// 007B1170 BSP_AuthoredPathSource_Save
// Address: 007B1170-007B11E0
void authored_path_source_save_007b1170(
    const AuthoredPathSource& src,
    PathArchiveHost& archive) {
    archive.write_named_string(0, "", 0, "entity");
    const std::uint32_t entity_id = (src.scene_path_08 != nullptr) ? src.scene_path_08->entity_14 : 0u;
    archive.write_named_u32(0, "pathID", 4, entity_id);
}

// 007B2250 BSP_EntityPathSource_CreateForSlot
// Address: 007B2250-007B2294
SimplePathSource** entity_path_source_create_for_slot_007b2250(
    SimplePathSource*& out_ptr,
    const std::vector<Vec3>* slot_points) {
    SimplePathSource* obj = new (std::nothrow) SimplePathSource{};
    if (obj != nullptr) {
        obj->vtable_00 = 0x00CEB130u;
        obj->slot_points_08 = slot_points;
        out_ptr = obj;
        obj->ref_count_04 = 1;
        obj->vtable_00 = 0x00D05318u;
        return &out_ptr;
    }
    out_ptr = nullptr;
    return &out_ptr;
}

// 007B22A0 BSP_EntityPathSource_CreateForEntity
// Address: 007B22A0-007B22E4
AuthoredPathSource** entity_path_source_create_for_entity_007b22a0(
    AuthoredPathSource*& out_ptr,
    PathComponentState* scene_path) {
    AuthoredPathSource* obj = new (std::nothrow) AuthoredPathSource{};
    if (obj != nullptr) {
        obj->vtable_00 = 0x00CEB130u;
        obj->scene_path_08 = scene_path;
        out_ptr = obj;
        obj->ref_count_04 = 1;
        obj->vtable_00 = 0x00D0534Cu;
        return &out_ptr;
    }
    out_ptr = nullptr;
    return &out_ptr;
}

// 007B2AB0 BSP_OwnedPointsPathSource_Construct
// Address: 007B2AB0-007B2B08
void owned_points_path_source_construct_007b2ab0(
    OwnedPointsPathSource& src,
    const std::vector<Vec3>& points) {
    src.vtable_00 = 0x00CEB130u;
    src.ref_count_04 = 1;
    src.vtable_00 = 0x00D05440u;
    src.points_0c = points;
}

// 007B2C00 BSP_OwnedPointsPathSource_Save
// Address: 007B2C00-007B2CE8
void owned_points_path_source_save_007b2c00(
    const OwnedPointsPathSource& src,
    PathArchiveHost& archive) {
    archive.write_named_string(0, "", 0, "simple");
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(src.points_0c.size()); ++i) {
        const Vec3 pt = src.points_0c[i];
        archive.write_indexed_vec3(1, i, 5, pt);
    }
}

// 007B2CF0 BSP_OwnedPointsPathSource_FreePoints
// Address: 007B2CF0-007B2D26
void owned_points_path_source_free_points_007b2cf0(OwnedPointsPathSource& src) noexcept {
    src.points_0c.clear();
    src.points_0c.shrink_to_fit();
    src.vtable_00 = 0x00D052DCu;
}

// 007B2D80 BSP_OwnedPointsPathSource_ConstructEmpty
// Address: 007B2D80-007B2DC4
void owned_points_path_source_construct_empty_007b2d80(OwnedPointsPathSource& src) noexcept {
    src.vtable_00 = 0x00CEB130u;
    src.ref_count_04 = 1;
    src.vtable_00 = 0x00D05440u;
    src.points_0c.clear();
}

// 007ADB90 BSP_PathCursor_PointAtIndex
// Address: 007ADB90-007ADC19
Vec3& path_cursor_point_at_index_007adb90(
    const PathCursorState& cursor,
    Vec3& out,
    int index) noexcept {
    if (cursor.source_04 == nullptr) {
        out = Vec3{0.0f, 0.0f, 0.0f};
        return out;
    }
    if (index < 0) {
        index = cursor.index_08;
    }
    out = cursor.source_04->get_point_vslot08(index);
    return out;
}

// 007ADC60 BSP_EntityCommand_IsOnFinalLeg
// Address: 007ADC60-007ADCBD
bool entity_command_is_on_final_leg_007adc60(const PathCursorState& cursor) noexcept {
    if (cursor.source_04 == nullptr) {
        return true;
    }
    const int count = cursor.source_04->point_count_vslot0c();
    if (count <= 0) {
        return true;
    }
    if (cursor.follow_mode_0c != 1) {
        return false;
    }
    if (cursor.forward_10) {
        const int count_check = cursor.source_04->point_count_vslot0c();
        if (cursor.index_08 == count_check - 1) {
            return true;
        }
        if (cursor.forward_10) {
            return false;
        }
    }
    return cursor.index_08 == 0;
}

// 007ADCC0 BSP_EntityCommand_PathCursorNextIndex
// Address: 007ADCC0-007ADD4A
int entity_command_path_cursor_next_index_007adcc0(PathCursorState& cursor) noexcept {
    const int mode = cursor.follow_mode_0c;
    const int idx = cursor.index_08;
    if (mode == 1) {
        return cursor.forward_10 ? (idx + 1) : (idx - 1);
    }
    if (mode == 2) {
        if (idx == 0) {
            cursor.forward_10 = true;
            return 1;
        }
        const int count = cursor.source_04->point_count_vslot0c();
        if (idx == count - 1) {
            cursor.forward_10 = false;
            return idx - 1;
        }
        return cursor.forward_10 ? (idx + 1) : (idx - 1);
    }
    if (mode == 3) {
        if (cursor.forward_10) {
            const int count = cursor.source_04->point_count_vslot0c();
            if (idx == count - 1) {
                return 0;
            }
            return idx + 1;
        }
        if (idx == 0) {
            return cursor.source_04->point_count_vslot0c() - 1;
        }
        return idx - 1;
    }
    return idx;
}

// 007ADD50 BSP_PathCursor_AdvanceToNextIndex
// Address: 007ADD50-007ADD65
void path_cursor_advance_to_next_index_007add50(
    PathCursorState& cursor,
    PathCursorHost& host) {
    const int next_idx = entity_command_path_cursor_next_index_007adcc0(cursor);
    host.cursor_set_index_vslot08(cursor, next_idx);
}

// 007AE060 BSP_PathCursor_SetIndexFromMessage
// Address: 007AE060-007AE09B
void path_cursor_set_index_from_message_007ae060(
    PathCursorState& cursor,
    int new_index) noexcept {
    if (cursor.source_04 != nullptr) {
        const int count = cursor.source_04->point_count_vslot0c();
        if (count > 0) {
            const int count2 = cursor.source_04->point_count_vslot0c();
            if (new_index <= count2 - 1) {
                cursor.index_08 = new_index;
            }
        }
    }
}

// 007B1320 BSP_PathCursor_NextPoint
// Address: 007B1320-007B1340
Vec3& path_cursor_next_point_007b1320(
    PathCursorState& cursor,
    Vec3& out,
    PathCursorHost& host) {
    const int next_idx = entity_command_path_cursor_next_index_007adcc0(cursor);
    out = host.cursor_get_point_vslot04(cursor, next_idx);
    return out;
}

// 007B1350 BSP_PathCursor_FollowModeLastPoint
// Address: 007B1350-007B13C2
void path_cursor_follow_mode_last_point_007b1350(
    const PathCursorState& cursor,
    FollowModeLastPointResult& out,
    PathCursorHost& host) {
    if (cursor.follow_mode_0c == 1 && cursor.source_04 != nullptr) {
        const int count = cursor.source_04->point_count_vslot0c();
        if (count > 0) {
            const int count2 = cursor.source_04->point_count_vslot0c();
            const Vec3 pt = host.cursor_get_point_vslot04(cursor, count2 - 1);
            out.point.x = pt.x;
            out.valid_0c = true;
            out.point.y = pt.y;
            out.point.z = pt.z;
            return;
        }
    }
    out.valid_0c = false;
}

// 007B19B0 BSP_PathSourceRef_Assign
// Address: 007B19B0-007B19EF
PathSourceBase** path_source_ref_assign_007b19b0(
    PathSourceBase*& ref_slot,
    PathSourceBase* new_src) noexcept {
    PathSourceBase* old_src = ref_slot;
    if (old_src != nullptr) {
        if (--old_src->ref_count_04 == 0) {
            delete old_src;
        }
        ref_slot = nullptr;
    }
    ref_slot = new_src;
    return &ref_slot;
}

// 007B1BF0 BSP_PathSourceRef_CopyFrom
// Address: 007B1BF0-007B1C45
void path_source_ref_copy_from_007b1bf0(
    PathCursorState& dst,
    const PathCursorState& src) noexcept {
    PathSourceBase* new_src = src.source_04;
    PathSourceBase* old_src = dst.source_04;
    if (old_src != new_src) {
        dst.source_04 = new_src;
        if (new_src != nullptr) {
            ++new_src->ref_count_04;
        }
        if (old_src != nullptr && --old_src->ref_count_04 == 0) {
            delete old_src;
        }
    }
    dst.index_08 = src.index_08;
    dst.follow_mode_0c = src.follow_mode_0c;
    dst.forward_10 = src.forward_10;
}

// 007B1D90 BSP_PathSourceRef_Release
// Address: 007B1D90-007B1DD3
void path_source_ref_release_007b1d90(PathCursorState& cursor) noexcept {
    PathSourceBase* old_src = cursor.source_04;
    if (old_src != nullptr) {
        if (--old_src->ref_count_04 == 0) {
            delete old_src;
        }
        cursor.source_04 = nullptr;
    }
    cursor.source_04 = nullptr;
    cursor.index_08 = 0;
    cursor.follow_mode_0c = 1;
    cursor.forward_10 = true;
}

// 007B1C50 BSP_EntityCommand_PathCursorStart
// Address: 007B1C50-007B1D2E
void entity_command_path_cursor_start_007b1c50(
    PathCursorState& cursor,
    PathSourceBase* holder_source,
    const Vec3& pos,
    float radius,
    int follow_mode,
    int start_mode,
    PathCursorHost& host) {
    PathSourceBase* old_src = cursor.source_04;
    if (old_src != holder_source) {
        cursor.source_04 = holder_source;
        if (holder_source != nullptr) {
            ++holder_source->ref_count_04;
        }
        if (old_src != nullptr && --old_src->ref_count_04 == 0) {
            delete old_src;
        }
    }
    cursor.follow_mode_0c = follow_mode;
    cursor.forward_10 = true;
    if (start_mode == 6) {
        cursor.index_08 = 0;
        return;
    }
    if (start_mode == 5 || start_mode == 7) {
        if (start_mode == 7) {
            const float draw = host.uniform_float_00bd2f10(1, 0.0f, 1.0f);
            cursor.forward_10 = (draw > 0.5f);
        } else {
            cursor.forward_10 = true;
        }
    } else if (start_mode == 8) {
        cursor.forward_10 = false;
    } else {
        return;
    }
    cursor.index_08 = cursor.source_04->start_index_for_pos_vslot10(pos, radius, cursor.forward_10);
}

// 007B1D30 BSP_EntityCommand_PathCursorStartThunk
// Address: 007B1D30-007B1D54
void entity_command_path_cursor_start_thunk_007b1d30(
    PathCursorState& cursor,
    PathSourceBase* holder_source,
    const Vec3& pos,
    float radius,
    const int params[2],
    PathCursorHost& host) {
    entity_command_path_cursor_start_007b1c50(
        cursor, holder_source, pos, radius, params[0], params[1], host);
}

// ============================================================================
// 5. MovCam & MovieCam Helpers
// ============================================================================

// 007A6270 BSP_MovCam_SetMode1FlagBit1
// Address: 007A6270-007A6307
void movcam_set_mode1_flag_bit1_007a6270(MovCamState& cam, bool flag, MovCamHost& host) {
    if (cam.target_38c != 0 && cam.mode_384 != 2) {
        if (cam.mode_384 != 0) {
            const int bit = flag ? 0 : 1;
            if (cam.mode_384 != 1) {
                cam.mode_384 = 1;
                cam.submode_388 = bit * 2;
                host.enter_mode_007a5110(cam, 0);
                host.step_subobject_vslot0c_170(cam);
                return;
            }
            cam.submode_388 = cam.submode_388 + (bit - (cam.submode_388 / 2)) * 2;
        }
        host.step_subobject_vslot0c_170(cam);
    }
}

// 007A6310 BSP_MovCam_SetMode1FlagBit0
// Address: 007A6310-007A63AA
void movcam_set_mode1_flag_bit0_007a6310(MovCamState& cam, bool flag, MovCamHost& host) {
    if (cam.target_38c != 0 && cam.mode_384 != 2) {
        if (cam.mode_384 != 0) {
            const int bit = flag ? 0 : 1;
            if (cam.mode_384 != 1) {
                cam.mode_384 = 1;
                cam.submode_388 = bit;
                host.enter_mode_007a5110(cam, 0);
                host.step_subobject_vslot0c_170(cam);
                return;
            }
            const int rem = cam.submode_388 % 2;
            cam.submode_388 = (bit - rem) + cam.submode_388;
        }
        host.step_subobject_vslot0c_170(cam);
    }
}

// 007A63B0 BSP_MovCam_SetRefPosPointImpl
// Address: 007A63B0-007A64FA
void movcam_set_ref_pos_point_impl_007a63b0(
    MovCamState& cam,
    const Vec3& pt,
    bool keep_mode,
    int transition_flag,
    MovCamHost& host) {
    if (!keep_mode) {
        cam.mode_384 = 4;
        cam.cam_ref_3f0.target_404 = 0;
        cam.cam_ref_3f0.offset_408 = pt;
        cam.cam_ref_3f0.radius_414 = 0.0f;
        if (cam.cam_ref_3f0.target_404 != 0) {
            host.observer_register_pair_00694a60(cam.cam_ref_3f0.target_404);
        }
        host.enter_mode_007a5110(cam, 0);
        return;
    }
    cam.current_pos_394 = pt;
    if (cam.mode_384 != 0 && cam.mode_384 != 1) {
        cam.mode_384 = 0;
        cam.submode_388 = 0;
        host.enter_mode_007a5110(cam, transition_flag);
    }
    host.step_subobject_vslot0c_170(cam);
}

// 007A68F0 BSP_MovCam_OnActivate
// Address: 007A68F0-007A6902
void movcam_on_activate_007a68f0(MovCamState& cam, MovCamHost& host) {
    host.inherit_race_and_activate_0042a910();
    host.enter_mode_007a5110(cam, 0);
}

// 007A6910 BSP_MovCam_SetTarget
// Address: 007A6910-007A6A20
void movcam_set_target_007a6910(MovCamState& cam, std::uint32_t entity, MovCamHost& host) {
    if (entity != 0 && host.entity_is_kind_of(entity, 0x18u)) {
        entity = host.entity_parent_3d0(entity);
    }
    if (entity == cam.look_target_390) {
        host.set_look_target_00797070(cam, 0, 0);
    }
    if (entity != cam.target_38c) {
        if (cam.target_38c != 0 && host.observer_is_pair_registered_00694af0(cam.target_38c)) {
            host.observer_unregister_pair_006952a0(cam.target_38c);
        }
        if (entity == 0) {
            cam.target_38c = 0;
        } else if (host.entity_is_kind_of(entity, 5u)) {
            cam.target_38c = entity;
        } else if (host.entity_is_kind_of(entity, 0x18u)) {
            cam.target_38c = host.entity_parent_3d0(entity);
        }
        if (cam.target_38c == 0) {
            if (cam.mode_384 == 1 || cam.mode_384 == 2) {
                cam.mode_384 = 0;
                return;
            }
        } else {
            if (cam.mode_384 == 0) {
                cam.mode_384 = 1;
            } else if (cam.mode_384 == 2) {
                cam.submode_388 = host.lookup_saved_deck_index_007a0ac0(cam.target_38c);
            }
            if (!host.observer_is_pair_registered_00694af0(cam.target_38c)) {
                host.observer_register_pair_00694a60(cam.target_38c);
            }
            host.enter_mode_007a5110(cam, 0);
        }
    }
}

// 007A6A30 BSP_MovCam_OnObservedReleased
// Address: 007A6A30-007A6B32
void movcam_on_observed_released_007a6a30(MovCamState& cam, std::uint32_t entity, MovCamHost& host) {
    if (entity == cam.target_38c) {
        if (cam.mode_384 == 1 || cam.mode_384 == 2 || cam.mode_384 == 6) {
            cam.mode_384 = 0;
        } else if (cam.mode_384 == 7) {
            cam.tompi_pos_offset_4c4 =
                host.transform_target_world_via_scratch_00414d10(cam.target_38c, cam.tompi_pos_offset_4c4);
        }
        movcam_set_target_007a6910(cam, 0, host);
    }
    if (entity == cam.look_target_390) {
        if (cam.mode_384 == 7) {
            cam.tompi_look_offset_4b8 =
                host.transform_target_local_point_004142e0(cam.look_target_390, cam.tompi_look_offset_4b8);
        }
        host.set_look_target_00797070(cam, 0, 0);
        if (cam.mode_384 == 6) {
            cam.mode_384 = 1;
        }
    }
    if (entity == cam.extra_target_3e4) {
        cam.extra_target_3e4 = 0;
    }
}

// 007A6B40 BSP_MovCam_SetHandModeTargeted
// Address: 007A6B40-007A6B82
void movcam_set_hand_mode_targeted_007a6b40(
    MovCamState& cam,
    std::uint32_t target,
    bool flag_a,
    bool flag_b,
    MovCamHost& host) {
    cam.mode_384 = 1;
    movcam_set_target_007a6910(cam, target, host);
    cam.submode_388 = (flag_a ? 0 : 2) + (flag_b ? 0 : 1);
    host.enter_mode_007a5110(cam, 0);
}

// 007A6B90 BSP_MovCam_SetHandModeDeck
// Address: 007A6B90-007A6BBD
void movcam_set_hand_mode_deck_007a6b90(
    MovCamState& cam,
    std::uint32_t target,
    int deck_index,
    MovCamHost& host) {
    cam.mode_384 = 2;
    movcam_set_target_007a6910(cam, target, host);
    cam.submode_388 = deck_index;
    host.enter_mode_007a5110(cam, 0);
}

// 007A6BC0 BSP_MovCam_SetHandModeOverview
// Address: 007A6BC0-007A6CCF
void movcam_set_hand_mode_overview_007a6bc0(
    MovCamState& cam,
    std::uint32_t target,
    int transition_flag,
    float duration,
    const char* cb_name,
    std::size_t cb_name_len,
    std::uint32_t cb_args,
    MovCamHost& host) {
    assign_callback_fields(cam, cb_name, cb_name_len, cb_args, host);
    cam.mode_384 = 3;
    movcam_set_target_007a6910(cam, target, host);
    host.enter_mode_007a5110(cam, transition_flag);
    apply_camera_ref_speed_limit(cam, transition_flag, duration, host);
}

// 007A6CD0 BSP_MovCam_SetRefPos
// Address: 007A6CD0-007A6E51
void movcam_set_ref_pos_007a6cd0(
    MovCamState& cam,
    std::uint32_t target,
    const Vec3& offset,
    int transition_flag,
    float duration,
    const char* cb_name,
    std::size_t cb_name_len,
    std::uint32_t cb_args,
    MovCamHost& host) {
    assign_callback_fields(cam, cb_name, cb_name_len, cb_args, host);
    cam.mode_384 = 4;
    movcam_set_target_007a6910(cam, target, host);
    cam.cam_ref_3f0 = host.construct_camera_ref_point_00793920(target, offset);
    if (cam.cam_ref_3f0.target_404 != 0) {
        host.observer_register_pair_00694a60(cam.cam_ref_3f0.target_404);
    }
    host.enter_mode_007a5110(cam, transition_flag);
    apply_camera_ref_speed_limit(cam, transition_flag, duration, host);
}

// 007A6E60 BSP_MovCam_SetRefPosPolar
// Address: 007A6E60-007A7060
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
    MovCamHost& host) {
    assign_callback_fields(cam, cb_name, cb_name_len, cb_args, host);
    const float sin_a = to_f32(std::sin(static_cast<double>(angle_a)));
    const float cos_b1 = to_f32(std::cos(static_cast<double>(angle_b)));
    const float x = to_f32(static_cast<double>(-sin_a) * static_cast<double>(r) * static_cast<double>(cos_b1));
    const float sin_b = to_f32(std::sin(static_cast<double>(angle_b)));
    const float y = to_f32(static_cast<double>(sin_b) * static_cast<double>(r));
    const float cos_a = to_f32(std::cos(static_cast<double>(angle_a)));
    const float cos_b2 = to_f32(std::cos(static_cast<double>(angle_b)));
    cam.mode_384 = 4;
    const float z = to_f32(static_cast<double>(cos_a) * static_cast<double>(r) * static_cast<double>(cos_b2));
    const Vec3 offset{x, y, z};
    movcam_set_target_007a6910(cam, target, host);
    cam.cam_ref_3f0 = host.construct_camera_ref_point_00793920(target, offset);
    if (cam.cam_ref_3f0.target_404 != 0) {
        host.observer_register_pair_00694a60(cam.cam_ref_3f0.target_404);
    }
    host.enter_mode_007a5110(cam, transition_flag);
    apply_camera_ref_speed_limit(cam, transition_flag, duration, host);
}

// 007A7070 BSP_MovCam_SetRefPosRadius
// Address: 007A7070-007A71F4
void movcam_set_ref_pos_radius_007a7070(
    MovCamState& cam,
    std::uint32_t target,
    float radius,
    int transition_flag,
    float duration,
    const char* cb_name,
    std::size_t cb_name_len,
    std::uint32_t cb_args,
    MovCamHost& host) {
    assign_callback_fields(cam, cb_name, cb_name_len, cb_args, host);
    cam.mode_384 = 4;
    movcam_set_target_007a6910(cam, target, host);
    cam.cam_ref_3f0 = host.construct_camera_ref_radius_007939b0(target, radius);
    if (cam.cam_ref_3f0.target_404 != 0) {
        host.observer_register_pair_00694a60(cam.cam_ref_3f0.target_404);
    }
    host.enter_mode_007a5110(cam, transition_flag);
    apply_camera_ref_speed_limit(cam, transition_flag, duration, host);
}

// 007A7DE0 BSP_MovCam_SetViktorMode
// Address: 007A7DE0-007A7F26
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
    MovCamHost& host) {
    assign_callback_fields(cam, cb_name, cb_name_len, cb_args, host);
    cam.viktor_angles_488.x = angle_a;
    cam.viktor_angles_488.y = angle_b;
    cam.mode_384 = 0;
    movcam_set_target_007a6910(cam, 0, host);
    host.set_look_target_00797070(cam, look_target, 0);
    movcam_set_target_007a6910(cam, target, host);
    cam.mode_384 = 6;
    host.enter_mode_007a5110(cam, transition_flag);
    apply_camera_ref_speed_limit(cam, transition_flag, duration, host);
}

// 007A7F30 BSP_MovCam_SetTompiModeWeak
// Address: 007A7F30-007A80F1
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
    MovCamHost& host) {
    assign_callback_fields(cam, cb_name, cb_name_len, cb_args, host);
    cam.mode_384 = 0;
    movcam_set_target_007a6910(cam, 0, host);
    host.set_look_target_00797070(cam, look_target, 0);
    movcam_set_target_007a6910(cam, target, host);
    cam.mode_384 = 4;
    cam.cam_ref_3f0 = host.construct_camera_ref_point_00793920(target, pos_offset);
    if (cam.cam_ref_3f0.target_404 != 0) {
        host.observer_register_pair_00694a60(cam.cam_ref_3f0.target_404);
    }
    host.enter_mode_007a5110(cam, transition_flag);
    cam.tompi_flag_4b4 = true;
    cam.tompi_look_offset_4b8 = look_offset;
    apply_camera_ref_speed_limit(cam, transition_flag, duration, host);
}

// 007A8100 BSP_MovCam_SetTompiMode
// Address: 007A8100-007A8174
void movcam_set_tompi_mode_007a8100(
    MovCamState& cam,
    std::uint32_t target,
    const Vec3& pos_offset,
    std::uint32_t look_target,
    const Vec3& look_offset,
    MovCamHost& host) {
    cam.mode_384 = 7;
    movcam_set_target_007a6910(cam, target, host);
    host.set_look_target_00797070(cam, look_target, 0);
    host.enter_mode_007a5110(cam, 1);
    cam.tompi_pos_offset_4c4 = pos_offset;
    cam.tompi_flag_4b4 = true;
    cam.tompi_look_offset_4b8 = look_offset;
}

// 007A8180 BSP_MovCam_PickDeckPosition
// Address: 007A8180-007A81D1
void movcam_pick_deck_position_007a8180(
    MovCamState& cam,
    int group,
    std::uint32_t target,
    std::uint32_t look_target,
    MovCamHost& host) {
    movcam_set_target_007a6910(cam, target, host);
    host.set_look_target_00797070(cam, look_target, 0);
    cam.mode_384 = 2;
    cam.submode_388 = host.pick_random_deck_camera_007a49a0(cam.target_38c, cam.look_target_390, group);
    host.enter_mode_007a5110(cam, 0);
}

// 007A82C0 BSP_MovCam_SetRefPosPoint
// Address: 007A82C0-007A8411
void movcam_set_ref_pos_point_007a82c0(
    MovCamState& cam,
    std::uint32_t target,
    const Vec3& pt,
    bool keep_mode,
    int transition_flag,
    MovCamHost& host) {
    if (!keep_mode) {
        cam.mode_384 = 4;
        cam.cam_ref_3f0 = host.construct_camera_ref_point_00793920(target, pt);
        if (cam.cam_ref_3f0.target_404 != 0) {
            host.observer_register_pair_00694a60(cam.cam_ref_3f0.target_404);
        }
        host.enter_mode_007a5110(cam, 0);
        return;
    }
    movcam_set_target_007a6910(cam, target, host);
    cam.current_pos_394 = host.transform_target_local_point_004142e0(cam.target_38c, pt);
    movcam_set_ref_pos_point_impl_007a63b0(cam, cam.current_pos_394, keep_mode, transition_flag, host);
}

// 007A8420 BSP_MovCam_SetRefPosSpherical
// Address: 007A8420-007A84C4
void movcam_set_ref_pos_spherical_007a8420(
    MovCamState& cam,
    std::uint32_t target,
    float r,
    float angle_a,
    float angle_b,
    bool keep_mode,
    int transition_flag,
    MovCamHost& host) {
    const float sin_a = to_f32(std::sin(static_cast<double>(angle_a)));
    const float cos_b1 = to_f32(std::cos(static_cast<double>(angle_b)));
    const float x = to_f32(static_cast<double>(-sin_a) * static_cast<double>(r) * static_cast<double>(cos_b1));
    const float sin_b = to_f32(std::sin(static_cast<double>(angle_b)));
    const float y = to_f32(static_cast<double>(sin_b) * static_cast<double>(r));
    const float cos_a = to_f32(std::cos(static_cast<double>(angle_a)));
    const float cos_b2 = to_f32(std::cos(static_cast<double>(angle_b)));
    const float z = to_f32(static_cast<double>(cos_a) * static_cast<double>(r) * static_cast<double>(cos_b2));
    movcam_set_ref_pos_point_007a82c0(cam, target, Vec3{x, y, z}, keep_mode, transition_flag, host);
}

// 007AADB0 BSP_MovieCamLookat_Activate
// Address: 007AADB0-007AADC0
void movie_cam_lookat_activate_007aadb0(MovieCamEntityState& entity) {
    entity.inherit_race_and_activate_thunk_009277e0();
    entity.field_58 = 0;
}

// 007AADD0 BSP_MovieCamPos_Activate
// Address: 007AADD0-007AADE0
void movie_cam_pos_activate_007aadd0(MovieCamEntityState& entity) {
    entity.inherit_race_and_activate_thunk_009277e0();
    entity.field_58 = 0;
}

// ============================================================================
// 6. MParatrooper Residue & Lua Float Field Readers
// ============================================================================

// 007AAE50 BSP_LuaObject_GetFloatField_007aae50
// Address: 007AAE50-007AAEC0
void lua_object_get_float_field_007aae50(const char* field_name, float& out, LuaTableReaderHost& host) {
    out = to_f32(host.get_number_by_name(field_name));
}

// 007AAED0 BSP_LuaObject_GetFloatField_007aaed0
// Address: 007AAED0-007AAF40
void lua_object_get_float_field_007aaed0(const char* field_name, float& out, LuaTableReaderHost& host) {
    out = to_f32(host.get_number_by_name(field_name));
}

// 007AAF50 BSP_LuaObject_GetFloatField_007aaf50
// Address: 007AAF50-007AAFC0
void lua_object_get_float_field_007aaf50(const char* field_name, float& out, LuaTableReaderHost& host) {
    out = to_f32(host.get_number_by_name(field_name));
}

// 007AAFD0 BSP_LuaObject_GetFloatField_007aafd0
// Address: 007AAFD0-007AB040
void lua_object_get_float_field_007aafd0(const char* field_name, float& out, LuaTableReaderHost& host) {
    out = to_f32(host.get_number_by_name(field_name));
}

// 007AC780 BSP_MParatrooperClass_ReadLuaFields
// Address: 007AC780-007AC944
void mparatrooper_class_read_lua_fields_007ac780(
    ParatrooperClassResidue& cls,
    LuaTableReaderHost& host) {
    host.read_bomb_class_lua_fields_006e1be0();
    lua_object_get_float_field_007aae50("SlowFactorClosed", cls.slow_factor_closed_dc, host);
    lua_object_get_float_field_007aae50("SlowFactorOpened", cls.slow_factor_opened_e0, host);
    lua_object_get_float_field_007aaed0("OpenDuration", cls.open_duration_e4, host);
    host.read_number_field_007003b0("CloseDuration", cls.close_duration_e8);
    lua_object_get_float_field_007aaf50("CaptureDuration", cls.capture_duration_f8, host);
    const char* soldier_class_name = host.get_string_by_name("SoldierClass");
    cls.soldier_class_ec = host.get_or_load_soldier_class_004b1400(soldier_class_name);
    host.read_string_field_004fe080("SoldierAnim", cls.soldier_anim_f0);
    lua_object_get_float_field_007aafd0("Damage", cls.damage_fc, host);
    lua_object_get_float_field_007aaed0("CapturePower", cls.capture_power_d8, host);
}

// 007AB7D0 BSP_MParatrooper_SendCreateMessage
// Address: 007AB7D0-007AB894
void mparatrooper_send_create_message_007ab7d0(
    std::uint32_t peer_id,
    std::uint32_t class_id_0c,
    const Vec3& target_47c,
    ParatrooperResidueHost& host) {
    host.send_paratrooper_create_msg_b9(peer_id, class_id_0c, target_47c);
}

// 007AB8A0 BSP_MParatrooper_SetLandingPosAdjust120
// Address: 007AB8A0-007AB8B2
void mparatrooper_set_landing_pos_adjust_120_007ab8a0(
    const Vec3& msg_landing_pos_120,
    std::uint32_t entity,
    ParatrooperResidueHost& host) {
    host.set_landing_pos_007ab230(entity, msg_landing_pos_120);
}

// 007AB9D0 BSP_MParatrooper_GetFlagPairFalse
// Address: 007AB9D0-007AB9E0
void mparatrooper_get_flag_pair_false_007ab9d0(bool& out_a, bool& out_b) noexcept {
    out_a = false;
    out_b = false;
}

// 007ABAB0 BSP_MParatrooper_RegisterInWorldLists
// Address: 007ABAB0-007ABAE3
void mparatrooper_register_in_world_lists_007abab0(ParatrooperResidueHost& host) {
    host.register_in_parent_entity_list_00928560();
    host.world_list_push_back_00484540(0x30u);
    host.world_list_push_back_00484540(0x210u);
    host.world_list_push_back_00484540(0x264u);
}

// 007ABAF0 BSP_MParatrooper_UnregisterFromWorldLists
// Address: 007ABAF0-007ABB25
void mparatrooper_unregister_from_world_lists_007abaf0(ParatrooperResidueHost& host) {
    host.detach_bomb_family_from_world_lists_006e29c0();
    host.world_list_264_erase_self_004837d0();
}

// 007AC4C0 BSP_MParatrooperClass_Create
// Address: 007AC4C0-007AC52E
void mparatrooper_class_create_007ac4c0(
    const ParatrooperClassResidue& cls,
    std::uint32_t spawn_param,
    ParatrooperCreatedEntity& out_entity,
    ParatrooperResidueHost& host) {
    host.construct_paratrooper_007ab8c0(spawn_param);
    out_entity.cls_314 = &cls;
}

// 007AC2C0 BSP_MParatrooperClass_CreateWithPose
// Address: 007AC2C0-007AC4B8
void mparatrooper_class_create_with_pose_007ac2c0(
    const ParatrooperClassResidue& cls,
    std::uint32_t spawn_param,
    std::uint32_t side,
    const Vec3& pos,
    const Vec3& dir,
    ParatrooperCreatedEntity& out_entity,
    ParatrooperResidueHost& host) {
    host.construct_paratrooper_007ab8c0(spawn_param);
    out_entity.side_54 = side;
    out_entity.cls_314 = &cls;
    host.attach_to_world_scene_vslot98();
    const float speed = cls.initial_speed_50;
    out_entity.velocity_318 = Vec3{
        to_f32(static_cast<double>(speed) * static_cast<double>(dir.x)),
        to_f32(static_cast<double>(speed) * static_cast<double>(dir.y)),
        to_f32(static_cast<double>(speed) * static_cast<double>(dir.z)),
    };
    out_entity.forward_94 = dir;
    host.orthonormalize_basis_rows_0085dc80(out_entity.forward_94);
    out_entity.pos_a4 = pos;
    out_entity.bullet_pos_370 = pos;
    out_entity.bullet_prev_pos_37c = pos;
    out_entity.pose_valid_c8 = false;
    out_entity.inv_pose_valid_10c = false;
    host.invalidate_subtree_pose_0042ed50();
}

}  // namespace bsp::path_movcam_seg46_argon

namespace bsp::path_movcam_seg46_argon {
// 007AE930..007AED09 (986 bytes). Original ECX=array, stack origin/dir,
// ST0=float, RET8. EDX adds the immutable borrowed target table.
// Reserve 16 extra tail bytes to save EBP without changing original
// call-site alignment modulo16. Original locals keep every offset;
// only entry argument loads shift by10h. All 280 original instructions
// retain their schedule, apart from the explicit frame/target binding.
__declspec(naked) float __fastcall point3_ptr_array_distance_to_line_007ae930(
    const void*, const PointLineDistanceTargets_007ae930*, const Vec3*, const Vec3*) {
    __asm {
        SUB ESP,0x94 // 007ae930
        mov dword ptr [esp + 0x84], ebp // added save outside original locals
        mov ebp, edx // borrowed immutable native target bindings
        PUSH EBX // 007ae936
        PUSH ESI // 007ae937
        PUSH EDI // 007ae938
        MOV EDI,dword ptr [ESP + 0xa8] // 007ae939
        MOV ESI,ECX // 007ae940
        MOV ECX,EDI // 007ae942
        CALL dword ptr [EBP + 0x0] // 007ae944
        FSTP dword ptr [ESP + 0x14] // 007ae949
        FLDZ // 007ae94d
        FLD dword ptr [ESP + 0x14] // 007ae94f
        FCOMI st(0),st(1) // 007ae953
        FSTP st(1) // 007ae955
        JBE loc_007ae963 // 007ae957
        FLD1 // 007ae959
        FDIVRP st(1), st(0) // 007ae95b
        FSTP dword ptr [ESP + 0xc] // 007ae95d
        JMP loc_007ae96e // 007ae961
    loc_007ae963:
        XORPS XMM0,XMM0 // 007ae963
        FSTP st(0) // 007ae966
        MOVSS dword ptr [ESP + 0xc],XMM0 // 007ae968
    loc_007ae96e:
        FLD dword ptr [EDI] // 007ae96e
        MOV EAX,dword ptr [ESI + 0x8] // 007ae970
        TEST EAX,EAX // 007ae973
        FLD dword ptr [ESP + 0xc] // 007ae975
        FLD st(0) // 007ae979
        FMULP st(2), st(0) // 007ae97b
        FXCH st(1) // 007ae97d
        FSTP dword ptr [ESP + 0x14] // 007ae97f
        FLD dword ptr [EDI + 0x4] // 007ae983
        FMUL st(0), st(1) // 007ae986
        FSTP dword ptr [ESP + 0xc] // 007ae988
        FMUL dword ptr [EDI + 0x8] // 007ae98c
        FSTP dword ptr [ESP + 0x10] // 007ae98f
        FLD dword ptr [ESP + 0x14] // 007ae993
        FSTP dword ptr [ESP + 0x30] // 007ae997
        FLD dword ptr [ESP + 0xc] // 007ae99b
        FSTP dword ptr [ESP + 0x34] // 007ae99f
        FLD dword ptr [ESP + 0x10] // 007ae9a3
        FSTP dword ptr [ESP + 0x38] // 007ae9a7
        JZ loc_007ae9b7 // 007ae9ab
        MOV ECX,dword ptr [ESI + 0xc] // 007ae9ad
        SUB ECX,EAX // 007ae9b0
        SAR ECX,0x2 // 007ae9b2
        JNZ loc_007ae9bc // 007ae9b5
    loc_007ae9b7:
        CALL dword ptr [EBP + 0xc] // 007ae9b7
    loc_007ae9bc:
        MOV EDX,dword ptr [ESI + 0x8] // 007ae9bc
        MOV EAX,dword ptr [EDX] // 007ae9bf
        MOVSS XMM0,dword ptr [EAX] // 007ae9c1
        MOV EBX,dword ptr [ESP + 0xa4] // 007ae9c5
        MOVSS dword ptr [ESP + 0x18],XMM0 // 007ae9cc
        MOVSS XMM0,dword ptr [EAX + 0x4] // 007ae9d2
        MOVSS dword ptr [ESP + 0x1c],XMM0 // 007ae9d7
        MOVSS XMM0,dword ptr [EAX + 0x8] // 007ae9dd
        LEA EAX,[ESP + 0x30] // 007ae9e2
        PUSH EAX // 007ae9e6
        MOV EDX,EBX // 007ae9e7
        LEA ECX,[ESP + 0x1c] // 007ae9e9
        MOVSS dword ptr [ESP + 0x24],XMM0 // 007ae9ed
        CALL dword ptr [EBP + 0x4] // 007ae9f3
        FSTP dword ptr [ESP + 0x10] // 007ae9f8
        MOV ECX,ESI // 007ae9fc
        MOV EDI,0x1 // 007ae9fe
        CALL dword ptr [EBP + 0x8] // 007aea03
        CMP EAX,EDI // 007aea08
        JLE loc_007aecfa // 007aea0a
    loc_007aea10:
        MOV ECX,dword ptr [ESI + 0x8] // 007aea10
        TEST ECX,ECX // 007aea13
        JZ loc_007aea23 // 007aea15
        MOV EAX,dword ptr [ESI + 0xc] // 007aea17
        SUB EAX,ECX // 007aea1a
        SAR EAX,0x2 // 007aea1c
        CMP EDI,EAX // 007aea1f
        JC loc_007aea28 // 007aea21
    loc_007aea23:
        CALL dword ptr [EBP + 0xc] // 007aea23
    loc_007aea28:
        MOV ECX,dword ptr [ESI + 0x8] // 007aea28
        MOV EAX,dword ptr [ECX + EDI*0x4] // 007aea2b
        MOVSS XMM0,dword ptr [EAX] // 007aea2e
        MOVSS dword ptr [ESP + 0x60],XMM0 // 007aea32
        MOVSS XMM0,dword ptr [EAX + 0x4] // 007aea38
        LEA EDX,[ESP + 0x30] // 007aea3d
        MOVSS dword ptr [ESP + 0x64],XMM0 // 007aea41
        MOVSS XMM0,dword ptr [EAX + 0x8] // 007aea47
        PUSH EDX // 007aea4c
        MOV EDX,EBX // 007aea4d
        LEA ECX,[ESP + 0x64] // 007aea4f
        MOVSS dword ptr [ESP + 0x6c],XMM0 // 007aea53
        CALL dword ptr [EBP + 0x4] // 007aea59
        FSTP dword ptr [ESP + 0xc] // 007aea5e
        FLD dword ptr [ESP + 0xc] // 007aea62
        FLD dword ptr [ESP + 0x10] // 007aea66
        FCOMIP st(0),st(1) // 007aea6a
        FSTP st(0) // 007aea6c
        JBE loc_007aea7c // 007aea6e
        MOVSS XMM0,dword ptr [ESP + 0xc] // 007aea70
        MOVSS dword ptr [ESP + 0x10],XMM0 // 007aea76
    loc_007aea7c:
        MOVSS XMM0,dword ptr [ESP + 0x30] // 007aea7c
        PUSH 0x0 // 007aea82
        LEA EAX,[ESP + 0x88] // 007aea84
        PUSH EAX // 007aea8b
        SUB ESP,0xc // 007aea8c
        MOV EAX,ESP // 007aea8f
        MOVSS dword ptr [EAX],XMM0 // 007aea91
        MOVSS XMM0,dword ptr [ESP + 0x48] // 007aea95
        MOVSS dword ptr [EAX + 0x4],XMM0 // 007aea9b
        MOVSS XMM0,dword ptr [ESP + 0x4c] // 007aeaa0
        MOVSS dword ptr [EAX + 0x8],XMM0 // 007aeaa6
        MOV ECX,dword ptr [ESI + 0x8] // 007aeaab
        TEST ECX,ECX // 007aeaae
        JZ loc_007aeac1 // 007aeab0
        MOV EAX,dword ptr [ESI + 0xc] // 007aeab2
        SUB EAX,ECX // 007aeab5
        SAR EAX,0x2 // 007aeab7
        LEA ECX,[EDI - 0x1] // 007aeaba
        CMP ECX,EAX // 007aeabd
        JC loc_007aeac6 // 007aeabf
    loc_007aeac1:
        CALL dword ptr [EBP + 0xc] // 007aeac1
    loc_007aeac6:
        MOV EDX,dword ptr [ESI + 0x8] // 007aeac6
        MOV EAX,dword ptr [EDX + EDI*0x4 - 0x4] // 007aeac9
        MOVSS XMM0,dword ptr [EAX] // 007aeacd
        MOVSS dword ptr [ESP + 0x2c],XMM0 // 007aead1
        MOVSS XMM0,dword ptr [EAX + 0x4] // 007aead7
        MOV ECX,EDX // 007aeadc
        TEST ECX,ECX // 007aeade
        MOVSS dword ptr [ESP + 0x30],XMM0 // 007aeae0
        MOVSS XMM0,dword ptr [EAX + 0x8] // 007aeae6
        MOVSS dword ptr [ESP + 0x34],XMM0 // 007aeaeb
        JZ loc_007aeaff // 007aeaf1
        MOV EAX,dword ptr [ESI + 0xc] // 007aeaf3
        SUB EAX,ECX // 007aeaf6
        SAR EAX,0x2 // 007aeaf8
        CMP EDI,EAX // 007aeafb
        JC loc_007aeb04 // 007aeafd
    loc_007aeaff:
        CALL dword ptr [EBP + 0xc] // 007aeaff
    loc_007aeb04:
        MOV EAX,dword ptr [ESI + 0x8] // 007aeb04
        MOV EAX,dword ptr [EAX + EDI*0x4] // 007aeb07
        FLD dword ptr [EAX] // 007aeb0a
        FSTP dword ptr [ESP + 0x50] // 007aeb0c
        FLD dword ptr [EAX + 0x4] // 007aeb10
        FSTP dword ptr [ESP + 0x54] // 007aeb13
        FLD dword ptr [EAX + 0x8] // 007aeb17
        FSTP dword ptr [ESP + 0x58] // 007aeb1a
        FLD dword ptr [ESP + 0x50] // 007aeb1e
        FSUB dword ptr [ESP + 0x2c] // 007aeb22
        FSTP dword ptr [ESP + 0x38] // 007aeb26
        FLD dword ptr [ESP + 0x54] // 007aeb2a
        FSUB dword ptr [ESP + 0x30] // 007aeb2e
        FSTP dword ptr [ESP + 0x3c] // 007aeb32
        FLD dword ptr [ESP + 0x58] // 007aeb36
        FSUB dword ptr [ESP + 0x34] // 007aeb3a
        FSTP dword ptr [ESP + 0x40] // 007aeb3e
        FLD dword ptr [ESP + 0x3c] // 007aeb42
        FLD dword ptr [ESP + 0x38] // 007aeb46
        FLD dword ptr [ESP + 0x40] // 007aeb4a
        FLD st(2) // 007aeb4e
        FMULP st(3), st(0) // 007aeb50
        FXCH st(2) // 007aeb52
        FSTP dword ptr [ESP + 0x20] // 007aeb54
        FLD dword ptr [ESP + 0x20] // 007aeb58
        FLD st(1) // 007aeb5c
        FMULP st(2), st(0) // 007aeb5e
        FXCH st(1) // 007aeb60
        FSTP dword ptr [ESP + 0x20] // 007aeb62
        FADD dword ptr [ESP + 0x20] // 007aeb66
        FLD st(1) // 007aeb6a
        FMULP st(2), st(0) // 007aeb6c
        FXCH st(1) // 007aeb6e
        FSTP dword ptr [ESP + 0x20] // 007aeb70
        FADD dword ptr [ESP + 0x20] // 007aeb74
        FSTP dword ptr [ESP + 0x20] // 007aeb78
        FLD dword ptr [ESP + 0x20] // 007aeb7c
        CALL dword ptr [EBP + 0x10] // 007aeb80
        FSTP dword ptr [ESP + 0x20] // 007aeb85
        FLD dword ptr [ESP + 0x20] // 007aeb89
        FSTP dword ptr [ESP + 0x20] // 007aeb8d
        FLDZ // 007aeb91
        FLD dword ptr [ESP + 0x20] // 007aeb93
        FCOMI st(0),st(1) // 007aeb97
        FSTP st(1) // 007aeb99
        JBE loc_007aeba7 // 007aeb9b
        FLD1 // 007aeb9d
        FDIVRP st(1), st(0) // 007aeb9f
        FSTP dword ptr [ESP + 0x28] // 007aeba1
        JMP loc_007aebb2 // 007aeba5
    loc_007aeba7:
        XORPS XMM0,XMM0 // 007aeba7
        FSTP st(0) // 007aebaa
        MOVSS dword ptr [ESP + 0x28],XMM0 // 007aebac
    loc_007aebb2:
        FLD dword ptr [ESP + 0x38] // 007aebb2
        MOV ECX,dword ptr [ESI + 0x8] // 007aebb6
        TEST ECX,ECX // 007aebb9
        FLD dword ptr [ESP + 0x28] // 007aebbb
        FLD st(0) // 007aebbf
        FMULP st(2), st(0) // 007aebc1
        FXCH st(1) // 007aebc3
        FSTP dword ptr [ESP + 0x5c] // 007aebc5
        FLD dword ptr [ESP + 0x3c] // 007aebc9
        FMUL st(0), st(1) // 007aebcd
        FSTP dword ptr [ESP + 0x60] // 007aebcf
        FMUL dword ptr [ESP + 0x40] // 007aebd3
        FSTP dword ptr [ESP + 0x64] // 007aebd7
        JZ loc_007aebe9 // 007aebdb
        MOV EAX,dword ptr [ESI + 0xc] // 007aebdd
        SUB EAX,ECX // 007aebe0
        SAR EAX,0x2 // 007aebe2
        CMP EDI,EAX // 007aebe5
        JC loc_007aebee // 007aebe7
    loc_007aebe9:
        CALL dword ptr [EBP + 0xc] // 007aebe9
    loc_007aebee:
        MOV ECX,dword ptr [ESI + 0x8] // 007aebee
        MOV EAX,dword ptr [ECX + EDI*0x4] // 007aebf1
        MOVSS XMM0,dword ptr [EAX] // 007aebf4
        MOVSS dword ptr [ESP + 0x8c],XMM0 // 007aebf8
        MOVSS XMM0,dword ptr [EAX + 0x4] // 007aec01
        MOVSS dword ptr [ESP + 0x90],XMM0 // 007aec06
        MOVSS XMM0,dword ptr [EAX + 0x8] // 007aec0f
        PUSH EBX // 007aec14
        LEA EDX,[ESP + 0x60] // 007aec15
        LEA ECX,[ESP + 0x90] // 007aec19
        MOVSS dword ptr [ESP + 0x98],XMM0 // 007aec20
        CALL dword ptr [EBP + 0x14] // 007aec29
        FSTP dword ptr [ESP + 0xc] // 007aec2e
        FLD dword ptr [ESP + 0xc] // 007aec32
        FLD dword ptr [ESP + 0x10] // 007aec36
        FCOMIP st(0),st(1) // 007aec3a
        FSTP st(0) // 007aec3c
        JBE loc_007aece8 // 007aec3e
        MOV ECX,dword ptr [ESI + 0x8] // 007aec44
        TEST ECX,ECX // 007aec47
        JZ loc_007aec57 // 007aec49
        MOV EAX,dword ptr [ESI + 0xc] // 007aec4b
        SUB EAX,ECX // 007aec4e
        SAR EAX,0x2 // 007aec50
        CMP EDI,EAX // 007aec53
        JC loc_007aec5c // 007aec55
    loc_007aec57:
        CALL dword ptr [EBP + 0xc] // 007aec57
    loc_007aec5c:
        MOV EDX,dword ptr [ESI + 0x8] // 007aec5c
        MOV EAX,dword ptr [EDX + EDI*0x4] // 007aec5f
        MOVSS XMM0,dword ptr [EAX] // 007aec62
        MOVSS dword ptr [ESP + 0x54],XMM0 // 007aec66
        MOVSS XMM0,dword ptr [EAX + 0x4] // 007aec6c
        MOV ECX,EDX // 007aec71
        TEST ECX,ECX // 007aec73
        MOVSS dword ptr [ESP + 0x58],XMM0 // 007aec75
        MOVSS XMM0,dword ptr [EAX + 0x8] // 007aec7b
        MOVSS dword ptr [ESP + 0x5c],XMM0 // 007aec80
        JZ loc_007aec97 // 007aec86
        MOV EAX,dword ptr [ESI + 0xc] // 007aec88
        SUB EAX,ECX // 007aec8b
        SAR EAX,0x2 // 007aec8d
        LEA ECX,[EDI - 0x1] // 007aec90
        CMP ECX,EAX // 007aec93
        JC loc_007aec9c // 007aec95
    loc_007aec97:
        CALL dword ptr [EBP + 0xc] // 007aec97
    loc_007aec9c:
        MOV EDX,dword ptr [ESI + 0x8] // 007aec9c
        MOV EAX,dword ptr [EDX + EDI*0x4 - 0x4] // 007aec9f
        MOVSS XMM0,dword ptr [EAX] // 007aeca3
        MOVSS dword ptr [ESP + 0x6c],XMM0 // 007aeca7
        MOVSS XMM0,dword ptr [EAX + 0x4] // 007aecad
        MOVSS dword ptr [ESP + 0x70],XMM0 // 007aecb2
        MOVSS XMM0,dword ptr [EAX + 0x8] // 007aecb8
        LEA EAX,[ESP + 0x54] // 007aecbd
        PUSH EAX // 007aecc1
        LEA EDX,[ESP + 0x70] // 007aecc2
        LEA ECX,[ESP + 0x88] // 007aecc6
        MOVSS dword ptr [ESP + 0x78],XMM0 // 007aeccd
        CALL dword ptr [EBP + 0x18] // 007aecd3
        TEST AL,AL // 007aecd8
        JZ loc_007aece8 // 007aecda
        MOVSS XMM0,dword ptr [ESP + 0xc] // 007aecdc
        MOVSS dword ptr [ESP + 0x10],XMM0 // 007aece2
    loc_007aece8:
        MOV ECX,ESI // 007aece8
        ADD EDI,0x1 // 007aecea
        CALL dword ptr [EBP + 0x8] // 007aeced
        CMP EDI,EAX // 007aecf2
        JL loc_007aea10 // 007aecf4
    loc_007aecfa:
        FLD dword ptr [ESP + 0x10] // 007aecfa
        POP EDI // 007aecfe
        POP ESI // 007aecff
        POP EBX // 007aed00
        mov ebp, dword ptr [esp + 0x84] // restore borrowed-register save
        ADD ESP,0x94 // 007aed01
        RET 0x8 // 007aed07
    }
}
}  // namespace bsp::path_movcam_seg46_argon
