// Packet cc13-sweepA3: segments 0..14 leftovers, third pass (cc13_sweepA3 marker).
// See include/bsp/cc13_sweepA3.hpp and docs/CC13_SWEEPA3.md. Each body carries
// its address, the original ABI and every simplification next to it.
// Common simplification: SEH unwind actions are not modelled (no body here
// throws itself).

#include "bsp/cc13_sweepA3.hpp"

#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string.h>  // _stricmp (00BF7FBF, the CRT, bound directly)

namespace bsp::cc13_sweepA3 {

namespace {

constexpr std::uint32_t kGame = 0x00e188a8;       // [00E188A8] the game object
constexpr std::uint32_t kPlayer = 0x00e188d8;     // [00E188D8] the local player / commander
constexpr std::uint32_t kFrontend = 0x00e198ac;   // [00E198AC] front-end manager
constexpr std::uint32_t kHudManager = 0x00e198c4; // [00E198C4] (+84h the binoculars HUD)
constexpr std::uint32_t kDialogManager = 0x00f8bbcc;  // [00F8BBCC] streamed dialog manager
constexpr std::uint32_t kSoundManager = 0x00f8bbd8;   // [00F8BBD8]
constexpr std::uint32_t kSceneDatabase = 0x00e18680;  // [00E18680]

Raw raw(void* pointer) { return Raw{static_cast<std::uint8_t*>(pointer)}; }

// The input bank: [004BEC00() + 4].
Raw input_bank(SweepA3Host& host) { return raw(raw(host.input_manager_get_singleton_004bec00()).ptr(4)); }

// The inlined release of an 8-byte string {length +0, data +4}: with data
// set, 00BD1510(00419CC0(), data, length + 1, 1).
void release_string(Raw string, SweepA3Host& host) {
    if (void* const data = string.ptr(4); data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                      string.at<std::uint32_t>(0) + 1, 1);
    }
}

// The inlined construction from a literal: {0, 0}, 0041DD40(length, 1),
// then memcpy(data, text, length + 1) when data is set.
void construct_literal(Raw string, const char* text, std::uint32_t length, SweepA3Host& host) {
    string.at<std::uint32_t>(0) = 0;
    string.ptr(4) = nullptr;
    host.native_string_resize_0041dd40(string, length, 1);
    if (void* const data = string.ptr(4); data != nullptr) std::memcpy(data, text, string.at<std::uint32_t>(0) + 1);
}

// The inlined assignment: unless the same object, 0041DD40(source length, 1)
// and, when the source length (re-read) is not 0, memcpy(data, source data,
// destination length).
void assign_string(Raw destination, Raw source, SweepA3Host& host) {
    if (destination.p == source.p) return;
    host.native_string_resize_0041dd40(destination, source.at<std::uint32_t>(0), 1);
    if (source.at<std::uint32_t>(0) != 0)
        std::memcpy(destination.ptr(4), source.ptr(4), destination.at<std::uint32_t>(0));
}

// The inlined append: when the suffix length is not 0, 0041DD40(old +
// suffix length, 1) and memcpy(data + old, suffix data, suffix length).
void append_string(Raw string, Raw suffix, SweepA3Host& host) {
    const std::uint32_t added = suffix.at<std::uint32_t>(0);
    if (added == 0) return;
    const std::uint32_t old = string.at<std::uint32_t>(0);
    host.native_string_resize_0041dd40(string, old + added, 1);
    std::memcpy(static_cast<std::uint8_t*>(string.ptr(4)) + old, suffix.ptr(4), added);
}

const char* literal(std::uint32_t address, SweepA3Host& host) {
    return static_cast<const char*>(host.global_address(address));
}

// A checked index into the pointer vector at object+first (end at
// +first+4): first set and the index below the image's SAR (signed) count,
// compared unsigned.
bool remap_index_valid(Raw object, std::size_t first, std::uint32_t index) {
    const auto begin = object.at<std::uintptr_t>(first);
    if (begin == 0) return false;
    return index < static_cast<std::uint32_t>(static_cast<std::int32_t>(object.at<std::uintptr_t>(first + 4) - begin) >> 2);
}

}  // namespace

// ============================ segment 3 ====================================

namespace {

// 00441780 then, when id >= count (signed JL), 00440180(registry+4, id + 1):
// the slot array registry+4 {data +0, count +4}.
Raw device_class_slots(std::int32_t id, SweepA3Host& host) {
    const Raw slots{static_cast<std::uint8_t*>(host.device_class_registry_get_singleton_00441780()) + 4};
    if (!(id < slots.at<std::int32_t>(4))) host.device_class_registry_resize_slots_00440180(slots, id + 1);
    return slots;
}

}  // namespace

// 00443090 BSP_DeviceClass_ResolveFromLua, __fastcall(ECX = signed id, DL =
// retain selector), EAX = class, plain RET. docs/NATIVE_DEVICE_CLASS_
// RESOLUTION_ORCH4.md reads the same body. A non-null cached slot: with DL
// set, the registry is re-acquired (and resized when needed) and
// InterlockedIncrement([00CE221C]) bumps the entry's +4; the slot is
// re-read after another acquire and returned. Otherwise the globals of
// [00E188A8]+1A0Ch (00B67980) give DeviceClass (00B67800), its row [id]
// (00B67720) and the row's "Type" (00B67800); the Type text (00B662B0) is
// copied (0041E870), the row gets Got = true (00B673A0 with a "Got" string,
// released inline before any compare), then Type is compared (00425850) with
// the seven names in order. A match allocates (00BF681B; DCh, DCh, DCh, E8h,
// 18Ch, DCh, ECh) and constructs: the two rapid guns through 00442B90 with
// the vptr then set to 00CE45E0 / 00CE4614; the others through their own
// constructor (EAX taken as the class). A failed allocation gives a null
// class that is still dereferenced (no guard, as in the image). The class
// reads its row (vtable +8), +6Ch = id, the slot is (re)acquired and set, the
// Type string is released inline and the four Lua objects destroyed (Type,
// row, DeviceClass, globals); the slot is re-read after a final acquire. No
// match: the Type string is destroyed with 0041DD20, the four objects too,
// and 0 is returned.
void* device_class_resolve_from_lua_00443090(std::int32_t id, std::uint8_t retain, SweepA3Host& host) {
    Raw slots = device_class_slots(id, host);
    if (static_cast<void**>(slots.ptr(0))[id] != nullptr) {
        if (retain != 0) {
            slots = device_class_slots(id, host);
            host.interlocked_increment_import_00ce221c(raw(static_cast<void**>(slots.ptr(0))[id]).field(4));
        }
        slots = device_class_slots(id, host);
        return static_cast<void**>(slots.ptr(0))[id];
    }
    LuaObjectStorage globals;
    LuaObjectStorage table;
    LuaObjectStorage row;
    LuaObjectStorage type;
    host.lua_state_owner_get_globals_00b67980(raw(host.global_pointer(kGame)).field(0x1a0c), &globals);
    host.lua_object_get_by_name_00b67800(&globals, &table, literal(0x00ce44bc, host));  // "DeviceClass"
    host.lua_object_get_by_index_00b67720(&table, &row, id);
    host.lua_object_get_by_name_00b67800(&row, &type, literal(0x00ce4780, host));  // "Type"
    alignas(4) std::uint8_t type_storage[8];
    const Raw type_text{type_storage};
    host.native_string_construct_0041e870(type_text, host.lua_object_get_string_00b662b0(&type));
    alignas(4) std::uint8_t got_storage[8];
    const Raw got{got_storage};
    host.native_string_construct_0041e870(got, literal(0x00ce452c, host));  // "Got"
    host.lua_object_set_bool_00b673a0(&row, got, 1);
    release_string(got, host);

    void* device_class = nullptr;
    bool matched = true;
    const auto is = [&](std::uint32_t name) {
        return host.native_string_equals_cstring_insensitive_00425850(type_text, literal(name, host));
    };
    if (is(0x00ce4768)) {  // Rapid_Fixed_Slave_Gun
        if (void* const block = host.operator_new_00bf681b(0xdc); block != nullptr) {
            host.gun_descriptor_construct_common_00442b90(block);
            raw(block).at<std::uint32_t>(0) = 0x00ce45e0;
            device_class = block;
        }
    } else if (is(0x00ce4754)) {  // Rapid_Turning_Gun
        if (void* const block = host.operator_new_00bf681b(0xdc); block != nullptr) {
            host.gun_descriptor_construct_common_00442b90(block);
            raw(block).at<std::uint32_t>(0) = 0x00ce4614;
            device_class = block;
        }
    } else if (is(0x00ce4740)) {  // Single_Turning_Gun
        if (void* const block = host.operator_new_00bf681b(0xdc); block != nullptr)
            device_class = host.construct_single_turning_gun_class_00442ec0(block);
    } else if (is(0x00ce4730)) {  // BombPlatform
        if (void* const block = host.operator_new_00bf681b(0xe8); block != nullptr)
            device_class = host.construct_bomb_platform_class_00442c50(block);
    } else if (is(0x00ce471c)) {  // MultiBombPlatform
        if (void* const block = host.operator_new_00bf681b(0x18c); block != nullptr)
            device_class = host.construct_multibomb_platform_class_00442ce0(block);
    } else if (is(0x00ce4704)) {  // Depth_Charge_Launcher
        if (void* const block = host.operator_new_00bf681b(0xdc); block != nullptr)
            device_class = host.construct_depth_charge_launcher_class_00442f50(block);
    } else if (is(0x00ce46f8)) {  // Catapult
        if (void* const block = host.operator_new_00bf681b(0xec); block != nullptr)
            device_class = host.construct_catapult_class_00442ff0(block);
    } else {
        matched = false;
    }
    if (!matched) {
        host.native_string_destroy_storage_0041dd20(type_text);
        host.lua_object_destruct_00b67700(&type);
        host.lua_object_destruct_00b67700(&row);
        host.lua_object_destruct_00b67700(&table);
        host.lua_object_destruct_00b67700(&globals);
        return nullptr;
    }
    host.vcall_device_class_read_lua_8(device_class, &row);
    raw(device_class).at<std::int32_t>(0x6c) = id;
    slots = device_class_slots(id, host);
    static_cast<void**>(slots.ptr(0))[id] = device_class;
    release_string(type_text, host);
    host.lua_object_destruct_00b67700(&type);
    host.lua_object_destruct_00b67700(&row);
    host.lua_object_destruct_00b67700(&table);
    host.lua_object_destruct_00b67700(&globals);
    slots = device_class_slots(id, host);
    return static_cast<void**>(slots.ptr(0))[id];
}

namespace {

std::uint32_t float_bits(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

std::uint32_t pointer_bits(const void* pointer) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
}

// The low byte of a dword, the rest kept (the image's MOV byte stores into
// a frame slot that still holds the previous value).
void set_low_byte(std::uint32_t& slot, std::uint8_t byte) { slot = (slot & 0xffffff00u) | byte; }

}  // namespace

