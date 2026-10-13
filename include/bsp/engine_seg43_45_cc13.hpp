#pragma once
// Packet cc13-cam2 (docs/CC13_CAM_2.md): segment 45 (0078D880-007A4860, camera,
// movie camera, MovCam) and segment 43 (0076AA00-00787040, voice and session
// plumbing). Read from the listings and the disk bytes; descriptive names are
// hypotheses. Nothing here is ABI compatible or game-validated: entities are
// host handles (0 = none), engine helpers are host callbacks, and the x87
// chains are written in double with a float cast at each native FSTP (exact
// at 53-bit precision control when the products of two floats are exact).

#include "bsp/band_0076_cc13.hpp"
#include "bsp/cc13_camera_controllers.hpp"
#include "bsp/multiplayer_session_cc13.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_bit_cursor_read.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp::cc13cam2 {

using Vec3 = std::array<float, 3>;
using Mat4 = std::array<float, 16>;

// ---------------------------------------------------------------------------
// Entity observers. 00694A60 register, 006952A0 unregister, 00694AF0 is
// registered: ECX = the observed entity, EDX = the observer object (a
// CameraRef or path record at +0, the MovCam's observer base at +10h).
struct ObserverHost {
    virtual ~ObserverHost() = default;
    virtual void register_00694a60(std::size_t entity, const void* observer) = 0;
    virtual void unregister_006952a0(std::size_t entity, const void* observer) = 0;
    virtual bool is_registered_00694af0(std::size_t entity, const void* observer) = 0;
};

// What the camera code asks of an entity.
struct CameraEntityHost : ObserverHost {
    // byte +5Eh, the "being destroyed" flag the CameraRef constructors test.
    virtual bool destroyed_5e(std::size_t entity) = 0;
    // The world matrix +CCh after 00414DB0 (run when byte +C8h is clear).
    virtual Mat4 world_cc(std::size_t entity) = 0;
    // 004142E0(point, out, matrix): the affine transform.
    virtual Vec3 transform_004142e0(const Vec3& point, const Mat4& m) = 0;
    // 0085DAD0 on a matrix whose translation row was replaced by 00F8758C..94.
    virtual void orthonormalize_0085dad0(Mat4& m) = 0;
    // 00F8758C..00F87594 (loader zero-fill; written at run time).
    virtual Vec3 global_f8758c() = 0;
};

// ---------------------------------------------------------------------------
// The camera reference (vtable 00D04660, scalar dtor 00793900): an observer
// base (+4..+10h, cleared), +14h entity, +18h point, +24h radius. The layout
// is cc13cam::CameraRef; `observer` is the identity passed to the host.

// 00793920: ECX ref, stack (entity, const float* point), RET 8. An entity
// whose +5Eh is set is dropped; radius 0; registers the observer.
void camera_ref_construct_point_00793920(cc13cam::CameraRef& ref, std::size_t entity, const Vec3& point,
                                         CameraEntityHost& host);
// 007939B0: ECX ref, stack (entity, radius), RET 8. Point zero.
void camera_ref_construct_radius_007939b0(cc13cam::CameraRef& ref, std::size_t entity, float radius,
                                          CameraEntityHost& host);
// 00793A40: ECX ref, stack (const CameraRef*), RET 4. Copies +14h..+24h, then
// registers when the entity is set (no +5Eh test).
void camera_ref_copy_construct_00793a40(cc13cam::CameraRef& ref, const cc13cam::CameraRef& src,
                                        CameraEntityHost& host);
// 00790320: ECX dst, stack (const CameraRef*), RET 4, EAX = dst. As the copy
// constructor; the old entity is NOT unregistered first.
void camera_ref_assign_00790320(cc13cam::CameraRef& dst, const cc13cam::CameraRef& src, CameraEntityHost& host);

// 00793F10: ECX ref, stack (out, level byte), RET 8. No entity: the point.
// Otherwise the point through the entity world (level: translation replaced
// by 00F8758C..94, then 0085DAD0). The radius is ignored (00793AC0's non-
// radius branch).
Vec3 camera_ref_to_world_00793f10(const cc13cam::CameraRef& ref, bool level, CameraEntityHost& host);
// 00793CD0: ECX ref, stack (const float* p, level byte), RET 8. No entity:
// point = p. Otherwise point = p through the entity world (level as above).
// Radius = 0 in both cases.
void camera_ref_set_point_00793cd0(cc13cam::CameraRef& ref, const Vec3& p, bool level, CameraEntityHost& host);
// 00793DC0 (fastcall ref) and 00796F00 (ECX ref, stack entity, RET 4, only
// when entity == +14h): point = 00793AC0(level 1), entity = 0. Neither calls
// the unregister.
void camera_ref_freeze_00793dc0(cc13cam::CameraRef& ref, cc13cam::CameraRefHost& resolve);
void camera_ref_on_released_00796f00(cc13cam::CameraRef& ref, std::size_t entity,
                                     cc13cam::CameraRefHost& resolve);

// The 4Ch-byte observer class 00793E60 builds (vtable 00D043D8, scalar dtor
// 00790360): +14h entity, +18h..+20h three words, +24h..+38h and +44h/+48h
// zero. Owner not identified (no callers in the index).
struct CameraTargetLink {
    std::size_t entity_14{0};
    std::array<std::uint32_t, 3> words_18{};
    std::array<std::uint32_t, 6> zero_24{};
    std::uint32_t field_44{0};
    std::uint32_t field_48{0};
};
// 00793E60: ECX link, stack (entity, a, b, c), RET 10h. Registers the
// observer unconditionally (even for a null entity).
void camera_target_link_construct_00793e60(CameraTargetLink& link, std::size_t entity,
                                           const std::array<std::uint32_t, 3>& words, ObserverHost& host);

// ---------------------------------------------------------------------------
// The path record (camera +47Ch pointer array; cc13cam::PathCameraRecord adds
// the blend fields). Observer base +0, +14h anchor entity, +18h point, +24h a
// second watched entity, +28h point, +34h/+38h angles, +48h mode.
struct PathRecordEntities {
    std::size_t anchor_14{0};
    Vec3 point_18{};
    std::size_t second_24{0};
    Vec3 point_28{};
    float angle_34{0.0f};
    float angle_38{0.0f};
    std::uint32_t mode_48{0};
};
// 0078E3D0 / 0078E410: ECX record, stack entity, RET 4. Swap +14h (+24h):
// unregister the old entity unless it is the new one or the other slot,
// register the new one unless it is the other slot. (The ledger had these as
// MovCam setters; the two slots are the path record's, see 00796F40.)
void path_record_set_anchor_0078e3d0(PathRecordEntities& r, std::size_t entity, ObserverHost& host);
void path_record_set_second_0078e410(PathRecordEntities& r, std::size_t entity, ObserverHost& host);
// 00796F40: ECX record, stack entity, RET 4: the observer release callback.
// For the anchor: point = 00793F10(level 1); mode 3 also moves +28h through
// the anchor world and becomes 2; mode 5 adds the anchor world's 0042D2E0
// angles (x, y) to +34h/+38h and becomes 4; the anchor is cleared. For the
// second entity: mode 1 copies its position +FCh into +28h and becomes 2; the
// slot is cleared. No unregister.
struct PathRecordReleaseHost : CameraEntityHost {
    virtual Vec3 position_fc(std::size_t entity) = 0;  // after 00414DB0
};
void path_record_on_released_00796f40(PathRecordEntities& r, std::size_t entity, PathRecordReleaseHost& host);

// ---------------------------------------------------------------------------
// Keyframe modifier slots (cc13cam modifier classes; docs/CC13_CAMERA_CONTROLLERS.md).

// 0078E780: ECX modifier, stack owner, RET 4: +4h = owner (the shared base slot).
void modifier_set_owner_0078e780(cc13cam::ModifierKeyframe*& owner_4, cc13cam::ModifierKeyframe* owner) noexcept;
// Type ids (slot +30h): 0078E800 orbit input 1, 0079DC30 goaround 2,
// 0079DCF0 gamecamera 3.
int modifier_type_id_0078e800() noexcept;
int modifier_type_id_0079dc30() noexcept;
int modifier_type_id_0079dcf0() noexcept;
// 0079DD00: fastcall gamecamera modifier: EAX = +8h.
std::size_t gamecamera_camera_0079dd00(const cc13cam::GameCameraModifier& m) noexcept;
// 0078E870: ECX gamecamera modifier, stack dt, RET 4. With a camera (+8h):
// slot +4 of its +170h step sub-object with dt.
struct GameCameraSlotHost {
    virtual ~GameCameraSlotHost() = default;
    virtual void step_slot4(std::size_t camera, float dt) = 0;               // ECX camera+170h
    virtual void pick_camera_0079cbd0(cc13cam::GameCameraModifier& m) = 0;   // builds +8h
    virtual void camera_slot124(std::size_t camera, std::uintptr_t node) = 0;
    virtual void kill_camera_00926d90(std::size_t camera, int flag) = 0;      // ECX camera
    virtual void target_observer_destruct_0042bca0(std::size_t target) = 0;  // the +38h observer
};
void gamecamera_track_tick_0078e870(const cc13cam::GameCameraModifier& m, float dt, GameCameraSlotHost& host);
// 0079CF80: ECX modifier, stack (owner camera, second), RET 8. +Ch = 0;
// 0079CBD0; with a camera and a non-zero owner+3B8h: camera slot +124h(that).
void gamecamera_begin_0079cf80(cc13cam::GameCameraModifier& m, std::uintptr_t owner_node_3b8, GameCameraSlotHost& host);
// 0079DC40: fastcall. Camera +8h -> 00926D90(1), cleared; the two entity
// observer vectors (0079D2D0), the +38h observer (0042BCA0), the two curves
// (+1Ch, +2Ch data) freed; base vtable 00D04414.
void gamecamera_destruct_0079dc40(cc13cam::GameCameraModifier& m, GameCameraSlotHost& host);
// 0079DB90: fastcall. Frees the five curve vectors (+58h, +48h, +38h, +28h,
// +18h); base vtable 00D04414.
void goaround_destruct_0079db90(cc13cam::GoAroundModifier& m) noexcept;

// Deletes an owned object through its scalar deleting destructor (slot 0, 1).
struct OwnedObjectHost {
    virtual ~OwnedObjectHost() = default;
    virtual void scalar_delete(std::uintptr_t object) = 0;
};

// ---------------------------------------------------------------------------
// Small movie-camera helpers.

// 007996C0: cdecl (NativeString by value: length, data). fopen("moviecamera.log",
// "at"); "********\n" once per process (byte 00F871BA); the text (the empty
// buffer 00F871BB when data is null) and "\n" (00CE4390); fclose; the string
// storage goes back to the sized pool.
struct LogFileHost {
    virtual ~LogFileHost() = default;
    virtual std::uintptr_t fopen_bf838e(const char* name, const char* mode) = 0;
    virtual void fwrite_bfb08b(const void* data, std::size_t size, std::uintptr_t file) = 0;
    virtual void fclose_bf8081(std::uintptr_t file) = 0;
};
void movie_camera_log_append_007996c0(const std::string& text, bool& banner_written_f871ba, LogFileHost& host);
// 0079F6D0: ECX camera, the 007900B0 record by value (18h bytes), RET 18h:
// push_back on the interpolator list [+4D8h] (00796930, length check 0079DE40).
void movcam_add_interpolator_0079f6d0(std::vector<cc13cam::MovCamInterpolator>& list_4d8,
                                      const cc13cam::MovCamInterpolator& record);
// 0078E540: ECX keyframe, stack event, RET 4. A different event replaces
// +ECh; the old one is deleted (slot 0 with 1).
void movie_key_set_event_0078e540(std::uintptr_t& event_ec, std::uintptr_t event, OwnedObjectHost& host);
// 00796BD0: ECX observer, stack const observer*, RET 4, EAX observer. Base
// vtable 00CE3C8C, +4h..+Ch zero, +14h = source +14h, +10h = 1, registered
// when the entity is set. Callers then install vtable 00CE3CF0.
struct EntityObserverBase {
    bool active_10{false};
    std::size_t entity_14{0};
};
void entity_observer_copy_base_00796bd0(EntityObserverBase& o, const EntityObserverBase& src, ObserverHost& host);

// ---------------------------------------------------------------------------
// Ocean environment settings records (A0h bytes: two words, a NativeString at
// +8h, four words +10h..+1Ch, then 32 words +20h..+9Fh).
struct OceanEffectSettings {
    std::array<std::uint32_t, 2> head{};
    std::string name_8;
    std::array<std::uint32_t, 4> words_10{};
    std::array<std::uint32_t, 32> table_20{};
};
// 0078D940 (water particles, at object +20h) and 0078D9F0 (beams, at object
// +2Ch): ECX object, stack const record*, RET 4. Member-wise copy; the string
// assignment is skipped only for self-assignment.
void ocean_settings_assign_0078d940(OceanEffectSettings& dst, const OceanEffectSettings& src);
// 0078E510: fastcall environment. Deletes the +E4h member (slot 0 with 1)
// and clears it.
void ocean_environment_delete_member_0078e510(std::uintptr_t& member_e4, OwnedObjectHost& host);

// ---------------------------------------------------------------------------
// MovCam (vtable 00D04B58, constructor 0079F480). Fields by image offset.
struct MovCamAngles {
    Vec3 position_394{};
    float theta_3c4{0.0f};
    float rho_3c8{0.0f};
};

