// Segment 2 camera movers. See include/bsp/segment2_camera_cc13.hpp and
// docs/CC13_SEGMENT2_CAMERA_MATH.md.
#include "bsp/segment2_camera_cc13.hpp"

#include "bsp/camera_decomposition.hpp"
#include "bsp/dive_bomb_task.hpp"
#include "bsp/gamepad_force_events.hpp"
#include "bsp/native_scalar_float_by_ref.hpp"
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

// ---- cc13-camera2 -------------------------------------------------------

void random_wander_evaluate_0042c090(RandomWander& w) noexcept {
    float t = f32(static_cast<double>(w.elapsed_24) / w.period_28);
    if (!(1.0 > t)) t = 1.0f;   // FCOMI 1.0, t; JBE -> [00D7A24C]
    const float dx = f32(static_cast<double>(w.start_00.x) - w.target_0c.x);
    const float dy = f32(static_cast<double>(w.start_00.y) - w.target_0c.y);
    const float dz = f32(static_cast<double>(w.start_00.z) - w.target_0c.z);
    const float s = f32(static_cast<double>(t) * t);
    const float keep = f32(1.0 - s);
    const float mx = f32(static_cast<double>(dx) * keep), my = f32(static_cast<double>(dy) * keep),
                mz = f32(static_cast<double>(keep) * dz);
    w.output_18 = Vec3{f32(static_cast<double>(w.target_0c.x) + mx),
                       f32(static_cast<double>(w.target_0c.y) + my),
                       f32(static_cast<double>(w.target_0c.z) + mz)};
    float u = f32(static_cast<double>(w.elapsed_24) / w.life_30);
    if (!(1.0 > u)) u = 1.0f;
    const float u2 = f32(static_cast<double>(u) * u);
    const float envelope = f32(1.0 - u2);
    w.output_18.x = f32(static_cast<double>(w.output_18.x) * envelope);
    w.output_18.y = f32(static_cast<double>(envelope) * w.output_18.y);
    w.output_18.z = f32(static_cast<double>(envelope) * w.output_18.z);
}

void random_wander_retarget_0042c1b0(RandomWander& w, WanderRandom& rng) {
    // 0042C1D2/0042C227 FUCOMIP: a NaN bound counts as "not equal" and draws.
    if (w.period_lo_40 == w.period_hi_44) w.period_28 = w.period_lo_40;
    else w.period_28 = rng.uniform(w.period_lo_40, w.period_hi_44);           // 0042C1F9
    float radius = w.radius_lo_34;
    if (w.radius_lo_34 != w.radius_hi_38) radius = rng.uniform(w.radius_lo_34, w.radius_hi_38);   // 0042C23D
    w.elapsed_24 = 0.0f;
    const float step = rng.uniform(0.5f, 5.78318548f);                         // 0042C26C
    const float azimuth = wrapped_angle_add_00438aa0(w.azimuth_3c, step);
    w.azimuth_3c = azimuth;
    const float elevation = rng.uniform(0.0f, 3.14159274f);                    // 0042C2A6
    const float cos_el = f32(std::cos(static_cast<double>(elevation)));
    const float sin_el = f32(std::sin(static_cast<double>(elevation)));
    const float cos_az = f32(std::cos(static_cast<double>(azimuth)));
    const float sin_az = f32(std::sin(static_cast<double>(azimuth)));
    w.target_0c = Vec3{f32(static_cast<double>(sin_az) * sin_el * radius),
                       f32(static_cast<double>(cos_az) * sin_el * radius),
                       f32(static_cast<double>(radius) * cos_el)};
    w.start_00 = w.output_18;
}

void random_wander_advance_0042daf0(RandomWander& w, float dt, WanderRandom& rng) {
    if (w.elapsed_24 > w.life_30) return;   // JA: no evaluate either
    w.elapsed_24 = f32(static_cast<double>(w.elapsed_24) + dt);
    if (!(w.elapsed_24 >= w.period_28)) {   // JC: unordered too
        random_wander_evaluate_0042c090(w);
        return;
    }
    for (;;) {
        const float period = w.period_28;
        if (!(w.budget_2c >= period)) break;
        const float rest = f32(static_cast<double>(w.elapsed_24) - period);
        w.elapsed_24 = period;   // evaluate at the end of the finished period
        w.budget_2c = f32(static_cast<double>(w.budget_2c) - period);
        random_wander_evaluate_0042c090(w);
        random_wander_retarget_0042c1b0(w, rng);
        w.elapsed_24 = rest;
        if (!(w.elapsed_24 >= w.period_28)) break;   // JNC loops
    }
    random_wander_evaluate_0042c090(w);
}

namespace {

void ease_angle(EasedAngle& a, float dt) noexcept {
    const double k = static_cast<double>(dt) * a.rate / a.period;
    a.current = f32(a.current + (static_cast<double>(a.target) - a.current) * k);
}

bool angle_significant(float a) noexcept {
    const float magnitude = a > 0.0f ? a : -0.0f - a;   // COMISS / SUBSS from -0.0
    return static_cast<double>(magnitude) > static_cast<double>(1.0e-4f);   // 00D7A268 double
}

}  // namespace

