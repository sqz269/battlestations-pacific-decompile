#include "bsp/oct18_tail_a.hpp"
#include <cstring>
#include "bsp/native_string.hpp"
#include "bsp/entity_mission_seg60_argon.hpp"
#include "bsp/native_scene_dword_erase_range.hpp"
#include "bsp/cc13_big53.hpp"
#include "bsp/cc13_left8b_seg8.hpp"
#include <intrin.h>
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_lua_field_setters.hpp"
#include "bsp/native_lua_class_setters.hpp"
#include "bsp/native_lua_object_boolean.hpp"
#include "bsp/native_lua_number_field.hpp"
#include "bsp/native_lua_string_field.hpp"
#include "bsp/native_lua_numeric_element.hpp"

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
namespace {
using Word = std::uint32_t;
Word bits(const void* p) noexcept { return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p)); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)); }
Word vector_count(const void* vector) noexcept {
    const Word begin = read<Word>(vector, 4);
    return begin == 0 ? 0 : static_cast<Word>(static_cast<std::int32_t>(read<Word>(vector, 8) - begin) >> 2);
}
void clear_layer_bytes(void* vector, GridServices& services) {
    const Word end = read<Word>(vector, 8);
    if (read<Word>(vector, 4) > end) services.invalid_parameter_00bf6713();
    const Word begin = read<Word>(vector, 4);
    if (begin > read<Word>(vector, 8)) services.invalid_parameter_00bf6713();
    if (begin != end) std::memset(pointer(begin), 0, end - begin);
}
void copy_three_words(void* destination, const void* source) noexcept {
    const Word a = read<Word>(source, 0), b = read<Word>(source, 4), c = read<Word>(source, 8);
    write(destination, 0, a); write(destination, 4, b); write(destination, 8, c);
}
struct LegacyName {
    NativeLegacySboStringStorage storage;
    explicit LegacyName(const char* name, Word size) : storage{} {
        storage.capacity_18 = 15;
        native_legacy_sbo_string_assign_counted_00408720(storage, name, size);
    }
    ~LegacyName() noexcept { native_legacy_sbo_string_destroy_004072d0(storage); }
};
}

