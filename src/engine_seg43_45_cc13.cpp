// Packet cc13-cam2, docs/CC13_CAM_2.md. Addresses, ABI and evidence are on the
// declarations in include/bsp/engine_seg43_45_cc13.hpp; listing addresses in the
// bodies mark where a step was read.
#include "bsp/engine_seg43_45_cc13.hpp"

#include "bsp/band_0076_cc13.hpp"
#include "bsp/camera_decomposition.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/native_bit_cursor_numeric_array.hpp"
#include "bsp/native_bit_cursor_write.hpp"
#include "bsp/ship_ai_bearing_rating.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>

namespace bsp::cc13cam2 {
namespace {

// The square-sum length the MovCam steps inline: the float-spilled sum, zero
// unless above 1e-10 (00CE3820), else the float sqrt.
float spilled_length(double sum) noexcept {
    const float s = static_cast<float>(sum);
    if (!(static_cast<double>(s) > 1e-10)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(s)));
}

float length3_yxz(float x, float y, float z) noexcept {
    return spilled_length(static_cast<double>(y) * y + static_cast<double>(x) * x + static_cast<double>(z) * z);
}

float length2_yx(float x, float y) noexcept {
    return spilled_length(static_cast<double>(y) * y + static_cast<double>(x) * x);
}

Vec3 entity_frame_point(std::size_t entity, const Vec3& p, bool level, CameraEntityHost& host) {
    Mat4 m = host.world_cc(entity);
    if (level) {
        const Vec3 g = host.global_f8758c();
        m[12] = g[0];
        m[13] = g[1];
        m[14] = g[2];
        host.orthonormalize_0085dad0(m);
    }
    return host.transform_004142e0(p, m);
}

}  // namespace

// ---------------------------------------------------------------------------
// CameraRef

void camera_ref_construct_point_00793920(cc13cam::CameraRef& ref, std::size_t entity, const Vec3& point,
                                         CameraEntityHost& host) {
    if (entity != 0 && host.destroyed_5e(entity)) entity = 0;  // 00793968
    ref.entity_14 = entity;
    ref.point_18 = point;
    ref.radius_24 = 0.0f;
    if (entity != 0) host.register_00694a60(entity, &ref);
}

void camera_ref_construct_radius_007939b0(cc13cam::CameraRef& ref, std::size_t entity, float radius,
                                          CameraEntityHost& host) {
    if (entity != 0 && host.destroyed_5e(entity)) entity = 0;
    ref.entity_14 = entity;
    ref.point_18 = {0.0f, 0.0f, 0.0f};
    ref.radius_24 = radius;
    if (entity != 0) host.register_00694a60(entity, &ref);
}

void camera_ref_copy_construct_00793a40(cc13cam::CameraRef& ref, const cc13cam::CameraRef& src,
                                        CameraEntityHost& host) {
    ref.entity_14 = src.entity_14;
    ref.point_18 = src.point_18;
    ref.radius_24 = src.radius_24;
    if (ref.entity_14 != 0) host.register_00694a60(ref.entity_14, &ref);
}

void camera_ref_assign_00790320(cc13cam::CameraRef& dst, const cc13cam::CameraRef& src, CameraEntityHost& host) {
    dst.entity_14 = src.entity_14;
    dst.point_18 = src.point_18;
    dst.radius_24 = src.radius_24;
    if (dst.entity_14 != 0) host.register_00694a60(dst.entity_14, &dst);  // 0079034E
}

Vec3 camera_ref_to_world_00793f10(const cc13cam::CameraRef& ref, bool level, CameraEntityHost& host) {
    if (ref.entity_14 == 0) return ref.point_18;
    return entity_frame_point(ref.entity_14, ref.point_18, level, host);
}

void camera_ref_set_point_00793cd0(cc13cam::CameraRef& ref, const Vec3& p, bool level, CameraEntityHost& host) {
    if (ref.entity_14 == 0) {
        ref.point_18 = p;
    } else {
        ref.point_18 = entity_frame_point(ref.entity_14, p, level, host);
    }
    ref.radius_24 = 0.0f;
}

void camera_ref_freeze_00793dc0(cc13cam::CameraRef& ref, cc13cam::CameraRefHost& resolve) {
    ref.point_18 = cc13cam::camera_ref_resolve_00793ac0(ref, true, resolve);
    ref.entity_14 = 0;
}

void camera_ref_on_released_00796f00(cc13cam::CameraRef& ref, std::size_t entity,
                                     cc13cam::CameraRefHost& resolve) {
    if (entity == ref.entity_14) camera_ref_freeze_00793dc0(ref, resolve);
}

void camera_target_link_construct_00793e60(CameraTargetLink& link, std::size_t entity,
                                           const std::array<std::uint32_t, 3>& words, ObserverHost& host) {
    link.entity_14 = entity;
    host.register_00694a60(entity, &link);  // 00793EB4, no null test
    link.words_18 = words;
    link.zero_24 = {};
    link.field_48 = 0;
    link.field_44 = 0;
}

// ---------------------------------------------------------------------------
// Path record

void path_record_set_anchor_0078e3d0(PathRecordEntities& r, std::size_t entity, ObserverHost& host) {
    const std::size_t old = r.anchor_14;
    if (old != 0 && old != entity && old != r.second_24) host.unregister_006952a0(old, &r);
    r.anchor_14 = entity;
    if (entity != 0 && entity != r.second_24) host.register_00694a60(entity, &r);
}

void path_record_set_second_0078e410(PathRecordEntities& r, std::size_t entity, ObserverHost& host) {
    const std::size_t old = r.second_24;
    if (old != 0 && old != entity && old != r.anchor_14) host.unregister_006952a0(old, &r);
    r.second_24 = entity;
    if (entity != 0 && entity != r.anchor_14) host.register_00694a60(entity, &r);
}

void path_record_on_released_00796f40(PathRecordEntities& r, std::size_t entity, PathRecordReleaseHost& host) {
    if (entity == r.anchor_14) {
        cc13cam::CameraRef ref;
        ref.entity_14 = r.anchor_14;
        ref.point_18 = r.point_18;
        r.point_18 = camera_ref_to_world_00793f10(ref, true, host);
        if (r.mode_48 == 3) {
            r.point_28 = host.transform_004142e0(r.point_28, host.world_cc(r.anchor_14));
            r.mode_48 = 2;
        } else if (r.mode_48 == 5) {
            const Mat4 world = host.world_cc(r.anchor_14);
            float x = 0.0f, y = 0.0f, z = 0.0f;
            extract_camera_matrix_angles_0042d2e0(world, x, y, z);  // 00796FF8
            r.mode_48 = 4;
            r.angle_34 = static_cast<float>(static_cast<double>(r.angle_34) + x);
            r.angle_38 = static_cast<float>(static_cast<double>(r.angle_38) + y);
        }
        r.anchor_14 = 0;
    }
    if (entity == r.second_24) {
        if (r.mode_48 == 1) {
            r.point_28 = host.position_fc(r.second_24);
            r.mode_48 = 2;
        }
        r.second_24 = 0;
    }
}

// ---------------------------------------------------------------------------
// Keyframe modifier slots

void modifier_set_owner_0078e780(cc13cam::ModifierKeyframe*& owner_4, cc13cam::ModifierKeyframe* owner) noexcept {
    owner_4 = owner;
}
int modifier_type_id_0078e800() noexcept { return 1; }
int modifier_type_id_0079dc30() noexcept { return 2; }
int modifier_type_id_0079dcf0() noexcept { return 3; }

std::size_t gamecamera_camera_0079dd00(const cc13cam::GameCameraModifier& m) noexcept { return m.camera_8; }

void gamecamera_track_tick_0078e870(const cc13cam::GameCameraModifier& m, float dt, GameCameraSlotHost& host) {
    if (m.camera_8 != 0) host.step_slot4(m.camera_8, dt);
}

void gamecamera_begin_0079cf80(cc13cam::GameCameraModifier& m, std::uintptr_t owner_node_3b8, GameCameraSlotHost& host) {
    m.last_time_c = 0.0f;  // 0079CF86
    host.pick_camera_0079cbd0(m);
    if (m.camera_8 != 0 && owner_node_3b8 != 0) host.camera_slot124(m.camera_8, owner_node_3b8);
}

void gamecamera_destruct_0079dc40(cc13cam::GameCameraModifier& m, GameCameraSlotHost& host) {
    if (m.camera_8 != 0) {
        host.kill_camera_00926d90(m.camera_8, 1);
        m.camera_8 = 0;
    }
    m.rho_entities_68.clear();
    m.theta_entities_58.clear();
    host.target_observer_destruct_0042bca0(m.target_4c);
    m.rho_28 = cc13cam::ModifierCurve{};
    m.theta_18 = cc13cam::ModifierCurve{};
}

void goaround_destruct_0079db90(cc13cam::GoAroundModifier& m) noexcept {
    m.patterntime_54 = cc13cam::ModifierCurve{};
    m.zoom_44 = cc13cam::ModifierCurve{};
    m.rho_34 = cc13cam::ModifierCurve{};
    m.theta_24 = cc13cam::ModifierCurve{};
    m.radius_14 = cc13cam::ModifierCurve{};
}

// ---------------------------------------------------------------------------
// Small movie-camera helpers

void movie_camera_log_append_007996c0(const std::string& text, bool& banner_written_f871ba, LogFileHost& host) {
    const std::uintptr_t file = host.fopen_bf838e("moviecamera.log", "at");
    if (!banner_written_f871ba) {
        host.fwrite_bfb08b("********\n", 9, file);
        banner_written_f871ba = true;
    }
    host.fwrite_bfb08b(text.data(), text.size(), file);
    host.fwrite_bfb08b("\n", 1, file);  // 00CE4390
    host.fclose_bf8081(file);
}

void movcam_add_interpolator_0079f6d0(std::vector<cc13cam::MovCamInterpolator>& list_4d8,
                                      const cc13cam::MovCamInterpolator& record) {
    list_4d8.push_back(record);
}

void movie_key_set_event_0078e540(std::uintptr_t& event_ec, std::uintptr_t event, OwnedObjectHost& host) {
    if (event_ec == event) return;
    if (event_ec != 0) host.scalar_delete(event_ec);
    event_ec = event;
}

void entity_observer_copy_base_00796bd0(EntityObserverBase& o, const EntityObserverBase& src, ObserverHost& host) {
    o.entity_14 = src.entity_14;
    o.active_10 = true;
    if (o.entity_14 != 0) host.register_00694a60(o.entity_14, &o);
}

// ---------------------------------------------------------------------------
// Ocean environment settings

void ocean_settings_assign_0078d940(OceanEffectSettings& dst, const OceanEffectSettings& src) {
    dst.head = src.head;
    if (&dst.name_8 != &src.name_8) dst.name_8 = src.name_8;
    dst.words_10 = src.words_10;
    dst.table_20 = src.table_20;  // four 8-word blocks in the image loop
}

void ocean_environment_delete_member_0078e510(std::uintptr_t& member_e4, OwnedObjectHost& host) {
    if (member_e4 == 0) return;
    host.scalar_delete(member_e4);
    member_e4 = 0;
}

// ---------------------------------------------------------------------------
// MovCam

void movcam_wrap_and_clamp_007904d0(MovCamAngles& a) noexcept {
    a.theta_3c4 = ship_ai_firepower_wrap_angle_00605070(a.theta_3c4);
    a.rho_3c8 = ship_ai_firepower_wrap_angle_00605070(a.rho_3c8);
    if (static_cast<double>(a.theta_3c4) > 1.5707963705062866) {          // 00CE3830
        a.theta_3c4 = 1.5707964f;                                         // 00CE3C64
    } else if (static_cast<double>(a.theta_3c4) < -1.5707963705062866) {  // 00CF48A0
        a.theta_3c4 = -1.5707964f;                                        // 00CE3CCC
    }
}

void movcam_seed_from_mover_00794360(MovCamAngles& a, const Vec3& mover_position_a4,
                                     const Mat4& mover_matrix_74) {
    a.position_394 = mover_position_a4;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    extract_camera_matrix_angles_0042d2e0(mover_matrix_74, x, y, z);  // 0079439B
    a.theta_3c4 = -0.0f - x;                                          // SUBSS from 00D7A208
    a.rho_3c8 = -0.0f - y;
}