// 007904D0: fastcall camera. 00605070 wraps theta +3C4h and rho +3C8h; theta
// is then clamped to [-pi/2, pi/2] (doubles 00CF48A0 / 00CE3830, stores the
// floats 00CE3CCC / 00CE3C64).
void movcam_wrap_and_clamp_007904d0(MovCamAngles& a) noexcept;

// 00794360: fastcall camera. position = the mover's +A4h..+ACh; 0042D2E0 on
// the mover matrix +74h gives (x, y, z); theta = -0.0 - x, rho = -0.0 - y
// (00D7A208).
void movcam_seed_from_mover_00794360(MovCamAngles& a, const Vec3& mover_position_a4,
                                     const Mat4& mover_matrix_74);

// 00794000: fastcall camera. Clears +3A0h..+3B4h and +3CCh..+3D8h, then
// tail-jumps to the +170h step sub-object's slot +Ch (`step_reset`).
struct MovCamResetHost {
    virtual ~MovCamResetHost() = default;
    virtual void step_reset_slot_c() = 0;
};
void movcam_reset_motion_00794000(cc13cam::MovCamMotion& m, cc13cam::MovCamTurn& t, MovCamResetHost& host);

// 00790460: fastcall camera, copies eight mover words into +370h..+38Ch.
struct MovCamTrackedSnapshot {
    std::array<std::uint32_t, 3> from_224{};  // +224h..+22Ch -> +370h..+378h
    std::uint32_t from_254{0};                // -> +37Ch
    std::uint32_t from_258{0};                // -> +380h
    std::uint32_t from_2f0{0};                // -> +384h
    std::uint32_t from_278{0};                // -> +388h
    std::uint32_t from_2f8{0};                // -> +38Ch
};
void movcam_snapshot_00790460(std::array<std::uint32_t, 8>& out_370, const MovCamTrackedSnapshot& in) noexcept;

// 0078E480: ECX camera, stack zoom, RET 4. +460h = zoom; the camera node
// [+1BCh] gets field of view f(base / zoom) through 00B6FBB0, base = the
// global 00F889B4 (zero-fill, written at run time); +4F4h = +460h.
struct MovCamZoom {
    float zoom_460{1.0f};
    float zoom_copy_4f4{1.0f};
};
float movcam_set_zoom_0078e480(MovCamZoom& z, float zoom, float base_fov_f889b4) noexcept;  // returns the FOV

// 007943D0: ECX camera, stack dt, RET 4. The acceleration +3ACh is clamped to
// [+4D0h]+4Ch, the velocity +3A0h advanced by f(a*dt) and then kept within
// [+4D0h]+34h of the reference velocity +3B8h; the rates +3CCh likewise with +3D4h clamped
// to +50h and the step to +38h. Each length is sqrt of the float-spilled sum
// (y*y + x*x) + z*z, zero unless above 1e-10 (00CE3820); 00793730 / 00793800
// do the rescaling.
void movcam_integrate_007943d0(cc13cam::MovCamMotion& m, cc13cam::MovCamTurn& t,
                               const cc13cam::MovCamLimits& limits, const cc13cam::MovCamTurnLimits& turn,
                               float dt) noexcept;

// 00794F70: ECX camera, stack (d.x, d.y, target), RET 0Ch. r = |rate -
// reference rate| / [+4D0h]+50h; a non-null target arms the follow cone (+3E8h =
// 00414C60(d) on a fresh target else the minimum, +3ECh = 0.9 (00CE3860),
// +3E4h = target); the wanted turn is d - 0.5*r*(rate - reference rate), its
// length times 1000 capped at 2.0 scales +50h, and 00793800's result is added
// to +3D4h.
void movcam_steer_angles_braking_00794f70(cc13cam::MovCamTurn& t, const cc13cam::MovCamTurnLimits& limits,
                                          const std::array<float, 2>& d, std::size_t target,
                                          float length_00414c60_of_d) noexcept;

// 00795300: ECX camera, stack (out matrix, target), RET 8, EAX = out.
// Identity, then row 2 = normalised(target +FCh - camera +394h) (00419440,
// zero when the length is not > 0), row 1 = 00F8758C..94 when |row2.y| <
// 0.9999 (00D04680) else the mover's +84h..+8Ch; then 0085DC80.
struct LookBasisHost {
    virtual ~LookBasisHost() = default;
    virtual Vec3 target_position_fc(std::size_t target) = 0;  // after 00414DB0
    virtual float length_00419440(const Vec3& v) = 0;
    virtual void orthonormalize_0085dc80(Mat4& m) = 0;
    virtual Vec3 global_f8758c() = 0;
};
Mat4 movcam_look_basis_00795300(const Vec3& position_394, const Vec3& mover_up_84, std::size_t target,
                                LookBasisHost& host);

// 00797070: ECX camera, stack (target, aim byte), RET 8. A vehicle (slot +5Ch
// kind 18h) is replaced by its +3D0h; on change the old +390h is unregistered
// (unless it is +38Ch and only when registered), the new one stored; in mode
// 1 an even sub-mode +388h becomes odd; aim: 00794130(target, -1.0), +3E4h =
// target, +4ECh/+4F0h = theta/rho; else +3E4h is cleared when it differs;
// then registered when not already. The observer is camera+10h.
struct MovCamLook {
    std::size_t main_target_38c{0};
    std::size_t look_target_390{0};
    std::uint32_t mode_384{0};
    std::int32_t sub_mode_388{0};
    std::size_t follow_target_3e4{0};
    float theta_3c4{0.0f};
    float rho_3c8{0.0f};
    float aimed_theta_4ec{0.0f};
    float aimed_rho_4f0{0.0f};
    float wanted_theta_418{0.0f};
    float wanted_rho_41c{0.0f};
};
struct MovCamLookHost : ObserverHost {
    virtual bool is_kind_5c(std::size_t entity, int kind) = 0;
    virtual std::size_t vehicle_owner_3d0(std::size_t entity) = 0;
    // 00794130(target, scale): writes theta/rho.
    virtual void aim_at_target_00794130(MovCamLook& camera, std::size_t target, float scale) = 0;
    virtual float length_00414c60(const std::array<float, 2>& v) = 0;
    virtual void set_angular_speed_limit_005cba20(float rate) = 0;  // ECX = [camera+4D0h]
};
void movcam_set_look_target_00797070(MovCamLook& c, const void* observer_10, std::size_t target, bool aim,
                                     MovCamLookHost& host);
// 00797230: ECX camera, stack (dTheta, dRho, instant byte, time), RET 10h.
// 00797070(0, 0); wanted = angles + delta; instant: angles = wanted; else for
// time > 0, the limit block's 005CBA20 gets 00414C60(angles - wanted) / time.
void movcam_turn_by_00797230(MovCamLook& c, const void* observer_10, float d_theta, float d_rho, bool instant,
                             float time, MovCamLookHost& host);
// 00797180 (no Ghidra function; between 00797070's INT3 padding at 00797176
// and 00797230): ECX camera, stack (theta, rho, instant byte, time), RET 10h.
// As 00797230 with absolute wanted angles (MOVSS copies).
void movcam_turn_to_00797180(MovCamLook& c, const void* observer_10, float theta, float rho, bool instant,
                             float time, MovCamLookHost& host);

// 007905B0: fastcall (ECX out, EDX const float* centre), stack (a, b, dist,
// const matrix*), RET 10h, EAX out. p = (dist * axis 00F87598..A0) through
// 00413920(RotX(-0.0 - b), RotY(-0.0 - a)), plus the centre, then through
// the matrix (004142E0 both times).
struct OrbitPointHost {
    virtual ~OrbitPointHost() = default;
    virtual Mat4 rotation_x_00b64640(float angle) = 0;
    virtual Mat4 rotation_y_00b646e0(float angle) = 0;
    virtual Mat4 multiply_00413920(const Mat4& left, const Mat4& right) = 0;
    virtual Vec3 transform_004142e0(const Vec3& point, const Mat4& m) = 0;
    virtual Vec3 axis_f87598() = 0;  // zero-fill, written at run time
};
Vec3 camera_orbit_point_007905b0(const Vec3& centre, float a, float b, float dist, const Mat4& m,
                                 OrbitPointHost& host);

// 007A01F0 (ledger "UpdateDeckPosition"; it is a debug draw): fastcall
// camera. For every element i of the main target's (+38Ch) class vehicle
// camera array (+538h -> +BCh, 5Ch bytes) it draws 007906E0 with ECX = the
// target world +CCh, EDX = element+38h, stack (+48h, +4Ch, +50h, +54h,
// element, &colour), colour = (((i/4)%2)*0.5+0.5, ((i/2)%2)*0.5+0.5,
// (i%2)*0.5+0.5, 1.0) (00D7A280, 00D7A24C).
struct VehicleCameraDrawHost {
    virtual ~VehicleCameraDrawHost() = default;
    virtual std::size_t camera_count() = 0;   // 0 without a target or an array
    virtual void refresh_target_pose_00414db0() = 0;
    virtual void draw_sector_007906e0(std::size_t element, const std::array<float, 4>& colour) = 0;
};
void movcam_debug_draw_vehicle_cameras_007a01f0(VehicleCameraDrawHost& host);

// The vehicle camera array of a unit class (+538h -> +BCh): 5Ch-byte
// elements, NativeString name +0, a local matrix +8h, point +38h, yaw window
// +48h/+4Ch and pitch window +50h/+54h.
struct VehicleCameraElement {
    Mat4 local_8{};
    float yaw_lo_48{0.0f};
    float yaw_hi_4c{0.0f};
    float pitch_lo_50{0.0f};
    float pitch_hi_54{0.0f};
};
// 0079F710: ECX camera, stack (out float[2], entity, index, bool* inside),
// RET 10h. Without the main target +38Ch, its array, or with count < index:
// out = (0, 0). (index == count passes, and 005CD640 grows the array to
// index+1 first.) P = row 3 of 00413920(element +8h, target world +CCh);
// delta = 00794750(entity position +FCh - P, target forward row +ECh);
// centre_yaw = f((lo+hi)*0.5 + (hi <= lo ? pi : 0)) (00D7A280, 00D7A264),
// centre_pitch likewise; inside = 0078E2B0(windows, delta[1], delta[0]);
// out = (|00438B10(delta[0], centre_pitch)|, |00438B10(delta[1], centre_yaw)|).
struct VehicleCameraPoseHost {
    virtual ~VehicleCameraPoseHost() = default;
    virtual Mat4 multiply_00413920(const Mat4& left, const Mat4& right) = 0;
};
std::array<float, 2> movcam_vehicle_camera_pose_0079f710(std::vector<VehicleCameraElement>* cameras, int index,
                                                         const Mat4& target_world_cc, const Vec3& entity_position_fc,
                                                         bool* inside, VehicleCameraPoseHost& host);
// 007A0DD0: (index, entity) -> the `inside` result of 0079F710 into a scratch
// (ECX camera passed through; RET 8; returns AL).
bool movcam_has_vehicle_camera_pose_007a0dd0(std::vector<VehicleCameraElement>* cameras, int index,
                                             const Mat4& target_world_cc, const Vec3& entity_position_fc,
                                             VehicleCameraPoseHost& host);

// 0078FAF0: ECX state {+0 current, +4 target, +8 accumulator}, stack dt, RET
// 4 (the movie camera's +51Ch ground-offset state). x = |target - current|
// (f(target - current), or -0.0 minus it when not > 0); step = f(x*dt*1.4)
// (00D045F0, one rounding); current moves toward target by step without
// passing it; accumulator = f(dt*current + accumulator). No angle wrap
// despite the ledger name.
void movcam_approach_0078faf0(std::array<float, 3>& state, float dt) noexcept;

// ---------------------------------------------------------------------------
// MovCam pose publication. Both builders compose M = (RotX * RotY) * T with
// 00413920 (ECX left, stack out/right, row-major left*right), T the identity
// with the position in row 3; M goes to the mover matrix +74h, the pose bytes
// +C8h/+10Ch are cleared, 0042ED50 runs on every child node (+48h, next +44h)
// and the camera node [+1BCh] slot +34h receives +74h.
struct MovCamPoseHost {
    virtual ~MovCamPoseHost() = default;
    virtual Mat4 rotation_x_00b64640(float angle) = 0;
    virtual Mat4 rotation_y_00b646e0(float angle) = 0;
    virtual Mat4 multiply_00413920(const Mat4& left, const Mat4& right) = 0;
    virtual void invalidate_children_0042ed50() = 0;              // the +48h/+44h walk
    virtual void push_pose_slot34(const Mat4& matrix_74) = 0;     // [[+1BCh]]+34h
    // 0042C4C0(a, b, c, d, e, f) on the mover, RET 18h.
    virtual void set_shake_0042c4c0(const std::array<float, 6>& v) = 0;
    virtual void reset_motion_00794000() = 0;
};
struct MovCamPose {
    Mat4 matrix_74{};
    bool pose_valid_c8{false};
    bool pose_valid_10c{false};
    Mat4 matrix_420{};
    std::uint32_t mode_384{0};
};
// 00799750: fastcall camera. RotY(-0.0 - rho), RotX(-0.0 - theta) (SUBSS from
// 00D7A208), position +394h.
void movcam_apply_pose_00799750(MovCamPose& p, const MovCamAngles& a, MovCamPoseHost& host);
// 007998C0: ECX camera, stack (x, y, z, pitch degrees, yaw degrees), RET 14h.
// Mode +384h = 8; +420h = identity; 0042C4C0(0, 0, 0, 1, 1, 1); RotY(f(k*yaw)),
// RotX(f(pitch*k)) with k = 0.01745329238474369 (double 00D046D8; the pitch
// product comes from the DC-form FMUL at 00799999 and an FXCH); then the
// publication and 00794000.
void movcam_set_pos_rot_zoom_007998c0(MovCamPose& p, const Vec3& position, float pitch_degrees, float yaw_degrees,
                                      MovCamPoseHost& host);

