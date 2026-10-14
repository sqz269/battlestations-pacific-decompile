// Segments 0 and 9, cc13-segA. See include/bsp/segs_0_9_11_cc13.hpp and docs/CC13_SEGS_0_9_11.md.
// Names are hypotheses. Each routine lists its address, original ABI and any uncertainty.
#include "bsp/segs_0_9_11_cc13.hpp"

namespace bsp::segs_0_9_11_cc13 {
namespace {

GuiLuaVariant name_key(const char* text) noexcept {
    GuiLuaVariant v;
    v.tag = 0;
    v.value.text = text;
    return v;
}
GuiLuaVariant field(std::int32_t type, void* address) noexcept {
    GuiLuaVariant v;
    v.tag = type;
    v.value.pointer = address;
    return v;
}

// The image's key strings. Literal pointers stand in for the image addresses; nothing compares
// them by identity.
constexpr const char* kFreq = "freq";              // 00CE37B0
constexpr const char* kTime = "time";              // 00CE37A8
constexpr const char* kStateIndex = "stateIndex";  // 00CE37B8
constexpr const char* kUndefined = "undefined";    // 00CE37C4

constexpr std::int32_t kFieldInt = 1;    // GuiLuaFieldType::Int
constexpr std::int32_t kFieldFloat = 2;  // GuiLuaFieldType::Float

// Shared body of the two handle destructors.
void destroy_ref(Segs0911Host& host, Raw handle, std::uint32_t own_vtable, std::uint32_t base_vtable,
                 bool writer) {
    handle.at<std::uint32_t>(0) = own_vtable;
    auto* const count = static_cast<std::int32_t*>(handle.ptr(4));
    *count -= 1;
    if (*handle.at<std::int32_t*>(4) == 0) {
        host.free_00bf65ac(handle.ptr(4));
        if (writer)
            host.writer_end_08(handle.ptr(8));
        else
            host.visitor_leave_08(handle.ptr(8));
    }
    handle.at<std::uint32_t>(0) = base_vtable;
}

} // namespace

// ---------------------------------------------------------------------------------------------
// Archive handles
// ---------------------------------------------------------------------------------------------

// 00410630, ECX = handle, RET 0, EH frame 00C5DF68 (state 0 only). The count is freed before the
// visitor pointer is read. The handle's own vtable is re-stamped first, as a destructor does.
void archive_reader_ref_destroy_00410630(Segs0911Host& host, Raw handle) {
    destroy_ref(host, handle, 0x00CE3768u, 0x00CE374Cu, false);
}

// 004107B0, ECX = handle, RET 0, EH frame 00C5DF88. Same body over 00CE3794/00CE3784.
void archive_writer_ref_destroy_004107b0(Segs0911Host& host, Raw handle) {
    destroy_ref(host, handle, 0x00CE3794u, 0x00CE3784u, true);
}

// 004106A0, ECX = handle, RET 8: [handle+8] vtable +4h with the key.
void archive_reader_ref_enter_004106a0(Segs0911Host& host, Raw handle, const GuiLuaVariant& key) {
    host.visitor_enter_04(handle.ptr(8), key);
}

// 004106C0, ECX = handle: tail jump to [handle+8] vtable +8h.
void archive_reader_ref_leave_004106c0(Segs0911Host& host, Raw handle) {
    host.visitor_leave_08(handle.ptr(8));
}

// 00410700, ECX = handle, RET 18h (slot +0Ch, read with a default). Pushes only the first four
// stack words: the default is dropped and the visitor's plain read +10h is called.
bool archive_reader_ref_read_or_default_00410700(Segs0911Host& host, Raw handle,
                                                 const GuiLuaVariant& key,
                                                 const GuiLuaVariant& field_value,
                                                 const GuiLuaVariant& /*fallback, unread*/) {
    return host.visitor_read_10(handle.ptr(8), key, field_value);
}

// 004106D0, ECX = handle, RET 10h (slot +10h): [handle+8] vtable +10h.
bool archive_reader_ref_read_004106d0(Segs0911Host& host, Raw handle, const GuiLuaVariant& key,
                                      const GuiLuaVariant& field_value) {
    return host.visitor_read_10(handle.ptr(8), key, field_value);
}

// 00410730, ECX = handle, RET 8: [handle+8] vtable +14h.
bool archive_reader_ref_has_key_00410730(Segs0911Host& host, Raw handle, const GuiLuaVariant& key) {
    return host.visitor_has_key_14(handle.ptr(8), key);
}

// 00410750, ECX = handle: tail jump to [handle+8] vtable +18h with the caller's stack untouched.
// The argument list is the visitor slot's (EventArchiveReader::enumerate_keys_18, RET 4).
std::uint32_t archive_reader_ref_enumerate_keys_00410750(Segs0911Host& host, Raw handle,
                                                         GuiLuaKeyArray& keys) {
    return host.visitor_enumerate_keys_18(handle.ptr(8), keys);
}

// 00410820, ECX = handle, RET 8: [handle+8] vtable +4h.
void archive_writer_ref_begin_00410820(Segs0911Host& host, Raw handle, const GuiLuaVariant& key) {
    host.writer_begin_04(handle.ptr(8), key);
}

// 00410840, ECX = handle: tail jump to [handle+8] vtable +8h.
void archive_writer_ref_end_00410840(Segs0911Host& host, Raw handle) {
    host.writer_end_08(handle.ptr(8));
}

// 00410850, ECX = handle, RET 10h: [handle+8] vtable +0Ch with both pairs.
void archive_writer_ref_write_00410850(Segs0911Host& host, Raw handle, const GuiLuaVariant& key,
                                       const GuiLuaVariant& value) {
    host.writer_write_0c(handle.ptr(8), key, value);
}

// 004108A0, ECX = self, RET 4. No static or vtable referrer was found; the owning class is unknown.
void attach_and_call_slot4_004108a0(Segs0911Host& host, Raw self, void* object) {
    self.ptr(4) = object;
    host.call_vslot_04(object);
}

// 00410D80, ECX = pair, stack handle (12 bytes) RET 0Ch, EH frame 00C5DFA8. Both reads dispatch
// through the handle's vtable slot +10h with ECX = the stack handle (normally 004106D0).
void time_freq_pair_read_00410d80(Segs0911Host& host, float* pair, Raw handle) {
    host.visitor_read_10(handle.p, name_key(kFreq), field(kFieldFloat, &pair[0]));
    host.visitor_read_10(handle.p, name_key(kTime), field(kFieldFloat, &pair[1]));
    archive_reader_ref_destroy_00410630(host, handle);
}

// 00410E00, ECX = pair, stack handle RET 0Ch, EH frame 00C5DFC8. The value word of each pair is
// the float's bit pattern (MOVSS to a stack slot, then a dword push); slot +0Ch of the handle.
void time_freq_pair_write_00410e00(Segs0911Host& host, const float* pair, Raw handle) {
    GuiLuaVariant value;
    value.tag = kFieldFloat;
    value.value.number = pair[0];
    host.writer_write_0c(handle.p, name_key(kFreq), value);
    value.value.number = pair[1];
    host.writer_write_0c(handle.p, name_key(kTime), value);
    archive_writer_ref_destroy_004107b0(host, handle);
}

// ---------------------------------------------------------------------------------------------
// Bot-state registry
// ---------------------------------------------------------------------------------------------

// 00411170, ECX = registry, stack machine, reader RET 8. The Int target is the dword PUSH ECX
// reserved, so it starts as the registry's address; a reader that leaves it unwritten gives an
// index the range test rejects. The call is reader vtable +10h ({0, "stateIndex"}, {1, &index}).
// Out-of-range indexes report through 00BF6713 and the read continues, as in the image.
void bot_state_registry_read_current_00411170(Segs0911Host& host,
                                              NativeBotStateRegistryStorage& registry,
                                              Raw machine, void* reader) {
    auto index = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&registry));
    host.visitor_read_10(reader, name_key(kStateIndex), field(kFieldInt, &index));
    NativeBotStatePair* const begin = registry.vector_04.begin_04;
    if (begin == nullptr ||
        index >= static_cast<std::uint32_t>((registry.vector_04.end_08 - begin)))
        host.invalid_parameter_00bf6713();
    void* const state = registry.vector_04.begin_04[index].state_04;
    machine.ptr(4) = state;
    host.call_vslot_04(state);
}

