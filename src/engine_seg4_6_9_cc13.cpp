#include "bsp/engine_seg4_6_9_cc13.hpp"

#include <cmath>
#include <limits>
#include <cstdio>
#include <cstring>
#include <vector>
#include <stdexcept>

#include "bsp/camera_decomposition.hpp"
#include "bsp/gamepad_force_events.hpp"
#include "bsp/mission_scene_load.hpp"
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
    float separation = 0.0f;
    if (dot3(u, v) == 1.0f) {                              // 0046F943..0046F983
        a = p1;                                            // 0046FBE8..0046FC1F
        b = closest_point_on_line_0046f550(p1, p3, v, separation); // 0046FC25
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
        separation = force_event_vector_length_0042b2f0(sub3(a, b)); // 0046FBB3..0046FBD6
    }
    if (out_a != nullptr) *out_a = a;                      // 0046FC38..0046FC5B
    if (out_b != nullptr) *out_b = b;                      // 0046FC60..0046FC75
    return separation;
}

F3 reject_from_axis_0046fe80(const F3& v, const F3& axis, const GeometryLength& length) {
    const float inv = inverse_or_zero(length(axis));       // 0046FE90..0046FEB4
    const F3 n = scale3(axis, inv);                        // 0046FEBA..0046FED8
    const float d = dot3(v, n);                            // 0046FEDC..0046FF1F
    return sub3(v, scale3(n, d));                          // 0046FF23..0046FF60
}

