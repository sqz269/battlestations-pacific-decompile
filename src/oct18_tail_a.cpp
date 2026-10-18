#include "bsp/oct18_tail_a.hpp"
#include <cstring>
#include "bsp/native_string.hpp"
#include "bsp/entity_mission_seg60_argon.hpp"
#include "bsp/native_scene_dword_erase_range.hpp"
#include "bsp/cc13_big53.hpp"

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

} // namespace bsp::oct18_tail_a