void plane_cockpit_camera_step_00434810(PlaneCockpitCamera& camera, float dt,
                                        PlaneCockpitHost& host, WanderRandom& rng) {
    if (camera.plane_3c0 != nullptr && host.plane_released_44(camera.plane_3c0))
        host.release_reference_0042d9a0(camera.plane_3c0);
    if (camera.plane_3c0 != nullptr) {
        const float max_head_move = host.tuning_float(0xAC);   // CockpitMaxHeadMoveDist
        for (RandomWander& w : camera.wander) random_wander_advance_0042daf0(w, dt, rng);
        Vec3& h = camera.head_move_3dc;
        h = camera.wander[0].output_18;
        h.x = f32(static_cast<double>(camera.wander[1].output_18.x) + h.x);
        h.y = f32(static_cast<double>(camera.wander[1].output_18.y) + h.y);
        h.z = f32(static_cast<double>(camera.wander[1].output_18.z) + h.z);
        h.x = f32(static_cast<double>(camera.wander[2].output_18.x) + h.x);
        h.y = f32(static_cast<double>(camera.wander[2].output_18.y) + h.y);
        h.z = f32(static_cast<double>(camera.wander[2].output_18.z) + h.z);
        const float length = force_event_vector_length_0042b2f0({h.x, h.y, h.z});
        const float ratio = f32(static_cast<double>(length) / max_head_move);
        const float curve = dive_bomb_interpolate_clamped_00419010(0.7f, 0.7f, 2.0f, 1.2f, ratio);
        const float scale = f32(static_cast<double>(ratio) / curve);   // 0043495F FDIVR double
        h.x = f32(static_cast<double>(scale) * h.x);
        h.y = f32(static_cast<double>(scale) * h.y);
        h.z = f32(static_cast<double>(scale) * h.z);
        if (dt != 0.0f) {   // UCOMISS / TEST AH,44h: NaN also updates
            ease_angle(camera.yaw_384, dt);
            ease_angle(camera.pitch_398, dt);
            ease_angle(camera.roll_3ac, dt);
        }
        const Mat4Rows plane = host.entity_world_matrix(camera.plane_3c0);
        Mat4Rows rotation_only = plane;
        rotation_only.position_a4 = host.global_vector_00f87574();
        camera.pose = plane;
        const Vec3 eye_local{f32(static_cast<double>(camera.eye_offset_3c4.x) + h.x),
                             f32(static_cast<double>(h.y) + camera.eye_offset_3c4.y),
                             f32(static_cast<double>(h.z) + camera.eye_offset_3c4.z)};
        const Vec3 eye_world = host.transform_direction_0042d0d0(eye_local, rotation_only);
        Vec3& p = camera.pose.position_a4;
        p.x = f32(static_cast<double>(p.x) + eye_world.x);
        p.y = f32(static_cast<double>(p.y) + eye_world.y);
        p.z = f32(static_cast<double>(eye_world.z) + p.z);
        const Vec3 eye = p;
        p = host.global_vector_00f87574();   // rotate about the eye, then restore it
        if (angle_significant(camera.yaw_384.current))
            host.premultiply_rotation_y(camera.pose, camera.yaw_384.current);
        if (angle_significant(camera.pitch_398.current))
            host.premultiply_rotation_x(camera.pose, camera.pitch_398.current);
        if (angle_significant(camera.roll_3ac.current))
            host.premultiply_rotation_z(camera.pose, -0.0f - camera.roll_3ac.current);
        if (camera.modifier_514 != nullptr && host.modifier_active_8c(camera.modifier_514))
            host.premultiply_modifier_94(camera.modifier_514, camera.pose);
        camera.pose.position_a4 = eye;
        camera.look_frame_3e8 = plane;
        const Vec3 look_local{f32(static_cast<double>(camera.look_offset_3d0.x) + h.x),
                              f32(static_cast<double>(camera.look_offset_3d0.y) + h.y),
                              f32(static_cast<double>(camera.look_offset_3d0.z) + h.z)};
        const Vec3 look = host.transform_direction_0042d0d0(look_local, plane);
        Vec3& q = camera.look_frame_3e8.position_a4;
        q.x = f32(static_cast<double>(q.x) + look.x);
        q.y = f32(static_cast<double>(look.y) + q.y);
        q.z = f32(static_cast<double>(look.z) + q.z);
    }
    camera.published_2b8 = camera.pose;
    host.apply_shake_and_publish_004329d0(dt);
}

void rotate_yaw_then_pitch_0042b6e0(Mat4Rows& m, float pitch, float yaw, MoverHost& host) {
    host.premultiply_rotation_y(m, -0.0f - yaw);     // 0042B6EE SUBSS from -0.0
    host.premultiply_rotation_x(m, -0.0f - pitch);   // 0042B739
}

namespace {

constexpr double kPi = 3.1415927410125732;      // 00CE3D28 (00CE3D18 is -kPi)
constexpr double kTwoPi = 6.2831854820251465;   // 00CE3828

// The x87 ease used throughout 0042F800: current + (want - current) * k,
// float-stored, k unspilled.
float ease(float current, double want, double k) noexcept {
    return f32(current + (want - current) * k);
}

// Wrap a float-stored angle once into [-pi, pi] (0042FD5D / 0042FE78).
float wrap_once(float r) noexcept {
    if (-kPi > r) return f32(r + kTwoPi);
    if (r > kPi) return f32(r - kTwoPi);
    return r;
}

}  // namespace

