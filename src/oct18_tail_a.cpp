#include "bsp/oct18_tail_a.hpp"
#include <cstring>
#include "bsp/native_string.hpp"
#include "bsp/entity_mission_seg60_argon.hpp"

namespace bsp::oct18_tail_a {
// Original: MOV EAX,4; RET. No register/stack input is read.
std::uint32_t air_ops_block_kind_006ca5b0() noexcept { return 4; }
// Original: RET 4; ECX and one stack word are unobserved.
void air_ops_site_platform_tick_006cebe0(void*, std::uint32_t) noexcept {}
// Original: RET 0Ch; ECX and three stack words are unobserved.
void air_ops_site_slot50_006cec10(void*, std::uint32_t, std::uint32_t,
                                std::uint32_t) noexcept {}
// Original: MOV AL,1; RET. Upper EAX is unspecified.
std::uint8_t airfield_site_slot30_006cf3f0() noexcept { return 1; }
// Original: RET 4, unobserved ECX and stack word.
void mothership_site_slot14_006cfe30(void*, std::uint32_t) noexcept {}
// Original: LEA EAX,[ECX+15Ch]; RET. Interior pointer, not a load.
void* air_ops_block_member_006d0d20(void* self) noexcept {
    return static_cast<std::uint8_t*>(self) + 0x15c;
}
// Original: MOV EAX,[ECX+8C8h]; RET. Pointer-valued field, original Win32 layout.
void* airfield_weapon_director_006d0d30(const void* self) noexcept {
    void* result;
    std::memcpy(&result, static_cast<const std::uint8_t*>(self) + 0x8c8, sizeof(result));
    return result;
}
// Original: RET; no arguments are observed. Virtual slot's semantic type remains unknown.
void unit_slot1a8_006d1640() noexcept {}
// Original: MOV EAX,10h; RET.
std::uint32_t airfield_class_recon_category_006d1d20() noexcept { return 0x10; }
// Original: MOV AL,1; RET; upper EAX unspecified.
std::uint8_t airfield_slotb4_006d1d40() noexcept { return 1; }
// Original: RET 4; the incoming sync message is ignored.
void airfield_apply_sync_message_006d1d60(void*, std::uint32_t) noexcept {}
// Original: MOV EAX,14h; RET.
std::uint32_t air_ops_block_kind_006d1d70() noexcept { return 0x14; }
// Original: MOV EAX,6; RET.
std::uint32_t unit_slot190_006d1de0() noexcept { return 6; }
// Original: XOR EAX,EAX; RET.
std::uint32_t unit_slot1d4_006d1ee0() noexcept { return 0; }
// Original: MOV AL,[00E17BF2]; RET. Explicit borrowed global, not a cached value.
std::uint8_t unit_slot1e4_006d1f40(const volatile std::uint8_t& flag) noexcept { return flag; }
// Original: RET 4; ECX and one stack word unobserved.
void unit_slot1ec_006d1f90(void*, std::uint32_t) noexcept {}
// Original: MOV EAX,00CFB38C; RET, returning this exact text in image storage.
const char* command_class_base_name_006f7f30() noexcept { return "undefined command name"; }

namespace {
template<class T> T read(const void* base, std::uint32_t offset) noexcept {
    T result;
    std::memcpy(&result, static_cast<const std::uint8_t*>(base) + offset, sizeof(result));
    return result;
}
template<class T> void write(void* base, std::uint32_t offset, T value) noexcept {
    std::memcpy(static_cast<std::uint8_t*>(base) + offset, &value, sizeof(value));
}
void* add(void* p, std::uint32_t n) noexcept { return static_cast<std::uint8_t*>(p) + n; }
const void* add(const void* p, std::uint32_t n) noexcept {
    return static_cast<const std::uint8_t*>(p) + n;
}
void empty_header(void* header) noexcept {
    write<std::uint32_t>(header, 0, 0);
    write<void*>(header, 4, nullptr);
}
void copy_header(void* destination, const void* source, NativeStringRawPoolContext& strings) {
    if (destination != source) {
        resize_native_string_header_0041dd40(destination, strings,
                                             read<std::uint32_t>(source, 0), true);
        // Native checks current SOURCE length, but copies current DESTINATION length.
        if (read<std::uint32_t>(source, 0) != 0) {
            std::memmove(read<void*>(destination, 4), read<const void*>(source, 4),
                        read<std::uint32_t>(destination, 0));
        }
    }
}
struct HeaderCleanup {
    void* header;
    NativeStringRawPoolContext& strings;
    bool armed{true};
    ~HeaderCleanup() noexcept {
        if (armed) destroy_native_string_header_0041dd20(header, strings);
    }
};
} // namespace

// These three original ECX getters load EAX and RET. Field meanings are hypotheses.
void* plane_chase_camera_target_0042abd0(const void* self) noexcept {
    return read<void*>(self, 0x388);
}
void* bomb_view_camera_target_00435390(const void* self) noexcept {
    return read<void*>(self, 0x398);
}
void* ship_camera_target_00519340(const void* self) noexcept {
    return read<void*>(self, 0x3ac);
}
// ECX 8-byte key, stack text, EAX original self, RET4. Padding bytes +5..7 untouched.
void* construct_borrowed_key_00438f80(void* self, const char* text) noexcept {
    write(self, 0, text);
    write<std::uint8_t>(self, 4, 0);
    return self;
}
// FLD dword [ECX+3E0h]; RET. MSVC Win32 returns float through ST0.
float weapon_scale_006e2850(const void* self) noexcept { return read<float>(self, 0x3e0); }
// MOV EAX,[00E188D8]; RET; incoming ECX unused.
void* hud_controlled_unit_00644230(void* const volatile& global) noexcept { return global; }
// Single RET. Message is passed in ECX and ignored in this retail image.
void trace_message_004c9c90(const char*) noexcept {}

// ECX fresh14h destination -> EAX self; RET. C++ cleanup models the armed native
// string state; this does not reproduce FH3/FS registrations or native exception ABI.
void* checkpoint_record_construct_00436690(void* self, NativeStringRawPoolContext& strings) {
    void* const header = add(self, 0xc);
    empty_header(header);
    HeaderCleanup cleanup{header, strings};
    write<std::uint32_t>(self, 0, 0);
    write<std::uint32_t>(self, 4, 0xffffffffu);
    write<std::uint32_t>(self, 8, 0xffffffffu);
    resize_native_string_header_0041dd40(header, strings, 0, false);
    if (void* const data = read<void*>(header, 4)) {
        // Native copies length bytes of empty literal 00CE3A0C. In the valid fresh
        // header domain length remains zero; retain the actual zero-byte copy call.
        std::memcpy(data, "", read<std::uint32_t>(header, 0));
    }
    cleanup.armed = false;
    return self;
}
// ECX destination, stack source; EAX self; RET4. No native EH frame. Zeroing the
// destination header precedes the alias check, including a self-copy.
void* checkpoint_record_copy_construct_00437490(void* self, const void* source,
                                             NativeStringRawPoolContext& strings) {
    write(self, 0, read<std::uint32_t>(source, 0));
    write(self, 4, read<std::uint32_t>(source, 4));
    write(self, 8, read<std::uint32_t>(source, 8));
    void* const header = add(self, 0xc);
    empty_header(header);
    copy_header(header, add(source, 0xc), strings);
    return self;
}
// ECX fresh10h destination; two stack string-header pointers; EAX self; RET8.
// Only the completed FIRST header is armed while constructing the second.
void* traffic_string_pair_construct_0048e1f0(void* self, const void* first,
                                          const void* second, NativeStringRawPoolContext& strings) {
    empty_header(self);
    copy_header(self, first, strings);
    HeaderCleanup cleanup{self, strings};
    void* const header = add(self, 8);
    empty_header(header);
    copy_header(header, second, strings);
    cleanup.armed = false;
    return self;
}
// ECX fresh10h destination; stack pair source; EAX self; RET4.
void* traffic_string_pair_copy_construct_0048e500(void* self, const void* source,
                                               NativeStringRawPoolContext& strings) {
    empty_header(self);
    copy_header(self, source, strings);
    HeaderCleanup cleanup{self, strings};
    void* const header = add(self, 8);
    empty_header(header);
    copy_header(header, add(source, 8), strings);
    cleanup.armed = false;
    return self;
}
// Each original is one JMP 0048DBD0; ECX entity, no stack inputs, RET via target.
void entity_init_thunk_004a6b90(cc13_seg7::Seg7Host& host, cc13_seg7::Object self) {
    cc13_seg7::class_entity37_sentity_init_base_0048dbd0(host, self);
}
void entity_init_thunk_004ab030(cc13_seg7::Seg7Host& host, cc13_seg7::Object self) {
    cc13_seg7::class_entity37_sentity_init_base_0048dbd0(host, self);
}
// Original JMP 004F71D0, ECX screen. Target source API takes the slot-0 result
// explicitly; table/screen representation and its limits are inherited unchanged.
void frontend_register_thunk_0054ed00(FrontEndScreenTable& table, FrontEndScreen& screen,
                                    int screen_id) noexcept {
    register_front_end_screen_004f71d0(table, screen, screen_id);
}
// Original JMP 0042A910, itself JMP 00923840. ECX entity, RET via final target.
void inherit_race_thunk_0060c610(void* self) noexcept {
    entity_mission_seg60_argon::entity_inherit_race_and_activate_00923840(self);
}
// Original JMP 0072D7E0. ECX gun, RET via target; reuse existing whole target source.
void catapult_init_thunk_006eb600(void* self, Cc13Seg48UHost& host) {
    cc13_seg48U_gun_base_init_device_0072d7e0(self, host);
}
// ECX entity; RET. Reload +30h AFTER parent registration, as native assembly does.
void register_world_lists_004f1870(void* self, WorldListServices& services) {
    services.register_parent_00928560(self);
    services.push_back_00484540(add(read<void*>(self, 0x30), 0x30), self);
}
// ECX entity; RET. First matching payload only, ordinary null-terminated UnitList.
void unregister_world_lists_004f1890(void* self, WorldListServices& services) {
    services.unregister_parent_00928570(self);
    void* const list = add(read<void*>(self, 0x30), 0x30);
    void* node = read<void*>(list, 4);
    while (node != nullptr) {
        if (read<void*>(node, 8) == self) {
            services.erase_004837d0(list, node);
            return;
        }
        node = read<void*>(node, 4);
    }
}
// ECX dialog owner, stack output vector; RET4. EDX is scratch, not an input.
// Checked STL iteration/append remain required bindings, rather than guessed trees.
void collect_active_dialog_ids_00450750(void* self, void* output, DialogServices& services) {
    void* const owner = add(self, 0x1c);
    DialogIterator iterator{owner, read<void*>(read<void*>(self, 0x20), 0)};
    for (;;) {
        void* const current_owner = iterator.owner;
        void* const current_node = iterator.node;
        void* const sentinel = read<void*>(owner, 4); // Native EBX captured before validation.
        if (current_owner == nullptr || current_owner != owner)
            services.invalid_parameter_00bf6713();
        if (current_node == sentinel) return;
        if (current_owner == nullptr)
            services.invalid_parameter_00bf6713();
        if (current_node == read<void*>(current_owner, 4))
            services.invalid_parameter_00bf6713();
        services.append_native_string_00450540(output, add(current_node, 0xc));
        services.increment_tree_iterator_00448c10(iterator);
    }
}
} // namespace bsp::oct18_tail_a