// ---------------------------------------------------------------------------
// MovCam arrival callbacks and the secondary target. The callback names are
// NativeStrings (+0 length) at +49Ch and +4A4h; the argument vectors (a
// LuaVariant vector freed through 006B88E0 and _free) at +4ACh and +4B0h.
struct MovCamCallback {
    std::string name;
    std::uintptr_t args{0};  // host-owned argument vector, 0 = none
};
struct MovCamCallbackHost {
    virtual ~MovCamCallbackHost() = default;
    // 00887E50([[00E188A8]+1A08h], 0, &name, args, 0, -1): the mission Lua
    // host runs `name` with the arguments; ownership of `args` stays with the
    // caller, who frees it right after.
    virtual void call_named_00887e50(const std::string& name, std::uintptr_t args) = 0;
    virtual void free_args(std::uintptr_t args) = 0;  // 006B88E0(0), _free(data), _free(vector)
};
// 0079D960: ECX camera, stack (goal.x, goal.y, goal.z, threshold), RET 10h.
// With a +49Ch name, when 0042B2F0(goal - position +394h) < threshold the name
// is copied, +49Ch emptied (00CE3A0C ""), +4ACh taken and cleared, the copy
// called with those arguments, which are then freed.
// `length_0042b2f0_of_difference` is 0042B2F0 of the float differences.
void movcam_check_arrival_0079d960(MovCamCallback& cb_49c, float threshold, float length_0042b2f0_of_difference,
                                   MovCamCallbackHost& host);
// 0079DA90: ECX camera, stack (target, threshold), RET 8. With a target and a
// +4A4h name, when 00414C60(00794910(target, [+4D0h]+54h)) < threshold, as
// above with +4B0h (freed through 00733D50).
void movcam_check_secondary_arrival_0079da90(MovCamCallback& cb_4a4, std::size_t target, float threshold,
                                             float aim_length, MovCamCallbackHost& host);
// 0079D870: ECX camera, stack (target, aim byte, time, const NativeString*
// name, args), RET 14h. A non-empty name replaces +4A4h and the arguments
// +4B0h (the old vector freed); then 00797070(target, aim); without aim and
// for time > 0, the limit block's 005CBA20 gets 00414C60(00794910(target,
// [+4D0h]+54h)) / time. `aim_length` is that 00414C60 value.
void movcam_set_secondary_target_0079d870(MovCamLook& c, MovCamCallback& cb_4a4, const void* observer_10,
                                          std::size_t target, bool aim, float time, const std::string& name,
                                          std::uintptr_t args, float aim_length, MovCamLookHost& look,
                                          MovCamCallbackHost& callbacks);

// 007901D0: ECX interpolator record (007900B0's layout), stack camera, RET 4.
// Words +4 from, +8 to, +Ch start, +10h duration, +14h clock (all floats).
// Unless start > clock: u = f((clock - start) / duration), value =
// f(from + (to - from) * u) (the product stays on the x87 stack), or `to`
// when u >= 1; then by id: 1 -> +50Ch, 2 -> +510h, 3 -> +504h, 4 -> +508h,
// 5 -> +514h, 6 -> 005CB9F0 (linear limit), 7 -> 005CBA20 (angular limit),
// 8 -> 0078E480 (zoom). Other ids write nothing.
struct MovCamInputs {
    float walk_h_504{0.0f};
    float walk_v_508{0.0f};
    float turn_h_50c{0.0f};
    float turn_v_510{0.0f};
    float zoom_514{0.0f};
};
struct MovCamInterpolatorHost {
    virtual ~MovCamInterpolatorHost() = default;
    virtual void set_linear_speed_limit_005cb9f0(float v) = 0;
    virtual void set_angular_speed_limit_005cba20(float v) = 0;
    virtual void set_zoom_0078e480(float v) = 0;
};
void movcam_apply_interpolator_007901d0(const cc13cam::MovCamInterpolator& r, MovCamInputs& inputs,
                                        MovCamInterpolatorHost& host);

// 0079F400: fastcall (unit, const NativeString* name). For a unit of kind 5
// (slot +5Ch) whose class (+538h) has a vehicle camera array (+BCh: +0 data,
// +4 count, 5Ch-byte elements, NativeString name at +0), the index of the
// first case-insensitive match (00435C40), else -1.
int vehicle_camera_index_of_name_0079f400(bool unit_is_kind5, const std::vector<std::string>* camera_names,
                                          const std::string& name);

// ---------------------------------------------------------------------------
// Movie-camera tracks (84h bytes, camera+414h look-at and camera+498h camera):
// the key list at +68h (+6Ch head node, +70h count), +74h the owning camera,
// +7Ch the current key. Keys carry the start +F0h, the blend +F4h and the id
// short +DAh.
struct MovieKeyTimes {
    float start_f0{0.0f};
    float blend_f4{0.0f};
    std::int16_t id_da{-1};
};
struct MovieTrackKeys {
    std::vector<MovieKeyTimes> keys;  // list order
    int current_7c{-1};               // index into keys, -1 = none
};
// 00797A00: ECX track, stack back byte, RET 4. With a current key: clock =
// its start; with `back`, when the current key is not the first and the clock
// already equals its start, the previous key becomes current first.
void movie_track_sync_time_00797a00(MovieTrackKeys& t, float& clock_3b0, bool back) noexcept;
// 00797AF0: fastcall track. end = blend + start of the current key; next =
// the following key's start, 999999 (00CF87D0) for the last. When end <=
// next: clock = end if clock < end, or when next >= 999998 (00D04688).
// Otherwise the next key becomes current (the last key stays) and clock = its
// start.
void movie_track_advance_00797af0(MovieTrackKeys& t, float& clock_3b0) noexcept;
// 00797F50: ECX track, stack (out start, out end, id), RET 0Ch. Empty: both
// 0. Otherwise start = the start of the key before the match (0 for the
// first), end = the following key's start (999999 when none). id < 0 matches
// the current key, else the id short. Without a match start is the last
// key's start and end 999999.
void movie_track_key_span_00797f50(const MovieTrackKeys& t, int id, float& start, float& end) noexcept;
// 00798E70: ECX camera, stack (out start, out end), RET 8. Zero without the
// selected key +514h; else 00797F50(-1) on the camera track +498h,
// intersected with 00797F50(selected id) when that id is >= 0.
void movie_camera_time_span_00798e70(const MovieTrackKeys& camera_track_498, bool has_selected_514,
                                     std::int16_t selected_id_da, float& start, float& end) noexcept;

// ---------------------------------------------------------------------------
// Keyframe events (cc13cam::KeyframeEvent; base vtable 00D046AC, name string
// +4h/+8h, fired byte +Ch, observer sub-object +10h (vtable 00CE3CF0, its
// +20h byte set), watched entity +24h, sunk depth +28h).

// 00799FA0 (noentity, vtable 00D046F0, 28h bytes), 0079A040 (killed, 00D046FC,
// 28h), 0079A0E0 (sunk, 00D04708, 2Ch; depth -10.0 from 00CE6848): fastcall,
// every word zero but the observer flag; `NoParent` has no constructor here.
cc13cam::KeyframeEvent keyframe_event_construct(cc13cam::KeyframeEventKind kind);
// 007995F0 / 00799620 / 00799650 (one body per class): ECX event, stack
// entity, RET 4. On change: unregister the old entity, store, register the
// new one; the observer is event+10h.
void keyframe_event_set_entity_007995f0(cc13cam::KeyframeEvent& e, std::size_t entity, ObserverHost& host);
// 0079C000: ECX event, stack const NativeString*, RET 4: +4h = the name
// (0041DD40 resize and _memcpy; self-assignment skipped).
void keyframe_event_set_function_0079c000(cc13cam::KeyframeEvent& e, const std::string& name);
// 00799F30 / 00799FD0 / 0079A070 / 0079A120: fastcall event. 0042BCA0 on the
// observer +10h, then 00799580 (base vtable, the name returned to the sized
// pool 00419CC0 / 00BD1510).
struct KeyframeEventDestroyHost {
    virtual ~KeyframeEventDestroyHost() = default;
    virtual void target_observer_destruct_0042bca0(std::size_t entity, const void* observer) = 0;
};
void keyframe_event_destruct_00799fd0(cc13cam::KeyframeEvent& e, KeyframeEventDestroyHost& host);

// ---------------------------------------------------------------------------
// Keyframe linking and insertion (104h-byte keys; camera tracks +414h
// look-at, +498h camera; 007A0770 append, 00798080 pair).
struct MovieKeyLinks {
    MovieKeyLinks* partner_20{nullptr};
    bool linked_25{false};
    bool camera_28{true};
    Vec3 local_34{};
    Vec3 work_40{};
    std::uint32_t field_cc{0};
    std::uint32_t field_d0{0};
    std::int16_t id_da{-1};
    std::uintptr_t modifier_e4{0};
    float start_f0{0.0f};
};
struct MovieKeyHost {
    virtual ~MovieKeyHost() = default;
    virtual void evaluate_position_00795650(MovieKeyLinks& k) = 0;
    virtual std::uintptr_t new_default_modifier_007a06d0() = 0;  // operator new(7Ch), 007A06D0(0); 0 on failure
    virtual void append_007a0770(bool camera_track_498, MovieKeyLinks& k) = 0;
    virtual void pair_00798080(bool camera_track_498, MovieKeyLinks& k) = 0;
    virtual MovieKeyLinks* construct_at_0079c910(std::uintptr_t source) = 0;  // operator new(104h); null on failure
    virtual bool session_present_1fe4() = 0;                                    // [00E188A8]+1FE4h
    virtual void broadcast_new_add_position_0076d6d0(MovieKeyLinks& k) = 0;      // ECX game+1EF0h
};
// 00795BD0: ECX key, stack const float*, RET 4. Only without a modifier
// (+E4h): +40h and +34h = p, then 00795650.
void movie_key_set_fixed_position_00795bd0(MovieKeyLinks& k, const Vec3& p, MovieKeyHost& host);
// 007A0E00: ECX key, stack partner, RET 4. +25h = 1, +CCh = +D0h = 0; a partner
// links both +20h and makes this a look-at key (+28h = 0); without one the key
// is a camera key (+28h = 1) and gets a new 7Ch-byte default modifier at +E4h.
void movie_key_link_partner_007a0e00(MovieKeyLinks& k, MovieKeyLinks* partner, MovieKeyHost& host);
// 007A2D30: ECX camera, stack (key, dt), RET 8. start = f(dt + start), 0 when
// negative; a camera key is appended to +498h and paired in +414h, a look-at
// key the other way round.
void movie_key_shift_start_007a2d30(MovieKeyLinks& k, float dt, MovieKeyHost& host);
// 007A2CD0: ECX camera, stack key, RET 4. Append and pair as 007A2D30, then,
// with a session (game +1FE4h), 0076D6D0 broadcasts the key.
void movie_camera_insert_key_007a2cd0(MovieKeyLinks& k, MovieKeyHost& host);
// 007A2BB0: ECX camera, stack source, RET 4. With a source: a camera key
// (0079C910, 007A0E00 with no partner) appended to +498h, then a look-at key
// linked to it appended to +414h; both get the id word +3C0h, which then
// increments.
void movie_camera_add_key_pair_007a2bb0(std::uintptr_t source, std::uint32_t& next_id_3c0, MovieKeyHost& host);

