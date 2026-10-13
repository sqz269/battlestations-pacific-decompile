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

// 007943D0: ECX camera, stack dt, RET 4. The velocity +3ACh is clamped to
// [+4D0h]+4Ch, the position +3A0h advanced by f(v*dt) and then kept within
// [+4D0h]+34h of +3B8h; the angles +3CCh likewise with the rate +3D4h clamped
// to +50h and the step to +38h. Each length is sqrt of the float-spilled sum
// (y*y + x*x) + z*z, zero unless above 1e-10 (00CE3820); 00793730 / 00793800
// do the rescaling.
void movcam_integrate_007943d0(cc13cam::MovCamMotion& m, cc13cam::MovCamTurn& t,
                               const cc13cam::MovCamLimits& limits, const cc13cam::MovCamTurnLimits& turn,
                               float dt) noexcept;

// 00794F70: ECX camera, stack (d.x, d.y, target), RET 0Ch. r = |angles -
// previous| / [+4D0h]+50h; a non-null target arms the follow cone (+3E8h =
// 00414C60(d) on a fresh target else the minimum, +3ECh = 0.9 (00CE3860),
// +3E4h = target); the wanted turn is d - 0.5*r*(angles - previous), its
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

}  // namespace bsp::cc13cam2