void movcam_reset_motion_00794000(cc13cam::MovCamMotion& m, cc13cam::MovCamTurn& t, MovCamResetHost& host) {
    m.acceleration_3ac = {0.0f, 0.0f, 0.0f};
    m.velocity_3a0 = {0.0f, 0.0f, 0.0f};
    t.turn_acceleration_3d4 = {0.0f, 0.0f};
    t.rate_3cc = {0.0f, 0.0f};
    host.step_reset_slot_c();  // 0079406E, JMP [[+170h]+Ch]
}

void movcam_snapshot_00790460(std::array<std::uint32_t, 8>& out_370, const MovCamTrackedSnapshot& in) noexcept {
    out_370 = {in.from_224[0], in.from_224[1], in.from_224[2], in.from_254,
               in.from_258,    in.from_2f0,    in.from_278,    in.from_2f8};
}

float movcam_set_zoom_0078e480(MovCamZoom& z, float zoom, float base_fov_f889b4) noexcept {
    z.zoom_460 = zoom;
    const float fov = static_cast<float>(static_cast<double>(base_fov_f889b4) / zoom);  // 0078E4A2
    z.zoom_copy_4f4 = z.zoom_460;
    return fov;
}

void movcam_integrate_007943d0(cc13cam::MovCamMotion& m, cc13cam::MovCamTurn& t,
                               const cc13cam::MovCamLimits& limits, const cc13cam::MovCamTurnLimits& turn,
                               float dt) noexcept {
    auto& v = m.acceleration_3ac;
    if (limits.accel_4c < length3_yxz(v[0], v[1], v[2])) {  // 0079443A
        v = cc13cam::scale_to_length_00793730(v, limits.accel_4c);
    }
    for (int i = 0; i < 3; ++i) {
        const float step = static_cast<float>(static_cast<double>(v[i]) * dt);
        m.velocity_3a0[i] = static_cast<float>(static_cast<double>(m.velocity_3a0[i]) + step);
    }
    Vec3 d{};
    for (int i = 0; i < 3; ++i) {
        d[i] = static_cast<float>(static_cast<double>(m.velocity_3a0[i]) - m.reference_velocity_3b8[i]);
    }
    if (limits.top_speed_34 < length3_yxz(d[0], d[1], d[2])) {  // 00794537
        const Vec3 s = cc13cam::scale_to_length_00793730(d, limits.top_speed_34);
        for (int i = 0; i < 3; ++i) {
            m.velocity_3a0[i] = static_cast<float>(static_cast<double>(s[i]) + m.reference_velocity_3b8[i]);
        }
    }
    auto& r = t.turn_acceleration_3d4;
    if (turn.accel_50 < length2_yx(r[0], r[1])) {  // 00794618
        r = cc13cam::scale_to_length_00793800(r, turn.accel_50);
    }
    for (int i = 0; i < 2; ++i) {
        const float step = static_cast<float>(static_cast<double>(r[i]) * dt);
        t.rate_3cc[i] = static_cast<float>(static_cast<double>(t.rate_3cc[i]) + step);
    }
    std::array<float, 2> a{};
    for (int i = 0; i < 2; ++i) {
        a[i] = static_cast<float>(static_cast<double>(t.rate_3cc[i]) - t.reference_rate_3dc[i]);
    }
    if (turn.top_rate_38 < length2_yx(a[0], a[1])) {  // 007946DA
        const auto s = cc13cam::scale_to_length_00793800(a, turn.top_rate_38);
        for (int i = 0; i < 2; ++i) {
            t.rate_3cc[i] = static_cast<float>(static_cast<double>(s[i]) + t.reference_rate_3dc[i]);
        }
    }
}

void movcam_steer_angles_braking_00794f70(cc13cam::MovCamTurn& t, const cc13cam::MovCamTurnLimits& limits,
                                          const std::array<float, 2>& d, std::size_t target,
                                          float length_00414c60_of_d) noexcept {
    std::array<float, 2> delta{};
    for (int i = 0; i < 2; ++i) {
        delta[i] = static_cast<float>(static_cast<double>(t.rate_3cc[i]) - t.reference_rate_3dc[i]);
    }
    const float r = static_cast<float>(static_cast<double>(length2_yx(delta[0], delta[1])) / limits.accel_50);
    if (target != 0) {  // 00794FEB
        if (t.target_3e4 == 0) {
            t.tolerance_3e8 = length_00414c60_of_d;
        } else {
            float len = length_00414c60_of_d;
            if (t.tolerance_3e8 < len) len = t.tolerance_3e8;
            t.tolerance_3e8 = len;
        }
        t.growth_3ec = 0.9f;  // 00CE3860
        t.target_3e4 = target;
    }
    std::array<float, 2> want{};
    for (int i = 0; i < 2; ++i) {  // 00795062..007950BE
        const float rd = static_cast<float>(static_cast<double>(delta[i]) * r);
        const float brake = static_cast<float>(static_cast<double>(rd) * 0.5);
        want[i] = static_cast<float>(static_cast<double>(d[i]) - brake);
    }
    float s = static_cast<float>(static_cast<double>(length2_yx(want[0], want[1])) * 1000.0);  // 00CE47A0
    if (static_cast<double>(s) > 2.0) s = 2.0f;                                               // 00D7A308
    const float len = static_cast<float>(static_cast<double>(limits.accel_50) * s);
    const auto add = cc13cam::scale_to_length_00793800(want, len);
    for (int i = 0; i < 2; ++i) {
        t.turn_acceleration_3d4[i] = static_cast<float>(static_cast<double>(add[i]) + t.turn_acceleration_3d4[i]);
    }
}

