// Segments 3 and 4 reconstructions, packet cc13-seg3 (docs/CC13_SEG3_4.md).
#include "bsp/seg3_4_cc13.hpp"

#include <cstddef>
#include <cstring>

namespace bsp::seg3_4_cc13 {
namespace {

template <class T>
T read_field(const void* object, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof(value));
    return value;
}

template <class T>
void write_field(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof(value));
}

std::byte* at(void* object, std::size_t offset) noexcept {
    return static_cast<std::byte*>(object) + offset;
}

TaggedValue key(PropertyStreamHost& host, std::string_view name) {
    return {kTagString, host.literal(name)};
}

TaggedValue field_ref(PropertyStreamHost& host, std::uint32_t kind, void* object, std::size_t offset) {
    return {kind, host.address_of(at(object, offset))};
}

// "panel" read with default 1 into +4h, then +4h = 0 (0044A3F0, inlined in
// 0044A440 / 0044A4C0).
void read_panel(void* command, void* reader, DialogCommandHost& host) {
    host.read_with_default_0c(reader, key(host, kKeyPanel), field_ref(host, kTagInt, command, 0x4), {kTagInt, 1});
    write_field<std::uint32_t>(command, 0x4, 0);
}

// A NativeString's text for a write: +4h of the string, or 00E17BFB when null.
TaggedValue string_text(DialogCommandHost& host, const void* string) {
    const auto text = read_field<std::uintptr_t>(string, 0x4);
    return {kTagString, text != 0 ? text : host.literal(kEmptyCString)};
}

void release_string(void* string, DialogCommandHost& host) {
    if (void* text = read_field<void*>(string, 0x4)) host.pool_return_block(text, read_field<std::uint32_t>(string, 0) + 1);
}

void* scalar_delete(void* command, std::uint32_t flags, DialogCommandHost& host) {
    write_field<std::uint32_t>(command, 0x0, kDialogCommandBaseVtable);
    if (flags & 1) host.free_00bf65ac(command);
    return command;
}

}  // namespace

// ---------------------------------------------------------------------------
// Dialog command classes
// ---------------------------------------------------------------------------

void* dialog_command_scalar_delete_00449a30(void* command, std::uint32_t flags, DialogCommandHost& host) {
    return scalar_delete(command, flags, host);
}

void* dialog_command_scalar_delete_00449a50(void* command, std::uint32_t flags, DialogCommandHost& host) {
    return scalar_delete(command, flags, host);
}

void dialog_command_read_base_0044a3f0(void* command, void* reader, DialogCommandHost& host) {
    read_panel(command, reader, host);
}

void dialog_command_msg_read_0044a440(void* command, void* reader, DialogCommandHost& host) {
    read_panel(command, reader, host);
    host.read_10(reader, key(host, kKeyMessage), field_ref(host, kTagString, command, 0x8));
}

void dialog_command_setpanel_read_0044a4c0(void* command, void* reader, DialogCommandHost& host) {
    read_panel(command, reader, host);
    host.read_10(reader, key(host, kKeyName), field_ref(host, kTagString, command, 0x8));
    host.read_10(reader, key(host, kKeyCharacter), field_ref(host, kTagString, command, 0x10));
}

void dialog_command_pause_read_0044a570(void* command, void* reader, DialogCommandHost& host) {
    write_field<std::uint32_t>(command, 0x4, 0);
    host.read_10(reader, key(host, kKeyTime), field_ref(host, kTagFloat, command, 0x8));
}

void dialog_command_callback_read_0044a5b0(void* command, void* reader, DialogCommandHost& host) {
    write_field<std::uint32_t>(command, 0x4, 0);
    host.read_10(reader, key(host, kKeyCallback), field_ref(host, kTagString, command, 0x8));
}

std::int32_t dialog_command_msg_type_0044b380() noexcept { return 0; }
std::int32_t dialog_command_setpanel_type_0044b3b0() noexcept { return 1; }
std::int32_t dialog_command_callback_type_0044b3e0() noexcept { return 4; }

void dialog_command_msg_destruct_0044b3f0(void* command, DialogCommandHost& host) {
    release_string(at(command, 0x8), host);
    write_field<std::uint32_t>(command, 0x0, kDialogCommandBaseVtable);
}

void dialog_command_setpanel_destruct_0044b450(void* command, DialogCommandHost& host) {
    release_string(at(command, 0x10), host);
    release_string(at(command, 0x8), host);
    write_field<std::uint32_t>(command, 0x0, kDialogCommandBaseVtable);
}