// 00411350, ECX = registry, stack state RET 4. A checked-iterator walk of the whole vector. The
// match branch (00411391..004113A0) only repeats the iterator-compatibility test and rejoins the
// loop; the single exit (004113B6) is OR EAX,-1. So the result is -1 for every input, and
// 00411450 always saves -1. The registry-equals-itself tests (CMP ESI,ESI) are omitted: they
// cannot report.
std::int32_t bot_state_registry_index_of_00411350(Segs0911Host& host,
                                                  NativeBotStateRegistryStorage& registry,
                                                  const void* state) {
    NativeBotStateVectorStorage& v = registry.vector_04;
    NativeBotStatePair* it = v.begin_04;
    if (it > v.end_08) host.invalid_parameter_00bf6713();
    for (;;) {
        NativeBotStatePair* const end = v.end_08;
        if (v.begin_04 > end) host.invalid_parameter_00bf6713();
        if (it == end) break;
        if (it >= v.end_08) host.invalid_parameter_00bf6713();
        if (it->state_04 == state) {
            if (v.begin_04 > v.end_08) host.invalid_parameter_00bf6713();
        }
        if (it >= v.end_08) host.invalid_parameter_00bf6713();
        ++it;
    }
    return -1;
}

// 004113C0, ECX = registry, stack state RET 4. The same checked walk; the first match returns
// its name, the end returns 0.
const char* bot_state_registry_find_name_004113c0(Segs0911Host& host,
                                                  NativeBotStateRegistryStorage& registry,
                                                  const void* state) {
    NativeBotStateVectorStorage& v = registry.vector_04;
    NativeBotStatePair* it = v.begin_04;
    if (it > v.end_08) host.invalid_parameter_00bf6713();
    for (;;) {
        NativeBotStatePair* const end = v.end_08;
        if (v.begin_04 > end) host.invalid_parameter_00bf6713();
        if (it == end) return nullptr;
        if (it >= v.end_08) host.invalid_parameter_00bf6713();
        if (it->state_04 == state) {
            if (it >= v.end_08) host.invalid_parameter_00bf6713();
            return it->name_00;
        }
        if (it >= v.end_08) host.invalid_parameter_00bf6713();
        ++it;
    }
}