// 0044B5B0 BSP_PanelSequence_DescribeProperties, __thiscall(ECX = panel
// sequence, sink), RET 4; sole caller 0044BD30. The sink's +0Ch writes
// (key {0, name}, value) and +4/+8 open and close groups; 00BD5680 compares a
// value with its default and the write is skipped when equal. Two frame
// slots hold value {tag, v} and default {tag, v}: tag-3 (byte) entries store
// only their low byte, so the upper bytes keep the previous entry's dword
// (modelled here by keeping the two slots across entries). "priority" {2,
// +4} unconditionally; "requestTime" {2, +8} vs {2, [00F876A4]};
// "suppressInterruptMsg" {3, +10h} vs {3, 0}; "defaultPause" {2, +0Ch} vs
// {2, [[00E188A8]+21E4h]+30h}; "nextUnit" {1, +24h} vs {1, 0}; group
// "sequence": per element i of +18h/+1Ch (checked) group {1, i + 1}, the
// element's +0Ch(sink), close; close; "breakRequested" {3, +38h} vs {3, 0};
// "lastPanel" {1, +3Ch} vs {1, 0}; group "panelStates": per 14h record i of
// +2Ch/+30h (signed /14h, checked) group {1, i}, "active" {3, byte +0},
// "name" {0, +8 or "" 00E17BFB}, "characterID" {0, +10h or ""}, close;
// close. The "active" value's upper bytes are uninitialised stack in the
// image; zero here. The record byte offset lives in the sink's own argument
// slot in the image (ESI keeps the sink).
void panel_sequence_describe_properties_0044b5b0(Raw sequence, void* sink, SweepA3Host& host) {
    const auto key = [&](std::uint32_t name) { return TaggedValue{0, pointer_bits(literal(name, host))}; };
    host.vcall_sink_write_c(sink, key(0x00ce4a84), TaggedValue{2, sequence.at<std::uint32_t>(4)});
    TaggedValue value{2, sequence.at<std::uint32_t>(8)};
    TaggedValue fallback{2, float_bits(*static_cast<const float*>(host.global_address(0x00f876a4)))};
    if (!host.tagged_value_equals_00bd5680(&value, &fallback)) host.vcall_sink_write_c(sink, key(0x00ce4a78), value);
    value.tag = 3;
    fallback.tag = 3;
    set_low_byte(value.value, sequence.at<std::uint8_t>(0x10));
    set_low_byte(fallback.value, 0);
    if (!host.tagged_value_equals_00bd5680(&value, &fallback)) host.vcall_sink_write_c(sink, key(0x00ce4a60), value);
    fallback = TaggedValue{2, raw(raw(host.global_pointer(kGame)).ptr(0x21e4)).at<std::uint32_t>(0x30)};
    value = TaggedValue{2, sequence.at<std::uint32_t>(0xc)};
    if (!host.tagged_value_equals_00bd5680(&value, &fallback)) host.vcall_sink_write_c(sink, key(0x00ce4a50), value);
    value = TaggedValue{1, sequence.at<std::uint32_t>(0x24)};
    fallback = TaggedValue{1, 0};
    if (!host.tagged_value_equals_00bd5680(&value, &fallback)) host.vcall_sink_write_c(sink, key(0x00ce4a44), value);

    host.vcall_sink_begin_4(sink, key(0x00ce4a38));  // "sequence"
    for (std::uint32_t i = 0; remap_index_valid(sequence, 0x18, i); ++i) {
        if (!remap_index_valid(sequence, 0x18, i)) host.invalid_parameter_noinfo_00bf6713();
        void* const element = static_cast<void**>(sequence.ptr(0x18))[i];
        host.vcall_sink_begin_4(sink, TaggedValue{1, i + 1});
        host.vcall_describe_c(element, sink);
        host.vcall_sink_end_8(sink);
    }
    host.vcall_sink_end_8(sink);

    value.tag = 3;
    fallback.tag = 3;
    set_low_byte(fallback.value, 0);
    set_low_byte(value.value, sequence.at<std::uint8_t>(0x38));
    if (!host.tagged_value_equals_00bd5680(&value, &fallback)) host.vcall_sink_write_c(sink, key(0x00ce4a28), value);
    value = TaggedValue{1, sequence.at<std::uint32_t>(0x3c)};
    fallback = TaggedValue{1, 0};
    if (!host.tagged_value_equals_00bd5680(&value, &fallback)) host.vcall_sink_write_c(sink, key(0x00ce4a1c), value);

    host.vcall_sink_begin_4(sink, key(0x00ce4a10));  // "panelStates"
    const auto states_count = [&]() -> std::uint32_t {
        const auto first = sequence.at<std::uintptr_t>(0x2c);
        return static_cast<std::uint32_t>(static_cast<std::int32_t>(sequence.at<std::uintptr_t>(0x30) - first) / 0x14);
    };
    for (std::uint32_t i = 0; sequence.ptr(0x2c) != nullptr && i < states_count(); ++i) {
        host.vcall_sink_begin_4(sink, TaggedValue{1, i});
        if (sequence.ptr(0x2c) == nullptr || i >= states_count()) host.invalid_parameter_noinfo_00bf6713();
        const Raw state{static_cast<std::uint8_t*>(sequence.ptr(0x2c)) + i * 0x14};
        host.vcall_sink_write_c(sink, key(0x00ce4a08), TaggedValue{3, state.at<std::uint8_t>(0)});
        const void* name = state.ptr(8);
        if (name == nullptr) name = host.global_address(0x00e17bfb);
        host.vcall_sink_write_c(sink, key(0x00ce49b8), TaggedValue{0, pointer_bits(name)});
        const void* character = state.ptr(0x10);
        if (character == nullptr) character = host.global_address(0x00e17bfb);
        host.vcall_sink_write_c(sink, key(0x00ce49fc), TaggedValue{0, pointer_bits(character)});
        host.vcall_sink_end_8(sink);
    }
    host.vcall_sink_end_8(sink);
}

// ============================ segment 5 ====================================

// 0045ECA0, stdcall-shaped: [ESP+4] = tokenizer, [ESP+8] = owner, RET 8 (ECX
// is not read on entry); sole caller 004602C0 (BSP_EngineMovie_ParseFile).
// Reads one spline point: Point { PointType = int, ActivateTime = float,
// Comment = token, PointPos = 3 floats, optional Alpha = float (only when the
// peeked token equals "Alpha" by 00438E10; default 0.0f), InTangent = 3
// floats, OutTangent = 3 floats } (00CE4EAC/4D38/4EA0/4E5C/4E54/4E94/4E8C/
// 4E80/4E74/4CD4). Each "=" is consumed by a bare 008D9980. The ok byte of
// every 008D9AD0/008D9B40 call is the tokenizer's own argument slot in the
// image (never re-read: ESI keeps the tokenizer); a local here. The comment
// is sized with 0041DD40(strlen or 0, 0) and copied (length bytes) when its
// data is set; a by-value copy (0041DD40(length, 1), memcpy when the length
// is not 0) goes to 0045B420(owner, position, time, in, out, type, alpha,
// copy), and the local is released through the pool (length + 1).
void movie_spline_read_point_0045eca0(void* tokenizer, void* owner, SweepA3Host& host) {
    std::uint8_t ok = 0;
    float alpha = 0.0f;
    alignas(4) std::uint8_t comment_storage[8] = {};
    const Raw comment{comment_storage};
    host.scene_tokenizer_expect_008d9930(tokenizer, literal(0x00ce4eac, host));  // "Point"
    host.scene_tokenizer_expect_008d9930(tokenizer, literal(0x00ce4d38, host));  // "{"
    host.scene_tokenizer_expect_008d9930(tokenizer, literal(0x00ce4ea0, host));  // "PointType"
    host.scene_tokenizer_read_008d9980(tokenizer);
    const std::int32_t type = host.scene_tokenizer_read_int_008d9ad0(tokenizer, &ok);
    host.scene_tokenizer_expect_008d9930(tokenizer, literal(0x00ce4e5c, host));  // "ActivateTime"
    host.scene_tokenizer_read_008d9980(tokenizer);
    const float time = host.scene_tokenizer_read_float_008d9b40(tokenizer, &ok);
    host.scene_tokenizer_expect_008d9930(tokenizer, literal(0x00ce4e54, host));  // "Comment"
    host.scene_tokenizer_read_008d9980(tokenizer);
    const char* const text = host.scene_tokenizer_read_008d9980(tokenizer);
    host.native_string_resize_0041dd40(comment, text != nullptr ? static_cast<std::uint32_t>(std::strlen(text)) : 0,
                                       0);
    void* const comment_data = comment.ptr(4);
    const std::uint32_t comment_length = comment.at<std::uint32_t>(0);
    if (comment_data != nullptr) std::memcpy(comment_data, text, comment_length);
    float position[3];
    host.scene_tokenizer_expect_008d9930(tokenizer, literal(0x00ce4e94, host));  // "PointPos"
    host.scene_tokenizer_read_008d9980(tokenizer);
    for (float& v : position) v = host.scene_tokenizer_read_float_008d9b40(tokenizer, &ok);
    if (host.cstring_compare_insensitive_00438e10(host.scene_tokenizer_peek_token_008d8a70(tokenizer),
                                                  literal(0x00ce4e8c, host)) == 0) {  // "Alpha"
        host.scene_tokenizer_expect_008d9930(tokenizer, literal(0x00ce4e8c, host));
        host.scene_tokenizer_read_008d9980(tokenizer);
        alpha = host.scene_tokenizer_read_float_008d9b40(tokenizer, &ok);
    }
    float in_tangent[3];
    host.scene_tokenizer_expect_008d9930(tokenizer, literal(0x00ce4e80, host));  // "InTangent"
    host.scene_tokenizer_read_008d9980(tokenizer);
    for (float& v : in_tangent) v = host.scene_tokenizer_read_float_008d9b40(tokenizer, &ok);
    float out_tangent[3];
    host.scene_tokenizer_expect_008d9930(tokenizer, literal(0x00ce4e74, host));  // "OutTangent"
    host.scene_tokenizer_read_008d9980(tokenizer);
    for (float& v : out_tangent) v = host.scene_tokenizer_read_float_008d9b40(tokenizer, &ok);
    host.scene_tokenizer_expect_008d9930(tokenizer, literal(0x00ce4cd4, host));  // "}"

    alignas(4) std::uint8_t copy_storage[8] = {};
    const Raw copy{copy_storage};
    assign_string(copy, comment, host);
    host.movie_spline_add_point_0045b420(owner, position, time, in_tangent, out_tangent, type, alpha, copy);
    if (comment_data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), comment_data,
                                                      comment_length + 1, 1);
    }
}