// ECX layer, stack signed dimension, RET4. Preserve native FILD/FDIVR64/FSTP32,
// including the caller's x87 control word and signed zero/infinity behavior.
void terrain_layer_set_dimension_00423960(void* self, std::int32_t dimension,
                                        const GridConstants& constants, GridServices& services) {
    const float extent = constants.extent_00ce3968;
    const double diameter = constants.diameter_00ce3960;
    float width;
    __asm {
        fild dword ptr dimension
        fdivr qword ptr diameter
        fstp dword ptr width
    }
    write(self, 4, extent);
    const float multiplier = constants.multiplier_00ce3958;
    write(self, 8, dimension);
    write(self, 0x10, multiplier);
    write(self, 0xc, width);
    void* const vector = add(self, 0x18);
    const Word unsigned_dimension = static_cast<Word>(dimension);
    services.resize_bytes_004219a0(vector, unsigned_dimension * unsigned_dimension, 0);
    clear_layer_bytes(vector, services);
}
// ECX registry, RET. Entry sentinel is captured; the live sentinel is rechecked
// before dereference and increment, so returning handlers may repair storage.
void reset_avoid_zone_layers_00423af0(void* self, const GridConstants& constants,
                                   GridServices& services) {
    void* const owner = add(self, 4);
    void* const stop = read<void*>(self, 8);
    void* node = read<void*>(stop, 0);
    while (node != stop) {
        if (node == read<void*>(owner, 4)) services.invalid_parameter_00bf6713();
        void* const layer = read<void*>(node, 8);
        write<float>(layer, 4, constants.extent_00ce3968);
        write<float>(layer, 0xc, constants.default_width_00ce396c);
        const float multiplier = constants.multiplier_00ce3958;
        write<std::int32_t>(layer, 8, 10);
        write(layer, 0x10, multiplier);
        void* const vector = add(layer, 0x18);
        services.resize_bytes_004219a0(vector, 100, 0);
        clear_layer_bytes(vector, services);
        if (node == read<void*>(owner, 4)) services.invalid_parameter_00bf6713();
        node = read<void*>(node, 0);
    }
}
// No native arguments; RET. The original vector/list globals are borrowed.
// Compose the already reconstructed complete checked DWORD-range erase directly.
void clear_pending_class_ids_004c8aa0(PendingClassState& state,
                                    const SingletonLifetimeCallbacks& callbacks) {
    void* const vector = state.vector_00f8a09c;
    Word end = read<Word>(vector, 8), first = read<Word>(vector, 4);
    const Word saved_end = end;
    if (first > end) {
        callbacks.invalid_parameter(callbacks.context);
        end = read<Word>(vector, 8);
        first = read<Word>(vector, 4);
    }
    if (first > end) callbacks.invalid_parameter(callbacks.context);
    void* output[2];
    void* parameters[5]{output, vector, pointer(first), vector, pointer(saved_end)};
    erase_native_scene_dword_range_004685a0(vector, parameters);
    void* head = state.head_00f8a0b0;
    void* node = read<void*>(head, 0);
    write(head, 0, head);
    head = state.head_00f8a0b0;
    write(head, 4, head);
    state.count_00f8a0b4 = 0;
    while (node != state.head_00f8a0b0) {
        void* const next = read<void*>(node, 0);
        singleton_lifetime_free(node);
        node = next;
    }
}
// ECX block, one unread stack word, RET4 (the decompiler omitted the word).
void pull_ready_plane_006c6540(void* self, std::uint32_t, AirOpsServices& services) {
    if (read<void*>(self, 0x38) != nullptr || read<Word>(self, 0xdc) == 0) return;
    void* const head = read<void*>(self, 0xd8);
    void* const node = read<void*>(head, 0);
    if (node == head) services.invalid_parameter_00bf6713();
    void* const plane = read<void*>(node, 8);
    services.remove_ready_front_006c4a70(add(self, 0xc0));
    void* const old = read<void*>(self, 0x38);
    void* const observer = add(self, 0x24);
    if (old != plane) {
        if (old != nullptr) services.unregister_observer_006952a0(old, observer);
        write(self, 0x38, plane);
        if (plane != nullptr) services.register_observer_00694a60(plane, observer);
    }
    services.scene_enable_00922f30(plane, 0);
}
// ECX HUD root, RET. Capture the actual slot BEFORE deletion; reload the vector
// for the following iteration. Native valid-container domain is retained.
void hud_root_exit_00646c50(void* self, NativeStringRawPoolContext& strings, HudServices& services) {
    write<Word>(self, 0x6c, 0);
    void* const header = add(self, 0x70);
    resize_native_string_header_0041dd40(header, strings, 0, false);
    if (void* const data = read<void*>(header, 4))
        std::memcpy(data, "", read<Word>(header, 0));
    void* const vector = add(self, 0x104);
    Word index = 0, begin;
    for (;;) {
        begin = read<Word>(vector, 4);
        if (begin == 0 || index >= vector_count(vector)) break;
        if (begin == 0 || index >= vector_count(vector)) services.invalid_parameter_00bf6713();
        void* const slot = pointer(read<Word>(vector, 4) + index * 4);
        if (void* const child = read<void*>(slot, 0)) {
            services.delete_slot4(child, 1);
            write<void*>(slot, 0, nullptr);
        }
        ++index;
    }
    const Word saved_end = read<Word>(vector, 8);
    if (begin > saved_end) services.invalid_parameter_00bf6713();
    const Word current_begin = read<Word>(vector, 4);
    if (current_begin > read<Word>(vector, 8)) services.invalid_parameter_00bf6713();
    if (current_begin != saved_end) {
        const std::int32_t tail_count = static_cast<std::int32_t>(read<Word>(vector, 8) - saved_end) >> 2;
        const Word bytes = static_cast<Word>(tail_count) * 4;
        if (tail_count > 0) std::memmove(pointer(current_begin), pointer(saved_end), bytes);
        write<Word>(vector, 8, current_begin + bytes);
    }
}
// ECX marker screen; stack unit/three flag words/kind, RET14h. Last incoming kind
// becomes a boolean; the marker kind sent to 00642040 is always2.
void add_subunit_markers_00642b80(void* screen, void* unit, Word a, Word b, Word c,
                                std::int32_t flag, HudServices& services) {
    void* const vector = add(unit, 0x394);
    for (Word i = 0; i < vector_count(vector); ++i) {
        if (read<Word>(vector, 4) == 0 || i >= vector_count(vector))
            services.invalid_parameter_00bf6713();
        if (read<void*>(pointer(read<Word>(vector, 4) + i * 4), 0) != nullptr) {
            if (read<Word>(vector, 4) == 0 || i >= vector_count(vector))
                services.invalid_parameter_00bf6713();
            void* const child = read<void*>(pointer(read<Word>(vector, 4) + i * 4), 0);
            services.build_marker_00642040(screen, child, a, b, c, flag != 0, 2);
        }
    }
}
// Whole five-byte JMP00928860, ECX entity, stack three pointers, RET0Ch via target.
void place_in_world_thunk_006dfe40(entity_mission_seg60_argon::Seg60Host& host,
    void* self, void* hierarchy_parent, void* world_node, const void* matrix) {
    entity_mission_seg60_argon::game_entity_place_in_world_00928860(
        host, self, hierarchy_parent, world_node, matrix);
}
// Whole JMP00879AD0, ECX class, RET; compose existing complete borrowed native binder.
void damageable_class_bind_thunk_006eb7c0(void* self, const NativeDamageableClassBindingContext& context) {
    bind_native_damageable_class_model_00879ad0(self, context);
}
// Whole JMP00765000, ECX message, stack cursor, RET4 via target.
// The existing target codec uses a typed message snapshot; raw layout ABI is not added.
void write_entity_create_thunk_00767d70(const net_cc13::EntityCreateMessage& message,
    NativeBitCursor* cursor, const NativeBitNumericContext& numeric) {
    net_cc13::write_entity_create_00765000(message, cursor, numeric);
}
// ECX class, RET. Ship base first, last named groups/index2 then0, first points.
void mothership_bind_model_00759120(void* self, cc13_big53::Big53Host& base,
                                  const SingletonLifetimeCallbacks& callbacks) {
    cc13_big53::ship_class_bind_model_data_0082fe30(base, self);
    {
        LegacyName name("liftexitpoint", 13);
        void* const group = find_native_game_group_00718000(read<void*>(self, 0x50), name.storage, 2, callbacks);
        if (read<void*>(group, 0x48) == nullptr || count_native_named_group_points_004fba10(group) == 0)
            callbacks.invalid_parameter(callbacks.context);
        copy_three_words(add(self, 0x808), read<void*>(group, 0x48));
    }
    {
        LegacyName name("runwaycenter", 12);
        void* const group = find_native_game_group_00718000(read<void*>(self, 0x50), name.storage, 0, callbacks);
        if (read<void*>(group, 0x48) == nullptr || count_native_named_group_points_004fba10(group) == 0)
            callbacks.invalid_parameter(callbacks.context);
        copy_three_words(add(self, 0x814), read<void*>(group, 0x48));
    }
    void* const destination = add(self, 0x80c);
    const void* const source = add(self, 0x818);
    __asm {
        mov eax, source
        fld dword ptr [eax]
        mov eax, destination
        fstp dword ptr [eax]
    }
}
// ECX library screen, RET. Exact ordinary string lifetime/call ordering, fresh
// player/sound publications, actual raw pooled strings, required movie services.
void play_library_movie_004ff580(void* self, MovieState& state,
                               NativeStringRawPoolContext& strings, MovieServices& services) {
    if (services.is_playing_004f89a0(state.player_00e18d48)) return;
    const Word index = read<Word>(self, 0x434);
    const Word begin = read<Word>(self, 0x448);
    const Word count = static_cast<Word>(static_cast<std::int32_t>(read<Word>(self, 0x44c) - begin) / 0x6c);
    if (begin == 0 || index >= count) services.invalid_parameter_00bf6713();
    if (read<std::uint8_t>(pointer(read<Word>(self, 0x448) + index * 0x6c), 0x45) != 0) return;
    Word path[2], filename[2];
    construct_native_string_header_0041e870(path, strings, "movies/");
    HeaderCleanup path_cleanup{path, strings};
    construct_native_string_header_0041e870(filename, strings,
        state.names_00e084b8[read<Word>(self, 0x434)]);
    HeaderCleanup filename_cleanup{filename, strings};
    const Word filename_length = read<Word>(filename, 0);
    const void* const filename_data = read<void*>(filename, 4);
    if (filename_length != 0) {
        const Word old_length = read<Word>(path, 0);
        resize_native_string_header_0041dd40(path, strings, old_length + filename_length, true);
        std::memmove(add(read<void*>(path, 4), old_length), filename_data, filename_length);
    }
    filename_cleanup.armed = false;
    destroy_native_string_header_0041dd20(filename, strings);
    services.play_004f8a20(state.player_00e18d48, path, 1, state.value_00ceb364, 0);
    services.completion_004f8970(state.player_00e18d48, 0x004fe7a0);
    services.request_fade_00a85c00(read<void*>(state.sound_owner_00e198ac, 0x50));
    path_cleanup.armed = false;
    destroy_native_string_header_0041dd20(path, strings);
}