// ---------------------------------------------------------------------------
// Keyframe network serialization (00791360 write, 007A2370 read). Floats are
// raw bits for the native numeric codec (004295C0 / 004293F0 and the array
// forms 00429790 / 004294F0). Field order and codec arguments (zero flag,
// signed flag, scale, bits):
//   +D9h bool; +CCh (0,0,8.0,16); +34h[3] and +58h[3] (0,1,FLT_MAX,32);
//   +C4h (0,0,64.0,16); +F4h then +F0h (0,1,256.0,16); +D4h (1,0,8.0,10);
//   bools +28h +25h +26h; +2Ch in 4 bits; bools +D8h +27h +30h +DCh;
//   parent: bool, then the parent's 12-bit net id (+174h) when +24h and +1Ch
//   are set; modifier type id (slot +30h) in 4 bits (0 without one), then its
//   slot +24h writer / +28h reader.
struct MovieKeyBits {
    bool terrain_avoid_d9{false};
    std::uint32_t field_cc{0};
    std::array<std::uint32_t, 3> local_34{};
    std::array<std::uint32_t, 3> up_58{};
    std::uint32_t zoom_c4{0};
    std::uint32_t blend_f4{0};
    std::uint32_t start_f0{0};
    std::uint32_t nonlinear_d4{0};
    bool camera_28{false};
    bool linked_25{false};
    bool wanderer_26{false};
    std::uint32_t transform_2c{0};
    bool use_current_d8{false};
    bool has_position_27{false};
    bool then_none_30{false};
    bool flag_dc{false};
    bool parent_attached_24{false};
    std::size_t parent_1c{0};
    std::uintptr_t modifier_e4{0};
};
struct MovieKeyBitsHost {
    virtual ~MovieKeyBitsHost() = default;
    virtual const NativeBitNumericContext& numeric_context() = 0;
    virtual std::uint16_t net_id_174(std::size_t entity) = 0;
    virtual int modifier_type_slot30(std::uintptr_t modifier) = 0;
    virtual void modifier_write_slot24(std::uintptr_t modifier, NativeBitCursor* cursor) = 0;
    // Reader side.
    // 00F89A10/00F89A0C/00F89A54 or 00F89A60/00F89AA8: the entity of a net id.
    virtual std::size_t entity_from_net_id(std::uint16_t id) = 0;
    virtual void register_00694a60(std::size_t entity, const void* observer) = 0;
    virtual void unregister_006952a0(std::size_t entity, const void* observer) = 0;
    virtual bool destroyed_5e(std::size_t entity) = 0;
    virtual std::array<float, 3> position_fc(std::size_t entity) = 0;  // after 00414DB0
    // operator new + 0078E790 (2Ch) / 007A0650 (64h) / 007A06D0(0) (7Ch); 0 on failure.
    virtual std::uintptr_t new_modifier(int type) = 0;
    virtual void modifier_read_slot28(std::uintptr_t modifier, NativeBitCursor* cursor) = 0;
    virtual void set_modifier_00799de0(MovieKeyBits& k, std::uintptr_t modifier) = 0;
};
// 00791360: ECX keyframe, stack cursor, RET 4.
void movie_key_write_bits_00791360(const MovieKeyBits& k, NativeBitCursor* cursor, MovieKeyBitsHost& host);
// 007A2370: ECX keyframe, stack cursor, RET 4; SEH. A parent that is already
// being destroyed (+5Eh) is folded into the local point (+34h += its
// position, float adds) and dropped (unregistered, +24h = 0). Unknown modifier
// types return before the modifier step.
void movie_key_read_bits_007a2370(MovieKeyBits& k, NativeBitCursor* cursor, MovieKeyBitsHost& host);

// ---------------------------------------------------------------------------
// 0079C040: fastcall (ECX LuaObject), AL result; SEH. True when the object is
// a table (00B661B0) whose every entry has a string key (00B660A0, Lua's
// isstring) and a number value (00B66050) with the key "x" (__stricmp, only
// when the key text is non-null), "y" or "z" (00425850, case-insensitive),
// and there are exactly three entries. Iteration is 00B67080/00B67190 until
// the key is unbound (00B66420).
struct LuaEntryView {
    bool key_is_string{false};
    std::string key;   // 00B662B0 text of the key
    bool key_text_null{false};
    bool value_is_number{false};
};
bool lua_is_vector3_table_0079c040(bool is_table, const std::vector<LuaEntryView>& entries);
// 0078FD70: ECX out float[3] (EAX out). Walks the table; a key equal
// (00B660F0) to "x" (00CEB488), "y" or "z" stores f(GetNumber 00B66270) of its
// value into that component; the last match wins. A component with no key is
// whatever the image's stack slot held (uninitialised); here it keeps the
// value `out` had.
struct LuaNumberEntry {
    std::string key;     // empty for a non-string key
    double number{0.0};  // 00B66270
};
void lua_read_xyz_0078fd70(std::array<float, 3>& out, const std::vector<LuaNumberEntry>& entries);

// 0078E8E0: fastcall camera. 0042A910 (ECX camera), then +391h = 0 and the
// field of view +410h = 0.6981317 (00CE7D20).
struct MovieCameraStopHost {
    virtual ~MovieCameraStopHost() = default;
    virtual void inherit_race_and_activate_0042a910() = 0;
};
void movie_camera_stop_0078e8e0(bool& running_391, float& fov_410, MovieCameraStopHost& host);

// ===========================================================================
// Segment 43: session plumbing (the session is MMultiPlayer at game+1EF0h).

// A message built on the stack: base 0075B430(type) (+4h = 1 after the
// caller's store, +10h type), the class vtable written over it, payload bytes
// from +18h.
struct SessionMessageOut {
    std::uint8_t type{0};
    std::uint32_t vtable{0};
    std::array<std::uint8_t, 0x28> body{};  // msg+18h..+3Fh
    void put32(std::size_t offset, std::uint32_t v) noexcept;  // offset from msg+0
    void put16(std::size_t offset, std::uint16_t v) noexcept;
    void put8(std::size_t offset, std::uint8_t v) noexcept;
    std::uint32_t get32(std::size_t offset) const noexcept;
};
struct SessionBroadcastHost {
    virtual ~SessionBroadcastHost() = default;
    // 00784790 on [session+188h] (the server transport), exclude peer 0.
    virtual void broadcast_except_00784790(const SessionMessageOut& m) = 0;
};

// The MT_MOVCAM_* broadcasts: __thiscall(session, ...), each builds one
// message and broadcasts it. `camera_id_174` is the camera's word +174h.
// 0076D390, RET 0Ch (camera, a, b): 2Dh HANDMODETARGETED, vtable 00D031C4;
// +18h word id, +1Ah byte a, +1Bh byte b.
SessionMessageOut session_broadcast_movcam_hand_targeted_0076d390(SessionBroadcastHost& host, std::uint16_t camera_id_174,
                                                                  std::uint8_t a, std::uint8_t b);
// 0076D460, RET 14h (camera, a, b, c, d byte, e): 2Eh REFPOSMODEPOLAR,
// 00D031D8; +18h word id, +1Ch a, +20h b, +24h c, +28h byte d, +2Ch e.
SessionMessageOut session_broadcast_movcam_refpos_polar_0076d460(SessionBroadcastHost& host, std::uint16_t camera_id_174,
                                                                 std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                                                 std::uint8_t d, std::uint32_t e);
// 0076D510, RET 10h (camera, a, b byte, c): 2Fh REFPOSMODERADIUS, 00D031EC;
// +18h word id, +1Ch a, +20h byte b, +24h c.
SessionMessageOut session_broadcast_movcam_refpos_radius_0076d510(SessionBroadcastHost& host, std::uint16_t camera_id_174,
                                                                  std::uint32_t a, std::uint8_t b, std::uint32_t c);
// 0076D5B0, RET 10h (const dword p[3], a, b byte, c): 30h PATHWALK point
// property, 00D03200; +18h..+20h p, +24h a, +28h byte b, +2Ch c.
SessionMessageOut session_broadcast_movcam_path_point_0076d5b0(SessionBroadcastHost& host,
                                                               const std::array<std::uint32_t, 3>& p, std::uint32_t a,
                                                               std::uint8_t b, std::uint32_t c);
// 0076D660, RET 4 (a): 32h SETDEVIATIONFACTOR, 00D03228; +18h a.
SessionMessageOut session_broadcast_movcam_deviation_0076d660(SessionBroadcastHost& host, std::uint32_t a);
// 0076D6D0, RET 4 (key): 33h MOVCAMNEW_ADDPOSITION, 00D02ED4; +18h key, +1Ch
// 0; destroyed through 0075B2D0 after the send; then session +280h += 1.
SessionMessageOut session_broadcast_movcam_new_add_position_0076d6d0(SessionBroadcastHost& host, std::uint32_t key,
                                                                     std::uint32_t& counter_280);
// 0076E750, RET 4 (const dword p[5]): 31h ADDINTERPOLATOR, 00D03214; +18h..+28h
// the five words of the 007900B0 record (id and the four arguments).
SessionMessageOut session_broadcast_movcam_add_interpolator_0076e750(SessionBroadcastHost& host,
                                                                     const std::array<std::uint32_t, 5>& p);

// 0076CE40 / 0076CE70: vtable 00CF17F8 slots 1 and 2 of MT_MOVCAM_BLACKBAR
// (34h, constructor 0075E4D0): ECX message, stack cursor, RET 4. Write: the
// type byte +10h in 8 bits (00428FF0), then the flag +18h (004290B0). Read:
// 00428C70(+10h, 8), 00428D70(+18h).
struct BlackbarMessage {
    std::uint8_t type_10{0x34};
    bool flag_18{false};
};
void blackbar_write_0076ce40(const BlackbarMessage& m, bsp::NativeBitCursor* cursor);
void blackbar_read_0076ce70(BlackbarMessage& m, bsp::NativeBitCursor* cursor);

// 0076E340: fastcall session; vtable slot +154h(slot, 8) for slots 0..8.
struct SessionSlotHost {
    virtual ~SessionSlotHost() = default;
    virtual void set_slot_state_154(int slot, int state) = 0;
};
void session_reset_all_slots_0076e340(SessionSlotHost& host);

// ---------------------------------------------------------------------------
// Chat (MT_CHAT 21h, class 33, vtable 00D036A8) and the single-string class
// 34 (22h, vtable 00D036BC). NativeStrings are std::string here.
struct ChatMessage33 {
    std::string text_18;
    std::uint32_t word_20{0};
    std::uint32_t word_24{0};
    std::string text_28;
    std::string text_30;
    std::uint32_t word_38{0};
    std::uint32_t word_3c{0};
};
// 0076F8C0: ECX msg, stack (text, a, b, text2, text3, c, d), RET 1Ch.
ChatMessage33 chat_message_construct_0076f8c0(const std::string& text, std::uint32_t a, std::uint32_t b,
                                              const std::string& text2, const std::string& text3, std::uint32_t c,
                                              std::uint32_t d);
// 0076F9E0: ECX msg, stack const NativeString*, RET 4: +18h = the string.
std::string message34_construct_0076f9e0(const std::string& text);
// 00772170: ECX session, the seven 0076F8C0 arguments, RET 1Ch. Host (+F4h ==
// 1): 00772020 relays the message. Otherwise 00770AF0 sends it to the server
// and, when the chat screen 00E198B4 exists, 005D0770 shows it locally.
struct ChatSendHost {
    virtual ~ChatSendHost() = default;
    virtual void host_relay_chat_00772020(const ChatMessage33& m) = 0;
    virtual void send_to_server_00770af0(const ChatMessage33& m) = 0;
    virtual bool chat_screen_present_00e198b4() = 0;
    virtual void chat_screen_add_005d0770(const ChatMessage33& m) = 0;
};
void session_send_chat_00772170(std::int32_t mode_f4, const ChatMessage33& args, ChatSendHost& host);

// ---------------------------------------------------------------------------
// Countdown (MT_COUNTDOWN 2Ch, vtable 00D0370C, destructor 00766550).
struct CountdownMessage {
    std::string text_18;
    std::uint32_t word_20{0};
    std::uint32_t word_24{0};
    bool stop_28{false};
};
// 00771240: ECX msg, stack (const NativeString* text, a, b), RET 0Ch; +28h = 0.
CountdownMessage countdown_construct_00771240(const std::string& text, std::uint32_t a, std::uint32_t b);
struct CountdownHost {
    virtual ~CountdownHost() = default;
    virtual void broadcast_except_00784790(const CountdownMessage& m) = 0;  // [session+188h], peer 0
};
// 00772B30: (text, a, b): 00771240, broadcast, destroy.
void session_broadcast_countdown_00772b30(const std::string& text, std::uint32_t a, std::uint32_t b, CountdownHost& host);
// 00770940: empty text, +20h = +24h = 0, +28h = 1, broadcast, destroy.
void session_broadcast_countdown_stop_00770940(CountdownHost& host);

// ---------------------------------------------------------------------------
// Voice (MT_VOICEDATA 29h, vtable 00D03754, +4 = 2; +18h/+1Ch the sender xuid,
// the voice bytes copied by 00774150).
struct VoicePacket {
    std::uint64_t xuid{0};
    std::vector<std::uint8_t> data;
};
struct VoiceMessage {
    std::uint64_t xuid_18{0};
    std::vector<std::uint8_t> data;
};
// 00774DC0: ECX msg, stack const packet*, RET 4.
VoiceMessage voice_message_construct_00774dc0(const VoicePacket& p);
// 00774E30: ECX session, stack const packet*, RET 4. In modes 1 or 2, for
// the player record 0076D7D0(xuid) with a peer (+50h): 007F7490(peer+D50h, 1)
// stamps the voice time; in play (game +5D4h == 0Dh) the message goes to the
// sender's team (+28h of the record) through 00770D40, otherwise to every
// unmuted player through 00770C50; both pass (xuid, peer) to skip.
struct VoiceSendHost {
    virtual ~VoiceSendHost() = default;
    struct Record {
        bool found{false};
        std::uintptr_t peer_50{0};
        std::uint32_t team_28{0};
    };
    virtual Record find_player_by_xuid_0076d7d0(std::uint64_t xuid) = 0;
    virtual void stamp_voice_time_007f7490(std::uintptr_t peer, int flag) = 0;  // ECX = [peer+D50h]
    virtual std::uint32_t game_state_5d4() = 0;                                  // [00E188A8]+5D4h
    virtual void send_to_team_unmuted_00770d40(std::uint32_t team, const VoiceMessage& m, std::uint64_t xuid,
                                               std::uintptr_t peer) = 0;
    virtual void send_to_unmuted_00770c50(const VoiceMessage& m, std::uint64_t xuid, std::uintptr_t peer) = 0;
};
void session_send_local_voice_00774e30(std::int32_t mode_f4, const VoicePacket& p, VoiceSendHost& host);

