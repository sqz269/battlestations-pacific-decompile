// Segment 2 camera movers. See include/bsp/segment2_camera_cc13.hpp and
// docs/CC13_SEGMENT2_CAMERA_MATH.md.
#include "bsp/segment2_camera_cc13.hpp"

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

}  // namespace bsp::seg2_cc13