void orthonormal_frame_from_axis_004702e0(F3& axis, F3& v, F3& out, const GeometryLength& length) {
    v = reject_from_axis_0046fe80(v, axis, length);                               // 004702EE..00470302
    normalize_camera_basis_0042b260(axis);                                        // 00470305
    normalize_camera_basis_0042b260(v);                                           // 0047030C
    auto diff = [](float p, float q, float r, float s) {
        return f32(static_cast<double>(p) * q - static_cast<double>(r) * s);
    };
    out[0] = diff(axis[1], v[2], axis[2], v[1]);                                  // 00470311..00470323
    out[1] = diff(v[0], axis[2], axis[0], v[2]);                                  // 00470325..00470331
    out[2] = diff(axis[0], v[1], axis[1], v[0]);                                  // 00470334..00470342
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
// Packet cc13-sys4: segment-6 matrix helpers.
// ---------------------------------------------------------------------------

void matrix_multiply_into_0047c210(Matrix4& m, const Matrix4& right, const MatrixMultiply& multiply_00413920) {
    const Matrix4 product = multiply_00413920(m, right);                          // 0047C222
    m = product;                                                                  // 0047C22A 004134F0
}

void transform_init_translation_0047c240(Matrix4& out, const std::array<float, 3>& t) {
    out = Matrix4{};
    out[0] = out[5] = out[10] = out[15] = 1.0f;
    out[12] = t[0];                                                               // 0047C24F
    out[13] = t[1];                                                               // 0047C25E
    out[14] = t[2];                                                               // 0047C26E
}

void matrix_build_from_euler_yxz_0047c2a0(Matrix4& out, const std::array<float, 3>& angles, EulerRotationHost& host) {
    const Matrix4 ry = host.rotation_y_00b646e0(angles[0]);                       // 0047C2B3
    const Matrix4 rx = host.rotation_x_00b64640(angles[1]);                       // 0047C2C1
    const Matrix4 rz = host.rotation_z_00b64780(angles[2]);                       // 0047C2D6
    const Matrix4 zx = host.multiply_00413920(rz, rx);                            // 0047C2DD
    out = host.multiply_00413920(zx, ry);                                         // 0047C2E4
}

void* memory_alloc_zeroed_00470b80(std::uint32_t size, const std::function<void*(std::uint32_t)>& operator_new_00bf55be) {
    void* block = operator_new_00bf55be(size);                                    // 00470B85
    std::memset(block, 0, size);                                                  // 00470B90, no null test
    return block;
}

void class_entity37_velocity_00470ba0(std::array<float, 3>& out, const std::array<float, 3>& static_00f87574) {
    out = static_00f87574;
}

void scene_object_forward_slot_d8_00472540(void* object, const std::function<void(void*)>& vslot_d8) {
    vslot_d8(object);
}

void observer_link_destroy_base_0047bec0(ObserverSlotFields& link,
                                         const std::function<void(void*, void*)>& unregister_006952a0,
                                         const std::function<void(void*)>& destroy_callback_owner_00695870) {
    link.vtable = 0x00CE6430;                                                     // 0047BEDD
    if (link.subject_14 != nullptr) unregister_006952a0(link.subject_14, &link);  // 0047BEF4
    destroy_callback_owner_00695870(&link);                                       // 0047BF03
}

void node_or_flags_recursive_00475e60(void* node, std::uint32_t mask) {
    auto* bytes = static_cast<unsigned char*>(node);
    void* child = nullptr;
    std::memcpy(&child, bytes + 0x34, sizeof child);
    std::uint32_t flags = 0;
    std::memcpy(&flags, bytes + 0x48, sizeof flags);
    flags |= mask;
    std::memcpy(bytes + 0x48, &flags, sizeof flags);
    while (child != nullptr) {
        node_or_flags_recursive_00475e60(child, mask);
        std::memcpy(&child, static_cast<unsigned char*>(child) + 0x3C, sizeof child);
    }
}

RawListNode* raw_list_erase_node_00467b50(std::int32_t& count, RawListNode*& head, RawListNode* node,
                                          const std::function<void(void*)>& free_00bf65ac) {
    if (node->prev == nullptr) head = node->next;
    else node->prev->next = node->next;
    RawListNode* next = node->next;
    if (next != nullptr) next->prev = node->prev;
    --count;
    free_00bf65ac(node);
    return next;
}

void raw_list_clear_storage_004683b0(std::int32_t& count, RawListNode*& head, RawListNode*& tail,
                                     const std::function<void(void*)>& free_00bf65ac) {
    while (count != 0) {
        RawListNode* node = head;
        if (node->prev == nullptr) head = node->next;
        else node->prev->next = node->next;
        if (node->next == nullptr) tail = node->prev;
        else node->next->prev = node->prev;
        --count;
        free_00bf65ac(node);
    }
}

void pod_pointer_array_resize_00468050(PodPointerArray& array, std::int32_t n,
                                       const std::function<void(PodPointerArray&, std::int32_t)>& reserve_004674f0) {
    if (n > array.capacity) reserve_004674f0(array, n);                           // 00468058..0046805E
    for (std::int32_t i = array.size; i < n; ++i) {                               // 00468063..00468084
        const std::uintptr_t slot = reinterpret_cast<std::uintptr_t>(array.data) + static_cast<std::uintptr_t>(i) * 4;
        if (slot != 0) *reinterpret_cast<std::uint32_t*>(slot) = 0;
    }
    while (n < array.size) --array.size;                                          // 00468086..00468096
    array.size = n;                                                               // 00468098
}

void sound_record_set_destruct_004c9870(std::array<PodPointerArray*, 4> arrays_28_1c_10_04,
                                        const std::function<void(PodPointerArray&, std::int32_t)>& reserve_004674f0,
                                        const std::function<void(void*)>& free_block) {
    for (PodPointerArray* array : arrays_28_1c_10_04) {                           // 004C988E..004C98FC
        pod_pointer_array_resize_00468050(*array, 0, reserve_004674f0);
        free_block(array->data);
    }
}

void pod_pointer_array_push_back_00467f60(PodPointerArray& array, std::uint32_t value,
                                          const std::function<void(PodPointerArray&, std::int32_t)>& reserve_004674f0) {
    if (array.size == array.capacity) {
        std::int32_t grown = array.capacity * 2;
        if (!(grown > 1)) grown = 1;                                              // 00467F6D CMP; JG
        reserve_004674f0(array, grown);
    }
    const std::uintptr_t slot = reinterpret_cast<std::uintptr_t>(array.data) + static_cast<std::uintptr_t>(array.size) * 4;
    if (slot != 0) *reinterpret_cast<std::uint32_t*>(slot) = value;               // 00467F85
    ++array.size;                                                                 // 00467F91
}

void cloud_particle_array_push_back_00476e40(CloudParticleArray& array, const std::array<std::uint32_t, 8>& element,
                                             const CloudParticleReserve& reserve_00476550) {
    if (array.size == array.capacity) {
        std::int32_t grown = array.capacity * 2;
        if (!(grown > 1)) grown = 1;
        reserve_00476550(array, grown);
    }
    const std::uintptr_t slot =
        reinterpret_cast<std::uintptr_t>(array.data) + static_cast<std::uintptr_t>(array.size) * 0x20;
    if (slot != 0) *reinterpret_cast<std::array<std::uint32_t, 8>*>(slot) = element;
    ++array.size;                                                                 // 00476E99
}

void cloud_particle_array_clear_00476ee0(CloudParticleArray& array, const CloudParticleReserve& reserve_00476550) {
    if (array.capacity < 0) reserve_00476550(array, 0);
    while (array.size > 0) --array.size;
    array.size = 0;
}

void cloud_particle_array_tidy_00477020(CloudParticleArray& array, const CloudParticleReserve& reserve_00476550,
                                        const std::function<void(void*)>& free_block) {
    cloud_particle_array_clear_00476ee0(array, reserve_00476550);                 // 0047702B..0047704C
    free_block(array.data);                                                       // 00477053
}

void cloud_class_destruct_00477110(
    std::uint32_t& vtable, const void* cloud_class,
    const std::function<void(void*, std::uint32_t, std::int32_t)>& pool_return_00bd1510,
    const std::function<void(const void*)>& named_class_base_destroy_00489e80) {
    vtable = 0x00CE5FB4;
    static constexpr PooledStringReleaseRow kRow{0x00477110, "__fastcall(ECX class), RET", {{{0x2C, 0x30, 1}}}, 1};
    release_pooled_strings(kRow, cloud_class, pool_return_00bd1510);
    named_class_base_destroy_00489e80(cloud_class);
}

// ---------------------------------------------------------------------------
// Packet cc13-sys4: segment-4 render settings and the system error box.
// ---------------------------------------------------------------------------

void depth_of_field_set_range_00452a50(float& near_14, float& far_1c, float near_value, float far_value) {
    near_14 = near_value;
    far_1c = far_value;
}

void lens_flare_pow_response_00452b00(float k_1c, float x, float& out_04, float& out_08) {
    float result = 0.0f;
    if (!(x == 0.0f)) {                                                           // 00452B17..00452B24
        float magnitude = x;
        if (0.0f > x) magnitude = -0.0f - x;                                      // 00452B26..00452B37
        // cc13-fixture2 (fixture-found): an infinite k * log2|x| gives inf - inf after FRNDINT, so
        // the image stores the default NaN.
        const double y = static_cast<double>(k_1c) * std::log2(static_cast<double>(magnitude));
        result = std::isinf(y) ? -std::numeric_limits<float>::quiet_NaN() : static_cast<float>(std::exp2(y));
    }
    out_04 = result;                                                              // 00452B61
    out_08 = result;
}

namespace {

// FYL2X/FRNDINT/F2XM1/FSCALE on |v| with exponent e, stored float; 0 for v == 0.
float x87_power_response(float v, double e) {
    if (v == 0.0f) return 0.0f;
    const float magnitude = 0.0f > v ? -0.0f - v : v;
    return static_cast<float>(std::exp2(e * std::log2(static_cast<double>(magnitude))));
}

} // namespace

void lens_flare_update_from_camera_00452bd0(LensFlareFields& flare, float dt, float zoom, LensFlareHost& host) {
    if (!host.camera_world_current()) host.refresh_camera_world_00b6db70();      // 00452BF7..00452C03
    const std::array<float, 3> pos = host.camera_position_120();
    if (0.0f >= pos[1]) {                                                         // 00452C3B COMISS; JB (NaN continues)
        host.set_lens_effect_00b0d020(0, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        return;
    }
    if (!host.camera_world_current()) host.refresh_camera_world_00b6db70();      // 00452C6C..00452C74
    const std::array<float, 3> fwd = host.camera_forward_110();
    const std::array<float, 3> reach{static_cast<float>(fwd[0] * 10000.0), static_cast<float>(fwd[1] * 10000.0),
                                     static_cast<float>(10000.0 * fwd[2])};      // 00452C79..00452CA3
    const std::array<float, 3> end{reach[0] + pos[0], reach[1] + pos[1], reach[2] + pos[2]}; // 00452CBF..00452CD7
    const float inv_scale = static_cast<float>(1.0 / flare.scale_18);            // 00452CF8..00452D0E
    const float brightness = static_cast<float>(
        (static_cast<double>(zoom) - 1.0) * (-0.699999988079071 / (static_cast<double>(inv_scale) - 1.0)) + 1.0); // 00452D12..00452D2B
    auto dot_forward = [&](const std::array<float, 3>& d) {
        return static_cast<float>((static_cast<double>(fwd[1]) * d[1] + static_cast<double>(fwd[0]) * d[0])
                                  + static_cast<double>(d[2]) * fwd[2]);
    };
    float v = 0.0f;
    std::array<float, 3> hit{};
    if (host.query_segment_0098add0(pos, end, hit)) {                            // 00452D49
        const float d = dot_forward({hit[0] - pos[0], hit[1] - pos[1], hit[2] - pos[2]}); // 00452D5E..00452DA7
        v = 1.0f > d ? 1.0f : d;                                                  // 00452DB9
    } else {
        const float quarter = static_cast<float>(host.camera_fov_1c4() * 0.25);   // 00452DF1..00452E05
        float best_dot = static_cast<float>(std::cos(static_cast<double>(quarter))); // 00452E0D FCOS
        void* best = nullptr;
        float last_y = reach[1];
        for (void* light : host.lights()) {                                       // 00452E40..00452EE5
            if (!host.light_pose_current_c8(light)) host.refresh_light_pose_00414db0(light);
            const std::array<float, 3> lp = host.light_position_fc(light);
            std::array<float, 3> d{lp[0] - pos[0], lp[1] - pos[1], lp[2] - pos[2]};
            normalize_camera_basis_0042b260(d);                                   // 00452E99
            last_y = d[1];
            const float dot = dot_forward(d);
            if (dot > best_dot) {                                                 // 00452ECC FCOMIP; JBE
                best_dot = dot;
                best = light;
            }
        }
        if (best != nullptr) {
            if (!host.light_pose_current_c8(best)) host.refresh_light_pose_00414db0(best); // 00452EEF..00452EF9
            v = 100000.0f;                                                        // 00CE4BC8
        } else if (!(0.0f > last_y)) {                                            // 00452F25 COMISS; JBE
            v = 10000.0f;                                                         // 00CE3D64
        } else {
            const float sine = static_cast<float>(std::sin(static_cast<double>(-0.0f - fwd[1]))); // 00452F2C..00452F43
            const float r = static_cast<float>(static_cast<double>(pos[1]) / sine); // 00452F47..00452F53
            if (1.0f > r) v = 1.0f;                                               // 00452F5D
            else if (r > 100000.0f) v = 100000.0f;                                // 00452F7E JA 00452DBE
            else v = 1.0f > r ? 1.0f : r;                                         // 00452DB1
        }
    }
    const float p = x87_power_response(v, flare.exponent_1c);                    // 00452DC6..00452FD5
    const double w = static_cast<double>(flare.gain_14) * dt;                     // 00452FE2..00452FE5
    const float a1 = flare.value_04;
    flare.value_04 = static_cast<float>(a1 + (static_cast<double>(p) - a1) * w);  // 00452FF0..00453006
    const float a2 = flare.value_08;
    flare.value_08 = static_cast<float>(a2 + w * (static_cast<double>(flare.value_04) - a2)); // 00453009..0045302F
    const float inv_exponent = static_cast<float>(1.0 / flare.exponent_1c);      // 00453036..00453043
    const float out = x87_power_response(flare.value_08, inv_exponent);          // 00453047..00453086
    host.set_lens_effect_00b0d020(1, out, out, out, brightness, 1.0f);            // 004530B4
}

void lens_flare_set_enabled_00452b80(
    std::uint8_t on, std::uint8_t& enabled_00, float& value_04,
    const std::function<void(std::int32_t, float, float, float, float, float)>& set_lens_effect_00b0d020) {
    enabled_00 = on;
    if (on != 0) return;
    set_lens_effect_00b0d020(0, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);                    // 00452BAF
    value_04 = 9.999999747378752e-05f;                                            // 00CE3C68
}

void show_last_system_error_box_004531f0(const char* context, SystemErrorBoxHost& host) {
    const std::string message = host.format_system_message(host.get_last_error(), 0x3FF); // 00453201..0045320F
    const std::string text = std::string(context) + "\n" + message;               // 00453227 "%s\n%s"
    if (text.size() >= 0x400) throw std::length_error("error text does not fit 00E177E8");
    host.message_box(text, "System error", 0x10);                                 // 00453232
}

// ---------------------------------------------------------------------------
// Packet cc13-sys4: small segment-9 helpers.
// ---------------------------------------------------------------------------

std::uint8_t selected_user_live_byte_004bb600(SelectedUserLiveHost& host) {
    if (host.has_selected_user_00a3e510() && host.selected_sign_in_state_00a3ead0() == 2) // 004BB609..004BB61C
        return host.active_slot_byte_118_00a3eb20();                                        // 004BB629 JMP
    return 0;                                                                               // 004BB61E
}

bool game_clip_segment_to_world_bounds_004bbdd0(
    const WorldBoundsXZ& box, const std::array<float, 3>& start, std::array<float, 3>& end,
    std::array<float, 3>& out,
    const std::function<std::array<float, 3>(const std::array<float, 3>&)>& normalize_00419510) {
    const std::array<float, 3> original = end;
    const double sx = start[0], sz = start[2];
    bool clipped = false;
    // f(s + f(e - s) * r): the difference is stored, r stays on the stack.
    auto lerp = [](double s, float e, double r) { return static_cast<float>(s + static_cast<double>(static_cast<float>(e - s)) * r); };
    const double x = end[0];
    if (box.min_x_711c > x) {                                                     // 004BBDF5 FCOMIP; JBE
        clipped = true;
        end[2] = lerp(sz, end[2], (sx - box.min_x_711c) / (sx - x));              // 004BBE24..004BBE51
        end[0] = static_cast<float>(static_cast<double>(box.min_x_711c) + 1.0);   // 004BBE54..004BBE5C
    } else if (x > box.max_x_7128) {                                              // 004BBE68 FCOMI; JBE
        clipped = true;
        end[2] = lerp(sz, end[2], (box.max_x_7128 - sx) / (x - sx));              // 004BBE7D..004BBEAA
        end[0] = static_cast<float>(static_cast<double>(box.max_x_7128) - 1.0);   // 004BBEAD..004BBEB5
    }
    const double z = end[2];
    if (z > box.max_z_7124) {                                                     // 004BBECC FCOMI; JBE
        clipped = true;
        end[0] = lerp(sx, end[0], (box.max_z_7124 - sz) / (z - sz));              // 004BBEE1..004BBF0B
        end[2] = static_cast<float>(static_cast<double>(box.max_z_7124) - 1.0);   // 004BBF0D FSUBR
    } else if (box.min_z_7130 > z) {                                              // 004BBF1E FCOMIP; JBE
        clipped = true;
        end[0] = lerp(sx, end[0], (sz - box.min_z_7130) / (sz - z));              // 004BBF31..004BBF5B
        end[2] = static_cast<float>(1.0 + box.min_z_7130);                        // 004BBF5D
    } else if (!clipped) {
        return false;                                                             // 004BBF6E
    }
    auto magnitude = [](float v) { return v > 0.0f ? v : -0.0f - v; };
    float dx = end[0] - start[0];                                                 // 004BBF74..004BBF80
    float dz = end[2] - start[2];                                                 // 004BBF84..004BBF8A
    const bool too_short = 1.0f > magnitude(dx) || 1.0f > magnitude(dz);          // 004BBFB3, 004BBFCF
    if (too_short) {                                                              // 004BBFD4..004BBFFE
        end[0] = original[0];
        end[2] = original[2];
        dx = original[0] - start[0];
        dz = original[2] - start[2];
    }
    const std::array<float, 3> n = normalize_00419510({dx, 0.0f, dz});            // 004BC010
    const float az = magnitude(n[2]);
    const float ax = magnitude(n[0]);
    float m = az > ax ? ax : az;                                                  // 004BC073 FCOMIP; JA
    if (0.019999999552965164 > m) m = 0.02f;                                      // 004BC08C, 00CE746C
    const float s = static_cast<float>(3000.0 / m);                               // 004BC0B0 FDIVR
    out[0] = end[0] + n[0] * s;                                                   // 004BC0BA..004BC0E4
    out[1] = original[1] + s * n[1];
    out[2] = end[2] + s * n[2];
    return true;
}

std::int32_t scene_mode_suffix_index_004cdb32(const ShortNameModeFields& game) {
    if (game.byte_61c != 0) return game.mode_614;                                 // 004CDB37..004CDB40
    if (game.field_1fe4 != 0) return game.mode_614;                               // 004CDB48..004CDB55
    if (game.mode_614 == 9 || game.mode_614 == 8) return game.mode_614;           // 004CDB57..004CDB5F
    return 8;                                                                     // 004CDB61
}

std::string derive_scene_short_name_004cd7f0(const std::string& path, bool append_mode, const ShortNameModeFields& game) {
    std::string name = derive_scene_short_name(path, false);                             // 004CD80B..004CDB22
    if (!append_mode) return name;                                                // 004CDB27
    const std::int32_t index = scene_mode_suffix_index_004cdb32(game);
    if (index < 0 || index >= static_cast<std::int32_t>(kSceneModeSuffixes_00e08240.size()))
        throw std::out_of_range("scene mode index outside 00E08240");             // the image reads past the table
    name += kSceneModeSuffixes_00e08240[static_cast<std::size_t>(index)];         // 004CDB66..004CDBA2
    return name;
}

void game_update_presence_context_004c0170(const PresenceGameFields& game, PresenceState& state, PresenceHost& host) {
    if (!host.has_selected_user_00a3e510()) return;                               // 004C0180
    std::int64_t ticks = 0, frequency = 1;
    host.frame_clock_vslot_20(ticks, frequency);                                  // 004C01A3
    const bool online = game.mode.field_1fe4 != 0;                                // 004C01BE
    const float t = static_cast<float>(static_cast<double>(ticks) / static_cast<double>(frequency)); // 004C01A5..004C01CE
    if (state.first_00e082e8 != 0) state.stamp_00e18b30 = t;
    state.first_00e082e8 = 0;
    bool force = false;
    if (static_cast<double>(t) - state.stamp_00e18b30 > 3.0) {                    // 004C01ED..004C01FF
        force = true;
        state.stamp_00e18b30 = t;
    }
    bool hosted = false;                                                          // 004C020E..004C023D
    if (host.has_object_00e198b4() && game.state_5d4 == 8)
        hosted = host.object_00e198b4_test_00687860(0x14, 0) && game.byte_218c == 0;
    const bool outside = game.state_5d4 != 0x0D && !hosted;                       // 004C023F..004C024C
    if (!game.has_record_block_5fc || outside) {                                  // 004C0251..004C0260
        if (force) host.x_user_set_context(host.active_user_slot_00a3eac0(), 0x8001, 5); // 004C026D..004C0280
        return;
    }
    force = *game.presence_dirty_22dc != 0 || force;                              // 004C028B..004C02A0
    *game.presence_dirty_22dc = 0;
    const std::int32_t record_id = game.record_id_1098;
    const std::int32_t mode_index = scene_mode_suffix_index_004cdb32(game.mode);  // 004C02B3..004C02F3
    const std::uint64_t xuid = host.active_xuid_00a3eb00();                       // 004C02FB
    if (xuid != state.xuid_00e18b28) force = true;
    if (state.record_id_00e18b24 != record_id) force = true;
    if (state.mode_index_00e18b20 == mode_index && !force) return;               // 004C032D..004C033A
    const std::int32_t kind = host.mission_campaign_kind_004bffa0();              // 004C0343
    const std::uint32_t players = online ? 3 : 2;                                 // 004C0348..004C035B
    std::uint32_t bucket = 0;                                                     // 004C0361 table 004C0490
    switch (mode_index) {
    case 4: bucket = 1; break;
    case 5: bucket = 2; break;
    case 6: bucket = 3; break;
    case 7: bucket = 4; break;
    default: bucket = 0; break;
    }
    std::uint32_t game_type = 1;                                                  // 004C0393 table 004C04B0
    switch (kind) {
    case 1: game_type = 3; break;
    case 2: game_type = 4; break;
    case 3: game_type = 5; break;
    case 4:
        if (game.byte_218c != 0) game_type = 2;
        else if (game.state_5d4 == 8 && host.has_object_00e198b4() && host.object_00e198b4_test_00687860(0x14, 0) &&
                 game.state_5d4 == 8)
            game_type = 6;
        break;
    default: break;
    }
    host.x_user_set_context_ex(host.active_user_slot_00a3eac0(), 0, game_type, nullptr);           // 004C0404
    const std::uint32_t record_value = (kind == 1 || kind == 2) ? 0u : static_cast<std::uint32_t>(record_id);
    host.x_user_set_context_ex(host.active_user_slot_00a3eac0(), 1, record_value, nullptr);        // 004C042C
    host.x_user_set_context_ex(host.active_user_slot_00a3eac0(), 0x0F, bucket, nullptr);           // 004C0442
    host.x_user_set_context_ex(host.active_user_slot_00a3eac0(), 0x8001, players, nullptr);        // 004C045F
    state.xuid_00e18b28 = xuid;                                                   // 004C0470..004C0481
    state.record_id_00e18b24 = record_id;
    state.mode_index_00e18b20 = mode_index;
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

// Offsets from each body's MOV reg,[ECX+data] / MOV reg,[ECX+length] and the
// ADD 1 / LEA [len+len+2] before the pool call.
const std::array<PooledStringReleaseRow, 14> kPooledStringReleases{{
    {0x004C67E0, "__fastcall(ECX object), RET", {{{0x08, 0x0C, 1}}}, 1},
    {0x004C6800, "__fastcall(ECX object), RET", {{{0x08, 0x0C, 1}}}, 1},
    {0x004C89E0, "__fastcall(ECX object), RET", {{{0x08, 0x0C, 1}}}, 1},
    {0x004C8260, "__fastcall(ECX object), RET", {{{0x08, 0x0C, 2}}}, 1},
    {0x004C8DB0, "__fastcall(ECX object), RET", {{{0x0C, 0x10, 1}}}, 1},
    {0x004CE690, "__stdcall(object), RET 4", {{{0x08, 0x0C, 2}}}, 1},
    {0x004C5720, "__fastcall(ECX record), RET", {{{0x10, 0x14, 1}, {0x08, 0x0C, 1}, {0x00, 0x04, 1}}}, 3},
    {0x004C6760, "__fastcall(ECX record), RET", {{{0x10, 0x14, 1}, {0x00, 0x04, 1}}}, 2},
    {0x0044BA70, "__fastcall(ECX string), RET", {{{0x00, 0x04, 1}}}, 1},
    {0x00468B30, "__fastcall(ECX string), RET", {{{0x00, 0x04, 1}}}, 1},
    {0x00468B50, "__fastcall(ECX string), RET", {{{0x00, 0x04, 1}}}, 1},
    {0x00471340, "__fastcall(ECX string), RET", {{{0x00, 0x04, 1}}}, 1},
    {0x00474780, "__fastcall(ECX string), RET", {{{0x00, 0x04, 1}}}, 1},
    {0x00477200, "__fastcall(ECX string), RET", {{{0x00, 0x04, 1}}}, 1},
}};

const PooledStringReleaseRow* find_pooled_string_release(std::uint32_t address) {
    for (const auto& row : kPooledStringReleases) {
        if (row.address == address) return &row;
    }
    return nullptr;
}

void release_pooled_strings(const PooledStringReleaseRow& row, const void* object,
                            const std::function<void(void*, std::uint32_t, std::int32_t)>& pool_return_00bd1510) {
    const auto* bytes = static_cast<const unsigned char*>(object);
    for (std::uint8_t i = 0; i < row.count; ++i) {
        const PooledStringField& field = row.fields[i];
        void* data = nullptr;
        std::uint32_t length = 0;
        std::memcpy(&data, bytes + field.data_offset, sizeof data);
        if (data == nullptr) continue;
        std::memcpy(&length, bytes + field.length_offset, sizeof length);
        pool_return_00bd1510(data, field.unit == 2 ? length * 2 + 2 : length + 1, 1);
    }
}

NativeWideString& wide_string_copy_construct_004c8dd0(
    NativeWideString& dst, const NativeWideString& src,
    const std::function<void(NativeWideString&, std::uint32_t, std::int32_t)>& size_004c53e0) {
    dst.length = 0;                                                               // 004C8DDA
    dst.data = nullptr;                                                           // 004C8DE0
    return wide_string_assign_004c5e20(dst, src, size_004c53e0);
}

void game_clear_scene_records_004c8900(const std::function<void()>& destroy_all_004bf930, void*& record_block_5fc,
                                       NativeNarrowString& text_600, const NarrowStringResize& resize_0041dd40) {
    destroy_all_004bf930();
    record_block_5fc = nullptr;
    resize_0041dd40(text_600, 0, 0);
    static const char kEmpty_00ce3a0c[1] = {0};
    if (text_600.data != nullptr) std::memcpy(text_600.data, kEmpty_00ce3a0c, text_600.length);
}

void game_set_command_line_text_004c8950(NativeNarrowString& text_600, const char* text,
                                         const NarrowStringResize& resize_0041dd40) {
    const std::uint32_t length = text == nullptr ? 0 : static_cast<std::uint32_t>(std::strlen(text));
    resize_0041dd40(text_600, length, 0);
    if (text_600.data != nullptr && text_600.length != 0) std::memcpy(text_600.data, text, text_600.length);
}

void game_assign_console_string_004c89a0(NativeNarrowString& string_2198, const NativeNarrowString& src,
                                         const NarrowStringResize& resize_0041dd40) {
    if (&string_2198 == &src) return;
    resize_0041dd40(string_2198, src.length, 1);
    if (src.length != 0) std::memcpy(string_2198.data, src.data, string_2198.length);
}

LocalisedEntry60& localised_entry_assign_004c9220(
    LocalisedEntry60& dst, const LocalisedEntry60& src,
    const std::function<void(NativeWideString&, std::uint32_t, std::int32_t)>& size_004c53e0) {
    dst.byte_00 = src.byte_00;
    dst.dword_04 = src.dword_04;
    wide_string_assign_004c5e20(dst.text_08, src.text_08, size_004c53e0);
    dst.dwords_10 = src.dwords_10;
    return dst;
}

NamedEntry16& named_entry_construct_004c95d0(NamedEntry16& entry, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                             const NativeNarrowString& name, std::uint8_t flag,
                                             const NarrowStringResize& resize_0041dd40) {
    entry.a_00 = a;                                                               // 004C95EA..004C95EF
    entry.b_04 = b;
    entry.c_08 = c;
    entry.name_0c = NativeNarrowString{};                                         // 004C95F2..004C95F8
    if (&entry.name_0c != &name) {
        resize_0041dd40(entry.name_0c, name.length, 1);                           // 004C9609
        if (name.length != 0) std::memcpy(entry.name_0c.data, name.data, entry.name_0c.length); // 004C961F
    }
    entry.flag_14 = flag;
    entry.byte_15 = 0;
    return entry;
}

void named_record_scalar_deleting_destructor_004c9420(
    void* record, std::uint8_t flags, const std::function<void(void*, std::uint32_t, std::int32_t)>& pool_return_00bd1510,
    std::uint32_t node_pool, const std::function<void(std::uint32_t, void*)>& node_pool_return_0043b0a0) {
    static constexpr PooledStringReleaseRow kRow{0x004C9420, "__thiscall(ECX record, flags), RET 4",
                                                 {{{0x00, 0x04, 1}}}, 1};
    release_pooled_strings(kRow, record, pool_return_00bd1510);
    if ((flags & 1) != 0) node_pool_return_0043b0a0(node_pool, record);
}

void front_end_frame_select_default_004c9680(FrontEndFrameSelectHost& host) {
    NativeNarrowString name;
    host.resize_0041dd40(name, 0, 1);                                             // 004C96B0
    static const char kEmpty_00ce3a0c[1] = {0};
    // 004C96CA copies length + 1 bytes; the length is 0 after the resize, so one byte.
    if (name.data != nullptr) std::memcpy(name.data, kEmpty_00ce3a0c, 1);
    host.select_00518d60(-1, -1, -1, name);                                       // 004C96E5
    if (name.data != nullptr) host.pool_return_00bd1510(name.data, name.length + 1, 1); // 004C9704..004C970B
}

void game_process_window_close_request_004ca2f0(std::int32_t state_5d4, WindowCloseHost& host) {
    if (!host.take_close_request_180()) return;                                   // 004CA30F..004CA315
    if (host.platform_busy_3e8()) return;                                         // 004CA327
    if (state_5d4 == 1 || state_5d4 == 2 || state_5d4 == 4) {                     // 004CA33A..004CA344
        host.set_exit_flag_00e1ae75();                                            // 004CA349
        return;
    }
    host.raise_prompt_00531b00(6, "FE_pc.main_quit_confirm", 1, 0x004BBC50, 1, "", 0, 0); // 004CA3CB
    host.prompt_screen_00530c20();                                                // 004CA422
}

void game_show_high_latency_warning_004ca120(std::int32_t state_5d4, void* client_21e8, HighLatencyWarningHost& host) {
    if (state_5d4 != 0x0D || client_21e8 == nullptr) return;                      // 004CA13B..004CA14B
    NativeNarrowString key = host.make_string_0041e870("ingame.multi_highlatency"); // 004CA15C, 00CE76B8
    NativeNarrowString empty;
    host.client_007349c0(client_21e8, key, empty, 0);                             // 004CA183
    host.release_string(empty);
    host.release_string(key);
}

void native_string_vector_pop_back_004caf10(
    NativeNarrowString* data, std::uint32_t& count,
    const std::function<void(void*, std::uint32_t, std::int32_t)>& pool_return_00bd1510) {
    if (count == 0) return;
    NativeNarrowString& last = data[count - 1];
    if (last.data != nullptr) pool_return_00bd1510(last.data, last.length + 1, 1);
    --count;
}

void native_string_pointer_delete_004cc8e0(
    NativeNarrowString*& slot, const std::function<void(void*, std::uint32_t, std::int32_t)>& pool_return_00bd1510,
    std::uint32_t node_pool, const std::function<void(std::uint32_t, void*)>& node_pool_return_0043b0a0) {
    NativeNarrowString* string = slot;
    if (string == nullptr) return;
    if (string->data != nullptr) pool_return_00bd1510(string->data, string->length + 1, 1);
    node_pool_return_0043b0a0(node_pool, string);
    slot = nullptr;
}

void game_set_paused_by_platform_004ceb40(bool paused, const PlatformPauseGame& game, std::uint8_t& flag_00e188ae,
                                          std::uint8_t& paused_00e188bc, PlatformPauseHost& host) {
    flag_00e188ae = paused ? 1 : 0;                                               // 004CEB48
    if (game.state_5d4 != 0x0D) return;                                           // 004CEB4D
    if (game.field_1fe4 != 0 && game.byte_218c == 0) return;                      // 004CEB5A..004CEB6A
    if (paused) {
        if (game.byte_634 != 0) return;                                           // 004CEB74
        paused_00e188bc = 1;
        host.set_cinematic_mode_004cd0f0(1, 0, 0);                                // 004CEB8A
        host.hud_pause_open_005ff0d0();                                           // 004CEB9A
        host.dialogs_set_all_paused_00a77760(1);                                  // 004CEBA7
        return;
    }
    if (paused_00e188bc != 0) {
        host.set_cinematic_mode_004cd0f0(0, 0, 0);                                // 004CEBBC
        host.hud_pause_resume_005fd440();                                         // 004CEBCD
        host.dialogs_set_all_paused_00a77760(0);                                  // 004CEBDA
        host.input_update_00a92c40(0.0f);                                         // 004CEBEC
    }
    paused_00e188bc = 0;                                                          // 004CEBF1
}

void game_reapply_platform_pause_004cec00(const PlatformPauseGame& game, std::uint8_t& flag_00e188ae,
                                          std::uint8_t& paused_00e188bc, PlatformPauseHost& host) {
    if (flag_00e188ae != 0) game_set_paused_by_platform_004ceb40(true, game, flag_00e188ae, paused_00e188bc, host);
}

void vector_tidy_resize_then_free(void* vector, void* const& data_slot, const std::function<void(void*)>& resize_to_zero,
                                  const std::function<void(void*)>& free_block) {
    resize_to_zero(vector);
    free_block(data_slot); // read after the resize ([ESI] at 004CEC2A)
}

void narrow_string_deque_pop_back_004cde20(
    NarrowStringDeque& deque, const std::function<void(void*, std::uint32_t, std::int32_t)>& pool_return_00bd1510) {
    if (deque.count == 0) return;
    const std::uint32_t index = deque.count - 1 + deque.offset;
    std::uint32_t block = index >> 1;
    if (deque.map_size <= block) block -= deque.map_size;
    NativeNarrowString& element = deque.map[block][index & 1];
    if (element.data != nullptr) pool_return_00bd1510(element.data, element.length + 1, 1);
    if (--deque.count == 0) deque.offset = 0;
}

void award_tracker_drain_hint_deque_004d0140(
    NarrowStringDeque& deque, const std::function<void(void*, std::uint32_t, std::int32_t)>& pool_return_00bd1510,
    const std::function<void(void*)>& free_00bf65ac) {
    while (deque.count != 0) narrow_string_deque_pop_back_004cde20(deque, pool_return_00bd1510);
    for (std::uint32_t i = deque.map_size; i != 0;) {
        --i;
        if (deque.map[i] != nullptr) free_00bf65ac(deque.map[i]);
    }
    if (deque.map != nullptr) free_00bf65ac(deque.map);
    deque.map = nullptr;
    deque.map_size = 0;
}

void counted_reference_vector_reserve_004c8310(CountedReferenceVector& vector, std::int32_t n,
                                               CountedReferenceHost& host) {
    if (n < 1) n = 1;
    if (!(vector.capacity < n)) return;
    auto** block = static_cast<void**>(host.operator_new(static_cast<std::uint32_t>(n) * 4));
    for (std::int32_t i = 0; i < vector.size; ++i) {
        const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(block) + static_cast<std::uintptr_t>(i) * 4;
        if (address == 0) continue;                                               // the image tests the slot address
        void** slot = reinterpret_cast<void**>(address);
        *slot = nullptr;
        if (void* counted = vector.data[i]) {
            *slot = counted;
            host.add_ref(counted);
        }
    }
    for (std::int32_t i = 0; i < vector.size; ++i) {
        if (vector.data[i] != nullptr) {
            host.release(vector.data[i]);
            vector.data[i] = nullptr;
        }
    }
    host.free_block(vector.data);
    vector.data = block;
    vector.capacity = n;
}

void two_vector_record_tidy_004ce460(TwoVectorRecord& record, const std::function<void(void*)>& free_00bf65ac) {
    if (record.vector_14[0] != nullptr) free_00bf65ac(record.vector_14[0]);
    record.vector_14 = {};
    if (record.vector_04[0] != nullptr) free_00bf65ac(record.vector_04[0]);
    record.vector_04 = {};
}

void game_set_presence_context_004bc4e0(std::uint8_t& presence_set_22dc, GameLiveHost& host) {
    if (host.has_selected_user_00a3e510())                                        // 004BC4E9
        host.x_user_set_context(host.active_user_slot_00a3eac0(), 0x8001, 4);     // 004BC4FF..004BC505
    presence_set_22dc = 1;                                                        // 004BC50A
}

void game_create_friends_enumerator_004bc520(void* friends_buffer_22de, std::uint32_t* friends_count_7034,
                                             GameLiveHost& host) {
    std::uint32_t size = 0;
    void* handle = nullptr;
    if (host.x_friends_create_enumerator(host.active_user_slot_00a3eac0(), 0, 100, &size, &handle) != 0) return; // 004BC540
    if (host.x_enumerate(handle, friends_buffer_22de, 0x4D54, friends_count_7034, nullptr) != 0) return;      // 004BC562
    host.close_handle(handle);                                                                                  // 004BC570
}

void game_begin_friends_leaderboard_read_004bc580(const LeaderboardFields& game, std::uint32_t view, GameLiveHost& host) {
    if (*game.buffer_7054 != nullptr) {                                           // 004BC589..004BC59C
        host.free_block(*game.buffer_7054);
        *game.buffer_7054 = nullptr;
    }
    *game.state_7058 = 2;                                                         // 004BC5AF
    const std::uint32_t friends = *game.friends_count_7034;
    std::vector<std::uint64_t> xuids(friends + 1);
    for (std::uint32_t i = 0; i < friends; ++i)                   // 004BC5D0..004BC5EE
        std::memcpy(&xuids[i], game.friends_22de + i * 0xC4, 8);
    xuids[friends] = host.active_xuid_00a3eb00();                 // 004BC5FC..004BC613
    const std::uint32_t spec[3] = {view, 1, 0};
    std::uint32_t size = 0;
    const std::uint32_t count = friends + 1;
    if (host.x_user_read_stats(0, count, xuids.data(), 1, spec, &size, nullptr, nullptr) != 0x7A) return; // 004BC64F
    *game.buffer_7054 = host.operator_new(size);                                  // 004BC65F
    game.overlapped_7038->fill(0);                                                // 004BC66C..004BC694
    host.x_user_read_stats(0, count, xuids.data(), 1, spec, &size, *game.buffer_7054, nullptr); // 004BC6C1
}

void game_begin_leaderboard_read_004bc6d0(const LeaderboardFields& game, std::uint32_t rank_start, std::uint64_t xuid,
                                          std::uint32_t view, bool friends, GameLiveHost& host) {
    if (friends) {
        game_create_friends_enumerator_004bc520(game.friends_22de, game.friends_count_7034, host); // 004BC6E3
        game_begin_friends_leaderboard_read_004bc580(game, view, host);          // 004BC702
        return;
    }
    if (*game.buffer_7054 != nullptr) {                                           // 004BC711..004BC724
        host.free_block(*game.buffer_7054);
        *game.buffer_7054 = nullptr;
    }
    const std::uint32_t spec[3] = {view, 0, 0};
    std::uint32_t size = 0;
    void* handle = nullptr;
    if (rank_start != 0)
        host.x_user_create_stats_enumerator_by_rank(0, rank_start, 0x11, 1, spec, &size, &handle); // 004BC762
    else
        host.x_user_create_stats_enumerator_by_xuid(0, xuid, 0x11, 1, spec, &size, &handle);       // 004BC78E
    *game.buffer_7054 = host.operator_new(size);                                  // 004BC798
    game.overlapped_7038->fill(0);                                                // 004BC7AF..004BC7C5
    host.x_enumerate(handle, *game.buffer_7054, size, nullptr, game.overlapped_7038->data());       // 004BC7D5
    *game.state_7058 = 2;                                                         // 004BC7DA
}

void game_cancel_leaderboard_read_004bc7f0(void* overlapped_7038, void*& buffer_7054, std::uint32_t& field_7058,
                                           GameLiveHost& host) {
    host.x_cancel_overlapped(overlapped_7038);
    if (buffer_7054 != nullptr) {
        host.free_block(buffer_7054);
        buffer_7054 = nullptr;
    }
    field_7058 = 0;
}

void game_reset_lobby_slots_for_mode_004bb3e0(std::int32_t mode_614, std::array<std::int32_t*, 8> slot_states) {
    const bool standard = mode_614 >= 0 && mode_614 <= 3;                         // 004BB3E6..004BB3FD
    for (std::int32_t* state : slot_states) {                                     // 004BB406 ESI = 8
        if (*state == 3 && standard) {
            *state = 1;                                                           // 004BB41B
        } else if (*state == 1 && !standard) {
            *state = 3;                                                           // 004BB42C
        }
    }
}

void game_select_next_scene_record_004c6b00(std::int32_t& index_60c, std::int32_t count_5f0,
                                            const std::function<void(std::int32_t)>& select_004c6890) {
    index_60c = (index_60c + 1) % count_5f0;                                      // 004C6B0A IDIV
    select_004c6890(index_60c);
}

namespace {

std::string diagnostic_compose(std::size_t count, const std::function<void()>& sink_get_004c14c0,
                               const char* format, va_list args) {
    std::string text(count, '\0');
    const int n = std::vsnprintf(text.data(), count, format, args);               // 00BF7289
    if (n < 0 || static_cast<std::size_t>(n) >= count)
        throw std::length_error("diagnostic text does not fit the buffer");
    text.resize(static_cast<std::size_t>(n));
    text += '\n';                                                                 // "%s\n" 00CE3A30
    if (text.size() >= count) throw std::length_error("diagnostic text does not fit the buffer");
    sink_get_004c14c0();                                                          // 004C70AC
    return text;
}

} // namespace

std::string diagnostic_printf_004c7070(const std::function<void()>& sink_get_004c14c0, const char* format, ...) {
    va_list args;
    va_start(args, format);
    try {
        std::string text = diagnostic_compose(0xFF, sink_get_004c14c0, format, args);
        va_end(args);
        return text;
    } catch (...) {
        va_end(args);
        throw;
    }
}

std::string diagnostic_printf_large_004c70c0(const std::function<void()>& sink_get_004c14c0, const char* format, ...) {
    va_list args;
    va_start(args, format);
    try {
        std::string text = diagnostic_compose(0xFFC, sink_get_004c14c0, format, args);
        va_end(args);
        return text;
    } catch (...) {
        va_end(args);
        throw;
    }
}

LocalisedEntry60& localised_entry_copy_construct_004cc390(
    LocalisedEntry60& dst, const LocalisedEntry60& src,
    const std::function<void(NativeWideString&, std::uint32_t, std::int32_t)>& size_004c53e0) {
    dst.byte_00 = src.byte_00;
    dst.dword_04 = src.dword_04;
    wide_string_copy_construct_004c8dd0(dst.text_08, src.text_08, size_004c53e0);
    dst.dwords_10 = src.dwords_10;
    return dst;
}

void named_vector_record_destruct_004cc650(
    void* record, const std::function<void(void*)>& destruct_004c8f00,
    const std::function<void(void*, std::uint32_t, std::int32_t)>& pool_return_00bd1510) {
    destruct_004c8f00(static_cast<unsigned char*>(record) + 8);                   // 004CC678
    static constexpr PooledStringReleaseRow kRow{0x004CC650, "__fastcall(ECX record), RET", {{{0x00, 0x04, 1}}}, 1};
    release_pooled_strings(kRow, record, pool_return_00bd1510);                   // 004CC67D..004CC69C
}

void reference_pointer_range_release_forward_004cf590(void* first, void* last,
                                                      const std::function<void(void*, void*)>& release_004cc760) {
    release_004cc760(first, last);
}

NarrowStringPair& narrow_string_pair_construct(NarrowStringPair& dst, const NativeNarrowString& key,
                                               std::uint32_t value, const NarrowStringResize& resize_0041dd40) {
    dst.key = NativeNarrowString{};                                               // 00469AB8..00469ABF
    if (&dst.key != &key) {
        resize_0041dd40(dst.key, key.length, 1);                                  // 00469AC9
        if (key.length != 0) std::memcpy(dst.key.data, key.data, dst.key.length); // 00469ADE
    }
    dst.value_08 = value;                                                         // 00469AF2
    return dst;
}

NarrowStringPair& narrow_string_pair_copy_construct_0044c6e0(NarrowStringPair& dst, const NarrowStringPair& src,
                                                             const NarrowStringResize& resize_0041dd40) {
    return narrow_string_pair_construct(dst, src.key, src.value_08, resize_0041dd40); // 0044C71B
}

void session_message_e1_destruct_0044cce0(
    std::uint32_t& vtable, const void* message,
    const std::function<void(void*, std::uint32_t, std::int32_t)>& pool_return_00bd1510) {
    vtable = 0x00CE4B2C;                                                          // 0044CCFD
    static constexpr PooledStringReleaseRow kRow{
        0x0044CCE0, "__fastcall(ECX message), RET", {{{0x24, 0x28, 1}, {0x18, 0x1C, 1}}}, 2};
    release_pooled_strings(kRow, message, pool_return_00bd1510);                  // 0044CD03..0044CD45
    vtable = 0x00CE4974;                                                          // 0044CD4E
}

void record14_clear_004c7f80(Record14& record) {
    record.field_00 = record.field_04 = record.field_08 = 0;
    record.field_0c = record.field_10 = 0.0f;
}

// ---------------------------------------------------------------------------
// Packet cc13-left3 (docs/CC13_LEFT_3.md).
// ---------------------------------------------------------------------------

void session_message_e1_construct_0044c970(SessionMessageE1& message, const std::string& name,
                                           LuaRef table, std::int32_t value, DialogScriptLuaHost& host)
{
    host.construct_base_0075b430(0xE1);
    message.word_04 = 1;
    message.name_18 = name;                                   // skipped when the same object
    message.value_20 = value;
    message.script_24.clear();
    // 0044CA08..0044CA74: a string builder (00426500), 00BD1A60/00BD1A20 appends, then the
    // result appended to +24h.
    message.script_24 += std::string("Dialogues = { [\"") + name + "\"]={[\"sequence\"]={";
    if (host.is_table_00b661b0(table)) {
        // 0044CA9E..0044CAC1: the by-value table becomes table["sequence"].
        host.assign_00b67690(table, host.get_by_name_00b67800(table, "sequence"));
        LuaRef key = 0;
        LuaRef entry = 0;
        host.iterate_first_00b67080(table, key, entry);
        while (!host.is_unbound_00b66420(key)) {
            if (host.is_table_00b661b0(entry)) {
                const LuaRef text = host.get_by_name_00b67800(entry, "message");
                if (!host.is_nil_00b65fb0(text)) {
                    message.script_24 += "{ [\"message\"]= \"" + host.get_string_00b662b0(text) + "\",},";
                }
            }
            host.iterate_next_00b67190(table, key, entry);
        }
    }
    message.script_24 += "},},};";                            // 0044CC1C, six characters
}

namespace {

void begin_scene_once(GameFrameHost& host)
{
    // [game+34h] == 0 and 00B1BF90 on the render queue == 0: [00F8D394] vtable +0Ch, then 1.
    std::uint32_t& begun = host.scene_begun_34();
    if (begun == 0 && host.render_queue_retention_00b1bf90() == 0) {
        host.renderer_begin_0c();
        begun = 1;
    }
}

} // namespace

void game_pump_front_end_frame_004cac30(GameFrameHost& host)
{
    host.front_end_screens_pump_004f8830(0.0f);
    host.gui_manager_update_00aa4f80(host.gui_manager_004c12b0(), 0.0f, 0);
    begin_scene_once(host);
    host.game_render_004ca440();
    host.game_finish_render_frame_004ca1f0();
}

bool game_render_foliage_and_shadows_004ccbd0(GameFrameHost& host)
{
    void* rig = host.camera_rig_a8();
    void* view = host.camera_rig_view(rig, host.camera_rig_alternate_cc(rig));
    host.foliage_build_visible_set_00af0c50(host.camera_node_19fc(), view);
    host.foliage_frame_counter_increment_20();
    begin_scene_once(host);
    host.shadow_driver_virtual_0c(0.0f);
    host.shadow_driver_virtual_04();
    host.shadow_map_update_from_camera_00a8f3b0(host.light_shadow_map_owner_00b7aab0(host.camera_node_19fc()));
    host.submit_main_scene_0068a0d0();
    host.game_finish_render_frame_004ca1f0();
    return true;                                              // MOV AL, 1
}

// Offset, width, bits; resolved by local/l3_envblock.py from the live decompile.
const std::array<EnvironmentBlockStore, 118> kEnvironmentBlockDefaults_004cb420{{
    {0x00C, 1, 0x00000000u}, {0x01C, 4, 0x00000000u}, {0x020, 4, 0x00000000u},
    {0x024, 4, 0x00000000u}, {0x028, 4, 0x00000000u}, {0x02C, 4, 0x3F19999Au},
    {0x030, 4, 0x42C80000u}, {0x034, 4, 0x3F800000u}, {0x044, 4, 0x00000000u},
    {0x048, 4, 0x00000000u}, {0x04C, 4, 0x3F000000u}, {0x050, 4, 0x42800000u},
    {0x054, 4, 0x42A00000u}, {0x058, 4, 0x3D23D70Au}, {0x05C, 4, 0x3F800000u},
    {0x060, 1, 0x00000000u}, {0x064, 4, 0x00000578u}, {0x068, 4, 0x40C00000u},
    {0x06C, 4, 0xC1400000u}, {0x070, 4, 0x40400000u}, {0x074, 4, 0x80D0D0FFu},
    {0x078, 4, 0x3E19999Au}, {0x07C, 4, 0x3F866666u}, {0x080, 4, 0x3F24DD2Fu},
    {0x084, 4, 0x3F23D70Au}, {0x088, 4, 0x3F2AC083u}, {0x08C, 4, 0x00000000u},
    {0x090, 4, 0x3ECCCCCDu}, {0x094, 4, 0x3ECCCCCDu}, {0x098, 4, 0x3ECCCCCDu},
    {0x09C, 4, 0x00000000u}, {0x0A0, 4, 0x3ECCCCCDu}, {0x0A4, 4, 0x3ECCCCCDu},
    {0x0A8, 4, 0x3ECCCCCDu}, {0x0AC, 4, 0x00000000u}, {0x0B0, 4, 0x3ECCCCCDu},
    {0x0B4, 4, 0x3ECCCCCDu}, {0x0B8, 4, 0x3ECCCCCDu}, {0x0BC, 4, 0x00000000u},
    {0x0C0, 4, 0x3ECCCCCDu}, {0x0C4, 4, 0x3ECCCCCDu}, {0x0C8, 4, 0x3ECCCCCDu},
    {0x0CC, 4, 0x00000000u}, {0x0D0, 4, 0x3F333333u}, {0x0D4, 4, 0x42C80000u},
    {0x0D8, 4, 0x45EA6000u}, {0x0DC, 4, 0x3F333333u}, {0x0E0, 4, 0x43480000u},
    {0x0E4, 4, 0xC2F00000u}, {0x0E8, 4, 0x46EA6000u}, {0x0EC, 4, 0x46EA6000u},
    {0x0F0, 4, 0x46EA6000u}, {0x0F4, 4, 0x46EA6000u}, {0x0F8, 4, 0x46EA6000u},
    {0x0FC, 4, 0x46EA6000u}, {0x100, 1, 0x00000001u}, {0x104, 4, 0x44480000u},
    {0x108, 4, 0x3F7851ECu}, {0x10C, 4, 0xC3480000u}, {0x110, 4, 0xC30C0000u},
    {0x114, 4, 0x3F7EB852u}, {0x118, 4, 0x3E5CE076u}, {0x11C, 4, 0x3E74F0D8u},
    {0x120, 4, 0x3E04816Fu}, {0x124, 4, 0x3F800000u}, {0x128, 4, 0x3E99999Au},
    {0x12C, 4, 0x3F24DD2Fu}, {0x130, 4, 0x3F23D70Au}, {0x134, 4, 0x3F5DF3B6u},
    {0x138, 4, 0x00000000u}, {0x140, 4, 0x00000000u}, {0x164, 4, 0x3E051EB8u},
    {0x168, 4, 0x3E8F5C29u}, {0x16C, 4, 0x3E9EB852u}, {0x170, 4, 0x3F800000u},
    {0x174, 4, 0x3E051EB8u}, {0x178, 4, 0x3E8F5C29u}, {0x17C, 4, 0x3E9EB852u},
    {0x180, 4, 0x3F800000u}, {0x184, 4, 0x3E051EB8u}, {0x188, 4, 0x3E8F5C29u},
    {0x18C, 4, 0x3E9EB852u}, {0x190, 4, 0x3F800000u}, {0x194, 4, 0x42A00000u},
    {0x198, 4, 0x3E4CCCCDu}, {0x19C, 4, 0x42A00000u}, {0x1E4, 4, 0x3F800000u},
    {0x1E8, 4, 0x3F800000u}, {0x1EC, 4, 0x3F800000u}, {0x1F0, 4, 0x3F800000u},
    {0x1F4, 4, 0x3F666666u}, {0x1F8, 4, 0x41200000u}, {0x1FC, 4, 0x428C0000u},
    {0x200, 4, 0x437A0000u}, {0x204, 4, 0x43960000u}, {0x20C, 4, 0x00000000u},
    {0x210, 4, 0x00000000u}, {0x214, 4, 0x00000000u}, {0x218, 4, 0x00000000u},
    {0x21C, 4, 0x00000000u}, {0x220, 4, 0x00000000u}, {0x224, 4, 0x41200000u},
    {0x228, 4, 0x447A0000u}, {0x22C, 4, 0x3F800000u}, {0x230, 4, 0x3F800000u},
    {0x234, 4, 0x00000000u}, {0x238, 4, 0x00000000u}, {0x23C, 4, 0x00000000u},
    {0x240, 4, 0x00000000u}, {0x244, 4, 0x00000000u}, {0x248, 4, 0x00000000u},
    {0x24C, 4, 0x00000000u}, {0x250, 4, 0x00000000u}, {0x254, 4, 0x00000000u},
    {0x258, 4, 0x00000000u}, {0x25C, 1, 0x00000001u}, {0x280, 4, 0x00000000u},
    {0x284, 4, 0x00000000u},
}};

const std::array<EnvironmentGlobalStore, 85> kEnvironmentGlobalDefaults_004cb420{{
    {0x00E18804u, 0x00000000u}, {0x00E18808u, 0x3E4CCCCDu}, {0x00E1880Cu, 0x3E4CCCCDu},
    {0x00E18810u, 0x3E4CCCCDu}, {0x00E18814u, 0x3E4CCCCDu}, {0x00E18818u, 0x3E4CCCCDu},
    {0x00E1881Cu, 0x3E4CCCCDu}, {0x00E18820u, 0x3E4CCCCDu}, {0x00E18824u, 0x3E4CCCCDu},
    {0x00E18828u, 0x3E4CCCCDu}, {0x00E1882Cu, 0x3E4CCCCDu}, {0x00E18830u, 0x3E4CCCCDu},
    {0x00E18834u, 0x3E4CCCCDu}, {0x00E18838u, 0x3E4CCCCDu}, {0x00E1883Cu, 0x3E4CCCCDu},
    {0x00E18840u, 0x3E4CCCCDu}, {0x00E18844u, 0x3E4CCCCDu}, {0x00E18848u, 0x3E4CCCCDu},
    {0x00E1884Cu, 0x3E4CCCCDu}, {0x00E18850u, 0x3E4CCCCDu}, {0x00E18854u, 0x3E4CCCCDu},
    {0x00E18858u, 0x3E4CCCCDu}, {0x00E1885Cu, 0x3E4CCCCDu}, {0x00E18860u, 0x3E4CCCCDu},
    {0x00E18864u, 0x3E4CCCCDu}, {0x00E18868u, 0x3E4CCCCDu}, {0x00E1886Cu, 0x3E4CCCCDu},
    {0x00E18870u, 0x3E4CCCCDu}, {0x00E18874u, 0x3E4CCCCDu}, {0x00E18878u, 0x3E4CCCCDu},
    {0x00E1887Cu, 0x3E4CCCCDu}, {0x00E18880u, 0x3E4CCCCDu}, {0x00E18884u, 0x3E4CCCCDu},
    {0x00E18888u, 0x3E4CCCCDu}, {0x00E1888Cu, 0x3E4CCCCDu}, {0x00E18890u, 0x3E4CCCCDu},
    {0x00E18894u, 0x3E4CCCCDu}, {0x00E18898u, 0x3E4CCCCDu}, {0x00E1889Cu, 0x3E4CCCCDu},
    {0x00E188A0u, 0x3E4CCCCDu}, {0x00E188A4u, 0x3E4CCCCDu}, {0x00E188E0u, 0x3E99999Au},
    {0x00E188E4u, 0x3C23D70Au}, {0x00E188E8u, 0x3E99999Au}, {0x00E188ECu, 0x3DCCCCCDu},
    {0x00E188F0u, 0x3E19999Au}, {0x00E188F4u, 0x3E19999Au}, {0x00E188F8u, 0x3B03126Fu},
    {0x00E188FCu, 0x3D4CCCCDu}, {0x00E18900u, 0x3CA3D70Au}, {0x00E18904u, 0x3D4CCCCDu},
    {0x00E18908u, 0x3F800000u}, {0x00E1890Cu, 0x3F800000u}, {0x00E18910u, 0x3F800000u},
    {0x00E18914u, 0x3F800000u}, {0x00E18918u, 0x3F800000u}, {0x00E1891Cu, 0x447A0000u},
    {0x00E18920u, 0x00000000u}, {0x00E18924u, 0x00000000u}, {0x00E18928u, 0x40600000u},
    {0x00E1892Cu, 0x41F00000u}, {0x00E18930u, 0x48127C00u}, {0x00E18934u, 0x3F4CCCCDu},
    {0x00E18938u, 0x40C00000u}, {0x00E1893Cu, 0x3F800000u}, {0x00E18940u, 0x40400000u},
    {0x00E18944u, 0x40C00000u}, {0x00E18948u, 0x3F800000u}, {0x00E1894Cu, 0x3F333333u},
    {0x00E18950u, 0x40000000u}, {0x00E18954u, 0x40800000u}, {0x00E18958u, 0x3CF5C28Fu},
    {0x00E1895Cu, 0x40000000u}, {0x00E18960u, 0x00000000u}, {0x00E18964u, 0x00000000u},
    {0x00E18968u, 0x00000000u}, {0x00E1896Cu, 0x3F800000u}, {0x00E18970u, 0x00000000u},
    {0x00E18974u, 0x00000000u}, {0x00E18978u, 0x00000000u}, {0x00E1897Cu, 0x3F800000u},
    {0x00E18980u, 0x00000000u}, {0x00E18984u, 0x00000000u}, {0x00E18988u, 0x00000000u},
    {0x00E1898Cu, 0x3F800000u},
}};

void scene_environment_block_construct_004cb420(EnvironmentBlockImage& block, EnvironmentGlobalsHost& host)
{
    for (const EnvironmentBlockStore& s : kEnvironmentBlockDefaults_004cb420) {
        std::memcpy(&block.bytes[s.offset], &s.bits, s.width);
    }
    // The NativeString fields, in the image's order: +13Ch "CausticsDayLight", then "white.tga"
    // into +208h, +210h, +218h, +234h, +23Ch, +244h, +24Ch, +254h and the global 00E18990.
    block.strings.clear();
    block.strings.emplace_back(0x13C, "CausticsDayLight");
    for (std::uint16_t offset : std::initializer_list<std::uint16_t>{0x208, 0x210, 0x218, 0x234, 0x23C, 0x244, 0x24C, 0x254}) {
        block.strings.emplace_back(offset, "white.tga");
    }
    host.assign_string_00e18990("white.tga");
    for (const EnvironmentGlobalStore& g : kEnvironmentGlobalDefaults_004cb420) {
        host.store_dword(g.address, g.bits);
    }
}

void game_leave_session_004cccc0(LeaveSessionHost& host)
{
    if (host.session_mode_1fe4() != 0) host.session_set_mode_0076fad0(0);
    // `where` is a global address, or a game offset below 10000h.
    host.delete_slot(0x00E18678, 0x0);
    host.delete_slot(0x00E1867C, 0x0);
    host.class_registry_clear_globals_004a9ac0();
    host.delete_slot(0x19C8, 0xC);
    host.award_registry_destroy_00e19900();
    host.delete_slot(0x00E18DB0, 0xC);
    host.delete_slot(0x00E19698, 0xC);
    host.delete_slot(0x00E1930C, 0xC);
    host.delete_slot(0x21A0, 0x0);
    host.delete_slot(0x21D8, 0x0);
    host.free_record_7188();
    host.singleton_00f87440_destroy_004c0e40();
    host.singleton_00f8753c_destroy_004c0ef0();
    host.clear_byte_719c();
    host.singleton_lifetime_unregister_00f8a304();
    host.allocation_report_00be3fd0("After GGame::deinit");
}

void game_save_next_screenshot_004cd450(std::int32_t& counter_1940, ScreenshotHost& host)
{
    std::string path;
    do {
        ++counter_1940;
        // "c:\screenshot_" (14 characters), 004CACA0 appends the decimal counter, then ".png".
        path = "c:\\screenshot_" + std::to_string(counter_1940) + ".png";
    } while (host.file_exists_fopen_rb(path));
    host.renderer_save_screenshot_110(path);
}

void scene_ensure_default_lighting_004c9ec0(DefaultLightingHost& host)
{
    void* owner = host.scene_lighting_owner_00b72110();
    if (owner == nullptr) {
        owner = host.scene_resource_create_all_lights();
        host.scene_set_resource_00b723f0(owner);
        host.release(owner);                                  // the scene keeps its reference
    }
    if (host.ambient_light_19f4() == nullptr) host.ambient_light_19f4() = host.ambient_light_create_00b7c290();
    host.owner_set_ambient_00b825d0(owner, host.ambient_light_19f4());
    if (host.directional_light_19f8() == nullptr) {
        host.directional_light_19f8() = host.directional_light_create_00b7c6b0("TestDirectionalLight");
    }
    const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    host.light_virtual_38(host.directional_light_19f8(), identity);
    host.lighting_apply_004bacd0(host.ambient_light_19f4(), host.directional_light_19f8(),
                                 host.lighting_configuration_5fc());
    host.light_virtual_50(host.directional_light_19f8(), owner, 1);
}

void anim_entity_sentity_init_00471080(float& length_18c, float& phase_190, AnimEntityInitHost& host)
{
    host.sentity_init_base_0048dbd0();
    length_18c = host.animation_length_00b6dc90(0);            // FSTP, then FLD, FST +18Ch
    // 004710B0: XOR ECX, ECX -> stream 0; U(0, length).
    phase_190 = host.random_uniform_00bd2f10_stream0(0.0f, length_18c);
}

void vehicle_visual_entity_sentity_init_00479bd0(float& field_18c, AnimEntityInitHost& host)
{
    host.sentity_init_base_0048dbd0();
    field_18c = 0.0f;
}

void scene_name_hash_iterator_begin_00466bb0(SceneNameHashIterator& it)
{
    const SceneNameHashTable& t = *it.table_00;
    if (t.count_04 == 0) {
        it.node_04 = nullptr;
        return;
    }
    it.bucket_08 = 0;
    if (t.buckets_08[0] == nullptr) {
        // The test comes before the increment, so the scan can stop at index 40h.
        while (it.bucket_08 < 0x40) {
            ++it.bucket_08;
            if (t.buckets_08[it.bucket_08] != nullptr) break;
        }
    }
    it.node_04 = t.buckets_08[it.bucket_08];
}

bool scene_name_hash_iterator_next_00466c00(SceneNameHashIterator& it)
{
    if (it.node_04 == nullptr) return false;
    if (it.node_04->next_0c != nullptr) {
        it.node_04 = it.node_04->next_0c;
        return true;
    }
    const SceneNameHashTable& t = *it.table_00;
    ++it.bucket_08;
    while (it.bucket_08 < 0x40 && t.buckets_08[it.bucket_08] == nullptr) ++it.bucket_08;
    it.node_04 = it.bucket_08 < 0x40 ? t.buckets_08[it.bucket_08] : nullptr;
    return it.node_04 != nullptr;
}

void scene_property_value_construct_vector3_00467c80(ScenePropertyVector3Value& value, const float* xyz, float w)
{
    // Word +0h = 0, byte +1h = 1, the vector, +4h = 0, byte +0h = 0, word +2h = 0, +14h = w.
    value.byte_00 = 0;
    value.byte_01 = 1;
    for (int i = 0; i < 3; ++i) value.xyz_08[i] = xyz[i];
    value.word_04 = 0;
    value.word_02 = 0;
    value.w_14 = w;
}


} // namespace bsp::cc13_sys

// oct18b-nA1: newly defined, assembly-reviewed leaves; build-only interfaces.

namespace bsp::cc13_sys {
namespace {
template<class T> T& late_view_field(void* p, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(p) + offset);
}
}
void game_camera_set_fov_004b4920(void* game, float fov,
    const std::function<void(void*, float)>& camera_set_fov_00b6fbb0) {
    void* camera = late_view_field<void*>(game, 0x19fc);
    if (camera) camera_set_fov_00b6fbb0(camera, fov);
}
void world_view_job_empty_callback_004b5070(void*) noexcept {}
void* world_view_job_scalar_delete_004b8a40(void* object, std::uint32_t flags, void*& global,
    const std::function<void(void*)>& free_00bf65ac) {
    global = nullptr;
    late_view_field<std::uint32_t>(object, 0) = 0x00ce3818;
    if (flags & 1u) free_00bf65ac(object);
    return object;
}
float game_nearest_view_distance_squared_004bb970(void* game, const float point[3], NearestViewHost& host) {
    void* camera = late_view_field<void*>(host.game_00e188a8(), 0x19fc);
    if (!(late_view_field<std::uint8_t>(camera, 0x5c) & 2)) host.refresh_transform_00b6db70(camera);
    auto distance = [point](void* object, std::uint32_t offset) {
        float d[3];
        for (unsigned i = 0; i != 3; ++i)
            d[i] = static_cast<float>(static_cast<double>(late_view_field<float>(object, offset + 4 * i)) - point[i]);
        return static_cast<float>((static_cast<double>(d[0]) * d[0] + static_cast<double>(d[1]) * d[1]) +
                                  static_cast<double>(d[2]) * d[2]);
    };
    float best = distance(camera, 0x120);
    for (unsigned i = 0; i != 8; ++i) {
        void* object = late_view_field<void*>(game, 0x18cc + i * 4);
        if (object) {
            const float candidate = distance(object, 0xac);
            // FCOMIP candidate,best / JA skips: unordered replaces best.
            if (!(candidate > best)) best = candidate;
        }
    }
    return best;
}
}