// ---------------------------------------------------------------------------
// The multiplayer start gate (the game's player records game+18CCh[8], the
// local one at index game+18ECh; +9 the "local" byte, +10h the signed start
// word: < 1 waiting, 1 started, -2 the client's "running" value).

// 0076B400: fastcall record. True when the word +10h is >= 0; otherwise the
// word is incremented and the result is false.
bool start_gate_tick_0076b400(std::int16_t& start_10) noexcept;

// 0076FEC0: fastcall session. Host only (+F4h == 1): for every object of
// [[00E188A8]+19CCh]+34h's list (node +4 next, +8 object) whose +15Ch is
// non-zero, slot +Ch(NativeString(slot +14h()), 0): the object re-applies its
// own name. The class of those objects was not identified.
struct StartGateObjectsHost {
    virtual ~StartGateObjectsHost() = default;
    virtual std::size_t object_count() = 0;
    virtual bool flag_15c(std::size_t i) = 0;
    virtual std::string name_slot14(std::size_t i) = 0;
    virtual void apply_name_slotc(std::size_t i, const std::string& name, std::uint32_t zero) = 0;
};
void session_broadcast_start_gate_state_0076fec0(std::int32_t mode_f4, StartGateObjectsHost& host);

// 007713A0: ECX session, stack dt, RET 4, result in ST0 (the mission frame
// delta filter of docs/GAME_FRAME_CONTROL; cc9's host returns dt unchanged).
// Mode 0: dt. No game or no local record: 0. Local record started (+10h >=
// 1): dt. Otherwise the host, once 0076B400 passes and fewer than two non-
// local records are waiting and the optional delay ([00E19698]: +5 armed,
// +10h start, clock 01090AB0, 3.0 s from 00D7A2B0) has passed, runs 0076FEC0,
// 00874640(0), 008EB710 when [00F88C30] is set, broadcasts MT_LETSPLAY
// (0075D3F0), flushes the peer buffers (007848F0) and marks the local record
// started; a client, under the same tick and delay, sends MT_LETSPLAY to the
// server (00770AF0). Then dt when the local word is -2, else 0.
struct StartGateRecord {
    bool local_9{false};
    std::int16_t start_10{0};
};
struct StartGateHost {
    virtual ~StartGateHost() = default;
    virtual bool delay_passed_00e19698() = 0;  // no delay object, not armed, or >= 3.0 s elapsed
    virtual void broadcast_start_gate_state_0076fec0() = 0;
    virtual void mission_clock_reset_00874640(int value) = 0;
    virtual bool powerups_present_00f88c30() = 0;
    virtual void powerups_broadcast_008eb710() = 0;
    virtual void broadcast_lets_play_0075d3f0() = 0;   // build 11h, 00784790 on [session+188h]
    virtual void flush_peer_buffers_007848f0() = 0;
    virtual void send_lets_play_to_server_00770af0() = 0;
};
float session_update_start_gate_007713a0(std::int32_t mode_f4, bool has_game, std::array<StartGateRecord, 8>* records,
                                         int local_index_18ec, float dt, StartGateHost& host);

// ---------------------------------------------------------------------------
// Session sends. Player records: game+18CCh[8], +8 remote byte, +9 local
// byte, +28h team, +50h peer.
struct SessionPlayerRecord {
    bool remote_8{false};
    bool local_9{false};
    std::uint32_t team_28{0};
    std::uintptr_t peer_50{0};
};
struct SessionSendHost {
    virtual ~SessionSendHost() = default;
    virtual void broadcast_except_00784790(const SessionMessageOut& m) = 0;     // [session+188h]
    virtual void flush_peer_buffers_007848f0() = 0;
    virtual void send_to_peer_00770b50(std::uintptr_t peer, std::uintptr_t message) = 0;
    virtual void kick_peer_00784b10(std::uintptr_t peer, int flag) = 0;          // ECX [session+188h]
    virtual void apply_counter_record_0052cf60(std::uintptr_t message) = 0;
    virtual void broadcast_message_00784830(std::uintptr_t message) = 0;
    virtual void scalar_delete(std::uintptr_t message) = 0;
};
// 0076C190: fastcall session. Host (+F4h == 1): MT_LOADINGINFO (0Eh, class 14,
// vtable 00D0300C, +18h = 0) broadcast, then 007848F0; every mode: +4 = 0,
// +27Ch = 1.
void session_loading_begin_0076c190(std::int32_t mode_f4, bool& flag_4, bool& loading_27c, SessionSendHost& host);
// 0076C230: as above with +18h = 1, no flush; +27Ch = 0.
void session_loading_end_0076c230(std::int32_t mode_f4, bool& loading_27c, SessionSendHost& host);
// 0076C790: ECX session, stack record, RET 4: 00784B10(record peer +50h, 1).
void session_kick_player_peer_0076c790(const SessionPlayerRecord& record, SessionSendHost& host);
// 00772330: ECX session, stack message, RET 4. In modes 0 and 1: a message
// addressed (+1Ch) to the local slot (game+18ECh) goes to 0052CF60, any other
// to that record's peer when it has one (00770B50).
void session_send_to_player_slot_00772330(std::int32_t mode_f4, std::int32_t message_slot_1c, int local_slot_18ec,
                                          const std::array<SessionPlayerRecord, 8>& records, std::uintptr_t message,
                                          SessionSendHost& host);
// 00772AF0: cdecl (message, record). Unless a client (game +1FE4h == 2): to the
// record's peer when it has one and is remote (+8).
void session_send_to_remote_player_00772af0(std::int32_t game_session_1fe4, const SessionPlayerRecord& record,
                                            std::uintptr_t message, SessionSendHost& host);
// 0076C500: fastcall session. Every message of the outbound array +25Ch
// (count +260h) goes out through 00784830 and is deleted; count = 0.
void session_broadcast_queued_0076c500(std::vector<std::uintptr_t>& outbound_25c, SessionSendHost& host);
// 0076D280: ECX session, stack level, RET 4: MT_MUSICCONTROL_SETLEVEL (2Ah,
// vtable 00D0319C, +18h level), broadcast.
SessionMessageOut session_broadcast_music_level_0076d280(std::uint32_t level, SessionSendHost& host);
// 00770BD0: ECX session, stack (team, message, except peer), RET 0Ch. Host
// only: every record of the first [[game+5FCh]+988h] with that team, a peer
// other than `except`, and not local.
void session_host_relay_to_team_00770bd0(std::int32_t mode_f4, const std::array<SessionPlayerRecord, 8>& records,
                                         int record_count_988, std::uint32_t team, std::uintptr_t message,
                                         std::uintptr_t except_peer, SessionSendHost& host);

// ---------------------------------------------------------------------------
// Peer rows (band0076_cc13::PeerRows at session+8h) and the MNetwork client.

// 00786010: ECX rows, stack index, RET 4: rows[index].quality in ST0.
float peer_rows_quality_00786010(const band0076_cc13::PeerRows& rows, int index) noexcept;
// 007865D0: fastcall row. With a peer: the active transport (game +2078h,
// else +207Ch) gives latency = slot +Ch(peer) (ST0, float-spilled) and
// pending = slot +10h(peer) (int -> float); a = 00419010(8, 0, 16, 1,
// pending), b = 00419010(0.15, 0, 0.6, 1, latency) (00CE3918, 00CE6454,
// 00CE7818, 00CE3D30); 007862C0(row+4, b, a) smooths +4..+Ch.
struct PeerQualityHost {
    virtual ~PeerQualityHost() = default;
    virtual float transport_latency_c(std::uint32_t peer) = 0;
    virtual std::int32_t transport_pending_10(std::uint32_t peer) = 0;
    virtual float interpolate_clamped_00419010(float x0, float y0, float x1, float y1, float x) = 0;
    virtual void smoothed_triplet_update_007862c0(band0076_cc13::PeerRow& row, float first, float second) = 0;
};
void peer_row_update_quality_007865d0(band0076_cc13::PeerRow& row, PeerQualityHost& host);

// 007866B0: ECX rows (session+8h), stack dt, RET 4; SEH. Only when +E0h is set.
// dt is capped at 0.1 (00D7A3A0 / 00D7A2F0). Each row whose peer has a record
// (+D50h) with a slot (+1Ch) other than the row index moves there: into an
// empty row (the old row cleared: peer, +4, +8, +10h, +14h, +18h zero, quality
// 1.0), or by swapping with an occupied one and rescanning from row 0. Then
// 00786150(dt) on all eight rows; +E4h = f(+E4h - dt) and, once not above
// zero, +E4h = 0.2 (00D042F8), +E8h = f(+E8h + 0.2) (double 00CE3D10),
// 007865D0 on rows 1..7, and when +E8h reaches 0.5 (00D042FC) it is zeroed
// and, with a game, MT_CONNSTATE (00786390) goes to 0076A9F0(game+1EF0h, msg, 0).
struct PeerRowsUpdateHost {
    virtual ~PeerRowsUpdateHost() = default;
    // [peer+D50h] != 0 and its +1Ch slot; -1 when the peer has no record.
    virtual int peer_record_slot(std::uint32_t peer) = 0;
    virtual void send_probes_00786150(band0076_cc13::PeerRow& row, float dt) = 0;
    virtual void update_quality_007865d0(band0076_cc13::PeerRow& row) = 0;
    virtual bool game_present_00e188a8() = 0;
    virtual void send_connstate_00786390(const band0076_cc13::PeerRows& rows) = 0;  // then 0076A9F0
};
void peer_rows_update_007866b0(band0076_cc13::PeerRows& rows, float dt, PeerRowsUpdateHost& host);

// MNetwork client virtuals: +28h open, +2Ch begin LAN search (a, b), +30h end
// LAN search, +40h connect to the server, +48h disconnect; +9Ch the state.
struct MNetworkClientHost {
    virtual ~MNetworkClientHost() = default;
    virtual void open_slot28() = 0;
    virtual void begin_lan_search_slot2c(std::uint32_t a, std::uint32_t b) = 0;
    virtual void end_lan_search_slot30() = 0;
    virtual void connect_slot40() = 0;
    virtual void disconnect_slot48() = 0;
    virtual void clear_hosts_00785170() = 0;
};
// 00782880 / 007828C0: fastcall client; slot +28h / +48h, then +9Ch = 2.
void mnetwork_client_open_00782880(std::uint32_t& state_9c, MNetworkClientHost& host);
void mnetwork_client_disconnect_007828c0(std::uint32_t& state_9c, MNetworkClientHost& host);
// 007828A0 / 007828B0: tail jumps to slots +30h / +40h.
void mnetwork_client_end_lan_search_007828a0(MNetworkClientHost& host);
void mnetwork_client_connect_007828b0(MNetworkClientHost& host);
// 00785330: ECX client, stack (a, b), RET 8: 00785170, then slot +2Ch(a, b).
void mnetwork_client_clear_and_search_00785330(std::uint32_t a, std::uint32_t b, MNetworkClientHost& host);

// 0076B320 / 0076B350: fastcall transport. EnterCriticalSection (import
// 00CE2218) on the lock at +34h / +24h, lock depth +18h += 1; EAX = the
// guarded list at transport +28h / +18h.
struct TransportLock {
    std::uintptr_t critical_section{0};
    std::int32_t depth_18{0};
};
struct CriticalSectionHost {
    virtual ~CriticalSectionHost() = default;
    virtual void enter_critical_section(std::uintptr_t cs) = 0;
};
void mnetwork_lock_incoming_0076b320(TransportLock& lock_34, CriticalSectionHost& host);
void mnetwork_lock_removals_0076b350(TransportLock& lock_24, CriticalSectionHost& host);

// ===========================================================================
// Packet cc13-cam3 (docs/CC13_CAM_3.md).

// ---------------------------------------------------------------------------
// The MovCam's per-frame pose, 0079C200: slot +4h of the +170h sub-object's
// vtable 00D04B1C (.rdata 00D04B20; slot +8h is the fixed step 007A91A0, slot
// +Ch the snapshot 00790460). ECX = camera+170h, stack dt, RET 4; every call
// it makes on the camera passes ECX = sub-object - 170h. Offsets below are
// from the camera (the listing's are 170h lower).
//
// Input latch (0079C211..0079C2F7): the edge test 004C43C0 (ECX = [00E188A8])
// for actions 78h..7Bh is ORed into bytes +51Ch..+51Fh and for 82h, 83h, 81h
// into +534h, +535h, +536h; the floats +520h..+530h take +24h of the input
// records of actions 7Ch..80h ([[004BEC00()]+4] + action*30h). The bytes stay
// set until the step consumes them. The action meanings are not identified.
struct MovCamFrameInput {
    std::array<bool, 4> edges_51c{};   // actions 78h, 79h, 7Ah, 7Bh
    std::array<float, 5> axes_520{};   // actions 7Ch..80h, record +24h
    std::array<bool, 3> edges_534{};   // actions 82h, 83h, 81h
};
struct MovCamFrameInputHost {
    virtual ~MovCamFrameInputHost() = default;
    virtual bool was_pressed_004c43c0(std::int32_t action) = 0;
    virtual float record_value_24(std::int32_t action) = 0;
};
void movcam_latch_input_0079c200(MovCamFrameInput& in, MovCamFrameInputHost& host);