// 00456DD0 BSP_MovieTrack_BindEntities, __thiscall(ECX = track), plain RET
// (EBP frame); track vtable slot +4, reached from 00457160. The entity named
// by +120h is looked up with 00925180([game+19CCh], +120h): found -> a
// squadron (+5Ch(18h)) is replaced by its +3D0h, +11Ch = it and its byte
// +61h = 1; not found -> the message "EngineMovie: Unknown entity reference:
// " (0041E870) + name + "!!" (0041E870 into a second string, then appended)
// is built and released without being shown (no log call in the image).
// Then the action list (+108h, head +10Ch) is walked with the track's own
// iterator +114h (container) / +118h (node), every checked-iterator test
// reporting through 00BF6713: for each action (node+8) whose type entry in
// [00E18554] (58h-byte records, signed count, index action+10h checked) has
// byte +10h set, action+14h = 00925180(world, action+18h) and the message
// "Unknown entity reference: " (inline 1Ah) + name + "!!" (inline 2) is
// built and released the same way, found or not. The node advances by its
// +0 link.
void movie_track_bind_entities_00456dd0(Raw track, SweepA3Host& host) {
    const Raw name{track.field(0x120)};
    void* entity = host.world_find_entity_by_name_00925180(raw(host.global_pointer(kGame)).ptr(0x19cc), name);
    if (entity != nullptr) {
        if (host.vcall_entity_is_kind_5c(entity, 0x18)) entity = raw(entity).ptr(0x3d0);
        track.ptr(0x11c) = entity;
        raw(entity).at<std::uint8_t>(0x61) = 1;
    } else {
        alignas(4) std::uint8_t message_storage[8];
        alignas(4) std::uint8_t bang_storage[8];
        const Raw message{message_storage};
        const Raw bang{bang_storage};
        host.native_string_construct_0041e870(message, literal(0x00ce4c30, host));
        append_string(message, name, host);
        host.native_string_construct_0041e870(bang, literal(0x00ce4c2c, host));  // "!!"
        append_string(message, bang, host);
        release_string(bang, host);
        release_string(message, host);
    }
    void* const list = track.field(0x108);
    track.ptr(0x114) = list;
    track.ptr(0x118) = raw(track.ptr(0x10c)).ptr(0);
    for (;;) {
        void* const head = track.ptr(0x10c);
        if (track.ptr(0x114) == nullptr || track.ptr(0x114) != list) host.invalid_parameter_noinfo_00bf6713();
        if (track.ptr(0x118) == head) break;
        if (track.ptr(0x114) == nullptr) host.invalid_parameter_noinfo_00bf6713();
        if (track.ptr(0x118) == raw(track.ptr(0x114)).ptr(4)) host.invalid_parameter_noinfo_00bf6713();
        const Raw action = raw(raw(track.ptr(0x118)).ptr(8));
        const std::uint32_t type = action.at<std::uint32_t>(0x10);
        const auto types = reinterpret_cast<std::uintptr_t>(host.global_pointer(0x00e18554));
        if (types == 0 ||
            type >= static_cast<std::uint32_t>(
                        static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(host.global_pointer(0x00e18558)) -
                                                  types) /
                        0x58))
            host.invalid_parameter_noinfo_00bf6713();
        if (raw(host.global_pointer(0x00e18554)).at<std::uint8_t>(type * 0x58 + 0x10) != 0) {
            const Raw refer{action.field(0x18)};
            action.ptr(0x14) =
                host.world_find_entity_by_name_00925180(raw(host.global_pointer(kGame)).ptr(0x19cc), refer);
            alignas(4) std::uint8_t message_storage[8];
            alignas(4) std::uint8_t bang_storage[8];
            const Raw message{message_storage};
            const Raw bang{bang_storage};
            construct_literal(message, literal(0x00ce4c10, host), 0x1a, host);
            append_string(message, refer, host);
            construct_literal(bang, literal(0x00ce4c2c, host), 2, host);
            append_string(message, bang, host);
            release_string(bang, host);
            release_string(message, host);
        }
        if (track.ptr(0x114) == nullptr) host.invalid_parameter_noinfo_00bf6713();
        if (track.ptr(0x118) == raw(track.ptr(0x114)).ptr(4)) host.invalid_parameter_noinfo_00bf6713();
        track.ptr(0x118) = raw(track.ptr(0x118)).ptr(0);
    }
}

// 004602C0 BSP_EngineMovie_ParseFile, __thiscall(ECX = movie, const
// NativeString* path), RET 4; sole caller 004CCE50. A local copy of the path
// (inline assignment) is checked with 00BDF4C0([0109CEEC], &copy); a miss
// sets movie+60h = 5. Otherwise a tokenizer is built in operator new(838h)
// (008D9CF0 with a by-value copy of the ARGUMENT's text, "" 00E18511 when
// null, and the delimiters ";{}=:(,)" 00CE4F40; null when the allocation
// failed, then used unguarded as in the image) and the file is read:
// Animations { (0045DDA0 resets the movie) Movielen = float -> movie+0,
// AspectR = token -> a temporary assigned to movie+58h, then per Animation:
// { Entitypath = token (strcpy_s into a 400h frame buffer), 0045EFB0(movie,
// buffer by value) -> track, PointList { 0045D8A0(track, buffer by value),
// every Point through 0045ECA0(tokenizer, track) (this packet), } ActionList
// { every Action through 0045E5E0(movie, tokenizer, track) } }, the track
// appended to movie+64h (inline when capacity remains, else 00460120; first >
// last reported) } and the closing "}". The tokenizer is deleted (slot 0,
// flag 1), every track's slot +8 runs (checked index), 0045C5B0 rewinds the
// movie and the AspectR temporary is released. The path copy is released on
// both arms.
void engine_movie_parse_file_004602c0(Raw movie, Raw path, SweepA3Host& host) {
    alignas(4) std::uint8_t copy_storage[8] = {};
    const Raw copy{copy_storage};
    assign_string(copy, path, host);
    if (!host.vfs_resolve_existing_name_00bdf4c0(host.global_pointer(0x0109ceec), copy)) {
        movie.at<std::int32_t>(0x60) = 5;
        release_string(copy, host);
        return;
    }
    void* tokenizer = nullptr;
    if (void* const block = host.operator_new_00bf681b(0x838); block != nullptr) {
        const char* source = static_cast<const char*>(path.ptr(4));
        if (source == nullptr) source = literal(0x00e18511, host);
        alignas(4) std::uint8_t by_value_storage[8];
        const Raw by_value{by_value_storage};
        host.native_string_construct_0041e870(by_value, source);
        tokenizer = host.scene_tokenizer_construct_008d9cf0(block, by_value, literal(0x00ce4f40, host));
    }
    const auto expect = [&](std::uint32_t token) { host.scene_tokenizer_expect_008d9930(tokenizer, literal(token, host)); };
    const auto peek_is = [&](std::uint32_t token) {
        return host.cstring_compare_insensitive_00438e10(host.scene_tokenizer_peek_token_008d8a70(tokenizer),
                                                         literal(token, host)) == 0;
    };
    std::uint8_t ok = 0;
    expect(0x00ce4f34);  // "Animations"
    expect(0x00ce4d38);  // "{"
    host.engine_movie_reset_0045dda0(movie.p);
    expect(0x00ce4f28);  // "Movielen"
    host.scene_tokenizer_read_008d9980(tokenizer);
    movie.at<float>(0) = host.scene_tokenizer_read_float_008d9b40(tokenizer, &ok);
    expect(0x00ce4f20);  // "AspectR"
    host.scene_tokenizer_read_008d9980(tokenizer);
    alignas(4) std::uint8_t aspect_storage[8];
    const Raw aspect{aspect_storage};
    host.native_string_construct_0041e870(aspect, host.scene_tokenizer_read_008d9980(tokenizer));
    assign_string(Raw{movie.field(0x58)}, aspect, host);
    while (peek_is(0x00ce4f14)) {  // "Animation"
        expect(0x00ce4f14);
        expect(0x00ce4d38);
        expect(0x00ce4f08);  // "Entitypath"
        host.scene_tokenizer_read_008d9980(tokenizer);
        char buffer[0x400];
        strcpy_s(buffer, sizeof buffer, host.scene_tokenizer_read_008d9980(tokenizer));
        alignas(4) std::uint8_t name_storage[8];
        construct_literal(Raw{name_storage}, buffer, static_cast<std::uint32_t>(std::strlen(buffer)), host);
        void* track = host.engine_movie_create_track_0045efb0(movie.p, Raw{name_storage});
        expect(0x00ce4efc);  // "PointList"
        expect(0x00ce4d38);
        alignas(4) std::uint8_t entity_storage[8];
        construct_literal(Raw{entity_storage}, buffer, static_cast<std::uint32_t>(std::strlen(buffer)), host);
        host.movie_track_set_entity_path_0045d8a0(track, Raw{entity_storage});
        while (peek_is(0x00ce4eac)) movie_spline_read_point_0045eca0(tokenizer, track, host);  // "Point"
        expect(0x00ce4cd4);  // "}"
        expect(0x00ce4ef0);  // "ActionList"
        expect(0x00ce4d38);
        while (peek_is(0x00ce4e6c)) host.movie_animation_read_action_0045e5e0(movie.p, tokenizer, track);  // "Action"
        expect(0x00ce4cd4);
        expect(0x00ce4cd4);
        const Raw tracks{movie.field(0x64)};
        const auto first = tracks.at<std::uintptr_t>(4);
        const std::uint32_t count =
            first == 0 ? 0 : static_cast<std::uint32_t>(static_cast<std::int32_t>(tracks.at<std::uintptr_t>(8) - first) >> 2);
        if (first != 0 &&
            count < static_cast<std::uint32_t>(static_cast<std::int32_t>(tracks.at<std::uintptr_t>(0xc) - first) >> 2)) {
            *reinterpret_cast<void**>(tracks.at<std::uintptr_t>(8)) = track;
            tracks.at<std::uintptr_t>(8) += 4;
        } else {
            void* const last = tracks.ptr(8);
            if (first > reinterpret_cast<std::uintptr_t>(last)) host.invalid_parameter_noinfo_00bf6713();
            alignas(4) std::uint8_t result[8];
            host.track_vector_insert_one_00460120(tracks, result, tracks.p, last, &track);
        }
    }
    expect(0x00ce4cd4);
    if (tokenizer != nullptr) host.vcall_destroy_0(tokenizer, 1);
    for (std::uint32_t i = 0; remap_index_valid(movie, 0x68, i); ++i) {
        if (!remap_index_valid(movie, 0x68, i)) host.invalid_parameter_noinfo_00bf6713();
        host.vcall_track_finish_8(static_cast<void**>(movie.ptr(0x68))[i]);
    }
    host.engine_movie_rewind_0045c5b0(movie.p);
    release_string(aspect, host);
    release_string(copy, host);
}

// 0045D480 BSP_MovieActionType_Construct, __thiscall(ECX = 58h record; eight
// NativeStrings by value and six byte flags in dword slots, in the order of
// the parameters), RET 58h, EAX = record; sole caller 00460B80 (through
// 00460AD0). The eight strings of the record (+0, +8, +14h, +20h, +2Ch, +38h,
// +40h, +4Ch) are zeroed (the flags are not), then assigned in order with the
// flags stored where the image stores them: +10h after the refer label, +1Ch
// / +28h / +34h together after the third number label, +48h after the enum
// label, +54h after the offset label. Each assignment is the inlined one
// (0041DD40(source length, 1), memcpy of the destination length when the
// source length is not 0). The eight argument strings are released through
// the pool in argument order. include/bsp/engine_movie_actions.hpp holds the
// table of values the 32 calls pass, not this constructor.
Raw movie_action_type_construct_0045d480(Raw record, Raw name, Raw refer_label, std::uint8_t refer_flag,
                                         Raw num1_label, std::uint8_t num1_flag, Raw num2_label,
                                         std::uint8_t num2_flag, Raw num3_label, std::uint8_t num3_flag,
                                         Raw enum_label, Raw enum_options, std::uint8_t enum_flag,
                                         Raw offset_label, std::uint8_t offset_flag, SweepA3Host& host) {
    for (const std::size_t offset : {0x0u, 0x8u, 0x14u, 0x20u, 0x2cu, 0x38u, 0x40u, 0x4cu}) {
        record.at<std::uint32_t>(offset) = 0;
        record.ptr(offset + 4) = nullptr;
    }
    assign_string(record, name, host);
    assign_string(Raw{record.field(0x8)}, refer_label, host);
    record.at<std::uint8_t>(0x10) = refer_flag;
    assign_string(Raw{record.field(0x14)}, num1_label, host);
    assign_string(Raw{record.field(0x20)}, num2_label, host);
    assign_string(Raw{record.field(0x2c)}, num3_label, host);
    record.at<std::uint8_t>(0x1c) = num1_flag;
    record.at<std::uint8_t>(0x28) = num2_flag;
    record.at<std::uint8_t>(0x34) = num3_flag;
    assign_string(Raw{record.field(0x38)}, enum_label, host);    // the label, e.g. "Type:"
    assign_string(Raw{record.field(0x40)}, enum_options, host);  // the '|' options
    record.at<std::uint8_t>(0x48) = enum_flag;
    assign_string(Raw{record.field(0x4c)}, offset_label, host);
    record.at<std::uint8_t>(0x54) = offset_flag;
    for (const Raw argument : {name, refer_label, num1_label, num2_label, num3_label, enum_label, enum_options,
                               offset_label})
        release_string(argument, host);
    return record;
}

