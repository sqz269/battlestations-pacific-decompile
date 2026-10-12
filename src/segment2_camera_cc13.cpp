// Segment 2 camera movers. See include/bsp/segment2_camera_cc13.hpp and
// docs/CC13_SEGMENT2_CAMERA_MATH.md.
#include "bsp/segment2_camera_cc13.hpp"

#include "bsp/camera_decomposition.hpp"
#include "bsp/unit_rudder.hpp"

#include <array>
#include <cmath>

namespace bsp::seg2_cc13 {

namespace {

// Every intermediate the listings spill with FSTP float is rounded here;
// arithmetic between spills runs in double (the MSVC x87 53-bit setting).
float f32(double v) noexcept { return static_cast<float>(v); }

void retarget_observer(WeaponViewCamera& camera, void* target, WeaponViewHost& host) {
    if (camera.target_398 == target) return;
    if (camera.target_398 != nullptr) host.observer_unregister_006952a0(camera.target_398);
    camera.target_398 = target;
    if (target != nullptr) host.observer_register_00694a60(target);
}

}  // namespace

void construct_torpedo_view_00435120(WeaponViewCamera& camera) noexcept {
    camera.target_398 = nullptr;  // observer +14h; +388h..+390h = 0, +394h = 1
}

void construct_bomb_view_00435280(WeaponViewCamera& camera) noexcept {
    camera.target_398 = nullptr;
    camera.yaw_3f0 = 0.0f;    // 00435300 (XORPS / MOVSS)
    camera.pitch_3f4 = 0.0f;
}

void set_torpedo_view_target_0042fee0(WeaponViewCamera& camera, void* target,
                                      WeaponViewHost& host) {
    retarget_observer(camera, target, host);
    camera.pose = host.camera_node_world();      // 0042FF36
    camera.v_3a8 = camera.pose.position_a4;      // start position
    camera.v_3b4 = camera.pose.forward_94;       // start forward
    camera.v_3c0 = camera.pose.up_84;            // start up
    camera.v_39c = camera.target_398 != nullptr ? host.entity_world_position(camera.target_398)
                                                : camera.pose.position_a4;
}

void set_bomb_view_target_004307f0(WeaponViewCamera& camera, void* target, void* launcher,
                                   WeaponViewHost& host) {
    retarget_observer(camera, target, host);
    camera.f_3d8 = 1.0f;  // [00D7A24C]
    if (camera.target_398 != nullptr) {
        camera.pose = host.camera_node_world();
        camera.v_3cc = host.launcher_vector_34(launcher);  // no null check on the launcher
        camera.offset_3dc = host.global_vector_00f87574();
        camera.v_39c = camera.pose.forward_94;
        camera.v_3a8 = camera.pose.up_84;
        camera.v_3c0 = camera.pose.position_a4;
        camera.f_3e8 = 0.1f;   // [00D7A2F0]
        camera.f_3ec = 0.22f;  // [00CE3D38]
    }
    camera.yaw_3f0 = 0.0f;
    camera.pitch_3f4 = 0.0f;
}

void orbit_about_axis_0042d610(Mat4Rows& m, const Vec3& point, const Vec3& axis, float angle,
                               WeaponViewHost& host) {
    const float length = host.vector_length_00419440(axis);  // spilled float
    const float negated = -length;                          // FCHS, spilled
    const Vec3 unit{f32(axis.x / static_cast<double>(negated)),
                    f32(axis.y / static_cast<double>(negated)),
                    f32(axis.z / static_cast<double>(negated))};
    host.rotate_about_axis_0085e8c0(m, point, unit, f32(static_cast<double>(length) * angle));
}

void bomb_view_step_00430da0(WeaponViewCamera& camera, float dt, WeaponViewHost& host) {
    const float blend = f32(static_cast<double>(dt) * host.tuning_camera_pos_smooth_e8());
    bool torpedo_running = false;
    if (camera.target_398 != nullptr) {
        // 00430DDA..00430E01: an MTorpedo whose [+314h]+F4h is above 0.0.
        torpedo_running = host.is_torpedo(camera.target_398) &&
                          host.torpedo_value_314_f4(camera.target_398) > 0.0f;
        camera.v_3b4 = host.entity_world_position(camera.target_398);
        // 00430E45..00430E6C: keep the target point at least 3.0 high unless
        // a running torpedo is already below -15.0.
        if (camera.v_3b4.y < 3.0f &&
            (static_cast<double>(camera.v_3b4.y) > -15.0 || !torpedo_running)) {
            camera.v_3b4.y = 3.0f;
        }
    }
    const Vec3 wanted{f32(camera.v_3b4.x + static_cast<double>(camera.offset_3dc.x)),
                      f32(camera.v_3b4.y + static_cast<double>(camera.offset_3dc.y)),
                      f32(camera.v_3b4.z + static_cast<double>(camera.offset_3dc.z))};
    // The camera drifts with the launcher's vector.
    camera.v_3c0.x = f32(f32(static_cast<double>(dt) * camera.v_3cc.x) + static_cast<double>(camera.v_3c0.x));
    camera.v_3c0.y = f32(f32(static_cast<double>(camera.v_3cc.y) * dt) + static_cast<double>(camera.v_3c0.y));
    camera.v_3c0.z = f32(static_cast<double>(camera.v_3c0.z) + f32(static_cast<double>(dt) * camera.v_3cc.z));
    if (torpedo_running && wanted.y < 0.0f && camera.v_3c0.y > -3.0f) camera.v_3c0.y = -3.0f;

    const float dx = f32(static_cast<double>(wanted.x) - camera.v_3c0.x);
    const float dy = f32(static_cast<double>(wanted.y) - camera.v_3c0.y);
    const float dz = f32(static_cast<double>(wanted.z) - camera.v_3c0.z);
    const float square = f32((static_cast<double>(dy) * dy + static_cast<double>(dx) * dx) +
                             static_cast<double>(dz) * dz);
    // A zero distance divides by 0.0 below, as in the image.
    const float distance = static_cast<double>(square) > 1e-10
                               ? f32(std::sqrt(static_cast<double>(square)))
                               : 0.0f;
    const float keep = f32(1.0 - blend);
    auto mix = [&](float d, float old) {
        const float dir = f32(static_cast<double>(d) / distance);
        return f32(f32(static_cast<double>(dir) * blend) + f32(static_cast<double>(old) * keep));
    };
    camera.v_39c = Vec3{mix(dx, camera.v_39c.x), mix(dy, camera.v_39c.y), mix(dz, camera.v_39c.z)};

    // Up eases towards (0, 1, 0) at a third of the forward rate (3.0 is the
    // double at 00D7A2B0, 0.0 the double at 00D7A258).
    const float g = f32(static_cast<double>(blend) / 3.0);
    const float keep_up = f32(1.0 - g);
    const float zero_term = f32(0.0 * g);
    camera.v_3a8 = Vec3{f32(static_cast<double>(zero_term) + f32(static_cast<double>(camera.v_3a8.x) * keep_up)),
                        f32(static_cast<double>(g) + f32(static_cast<double>(camera.v_3a8.y) * keep_up)),
                        f32(static_cast<double>(zero_term) + f32(static_cast<double>(keep_up) * camera.v_3a8.z))};

    camera.pose.forward_94 = camera.v_39c;
    camera.pose.up_84 = camera.v_3a8;
    host.orthonormalize_0085dc80(camera.pose);
    camera.pose.position_a4 = camera.v_3c0;
    // Orbit by the yaw about world up, then by the pitch about row 0, both
    // through the wanted point.
    orbit_about_axis_0042d610(camera.pose, wanted, Vec3{0.0f, 1.0f, 0.0f}, camera.yaw_3f0, host);
    orbit_about_axis_0042d610(camera.pose, wanted, camera.pose.right_74, camera.pitch_3f4, host);
    camera.pose_valid_c8 = 0;
    camera.byte_10c = 0;
    host.invalidate_children_and_publish();
}

void torpedo_view_step_00430020(WeaponViewCamera& camera, float dt, WeaponViewHost& host) {
    // Aim point (frame +24h..+2Ch before the loop's two pushes).
    Vec3 aim;
    std::array<float, 3> flat{};
    if (camera.target_398 != nullptr) {
        camera.v_39c = host.entity_world_position(camera.target_398);   // 00430075
        host.entity_forward_xz(camera.target_398, flat[0], flat[2]);
        flat[1] = 0.0f;
        normalize_camera_basis_0042b260(flat);
        const double k = kTorpedoTrailDistance;
        aim.x = f32(camera.v_39c.x - static_cast<double>(f32(flat[0] * k)));
        aim.y = f32(camera.v_39c.y - static_cast<double>(f32(k * flat[1])));
        aim.z = f32(camera.v_39c.z - static_cast<double>(f32(k * flat[2])));
        // 0043012B: below 0 the aim height is 3.0, else 3.0 above it.
        aim.y = 0.0f > aim.y ? kTorpedoHeight : f32(static_cast<double>(aim.y) + kTorpedoHeight);
    } else {
        flat = {camera.pose.forward_94.x, 0.0f, camera.pose.forward_94.z};
        normalize_camera_basis_0042b260(flat);
        // DC C9 at 0043019A is FMUL ST1,ST0: both products use 100.0 (00D7A220).
        aim.x = f32(camera.v_39c.x - static_cast<double>(f32(flat[0] * 100.0)));
        aim.z = f32(camera.v_39c.z - static_cast<double>(f32(100.0 * flat[2])));
        aim.y = 30.0f;   // 00CE38C8
    }
    while (dt > 0.0f) {   // 004301D1 COMISS against 0.0f (00D7A218), then 00430784
        const float step = dt <= kTorpedoStepMax ? dt : kTorpedoStepMax;
        const float t = f32(static_cast<double>(kTorpedoBlendXZ) * step / kTorpedoStepMax);
        const Vec3 eye = camera.pose.position_a4;
        float part_top = 0.0f, entity_y = 0.0f;
        if (host.segment_obstacle(Vec3{eye.x, kTorpedoHeight, eye.z}, camera.v_39c, part_top,
                                  entity_y)) {
            const float top = f32(static_cast<double>(part_top) + entity_y + 4.0);   // 00D7A328
            if (top > aim.y) aim.y = top;   // persists into later sub-steps
        }
        const double ky = kTorpedoBlendY;
        camera.v_3a8.x = f32(camera.v_3a8.x + (static_cast<double>(aim.x) - camera.v_3a8.x) * t);
        camera.v_3a8.y = f32(camera.v_3a8.y + (static_cast<double>(aim.y) - camera.v_3a8.y) * ky);
        camera.v_3a8.z = f32((static_cast<double>(aim.z) - camera.v_3a8.z) * t + camera.v_3a8.z);
        const Vec3 position{f32((static_cast<double>(camera.v_3a8.x) - eye.x) * t + eye.x),
                            f32(ky * (static_cast<double>(camera.v_3a8.y) - eye.y) + eye.y),
                            f32(t * (static_cast<double>(camera.v_3a8.z) - eye.z) + eye.z)};
        // 00430476: the look-at height is 3.0 above a target at or above 0, else 3.0.
        const float look_y = 0.0f <= camera.v_39c.y
                                 ? f32(static_cast<double>(camera.v_39c.y) + kTorpedoHeight)
                                 : kTorpedoHeight;
        std::array<float, 3> dir{f32(static_cast<double>(camera.v_39c.x) - position.x),
                                 f32(static_cast<double>(look_y) - position.y),
                                 f32(static_cast<double>(camera.v_39c.z) - position.z)};
        normalize_camera_basis_0042b260(dir);   // 004304C5
        auto ease = [t](float& value, float goal) {   // three float spills per axis
            const float delta = f32(static_cast<double>(goal) - value);
            const float scaled = f32(static_cast<double>(delta) * t);
            value = f32(static_cast<double>(value) + scaled);
        };
        ease(camera.v_3b4.x, dir[0]);
        ease(camera.v_3b4.y, dir[1]);
        ease(camera.v_3b4.z, dir[2]);
        Vec3 forward = camera.pose.forward_94;
        ease(forward.x, camera.v_3b4.x);
        ease(forward.y, camera.v_3b4.y);
        ease(forward.z, camera.v_3b4.z);
        ease(camera.v_3c0.x, 0.0f);
        ease(camera.v_3c0.y, 1.0f);
        ease(camera.v_3c0.z, 0.0f);
        Vec3 up = camera.pose.up_84;
        ease(up.x, camera.v_3c0.x);
        ease(up.y, camera.v_3c0.y);
        ease(up.z, camera.v_3c0.z);
        camera.pose.position_a4 = position;
        camera.pose.forward_94 = forward;
        camera.pose.up_84 = up;
        host.orthonormalize_0085dc80(camera.pose);   // 00430741
        dt = f32(static_cast<double>(dt) - kTorpedoStepMax);
    }
    camera.pose_valid_c8 = 0;
    camera.byte_10c = 0;
    host.invalidate_children_and_publish();
}

void bomb_view_update_velocity_00430970(WeaponViewCamera& camera, WeaponViewHost& host) {
    float ground = host.ground_height_at(camera.v_3b4);
    if (8.0f > ground) ground = 8.0f;   // 00CE3918
    const float height = f32(static_cast<double>(camera.v_3b4.y) - ground);
    float follow;
    void* const target = camera.target_398;
    if (target == nullptr) {
        follow = host.tuning_float(0x138);   // FinalDist
    } else {
        std::uint32_t far_off = 0x124, near_alt_off = 0x11C, near_off = 0x120;   // bomb
        if (host.entity_is_kind(target, 0x2B)) {
            far_off = 0x104; near_alt_off = 0xFC; near_off = 0x100;   // torpedo
        } else if (host.entity_is_kind(target, 0x33)) {
            far_off = 0x114; near_alt_off = 0x10C; near_off = 0x110;   // rocket
        } else {
            (void)host.entity_is_kind(target, 0x2A);   // asked, result unused
        }
        // The three 0042E740 calls fetch near, near-alt and far in that order.
        const float near_dist = host.tuning_float(near_off);
        const float near_alt = host.tuning_float(near_alt_off);
        const float far_dist = host.tuning_float(far_off);
        follow = clamped_interpolate_00419010(0.0f, far_dist, near_alt, near_dist, height);
    }
    const double lead = static_cast<double>(1.6f);   // 00CE3D48
    Vec3 aim = camera.v_3b4;
    if (target != nullptr) {
        const Vec3 v = host.entity_velocity_34(target);
        const float lx = f32(v.x * lead), ly = f32(v.y * lead), lz = f32(v.z * lead);
        aim = Vec3{f32(static_cast<double>(lx) + aim.x), f32(static_cast<double>(ly) + aim.y),
                   f32(static_cast<double>(lz) + aim.z)};
    }
    const float dx = f32(static_cast<double>(aim.x) - camera.v_3c0.x);
    const float dy = f32(static_cast<double>(aim.y) - camera.v_3c0.y);
    const float dz = f32(static_cast<double>(aim.z) - camera.v_3c0.z);
    const float square = f32((static_cast<double>(dy) * dy + static_cast<double>(dx) * dx) +
                             static_cast<double>(dz) * dz);
    // At most 1e-10 (00CE3820) the length is 0 and the divisions below are unguarded.
    const float length = static_cast<double>(square) <= 1e-10 ? 0.0f : f32(std::sqrt(static_cast<double>(square)));
    const float ux = f32(static_cast<double>(dx) / length);
    const float uy = f32(static_cast<double>(dy) / length);
    const float uz = f32(static_cast<double>(dz) / length);
    const float speed = f32((static_cast<double>(length) - follow) / lead);
    float vx = f32(static_cast<double>(ux) * speed);
    float vy = f32(static_cast<double>(uy) * speed);
    float vz = f32(static_cast<double>(speed) * uz);
    if (target == nullptr) {
        const double damp = static_cast<double>(0.8f);   // 00CE3D40
        vx = f32(vx * damp);
        vy = f32(vy * damp);
        vz = f32(damp * vz);
        const float horizontal_sq = f32(static_cast<double>(vz) * vz + static_cast<double>(vx) * vx);
        const double half = static_cast<double>(f32(std::sqrt(static_cast<double>(horizontal_sq)))) * 0.5;   // 00D7A280
        if (half > vy) vy = f32(half);   // climb at least half the horizontal speed
    }
    const float b = camera.f_3d8;
    const float keep = f32(1.0 - b);
    const float nx = f32(static_cast<double>(keep) * vx), ny = f32(static_cast<double>(keep) * vy),
                nz = f32(static_cast<double>(keep) * vz);
    const float ox = f32(static_cast<double>(camera.v_3cc.x) * b),
                oy = f32(static_cast<double>(camera.v_3cc.y) * b),
                oz = f32(static_cast<double>(b) * camera.v_3cc.z);
    camera.v_3cc = Vec3{f32(static_cast<double>(ox) + nx), f32(static_cast<double>(oy) + ny),
                        f32(static_cast<double>(oz) + nz)};
    if (camera.f_3d8 > host.tuning_float(0xF4))   // MaxCamVelBlender
        camera.f_3d8 = f32(static_cast<double>(camera.f_3d8) - host.tuning_float(0xF0));
}

void airfield_camera_step_00433590(Mat4Rows& pose, bool active_380, const Mat4Rows& matrix_2b8,
                                   float dt, MoverHost& host) {
    if (!active_380) return;
    pose = matrix_2b8;
    host.apply_shake_and_publish_004329d0(dt);
}

void bomber_view_step_004335c0(BomberViewCamera& camera, BomberViewOverride& override_state,
                               float dt, MoverHost& host) {
    if (override_state.flag_e17670 != 0.0f) {
        camera.yaw_384 = override_state.yaw_e17678;
        camera.pitch_388 = override_state.pitch_e17674;
        override_state.flag_e17670 = 0.0f;
    }
    host.update_focus_0042dcd0(dt, 1);
    if (camera.target_3ac != nullptr) {
        Vec3 target;
        float fx = 0.0f, fz = 0.0f;
        host.entity_world_pose(camera.target_3ac, target, fx, fz);
        const float eye_y = f32(target.y + 10.0);   // 00CE3DC0, FSTP float
        camera.pose.forward_94 = Vec3{fx, 0.0f, fz};
        camera.pose.up_84 = Vec3{0.0f, 1.0f, 0.0f};
        host.orthonormalize_0085dc80(camera.pose);
        host.premultiply_rotation_y(camera.pose, -0.0f - camera.yaw_384);   // SUBSS from -0.0
        host.premultiply_rotation_x(camera.pose, -0.0f - camera.pitch_388);
        const float d = camera.distance_38c;
        const Vec3& f = camera.pose.forward_94;
        const float ox = f32(static_cast<double>(d) * f.x), oy = f32(static_cast<double>(f.y) * d),
                    oz = f32(static_cast<double>(d) * f.z);
        camera.pose.position_a4 = Vec3{f32(static_cast<double>(target.x) - ox),
                                       f32(static_cast<double>(eye_y) - oy),
                                       f32(static_cast<double>(target.z) - oz)};
        host.apply_zoom_0042c610();
    }
    host.apply_shake_and_publish_004329d0(dt);
}

void contact_point_list_append_0042b570(ContactPointList& list, const Vec3& point,
                                        const Vec3& normal) noexcept {
    if (list.count_c0 >= 8) return;
    list.point_normal[list.count_c0][0] = point;
    list.point_normal[list.count_c0][1] = normal;
    ++list.count_c0;
}

}  // namespace bsp::seg2_cc13
