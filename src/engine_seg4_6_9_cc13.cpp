#include "bsp/engine_seg4_6_9_cc13.hpp"

#include <cstdio>
#include <stdexcept>

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

} // namespace bsp::cc13_sys