// 0045F600 BSP_EngineMovie_Stop, __thiscall(ECX = movie), plain RET; callers
// 00427190, 0045F970, 004C6B20, 004DA780. movie+4 = movie+0, +60h = 4;
// 00A77760([00F8BBCC], 1); with [00F8BBD8]+6Ch not 0.0 (an unordered NaN also
// enters: LAHF/TEST 44h/JNP) it is zeroed and 00A7A440 gets +4Ch; [00F8BBCC]
// slot +4(0.0f); 00735B50([00E1AE90]); with [00E18680] set, 0046B730 on it;
// 004518B0 on [game+21E4h] (sweepA's reconstruction); with [00E18680] set
// (re-read), 0045F520(game, "luaPrecacheUnits", 0); the twelve class-table
// resets (004423A0 is sweepA2's reconstruction); with [[game+1A08h]+4] set,
// 006B8AD0 on it ("collectgarbage(\"collect\")", 0, 0, 2); game+193Ch = 0;
// 004C3CB0(game). "luaStageInit" is looked up in the globals (00B67980,
// 00B67800; 00B66200 runs on the object 00B67800 returns, its out argument)
// and both objects are destroyed. When it is a function and [00E18680] is
// set: the source is the string at [game+5FCh]+90Ch, or a new empty string
// when +5FCh is null (released at the end only then, the image's flag bit 4);
// a file block (frame space 1Ch bytes) is constructed (00BE0A30(path, 1))
// over "4_" + 004CD7F0(source, 1); the three temporaries are released
// through the pool; 0045F520(game, "luaStageInitMulti", 0), 0045F440(game,
// "luaStageInit", 0); 00BDCB30 destroys the block. Then
// [[00E198C4]+84h]+20h and +70h get slot +34h(0) (the global re-read for
// each), 00A77760([00F8BBCC], 0), 00B72030([game+19ECh]) and 004B46B0(game).
// Shared RNG: no direct 00BD2F10 call.
void engine_movie_stop_0045f600(Raw movie, SweepA3Host& host) {
    movie.at<float>(4) = movie.at<float>(0);
    movie.at<std::int32_t>(0x60) = 4;
    host.streamed_dialog_set_all_paused_00a77760(host.global_pointer(kDialogManager), 1);
    const Raw sound = raw(host.global_pointer(kSoundManager));
    if (!(sound.at<float>(0x6c) == 0.0f)) {
        sound.at<float>(0x6c) = 0.0f;
        host.sound_manager_apply_global_level_00a7a440(host.global_pointer(kSoundManager),
                                                       raw(host.global_pointer(kSoundManager)).at<float>(0x4c));
    }
    host.vcall_set_float_4(host.global_pointer(kDialogManager), 0.0f);
    host.sound_frame_service_inline_00735b50(host.global_pointer(0x00e1ae90));
    if (void* const database = host.global_pointer(kSceneDatabase); database != nullptr)
        host.scene_database_kill_pass_0046b730(database);
    cc13_sweepA::panel_owner_reset_004518b0(raw(raw(host.global_pointer(kGame)).ptr(0x21e4)), host);
    if (host.global_pointer(kSceneDatabase) != nullptr)
        host.game_call_lua_entry_point_forced_0045f520(host.global_pointer(kGame), literal(0x00ce4edc, host), 0);
    host.vehicle_class_registry_clear_lua_class_tables_0095ca70();
    cc13_sweepA2::device_class_registry_clear_lua_class_tables_004423a0(host);
    host.bullet_classes_reload_from_lua_006e9c50();
    host.stationary_reset_lua_class_table_0084eb60();
    host.wreck_class_clear_lua_global_004b1890();
    host.soldier_class_clear_lua_global_004af930();
    host.buoy_class_reset_lua_table_00474200();
    host.cloud_class_reset_lua_table_00476410();
    host.trivial_body_004708a0();
    host.plane_part_class_clear_lua_global_004a7120();
    host.marker_classes_reset_lua_table_006d74b0();
    host.effects_clear_lua_table_00869d20();
    if (void* const machine = raw(raw(host.global_pointer(kGame)).ptr(0x1a08)).ptr(4); machine != nullptr)
        host.lua_machine_run_string_006b8ad0(machine, literal(0x00ce4de0, host), nullptr, nullptr, 2);
    raw(host.global_pointer(kGame)).at<std::uint8_t>(0x193c) = 0;
    host.game_build_local_player_unit_lists_004c3cb0(host.global_pointer(kGame));

    LuaObjectStorage globals;
    LuaObjectStorage entry;
    LuaObjectStorage* const table =
        host.lua_state_owner_get_globals_00b67980(raw(host.global_pointer(kGame)).field(0x1a0c), &globals);
    host.lua_object_get_by_name_00b67800(table, &entry, literal(0x00ce4ecc, host));  // "luaStageInit"
    const bool stage_init =
        host.lua_object_is_function_00b66200(&entry) && host.global_pointer(kSceneDatabase) != nullptr;
    host.lua_object_destruct_00b67700(&entry);
    host.lua_object_destruct_00b67700(&globals);

    if (stage_init) {
        const Raw game = raw(host.global_pointer(kGame));
        alignas(4) std::uint8_t empty_storage[8];
        const Raw empty{empty_storage};
        bool built_empty = false;
        Raw source{nullptr};
        if (game.ptr(0x5fc) != nullptr) {
            source = Raw{static_cast<std::uint8_t*>(game.ptr(0x5fc)) + 0x90c};
        } else {
            host.native_string_construct_0041e870(empty, literal(0x00ce3a0c, host));  // ""
            source = empty;
            built_empty = true;
        }
        alignas(4) std::uint8_t short_storage[8];
        alignas(4) std::uint8_t prefix_storage[8];
        alignas(4) std::uint8_t path_storage[8];
        alignas(4) std::uint8_t block[0x1c];
        const Raw short_name = host.scene_derive_short_name_004cd7f0(Raw{short_storage}, source, 1);
        const Raw prefix{prefix_storage};
        host.native_string_construct_0041e870(prefix, literal(0x00ce4ec8, host));  // "4_"
        const Raw path = host.native_string_concat_004261a0(prefix, Raw{path_storage}, short_name);
        host.file_block_construct_00be0a30(block, path, 1);
        release_string(Raw{path_storage}, host);
        release_string(prefix, host);
        release_string(Raw{short_storage}, host);
        if (built_empty) release_string(empty, host);
        host.game_call_lua_entry_point_forced_0045f520(host.global_pointer(kGame), literal(0x00ce4eb4, host), 0);
        host.game_call_lua_entry_point_thread_safe_0045f440(host.global_pointer(kGame), literal(0x00ce4ecc, host), 0);
        host.file_block_destroy_00bdcb30(block);
    }
    host.vcall_set_visible_34(raw(raw(host.global_pointer(kHudManager)).ptr(0x84)).ptr(0x20), 0);
    host.vcall_set_visible_34(raw(raw(host.global_pointer(kHudManager)).ptr(0x84)).ptr(0x70), 0);
    host.streamed_dialog_set_all_paused_00a77760(host.global_pointer(kDialogManager), 0);
    host.scene_build_quad_trees_00b72030(raw(host.global_pointer(kGame)).ptr(0x19ec));
    host.allocator_list_trim_all_004b46b0(host.global_pointer(kGame));
}

// ============================ segment 7 ====================================

namespace {

// The checked dereference of a TrafficConfig tree iterator {container, node}
// (three reports, as inlined twice per pass), then the compare / replacement
// strings: mode A (+16Ch == 0) compares node+14h and replaces from +0Ch,
// mode B the reverse.
void remap_select_strings(Raw iterator, bool mode_a, Raw* compare, Raw* replacement, SweepA3Host& host) {
    const Raw container = raw(iterator.ptr(0));
    void* const node = iterator.ptr(4);
    if (container.p == nullptr) host.invalid_parameter_noinfo_00bf6713();
    if (node == container.ptr(4)) host.invalid_parameter_noinfo_00bf6713();
    if (node == container.ptr(4)) host.invalid_parameter_noinfo_00bf6713();
    *compare = Raw{static_cast<std::uint8_t*>(node) + (mode_a ? 0x14 : 0xc)};
    *replacement = Raw{static_cast<std::uint8_t*>(node) + (mode_a ? 0xc : 0x14)};
}

}  // namespace

