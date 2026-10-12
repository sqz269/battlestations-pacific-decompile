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
    // cc13-camera2. 00B64780 BSP_Matrix_BuildRotationZ, then as above.
    virtual void premultiply_rotation_z(Mat4Rows& pose, float angle) = 0;
};

// 0042B6E0 __thiscall(matrix, float pitch, float yaw) RET 8 (cc13-camera2):
// m = RotY(-0 - yaw) x m, then m = RotX(-0 - pitch) x m; returns the matrix.
void rotate_yaw_then_pitch_0042b6e0(Mat4Rows& m, float pitch, float yaw, MoverHost& host);

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

// ---- cc13-camera2 -------------------------------------------------------

// The 48h-byte random wander record (cc13-camera2): a vector that eases from
// its last value to a random point on a sphere shell, re-drawn every period.
// The plane cockpit camera embeds three at +428h, +470h, +4B8h; the plane code
// 007C7800 also calls the retarget (not read here).
struct RandomWander {
    Vec3 start_00;          // output at the last retarget
    Vec3 target_0c;         // random point: radius * (sin az sin el, cos az sin el, cos el)
    Vec3 output_18;         // the value consumers read
    float elapsed_24{0.0f}; // seconds since the last retarget
    float period_28{0.0f};  // current ease period
    float budget_2c{0.0f};  // total time left for retargets
    float life_30{0.0f};    // envelope length; the advance stops once elapsed exceeds it
    float radius_lo_34{0.0f}, radius_hi_38{0.0f};
    float azimuth_3c{0.0f};
    float period_lo_40{0.0f}, period_hi_44{0.0f};
};
static_assert(sizeof(RandomWander) == 0x48);

// The random source for 0042C1B0. In the image every draw is
// 00BD2F10 BSP_Random_UniformFloatRange with ECX = 0, i.e. the SHARED gameplay
// stream 0. A host must supply a separate presentation stream (cc13 RNG rule);
// drawing stream 0 here would shift every later gameplay draw (fire stagger,
// AI) and break same-binary deterministic pairs.
struct WanderRandom {
    virtual ~WanderRandom() = default;
    virtual float uniform(float lo, float hi) = 0;
};

// 0042C090 __fastcall(record), RET: output = target + (1 - t^2) (start -
// target) with t = min(elapsed / period, 1), then scaled by 1 - u^2 with
// u = min(elapsed / life, 1).
void random_wander_evaluate_0042c090(RandomWander& w) noexcept;
// 0042C1B0 __fastcall(record), RET: start = output; elapsed = 0; period and
// radius drawn only when their ranges are not degenerate; azimuth advanced by
// a random step in [0.5, 2 pi - 0.5] (wrapped, 00438AA0); elevation random in
// [0, pi]. Draw sites, all 00BD2F10 stream 0 in the image, in order:
//   0042C1F9  period  U(+40h, +44h)   skipped when +40h == +44h
//   0042C23D  radius  U(+34h, +38h)   skipped when +34h == +38h (radius = +34h)
//   0042C26C  azimuth step U(0.5 [00CE3800], 5.7831855 [00CE3CB0])
//   0042C2A6  elevation    U(0, 3.1415927 [00D7A264])
void random_wander_retarget_0042c1b0(RandomWander& w, WanderRandom& rng);
// 0042DAF0 __thiscall(record, float dt) RET 4: advance the clock, retarget
// once per elapsed period while the budget lasts, then evaluate.
void random_wander_advance_0042daf0(RandomWander& w, float dt, WanderRandom& rng);

// One eased angle of the cockpit camera (14h bytes): current += (dt * rate /
// period) * (target - current).
struct EasedAngle {
    float target{0.0f};
    float unused_4{0.0f};  // not read by 00434810
    float current{0.0f};
    float period{1.0f};
    float rate{0.0f};
};

// The plane cockpit camera fields (vtable 00CF4410, built by 006065D0 for
// kind 0Fh). Offsets from the mover.
struct PlaneCockpitCamera {
    Mat4Rows pose;                  // +74h
    Mat4Rows published_2b8;         // +2B8h, a copy of the pose each step
    EasedAngle yaw_384;             // +384h, applied about Y
    EasedAngle pitch_398;           // +398h, applied about X
    EasedAngle roll_3ac;            // +3ACh, applied about Z, negated
    void* plane_3c0{nullptr};       // +3C0h, a counted reference (0042D9A0)
    Vec3 eye_offset_3c4;            // +3C4h, in the plane's frame
    Vec3 look_offset_3d0;           // +3D0h, in the plane's frame
    Vec3 head_move_3dc;             // +3DCh, sum of the three wanders, rescaled
    Mat4Rows look_frame_3e8;        // +3E8h: plane world matrix, position +418h moved
    RandomWander wander[3];         // +428h, +470h, +4B8h
    void* modifier_514{nullptr};    // +514h
};