// Between fixed steps the frame extrapolates from the snapshot 00790460 took
// (+4E0h position, +4ECh theta, +4F0h rho, +4F4h zoom). The frame shows that
// +3A0h and +3CCh/+3D0h are rates per second (position = snapshot + rate*dt,
// and mode 5 writes them back as (now - snapshot)/dt); `MovCamMotion` and
// `MovCamTurn` in cc13_camera_controllers.hpp now name them so (cc13-cam3b).
struct MovCamFrame {
    std::uint32_t mode_384{0};
    std::size_t entity_38c{0};         // mode 2 deck frame, mode 7 eye frame
    std::size_t entity_390{0};         // mode 7 aim frame
    Vec3 position_394{};
    Vec3 velocity_3a0{};
    float theta_3c4{0.0f};
    float rho_3c8{0.0f};
    std::array<float, 2> angle_rate_3cc{};
    std::size_t follow_target_3e4{0};  // nonzero: 00795180 moves the angles
    Mat4 matrix_420{};                 // mode 2 deck-local transform
    float zoom_460{1.0f};
    float path_t_468{0.0f};
    float path_end_46c{0.0f};
    bool has_knot_path_474{false};
    Vec3 aim_point_4b8{};              // mode 7, in entity_390's frame when set
    Vec3 eye_point_4c4{};              // mode 7, in entity_38c's frame when set
    float top_speed_4d0_34{0.0f};      // [+4D0h]+34h, the path parameter's rate
    Vec3 snapshot_position_4e0{};
    float snapshot_theta_4ec{0.0f};
    float snapshot_rho_4f0{0.0f};
    float snapshot_zoom_4f4{1.0f};
    float snapshot_path_t_4fc{0.0f};   // 00790460's copy of +468h
    float zoom_rate_514{0.0f};
    Mat4 matrix_74{};                  // the mover matrix (mode 2 writes it)
    bool pose_valid_c8{false};
    bool pose_valid_10c{false};
};
struct MovCamFrameHost {
    virtual ~MovCamFrameHost() = default;
    virtual void follow_00795180(MovCamFrame& c, float dt) = 0;
    virtual void wrap_and_clamp_007904d0(MovCamFrame& c) = 0;
    virtual void keep_above_ground_00790540(MovCamFrame& c) = 0;
    virtual void apply_pose_00799750(MovCamFrame& c) = 0;
    virtual void path_update_007972f0(MovCamFrame& c) = 0;
    virtual void aim_at_point_00794070(MovCamFrame& c, const Vec3& point) = 0;
    // 007B04C0 on the path [+464h] (ECX), stack (t, &position, &direction, 0).
    virtual void sample_world_007b04c0(float t, Vec3& position, Vec3& direction) = 0;
    // 00B6FBB0 on the camera node [+1BCh], stack fov.
    virtual void set_field_of_view_00b6fbb0(float fov) = 0;
    virtual Mat4 world_matrix_0042d7e0(std::size_t entity) = 0;
    virtual Vec3 transform_point_00414d10(const Mat4& m, const Vec3& p) = 0;
    virtual Mat4 rotation_y_negated_0042b490(float angle) = 0;
    virtual Mat4 rotation_x_negated_0042b450(float angle) = 0;
    virtual Mat4 multiply_00413920(const Mat4& left, const Mat4& right) = 0;
    virtual void invalidate_children_0042ed50() = 0;   // the +48h/+44h walk
    virtual void apply_shake_and_publish_004329d0(float dt) = 0;
};
// After the latch, by mode +384h:
// - 0, 1, 3, 4, 6: position = snapshot + velocity*dt; angles = snapshot +
//   rate*dt unless a follow target is set (then 00795180(dt)); 007904D0.
// - 2 (deck): zoom +460h = ((dt*+514h) + 1.0) * +4F4h (double 00D7A210) clamped to
//   [1.0, 21.0] (00D7A24C, 00D04B18); 00B6FBB0(00F889B4 / zoom); the angles
//   as above, 007904D0; +74h = RotX'(theta) * RotY'(rho) * +420h * world(+38Ch)
//   (0042B450 / 0042B490 negate internally), and it skips 00790540 / 00799750.
// - 5 (path): t +468h = the snapshot +4FCh moved toward +46Ch at top
//   speed*dt (float [[+4D0h]+34h]) but stopping 0.01
//   (double 00D7A358) short (00415510 / 00415550); +468h is left alone when
//   the snapshot equals +46Ch. Then
//   007972F0 with a knot path, else 007B04C0(t) into +394h and 00794070 at
//   position + direction. With dt > 1e-4 (double 00D7A268) the rates are
//   rewritten as (now - snapshot) / dt.
// - 7: +394h = +4C4h (through world(+38Ch) when set); aim at +4B8h (through
//   world(+390h) when set). No 007904D0.
// - other (8): nothing but the tail.
// Modes 0..7 except 2 then run 00790540 and 00799750. Tail for every mode:
// clear the pose bytes +C8h/+10Ch, 0042ED50 on every child, 004329D0(dt).
void movcam_frame_0079c200(MovCamFrame& c, MovCamFrameInput& in, float dt, float base_fov_f889b4,
                           MovCamFrameInputHost& input, MovCamFrameHost& host);

// ---------------------------------------------------------------------------
// MovCam lifetime. 0079F480 (ECX camera, RET; built by 005CC000): the base
// 00432750, vtables 00D04B58 (+0), 00D04B3C (+10h), 00D04B34 (+24h),
// 00D04B1C (+170h); the CameraRef at +3F0h (vtable 00D04660) zeroed; the
// record array +47Ch..+484h and both callback NativeStrings (+49Ch, +4A4h)
// empty; the interpolator list +4D4h gets its head from 00796770 (+4D8h,
// size +4DCh = 0); mode, both targets, position, motion, angles and rates zero
// and the snapshot equal to them; +3E4h, +464h, +474h = 0; +4D0h =
// [[00E198C4]+98h], a limits block the camera shares, which the four resets
// 005CB970, 005CB9C0, 005CAFB0, 005CAFC0 (ECX = that block) then restore;
// bytes +538h/+4B4h, words +4ACh/+4B0h and floats +494h/+498h/+53Ch zero.
struct MovCamOwned {
    std::array<std::uint32_t, 4> vtables{};          // +0, +10h, +24h, +170h
    std::uint32_t camera_ref_vtable_3f0{0};
    cc13cam::CameraRef camera_ref_3f0{};             // its +14h..+24h at +404h..+414h
    std::vector<std::uintptr_t> records_47c;         // path records, virtual-deleted by 0079C830
    std::string callback_49c;
    std::string callback_4a4;
    std::uintptr_t args_4ac{0};                      // LuaVariant vectors, 0 = none
    std::uintptr_t args_4b0{0};
    bool flag_4b4{false};
    std::uintptr_t limits_4d0{0};
    std::uintptr_t list_head_4d8{0};
    std::vector<std::uintptr_t> interpolators_4d4;   // node payloads (+8h of each node)
    float field_494{0.0f};
    float field_498{0.0f};
    bool flag_538{false};
    float field_53c{0.0f};
};
struct MovCamConstructHost {
    virtual ~MovCamConstructHost() = default;
    virtual void construct_base_00432750() = 0;
    virtual std::uintptr_t buy_list_head_00796770() = 0;
    virtual std::uintptr_t limits_block_e198c4_98() = 0;
    virtual void reset_speed_limits_005cb970(std::uintptr_t limits) = 0;
    virtual void reset_accel_time_005cb9c0(std::uintptr_t limits) = 0;
    virtual void reset_deviation_factor_005cafb0(std::uintptr_t limits) = 0;
    virtual void reset_overview_height_005cafc0(std::uintptr_t limits) = 0;
};
void movcam_construct_0079f480(MovCamFrame& frame, MovCamOwned& owned, MovCamConstructHost& host);

// 0079D640 (fastcall camera; the deleting destructor 0079E2F0 calls it and
// then _free when bit 0 of its argument is set): vtables back; _free each
// interpolator payload; 0079C830 (knot path cleared, records virtual-deleted);
// the argument vectors +4ACh then +4B0h (006B88E0 resize 0, _free the storage,
// _free the vector); the list nodes and head (_free); the strings +4A4h then
// +49Ch back to the sized pool (00419CC0 / 00BD1510, length+1); _free the
// record storage; 00695870 on the CameraRef +3F0h; the base 0042A830.
struct MovCamDestroyHost {
    virtual ~MovCamDestroyHost() = default;
    virtual void free_crt(std::uintptr_t p) = 0;
    virtual void clear_path_records_0079c830() = 0;
    virtual void free_args_vector(std::uintptr_t vec) = 0;   // 006B88E0(0), _free storage, _free vec
    virtual void free_list_nodes_and_head(std::uintptr_t head) = 0;
    virtual void destroy_camera_ref_00695870() = 0;           // ECX = camera+3F0h
    virtual void destruct_base_0042a830() = 0;
};
void movcam_destruct_0079d640(MovCamOwned& owned, MovCamDestroyHost& host);

// ---------------------------------------------------------------------------
// Movie-camera keyframes (104h bytes, vtable 00D046E0) and tracks (84h, 00D04714).

// 0079C910: ECX key, stack (x, y, z, entity), RET 10h. The base words +Ch..+14h
// and +1Ch zero, byte +18h = 1, the curve vtable 00CE75CC at +8h, the name
// +FCh/+100h empty, id +DAh = -1; +34h, +40h and +4Ch all = (x, y, z). The
// keyframe SetEntity 00799D70 returns 0 when the entity is being destroyed
// (+5Eh); then, for a non-null entity, its position +FCh (after 00414DB0 when
// byte +C8h is clear) is added to all three vectors and the key keeps no
// entity. +24h = (entity kept); zooms +C4h/+C8h and smoothing +CCh/+D0h =
// 1.0 (00D7A24C); +D4h = 0.5 (00CE3800); up vectors +58h, +64h, +70h = (0, 1,
// 0); +2Ch = 1; bytes +26h, +27h, +28h = 1; everything else listed zero. It
// does not write +C0h (00799B00 stores -1.0 there).
struct MovieKeyframeInit {
    std::uint32_t field_4{0};
    std::size_t entity_1c{0};
    bool flag_18{false};
    bool parent_attached_24{false};
    bool linked_25{false};
    bool wanderer_26{false};
    bool has_position_27{false};
    bool camera_28{false};
    std::int32_t transform_2c{0};
    bool then_none_30{false};
    Vec3 local_34{};
    Vec3 work_40{};
    Vec3 eval_4c{};
    Vec3 up_58{};
    Vec3 work_up_64{};
    Vec3 eval_up_70{};
    bool flag_bc{false};
    float fov_divisor_c0{0.0f};
    float zoom_c4{0.0f};
    float zoom_c8{0.0f};
    float smooth_cc{0.0f};
    float smooth_d0{0.0f};
    float nonlinear_d4{0.0f};
    bool use_current_d8{false};
    bool terrain_avoid_d9{false};
    std::int16_t id_da{0};
    bool flag_dc{false};
    std::uintptr_t modifier_e4{0};
    bool flag_e8{false};
    std::uintptr_t event_ec{0};
    float start_f0{0.0f};
    float blend_f4{0.0f};
    std::int32_t state_f8{0};
    std::uintptr_t partner_20{0};
    std::string name_fc;
};
struct MovieKeyframeInitHost {
    virtual ~MovieKeyframeInitHost() = default;
    virtual bool set_entity_00799d70(MovieKeyframeInit& k, std::size_t entity) = 0;
    virtual Vec3 entity_position_fc(std::size_t entity) = 0;  // 00414DB0 first when +C8h is clear
};
void movie_keyframe_construct_at_0079c910(MovieKeyframeInit& k, const Vec3& p, std::size_t entity,
                                          MovieKeyframeInitHost& host);
// 00799B00: fastcall key, RET (the default constructor; cc9's hud_movie_camera.hpp
// models its result as MovieKeyframe). As 0079C910 with no position and no
// entity (all three vectors zero, +24h = 0), plus +C0h = -1.0 (00D7A260), byte
// +E8h = 0 and +4h = 0, which 0079C910 leaves unwritten. Its test of +1Ch right
// after storing 0 there (unregister when set) can never fire.
void movie_keyframe_construct_00799b00(MovieKeyframeInit& k);

// 00799CA0: fastcall key. Vtable back; the modifier +E4h virtual-deleted
// (slot 0, 1) and cleared; a partner +20h whose +20h points back has it
// cleared; the event +ECh virtual-deleted and cleared; the entity +1Ch
// unregistered (006952A0) and cleared; the name returned to the sized pool;
// 004B7EF0 (the observer-link base).
// 00799EA0: fastcall key. Entity unregistered and cleared; +24h = 0, +D8h = 1,
// +F0h = +F4h = 0.
struct MovieKeyframeLifetimeHost {
    virtual ~MovieKeyframeLifetimeHost() = default;
    virtual void delete_virtual(std::uintptr_t object) = 0;      // slot 0 with 1
    virtual void unregister_006952a0(std::size_t entity, const void* key) = 0;
    virtual MovieKeyframeInit* partner(std::uintptr_t partner_20) = 0;
    virtual void destroy_base_004b7ef0() = 0;
};
void movie_keyframe_destruct_00799ca0(MovieKeyframeInit& k, std::uintptr_t self, MovieKeyframeLifetimeHost& host);
void movie_keyframe_reset_00799ea0(MovieKeyframeInit& k, MovieKeyframeLifetimeHost& host);

