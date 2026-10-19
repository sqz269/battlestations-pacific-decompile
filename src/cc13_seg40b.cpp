// Segment 40 leftovers, packet cc13-seg40b (marker cc13_seg40b). See include/bsp/cc13_seg40b.hpp
// and docs/CC13_SEG40B.md. Not bound into bsp_game.

#include "bsp/cc13_seg40b.hpp"

#include <cstddef>

namespace bsp::cc13_seg40b {

namespace {
void store_vtable(void* object, std::uint32_t vtable) noexcept {
    *static_cast<std::uint32_t*>(object) = vtable;   // MOV dword ptr [ECX], imm32
}
}  // namespace

const char* gun_bot_default_name_0072bc70(const void*) noexcept { return ""; }

bool gun_bot_default_false_0072bc90(const void*, std::uint32_t) noexcept { return false; }

float gun_bot_default_one_0072bca0(const void*) noexcept { return 1.0f; }

float gun_bot_default_one_0072bcb0(const void*) noexcept { return 1.0f; }

void gun_bot_default_noop2_0072bcc0(void*, std::uint32_t, std::uint32_t) noexcept {}

std::uint32_t gun_bot_default_zero_0072bcd0(const void*) noexcept { return 0; }

void gun_bot_default_noop1_0072bd00(void*, std::uint32_t) noexcept {}

void session_message_root_store_vtable_00729090(void* object) noexcept {
    store_vtable(object, kSessionMessageRootVtable);
}

void session_message_root_store_vtable_007290a0(void* object) noexcept {
    store_vtable(object, kSessionMessageRootVtable);
}

void session_message_root_store_vtable_007293b0(void* object) noexcept {
    store_vtable(object, kSessionMessageRootVtable);
}

void shot_command_base_store_vtable_00730b50(void* object) noexcept {
    store_vtable(object, kShotCommandBaseVtable);
}

namespace {
template <class T>
T& field(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
template <class T>
const T& field(const void* base, std::size_t off) noexcept {
    return *reinterpret_cast<const T*>(static_cast<const std::uint8_t*>(base) + off);
}
}  // namespace

void gun_set_fire_mode_write_00728de0(Cc13Seg40Host& host, void* message, void* cursor) {
    host.session_message_write_extended_header_0075b480(message, cursor);              // 00728DE9
    host.bit_cursor_write_boolean_004290b0(cursor, field<std::uint8_t>(message, 0x1C));  // MOVZX, 00728DF5
    host.bit_cursor_write_unsigned_dword_bits_00429070(cursor, field<std::uint32_t>(message, 0x20), 2);
}

void gun_set_fire_mode_read_00728e10(Cc13Seg40Host& host, void* message, void* stream) {
    host.session_message_read_extended_header_0075b4c0(message, stream);
    // The bit reader is the stream's +4h member (00728E21 ADD EDI,4 / MOV ECX,EDI).
    void* reader = static_cast<std::uint8_t*>(stream) + 0x4;
    host.bit_cursor_read_boolean_00428d70(reader, &field<std::uint8_t>(message, 0x1C));
    host.bit_cursor_read_unsigned_dword_00428d10(reader, &field<std::uint32_t>(message, 0x20), 2);
}

void r_t_gun_set_contfiring_write_00728ec0(Cc13Seg40Host& host, void* message, void* cursor) {
    host.session_message_write_extended_header_0075b480(message, cursor);
    host.bit_cursor_write_boolean_004290b0(cursor, field<std::uint8_t>(message, 0x1C));
    host.bit_cursor_write_boolean_004290b0(cursor, field<std::uint8_t>(message, 0x20));
}

void r_t_gun_set_contfiring_read_00728ef0(Cc13Seg40Host& host, void* message, void* stream) {
    host.session_message_read_extended_header_0075b4c0(message, stream);
    void* reader = static_cast<std::uint8_t*>(stream) + 0x4;           // 00728F01 ADD EDI,4
    host.bit_cursor_read_boolean_00428d70(reader, &field<std::uint8_t>(message, 0x1C));
    host.bit_cursor_read_boolean_00428d70(reader, &field<std::uint8_t>(message, 0x20));
}

bool destroyed_property_get_0072a090(Cc13Seg40Host& host, const void* object, const void* key,
                                     std::uint8_t* out) {
    const char* name = field<const char*>(key, 0x4);                 // 0072A094
    if (name == nullptr) return false;                               // 0072A09C
    if (host.crt_stricmp_00bf7fbf(name, "destroyed") != 0) return false;   // 0072A0A4, SETZ
    *out = field<std::uint8_t>(object, 0x3B8);                       // 0072A0B5..0072A0BF
    return true;
}

void torpedo_shot_command_write_007312e0(Cc13Seg40Host& host, const void* command, void* writer) {
    // Each call passes two 8-byte tagged arguments: (kind 0, key text) then the value.
    host.archive_vcall_0c_put(writer, 0, "type", 0,
                              static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>("torpedo")));
    host.archive_vcall_0c_put(writer, 0, "heading", 2, field<std::uint32_t>(command, 0x4));    // 00731315
    host.archive_vcall_0c_put(writer, 0, "swimdepth", 2, field<std::uint32_t>(command, 0x8));  // 00731345
}

void* singleton_01090298_scalar_deleting_destructor_007358c0(Cc13Seg40Host& host, void* object,
                                                             std::uint32_t flags,
                                                             void** singleton_cell_01090298) {
    *singleton_cell_01090298 = nullptr;                              // 007358C8
    store_vtable(object, 0x00CE3818u);                               // 007358D2, SingletonBase
    if ((flags & 1u) != 0) host.crt_free_00bf65ac(object);           // 007358DB
    return object;
}

void* game_member_1a0c_00735b40(void* game_00e188a8) noexcept {
    return static_cast<std::uint8_t*>(game_00e188a8) + 0x1A0C;       // ADD EAX,1A0Ch
}

}  // namespace bsp::cc13_seg40b
