#pragma once

// Packet cc13-sys2: engine code in segments 4, 6 and 9 that earlier passes
// named but did not reconstruct. Evidence: docs/CC13_SYS_2.md.
//
// Every name is a descriptive hypothesis, not a recovered symbol. Nothing here
// is ABI compatible: the host takes field values and callbacks where native
// code works on raw objects. Nothing is game-validated.

#include <array>
#include <cstdarg>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "bsp/engine_movie_actions.hpp"

namespace bsp {
struct NativeBitCursor;
struct NativeSessionReadStream;
struct NativeStringRawPoolContext;
}

namespace bsp::cc13_sys {

// ---------------------------------------------------------------------------
// Segment 6: engine-movie action constructors 00465C40..00466370.
// ---------------------------------------------------------------------------

// Each constructor is __thiscall(action, float activate_time), RET 4: it calls
// 00465B60 with the time, stores its own two-slot vtable {run, stop} and the
// type index at +10h. 00466170 (aParticleEffect) also clears +44h.
struct MovieActionConstructorRow {
    std::uint32_t constructor;
    std::int32_t type;      // +10h, the 00460B80 index
    std::uint32_t vtable;
};
extern const std::array<MovieActionConstructorRow, 31> kMovieActionConstructors;

// 00465B60 (base, vtable 00CE5498 {_purecall, 004656C0}) after 00465850 (the
// timed item at +4: time, then the comment NativeString at +8, both empty
// strings from 00CE3A0C), then the derived constructor for `type`. Index 4
// has no constructor (no factory case in 0045DE50): nullopt.
std::optional<MovieAction> construct_movie_action(std::int32_t type, float activate_time);

// 00465D00 (pEngineFailure run), __thiscall(action, entity), RET 4. ECX is not
// read; the entity's vtable +194h is called with ("explosion_delayed"
// (00CE54C4), 0).
void movie_engine_failure_run_00465d00(
    const std::function<void(const char* effect_name, std::int32_t arg)>& entity_slot_194);

// 004661A0 (aParticleEffect stop), __fastcall(ECX action), RET: when the
// handle at +44h is non-null it is passed to 008673B0 (ECX = handle), then
// +44h is cleared.
void movie_particle_effect_stop_004661a0(void*& handle_at_44, const std::function<void(void*)>& release);

// ---------------------------------------------------------------------------
// Segment 6: visual class entities and classes.
// ---------------------------------------------------------------------------

// The five create-entity slots (class vtable +8, __fastcall(ECX class), RET):
// operator new(bytes) + memset 0, 00489F30 (the 37h visual base), the three
// vtables at +0, +10h and +24h, the class id at +C4h, then 0047B9E0 (class at
// +170h) and class at +188h. Bullets and Cloud then call 00926BE0 (push the
// entity on the pending list); Cloud also clears dword +1A4h and byte +1A8h.
struct VisualEntityKind {
    std::uint32_t create_entity;
    std::uint32_t entity_constructor; // the same field writes without allocation
    const char* table;
    std::uint32_t bytes;
    std::int32_t class_id;
    std::uint32_t vtable_0;
    std::uint32_t vtable_10;
    std::uint32_t vtable_24;
    bool pushes_pending;
};
extern const std::array<VisualEntityKind, 5> kVisualEntityKinds;

// The host view of the class link fields the create slot writes.
struct VisualEntityClassLink {
    std::int32_t class_id_c4{0};
    const void* class_170{nullptr};
    const void* class_188{nullptr};
};

// Runs the field writes of one create slot on a host link record and calls
// `push_pending` for the two kinds that do. Returns false for an unknown
// create-entity address.
bool create_visual_entity(std::uint32_t create_entity, const void* cls, VisualEntityClassLink& link,
                          const std::function<void()>& push_pending);

// 00472FF0 (Bullets) and 0047A800 (VehicleClass), __thiscall(class, source),
// RET 4: named-class base 00489E60, own vtable (00CE5CB4 / 00CE6254), +28h =
// source, and the name NativeString at +4 = source+14h (the empty string
// 00E18694 when null).
struct SourcedVisualClassFields {
    std::string name;
    const void* source{nullptr};
};
SourcedVisualClassFields construct_sourced_visual_class(const void* source, const char* source_name_at_14);

// The registry teardown slots 00472420 (Anim), 00473CE0 (Bullets), 00475860
// (Buoy), 00479AC0 (Cloud), 0047B4E0 (VehicleClass), __cdecl, RET: for every
// node, call the mapped class's scalar deleting destructor with 1 (vtable
// slot 0) and null the node value, then erase all nodes and reset the head.
// BSP_Game_DestroyWorldContainers (004D2B10) calls four of them at
// 004D2B8A..004D2B99; the Bullets one has no rel32 or absolute reference, so
// the Bullets visual classes are never deleted.
template <class Map>
void delete_all_scene_classes(Map& registry) {
    for (auto& entry : registry) entry.second.reset();
    registry.clear();
}

// The registry "clear singleton pointer" unwind slots 00470D30, 00472990,
// 00474220, 00476440, 0047A080, and 0047C120 (soldier globals),
// __fastcall(ECX object), RET: the global slot is set to null and the object's
// vtable to the singleton base 00CE3818.
struct SingletonSlotView {
    void** global{nullptr};
    std::uint32_t* vtable{nullptr};
};
void clear_singleton_pointer(SingletonSlotView slot);

// 00476B90, __fastcall(ECX entity), RET: the visual at entity+360h, then its
// field +160h, or 0 without a visual.
std::uint32_t entity_visual_class_field_00476b90(const void* visual_at_360,
                                                 const std::function<std::uint32_t(const void*)>& field_160);

// 0046FDD0, __fastcall(ECX = Vec3 out, EDX = v, [ESP+4] = axis), RET 4:
// s = 00419440 length of axis (stored float); inv = 1/s when s > 0, else 0
// (NaN gives 0). n = axis * inv per component, stored float. d = (v0*n0 +
// v1*n1) + v2*n2 on the x87 stack, stored float. out = n * d per component.
// The host evaluates the dot in double, which matches 53-bit precision control.
std::array<float, 3> project_onto_axis_0046fdd0(const std::array<float, 3>& v, const std::array<float, 3>& axis,
                                                 const std::function<float(const std::array<float, 3>&)>& length);

// ---------------------------------------------------------------------------
// Segment 6: small helpers.
// ---------------------------------------------------------------------------

// 00467310, __cdecl(buffer, format, ...): _vsprintf_s(buffer, 20h, format,
// args). The host returns the formatted text; the CRT invalid-parameter path
// for over-long output is reported as std::length_error.
std::string format_to_buffer32_00467310(const char* format, ...);

// 00467F30, __thiscall(list, int index), RET 4: null for index < 0 or
// index >= count (+8h); otherwise walks `index` links from the head (+0h)
// through node+44h, returning null if a link is null.
const void* scene_node_child_at_00467f30(const void* head, std::int32_t count, std::int32_t index,
                                         const std::function<const void*(const void* node)>& next_at_44);

// ---------------------------------------------------------------------------
// Segment 9.
// ---------------------------------------------------------------------------

// 004BE940, __thiscall(list, int index), RET 4: node = list+4; for each step
// a null node gives 0, and a node whose next (+4) is null ends the walk early,
// so an index past the end returns the last payload (+8).
struct PayloadNode {
    const PayloadNode* next{nullptr}; // +4
    std::uint32_t payload{0};         // +8
};
std::uint32_t linked_list_nth_payload_004be940(const PayloadNode* head_at_4, std::int32_t index);

// 004C2DF0: the string object's compare(0, size(+14h), token, strlen(token))
// through 004BEB60; true when it returns 0, i.e. exact case-sensitive equality.
bool command_line_matches_token_004c2df0(const std::string& text, const char* token);

// 004C6820, __fastcall(ECX NativeString* name), RET: assigns the global base
// name 00E18998 unless the argument is that global, stores -1 at 00E08210 and,
// when byte 00E188AD is set, tail-jumps to 004BFBF0 (begin the sequence).
struct ScreenshotNaming {
    std::string base_name_e18998;
    std::int32_t index_e08210{0};
    bool sequence_enabled_e188ad{false};
};
void screenshot_set_base_name_004c6820(ScreenshotNaming& state, const std::string* name,
                                       const std::function<void()>& begin_sequence);

// 004BBC50, __fastcall(ECX = dialog result), RET: byte [[0109CF04]+180h] = 0
// and, for result 1, byte 00E1AE75 = 1.
struct QuitConfirmationFlags {
    bool dialog_open_180{true};
    bool quit_requested_e1ae75{false};
};
void on_quit_confirmation_004bbc50(std::int32_t result, QuitConfirmationFlags& flags);

// 004C88A0, __fastcall(ECX game), RET: pops every element of the deque at
// game+5DCh (map size +5E0h, head +5E4h, count +5E8h). Elements are trivial,
// so only the head and count move: the head advances modulo 4 * map size and
// returns to 0 when the count reaches 0.
struct DwordDequeHeader {
    std::uint32_t map_size_5e0{0};
    std::uint32_t head_5e4{0};
    std::uint32_t count_5e8{0};
};
void drain_state_requests_004c88a0(DwordDequeHeader& deque);

// 004C9800 / 004C9820: the player-slot record keeps five 38h-byte slot units
// at +8; `eh vector constructor/destructor iterator` with 004C6750 (clears
// +0h, +4h, +10h, +14h) and 004C6760.
struct SceneRecordSlotUnitHead {
    std::uint32_t field_00{1}, field_04{1}, field_10{1}, field_14{1};
};
void construct_slot_unit_004c6750(SceneRecordSlotUnitHead& unit);

// 0044CCD0 and the E1 message bodies (vtable 00CE4B30): the message keeps its
// type byte at +10h (the session-message storage), a NativeString at +18h, a
// byte at +20h and a NativeString at +24h.
bool session_message_e1_type_equals_0044ccd0(std::uint32_t type);

struct NativeSessionMessageE1Fields {
    std::uint8_t* type_10;
    void* text_18;           // 8-byte NativeString header
    std::uint8_t* small_20;  // two bits on the wire
    void* text_24;
};
// 0044CCE0, __fastcall(ECX = message), RET, under an EH frame: vtable
// 00CE4B2C, the strings at +24h/+28h and +18h/+1Ch back to the pool, base
// vtable 00CE4974.
void session_message_e1_destruct_0044cce0(
    std::uint32_t& vtable, const void* message,
    const std::function<void(void* data, std::uint32_t bytes, std::int32_t count)>& pool_return_00bd1510);
// 00449A70, __thiscall(message, NativeBitCursor*), RET 4.
void write_session_message_e1_00449a70(const NativeSessionMessageE1Fields& m, NativeBitCursor* cursor,
                                       const char* empty_string_backing_00e17669);
// 00449AB0, __thiscall(message, NativeSessionReadStream*), RET 4; reads from
// stream+4.
void read_session_message_e1_00449ab0(const NativeSessionMessageE1Fields& m, NativeBitCursor* stream_cursor_04,
                                      NativeStringRawPoolContext& pool);

// ---------------------------------------------------------------------------
// Packet cc13-sys4: the lazy singleton getters of segments 6 and 9.
// ---------------------------------------------------------------------------

// Every getter is __cdecl(void), plain RET, EAX = the cached global. Shape
// (00472200 listing 0047225A..004722BC): test the global; on null take the
// lifetime manager's lock (00415350, critical section at +10h, depth at +18h),
// test again, operator new (00BF681B), on success construct (a __thiscall
// constructor returning `this`, or inline vtable stores), on failure store 0,
// publish, then register (00BD0C30, ECX = manager) the global plus
// `registered_offset`, or null when the allocation failed. The register call
// is made even with a null pointer. Unlock, return the global.
struct LazySingletonGetterRow {
    std::uint32_t getter;
    std::uint32_t global;
    std::uint32_t allocation_size;
    std::uint32_t constructor;       // 0: the inline vtable stores below
    std::uint32_t registered_offset; // lifetime sub-object
    // Inline construction only: [+4] = base vtable, [+0] = primary, then [+4] = lifetime vtable.
    std::uint32_t base_vtable_04;
    std::uint32_t primary_vtable_00;
    std::uint32_t lifetime_vtable_04;
};
extern const std::array<LazySingletonGetterRow, 13> kLazySingletonGetters;
const LazySingletonGetterRow* find_lazy_singleton_getter(std::uint32_t getter);

struct LazySingletonHost {
    virtual ~LazySingletonHost() = default;
    virtual void lock() = 0;                                        // 00415350, EnterCriticalSection, depth++
    virtual void unlock() = 0;                                      // depth--, LeaveCriticalSection
    virtual void* operator_new(std::uint32_t size) = 0;             // 00BF681B
    virtual void* construct(std::uint32_t constructor, void* memory) = 0; // ECX = memory, EAX = this
    virtual void register_lifetime(void* subobject) = 0;            // 00BD0C30
};
// The shared body. The host stores the inline vtables into the allocation.
void* lazy_singleton_get(const LazySingletonGetterRow& row, void*& global, LazySingletonHost& host);

// 004C0E40 (00F87440) and 004C0EF0 (00F8753C), __cdecl(void), RET: the
// matching teardown (same template as 007361A0). When the global is set: lock, test again,
// unregister of the global (00BCFCA0, ECX = manager),
// the deleting destructor (vtable slot 0 with 1) when still non-null, clear
// the global, unlock.
struct LazySingletonDestroyHost {
    virtual ~LazySingletonDestroyHost() = default;
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual void unregister_lifetime(void* object) = 0;
    virtual void delete_object(void* object) = 0; // (*vtable[0])(1)
};
void lazy_singleton_destroy(void*& global, LazySingletonDestroyHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-sys4: 004656D0, aParticleEffect run (vtable 00CE5558 slot 0),
// __thiscall(ECX = action, [ESP+4] = entity), RET 4.
// ---------------------------------------------------------------------------
// The effect's children update 00867790 dispatches into effect code that can
// draw random numbers; in the image those draws share stream 0 of 00BD2F10
// with gameplay. RNG RULE: a host wiring this run in must route them to a
// presentation stream or keep the run behind an OFF switch.
struct MovieParticleEffectRunHost {
    virtual ~MovieParticleEffectRunHost() = default;
    // entity vtable +18h, ECX = entity; skipped (parent 0) for a null entity.
    virtual void* entity_parent(void* entity) = 0;
    // byte entity+C8h; read even when the entity is null (the image faults).
    virtual bool entity_world_pose_current(void* entity) = 0;
    virtual void refresh_world_pose_00414db0(void* entity) = 0;
    // 00871BA0, ECX = the handle argument slot of 008687C0, EDX = name
    // (action+2Ch), [ESP+4] = 1.
    virtual void* acquire_effect_handle_00871ba0(const void* name_at_2c, std::int32_t flag) = 0;
    // 008687C0, ECX = result smart pointer, EDX = parent; stack: the handle
    // (consumed), the matrix, 1, 0, 0. Returns the effect (*result).
    virtual void* create_point_effect_008687c0(void* parent, void* handle, const std::array<float, 16>& matrix,
                                               std::uint32_t transform_word, std::uint32_t option_word,
                                               std::uint32_t tail_word) = 0;
    // The result smart pointer's release: InterlockedDecrement(+4), slot 0
    // when it reaches zero. The raw effect pointer is kept regardless.
    virtual void release_result_reference(void* effect) = 0;
    virtual void clear_effect_byte_09(void* effect) = 0;
    // 00867790, ECX = effect, [ESP+4] = float step, [ESP+8] = [[00E188A8]+19FCh].
    virtual void update_children_00867790(void* effect, float step) = 0;
};
// Identity 4x4 (1.0f from 00D7A24C), transform word 1. When StartTime
// (action+20h) > 0.0f (00D7A218): step = remaining when remaining <= 0.2
// (double 00CE3D10; NaN too), else 0.2f (00CE54A0); remaining = f(remaining
// - 0.2) (DC E9, ST1 - ST0); repeat while remaining > 0. The effect is stored
// at action+44h. (The older ledger note says 0.1f; the constants are 0.2.)
void movie_particle_effect_run_004656d0(void* entity, float start_time_at_20, const void* name_at_2c,
                                        void*& effect_at_44, MovieParticleEffectRunHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-sys4: the x87 geometry cluster 0046F550..004702E0 (with 0046FDD0
// above). Traced from the disk bytes (capstone). Every named intermediate is
// stored to a float slot and reloaded; the sums between those stores stay on
// the x87 stack, which the hosts evaluate in double (53-bit precision control,
// as for 0046FDD0). Float*float products are exact in double. `length` is
// 00419440 (ECX = vector, ST0 = length).
// ---------------------------------------------------------------------------
using GeometryLength = std::function<float(const std::array<float, 3>&)>;

// 0046F550, __fastcall(ECX = out, EDX = point, [ESP+4] = line_point,
// [ESP+8] = dir, [ESP+0Ch] = float* distance), RET 0Ch, EAX = out. dir is
// used as given (a unit direction is assumed, not checked). w = f(point -
// line_point); t = f((dir0*w0 + dir1*w1) + dir2*w2); out = f(line_point +
// f(dir * t)); *distance = 0042B2F0(f(point - out)) (sqrt when the float sum
// of squares is above 1e-10, else 0).
std::array<float, 3> closest_point_on_line_0046f550(const std::array<float, 3>& point,
                                                    const std::array<float, 3>& line_point,
                                                    const std::array<float, 3>& dir, float& distance);

// 0046F6D0, __fastcall(ECX = point, EDX = origin, [ESP+4] = dir), RET 4,
// ST0 = float: the length of the projection of (point - origin) on dir,
// computed as the 0042B2F0-style length of f(dir * t) (t as in 0046F550).
float distance_along_axis_0046f6d0(const std::array<float, 3>& point, const std::array<float, 3>& origin,
                                   const std::array<float, 3>& dir);

// 0046F7B0 (the ledger's "ClosestPointOnSegment" is wrong), same ABI as
// 0046F550: out = f(point - f(normal * t)), t = dot(f(point - plane_point),
// normal): the projection of point onto the plane through plane_point with
// unit normal `normal`. *distance = 0042B2F0(f(out - plane_point)), the
// in-plane distance from plane_point.
std::array<float, 3> project_onto_plane_0046f7b0(const std::array<float, 3>& point,
                                                 const std::array<float, 3>& plane_point,
                                                 const std::array<float, 3>& normal, float& distance);

// 0046F940 (the ledger's "SegmentPlaneIntersect" is wrong), __fastcall(ECX =
// p1, EDX = u, [ESP+4] = p3, [ESP+8..10h] = v by value, [ESP+14h] = out_a or
// null, [ESP+18h] = out_b or null), RET 18h, ST0 = float distance. Closest
// points of the lines p1 + t*u and p3 + s*v. When f(dot(u, v)) == 1.0 exactly
// (FUCOMIP, LAHF/TEST AH,44h/JNP; NaN takes the general path) a = p1 and b =
// 0046F550(p1, p3, v) with its distance. Otherwise uu, uv, vv, uw, vw (w =
// f(p1 - p3)) are float dots, s = f((vw*uu - uw*uv) / (vv*uu - uv*uv)) with
// numerator and denominator kept on the stack, t = f(-((uw - s*uv) / uu)),
// a = f(p1 + f(u*t)), b = f(p3 + f(v*s)), distance = 0042B2F0(f(a - b)).
// Anti-parallel unit directions (dot -1) divide by zero.
float closest_points_between_lines_0046f940(const std::array<float, 3>& p1, const std::array<float, 3>& u,
                                            const std::array<float, 3>& p3, const std::array<float, 3>& v,
                                            std::array<float, 3>* out_a, std::array<float, 3>* out_b);

// 0046FE80, __fastcall(ECX = out, EDX = v, [ESP+4] = axis), RET 4, EAX = out:
// the 0046FDD0 projection subtracted from v. n and d as in 0046FDD0; out =
// f(v - f(n * d)).
std::array<float, 3> reject_from_axis_0046fe80(const std::array<float, 3>& v, const std::array<float, 3>& axis,
                                               const GeometryLength& length);

// 0046FF70 (the ledger's "DirectionAndLength" is a hypothesis this replaces),
// __fastcall(ECX = p, EDX = a, [ESP+4] = b), RET 4, ST0 = float: distance from
// p to the segment [a, b]. n = f(f(b - a) * inv), inv = f(1/length) when
// length > 0 else 0. d1 = f(dot(f(p - a), n)): d1 < 0 gives 0042B2F0(p - a).
// d2 = f(dot(f(b - p), n)): d2 < 0 gives 0042B2F0(b - p). Otherwise
// 0042B2F0(f(f(p - a) - f(n * d1))). NaN dots fall through (FCOMI/JBE).
float point_segment_distance_0046ff70(const std::array<float, 3>& p, const std::array<float, 3>& a,
                                      const std::array<float, 3>& b, const GeometryLength& length);

// 00470150 (ledger "DirectionAndLengthChecked"), same ABI, AL = bool: the
// d1/d2 tests of 0046FF70; false when d1 < 0 (AL only, the rest of EAX is not
// cleared) or when d2 < 0 or d2 is NaN (JB after FCOMIP; XOR EAX), else true
// (MOV EAX,1). A NaN d1 passes the first test.
// 004702E0, __fastcall(ECX = axis, EDX = v, [ESP+4] = out), RET 4, all in
// place: v = 0046FE80(v, axis); 0042B260 normalises axis, then v; out =
// axis x v, each component one product difference rounded once
// (y: v0*a2 - a0*v2, as the listing orders it).
void orthonormal_frame_from_axis_004702e0(std::array<float, 3>& axis, std::array<float, 3>& v,
                                          std::array<float, 3>& out, const GeometryLength& length);

bool point_projects_onto_segment_00470150(const std::array<float, 3>& p, const std::array<float, 3>& a,
                                          const std::array<float, 3>& b, const GeometryLength& length);

// ---------------------------------------------------------------------------
// Packet cc13-sys4: segment-6 matrix helpers. Matrices are 16 row-major
// floats; 00413920 is out = left * right (ECX = left, [ESP+4] = out,
// [ESP+8] = right, RET 8).
// ---------------------------------------------------------------------------
using Matrix4 = std::array<float, 16>;
using MatrixMultiply = std::function<Matrix4(const Matrix4& left, const Matrix4& right)>;

// 0047C210, __thiscall(ECX = m, const Matrix4* right), RET 4, EAX = m: a
// stack temporary = m * right (00413920), then 004134F0 copies it into m.
void matrix_multiply_into_0047c210(Matrix4& m, const Matrix4& right, const MatrixMultiply& multiply_00413920);

// 0047C240, __fastcall(ECX = out, EDX = const float t[3]), RET, EAX = out:
// identity (1.0f from 00D7A24C) with the translation in +30h..+38h.
void transform_init_translation_0047c240(Matrix4& out, const std::array<float, 3>& t);

// 0047C2A0, __fastcall(ECX = out, EDX = const float angles[3]), RET, EAX =
// out: Ry = 00B646E0(angles[0]), Rx = 00B64640(angles[1]), Rz =
// 00B64780(angles[2]); tmp = Rz * Rx, out = tmp * Ry. The older ledger
// note left the order unchecked.
struct EulerRotationHost {
    virtual ~EulerRotationHost() = default;
    virtual Matrix4 rotation_y_00b646e0(const float& angle) = 0;
    virtual Matrix4 rotation_x_00b64640(const float& angle) = 0;
    virtual Matrix4 rotation_z_00b64780(const float& angle) = 0;
    virtual Matrix4 multiply_00413920(const Matrix4& left, const Matrix4& right) = 0;
};
void matrix_build_from_euler_yxz_0047c2a0(Matrix4& out, const std::array<float, 3>& angles, EulerRotationHost& host);

// 00470B80, __fastcall(ECX = size), RET, EAX = block: operator new
// (00BF55BE) then memset(block, 0, size) (00BF79F0) with no null test.
void* memory_alloc_zeroed_00470b80(std::uint32_t size, const std::function<void*(std::uint32_t)>& operator_new_00bf55be);

// 00470BA0, __thiscall(ECX unused, float out[3]), RET 4: out = the three
// floats at 00F87574/78/7C (a static vector).
void class_entity37_velocity_00470ba0(std::array<float, 3>& out, const std::array<float, 3>& static_00f87574);

// 00472530 and 00473E30: a bare JMP 0048DBD0 (the shared SEntity init base).
// 00472540, __thiscall(ECX = object, arg), RET 4: CALL [vtable+0D8h] with
// ECX = object and the stack as it is (the callee sees the caller's return
// address where an argument would be), then RET 4. Modelled as the slot
// called with no arguments; `arg` is not forwarded.
void scene_object_forward_slot_d8_00472540(void* object, const std::function<void(void*)>& vslot_d8);

// 0047BEC0 (body-identical to 004B7EF0 and 0042BCA0), __fastcall(ECX =
// link), RET, under an EH frame: vtable 00CE6430; when +14h is set, 006952A0
// (ECX = subject, EDX = link); then 00695870 (ECX = link).
struct ObserverSlotFields;
void observer_link_destroy_base_0047bec0(ObserverSlotFields& link,
                                         const std::function<void(void* subject, void* observer)>& unregister_006952a0,
                                         const std::function<void(void* link)>& destroy_callback_owner_00695870);

// 00475E60, __thiscall(ECX = node, u32 mask), RET 4: node+48h |= mask, then
// the same on every child (first child +34h, next sibling +3Ch), depth first.
void node_or_flags_recursive_00475e60(void* node, std::uint32_t mask);

// Intrusive list nodes {prev +0, next +4, payload ...} freed with _free.
struct RawListNode {
    RawListNode* prev;
    RawListNode* next;
};
// 00467B50 (body-identical to 004837D0 and 00928030), __thiscall(ECX = list
// {count, head}, node), RET 4, EAX = the next node or 0: unlink (head = next
// when there is no prev), count--, _free. No tail is kept.
RawListNode* raw_list_erase_node_00467b50(std::int32_t& count, RawListNode*& head, RawListNode* node,
                                          const std::function<void(void*)>& free_00bf65ac);
// 004683B0 and the thunk 00468790 (body-identical to 004C2CE0 and 004BF8E0),
// __fastcall(ECX = list {count, head, tail}), RET: while count != 0, unlink
// the head (tail = prev when it was last), count--, _free.
void raw_list_clear_storage_004683b0(std::int32_t& count, RawListNode*& head, RawListNode*& tail,
                                     const std::function<void(void*)>& free_00bf65ac);

// 00467F60, __thiscall(ECX = array {data, size, capacity}, const u32* value),
// RET 4: when size == capacity, 004674F0 reserves max(capacity*2, 1) (JG 1);
// the value is stored when the slot address is non-null, and size++ always.
struct PodPointerArray {
    std::uint32_t* data{nullptr};
    std::int32_t size{0};
    std::int32_t capacity{0};
};
// 00468050 (unnamed in Ghidra; BSP_PodPointerArray_Resize here),
// __thiscall(ECX = array, n), RET 4: 004674F0 reserves n when capacity < n;
// slots size..n-1 are zeroed when their address is non-null, the size counts
// down to n while it is above it, and 00468098 stores size = n (Ghidra's
// decompile drops that store).
void pod_pointer_array_resize_00468050(PodPointerArray& array, std::int32_t n,
                                       const std::function<void(PodPointerArray&, std::int32_t)>& reserve_004674f0);
// 004C9870, __fastcall(ECX = set), RET, under an EH frame: for the arrays at
// +28h, +1Ch, +10h and +4 in that order, 00468050(0) then operator delete
// (00BF6989) of the data.
void sound_record_set_destruct_004c9870(std::array<PodPointerArray*, 4> arrays_28_1c_10_04,
                                        const std::function<void(PodPointerArray&, std::int32_t)>& reserve_004674f0,
                                        const std::function<void(void*)>& free_block);
void pod_pointer_array_push_back_00467f60(PodPointerArray& array, std::uint32_t value,
                                          const std::function<void(PodPointerArray&, std::int32_t)>& reserve_004674f0);

// Cloud particle arrays {data, size, capacity} of 20h-byte elements; 00476550
// reserves (ECX = array, count).
struct CloudParticleArray {
    std::array<std::uint32_t, 8>* data{nullptr};
    std::int32_t size{0};
    std::int32_t capacity{0};
};
using CloudParticleReserve = std::function<void(CloudParticleArray&, std::int32_t)>;
// 00476E40, __thiscall(ECX = array, const element*), RET 4: grow to
// max(capacity*2, 1) when full, copy the eight dwords when the slot address is
// non-null, size++.
void cloud_particle_array_push_back_00476e40(CloudParticleArray& array, const std::array<std::uint32_t, 8>& element,
                                             const CloudParticleReserve& reserve_00476550);
// 00476EE0, __fastcall(ECX = array), RET: 00476550(0) when capacity < 0, the
// size counted down to 0 (the elements are trivial), size = 0.
// 00477020, __fastcall(ECX = array), RET: the same, then _free (00BF6989) of
// the data; the data pointer is left as it was.
void cloud_particle_array_clear_00476ee0(CloudParticleArray& array, const CloudParticleReserve& reserve_00476550);
void cloud_particle_array_tidy_00477020(CloudParticleArray& array, const CloudParticleReserve& reserve_00476550,
                                        const std::function<void(void*)>& free_block);

// 00477110, __fastcall(ECX = class), RET, under an EH frame: vtable 00CE5FB4,
// the string at +2Ch/+30h back to the pool, then 00489E80 (named class base).
void cloud_class_destruct_00477110(
    std::uint32_t& vtable, const void* cloud_class,
    const std::function<void(void* data, std::uint32_t bytes, std::int32_t count)>& pool_return_00bd1510,
    const std::function<void(const void* cloud_class)>& named_class_base_destroy_00489e80);

// ---------------------------------------------------------------------------
// Packet cc13-sys4: segment-4 render settings and the system error box.
// ---------------------------------------------------------------------------

// 00452A50, __thiscall(ECX = settings, near, far), RET 8: +14h = near, +1Ch = far.
void depth_of_field_set_range_00452a50(float& near_14, float& far_1c, float near_value, float far_value);

// 00452B00, __thiscall(ECX = flare, float x), RET 4: +4 = +8 = |x|^k with k
// at +1Ch, through FYL2X (k * log2|x|), FRNDINT, F2XM1, FLD1/FADDP, FSCALE,
// stored float. x == 0 (UCOMISS, LAHF/TEST AH,44h/JNP) gives 0; a negative x
// becomes -0.0f - x (00D7A208). The earlier note "2^(k*|x|)" missed the
// FYL2X. The host evaluates exp2(k * log2|x|) in double; the image keeps
// 64-bit extended precision until the float store.
void lens_flare_pow_response_00452b00(float k_1c, float x, float& out_04, float& out_08);

// 00452BD0, __thiscall(ECX = flare, float dt, float zoom), RET 8, under an EH
// frame; callers binocular/periscope HUD. The camera [00E188B0] world matrix
// is refreshed (00B6DB70) when bit 2 of +5Ch is clear. With the camera at or
// below y = 0 (+124h <= 0; NaN continues) the lens effect is cleared
// (00B0D020(0, 0,0,0,0, 1.0f)). Otherwise the forward (+110h..) times 10000.0
// (00CE4BD8) gives the segment end. Then
// brightness = f((zoom - 1) * (-0.7 / (f(1/s) - 1)) + 1), with s at +18h and
// -0.7 the double 00CE4BD0. The spatial index (0042E630, 0098ADD0) is queried
// along the segment:
//   hit:  v = max(dot(f(hit - pos), forward), 1) (a NaN dot passes);
//   miss: c = f(cos(f(fov+1C4h * 0.25))); the light list [[00E188A8]+19CCh]+0D0h
//         picks the light whose normalised direction dot exceeds c most
//         (each light's pose refreshed first); found: v = 100000 (00CE4BC8);
//         none: v = 10000 (00CE3D64) unless the last y examined (the last
//         light's normalised d.y, or forward.y*10000 with no lights) is < 0,
//         then r = f(pos.y / f(sin(-forward.y))), v = 1 if r < 1, 100000 if
//         r > 100000, else r.
// p = |v|^k (k at +1Ch; 0 for v == 0; FYL2X/F2XM1/FSCALE); w = +14h * dt on
// the stack; +4 = f(+4 + (p - +4) * w); +8 = f(+8 + w * (+4 - +8));
// out = |+8|^(f(1/k)) (0 for 0), then 00B0D020(1, out, out, out, brightness, 1.0f).
// No random numbers are drawn.
struct LensFlareFields {
    float value_04, value_08, gain_14, scale_18, exponent_1c;
};
struct LensFlareHost {
    virtual ~LensFlareHost() = default;
    virtual bool camera_world_current() = 0;                 // [00E188B0]+5Ch & 2
    virtual void refresh_camera_world_00b6db70() = 0;
    virtual std::array<float, 3> camera_position_120() = 0;
    virtual std::array<float, 3> camera_forward_110() = 0;
    virtual float camera_fov_1c4() = 0;
    virtual void set_lens_effect_00b0d020(std::int32_t on, float a, float b, float c, float brightness, float e) = 0;
    // 0042E630 then 0098ADD0(pos, end, 0, hit record, 0); the hit position is hit record +8.
    virtual bool query_segment_0098add0(const std::array<float, 3>& from, const std::array<float, 3>& to,
                                        std::array<float, 3>& hit_position) = 0;
    virtual std::vector<void*> lights() = 0;                  // node +8, next +4
    virtual bool light_pose_current_c8(void* light) = 0;
    virtual void refresh_light_pose_00414db0(void* light) = 0;
    virtual std::array<float, 3> light_position_fc(void* light) = 0;
};
void lens_flare_update_from_camera_00452bd0(LensFlareFields& flare, float dt, float zoom, LensFlareHost& host);

// 00452B80, __thiscall(ECX = flare, u8 on), RET 4: byte +0 = on; when off,
// 00B0D020 (ECX = [00F8D39C]) with (0, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f) and
// +4 = 1e-4f (00CE3C68).
void lens_flare_set_enabled_00452b80(
    std::uint8_t on, std::uint8_t& enabled_00, float& value_04,
    const std::function<void(std::int32_t, float, float, float, float, float)>& set_lens_effect_00b0d020);

// 004531F0, __fastcall(ECX = const char* context), RET:
// FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, null, GetLastError(), 0,
// 00E18000, 3FFh, null), then MessageBoxA(null, 00439AC0("%s\n%s", context,
// 00E18000), "System error", MB_ICONERROR (10h)). 00439AC0 formats into a static
// buffer; its size limit is not modelled.
struct SystemErrorBoxHost {
    virtual ~SystemErrorBoxHost() = default;
    virtual std::uint32_t get_last_error() = 0;
    virtual std::string format_system_message(std::uint32_t error, std::uint32_t size) = 0;
    virtual void message_box(const std::string& text, const char* caption, std::uint32_t type) = 0;
};
void show_last_system_error_box_004531f0(const char* context, SystemErrorBoxHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-sys4: small segment-9 helpers.
// ---------------------------------------------------------------------------

// 004BB600, __cdecl(void), AL. ECX = [00F8ABE8] (Xenon system manager) for
// each call: 00A3E510 (has a selected user) and 00A3EAD0 (sign-in state)
// == 2, then a tail jump to 00A3EB20 (the byte at manager+118h); otherwise
// XOR AL,AL.
struct SelectedUserLiveHost {
    virtual ~SelectedUserLiveHost() = default;
    virtual bool has_selected_user_00a3e510() = 0;
    virtual std::int32_t selected_sign_in_state_00a3ead0() = 0;
    virtual std::uint8_t active_slot_byte_118_00a3eb20() = 0;
};
std::uint8_t selected_user_live_byte_004bb600(SelectedUserLiveHost& host);

// 004BBDD0, __thiscall(ECX = game, const float* start, float* end, float*
// out), RET 0Ch, AL = clipped. Caller 0071FDE0 (director path AddPoint).
// The world box is minX +711Ch, maxZ +7124h, maxX +7128h, minZ +7130h. In x:
// end.x < minX moves end to minX + 1 and interpolates end.z with r = (sx -
// minX)/(sx - x); end.x > maxX moves it to maxX - 1 with r = (maxX - sx)/(x -
// sx). Then the same in z against maxZ (end.z - 1 side) and minZ, which
// interpolates end.x. Each new coordinate is f(s + f(e - s) * r), with r kept
// on the stack. With no clip it returns 0 and writes nothing. After a clip, a
// moved end closer than 1.0 to the start in |dx| or |dz| is put back. In that
// case `end` regains its x and z, and the function still returns 1. Then d =
// (f(end.x - sx), 0, f(end.z - sz)) is normalised (00419510) to n, m =
// min-like(|n.x|, |n.z|) (|n.z| > |n.x| keeps |n.x|, otherwise |n.z|), raised
// to 0.02f when below double 0.02 (00D7A2F8), s = f(3000.0 / m) (00CE7530),
// and out = f(end + f(n * s)), where out.y uses the end's y.
struct WorldBoundsXZ {
    float min_x_711c, max_z_7124, max_x_7128, min_z_7130;
};
bool game_clip_segment_to_world_bounds_004bbdd0(
    const WorldBoundsXZ& box, const std::array<float, 3>& start, std::array<float, 3>& end,
    std::array<float, 3>& out,
    const std::function<std::array<float, 3>(const std::array<float, 3>&)>& normalize_00419510);

// 004CD7F0, __fastcall(ECX = out NativeString, EDX = path NativeString, u8
// append_mode at [ESP+4]), RET 4, EAX = out. The body up to 004CDB22 is
// bsp::derive_scene_short_name (src/mission_scene_load.cpp; backslashes to
// '/', text after the last '/', cut at the second '_', ".scn" dropped). That
// host omits the tail 004CDB27..004CDBC1: when append_mode is non-zero (the
// mission load, scene-contents and scene-record callers push 1) a suffix from
// the table 00E08240 is appended. The table is IC1 IC2 IC3 IC4 Du Esc SiE Com
// SOLO SOLO. The index comes from the game ([00E188A8]): mode +614h when the
// byte +61Ch is set or +1FE4h != 0, otherwise +614h when it is 8 or 9, else
// 8. The index is not range-checked.
struct ShortNameModeFields {
    std::uint8_t byte_61c;
    std::int32_t mode_614;
    std::int32_t field_1fe4;
};
inline constexpr std::array<const char*, 10> kSceneModeSuffixes_00e08240{
    "IC1", "IC2", "IC3", "IC4", "Du", "Esc", "SiE", "Com", "SOLO", "SOLO"};
std::int32_t scene_mode_suffix_index_004cdb32(const ShortNameModeFields& game);
std::string derive_scene_short_name_004cd7f0(const std::string& path, bool append_mode, const ShortNameModeFields& game);

// 004C0170, __thiscall(ECX = game), RET: Live presence. Only with a selected
// user (00A3E510). t = f(ticks / frequency) from the frame clock [01090AB0]
// vtable +20h. The first call (flag 00E082E8) seeds the stamp 00E18B30, and
// t - stamp > 3.0 (00D7A2B0) sets `force` and restamps. `hosted` = [00E198B4]
// set, state +5D4h == 8, 00687860(14h, 0) on it true and the byte +218Ch
// clear. Outside state 0Dh without `hosted`, or with no scene record block
// (+5FCh), the call only does XUserSetContext(slot, 8001h, 5) when `force`
// is set. Otherwise `force` also takes the byte +22DCh (then cleared), a new
// xuid (00A3EB00 against 00E18B28/2C) or a new record id (block+1098h
// against 00E18B24). With `force`, or a mode index (the 004CDB32 rule)
// different from 00E18B20, it publishes four XUserSetContextEx calls:
//   context 0      = by campaign kind 004BFFA0: 1->3, 2->4, 3->5, 4->(+218Ch ? 2 :
//                    state 8 and the 00687860 test ? 6 : 1), else 1;
//   context 1      = 0 for kind 1 or 2, else the record id;
//   context 0Fh    = mode bucket: 0..3->0, 4->1, 5->2, 6->3, 7->4, else 0;
//   context 8001h  = (+1FE4h != 0) + 2;
// then stores the xuid, record id and mode index.
struct PresenceGameFields {
    std::int32_t state_5d4;
    bool has_record_block_5fc;
    std::int32_t record_id_1098;  // [+5FCh]+1098h
    std::uint8_t* presence_dirty_22dc;
    ShortNameModeFields mode;     // +61Ch, +614h, +1FE4h of [00E188A8]
    std::uint8_t byte_218c;       // of [00E188A8]
};
struct PresenceState {
    std::uint8_t first_00e082e8{1};
    float stamp_00e18b30{0.0f};
    std::int32_t mode_index_00e18b20{0};
    std::int32_t record_id_00e18b24{0};
    std::uint64_t xuid_00e18b28{0};
};
struct PresenceHost {
    virtual ~PresenceHost() = default;
    virtual bool has_selected_user_00a3e510() = 0;
    virtual void frame_clock_vslot_20(std::int64_t& ticks, std::int64_t& frequency) = 0;
    virtual bool has_object_00e198b4() = 0;
    virtual bool object_00e198b4_test_00687860(std::int32_t a, std::int32_t b) = 0;
    virtual std::int32_t mission_campaign_kind_004bffa0() = 0;
    virtual std::uint64_t active_xuid_00a3eb00() = 0;
    virtual std::uint32_t active_user_slot_00a3eac0() = 0;
    virtual void x_user_set_context(std::uint32_t slot, std::uint32_t context, std::uint32_t value) = 0;
    virtual void x_user_set_context_ex(std::uint32_t slot, std::uint32_t context, std::uint32_t value,
                                       void* overlapped) = 0;
};
void game_update_presence_context_004c0170(const PresenceGameFields& game, PresenceState& state, PresenceHost& host);

// 004BCB40, __thiscall(ECX = {u32 length, char* data}, needle, int start),
// RET 8: -1 for null data or needle; a negative start becomes 0, a start
// above the length (unsigned JA) gives -1; _strstr (00BF9440) from
// data + start; the match offset from data, or -1.
std::int32_t counted_string_find_004bcb40(std::uint32_t length, const char* data, const char* needle,
                                          std::int32_t start);

// 004BDF50, __fastcall(ECX = interface), RET: when +4 != +20h or +1Ch !=
// +38h, vtable +10h(+20h, +38h) (the pending size). +4/+1Ch are not written here.
struct PendingSizeFields {
    std::int32_t current_04, current_1c, pending_20, pending_38;
};
void multiplayer_interface_apply_pending_size_004bdf50(
    const PendingSizeFields& f, const std::function<void(std::int32_t, std::int32_t)>& vslot_10);

// 004BEA10 / 004BEA40 / 004BEA70, __fastcall(ECX = owner pointer slot), RET:
// when the slot is set, the destructor (ECX = object), _free (00BF65AC),
// slot = 0. Destructors: 004BEA10 -> 006B9380 (award registry storage),
// 004BEA40 -> 0045D930 (cutscene player), 004BEA70 -> 009496A0.
struct OwnedPointerDeleteRow {
    std::uint32_t address;
    std::uint32_t destroy;
};
extern const std::array<OwnedPointerDeleteRow, 3> kOwnedPointerDeletes;
void owned_pointer_delete(void*& slot, const std::function<void(void*)>& destroy,
                          const std::function<void(void*)>& free_block);

// 004BEAD0, __fastcall(ECX = unit), RET: the unit when non-null and its
// vtable +5Ch(0Fh) returns non-zero AL, else 0.
void* unit_cast_if_class_flag_0f_004bead0(void* unit, const std::function<bool(void*, std::int32_t)>& vslot_5c);

// 004BFD70, __fastcall(ECX = game), RET: 00B0FEB0 on [00F8D39C], the
// renderer's ([00F8D394]) vtable +124h, then when game+5FCh is set the three
// counted references at +D5Ch, +D70h, +D84h of that block are released
// (InterlockedDecrement(+4); vtable slot 0 at zero) and cleared.
struct SessionRenderResourcesHost {
    virtual ~SessionRenderResourcesHost() = default;
    virtual void reset_remap_textures_00b0feb0() = 0;
    virtual void renderer_vslot_124() = 0;
    virtual void release_reference(void* counted) = 0;
};
void release_session_render_resources_004bfd70(void* block_at_5fc, SessionRenderResourcesHost& host);

// The 18h-byte observer slot: vtable, three cleared dwords +4..+0Ch, byte
// +10h = 1, observed subject +14h.
struct ObserverSlotFields {
    std::uint32_t vtable{0};
    std::uint32_t field_04{0}, field_08{0}, field_0c{0};
    std::uint8_t flag_10{0};
    void* subject_14{nullptr};
};
// 004C2D40, __fastcall(ECX = slot), RET, EAX = slot: vtable 00CE75CC
// (slot 0 = deleting destructor 004C2DC0, slot 1 = 004C2D90), subject 0.
void construct_observer_slot_004c2d40(ObserverSlotFields& slot);
// 004BEF00, __thiscall(ECX = request, subject), RET 4, EAX = request: vtable
// 00CE3C8C, subject +14h, then 00694A60 (ECX = subject, EDX = request)
// when the subject is non-null.
void construct_interface_request_004bef00(ObserverSlotFields& request, void* subject,
                                          const std::function<void(void* subject, void* observer)>& register_pair);
// 004C2D60 and 004C2D90 (00CE75CC slot 1), __thiscall(ECX = slot, notifier),
// RET 4: notifier vtable +4h yields the object; when it is +14h, clear +14h
// and 006952A0 (ECX = object, EDX = slot). Same body as 004884F0.
void observer_slot_release_if_subject(ObserverSlotFields& slot, void* notifier,
                                      const std::function<void*(void* notifier)>& notifier_object_04,
                                      const std::function<void(void* subject, void* observer)>& unregister_pair);

// 004C2FD0, __fastcall(ECX = character count), RET, EAX = block: 00419CC0
// (sized storage pool) then 00BD1120 (ECX = pool, count*2 + 2 bytes, 1): a
// UTF-16 buffer with room for the terminator.
void* wide_string_allocate_storage_004c2fd0(
    std::uint32_t count, const std::function<void*(std::uint32_t bytes, std::int32_t flag)>& pool_allocate_00bd1120);

// 004C5140, __stdcall(context), RET 4: 004BEC00 (input manager), then
// 00A933F0 (ECX = manager, context, 5).
void input_context_level5_004c5140(std::int32_t context,
                                   const std::function<void(std::int32_t context, std::int32_t level)>& set_00a933f0);

// 004C5E00, __thiscall(ECX = storage manager, out string), RET 4, EAX = out:
// manager vtable +10h(out). The zeroed stack dword is the NRVO construction
// flag and is not read.
void storage_manager_newest_slot_name_004c5e00(void* out, const std::function<void(void* out)>& vslot_10);

// 004C5E20, __thiscall(ECX = this, other), RET 4, EAX = this. Strings are
// {u32 length, char16_t* data}. Self-assignment does nothing; otherwise
// 004C53E0 (ECX = this, other.length, 1) sizes the storage, and when
// other.length != 0 the UTF-16 units are copied up to and including the
// terminator.
struct NativeWideString {
    std::uint32_t length{0};
    char16_t* data{nullptr};
};
NativeWideString& wide_string_assign_004c5e20(
    NativeWideString& self, const NativeWideString& other,
    const std::function<void(NativeWideString& self, std::uint32_t length, std::int32_t flag)>& size_004c53e0);

// 004C5EC0, __thiscall(ECX = scope, index, const u32* colour), RET 8, EAX =
// scope: profiler (004C1DD0) +24h colour array[index] = *colour, scope.index
// = index, then 00BE3640 (ECX = profiler, index).
// 004C5EF0, __fastcall(ECX = scope), RET: 00BE3660 (ECX = profiler, scope.index).
struct ProfilerScopeHost {
    virtual ~ProfilerScopeHost() = default;
    virtual std::uint32_t* colour_array_24() = 0;
    virtual void begin_00be3640(std::int32_t index) = 0;
    virtual void end_00be3660(std::int32_t index) = 0;
};
void profiler_scope_begin_004c5ec0(std::int32_t& scope_index, std::int32_t index, const std::uint32_t& colour,
                                   ProfilerScopeHost& host);
void profiler_scope_end_004c5ef0(std::int32_t scope_index, ProfilerScopeHost& host);

// 004C5F10, __fastcall(ECX = message), RET: vtable 00CE7608; when the byte at
// +18h is 3, 004C0FD0 on +1Ch; then 004C1F70 on +1Ch; base vtable 00CE4974.
// 004C5F40, __thiscall(ECX unused, id), RET 4: id == 0E4h.
struct SessionMessageE4DestructHost {
    virtual ~SessionMessageE4DestructHost() = default;
    virtual void release_payload_004c0fd0(void* payload_1c) = 0;
    virtual void destruct_payload_004c1f70(void* payload_1c) = 0;
};
void session_message_e4_destruct_004c5f10(std::uint32_t& vtable, std::uint8_t kind_18, void* payload_1c,
                                          SessionMessageE4DestructHost& host);
bool session_message_e4_is_id_004c5f40(std::uint32_t id);

// Pooled NativeString storage release. Each field is a {u32 length, data}
// pair; when data is non-null it goes back to the sized pool (00419CC0, then
// 00BD1510 with ECX = pool, (data, bytes, 1)), bytes = length + 1 for narrow
// strings and length*2 + 2 for UTF-16. Fields are released in table order.
// The pool call does not clear the fields.
struct PooledStringField {
    std::uint16_t length_offset;
    std::uint16_t data_offset;
    std::uint8_t unit; // 1 or 2 bytes
};
struct PooledStringReleaseRow {
    std::uint32_t address;
    const char* abi;
    std::array<PooledStringField, 3> fields;
    std::uint8_t count;
};
// 004C67E0, 004C6800, 004C89E0 (+8/+0Ch narrow), 004C8260 (+8/+0Ch UTF-16),
// 004C8DB0 (+0Ch/+10h narrow), 004CE690 (__stdcall(object), RET 4, +8/+0Ch
// UTF-16), 004C5720 (+10h/+14h, +8/+0Ch, +0/+4, under an EH frame),
// 004C6760 (+10h/+14h, +0/+4), and the byte-identical +0/+4 bodies 0044BA70,
// 00468B30, 00468B50, 00471340, 00474780, 00477200.
extern const std::array<PooledStringReleaseRow, 14> kPooledStringReleases;
const PooledStringReleaseRow* find_pooled_string_release(std::uint32_t address);
void release_pooled_strings(
    const PooledStringReleaseRow& row, const void* object,
    const std::function<void(void* data, std::uint32_t bytes, std::int32_t count)>& pool_return_00bd1510);


// 004C7F80, __fastcall(ECX = record), RET, EAX = record: +0, +4, +8 = 0 and
// the floats +0Ch, +10h = 0.0f.
struct Record14 {
    std::uint32_t field_00{1}, field_04{1}, field_08{1};
    float field_0c{1.0f}, field_10{1.0f};
};
void record14_clear_004c7f80(Record14& record);

// 004C8DD0, __thiscall(ECX = dst, src), RET 4, EAX = dst: dst = {0, null},
// then the 004C5E20 assignment (self-copy skipped).
NativeWideString& wide_string_copy_construct_004c8dd0(
    NativeWideString& dst, const NativeWideString& src,
    const std::function<void(NativeWideString& self, std::uint32_t length, std::int32_t flag)>& size_004c53e0);

// The narrow NativeString {u32 length, char* data}; 0041DD40 (ECX = string,
// length, flag) resizes it, and each body then copies `length` bytes (the
// length after the resize) with _memcpy (00BF7680) when data is non-null.
struct NativeNarrowString {
    std::uint32_t length{0};
    char* data{nullptr};
};
using NarrowStringResize = std::function<void(NativeNarrowString& string, std::uint32_t length, std::int32_t flag)>;

// A narrow string key with a dword value at +8.
struct NarrowStringPair {
    NativeNarrowString key;
    std::uint32_t value_08{0};
};
// 00469AB0, 0044C5D0, 004713B0, 004747F0, 00478A50 (byte-identical apart from
// call displacements), __thiscall(ECX = dst, const NativeString* key, const
// u32* value), RET 8, EAX = dst: key = {0, null} then the 0041DD40 (flag 1)
// assignment, skipped for self and copied only when non-empty; +8 = *value.
// 0044C6E0, __thiscall(ECX = dst, const pair* src), RET 4: the same with the
// value taken from src+8.
NarrowStringPair& narrow_string_pair_construct(NarrowStringPair& dst, const NativeNarrowString& key,
                                               std::uint32_t value, const NarrowStringResize& resize_0041dd40);
NarrowStringPair& narrow_string_pair_copy_construct_0044c6e0(NarrowStringPair& dst, const NarrowStringPair& src,
                                                             const NarrowStringResize& resize_0041dd40);

// 004C8900, __fastcall(ECX = game), RET: 004BF930 (destroy every scene
// record), game+5FCh = 0, and the string at game+600h is resized to 0 (flag
// 0) and filled from the empty string 00CE3A0C.
void game_clear_scene_records_004c8900(const std::function<void()>& destroy_all_004bf930, void*& record_block_5fc,
                                       NativeNarrowString& text_600, const NarrowStringResize& resize_0041dd40);
// 004C8950, __thiscall(ECX = game, const char* text), RET 4: the string at
// game+600h takes strlen(text) (0 for null) with flag 0, then the bytes.
void game_set_command_line_text_004c8950(NativeNarrowString& text_600, const char* text,
                                         const NarrowStringResize& resize_0041dd40);
// 004C89A0, __thiscall(ECX = game, const NativeString* src), RET 4: the string
// at game+2198h takes src (flag 1) unless it is src itself; the copy is
// skipped when src is empty.
void game_assign_console_string_004c89a0(NativeNarrowString& string_2198, const NativeNarrowString& src,
                                         const NarrowStringResize& resize_0041dd40);

// 004C9220, __thiscall(ECX = dst, src), RET 4, EAX = dst: a 60h-byte entry.
// Byte +0 and dword +4, the UTF-16 string at +8 by 004C53E0 + unit copy
// (skipped for self), then the dwords +10h..+5Ch.
struct LocalisedEntry60 {
    std::uint8_t byte_00{0};
    std::uint32_t dword_04{0};
    NativeWideString text_08;
    std::array<std::uint32_t, 20> dwords_10{};
};
LocalisedEntry60& localised_entry_assign_004c9220(
    LocalisedEntry60& dst, const LocalisedEntry60& src,
    const std::function<void(NativeWideString& self, std::uint32_t length, std::int32_t flag)>& size_004c53e0);

// 004C95D0, __thiscall(ECX = entry, a, b, c, const NativeString* name, u8
// flag), RET 14h, EAX = entry: +0 a, +4 b, +8 c, the string at +0Ch cleared
// then assigned from name (0041DD40 flag 1; skipped when name is the field
// itself; bytes copied only for a non-empty name), byte +14h = flag, +15h = 0.
struct NamedEntry16 {
    std::uint32_t a_00{0}, b_04{0}, c_08{0};
    NativeNarrowString name_0c;
    std::uint8_t flag_14{0};
    std::uint8_t byte_15{0};
};
NamedEntry16& named_entry_construct_004c95d0(NamedEntry16& entry, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                             const NativeNarrowString& name, std::uint8_t flag,
                                             const NarrowStringResize& resize_0041dd40);

// 004C9420, __thiscall(ECX = record, u8 flags), RET 4, EAX = record: the
// narrow string at +0/+4 back to the pool, then 0043B0A0 (ECX = node pool:
// 00E175E8 here, 00E18514 for the body-identical 00469B20; 14h-byte node pool
// return) when flags & 1.
void named_record_scalar_deleting_destructor_004c9420(
    void* record, std::uint8_t flags,
    const std::function<void(void* data, std::uint32_t bytes, std::int32_t count)>& pool_return_00bd1510,
    std::uint32_t node_pool, const std::function<void(std::uint32_t pool, void* record)>& node_pool_return_0043b0a0);

// 004C9680, __cdecl(void), RET: a local empty string (0041DD40 to length 0,
// flag 1, then length+1 bytes from 00CE3A0C), 00518D60 with ECX = EDX = -1
// (backdrop and panel), title -1 and the string, then the string's block back
// to the pool.
struct FrontEndFrameSelectHost {
    virtual ~FrontEndFrameSelectHost() = default;
    virtual void resize_0041dd40(NativeNarrowString& string, std::uint32_t length, std::int32_t flag) = 0;
    virtual void select_00518d60(std::int32_t backdrop, std::int32_t panel, std::int32_t title,
                                 const NativeNarrowString& text) = 0;
    virtual void pool_return_00bd1510(void* data, std::uint32_t bytes, std::int32_t count) = 0;
};
void front_end_frame_select_default_004c9680(FrontEndFrameSelectHost& host);

// 004CA2F0, __fastcall(ECX = game), RET, under an EH frame. The close request
// byte [0109CF04]+180h is read and cleared. Nothing more happens without a
// request or while [00F8ABE8]+3E8h is set. In game states 1, 2 and 4
// (+5D4h) the exit flag 00E1AE75 is set. Otherwise the prompt screen
// (00425D10) raises the quit confirmation through 00531B00 with type 6, the
// key "FE_pc.main_quit_confirm" (00CE76D4), 1, the callback 004BBC50, 1, an
// empty string, 0 and 0. The argument list follows the decompile; the image
// passes the strings through stack copies. The temporary strings are
// released, then 00530C20 runs on the screen.
struct WindowCloseHost {
    virtual ~WindowCloseHost() = default;
    virtual bool take_close_request_180() = 0;            // read and clear [0109CF04]+180h
    virtual bool platform_busy_3e8() = 0;                 // [00F8ABE8]+3E8h
    virtual void set_exit_flag_00e1ae75() = 0;
    virtual void raise_prompt_00531b00(std::int32_t type, const std::string& key, std::int32_t a,
                                       std::uint32_t callback, std::int32_t b, const std::string& text,
                                       std::int32_t c, std::int32_t d) = 0;
    virtual void prompt_screen_00530c20() = 0;
};
void game_process_window_close_request_004ca2f0(std::int32_t state_5d4, WindowCloseHost& host);

// 004CA120, __fastcall(ECX = game), RET: only in game state 0Dh (+5D4h) with
// a client at +21E8h. Builds "ingame.multi_highlatency" (00CE76B8, through
// 0041E870) and an empty string, calls 007349C0 (ECX = client, &key, &empty,
// 0), then returns the empty string's block and the key's block to the pool.
struct HighLatencyWarningHost {
    virtual ~HighLatencyWarningHost() = default;
    virtual NativeNarrowString make_string_0041e870(const char* text) = 0;
    virtual void client_007349c0(void* client, const NativeNarrowString& key, NativeNarrowString& out,
                                 std::int32_t arg) = 0;
    virtual void release_string(NativeNarrowString& string) = 0; // pool return when data is set
};
void game_show_high_latency_warning_004ca120(std::int32_t state_5d4, void* client_21e8, HighLatencyWarningHost& host);

// 004CAF10, __fastcall(ECX = vector {data, count}), RET: when count != 0 the
// last 8-byte string's block goes back to the pool and the count drops by 1.
// 004CC8E0, __fastcall(ECX = slot), RET: when the slot holds a string, its
// block goes back to the pool, the 14h-byte node back to 0043B0A0 (ECX =
// 00E175E8; 00E18514 for the body-identical 0046A0C0), slot = 0.
// Same body as 0046A0C0.
void native_string_vector_pop_back_004caf10(
    NativeNarrowString* data, std::uint32_t& count,
    const std::function<void(void* data, std::uint32_t bytes, std::int32_t count)>& pool_return_00bd1510);
// 004CEB40, __fastcall(CL = paused), RET. Stores CL in 00E188AE first. Acts
// only when the game ([00E188A8]) is in state 0Dh (+5D4h) and either +1FE4h
// is 0 or the byte +218Ch is set. Pausing (CL != 0) needs the byte +634h
// clear: 00E188BC = 1, 004CD0F0(1, 0, 0), 005FF0D0 on the HUD (ECX =
// [[00E198C4]+0D8h]), 00A77760(1) on [00F8BBCC]. Resuming: when 00E188BC is
// set, 004CD0F0(0, 0, 0), 005FD440 on the HUD, 00A77760(0), then 00A92C40
// (ECX = input manager 004BEC00, 0.0f); 00E188BC = 0 in either case.
// 004CEC00 (no Ghidra function, no absolute reference; rel32 callers not
// scanned): when 00E188AE is set, a tail jump to 004CEB40 with CL = 1.
struct PlatformPauseGame {
    std::int32_t state_5d4;
    std::int32_t field_1fe4;
    std::uint8_t byte_218c;
    std::uint8_t byte_634;
};
struct PlatformPauseHost {
    virtual ~PlatformPauseHost() = default;
    virtual void set_cinematic_mode_004cd0f0(std::int32_t on, std::int32_t b, std::int32_t c) = 0;
    virtual void hud_pause_open_005ff0d0() = 0;
    virtual void hud_pause_resume_005fd440() = 0;
    virtual void dialogs_set_all_paused_00a77760(std::int32_t paused) = 0;
    virtual void input_update_00a92c40(float seconds) = 0;
};
void game_set_paused_by_platform_004ceb40(bool paused, const PlatformPauseGame& game, std::uint8_t& flag_00e188ae,
                                          std::uint8_t& paused_00e188bc, PlatformPauseHost& host);
void game_reapply_platform_pause_004cec00(const PlatformPauseGame& game, std::uint8_t& flag_00e188ae,
                                          std::uint8_t& paused_00e188bc, PlatformPauseHost& host);

// 004CEC20 / 004CEC40, __fastcall(ECX = vector), RET: resize to 0 (004CB160
// / 004CB220, ECX = vector), then operator delete (00BF6989) of the data.
void vector_tidy_resize_then_free(void* vector, void* const& data_slot, const std::function<void(void* vector)>& resize_to_zero,
                                  const std::function<void(void*)>& free_block);

// 004CDE20, __fastcall(ECX = deque), RET: a deque of 8-byte narrow strings,
// two per block (map +4, map size +8, offset +0Ch, count +10h). When count
// != 0: index = offset + count - 1, block = index >> 1 (wrapped by the map
// size once), element = map[block] + (index & 1)*8; its storage goes back
// to the pool, count--, and offset = 0 when the count reaches 0.
struct NarrowStringDeque {
    NativeNarrowString** map{nullptr};
    std::uint32_t map_size{0};
    std::uint32_t offset{0};
    std::uint32_t count{0};
};
void narrow_string_deque_pop_back_004cde20(
    NarrowStringDeque& deque,
    const std::function<void(void* data, std::uint32_t bytes, std::int32_t count)>& pool_return_00bd1510);

// 004D0140, __fastcall(ECX = tracker), RET: the hint deque at +4: pop_back
// (the 004CDE20 body, inlined) until the count is 0, then _free every map
// block from the last to the first (null ones skipped), _free the map, and
// zero the map pointer and size.
void award_tracker_drain_hint_deque_004d0140(
    NarrowStringDeque& deque,
    const std::function<void(void* data, std::uint32_t bytes, std::int32_t count)>& pool_return_00bd1510,
    const std::function<void(void*)>& free_00bf65ac);

// 004C8310, __thiscall(ECX = vector {data, size, capacity} of counted
// references, n), RET 4: n = max(n, 1); when capacity < n: operator new(n*4),
// copy each element with InterlockedIncrement(+4) (null stays null), release
// every old element (InterlockedDecrement(+4), vtable slot 0 at zero) and
// clear it, _free the old block, data = new block, capacity = n. The size is
// unchanged.
struct CountedReferenceVector {
    void** data{nullptr};
    std::int32_t size{0};
    std::int32_t capacity{0};
};
struct CountedReferenceHost {
    virtual ~CountedReferenceHost() = default;
    virtual void* operator_new(std::uint32_t bytes) = 0;
    virtual void add_ref(void* counted) = 0;
    virtual void release(void* counted) = 0;
    virtual void free_block(void* block) = 0;
};
void counted_reference_vector_reserve_004c8310(CountedReferenceVector& vector, std::int32_t n,
                                               CountedReferenceHost& host);

// 004CE460, __fastcall(ECX = record), RET: _free the vector data at +14h
// then at +4 (each only when non-null) and zero both {data, end, capacity}
// triples.
struct TwoVectorRecord {
    std::array<void*, 3> vector_04{};
    std::uint32_t field_10{0};
    std::array<void*, 3> vector_14{};
};
void two_vector_record_tidy_004ce460(TwoVectorRecord& record, const std::function<void(void*)>& free_00bf65ac);

// XLive calls of the game's online helpers. ECX = [00F8ABE8] (Xenon system
// manager) for the user-slot queries.
struct GameLiveHost {
    virtual ~GameLiveHost() = default;
    virtual bool has_selected_user_00a3e510() = 0;
    virtual std::uint32_t active_user_slot_00a3eac0() = 0;
    virtual std::uint32_t x_user_set_context(std::uint32_t slot, std::uint32_t context, std::uint32_t value) = 0;
    virtual std::uint32_t x_friends_create_enumerator(std::uint32_t slot, std::uint32_t start, std::uint32_t count,
                                                      std::uint32_t* buffer_size, void** handle) = 0;
    virtual std::uint32_t x_enumerate(void* handle, void* buffer, std::uint32_t buffer_size,
                                      std::uint32_t* items_returned, void* overlapped) = 0;
    virtual void close_handle(void* handle) = 0;
    virtual void x_cancel_overlapped(void* overlapped) = 0;
    virtual void free_block(void* block) = 0; // _free
    virtual void* operator_new(std::uint32_t bytes) = 0; // 00BF55BE
    virtual std::uint64_t active_xuid_00a3eb00() = 0;
    // XUSER_STATS_SPEC {view id, column count, column ids}; only the first
    // column id is used by these callers.
    virtual std::uint32_t x_user_read_stats(std::uint32_t title_id, std::uint32_t xuid_count, const std::uint64_t* xuids,
                                            std::uint32_t spec_count, const std::uint32_t spec[3],
                                            std::uint32_t* results_size, void* results, void* overlapped) = 0;
    virtual std::uint32_t x_user_create_stats_enumerator_by_rank(std::uint32_t title_id, std::uint32_t rank_start,
                                                                 std::uint32_t rows, std::uint32_t spec_count,
                                                                 const std::uint32_t spec[3], std::uint32_t* size,
                                                                 void** handle) = 0;
    virtual std::uint32_t x_user_create_stats_enumerator_by_xuid(std::uint32_t title_id, std::uint64_t xuid,
                                                                 std::uint32_t rows, std::uint32_t spec_count,
                                                                 const std::uint32_t spec[3], std::uint32_t* size,
                                                                 void** handle) = 0;};
// 004BC4E0, __fastcall(ECX = game), RET: with a selected user,
// XUserSetContext(slot, 8001h (presence), 4); byte game+22DCh = 1 always.
void game_set_presence_context_004bc4e0(std::uint8_t& presence_set_22dc, GameLiveHost& host);
// 004BC520, __fastcall(ECX = game), RET: XFriendsCreateEnumerator(slot, 0,
// 100, &size, &handle); on success XEnumerate(handle, game+22DEh, 4D54h,
// game+7034h, null) and CloseHandle only when that succeeds too (the handle
// leaks when XEnumerate fails).
void game_create_friends_enumerator_004bc520(void* friends_buffer_22de, std::uint32_t* friends_count_7034,
                                             GameLiveHost& host);
// 004BC7F0, __fastcall(ECX = game), RET: XCancelOverlapped(game+7038h),
// _free and clear the buffer at +7054h, zero +7058h.
// 004BC580, __thiscall(ECX = game, view, unused, unused), RET 0Ch: _free
// the result buffer +7054h, +7058h = 2, then the xuid list: each friend's
// xuid (game+22DEh, stride 0C4h, +7034h entries) and the active xuid last.
// XUserReadStats(0, n + 1, xuids, 1, {view, 1 column, column 0}, &size,
// null, null); on 7Ah (ERROR_INSUFFICIENT_BUFFER) a size-byte buffer goes to
// +7054h, the overlapped block +7038h..+7050h is zeroed, and the read is
// repeated into the buffer with a NULL overlapped pointer (synchronous).
struct LeaderboardFields {
    std::uint8_t* friends_22de;
    std::uint32_t* friends_count_7034;
    std::array<std::uint32_t, 7>* overlapped_7038;
    void** buffer_7054;
    std::uint32_t* state_7058;
};
void game_begin_friends_leaderboard_read_004bc580(const LeaderboardFields& game, std::uint32_t view, GameLiveHost& host);
// 004BC6D0, __thiscall(ECX = game, rank_start, xuid_low, xuid_high, view,
// unused, unused, u8 friends), RET 1Ch. With `friends`: 004BC520, then
// 004BC580(view, arg5, arg6). Otherwise: _free +7054h; with rank_start != 0
// XUserCreateStatsEnumeratorByRank(0, rank_start, 17, 1, {view, 0 columns}),
// else ByXuid(0, xuid, 17, ...) (results ignored); then a size-byte buffer
// at +7054h, the overlapped block zeroed, XEnumerate(handle, buffer, size,
// null, +7038h) asynchronously, +7058h = 2. The enumerator handle is not closed.
void game_begin_leaderboard_read_004bc6d0(const LeaderboardFields& game, std::uint32_t rank_start, std::uint64_t xuid,
                                          std::uint32_t view, bool friends, GameLiveHost& host);void game_cancel_leaderboard_read_004bc7f0(void* overlapped_7038, void*& buffer_7054, std::uint32_t& field_7058,
                                           GameLiveHost& host);

// 004BB3E0, __fastcall(ECX = game), RET: the eight lobby slot records at
// +1034h (stride 118h, first dword = slot state). For the modes 0..3
// (game+614h) a state of 3 becomes 1; for any other mode a state of 1
// becomes 3.
void game_reset_lobby_slots_for_mode_004bb3e0(std::int32_t mode_614, std::array<std::int32_t*, 8> slot_states);

// 004C6B00, __fastcall(ECX = game), RET: game+60Ch = (game+60Ch + 1) %
// game+5F0h (CDQ/IDIV, signed), then 004C6890 (ECX = game, index).
void game_select_next_scene_record_004c6b00(std::int32_t& index_60c, std::int32_t count_5f0,
                                            const std::function<void(std::int32_t)>& select_004c6890);

// 004C7070, __cdecl(a, b, c, fmt, ...), and 004C70C0, __cdecl(a, b, c, d,
// fmt, ...): _vsprintf_s (00BF7289) into a stack buffer (count 0FFh /
// 0FFCh), then "%s\n" (00CE3A30) through 004B8410 / 004B8450 (same counts) into a
// second buffer, then 004C14C0 (the diagnostic sink getter) with no
// arguments. The text is never handed to the sink: the release build formats
// and drops it. The leading arguments are not read. The host returns the
// composed text and throws where the CRT would call its invalid-parameter
// handler (text too long for a buffer).
std::string diagnostic_printf_004c7070(const std::function<void()>& sink_get_004c14c0, const char* format, ...);
std::string diagnostic_printf_large_004c70c0(const std::function<void()>& sink_get_004c14c0, const char* format, ...);

// 004CC390, __thiscall(ECX = dst, src), RET 4, EAX = dst: copy construction
// of the 60h-byte entry (byte, dword, 004C8DD0 on +8, dwords +10h..+5Ch).
LocalisedEntry60& localised_entry_copy_construct_004cc390(
    LocalisedEntry60& dst, const LocalisedEntry60& src,
    const std::function<void(NativeWideString& self, std::uint32_t length, std::int32_t flag)>& size_004c53e0);

// 004CC650, __fastcall(ECX = record), RET, under an EH frame: 004C8F00 on the
// member at +8, then the narrow string at +0/+4 back to the pool.
void named_vector_record_destruct_004cc650(
    void* record, const std::function<void(void* member_08)>& destruct_004c8f00,
    const std::function<void(void* data, std::uint32_t bytes, std::int32_t count)>& pool_return_00bd1510);

// 004CF590, __thiscall(ECX unused, first, last), RET 8: 004CC760 with ECX =
// first, EDX = last (its two stack arguments, the incoming ECX and `last`,
// are not read). Called from catch blocks.
void reference_pointer_range_release_forward_004cf590(
    void* first, void* last, const std::function<void(void* first, void* last)>& release_004cc760);

void native_string_pointer_delete_004cc8e0(
    NativeNarrowString*& slot,
    const std::function<void(void* data, std::uint32_t bytes, std::int32_t count)>& pool_return_00bd1510,
    std::uint32_t node_pool, const std::function<void(std::uint32_t pool, void* node)>& node_pool_return_0043b0a0);

// ---------------------------------------------------------------------------
// Packet cc13-left3: bodies left open by sys4b. Evidence: docs/CC13_LEFT_3.md.
// ---------------------------------------------------------------------------

// Lua objects are opaque handles owned by the host (B65F50 construct, B67700 destruct).
using LuaRef = std::uintptr_t;
struct DialogScriptLuaHost {
    virtual ~DialogScriptLuaHost() = default;
    virtual bool is_table_00b661b0(LuaRef object) = 0;
    virtual bool is_nil_00b65fb0(LuaRef object) = 0;
    virtual LuaRef get_by_name_00b67800(LuaRef table, const char* name) = 0;
    virtual void assign_00b67690(LuaRef& target, LuaRef source) = 0;
    virtual std::string get_string_00b662b0(LuaRef object) = 0;
    // 00B67080 / 00B67190 (table, key out, value out); 00B66420 tests the key for kind 0.
    virtual void iterate_first_00b67080(LuaRef table, LuaRef& key, LuaRef& value) = 0;
    virtual void iterate_next_00b67190(LuaRef table, LuaRef& key, LuaRef& value) = 0;
    virtual bool is_unbound_00b66420(LuaRef key) = 0;
    virtual void construct_base_0075b430(std::uint32_t type) = 0;
};
// The E1 message (vtable 00CE4B2C; slots WriteBody/ReadBody/TypeEquals).
struct SessionMessageE1 {
    std::uint32_t word_04{0};
    std::string name_18{};
    std::int32_t value_20{0};
    std::string script_24{};
};
// 0044C970, __thiscall(ECX = message, const NativeString* name, LuaObject table (14h, by
// value, destroyed by the callee), int value), RET 1Ch; sole caller BSP_LuaBinding_StartDialog.
// script = 'Dialogues = { ["' name '"]={["sequence"]={' then, when the table is a table, for each
// entry of table.sequence that is a table with a non-nil "message": '{ ["message"]= "' message
// '",},', and finally '},},};'.
void session_message_e1_construct_0044c970(SessionMessageE1& message, const std::string& name,
                                           LuaRef table, std::int32_t value, DialogScriptLuaHost& host);

// The render-frame calls of 004CAC30 and 004CCBD0.
struct GameFrameHost {
    virtual ~GameFrameHost() = default;
    virtual void front_end_screens_pump_004f8830(float dt) = 0;
    virtual void* gui_manager_004c12b0() = 0;
    virtual void gui_manager_update_00aa4f80(void* manager, float dt, std::uint32_t flag) = 0;
    virtual std::uint32_t& scene_begun_34() = 0;                  // [game+34h]
    virtual std::uint32_t render_queue_retention_00b1bf90() = 0;  // on 004C11F0
    virtual void renderer_begin_0c() = 0;                         // [00F8D394] vtable +0Ch
    virtual void game_render_004ca440() = 0;
    virtual void game_finish_render_frame_004ca1f0() = 0;
    // 004CCBD0
    virtual void* camera_rig_a8() = 0;                            // [[game+19F0h]+A8h]
    virtual bool camera_rig_alternate_cc(void* rig) = 0;          // byte +CCh
    virtual void* camera_rig_view(void* rig, bool alternate) = 0; // +D0h or +C8h
    virtual void* camera_node_19fc() = 0;
    virtual void foliage_build_visible_set_00af0c50(void* camera, void* view) = 0;  // on [00F8C274]
    virtual void foliage_frame_counter_increment_20() = 0;        // [[00F8C274]+20h] += 1
    virtual void shadow_driver_virtual_0c(float value) = 0;       // [00F88C20] vtable +0Ch
    virtual void shadow_driver_virtual_04() = 0;                  // [00F88C20] vtable +4h
    virtual void* light_shadow_map_owner_00b7aab0(void* camera) = 0;   // on [game+19F8h]
    virtual void shadow_map_update_from_camera_00a8f3b0(void* owner) = 0;
    virtual void submit_main_scene_0068a0d0() = 0;                // ECX = [00E198C4]
};
// 004CAC30, no register or stack input, plain RET: 004F8830(0.0), 00AA4F80(GUI manager, 0.0,
// 0), the scene begin when [game+34h] is 0 and the queue retains nothing, then 004CA440 and a tail
// JMP to 004CA1F0.
void game_pump_front_end_frame_004cac30(GameFrameHost& host);
// 004CCBD0, __fastcall(ECX = game), returns AL = 1.
bool game_render_foliage_and_shadows_004ccbd0(GameFrameHost& host);

// 004CB420, __fastcall(ECX = block), returns the block: the 288h-byte environment block's
// defaults. The scalar stores (118, including the zeroing of the string fields) and the 85
// global stores (00E18804..00E18988, the terrain shader globals) are tables resolved from the
// decompile and checked offset for offset against the listing (local/l3_envcheck.py).
struct EnvironmentBlockStore {
    std::uint16_t offset;
    std::uint8_t width;   // 1 or 4
    std::uint32_t bits;
};
struct EnvironmentGlobalStore {
    std::uint32_t address;
    std::uint32_t bits;
};
extern const std::array<EnvironmentBlockStore, 118> kEnvironmentBlockDefaults_004cb420;
extern const std::array<EnvironmentGlobalStore, 85> kEnvironmentGlobalDefaults_004cb420;
struct EnvironmentBlockImage {
    std::array<std::uint8_t, 0x288> bytes{};
    std::vector<std::pair<std::uint16_t, std::string>> strings{};  // NativeString fields by offset
};
struct EnvironmentGlobalsHost {
    virtual ~EnvironmentGlobalsHost() = default;
    virtual void store_dword(std::uint32_t address, std::uint32_t bits) = 0;
    virtual void assign_string_00e18990(const std::string& text) = 0;
};
void scene_environment_block_construct_004cb420(EnvironmentBlockImage& block, EnvironmentGlobalsHost& host);

// 004CCCC0, __fastcall(ECX = game), RET: leave the session and tear down the mission objects.
struct LeaveSessionHost {
    virtual ~LeaveSessionHost() = default;
    virtual std::int32_t session_mode_1fe4() = 0;
    virtual void session_set_mode_0076fad0(std::int32_t mode) = 0;   // ECX = game+1EF0h
    // Each slot: when non-null, the virtual named by slot (0 or 0Ch) with 1, then cleared.
    virtual void delete_slot(std::uint32_t where, std::uint32_t vtable_slot) = 0;
    virtual void class_registry_clear_globals_004a9ac0() = 0;
    // [00E19900]: 006B9380 then _free, cleared.
    virtual void award_registry_destroy_00e19900() = 0;
    // [game+7188h]: its string (+14h/+18h) back to the pool, _free, cleared.
    virtual void free_record_7188() = 0;
    virtual void singleton_00f87440_destroy_004c0e40() = 0;
    virtual void singleton_00f8753c_destroy_004c0ef0() = 0;
    virtual void clear_byte_719c() = 0;
    // 00415350 then 00BCFCA0([00F8A304]); then [00F8A304] slot 0 (1) and cleared.
    virtual void singleton_lifetime_unregister_00f8a304() = 0;
    virtual void allocation_report_00be3fd0(const char* caption) = 0;
};
void game_leave_session_004cccc0(LeaveSessionHost& host);

// 004CD450, __fastcall(ECX = game), unreferenced: +1940h is incremented until
// "c:\screenshot_<n>.png" does not open with fopen "rb", then [00F8D394] vtable +110h(path).
struct ScreenshotHost {
    virtual ~ScreenshotHost() = default;
    virtual bool file_exists_fopen_rb(const std::string& path) = 0;   // _fopen != null, then _fclose
    virtual void renderer_save_screenshot_110(const std::string& path) = 0;
};
void game_save_next_screenshot_004cd450(std::int32_t& counter_1940, ScreenshotHost& host);

// 004C9EC0, __fastcall(ECX = game), RET: the scene's lighting resource ("AllLights", 3Ch, made
// when 00B72110 finds none), the ambient light +19F4h (98h, 00B7C290) and the directional light
// +19F8h ("TestDirectionalLight", 1ECh pool 00B7BD40) when missing; the owner's ambient set
// (00B825D0), the directional light's vtable +38h with the identity matrix, 004BACD0(ambient,
// directional, [game+5FCh]) and the directional light's vtable +50h(owner, 1).
struct DefaultLightingHost {
    virtual ~DefaultLightingHost() = default;
    virtual void* scene_lighting_owner_00b72110() = 0;                     // on [game+19ECh]
    virtual void* scene_resource_create_all_lights() = 0;                  // new 3Ch, "AllLights"
    virtual void scene_set_resource_00b723f0(void* owner) = 0;
    virtual void release(void* object) = 0;
    virtual void*& ambient_light_19f4() = 0;
    virtual void* ambient_light_create_00b7c290() = 0;                     // new 98h
    virtual void owner_set_ambient_00b825d0(void* owner, void* ambient) = 0;
    virtual void*& directional_light_19f8() = 0;
    virtual void* directional_light_create_00b7c6b0(const std::string& name) = 0;  // pool 1ECh
    virtual void light_virtual_38(void* light, const float matrix[16]) = 0;
    virtual void* lighting_configuration_5fc() = 0;
    virtual void lighting_apply_004bacd0(void* ambient, void* directional, void* configuration) = 0;
    virtual void light_virtual_50(void* light, void* owner, std::uint32_t flag) = 0;
};
void scene_ensure_default_lighting_004c9ec0(DefaultLightingHost& host);

// 00471080, __fastcall(ECX = entity), RET; vtable slot +A0h of the anim entity: base 0048DBD0,
// +18Ch = 00B6DC90([[[e+178h]+160h]+0Ch], 0), +190h = U(0, +18Ch) from 00BD2F10 with ECX = 0.
// The draw is on stream 0, the shared gameplay stream (census site 004710BB, PRESENTATION).
struct AnimEntityInitHost {
    virtual ~AnimEntityInitHost() = default;
    virtual void sentity_init_base_0048dbd0() = 0;
    virtual float animation_length_00b6dc90(std::uint32_t arg) = 0;
    virtual float random_uniform_00bd2f10_stream0(float lo, float hi) = 0;
};
void anim_entity_sentity_init_00471080(float& length_18c, float& phase_190, AnimEntityInitHost& host);
// 00479BD0, __fastcall(ECX = entity), RET: base 0048DBD0, then +18Ch = 0.0.
void vehicle_visual_entity_sentity_init_00479bd0(float& field_18c, AnimEntityInitHost& host);

// The scene name hash: table +4h count, +8h 64 bucket heads; node next +0Ch. Iterator +0h table,
// +4h node, +8h bucket index. Both are __fastcall(ECX = iterator), RET.
struct SceneNameHashNode {
    const SceneNameHashNode* next_0c{nullptr};
};
struct SceneNameHashTable {
    std::uint32_t count_04{0};
    // Bucket heads; the image reads one slot past the 64 (table +108h) when every bucket from
    // 1 on is empty, which a non-zero count rules out.
    std::array<const SceneNameHashNode*, 65> buckets_08{};
};
struct SceneNameHashIterator {
    const SceneNameHashTable* table_00{nullptr};
    const SceneNameHashNode* node_04{nullptr};
    std::uint32_t bucket_08{0};
};
void scene_name_hash_iterator_begin_00466bb0(SceneNameHashIterator& it);
bool scene_name_hash_iterator_next_00466c00(SceneNameHashIterator& it);

// 00467C80, __thiscall(ECX = value, const float* xyz, float w), RET 8.
struct ScenePropertyVector3Value {
    std::uint8_t byte_00{0};
    std::uint8_t byte_01{0};
    std::uint16_t word_02{0};
    std::uint32_t word_04{0};
    float xyz_08[3]{};
    float w_14{0.0f};
};
void scene_property_value_construct_vector3_00467c80(ScenePropertyVector3Value& value, const float* xyz, float w);

} // namespace bsp::cc13_sys

// oct18b-nA1: newly defined, assembly-reviewed leaves; build-only interfaces.

namespace bsp::cc13_sys {
// ECX game, stack float fov, RET 4; only calls 00B6FBB0 when +19FCh is set.
void game_camera_set_fov_004b4920(void* game, float fov,
    const std::function<void(void*, float)>& camera_set_fov_00b6fbb0);
// RET 4 with no body effects. Data slot
// 00CE7484; relation to the adjacent world-view-job vtable remains provisional.
void world_view_job_empty_callback_004b5070(void* argument) noexcept;
// ECX object, flags stack, RET 4, EAX original object. Clears [00E18AD8].
void* world_view_job_scalar_delete_004b8a40(void* object, std::uint32_t flags, void*& global_00e18ad8,
    const std::function<void(void*)>& free_00bf65ac);
struct NearestViewHost {
    virtual ~NearestViewHost() = default;
    virtual void* game_00e188a8() = 0;
    virtual void refresh_transform_00b6db70(void* camera) = 0;
};
// ECX game, stack xyz, RET 4, ST0 = float-rounded nearest squared distance.
// Scans camera +120h and 8 optional objects at game+18CCh..18E8h, point +ACh.
float game_nearest_view_distance_squared_004bb970(void* game, const float point[3], NearestViewHost& host);
}