// 0079CFC0: ECX track, RET. Vtable 00D04714; list head from 007967E0 at +6Ch,
// size +70h, the camera back-pointer +74h, +78h, +80h zero; byte +64h = 0.
// 0079A190: fastcall track. Vtable back; +74h = 0; while the list is not
// empty, the front key is virtual-deleted, its node unlinked and _freed and
// the size decremented; then 00796800 frees the head.
struct MovieTrackList {
    std::uintptr_t head_6c{0};
    std::vector<std::uintptr_t> keys;   // node payloads, front first
    std::uintptr_t camera_74{0};
    std::uint32_t field_78{0};
    std::uint32_t field_80{0};
    bool flag_64{false};
};
struct MovieTrackHost {
    virtual ~MovieTrackHost() = default;
    virtual std::uintptr_t buy_list_head_007967e0() = 0;
    virtual void delete_virtual(std::uintptr_t key) = 0;
    virtual void tidy_list_00796800(std::uintptr_t head) = 0;
};
void movie_track_construct_0079cfc0(MovieTrackList& t, MovieTrackHost& host);
void movie_track_destruct_0079a190(MovieTrackList& t, MovieTrackHost& host);

// 0079A260: fastcall movie camera (the 00D04750 class of 0079D020). Vtables
// 00D04750/00D04738/00D04730/00D04718 back; then it reseeds RNG stream 1, not
// stream 0: [01090AB0] slot +20h returns (counter, frequency) as two int64;
// ms = f(f(counter / frequency) * 1000.0) (double 00CE47A0); 00BD2FD0 with
// ECX = 1 and EDX = the truncated (FISTP under RC = 11b) low dword. Then the
// tracks +498h and +414h (0079A190) and the base 0042A830.
struct HudMovieCameraDestroyHost {
    virtual ~HudMovieCameraDestroyHost() = default;
    virtual std::array<std::int64_t, 2> clock_counter_frequency_slot20() = 0;
    virtual void seed_stream_00bd2fd0(std::int32_t stream, std::uint32_t seed) = 0;
    virtual void destruct_track_0079a190(bool camera_track_498) = 0;
    virtual void destruct_base_0042a830() = 0;
};
void hud_movie_camera_destruct_0079a260(HudMovieCameraDestroyHost& host);

// ---------------------------------------------------------------------------
// 0079F9E0, the vehicle-camera window editor (ledger BSP_MovCam_DumpPositions;
// fastcall camera, called once by the step 007A91A0). Inputs walk_h +504h and
// walk_v +508h are classified against -0.5 (00CE69D0) and 0.5 (00CE3800):
// horizontal below -> 1 (MaxHorzAngle), above -> 0 (MinHorzAngle); vertical
// below -> 0 (MinVertAngle), above -> 1 (MaxVertAngle); otherwise -1. With
// both active it returns at once. It builds
//   name + "; mov.cam.pos.: [" + (index +388h)+1 + "]\n\t\t\t\t"
// (name = [[target +38Ch]+538h]+54h) and, for the horizontal code when set,
// else the vertical one, appends "[\"" + key + "\"] = " + "%f"(value) + ","
// (value: rho +3C8h for horizontal, theta +3C4h for vertical, radians). The
// text goes to 007996C0 (moviecamera.log) even when both are idle. Then the
// class camera array (+538h -> +BCh) is grown to index+1 by 005CD640 when
// index >= count, and the value is stored in element `index`: +4Ch (horz 1),
// +48h (horz 0), +54h (vert 1), +50h (vert 0); 005CBA50 on [+4D0h] then shows
// message 1, 3, 0 or 2 respectively. Both idle: no store, no message.
struct VehicleCameraEditHost {
    virtual ~VehicleCameraEditHost() = default;
    virtual std::string class_name_54() = 0;                       // [[+38Ch]+538h]+54h
    virtual std::vector<VehicleCameraElement>& class_cameras_bc() = 0;
    virtual void grow_cameras_005cd640(std::size_t count) = 0;     // to `count` elements
    virtual void append_log_007996c0(const std::string& text) = 0;
    virtual void show_message_005cba50(std::int32_t kind) = 0;     // ECX = [+4D0h]
};
// Returns the text logged, empty when it returned early.
std::string movcam_edit_vehicle_camera_window_0079f9e0(const MovCamInputs& in, std::int32_t index_388,
                                                       float theta_3c4, float rho_3c8, VehicleCameraEditHost& host);

// ---------------------------------------------------------------------------
// 007906E0 (ledger BSP_Camera_DebugDrawOrbitSector): fastcall (ECX const
// matrix*, EDX const float* point), stack (yaw_lo, yaw_hi, pitch_lo, pitch_hi,
// element, const float colour[4]*), RET 18h; 007A01F0 passes a vehicle camera
// element's windows (+48h..+54h), point +38h and the target world. Every
// point is 007905B0(point, yaw, pitch, 4.0 (00CE3D34), matrix). It writes the
// element (its NativeString name at +0) at the transformed point (004142E0,
// 00861260), draws lines (0085F990, colour at both ends) from there to the
// four corners, then, with yaw step s = f((hi - lo) * 0.125) (00D04380, the
// DC-form FMUL at 0079099B) plus pi/4 (00CEDCD0, i.e. 2*pi/8) when hi < lo,
// and the pitch step likewise, eight segments along each of the four window
// edges, and a cross at the window centre (yaw lo + 4s with pitch lo + 3t ..
// lo + 5t, then pitch lo + 4t with yaw lo + 3s .. lo + 5s; doubles 00D7A328,
// 00D7A2B0, 00D7A370). Angles are f(k*step + lo), one rounding each.
struct SectorDebugDrawHost {
    virtual ~SectorDebugDrawHost() = default;
    virtual void text_at_world_point_00861260(const Vec3& p, std::size_t element) = 0;
    virtual void line_0085f990(const Vec3& p0, const Vec3& p1, const std::array<float, 4>& c0,
                               const std::array<float, 4>& c1) = 0;
};
void camera_debug_draw_sector_007906e0(const Mat4& m, const Vec3& point, float yaw_lo, float yaw_hi, float pitch_lo,
                                       float pitch_hi, std::size_t element, const std::array<float, 4>& colour,
                                       OrbitPointHost& orbit, SectorDebugDrawHost& draw);

// ---------------------------------------------------------------------------
// Session mode changes, 0076FAD0 (BSP_Session_SetMode): ECX session, stack
// mode, RET 4. Read from the listing (the transport deletes are slot-0 calls
// with 1, and the singleton call at 0076FB53 takes no argument: the PUSH EBP
// before it is the prologue's save, popped at 0076FE93).
// - mode 0 with a world ([00E198C4] != 0): only byte +27Dh = 1 (deferred).
// - same mode: nothing.
// - otherwise +F8h = old mode, +274h = 0, +278h = -1.0 (00D7A260); slot +154h
//   of [00F8A2FC]; 004C8900 on the game; game +608h = 1; then
//   1 (host): peer rows reset 007868C0(1) (ECX session+8); delete the client
//      transport +18Ch (slot 0 with the mode, 1) and the server transport
//      +188h; route default 00E0AF1C = 1; 0060D570 (please-wait frame);
//      00A42170 (ECX session, EDX &+188h, stack [00F1AF30], byte +16Ah, byte
//      game +6A9h) creates the server; 0060D570; its state +9Ch = 3; the
//      scene count 00F1AF2C = the records of [[00E198ACh]+5Ch]+24h (434h
//      bytes each); each record's name (+24h, or the empty string 00F1B03C)
//      goes through 004E1D70(name, 0, -1, 0) and a please-wait frame;
//      004C6890(game +60Ch); 004BB160; game +1EE5h = 0; 00778590 (session+190h).
//   2 (client): rows reset (0); delete +18Ch then +188h; +125h = FFh; route
//      default 2; 00A41C70 (EDX &+18Ch, stack [00F1AF30], game +6A9h); +248h =
//      2; 004BB160; game +1EE5h = 0; 00778590.
//   0 (leave): rows reset (0); when the old mode was 2, with a client transport
//      and a peer from 0076F8A0, it prints "--- sent MQuitMsg" (004254B0) and,
//      when the transport state +9Ch is 7, sends message 19 (004B6050) through
//      00783DC0 and flushes with 007848F0; [00E19698]+4 = 0 when set; with a
//      game and a local slot (004B6BC0), that slot's word +10h = 1; route
//      default 1; 0076F0E0 when the old mode was 2; +278h = 2.0 (00CE3958);
//      bytes +128h/+12Ah/+12Bh and 00E17BF2 = 0; float 00E08178 = 1.0. The
//      transports are not deleted here.
//   then +F4h = mode, +270h = 0.0, and for mode 1 004BC890(game +618h, 0).
struct SessionModeState {
    std::int32_t mode_f4{0};
    std::int32_t previous_f8{0};
    std::uint8_t field_125{0};
    bool flag_128{false};
    bool flag_12a{false};
    bool flag_12b{false};
    bool server_option_16a{false};
    std::uintptr_t server_188{0};
    std::uintptr_t client_18c{0};
    std::int32_t field_248{0};
    float field_270{0.0f};
    bool flag_274{false};
    float timer_278{0.0f};
    bool leave_pending_27d{false};
};
struct SessionModeHost {
    virtual ~SessionModeHost() = default;
    virtual bool world_present_e198c4() = 0;
    virtual void singleton_f8a2fc_slot154() = 0;
    virtual void clear_scene_records_004c8900() = 0;
    virtual void set_game_608() = 0;
    virtual void reset_peer_rows_007868c0(bool host) = 0;
    virtual void delete_virtual(std::uintptr_t object) = 0;
    virtual void set_route_default_e0af1c(std::int32_t value) = 0;
    virtual void please_wait_frame_0060d570() = 0;
    virtual std::uintptr_t create_server_00a42170(bool option_16a) = 0;  // the stack bytes come from globals
    virtual std::uintptr_t create_client_00a41c70() = 0;
    virtual std::int32_t transport_state_9c(std::uintptr_t transport) = 0;
    virtual void set_transport_state_9c(std::uintptr_t transport, std::int32_t state) = 0;
    virtual std::vector<std::string> scene_record_names() = 0;  // null names already ""
    virtual void set_scene_count_f1af2c(std::int32_t count) = 0;
    virtual void build_scene_record_004e1d70(const std::string& name) = 0;
    virtual void select_scene_record_004c6890() = 0;             // game +60Ch
    virtual void reset_participants_004bb160() = 0;
    virtual void clear_game_1ee5() = 0;
    virtual void reset_peer_pairs_00778590() = 0;
    virtual std::uintptr_t client_peer_0076f8a0(std::uintptr_t transport) = 0;
    virtual void debug_print_004254b0(const char* text) = 0;
    virtual void send_quit_00783dc0(std::uintptr_t transport, std::uintptr_t peer) = 0;  // message 19 (004B6050)
    virtual void flush_007848f0(std::uintptr_t transport) = 0;
    virtual void clear_e19698_4() = 0;                           // when [00E19698] is set
    virtual void mark_local_slot_left() = 0;                    // game present and 004B6BC0: slot word +10h = 1
    virtual void refresh_from_online_session_0076f0e0() = 0;
    virtual void clear_e17bf2() = 0;
    virtual void set_e08178(float value) = 0;
    virtual void set_game_mode_004bc890() = 0;                   // (game +618h, 0)
};
void session_set_mode_0076fad0(SessionModeState& s, std::int32_t mode, SessionModeHost& host);

// ---------------------------------------------------------------------------
// Peers joining and leaving (sole caller BSP_PeerManager_PumpMessageQueues).
// [00F8A2FC] is the Live service object; its slots are named by offset. A
// peer's player record is peer +8h: name chars at +0, xuid +28h/+2Ch, byte
// +58h. The active transport is +188h (server) or else +18Ch (client).
struct PeerPlayer {
    std::string name_0;
    std::uint64_t xuid_28{0};
    bool flag_58{false};
};
struct LiveService {
    virtual ~LiveService() = default;
    virtual bool present_f8a2fc() = 0;
    virtual void register_talker_slot158(std::uint64_t xuid, const std::string& name, bool flag) = 0;
    virtual void unregister_talker_slotb0(std::uint64_t xuid) = 0;
    virtual std::uint32_t mute_state_slot24(std::uint64_t xuid) = 0;
    virtual void slot_ac(std::uint64_t xuid) = 0;
    virtual bool slot_180() = 0;
    virtual void slot_18c(bool flag) = 0;
    virtual void slot_190(bool flag) = 0;
    virtual void slot_15c(std::uint64_t xuid, const std::string& name) = 0;
    virtual bool slot_19c() = 0;
};

// 00774AB0: ECX session, stack peer, RET 4. Appends the peer to the active
// transport's list +Ch (0076EA10 node, 00773600 length check); in host mode
// (+F4h == 1) 007864E0 adds its row. With the Live object and a selected user
// (00A3E510 on [00F8ABE8]): slot 158h({xuid, name}, flag) for the record's
// xuid, name and +58h byte, or, without a record, for the local player (game
// +698h/+69Ch, name game +6A0h when its length is nonzero else +68Ch) with 0.
// Then, reading the record again (the image faults here without one):
// 00778620 adds the xuid to the mute table (ECX session+190h); 00771630
// (session) sets it muted per slot 24h(xuid); slot ACh(xuid) when the xuid is
// not the local one; slot 18Ch(+58h == 0) when slot 180h answers true.
struct PeerAddedHost {
    virtual ~PeerAddedHost() = default;
    virtual void append_to_active_transport_list(std::uintptr_t peer) = 0;
    virtual void add_peer_row_007864e0(std::uintptr_t peer) = 0;
    virtual bool has_selected_user_00a3e510() = 0;
    virtual PeerPlayer* player_record_8(std::uintptr_t peer) = 0;  // null when none
    virtual std::uint64_t local_xuid_698() = 0;
    virtual std::string local_name_6a0_or_68c() = 0;
    virtual void mute_table_add_00778620(std::uint64_t xuid) = 0;
    virtual void set_muted_00771630(std::uint64_t xuid, std::uint32_t muted) = 0;
};
// Returns false where the image would dereference a null record.
bool session_on_peer_added_00774ab0(std::int32_t mode_f4, std::uintptr_t peer, PeerAddedHost& host, LiveService& live);

