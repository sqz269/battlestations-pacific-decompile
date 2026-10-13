#pragma once
// Packet cc13-cam2 (docs/CC13_CAM_2.md): segment 45 (0078D880-007A4860, camera,
// movie camera, MovCam) and segment 43 (0076AA00-00787040, voice and session
// plumbing). Read from the listings and the disk bytes; descriptive names are
// hypotheses. Nothing here is ABI compatible or game-validated: entities are
// host handles (0 = none), engine helpers are host callbacks, and the x87
// chains are written in double with a float cast at each native FSTP (exact
// at 53-bit precision control when the products of two floats are exact).

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

}  // namespace bsp::cc13cam2
