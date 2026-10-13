#include "bsp/engine_seg4_6_9_cc13.hpp"

#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "bsp/gamepad_force_events.hpp"
#include "bsp/native_bit_cursor_owned_string.hpp"
#include "bsp/native_bit_cursor_read.hpp"
#include "bsp/native_bit_cursor_write.hpp"

namespace bsp::cc13_sys {

// Constructor, type (+10h) and vtable, read from each constructor's listing
// (MOV [ESI],vtable; MOV [ESI+10h],type after CALL 00465B60).
const std::array<MovieActionConstructorRow, 31> kMovieActionConstructors{{
    {0x00465C40, 0x00, 0x00CE54A4}, // sSetTarget
    {0x00465C70, 0x01, 0x00CE54AC}, // cSetParent
    {0x00465CD0, 0x02, 0x00CE54BC}, // pEngineFailure
    {0x00465DB0, 0x03, 0x00CE54D8}, // pForceRelease
    {0x00465CA0, 0x05, 0x00CE54B4}, // cUnParent
    {0x00465DE0, 0x06, 0x00CE54E0}, // ForceDepthCharge
    {0x00465E10, 0x07, 0x00CE54E8}, // SetGunFire
    {0x00465E40, 0x08, 0x00CE54F0}, // TogglePeriscope
    {0x00465E70, 0x09, 0x00CE54F8}, // ForceArtilleryAngle
    {0x00465EA0, 0x0A, 0x00CE5500}, // ForceArtilleryTarget
    {0x00465ED0, 0x0B, 0x00CE5508}, // ForceTorpedo
    {0x00465F00, 0x0C, 0x00CE5510}, // SetGunFireEnabled
    {0x00465F30, 0x0D, 0x00CE5518}, // SetGunFireDisabled
    {0x00465F60, 0x0E, 0x00CE5520}, // SetGunFireTargetOnly
    {0x00465F90, 0x0F, 0x00CE5528}, // TurnGun
    {0x00465FC0, 0x10, 0x00CE5530}, // SetDepth
    {0x00465FF0, 0x11, 0x00CE5538}, // ForceAAFlak
    {0x00466020, 0x12, 0x00CE5540}, // cFOV
    {0x00466080, 0x13, 0x00CE5550}, // cBlackOut
    {0x00466170, 0x14, 0x00CE5558}, // aParticleEffect (+44h = 0)
    {0x004661C0, 0x15, 0x00CE5560}, // DontMove
    {0x00466050, 0x16, 0x00CE5548}, // cFOV_Now
    {0x004661F0, 0x17, 0x00CE5568}, // pSetGear
    {0x00466220, 0x18, 0x00CE5570}, // CameraShake
    {0x00466250, 0x19, 0x00CE5578}, // Binocular
    {0x00466280, 0x1A, 0x00CE5580}, // TakeOffHack
    {0x004662B0, 0x1B, 0x00CE5588}, // WingTipEffect
    {0x004662E0, 0x1C, 0x00CE5590}, // MoveToPoint
    {0x00466310, 0x1D, 0x00CE5598}, // WideBlackBars
    {0x00466340, 0x1E, 0x00CE55A0}, // UseCatapult
    {0x00466370, 0x1F, 0x00CE55A8}, // HideEntity
}};

std::optional<MovieAction> construct_movie_action(std::int32_t type, float activate_time) {
    for (const auto& row : kMovieActionConstructors) {
        if (row.type != type) continue;
        // 00465850: +4 time, +8 empty comment. 00465B60: +14h = 0, the strings
        // at +18h and +2Ch empty, +20h..+28h and +34h..+3Ch zero.
        MovieAction action;
        action.type = type;
        action.activate_time = activate_time;
        return action;
    }
    return std::nullopt;
}

void movie_engine_failure_run_00465d00(
    const std::function<void(const char* effect_name, std::int32_t arg)>& entity_slot_194) {
    // 00465D18..00465D4A: NativeString "explosion_delayed" (11h bytes), then
    // 00465D52..00465D6D: [ESP+18h] (the entity) -> vtable +194h(&name, 0).
    entity_slot_194("explosion_delayed", 0);
}

void movie_particle_effect_stop_004661a0(void*& handle_at_44, const std::function<void(void*)>& release) {
    if (handle_at_44) release(handle_at_44); // 004661AA CALL 008673B0
    handle_at_44 = nullptr;                  // 004661AF
}

// Create slots: listing constants (operator new size, the three vtable stores,
// MOV [ESI+C4h],id). Table names and entity bytes agree with
// docs/CC13_SCENE_BROWSER.md.
const std::array<VisualEntityKind, 5> kVisualEntityKinds{{
    {0x00470C90, 0x004707A0, "AnimClass", 0x194, 0x3E, 0x00CE5A20, 0x00CE5A08, 0x00CE5A00, false},
    {0x004728F0, 0x004724D0, "Bullets", 0x18C, 0x39, 0x00CE5B90, 0x00CE5B78, 0x00CE5B70, true},
    {0x00474160, 0x00473DB0, "BuoyClass", 0x18C, 0x3C, 0x00CE5CF0, 0x00CE5CD8, 0x00CE5CD0, false},
    {0x00476360, 0x004762D0, "CloudClass", 0x1AC, 0x3D, 0x00CE5E68, 0x00CE5E50, 0x00CE5E48, true},
    {0x00479FE0, 0x00479B70, "VehicleClass", 0x190, 0x38, 0x00CE6130, 0x00CE6114, 0x00CE610C, false},
}};

bool create_visual_entity(std::uint32_t create_entity, const void* cls, VisualEntityClassLink& link,
                          const std::function<void()>& push_pending) {
    for (const auto& kind : kVisualEntityKinds) {
        if (kind.create_entity != create_entity) continue;
        link.class_id_c4 = kind.class_id;
        link.class_170 = cls; // 0047B9E0
        link.class_188 = cls;
        if (kind.pushes_pending) push_pending(); // 00926BE0
        return true;
    }
    return false;
}

SourcedVisualClassFields construct_sourced_visual_class(const void* source, const char* source_name_at_14) {
    SourcedVisualClassFields fields;
    fields.source = source;
    fields.name = source_name_at_14 ? source_name_at_14 : "";
    return fields;
}

void clear_singleton_pointer(SingletonSlotView slot) {
    *slot.global = nullptr;      // MOV [global],0
    *slot.vtable = 0x00CE3818;   // MOV [ECX],00CE3818
}

std::uint32_t entity_visual_class_field_00476b90(const void* visual_at_360,
                                                 const std::function<std::uint32_t(const void*)>& field_160) {
    return visual_at_360 ? field_160(visual_at_360) : 0;
}

std::array<float, 3> project_onto_axis_0046fdd0(const std::array<float, 3>& v, const std::array<float, 3>& axis,
                                                 const std::function<float(const std::array<float, 3>&)>& length) {
    const float s = length(axis);                   // 0046FDE0 CALL 00419440, FSTP float
    const float inv = s > 0.0f ? 1.0f / s : 0.0f;   // 0046FDEF FCOMI; JBE also takes NaN
    const std::array<float, 3> n{axis[0] * inv, axis[1] * inv, axis[2] * inv}; // 0046FE0A..0046FE28
    // 0046FE2C..0046FE55: (v1*n1 + v0*n0) + v2*n2, one rounding to float.
    const double dot = (static_cast<double>(v[1]) * n[1] + static_cast<double>(v[0]) * n[0])
                       + static_cast<double>(v[2]) * n[2];
    const float d = static_cast<float>(dot);
    return {n[0] * d, n[1] * d, n[2] * d};         // 0046FE59..0046FE70
}

std::string format_to_buffer32_00467310(const char* format, ...) {
    char buffer[0x20];
    va_list args;
    va_start(args, format);
    const int count = std::vsnprintf(buffer, sizeof buffer, format, args);
    va_end(args);
    // _vsprintf_s reports truncation through the invalid-parameter handler.
    if (count < 0 || count >= static_cast<int>(sizeof buffer))
        throw std::length_error("formatted text does not fit 20h bytes");
    return std::string(buffer, static_cast<std::size_t>(count));
}

const void* scene_node_child_at_00467f30(const void* head, std::int32_t count, std::int32_t index,
                                         const std::function<const void*(const void* node)>& next_at_44) {
    if (index < 0 || count <= index) return nullptr;
    const void* node = head;
    if (!node) return nullptr;
    while (index != 0) {
        node = next_at_44(node);
        --index;
        if (!node) return nullptr;
    }
    return node;
}

std::uint32_t linked_list_nth_payload_004be940(const PayloadNode* head_at_4, std::int32_t index) {
    const PayloadNode* node = head_at_4;
    for (; index > 0; --index) {
        if (!node) return 0;
        if (!node->next) return node->payload; // 004BE96D: stop on the last node
        node = node->next;
    }
    return node ? node->payload : 0;
}

bool command_line_matches_token_004c2df0(const std::string& text, const char* token) {
    return text.compare(0, text.size(), token) == 0;
}

void screenshot_set_base_name_004c6820(ScreenshotNaming& state, const std::string* name,
                                       const std::function<void()>& begin_sequence) {
    if (name != &state.base_name_e18998) state.base_name_e18998 = *name; // 004C6835..004C6850
    state.index_e08210 = -1;                                             // 004C685F
    if (state.sequence_enabled_e188ad) begin_sequence();                 // 004C686C JMP 004BFBF0
}

void on_quit_confirmation_004bbc50(std::int32_t result, QuitConfirmationFlags& flags) {
    flags.dialog_open_180 = false;
    if (result == 1) flags.quit_requested_e1ae75 = true;
}

void drain_state_requests_004c88a0(DwordDequeHeader& deque) {
    while (deque.count_5e8 != 0) {
        ++deque.head_5e4;
        if (deque.map_size_5e0 * 4 <= deque.head_5e4) deque.head_5e4 = 0;
        if (--deque.count_5e8 == 0) deque.head_5e4 = 0;
    }
}

void construct_slot_unit_004c6750(SceneRecordSlotUnitHead& unit) {
    unit.field_00 = 0;
    unit.field_04 = 0;
    unit.field_10 = 0;
    unit.field_14 = 0;
}

bool session_message_e1_type_equals_0044ccd0(std::uint32_t type) {
    return type == 0xE1;
}

void write_session_message_e1_00449a70(const NativeSessionMessageE1Fields& m, NativeBitCursor* cursor,
                                       const char* empty_string_backing_00e17669) {
    write_native_byte_bits_00428ff0(cursor, *m.type_10, 8);                    // 00449A81
    write_native_owned_string_00429ac0(cursor, m.text_18, empty_string_backing_00e17669); // 00449A8C
    write_native_byte_bits_00428ff0(cursor, *m.small_20, 2);                   // 00449A9A
    write_native_owned_string_00429ac0(cursor, m.text_24, empty_string_backing_00e17669);
}

void read_session_message_e1_00449ab0(const NativeSessionMessageE1Fields& m, NativeBitCursor* stream_cursor_04,
                                      NativeStringRawPoolContext& pool) {
    read_native_u8_bits_00428c70(stream_cursor_04, m.type_10, 8);   // 00449AC3
    read_native_owned_string_00429f20(stream_cursor_04, m.text_18, pool); // 00449ACE
    read_native_u8_bits_00428c70(stream_cursor_04, m.small_20, 2);  // 00449ADB
    read_native_owned_string_00429f20(stream_cursor_04, m.text_24, pool); // 00449AE6
}

// ---------------------------------------------------------------------------
// Packet cc13-sys4: lazy singleton getters.
// ---------------------------------------------------------------------------

// Global, size, constructor and registered offset from each getter's
// listing (PUSH size; CALL 00BF681B; CALL ctor; MOV [global],EAX; the
// offset added before CALL 00BD0C30).
const std::array<LazySingletonGetterRow, 13> kLazySingletonGetters{{
    {0x00472200, 0x00E18688, 0x10, 0x00472100, 0, 0, 0, 0},   // AnimClass registry
    {0x00473AF0, 0x00E18690, 0x10, 0x004739F0, 0, 0, 0, 0},   // Bullets visual class registry
    {0x00475640, 0x00E18698, 0x10, 0x00475540, 0, 0, 0, 0},   // BuoyClass registry
    {0x004798A0, 0x00E186A0, 0x10, 0x004797A0, 0, 0, 0, 0},   // CloudClass registry
    {0x0047B2F0, 0x00E186DC, 0x10, 0x0047B1F0, 0, 0, 0, 0},   // VehicleClass registry
    {0x004C0960, 0x00E18AD8, 0x08, 0, 4, 0x00CE7488, 0x00CE753C, 0x00CE7538}, // world-view job
    {0x004C0A30, 0x00E18ADC, 0x08, 0, 4, 0x00CE748C, 0x00CE7544, 0x00CE7540}, // camera-update job
    {0x004C17D0, 0x00E17620, 0x14, 0x00424730, 0, 0, 0, 0},   // avoid-zone registry
    {0x004C1AC0, 0x00E18D80, 0x164, 0x00517D10, 8, 0, 0, 0},  // front-end frame screen
    {0x004C1B90, 0x00F89B34, 0x28, 0x00940DD0, 0, 0, 0, 0},   // sound request queue
    {0x004C1C50, 0x00F8AAFC, 0xFD8, 0x00A37130, 0, 0, 0, 0},  // game singleton 00F8AAFC
    {0x004C1D10, 0x00E19A64, 0x50, 0x004BF9A0, 0, 0, 0, 0},   // game singleton 00E19A64
    {0x004C1DD0, 0x0109DB50, 0x30, 0x00BE3820, 0, 0, 0, 0},   // profiler
}};

const LazySingletonGetterRow* find_lazy_singleton_getter(std::uint32_t getter) {
    for (const auto& row : kLazySingletonGetters) {
        if (row.getter == getter) return &row;
    }
    return nullptr;
}

void* lazy_singleton_get(const LazySingletonGetterRow& row, void*& global, LazySingletonHost& host) {
    if (global == nullptr) {
        host.lock();
        if (global == nullptr) {
            void* memory = host.operator_new(row.allocation_size);
            if (memory == nullptr) {
                global = nullptr;
            } else if (row.constructor != 0) {
                global = host.construct(row.constructor, memory);
            } else {
                auto* bytes = static_cast<unsigned char*>(memory);
                std::memcpy(bytes + 4, &row.base_vtable_04, 4);
                std::memcpy(bytes, &row.primary_vtable_00, 4);
                std::memcpy(bytes + 4, &row.lifetime_vtable_04, 4);
                global = memory;
            }
            // The native re-reads the global for the register argument.
            void* registered =
                global == nullptr ? nullptr : static_cast<unsigned char*>(global) + row.registered_offset;
            host.register_lifetime(registered);
        }
        host.unlock();
    }
    return global;
}

void lazy_singleton_destroy(void*& global, LazySingletonDestroyHost& host) {
    if (global == nullptr) return;
    host.lock();
    if (global != nullptr) {
        host.unregister_lifetime(global);
        if (global != nullptr) host.delete_object(global);
        global = nullptr;
    }
    host.unlock();
}

void movie_particle_effect_run_004656d0(void* entity, float start_time_at_20, const void* name_at_2c,
                                        void*& effect_at_44, MovieParticleEffectRunHost& host) {
    void* parent = entity != nullptr ? host.entity_parent(entity) : nullptr;      // 004656E2..004656EC
    if (!host.entity_world_pose_current(entity)) host.refresh_world_pose_00414db0(entity); // 004656EE..004656F9
    std::array<float, 16> matrix{};                                               // 00465720..0046577A
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
    void* handle = host.acquire_effect_handle_00871ba0(name_at_2c, 1);            // 00465780
    void* effect = host.create_point_effect_008687c0(parent, handle, matrix, 1, 0, 0); // 0046578B
    host.release_result_reference(effect);                                        // 00465792..004657B3
    host.clear_effect_byte_09(effect);                                            // 004657BB
    float remaining = start_time_at_20;
    if (remaining > 0.0f) {                                                       // 004657C4 COMISS; JBE
        constexpr double kStep = 0.20000000298023224;                             // 00CE3D10
        do {
            const float step = !(remaining > kStep) ? remaining : 0.2f;           // 004657E5 FCOMIP; JBE
            host.update_children_00867790(effect, step);                         // 00465810
            remaining = static_cast<float>(remaining - kStep);                    // 0046581F DC E9
        } while (remaining > 0.0f);                                               // 0046582D FCOMI; JA
    }
    effect_at_44 = effect;                                                        // 00465837
}

// ---------------------------------------------------------------------------
// Packet cc13-sys4: the x87 geometry cluster.
// ---------------------------------------------------------------------------
namespace {

using F3 = std::array<float, 3>;

float f32(double v) { return static_cast<float>(v); }

F3 sub3(const F3& a, const F3& b) { return {a[0] - b[0], a[1] - b[1], a[2] - b[2]}; }

// (x0*y0 + x1*y1) + x2*y2 on the stack, one rounding at the FSTP.
float dot3(const F3& x, const F3& y) {
    return f32((static_cast<double>(x[0]) * y[0] + static_cast<double>(x[1]) * y[1])
               + static_cast<double>(x[2]) * y[2]);
}

F3 scale3(const F3& v, float s) { return {v[0] * s, v[1] * s, v[2] * s}; }

F3 add3(const F3& a, const F3& b) { return {a[0] + b[0], a[1] + b[1], a[2] + b[2]}; }

// FLDZ; FCOMI; JBE: 1/s only when s > 0 (NaN and zero give 0).
float inverse_or_zero(float s) { return s > 0.0f ? 1.0f / s : 0.0f; }

} // namespace

F3 closest_point_on_line_0046f550(const F3& point, const F3& line_point, const F3& dir, float& distance) {
    const F3 w = sub3(point, line_point);                 // 0046F576..0046F5B8
    const float t = dot3(dir, w);                         // 0046F5D0..0046F5EC
    const F3 out = add3(line_point, scale3(dir, t));      // 0046F5F0..0046F649
    distance = force_event_vector_length_0042b2f0(sub3(point, out)); // inline copy 0046F64C..0046F6CD
    return out;
}

float distance_along_axis_0046f6d0(const F3& point, const F3& origin, const F3& dir) {
    const float t = dot3(dir, sub3(point, origin));       // 0046F6D3..0046F729
    return force_event_vector_length_0042b2f0(scale3(dir, t)); // inline copy 0046F747..0046F7A4
}

F3 project_onto_plane_0046f7b0(const F3& point, const F3& plane_point, const F3& normal, float& distance) {
    const float t = dot3(normal, sub3(point, plane_point)); // 0046F7B7..0046F84C
    const F3 out = sub3(point, scale3(normal, t));          // 0046F850..0046F8A9
    distance = force_event_vector_length_0042b2f0(sub3(out, plane_point)); // 0046F8AC..0046F931
    return out;
}

float closest_points_between_lines_0046f940(const F3& p1, const F3& u, const F3& p3, const F3& v, F3* out_a,
                                            F3* out_b) {
    F3 a{}, b{};
    float distance = 0.0f;
    if (dot3(u, v) == 1.0f) {                              // 0046F943..0046F983
        a = p1;                                            // 0046FBE8..0046FC1F
        b = closest_point_on_line_0046f550(p1, p3, v, distance); // 0046FC25
    } else {
        const float uu = dot3(u, u);                       // 0046F9BB
        const float uv = dot3(u, v);                       // 0046F9CD
        const F3 w = sub3(p1, p3);                         // 0046F9D0..0046FA30
        const float uw = dot3(u, w);                       // 0046FA4C
        const float vw = dot3(v, w);                       // 0046FA93
        const float vv = dot3(v, v);                       // 0046FABE
        // 0046FA97..0046FAD4: both differences stay on the stack.
        const double numerator = static_cast<double>(vw) * uu - static_cast<double>(uw) * uv;
        const double denominator = static_cast<double>(vv) * uu - static_cast<double>(uv) * uv;
        const float s = f32(numerator / denominator);
        const float t = f32(-((uw - static_cast<double>(s) * uv) / uu)); // 0046FAD8..0046FAF1
        a = add3(p1, scale3(u, t));                        // 0046FAF5..0046FB51
        b = add3(p3, scale3(v, s));                        // 0046FB55..0046FBAF
        distance = force_event_vector_length_0042b2f0(sub3(a, b)); // 0046FBB3..0046FBD6
    }
    if (out_a != nullptr) *out_a = a;                      // 0046FC38..0046FC5B
    if (out_b != nullptr) *out_b = b;                      // 0046FC60..0046FC75
    return distance;
}

F3 reject_from_axis_0046fe80(const F3& v, const F3& axis, const GeometryLength& length) {
    const float inv = inverse_or_zero(length(axis));       // 0046FE90..0046FEB4
    const F3 n = scale3(axis, inv);                        // 0046FEBA..0046FED8
    const float d = dot3(v, n);                            // 0046FEDC..0046FF1F
    return sub3(v, scale3(n, d));                          // 0046FF23..0046FF60
}

namespace {

struct SegmentDots {
    F3 pa, bp, n;
    float d1, d2;
};

// The shared prefix of 0046FF70 and 00470150.
SegmentDots segment_dots(const F3& p, const F3& a, const F3& b, const GeometryLength& length) {
    SegmentDots r{};
    const F3 ab = sub3(b, a);
    r.n = scale3(ab, inverse_or_zero(length(ab)));
    r.pa = sub3(p, a);
    r.bp = sub3(b, p);
    r.d1 = dot3(r.pa, r.n);
    r.d2 = dot3(r.bp, r.n);
    return r;
}

} // namespace

float point_segment_distance_0046ff70(const F3& p, const F3& a, const F3& b, const GeometryLength& length) {
    const SegmentDots s = segment_dots(p, a, b, length);
    if (s.d1 < 0.0f) return force_event_vector_length_0042b2f0(s.pa);     // 004700A7 FCOMI; JBE
    if (s.d2 < 0.0f) return force_event_vector_length_0042b2f0(s.bp);     // 004700E8 FCOMIP; JBE
    return force_event_vector_length_0042b2f0(sub3(s.pa, scale3(s.n, s.d1))); // 00470101..0047013B
}

bool point_projects_onto_segment_00470150(const F3& p, const F3& a, const F3& b, const GeometryLength& length) {
    const SegmentDots s = segment_dots(p, a, b, length);
    if (s.d1 < 0.0f) return false;                         // 00470261 FCOMI; JBE
    return !(s.d2 < 0.0f || s.d2 != s.d2);                 // 004702BB FCOMIP; JB (unordered sets CF)
}

// ---------------------------------------------------------------------------
// Packet cc13-sys4: small segment-9 helpers.
// ---------------------------------------------------------------------------

std::uint8_t selected_user_live_byte_004bb600(SelectedUserLiveHost& host) {
    if (host.has_selected_user_00a3e510() && host.selected_sign_in_state_00a3ead0() == 2) // 004BB609..004BB61C
        return host.active_slot_byte_118_00a3eb20();                                        // 004BB629 JMP
    return 0;                                                                               // 004BB61E
}

std::int32_t counted_string_find_004bcb40(std::uint32_t length, const char* data, const char* needle,
                                          std::int32_t start) {
    if (data == nullptr || needle == nullptr) return -1;                          // 004BCB46..004BCB50
    if (start < 0) {
        start = 0;                                                                // 004BCB5A
    } else if (static_cast<std::uint32_t>(start) > length) {
        return -1;                                                                // 004BCB5E CMP; JA
    }
    const char* match = std::strstr(data + start, needle);                        // 004BCB66
    return match != nullptr ? static_cast<std::int32_t>(match - data) : -1;
}

void multiplayer_interface_apply_pending_size_004bdf50(
    const PendingSizeFields& f, const std::function<void(std::int32_t, std::int32_t)>& vslot_10) {
    if (f.current_04 != f.pending_20 || f.current_1c != f.pending_38) vslot_10(f.pending_20, f.pending_38);
}

const std::array<OwnedPointerDeleteRow, 3> kOwnedPointerDeletes{{
    {0x004BEA10, 0x006B9380},
    {0x004BEA40, 0x0045D930},
    {0x004BEA70, 0x009496A0},
}};

void owned_pointer_delete(void*& slot, const std::function<void(void*)>& destroy,
                          const std::function<void(void*)>& free_block) {
    void* object = slot;
    if (object == nullptr) return;
    destroy(object);
    free_block(object);
    slot = nullptr;
}

void* unit_cast_if_class_flag_0f_004bead0(void* unit, const std::function<bool(void*, std::int32_t)>& vslot_5c) {
    if (unit != nullptr && vslot_5c(unit, 0x0F)) return unit;
    return nullptr;
}

void release_session_render_resources_004bfd70(void* block_at_5fc, SessionRenderResourcesHost& host) {
    host.reset_remap_textures_00b0feb0();                                         // 004BFD79
    host.renderer_vslot_124();                                                    // 004BFD8C
    if (block_at_5fc == nullptr) return;
    for (std::uint32_t offset = 0xD5C; offset < 0xD98; offset += 0x14) {          // 004BFD9A..004BFDDA
        void*& counted = *reinterpret_cast<void**>(static_cast<unsigned char*>(block_at_5fc) + offset);
        if (counted != nullptr) host.release_reference(counted);
        counted = nullptr;
    }
}

void construct_observer_slot_004c2d40(ObserverSlotFields& slot) {
    slot.field_04 = slot.field_08 = slot.field_0c = 0;                            // 004C2D44..004C2D4A
    slot.subject_14 = nullptr;                                                    // 004C2D4D
    slot.flag_10 = 1;
    slot.vtable = 0x00CE75CC;
}

void construct_interface_request_004bef00(ObserverSlotFields& request, void* subject,
                                          const std::function<void(void*, void*)>& register_pair) {
    request.field_04 = request.field_08 = request.field_0c = 0;                   // 004BEF1F..004BEF25
    request.vtable = 0x00CE3C8C;
    request.subject_14 = subject;
    request.flag_10 = 1;
    if (subject != nullptr) register_pair(subject, &request);                    // 004BEF43
}

void observer_slot_release_if_subject(ObserverSlotFields& slot, void* notifier,
                                      const std::function<void*(void*)>& notifier_object_04,
                                      const std::function<void(void*, void*)>& unregister_pair) {
    void* object = notifier_object_04(notifier);                                  // 004C2D6C
    if (object != slot.subject_14) return;
    slot.subject_14 = nullptr;                                                    // 004C2D77
    unregister_pair(object, &slot);                                               // 004C2D7E
}

void* wide_string_allocate_storage_004c2fd0(
    std::uint32_t count, const std::function<void*(std::uint32_t, std::int32_t)>& pool_allocate_00bd1120) {
    return pool_allocate_00bd1120(count * 2 + 2, 1);
}

void input_context_level5_004c5140(std::int32_t context, const std::function<void(std::int32_t, std::int32_t)>& set_00a933f0) {
    set_00a933f0(context, 5);
}

void storage_manager_newest_slot_name_004c5e00(void* out, const std::function<void(void*)>& vslot_10) {
    vslot_10(out);
}

NativeWideString& wide_string_assign_004c5e20(
    NativeWideString& self, const NativeWideString& other,
    const std::function<void(NativeWideString&, std::uint32_t, std::int32_t)>& size_004c53e0) {
    if (&self == &other) return self;                                             // 004C5E28
    size_004c53e0(self, other.length, 1);                                         // 004C5E31
    if (other.length == 0) return self;                                           // 004C5E36
    const char16_t* from = other.data;
    char16_t* to = self.data;
    char16_t unit;
    do {                                                                          // 004C5E41..004C5E50
        unit = *from++;
        *to++ = unit;
    } while (unit != 0);
    return self;
}

void profiler_scope_begin_004c5ec0(std::int32_t& scope_index, std::int32_t index, const std::uint32_t& colour,
                                   ProfilerScopeHost& host) {
    host.colour_array_24()[index] = colour;                                       // 004C5ED5
    scope_index = index;                                                          // 004C5ED9
    host.begin_00be3640(index);                                                   // 004C5EE2
}

void profiler_scope_end_004c5ef0(std::int32_t scope_index, ProfilerScopeHost& host) {
    host.end_00be3660(scope_index);
}

void session_message_e4_destruct_004c5f10(std::uint32_t& vtable, std::uint8_t kind_18, void* payload_1c,
                                          SessionMessageE4DestructHost& host) {
    vtable = 0x00CE7608;
    if (kind_18 == 3) host.release_payload_004c0fd0(payload_1c);
    host.destruct_payload_004c1f70(payload_1c);
    vtable = 0x00CE4974;
}

bool session_message_e4_is_id_004c5f40(std::uint32_t id) {
    return id == 0xE4;
}

} // namespace bsp::cc13_sys