// 00772EC0: ECX session, stack peer, RET 4. Host mode: 00786BE0 removes the
// row. Queued messages (+24Ch, count +250h) whose sender +8h is the peer are
// virtual-deleted (slot 0, 1) and removed. With the Live object and a peer:
// for a record, slot B0h(xuid), 00778690 (mute table), and when slot 180h
// answers, slot 190h(+58h == 0) and slot 15Ch({xuid, name}); then, unless game
// +218Ch, when slot 19Ch answers and the peer's key record +D50h has +1Ch below
// game +18C8h, "%x UnregisterKey remove peer" (byte +DBFh) and
// XNetUnregisterKey(peer +DB8h). It prints "Remove Valid Peer!" (a peer) or
// "Remove Empty Peer!" (none), asks [00F8ABDC] to disconnect the address
// peer +D74h after 2.0 s (00A3BBB0; the image passes 0D74h for a null peer),
// removes the peer from the active transport's list (0076F6F0) and
// virtual-deletes it.
struct PeerRemovedHost {
    virtual ~PeerRemovedHost() = default;
    virtual void remove_peer_row_00786be0(std::uintptr_t peer) = 0;
    virtual std::uintptr_t message_sender_8(std::uintptr_t message) = 0;
    virtual void delete_virtual(std::uintptr_t object) = 0;
    virtual PeerPlayer* player_record_8(std::uintptr_t peer) = 0;
    virtual void mute_table_remove_00778690(std::uint64_t xuid) = 0;
    virtual bool game_218c() = 0;
    // The key record +D50h: present and its +1Ch below game +18C8h.
    virtual bool key_index_below_player_count(std::uintptr_t peer) = 0;
    virtual std::uint8_t key_byte_dbf(std::uintptr_t peer) = 0;
    virtual void xnet_unregister_key_00a4d488(std::uintptr_t peer) = 0;  // &peer+DB8h
    virtual void debug_print_004254b0(const std::string& text) = 0;
    virtual void request_disconnect_00a3bbb0(std::uintptr_t peer, float seconds) = 0;  // &peer+D74h
    virtual void remove_from_active_transport_list_0076f6f0(std::uintptr_t peer) = 0;
};
void session_on_peer_removed_00772ec0(std::int32_t mode_f4, std::vector<std::uintptr_t>& messages_24c,
                                      std::uintptr_t peer, PeerRemovedHost& host, LiveService& live);

// ---------------------------------------------------------------------------
// 00773860 (BSP_Session_SendGameSetupMessage): ECX session, RET. One stack
// message 16h (0075B430, vtable 00D035CC, +4h = 1) is broadcast repeatedly
// through the server transport [+188h] (00784790(msg, 0)). Kind +18h = 0..5:
// one broadcast per element of the NativeString vectors at 00E196CC,
// 00E196DC, 00E196EC, 00E19768, 00E19778, 00E19788 (8-byte elements, text +4h,
// null -> ""), the text copied into the 256-byte payload +1Ch with no bound.
// Kind 6, one broadcast: the payload holds, back to back, the bits of the
// vector<bool>s at 00E196FC, 00E19710, 00E19724 (their size words; one byte
// per bit) and the low bytes of the int vectors at 00E19738, 00E19748,
// 00E19758; +11Ch..+130h hold the six counts. What the lists hold is not
// identified here; the image writes past +11Bh when they are long, the
// reconstruction stops at 256 bytes.
struct GameSetupMessage {
    std::int32_t kind_18{0};
    std::array<std::uint8_t, 256> payload_1c{};
    std::array<std::int32_t, 6> counts_11c{};
};
struct GameSetupLists {
    std::array<std::vector<std::string>, 6> texts;      // kinds 0..5
    std::array<std::vector<bool>, 3> flags;              // 00E196FC, 00E19710, 00E19724
    std::array<std::vector<std::int32_t>, 3> values;     // 00E19738, 00E19748, 00E19758
};
struct GameSetupHost {
    virtual ~GameSetupHost() = default;
    virtual void broadcast_00784790(const GameSetupMessage& m) = 0;
};
void session_send_game_setup_00773860(const GameSetupLists& lists, GameSetupHost& host);

// ---------------------------------------------------------------------------
// 007702E0 (BSP_Session_HostRemovePeerPlayer): ECX session, stack peer, RET 4.
// For the peer's player record (peer +D50h; +1Ch slot, +20h player index,
// +58h name, +80h/+84h xuid): keep a copy of the name; 0076C840(slot, -1)
// deselects the slot; 0076F2B0(player index) hands the units on; 0077E4C0 on
// the record; message 06h (vtable 00D0305C, +18h = slot) broadcast through
// [+188h]; 004D0FC0 on the game (record); peer +D50h = 0; when [00F8A2FC]
// +4Dh is set, 00A4B180 on [00F8A2FC]+2098h removes the xuid; 0076DBE0 on
// session+100h recounts the slots; in game states 0Dh, 0Ah and 10h (game
// +5D4h) the name is copied back into game player [+18CCh + index*4] +58h;
// MT_SERVERINFO (0076B670, the session bytes and 00786030's average peer
// quality) is broadcast; "MMultiPlayer::ServerRemovePlayer: slot: %d/%d,
// private: %d/%d, will: %d\n" (004254B0; +128h, +127h, +12Ah, +129h, +12Bh);
// 005CF320 (ECX = EDX = 8) posts a system chat line with the name and "".
struct HostRemovedPlayer {
    std::int32_t slot_1c{0};
    std::int32_t player_index_20{0};
    std::string name_58;
    std::uint64_t xuid_80{0};
};
struct SessionSlotCounts {
    std::int8_t field_127{0};
    std::int8_t field_128{0};
    std::int8_t field_129{0};
    std::int8_t field_12a{0};
    std::int8_t field_12b{0};
};
struct HostRemovePlayerHost {
    virtual ~HostRemovePlayerHost() = default;
    virtual HostRemovedPlayer* player_record_d50(std::uintptr_t peer) = 0;  // null: the image faults
    virtual void clear_player_record_d50(std::uintptr_t peer) = 0;
    virtual void select_player_0076c840(std::int32_t slot, std::int32_t selection) = 0;
    virtual void reassign_peer_units_0076f2b0(std::int32_t player_index) = 0;
    virtual void record_0077e4c0(HostRemovedPlayer& record) = 0;
    virtual void broadcast_remove_player_06(std::int32_t slot) = 0;
    virtual void notify_player_left_004d0fc0(HostRemovedPlayer& record) = 0;
    virtual bool live_session_4d() = 0;
    virtual void live_remove_remote_player_00a4b180(std::uint64_t xuid) = 0;
    virtual void recount_slots_0076dbe0() = 0;
    virtual SessionSlotCounts slot_counts() = 0;
    virtual std::int32_t game_state_5d4() = 0;
    virtual void set_game_player_name(std::int32_t player_index, const std::string& name) = 0;
    virtual void broadcast_server_info_0076b670() = 0;  // with 00786030's average quality
    virtual void debug_print_004254b0(const std::string& text) = 0;
    virtual void post_system_chat_line_005cf320(const std::string& name, const std::string& text) = 0;
};
// Returns false where the image would read a null player record.
bool session_host_remove_peer_player_007702e0(std::uintptr_t peer, HostRemovePlayerHost& host);

// ---------------------------------------------------------------------------
// MT_CONNSTATE (cc13-cam2b's peer_rows_update_007866b0 sends it through a host
// callback).
//
// 00786390: ECX message, stack rows, RET 4. 0075B430(10h), vtable 00D02FD0,
// +4h = 0, and per row i the floats +4h/+8h/+Ch at +18h + 0Ch*i.
struct ConnStateMessage {
    std::array<std::array<float, 3>, 8> rows_18{};
};
ConnStateMessage connstate_construct_00786390(const band0076_cc13::PeerRows& rows) noexcept;


// ---------------------------------------------------------------------------
// The MultiScore net table message E0h (multiplayer_session_cc13.hpp, batch 3).
// A key or value variant is {+0h kind (1 string, 0 number), +4h/+8h
// NativeString, +Ch number}.

// 00781430: ECX variant, stack const char*, RET 4. The string assigned
// (0041E870 into a temporary, then copied), kind = 1; the number is kept.
void net_table_key_set_string_00781430(mp_cc13::NetTableKey& key, const char* text);
// 007814E0: ECX variant, stack int, RET 4. A held string goes back to the
// sized pool and +4h/+8h are cleared; number = the argument, kind = 0.
void net_table_key_set_integer_007814e0(mp_cc13::NetTableKey& key, std::uint32_t number);
// 00781540: ECX variant, stack out NativeString*, RET 4, EAX = out: a copy of
// the variant's string.
std::string net_table_key_copy_name_00781540(const mp_cc13::NetTableKey& key);

// 00781F60: ECX message, stack const entry list*, RET 4. 0075B430(E0h), word
// +18h and byte +1Ah = 0, vtable 00D03824, a fresh list head at +20h
// (006EDEA0, byte +31h = 1), +24h..+2Ch = 0, +4h = 1; then the argument's
// entries are copied in (006EE960 erase, 00781890 tree copy) unless it is
// the message's own list.
struct NetTableSyncMessage {
    std::vector<mp_cc13::NetTableEntry> entries;  // +20h list, count +24h
    std::string text_28;                 // filled by the reader 00781910
};
NetTableSyncMessage net_table_sync_construct_00781f60(const std::vector<mp_cc13::NetTableEntry>& entries);

// 00781590 (vtable +4h): ECX message, stack bit cursor, RET 4. 0075B480 (the
// extended header), the count +24h in 5 bits (00429070), then per entry: bool
// key is string (004290B0); the key's string copy (00781540, 00429AC0) or its
// number in 32 bits; bool value is string; the value string (00429AC0) or
// 004295C0(value, 0, 1, [00D7A248], 32). The list is walked with 006EDA20.
struct NetTableWriteHost {
    virtual ~NetTableWriteHost() = default;
    virtual void write_extended_header_0075b480() = 0;
    virtual void write_bits_00429070(std::uint32_t value, int bits) = 0;
    virtual void write_bool_004290b0(bool value) = 0;
    virtual void write_owned_string_00429ac0(const std::string& text) = 0;
    virtual void write_numeric_float_004295c0(float value) = 0;  // (0, 1, [00D7A248], 32)
};
void net_table_sync_write_00781590(const NetTableSyncMessage& m, NetTableWriteHost& host);

// ---------------------------------------------------------------------------
// Small session helpers.

// 0076C450 (vtable 00D039EC slot 0; ECX unused): stack (peer, value), RET 8.
// The first of the eight game player records (game +748h, stride 118h) whose
// +54h is the peer gets 007F7490(value) (ECX = that record).
struct VoiceStampHost {
    virtual ~VoiceStampHost() = default;
    virtual std::uintptr_t player_record_peer_54(int index) = 0;
    virtual void stamp_voice_time_007f7490(int index, std::uint32_t value) = 0;
};
void session_stamp_player_voice_time_0076c450(std::uintptr_t peer, std::uint32_t value, VoiceStampHost& host);

// 0076C740 (vtable 00D039CC slot 0; ECX unused): stack (info, out int*), RET
// 8. info +14h = [info +8h (peer)] +D50h; *out = 2 when info's slot Ch answers
// true for 2 or else for 3, otherwise 0.
struct HostInfoClassifyHost {
    virtual ~HostInfoClassifyHost() = default;
    virtual std::uintptr_t peer_record_d50() = 0;   // [info+8h]+D50h
    virtual bool slot_c(std::int32_t kind) = 0;
};
std::int32_t session_host_info_classify_0076c740(std::uintptr_t& info_14, HostInfoClassifyHost& host);

// 0076D220: 0091C650, then byte [00F8A2FC]+74h = 0.
struct OnlineFlagHost {
    virtual ~OnlineFlagHost() = default;
    virtual void call_0091c650() = 0;
    virtual void clear_live_74() = 0;
};
void session_reset_online_flag_0076d220(OnlineFlagHost& host);

// 0076E7F0 (ledger BSP_WideString_FindSubstring; it is a prefix test): ECX
// wide string {+0 length, +4 data}, stack (needle wide string*, pos), RET 8,
// AL. False when the data pointer is null or pos > length; otherwise the
// UTF-16 units from pos are compared with the needle's (null data -> the
// empty 00F1B040) until either ends or they differ, and the result is
// whether the needle ended. `data_present` stands for the non-null pointer.
bool wide_string_matches_at_0076e7f0(const std::u16string& s, bool data_present, const std::u16string& needle,
                                     std::uint32_t pos) noexcept;

}  // namespace bsp::cc13cam2