struct PlaneCockpitHost : MoverHost {
    // [plane+44h] != 0: the plane is going away; 0042D9A0(&+3C0h, 0) then
    // replaces the counted reference with null.
    virtual bool plane_released_44(void* plane) = 0;
    virtual void release_reference_0042d9a0(void*& reference) = 0;
    // [0042E740()]+offset; this step reads only ACh,
    // PlaneCamera/CockpitMaxHeadMoveDist (docs/GAME_TUNING_SINGLETON.md).
    virtual float tuning_float(std::uint32_t offset) = 0;
    // The plane's world matrix at +F0h (00B6DB70 first when bit 2 of +5Ch is clear).
    virtual Mat4Rows entity_world_matrix(void* entity) = 0;
    // BSS vector 00F87574..00F8757C.
    virtual Vec3 global_vector_00f87574() = 0;
    // 0042D0D0(out, &v, &m, 0): 3x3 direction transform, no normalize.
    virtual Vec3 transform_direction_0042d0d0(const Vec3& v, const Mat4Rows& m) = 0;
    // modifier->vtable[8Ch]() -> bool; modifier->vtable[94h](&out) returns a
    // matrix M and 00413920 makes pose = M x pose.
    virtual bool modifier_active_8c(void* modifier) = 0;
    virtual void premultiply_modifier_94(void* modifier, Mat4Rows& pose) = 0;
};

// 00434810 __thiscall(camera, float dt) RET 4, vtable 00CF4410 slot DCh
// (renamed BSP_PlaneCockpitCamera_Step, provisional: it reads only
// CockpitMaxHeadMoveDist from the tuning). With a plane: advance the three
// wanders, head move = their sum scaled by r / InterpolateClamped(0.7, 0.7,
// 2.0, 1.2, r) with r = |sum| / CockpitMaxHeadMoveDist; ease the three angles;
// pose = plane world matrix, eye moved by (eye offset + head move) in the
// plane's frame, then premultiplied by Y(yaw), X(pitch), Z(-roll) about the
// eye (each only when |angle| > 1e-4) and the modifier; the look frame is the
// plane matrix moved by (look offset + head move). Always: +2B8h = pose, then
// 004329D0(dt). The wanders draw the random source: see 0042C1B0.
void plane_cockpit_camera_step_00434810(PlaneCockpitCamera& camera, float dt,
                                        PlaneCockpitHost& host, WanderRandom& rng);

// The default unit camera fields read by 00434E50 (vtable 00CF4238, built by
// 00605130 for every kind without its own class).
struct DefaultUnitCamera {
    Mat4Rows pose;                 // +74h
    Mat4Rows chase_2b8;            // +2B8h, written by 00431480; position row +2E8h
    std::uint8_t active_380{0};    // +380h
    float class_scale_384{1.0f};   // +384h, divides [[target+538h]+168h]
    void* target_388{nullptr};     // +388h
    Vec3 target_position_38c;      // +38Ch
    // Read and written by 0042F800 / 00431480 (cc13-camera2). Wanted values
    // +39Ch..+3A8h ease into the applied values +3B0h..+3BCh.
    float z_rot_want_39c{0.0f}, x_dist_want_3a0{0.0f}, y_dist_want_3a4{0.0f},
        z_dist_want_3a8{0.0f};
    float z_rot_3b0{0.0f};         // applied as RotZ(-0 - z_rot)
    float x_dist_3b4{0.0f};        // along the target's right row
    float y_dist_3b8{0.0f};        // along the target's up row, plus [class+16Ch]
    float z_dist_3bc{0.0f};        // back along the chase forward, plus [class+168h]/+384h
    float sub_dist_3c0{0.0f};      // BombCamera Torpedo/BombSubDist; feeds y_dist_want
    float look_yaw_want_3c4{0.0f}, look_pitch_want_3c8{0.0f};
    float look_yaw_3cc{0.0f}, look_pitch_3d0{0.0f};   // first 0042B6E0
    float aim_yaw_want_3d4{0.0f}, aim_pitch_want_3d8{0.0f};
    float aim_yaw_3dc{0.0f}, aim_pitch_3e0{0.0f};     // second 0042B6E0
    float yaw_input_3e4{0.0f};     // zeroed when the chase loses its target
    float pitch_input_3e8{0.0f};   // likewise; an angle (0042F800 turns it into degrees)
    float timer_3ec{0.0f};         // +3ECh, counts down at 0.08 per second
    std::uint8_t flag_3f0{0};      // +3F0h, cleared when target+900h == 6
    float speed_share_3f4{0.25f};  // +3F4h, eases to 0.25 at 0.1 per second
    std::uint32_t counter_3f8{0};  // +3F8h, decremented once per step while nonzero
};