// Original entries are single JMPs; ECX is forwarded, no own stack arguments/EH.
void health_changed_thunk_006d0d40(void* self, DeferredTargetServices& targets) {
    targets.health_changed_00958a30(self);
}
void gun_setup_thunk_006e35a0(void* self, DeferredTargetServices& targets) {
    targets.gun_setup_0072e6d0(self);
}
// Whole 004A5620..004A5711; ECX manager, RET. The record iterator and captured
// end survive returning diagnostics. Only constructor failure frees allocation:
// a subsequent map-insertion failure does NOT destroy the completed runtime.
void build_traffic_runtime_004a5620(void* self, TrafficRecordRange& range,
    TrafficBuildServices& services, cc13_left8b::Left8bHost& constructor) {
    Word end = bits(range.end_00f8a08c), cursor = bits(range.begin_00f8a088);
    if (cursor > end) { services.invalid_parameter_00bf6713(); end = bits(range.end_00f8a08c); }
    for (;;) {
        const Word stop = end;
        if (bits(range.begin_00f8a088) > end) {
            services.invalid_parameter_00bf6713();
            end = bits(range.end_00f8a08c);
        }
        if (cursor == stop) return;
        if (cursor >= end) services.invalid_parameter_00bf6713();
        void* const record = read<void*>(pointer(cursor), 0);
        void* key = record;
        void* const map = add(self, 0x10);
        if (*services.map_subscript_00499320(map, key) == nullptr) {
            void* const allocation = services.allocate_00bf681b(0x170);
            void* runtime = nullptr;
            try {
                if (allocation) runtime = cc13_left8b::scene_traffic_runtime_construct_004a50d0(
                    constructor, static_cast<std::uint8_t*>(allocation),
                    read<std::uint8_t*>(self, 8), static_cast<std::uint8_t*>(record));
            } catch (...) {
                services.free_00bf65ac(allocation); // Native state0 -> funclet00C63C80.
                throw;
            }
            *services.map_subscript_00499320(map, key) = runtime;
        }
        end = bits(range.end_00f8a08c);
        if (cursor >= end) { services.invalid_parameter_00bf6713(); end = bits(range.end_00f8a08c); }
        cursor += 4;
    }
}
namespace {
void copy_native_float32_a(void* destination, const void* source) noexcept {
    __asm {
        mov eax, source
        fld dword ptr [eax]
        mov eax, destination
        fstp dword ptr [eax]
    }
}
}
// ECX destination, stack source, EAX destination, RET4. No parent EH frame and no
// rollback: completed member assignments remain visible if a later binding throws.
void* assign_mission_record_005c7b80(void* self, const void* source,
                                  NativeStringRawPoolContext& strings, MissionAssignServices& services) {
    for (Word offset = 0; offset != 0x60; offset += 8)
        copy_header(add(self, offset), add(source, offset), strings);
    for (Word offset = 0x60; offset != 0x6c; offset += 4)
        write(self, offset, read<Word>(source, offset));
    for (Word offset = 0x6c; offset != 0x78; offset += 4)
        copy_native_float32_a(add(self, offset), add(source, offset));
    services.string_vector_assign_00506c80(add(self, 0x78), add(source, 0x78));
    services.pointer_vector_assign_004f60e0(add(self, 0x88), add(source, 0x88));
    copy_header(add(self, 0x98), add(source, 0x98), strings);
    void* const incoming = read<void*>(source, 0xa0);
    void* const outgoing = read<void*>(self, 0xa0);
    if (incoming != outgoing) {
        write(self, 0xa0, incoming);
        if (incoming) _InterlockedIncrement(reinterpret_cast<volatile long*>(add(incoming, 4)));
        if (outgoing && _InterlockedDecrement(reinterpret_cast<volatile long*>(add(outgoing, 4))) == 0)
            services.picture_destroy_slot0(outgoing);
    }
    for (Word offset = 0xa4; offset != 0xb8; offset += 4)
        write(self, offset, read<Word>(source, offset));
    services.header_assign_005c5470(add(self, 0xb8), add(source, 0xb8));
    services.header_assign_005c5470(add(self, 0x20c), add(source, 0x20c));
    write(self, 0x360, read<std::uint8_t>(source, 0x360));
    for (Word offset = 0x364; offset != 0x424; offset += 4)
        copy_native_float32_a(add(self, offset), add(source, offset));
    write(self, 0x424, read<std::uint8_t>(source, 0x424));
    void* const tree = add(self, 0x428);
    if (tree != add(source, 0x428)) {
        void* const head = read<void*>(tree, 4);
        DialogIterator output;
        const DialogIterator first{tree, read<void*>(head, 0)}, last{tree, head};
        services.erase_tree_00576740(tree, output, first, last);
        services.copy_tree_005c6790(tree, add(source, 0x428));
    }
    return self;
}
namespace {
struct InitBlockCleanup {
    void* block;
    InitOnceServices& services;
    bool armed{true};
    ~InitBlockCleanup() noexcept { if (armed) services.file_block_destroy_00bdcb30(block); }
};
}
// ECX game, stack byte initialize, RET4. Image callbacks are address keys for the
// required bindings; no host callback pointer or raw application wiring is invented.
void game_on_init_once_004dd5b0(void* self, std::uint8_t initialize, InitOnceState& state,
                              NativeStringRawPoolContext& strings, InitOnceServices& services) {
    if (!initialize) {
        services.init_title_004c9a70(self);
        services.preload_cache_008fafc0(services.resource_cache_004dc200());
        return;
    }
    NativeString label;
    construct_native_string_header_0041e870(&label, strings, "GGame::OnInitOnce");
    HeaderCleanup label_cleanup{&label, strings};
    alignas(4) std::uint8_t block[0x1c];
    services.file_block_construct_00be0a30(block, &label, 1);
    InitBlockCleanup block_cleanup{block, services};
    label_cleanup.armed = false;
    destroy_native_string_header_0041dd20(&label, strings);
    services.mission_lua_initialize_00884be0(read<void*>(self, 0x1a08));
    void* const state_pointer = read<void*>(read<void*>(read<void*>(self, 0x1a08), 4), 4);
    services.borrowed_lua_construct_00b65e20(add(self, 0x1a0c), state_pointer);
    write<Word>(self, 0x1a1c, 0x004c6880);
    {
        NativeString script;
        construct_native_string_header_0041e870(&script, strings, "Scripts\\global\\luaMW_init.lua");
        HeaderCleanup script_cleanup{&script, strings};
        services.lua_script_00b69d40(add(self, 0x1a0c), &script, 0);
        script_cleanup.armed = false;
        destroy_native_string_header_0041dd20(&script, strings);
    }
    services.compatibility_008d44c0(state.settings_00f88980, state.compatibility_00f88a30);
    services.configure_input_00a917e0(state.backend_00f8bbf4, 0, 1, nullptr);
    services.configure_input_00a917e0(state.backend_00f8bbf4, 1, 1, nullptr);
    services.configure_input_00a917e0(state.backend_00f8bbf4, 2, 1, nullptr);
    services.pointer_reader_00aa6be0(0x00696430);
    services.keyboard_reader_00a9ac60(0x00696470);
    services.sound_set_initialize_00940ec0(services.sound_queue_004c1b90());
    services.selection_reader_00a9ac80(0x00941250);
    services.button_selection_reader_00aab5e0(0x00941250);
    write<Word>(services.gui_manager_004c12b0(), 4, 0x00696590);
    state.input_callback_00f8bbfc = 0x006965a0;
    services.safe_zone_00aa2110(services.gui_manager_004c12b0(), state.safe_zone_00f88a31);
    if (!read<std::uint8_t>(self, 0x55c)) services.input_configuration_00698a10(add(self, 0x3c));
    services.register_slot10(services.menu_command_00425d10());
    services.register_slot10(services.frontend_frame_004c1ac0());
    services.game_singleton_004c1c50();
    block_cleanup.armed = false;
    services.file_block_destroy_00bdcb30(block);
}