// 00411430, ECX = registry passed through, stack state RET 4.
const char* bot_state_registry_name_or_undefined_00411430(Segs0911Host& host,
                                                         NativeBotStateRegistryStorage& registry,
                                                         const void* state) {
    const char* const name = bot_state_registry_find_name_004113c0(host, registry, state);
    return name != nullptr ? name : kUndefined;
}

// 00411450, ECX = registry, stack machine, writer RET 8: writer vtable +0Ch with
// ({0, "stateIndex"}, {1, 00411350(machine+4)}).
void bot_state_registry_write_current_00411450(Segs0911Host& host,
                                               NativeBotStateRegistryStorage& registry,
                                               Raw machine, void* writer) {
    const std::int32_t index =
        bot_state_registry_index_of_00411350(host, registry, machine.ptr(4));
    GuiLuaVariant value;
    value.tag = kFieldInt;
    value.value.integer = index;
    host.writer_write_0c(writer, name_key(kStateIndex), value);
}

// ---------------------------------------------------------------------------------------------
// Bot-state pair STL thunks
// ---------------------------------------------------------------------------------------------

// 004109C0: MOV ECX,[ESP+4]; XOR EDX,EDX; CALL 00410400; RET 4.
NativeBotStatePair* allocate_pairs_thunk_004109c0(std::uint32_t count) {
    return allocate_native_bot_state_pairs_00410400(count);
}