void chase_camera_update_offsets_0042f800(DefaultUnitCamera& c, float dt, ChaseCameraHost& host) {
    void* t = c.target_388;   // dereferenced before the null test (0042F818)
    const float a = -0.0f - host.entity_float(t, 0xB54);    // pitch rate, negated
    const float b = -0.0f - host.entity_float(t, 0xB50);    // roll rate, negated
    const float yaw_rate = host.entity_float(t, 0xB58);
    const float look_rate = host.tuning_float(0x80);        // PlaneCamera/LookAroundSmoothRate
    if (t != nullptr) {
        const std::uint32_t state = host.entity_u32(t, 0x900);
        const float height = host.entity_float(t, 0x9B4);
        if ((state == 4 || state == 5) && 3.0f > height) {   // 00CE3854
            const float bias = f32(static_cast<double>(dive_bomb_interpolate_clamped_00419010(
                                       1.0f, 0.6f, 4.0f, 0.0f, height)) *
                                   host.class_float(t, 0x200));
            c.timer_3ec = max_native_float_by_ref_00415550(&c.timer_3ec, &bias);
        }
    }
    const float g = c.pitch_input_3e8;
    const float abs_g = g > 0.0f ? g : -0.0f - g;
    const float h = c.timer_3ec;
    const float m = (h > 0.0f && 1.0f > abs_g) ? f32((1.0 - abs_g) * h + g) : g;

    const double k58 = static_cast<double>(host.tuning_float(0x58)) * dt;   // ZRotSmoothRate
    const double k60 = static_cast<double>(host.tuning_float(0x60)) * dt;   // XDistSmoothRate
    const double k68 = static_cast<double>(host.tuning_float(0x68)) * dt;   // YDistSmoothRate
    const double k78 = static_cast<double>(host.tuning_float(0x78)) * dt;   // ZDistSmoothRate
    c.z_rot_want_39c = ease(c.z_rot_want_39c, static_cast<double>(host.tuning_float(0x54)) * b, k58);
    c.x_dist_want_3a0 = ease(c.x_dist_want_3a0,
                             static_cast<double>(host.tuning_float(0x5C)) * yaw_rate -
                                 static_cast<double>(b) * host.tuning_float(0x6C),
                             k60);
    c.y_dist_want_3a4 = ease(c.y_dist_want_3a4,
                             -static_cast<double>(host.tuning_float(0x64)) * a + c.sub_dist_3c0, k68);
    c.z_dist_want_3a8 = ease(c.z_dist_want_3a8,
                             static_cast<double>(host.entity_float(t, 0xB28)) * host.tuning_float(0x74),
                             k78);
    c.z_rot_3b0 = ease(c.z_rot_3b0, c.z_rot_want_39c, k58);
    c.x_dist_3b4 = ease(c.x_dist_3b4, c.x_dist_want_3a0, k60);
    c.y_dist_3b8 = ease(c.y_dist_3b8, c.y_dist_want_3a4, k68);
    c.z_dist_3bc = ease(c.z_dist_3bc, c.z_dist_want_3a8, k78);

    const double k3 = static_cast<double>(dt) * 3.0;   // 00D7A2B0
    if (c.flag_3f0 == 0) {   // 0042FC5E: free look
        c.sub_dist_3c0 = 0.0f;
        c.aim_yaw_want_3d4 = ease(c.aim_yaw_want_3d4, 0.0, k3);
        c.aim_pitch_want_3d8 = ease(c.aim_pitch_want_3d8, 0.0, k3);
        const float want_yaw = -c.yaw_input_3e4;   // FCHS, float spill
        const float c4 = c.look_yaw_want_3c4;
        float unwrapped = want_yaw;
        if (static_cast<double>(want_yaw) - c4 > kPi) unwrapped = f32(want_yaw - kTwoPi);
        else if (static_cast<double>(c4) - want_yaw > kPi) unwrapped = f32(want_yaw + kTwoPi);
        const double kl = static_cast<double>(dt) * look_rate;
        const float eased = ease(c4, unwrapped, kl);
        c.look_pitch_want_3c8 = ease(c.look_pitch_want_3c8, -static_cast<double>(m), kl);
        c.look_yaw_want_3c4 = wrap_once(f32(std::fmod(static_cast<double>(eased), kTwoPi)));   // _CIfmod
    } else {
        bool fixed_pitch = false;
        const std::uint32_t mode = host.entity_u32(t, 0x970);
        if (mode != 0x12) {
            const double degrees = static_cast<double>(g) / kPi * 180.0;   // 00CE3D20
            const float limit = host.tuning_float(0x13C);                  // BombSubDistLimit
            if (!(limit < degrees)) {
                if (mode == 10) {
                    const bool second = host.entity_is_kind(t, 0x16);
                    c.sub_dist_3c0 = host.tuning_float(second ? 0x14C : 0x148);   // TorpedoSubDist(2)
                    const float angle = host.tuning_float(second ? 0x154 : 0x150); // TorpedoSubAngle(2)
                    c.aim_pitch_want_3d8 = ease(c.aim_pitch_want_3d8, angle, k3);
                    fixed_pitch = true;
                } else {
                    const float sub = f32((limit - degrees) * host.tuning_float(0x144));   // BombSubDistMul
                    const float floor = host.tuning_float(0x140);                          // BombSubDistMin
                    c.sub_dist_3c0 = -0.0f - (floor > sub ? sub : floor);
                }
            }
        }
        c.aim_yaw_want_3d4 = ease(c.aim_yaw_want_3d4, -static_cast<double>(c.yaw_input_3e4), k3);
        if (!fixed_pitch) c.aim_pitch_want_3d8 = ease(c.aim_pitch_want_3d8, -static_cast<double>(m), k3);
        c.look_yaw_want_3c4 = ease(c.look_yaw_want_3c4, 0.0, k3);
        c.look_pitch_want_3c8 = ease(c.look_pitch_want_3c8, 0.0, k3);
    }
    // 0042FDB3, common tail.
    c.aim_yaw_3dc = ease(c.aim_yaw_3dc, c.aim_yaw_want_3d4, k3);
    c.aim_pitch_3e0 = ease(c.aim_pitch_3e0, c.aim_pitch_want_3d8, k3);
    float w = f32(static_cast<double>(c.look_yaw_want_3c4) - c.look_yaw_3cc);
    if (-kPi > w) w = f32(w + kTwoPi);
    else if (w > kPi) w = f32(w - kTwoPi);
    const double kl = static_cast<double>(dt) * look_rate;   // FST double [ESP+28h]
    // w + (0 - w) * kl: the stored yaw lands kl * w short of the wanted one.
    const float w2 = f32(w + (0.0 - w) * kl);
    const float yaw = f32(static_cast<double>(w2) + c.look_yaw_3cc);
    c.look_yaw_3cc = wrap_once(f32(std::fmod(static_cast<double>(yaw), kTwoPi)));
    c.look_pitch_3d0 = ease(c.look_pitch_3d0, c.look_pitch_want_3c8, kl);
}