// 00487470, __fastcall(ECX = scene traffic runtime), plain RET (EBP frame);
// callers 0048A650 and 004A50D0. docs/SCENE_TRAFFIC_RUNTIME.md "the side
// remap". config = [[00E188A8]+21D0h]. For every soldier template slot
// (vector +F4h/+F8h) the tree at config+40h is walked (begin = head+0, end =
// head = [container+4], 00480620 to advance; every checked-iterator test of
// the image reports through 00BF6713): the compare string matches the
// template's name (+4 length, +8 data) when the lengths are equal and either
// both are empty or _stricmp(compare, name) == 0; a match stores 004B1400
// (replacement) into the slot when that is not null. Then, with config
// re-read, every vehicle slot (+E4h/+E8h) takes class = [slot+28h] (00479BC0
// is MOV EAX,[ECX+28h]; RET, read inline) and walks config+4Ch: a match is a
// null compare data with a null or empty class name +54h, a non-null compare
// with a null name and length 0, or _stricmp(compare data, name) == 0; it
// stores 0047B3C0(00965480(replacement)) when 00965480 is not null. Mode A is
// +16Ch == 0 (SETZ at 00487491).
void traffic_runtime_remap_side_templates_00487470(Raw runtime, SweepA3Host& host) {
    void* const soldiers = raw(raw(host.global_pointer(kGame)).ptr(0x21d0)).field(0x40);
    const bool mode_a = runtime.at<std::int32_t>(0x16c) == 0;
    for (std::uint32_t i = 0; remap_index_valid(runtime, 0xf4, i); ++i) {
        if (!remap_index_valid(runtime, 0xf4, i)) host.invalid_parameter_noinfo_00bf6713();
        void** const slot = static_cast<void**>(runtime.ptr(0xf4)) + i;
        alignas(4) std::uint8_t iterator_storage[8];
        const Raw iterator{iterator_storage};
        iterator.ptr(0) = soldiers;
        iterator.ptr(4) = raw(raw(soldiers).ptr(4)).ptr(0);
        for (;;) {
            void* const head = raw(soldiers).ptr(4);
            if (iterator.ptr(0) == nullptr || iterator.ptr(0) != soldiers) host.invalid_parameter_noinfo_00bf6713();
            if (iterator.ptr(4) == head) break;
            Raw compare{nullptr};
            Raw replacement{nullptr};
            remap_select_strings(iterator, mode_a, &compare, &replacement, host);
            const Raw templ = raw(*slot);
            const std::uint32_t length = templ.at<std::uint32_t>(4);
            if (compare.at<std::uint32_t>(0) == length) {
                std::int32_t order;
                if (compare.at<std::uint32_t>(0) == 0) order = length != 0 ? -1 : 0;
                else if (length == 0) order = 1;
                else order = _stricmp(static_cast<const char*>(compare.ptr(4)), static_cast<const char*>(templ.ptr(8)));
                if (order == 0) {
                    if (void* const cls = host.soldier_class_get_or_load_004b1400(replacement); cls != nullptr) *slot = cls;
                }
            }
            host.tree_iterator_increment_00480620(iterator);
        }
    }
    void* const vehicles = raw(raw(host.global_pointer(kGame)).ptr(0x21d0)).field(0x4c);
    for (std::uint32_t i = 0; remap_index_valid(runtime, 0xe4, i); ++i) {
        if (!remap_index_valid(runtime, 0xe4, i)) host.invalid_parameter_noinfo_00bf6713();
        void** const slot = static_cast<void**>(runtime.ptr(0xe4)) + i;
        const Raw vehicle_class = raw(raw(*slot).ptr(0x28));  // 00479BC0
        alignas(4) std::uint8_t iterator_storage[8];
        const Raw iterator{iterator_storage};
        iterator.ptr(0) = vehicles;
        iterator.ptr(4) = raw(raw(vehicles).ptr(4)).ptr(0);
        for (;;) {
            void* const head = raw(vehicles).ptr(4);
            if (iterator.ptr(0) == nullptr || iterator.ptr(0) != vehicles) host.invalid_parameter_noinfo_00bf6713();
            if (iterator.ptr(4) == head) break;
            Raw compare{nullptr};
            Raw replacement{nullptr};
            remap_select_strings(iterator, mode_a, &compare, &replacement, host);
            const char* const compare_text = static_cast<const char*>(compare.ptr(4));
            const char* const name = static_cast<const char*>(vehicle_class.ptr(0x54));
            bool match;
            if (compare_text == nullptr) match = name == nullptr || std::strlen(name) == 0;
            else if (name == nullptr) match = compare.at<std::uint32_t>(0) == 0;
            else match = _stricmp(compare_text, name) == 0;
            if (match) {
                if (void* const found = host.vehicle_class_find_by_name_00965480(replacement); found != nullptr)
                    *slot = host.vehicle_visual_class_get_or_create_0047b3c0(found);
            }
            host.tree_iterator_increment_00480620(iterator);
        }
    }
}

// ============================ segment 10 ===================================

// 004DD7C0 BSP_Game_CompleteLeaderboardRead, __fastcall(ECX = game) -> EAX
// (HRESULT-like), plain RET; only caller 00565540. State game+7058h: 0 idle
// (8000FFFFh); the overlapped at +7038h still 3E5h (pending) and state not 1
// -> 3E5h untouched. XGetOverlappedResult(+7038h, null, TRUE) non-zero with
// state not 1 -> state 0, 80004005h. State 2 also needs
// XGetOverlappedExtendedError(+7038h) >= 0 (else the same failure); a state
// other than 1/2 -> state 0, 3E5h. Then state 0 and, with results
// [+7054h]: a local row vector (frame +30h; allocator dword +0 left as the
// stack held it, +4/+8/+0Ch zeroed). Results +0 == 0 (unsigned) raises the
// "FE_pc.xsm_requireslive" prompt; otherwise view = [results+4]:
// [[[00E198AC]+6Ch]+14Ch] = view+4, 004DCA80(rows, view+8), and each 30h-byte
// source row i (view+0Ch base, count re-read from view+8 every pass) fills
// 004BCD20(rows, i): byte +0 = 1, +4 = src+8, wide +8 assigned from a
// temporary built from the char text at src+18h (temporary released inline
// through the pool: length*2+2), +10h = src+10h, +58h = src+0, +5Ch = src+4,
// +14h = src+28h. Both arms end with 00564F60([[00E198AC]+6Ch], rows) and
// 004D9CA0(rows); EAX = 0 (also when +7054h is null).
std::uint32_t game_complete_leaderboard_read_004dd7c0(Raw game, SweepA3Host& host) {
    std::int32_t& state = game.at<std::int32_t>(0x7058);
    if (state == 0) return 0x8000ffffu;
    void* const overlapped = game.field(0x7038);
    if (game.at<std::int32_t>(0x7038) == 0x3e5 && state != 1) return 0x3e5;
    if (host.x_get_overlapped_result_00a4d42e(overlapped, nullptr, 1) != 0 && state != 1) {
        state = 0;
        return 0x80004005u;
    }
    if (state != 1) {
        if (state != 2) {
            state = 0;
            return 0x3e5;
        }
        if (host.x_get_overlapped_extended_error_00a4d422(overlapped) < 0) {
            state = 0;
            return 0x80004005u;
        }
    }
    state = 0;
    const Raw results = raw(game.ptr(0x7054));
    if (results.p == nullptr) return 0;

    alignas(4) std::uint8_t rows_storage[0x10] = {};
    const Raw rows{rows_storage};
    if (results.at<std::uint32_t>(0) == 0) {
        alignas(4) std::uint8_t empty_storage[8];
        alignas(4) std::uint8_t text_storage[8];
        alignas(4) std::uint8_t by_value_storage[8];
        const Raw empty{empty_storage};
        const Raw text{text_storage};
        const Raw by_value{by_value_storage};
        const char* const blank = static_cast<const char*>(host.global_address(0x00ce3a0c));  // ""
        host.native_string_construct_0041e870(empty, blank);
        host.native_string_construct_0041e870(text, static_cast<const char*>(host.global_address(0x00ce7cf0)));
        host.native_string_construct_0041e870(by_value, blank);
        void* const menu = host.menu_command_screen_get_singleton_00425d10();
        host.menu_prompt_screen_raise_00531b00(menu, 2, text, 2, 0, 0, empty, 0.0f, 0, by_value, 1);
        host.native_string_destroy_storage_0041dd20(text);
        host.native_string_destroy_storage_0041dd20(empty);
    } else {
        const Raw view = raw(results.ptr(4));
        raw(raw(host.global_pointer(kFrontend)).ptr(0x6c)).at<std::uint32_t>(0x14c) = view.at<std::uint32_t>(4);
        host.leaderboard_process_read_results_004dca80(rows, view.at<std::uint32_t>(8));
        for (std::uint32_t i = 0; i < view.at<std::uint32_t>(8); ++i) {
            const Raw source{static_cast<std::uint8_t*>(view.ptr(0xc)) + i * 0x30};
            const Raw row = host.leaderboard_row_at_004bcd20(rows, i);
            row.at<std::uint8_t>(0) = 1;
            row.at<std::uint32_t>(4) = source.at<std::uint32_t>(8);
            alignas(4) std::uint8_t wide_storage[8];
            const Raw wide{wide_storage};
            host.wide_string_construct_from_cstring_004c5e60(wide, reinterpret_cast<const char*>(source.field(0x18)));
            host.wide_string_assign_004c5e20(Raw{row.field(8)}, wide);
            if (void* const data = wide.ptr(4); data != nullptr) {
                host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                              wide.at<std::uint32_t>(0) * 2 + 2, 1);
            }
            row.at<std::uint32_t>(0x10) = source.at<std::uint32_t>(0x10);
            row.at<std::uint32_t>(0x58) = source.at<std::uint32_t>(0);
            row.at<std::uint32_t>(0x5c) = source.at<std::uint32_t>(4);
            row.at<std::uint32_t>(0x14) = source.at<std::uint32_t>(0x28);
        }
    }
    host.leaderboards_on_read_complete_00564f60(raw(host.global_pointer(kFrontend)).ptr(0x6c), rows);
    host.stl_vector_tidy_destroy_004d9ca0(rows);
    return 0;
}

// 004CCE50 BSP_Game_BeginEngineMovie, __thiscall(ECX = game), plain RET;
// sole caller 004E5147 (BSP_Game_OnMove). game+38h = operator new(74h)
// (00BF681B) constructed by 00460B80 (null when the allocation failed). The
// path: a copy of the string at [game+5FCh]+90Ch, cut before its last "."
// (00467CF0 with position 7FFFFFFFh, then 00469840(result, 0, index) and an
// assignment back), with ".ema" (00CE7838) appended; 004602C0([game+38h],
// path). Movie state +60h == 5: 00B72030([game+19ECh]), slot 0 of every node
// on the list [00E188B4] (next +8), then the movie is destroyed (0045D930,
// sweepA2's reconstruction) and freed (00BF65AC) and game+38h = 0. Otherwise
// 0045DB40(movie) and 004CC460([00E198C4], 2Dh, 0). All temporaries are
// released inline through the sized pool (length + 1). SEH states 0..4 only
// guard the temporaries.
void game_begin_engine_movie_004cce50(Raw game, SweepA3Host& host) {
    void* const block = host.operator_new_00bf681b(0x74);
    game.ptr(0x38) = block != nullptr ? host.engine_movie_construct_00460b80(block) : nullptr;

    alignas(4) std::uint8_t path_storage[8] = {};
    const Raw path{path_storage};
    assign_string(path, Raw{static_cast<std::uint8_t*>(game.ptr(0x5fc)) + 0x90c}, host);

    alignas(4) std::uint8_t separator_storage[8];
    const Raw separator{separator_storage};
    construct_literal(separator, static_cast<const char*>(host.global_address(0x00ce3a70)), 1, host);  // "."
    const std::uint32_t cut = host.native_string_rfind_00467cf0(path, separator, 0x7fffffff);
    alignas(4) std::uint8_t stem_storage[8];
    const Raw stem{stem_storage};
    assign_string(path, host.native_string_substring_00469840(path, stem, 0, cut), host);
    release_string(stem, host);
    release_string(separator, host);

    alignas(4) std::uint8_t suffix_storage[8];
    const Raw suffix{suffix_storage};
    construct_literal(suffix, static_cast<const char*>(host.global_address(0x00ce7838)), 4, host);  // ".ema"
    append_string(path, suffix, host);
    release_string(suffix, host);

    host.engine_movie_parse_file_004602c0(game.ptr(0x38), path);
    void* const movie = game.ptr(0x38);
    if (raw(movie).at<std::int32_t>(0x60) == 5) {
        host.scene_build_quad_trees_00b72030(game.ptr(0x19ec));
        for (void* node = host.global_pointer(0x00e188b4); node != nullptr; node = raw(node).ptr(8))
            host.vcall_slot0_void(node);
        if (void* const owned = game.ptr(0x38); owned != nullptr) {
            cc13_sweepA2::cutscene_player_destruct_0045d930(raw(owned), host);
            host.free_00bf65ac(owned);
            game.ptr(0x38) = nullptr;
        }
    } else {
        host.engine_movie_start_0045db40(movie);
        host.frontend_push_interface_request_004cc460(host.global_pointer(kHudManager), 0x2d, 0);
    }
    release_string(path, host);
}