// 00411140 / 004115E0: PUSH ECX (tag slot, byte 0), then 00410C00(ECX, EDX, [dest, unused,
// unused, tag]); POP ECX; RET 8. 00410C00 reads only its first stack word.
NativeBotStatePair* copy_pairs_dispatch_00411140(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest) {
    return copy_native_bot_state_pairs_00410c00(first, last, dest);
}
NativeBotStatePair* copy_pairs_dispatch_004115e0(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest) {
    return copy_native_bot_state_pairs_00410c00(first, last, dest);
}
// 00411320: byte-identical to 00411140 except RET 10h.
NativeBotStatePair* copy_pairs_dispatch_00411320(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest) {
    return copy_native_bot_state_pairs_00410c00(first, last, dest);
}

// 004115B0: 00410C00(ECX = [ESP+8] first.ptr, EDX = [ESP+10h] last.ptr, [dest = incoming ECX, ...]).
NativeBotStatePair* copy_pairs_dispatch_004115b0(NativeBotStatePair* dest,
                                                 const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last) {
    return copy_native_bot_state_pairs_00410c00(first, last, dest);
}

// 00411640: 00410C00(ECX = [ESP+8] first.ptr, EDX = [ESP+10h] last.ptr, [dest = [ESP+14h], ...]).
// The incoming ECX is pushed as an unused word.
NativeBotStatePair* copy_pairs_dispatch_00411640(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest) {
    return copy_native_bot_state_pairs_00410c00(first, last, dest);
}

// 004112B0: 00410FE0(ECX dest, EDX count, [pair*, unused, unused, tag]); RET 8.
void fill_pairs_dispatch_004112b0(NativeBotStatePair* dest, std::uint32_t count,
                                  const NativeBotStatePair& pair) {
    fill_native_bot_state_pairs_00410fe0(dest, count, pair);
}

// 00410A30. No callers (an unreferenced instantiation, same body as 004104F0).
void construct_pair_at_00410a30(NativeBotStatePair* dest, const NativeBotStatePair* src) {
    if (dest != nullptr) {
        dest->name_00 = src->name_00;
        dest->state_04 = src->state_04;
    }
}

namespace {
NativeBotStatePair* copy_backward(const NativeBotStatePair* first, const NativeBotStatePair* last,
                                  NativeBotStatePair* dest_end) {
    NativeBotStatePair* const result = dest_end - (last - first);
    const std::ptrdiff_t delta = dest_end - last;
    while (last != first) {
        --last;
        NativeBotStatePair* const out = const_cast<NativeBotStatePair*>(last) + delta;
        out->name_00 = last->name_00;
        out->state_04 = last->state_04;
    }
    return result;
}
} // namespace

// 00410B50 and 00411010 (byte-identical, no callers). The result is computed before the copy.
NativeBotStatePair* copy_pairs_backward_00410b50(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest_end) {
    return copy_backward(first, last, dest_end);
}
NativeBotStatePair* copy_pairs_backward_00411010(const NativeBotStatePair* first,
                                                 const NativeBotStatePair* last,
                                                 NativeBotStatePair* dest_end) {
    return copy_backward(first, last, dest_end);
}

// 00411D30, ECX = vector, RET 4, returns AL. The three pointers are zeroed before the n test.
// If 00411860 returned (it throws in the image) the allocation would follow, as the code falls
// through to it.
bool vector_buy_00411d30(Segs0911Host& host, NativeBotStateVectorStorage& vector, std::uint32_t n) {
    vector.begin_04 = nullptr;
    vector.end_08 = nullptr;
    vector.capacity_end_0c = nullptr;
    if (n == 0) return false;
    if (n > 0x1FFFFFFFu) host.vector_length_error_00411860();
    NativeBotStatePair* const block = allocate_native_bot_state_pairs_00410400(n);
    vector.begin_04 = block;
    vector.end_08 = block;
    vector.capacity_end_0c = block + n;
    return true;
}

// 004114A0, ECX = vector. No rel32 or absolute-dword referrer (dead or vtable-free helper).
void pointer_vector_tidy_004114a0(Segs0911Host& host, Raw vector) {
    if (vector.ptr(4) != nullptr) host.free_00bf65ac(vector.ptr(4));
    vector.at<std::uint32_t>(4) = 0;
    vector.at<std::uint32_t>(8) = 0;
    vector.at<std::uint32_t>(0xC) = 0;
}