void dialog_command_callback_destruct_0044b4d0(void* command, DialogCommandHost& host) {
    release_string(at(command, 0x8), host);
    write_field<std::uint32_t>(command, 0x0, kDialogCommandBaseVtable);
}

void* dialog_command_create_from_archive_0044be50(void* reader, DialogCommandHost& host) {
    struct LocalString {
        std::uint32_t length;
        std::uintptr_t text;
    } type{0, 0};
    host.read_with_default_0c(reader, key(host, kKeyType), {kTagString, host.address_of(&type)},
                              {kTagString, host.literal("msg")});
    void* command = nullptr;
    auto make = [&](std::uint32_t size, std::uint32_t vtable, std::size_t zero_end) {
        void* object = host.operator_new_00bf681b(size);
        if (object == nullptr) return object;
        write_field<std::uint32_t>(object, 0x0, vtable);
        for (std::size_t offset = 0x8; offset < zero_end; offset += 4) write_field<std::uint32_t>(object, offset, 0);
        return object;
    };
    if (type.text != 0 && host.stricmp_00bf7fbf(reinterpret_cast<const char*>(type.text), "msg") == 0) {
        command = make(0x10, kDialogMsgVtable, 0x10);
    } else if (host.native_string_equals_ci_00425850(&type, "setpanel")) {
        command = make(0x18, kDialogSetPanelVtable, 0x18);
    } else if (host.native_string_equals_ci_00425850(&type, "hidepanel")) {
        command = make(0x8, kDialogHidePanelVtable, 0x8);
    } else if (host.native_string_equals_ci_00425850(&type, "pause")) {
        command = make(0xC, kDialogPauseVtable, 0x8);
    } else if (host.native_string_equals_ci_00425850(&type, "callback")) {
        command = make(0x10, kDialogCallbackVtable, 0x10);
    }
    host.command_read_08(command, reader);
    if (type.text != 0) host.pool_return_block(reinterpret_cast<void*>(type.text), type.length + 1);
    return command;
}

void dialog_command_write_base_0044c060(void* command, void* writer, DialogCommandHost& host) {
    host.write_0c(writer, key(host, kKeyPanel), {kTagInt, read_field<std::uint32_t>(command, 0x4) + 1});
    std::string_view name;
    bool named = true;
    switch (host.command_type_04(command)) {
    case 0: name = "msg"; break;        // 00CE4AD8
    case 1: name = "setpanel"; break;   // 00CE4AC4
    case 2: name = "hidepanel"; break;  // 00CE4AB8
    case 3: name = "pause"; break;      // 00CE4AB0
    case 4: name = kKeyCallback; break; // 00CE49C0
    default: named = false; break;
    }
    const std::uintptr_t text = named ? host.temporary_string(name) : 0;
    host.write_0c(writer, key(host, kKeyType), {kTagString, text != 0 ? text : host.literal(kEmptyCString)});
    if (text != 0) host.release_temporary_string(text);
}

void dialog_command_msg_write_0044c220(void* command, void* writer, DialogCommandHost& host) {
    dialog_command_write_base_0044c060(command, writer, host);
    host.write_0c(writer, key(host, kKeyMessage), string_text(host, at(command, 0x8)));
}

void dialog_command_setpanel_write_0044c270(void* command, void* writer, DialogCommandHost& host) {
    dialog_command_write_base_0044c060(command, writer, host);
    host.write_0c(writer, key(host, kKeyName), string_text(host, at(command, 0x8)));
    host.write_0c(writer, key(host, kKeyCharacter), string_text(host, at(command, 0x10)));
}

void dialog_command_pause_write_0044c2f0(void* command, void* writer, DialogCommandHost& host) {
    dialog_command_write_base_0044c060(command, writer, host);
    std::uint32_t bits = read_field<std::uint32_t>(command, 0x8);  // MOVSS the float bits
    host.write_0c(writer, key(host, kKeyTime), {kTagFloat, bits});
}

void dialog_command_callback_write_0044c340(void* command, void* writer, DialogCommandHost& host) {
    dialog_command_write_base_0044c060(command, writer, host);
    host.write_0c(writer, key(host, kKeyCallback), string_text(host, at(command, 0x8)));
}

// ---------------------------------------------------------------------------
// Device entity virtuals and the device class lifetime
// ---------------------------------------------------------------------------