void default_unit_camera_update_chase_00431480(DefaultUnitCamera& c, float dt,
                                               ChaseCameraHost& host) {
    if (!(dt >= 0.0f)) return;   // 0043149F JC
    Mat4Rows& m = c.chase_2b8;
    Vec3& p = m.position_a4;
    float rest = dt;
    do {
        const float step = rest >= 0.01f ? 0.01f : rest;   // 00D7A238
        bool live = false;
        if (c.target_388 != nullptr) {
            chase_camera_update_offsets_0042f800(c, step, host);
            void* t = c.target_388;
            live = t != nullptr && host.entity_byte(t, 0x5D) == 0 && host.entity_byte(t, 0x911) == 0;
        }
        if (live) {
            void* t = c.target_388;
            const Mat4Rows tp = host.entity_pose_cc(t);
            m = tp;
            const float back_extra = f32(host.class_float(t, 0x168) / static_cast<double>(c.class_scale_384));
            const float up_extra = host.class_float(t, 0x16C);
            rotate_yaw_then_pitch_0042b6e0(m, c.look_pitch_3d0, c.look_yaw_3cc, host);
            host.premultiply_rotation_z(m, -0.0f - c.z_rot_3b0);
            const float back = f32(static_cast<double>(c.z_dist_3bc) + back_extra);
            const Vec3 vb{f32(static_cast<double>(m.forward_94.x) * back),
                          f32(static_cast<double>(m.forward_94.y) * back),
                          f32(static_cast<double>(back) * m.forward_94.z)};
            // 00431630/00431680 refresh the target pose again before each row read.
            const float up = f32(static_cast<double>(c.y_dist_3b8) + up_extra);
            const Vec3 vu{f32(static_cast<double>(up) * tp.up_84.x), f32(static_cast<double>(tp.up_84.y) * up),
                          f32(static_cast<double>(up) * tp.up_84.z)};
            const float side = c.x_dist_3b4;
            const Vec3 vs{f32(static_cast<double>(tp.right_74.x) * side),
                          f32(static_cast<double>(tp.right_74.y) * side),
                          f32(static_cast<double>(side) * tp.right_74.z)};
            const Vec3 sum{f32(static_cast<double>(vs.x) + vu.x), f32(static_cast<double>(vs.y) + vu.y),
                           f32(static_cast<double>(vs.z) + vu.z)};
            p.x = f32(p.x + static_cast<double>(f32(static_cast<double>(sum.x) - vb.x)));
            p.y = f32(static_cast<double>(f32(static_cast<double>(sum.y) - vb.y)) + p.y);
            p.z = f32(static_cast<double>(f32(static_cast<double>(sum.z) - vb.z)) + p.z);
            rotate_yaw_then_pitch_0042b6e0(m, c.aim_pitch_3e0, c.aim_yaw_3dc, host);
        } else {
            // 00431792
            c.pitch_input_3e8 = 0.0f;
            c.yaw_input_3e4 = 0.0f;
            const Vec3 tp = c.target_position_38c;
            std::array<float, 3> d{f32(static_cast<double>(tp.x) - p.x), f32(static_cast<double>(tp.y) - p.y),
                                   f32(static_cast<double>(tp.z) - p.z)};
            const float sq = f32((static_cast<double>(d[0]) * d[0] + static_cast<double>(d[1]) * d[1]) +
                                 static_cast<double>(d[2]) * d[2]);
            normalize_camera_basis_0042b260(d);
            if (400.0 > sq) {   // 00CE3D90: closer than 20 -> put it at 20
                const float k = f32(static_cast<double>(f32(std::sqrt(static_cast<double>(sq)))) - 20.0);
                p.x = f32(p.x + static_cast<double>(f32(static_cast<double>(d[0]) * k)));
                p.y = f32(p.y + static_cast<double>(f32(static_cast<double>(d[1]) * k)));
                p.z = f32(static_cast<double>(f32(static_cast<double>(k) * d[2])) + p.z);
            }
            const Vec3 f = m.forward_94, u = m.up_84;
            const float kf = f32(4.25 * step);   // 00CE3D80
            const Vec3 df{f32(static_cast<double>(d[0]) - f.x), f32(static_cast<double>(d[1]) - f.y),
                          f32(static_cast<double>(d[2]) - f.z)};
            const Vec3 new_forward{f32(f.x + static_cast<double>(f32(static_cast<double>(df.x) * kf))),
                                   f32(static_cast<double>(f32(static_cast<double>(df.y) * kf)) + f.y),
                                   f32(static_cast<double>(f32(static_cast<double>(kf) * df.z)) + f.z)};
            const Vec3 du{f32(0.0 - u.x), f32(1.0 - u.y), f32(0.0 - u.z)};
            const float ku = f32(static_cast<double>(step) * 1.5);   // 00CE3D78
            const Vec3 new_up{f32(static_cast<double>(f32(static_cast<double>(du.x) * ku)) + u.x),
                              f32(static_cast<double>(f32(static_cast<double>(du.y) * ku)) + u.y),
                              f32(static_cast<double>(f32(static_cast<double>(ku) * du.z)) + u.z)};
            void* t = c.target_388;
            std::array<float, 3> e{};
            if (t != nullptr && host.entity_byte(t, 0x911) == 0) {
                const Vec3 v{f32(static_cast<double>(d[0]) * step), f32(static_cast<double>(d[1]) * step),
                             f32(static_cast<double>(step) * d[2])};
                const float speed = host.entity_speed_38(t);
                const Vec3 vv{f32(static_cast<double>(v.x) * speed), f32(static_cast<double>(v.y) * speed),
                              f32(static_cast<double>(speed) * v.z)};
                const float share = c.speed_share_3f4;
                p.x = f32(p.x + static_cast<double>(f32(static_cast<double>(vv.x) * share)));
                p.y = f32(static_cast<double>(f32(static_cast<double>(vv.y) * share)) + p.y);
                p.z = f32(p.z + static_cast<double>(f32(static_cast<double>(share) * vv.z)));
                const float rate = f32(static_cast<double>(step) * 0.10000000149011612);   // 00D7A3A0
                const float cur = c.speed_share_3f4;
                const float dist = std::fabs(f32(static_cast<double>(cur) - 0.25));         // AND 7FFFFFFFh
                if (rate > dist) c.speed_share_3f4 = 0.25f;
                else c.speed_share_3f4 = f32(0.25f > cur ? static_cast<double>(cur) + rate
                                                         : static_cast<double>(cur) - rate);
                e = {f32(static_cast<double>(tp.x) - p.x), 0.0f, f32(static_cast<double>(tp.z) - p.z)};
                const float sq2 = f32((static_cast<double>(e[0]) * e[0] + 0.0) + static_cast<double>(e[2]) * e[2]);
                normalize_camera_basis_0042b260(e);
                if (3600.0 > sq2) {   // 00CE3D70: horizontally closer than 60
                    const Vec3 es{f32(static_cast<double>(e[0]) * step), f32(static_cast<double>(e[1]) * step),
                                  f32(static_cast<double>(step) * e[2])};
                    const float k = f32(static_cast<double>(f32(std::sqrt(static_cast<double>(sq2)))) - 60.0);
                    p.x = f32(static_cast<double>(f32(static_cast<double>(es.x) * k)) + p.x);
                    p.y = f32(static_cast<double>(f32(static_cast<double>(es.y) * k)) + p.y);
                    p.z = f32(static_cast<double>(f32(static_cast<double>(k) * es.z)) + p.z);
                }
            } else {
                const Vec3 r = m.right_74;
                const Vec3 r5{f32(static_cast<double>(r.x) * 5.0), f32(static_cast<double>(r.y) * 5.0),
                              f32(5.0 * r.z)};   // 00D7A370
                const Vec3 drift{f32(static_cast<double>(r5.x) * step), f32(static_cast<double>(r5.y) * step),
                                 f32(static_cast<double>(step) * r5.z)};
                p.x = f32(static_cast<double>(drift.x) + p.x);
                p.y = f32(p.y + static_cast<double>(drift.y));
                p.z = f32(static_cast<double>(drift.z) + p.z);
                e = {f32(static_cast<double>(tp.x) - p.x), 0.0f, f32(static_cast<double>(tp.z) - p.z)};
                const float sq2 = f32((0.0 + static_cast<double>(e[0]) * e[0]) + static_cast<double>(e[2]) * e[2]);
                normalize_camera_basis_0042b260(e);
                if (10000.0 > sq2) {   // 00CE3D64: horizontally closer than 100
                    const Vec3 es{f32(static_cast<double>(e[0]) * step), f32(static_cast<double>(e[1]) * step),
                                  f32(static_cast<double>(step) * e[2])};
                    const float k = f32(static_cast<double>(f32(std::sqrt(static_cast<double>(sq2)))) - 100.0);
                    p.x = f32(static_cast<double>(f32(static_cast<double>(es.x) * k)) + p.x);
                    p.y = f32(static_cast<double>(f32(static_cast<double>(es.y) * k)) + p.y);
                    p.z = f32(static_cast<double>(f32(static_cast<double>(k) * es.z)) + p.z);
                }
            }
            if (45.0f > p.y) {   // 00CE3D60
                const float lift = f32((45.0 - p.y) * step);
                p.y = f32(static_cast<double>(lift) + p.y);
            }
            m.forward_94 = new_forward;
            m.up_84 = new_up;
        }
        host.orthonormalize_0085dc80(m);
        rest = f32(static_cast<double>(rest) - static_cast<double>(0.01f));   // 00D7A358
    } while (rest >= 0.0f);
}