// 00411760, ECX = storage, stack message RET 4, returns this.
NativeLegacyExceptionStorage& construct_length_error_00411760(
    NativeLegacyExceptionStorage& storage, const NativeLegacySboStringStorage& message) {
    construct_native_legacy_logic_error_00411700(storage, message);
    storage.native_vtable_00 = 0x00D69260u;  // std::length_error::vftable
    return storage;
}

// ---------------------------------------------------------------------------------------------
// Leaves
// ---------------------------------------------------------------------------------------------

// 00410AF0, ECX = page, RET 4: the dword at +10h of each of the 40h slots (stride 14h). No callers.
void node_page_fill_slot_field_10_00410af0(Raw page, std::uint32_t value) {
    for (std::size_t i = 0; i < 0x40; ++i) page.at<std::uint32_t>(0x10 + i * 0x14) = value;
}

// 00410B10 / 00410B20, ECX = page: word +580h == 40h / == 0. No callers.
bool node_page_count_is_64_00410b10(Raw page) { return page.at<std::int16_t>(0x580) == 0x40; }
bool node_page_count_is_zero_00410b20(Raw page) { return page.at<std::int16_t>(0x580) == 0; }

// 00411EB0, ECX = guard, stack owner RET 4, returns guard. Vtable first, then owner, then (only for
// a non-null owner) EnterCriticalSection and ++depth.
void* scoped_lock_construct_00411eb0(Segs0911Host& host, Raw guard, void* owner) {
    guard.at<std::uint32_t>(0) = 0x00CE37FCu;
    guard.ptr(4) = owner;
    if (owner != nullptr) {
        host.enter_critical_section(owner);
        raw(owner).at<std::int32_t>(0x18) += 1;
    }
    return guard.p;
}

// 00411F00, ECX = guard, stack flags RET 4, returns guard: --depth and LeaveCriticalSection for a
// non-null owner (depth first), then _free(guard) when flags & 1.
void* scoped_lock_scalar_delete_00411f00(Segs0911Host& host, Raw guard, std::uint32_t flags) {
    void* const owner = guard.ptr(4);
    guard.at<std::uint32_t>(0) = 0x00CE37FCu;
    if (owner != nullptr) {
        raw(owner).at<std::int32_t>(0x18) -= 1;
        host.leave_critical_section(owner);
    }
    if (flags & 1u) host.free_00bf65ac(guard.p);
    return guard.p;
}

// 00411F40, ECX = record. 00D7A24C = 1.0f, 00CE3804 = 1000.0f, 00CE3800 = 0.5f. No callers.
void float_record5_defaults_00411f40(float* record) {
    record[0] = 1.0f;
    record[1] = 1.0f;
    record[3] = 1.0f;
    record[2] = 1000.0f;
    record[4] = 0.5f;
}

// 00411F80, ECX = record. 00CE3814 = 1.2f (3F99999Ah), 00CE3810 = 2000000.0f, 00CE380C = 1.5f,
// 00CE3808 = 150.0f, in that field order. No callers.
void float_record4_defaults_00411f80(float* record) {
    record[0] = 1.2f;
    record[1] = 2000000.0f;
    record[2] = 1.5f;
    record[3] = 150.0f;
}

// 00411FF0, ECX = self, stack float RET 4. FUCOMIP then LAHF/TEST AH,44h/JP: 1 only for ordered
// equality (a NaN on either side gives 0).
std::int32_t float_equals_00411ff0(const float* self, float value) {
    return *self == value ? 1 : 0;
}

// 00412270, ECX = self: NEG AL/SBB/AND gives 00E17628 when byte +8 is non-zero, else 0.
void* flagged_global_if_set_00412270(Segs0911Host& host, Raw self) {
    return self.at<std::uint8_t>(8) != 0 ? host.global(0x00E17628u) : nullptr;
}

// 004122A0, ECX = iterator: *it = (*it)->next (+4).
void list_iterator_next_004122a0(void** iterator) {
    *iterator = raw(*iterator).ptr(4);
}

