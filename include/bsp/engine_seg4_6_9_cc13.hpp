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
bool point_projects_onto_segment_00470150(const std::array<float, 3>& p, const std::array<float, 3>& a,
                                          const std::array<float, 3>& b, const GeometryLength& length);

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

} // namespace bsp::cc13_sys
