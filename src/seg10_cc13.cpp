#include "bsp/seg10_cc13.hpp"

#include <initializer_list>

namespace bsp::cc13_seg10 {

namespace {

void release_native_string(Seg10Host& host, Object string) {
    void* data = native_at<void*>(string, 4);
    if (data == nullptr) return;
    const std::uint32_t bytes = native_at<std::uint32_t>(string, 0) + 1;
    Object pool = host.sized_pool_singleton_00419cc0(data, bytes, 1);
    host.sized_pool_return_block_00bd1510(pool, data, bytes, 1);
}

// The double-checked singleton shared by 004D6CE0, 004DC200 and 004E1CA0
// (listings identical apart from the slot, size and constructor): with the
// slot null, section = [00415350()]+10h; a non-null section is entered
// (import 00CE2218) and its counter +18h incremented; if the slot is still
// null, new(size) and the constructor (null on a failed allocation) fill
// it and 00BD0C30(ECX 00415350(), instance) registers it; then the counter
// is decremented and the section left (00CE2210). Returns the slot.
template <class Construct>
Object lazy_singleton(Seg10Host& host, std::uint32_t slot, std::uint32_t size, Construct construct) {
    if (host.global_slot(slot) != nullptr) return host.global_slot(slot);
    Object section = native_at<Object>(host.singleton_manager_00415350(), 0x10);
    if (section != nullptr) {
        host.enter_critical_section_import_00ce2218(section);
        native_at<std::int32_t>(section, 0x18) += 1;
    }
    if (host.global_slot(slot) == nullptr) {
        Object memory = host.operator_new_00bf681b(size);
        host.set_global_slot(slot, memory != nullptr ? construct(memory) : nullptr);
        host.singleton_register_00bd0c30(host.singleton_manager_00415350(), host.global_slot(slot));
    }
    if (section != nullptr) {
        native_at<std::int32_t>(section, 0x18) -= 1;
        host.leave_critical_section_import_00ce2210(section);
    }
    return host.global_slot(slot);
}

// A tree head from the buy-head helper: the _Isnil byte at `isnil` = 1, the
// three links point at the head, size (+8 of the tree object) = 0.
void init_tree(Object tree, Object head, std::uint32_t isnil) {
    native_at<Object>(tree, 0x04) = head;
    native_at<std::uint8_t>(head, isnil) = 1;
    native_at<Object>(native_at<Object>(tree, 0x04), 0x04) = native_at<Object>(tree, 0x04);
    native_at<Object>(native_at<Object>(tree, 0x04), 0x00) = native_at<Object>(tree, 0x04);
    native_at<Object>(native_at<Object>(tree, 0x04), 0x08) = native_at<Object>(tree, 0x04);
    native_at<std::uint32_t>(tree, 0x08) = 0;
}

// Erase from a world list: the first node of the chain at [entity+30h]+head
// (next +4) whose +8 is the entity goes to 004837D0(list, node).
void unregister_from(Seg10Host& host, Object entity, std::uint32_t list_offset) {
    host.game_entity_unregister_base_00928570(entity);
    Object world = native_at<Object>(entity, 0x30);
    for (Object node = native_at<Object>(world, list_offset + 4); node != nullptr; node = native_at<Object>(node, 0x04)) {
        if (native_at<Object>(node, 0x08) == entity) {
            host.unit_list_erase_node_004837d0(world + list_offset, node);
            return;
        }
    }
}

void register_in(Seg10Host& host, Object entity, std::uint32_t list_offset) {
    host.game_entity_register_in_parent_00928560(entity);
    host.unit_list_push_back_00484540(native_at<Object>(entity, 0x30) + list_offset, entity);
}

template <std::uint32_t Element>
void shared_array_reserve(Seg10Host& host, Object holder, std::int32_t capacity) {
    Object block = native_at<Object>(holder, 0);
    const std::int32_t current = block != nullptr ? native_at<std::int32_t>(block, 0x08) : 0;
    if (capacity <= current) {
        native_at<std::int32_t>(block, 0x08) = capacity;  // a null block with capacity <= 0 writes to address 8
        return;
    }
    Object fresh = host.operator_new_00bf55be(static_cast<std::uint32_t>(capacity) * Element + 0x18);
    Object aligned = reinterpret_cast<Object>(reinterpret_cast<std::uintptr_t>(fresh + 0x18) & ~std::uintptr_t{0xF});
    Object old = native_at<Object>(holder, 0);
    if (old != nullptr && native_at<std::int32_t>(old, 0x08) != 0) {
        host.crt_memcpy_00bf7680(aligned, native_at<void*>(old, 0x00),
                                 static_cast<std::uint32_t>(native_at<std::int32_t>(old, 0x08)) * Element);
    }
    native_at<Object>(fresh, 0x00) = aligned;
    old = native_at<Object>(holder, 0);
    native_at<std::uint32_t>(fresh, 0x04) = old != nullptr ? native_at<std::uint32_t>(old, 0x04) : 0;
    native_at<std::int32_t>(fresh, 0x08) = capacity;
    host.crt_free_00bf6989(native_at<void*>(holder, 0));
    native_at<Object>(holder, 0) = fresh;
}

void push_level(Seg10Host& host, Object game, const std::int32_t* ids, std::int32_t level) {
    alignas(4) std::uint8_t vector[0x10];
    native_at<std::uint32_t>(vector, 0x04) = 0;
    native_at<std::uint32_t>(vector, 0x08) = 0;
    native_at<std::uint32_t>(vector, 0x0C) = 0;
    std::int32_t value = ids[0];
    if (value != 0) {
        const std::int32_t* next = ids;
        do {
            host.vector_push_back_004d6840(vector, &value);
            value = *++next;
        } while (value != 0);
    }
    input_contexts_set_level_set_004d6410(host, game, level, vector);
    if (native_at<void*>(vector, 0x04) != nullptr) host.crt_free_00bf65ac(native_at<void*>(vector, 0x04));
}

} // namespace

// ---------------------------------------------------------------------------
// Container helpers.
// ---------------------------------------------------------------------------

// 004D1720 / 004D38E0 (byte-identical), __fastcall(list): 004CF3F0, free the
// head (+4), +4 = 0.
void small_string_node_list_tidy_004d1720(Seg10Host& host, Object list) {
    host.small_string_list_clear_004cf3f0(list);
    host.crt_free_00bf65ac(native_at<void*>(list, 0x04));
    native_at<Object>(list, 0x04) = nullptr;
}
void small_string_node_list_tidy_004d38e0(Seg10Host& host, Object list) {
    small_string_node_list_tidy_004d1720(host, list);
}

// 004D1890 / 004D3A30 (byte-identical), __fastcall(vector): with storage,
// 004CC760(ECX first, EDX last, vector, vector) and free it; +4..+0Ch = 0.
void reference_pointer_vector_tidy_004d1890(Seg10Host& host, Object vector) {
    Object first = native_at<Object>(vector, 0x04);
    if (first != nullptr) {
        host.reference_range_release_004cc760(first, native_at<Object>(vector, 0x08), vector, vector);
        host.crt_free_00bf65ac(native_at<void*>(vector, 0x04));
    }
    native_at<std::uint32_t>(vector, 0x04) = 0;
    native_at<std::uint32_t>(vector, 0x08) = 0;
    native_at<std::uint32_t>(vector, 0x0C) = 0;
}
void reference_pointer_vector_tidy_004d3a30(Seg10Host& host, Object vector) {
    reference_pointer_vector_tidy_004d1890(host, vector);
}

// 004D28A0 __fastcall(object): 004D05E0 on the list +8, free its head
// (+0Ch) and clear it, then the NativeString +0 goes back to the pool.
void pooled_string_list_destruct_004d28a0(Seg10Host& host, Object object) {
    host.pooled_string_list_clear_004d05e0(object + 0x08);
    host.crt_free_00bf65ac(native_at<void*>(object, 0x0C));
    native_at<Object>(object, 0x0C) = nullptr;
    release_native_string(host, object);
}

// 004D4830 __fastcall(object), RET (EAX): +0/+4 = 0; list +8 head from
// 004C3020, size +10h = 0; +24h..+14h = 0.
Object pooled_string_list_construct_004d4830(Seg10Host& host, Object object) {
    native_at<std::uint32_t>(object, 0x00) = 0;
    native_at<std::uint32_t>(object, 0x04) = 0;
    native_at<Object>(object, 0x0C) = host.pooled_string_list_sentinel_004c3020(object + 0x08);
    native_at<std::uint32_t>(object, 0x10) = 0;
    for (std::uint32_t offset = 0x24; offset >= 0x14; offset -= 4) native_at<std::uint32_t>(object, offset) = 0;
    return object;
}

// 004D4DB0 __fastcall(record): +4..+0Ch and +14h..+1Ch = 0.
void two_vector_record_construct_004d4db0(Object record) {
    for (std::uint32_t offset : {0x04u, 0x08u, 0x0Cu, 0x14u, 0x18u, 0x1Cu}) native_at<std::uint32_t>(record, offset) = 0;
}

// 004D9E60 __fastcall(array {data, count}): with a count, 004D45A0 on the
// last 2Ch-byte record, then count - 1.
void string_alias_array_pop_back_004d9e60(Seg10Host& host, Object array) {
    const std::uint32_t count = native_at<std::uint32_t>(array, 0x04);
    if (count == 0) return;
    host.string_alias_record_destroy_004d45a0(native_at<Object>(array, 0x00) + count * 0x2C - 0x2C);
    native_at<std::uint32_t>(array, 0x04) -= 1;
}

// 004E7730 / 004E7820 __thiscall(Block** holder, int capacity), RET 4: a
// counted block {aligned data, +4, capacity +8} with 1Ch / 4-byte elements.
// When the capacity is not above the current one it is simply stored;
// otherwise a new block of capacity*size + 18h (00BF55BE) gets the data
// 16-byte aligned past its 18h-byte header, the old elements (count = the old
// +8) are copied, +4 is carried over, and the old block is freed (00BF6989).
void shared_array1c_reserve_004e7730(Seg10Host& host, Object holder, std::int32_t capacity) {
    shared_array_reserve<0x1C>(host, holder, capacity);
}
void shared_array4_reserve_004e7820(Seg10Host& host, Object holder, std::int32_t capacity) {
    shared_array_reserve<0x04>(host, holder, capacity);
}

// 004E77F0 __fastcall(array): free a non-null +0Ch (00BF6989) and clear it;
// +8 = +4 = 0.
void simple_array_tidy_004e77f0(Seg10Host& host, Object array) {
    if (native_at<void*>(array, 0x0C) != nullptr) {
        host.crt_free_00bf6989(native_at<void*>(array, 0x0C));
        native_at<std::uint32_t>(array, 0x0C) = 0;
    }
    native_at<std::uint32_t>(array, 0x08) = 0;
    native_at<std::uint32_t>(array, 0x04) = 0;
}

// ---------------------------------------------------------------------------
// Game world and records.
// ---------------------------------------------------------------------------

// 004D2B10 __fastcall(game): 007FFD60(ECX game), 006EA1A0,
// 004CB220(ECX game+7178h, 0), 00442170, 0095BA30; unless the state queue
// (+5E8h set) front (004C80A0 on +5D8h) is 0Bh, the vehicle registry
// (00437F50) gets index tables +10h and +2010h filled with 0..7FFh; then
// 006F7B60, 00850930, 004B3E60, 004B1540, 00475860, 00479AC0, 00472420,
// 0047B4E0 and a tail jump to 00900BB0.
void game_destroy_world_containers_004d2b10(Seg10Host& host, Object game) {
    host.race_records_delete_all_007ffd60(game);
    host.bullet_class_cache_clear_006ea1a0();
    host.reference_slots_resize_004cb220(game + 0x7178, 0);
    host.device_class_registry_clear_00442170();
    host.vehicle_class_registry_clear_0095ba30();
    bool skip = false;
    if (native_at<std::uint32_t>(game, 0x5E8) != 0) skip = *host.state_queue_front_004c80a0(game + 0x5D8) == 0x0B;
    if (!skip) {
        Object registry = host.vehicle_class_registry_00437f50();
        for (std::uint32_t i = 0; i < 0x800; ++i) {
            native_at<std::uint32_t>(registry, 0x10 + i * 4) = i;
            native_at<std::uint32_t>(registry, 0x2010 + i * 4) = i;
        }
    }
    host.destroy_006f7b60();
    host.stationary_class_map_destroy_00850930();
    host.wreck_class_registry_clear_004b3e60();
    host.soldier_registry_destroy_004b1540();
    host.buoy_class_registry_delete_00475860();
    host.cloud_class_registry_delete_00479ac0();
    host.anim_class_registry_delete_00472420();
    host.vehicle_visual_registry_delete_0047b4e0();
    host.robot_registry_clear_00900bb0();
}

// 004D3040 (ECX unused), RET: new(10Ch) for the enum registry [00E1867C]
// (vtables 00CE78BC / +4 00CE7514, +8 = 0, 40h zero dwords from +0Ch; null
// on failure); new(10Ch) + 008F5670 into [00E18678]; 008F7100(ECX library,
// "universe\Library\" 00CE78E0).
void game_create_enum_registry_and_universe_library_004d3040(Seg10Host& host) {
    Object registry = host.operator_new_00bf681b(0x10C);
    if (registry != nullptr) {
        native_at<std::uint32_t>(registry, 0x00) = 0x00CE78BCu;
        native_at<std::uint32_t>(registry, 0x04) = 0x00CE7514u;
        native_at<std::uint32_t>(registry, 0x08) = 0;
        for (std::uint32_t i = 0; i < 0x40; ++i) native_at<std::uint32_t>(registry, 0x0C + i * 4) = 0;
    }
    host.set_global_slot(0x00E1867Cu, registry);
    Object memory = host.operator_new_00bf681b(0x10C);
    Object library = memory != nullptr ? host.property_tree_library_construct_008f5670(memory) : nullptr;
    host.set_global_slot(0x00E18678u, library);
    host.property_tree_library_load_folder_008f7100(library, "universe\\Library\\");
}

// 004D4A20 __fastcall(record), RET (EAX): vtable 00CE78FC, the eight
// vectors +8..+80h zeroed, 009F89F0(ECX record).
Object game_record_ce78fc_construct_004d4a20(Seg10Host& host, Object record) {
    native_at<std::uint32_t>(record, 0x00) = 0x00CE78FCu;
    for (std::uint32_t base = 0x08; base <= 0x78; base += 0x10) {
        native_at<std::uint32_t>(record, base + 0) = 0;
        native_at<std::uint32_t>(record, base + 4) = 0;
        native_at<std::uint32_t>(record, base + 8) = 0;
    }
    host.squadron_free_attack_targets_load_009f89f0(record);
    return record;
}

// 004D4AB0 __fastcall(record): each vector from +78h down to +8 frees a
// non-null data pointer (00BF65AC) and zeroes its three words; [00F8A71C] =
// 0; vtable 00CE3818.
void game_record_ce78fc_destruct_004d4ab0(Seg10Host& host, Object record) {
    for (std::int32_t base = 0x78; base >= 0x08; base -= 0x10) {
        void* data = native_at<void*>(record, static_cast<std::uint32_t>(base));
        if (data != nullptr) host.crt_free_00bf65ac(data);
        native_at<std::uint32_t>(record, static_cast<std::uint32_t>(base) + 0) = 0;
        native_at<std::uint32_t>(record, static_cast<std::uint32_t>(base) + 4) = 0;
        native_at<std::uint32_t>(record, static_cast<std::uint32_t>(base) + 8) = 0;
    }
    host.set_global_slot(0x00F8A71Cu, nullptr);
    native_at<std::uint32_t>(record, 0x00) = 0x00CE3818u;
}

// 004D9D10 __fastcall(record): vtable 00CE7AAC, three vectors +8..+30h
// zeroed.
void game_record_ce7aac_construct_004d9d10(Object record) {
    native_at<std::uint32_t>(record, 0x00) = 0x00CE7AACu;
    for (std::uint32_t offset : {0x08u, 0x0Cu, 0x10u, 0x18u, 0x1Cu, 0x20u, 0x28u, 0x2Cu, 0x30u}) {
        native_at<std::uint32_t>(record, offset) = 0;
    }
}

// 004D9D60 __fastcall(record): the ref-pointer vectors at +24h, +14h and +4
// each release their range (004CE600 / 004CE5C0 / 004CE580 with ECX first,
// EDX last, vector, record) and free the data, then zero; [00F8998C] = 0;
// vtable 00CE3818.
void game_record_ce7aac_destruct_004d9d60(Seg10Host& host, Object record) {
    struct Step { std::uint32_t vector; int which; };
    for (const Step step : {Step{0x24, 0}, Step{0x14, 1}, Step{0x04, 2}}) {
        Object vector = record + step.vector;
        Object first = native_at<Object>(vector, 0x04);
        if (first != nullptr) {
            Object last = native_at<Object>(vector, 0x08);
            if (step.which == 0) host.ref_ptr_range_release_004ce600(first, last, vector, record);
            else if (step.which == 1) host.ref_ptr_range_release_004ce5c0(first, last, vector, record);
            else host.ref_ptr_range_release_004ce580(first, last, vector, record);
            host.crt_free_00bf65ac(native_at<void*>(vector, 0x04));
        }
        native_at<std::uint32_t>(vector, 0x04) = 0;
        native_at<std::uint32_t>(vector, 0x08) = 0;
        native_at<std::uint32_t>(vector, 0x0C) = 0;
    }
    host.set_global_slot(0x00F8998Cu, nullptr);
    native_at<std::uint32_t>(record, 0x00) = 0x00CE3818u;
}

// 004D6CE0 (84h, 004D4A20, [00F8A71C]), 004DC200 (34h, 004D9D10,
// [00F8998C]) and 004E1CA0 (A0h, 004E18E0, [00E198D0]).
Object game_singleton_f8a71c_004d6ce0(Seg10Host& host) {
    return lazy_singleton(host, 0x00F8A71Cu, 0x84, [&](Object m) { return game_record_ce78fc_construct_004d4a20(host, m); });
}
Object game_singleton_f8998c_004dc200(Seg10Host& host) {
    return lazy_singleton(host, 0x00F8998Cu, 0x34, [&](Object m) { game_record_ce7aac_construct_004d9d10(m); return m; });
}
Object award_tracker_singleton_004e1ca0(Seg10Host& host) {
    return lazy_singleton(host, 0x00E198D0u, 0xA0, [&](Object m) { return award_tracker_construct_004e18e0(host, m); });
}

// 004E18E0 __fastcall(tracker), RET (EAX): vtable 00CE7F08; +8..+14h,
// +1Ch, +20h, +28h, +2Ch = 0; trees +30h (004C2A80, _Isnil +13Dh) and +3Ch
// (00443E20, +19h); lists +48h and +54h (004C3020, size 0); +60h, +64h,
// +6Ch..+78h = 0; trees +7Ch, +88h, +94h (004C2AD0, +21h).
Object award_tracker_construct_004e18e0(Seg10Host& host, Object tracker) {
    native_at<std::uint32_t>(tracker, 0x00) = 0x00CE7F08u;
    for (std::uint32_t offset : {0x08u, 0x0Cu, 0x10u, 0x14u, 0x1Cu, 0x20u, 0x28u, 0x2Cu}) {
        native_at<std::uint32_t>(tracker, offset) = 0;
    }
    init_tree(tracker + 0x30, host.tree_buy_head_004c2a80(tracker + 0x30), 0x13D);
    init_tree(tracker + 0x3C, host.tree_buy_head_00443e20(tracker + 0x3C), 0x19);
    native_at<Object>(tracker, 0x4C) = host.pooled_string_list_sentinel_004c3020(tracker + 0x48);
    native_at<std::uint32_t>(tracker, 0x50) = 0;
    native_at<Object>(tracker, 0x58) = host.pooled_string_list_sentinel_004c3020(tracker + 0x54);
    native_at<std::uint32_t>(tracker, 0x5C) = 0;
    for (std::uint32_t offset : {0x60u, 0x64u, 0x6Cu, 0x70u, 0x74u, 0x78u}) native_at<std::uint32_t>(tracker, offset) = 0;
    init_tree(tracker + 0x7C, host.tree_buy_head_004c2ad0(tracker + 0x7C), 0x21);
    init_tree(tracker + 0x88, host.tree_buy_head_004c2ad0(tracker + 0x88), 0x21);
    init_tree(tracker + 0x94, host.tree_buy_head_004c2ad0(tracker + 0x94), 0x21);
    return tracker;
}

// 004D4BB0 __thiscall(message, stream), RET 4: through the cursor stream+4,
// 8 bits into +10h and 3 bits into +18h; kind 3 reads an 8-bit count into
// the argument slot, then per record a qword (+8), two 32-bit ints (+10h,
// +14h), a bool (+18h) and six 32-bit ints (+1Ch..+30h) into a 38h-byte
// stack value whose first 8 bytes are never written, appended to the list
// +1Ch (004C6090 at the tail, 004CF800(1), relinked). Other kinds read 4
// bits into +19h.
void session_message_e4_read_004d4bb0(Seg10Host& host, Object message, Object stream) {
    Object cursor = stream + 4;
    host.bit_cursor_read_unsigned_byte_00428c70(cursor, message + 0x10, 8);
    host.bit_cursor_read_unsigned_byte_00428c70(cursor, message + 0x18, 3);
    if (native_at<std::uint8_t>(message, 0x18) != 3) {
        host.bit_cursor_read_unsigned_byte_00428c70(cursor, message + 0x19, 4);
        return;
    }
    alignas(4) std::uint8_t count[4];
    count[0] = 0;
    host.bit_cursor_read_unsigned_byte_00428c70(cursor, count, 8);
    if (count[0] == 0) return;
    Object list = message + 0x1C;
    do {
        alignas(8) std::uint8_t value[0x38];
        host.bit_cursor_read_qword_00428e90(cursor, value + 0x08);
        host.bit_cursor_read_signed_dword_00428d30(cursor, value + 0x10, 0x20);
        host.bit_cursor_read_signed_dword_00428d30(cursor, value + 0x14, 0x20);
        host.bit_cursor_read_boolean_00428d70(cursor, value + 0x18);
        for (std::uint32_t offset = 0x1C; offset <= 0x30; offset += 4) {
            host.bit_cursor_read_signed_dword_00428d30(cursor, value + offset, 0x20);
        }
        Object head = native_at<Object>(list, 0x04);
        Object node = host.list_buy_node40_004c6090(list, head, native_at<Object>(head, 0x04), value);
        host.list_inc_size_004cf800(list, 1);
        native_at<Object>(head, 0x04) = node;
        native_at<Object>(native_at<Object>(node, 0x04), 0x00) = node;
        count[0] = static_cast<std::uint8_t>(count[0] - 1);
    } while (count[0] != 0);
}

// 004D0FC0 __thiscall(game, record), RET 4: with a warning manager
// (+21E0h), a "leave" NativeString (0041E870, 00CE78B4) goes to
// 00985920(ECX manager, record+20h, &text) and is released; the record's
// name (+58h) is inserted into the set object at 00E18A60 (004D0640) and the
// temporary released; then the record is reset (+8, +0Ch, +18h, +1Bh = 0,
// +50h = 0, +0Dh = 1, +10h = FFFDh), game+18F0h[record+1Ch] = 0 and
// record+110h = -1.
void game_notify_player_left_004d0fc0(Seg10Host& host, Object game, Object record) {
    alignas(4) std::uint8_t text[8];
    Object manager = native_at<Object>(game, 0x21E0);
    if (manager != nullptr) {
        host.native_string_construct_0041e870(text, "leave");
        host.warning_dispatch_player_channel_00985920(native_at<Object>(game, 0x21E0),
                                                      native_at<std::uint32_t>(record, 0x20), text);
        release_native_string(host, text);
    }
    host.native_string_construct_0041e870(text, reinterpret_cast<const char*>(record + 0x58));
    alignas(4) std::uint8_t result[8];
    host.native_string_set_insert_004d0640(host.static_object(0x00E18A60u), result, text);
    release_native_string(host, text);
    native_at<std::uint8_t>(record, 0x08) = 0;
    native_at<std::uint8_t>(record, 0x0C) = 0;
    native_at<std::uint8_t>(record, 0x18) = 0;
    native_at<std::uint8_t>(record, 0x1B) = 0;
    native_at<std::uint32_t>(record, 0x50) = 0;
    native_at<std::uint8_t>(record, 0x0D) = 1;
    native_at<std::uint16_t>(record, 0x10) = 0xFFFD;
    native_at<std::uint32_t>(game, 0x18F0 + native_at<std::uint32_t>(record, 0x1C) * 4) = 0;
    native_at<std::int32_t>(record, 0x110) = -1;
}

// 004D2A80 __fastcall(holder): the NativeString +10h/+14h back to the pool;
// the NativeString vector +4..+0Ch destroyed (00432050(ECX first, EDX last,
// holder, holder)), freed and zeroed.
void game_clear_pending_mission_name_004d2a80(Seg10Host& host, Object holder) {
    release_native_string(host, holder + 0x10);
    Object first = native_at<Object>(holder, 0x04);
    if (first != nullptr) {
        host.native_string_range_destroy_00432050(first, native_at<Object>(holder, 0x08), holder, holder);
        host.crt_free_00bf65ac(native_at<void*>(holder, 0x04));
    }
    native_at<std::uint32_t>(holder, 0x04) = 0;
    native_at<std::uint32_t>(holder, 0x08) = 0;
    native_at<std::uint32_t>(holder, 0x0C) = 0;
}

// ---------------------------------------------------------------------------
// Prompt callbacks, state requests and presence.
// ---------------------------------------------------------------------------

// 004D7B10 (no arguments), RET: [00F1B038] = 2; hosting (game+624h set and
// +1FE4h == 1) -> 004D7970(ECX game, 0); else with a multiplayer menu, tail
// jump to 00688C70 on it.
void prompt_leave_session_004d7b10(Seg10Host& host) {
    Object game = host.game_00e188a8();
    host.set_prompt_status_00f1b038(2);
    if (native_at<std::uint32_t>(game, 0x624) != 0 && native_at<std::int32_t>(game, 0x1FE4) == 1) {
        host.game_end_scene_004d7970(game, 0);
        return;
    }
    if (host.multi_menu_00e198b4() != nullptr) host.multi_menu_leave_to_main_00688c70(host.multi_menu_00e198b4());
}

// 004D7B50: [00F1B038] = 2; with a multiplayer menu: when game+1FE4h is 2
// (non-zero), +218Ch is clear and a mission exists, 004D7970(game, 0); then
// the tail jump to 00688C70.
void prompt_leave_session_004d7b50(Seg10Host& host) {
    host.set_prompt_status_00f1b038(2);
    if (host.multi_menu_00e198b4() == nullptr) return;
    Object game = host.game_00e188a8();
    if (native_at<std::int32_t>(game, 0x1FE4) != 0 && native_at<std::uint8_t>(game, 0x218C) == 0 &&
        native_at<std::int32_t>(game, 0x1FE4) == 2 && host.mission_00e198c4() != nullptr) {
        host.game_end_scene_004d7970(game, 0);
    }
    host.multi_menu_leave_to_main_00688c70(host.multi_menu_00e198b4());
}

// 004DB240 __fastcall(ECX result): 1 -> tail jump to 004D8000(ECX game).
void prompt_set_presence_004db240(Seg10Host& host, std::int32_t result) {
    if (result == 1) game_set_user_presence_context_004d8000(host, host.game_00e188a8());
}

// 004DB260 __fastcall(ECX result): 1 -> when game+1FE4h == 1,
// 007728B0(ECX game+1EF0h, 0); then the tail jump to 004D8000.
void prompt_close_hosted_game_004db260(Seg10Host& host, std::int32_t result) {
    if (result != 1) return;
    Object game = host.game_00e188a8();
    if (native_at<std::int32_t>(game, 0x1FE4) == 1) {
        host.session_broadcast_game_closed_007728b0(game + 0x1EF0, 0);
        game = host.game_00e188a8();
    }
    game_set_user_presence_context_004d8000(host, game);
}

// 004D7920 __thiscall(game, int state), RET 4: 004D3ED0(ECX game+5D8h,
// &the argument).
void game_request_state_004d7920(Seg10Host& host, Object game, std::int32_t state) {
    host.enqueue_state_request_004d3ed0(game + 0x5D8, &state);
}

// 004D7940 __fastcall(game): enqueue 0Ah, then the tail jump to
// 00772610(ECX game+1EF0h).
void game_request_mission_reload_004d7940(Seg10Host& host, Object game) {
    const std::int32_t state = 0x0A;
    host.enqueue_state_request_004d3ed0(game + 0x5D8, &state);
    host.session_begin_mission_reload_00772610(game + 0x1EF0);
}

// 004D7F90 __fastcall(game): 004C88A0; with a mission enqueue 10h then 2,
// otherwise game+5D4h = 2; when 0067C8F0 is false, the tail jump to
// 00A3E6A0(ECX [00F8ABE8]).
void game_request_return_to_title_004d7f90(Seg10Host& host, Object game) {
    host.drain_state_requests_004c88a0(game);
    if (host.mission_00e198c4() != nullptr) {
        std::int32_t state = 0x10;
        host.enqueue_state_request_004d3ed0(game + 0x5D8, &state);
        state = 2;
        host.enqueue_state_request_004d3ed0(game + 0x5D8, &state);
    } else {
        native_at<std::int32_t>(game, 0x5D4) = 2;
    }
    if (!host.is_sign_in_state_valid_0067c8f0()) host.xlive_restart_signin_00a3e6a0(host.xlive_00f8abe8());
}

// 004D8000 __fastcall(game). The global game pointer is latched first.
// With a selected user (00A3E510): XUserSetContext(00A3EAC0(), 8001h, 4)
// (00A3EAC0 takes no stack arguments; the 4 and 8001h pushed before it are
// the import's last two arguments). Then [game]+22DCh = 1; +218Ch cleared when set;
// [00E19880] = 00A3E460(). With a pending invite whose slot (00A3E470)
// matches the active user (00A3EAC0): stop a playing movie (004F89A0,
// 004F8AC0), 00688B20(menu, 0Fh), 0067D6E0, tail jump 004BFC70(game).
// Otherwise unless game+5D4h is 2, the tail jump to 004D7F90.
void game_set_user_presence_context_004d8000(Seg10Host& host, Object game) {
    Object global_game = host.game_00e188a8();
    if (host.xenon_has_selected_user_00a3e510(host.xlive_00f8abe8())) {
        const std::uint32_t slot = host.xenon_active_user_slot_00a3eac0(host.xlive_00f8abe8());
        host.x_user_set_context_00a4d3f2(slot, 0x8001, 4);
    }
    native_at<std::uint8_t>(global_game, 0x22DC) = 1;
    if (native_at<std::uint8_t>(game, 0x218C) != 0) native_at<std::uint8_t>(game, 0x218C) = 0;
    Object xlive = host.xlive_00f8abe8();
    host.set_accepted_invite_00e19880(host.xenon_accepted_invite_info_00a3e460(xlive));
    if (host.pending_invite_00e198ac() != 0) {
        const std::uint32_t invitee = host.xenon_find_slot_for_invitee_00a3e470(xlive);
        if (invitee == host.xenon_active_user_slot_00a3eac0(xlive)) {
            Object player = host.movie_player_00e18d48();
            if (player != nullptr && host.movie_player_is_playing_004f89a0(player)) {
                host.movie_player_stop_004f8ac0(host.movie_player_00e18d48());
            }
            host.multi_menu_pop_request_interface_up_to_00688b20(host.multi_menu_00e198b4(), 0x0F);
            host.game_accept_pending_invite_0067d6e0();
            host.game_raise_multiplayer_menu_004bfc70(game);
            return;
        }
    }
    if (native_at<std::int32_t>(game, 0x5D4) != 2) game_request_return_to_title_004d7f90(host, game);
}

// 004DCA80 __thiscall(ECX passed through, arg), RET 4: a 60h-byte stack
// value with +8 and +0Ch zeroed (the rest unwritten) is passed by value to
// 004DC2C0(ECX, arg, value), which pops it.
void leaderboard_process_read_results_004dca80(Seg10Host& host, Object ecx, Object arg) {
    alignas(4) std::uint8_t value[0x60];
    native_at<std::uint32_t>(value, 0x08) = 0;
    native_at<std::uint32_t>(value, 0x0C) = 0;
    host.leaderboard_process_004dc2c0(ecx, arg, value);
}

// ---------------------------------------------------------------------------
// Input.
// ---------------------------------------------------------------------------

// 004D9420 __thiscall(game, action), RET 4 (ST0): last = 004D6900(ECX
// game+5C8h, &action); FCOMIP last vs now (+64Ch): unless last < now (or
// unordered) returns 0.0; else last = now and returns the input manager's
// [+4]+action*30h+24h float.
float input_repeat_take_axis_value_004d9420(Seg10Host& host, Object game, std::int32_t action) {
    const float last = *host.input_repeat_map_subscript_004d6900(game + 0x5C8, &action);
    if (last >= native_at<float>(game, 0x64C)) return 0.0f;
    *host.input_repeat_map_subscript_004d6900(game + 0x5C8, &action) = native_at<float>(game, 0x64C);
    Object manager = host.input_manager_004bec00();
    return native_at<float>(native_at<Object>(manager, 0x04), 0x24 + static_cast<std::uint32_t>(action) * 0x30);
}

// 004D9480 __thiscall(game, action), RET 4 (AL): the action record
// [manager+4]+action*30h must have +28h set and +20h clear; then the same
// repeat gate as 004D9420, returning 1 when it fires.
bool input_repeat_action_fired_004d9480(Seg10Host& host, Object game, std::int32_t action) {
    Object manager = host.input_manager_004bec00();
    Object record = native_at<Object>(manager, 0x04) + static_cast<std::uint32_t>(action) * 0x30;
    if (native_at<std::uint8_t>(record, 0x28) == 0 || native_at<std::uint8_t>(record, 0x20) != 0) return false;
    const float last = *host.input_repeat_map_subscript_004d6900(game + 0x5C8, &action);
    if (last >= native_at<float>(game, 0x64C)) return false;
    *host.input_repeat_map_subscript_004d6900(game + 0x5C8, &action) = native_at<float>(game, 0x64C);
    return true;
}

// 004D6410 __thiscall(game, level, ids), RET 8: 004D3740(ECX
// game+(level+56h)*10h, ids); 004C4300(game, level); the highest non-empty
// level among the vectors game+5A4h, +594h, +584h (default 1); for level < 4,
// 00A933F0(ECX input manager, 1Dh, that level).
void input_contexts_set_level_set_004d6410(Seg10Host& host, Object game, std::int32_t level, Object ids) {
    host.pointer_vector_assign_004d3740(game + static_cast<std::uint32_t>(level + 0x56) * 0x10, ids);
    host.input_contexts_apply_levels_004c4300(game, level);
    std::int32_t top = 4;
    Object vector = game + 0x5A4;
    do {
        Object first = native_at<Object>(vector, 0x00);
        if (first != nullptr && (static_cast<std::int32_t>(native_at<Object>(vector, 0x04) - first) >> 2) != 0) break;
        top -= 1;
        vector -= 0x10;
    } while (top > 1);
    if (level < 4) host.input_manager_set_context_level_00a933f0(host.input_manager_004bec00(), 0x1D, top);
}

// 004D8AE0 / 004D8B70: the ids are pushed into a stack vector (004D6840),
// 004D6410(game, 2 / 3, &vector), and the vector's data is freed.
void input_contexts_push_level2_004d8ae0(Seg10Host& host, Object game, const std::int32_t* ids) {
    push_level(host, game, ids, 2);
}
void input_contexts_push_level3_004d8b70(Seg10Host& host, Object game, const std::int32_t* ids) {
    push_level(host, game, ids, 3);
}

// 004DB030 __fastcall(game): 00A92C40(input manager, 0.0f). Single player
// (+1FE4h == 0): 004CD0F0(game, +634h == 0, 0, 1); a shown HUD (+634h clear)
// with the mission's [+A8h] +8 clear or +9 set resumes ([+DCh]+4 = 0,
// 005FD440 on [+D8h], 004D95F0(0), clears +9 when set); otherwise opens
// (005FF0D0 on [+D8h], or [+DCh]+4 = 1 when +635h is set), 004D95F0(1) and
// [+A8h]+9 = 1. Multiplayer: 004CD0F0 when +218Ch is set; then on the menu's
// +3Ch: clear -> [+DCh]+4 = 1 and 004D95F0(1) when +635h is set, else
// 00689030 and 004D95F0(1); set -> [+DCh]+4 = 0, 006882A0, 004D95F0(0).
void game_toggle_pause_menu_004db030(Seg10Host& host, Object game) {
    host.input_manager_update_00a92c40(host.input_manager_004bec00(), 0.0f);
    if (native_at<std::int32_t>(game, 0x1FE4) != 0) {
        if (native_at<std::uint8_t>(game, 0x218C) != 0) {
            host.game_set_cinematic_mode_004cd0f0(game, native_at<std::uint8_t>(game, 0x634) == 0 ? 1 : 0, 0, 1);
        }
        Object menu = host.multi_menu_00e198b4();
        if (native_at<std::uint8_t>(menu, 0x3C) == 0) {
            if (native_at<std::uint8_t>(game, 0x635) != 0) {
                native_at<std::uint8_t>(native_at<Object>(host.mission_00e198c4(), 0xDC), 0x04) = 1;
                host.game_set_system_pause_004d95f0(game, 1);
                return;
            }
            host.multi_menu_open_pause_00689030(menu);
            host.game_set_system_pause_004d95f0(game, 1);
            return;
        }
        native_at<std::uint8_t>(native_at<Object>(host.mission_00e198c4(), 0xDC), 0x04) = 0;
        host.multi_menu_close_pause_006882a0(host.multi_menu_00e198b4());
        host.game_set_system_pause_004d95f0(game, 0);
        return;
    }
    host.game_set_cinematic_mode_004cd0f0(game, native_at<std::uint8_t>(game, 0x634) == 0 ? 1 : 0, 0, 1);
    Object mission = host.mission_00e198c4();
    if (native_at<std::uint8_t>(game, 0x634) == 0) {
        Object hud = native_at<Object>(mission, 0xA8);
        if (native_at<std::uint8_t>(hud, 0x08) == 0 || native_at<std::uint8_t>(hud, 0x09) != 0) {
            native_at<std::uint8_t>(native_at<Object>(mission, 0xDC), 0x04) = 0;
            host.hud_pause_screen_resume_005fd440(native_at<Object>(host.mission_00e198c4(), 0xD8));
            host.game_set_system_pause_004d95f0(game, 0);
            Object hud_now = native_at<Object>(host.mission_00e198c4(), 0xA8);
            if (native_at<std::uint8_t>(hud_now, 0x09) != 0) native_at<std::uint8_t>(hud_now, 0x09) = 0;
            return;
        }
    }
    if (native_at<std::uint8_t>(game, 0x635) != 0) {
        native_at<std::uint8_t>(native_at<Object>(mission, 0xDC), 0x04) = 1;
    } else {
        host.hud_pause_screen_open_005ff0d0(native_at<Object>(mission, 0xD8));
    }
    host.game_set_system_pause_004d95f0(game, 1);
    native_at<std::uint8_t>(native_at<Object>(host.mission_00e198c4(), 0xA8), 0x09) = 1;
}

// ---------------------------------------------------------------------------
// Camera.
// ---------------------------------------------------------------------------

// 004DC940 __thiscall(game, float scale, int index), RET 8: game+1A04h =
// index. With [00E188A8]+634h set it continues only for mode 8 (+614h; when
// +61Ch is clear, single player and modes other than 8/9 continue too) with a
// mission whose [[+A8h]+20h] vtable+38h() is true. Then with a camera
// (+19FCh): 00B6FBB0(camera, float(config+F4h[index] * [00F889B4] *
// scale)), the products on the x87 stack.
void game_set_view_fov_004dc940(Seg10Host& host, Object game, float scale, std::int32_t index) {
    native_at<std::int32_t>(game, 0x1A04) = index;
    Object global_game = host.game_00e188a8();
    if (native_at<std::uint8_t>(global_game, 0x634) != 0) {
        const std::int32_t mode = native_at<std::int32_t>(global_game, 0x614);
        bool needs_mode8 = true;
        if (native_at<std::uint8_t>(global_game, 0x61C) == 0 && native_at<std::int32_t>(global_game, 0x1FE4) == 0 &&
            mode != 9 && mode != 8) {
            needs_mode8 = false;
        }
        if (needs_mode8 && mode != 8) return;
        Object mission = host.mission_00e198c4();
        if (mission == nullptr) return;
        if (!host.vcall_38_bool(native_at<Object>(native_at<Object>(mission, 0xA8), 0x20))) return;
    }
    Object config = host.global_config_00432650();
    Object camera = native_at<Object>(game, 0x19FC);
    if (camera == nullptr) return;
    const double base = static_cast<double>(native_at<float>(config, 0xF4 + static_cast<std::uint32_t>(native_at<std::int32_t>(game, 0x1A04)) * 4));
    const float fov = static_cast<float>(base * static_cast<double>(host.fov_scale_00f889b4()) * static_cast<double>(scale));
    host.camera_set_fov_00b6fbb0(camera, fov);
}

// 004DCDF0 __fastcall(game): with a camera: 00B6FBD0(aspect [0109CF04]+10h),
// 00B6FE10(7), 00B1F940(00B6FDE0(camera), {[00F88994], [00F88998]}),
// 00B6FC10(20000.0f, 00CE3CC0); then 004DC940(game, float(camera+1C4h /
// (config+F4h[index] * [00F889B4])), game+1A04h) (FDIVP ST(1),ST(0)).
void game_apply_camera_display_settings_004dcdf0(Seg10Host& host, Object game) {
    if (native_at<Object>(game, 0x19FC) == nullptr) return;
    host.camera_set_aspect_00b6fbd0(native_at<Object>(game, 0x19FC), native_at<float>(host.display_0109cf04(), 0x10));
    host.camera_set_clear_flags_00b6fe10(native_at<Object>(game, 0x19FC), 7);
    const std::uint32_t size[2] = {host.resolution_00f88994(), host.resolution_00f88998()};
    host.viewport_set_dimensions_00b1f940(host.camera_get_viewport_00b6fde0(native_at<Object>(game, 0x19FC)), size);
    constexpr float kFarPlane = 20000.0f;  // 00CE3CC0
    host.camera_set_far_plane_00b6fc10(native_at<Object>(game, 0x19FC), kFarPlane);
    const float current = native_at<float>(native_at<Object>(game, 0x19FC), 0x1C4);
    Object config = host.global_config_00432650();
    const std::int32_t index = native_at<std::int32_t>(game, 0x1A04);
    const double denominator = static_cast<double>(native_at<float>(config, 0xF4 + static_cast<std::uint32_t>(index) * 4)) *
                               static_cast<double>(host.fov_scale_00f889b4());
    const float ratio = static_cast<float>(static_cast<double>(current) / denominator);
    game_set_view_fov_004dc940(host, game, ratio, index);
}

// ---------------------------------------------------------------------------
// Scene entities.
// ---------------------------------------------------------------------------

// 004E5850 __thiscall(entity, id), RET 4 (EAX): 00928630(entity, 0, id),
// 007B28F0(ECX entity+1E4h, entity), vtables (+1E4h 00CE84E8, +0
// 00CE8390, +10h 00CE8378, +24h 00CE8370, +170h 00CE836C), class id
// +C4h = 4Ah.
Object camera_path_construct_004e5850(Seg10Host& host, Object entity, std::uint32_t network_id) {
    host.game_entity_construct_00928630(entity, 0, network_id);
    host.camera_path_component_construct_007b28f0(entity + 0x1E4, entity);
    native_at<std::uint32_t>(entity, 0x1E4) = 0x00CE84E8u;
    native_at<std::uint32_t>(entity, 0x000) = 0x00CE8390u;
    native_at<std::uint32_t>(entity, 0x010) = 0x00CE8378u;
    native_at<std::uint32_t>(entity, 0x024) = 0x00CE8370u;
    native_at<std::uint32_t>(entity, 0x170) = 0x00CE836Cu;
    native_at<std::int32_t>(entity, 0x0C4) = 0x4A;
    return entity;
}

// 004E58E0 __fastcall(entity): the same vtables restored, 007B1BE0(ECX
// entity+1E4h), 009287B0(entity).
void camera_path_destruct_004e58e0(Seg10Host& host, Object entity) {
    native_at<std::uint32_t>(entity, 0x000) = 0x00CE8390u;
    native_at<std::uint32_t>(entity, 0x010) = 0x00CE8378u;
    native_at<std::uint32_t>(entity, 0x024) = 0x00CE8370u;
    native_at<std::uint32_t>(entity, 0x170) = 0x00CE836Cu;
    native_at<std::uint32_t>(entity, 0x1E4) = 0x00CE84E8u;
    host.camera_path_component_destroy_007b1be0(entity + 0x1E4);
    host.game_entity_destruct_009287b0(entity);
}

// 004E5980 __thiscall(entity, id), RET 4 (EAX): 00928630(entity, 0, id),
// vtables 00CE8550 / +10h 00CE8534 / +24h 00CE852C / +170h 00CE8528, +C4h = 41h.
Object nav_point_construct_004e5980(Seg10Host& host, Object entity, std::uint32_t network_id) {
    host.game_entity_construct_00928630(entity, 0, network_id);
    native_at<std::uint32_t>(entity, 0x000) = 0x00CE8550u;
    native_at<std::uint32_t>(entity, 0x010) = 0x00CE8534u;
    native_at<std::uint32_t>(entity, 0x024) = 0x00CE852Cu;
    native_at<std::uint32_t>(entity, 0x170) = 0x00CE8528u;
    native_at<std::int32_t>(entity, 0x0C4) = 0x41;
    return entity;
}

// 004E5BB0 (four stack arguments, RET 10h): 0053D700(ECX second argument),
// returns 0.
std::uint32_t free_camera_set_matrix_return_zero_004e5bb0(Seg10Host& host, std::uint32_t, Object camera,
                                                         std::uint32_t, std::uint32_t) {
    host.free_camera_set_matrix_0053d700(camera);
    return 0;
}

// 004E5B20 __thiscall(dest, source), RET 4: vtable 00CE89D4, +4, +8, then
// +0Ch copied six times in a row as dword, float (FLD/FSTP), dword, byte,
// dword, dword (the union members' copies), then +10h..+28h dwords, byte
// +2Ch, dwords +30h and +34h.
Object scene_command_record_copy_construct_004e5b20(Object dest, Object source) {
    native_at<std::uint32_t>(dest, 0x00) = 0x00CE89D4u;
    native_at<std::uint32_t>(dest, 0x04) = native_at<std::uint32_t>(source, 0x04);
    native_at<std::uint32_t>(dest, 0x08) = native_at<std::uint32_t>(source, 0x08);
    native_at<std::uint32_t>(dest, 0x0C) = native_at<std::uint32_t>(source, 0x0C);
    native_at<float>(dest, 0x0C) = native_at<float>(source, 0x0C);
    native_at<std::uint32_t>(dest, 0x0C) = native_at<std::uint32_t>(source, 0x0C);
    native_at<std::uint8_t>(dest, 0x0C) = native_at<std::uint8_t>(source, 0x0C);
    native_at<std::uint32_t>(dest, 0x0C) = native_at<std::uint32_t>(source, 0x0C);
    native_at<std::uint32_t>(dest, 0x0C) = native_at<std::uint32_t>(source, 0x0C);
    for (std::uint32_t offset = 0x10; offset <= 0x28; offset += 4) {
        native_at<std::uint32_t>(dest, offset) = native_at<std::uint32_t>(source, offset);
    }
    native_at<std::uint8_t>(dest, 0x2C) = native_at<std::uint8_t>(source, 0x2C);
    native_at<std::uint32_t>(dest, 0x30) = native_at<std::uint32_t>(source, 0x30);
    native_at<std::uint32_t>(dest, 0x34) = native_at<std::uint32_t>(source, 0x34);
    return dest;
}

// 004E6540 __thiscall(node, value), RET 4 (EAX): vtable 00CE89E8,
// 004E6480(ECX node), +4Ch = value.
Object collision_node_record_construct_004e6540(Seg10Host& host, Object node, std::uint32_t value) {
    native_at<std::uint32_t>(node, 0x00) = 0x00CE89E8u;
    host.collision_node_init_fields_004e6480(node);
    native_at<std::uint32_t>(node, 0x4C) = value;
    return node;
}

// 004E68A0 (no ECX use), RET: new(48h); 0.0f stores at +0..+1Ch and
// +28h..+38h, dwords +20h/+24h = 0, 0.3f (00CE69C8) at +3Ch and +40h, 0.0f
// at +44h; null on failure.
Object scene_object_record_create_004e68a0(Seg10Host& host) {
    Object record = host.operator_new_00bf681b(0x48);
    if (record == nullptr) return nullptr;
    constexpr float kDefault = 0.3f;  // 00CE69C8 (0x3E99999A)
    for (std::uint32_t offset = 0x00; offset <= 0x1C; offset += 4) native_at<float>(record, offset) = 0.0f;
    native_at<std::uint32_t>(record, 0x20) = 0;
    native_at<std::uint32_t>(record, 0x24) = 0;
    for (std::uint32_t offset = 0x28; offset <= 0x38; offset += 4) native_at<float>(record, offset) = 0.0f;
    native_at<float>(record, 0x3C) = kDefault;
    native_at<float>(record, 0x40) = kDefault;
    native_at<float>(record, 0x44) = 0.0f;
    return record;
}

// World lists: register = 00928560 then 00484540(ECX [entity+30h]+list,
// entity); unregister = 00928570 then the chain walk (head pointer at
// list+4) and 004837D0.
void nav_point_register_004e78d0(Seg10Host& host, Object entity) { register_in(host, entity, 0x324); }
void nav_point_unregister_004e78f0(Seg10Host& host, Object entity) { unregister_from(host, entity, 0x324); }
void movie_cam_pos_register_004e7930(Seg10Host& host, Object entity) { register_in(host, entity, 0x330); }
void movie_cam_pos_unregister_004e7950(Seg10Host& host, Object entity) { unregister_from(host, entity, 0x330); }
void movie_cam_lookat_register_004e7990(Seg10Host& host, Object entity) { register_in(host, entity, 0x33C); }

// 004E60E0.
void format_string64_004e60e0(Seg10Host& host, char* dest, const char* format, ...) {
    va_list args;
    va_start(args, format);
    host.crt_vsprintf_s_00bf7289(dest, 0x40, format, args);
    va_end(args);
}

} // namespace bsp::cc13_seg10