// 00412500, ECX = text: the inlined strlen loop.
std::int32_t c_string_length_00412500(const char* text) {
    const char* const start = text + 1;
    char c;
    do {
        c = *text++;
    } while (c != '\0');
    return static_cast<std::int32_t>(text - start);
}

// ---------------------------------------------------------------------------------------------
// Segment 9
// ---------------------------------------------------------------------------------------------

// 004BC410, ECX = game, stack mover RET 4. game+1ED4h is the current camera mover.
void game_set_camera_mover_004bc410(Segs0911Host& host, Raw game, void* mover) {
    void* const current = game.ptr(0x1ED4);
    if (mover == current) return;
    if (mover != nullptr) host.mover_link_previous_124(mover, current);
    if (game.ptr(0x1ED4) != nullptr) {
        host.mission_entity_kill_00926d90(game.ptr(0x1ED4), 2);
        game.ptr(0x1ED4) = nullptr;
    }
    game.ptr(0x1ED4) = mover;
    if (mover != nullptr) {
        host.scene_node_enable_00922f30(mover, 0);
        host.camera_mover_set_camera_node_0042a920(game.ptr(0x1ED4), game.ptr(0x19FC));
        host.weather_apply_camera_shake_00445790(game.ptr(0x21C4), game.ptr(0x1ED4));
        return;
    }
    if (*static_cast<std::uint32_t*>(host.global(0x00E198C4u)) != 0)
        *static_cast<std::uint8_t*>(host.global(0x00E19894u)) = 0;
}

// 004BC890, ECX = game, stack mode, forced (byte) RET 8. With a session (+1FE4h) and a scene record
// (+5FCh), each of the eight views i gets one value at record+4h+i*120h, [game+18CCh+i*4]+28h and
// game+1030h+i*118h: record+98Ch for mode 7, otherwise 0 for views 0..3 and 1 for views 4..7.
void game_set_game_mode_004bc890(Raw game, std::int32_t mode, std::uint8_t forced) {
    game.at<std::uint8_t>(0x61C) = forced;
    game.at<std::int32_t>(0x618) = mode;
    game.at<std::int32_t>(0x614) = mode;
    if (game.at<std::uint32_t>(0x1FE4) == 0 || game.ptr(0x5FC) == nullptr) return;
    auto store = [&](std::size_t view, std::uint32_t value) {
        raw(game.ptr(0x5FC)).at<std::uint32_t>(4 + view * 0x120) = value;
        raw(game.ptr(0x18CC + view * 4)).at<std::uint32_t>(0x28) = value;
        game.at<std::uint32_t>(0x1030 + view * 0x118) = value;
    };
    if (mode == 7) {
        const std::uint32_t value = raw(game.ptr(0x5FC)).at<std::uint32_t>(0x98C);
        for (std::size_t view = 0; view < 8; ++view) store(view, value);
        return;
    }
    for (std::size_t view = 0; view < 4; ++view) store(view, 0);
    for (std::size_t view = 4; view < 8; ++view) store(view, 1);
}

// 004BF9A0, ECX = self, returns self, EH frame 00C64D88. The vtable 00CE754C is stamped and byte
// +44h cleared before 006F7670 (which tests +44h to run luaMW_init.lua once). The singleton
// creator 004C1D10 calls it for the 50h-byte object at 00E19A64.
void* game_singleton_construct_base_004bf9a0(Segs0911Host& host, Raw self) {
    self.at<std::uint32_t>(0) = 0x00CE754Cu;
    self.at<std::uint8_t>(0x44) = 0;
    host.lua_mw_run_init_script_once_006f7670(self.p);
    return self.p;
}