namespace {
// Both original number branches round input and floor result to float32, subtract
// with x87, and accept only ordered zero. Infinity minus infinity stays unordered.
bool profile_number_is_integral(float number, float floor_value) noexcept {
    std::uint8_t result;
    __asm {
        fld dword ptr number
        fsub dword ptr floor_value
        fldz
        fxch st(1)
        fucomip st(0), st(1)
        fstp st(0)
        lahf
        test ah, 44h
        setnp result
    }
    return result != 0;
}
float profile_float_from_double(double value) noexcept {
    float result;
    __asm { fld qword ptr value }
    __asm { fstp dword ptr result }
    return result;
}
struct ProfileLuaCleanup {
    NativeLuaObjectStorage& object;
    bool armed{true};
    ~ProfileLuaCleanup() noexcept { if (armed) destroy_native_lua_object_00b67700(object); }
    void close() { armed=false; destroy_native_lua_object_00b67700(object); }
};
void close_profile_string(HeaderCleanup& cleanup) {
    cleanup.armed=false;
    destroy_native_string_header_0041dd20(cleanup.header,cleanup.strings);
}
void copy_profile_children(void* manager, NativeLuaObjectStorage& destination,
                           NativeLuaObjectStorage& value, ProfileLuaContext& context) {
    NativeLuaObjectStorage key, child;
    construct_native_lua_object_00b65f50(&key);
    ProfileLuaCleanup key_cleanup{key};
    construct_native_lua_object_00b65f50(&child);
    ProfileLuaCleanup child_cleanup{child};
    native_lua_iterate_first_00b67080(value,key,child);
    while (!native_lua_is_unbound_00b66420(key)) {
        copy_profile_lua_value_00436c50(manager,destination,key,child,context);
        native_lua_iterate_next_00b67190(value,key,child);
    }
    child_cleanup.close();
    key_cleanup.close();
}
}
// ECX manager (recursion only), stack destination/key/value, RET0Ch. Unlike the
// older profile projection there is no cycle, finiteness or recursion-depth guard.
void copy_profile_lua_value_00436c50(void* manager, NativeLuaObjectStorage& destination,
    NativeLuaObjectStorage& key, NativeLuaObjectStorage& value, ProfileLuaContext& context) {
    ProfileLuaServices& service=context.services;
    const bool string_key=service.type_00b65f80(key)==4;
    if (!string_key) service.type_00b65f80(key); // Original redundant call is observable.
    const std::int32_t type=service.type_00b65f80(value);
    auto integer=[&](NativeLuaObjectStorage& object) {
        return native_lua_integer_00b66290(object,context.crt_sse2_conversion);
    };
    if (string_key) {
        if (type==1) {
            NativeString name;
            construct_native_string_header_0041e870(&name,context.strings,native_lua_string_00b662b0(key));
            HeaderCleanup cleanup{&name,context.strings};
            const std::uint32_t boolean=native_lua_boolean_00b66250(value);
            native_lua_set_boolean_00b673a0(destination,name,boolean);
            close_profile_string(cleanup);
        } else if (type==3) {
            const float number=native_lua_number_00b66270(value);
            const float floor_value=profile_float_from_double(service.floor_00bf85b0(static_cast<double>(number)));
            const bool integral=profile_number_is_integral(number,floor_value);
            NativeString name;
            construct_native_string_header_0041e870(&name,context.strings,native_lua_string_00b662b0(key));
            HeaderCleanup cleanup{&name,context.strings};
            if (integral) {
                const std::int32_t n=integer(value);
                native_lua_set_integer_00b67460(destination,name,n);
            } else {
                const float n=native_lua_number_00b66270(value);
                native_lua_set_number_field_00b67400(destination,name,n);
            }
            close_profile_string(cleanup);
        } else if (type==4) {
            NativeString text;
            construct_native_string_header_0041e870(&text,context.strings,native_lua_string_00b662b0(value));
            HeaderCleanup text_cleanup{&text,context.strings};
            NativeString name;
            construct_native_string_header_0041e870(&name,context.strings,native_lua_string_00b662b0(key));
            HeaderCleanup name_cleanup{&name,context.strings};
            set_native_lua_string_field_00b674c0(&destination,context.empty_0108ff2c,&name,&text);
            close_profile_string(name_cleanup);
            close_profile_string(text_cleanup);
        } else if (type==5) {
            {
                NativeString name;
                construct_native_string_header_0041e870(&name,context.strings,native_lua_string_00b662b0(key));
                HeaderCleanup cleanup{&name,context.strings};
                native_lua_set_new_table_00b67580(destination,name);
                close_profile_string(cleanup);
            }
            NativeLuaObjectStorage child;
            {
                NativeString name;
                construct_native_string_header_0041e870(&name,context.strings,native_lua_string_00b662b0(key));
                HeaderCleanup cleanup{&name,context.strings};
                native_lua_get_by_string_00b68100(destination,&child,name);
                // Child is armed before name release (native state3 -> release name).
                ProfileLuaCleanup child_cleanup{child};
                close_profile_string(cleanup);
                copy_profile_children(manager,child,value,context);
                child_cleanup.close();
            }
        }
        return;
    }
    if (type==1) {
        const std::uint32_t boolean=native_lua_boolean_00b66250(value);
        const std::int32_t index=integer(key);
        service.boolean_index_00b66530(destination,index,boolean);
    } else if (type==3) {
        const float number=native_lua_number_00b66270(value);
        const float floor_value=profile_float_from_double(service.floor_00bf85b0(static_cast<double>(number)));
        if (profile_number_is_integral(number,floor_value)) {
            const std::int32_t n=integer(value),index=integer(key);
            service.integer_index_00b665d0(destination,index,n);
        } else {
            const float n=native_lua_number_00b66270(value);
            const std::int32_t index=integer(key);
            set_native_lua_numeric_element_00b66580(&destination,nullptr,index,n);
        }
    } else if (type==4) {
        const char* const text=native_lua_string_00b662b0(value);
        const std::int32_t index=integer(key);
        service.string_index_00b66740(destination,index,text);
    } else if (type==5) {
        service.table_index_00b66670(destination,integer(key));
        const std::int32_t index=integer(key);
        NativeLuaObjectStorage child;
        native_lua_get_by_index_00b67720(destination,&child,index);
        ProfileLuaCleanup cleanup{child};
        copy_profile_children(manager,child,value,context);
        cleanup.close();
    }
}