namespace {
void die_if_depleted(void* entity, DeviceEntityHost& host) {
    // XORPS; COMISS 0.0, [+370h]; JC skips while 0.0 < health or unordered.
    if (read_field<float>(entity, 0x370) <= 0.0f) host.entity_die_70(entity, 1);
}

void release_reference(void* slot, DeviceEntityHost& host) {
    void* object = read_field<void*>(slot, 0x0);
    if (object == nullptr) return;
    if (host.interlocked_decrement_ce2220(at(object, 0x4)) == 0) host.object_destroy_00(object);
    write_field<void*>(slot, 0x0, nullptr);
}
}  // namespace

void device_die_if_health_depleted_0043f120(void* entity, DeviceEntityHost& host) { die_if_depleted(entity, host); }
void device_die_if_health_depleted_0043f7d0(void* entity, DeviceEntityHost& host) { die_if_depleted(entity, host); }

void device_init_health_and_parts_0043f7b0(void* entity, DeviceEntityHost& host) {
    host.unit_init_health_and_parts_0087bcc0(entity);
    write_field<std::uint32_t>(entity, 0x168, host.decal_list_register_owner_00740fe0(host.decal_list_00e1aea0()));
}

bool device_handle_messages_4b4c_0043f860(void* entity, void* message, DeviceEntityHost& host) {
    const std::uint32_t type = read_field<std::uint8_t>(message, 0x10);
    if (type - 0x4B <= 1) return true;
    return host.entity_handle_message_fallback_00878350(entity, message);
}

void device_write_state_0043fea0(void* entity, void* writer, DeviceEntityHost& host) {
    host.unit_write_state_008776e0(entity, writer);
    const auto device_class = read_field<std::uint32_t>(read_field<void*>(entity, 0x354), 0x6C);
    host.write_0c(writer, key(host, kKeyDeviceClass), {kTagInt, device_class});
    host.open_table_04(writer, key(host, kKeyDevice));
    host.write_0c(writer, key(host, kKeyPlatform), {kTagInt, read_field<std::uint32_t>(entity, 0x38C)});
    host.close_table_08(writer);
}

void* device_class_construct_00440770(void* device_class, DeviceEntityHost& host) {
    host.damageable_class_construct_base_0087c640(device_class);
    write_field<std::uint32_t>(device_class, 0x0, kDeviceClassVtable);
    write_field<std::uint32_t>(device_class, 0x70, 0);
    return device_class;
}

void ref_slot_release_00440a30(void* slot, DeviceEntityHost& host) { release_reference(slot, host); }

void device_class_destruct_004420d0(void* device_class, DeviceEntityHost& host) {
    write_field<std::uint32_t>(device_class, 0x0, kDeviceClassVtable);
    void* registry = host.device_class_registry_00441780();
    const auto index = read_field<std::int32_t>(device_class, 0x6C);
    void* slots = at(registry, 0x4);
    if (index >= read_field<std::int32_t>(registry, 0x8)) host.device_class_registry_resize_00440180(slots, index + 1);
    write_field<std::uint32_t>(read_field<void*>(slots, 0x0), static_cast<std::size_t>(index) * 4, 0);
    release_reference(at(device_class, 0x70), host);
    host.damageable_class_destruct_base_0087c700(device_class);
}

// ---------------------------------------------------------------------------
// Panel sequence records
// ---------------------------------------------------------------------------

namespace {
const std::byte* cat(const void* object, std::size_t offset) noexcept {
    return static_cast<const std::byte*>(object) + offset;
}

// The inline NativeString assignment: skipped for the same object; else the
// destination is resized to the source length and, for a non-empty source,
// memcpy(dst text, src text, dst length).
void assign_string(void* dst, const void* src, PanelRecordHost& host) {
    if (dst == src) return;
    host.native_string_resize_0041dd40(dst, read_field<std::uint32_t>(src, 0), 1);
    if (read_field<std::uint32_t>(src, 0) != 0)
        host.memcpy_00bf7680(read_field<void*>(dst, 4), read_field<const void*>(src, 4), read_field<std::uint32_t>(dst, 0));
}

void construct_string(void* dst, const void* src, PanelRecordHost& host) {
    write_field<std::uint32_t>(dst, 0, 0);
    write_field<std::uint32_t>(dst, 4, 0);
    assign_string(dst, src, host);
}
}  // namespace

void* panel_sequence_slot_copy_construct_0044c620(void* dst, const void* src, PanelRecordHost& host) {
    write_field(dst, 0x0, read_field<std::uint8_t>(src, 0x0));
    construct_string(at(dst, 0x4), cat(src, 0x4), host);
    construct_string(at(dst, 0xC), cat(src, 0xC), host);
    return dst;
}

