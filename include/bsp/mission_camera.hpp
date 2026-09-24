#pragma once
// The in-mission camera a ship gets: the "ShipCaptain" camera mover and the
// Operator camera node it drives. Packet cc9_mission_camera,
// docs/MISSION_CAMERA.md. Names are descriptive hypotheses, not recovered
// symbols; field comments give the native offsets on the mover (vtable
// 00CF5CE8, 404h bytes) or on the node (game+19FCh, camera 00B71A80).
//
// Coverage:
//   0064DA40 (the bind step that matters for the pose), 0064B650 + 00519290 +
//   00432750 (the fields the update reads), 00432E60 + 0042F650 (set target),
//   00432ED0 (the update, complete), 0042DCD0 (its gate; the focus branch is
//   unreachable here, see the header of mission_camera.cpp), 0042D5A0,
//   0042C610, 004329D0 (the pose publication; the velocity tail is omitted
//   and labelled), 0042F0C0 (complete except the ray test against the
//   target's collision, a labelled substitution).
// Not ABI-compatible: new C++ interfaces over plain storage.

#include "bsp/camera_transform.hpp"

#include <array>
#include <cstdint>

namespace bsp {

using CameraMatrix16 = std::array<float, 16>;

// What the mover reads off its target unit each frame.
struct ShipCaptainTargetView {
    CameraMatrix16 world{};   // unit+CCh..+108h: rows right, up, forward, translation
    float throttle{0.0f};     // unit+980h
    float rudder{0.0f};       // unit+984h
    bool gate_5d{false};      // unit+5Dh
    // [unit+538h] class fields (ship_class_fields.hpp ShipClassCameraFields).
    float min_height{0.0f};       // +548h CameraMinHeight
    float distance_front{0.0f};   // +53Ch CameraDistanceFront
    float distance_side{0.0f};    // +540h CameraDistanceSide
    float distance_vertical{0.0f};// +544h CameraDistanceVertical
    float length{0.0f};           // +A0h Length
};

// ShipGlobals["ShipCamera"], the 00424C40 settings block fields the mover reads.
struct ShipCameraSettings {
    float zoom_offset{0.0f};      // +450h ZoomOffset
    float length_mult{0.0f};      // +454h LengthMult
    float min_angle_deg{0.0f};    // +458h MinCameraAngle
    float max_angle_deg{0.0f};    // +45Ch MaxCameraAngle
};

// The world up vector 00F8758C..00F87594, copied from 00E0B68C = (0, 1, 0) by
// the static initializer at 00CD2280.
inline constexpr std::array<float, 3> kWorldUp00f8758c{0.0f, 1.0f, 0.0f};

// The five probe offsets 0042F0C0 walks, 00E08088..00E080C3 in .data.
inline constexpr float kCameraProbes00e08088[5][3] = {
    {0.0f, 0.0f, 0.0f}, {-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f},
    {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}};

struct ShipCaptainCamera {
    bool has_target{false};             // +3ACh non-null
    int mode_3f4{0};                    // +3F4h, 1 = yaw fixed in the world
    bool detached_400{false};           // +400h
    int countdown_3e8{0};               // +3E8h
    float yaw_384{0.0f};                // +384h
    float pitch_388{0.0f};              // +388h
    float distance_38c[4]{};            // +38Ch..+398h, seeded, not read by the update
    bool zoom_enabled_3a0{false};       // +3A0h
    float zoom_length_3a4{0.0f};        // +3A4h
    float zoom_3a8{0.0f};               // +3A8h
    float focus_timer_3d4{0.0f};        // +3D4h, set only through vtable +11Ch
    bool focus_flag_3d8{false};         // +3D8h
    float focus_3e0{0.0f};              // +3E0h
    float focus_3e4{0.0f};              // +3E4h
    float min_pitch_3ec{-0.5235988f};   // +3ECh (00CEC728), then MinCameraAngle
    float max_pitch_3f0{1.5533431f};    // +3F0h (00CEC400), then MaxCameraAngle
    float sway_3f8{0.0f};               // +3F8h
    float sway_goal_3fc{0.0f};          // +3FCh
    bool enabled_380{true};             // +380h, 00432750
    bool skip_33c{false};               // +33Ch
    float shake_1c0{0.0f};              // +1C0h, 00432750 stores 0
    float phase_rate_1dc[3]{1.0f, 1.0f, 1.0f}; // +1DCh..+1E4h
    float phase_1e8[3]{};               // +1E8h..+1F0h, see the substitution note
    CameraMatrix16 orient_2b8{};        // +2B8h, the working matrix
    CameraMatrix16 out_2f8{};           // +2F8h, what vtable +120h returns
    CameraMatrix16 local_74{};          // +74h, the mover entity's local matrix
    std::uint64_t frames{0};
};

// The ocean sampler the probes use (0078CF20 through the host's wave field).
struct MissionCameraOcean {
    virtual ~MissionCameraOcean() = default;
    virtual bool present() = 0;                 // game != 0 and [game+19F0h] != 0
    virtual float water_height(float x, float z) = 0;
};

// 0064B650 over 00519290 and 00432750: the fields the update reads.
void construct_ship_captain_0064b650(ShipCaptainCamera& camera) noexcept;

// 0064DA40's part that shapes the pose, for a view whose previous target is
// not this unit: pitch limits, 00432E60(unit, 1), then the seed.
void bind_ship_captain_0064da40(ShipCaptainCamera& camera, const ShipCaptainTargetView& unit,
    const ShipCameraSettings& settings) noexcept;

// 00432ED0 with 004329D0 and 0042F0C0. Returns true when the pose was
// published to the node (the 00432B2B call), with the world matrix in `world`.
bool update_ship_captain_00432ed0(ShipCaptainCamera& camera, const ShipCaptainTargetView& unit,
    const ShipCameraSettings& settings, MissionCameraOcean& ocean, float scaled_delta,
    CameraMatrix16& world) noexcept;

// The Operator node's projection terms in mission. See docs/MISSION_CAMERA.md.
struct MissionCameraProjection {
    float fov{0.6981317f};      // +1C4h: 00B71A80's 00CE7D20 (see the doc's substitution)
    float aspect{1.3333334f};   // +1C8h: 004DCDF0 writes platform+10h; 4/3 substituted
    float near_plane{1.0f};     // +1D4h: 00B71A80's 00D7A24C
    float far_plane{20000.0f};  // +1D8h: 004DCDF0 writes 00CE3CC0
};

// The Operator node as this process holds it: one camera state that the mover
// publishes into each frame and the HUD reads through 00B6DB70 and 00B70490.
// Process-level for the same reason as scene_spawn_pool(): the publisher (the
// in-game interface host) and the readers (the minimap and markers hosts) do
// not share an owner. Cleared with the world.
struct MissionCameraPublication {
    bool ready{false};
    CameraState state;
    std::uint64_t publishes{0};
};
MissionCameraPublication& mission_camera_publication() noexcept;
// node->vtable[34h](world) (00B71460 on the Operator camera) after the
// projection terms are stored.
void publish_mission_camera(const CameraMatrix16& world, const MissionCameraProjection& projection);
void clear_mission_camera() noexcept;

} // namespace bsp