namespace {
#include "bsp/oct18_tail_a_factory.inc"
}
// Original ECX stream -> EAX message, RET. Peek/rewind exactly8 bits before
// allocation/constructor selection. NO synthesized null/unknown-tag return.
// Each constructor cleanup is armed only in the22 original FH3 case states.
void* session_message_from_bitstream_00768530(void* stream, void* default_message_preimage,
                                           SessionFactoryServices& services) {
    std::uint8_t tag;
    services.read_byte_00428c70(add(stream, 4),tag,8);
    services.rewind_00428b80(add(stream, 4),8);
    void* message=default_message_preimage;
    if (tag>=1 && tag<=233) {
        const SessionFactoryRoute& route=session_factory_routes_00768530[tag-1];
        if (route.allocation_bytes) {
            message=nullptr; // Every allocation-null arm explicitly selects zero.
            void* const allocation=services.allocate_00bf681b(route.allocation_bytes);
            if (allocation) {
                if (route.unwind_state>=0) {
                    try {
                        message=services.construct(route,allocation);
                        if (route.vtable_stamp) write(message,0,route.vtable_stamp);
                    } catch (...) {
                        services.free_00bf65ac(allocation);
                        throw;
                    }
                } else {
                    message=services.construct(route,allocation);
                    if (route.vtable_stamp) write(message,0,route.vtable_stamp);
                }
            }
        } else if (route.callee==0x00733720) {
            message=services.direct_getter_00733720();
        } else if (route.callee==0x008e1530) {
            message=services.direct_getter_008e1530();
        }
    }
    const Word vtable=read<Word>(message,0);
    const Word reader=read<Word>(pointer(vtable),8);
    services.read_slot8(message,reader,stream);
    return message;
}