namespace {

// [slot] non-null -> [[slot]]+0(1) and slot = 0 (the scalar deleting
// destructor of each owned world object).
void delete_slot0(void*& slot, SweepA3Host& host) {
    if (slot != nullptr) {
        host.vcall_destroy_0(slot, 1);
        slot = nullptr;
    }
}

// The point-effect release of 004D2BB0: a non-null effect has its children
// stopped (00867B10) and byte +9 set; the slot is re-read and, when set,
// InterlockedDecrement(+4) reaching 0 calls its slot 0; the slot is cleared
// twice, as in the image.
void release_point_effect(Raw game, std::size_t offset, SweepA3Host& host) {
    void* const effect = game.ptr(offset);
    if (effect == nullptr) return;  // NEG/SBB/TEST 00E186EC: non-null
    host.point_effect_stop_children_00867b10(effect);
    raw(effect).at<std::uint8_t>(9) = 1;
    if (void* const again = game.ptr(offset); again != nullptr) {
        if (host.interlocked_decrement_ce2220(static_cast<volatile long*>(static_cast<void*>(raw(again).field(4)))) == 0)
            host.vcall_release_0(again);
        game.ptr(offset) = nullptr;
    }
    game.ptr(offset) = nullptr;
}

}  // namespace

// 004D2BB0 BSP_Game_DestroyWorld, __thiscall(ECX = game), plain RET or a
// tail jump to 0060D3E0; sole caller 004DA780. 00A933F0(004BEC00(), 1Dh, 0);
// with [00E18680], 0046B680 on it; nothing more when game+19CCh is null.
// Otherwise: 0060D6E0([00E19698], 1); +19E8h slot +4(1) and cleared;
// 008658E0(00865CF0()); 004BFDF0(game); +193Ch = 0; with +1ED4h:
// 004254B0(00CE78C4, 0), 00926D90(+1ED4h, 2) when still set, +1ED4h = 0 twice,
// byte [00E19894] = 0 when [00E198C4] is set; slot-0 deletes (flag 1) of
// +21D0h, +21D4h, +21A4h..+21ACh (three), +21C4h, +21C8h, +21CCh, +21DCh,
// +21E0h, +21E4h, +21E8h; [00F89B3C] destructed (009496A0), freed (00BF65AC)
// and cleared; [00F88C30] slot-0 deleted and cleared; the vector [00F87560,
// 00F87564) is emptied by the inlined erase(begin, end) (first > last
// reported twice, memmove_s only for a positive tail, which is always empty);
// +19CCh slot-0 deleted; 00B0D190([00F8D39C]); the point effects +19D8h and
// +19E4h released; with [[00E188A8]+5FCh], 004CB160([..]+D50h, 0) (sweepA's
// reconstruction); 008674A0(004D1100()); +19F0h slot +4(1); 00A7B9C0 and
// slot +4(identity matrix, zero position) on [00F8BBD8]; +19F4h released by
// InterlockedDecrement (slot 0 at zero) and cleared when set; +19F8h
// (00B6DFA0) and +19ECh (00B72250) torn down and cleared when set;
// 00AD7B90([00F8C210], 0); 004C7DD0(game); 004D2B10(game); for the eight
// slots +21A4h.. a slot-0 delete and 007F8180([game+18CCh + i*4]); finally
// 0060D3E0([00E19698]).
void game_destroy_world_004d2bb0(Raw game, SweepA3Host& host) {
    host.input_manager_set_context_level_00a933f0(host.input_manager_get_singleton_004bec00(), 0x1d, 0);
    if (void* const database = host.global_pointer(kSceneDatabase); database != nullptr)
        host.scene_database_clear_hidden_records_0046b680(database);
    if (game.ptr(0x19cc) == nullptr) return;
    host.please_wait_show_with_worker_0060d6e0(host.global_pointer(0x00e19698), 1);
    if (void* const owner = game.ptr(0x19e8); owner != nullptr) {
        host.vcall_destroy_4(owner, 1);
        game.ptr(0x19e8) = nullptr;
    }
    host.world_atmosphere_release_node_008658e0(host.rain_descriptor_get_singleton_00865cf0());
    host.game_clear_local_player_unit_lists_004bfdf0(game.p);
    game.at<std::uint8_t>(0x193c) = 0;
    if (game.ptr(0x1ed4) != nullptr) {
        host.debug_print1_004254b0(literal(0x00ce78c4, host), 0);
        if (void* const viewer = game.ptr(0x1ed4); viewer != nullptr) {
            host.mission_entity_kill_00926d90(viewer, 2);
            game.ptr(0x1ed4) = nullptr;
        }
        game.ptr(0x1ed4) = nullptr;
        if (host.global_pointer(kHudManager) != nullptr)
            *static_cast<std::uint8_t*>(host.global_address(0x00e19894)) = 0;
    }
    delete_slot0(game.ptr(0x21d0), host);
    delete_slot0(game.ptr(0x21d4), host);
    for (std::size_t i = 0; i < 3; ++i) delete_slot0(game.ptr(0x21a4 + 4 * i), host);
    for (const std::size_t offset : {0x21c4u, 0x21c8u, 0x21ccu, 0x21dcu, 0x21e0u, 0x21e4u, 0x21e8u})
        delete_slot0(game.ptr(offset), host);
    void*& singleton = *static_cast<void**>(host.global_address(0x00f89b3c));
    if (void* const object = singleton; object != nullptr) {
        host.destruct_009496a0(object);
        host.free_00bf65ac(object);
        singleton = nullptr;
    }
    delete_slot0(*static_cast<void**>(host.global_address(0x00f88c30)), host);

    auto& first = *static_cast<std::uintptr_t*>(host.global_address(0x00f87560));
    auto& last = *static_cast<std::uintptr_t*>(host.global_address(0x00f87564));
    const std::uintptr_t erase_last = last;
    if (first > last) {
        host.invalid_parameter_noinfo_00bf6713();
        if (first > last) host.invalid_parameter_noinfo_00bf6713();
    }
    if (first != erase_last) {
        const std::int32_t tail = static_cast<std::int32_t>(last - erase_last) >> 2;
        const std::size_t bytes = static_cast<std::size_t>(tail) * 4;
        const std::uintptr_t new_last = first + bytes;
        if (tail > 0)
            memmove_s(reinterpret_cast<void*>(first), bytes, reinterpret_cast<const void*>(erase_last), bytes);
        last = new_last;
    }

    delete_slot0(game.ptr(0x19cc), host);
    host.render_service_flush_listener_00b0d190(host.global_pointer(0x00f8d39c));
    release_point_effect(game, 0x19d8, host);
    release_point_effect(game, 0x19e4, host);
    if (void* const scene_contents = raw(host.global_pointer(kGame)).ptr(0x5fc); scene_contents != nullptr)
        cc13_sweepA::effect_handle_vector_resize_004cb160(Raw{static_cast<std::uint8_t*>(scene_contents) + 0xd50}, 0,
                                                          host);
    host.effect_manager_clear_008674a0(host.effect_manager_get_singleton_004d1100());
    if (void* const object = game.ptr(0x19f0); object != nullptr) {
        host.vcall_destroy_4(object, 1);
        game.ptr(0x19f0) = nullptr;
    }
    host.sound_manager_reset_00a7b9c0(host.global_pointer(kSoundManager));
    const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    const float origin[3] = {0, 0, 0};
    host.vcall_set_listener_4(host.global_pointer(kSoundManager), identity, origin);
    if (void* const object = game.ptr(0x19f4); object != nullptr) {
        if (host.interlocked_decrement_ce2220(static_cast<volatile long*>(static_cast<void*>(raw(object).field(4)))) == 0)
            host.vcall_release_0(object);
        game.ptr(0x19f4) = nullptr;
    }
    if (void* const object = game.ptr(0x19f8); object != nullptr) {
        host.teardown_00b6dfa0(object);
        game.ptr(0x19f8) = nullptr;
    }
    if (void* const scene = game.ptr(0x19ec); scene != nullptr) {
        host.scene_teardown_00b72250(scene);
        game.ptr(0x19ec) = nullptr;
    }
    host.gui_root_reset_00ad7b90(host.global_pointer(0x00f8c210), 0);
    host.game_payload_lists_clear_004c7dd0(game.p);
    host.game_destroy_world_containers_004d2b10(game.p);
    for (std::size_t i = 0; i < 8; ++i) {
        delete_slot0(game.ptr(0x21a4 + 4 * i), host);
        host.game_participant_release_references_007f8180(game.ptr(0x18cc + 4 * i));
    }
    host.please_wait_hide_0060d3e0(host.global_pointer(0x00e19698));
}

// 004D32A0 BSP_Game_NilScriptAddedGlobals, __fastcall(ECX = game), plain RET
// (EBP frame); sole caller 004DA780. Nothing when the set size game+1938h is
// 0. Otherwise: a local name vector {data, count, capacity} zeroed, key and
// value LuaObjects nil-constructed (00B65F50), the globals of game+1A0Ch
// (00B67980) walked with 00B67080/00B67190 until 00B66420(globals, key)
// holds. Per entry the key text (00B662B0; strlen without a null check, as in
// the image) becomes a pooled string (0041DD40(length, 1), memcpy length + 1);
// when the value is a function (00B66200) the set is searched (004C7F10 with
// the head [game+1934h] read before the call), a result whose container is
// null or not the set is reported (00BF6713), and a miss (node == head) is
// appended (004CDC20); the name is released. Then, last element first, each
// name + " = nil" (00CE78F4, built with 0041DD40(6, 1), joined by 004261A0)
// runs through 00B66C60(game+1A0Ch, &statement); the statement, the suffix
// and the element are released and the count drops. Finally 004CEC60 erases
// the tree from [head+4], the head is relinked to itself (+4, then size 0,
// +0, +8), the globals, value and key objects are destroyed in that order,
// 00427110(vector, 0) and _free (00BF6989) of the vector data.
void game_nil_script_added_globals_004d32a0(Raw game, SweepA3Host& host) {
    if (game.at<std::uint32_t>(0x1938) == 0) return;
    alignas(4) std::uint8_t names_storage[0xc] = {};
    const Raw names{names_storage};
    LuaObjectStorage key;
    LuaObjectStorage value;
    LuaObjectStorage globals;
    host.lua_object_construct_00b65f50(&key);
    host.lua_object_construct_00b65f50(&value);
    host.lua_state_owner_get_globals_00b67980(game.field(0x1a0c), &globals);
    host.lua_object_iterate_first_00b67080(&globals, &key, &value);
    while (!host.lua_object_is_unbound_00b66420(&globals, &key)) {
        const char* const text = host.lua_object_get_string_00b662b0(&key);
        alignas(4) std::uint8_t name_storage[8] = {};
        const Raw name{name_storage};
        host.native_string_resize_0041dd40(name, static_cast<std::uint32_t>(std::strlen(text)), 1);
        void* const data = name.ptr(4);
        if (data != nullptr) std::memcpy(data, text, name.at<std::uint32_t>(0) + 1);
        if (host.lua_object_is_function_00b66200(&value)) {
            void* const head = game.ptr(0x1934);
            const Raw set{game.field(0x1930)};
            alignas(4) std::uint8_t found_storage[8];
            const Raw found = host.native_string_set_find_004c7f10(set, Raw{found_storage}, name);
            if (found.ptr(0) == nullptr || found.ptr(0) != set.p) host.invalid_parameter_noinfo_00bf6713();
            if (found.ptr(4) == head) host.native_string_vector_append_004cdc20(names.p, name);
        }
        if (data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                          name.at<std::uint32_t>(0) + 1, 1);
        }
        host.lua_object_iterate_next_00b67190(&globals, &key, &value);
    }
    while (names.at<std::uint32_t>(4) != 0) {
        alignas(4) std::uint8_t suffix_storage[8];
        const Raw suffix{suffix_storage};
        construct_literal(suffix, literal(0x00ce78f4, host), 6, host);  // " = nil"
        const Raw last{static_cast<std::uint8_t*>(names.ptr(0)) + names.at<std::uint32_t>(4) * 8 - 8};
        alignas(4) std::uint8_t statement_storage[8];
        const Raw statement{statement_storage};
        host.lua_state_owner_execute_string_00b66c60(game.field(0x1a0c),
                                                     host.native_string_concat_004261a0(last, statement, suffix));
        release_string(statement, host);
        release_string(suffix, host);
        const std::uint32_t count = names.at<std::uint32_t>(4);
        if (count == 0) break;
        release_string(Raw{static_cast<std::uint8_t*>(names.ptr(0)) + count * 8 - 8}, host);
        names.at<std::uint32_t>(4) = count - 1;
    }
    const Raw set{game.field(0x1930)};
    host.native_string_set_erase_subtree_004cec60(set, raw(set.ptr(4)).ptr(4));
    raw(set.ptr(4)).ptr(4) = set.ptr(4);
    set.at<std::uint32_t>(8) = 0;
    raw(set.ptr(4)).ptr(0) = set.ptr(4);
    raw(set.ptr(4)).ptr(8) = set.ptr(4);
    host.lua_object_destruct_00b67700(&globals);
    host.lua_object_destruct_00b67700(&value);
    host.lua_object_destruct_00b67700(&key);
    host.native_string_vector_resize_00427110(names, 0);
    host.free_00bf6989(names.ptr(0));
}