Mat4 movcam_look_basis_00795300(const Vec3& position_394, const Vec3& mover_up_84, std::size_t target,
                                LookBasisHost& host) {
    Mat4 m{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const Vec3 p = host.target_position_fc(target);
    Vec3 d{};
    for (int i = 0; i < 3; ++i) d[i] = static_cast<float>(static_cast<double>(p[i]) - position_394[i]);
    const float len = host.length_00419440(d);
    const float inv = len > 0.0f ? static_cast<float>(1.0 / len) : 0.0f;  // 007953BD
    for (int i = 0; i < 3; ++i) m[8 + i] = static_cast<float>(static_cast<double>(d[i]) * inv);
    const Vec3 up = std::fabs(m[9]) < 0.9999000000025262 ? host.global_f8758c() : mover_up_84;  // 00795428
    m[4] = up[0];
    m[5] = up[1];
    m[6] = up[2];
    host.orthonormalize_0085dc80(m);
    return m;
}

void movcam_set_look_target_00797070(MovCamLook& c, const void* observer_10, std::size_t target, bool aim,
                                     MovCamLookHost& host) {
    if (target != 0 && host.is_kind_5c(target, 0x18)) target = host.vehicle_owner_3d0(target);  // 0079708B
    const std::size_t old = c.look_target_390;
    if (old == target) return;
    if (old != 0 && old != c.main_target_38c && host.is_registered_00694af0(old, observer_10)) {
        host.unregister_006952a0(old, observer_10);
    }
    c.look_target_390 = target;
    if (target == 0) return;
    if (c.mode_384 == 1 && c.sub_mode_388 % 2 == 0) c.sub_mode_388 += 1;  // 007970E7
    if (aim) {
        host.aim_at_target_00794130(c, target, -1.0f);  // 00D7A260
        c.follow_target_3e4 = c.look_target_390;
        c.aimed_theta_4ec = c.theta_3c4;
        c.aimed_rho_4f0 = c.rho_3c8;
    } else if (c.follow_target_3e4 != target) {
        c.follow_target_3e4 = 0;
    }
    if (!host.is_registered_00694af0(c.look_target_390, observer_10)) {
        host.register_00694a60(c.look_target_390, observer_10);
    }
}

void movcam_turn_by_00797230(MovCamLook& c, const void* observer_10, float d_theta, float d_rho, bool instant,
                             float time, MovCamLookHost& host) {
    movcam_set_look_target_00797070(c, observer_10, 0, false, host);
    c.wanted_theta_418 = static_cast<float>(static_cast<double>(c.theta_3c4) + d_theta);
    c.wanted_rho_41c = static_cast<float>(static_cast<double>(c.rho_3c8) + d_rho);
    if (instant) {
        c.theta_3c4 = c.wanted_theta_418;
        c.rho_3c8 = c.wanted_rho_41c;
        return;
    }
    if (time > 0.0f) {  // 00D7A218
        const std::array<float, 2> left{
            static_cast<float>(static_cast<double>(c.theta_3c4) - c.wanted_theta_418),
            static_cast<float>(static_cast<double>(c.rho_3c8) - c.wanted_rho_41c)};
        const float rate = static_cast<float>(static_cast<double>(host.length_00414c60(left)) / time);  // 007972CD
        host.set_angular_speed_limit_005cba20(rate);
    }
}

void movcam_turn_to_00797180(MovCamLook& c, const void* observer_10, float theta, float rho, bool instant,
                             float time, MovCamLookHost& host) {
    movcam_set_look_target_00797070(c, observer_10, 0, false, host);
    c.wanted_theta_418 = theta;  // 00797197
    c.wanted_rho_41c = rho;
    if (instant) {
        c.theta_3c4 = c.wanted_theta_418;
        c.rho_3c8 = c.wanted_rho_41c;
        return;
    }
    if (time > 0.0f) {  // 007971D1
        const std::array<float, 2> left{
            static_cast<float>(static_cast<double>(c.theta_3c4) - c.wanted_theta_418),
            static_cast<float>(static_cast<double>(c.rho_3c8) - c.wanted_rho_41c)};
        const float rate = static_cast<float>(static_cast<double>(host.length_00414c60(left)) / time);  // 00797203
        host.set_angular_speed_limit_005cba20(rate);
    }
}

Vec3 camera_orbit_point_007905b0(const Vec3& centre, float a, float b, float dist, const Mat4& m,
                                 OrbitPointHost& host) {
    const Mat4 ry = host.rotation_y_00b646e0(-0.0f - a);  // 007905D8
    const Mat4 rx = host.rotation_x_00b64640(-0.0f - b);  // 00790609
    const Vec3 axis = host.axis_f87598();
    const Vec3 v{static_cast<float>(static_cast<double>(axis[0]) * dist),
                 static_cast<float>(static_cast<double>(axis[1]) * dist),
                 static_cast<float>(static_cast<double>(dist) * axis[2])};  // 00790642..00790658
    const Vec3 r = host.transform_004142e0(v, host.multiply_00413920(rx, ry));
    const Vec3 p{static_cast<float>(static_cast<double>(centre[0]) + r[0]),
                 static_cast<float>(static_cast<double>(centre[1]) + r[1]),
                 static_cast<float>(static_cast<double>(centre[2]) + r[2])};
    return host.transform_004142e0(p, m);
}

void movcam_debug_draw_vehicle_cameras_007a01f0(VehicleCameraDrawHost& host) {
    auto mod2 = [](int v) { return v % 2; };  // the 80000001h idiom: signed remainder
    for (std::size_t i = 0; i < host.camera_count(); ++i) {
        const int n = static_cast<int>(i);
        const float half = 0.5f;  // 00D7A280 as float
        const std::array<float, 4> colour{
            static_cast<float>(mod2(n / 4)) * half + half, static_cast<float>(mod2(n / 2)) * half + half,
            static_cast<float>(mod2(n)) * half + half, 1.0f};
        host.refresh_target_pose_00414db0();  // when +C8h is clear
        host.draw_sector_007906e0(i, colour);
    }
}

std::array<float, 2> movcam_vehicle_camera_pose_0079f710(std::vector<VehicleCameraElement>* cameras, int index,
                                                         const Mat4& target_world_cc, const Vec3& entity_position_fc,
                                                         bool* inside, VehicleCameraPoseHost& host) {
    if (cameras == nullptr || static_cast<int>(cameras->size()) < index) return {0.0f, 0.0f};  // 0079F749 JL
    if (static_cast<int>(cameras->size()) <= index) cameras->resize(static_cast<std::size_t>(index) + 1);  // 005CD640
    const VehicleCameraElement& e = (*cameras)[static_cast<std::size_t>(index)];
    const Mat4 m = host.multiply_00413920(e.local_8, target_world_cc);  // 0079F7A1
    Vec3 d{};
    for (int i = 0; i < 3; ++i) {
        d[static_cast<std::size_t>(i)] =
            static_cast<float>(static_cast<double>(entity_position_fc[static_cast<std::size_t>(i)]) - m[12 + i]);
    }
    const Vec3 forward{target_world_cc[8], target_world_cc[9], target_world_cc[10]};  // +ECh
    const std::array<float, 2> delta = cc13cam::camera_direction_delta_00794750(d, forward);
    constexpr float kPi = 3.14159274f;  // 00D7A264
    const float centre_yaw = static_cast<float>((static_cast<double>(e.yaw_lo_48) + e.yaw_hi_4c) * 0.5 +
                                                (!(e.yaw_hi_4c > e.yaw_lo_48) ? kPi : 0.0f));
    const float centre_pitch = static_cast<float>(0.5 * (static_cast<double>(e.pitch_lo_50) + e.pitch_hi_54) +
                                                  (!(e.pitch_hi_54 > e.pitch_lo_50) ? kPi : 0.0f));
    if (inside != nullptr) {
        const band0076_cc13::AngleWindows w{e.yaw_lo_48, e.yaw_hi_4c, e.pitch_lo_50, e.pitch_hi_54};
        *inside = band0076_cc13::movcam_angle_windows_contain_0078e2b0(w, delta[1], delta[0]);  // 0079F944
    }
    const float r_yaw = wrapped_angle_subtract_00438b10(delta[1], centre_yaw);      // 0079F95D
    const float r_pitch = wrapped_angle_subtract_00438b10(delta[0], centre_pitch);  // 0079F98B
    return {std::fabs(r_pitch), std::fabs(r_yaw)};
}

bool movcam_has_vehicle_camera_pose_007a0dd0(std::vector<VehicleCameraElement>* cameras, int index,
                                             const Mat4& target_world_cc, const Vec3& entity_position_fc,
                                             VehicleCameraPoseHost& host) {
    bool inside = false;
    movcam_vehicle_camera_pose_0079f710(cameras, index, target_world_cc, entity_position_fc, &inside, host);
    return inside;
}

void movcam_approach_0078faf0(std::array<float, 3>& state, float dt) noexcept {
    // cc13-fixture5: per x87 operation at the main thread's 24-bit precision; the
    // double model differed from the bytes on 492 of 4000 inputs.
    const float current = state[0];
    const float target = state[1];
    const float d = target - current;                 // 0078FB0A, stored at 0078FB0E
    const float x = d > 0.0f ? d : -0.0f - d;
    const float step = (x * dt) * 1.4f;               // 0078FB3E, then the double 1.4f at 0078FB42
    float next;
    if (!(target >= current)) {  // 0078FB4E FCOMIP / JC: below, or unordered
        next = current - step;
        if (target > next) next = target;  // 0078FB9E
    } else {
        next = current + step;
        if (next > target) next = target;  // 0078FB6B
    }
    state[0] = next;
    state[2] = dt * next + state[2];                  // 0078FBC1-0078FBC3
}

// ---------------------------------------------------------------------------
// MovCam pose publication

namespace {
Mat4 translation(const Vec3& p) {
    return {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, p[0], p[1], p[2], 1.0f};
}
void publish_pose(MovCamPose& p, const Mat4& rx, const Mat4& ry, const Vec3& position, MovCamPoseHost& host) {
    const Mat4 r = host.multiply_00413920(rx, ry);
    p.matrix_74 = host.multiply_00413920(r, translation(position));
    p.pose_valid_c8 = false;
    p.pose_valid_10c = false;
    host.invalidate_children_0042ed50();
    host.push_pose_slot34(p.matrix_74);
}
}  // namespace

void movcam_apply_pose_00799750(MovCamPose& p, const MovCamAngles& a, MovCamPoseHost& host) {
    const Mat4 ry = host.rotation_y_00b646e0(-0.0f - a.rho_3c8);  // 007997CD
    const Mat4 rx = host.rotation_x_00b64640(-0.0f - a.theta_3c4);
    publish_pose(p, rx, ry, a.position_394, host);
}

void movcam_set_pos_rot_zoom_007998c0(MovCamPose& p, const Vec3& position, float pitch_degrees, float yaw_degrees,
                                      MovCamPoseHost& host) {
    p.mode_384 = 8;
    p.matrix_420 = translation({0.0f, 0.0f, 0.0f});
    host.set_shake_0042c4c0({0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f});
    constexpr double k = 0.01745329238474369;  // 00D046D8
    const float pitch = static_cast<float>(static_cast<double>(pitch_degrees) * k);  // 00799999 DC C9, FXCH, FSTP
    const float yaw = static_cast<float>(k * yaw_degrees);                          // 007999AB
    const Mat4 ry = host.rotation_y_00b646e0(yaw);
    const Mat4 rx = host.rotation_x_00b64640(pitch);
    publish_pose(p, rx, ry, position, host);
    host.reset_motion_00794000();
}

// ---------------------------------------------------------------------------
// MovCam callbacks

namespace {
void fire_callback(MovCamCallback& cb, MovCamCallbackHost& host) {
    const std::string name = cb.name;  // 0079D9EE copy, then the member is emptied
    cb.name.clear();
    const std::uintptr_t args = cb.args;
    cb.args = 0;
    host.call_named_00887e50(name, args);
    if (args != 0) host.free_args(args);
}
}  // namespace

void movcam_check_arrival_0079d960(MovCamCallback& cb_49c, float threshold, float length_0042b2f0_of_difference,
                                   MovCamCallbackHost& host) {
    if (cb_49c.name.empty()) return;
    if (length_0042b2f0_of_difference < threshold) fire_callback(cb_49c, host);  // 0079D9DF
}

void movcam_check_secondary_arrival_0079da90(MovCamCallback& cb_4a4, std::size_t target, float threshold,
                                             float aim_length, MovCamCallbackHost& host) {
    if (target == 0 || cb_4a4.name.empty()) return;
    if (aim_length < threshold) fire_callback(cb_4a4, host);
}

void movcam_set_secondary_target_0079d870(MovCamLook& c, MovCamCallback& cb_4a4, const void* observer_10,
                                          std::size_t target, bool aim, float time, const std::string& name,
                                          std::uintptr_t args, float aim_length, MovCamLookHost& look,
                                          MovCamCallbackHost& callbacks) {
    if (!name.empty()) {
        cb_4a4.name = name;
        if (cb_4a4.args != 0) {
            callbacks.free_args(cb_4a4.args);
            cb_4a4.args = 0;
        }
        cb_4a4.args = args;
    }
    movcam_set_look_target_00797070(c, observer_10, target, aim, look);
    if (!aim && time > 0.0f) {
        look.set_angular_speed_limit_005cba20(static_cast<float>(static_cast<double>(aim_length) / time));
    }
}

void movcam_apply_interpolator_007901d0(const cc13cam::MovCamInterpolator& r, MovCamInputs& inputs,
                                        MovCamInterpolatorHost& host) {
    auto word = [](std::uint32_t w) {
        float f;
        std::memcpy(&f, &w, sizeof f);
        return f;
    };
    const float from = word(r.args_4[0]);
    const float to = word(r.args_4[1]);
    const float start = word(r.args_4[2]);
    const float duration = word(r.args_4[3]);
    const float clock = word(r.field_14);
    if (start > clock) return;  // 007901E9 JA
    const float u = static_cast<float>((static_cast<double>(clock) - start) / duration);
    float value = static_cast<float>(static_cast<double>(from) + (static_cast<double>(to) - from) * u);
    if (u >= 1.0f) value = to;  // 00790227 JB keeps the blend (also for NaN)
    switch (r.id_0) {
    case 1: inputs.turn_h_50c = value; break;
    case 2: inputs.turn_v_510 = value; break;
    case 3: inputs.walk_h_504 = value; break;
    case 4: inputs.walk_v_508 = value; break;
    case 5: inputs.zoom_514 = value; break;
    case 6: host.set_linear_speed_limit_005cb9f0(value); break;
    case 7: host.set_angular_speed_limit_005cba20(value); break;
    case 8: host.set_zoom_0078e480(value); break;
    default: break;
    }
}

int vehicle_camera_index_of_name_0079f400(bool unit_is_kind5, const std::vector<std::string>* camera_names,
                                          const std::string& name) {
    if (!unit_is_kind5 || camera_names == nullptr) return -1;
    auto lower = [](char ch) { return (ch >= 'A' && ch <= 'Z') ? static_cast<char>(ch - 'A' + 'a') : ch; };
    for (std::size_t i = 0; i < camera_names->size(); ++i) {
        const std::string& s = (*camera_names)[i];
        if (s.size() != name.size()) continue;
        bool same = true;
        for (std::size_t k = 0; k < s.size() && same; ++k) same = lower(s[k]) == lower(name[k]);
        if (same) return static_cast<int>(i);
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Movie-camera tracks

void movie_track_sync_time_00797a00(MovieTrackKeys& t, float& clock_3b0, bool back) noexcept {
    if (t.current_7c < 0) return;
    if (back && t.current_7c != 0 && clock_3b0 == t.keys[t.current_7c].start_f0) {
        t.current_7c -= 1;  // 00796680 (it--), 00796610 (*it)
    }
    clock_3b0 = t.keys[t.current_7c].start_f0;
}

void movie_track_advance_00797af0(MovieTrackKeys& t, float& clock_3b0) noexcept {
    if (t.current_7c < 0) return;
    const MovieKeyTimes& k = t.keys[t.current_7c];
    const float end = static_cast<float>(static_cast<double>(k.blend_f4) + k.start_f0);
    const bool last = t.current_7c + 1 >= static_cast<int>(t.keys.size());
    const int next = last ? t.current_7c : t.current_7c + 1;  // 00797B9C: past the end wraps to the tail
    const float next_start = last ? 999999.0f : t.keys[next].start_f0;
    if (end <= next_start) {
        if (clock_3b0 < end) {
            clock_3b0 = end;
            return;
        }
        if (static_cast<double>(next_start) >= 999998.0) {
            clock_3b0 = end;
            return;
        }
    }
    t.current_7c = next;
    clock_3b0 = t.keys[next].start_f0;
}

void movie_track_key_span_00797f50(const MovieTrackKeys& t, int id, float& start, float& end) noexcept {
    if (t.keys.empty()) {
        end = 0.0f;
        start = 0.0f;
        return;
    }
    start = 0.0f;
    end = 999999.0f;
    for (std::size_t i = 0; i < t.keys.size(); ++i) {
        const bool match = id < 0 ? static_cast<int>(i) == t.current_7c : t.keys[i].id_da == id;
        if (match) {
            if (i + 1 < t.keys.size()) end = t.keys[i + 1].start_f0;  // 00793400, the next node
            return;
        }
        start = t.keys[i].start_f0;
    }
}

void movie_camera_time_span_00798e70(const MovieTrackKeys& camera_track_498, bool has_selected_514,
                                     std::int16_t selected_id_da, float& start, float& end) noexcept {
    if (!has_selected_514) {
        end = 0.0f;
        start = 0.0f;
        return;
    }
    movie_track_key_span_00797f50(camera_track_498, -1, start, end);
    if (selected_id_da >= 0) {
        float s = 0.0f, e = 0.0f;
        movie_track_key_span_00797f50(camera_track_498, selected_id_da, s, e);
        if (start < s) start = s;
        if (e < end) end = e;
    }
}

// ---------------------------------------------------------------------------
// Keyframe events

cc13cam::KeyframeEvent keyframe_event_construct(cc13cam::KeyframeEventKind kind) {
    cc13cam::KeyframeEvent e;
    e.kind = kind;
    e.fired_c = false;
    e.entity_24 = 0;
    e.depth_28 = kind == cc13cam::KeyframeEventKind::Sunk ? -10.0f : 0.0f;  // 0079A0E0: 00CE6848
    return e;
}

void keyframe_event_set_entity_007995f0(cc13cam::KeyframeEvent& e, std::size_t entity, ObserverHost& host) {
    if (e.entity_24 == entity) return;
    const void* observer = &e;  // event+10h in the image
    if (e.entity_24 != 0) host.unregister_006952a0(e.entity_24, observer);
    e.entity_24 = entity;
    if (entity != 0) host.register_00694a60(entity, observer);
}

void keyframe_event_set_function_0079c000(cc13cam::KeyframeEvent& e, const std::string& name) {
    if (&e.function_4 != &name) e.function_4 = name;
}

void keyframe_event_destruct_00799fd0(cc13cam::KeyframeEvent& e, KeyframeEventDestroyHost& host) {
    host.target_observer_destruct_0042bca0(e.entity_24, &e);
    e.function_4.clear();
    e.function_4.shrink_to_fit();
}

// ---------------------------------------------------------------------------
// Keyframe linking

void movie_key_set_fixed_position_00795bd0(MovieKeyLinks& k, const Vec3& p, MovieKeyHost& host) {
    if (k.modifier_e4 != 0) return;
    k.work_40 = p;
    k.local_34 = k.work_40;
    host.evaluate_position_00795650(k);
}

void movie_key_link_partner_007a0e00(MovieKeyLinks& k, MovieKeyLinks* partner, MovieKeyHost& host) {
    k.linked_25 = true;
    k.field_d0 = 0;
    k.field_cc = 0;
    if (partner != nullptr) {
        k.camera_28 = false;
        k.partner_20 = partner;
        partner->partner_20 = &k;
        return;
    }
    k.camera_28 = true;
    k.modifier_e4 = host.new_default_modifier_007a06d0();
}

namespace {
void append_and_pair(MovieKeyLinks& k, MovieKeyHost& host) {
    host.append_007a0770(k.camera_28, k);
    host.pair_00798080(!k.camera_28, k);
}
}  // namespace

void movie_key_shift_start_007a2d30(MovieKeyLinks& k, float dt, MovieKeyHost& host) {
    const float start = static_cast<float>(static_cast<double>(dt) + k.start_f0);  // 007A2D42
    k.start_f0 = start;
    if (0.0f > start) k.start_f0 = 0.0f;
    append_and_pair(k, host);
}

void movie_camera_insert_key_007a2cd0(MovieKeyLinks& k, MovieKeyHost& host) {
    append_and_pair(k, host);
    if (host.session_present_1fe4()) host.broadcast_new_add_position_0076d6d0(k);
}

void movie_camera_add_key_pair_007a2bb0(std::uintptr_t source, std::uint32_t& next_id_3c0, MovieKeyHost& host) {
    if (source == 0) return;
    MovieKeyLinks* camera_key = host.construct_at_0079c910(source);
    movie_key_link_partner_007a0e00(*camera_key, nullptr, host);
    host.append_007a0770(true, *camera_key);
    MovieKeyLinks* look_key = host.construct_at_0079c910(source);
    movie_key_link_partner_007a0e00(*look_key, camera_key, host);
    host.append_007a0770(false, *look_key);
    const auto id = static_cast<std::int16_t>(next_id_3c0);  // 007A2C9A, the low word
    look_key->id_da = id;
    camera_key->id_da = id;
    next_id_3c0 += 1;
}

// ---------------------------------------------------------------------------
// Keyframe serialization

namespace {
constexpr std::uint32_t kF8 = 0x41000000u;      // 8.0, 00CE3918
constexpr std::uint32_t kFMax = 0x7F7FFFFFu;    // FLT_MAX, 00D7A248
constexpr std::uint32_t kF64 = 0x42800000u;     // 64.0, 00CE7820
constexpr std::uint32_t kF256 = 0x43800000u;    // 256.0, 00CF8B3C
}  // namespace

void movie_key_write_bits_00791360(const MovieKeyBits& k, NativeBitCursor* cursor, MovieKeyBitsHost& host) {
    const NativeBitNumericContext& ctx = host.numeric_context();
    write_native_bool_bit_004290b0(cursor, k.terrain_avoid_d9);
    write_native_numeric_float_004295c0(cursor, k.field_cc, 0, 0, kF8, 16, ctx);
    write_native_numeric_float_array_00429790(cursor, k.local_34.data(), 3, 0, 1, kFMax, 32, ctx);
    write_native_numeric_float_array_00429790(cursor, k.up_58.data(), 3, 0, 1, kFMax, 32, ctx);
    write_native_numeric_float_004295c0(cursor, k.zoom_c4, 0, 0, kF64, 16, ctx);
    write_native_numeric_float_004295c0(cursor, k.blend_f4, 0, 1, kF256, 16, ctx);
    write_native_numeric_float_004295c0(cursor, k.start_f0, 0, 1, kF256, 16, ctx);
    write_native_numeric_float_004295c0(cursor, k.nonlinear_d4, 1, 0, kF8, 10, ctx);
    write_native_bool_bit_004290b0(cursor, k.camera_28);
    write_native_bool_bit_004290b0(cursor, k.linked_25);
    write_native_bool_bit_004290b0(cursor, k.wanderer_26);
    write_native_unsigned_dword_bits_00429070(cursor, k.transform_2c, 4);
    write_native_bool_bit_004290b0(cursor, k.use_current_d8);
    write_native_bool_bit_004290b0(cursor, k.has_position_27);
    write_native_bool_bit_004290b0(cursor, k.then_none_30);
    write_native_bool_bit_004290b0(cursor, k.flag_dc);
    if (!k.parent_attached_24 || k.parent_1c == 0) {
        write_native_bool_bit_004290b0(cursor, 0);
    } else {
        write_native_bool_bit_004290b0(cursor, 1);
        write_native_word_bits_00429120(cursor, host.net_id_174(k.parent_1c), 12);  // 007914E1
    }
    if (k.modifier_e4 != 0) {
        write_native_unsigned_dword_bits_00429070(cursor, static_cast<std::uint32_t>(host.modifier_type_slot30(k.modifier_e4)), 4);
        host.modifier_write_slot24(k.modifier_e4, cursor);  // 0079151D
        return;
    }
    write_native_unsigned_dword_bits_00429070(cursor, 0, 4);
}

void movie_key_read_bits_007a2370(MovieKeyBits& k, NativeBitCursor* cursor, MovieKeyBitsHost& host) {
    const NativeBitNumericContext& ctx = host.numeric_context();
    read_native_bool_bit_00428d70(cursor, &k.terrain_avoid_d9);
    read_native_numeric_float_004293f0(cursor, &k.field_cc, 0, 0, kF8, 16, ctx);
    read_native_numeric_float_array_004294f0(cursor, k.local_34.data(), 3, 0, 1, kFMax, 32, ctx);
    read_native_numeric_float_array_004294f0(cursor, k.up_58.data(), 3, 0, 1, kFMax, 32, ctx);
    read_native_numeric_float_004293f0(cursor, &k.zoom_c4, 0, 0, kF64, 16, ctx);
    read_native_numeric_float_004293f0(cursor, &k.blend_f4, 0, 1, kF256, 16, ctx);
    read_native_numeric_float_004293f0(cursor, &k.start_f0, 0, 1, kF256, 16, ctx);
    read_native_numeric_float_004293f0(cursor, &k.nonlinear_d4, 1, 0, kF8, 10, ctx);
    read_native_bool_bit_00428d70(cursor, &k.camera_28);
    read_native_bool_bit_00428d70(cursor, &k.linked_25);
    read_native_bool_bit_00428d70(cursor, &k.wanderer_26);
    read_native_u32_bits_00428d10(cursor, &k.transform_2c, 4);
    read_native_bool_bit_00428d70(cursor, &k.use_current_d8);
    read_native_bool_bit_00428d70(cursor, &k.has_position_27);
    read_native_bool_bit_00428d70(cursor, &k.then_none_30);
    read_native_bool_bit_00428d70(cursor, &k.flag_dc);
    read_native_bool_bit_00428d70(cursor, &k.parent_attached_24);
    if (k.parent_attached_24) {
        std::uint16_t id = 0;
        read_native_word_bits_00428e30(cursor, &id, 12);
        const std::size_t entity = host.entity_from_net_id(id);
        if (k.parent_1c != entity) {
            if (k.parent_1c != 0) host.unregister_006952a0(k.parent_1c, &k);
            k.parent_1c = entity;
            if (entity != 0) host.register_00694a60(entity, &k);
        }
        if (k.parent_1c != 0 && host.destroyed_5e(k.parent_1c)) {
            const std::array<float, 3> p = host.position_fc(k.parent_1c);
            for (std::size_t i = 0; i < 3; ++i) {
                float v;
                std::memcpy(&v, &k.local_34[i], sizeof v);
                v = static_cast<float>(static_cast<double>(p[i]) + v);
                std::memcpy(&k.local_34[i], &v, sizeof v);
            }
            host.unregister_006952a0(k.parent_1c, &k);
            k.parent_1c = 0;
            k.parent_attached_24 = false;
        }
    }
    std::uint32_t type = 0;
    read_native_u32_bits_00428d10(cursor, &type, 4);
    if (type != 1 && type != 2 && type != 3) return;
    const std::uintptr_t modifier = host.new_modifier(static_cast<int>(type));
    if (modifier == 0) return;
    host.modifier_read_slot28(modifier, cursor);  // 007A2665
    host.set_modifier_00799de0(k, modifier);
}

bool lua_is_vector3_table_0079c040(bool is_table, const std::vector<LuaEntryView>& entries) {
    if (!is_table) return false;
    auto iequal = [](const std::string& a, const char* b) {
        std::size_t i = 0;
        for (; i < a.size() && b[i] != '\0'; ++i) {
            const char x = (a[i] >= 'A' && a[i] <= 'Z') ? static_cast<char>(a[i] - 'A' + 'a') : a[i];
            const char y = (b[i] >= 'A' && b[i] <= 'Z') ? static_cast<char>(b[i] - 'A' + 'a') : b[i];
            if (x != y) return false;
        }
        return i == a.size() && b[i] == '\0';
    };
    int count = 0;
    for (const LuaEntryView& e : entries) {
        if (!e.key_is_string || !e.value_is_number) return false;  // 0079C0BB / 0079C0CC
        const bool x = !e.key_text_null && iequal(e.key, "x");    // __stricmp only with text
        if (!x && !iequal(e.key, "y") && !iequal(e.key, "z")) return false;
        ++count;
    }
    return count == 3;
}

void lua_read_xyz_0078fd70(std::array<float, 3>& out, const std::vector<LuaNumberEntry>& entries) {
    for (const LuaNumberEntry& e : entries) {
        if (e.key == "x") {
            out[0] = static_cast<float>(e.number);
        } else if (e.key == "y") {
            out[1] = static_cast<float>(e.number);
        } else if (e.key == "z") {
            out[2] = static_cast<float>(e.number);
        }
    }
}

void movie_camera_stop_0078e8e0(bool& running_391, float& fov_410, MovieCameraStopHost& host) {
    host.inherit_race_and_activate_0042a910();
    running_391 = false;
    fov_410 = 0.6981317f;  // 00CE7D20
}

// ===========================================================================
// Segment 43

void SessionMessageOut::put32(std::size_t offset, std::uint32_t v) noexcept {
    std::memcpy(body.data() + (offset - 0x18), &v, sizeof v);
}
void SessionMessageOut::put16(std::size_t offset, std::uint16_t v) noexcept {
    std::memcpy(body.data() + (offset - 0x18), &v, sizeof v);
}
void SessionMessageOut::put8(std::size_t offset, std::uint8_t v) noexcept { body[offset - 0x18] = v; }
std::uint32_t SessionMessageOut::get32(std::size_t offset) const noexcept {
    std::uint32_t v;
    std::memcpy(&v, body.data() + (offset - 0x18), sizeof v);
    return v;
}

namespace {
SessionMessageOut session_message(std::uint8_t type, std::uint32_t vtable) {
    SessionMessageOut m;
    m.type = type;      // 0075B430(type)
    m.vtable = vtable;  // stored over the base vtable with +4h = 1
    return m;
}
}  // namespace

SessionMessageOut session_broadcast_movcam_hand_targeted_0076d390(SessionBroadcastHost& host, std::uint16_t camera_id_174,
                                                                  std::uint8_t a, std::uint8_t b) {
    SessionMessageOut m = session_message(0x2D, 0x00D031C4u);
    m.put16(0x18, camera_id_174);
    m.put8(0x1A, a);
    m.put8(0x1B, b);
    host.broadcast_except_00784790(m);
    return m;
}

SessionMessageOut session_broadcast_movcam_refpos_polar_0076d460(SessionBroadcastHost& host, std::uint16_t camera_id_174,
                                                                 std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                                                 std::uint8_t d, std::uint32_t e) {
    SessionMessageOut m = session_message(0x2E, 0x00D031D8u);
    m.put16(0x18, camera_id_174);
    m.put32(0x1C, a);
    m.put32(0x20, b);
    m.put32(0x24, c);
    m.put8(0x28, d);
    m.put32(0x2C, e);
    host.broadcast_except_00784790(m);
    return m;
}

SessionMessageOut session_broadcast_movcam_refpos_radius_0076d510(SessionBroadcastHost& host, std::uint16_t camera_id_174,
                                                                  std::uint32_t a, std::uint8_t b, std::uint32_t c) {
    SessionMessageOut m = session_message(0x2F, 0x00D031ECu);
    m.put16(0x18, camera_id_174);
    m.put32(0x1C, a);
    m.put8(0x20, b);
    m.put32(0x24, c);
    host.broadcast_except_00784790(m);
    return m;
}

SessionMessageOut session_broadcast_movcam_path_point_0076d5b0(SessionBroadcastHost& host,
                                                               const std::array<std::uint32_t, 3>& p, std::uint32_t a,
                                                               std::uint8_t b, std::uint32_t c) {
    SessionMessageOut m = session_message(0x30, 0x00D03200u);
    m.put32(0x18, p[0]);
    m.put32(0x1C, p[1]);
    m.put32(0x20, p[2]);
    m.put32(0x24, a);
    m.put8(0x28, b);
    m.put32(0x2C, c);
    host.broadcast_except_00784790(m);
    return m;
}

SessionMessageOut session_broadcast_movcam_deviation_0076d660(SessionBroadcastHost& host, std::uint32_t a) {
    SessionMessageOut m = session_message(0x32, 0x00D03228u);
    m.put32(0x18, a);
    host.broadcast_except_00784790(m);
    return m;
}

SessionMessageOut session_broadcast_movcam_new_add_position_0076d6d0(SessionBroadcastHost& host, std::uint32_t key,
                                                                     std::uint32_t& counter_280) {
    SessionMessageOut m = session_message(0x33, 0x00D02ED4u);
    m.put32(0x18, key);
    m.put32(0x1C, 0);
    host.broadcast_except_00784790(m);
    counter_280 += 1;  // 0076D73A, after 0075B2D0
    return m;
}

SessionMessageOut session_broadcast_movcam_add_interpolator_0076e750(SessionBroadcastHost& host,
                                                                     const std::array<std::uint32_t, 5>& p) {
    SessionMessageOut m = session_message(0x31, 0x00D03214u);
    for (std::size_t i = 0; i < 5; ++i) m.put32(0x18 + 4 * i, p[i]);
    host.broadcast_except_00784790(m);
    return m;
}

void blackbar_write_0076ce40(const BlackbarMessage& m, bsp::NativeBitCursor* cursor) {
    write_native_byte_bits_00428ff0(cursor, m.type_10, 8);
    write_native_bool_bit_004290b0(cursor, m.flag_18 ? 1u : 0u);
}

void blackbar_read_0076ce70(BlackbarMessage& m, bsp::NativeBitCursor* cursor) {
    read_native_u8_bits_00428c70(cursor, &m.type_10, 8);
    read_native_bool_bit_00428d70(cursor, &m.flag_18);
}

void session_reset_all_slots_0076e340(SessionSlotHost& host) {
    for (int slot = 0; slot < 9; ++slot) host.set_slot_state_154(slot, 8);
}

ChatMessage33 chat_message_construct_0076f8c0(const std::string& text, std::uint32_t a, std::uint32_t b,
                                              const std::string& text2, const std::string& text3, std::uint32_t c,
                                              std::uint32_t d) {
    ChatMessage33 m;
    m.text_18 = text;
    m.word_20 = a;
    m.word_24 = b;
    m.text_28 = text2;
    m.text_30 = text3;
    m.word_38 = c;
    m.word_3c = d;
    return m;
}

std::string message34_construct_0076f9e0(const std::string& text) { return text; }

void session_send_chat_00772170(std::int32_t mode_f4, const ChatMessage33& args, ChatSendHost& host) {
    if (mode_f4 == 1) {
        host.host_relay_chat_00772020(args);
        return;
    }
    host.send_to_server_00770af0(args);
    if (host.chat_screen_present_00e198b4()) host.chat_screen_add_005d0770(args);
}

CountdownMessage countdown_construct_00771240(const std::string& text, std::uint32_t a, std::uint32_t b) {
    CountdownMessage m;
    m.text_18 = text;
    m.word_20 = a;
    m.word_24 = b;
    m.stop_28 = false;
    return m;
}

void session_broadcast_countdown_00772b30(const std::string& text, std::uint32_t a, std::uint32_t b, CountdownHost& host) {
    host.broadcast_except_00784790(countdown_construct_00771240(text, a, b));
}

void session_broadcast_countdown_stop_00770940(CountdownHost& host) {
    CountdownMessage m;
    m.stop_28 = true;  // 00770998
    host.broadcast_except_00784790(m);
}

VoiceMessage voice_message_construct_00774dc0(const VoicePacket& p) {
    VoiceMessage m;
    m.xuid_18 = p.xuid;
    m.data = p.data;  // 00774150
    return m;
}

void session_loading_begin_0076c190(std::int32_t mode_f4, bool& flag_4, bool& loading_27c, SessionSendHost& host) {
    if (mode_f4 == 1) {
        SessionMessageOut m = session_message(0x0E, 0x00D0300Cu);
        m.put32(0x18, 0);
        host.broadcast_except_00784790(m);
        host.flush_peer_buffers_007848f0();
    }
    flag_4 = false;
    loading_27c = true;
}

void session_loading_end_0076c230(std::int32_t mode_f4, bool& loading_27c, SessionSendHost& host) {
    if (mode_f4 == 1) {
        SessionMessageOut m = session_message(0x0E, 0x00D0300Cu);
        m.put32(0x18, 1);
        host.broadcast_except_00784790(m);
    }
    loading_27c = false;
}

void session_kick_player_peer_0076c790(const SessionPlayerRecord& record, SessionSendHost& host) {
    host.kick_peer_00784b10(record.peer_50, 1);
}

void session_send_to_player_slot_00772330(std::int32_t mode_f4, std::int32_t message_slot_1c, int local_slot_18ec,
                                          const std::array<SessionPlayerRecord, 8>& records, std::uintptr_t message,
                                          SessionSendHost& host) {
    if (mode_f4 != 1 && mode_f4 != 0) return;
    if (message_slot_1c == local_slot_18ec) {
        host.apply_counter_record_0052cf60(message);
        return;
    }
    const std::uintptr_t peer = records[static_cast<std::size_t>(message_slot_1c)].peer_50;
    if (peer != 0) host.send_to_peer_00770b50(peer, message);
}

void session_send_to_remote_player_00772af0(std::int32_t game_session_1fe4, const SessionPlayerRecord& record,
                                            std::uintptr_t message, SessionSendHost& host) {
    if (game_session_1fe4 == 2) return;
    if (record.peer_50 != 0 && record.remote_8) host.send_to_peer_00770b50(record.peer_50, message);
}

void session_broadcast_queued_0076c500(std::vector<std::uintptr_t>& outbound_25c, SessionSendHost& host) {
    for (const std::uintptr_t m : outbound_25c) {
        host.broadcast_message_00784830(m);
        if (m != 0) host.scalar_delete(m);
    }
    outbound_25c.clear();  // +260h = 0
}

SessionMessageOut session_broadcast_music_level_0076d280(std::uint32_t level, SessionSendHost& host) {
    SessionMessageOut m = session_message(0x2A, 0x00D0319Cu);
    m.put32(0x18, level);
    host.broadcast_except_00784790(m);
    return m;
}

void session_host_relay_to_team_00770bd0(std::int32_t mode_f4, const std::array<SessionPlayerRecord, 8>& records,
                                         int record_count_988, std::uint32_t team, std::uintptr_t message,
                                         std::uintptr_t except_peer, SessionSendHost& host) {
    if (mode_f4 != 1) return;
    for (int i = 0; i < record_count_988; ++i) {
        const SessionPlayerRecord& r = records[static_cast<std::size_t>(i)];
        if (r.peer_50 != except_peer && !r.local_9 && r.team_28 == team && r.peer_50 != 0) {
            host.send_to_peer_00770b50(r.peer_50, message);
        }
    }
}

float peer_rows_quality_00786010(const band0076_cc13::PeerRows& rows, int index) noexcept {
    return rows.rows[index].quality;
}

void peer_row_update_quality_007865d0(band0076_cc13::PeerRow& row, PeerQualityHost& host) {
    if (row.peer == 0) return;
    const float latency = host.transport_latency_c(row.peer);  // 007865FE FSTP
    const float pending = static_cast<float>(host.transport_pending_10(row.peer));  // CVTSI2SS
    const float a = host.interpolate_clamped_00419010(8.0f, 0.0f, 16.0f, 1.0f, pending);
    const float b = host.interpolate_clamped_00419010(0.15f, 0.0f, 0.6f, 1.0f, latency);
    host.smoothed_triplet_update_007862c0(row, b, a);  // 00786697, ECX = row+4
}

void peer_rows_update_007866b0(band0076_cc13::PeerRows& rows, float dt, PeerRowsUpdateHost& host) {
    if (rows.enabled == 0) return;
    if (0.10000000149011612 <= static_cast<double>(dt)) dt = 0.1f;
restart:
    for (int i = 0; i < 8; ++i) {
        band0076_cc13::PeerRow& row = rows.rows[i];
        if (row.peer == 0) continue;
        const int j = host.peer_record_slot(row.peer);
        if (j < 0 || j == i) continue;
        band0076_cc13::PeerRow& target = rows.rows[j];
        if (target.peer != 0) {  // 00786730: swap through a stack copy, rescan
            std::swap(row, target);
            goto restart;
        }
        target = row;
        row.peer = 0;
        row.probing = 0;
        row.probe_accum_small = 0.0f;
        row.probe_accum_large = 0.0f;
        row.value_04 = 0.0f;
        row.value_08 = 0.0f;
        row.quality = 1.0f;  // 00D7A24C
    }
    for (int i = 0; i < 8; ++i) host.send_probes_00786150(rows.rows[i], dt);
    const float timer = static_cast<float>(static_cast<double>(rows.period_timer) - dt);  // 007867B7
    rows.period_timer = timer;
    if (0.0f < timer) return;
    rows.period_timer = 0.2f;
    rows.connstate_accum = static_cast<float>(static_cast<double>(rows.connstate_accum) + 0.20000000298023224);
    for (int i = 1; i < 8; ++i) host.update_quality_007865d0(rows.rows[i]);
    if (rows.connstate_accum < 0.5f) return;
    rows.connstate_accum = 0.0f;
    if (!host.game_present_00e188a8()) return;
    host.send_connstate_00786390(rows);
}

void mnetwork_client_open_00782880(std::uint32_t& state_9c, MNetworkClientHost& host) {
    host.open_slot28();
    state_9c = 2;
}

void mnetwork_client_disconnect_007828c0(std::uint32_t& state_9c, MNetworkClientHost& host) {
    host.disconnect_slot48();
    state_9c = 2;
}

void mnetwork_client_end_lan_search_007828a0(MNetworkClientHost& host) { host.end_lan_search_slot30(); }

void mnetwork_client_connect_007828b0(MNetworkClientHost& host) { host.connect_slot40(); }

void mnetwork_client_clear_and_search_00785330(std::uint32_t a, std::uint32_t b, MNetworkClientHost& host) {
    host.clear_hosts_00785170();
    host.begin_lan_search_slot2c(a, b);
}

void mnetwork_lock_incoming_0076b320(TransportLock& lock_34, CriticalSectionHost& host) {
    host.enter_critical_section(lock_34.critical_section);
    lock_34.depth_18 += 1;
}

void mnetwork_lock_removals_0076b350(TransportLock& lock_24, CriticalSectionHost& host) {
    host.enter_critical_section(lock_24.critical_section);
    lock_24.depth_18 += 1;
}

bool start_gate_tick_0076b400(std::int16_t& start_10) noexcept {
    if (start_10 >= 0) return true;
    start_10 = static_cast<std::int16_t>(start_10 + 1);
    return false;
}

void session_broadcast_start_gate_state_0076fec0(std::int32_t mode_f4, StartGateObjectsHost& host) {
    if (mode_f4 != 1) return;
    for (std::size_t i = 0; i < host.object_count(); ++i) {
        if (!host.flag_15c(i)) continue;  // 0076FF13
        const std::string name = host.name_slot14(i);
        host.apply_name_slotc(i, name, 0);  // 0076FF77
    }
}

float session_update_start_gate_007713a0(std::int32_t mode_f4, bool has_game, std::array<StartGateRecord, 8>* records,
                                         int local_index_18ec, float dt, StartGateHost& host) {
    if (mode_f4 == 0) return dt;
    if (!has_game || records == nullptr) return 0.0f;
    StartGateRecord& local = (*records)[static_cast<std::size_t>(local_index_18ec)];
    if (local.start_10 >= 1) return dt;
    if (mode_f4 == 1) {
        if (start_gate_tick_0076b400(local.start_10)) {
            int waiting = 0;
            for (const StartGateRecord& r : *records) {
                if (!r.local_9 && r.start_10 < 1) ++waiting;
            }
            if (waiting < 2 && host.delay_passed_00e19698()) {
                host.broadcast_start_gate_state_0076fec0();
                host.mission_clock_reset_00874640(0);
                if (host.powerups_present_00f88c30()) host.powerups_broadcast_008eb710();
                host.broadcast_lets_play_0075d3f0();
                host.flush_peer_buffers_007848f0();
                local.start_10 = 1;
            }
        }
    } else if (start_gate_tick_0076b400(local.start_10) && host.delay_passed_00e19698()) {
        host.send_lets_play_to_server_00770af0();
    }
    return local.start_10 == -2 ? dt : 0.0f;
}

void session_send_local_voice_00774e30(std::int32_t mode_f4, const VoicePacket& p, VoiceSendHost& host) {
    if (mode_f4 != 1 && mode_f4 != 2) return;
    const VoiceSendHost::Record record = host.find_player_by_xuid_0076d7d0(p.xuid);
    if (!record.found || record.peer_50 == 0) return;
    host.stamp_voice_time_007f7490(record.peer_50, 1);  // 00774E8E
    if (host.game_state_5d4() == 0x0D) {
        const VoiceSendHost::Record again = host.find_player_by_xuid_0076d7d0(p.xuid);  // 00774EAA
        host.send_to_team_unmuted_00770d40(again.team_28, voice_message_construct_00774dc0(p), p.xuid, record.peer_50);
        return;
    }
    host.send_to_unmuted_00770c50(voice_message_construct_00774dc0(p), p.xuid, record.peer_50);
}

// ===========================================================================
// Packet cc13-cam3 (docs/CC13_CAM_3.md)

// ---------------------------------------------------------------------------
// MovCam frame, 0079C200

void movcam_latch_input_0079c200(MovCamFrameInput& in, MovCamFrameInputHost& host) {
    for (int i = 0; i < 4; ++i) {
        in.edges_51c[i] = in.edges_51c[i] || host.was_pressed_004c43c0(0x78 + i);  // 0079C211..0079C251
    }
    for (int i = 0; i < 5; ++i) in.axes_520[i] = host.record_value_24(0x7C + i);  // 0079C257..0079C2BA
    in.edges_534[0] = in.edges_534[0] || host.was_pressed_004c43c0(0x82);
    in.edges_534[1] = in.edges_534[1] || host.was_pressed_004c43c0(0x83);
    in.edges_534[2] = in.edges_534[2] || host.was_pressed_004c43c0(0x81);
}

namespace {
// 0079C3CD / 0079C775: each product spilled as a float, then the float sum.
void extrapolate_angles(MovCamFrame& c, float dt) {
    const float d_theta = static_cast<float>(static_cast<double>(c.angle_rate_3cc[0]) * dt);
    const float d_rho = static_cast<float>(static_cast<double>(dt) * c.angle_rate_3cc[1]);
    c.theta_3c4 = static_cast<float>(static_cast<double>(c.snapshot_theta_4ec) + d_theta);
    c.rho_3c8 = static_cast<float>(static_cast<double>(c.snapshot_rho_4f0) + d_rho);
}

void frame_angles(MovCamFrame& c, float dt, MovCamFrameHost& host) {
    if (c.follow_target_3e4 != 0) {
        host.follow_00795180(c, dt);
    } else {
        extrapolate_angles(c, dt);
    }
    host.wrap_and_clamp_007904d0(c);
}
}  // namespace

void movcam_frame_0079c200(MovCamFrame& c, MovCamFrameInput& in, float dt, float base_fov_f889b4,
                           MovCamFrameInputHost& input, MovCamFrameHost& host) {
    movcam_latch_input_0079c200(in, input);
    const std::uint32_t mode = c.mode_384;
    bool ground_and_pose = true;
    if (mode == 0 || mode == 1 || mode == 4 || mode == 3 || mode == 6) {  // 0079C6EF
        for (int i = 0; i < 3; ++i) {
            const float step = static_cast<float>(static_cast<double>(c.velocity_3a0[i]) * dt);
            c.position_394[i] = static_cast<float>(static_cast<double>(c.snapshot_position_4e0[i]) + step);
        }
        frame_angles(c, dt, host);
    } else if (mode == 2) {  // 0079C33F
        const float zoom =
            static_cast<float>((static_cast<double>(dt) * c.zoom_rate_514 + 1.0) * c.snapshot_zoom_4f4);
        c.zoom_460 = zoom;
        if (1.0 > static_cast<double>(zoom)) {
            c.zoom_460 = 1.0f;  // 00D7A24C
        } else if (zoom > 21.0f) {
            c.zoom_460 = 21.0f;  // 00D04B18
        }
        host.set_field_of_view_00b6fbb0(static_cast<float>(static_cast<double>(base_fov_f889b4) / c.zoom_460));
        frame_angles(c, dt, host);
        const Mat4 world = host.world_matrix_0042d7e0(c.entity_38c);  // 0079C42F
        const Mat4 ry = host.rotation_y_negated_0042b490(c.rho_3c8);
        const Mat4 rx = host.rotation_x_negated_0042b450(c.theta_3c4);
        const Mat4 r = host.multiply_00413920(rx, ry);
        const Mat4 local = host.multiply_00413920(r, c.matrix_420);
        c.matrix_74 = host.multiply_00413920(local, world);  // 0042B680 into +74h
        ground_and_pose = false;
    } else if (mode == 5) {  // 0079C4AA
        const float t = c.snapshot_path_t_4fc;
        const float end = c.path_end_46c;
        if (end > t) {
            const float a = static_cast<float>(static_cast<double>(end) - 0.009999999776482582);
            const float b = static_cast<float>(static_cast<double>(t) + static_cast<double>(c.top_speed_4d0_34) * dt);
            c.path_t_468 = b > a ? a : b;  // 00415510(&a, &b)
        } else if (t > end) {
            const float a = static_cast<float>(static_cast<double>(end) + 0.009999999776482582);
            const float b = static_cast<float>(static_cast<double>(t) - static_cast<double>(c.top_speed_4d0_34) * dt);
            c.path_t_468 = a > b ? a : b;  // 00415550(&a, &b)
        }
        if (c.has_knot_path_474) {
            host.path_update_007972f0(c);
        } else {
            Vec3 direction{};
            host.sample_world_007b04c0(c.path_t_468, c.position_394, direction);
            const Vec3 point{c.position_394[0] + direction[0], c.position_394[1] + direction[1],
                             c.position_394[2] + direction[2]};  // 004142A0
            host.aim_at_point_00794070(c, point);
        }
        if (static_cast<double>(dt) > 9.999999747378752e-05) {  // 0079C594
            for (int i = 0; i < 3; ++i) {
                const float d = c.position_394[i] - c.snapshot_position_4e0[i];  // 00427C90
                c.velocity_3a0[i] = d / dt;                                      // 0042AE20
            }
            const float d_theta = c.theta_3c4 - c.snapshot_theta_4ec;
            const float d_rho = c.rho_3c8 - c.snapshot_rho_4f0;
            c.angle_rate_3cc[0] = d_theta / dt;
            c.angle_rate_3cc[1] = d_rho / dt;
        }
    } else if (mode == 7) {  // 0079C652
        c.position_394 = c.entity_38c != 0
                             ? host.transform_point_00414d10(host.world_matrix_0042d7e0(c.entity_38c), c.eye_point_4c4)
                             : c.eye_point_4c4;
        const Vec3 aim = c.entity_390 != 0
                             ? host.transform_point_00414d10(host.world_matrix_0042d7e0(c.entity_390), c.aim_point_4b8)
                             : c.aim_point_4b8;
        host.aim_at_point_00794070(c, aim);
    } else {
        ground_and_pose = false;
    }
    if (ground_and_pose) {  // 0079C7C8
        host.keep_above_ground_00790540(c);
        host.apply_pose_00799750(c);
    }
    c.pose_valid_c8 = false;  // 0079C7E8
    c.pose_valid_10c = false;
    host.invalidate_children_0042ed50();
    host.apply_shake_and_publish_004329d0(dt);
}

// ---------------------------------------------------------------------------
// MovCam lifetime

void movcam_construct_0079f480(MovCamFrame& frame, MovCamOwned& owned, MovCamConstructHost& host) {
    host.construct_base_00432750();
    owned.vtables = {0x00D04B58u, 0x00D04B3Cu, 0x00D04B34u, 0x00D04B1Cu};  // 0079F4A7..0079F4BB
    owned.camera_ref_3f0 = cc13cam::CameraRef{};
    owned.camera_ref_vtable_3f0 = 0x00D04660u;
    owned.records_47c.clear();
    owned.callback_49c.clear();
    owned.callback_4a4.clear();
    owned.list_head_4d8 = host.buy_list_head_00796770();  // 0079F54A
    owned.interpolators_4d4.clear();
    frame.mode_384 = 0;
    frame.entity_38c = 0;
    frame.entity_390 = 0;
    frame.velocity_3a0 = {0.0f, 0.0f, 0.0f};
    frame.position_394 = {0.0f, 0.0f, 0.0f};
    frame.angle_rate_3cc = {0.0f, 0.0f};
    frame.theta_3c4 = 0.0f;
    frame.rho_3c8 = 0.0f;
    frame.snapshot_theta_4ec = frame.theta_3c4;  // 0079F5FF
    frame.snapshot_rho_4f0 = frame.rho_3c8;
    frame.snapshot_position_4e0 = frame.position_394;
    frame.follow_target_3e4 = 0;
    frame.has_knot_path_474 = false;  // +464h (the path) is zeroed too
    owned.limits_4d0 = host.limits_block_e198c4_98();  // 0079F64D
    host.reset_speed_limits_005cb970(owned.limits_4d0);
    host.reset_accel_time_005cb9c0(owned.limits_4d0);
    host.reset_deviation_factor_005cafb0(owned.limits_4d0);
    host.reset_overview_height_005cafc0(owned.limits_4d0);
    owned.flag_538 = false;
    owned.args_4b0 = 0;
    owned.args_4ac = 0;
    owned.flag_4b4 = false;
    owned.field_494 = 0.0f;
    owned.field_498 = 0.0f;
    owned.field_53c = 0.0f;
}

void movcam_destruct_0079d640(MovCamOwned& owned, MovCamDestroyHost& host) {
    for (std::uintptr_t& payload : owned.interpolators_4d4) {
        if (payload != 0) {
            host.free_crt(payload);
            payload = 0;
        }
    }
    host.clear_path_records_0079c830();
    if (owned.args_4ac != 0) {
        host.free_args_vector(owned.args_4ac);
        owned.args_4ac = 0;
    }
    if (owned.args_4b0 != 0) {
        host.free_args_vector(owned.args_4b0);
        owned.args_4b0 = 0;
    }
    host.free_list_nodes_and_head(owned.list_head_4d8);
    owned.interpolators_4d4.clear();
    owned.list_head_4d8 = 0;
    owned.callback_4a4.clear();  // 0079D776, sized pool
    owned.callback_49c.clear();  // 0079D79E
    owned.records_47c.clear();   // 0079D7C2 _free, then +47Ch..+484h = 0
    host.destroy_camera_ref_00695870();
    host.destruct_base_0042a830();
}

// ---------------------------------------------------------------------------
// Movie-camera keyframes and tracks

void movie_keyframe_construct_at_0079c910(MovieKeyframeInit& k, const Vec3& p, std::size_t entity,
                                          MovieKeyframeInitHost& host) {
    k.entity_1c = 0;
    k.flag_18 = true;
    k.name_fc.clear();
    k.id_da = -1;
    k.eval_4c = p;
    k.work_40 = p;
    k.local_34 = p;
    if (!host.set_entity_00799d70(k, entity)) {  // 0079C9B4
        if (entity != 0) {
            Vec3* vectors[3] = {&k.local_34, &k.work_40, &k.eval_4c};
            for (Vec3* v : vectors) {
                const Vec3 e = host.entity_position_fc(entity);  // 0079C9D3 / 0079CA06 / 0079CA39
                for (int i = 0; i < 3; ++i) (*v)[i] = e[i] + (*v)[i];
            }
        }
        entity = 0;
    }
    k.flag_dc = false;
    k.zoom_c8 = 1.0f;  // 00D7A24C
    k.zoom_c4 = 1.0f;
    k.parent_attached_24 = entity != 0;
    k.eval_up_70 = {0.0f, 1.0f, 0.0f};
    k.work_up_64 = k.eval_up_70;
    k.up_58 = k.work_up_64;
    k.start_f0 = 0.0f;
    k.blend_f4 = 0.0f;
    k.state_f8 = 0;
    k.modifier_e4 = 0;
    k.event_ec = 0;
    k.use_current_d8 = false;
    k.terrain_avoid_d9 = false;
    k.linked_25 = false;
    k.partner_20 = 0;
    k.flag_bc = false;
    k.then_none_30 = false;
    k.wanderer_26 = true;
    k.smooth_d0 = 1.0f;
    k.smooth_cc = 1.0f;
    k.has_position_27 = true;
    k.nonlinear_d4 = 0.5f;  // 00CE3800
    k.camera_28 = true;
    k.transform_2c = 1;
}

void movie_keyframe_construct_00799b00(MovieKeyframeInit& k) {
    k.entity_1c = 0;
    k.flag_18 = true;
    k.name_fc.clear();
    k.fov_divisor_c0 = -1.0f;  // 00D7A260
    k.id_da = -1;
    k.parent_attached_24 = false;
    k.eval_4c = {0.0f, 0.0f, 0.0f};
    k.work_40 = k.eval_4c;
    k.local_34 = k.work_40;
    k.zoom_c8 = 1.0f;  // 00D7A24C
    k.zoom_c4 = 1.0f;
    k.eval_up_70 = {0.0f, 1.0f, 0.0f};
    k.work_up_64 = k.eval_up_70;
    k.up_58 = k.work_up_64;
    k.start_f0 = 0.0f;
    k.blend_f4 = 0.0f;
    k.flag_dc = false;
    k.flag_e8 = false;
    k.flag_bc = false;
    k.field_4 = 0;
    k.state_f8 = 0;
    k.modifier_e4 = 0;
    k.event_ec = 0;
    k.use_current_d8 = false;
    k.terrain_avoid_d9 = false;
    k.linked_25 = false;
    k.partner_20 = 0;
    k.then_none_30 = false;
    k.wanderer_26 = true;
    k.smooth_d0 = 1.0f;
    k.smooth_cc = 1.0f;
    k.has_position_27 = true;
    k.nonlinear_d4 = 0.5f;  // 00CE3800
    k.camera_28 = true;
    k.transform_2c = 1;
}

void movie_keyframe_destruct_00799ca0(MovieKeyframeInit& k, std::uintptr_t self, MovieKeyframeLifetimeHost& host) {
    if (k.modifier_e4 != 0) {
        host.delete_virtual(k.modifier_e4);
        k.modifier_e4 = 0;
    }
    if (k.partner_20 != 0) {
        MovieKeyframeInit* partner = host.partner(k.partner_20);
        if (partner != nullptr && partner->partner_20 == self) partner->partner_20 = 0;
    }
    if (k.event_ec != 0) host.delete_virtual(k.event_ec);
    k.event_ec = 0;
    if (k.entity_1c != 0) {
        host.unregister_006952a0(k.entity_1c, &k);
        k.entity_1c = 0;
    }
    k.name_fc.clear();  // sized pool, length+1
    host.destroy_base_004b7ef0();
}

void movie_keyframe_reset_00799ea0(MovieKeyframeInit& k, MovieKeyframeLifetimeHost& host) {
    if (k.entity_1c != 0) {
        host.unregister_006952a0(k.entity_1c, &k);
        k.entity_1c = 0;
    }
    k.parent_attached_24 = false;
    k.use_current_d8 = true;
    k.blend_f4 = 0.0f;
    k.start_f0 = 0.0f;
}

void movie_track_construct_0079cfc0(MovieTrackList& t, MovieTrackHost& host) {
    t.head_6c = host.buy_list_head_007967e0();
    t.keys.clear();
    t.camera_74 = 0;
    t.field_80 = 0;
    t.flag_64 = false;
    t.field_78 = 0;
}

void movie_track_destruct_0079a190(MovieTrackList& t, MovieTrackHost& host) {
    t.camera_74 = 0;
    while (!t.keys.empty()) {
        const std::uintptr_t key = t.keys.front();
        if (key != 0) host.delete_virtual(key);
        t.keys.erase(t.keys.begin());  // node unlinked and _freed, size - 1
    }
    host.tidy_list_00796800(t.head_6c);
}

void hud_movie_camera_destruct_0079a260(HudMovieCameraDestroyHost& host) {
    const std::array<std::int64_t, 2> c = host.clock_counter_frequency_slot20();  // 0079A2B5
    const float seconds = static_cast<float>(static_cast<double>(c[0]) / static_cast<double>(c[1]));
    const float ms = static_cast<float>(static_cast<double>(seconds) * 1000.0);  // 00CE47A0
    const auto truncated = static_cast<std::int64_t>(ms);                         // FISTP, RC = 11b
    host.seed_stream_00bd2fd0(1, static_cast<std::uint32_t>(truncated));          // RNG stream 1
    host.destruct_track_0079a190(true);   // +498h
    host.destruct_track_0079a190(false);  // +414h
    host.destruct_base_0042a830();
}

// ---------------------------------------------------------------------------
// Vehicle-camera window editor, 0079F9E0

std::string movcam_edit_vehicle_camera_window_0079f9e0(const MovCamInputs& in, std::int32_t index_388,
                                                       float theta_3c4, float rho_3c8, VehicleCameraEditHost& host) {
    const auto classify = [](float v, int below, int above) {
        if (-0.5f > v) return below;  // 00CE69D0
        if (v > 0.5f) return above;   // 00CE3800
        return -1;
    };
    const int horz = classify(in.walk_h_504, 1, 0);
    const int vert = classify(in.walk_v_508, 0, 1);
    if (horz != -1 && vert != -1) return {};
    std::string text = host.class_name_54() + "; mov.cam.pos.: [" + std::to_string(index_388 + 1) + "]\n\t\t\t\t";
    const auto append = [&text](const char* key, float value) {
        char number[64];
        std::snprintf(number, sizeof number, "%f", static_cast<double>(value));  // 006AF2B0
        text += std::string("[\"") + key + "\"] = " + number + ",";
    };
    if (horz != -1) {
        append(horz != 0 ? "MaxHorzAngle" : "MinHorzAngle", rho_3c8);
    } else if (vert != -1) {
        append(vert != 0 ? "MaxVertAngle" : "MinVertAngle", theta_3c4);
    }
    host.append_log_007996c0(text);  // 007A011C
    std::vector<VehicleCameraElement>& cameras = host.class_cameras_bc();
    if (static_cast<std::int64_t>(index_388) >= static_cast<std::int64_t>(cameras.size())) {
        host.grow_cameras_005cd640(static_cast<std::size_t>(index_388) + 1);
    }
    VehicleCameraElement& e = host.class_cameras_bc()[static_cast<std::size_t>(index_388)];
    std::int32_t message = 0;
    if (horz == 1) {
        e.yaw_hi_4c = rho_3c8;
        message = 1;
    } else if (horz == 0) {
        e.yaw_lo_48 = rho_3c8;
        message = 3;
    } else if (vert == 1) {
        e.pitch_hi_54 = theta_3c4;
        message = 0;
    } else if (vert == 0) {
        e.pitch_lo_50 = theta_3c4;
        message = 2;
    } else {
        return text;
    }
    host.show_message_005cba50(message);
    return text;
}

// ---------------------------------------------------------------------------
// Sector debug draw, 007906E0

void camera_debug_draw_sector_007906e0(const Mat4& m, const Vec3& point, float yaw_lo, float yaw_hi, float pitch_lo,
                                       float pitch_hi, std::size_t element, const std::array<float, 4>& colour,
                                       OrbitPointHost& orbit, SectorDebugDrawHost& draw) {
    constexpr float kDist = 4.0f;  // 00CE3D34
    const auto at = [&](float yaw, float pitch) { return camera_orbit_point_007905b0(point, yaw, pitch, kDist, m, orbit); };
    const auto line = [&](const Vec3& p0, const Vec3& p1) { draw.line_0085f990(p0, p1, colour, colour); };
    const Vec3 c1 = at(yaw_lo, pitch_lo);
    const Vec3 c2 = at(yaw_lo, pitch_hi);
    const Vec3 c3 = at(yaw_hi, pitch_lo);
    const Vec3 c4 = at(yaw_hi, pitch_hi);
    const Vec3 centre = orbit.transform_004142e0(point, m);  // 0079082F
    draw.text_at_world_point_00861260(centre, element);
    line(centre, c1);
    line(centre, c2);
    line(centre, c3);
    line(centre, c4);
    constexpr double kQuarterPi = 0.7853981852531433;  // 00CEDCD0
    float yaw_step = static_cast<float>(0.125 * (static_cast<double>(yaw_hi) - yaw_lo));  // 0079099B DC CA
    if (yaw_lo > yaw_hi) yaw_step = static_cast<float>(yaw_step + kQuarterPi);
    float pitch_step = static_cast<float>(0.125 * (static_cast<double>(pitch_hi) - pitch_lo));
    if (pitch_lo > pitch_hi) pitch_step = static_cast<float>(kQuarterPi + pitch_step);
    const auto step = [](double k, float s, float lo) { return static_cast<float>(k * s + lo); };
    for (int i = 0; i < 8;) {  // 00790A0E
        const float fi = static_cast<float>(i);
        const float p0 = step(fi, pitch_step, pitch_lo);
        ++i;
        const float fi1 = static_cast<float>(i);
        const float p1 = step(fi1, pitch_step, pitch_lo);
        line(at(yaw_lo, p0), at(yaw_lo, p1));
        line(at(yaw_hi, p0), at(yaw_hi, p1));
        const float y0 = step(fi, yaw_step, yaw_lo);  // 00790BFA
        const float y1 = step(fi1, yaw_step, yaw_lo);
        line(at(y0, pitch_lo), at(y1, pitch_lo));
        line(at(y0, pitch_hi), at(y1, pitch_hi));
    }
    const float mid_pitch = step(4.0, pitch_step, pitch_lo);  // 00790DE8
    line(at(step(3.0, yaw_step, yaw_lo), mid_pitch), at(step(5.0, yaw_step, yaw_lo), mid_pitch));
    const float mid_yaw = step(4.0, yaw_step, yaw_lo);  // 00790EFA
    line(at(mid_yaw, step(3.0, pitch_step, pitch_lo)), at(mid_yaw, step(5.0, pitch_step, pitch_lo)));
}

// ---------------------------------------------------------------------------
// Session mode, 0076FAD0

namespace {
void delete_transport(std::uintptr_t& t, SessionModeHost& host) {
    if (t == 0) return;
    host.delete_virtual(t);
    t = 0;
}
}  // namespace

void session_set_mode_0076fad0(SessionModeState& s, std::int32_t mode, SessionModeHost& host) {
    if (mode == 0 && host.world_present_e198c4()) {
        s.leave_pending_27d = true;  // 0076FB00
        return;
    }
    if (s.mode_f4 == mode) return;
    s.previous_f8 = s.mode_f4;
    s.flag_274 = false;
    s.timer_278 = -1.0f;  // 00D7A260
    host.singleton_f8a2fc_slot154();
    host.clear_scene_records_004c8900();
    host.set_game_608();
    if (mode == 1) {
        host.reset_peer_rows_007868c0(true);
        delete_transport(s.client_18c, host);
        delete_transport(s.server_188, host);
        host.set_route_default_e0af1c(1);
        host.please_wait_frame_0060d570();
        s.server_188 = host.create_server_00a42170(s.server_option_16a);  // 0076FBDF
        host.please_wait_frame_0060d570();
        if (host.transport_state_9c(s.server_188) != 3) host.set_transport_state_9c(s.server_188, 3);
        const std::vector<std::string> names = host.scene_record_names();
        host.set_scene_count_f1af2c(static_cast<std::int32_t>(names.size()));
        for (const std::string& name : names) {
            host.build_scene_record_004e1d70(name);  // 0076FC7C (name, 0, -1, 0)
            host.please_wait_frame_0060d570();
        }
        host.select_scene_record_004c6890();
        host.reset_participants_004bb160();
        host.clear_game_1ee5();
        host.reset_peer_pairs_00778590();
    } else if (mode == 2) {
        host.reset_peer_rows_007868c0(false);
        delete_transport(s.client_18c, host);
        delete_transport(s.server_188, host);
        s.field_125 = 0xFF;
        host.set_route_default_e0af1c(2);
        s.client_18c = host.create_client_00a41c70();  // 0076FD3F
        s.field_248 = 2;
        host.reset_participants_004bb160();
        host.clear_game_1ee5();
        host.reset_peer_pairs_00778590();
    } else if (mode == 0) {
        host.reset_peer_rows_007868c0(false);
        if (s.mode_f4 == 2 && s.client_18c != 0) {
            const std::uintptr_t peer = host.client_peer_0076f8a0(s.client_18c);
            if (peer != 0) {
                host.debug_print_004254b0("--- sent MQuitMsg");  // 00D039F8
                if (host.transport_state_9c(s.client_18c) == 7) {
                    host.send_quit_00783dc0(s.client_18c, host.client_peer_0076f8a0(s.client_18c));
                    host.flush_007848f0(s.client_18c);
                }
            }
        }
        host.clear_e19698_4();
        host.mark_local_slot_left();
        host.set_route_default_e0af1c(1);
        if (s.mode_f4 == 2) host.refresh_from_online_session_0076f0e0();
        s.timer_278 = 2.0f;  // 00CE3958
        s.flag_128 = false;
        s.flag_12a = false;
        s.flag_12b = false;
        host.clear_e17bf2();
        host.set_e08178(1.0f);  // 00D7A24C
    }
    s.mode_f4 = mode;
    s.field_270 = 0.0f;
    if (mode == 1) host.set_game_mode_004bc890();
}

// ---------------------------------------------------------------------------
// Peers joining and leaving, 00774AB0 / 00772EC0

bool session_on_peer_added_00774ab0(std::int32_t mode_f4, std::uintptr_t peer, PeerAddedHost& host, LiveService& live) {
    host.append_to_active_transport_list(peer);  // 00774AF4
    if (mode_f4 == 1) host.add_peer_row_007864e0(peer);
    if (!live.present_f8a2fc() || !host.has_selected_user_00a3e510()) return true;
    PeerPlayer* record = host.player_record_8(peer);
    std::uint64_t xuid = 0;
    if (record != nullptr) {
        xuid = record->xuid_28;
        live.register_talker_slot158(xuid, record->name_0, record->flag_58);  // 00774BA0
    } else {
        xuid = host.local_xuid_698();
        live.register_talker_slot158(xuid, host.local_name_6a0_or_68c(), false);  // 00774C14
        return false;  // 00774C16 reads [0+2Ch]
    }
    host.mute_table_add_00778620(record->xuid_28);
    const std::uint32_t muted = live.mute_state_slot24(record->xuid_28);
    host.set_muted_00771630(record->xuid_28, muted);  // 00774C4C
    if (host.local_xuid_698() != xuid) live.slot_ac(xuid);
    if (live.slot_180()) live.slot_18c(!record->flag_58);
    return true;
}

void session_on_peer_removed_00772ec0(std::int32_t mode_f4, std::vector<std::uintptr_t>& messages_24c,
                                      std::uintptr_t peer, PeerRemovedHost& host, LiveService& live) {
    if (mode_f4 == 1) host.remove_peer_row_00786be0(peer);
    for (std::size_t i = 0; i < messages_24c.size();) {  // 00772F10
        if (host.message_sender_8(messages_24c[i]) == peer) {
            host.delete_virtual(messages_24c[i]);
            messages_24c.erase(messages_24c.begin() + static_cast<std::ptrdiff_t>(i));
        } else {
            ++i;
        }
    }
    if (live.present_f8a2fc() && peer != 0) {
        if (PeerPlayer* record = host.player_record_8(peer)) {
            live.unregister_talker_slotb0(record->xuid_28);  // 00772FB0
            host.mute_table_remove_00778690(record->xuid_28);
            if (live.slot_180()) {
                live.slot_190(!record->flag_58);
                live.slot_15c(record->xuid_28, record->name_0);  // 00773039
            }
        }
        if (!host.game_218c() && live.slot_19c() && host.key_index_below_player_count(peer)) {
            char text[64];
            std::snprintf(text, sizeof text, "%x UnregisterKey remove peer", host.key_byte_dbf(peer));
            host.debug_print_004254b0(text);
            host.xnet_unregister_key_00a4d488(peer);
        }
    }
    host.debug_print_004254b0(peer != 0 ? "Remove Valid Peer!" : "Remove Empty Peer!");
    host.request_disconnect_00a3bbb0(peer, 2.0f);  // 00CE3958
    host.remove_from_active_transport_list_0076f6f0(peer);
    if (peer != 0) host.delete_virtual(peer);
}

// ---------------------------------------------------------------------------
// Game setup broadcast, 00773860

void session_send_game_setup_00773860(const GameSetupLists& lists, GameSetupHost& host) {
    GameSetupMessage m;
    for (std::int32_t kind = 0; kind < 6; ++kind) {
        m.kind_18 = kind;
        for (const std::string& text : lists.texts[static_cast<std::size_t>(kind)]) {
            const std::size_t n = std::min(text.size(), m.payload_1c.size() - 1);  // the image copies unbounded
            std::memcpy(m.payload_1c.data(), text.data(), n);
            m.payload_1c[n] = 0;
            host.broadcast_00784790(m);  // 0077391F
        }
    }
    m.kind_18 = 6;  // 00773C05
    std::size_t at = 0;
    const auto put = [&m, &at](std::uint8_t b) {
        if (at < m.payload_1c.size()) m.payload_1c[at] = b;
        ++at;
    };
    for (std::size_t i = 0; i < 3; ++i) {
        m.counts_11c[i] = static_cast<std::int32_t>(lists.flags[i].size());
        for (bool bit : lists.flags[i]) put(bit ? 1 : 0);
    }
    for (std::size_t i = 0; i < 3; ++i) {
        m.counts_11c[3 + i] = static_cast<std::int32_t>(lists.values[i].size());
        for (std::int32_t v : lists.values[i]) put(static_cast<std::uint8_t>(v));
    }
    host.broadcast_00784790(m);  // 00773F71
}

// ---------------------------------------------------------------------------
// Host removes a player, 007702E0

bool session_host_remove_peer_player_007702e0(std::uintptr_t peer, HostRemovePlayerHost& host) {
    HostRemovedPlayer* record = host.player_record_d50(peer);
    if (record == nullptr) return false;
    const std::string name = record->name_58;  // 00770316
    const std::int32_t player_index = record->player_index_20;
    host.select_player_0076c840(record->slot_1c, -1);
    host.reassign_peer_units_0076f2b0(record->player_index_20);
    host.record_0077e4c0(*record);
    host.broadcast_remove_player_06(record->slot_1c);  // 00770384
    host.notify_player_left_004d0fc0(*record);
    host.clear_player_record_d50(peer);
    if (host.live_session_4d()) host.live_remove_remote_player_00a4b180(record->xuid_80);
    host.recount_slots_0076dbe0();
    const std::int32_t state = host.game_state_5d4();
    if (state == 0x0D || state == 0x0A || state == 0x10) host.set_game_player_name(player_index, name);
    host.broadcast_server_info_0076b670();  // 00770494
    const SessionSlotCounts c = host.slot_counts();
    char text[160];
    std::snprintf(text, sizeof text, "MMultiPlayer::ServerRemovePlayer: slot: %d/%d, private: %d/%d, will: %d\n",
                  c.field_128, c.field_127, c.field_12a, c.field_129, c.field_12b);
    host.debug_print_004254b0(text);
    host.post_system_chat_line_005cf320(name, std::string());  // 00CE3A0C ""
    return true;
}

// ---------------------------------------------------------------------------
// MT_CONNSTATE, 00786390

ConnStateMessage connstate_construct_00786390(const band0076_cc13::PeerRows& rows) noexcept {
    ConnStateMessage m;
    for (std::size_t i = 0; i < 8; ++i) {
        m.rows_18[i] = {rows.rows[i].value_04, rows.rows[i].value_08, rows.rows[i].quality};
    }
    return m;
}


// ---------------------------------------------------------------------------
// MultiScore net table message E0h

void net_table_key_set_string_00781430(mp_cc13::NetTableKey& key, const char* text) {
    key.text = text != nullptr ? text : "";
    key.is_string = true;
}

void net_table_key_set_integer_007814e0(mp_cc13::NetTableKey& key, std::uint32_t number) {
    key.text.clear();
    key.number = number;
    key.is_string = false;
}

std::string net_table_key_copy_name_00781540(const mp_cc13::NetTableKey& key) {
    return key.text;
}

NetTableSyncMessage net_table_sync_construct_00781f60(const std::vector<mp_cc13::NetTableEntry>& entries) {
    NetTableSyncMessage m;
    m.entries = entries;  // 00781890
    return m;
}

void net_table_sync_write_00781590(const NetTableSyncMessage& m, NetTableWriteHost& host) {
    host.write_extended_header_0075b480();
    host.write_bits_00429070(static_cast<std::uint32_t>(m.entries.size()), 5);  // 007815C0, not clamped
    for (const mp_cc13::NetTableEntry& e : m.entries) {
        host.write_bool_004290b0(e.key.is_string);
        if (e.key.is_string) {
            host.write_owned_string_00429ac0(net_table_key_copy_name_00781540(e.key));
        } else {
            host.write_bits_00429070(e.key.number, 32);  // 00781694
        }
        host.write_bool_004290b0(e.value.is_string);
        if (e.value.is_string) {
            host.write_owned_string_00429ac0(e.value.text);
        } else {
            host.write_numeric_float_004295c0(e.value.number);  // 007816FE
        }
    }
}

// ---------------------------------------------------------------------------
// Small session helpers

void session_stamp_player_voice_time_0076c450(std::uintptr_t peer, std::uint32_t value, VoiceStampHost& host) {
    for (int i = 0; i < 8; ++i) {
        if (host.player_record_peer_54(i) == peer) {
            host.stamp_voice_time_007f7490(i, value);  // 0076C48B
            return;
        }
    }
}

std::int32_t session_host_info_classify_0076c740(std::uintptr_t& info_14, HostInfoClassifyHost& host) {
    info_14 = host.peer_record_d50();
    if (host.slot_c(2) || host.slot_c(3)) return 2;
    return 0;
}

void session_reset_online_flag_0076d220(OnlineFlagHost& host) {
    host.call_0091c650();
    host.clear_live_74();
}

bool wide_string_matches_at_0076e7f0(const std::u16string& s, bool data_present, const std::u16string& needle,
                                     std::uint32_t pos) noexcept {
    if (!data_present || pos > s.size()) return false;
    std::size_t i = pos;
    std::size_t j = 0;
    // Both sides stop at their terminator, as the image's NUL-terminated walk.
    while (i < s.size() && s[i] != 0 && j < needle.size() && needle[j] != 0 && s[i] == needle[j]) {
        ++i;
        ++j;
    }
    return j >= needle.size() || needle[j] == 0;
}

}  // namespace bsp::cc13cam2
