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

} // namespace bsp::cc13_sys