void set_forward_orthonormalize_0042d5e0(Mat4Rows& m, const Vec3& forward, MoverHost& host) {
    m.forward_94 = forward;
    host.orthonormalize_0085dc80(m);
}

namespace {

// The shared tail of 00433800 (00433A1D..00433C1C) and 00433CB0
// (00433EC1..004340C4): split the chase forward F into its part along the
// target's flattened heading n, its part along world up, and the rest, scale
// each by a class length times ShipCamera.LengthMult, and subtract the sum
// from the anchor point.
Vec3 submarine_offset_point(const SubmarineCamera& c, void* t, const Vec3& anchor,
                            SubmarineCameraHost& host) {
    const Vec3 f = c.chase_2b8.forward_94;   // saved before the normalize call
    const Mat4Rows tp = host.entity_pose_cc(t);
    std::array<float, 3> n{tp.forward_94.x, 0.0f, tp.forward_94.z};
    normalize_camera_basis_0042b260(n);
    const float a = f32(static_cast<double>(n[2]) * f.z +
                        (static_cast<double>(n[1]) * f.y + static_cast<double>(n[0]) * f.x));
    const Vec3 along{f32(static_cast<double>(n[0]) * a), f32(static_cast<double>(n[1]) * a),
                     f32(static_cast<double>(a) * n[2])};
    const Vec3 up = host.world_up_00f8758c();
    const float b = f32((static_cast<double>(f.x) * up.x + static_cast<double>(up.y) * f.y) +
                        static_cast<double>(f.z) * up.z);
    const Vec3 vertical{f32(static_cast<double>(up.x) * b), f32(static_cast<double>(up.y) * b),
                        f32(static_cast<double>(b) * up.z)};
    const Vec3 rest{f32(static_cast<double>(f.x) - f32(static_cast<double>(vertical.x) + along.x)),
                    f32(static_cast<double>(f.y) - f32(static_cast<double>(vertical.y) + along.y)),
                    f32(static_cast<double>(f.z) - f32(static_cast<double>(vertical.z) + along.z))};
    const double mult = host.ship_camera_length_mult_454();
    const float k_along = f32(static_cast<double>(host.class_float(t, 0x53C)) * mult);
    const float k_up = f32(static_cast<double>(host.class_float(t, 0x544)) * mult);
    const float k_rest = f32(mult * host.class_float(t, 0x540));
    const Vec3 sa{f32(static_cast<double>(along.x) * k_along), f32(static_cast<double>(along.y) * k_along),
                  f32(static_cast<double>(k_along) * along.z)};
    const Vec3 su{f32(static_cast<double>(vertical.x) * k_up), f32(static_cast<double>(vertical.y) * k_up),
                  f32(static_cast<double>(k_up) * vertical.z)};
    const Vec3 sr{f32(static_cast<double>(rest.x) * k_rest), f32(static_cast<double>(rest.y) * k_rest),
                  f32(static_cast<double>(k_rest) * rest.z)};
    const Vec3 total{f32(static_cast<double>(f32(static_cast<double>(sr.x) + su.x)) + sa.x),
                     f32(static_cast<double>(f32(static_cast<double>(sr.y) + su.y)) + sa.y),
                     f32(static_cast<double>(f32(static_cast<double>(sr.z) + su.z)) + sa.z)};
    return Vec3{f32(static_cast<double>(anchor.x) - total.x), f32(static_cast<double>(anchor.y) - total.y),
                f32(static_cast<double>(anchor.z) - total.z)};
}

// Both mode steps start the chase basis from the flattened target forward
// with up (0, 1, 0) and RotY(-yaw).
void submarine_basis(SubmarineCamera& c, void* t, SubmarineCameraHost& host) {
    const Mat4Rows tp = host.entity_pose_cc(t);
    c.chase_2b8.forward_94 = Vec3{tp.forward_94.x, 0.0f, tp.forward_94.z};
    c.chase_2b8.up_84 = Vec3{0.0f, 1.0f, 0.0f};
    host.orthonormalize_0085dc80(c.chase_2b8);
    host.premultiply_rotation_y(c.chase_2b8, -0.0f - c.yaw_384);
}

}  // namespace