// The target reads of 0042F800 and 00431480, by raw offset (cc13-camera2).
struct ChaseCameraHost : MoverHost {
    virtual float tuning_float(std::uint32_t offset) = 0;              // [0042E740()]+offset
    virtual float entity_float(void* entity, std::uint32_t offset) = 0;
    virtual std::uint32_t entity_u32(void* entity, std::uint32_t offset) = 0;
    virtual std::uint8_t entity_byte(void* entity, std::uint32_t offset) = 0;
    virtual float class_float(void* entity, std::uint32_t offset) = 0; // [[entity+538h]+offset]
    virtual bool entity_is_kind(void* entity, std::uint32_t kind) = 0; // vtable[5Ch](kind)
    virtual float entity_speed_38(void* entity) = 0;                   // vtable[38h]()
    // The entity matrix at +CCh (00414DB0 first when +C8h is clear).
    virtual Mat4Rows entity_pose_cc(void* entity) = 0;
};

// 0042F800 __thiscall(camera, float dt) RET 4 (renamed
// BSP_PlaneChaseCamera_UpdateOffsets, provisional; only caller 00431480).
// Reads the target's rotation rates +B50h/+B54h/+B58h, +B28h, +900h, +970h,
// +9B4h, [class+200h] and the PlaneCamera / BombCamera tunables (offsets 54h..80h,
// 13Ch..154h), and eases the chase offsets and look angles.
void chase_camera_update_offsets_0042f800(DefaultUnitCamera& camera, float dt,
                                          ChaseCameraHost& host);

// 00431480 __thiscall(camera, float dt) RET 4 (BSP_DefaultUnitCamera_UpdateChase):
// returns at once when dt < 0; otherwise sub-steps of min(rest, 0.01) while
// rest (reduced by 0.01 each pass) stays >= 0. With a live target (non-null,
// +5Dh and +911h clear) the chase matrix is rebuilt from the target pose and
// the offsets of 0042F800. Otherwise the chase matrix keeps its basis: the
// position steers towards the stored target position (pushed out to 20 when
// closer; then a speed-scaled pull and a push-out by (distance - 60) * dt when
// horizontally closer than 60, or with no target a sideways drift of 5 per
// second and the same push-out at 100), the
// forward and up rows ease toward the target direction and world up, and the
// height is pulled toward 45 when below it. Then 0085DC80.
void default_unit_camera_update_chase_00431480(DefaultUnitCamera& camera, float dt,
                                               ChaseCameraHost& host);

struct DefaultUnitCameraHost : MoverHost {
    // [00E188A8]+635h; when set, [mover+170h]->vtable[4]([game+21ECh]).
    virtual bool game_flag_635() = 0;
    virtual void element_170_slot4(float game_21ec) = 0;
    virtual float game_float_21ec() = 0;
    virtual std::uint32_t target_state_900(void* target) = 0;
    virtual bool target_byte_911(void* target) = 0;
    // +FCh..+104h after 00414DB0 when +C8h is clear.
    virtual Vec3 entity_world_position(void* entity) = 0;
    // 00431480 (ECX mover, dt): writes the chase matrix +2B8h.
    virtual void update_chase_00431480(float dt) = 0;
    // [00E188A8] and [game+19CCh] non-null; 00903860 BSP_World_GroundHeightAt.
    virtual bool has_ground() = 0;
    virtual float ground_height_at(const Vec3& p) = 0;
    // [00E188A8] and [game+19F0h] non-null; 0078CF20 (ECX = [game+19F0h], x, z).
    virtual bool has_water() = 0;
    virtual float water_height_0078cf20(float x, float z) = 0;
    // [mover+1BCh] non-null: the sink whose +1B8h..+1C0h receive the
    // target's vtable[34h] vector (its velocity).
    virtual bool has_sink_1bc() = 0;
    virtual Vec3 entity_velocity_34(void* entity) = 0;
    virtual void set_sink_velocity_1b8(const Vec3& v) = 0;
};