// 004C2C80, ECX = list {count +0, head +4, tail +8}, stack payload RET 4. Node {prev +0, next +4,
// payload +8} from operator new(0Ch); no null test after the zeroing branch (a failed allocation
// writes through null, as in the image).
void scene_record_list_push_back_004c2c80(Segs0911Host& host, Raw list, void* payload) {
    void* const block = host.operator_new_00bf681b(0xC);
    Raw node = raw(block);
    if (block != nullptr) {
        node.at<std::uint32_t>(0) = 0;
        node.at<std::uint32_t>(4) = 0;
        node.at<std::uint32_t>(8) = 0;
    }
    node.ptr(8) = payload;
    node.ptr(0) = list.ptr(8);
    if (list.at<std::uint32_t>(0) != 0) {
        raw(list.ptr(8)).ptr(4) = node.p;
        list.ptr(8) = node.p;
    } else {
        list.ptr(4) = node.p;
        list.ptr(8) = node.p;
    }
    node.ptr(4) = nullptr;
    list.at<std::uint32_t>(0) += 1;
}

// 004C3C90, ECX = game: 004BFDF0(game) then byte +193Ch = 0.
void game_reset_local_player_units_004c3c90(Segs0911Host& host, Raw game) {
    host.clear_all_unit_lists_004bfdf0(game.p);
    game.at<std::uint8_t>(0x193C) = 0;
}

// 004C60E0, ECX = message, stack cursor RET 4 (a vtable slot; pairs with 004D4BB0). Every write
// is __thiscall(cursor, ...). The list is a std::list at +1Ch whose head node is [+20h]; nodes
// carry a qword at +10h and dwords/bool at +18h..+38h. The checked-iterator tests are kept: the
// container test (iterator container null or not this list) can never report here; the node ==
// head tests report through 00BF6713 and the write continues.
void session_message_e4_write_004c60e0(Segs0911Host& host, Raw message, void* cursor) {
    host.bit_write_byte_00428ff0(cursor, message.at<std::uint8_t>(0x10), 8);
    host.bit_write_byte_00428ff0(cursor, message.at<std::uint8_t>(0x18), 3);
    if (message.at<std::uint8_t>(0x18) != 3) {
        host.bit_write_byte_00428ff0(cursor, message.at<std::uint8_t>(0x19), 4);
        return;
    }
    host.bit_write_byte_00428ff0(cursor, message.at<std::uint8_t>(0x24), 8);
    Raw node = raw(raw(message.ptr(0x20)).ptr(0));
    auto check = [&] {
        if (node.p == message.ptr(0x20)) host.invalid_parameter_00bf6713();
    };
    for (;;) {
        if (node.p == message.ptr(0x20)) return;  // EBP = [list+4], reloaded at the loop top
        check();
        host.bit_write_qword_00429180(cursor, node.at<std::uint32_t>(0x10),
                                      node.at<std::uint32_t>(0x14));
        check();
        host.bit_write_signed_dword_00429090(cursor, node.at<std::uint32_t>(0x18), 0x20);
        check();
        host.bit_write_signed_dword_00429090(cursor, node.at<std::uint32_t>(0x1C), 0x20);
        check();
        host.bit_write_bool_004290b0(cursor, node.at<std::uint8_t>(0x20));
        for (std::size_t off = 0x24; off <= 0x38; off += 4) {
            check();
            host.bit_write_signed_dword_00429090(cursor, node.at<std::uint32_t>(off), 0x20);
        }
        check();
        node = raw(node.ptr(0));
    }
}

namespace {
// The shared body of 004C62A0 and 004C63E0 (byte-identical apart from the EH handler).
void* construct_texture_layer(Segs0911Host& host, Raw layer) {
    auto* const name = &layer.at<NativeStr>(8);
    name->length = 0;
    name->data = nullptr;
    layer.at<std::int32_t>(0) = 500;
    layer.at<float>(4) = 1.0f;  // 00D7A24C
    host.native_string_resize_0041dd40(name, 9, 0);
    if (name->data != nullptr) host.memcpy_00bf7680(name->data, "white.dds", name->length);  // 00CE761C
    for (std::size_t off = 0x10; off <= 0x1C; off += 4) layer.at<float>(off) = 1.0f;
    for (std::size_t off = 0x20; off <= 0x9C; off += 4) layer.at<std::uint32_t>(off) = 0;
    return layer.p;
}
} // namespace