void submarine_camera_step_mode0_00433800(SubmarineCamera& c, SubmarineCameraHost& host) {
    void* t = c.target_3ac;   // [t+538h] is read before the null test (00433814)
    const Mat4Rows tp = host.entity_pose_cc(t);
    const float tilt = f32(static_cast<double>(f32(static_cast<double>(tp.forward_94.y) *
                                                   host.class_float(t, 0xA0))) *
                           0.30000001192092896);   // 00CE3DC8
    const float y = f32(static_cast<double>(f32(static_cast<double>(tp.position_a4.y) - tilt)) +
                        host.class_float(t, 0x538));
    const Vec3 anchor{tp.position_a4.x, y, tp.position_a4.z};
    submarine_basis(c, t, host);
    if (0.0f > c.pitch_388) host.premultiply_rotation_x(c.chase_2b8, -0.0f - c.pitch_388);
    const Vec3 p = submarine_offset_point(c, t, anchor, host);
    if (c.pitch_388 >= 0.0f) host.premultiply_rotation_x(c.chase_2b8, -0.0f - c.pitch_388);
    c.chase_2b8.position_a4 = p;
    c.byte_404 = 0;
}

void submarine_camera_step_mode1_00433cb0(SubmarineCamera& c, bool clamp, SubmarineCameraHost& host) {
    void* t = c.target_3ac;
    const Mat4Rows tp = host.entity_pose_cc(t);
    const Vec3 anchor = tp.position_a4;
    const float limit = f32(static_cast<double>(host.entity_float(t, 0x100)) / 50.0);   // 00CE3938
    const float pitch = c.pitch_388 > limit ? c.pitch_388 : limit;
    if (clamp) c.pitch_388 = pitch;
    submarine_basis(c, t, host);
    host.premultiply_rotation_x(c.chase_2b8, -0.0f - c.pitch_388);
    Vec3 p = submarine_offset_point(c, t, anchor, host);
    const float depth = host.class_float(t, 0x538);
    const float raise = 10.0f > depth ? depth : 10.0f;   // 00CE38B8
    p.y = f32(static_cast<double>(raise) + p.y);
    if (!(-3.0f > p.y)) p.y = -3.0f;   // 00CE3D50: stay at least 3 below the surface
    c.chase_2b8.position_a4 = p;
    c.byte_404 = 0;
}