// 00434E50 __thiscall(camera, float dt) RET 4, vtable 00CF4238 slot DCh
// (BSP_DefaultUnitCamera_Step). Body 00434E50..0043511C; Ghidra's function
// stops at 00434F4F because of a stale misaligned unit at 00434F52, so the
// stretch 00434F50..00434FF1 (the ground clamp) is read with disasm-raw.
// Updates the timer and the target position, runs the chase update, lifts
// the chase matrix to 5.0 above the ground (stored back into +2E8h), sets the
// pose from it; when active, lifts it again below 10.0 to 2.0 above the water
// (at least 2.0), publishes (004329D0) and forwards the target velocity.
void default_unit_camera_step_00434e50(DefaultUnitCamera& camera, float dt,
                                       DefaultUnitCameraHost& host);

// The submarine camera fields (vtable 00CF6250; cc13-camera2).
struct SubmarineCamera {
    Mat4Rows pose;                  // +74h
    Mat4Rows chase_2b8;             // +2B8h
    std::uint8_t active_380{0};     // +380h
    float yaw_384{0.0f};            // +384h, applied as RotY(-0 - yaw)
    float pitch_388{0.0f};          // +388h, applied as RotX(-0 - pitch)
    void* target_3ac{nullptr};      // +3ACh, the submarine
    std::int32_t countdown_3e8{0};  // +3E8h, decremented while positive
    void* node_3f4{nullptr};        // +3F4h, the periscope node (mode 2)
    Vec3 node_offset_3f8;           // +3F8h, local point on that node
    std::uint8_t byte_404{0};       // +404h, cleared by both mode steps
    std::uint8_t lost_405{0};       // +405h, latched once the target is gone or dying
    float lost_height_408{0.0f};    // +408h, the chase height when it latched
    std::int32_t mode_40c{0};       // +40Ch: 0 surfaced, 1 submerged, 2 periscope
};

struct SubmarineCameraHost : MoverHost {
    virtual float entity_float(void* entity, std::uint32_t offset) = 0;
    virtual std::uint8_t entity_byte(void* entity, std::uint32_t offset) = 0;
    virtual float class_float(void* entity, std::uint32_t offset) = 0;   // [[entity+538h]+offset]
    // The entity matrix at +CCh (00414DB0 first when +C8h is clear).
    virtual Mat4Rows entity_pose_cc(void* entity) = 0;
    // [00424C40()]+454h, ShipCamera.LengthMult (game settings).
    virtual float ship_camera_length_mult_454() = 0;
    virtual Vec3 world_up_00f8758c() = 0;
    // 004142E0 of the point through the node world matrix +F0h (00B6DB70
    // first when bit 2 of node+5Ch is clear).
    virtual Vec3 node_world_point(void* node, const Vec3& local) = 0;
    // 00419510 BSP_Vector3f_Normalize (zero vector when the length is not > 0).
    virtual Vec3 normalize_00419510(const Vec3& v) = 0;
};

// 0042D5E0 __thiscall(matrix, &forward) RET 4 (cc13-camera2): forward row
// (+20h) = forward, then 0085DC80; returns the matrix.
void set_forward_orthonormalize_0042d5e0(Mat4Rows& m, const Vec3& forward, MoverHost& host);

// 00433800 __thiscall(camera, float dt) RET 4 (renamed BSP_SubmarineCamera_PlaceSurfaced;
// dt unused): chase basis = the target's flattened forward with
// world up, RotY(-yaw), RotX(-pitch) before the offset when pitch < 0 (after
// it otherwise); position = (target x, target y - 0.3 * forward.y *
// [class+A0h] + [class+538h], target z) minus the forward split into its
// horizontal-heading, world-up and remaining parts scaled by [class+53Ch],
// [class+544h], [class+540h] times ShipCamera.LengthMult.
void submarine_camera_step_mode0_00433800(SubmarineCamera& camera, SubmarineCameraHost& host);
// 00433CB0 __thiscall(camera, float dt, bool clamp) RET 8
// (renamed BSP_SubmarineCamera_PlaceSubmerged; dt unused): pitch limited below
// by target y / 50 (stored only when clamp); same basis and offsets from the
// target position with both rotations first; height raised by min([class+538h],
// 10) and capped at -3.0.
void submarine_camera_step_mode1_00433cb0(SubmarineCamera& camera, bool clamp,
                                          SubmarineCameraHost& host);