// ============================ segment 12 ===================================

namespace {

// A checked index into a dword vector {+4 first, +8 last}: first set and the
// index below the (SAR, signed) count compared unsigned.
bool dword_index_valid(Raw vector, std::uint32_t index) {
    const auto first = vector.at<std::uintptr_t>(4);
    if (first == 0) return false;
    return index < static_cast<std::uint32_t>(static_cast<std::int32_t>(vector.at<std::uintptr_t>(8) - first) >> 2);
}

}  // namespace

// 0050B440, __thiscall(ECX = tactical library screen), plain RET (EBP
// frame); sole caller 0050D900 (BSP_TacticalLibraryScreen_ClearList, view
// 6). Erases the shown entry vector +444h (begin > end reported twice, then
// 00505DE0 over [begin, end)). When the list widget [+B8h] has a selected
// item (00A9CA00, called again for the item) whose +28h text (data +4, ""
// 00E18D78 when null) reads 99999 by atol, it returns. Otherwise, with a local
// dword vector "seen" {+4, +8, +0Ch} zeroed and ONE found byte cleared once
// (never reset, as in the image: after the first duplicate every later id is
// skipped): for every 2Ch-byte record i of +470h/+474h (signed /2Ch) and every
// id j of its dword vector at record+18h (00415450 sizes re-read each pass),
// a match against any seen id sets the byte; while it is clear the id's entry
// (004FB090, or [+5B0h] when that is null and the id is -1) is pushed onto
// +444h (0050AD00) and the id onto seen (inline when capacity remains, else
// 00441F30 at the end, last > first reported). Every checked index of the
// image reports through 00BF6713 at the same point. seen's data is freed
// with 00BF65AC when set.
void tactical_library_rebuild_entry_list_0050b440(Raw screen, SweepA3Host& host) {
    const Raw shown{screen.field(0x444)};
    void* const end = shown.ptr(8);
    if (shown.at<std::uintptr_t>(4) > reinterpret_cast<std::uintptr_t>(end)) host.invalid_parameter_noinfo_00bf6713();
    void* const begin = shown.ptr(4);
    if (reinterpret_cast<std::uintptr_t>(begin) > shown.at<std::uintptr_t>(8)) host.invalid_parameter_noinfo_00bf6713();
    void* erased = nullptr;
    host.entry_vector_erase_00505de0(shown, &erased, shown.p, begin, shown.p, end);
    if (host.list_widget_selected_item_00a9ca00(screen.ptr(0xb8)) != nullptr) {
        const Raw text = host.vcall_item_text_28(host.list_widget_selected_item_00a9ca00(screen.ptr(0xb8)));
        const char* chars = static_cast<const char*>(text.ptr(4));
        if (chars == nullptr) chars = literal(0x00e18d78, host);
        if (std::atol(chars) == 99999) return;
    }
    bool found = false;
    alignas(4) std::uint8_t seen_storage[0x10] = {};
    const Raw seen{seen_storage};
    for (std::uint32_t i = 0;; ++i) {
        const auto records = screen.at<std::uintptr_t>(0x470);
        if (records == 0 ||
            i >= static_cast<std::uint32_t>(static_cast<std::int32_t>(screen.at<std::uintptr_t>(0x474) - records) / 0x2c))
            break;
        if (records == 0 ||
            i >= static_cast<std::uint32_t>(static_cast<std::int32_t>(screen.at<std::uintptr_t>(0x474) - records) / 0x2c))
            host.invalid_parameter_noinfo_00bf6713();
        const Raw ids{static_cast<std::uint8_t*>(screen.ptr(0x470)) + i * 0x2c + 0x18};
        for (std::uint32_t j = 0; j < host.vector_size_00415450(ids); ++j) {
            for (std::uint32_t k = 0; k < host.vector_size_00415450(seen); ++k) {
                if (!dword_index_valid(seen, k)) host.invalid_parameter_noinfo_00bf6713();
                if (!dword_index_valid(ids, j)) host.invalid_parameter_noinfo_00bf6713();
                if (static_cast<std::uint32_t*>(seen.ptr(4))[k] == static_cast<std::uint32_t*>(ids.ptr(4))[j]) found = true;
            }
            if (found) continue;
            if (!dword_index_valid(ids, j)) host.invalid_parameter_noinfo_00bf6713();
            const std::int32_t id = static_cast<std::int32_t*>(ids.ptr(4))[j];
            void* entry = host.tactical_library_find_entry_by_id_004fb090(screen, id);
            if (entry == nullptr) {
                if (!dword_index_valid(ids, j)) host.invalid_parameter_noinfo_00bf6713();
                if (static_cast<std::int32_t*>(ids.ptr(4))[j] == -1) entry = screen.ptr(0x5b0);
            }
            host.entry_vector_push_back_0050ad00(shown, entry);
            if (!dword_index_valid(ids, j)) host.invalid_parameter_noinfo_00bf6713();
            const std::uint32_t* const value = static_cast<std::uint32_t*>(ids.ptr(4)) + j;
            const auto first = seen.at<std::uintptr_t>(4);
            const auto last = seen.at<std::uintptr_t>(8);
            if (first != 0 && static_cast<std::uint32_t>(static_cast<std::int32_t>(last - first) >> 2) <
                                  static_cast<std::uint32_t>(static_cast<std::int32_t>(seen.at<std::uintptr_t>(0xc) - first) >> 2)) {
                *reinterpret_cast<std::uint32_t*>(last) = *value;
                seen.at<std::uintptr_t>(8) = last + 4;
            } else {
                if (first > last) host.invalid_parameter_noinfo_00bf6713();
                alignas(4) std::uint8_t result[8];
                host.uint_vector_insert_one_00441f30(seen, result, seen.p, reinterpret_cast<void*>(last), value);
            }
        }
    }
    if (void* const data = seen.ptr(4); data != nullptr) host.free_00bf65ac(data);
}

// ============================ segment 14 ===================================