void submarine_camera_step_00434140(SubmarineCamera& c, float dt, SubmarineCameraHost& host) {
    host.update_focus_0042dcd0(dt, 1);
    if (c.lost_405 == 0 && (c.target_3ac == nullptr || host.entity_byte(c.target_3ac, 0x5D) != 0)) {
        c.lost_405 = 1;
        c.lost_height_408 = c.chase_2b8.position_a4.y;
    }
    if (c.countdown_3e8 > 0) --c.countdown_3e8;
    if (void* t = c.target_3ac; t != nullptr) {
        const float target_y = host.entity_pose_cc(t).position_a4.y;
        if (c.mode_40c == 2) {   // periscope
            float p = c.pitch_388;
            if (-0.20943951606750488 > p) p = -0.209439516f;      // 00CE3E10 / 00CE3E08
            else if (p > 0.20943951606750488) p = 0.209439516f;   // 00CE3E00 / 00CE3DF8
            c.pitch_388 = p;
            // 004342B9/004342C0: (RotX x RotY) x target pose; the host applies
            // the two premultiplies in that order.
            c.chase_2b8 = host.entity_pose_cc(t);
            host.premultiply_rotation_y(c.chase_2b8, -0.0f - c.yaw_384);
            host.premultiply_rotation_x(c.chase_2b8, -0.0f - c.pitch_388);
            const Vec3 eye = host.node_world_point(c.node_3f4, c.node_offset_3f8);
            float y = f32(static_cast<double>(eye.y) + 2.0);         // 00D7A308
            if (y > 0.0f) y = f32(static_cast<double>(y) * 1.100000023841858);   // 00CE3DF0
            c.chase_2b8.position_a4 = Vec3{eye.x, y, eye.z};
        } else {
            float h = f32(static_cast<double>(host.entity_float(t, 0x1204)) + 2.0);
            if (-8.0 > h) h = -8.0f;   // 00CE3DE8 / 00CE3CC8
            if (c.mode_40c == 0) {
                if (static_cast<double>(h) - 0.5 > target_y) c.mode_40c = 1;
            } else if (c.mode_40c == 1) {
                if (target_y > static_cast<double>(h) + 0.5) c.mode_40c = 0;
            }
            if (c.mode_40c != 0) {
                submarine_camera_step_mode1_00433cb0(c, true, host);
            } else {
                submarine_camera_step_mode0_00433800(c, host);
                if (static_cast<double>(h) + 2.5 > target_y) {   // 00CE3DE0: near the surface, blend
                    const float lo = f32(static_cast<double>(h) + 0.5), hi = f32(static_cast<double>(h) + 2.5);
                    const float t_up = f32(dive_bomb_interpolate_clamped_00419010(lo, 0.0f, hi, 1.0f, target_y));
                    const Vec3 pa = c.chase_2b8.position_a4, fa = c.chase_2b8.forward_94;
                    const Vec3 aa{f32(static_cast<double>(f32(static_cast<double>(fa.x) * 150.0)) + pa.x),
                                  f32(static_cast<double>(f32(static_cast<double>(fa.y) * 150.0)) + pa.y),
                                  f32(static_cast<double>(f32(150.0 * fa.z)) + pa.z)};   // 00CE3DD8
                    submarine_camera_step_mode1_00433cb0(c, false, host);
                    const Vec3 pb = c.chase_2b8.position_a4, fb = c.chase_2b8.forward_94;
                    const Vec3 ab{f32(static_cast<double>(f32(static_cast<double>(fb.x) * 150.0)) + pb.x),
                                  f32(static_cast<double>(f32(static_cast<double>(fb.y) * 150.0)) + pb.y),
                                  f32(static_cast<double>(f32(150.0 * fb.z)) + pb.z)};
                    const float u = f32(1.0 - t_up);
                    auto mix = [&](float a, float b) {
                        return f32(static_cast<double>(f32(static_cast<double>(a) * t_up)) +
                                   f32(static_cast<double>(b) * u));
                    };
                    const Vec3 p{mix(pa.x, pb.x), mix(pa.y, pb.y), mix(pa.z, pb.z)};
                    const Vec3 q{mix(aa.x, ab.x), mix(aa.y, ab.y), mix(aa.z, ab.z)};
                    const Vec3 d{f32(static_cast<double>(q.x) - p.x), f32(static_cast<double>(q.y) - p.y),
                                 f32(static_cast<double>(q.z) - p.z)};
                    set_forward_orthonormalize_0042d5e0(c.chase_2b8, host.normalize_00419510(d), host);
                    host.orthonormalize_0085dc80(c.chase_2b8);
                    c.chase_2b8.position_a4 = p;
                }
            }
        }
        host.apply_zoom_0042c610();
    }
    if (c.lost_405 != 0) {
        const float v = c.lost_height_408;
        const float floor = v > 0.0f ? (3.0 > v ? 3.0f : v) : -125.0f;   // 00D7A2B0 / 00CE3854 / 00CE3DD0
        float& y = c.chase_2b8.position_a4.y;
        if (!(y > floor)) y = floor;
    }
    if (c.active_380 != 0) {
        c.pose = c.chase_2b8;
        host.apply_shake_and_publish_004329d0(dt);
    }
}

void orbit_target_from_yaw_0042c940(OrbitMover& m, float yaw_offset, OrbitMoverHost& host) {
    const float quarter = f32((static_cast<double>(m.pitch_min_far_3f0) - m.pitch_min_3ec) * 0.25);   // 00D7A348
    const float lo = f32(static_cast<double>(m.pitch_min_3ec) + quarter);
    const float p = m.pitch_388;
    m.pitch_388 = lo > p ? lo : (p > 0.0f ? 0.0f : p);
    const float yaw = -f32(static_cast<double>(m.yaw_384) + yaw_offset);   // FCHS of the float spill
    // 0042CA1E: RotX(-0.075 [00CE3CC4]) x RotY(yaw); identity x RotY is exact.
    Mat4Rows basis;
    basis.right_74 = Vec3{1.0f, 0.0f, 0.0f};
    basis.up_84 = Vec3{0.0f, 1.0f, 0.0f};
    basis.forward_94 = Vec3{0.0f, 0.0f, 1.0f};
    host.premultiply_rotation_y(basis, yaw);
    host.premultiply_rotation_x(basis, -0.075000003f);
    m.chase_2b8 = basis;
    const float twice = f32(static_cast<double>(m.distance_38c) + m.distance_38c);
    const Vec3 f = m.chase_2b8.forward_94;
    const Vec3 back{f32(static_cast<double>(f.x) * twice), f32(static_cast<double>(f.y) * twice),
                    f32(static_cast<double>(twice) * f.z)};
    const Vec3 t = host.entity_world_position(m.target_3ac);
    m.chase_2b8.position_a4 = Vec3{f32(static_cast<double>(t.x) - back.x), f32(static_cast<double>(t.y) - back.y),
                                   f32(static_cast<double>(t.z) - back.z)};
    host.premultiply_rotation_x(m.chase_2b8, -0.0f - m.pitch_388);
    host.apply_zoom_0042c610();
    m.placed_400 = 1;
}