namespace {
bool blast_bounds_a(const float* center, const void* minimum, const void* maximum,
                    float radius_squared, void (__cdecl* target)()) {
    unsigned char inside;
    __asm {
        push maximum
        mov edx, minimum
        mov ecx, center
        call target
        fld radius_squared
        fcomip st(0), st(1)
        fstp st(0)
        seta inside
    }
    return inside != 0;
}
float blast_distance_a(const void* position, const float* center) {
    float delta_x, dy, dz, result;
    __asm {
        mov eax, position
        mov edx, center
        fld dword ptr [eax]
        fsub dword ptr [edx]
        fstp delta_x
        fld dword ptr [eax+4]
        fsub dword ptr [edx+4]
        fstp dy
        fld dword ptr [eax+8]
        fsub dword ptr [edx+8]
        fstp dz
        fld dy
        fld delta_x
        fld dz
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        fstp result
    }
    return result;
}
bool blast_less_a(float distance, float square) {
    unsigned char result;
    __asm {
        fld distance
        fld square
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
Word blast_count_a(const void* definition) {
    const Word begin = read<Word>(definition, 0x60);
    return begin ? Word(std::int32_t(read<Word>(definition,0x64)-begin) >> 3) : 0;
}
}
// ECX center; radius and one ignored stack word, RET8. Raw storage domain.
void blast_sites_00428800(const float* center, float radius, Word, BlastContext& context) {
    auto& s = context.services;
    void* tree = add(read<void*>(context.game_00e188a8,0x21d0),0x28);
    if (!read<Word>(tree,8)) return;
    float square;
    __asm {
        fld radius
        fmul st(0), st(0)
        fstp square
    }
    DialogIterator outer{tree,read<void*>(read<void*>(tree,4),0)};
    for (;;) {
        void* stop = read<void*>(tree,4);
        if (!outer.owner || outer.owner != tree) s.invalid_00bf6713();
        if (outer.node == stop) break;
        if (!outer.owner) s.invalid_00bf6713();
        if (outer.node == read<void*>(outer.owner,4)) s.invalid_00bf6713();
        void* cluster = read<void*>(outer.node,0x10);
        if (blast_bounds_a(center,add(cluster,0x9c),add(cluster,0xa8),square,
                           context.distance_0085bf90)) {
            for (Word list_index = 0; list_index != 3; ++list_index) {
                void* list = add(cluster,0xb4 + 12*list_index);
                void* owner = list;
                void* node = read<void*>(read<void*>(list,4),0);
                auto checked = [&]() {
                    if (node == read<void*>(owner,4)) s.invalid_00bf6713();
                };
                for (;;) {
                    void* end = read<void*>(list,4);
                    if (!owner || owner != list) s.invalid_00bf6713();
                    if (node == end) break;
                    if (!owner) s.invalid_00bf6713();
                    checked();
                    void* entity = read<void*>(node,0xc);
                    if (s.class_slot5c(entity,0x3a)) {
                        checked();
                        if (read<Word>(node,0x34) != 3) {
                            if (!read<std::uint8_t>(entity,0xc8)) {
                                if (void* parent=read<void*>(entity,0x3c)) s.pose_00414db0(parent);
                                const void* matrix = add(entity,0x74);
                                alignas(4) unsigned char temporary[64];
                                if (void* parent=read<void*>(entity,0x3c))
                                    matrix=s.multiply_00413920(add(entity,0x74),temporary,add(parent,0xcc));
                                for (Word offset=0; offset != 64; offset+=4)
                                    copy_native_float32_a(add(entity,0xcc+offset),add(matrix,offset));
                                write<std::uint8_t>(entity,0xc8,1);
                                write<std::uint8_t>(entity,0x10c,0);
                            }
                            const float distance=blast_distance_a(add(entity,0xfc),center);
                            if (blast_less_a(distance,square)) {
                                checked(); write<Word>(node,0x34,3);
                                checked(); write<float>(node,0x38,context.scalar_00ce3930);
                                checked(); s.stop_00480bf0(read<void*>(node,0xc));
                                checked(); const Word count=blast_count_a(read<void*>(node,8));
                                checked(); const Word index=s.random_00bd2fc0() % count;
                                void* definition=read<void*>(node,8);
                                if (!read<Word>(definition,0x60) || index>=blast_count_a(definition))
                                    s.invalid_00bf6713();
                                const void* effect=add(read<void*>(definition,0x60),index*8);
                                checked(); s.play_004b0f80(read<void*>(node,0xc),effect,0,0);
                                checked();
                                if (!read<std::uint8_t>(read<void*>(node,8),0x39)) {
                                    checked(); float argument;
                                    copy_native_float32_a(&argument,add(read<void*>(node,8),0x1c));
                                    s.cluster_00480c20(cluster,argument);
                                }
                            }
                        }
                    }
                    checked(); node=read<void*>(node,0);
                }
            }
        }
        s.tree_next_00427fe0(outer);
    }
}

namespace {
float point_integer_float_a(std::int32_t value) {
    float result;
    __asm {
        cvtsi2ss xmm0, value
        movss result, xmm0
    }
    return result;
}
float point_random_a(PointAnnouncementContext& context,float minimum,float maximum,
                     const std::int32_t* sign=nullptr) {
    auto target=context.random_00bd2f10;
    float result;
    __asm {
        push maximum
        push minimum
        xor ecx, ecx
        call target
        mov eax, sign
        test eax, eax
        jz point_random_store
        fimul dword ptr [eax]
    point_random_store:
        fstp result
    }
    return result;
}
std::int32_t point_count_a(PointAnnouncementContext& context,float minimum,float maximum) {
    auto random=context.random_00bd2f10;
    auto convert=context.truncate_st0_00bf7420;
    std::int32_t result;
    __asm {
        push maximum
        push minimum
        xor ecx, ecx
        call random
        call convert
        mov result, eax
    }
    return result;
}
std::int32_t point_sign_a(float sample, bool first) {
    std::int32_t result=0;
    if (first) {
        __asm {
            fld sample
            fldz
            fcomip st(0), st(1)
            fstp st(0)
            ja point_sign_negative_x87
            xorps xmm0,xmm0
            movss xmm1,sample
            comiss xmm1,xmm0
            ja point_sign_positive_x87
            jmp point_sign_done_x87
        point_sign_negative_x87:
            mov result,-1
            jmp point_sign_done_x87
        point_sign_positive_x87:
            mov result,1
        point_sign_done_x87:
        }
    } else {
        __asm {
            xorps xmm1,xmm1
            movss xmm0,sample
            comiss xmm1,xmm0
            jp point_sign_done_sse
            ja point_sign_negative_sse
            jb point_sign_positive_sse
            jmp point_sign_done_sse
        point_sign_negative_sse:
            mov result,-1
            jmp point_sign_done_sse
        point_sign_positive_sse:
            mov result,1
        point_sign_done_sse:
        }
    }
    return result;
}
void point_direction_a(float yaw,float pitch,float speed,float* output) {
    float temporary, rounded, cosine, sine;
    __asm {
        mov eax,output
        fld yaw
        fsin
        fstp temporary
        fld temporary
        fstp rounded
        fld pitch
        fcos
        fstp cosine
        fld cosine
        fmul rounded
        fmul speed
        fstp dword ptr [eax]
        fld yaw
        fcos
        fstp temporary
        fld temporary
        fstp rounded
        fld pitch
        fcos
        fstp cosine
        fld cosine
        fmul rounded
        fmul speed
        fstp dword ptr [eax+8]
        fld pitch
        fsin
        fstp sine
        fld sine
        fmul speed
        fstp dword ptr [eax+4]
    }
}
void point_add_velocity_a(void* record,const float* velocity) {
    __asm {
        mov eax,record
        mov edx,velocity
        fld dword ptr [edx]
        fadd dword ptr [eax+14h]
        fstp dword ptr [eax+14h]
        fld dword ptr [eax+18h]
        fadd dword ptr [edx+4]
        fstp dword ptr [eax+18h]
        fld dword ptr [eax+1ch]
        fadd dword ptr [edx+8]
        fstp dword ptr [eax+1ch]
    }
}
}
// ECX self, four stacked arguments, EAX self, RET10h. Explicit original stack
// preimage is the 76-byte by-value fill record copied by 00495E20, not zero fill.
void* construct_point_announcement_0049c000(void* self,Word base_argument,
    const float* center,const float* velocity,void* definition,
    const void* preimage,PointAnnouncementContext& context) {
    auto& s=context.services;
    s.base_construct_00496920(self,base_argument);
    bool vector_live=false;
    try {
        write<Word>(self,0,0xce6850);
        void* vector=add(self,0x20);
        write<Word>(vector,4,0); write<Word>(vector,8,0); write<Word>(vector,12,0);
        vector_live=true;
        write<Word>(self,4,2);
        void* table=s.parts_004aed50(definition,1);
        if (!table) return self;
        const auto maximum=point_integer_float_a(read<std::int32_t>(table,12));
        const auto minimum=point_integer_float_a(read<std::int32_t>(table,8));
        const auto count=point_count_a(context,minimum,maximum);
        if (!count) return self;
        s.resize_00495e20(vector,Word(count),preimage);
        for (std::int32_t index=0;index<count;++index) {
            void* entry=s.pick_004ab9a0(table);
            auto current=[&]() -> void* {
                Word begin=read<Word>(vector,4);
                const Word length=begin ? Word(std::int32_t(read<Word>(vector,8)-begin)/0x4c):0;
                if (!begin || Word(index)>=length) s.invalid_00bf6713();
                return add(read<void*>(vector,4),Word(index)*0x4c);
            };
            auto random_field=[&](Word lo,Word hi,const std::int32_t* sign=nullptr) {
                void* type=read<void*>(entry,4);
                float maximum,minimum;
                copy_native_float32_a(&maximum,add(type,hi));
                copy_native_float32_a(&minimum,add(type,lo));
                return point_random_a(context,minimum,maximum,sign);
            };
            const float lifetime=random_field(0x38,0x3c);
            write<float>(current(),0x44,lifetime);
            write<std::uint8_t>(current(),0x48,1);
            write<void*>(current(),4,read<void*>(entry,4));
            write<Word>(current(),0,0);
            void* position=add(current(),8);
            for (Word axis=0;axis!=3;++axis)
                copy_native_float32_a(add(position,axis*4),center+axis);
            float angles[3];
            for (auto& angle:angles) angle=point_random_a(context,
                context.angle_min_00ce684c,context.angle_max_00d7a264);
            std::memcpy(add(current(),0x20),angles,sizeof angles);
            float sign_samples[3];
            for(auto& sample:sign_samples) sample=point_random_a(context,
                context.sign_min_00ce6848,context.sign_max_00ce38b8);
            std::int32_t signs[3];
            for(Word axis=0;axis!=3;++axis) signs[axis]=point_sign_a(sign_samples[axis],axis==0);
            float rotations[3];
            for (Word axis=0;axis!=3;++axis) rotations[axis]=random_field(0x40,0x44,&signs[2-axis]);
            std::memcpy(add(current(),0x2c),rotations,sizeof rotations);
            float sizes[3]; for (auto& value:sizes) value=random_field(0x48,0x4c);
            std::memcpy(add(current(),0x38),sizes,sizeof sizes);
            const float yaw=point_random_a(context,context.angle_min_00ce684c,context.angle_max_00d7a264);
            const float pitch=random_field(0x30,0x34);
            const float speed=random_field(0x28,0x2c);
            float direction[3]; point_direction_a(yaw,pitch,speed,direction);
            write<float>(current(),0x14,direction[0]);
            write<float>(current(),0x18,direction[1]);
            write<float>(current(),0x1c,direction[2]);
            point_add_velocity_a(current(),velocity);
        }
        return self;
    } catch (...) {
        // D8B418: state1 -> C632F8 vector; state0 -> C632F0 base.
        if (vector_live) s.vector_destroy_0048e160(add(self,0x20));
        s.base_destroy_00489dd0(self);
        throw;
    }
}

namespace {
struct FrontendFileCleanupA {
    FrontendShellServices& services;void* block;bool armed=true;
    ~FrontendFileCleanupA() noexcept { if(armed) services.file_destroy_00bdcb30(block); }
};
struct FrontendConfigCleanupA {
    FrontendShellServices& services;void* config;bool armed=true;
    ~FrontendConfigCleanupA() noexcept { if(armed) services.config_destroy_004d2a80(config); }
};
}
// ECX game, RET. Whole raw caller body; required callees and current singleton
// references remain explicit, with no front-end host installation in this lane.
void enter_frontend_shell_004e4000(void* self,const void* config_preimage,
    FrontendShellState& globals,NativeStringRawPoolContext& strings,FrontendShellServices& s) {
    write<Word>(globals.renderer_00f8d394,0x18,0x20000000);
    s.renderer_slot70(globals.renderer_00f8d394,"Textures before mainmenu");
    s.sound_stats_00a7a460(globals.sound_00f8bbd8,"Sounds before mainmenu");
    s.effects_stats_0086b0b0(s.effects_004c1650(),"Effects before mainmenu");
    if(globals.title_00e198c8) {
        s.delete_slot0(globals.title_00e198c8,1); globals.title_00e198c8=nullptr;
    }
    const auto returning=read<std::uint8_t>(self,0x719c);
    if(!returning || !globals.options_00e198b8 || !globals.main_00e198ac) {
        Word label[2];
        construct_native_string_header_0041e870(label,strings,returning ?
            "GILoading::SLM_LOAD_FRONTEND_RETURN":"GILoading::SLM_LOAD_FRONTEND");
        HeaderCleanup label_guard{label,strings};
        alignas(4) unsigned char block[28];
        s.file_construct_00be0a30(block,label,1);
        FrontendFileCleanupA block_guard{s,block};
        label_guard.armed=false; destroy_native_string_header_0041dd20(label,strings);
        alignas(4) unsigned char config[28]; std::memcpy(config,config_preimage,28);
        void* published=s.default_config_0057d0d0(config);
        FrontendConfigCleanupA config_guard{s,config};
        s.publish_config_0057cff0(published);
        config_guard.armed=false; s.config_destroy_004d2a80(config);
        s.loading_begin_0057cb60(0);
        block_guard.armed=false; s.file_destroy_00bdcb30(block);
    }
    float progress; copy_native_float32_a(&progress,&globals.progress_00d7a2f0);
    s.progress_0057bec0(progress);
    s.on_init_004e3aa0(globals.game_00e188a8);
    s.platform_events_004db290(self);
    if(read<Word>(self,0x5d4)!=3) {s.loading_end_0057c250();return;}
    auto ensure_menu=[&](void*& singleton,Word bytes,Word target) {
        if(singleton) return;
        void* allocation=s.allocate_00bf681b(bytes);void* object=nullptr;
        try { if(allocation) object=s.menu_construct(target,allocation); }
        catch(...) {s.free_00bf65ac(allocation);throw;}
        singleton=object; // state is disarmed before slot4; null still reaches it.
        s.menu_slot4(object);
    };
    ensure_menu(globals.options_00e198b8,0x4c,0x689800);
    ensure_menu(globals.main_00e198ac,0x78,0x686170);
    ensure_menu(globals.multiplayer_00e198b4,0x68,0x6887e0);
    if(void* lua=read<void*>(read<void*>(self,0x1a08),4))
        s.lua_run_006b8ad0(lua,"collectgarbage(\"collect\")",0,0,2);
    if(read<Word>(globals.main_00e198ac,4)!=4)
        s.interface_request_004cc460(globals.main_00e198ac,1,0);
    s.menu_slot8(globals.main_00e198ac);
    s.title_music_005884a0(read<void*>(globals.main_00e198ac,0x58));
    write<Word>(self,0x5d4,5);
    if(s.pending_invite_0067d6e0()) s.raise_multiplayer_004bfc70(self);
    bool grant=false;
    {
        Word key[2];construct_native_string_header_0041e870(key,strings,"GA_HM");
        HeaderCleanup guard{key,strings};
        void* tree=add(globals.game_00e188a8,0x6f0);
        void* stop=read<void*>(tree,4);
        DialogIterator output;
        auto* found=s.achievement_find_004c8b80(tree,output,key);
        if(!found->owner || found->owner!=tree) s.invalid_00bf6713();
        if(found->node==stop)
            grant=s.midway_folder_0090c5d0(read<void*>(globals.game_00e188a8,0x21a0));
        guard.armed=false;destroy_native_string_header_0041dd20(key,strings);
    }
    if(grant) {
        Word key[2];construct_native_string_header_0041e870(key,strings,"GA_HM");
        HeaderCleanup guard{key,strings};
        const Word id=s.achievement_id_006b8da0(globals.achievements_00e19900,key);
        guard.armed=false;destroy_native_string_header_0041dd20(key,strings);
        if(id-1u<=0x62u && s.live_enabled_00a3e520(globals.live_00f8abe8) &&
           s.signed_in_004b44f0(globals.live_00f8abe8)) {
            s.grant_00a410a0(globals.live_00f8abe8,id);
            Word award[2];construct_native_string_header_0041e870(award,strings,"GA_HM");
            HeaderCleanup award_guard{award,strings};
            s.record_award_007fbe20(add(globals.game_00e188a8,0x650),award,1);
            award_guard.armed=false;destroy_native_string_header_0041dd20(award,strings);
        }
    }
    s.loading_end_0057c250();
    if(read<std::uint8_t>(self,0x216d)) s.session_mode_0076fad0(add(self,0x1ef0),0);
    write<std::uint8_t>(self,0x216d,0);
}

} // namespace bsp::oct18_tail_a