// 00434140 __thiscall(camera, float dt) RET 4 (BSP_SubmarineCamera_Step).
// Mode 2 is the periscope (pitch clamped to +-0.2094 rad, i.e. 12 degrees).
// Modes 0/1 switch with 0.5 hysteresis about h = [target+1204h] + 2 (floored at
// -8); surfaced below h + 2.5 blends the surfaced and submerged poses.
void submarine_camera_step_00434140(SubmarineCamera& camera, float dt,
                                    SubmarineCameraHost& host);

// 0042C750 __thiscall(mover, float yaw, float yaw_step, float pitch, float
// pitch_step) RET 10h -> AL (renamed BSP_CameraMover_TurnTowardAngles,
// provisional; callers 0042DCD0 BSP_CameraMover_UpdateFocus and 0051F050
// BSP_HudBinoculars_TurnMover). Turns the mover's +384h yaw and +388h pitch
// toward the request by at most the steps, scaling the smaller axis's step so
// both arrive together; snaps an axis already within its step. Returns 1 only
// when both were within their steps; nothing moves when both steps are below
// 0.001. Ghidra splits the pitch-step tail into a bogus FUN_0042C8F3
// (0042C8F3..0042C931, no callers; reached by the JBE at 0042C8D8).
// The fields 0042C940 reads (an orbit mover; callers 00545270 / 005452B0 in
// segment 16).
struct OrbitMover {
    Mat4Rows chase_2b8;              // +2B8h
    float yaw_384{0.0f};             // +384h
    float pitch_388{0.0f};           // +388h, clamped here
    float distance_38c{0.0f};        // +38Ch, the orbit puts the eye 2x this behind
    void* target_3ac{nullptr};       // +3ACh
    float pitch_min_3ec{0.0f};       // +3ECh
    float pitch_min_far_3f0{0.0f};   // +3F0h, a quarter of the way from +3ECh
    std::uint8_t placed_400{0};      // +400h, set to 1
};
struct OrbitMoverHost : MoverHost {
    // +FCh..+104h after 00414DB0 when +C8h is clear.
    virtual Vec3 entity_world_position(void* entity) = 0;
};
// 0042C940 __thiscall(mover, float yaw_offset) RET 4 (BSP_CameraMover_OrbitTargetFromYaw):
// pitch = clamp(pitch, min + (far - min) / 4, 0); basis = RotX(-0.075) x
// RotY(-(yaw + offset)); eye = target - 2 * distance * forward; then
// RotX(-0 - pitch) premultiplied (look only), 0042C610, +400h = 1.
void orbit_target_from_yaw_0042c940(OrbitMover& mover, float yaw_offset, OrbitMoverHost& host);

// The view frustum planes 0042E180 builds once (gate byte +1F4h): six local
// planes {nx, ny, nz, d} at +258h and their world copies at +1F8h.
struct MoverFrustum {
    std::uint8_t built_1f4{0};
    float world_1f8[6][4]{};
    float local_258[6][4]{};
};
struct MoverFrustumHost {
    virtual ~MoverFrustumHost() = default;
    virtual float node_float(std::uint32_t offset) = 0;   // [mover+1BCh]+offset
    // [[[00E188A8]+19FCh]+1C4h], the world camera node's value of the same field.
    virtual float world_camera_node_1c4() = 0;
    // 00B6FC30 BSP_Camera_GetFovAspectProduct (ECX = the node), ST0.
    virtual float fov_aspect_product_00b6fc30() = 0;
    // The node world matrix +F0h (00B6DB70 first when bit 2 of +5Ch is clear).
    virtual Mat4Rows node_world_matrix() = 0;
    virtual Vec3 transform_direction_0042d0d0(const Vec3& v, const Mat4Rows& m) = 0;
};
// 0042E180 __fastcall(mover), RET (renamed BSP_CameraMover_BuildFrustumPlanes,
// provisional; callers only the unreferenced thunks 0042E3D0 / 0042E460). Local
// planes: near (0,0,-1, node+1D4h), far (0,0,1, 300 / world node+1C4h), and the
// four sides from half of node+1C4h (vertical) and half of 00B6FC30
// (horizontal); each world plane is the direction transform of its normal
// with d = n . node position + local d.
void mover_build_frustum_planes_0042e180(MoverFrustum& f, MoverFrustumHost& host);

bool camera_mover_turn_toward_0042c750(float& yaw_384, float& pitch_388, float yaw, float yaw_step,
                                       float pitch, float pitch_step) noexcept;

}  // namespace bsp::seg2_cc13