void mover_build_frustum_planes_0042e180(MoverFrustum& f, MoverFrustumHost& host) {
    if (f.built_1f4 != 0) return;
    const float horizontal = host.fov_aspect_product_00b6fc30();
    auto set = [&](int i, float x, float y, float z, float d) {
        f.local_258[i][0] = x; f.local_258[i][1] = y; f.local_258[i][2] = z; f.local_258[i][3] = d;
    };
    set(0, 0.0f, 0.0f, -1.0f, host.node_float(0x1D4));                                   // near
    set(1, 0.0f, 0.0f, 1.0f, f32(300.0 / host.world_camera_node_1c4()));                   // 00CE3CA8
    const float half_v = f32(static_cast<double>(host.node_float(0x1C4)) * 0.5);           // 00D7A280
    const float sin_v = f32(std::sin(static_cast<double>(half_v)));
    const float cos_v = f32(std::cos(static_cast<double>(half_v)));
    set(2, 0.0f, cos_v, -sin_v, 0.0f);
    set(3, 0.0f, -cos_v, -sin_v, 0.0f);
    const float half_h = f32(static_cast<double>(horizontal) * 0.5);
    const float sin_h = f32(std::sin(static_cast<double>(half_h)));
    const float cos_h = f32(std::cos(static_cast<double>(half_h)));
    set(4, cos_h, 0.0f, -sin_h, 0.0f);
    set(5, -cos_h, 0.0f, -sin_h, 0.0f);
    const Vec3 origin = host.node_world_matrix().position_a4;   // +120h..+128h
    for (int i = 0; i < 6; ++i) {
        const Vec3 n = host.transform_direction_0042d0d0(
            Vec3{f.local_258[i][0], f.local_258[i][1], f.local_258[i][2]}, host.node_world_matrix());
        f.world_1f8[i][0] = n.x;
        f.world_1f8[i][1] = n.y;
        f.world_1f8[i][2] = n.z;
        f.world_1f8[i][3] = f32(((static_cast<double>(n.x) * origin.x + static_cast<double>(n.y) * origin.y) +
                                 static_cast<double>(origin.z) * n.z) +
                                f.local_258[i][3]);
    }
    f.built_1f4 = 1;
}

bool camera_mover_turn_toward_0042c750(float& yaw_384, float& pitch_388, float yaw, float yaw_step,
                                       float pitch, float pitch_step) noexcept {
    const float d_yaw = wrapped_angle_subtract_00438b10(yaw, yaw_384);
    const float d_pitch = wrapped_angle_subtract_00438b10(pitch, pitch_388);
    const float a_yaw = std::fabs(d_yaw), a_pitch = std::fabs(d_pitch);   // AND 7FFFFFFFh
    if (0.001f > yaw_step && 0.001f > pitch_step) return false;            // 00D7A23C
    if (yaw_step > a_yaw && pitch_step > a_pitch) {
        yaw_384 = yaw;
        pitch_388 = pitch;
        return true;
    }
    if (a_yaw > a_pitch) pitch_step = f32(static_cast<double>(a_pitch) * pitch_step / a_yaw);
    else yaw_step = f32(static_cast<double>(a_yaw) * yaw_step / a_pitch);
    if (yaw_step > a_yaw) yaw_384 = yaw;
    else yaw_384 = d_yaw > 0.0f ? wrapped_angle_add_00438aa0(yaw_384, yaw_step)
                                : wrapped_angle_subtract_00438b10(yaw_384, yaw_step);
    if (pitch_step > a_pitch) pitch_388 = pitch;
    else pitch_388 = d_pitch > 0.0f ? wrapped_angle_add_00438aa0(pitch_388, pitch_step)
                                    : wrapped_angle_subtract_00438b10(pitch_388, pitch_step);
    return false;
}

void default_unit_camera_step_00434e50(DefaultUnitCamera& camera, float dt,
                                       DefaultUnitCameraHost& host) {
    if (host.game_flag_635()) host.element_170_slot4(host.game_float_21ec());
    if (void* target = camera.target_388; target != nullptr) {
        if (camera.timer_3ec > 0.0f) {
            const float t = f32(camera.timer_3ec - dt * 0.07999999821186066);   // 00CE3E20 double
            camera.timer_3ec = 0.0 > t ? 0.0f : t;   // XORPS default when 0 > t
        }
        if (host.target_state_900(target) == 6) camera.flag_3f0 = 0;
        if (!host.target_byte_911(target))
            camera.target_position_38c = host.entity_world_position(target);
    }
    host.update_chase_00431480(dt);
    Mat4Rows m = camera.chase_2b8;
    if (host.has_ground()) {   // 00434F50..00434FF1, outside Ghidra's body
        const Vec3 p = m.position_a4;
        const float floor = f32(host.ground_height_at(p) + 5.0);   // 00D7A370 double
        const float y = p.y > floor ? p.y : floor;
        camera.chase_2b8.position_a4 = Vec3{p.x, y, p.z};
        m.position_a4.y = y;
    }
    camera.pose = m;
    if (camera.active_380 != 0) {
        m = camera.chase_2b8;
        if (10.0f > m.position_a4.y) {   // 00CE38B8
            float floor = 2.0f;           // 00CE3958
            if (host.has_water()) {
                floor = f32(host.water_height_0078cf20(m.position_a4.x, m.position_a4.z) + 2.0);   // 00D7A308
                if (2.0f > floor) floor = 2.0f;
            }
            if (floor > m.position_a4.y) m.position_a4.y = floor;
        }
        camera.pose = m;
        host.apply_shake_and_publish_004329d0(dt);
        if (camera.target_388 != nullptr && host.has_sink_1bc())
            host.set_sink_velocity_1b8(host.entity_velocity_34(camera.target_388));
    }
    if (camera.counter_3f8 != 0) --camera.counter_3f8;
}

}  // namespace bsp::seg2_cc13
