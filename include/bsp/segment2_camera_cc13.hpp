#pragma once
// Segment 2 (0042A920-0043E840) camera movers, cc13-net3. See
// docs/CC13_SEGMENT2_CAMERA_MATH.md for the class map (mover base vtable
// 00CE3B28, constructor 00432750; every class's per-frame step is vtable slot
// DCh) and the evidence for each function. Names are hypotheses.
//
// This file reconstructs the INTF_BOMBVIEW (26h) pair that 0067B4D0 builds:
//   torpedo view  (3CCh bytes, vtable 00CE3E60): 00435120 construct,
//                 0042FEE0 set target, 00430020 step (not yet reconstructed)
//   bomb view     (3F8h bytes, vtable 00CE3FD0): 00435280 construct,
//                 004307F0 set target, 00430DA0 step
// and the orbit helper 0042D610 the bomb step calls.
// The mover base fields used here: the 4x4 pose matrix at +74h (rows +74h
// right, +84h up, +94h forward, +A4h position), the pose-valid byte +C8h,
// +10Ch, the child node chain +48h (linked through child+44h), and the
// sink at +1BCh whose vtable[34h] receives &mover+CCh.
#include <cstdint>

namespace bsp::seg2_cc13 {

struct Vec3 {
    float x{0.0f}, y{0.0f}, z{0.0f};
};

struct Mat4Rows {
    Vec3 right_74;    float w_80{0.0f};
    Vec3 up_84;       float w_90{0.0f};
    Vec3 forward_94;  float w_a0{0.0f};
    Vec3 position_a4; float w_b0{1.0f};
};

inline constexpr std::uint32_t kMoverBaseVtable = 0x00CE3B28u;
inline constexpr std::uint32_t kTorpedoViewVtable = 0x00CE3E60u;  // +0
inline constexpr std::uint32_t kBombViewVtable = 0x00CE3FD0u;     // +0
// The observer object both cameras embed at +384h (vtable 00CE3CF0): +14h,
// i.e. camera+398h, is the observed target; +10h is a byte set to 1.
inline constexpr std::uint32_t kTargetObserverVtable = 0x00CE3CF0u;

// The fields the two INTF_BOMBVIEW cameras add after the mover base.
struct WeaponViewCamera {
    Mat4Rows pose;                 // +74h
    std::uint8_t pose_valid_c8{0}; // +C8h
    std::uint8_t byte_10c{0};      // +10Ch
    void* target_398{nullptr};     // observer +14h
    // Torpedo view: +39Ch target position, +3A8h start position,
    // +3B4h start forward, +3C0h start up.
    // Bomb view: +39Ch forward, +3A8h up, +3B4h target position, +3C0h
    // camera position, +3CCh velocity, +3D8h, +3DCh offset, +3E8h, +3ECh,
    // +3F0h yaw, +3F4h pitch.
    Vec3 v_39c, v_3a8, v_3b4, v_3c0, v_3cc;
    float f_3d8{0.0f};
    Vec3 offset_3dc;
    float f_3e8{0.0f}, f_3ec{0.0f};
    float yaw_3f0{0.0f}, pitch_3f4{0.0f};
};

struct WeaponViewHost {
    virtual ~WeaponViewHost() = default;
    // 006952A0 (ECX = old target, EDX = &camera+384h) / 00694A60 (ECX = new
    // target, EDX = &camera+384h): BSP_Observer_UnregisterPair / RegisterPair.
    virtual void observer_unregister_006952a0(void* target) = 0;
    virtual void observer_register_00694a60(void* target) = 0;
    // [game+19FCh] is the world camera node; its world matrix (node+F0h) is
    // refreshed by 00B6DB70 when bit 2 of node+5Ch is clear, then copied into
    // camera+74h by 004134F0 (BSP_Matrix_Copy4x4X87).
    virtual Mat4Rows camera_node_world() = 0;
    // The target's world position (+FCh..+104h) after 00414DB0 when its +C8h
    // byte is clear.
    virtual Vec3 entity_world_position(void* entity) = 0;
    // target->vtable[5Ch](2Bh): is an MTorpedo.
    virtual bool is_torpedo(void* entity) = 0;
    // [[target+314h]+F4h], read only for a torpedo.
    virtual float torpedo_value_314_f4(void* entity) = 0;
    // launcher->vtable[34h](&out): the vector 004307F0 copies to +3CCh.
    virtual Vec3 launcher_vector_34(void* launcher) = 0;
    // BSS vector 00F87574..00F8757C (no initializer read here).
    virtual Vec3 global_vector_00f87574() = 0;
    virtual float tuning_camera_pos_smooth_e8() = 0;  // [0042E740()]+E8h
    // 0085DC80 BSP_Matrix_OrthonormalizeBasisRows(&camera+74h).
    virtual void orthonormalize_0085dc80(Mat4Rows& m) = 0;
    // 00419440 BSP_Vector3f_Length.
    virtual float vector_length_00419440(const Vec3& v) = 0;
    // 0085E8C0 (ECX = EDX = matrix, stack point, &axis, angle): rotate the
    // matrix about the axis through the point (not opened).
    virtual void rotate_about_axis_0085e8c0(Mat4Rows& m, const Vec3& point, const Vec3& axis,
                                            float angle) = 0;
    // 0042ED50 on each child of +48h, then 00414DB0 on the camera, then
    // [camera+1BCh]->vtable[34h](&camera+CCh).
    virtual void invalidate_children_and_publish() = 0;
    // cc13-render5. The entity's world forward row x and z (+ECh, +F4h of the
    // world matrix at +CCh), after 00414DB0 when +C8h is clear.
    virtual void entity_forward_xz(void* entity, float& x, float& z) = 0;
    // cc13-render5. 0042E630 BSP_SpatialIndex_GetSingleton, then
    // 0098ADD0 BSP_SpatialIndex_QuerySegment(&start, &end, 0, &hit, 0) on a
    // hit record reset by 00470470 and destroyed by 004704B0. True when the
    // query hits and the record's part (+38h of the record) is non-null; then
    // part_top = [part+24h]+4h and entity_y = hit entity +100h (after 00414DB0).
    virtual bool segment_obstacle(const Vec3& start, const Vec3& end, float& part_top,
                                  float& entity_y) = 0;
    // cc13-render5. 00903860 BSP_World_GroundHeightAt([[00E188A8]+19CCh], &p, &out).
    virtual float ground_height_at(const Vec3& p) = 0;
    // cc13-render5. entity->vtable[5Ch](kind): 2Bh MTorpedo, 33h, 2Ah.
    virtual bool entity_is_kind(void* entity, std::uint32_t kind) = 0;
    // cc13-render5. entity->vtable[34h](&out): the entity's velocity vector.
    virtual Vec3 entity_velocity_34(void* entity) = 0;
    // cc13-render5. [0042E740()]+offset, the BombCamera tunables
    // (docs/GAME_TUNING_SINGLETON.md): F0h CamVelBlenderAcceleration, F4h
    // MaxCamVelBlender, FCh/100h/104h Torpedo NearAlt/1, FollowDistNear/Far,
    // 10Ch/110h/114h Rocket, 11Ch/120h/124h Bomb, 138h FinalDist.
    virtual float tuning_float(std::uint32_t offset) = 0;
};

// 00435120 __fastcall(obj) and 00435280 __fastcall(obj): the constructors'
// own fields (after 00432750). Both set the observer to {vtable 00CE3CF0,
// +4h..+Ch = 0, +10h = 1, +14h = 0}; 00435280 also zeroes +3F0h/+3F4h.
void construct_torpedo_view_00435120(WeaponViewCamera& camera) noexcept;
void construct_bomb_view_00435280(WeaponViewCamera& camera) noexcept;

// 0042FEE0 __thiscall(camera, target) RET 4.
void set_torpedo_view_target_0042fee0(WeaponViewCamera& camera, void* target,
                                      WeaponViewHost& host);
// 004307F0 __thiscall(camera, target, launcher) RET 8.
void set_bomb_view_target_004307f0(WeaponViewCamera& camera, void* target, void* launcher,
                                   WeaponViewHost& host);
// 0042D610 __thiscall(matrix, &point, &axis, angle) RET 0Ch: rotate by
// angle * |axis| about -axis/|axis| through the point (returns the matrix).
void orbit_about_axis_0042d610(Mat4Rows& m, const Vec3& point, const Vec3& axis, float angle,
                               WeaponViewHost& host);
// 00430DA0 __thiscall(camera, float dt) RET 4, vtable 00CE3FD0 slot DCh.
void bomb_view_step_00430da0(WeaponViewCamera& camera, float dt, WeaponViewHost& host);

// Torpedo view tunables, initialized .data read only by 00430020.
inline constexpr float kTorpedoStepMax = 0.03f;        // 00E08074, fixed sub-step
inline constexpr float kTorpedoBlendXZ = 0.08f;        // 00E08078, per max step
inline constexpr float kTorpedoBlendY = 0.04f;         // 00E0807C, per sub-step
inline constexpr float kTorpedoTrailDistance = 30.0f;  // 00E08080
inline constexpr float kTorpedoHeight = 3.0f;          // 00E08084

// 00430020 __thiscall(camera, float dt) RET 4, vtable 00CE3E60 slot DCh
// (cc13-render5). Torpedo view fields: +39Ch target position, +3A8h smoothed
// eye, +3B4h smoothed view direction, +3C0h smoothed up. The aim point trails
// the target by 30 along its flattened forward (100 along the camera's own
// flattened forward when there is no target) at height 3 above the target, or
// 30 without a target; a segment query from the eye's x/z at height 3 to the
// target raises it to 4 above any hit part. dt is consumed in sub-steps of at
// most 0.03.
void torpedo_view_step_00430020(WeaponViewCamera& camera, float dt, WeaponViewHost& host);

// 00430970 __fastcall(element), RET: slot 4h of the bomb view's +170h element
// (vtable 00CE3F94); ECX = camera+170h, so element+228h is the target, +244h
// the target point +3B4h, +250h the camera position +3C0h, +25Ch the
// velocity +3CCh, +268h the blend +3D8h (cc13-render5). Steers the camera
// velocity towards keeping the follow distance from the target point led by
// 1.6 s of target velocity: distance interpolated over height above ground
// (floored at 8.0) per weapon kind, FinalDist without a target.
void bomb_view_update_velocity_00430970(WeaponViewCamera& camera, WeaponViewHost& host);

// Services the airfield and bomber-view steps call (cc13-render5).
struct MoverHost {
    virtual ~MoverHost() = default;
    // 004329D0 BSP_CameraMover_ApplyShakeAndPublishPose(dt) (cc9's
    // docs/MISSION_CAMERA.md), the common tail.
    virtual void apply_shake_and_publish_004329d0(float dt) = 0;
    // 0042DCD0 BSP_CameraMover_UpdateFocus(dt, 1).
    virtual void update_focus_0042dcd0(float dt, std::uint32_t mode) = 0;
    // 0042C610 BSP_CameraMover_ApplyZoom (ECX mover).
    virtual void apply_zoom_0042c610() = 0;
    virtual void entity_world_pose(void* entity, Vec3& position, float& forward_x,
                                   float& forward_z) = 0;   // +FCh.., +ECh/+F4h after 00414DB0
    virtual void orthonormalize_0085dc80(Mat4Rows& m) = 0;
    // 00B646E0 / 00B64640 BSP_Matrix_BuildRotationY / X (ECX out, EDX &angle),
    // then 00413920 BSP_Matrix_Multiply4x4 with ECX = the rotation, stack
    // (out, &pose): pose = rotation x pose.
    virtual void premultiply_rotation_y(Mat4Rows& pose, float angle) = 0;
    virtual void premultiply_rotation_x(Mat4Rows& pose, float angle) = 0;
};

// 00433590 __thiscall(mover, float dt) RET 4, vtable 00CEC1F8 slot DCh
// (MAirfield): when byte +380h is set, copy the matrix at +2B8h into the pose
// (004134F0) and run 004329D0(dt); otherwise nothing.
void airfield_camera_step_00433590(Mat4Rows& pose, bool active_380, const Mat4Rows& matrix_2b8,
                                   float dt, MoverHost& host);

// Global override written by luaMW_PlayerSetBomberView (008A0850): a nonzero
// float flag at 00E17670 with pitch 00E17674 and yaw 00E17678.
struct BomberViewOverride {
    float flag_e17670{0.0f}, pitch_e17674{0.0f}, yaw_e17678{0.0f};
};
// The class-56 (vtable 00CEC5F0) fields.
struct BomberViewCamera {
    Mat4Rows pose;            // +74h
    float yaw_384{0.0f};      // +384h
    float pitch_388{0.0f};    // +388h
    float distance_38c{0.0f}; // +38Ch
    void* target_3ac{nullptr};
};
// 004335C0 __thiscall(mover, float dt) RET 4, vtable 00CEC5F0 slot DCh: the
// player's bomber view (provisional: the only override writer is the Lua
// binding PlayerSetBomberView). Orbits the target: forward = the target's
// flattened forward, up = (0,1,0), then rotations by -yaw about Y and -pitch
// about X, eye = target position + (0, 10, 0) - distance * forward.
void bomber_view_step_004335c0(BomberViewCamera& camera, BomberViewOverride& override_state,
                               float dt, MoverHost& host);

// 0042B570 __thiscall(list, &point, &normal) RET 8 (cc13-render5): append one
// {point, normal} pair (18h bytes) when the count at +C0h is below 8; callers
// BSP_Plane_SweepCollisionPoints and 0060ABD0.
struct ContactPointList {
    Vec3 point_normal[8][2];
    std::uint32_t count_c0{0};
};
void contact_point_list_append_0042b570(ContactPointList& list, const Vec3& point,
                                        const Vec3& normal) noexcept;

}  // namespace bsp::seg2_cc13