void* panel_sequence_slot_assign_0044c7c0(void* dst, const void* src, PanelRecordHost& host) {
    write_field(dst, 0x0, read_field<std::uint8_t>(src, 0x0));
    assign_string(at(dst, 0x4), cat(src, 0x4), host);
    assign_string(at(dst, 0xC), cat(src, 0xC), host);
    return dst;
}

void panel_sequence_slot_construct_at_0044c8d0(void* place, const void* src, PanelRecordHost& host) {
    if (place != nullptr) panel_sequence_slot_copy_construct_0044c620(place, src, host);
}

void* record40_construct_0044f9d0(void* record) noexcept {
    write_field<std::uint32_t>(record, 0x0, kRecord40Vtable);
    for (std::size_t offset : {0x4u, 0x8u, 0xCu}) write_field<float>(record, offset, 0.0f);
    write_field<std::uint8_t>(record, 0x10, 0);
    for (std::size_t offset : {0x18u, 0x1Cu, 0x20u, 0x24u, 0x2Cu, 0x30u, 0x34u})
        write_field<std::uint32_t>(record, offset, 0);
    write_field<std::uint8_t>(record, 0x38, 0);
    write_field<std::uint32_t>(record, 0x3C, 0);
    return record;
}

void* record40_copy_construct_0044f380(void* dst, const void* src, PanelRecordHost& host) {
    write_field<std::uint32_t>(dst, 0x0, kRecord40Vtable);
    for (std::size_t offset : {0x4u, 0x8u, 0xCu}) write_field(dst, offset, read_field<float>(src, offset));
    write_field(dst, 0x10, read_field<std::uint8_t>(src, 0x10));
    host.copy_member_0044dc40(at(dst, 0x14), cat(src, 0x14));
    write_field(dst, 0x24, read_field<std::uint32_t>(src, 0x24));
    host.copy_member_0044ef30(at(dst, 0x28), cat(src, 0x28));
    write_field(dst, 0x38, read_field<std::uint8_t>(src, 0x38));
    write_field(dst, 0x3C, read_field<std::uint32_t>(src, 0x3C));
    return dst;
}

void* panel_sequence_pair_copy_construct_00450450(void* dst, const void* src, PanelRecordHost& host) {
    construct_string(dst, src, host);
    record40_copy_construct_0044f380(at(dst, 0x8), cat(src, 0x8), host);
    return dst;
}

void time_of_day_split_clock_00443a30(const void* clock, std::int32_t* hours, std::int32_t* minutes,
                                      std::int32_t* seconds, std::int32_t* milliseconds) noexcept {
    const float t = read_field<float>(clock, 0x4);
    const auto h = static_cast<std::int32_t>(t);
    *hours = h;
    if (minutes == nullptr) return;
    const auto mf = static_cast<float>((static_cast<double>(t) - h) * 60.0);  // 00CE3D68
    const auto m = static_cast<std::int32_t>(mf);
    *minutes = m;
    if (seconds == nullptr) return;
    const auto sf = static_cast<float>((static_cast<double>(mf) - m) * 60.0);
    const auto sec = static_cast<std::int32_t>(sf);
    *seconds = sec;
    if (milliseconds == nullptr) return;
    const auto msf = static_cast<float>((static_cast<double>(sf) - sec) * 1000.0);  // 00CE47A0
    *milliseconds = static_cast<std::int32_t>(msf);
}

void string_format_buffer_2a_0043ee60(SmallOwnerHost& host, char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    host.vsprintf_s_00bf7289(buffer, 0x2A, format, args);
    va_end(args);
}

void owned_ptr_delete_reset_00449310(void* slot, SmallOwnerHost& host) {
    void* object = read_field<void*>(slot, 0x0);
    if (object == nullptr) return;
    host.object_scalar_delete_00(object, 1);
    write_field<void*>(slot, 0x0, nullptr);
}

void weather_config_destruct_00445940(void* config, SmallOwnerHost& host) {
    write_field<std::uint32_t>(config, 0x0, kWeatherConfigVtable);
    for (std::size_t offset : {0x68u, 0x5Cu, 0x38u}) {
        void* map = at(config, offset);
        void* head = read_field<void*>(map, 0x4);
        std::byte out[8];
        host.float_map_erase_00444b10(map, out, map, read_field<void*>(head, 0x0), map, head);
        host.free_00bf65ac(read_field<void*>(map, 0x4));
        write_field<std::uint32_t>(map, 0x4, 0);
        write_field<std::uint32_t>(map, 0x8, 0);
    }
    if (void* text = read_field<void*>(config, 0x30)) host.pool_return_block(text, read_field<std::uint32_t>(config, 0x2C) + 1);
}

}  // namespace bsp::seg3_4_cc13