namespace {

// One 005220C0 sight ray: the viewer [[00E188A8]+1ED4h] (re-read per ray),
// its pose refreshed (00414DB0) when its byte +C8h is clear; 004B4B00 gives
// the ignored vehicle; 009043A0([game+19CCh], viewer+FCh, origin, hit,
// vehicle, 44h). Returns true when the ray "passes": no hit, the unit itself
// hit, or the hit point within 8 (64.0f at 00CE7820 > squared distance; the
// image sums (dy*dy + dx*dx) + dz*dz of float differences in x87 and rounds
// it to float).
bool sight_ray_passes(void* unit, const float* origin, Raw hit, SweepA3Host& host) {
    void* const viewer = raw(host.global_pointer(kGame)).ptr(0x1ed4);
    if (raw(viewer).at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(viewer);
    void* const vehicle = host.game_get_controlled_vehicle_004b4b00();
    if (!host.world_ray_test_009043a0(raw(host.global_pointer(kGame)).ptr(0x19cc),
                                      reinterpret_cast<const float*>(raw(viewer).field(0xfc)), origin, hit, vehicle,
                                      0x44))
        return true;
    if (hit.ptr(0) == unit) return true;
    const float dx = static_cast<float>(static_cast<double>(hit.at<float>(8)) - origin[0]);
    const float dy = static_cast<float>(static_cast<double>(hit.at<float>(0xc)) - origin[1]);
    const float dz = static_cast<float>(static_cast<double>(hit.at<float>(0x10)) - origin[2]);
    const float distance_sq = static_cast<float>((static_cast<double>(dy) * dy + static_cast<double>(dx) * dx) +
                                                 static_cast<double>(dz) * dz);
    return 64.0f > distance_sq;
}

// The unit's position +FCh..+104h after the conditional pose refresh, with
// `lift` added to y (x + 0.0 and 0.0 + z as the image adds them).
void sight_origin(void* unit, float lift, float* origin, SweepA3Host& host) {
    if (raw(unit).at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    origin[0] = raw(unit).at<float>(0xfc) + 0.0f;
    origin[1] = static_cast<float>(static_cast<double>(raw(unit).at<float>(0x100)) + lift);
    origin[2] = 0.0f + raw(unit).at<float>(0x104);
}

}  // namespace

// 005220C0, stdcall-shaped: [ESP+4] = unit, RET 4, AL (ECX is not read on
// entry; sweepA2 declared the hook as __thiscall(screen, unit), which passes
// the same stack argument). Callers 00522F10, 00523130, 00526A40, 00642040,
// 006434E0, 006435D0. 0 when [[00E188A8]+1ED4h] (the viewer) is null.
// Otherwise a local hit record (54h bytes, 00470470 / 004704B0 around the
// body) and a sight ray from the viewer to the unit's position: passing
// -> 1. Then four rays with the origin lifted by h = 4.0f (00CE3D34), h +=
// 4.0 (double 00D7A328) while 20.0f (00CE3930) > h; every ray is cast but
// only the last one's result is kept (the image overwrites BL each pass):
// passing -> 1. Otherwise a unit that is not kind 6 (+5Ch(6)) -> 0; a kind-6
// unit casts once more lifted by [[unit+538h]+A8h] * 0.5 (double 00D7A280)
// and returns whether that ray passes. Shared RNG: none.
bool unit_line_of_sight_from_viewer_005220c0(void* unit, SweepA3Host& host) {
    if (raw(host.global_pointer(kGame)).ptr(0x1ed4) == nullptr) return false;
    alignas(4) std::uint8_t hit_storage[0x54];
    const Raw hit{hit_storage};
    host.hit_record_reset_00470470(hit);
    bool result;
    float origin[3];
    if (raw(unit).at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    origin[0] = raw(unit).at<float>(0xfc);
    origin[1] = raw(unit).at<float>(0x100);
    origin[2] = raw(unit).at<float>(0x104);
    if (sight_ray_passes(unit, origin, hit, host)) {
        result = true;
    } else {
        bool passes = false;
        float lift = 4.0f;
        do {
            sight_origin(unit, lift, origin, host);
            passes = sight_ray_passes(unit, origin, hit, host);
            lift = static_cast<float>(lift + 4.0);
        } while (20.0f > lift);
        if (passes) {
            result = true;
        } else if (!host.vcall_entity_is_kind_5c(unit, 6)) {
            result = false;
        } else {
            const float lift_half = static_cast<float>(raw(raw(unit).ptr(0x538)).at<float>(0xa8) * 0.5);
            sight_origin(unit, lift_half, origin, host);
            result = sight_ray_passes(unit, origin, hit, host);
        }
    }
    host.hit_record_destroy_004704b0(hit);
    return result;
}

// 0051EAA0 BSP_HudBinoculars_StepInputAxes, __fastcall(ECX = float* value,
// EDX = float* limit, float divisor, float dt, byte* active, byte* rising,
// byte* falling, byte keep_limit), RET 18h; callers 0051EF00 and 00651800.
// With b = [004BEC00()+4] (004BEC00 called afresh before each read): analog
// = 00A92090(b+15C0h) || 00A92090(b+2A00h); axis = b+15E4h; toggle =
// 004C43C0([00E188A8], E0h) || (*rising && *falling); diff = (float)(axis -
// b+2A24h). axis == 0 (ordered): with toggle, *limit > 0.25f lowers *limit by
// 0.75 (double 00CEC9D8) unless keep_limit and sets falling/active, then steps;
// otherwise *limit = 1.0f, rising = 1, falling = active = 0, *value = *limit
// and the HUD binoculars [[00E198C4]+84h] (when set) gets byte +1Ch = 1 and
// [[+20h]]+34h(0). axis != 0 (or NaN): analog -> *value = r + (1 - r) *
// (diff + 1) * 0.5 with r = (float)(1/divisor); else StepTowards(value, 1.0f,
// diff*0.5*dt) for diff > 0, StepTowards(value, (float)(1/divisor),
// diff*-0.5*dt) otherwise. Then, with *active: rising adds dt*5.0, falling
// subtracts it, clamping at *limit (which clears active). x87 forms from the
// bytes: DE F1 at 0051EBAF and DE F2 at 0051EBD0 are FDIVRP (1/divisor), DE
// EA at 0051EBDE is ST2 = ST2 - ST0 (1 - r), DE E9 at 0051ECFD is ST1 = ST1 -
// ST0 (value - dt*5). Doubles 00D7A250 = -1.0, 00D7A280 = 0.5, 00CEC9E0 =
// -0.5, 00D7A370 = 5.0; floats 00CE3868 = 0.25, 00D7A24C = 1.0. Intermediates
// are double here, extended in the image.
void hud_binoculars_step_input_axes_0051eaa0(float* value, float* limit, float divisor, float dt,
                                             std::uint8_t* active, std::uint8_t* rising, std::uint8_t* falling,
                                             std::uint8_t keep_limit, SweepA3Host& host) {
    const bool analog = host.input_action_has_device_query_00a92090(input_bank(host).field(0x15c0)) ||
                        host.input_action_has_device_query_00a92090(input_bank(host).field(0x2a00));
    const float axis = input_bank(host).at<float>(0x15e4);
    const bool toggle = host.input_action_was_pressed_004c43c0(host.global_pointer(kGame), 0xe0) ||
                        (*rising != 0 && *falling != 0);
    const float diff = static_cast<float>(static_cast<double>(axis) - input_bank(host).at<float>(0x2a24));
    bool step = false;
    if (axis == 0.0f) {
        if (toggle) {
            if (*limit > 0.25f) {
                if (keep_limit == 0) *limit = static_cast<float>(*limit - 0.75);
                *falling = 1;
                *rising = 0;
                *active = 1;
                step = true;
            } else {
                *limit = 1.0f;
                *rising = 1;
                *falling = 0;
                *active = 0;
                *value = *limit;
                if (void* const binoculars = raw(host.global_pointer(kHudManager)).ptr(0x84); binoculars != nullptr) {
                    raw(binoculars).at<std::uint8_t>(0x1c) = 1;
                    host.vcall_set_visible_34(raw(raw(host.global_pointer(kHudManager)).ptr(0x84)).ptr(0x20), 0);
                }
            }
        }
    } else if (analog) {
        const float r = static_cast<float>(1.0 / divisor);
        *value = static_cast<float>(r + (1.0 - r) * (diff - -1.0) * 0.5);
    } else if (diff > 0.0f) {
        host.math_step_towards_0042ac60(value, 1.0f, static_cast<float>(diff * 0.5 * dt));
    } else {
        const float rate = static_cast<float>(diff * -0.5 * dt);
        host.math_step_towards_0042ac60(value, static_cast<float>(1.0 / divisor), rate);
    }
    if (!step && *active == 0) return;
    if (*rising != 0) {
        const float next = static_cast<float>(dt * 5.0 + *value);
        *value = next;
        if (next > *limit) {
            *value = *limit;
            *active = 0;
        }
    } else if (*falling != 0) {
        const float next = static_cast<float>(*value - dt * 5.0);
        *value = next;
        if (*limit > next) {
            *value = *limit;
            *active = 0;
        }
    }
}

namespace {

// The cycle list screen+C4h {+4 begin = screen+C8h, +8 end = screen+CCh} of
// 18h-byte records (unit at +14h): (end - begin) / 18h signed, 0 when begin
// is null (the image recomputes it at every use).
std::int32_t cycle_count(Raw screen) {
    const auto begin = screen.at<std::uintptr_t>(0xc8);
    if (begin == 0) return 0;
    return static_cast<std::int32_t>(screen.at<std::uintptr_t>(0xcc) - begin) / 0x18;
}

void* cycle_unit(Raw screen, std::uint32_t index) {
    return Raw{static_cast<std::uint8_t*>(screen.ptr(0xc8)) + index * 0x18}.ptr(0x14);
}

}  // namespace

// 00526720, __thiscall(ECX = unit pick screen), plain RET; sole caller
// 00527260 (BSP_HudUnitPickScreen_Update). With 0071C060(2, player =
// [00E188D8]): when screen+C0h is the player and 00927880(player) is not 0,
// 00526180 refreshes the list; otherwise 00525170(screen+C0h), +C0h = player
// and 00525F80 (sweepA2) rebuilds it. Then up to count passes: +D4h = (+D4h +
// 1) % count (unsigned DIV, count recomputed), a checked-iterator report
// (00BF6713) when begin is null or the index is not below count, unit =
// record +14h; a unit is skipped when null. The command is 00E08F78 when
// game+19C4h is set, player +5Ch(6) holds and player +16Ch("attackmove",
// unit) holds, else 00E08EF8; when player +16Ch(command +4 name, unit) holds,
// a 18h-byte descriptor {byte 1, byte 0, word unit+174h, unit, float3
// [00F87574..7Ch], 0.0f} goes to 0077D600(player, command, &descriptor,
// player +5Ch(18h) whole EAX), +D8h = 1 and the loop ends. When all passes
// were used (also count 0): +C0h = 0, the two iterator checks (begin > end
// reports), 00524DC0 erases [begin, end) and +D4h = -1. Always: the name is
// "N/A" (00CECE08) for an empty list, else the selected unit's +10h (index
// checked again; no null guard, as in the image); 004254B0("target cycle:
// %d/%d -> %s", +D4h, count, name). The player global is re-read where the
// image re-reads it.
// The modulo divides by a count the loop condition keeps above 0, as the
// image's DIV does; C4724 (potential mod by 0) is silenced, no guard added.
#pragma warning(push)
#pragma warning(disable : 4724)
void unit_pick_cycle_target_00526720(Raw screen, SweepA3Host& host) {
    if (host.unit_director_flags_allow_0071c060(2, host.global_pointer(kPlayer))) {
        void* const player = host.global_pointer(kPlayer);
        if (screen.ptr(0xc0) == player && host.controlled_target_00927880(player) != nullptr) {
            host.unit_pick_refresh_cycle_list_00526180(screen);
        } else {
            host.stl_inst_00525170(Raw{screen.field(0xc0)});
            screen.ptr(0xc0) = host.global_pointer(kPlayer);
            cc13_sweepA2::unit_pick_collect_candidates_00525f80(screen, host);
        }
        const std::int32_t count = cycle_count(screen);
        std::int32_t pass = 0;
        for (; pass < count; ++pass) {
            const std::uint32_t index =
                (screen.at<std::uint32_t>(0xd4) + 1) % static_cast<std::uint32_t>(cycle_count(screen));
            screen.at<std::uint32_t>(0xd4) = index;
            if (screen.ptr(0xc8) == nullptr || index >= static_cast<std::uint32_t>(cycle_count(screen)))
                host.invalid_parameter_noinfo_00bf6713();
            void* const unit = cycle_unit(screen, index);
            if (unit == nullptr) continue;
            void* const holder = host.global_pointer(kPlayer);
            void* command = host.global_address(0x00e08ef8);
            if (raw(host.global_pointer(kGame)).at<std::uint8_t>(0x19c4) != 0 &&
                host.vcall_entity_is_kind_5c(holder, 6) &&
                host.vcall_entity_accepts_command_16c(holder, literal(0x00cecca8, host),
                                                      static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(unit))))
                command = host.global_address(0x00e08f78);
            const char* const name = host.vcall_command_name_4(command);
            if (!host.vcall_entity_accepts_command_16c(host.global_pointer(kPlayer), name,
                                                       static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(unit))))
                continue;
            alignas(4) std::uint8_t descriptor[0x18];
            const Raw d{descriptor};
            d.at<std::uint8_t>(0) = 1;
            d.at<std::uint8_t>(1) = 0;
            d.at<std::uint16_t>(2) = raw(unit).at<std::uint16_t>(0x174);
            d.ptr(4) = unit;
            std::memcpy(descriptor + 8, host.global_address(0x00f87574), 12);
            d.at<float>(0x14) = 0.0f;
            const std::uint32_t flags = host.vcall_entity_is_kind_5c_eax(host.global_pointer(kPlayer), 0x18);
            host.entity_issue_command_0077d600(host.global_pointer(kPlayer), command, descriptor, flags);
            screen.at<std::uint8_t>(0xd8) = 1;
            break;
        }
        if (pass == count) {
            screen.ptr(0xc0) = nullptr;
            const Raw list{screen.field(0xc4)};
            void* const end = list.ptr(8);
            if (list.at<std::uintptr_t>(4) > reinterpret_cast<std::uintptr_t>(end)) host.invalid_parameter_noinfo_00bf6713();
            void* const begin = list.ptr(4);
            if (reinterpret_cast<std::uintptr_t>(begin) > list.at<std::uintptr_t>(8)) host.invalid_parameter_noinfo_00bf6713();
            alignas(4) std::uint8_t result[8];
            host.vector_erase_range_00524dc0(list, result, list.p, begin, list.p, end);
            screen.at<std::int32_t>(0xd4) = -1;
        }
    }
    const char* name = literal(0x00cece08, host);  // "N/A"
    if (screen.ptr(0xc8) != nullptr && cycle_count(screen) != 0) {
        const std::uint32_t index = screen.at<std::uint32_t>(0xd4);
        if (screen.ptr(0xc8) == nullptr || index >= static_cast<std::uint32_t>(cycle_count(screen)))
            host.invalid_parameter_noinfo_00bf6713();
        name = host.vcall_unit_name_10(cycle_unit(screen, index));
    }
    host.debug_print_004254b0(literal(0x00cecdec, host), screen.at<std::uint32_t>(0xd4),
                              static_cast<std::uint32_t>(cycle_count(screen)),
                              static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(name)));
}
#pragma warning(pop)

}  // namespace bsp::cc13_sweepA3
