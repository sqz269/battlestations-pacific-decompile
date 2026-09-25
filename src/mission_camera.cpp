#include "bsp/mission_camera.hpp"

#include "bsp/camera_decomposition.hpp"
#include "bsp/camera_multiply.hpp"
#include "bsp/gamepad_force_events.hpp"
#include "bsp/plane_pose_commit.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/world_entity_update.hpp"

#include <cmath>

// Packet cc9_mission_camera, docs/MISSION_CAMERA.md.
//
// Every FSTP to a dword in the listing is a float store here; values the
// listing keeps on the x87 stack between loads are computed in double.
//
// Unreachable here, and why:
// - 0042DCD0's focus branch (0042DD2E..0042DEE9) runs only while +3D4h > 0.
//   +3D4h is written by 0042EA40 (vtable +11Ch, a "look at this entity for a
//   while" request) and by class-sibling routines, none of which the host
//   calls; 00519290 and 0042F650 store 0. The gate is transcribed and the
//   branch is left out; a positive +3D4h would be a host change, not a run.
// - 004329D0's shake block needs +1C0h > 0, which only shake requests raise.

namespace bsp {
namespace {

constexpr float kNegativeZero = -0.0f;            // 00D7A208
constexpr double kSwayGain = 3.0;                 // 00D7A2B0
constexpr float kInitialPitch = -0.1745329350233078f;  // 00CECA08
constexpr double kSeedDistance = -2.0;            // 00CF60F8, as a double
constexpr double kDegToRadPi = 3.1415927410125732;     // 00CE3D28
constexpr double kDegToRadDen = 180.0;                 // 00CE3D20
constexpr float kZoomSnap = 20000.0f;             // 00CE3CC0
constexpr double kZoomStep = 20000.0;             // 00CE3CB8
constexpr double kZoomDirFloor = -0.10000000149011612;  // 00CE3928
constexpr float kZoomDirFloorF = -0.10000000149011612f; // 00CE3CB4
constexpr double kHalf = 0.5;                     // 00D7A280


void rot_y(CameraMatrix16& out, float angle) {
    matrix_interpolator_rotation_y_00b646e0(out.data(), angle);
}
void rot_x(CameraMatrix16& out, float angle) {
    matrix_interpolator_rotation_x_00b64640(out.data(), angle);
}
// 00413920 with left = a stack copy (004134F0) of `left`, right = +2B8h, then
// the product copied back into +2B8h, as 004330E5..0043310E does.
void premultiply(CameraMatrix16& target, const CameraMatrix16& left) {
    CameraMatrix16 product{};
    multiply_camera_matrices_00413920(product, left, target);
    target = product;
}

// 0042DCD0, __thiscall(mover, float dt, bool flag), RET 8. The gate only.
void focus_gate_0042dcd0(ShipCaptainCamera& camera) {
    // 0042DCD9..0042DCE9: COMISS 0.0, +3D4h; JAE takes the early path.
    if (!camera.has_target || !(camera.focus_timer_3d4 > 0.0f)) {
        camera.focus_3e0 = 0.0f;   // 0042DD15
        camera.focus_3e4 = 0.0f;   // 0042DD1D
        return;
    }
    // 0042DD2E..0042DEE9: not reached by this host (see the file header).
}

// 0042C610, __thiscall(mover), RET.
void apply_zoom_0042c610(ShipCaptainCamera& camera) {
    const float target = camera.zoom_enabled_3a0 ? camera.zoom_length_3a4 : 0.0f;
    const float current = camera.zoom_3a8;
    const float gap = std::fabs(static_cast<float>(current - target));
    if (kZoomSnap > gap) {
        camera.zoom_3a8 = target;                                   // 0042C66E
    } else if (target > current) {
        camera.zoom_3a8 = static_cast<float>(current + kZoomStep);  // 0042C67E
    } else {
        camera.zoom_3a8 = static_cast<float>(current - kZoomStep);  // 0042C686
    }
    std::array<float, 3> dir{camera.orient_2b8[8], camera.orient_2b8[9], camera.orient_2b8[10]};
    if (kZoomDirFloor > static_cast<double>(dir[1])) dir[1] = kZoomDirFloorF;  // 0042C6C8
    normalize_camera_basis_0042b260(dir);
    const float z = camera.zoom_3a8;
    const float dx = dir[0] * z;
    const float dy = z * dir[1];
    const float dz = z * dir[2];
    camera.orient_2b8[12] = camera.orient_2b8[12] + dx;
    camera.orient_2b8[13] = camera.orient_2b8[13] + dy;
    camera.orient_2b8[14] = camera.orient_2b8[14] + dz;
}

// 0042F0C0, __thiscall(mover), RET. The probe walk against the water.
void keep_above_water_0042f0c0(ShipCaptainCamera& camera, const ShipCaptainTargetView& unit,
                               MissionCameraOcean& ocean) {
    if (!camera.has_target || camera.detached_400) return;   // vtable +130h -> 0064B6D0
    CameraMatrix16& local = camera.local_74;
    float T[3] = {local[12], local[13], local[14]};           // +A4h..+ACh
    const float tx = unit.world[12], ty = unit.world[13], tz = unit.world[14];
    const float dx = T[0] - tx;
    const float dy = T[1] - ty;
    const float dz = T[2] - tz;
    const float ux = unit.world[4], uy = unit.world[5], uz = unit.world[6];  // +DCh..+E4h
    const float h = static_cast<float>(
        (static_cast<double>(ux) * dx + static_cast<double>(uy) * dy) + static_cast<double>(uz) * dz);
    const float hh = (1.0f <= h || std::isnan(h)) ? h : 1.0f;   // 0042F1CD..0042F1E5
    const float uxh = ux * hh;
    const float uyh = uy * hh;
    const float uzh = uz * hh;
    const float bx = tx + uxh;
    const float by = ty + uyh;
    const float bz = tz + uzh;
    const int kind = 2;                                        // vtable +128h, 0042A8E0
    for (const auto& probe : kCameraProbes00e08088) {
        const float zr[3] = {local[8] * probe[2], probe[2] * local[9], probe[2] * local[10]};
        const float yr[3] = {local[4] * probe[1], probe[1] * local[5], probe[1] * local[6]};
        const float xr[3] = {local[0] * probe[0], probe[0] * local[1], probe[0] * local[2]};
        float q[3];
        for (int i = 0; i < 3; ++i) {
            const float q0 = xr[i] + T[i];
            const float q1 = q0 + yr[i];
            q[i] = q1 + zr[i];
        }
        float r[3] = {q[0], q[1], q[2]};
        if (ocean.present() && kind != 0) {
            const float water_base = ocean.water_height(bx, bz);  // 0042F38B
            const float water_probe = ocean.water_height(q[0], q[2]);  // 0042F3B1
            if (water_probe > q[1] && !(by < water_base) && !std::isnan(by) &&
                !std::isnan(water_base)) {
                r[1] = water_probe;                            // 0042F413
                if (kind == 2) {
                    r[0] = static_cast<float>((static_cast<double>(q[0]) + bx) * kHalf);
                    r[2] = static_cast<float>((static_cast<double>(q[2]) + bz) * kHalf);
                }
            }
        }
        // 0042F4A8..0042F503: the segment base -> r is tested against the
        // target's collision (vtable +B0h, 0042E630, 0098B370) and a hit
        // replaces r. SUBSTITUTION: the host builds no collision shape for a
        // unit, so the test reports no hit and r stands.
        for (int i = 0; i < 3; ++i) {
            const float delta = r[i] - q[i];
            T[i] = delta + T[i];
        }
    }
    local[12] = T[0];
    local[13] = T[1];
    local[14] = T[2];
}

// 004329D0, __thiscall(mover, float dt), RET 4. Returns true when the pose was
// pushed into the node at 00432B2B.
bool publish_pose_004329d0(ShipCaptainCamera& camera, const ShipCaptainTargetView& unit,
                           MissionCameraOcean& ocean, float dt, CameraMatrix16& world) {
    if (!camera.enabled_380) return false;
    if (camera.skip_33c) {
        camera.skip_33c = false;
        return false;
    }
    // The node is always attached here (0042A920 ran in 004BC410).
    // 00432A0B..00432AA0 (shake decay) needs +1C0h > 0; see the file header.
    for (int i = 0; i < 3; ++i) {
        const float step = camera.phase_rate_1dc[i] * dt;
        camera.phase_1e8[i] = step + camera.phase_1e8[i];
    }
    keep_above_water_0042f0c0(camera, unit, ocean);          // 00432AFD
    // 0042ED50 invalidates, 00414DB0 refreshes: the mover is a world-root
    // entity, so its world is its local matrix.
    world = camera.local_74;
    // 00432B38..00432CA5: the node velocity (+1B8h..) and the previous
    // position (+1A4h..) feed the sound listener only and are not modelled.
    return true;
}

}  // namespace

void construct_ship_captain_0064b650(ShipCaptainCamera& camera) noexcept {
    camera = ShipCaptainCamera{};
    // 00432750: +380h = 1, +33Ch = 0, +1C0h = 0, +1DCh..+1E4h = 1.0f.
    // SUBSTITUTION: its three 00BD2F10 draws into +1E8h..+1F0h (00432938,
    // 00432955, 00432972) are not taken. They would advance the shared random
    // stream the ship AI draws from, and the phases never reach the pose.
    // 00519290: +3D4h = 0, +3E8h = 0, +3ECh/+3F0h, then 0064B650 overrides
    // +3ECh = 00CEC728, +3F0h = 00CEC400, +3F4h = 0, +400h = 0.
}

void bind_ship_captain_0064da40(ShipCaptainCamera& camera, const ShipCaptainTargetView& unit,
                                const ShipCameraSettings& settings) noexcept {
    // 0064DBA0..0064DBD0: the pitch limits, degrees to radians.
    camera.min_pitch_3ec = static_cast<float>(
        (kDegToRadPi * settings.min_angle_deg) / kDegToRadDen);
    camera.max_pitch_3f0 = static_cast<float>(
        (settings.max_angle_deg * kDegToRadPi) / kDegToRadDen);
    const bool retarget = !camera.has_target;                 // 0064DBDC..0064DBE6
    // 00432E60(unit, 1): +3F4h = 1, 0042F650 sets the target and clears focus.
    camera.mode_3f4 = 1;
    camera.has_target = true;
    camera.focus_timer_3d4 = 0.0f;
    camera.focus_flag_3d8 = false;
    if (camera.countdown_3e8 == 0) {
        camera.zoom_length_3a4 = settings.zoom_offset * unit.length;  // 00432E98
        camera.sway_goal_3fc = 0.0f;
        camera.sway_3f8 = 0.0f;
    }
    if (!retarget) return;
    // 0064DC0A..0064DC92: 00521370 on the unit's forward row, then the seed.
    const float yaw = static_cast<float>(std::atan2(unit.world[8], unit.world[10]));
    const float distance = static_cast<float>(unit.length * kSeedDistance);
    if (camera.countdown_3e8 == 0) {
        camera.yaw_384 = kNegativeZero - yaw;
        camera.pitch_388 = kInitialPitch;
        for (float& d : camera.distance_38c) d = distance;
    }
}

void weapon_group_screen_release_005470c0(HudWeaponGroupScreenState& screen,
                                          HudWeaponGroupScreenHost& host) {
    for (const HudWeaponGroupEntry& entry : screen.entries) {           // 005470C8
        if (entry.unit != 0) host.clear_child_flags_005470c0(entry.unit);
    }
    const std::uint32_t mask = weapon_group_role_mask_00545410(screen.group_44);
    for (const HudWeaponGroupEntry& entry : screen.entries) {           // 0054710F
        if (entry.unit == 0) continue;
        if (screen.group_44 >= 2 && screen.group_44 <= 5) {
            static_cast<void>(host.group_available_009542b0(entry.unit, screen.group_44));
        }
        if (mask != 0) host.role_request_0077c470(entry.unit, mask, false);
    }
    host.reset_widgets_005464e0();                                      // 00547180
    screen.hold_100 = false;
    screen.lst_rocket_d6 = false;
}

void weapon_group_screen_bind_00549260(HudWeaponGroupScreenState& screen,
                                       HudWeaponGroupScreenHost& host,
                                       std::size_t group_unit, bool bound) {
    // 0054927C..005492A2: +40h through the observer pair at +2Ch.
    screen.mover_40 = bound;
    screen.target_54 = 0;                                               // 005492AF
    if (group_unit == 0) {
        screen.entries.clear();                                         // 005467B0(0)
        return;
    }
    // 005492D2..005492E5: [ESP+3Ch] is reused as "the first entry changed".
    const bool changed = screen.entries.empty() || screen.entries.front().unit != group_unit;
    // 005492EA..00549323: 005467B0(0), 005460A0(group_unit), 00546730, 00545600.
    screen.entries.clear();
    screen.entries.push_back({group_unit});
    if (host.unit_is_kind_of(group_unit, 0x1C)) {                       // 00549328
        for (std::size_t unit : host.unit_records_778(group_unit)) {
            if (unit != 0) screen.entries.push_back({unit});
        }
    }
    if (changed) {                                                      // 0054936F
        screen.group_44 = 0;
        weapon_group_cycle_00548410(screen, host, true);
        if (host.unit_is_kind_of(group_unit, 6) && !host.unit_is_kind_of(group_unit, 8) &&
            !host.unit_is_kind_of(group_unit, 0x0E)) {
            weapon_group_select_005484b0(screen, host, 3);
        }
    } else if (weapon_group_any_available_00545b30(screen, host, screen.group_44)) {
        weapon_group_set_roles_00545bd0(screen, host, true);            // 005493D5
    } else {
        weapon_group_cycle_00548410(screen, host, true);                // 005493CA
    }
    weapon_group_hint_00548360(screen, host);                           // 005493DE
    screen.lst_rocket_d6 = host.player_unit_is_lst_rocket();            // 005493E3..0054940D
}

bool update_ship_captain_00432ed0(ShipCaptainCamera& camera, const ShipCaptainTargetView& unit,
    const ShipCameraSettings& settings, MissionCameraOcean& ocean, float dt,
    CameraMatrix16& world) noexcept {
    ++camera.frames;
    if (camera.countdown_3e8 != 0) --camera.countdown_3e8;     // 00432ED9
    if (camera.has_target) {
        // 00432F01..00432F81: the sway.
        const float twice = static_cast<float>(static_cast<double>(dt) + dt);
        const float t = (1.0f <= twice || std::isnan(twice)) ? 1.0f : twice;
        const double goal = static_cast<double>(unit.rudder) * unit.throttle * kSwayGain;
        const float a = camera.sway_goal_3fc;
        const float new_a = static_cast<float>(a + (goal - a) * t);
        camera.sway_goal_3fc = new_a;
        const float b = camera.sway_3f8;
        camera.sway_3f8 = static_cast<float>(b + (static_cast<double>(new_a) - b) * t);
    }
    focus_gate_0042dcd0(camera);                               // 00432F99
    if (camera.has_target && !camera.detached_400) {
        if (camera.mode_3f4 == 0 && unit.gate_5d) {            // 00432FC8..00433028
            camera.mode_3f4 = 1;
            float ax = 0.0f, ay = 0.0f, az = 0.0f;
            CameraMatrix basis = unit.world;
            extract_camera_matrix_angles_0042d2e0(basis, ax, ay, az);
            camera.yaw_384 = wrapped_angle_add_00438aa0(camera.yaw_384, kNegativeZero - ay);
        }
        // 0043303E..00433069: the pivot.
        float pivot[3] = {unit.world[12], unit.min_height, unit.world[14]};
        CameraMatrix16 rotation{};
        if (camera.mode_3f4 == 0) {
            // 00433085..004330BF: 0042D5A0 rows 2 and 1, then 0085DC80.
            camera.orient_2b8[8] = unit.world[8];
            camera.orient_2b8[9] = 0.0f;
            camera.orient_2b8[10] = unit.world[10];
            camera.orient_2b8[4] = kWorldUp00f8758c[0];
            camera.orient_2b8[5] = kWorldUp00f8758c[1];
            camera.orient_2b8[6] = kWorldUp00f8758c[2];
            orthonormalize_pose_matrix_0085dc80(camera.orient_2b8.data());
            rot_y(rotation, kNegativeZero - camera.yaw_384);
            premultiply(camera.orient_2b8, rotation);
            // 0043310E..00433158: the sway along row 0.
            const float s = camera.sway_3f8;
            const float o0 = camera.orient_2b8[0] * s;
            const float o1 = camera.orient_2b8[1] * s;
            const float o2 = s * camera.orient_2b8[2];
            pivot[0] = pivot[0] - o0;
            pivot[1] = pivot[1] - o1;
            pivot[2] = pivot[2] - o2;
        } else {
            // 0043315E..0043319B.
            rot_y(rotation, kNegativeZero - camera.yaw_384);
            camera.orient_2b8 = rotation;
        }
        const float pitch = camera.pitch_388;
        if (0.0f > pitch) {                                    // 004331A0..004331F1
            rot_x(rotation, kNegativeZero - pitch);
            premultiply(camera.orient_2b8, rotation);
        }
        // 004331F6..0043325C.
        const double rx = camera.orient_2b8[8];
        const double ry = camera.orient_2b8[9];
        const double rz = camera.orient_2b8[10];
        std::array<float, 3> v{unit.world[8], 0.0f, unit.world[10]};
        normalize_camera_basis_0042b260(v);
        const float dot = static_cast<float>((v[0] * rx + ry * v[1]) + v[2] * rz);
        float front[3] = {static_cast<float>(v[0] * static_cast<double>(dot)),
                          static_cast<float>(v[1] * static_cast<double>(dot)),
                          static_cast<float>(v[2] * static_cast<double>(dot))};
        const double gx = kWorldUp00f8758c[0], gy = kWorldUp00f8758c[1],
                     gz = kWorldUp00f8758c[2];
        const float dot2 = static_cast<float>((rx * gx + ry * gy) + rz * gz);
        float up[3] = {static_cast<float>(gx * dot2), static_cast<float>(dot2 * gy),
                       static_cast<float>(dot2 * gz)};
        float sum[3];
        for (int i = 0; i < 3; ++i) sum[i] = up[i] + front[i];
        const float r[3] = {static_cast<float>(rx), static_cast<float>(ry),
                            static_cast<float>(rz)};
        float side[3];
        for (int i = 0; i < 3; ++i) side[i] = r[i] - sum[i];
        // 0043333C..004333DD: the class distances times LengthMult.
        const float lm = settings.length_mult;
        const float kf = unit.distance_front * lm;
        for (float& c : front) c = c * kf;
        const float kv = unit.distance_vertical * lm;
        for (float& c : up) c = c * kv;
        const float ks = lm * unit.distance_side;
        for (float& c : side) c = c * ks;
        float offset[3];
        for (int i = 0; i < 3; ++i) {
            const float su = side[i] + up[i];
            offset[i] = su + front[i];
        }
        // 00433429..00433464: the zoom length from ZoomOffset.
        const float zo = settings.zoom_offset;
        const std::array<float, 3> h{offset[0] * zo, 0.0f, zo * offset[2]};
        camera.zoom_length_3a4 = force_event_vector_length_0042b2f0(h);
        float position[3];
        for (int i = 0; i < 3; ++i) position[i] = pivot[i] - offset[i];
        if (!(pitch < 0.0f) && !std::isnan(pitch)) {           // 0043346E..004334E2
            rot_x(rotation, kNegativeZero - pitch);
            premultiply(camera.orient_2b8, rotation);
        }
        camera.orient_2b8[12] = position[0];                   // 004334E7..00433510
        camera.orient_2b8[13] = position[1];
        camera.orient_2b8[14] = position[2];
        camera.out_2f8 = camera.orient_2b8;                    // 00433518
        apply_zoom_0042c610(camera);                           // 0043351F
    }
    bool published = false;
    if (camera.enabled_380) {                                  // 00433524
        camera.local_74 = camera.orient_2b8;                   // 00433538
        published = publish_pose_004329d0(camera, unit, ocean, dt, world);
    }
    camera.detached_400 = false;                               // 0043354F
    return published;
}

MissionCameraPublication& mission_camera_publication() noexcept {
    static MissionCameraPublication publication;
    return publication;
}

void publish_mission_camera(const CameraMatrix16& world, const MissionCameraProjection& projection) {
    MissionCameraPublication& node = mission_camera_publication();
    node.state.projection.fov = projection.fov;
    node.state.projection.aspect = projection.aspect;
    node.state.projection.near_plane = projection.near_plane;
    node.state.projection.far_plane = projection.far_plane;
    // The projection-valid bit is cleared so 00B70490 rebuilds from these terms
    // (the setters 00B6FBB0/00B6FBD0/00B6FC10 clear it the same way).
    node.state.projection.valid_flags &= 0xFFFFFF41u;
    CameraMatrix matrix = world;
    set_camera_world_matrix_00b71460(node.state, matrix);
    node.ready = true;
    ++node.publishes;
}

void clear_mission_camera() noexcept {
    MissionCameraPublication& node = mission_camera_publication();
    node.ready = false;
    node.publishes = 0;
}

float global_config_fov_0087ec0f(double fovs_degrees, float divisor) noexcept {
    // FMUL qword pi (00CE3D28), FDIV qword 180.0 (00CE3D20), FDIV dword divisor.
    return static_cast<float>(fovs_degrees * kDegToRadPi / kDegToRadDen / divisor);
}

float pipe_sight_fov_scale_0064e2d6(float zoom_rate, float zoom_state) noexcept {
    // FLD [settings+7Ch]; FMUL [00E197F4]; FLD1; FSUBRP.
    return static_cast<float>(1.0 - static_cast<double>(zoom_rate) * zoom_state);
}

float mission_fov_004dc940(float stored, float divisor, float scale) noexcept {
    // FLD [global+F4h+idx*4]; FMUL [00F889B4]; FMUL scale; FSTP.
    return static_cast<float>(static_cast<double>(stored) * divisor * scale);
}

}  // namespace bsp