// 004C62A0, ECX = layer, returns layer, EH frame 00C6517B. Callers 004DA2A0 and 00BA43B0.
void* scene_texture_layer_construct_a_004c62a0(Segs0911Host& host, Raw layer) {
    return construct_texture_layer(host, layer);
}
// 004C63E0, ECX = layer, returns layer, EH frame 00C6519B. Callers 004DA2A0 and 00BA3A60.
void* scene_texture_layer_construct_b_004c63e0(Segs0911Host& host, Raw layer) {
    return construct_texture_layer(host, layer);
}

// 004C6520, ECX = header, EH frame 00C6524F (states 0Bh..0). Releases thirteen native strings
// {length, data} in reverse member order; each data block goes back to the sized pool with
// length + 1. The data pointers are not cleared.
void scene_record_header_destruct_004c6520(Segs0911Host& host, Raw header) {
    static constexpr std::size_t kLengthOffsets[] = {0x280, 0x254, 0x24C, 0x244, 0x23C, 0x234, 0x218,
                                                     0x210, 0x208, 0x13C, 0x044, 0x024, 0x01C};
    for (const std::size_t off : kLengthOffsets) {
        void* const data = header.ptr(off + 4);
        if (data == nullptr) continue;
        const std::uint32_t size = header.at<std::uint32_t>(off) + 1;
        void* const pool = host.pool_singleton_00419cc0();
        host.pool_return_block_00bd1510(pool, data, size, 1);
    }
}

// 004C6B20, ECX = game, stack float RET 4. The cutscene player is game+38h. The prior doc read
// "00425D10(0)": the pushed 0 is 00530630's argument (00425D10 is __cdecl(void)), so the menu
// command screen's byte +3Ch is cleared.
void game_update_cutscene_playback_004c6b20(Segs0911Host& host, Raw game, float delta) {
    void* player = game.ptr(0x38);
    if (game.at<std::uint8_t>(0x634) != 0 && game.at<std::uint8_t>(0x635) == 0) return;
    if (player == nullptr) return;
    host.screen_set_byte_3c_00530630(host.menu_command_screen_00425d10(), 0);
    if (raw(player).at<std::int32_t>(0x60) == 4) return;
    if (host.input_action_was_pressed_004c43c0(game.p, 3)) {
        host.engine_movie_stop_0045f600(player);
        host.cutscene_player_ptr_delete_004bea40(&player);  // a stack copy of the pointer
        game.ptr(0x38) = nullptr;
        return;
    }
    host.engine_movie_advance_0045f970(player, delta);
}

// 004C6BE0, ECX = game, stack a, b, c, float d RET 10h. ECX for 0046DC10 is [00E18680].
void* game_generate_object_004c6be0(Segs0911Host& host, Raw game, std::uint32_t a,
                                    std::uint32_t b, std::uint32_t c, float d) {
    void* const scene_db = *static_cast<void**>(host.global(0x00E18680u));
    void* const entity = host.scene_create_hidden_entity_0046dc10(scene_db, a, b, c, d);
    if (game.at<std::uint32_t>(0x1FE4) != 0) host.game_assign_party_player_slots_004c3840(game.p, 0);
    return entity;
}

// 004C7150, ECX = world, stack edge, index, side RET 0Ch. list = world+7134h+edge*0Ch, head node
// [list+4]. Walks the whole list (no break) and stores side at [node+8]+4 for the index-th node.
// The list-equals-itself test (CMP EDI,EDI) is omitted: it cannot report.
void world_set_border_zone_side_004c7150(Segs0911Host& host, Raw world, std::int32_t edge,
                                         std::int32_t index, std::uint32_t side) {
    Raw list = raw(world.p + 0x7134 + edge * 0xC);
    Raw node = raw(raw(list.ptr(4)).ptr(0));
    for (std::int32_t i = 0;; ++i) {
        if (node.p == list.ptr(4)) return;
        if (i == index) {
            if (node.p == list.ptr(4)) host.invalid_parameter_00bf6713();
            raw(node.ptr(8)).at<std::uint32_t>(4) = side;
        }
        if (node.p == list.ptr(4)) host.invalid_parameter_00bf6713();
        node = raw(node.ptr(0));
    }
}

} // namespace bsp::segs_0_9_11_cc13
