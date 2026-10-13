// Packet cc13-cam2, docs/CC13_CAM_2.md. Addresses, ABI and evidence are on the
// declarations in include/bsp/engine_seg43_45_cc13.hpp; listing addresses in the
// bodies mark where a step was read.
#include "bsp/engine_seg43_45_cc13.hpp"

#include "bsp/band_0076_cc13.hpp"
#include "bsp/camera_decomposition.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/native_bit_cursor_write.hpp"
#include "bsp/ship_ai_bearing_rating.hpp"

#include <cmath>
#include <cstring>

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
    m.velocity_3ac = {0.0f, 0.0f, 0.0f};
    m.position_3a0 = {0.0f, 0.0f, 0.0f};
    t.rate_3d4 = {0.0f, 0.0f};
    t.angles_3cc = {0.0f, 0.0f};
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
    auto& v = m.velocity_3ac;
    if (limits.accel_4c < length3_yxz(v[0], v[1], v[2])) {  // 0079443A
        v = cc13cam::scale_to_length_00793730(v, limits.accel_4c);
    }
    for (int i = 0; i < 3; ++i) {
        const float step = static_cast<float>(static_cast<double>(v[i]) * dt);
        m.position_3a0[i] = static_cast<float>(static_cast<double>(m.position_3a0[i]) + step);
    }
    Vec3 d{};
    for (int i = 0; i < 3; ++i) {
        d[i] = static_cast<float>(static_cast<double>(m.position_3a0[i]) - m.previous_3b8[i]);
    }
    if (limits.top_speed_34 < length3_yxz(d[0], d[1], d[2])) {  // 00794537
        const Vec3 s = cc13cam::scale_to_length_00793730(d, limits.top_speed_34);
        for (int i = 0; i < 3; ++i) {
            m.position_3a0[i] = static_cast<float>(static_cast<double>(s[i]) + m.previous_3b8[i]);
        }
    }
    auto& r = t.rate_3d4;
    if (turn.accel_50 < length2_yx(r[0], r[1])) {  // 00794618
        r = cc13cam::scale_to_length_00793800(r, turn.accel_50);
    }
    for (int i = 0; i < 2; ++i) {
        const float step = static_cast<float>(static_cast<double>(r[i]) * dt);
        t.angles_3cc[i] = static_cast<float>(static_cast<double>(t.angles_3cc[i]) + step);
    }
    std::array<float, 2> a{};
    for (int i = 0; i < 2; ++i) {
        a[i] = static_cast<float>(static_cast<double>(t.angles_3cc[i]) - t.previous_3dc[i]);
    }
    if (turn.top_rate_38 < length2_yx(a[0], a[1])) {  // 007946DA
        const auto s = cc13cam::scale_to_length_00793800(a, turn.top_rate_38);
        for (int i = 0; i < 2; ++i) {
            t.angles_3cc[i] = static_cast<float>(static_cast<double>(s[i]) + t.previous_3dc[i]);
        }
    }
}

void movcam_steer_angles_braking_00794f70(cc13cam::MovCamTurn& t, const cc13cam::MovCamTurnLimits& limits,
                                          const std::array<float, 2>& d, std::size_t target,
                                          float length_00414c60_of_d) noexcept {
    std::array<float, 2> delta{};
    for (int i = 0; i < 2; ++i) {
        delta[i] = static_cast<float>(static_cast<double>(t.angles_3cc[i]) - t.previous_3dc[i]);
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
        t.rate_3d4[i] = static_cast<float>(static_cast<double>(add[i]) + t.rate_3d4[i]);
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
    const float current = state[0];
    const float target = state[1];
    const float d = static_cast<float>(static_cast<double>(target) - current);  // 0078FB0E
    const float x = d > 0.0f ? d : -0.0f - d;
    const float step = static_cast<float>(static_cast<double>(x) * dt * 1.399999976158142);  // 0078FB48
    float next;
    if (target < current) {  // 0078FB50 JB
        next = static_cast<float>(static_cast<double>(current) - step);
        if (target > next) next = target;  // 0078FB9E
    } else {
        next = static_cast<float>(static_cast<double>(current) + step);
        if (next > target) next = target;  // 0078FB6B
    }
    state[0] = next;
    state[2] = static_cast<float>(static_cast<double>(dt) * next + state[2]);
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

}  // namespace bsp::cc13cam2
