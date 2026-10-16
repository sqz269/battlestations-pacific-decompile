// Segment 33 upper half (packet cc13-seg33b; marker cc13_seg33b): MCommandBuilding helpers that
// the cc9 gameplay hosts do not already implement (the garrison and paratrooper vectors, the
// pad walks, the inferior-list session message, the Lua class reader, the message dispatcher,
// construction and destruction) and the shared land-fort virtuals. See
// include/bsp/cc13_seg33b.hpp and docs/CC13_SEG33B.md (which also lists the CommandBuilding
// routines recorded ledger-only because a host implements them, with adoption notes).

#include "bsp/cc13_seg33b.hpp"

#include <initializer_list>

namespace bsp::cc13_seg33b {
namespace {

// operator new's size argument as the image computes it for n dwords: n * 4, saturated to
// FFFFFFFFh when the 32-bit product overflows (MUL / SETO / NEG / OR).
std::uint32_t dword_bytes(std::uint32_t count) {
    const std::uint64_t bytes = static_cast<std::uint64_t>(count) * 4u;
    return bytes > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes);
}

Object section_of(Object building) { return obj(at<std::uint32_t>(building, 0x764)); }

void lock(Seg33bHost& host, Object building) {
    Object section = section_of(building);
    host.enter_critical_section(section);
    at<std::int32_t>(section, 0x18) += 1;
}

void unlock(Seg33bHost& host, Object building) {
    Object section = section_of(building);
    at<std::int32_t>(section, 0x18) -= 1;
    host.leave_critical_section(section);
}

void set_target(Seg33bHost& host, Object member, Object target) {
    Object old = obj(at<std::uint32_t>(member, 0x14));
    if (old == target) return;
    if (old != nullptr) host.observer_unregister_006952a0(old, member);
    at<std::uint32_t>(member, 0x14) = addr(target);
    if (target != nullptr) host.observer_register_00694a60(target, member);
}

// The garrison entry destructor the vector routines inline (006F4080..006F40C5,
// 006F4C21..006F4C56): the property bag at +58h through its deleting destructor (flags 1), the
// base vtable 00CEDD88, the observer pair (target +14h) unregistered, 00695870.
void destroy_entry_inline(Seg33bHost& host, Object entry) {
    Object bag = obj(at<std::uint32_t>(entry, 0x58));
    if (bag != nullptr) host.object_deleting_destructor_0(bag, 1);
    at<std::uint32_t>(entry, 0x0) = kGarrisonEntryBaseVtable;
    Object target = obj(at<std::uint32_t>(entry, 0x14));
    if (target != nullptr) host.observer_unregister_006952a0(target, entry);
    host.callback_owner_destroy_00695870(entry);
}

Object pad_at(Object building, std::uint32_t i) {
    return obj(at<std::uint32_t>(obj(at<std::uint32_t>(building, 0x794)), i * 4));
}

}  // namespace

// 006F1EE0 / 006F1F00: EnterCriticalSection then ++depth; --depth then LeaveCriticalSection.
void command_building_lock_006f1ee0(Seg33bHost& host, Object building) { lock(host, building); }
void command_building_unlock_006f1f00(Seg33bHost& host, Object building) { unlock(host, building); }

// 006F1F80: RET. The building does not react to the die notification.
void command_building_die_006f1f80(Object building) { (void)building; }

// 006F2250.
void free_and_null_006f2250(Seg33bHost& host, Object cell) {
    void* block = obj(at<std::uint32_t>(cell, 0x0));
    if (block != nullptr) {
        host.free_00bf6989(block);
        at<std::uint32_t>(cell, 0x0) = 0;
    }
}

// 006F2330: when n > capacity (unsigned): capacity = n, new[] (00BF55BE, saturated size), and
// when the old buffer is set copy size dwords and free it (00BF6989); data = new buffer.
void dword_vector_reserve_006f2330(Seg33bHost& host, Object vector, std::uint32_t count) {
    if (at<std::uint32_t>(vector, 0x8) < count) {
        at<std::uint32_t>(vector, 0x8) = count;
        Object fresh = host.operator_new_00bf55be(dword_bytes(count));
        Object old = obj(at<std::uint32_t>(vector, 0x0));
        if (old != nullptr) {
            for (std::uint32_t i = 0; i < at<std::uint32_t>(vector, 0x4); ++i) {
                at<std::uint32_t>(fresh, i * 4) = at<std::uint32_t>(old, i * 4);
            }
            host.free_00bf6989(old);
        }
        at<std::uint32_t>(vector, 0x0) = addr(fresh);
    }
}

// 006F23E0: +0h = range, +4h = -00BD2F10(stream 1)(0.0f, range). Draws from the shared RNG.
Object random_offset_pair_init_006f23e0(Seg33bHost& host, Object pair, float range) {
    at<float>(pair, 0x0) = range;
    at<float>(pair, 0x4) = -host.random_uniform_00bd2f10(1, 0.0f, range);
    return pair;
}

// 006F3010: every pad of +794h (bounds re-read each step) through 006AC4D0.
void command_building_refresh_pads_006f3010(Seg33bHost& host, Object building) {
    for (std::uint32_t i = 0; i != at<std::uint32_t>(building, 0x798); ++i) {
        host.pad_refresh_006ac4d0(pad_at(building, i));
    }
}

// 006F30A0 / 006F3320 / 006F34F0: unregister the old pair (006952A0, ECX = old, EDX =
// member), store +14h, register the new pair (00694A60) when non-null; nothing when equal.
void observer_member_set_target_006f30a0(Seg33bHost& host, Object member, Object target) {
    set_target(host, member, target);
}
void observer_member_set_target_006f3320(Seg33bHost& host, Object member, Object target) {
    set_target(host, member, target);
}
void observer_member_set_target_006f34f0(Seg33bHost& host, Object member, Object target) {
    set_target(host, member, target);
}

// 006F30D0: n = max(n, 1); when capacity < n (signed): new[] n * 4 (00BF55BE), copy size
// dwords (null test on the destination), free the old buffer unconditionally (00BF6989),
// data = new, capacity = n.
void pointer_vector_reserve_006f30d0(Seg33bHost& host, Object vector, std::int32_t count) {
    if (count < 1) count = 1;
    if (at<std::int32_t>(vector, 0x8) < count) {
        Object fresh = host.operator_new_00bf55be(static_cast<std::uint32_t>(count) * 4);
        for (std::int32_t i = 0; i < at<std::int32_t>(vector, 0x4); ++i) {
            if (fresh + i * 4 != nullptr) {
                at<std::uint32_t>(fresh, i * 4) = at<std::uint32_t>(obj(at<std::uint32_t>(vector, 0x0)), i * 4);
            }
        }
        host.free_00bf6989(obj(at<std::uint32_t>(vector, 0x0)));
        at<std::uint32_t>(vector, 0x0) = addr(fresh);
        at<std::int32_t>(vector, 0x8) = count;
    }
}

// 006F3140: n = 2 * capacity + 2; when n > capacity (unsigned) reallocate as 006F2330 does.
void dword_vector_grow_006f3140(Seg33bHost& host, Object vector) {
    const std::uint32_t count = at<std::uint32_t>(vector, 0x8) * 2 + 2;
    if (at<std::uint32_t>(vector, 0x8) < count) {
        at<std::uint32_t>(vector, 0x8) = count;
        Object fresh = host.operator_new_00bf55be(dword_bytes(count));
        Object old = obj(at<std::uint32_t>(vector, 0x0));
        if (old != nullptr) {
            for (std::uint32_t i = 0; i < at<std::uint32_t>(vector, 0x4); ++i) {
                at<std::uint32_t>(fresh, i * 4) = at<std::uint32_t>(old, i * 4);
            }
            host.free_00bf6989(old);
        }
        at<std::uint32_t>(vector, 0x0) = addr(fresh);
    }
}

// 006F31B0: the property bag at +58h through its deleting destructor (flags 1), then the base
// destructor 00545600. (Its SEH frame only orders the base after a throwing bag delete.)
void garrison_entry_destruct_006f31b0(Seg33bHost& host, Object entry) {
    Object bag = obj(at<std::uint32_t>(entry, 0x58));
    if (bag != nullptr) host.object_deleting_destructor_0(bag, 1);
    host.garrison_entry_base_destruct_00545600(entry);
}

// 006F32B0: the first element equal to *value is replaced by the last one (unless it is the
// last) and size drops by one; true when found.
bool pointer_vector_erase_value_006f32b0(Object vector, const std::uint32_t* value) {
    std::uint32_t* first = reinterpret_cast<std::uint32_t*>(obj(at<std::uint32_t>(vector, 0x0)));
    const std::uint32_t size = at<std::uint32_t>(vector, 0x4);
    std::int32_t index = -1;
    if (first < first + size) {
        std::uint32_t* p = first;
        while (*p != *value) {
            ++p;
            if (first + size <= p) return false;
        }
        index = static_cast<std::int32_t>(p - first);
        if (index != -1) {
            if (static_cast<std::uint32_t>(index) != size - 1) first[index] = first[size - 1];
            at<std::uint32_t>(vector, 0x4) -= 1;
        }
    }
    return index != -1;
}

namespace {
void push_pointer(Seg33bHost& host, Object vector, std::uint32_t value) {
    if (at<std::int32_t>(vector, 0x4) == at<std::int32_t>(vector, 0x8)) {
        std::int32_t count = at<std::int32_t>(vector, 0x8) * 2;
        if (!(count > 1)) count = 1;
        pointer_vector_reserve_006f30d0(host, vector, count);
    }
    Object slot = obj(at<std::uint32_t>(vector, 0x0)) + at<std::int32_t>(vector, 0x4) * 4;
    if (slot != nullptr) at<std::uint32_t>(slot, 0) = value;
    at<std::int32_t>(vector, 0x4) += 1;
}
}  // namespace

// 006F3360 / 006F3530: reserve max(2 * capacity, 1) when full (006F30D0), store unless the
// slot address is null, ++size.
void pointer_vector_push_back_006f3360(Seg33bHost& host, Object vector, const std::uint32_t* value) {
    push_pointer(host, vector, *value);
}
void pointer_vector_push_back_006f3530(Seg33bHost& host, Object vector, const std::uint32_t* value) {
    push_pointer(host, vector, *value);
}

// 006F33A0: reserve when n > capacity; zero-fill [size, n) (null test per slot); size = n
// (the shrink loop only counts down).
void pointer_vector_resize_006f33a0(Seg33bHost& host, Object vector, std::int32_t count) {
    if (at<std::int32_t>(vector, 0x8) < count) pointer_vector_reserve_006f30d0(host, vector, count);
    for (std::int32_t i = at<std::int32_t>(vector, 0x4); i < count; ++i) {
        Object slot = obj(at<std::uint32_t>(vector, 0x0)) + i * 4;
        if (slot != nullptr) at<std::uint32_t>(slot, 0) = 0;
    }
    while (count < at<std::int32_t>(vector, 0x4)) at<std::int32_t>(vector, 0x4) -= 1;
    at<std::int32_t>(vector, 0x4) = count;
}

// 006F3410: when size == capacity grow to 2n+2 inline (006F3140's body), then store without a
// null test and ++size.
void dword_vector_push_back_006f3410(Seg33bHost& host, Object vector, std::uint32_t value) {
    if (at<std::uint32_t>(vector, 0x4) == at<std::uint32_t>(vector, 0x8)) {
        dword_vector_grow_006f3140(host, vector);
    }
    at<std::uint32_t>(obj(at<std::uint32_t>(vector, 0x0)), at<std::uint32_t>(vector, 0x4) * 4) = value;
    at<std::uint32_t>(vector, 0x4) += 1;
}

// 006F3490.
Object entity_as_kind_0c_006f3490(Seg33bHost& host, Object entity) {
    if (entity != nullptr && host.object_is_kind_of_5c(entity, 0xC)) return entity;
    return nullptr;
}

// 006F34B0: under the lock, swap-erase the unit from +7DCh (006F32B0; the result is unused).
void command_building_remove_paratrooper_006f34b0(Seg33bHost& host, Object building, std::uint32_t unit) {
    lock(host, building);
    pointer_vector_erase_value_006f32b0(building + 0x7DC, &unit);
    unlock(host, building);
}

// 006F39B0: under the lock, the inline push_back of 006F3360 on +7DCh. The unlock is a tail
// jump to LeaveCriticalSection.
void command_building_add_paratrooper_006f39b0(Seg33bHost& host, Object building, std::uint32_t unit) {
    lock(host, building);
    push_pointer(host, building + 0x7DC, unit);
    unlock(host, building);
}

// 006F3570: reserve max(2 * capacity, 1) through 005296A0 when full, store unless null, ++size.
void word_array_push_back_006f3570(Seg33bHost& host, Object array, const std::uint16_t* value) {
    if (at<std::int32_t>(array, 0x4) == at<std::int32_t>(array, 0x8)) {
        std::int32_t count = at<std::int32_t>(array, 0x8) * 2;
        if (!(count > 1)) count = 1;
        host.word_array_reserve_005296a0(array, count);
    }
    Object slot = obj(at<std::uint32_t>(array, 0x0)) + at<std::int32_t>(array, 0x4) * 2;
    if (slot != nullptr) at<std::uint16_t>(slot, 0) = *value;
    at<std::int32_t>(array, 0x4) += 1;
}

// 006F35E0: the observer handle part (00545F50), vtable 00CEDDA0, the 40h bytes at +18h
// copied, the property bag cloned (008F41F0 on [from+58h]), +5Ch and +60h copied. The unwind
// that destroys the handle part when the clone throws is not modelled.
Object garrison_entry_copy_construct_006f35e0(Seg33bHost& host, Object to, Object from) {
    host.observer_handle_copy_00545f50(to, from);
    at<std::uint32_t>(to, 0x0) = kGarrisonEntryVtable;
    for (std::size_t i = 0; i < 0x10; ++i) {
        at<std::uint32_t>(to, 0x18 + i * 4) = at<std::uint32_t>(from, 0x18 + i * 4);
    }
    at<std::uint32_t>(to, 0x58) = host.property_bag_clone_008f41f0(obj(at<std::uint32_t>(from, 0x58)));
    at<std::uint32_t>(to, 0x5C) = at<std::uint32_t>(from, 0x5C);
    at<std::uint32_t>(to, 0x60) = at<std::uint32_t>(from, 0x60);
    return to;
}

// 006F3A20: for every pad of +794h, every element of the pad's std::list at +1FCh (head
// +200h; the checked-iterator tests call 00BF6713 when dereferencing end): the element's
// record r = node+8h, unit = [r] or else [r+8h]; true at the first element where
// 0048CBB0(ECX = [[00E188A8]+21D0h])(unit ? unit+1E4h : null) answers true.
bool command_building_any_pad_group_006f3a20(Seg33bHost& host, Object building) {
    for (std::uint32_t i = 0; i != at<std::uint32_t>(building, 0x798); ++i) {
        Object list = pad_at(building, i) + 0x1FC;
        Object node = obj(at<std::uint32_t>(obj(at<std::uint32_t>(list, 0x4)), 0x0));
        for (;;) {
            Object head = obj(at<std::uint32_t>(list, 0x4));
            if (node == head) break;
            if (node == obj(at<std::uint32_t>(list, 0x4))) host.invalid_iterator_00bf6713();
            Object record = obj(at<std::uint32_t>(node, 0x8));
            Object unit = obj(at<std::uint32_t>(record, 0x0));
            if (unit == nullptr) unit = obj(at<std::uint32_t>(record, 0x8));
            Object sub = unit != nullptr ? unit + 0x1E4 : nullptr;
            Object game = obj(host.global_u32(kGameGlobals));
            if (host.pad_group_query_0048cbb0(obj(at<std::uint32_t>(game, 0x21D0)), sub)) return true;
            if (node == obj(at<std::uint32_t>(list, 0x4))) host.invalid_iterator_00bf6713();
            node = obj(at<std::uint32_t>(node, 0x0));
        }
    }
    return false;
}

// 006F3FC0: pointer vector tidy, resize(0) (006F33A0) then free the buffer (00BF6989).
void pointer_vector_tidy_006f3fc0(Seg33bHost& host, Object vector) {
    pointer_vector_resize_006f33a0(host, vector, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(vector, 0x0)));
}

// 006F3FE0: n = max(n, 1); when capacity < n: new[] n * 64h (00BF55BE), copy-construct every
// entry into it (006F35E0, null test per slot), destroy the old entries (inline), free the
// old buffer, data = new, capacity = n.
void garrison_vector_reserve_006f3fe0(Seg33bHost& host, Object vector, std::int32_t count) {
    if (count < 1) count = 1;
    if (at<std::int32_t>(vector, 0x8) < count) {
        Object fresh = host.operator_new_00bf55be(static_cast<std::uint32_t>(count) * 100);
        for (std::int32_t i = 0; i < at<std::int32_t>(vector, 0x4); ++i) {
            if (fresh + i * 100 != nullptr) {
                garrison_entry_copy_construct_006f35e0(host, fresh + i * 100,
                                                       obj(at<std::uint32_t>(vector, 0x0)) + i * 100);
            }
        }
        for (std::int32_t i = 0; i < at<std::int32_t>(vector, 0x4); ++i) {
            destroy_entry_inline(host, obj(at<std::uint32_t>(vector, 0x0)) + i * 100);
        }
        host.free_00bf6989(obj(at<std::uint32_t>(vector, 0x0)));
        at<std::uint32_t>(vector, 0x0) = addr(fresh);
        at<std::int32_t>(vector, 0x8) = count;
    }
}

// 006F4B30: reserve max(2 * capacity, 1) when full, copy-construct at the end (null test),
// ++size.
void garrison_vector_push_back_006f4b30(Seg33bHost& host, Object vector, Object entry) {
    if (at<std::int32_t>(vector, 0x4) == at<std::int32_t>(vector, 0x8)) {
        std::int32_t count = at<std::int32_t>(vector, 0x8) * 2;
        if (!(count > 1)) count = 1;
        garrison_vector_reserve_006f3fe0(host, vector, count);
    }
    Object slot = obj(at<std::uint32_t>(vector, 0x0)) + at<std::int32_t>(vector, 0x4) * 100;
    if (slot != nullptr) garrison_entry_copy_construct_006f35e0(host, slot, entry);
    at<std::int32_t>(vector, 0x4) += 1;
}

// 006F4BA0: reserve when n > capacity; default-construct [size, n) inline (+4h, +8h, +0Ch,
// +14h = 0, byte +10h = 1, vtable 00CEDDA0, +58h = 0; null test per slot); when n < size
// destroy from the back (inline) down to n; size = n.
void garrison_vector_resize_006f4ba0(Seg33bHost& host, Object vector, std::int32_t count) {
    if (at<std::int32_t>(vector, 0x8) < count) garrison_vector_reserve_006f3fe0(host, vector, count);
    for (std::int32_t i = at<std::int32_t>(vector, 0x4); i < count; ++i) {
        Object entry = obj(at<std::uint32_t>(vector, 0x0)) + i * 100;
        if (entry != nullptr) {
            at<std::uint32_t>(entry, 0x4) = 0;
            at<std::uint32_t>(entry, 0x8) = 0;
            at<std::uint32_t>(entry, 0xC) = 0;
            at<std::uint32_t>(entry, 0x14) = 0;
            at<std::uint8_t>(entry, 0x10) = 1;
            at<std::uint32_t>(entry, 0x0) = kGarrisonEntryVtable;
            at<std::uint32_t>(entry, 0x58) = 0;
        }
    }
    if (count < at<std::int32_t>(vector, 0x4)) {
        do {
            at<std::int32_t>(vector, 0x4) -= 1;
            destroy_entry_inline(host, obj(at<std::uint32_t>(vector, 0x0)) + at<std::int32_t>(vector, 0x4) * 100);
        } while (count < at<std::int32_t>(vector, 0x4));
    }
    at<std::int32_t>(vector, 0x4) = count;
}

// 006F55F0: resize(0) (006F4BA0) then free the buffer (00BF6989).
void garrison_vector_tidy_006f55f0(Seg33bHost& host, Object vector) {
    garrison_vector_resize_006f4ba0(host, vector, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(vector, 0x0)));
}

// 006F4110: the D6h message: base (0075B430(D6h)), +18h word 0, +4h = 1, +1Ah byte 0, vtable
// 00CFAE84, the uint16 array +1Ch {data, size, cap} empty; then for every garrison entry its
// observed unit's id (target +14h, unit+174h) or 0, appended as 006F3570 does inline.
Object inferior_list_message_construct_006f4110(Seg33bHost& host, Object message, Object garrison) {
    host.session_message_construct_0075b430(message, 0xD6);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint32_t>(message, 0x4) = 1;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x0) = kInferiorListMessageVtable;
    at<std::uint32_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x20) = 0;
    at<std::uint32_t>(message, 0x24) = 0;
    for (std::int32_t i = 0; i < at<std::int32_t>(garrison, 0x4); ++i) {
        Object target = obj(at<std::uint32_t>(obj(at<std::uint32_t>(garrison, 0x0)), i * 100 + 0x14));
        const std::uint16_t id = target != nullptr ? at<std::uint16_t>(target, 0x174) : 0;
        word_array_push_back_006f3570(host, message + 0x1C, &id);
    }
    return message;
}

// 006F4220: the extended header (0075B480), the count's low byte in 8 bits, then each id in
// 12 bits.
void inferior_list_message_write_006f4220(Seg33bHost& host, Object message, Object stream) {
    host.session_message_write_header_0075b480(message, stream);
    host.bit_write_byte_00428ff0(stream, at<std::uint8_t>(message, 0x20), 8);
    for (std::int32_t i = 0; i < at<std::int32_t>(message, 0x20); ++i) {
        host.bit_write_word_00429120(stream, at<std::uint16_t>(obj(at<std::uint32_t>(message, 0x1C)), i * 2), 0xC);
    }
}

// 006F4270: the extended header (0075B4C0), an 8-bit count from the cursor at stream+4h, then
// that many 12-bit ids appended as 006F3570 does inline. The image reads each id into a dword
// slot that held the message pointer; only its low word is stored.
void inferior_list_message_read_006f4270(Seg33bHost& host, Object message, Object stream) {
    host.session_message_read_header_0075b4c0(message, stream);
    std::uint8_t count = 0;
    host.bit_read_byte_00428c70(stream + 0x4, &count, 8);
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(count); ++i) {
        std::uint16_t id = 0;
        host.bit_read_word_00428e30(stream + 0x4, &id, 0xC);
        word_array_push_back_006f3570(host, message + 0x1C, &id);
    }
}

// 006F4300: resize(0) (00529980) and free the id array, then the root vtable 00CE4974.
void inferior_list_message_destruct_006f4300(Seg33bHost& host, Object message) {
    host.word_array_resize_00529980(message + 0x1C, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(message, 0x1C)));
    at<std::uint32_t>(message, 0x0) = kSessionMessageRootVtable;
}

// 006F4360: the structure fields (00749210), then:
//   +180h = (float)RepairTime (int, default 200);
//   +190h = tuning singleton (004C1D10) +4Ch in game mode 0 ([[00E188A8]+1FE4h] == 0), else
//           +48h (read after the call);
//   +184h = +18Ch = +188h = (float)InferiorRepairTime (int, default 200);
//   +184h = InferiorRepairTimeSingle (float, default 200.0f) when it is a number and the mode
//           is 0;
//   +188h = AirFieldRepairTime and +18Ch = ShipYardRepairTime when each is a number;
//   +194h = (float)CapturePostDamage (int, default 1000); +198h = CameraDistance (150.0f);
//   +19Ch / +1A0h = Min/MaxCameraAngle (-30.0f / 89.0f) / 360.0 * 2pi, rounded to float
//           after the read, after the division and after the product;
//   SubType (a string): "CB0" +1A5h, "RadarStation" +1A4h, "CB3" +1A8h, "CB2" +1A7h, any
//           other string or a non-string +1A6h; the five bytes are cleared first.
// Each field is a fresh 00B67800 lookup destroyed after use (00B67700).
void command_building_class_read_lua_006f4360(Seg33bHost& host, Object klass, Object lua) {
    alignas(4) std::uint8_t field[0x14] = {};
    host.structure_class_read_lua_00749210(klass, lua);

    Object f = host.lua_get_by_name_00b67800(lua, field, "RepairTime");  // 00CFAF54
    at<float>(klass, 0x180) = static_cast<float>(host.lua_get_int_or_default_00b66380(f, 200));
    host.lua_object_destruct_00b67700(field);

    Object game = obj(host.global_u32(kGameGlobals));
    const bool mode_zero = at<std::int32_t>(game, 0x1FE4) == 0;
    Object tuning = host.game_tuning_singleton_004c1d10(game);
    at<float>(klass, 0x190) = mode_zero ? at<float>(tuning, 0x4C) : at<float>(tuning, 0x48);

    f = host.lua_get_by_name_00b67800(lua, field, "InferiorRepairTime");  // 00CFAF40
    const float inferior = static_cast<float>(host.lua_get_int_or_default_00b66380(f, 200));
    at<float>(klass, 0x184) = inferior;
    at<float>(klass, 0x18C) = inferior;
    at<float>(klass, 0x188) = inferior;
    host.lua_object_destruct_00b67700(field);

    f = host.lua_get_by_name_00b67800(lua, field, "InferiorRepairTimeSingle");  // 00CFAF24
    bool single = false;
    if (host.lua_is_number_00b66050(f)) {
        single = at<std::int32_t>(obj(host.global_u32(kGameGlobals)), 0x1FE4) == 0;
    }
    host.lua_object_destruct_00b67700(field);
    if (single) {
        f = host.lua_get_by_name_00b67800(lua, field, "InferiorRepairTimeSingle");
        at<float>(klass, 0x184) = host.lua_get_float_or_default_00b66330(f, kRepairTimeDefault);
        host.lua_object_destruct_00b67700(field);
    }

    f = host.lua_get_by_name_00b67800(lua, field, "AirFieldRepairTime");  // 00CFAF10
    bool present = host.lua_is_number_00b66050(f);
    host.lua_object_destruct_00b67700(field);
    if (present) {
        f = host.lua_get_by_name_00b67800(lua, field, "AirFieldRepairTime");
        at<float>(klass, 0x188) = host.lua_get_number_00b66270(f);
        host.lua_object_destruct_00b67700(field);
    }

    f = host.lua_get_by_name_00b67800(lua, field, "ShipYardRepairTime");  // 00CFAEFC
    present = host.lua_is_number_00b66050(f);
    host.lua_object_destruct_00b67700(field);
    if (present) {
        f = host.lua_get_by_name_00b67800(lua, field, "ShipYardRepairTime");
        at<float>(klass, 0x18C) = host.lua_get_number_00b66270(f);
        host.lua_object_destruct_00b67700(field);
    }

    f = host.lua_get_by_name_00b67800(lua, field, "CapturePostDamage");  // 00CFAEE8
    at<float>(klass, 0x194) = static_cast<float>(host.lua_get_int_or_default_00b66380(f, 1000));
    host.lua_object_destruct_00b67700(field);

    f = host.lua_get_by_name_00b67800(lua, field, "CameraDistance");  // 00CFAED8
    at<float>(klass, 0x198) = host.lua_get_float_or_default_00b66330(f, kCameraDistanceDefault);
    host.lua_object_destruct_00b67700(field);

    f = host.lua_get_by_name_00b67800(lua, field, "MinCameraAngle");  // 00CFAEC8
    float angle = host.lua_get_float_or_default_00b66330(f, kMinCameraAngleDefault);
    angle = static_cast<float>(angle / kDegreesPerTurn);
    at<float>(klass, 0x19C) = static_cast<float>(angle * kTwoPi);
    host.lua_object_destruct_00b67700(field);

    f = host.lua_get_by_name_00b67800(lua, field, "MaxCameraAngle");  // 00CFAEB8
    angle = host.lua_get_float_or_default_00b66330(f, kMaxCameraAngleDefault);
    angle = static_cast<float>(angle / kDegreesPerTurn);
    at<float>(klass, 0x1A0) = static_cast<float>(angle * kTwoPi);
    host.lua_object_destruct_00b67700(field);

    at<std::uint8_t>(klass, 0x1A8) = 0;
    at<std::uint8_t>(klass, 0x1A7) = 0;
    at<std::uint8_t>(klass, 0x1A6) = 0;
    at<std::uint8_t>(klass, 0x1A4) = 0;
    at<std::uint8_t>(klass, 0x1A5) = 0;
    alignas(4) std::uint8_t subtype[0x14] = {};
    f = host.lua_get_by_name_00b67800(lua, subtype, "SubType");  // 00CFA9D0
    if (!host.lua_is_string_00b660a0(subtype)) {
        at<std::uint8_t>(klass, 0x1A6) = 1;
    } else {
        alignas(4) std::uint8_t name[8] = {};  // {length, data}
        host.native_string_assign_0041e870(name, host.lua_get_string_00b662b0(subtype));
        if (host.native_string_equals_insensitive_00425850(name, "CB0")) {  // 00CFAEB0
            at<std::uint8_t>(klass, 0x1A5) = 1;
        } else if (host.native_string_equals_insensitive_00425850(name, "RadarStation")) {  // 00CFAEA0
            at<std::uint8_t>(klass, 0x1A4) = 1;
        } else if (host.native_string_equals_insensitive_00425850(name, "CB3")) {  // 00CFAE9C
            at<std::uint8_t>(klass, 0x1A8) = 1;
        } else if (host.native_string_equals_insensitive_00425850(name, "CB2")) {  // 00CFAE98
            at<std::uint8_t>(klass, 0x1A7) = 1;
        } else {
            at<std::uint8_t>(klass, 0x1A6) = 1;
        }
        void* data = obj(at<std::uint32_t>(name, 0x4));
        if (data != nullptr) host.pool_release_00419cc0_00bd1510(data, at<std::uint32_t>(name, 0x0) + 1);
    }
    host.lua_object_destruct_00b67700(subtype);
    (void)f;
}

// 006F4A00: for every garrison entry (+778h, 64h apart) whose observed unit (+14h) is non-null
// and IsKindOf(kind): build the string "InferiorFailure" (0041DD40(15, 1) then memcpy of
// length + 1 bytes from 00CF0B74), ask the unit's vtable +198h(&string, 0, &flag), free the
// string storage; true at the first unit that set the flag. The image leaves the flag byte
// uninitialised before the call (modelled as 0).
bool command_building_garrison_inferior_failure_006f4a00(Seg33bHost& host, Object building, std::int32_t kind) {
    for (std::int32_t i = 0; i < at<std::int32_t>(building, 0x77C); ++i) {
        Object unit = obj(at<std::uint32_t>(obj(at<std::uint32_t>(building, 0x778)), i * 100 + 0x14));
        if (unit == nullptr || !host.object_is_kind_of_5c(unit, kind)) continue;
        alignas(4) std::uint8_t name[8] = {};  // {length, data}
        host.native_string_resize_0041dd40(name, 0xF, 1);
        void* data = obj(at<std::uint32_t>(name, 0x4));
        if (data != nullptr) {
            host.memcpy_00bf7680(data, "InferiorFailure", at<std::uint32_t>(name, 0x0) + 1);
        }
        std::uint8_t flag = 0;
        host.object_virtual_198(unit, name, 0, &flag);
        data = obj(at<std::uint32_t>(name, 0x4));
        if (data != nullptr) host.pool_release_00419cc0_00bd1510(data, at<std::uint32_t>(name, 0x0) + 1);
        if (flag != 0) return true;
    }
    return false;
}

// 006F4CC0: the land-fort frame update (00745BE0(dt)); then, outside the debug-pause byte
// [[00E188A8]+19C4h], the debug owner is fetched (0051F460) and its +2Ch byte tested with no
// effect on either branch (a stripped debug draw).
void command_building_update_006f4cc0(Seg33bHost& host, Object building, float dt) {
    host.land_fort_frame_update_00745be0(building, dt);
    if (at<std::uint8_t>(obj(host.global_u32(kGameGlobals)), 0x19C4) == 0) {
        (void)at<std::uint8_t>(host.debug_feature_owner_0051f460(), 0x2C);
    }
}

// 006F5460: by message id (+10h): D3h 006F4D10([+1Ch], byte [+20h]); D4h 006F38E0([+1Ch],
// [+20h]); D5h 006F29D0(float [+1Ch], [+20h], [+24h]); D6h the inferior list: for every
// garrison entry with no observed unit, id = message ids[i] (+1Ch array): 0 drops the pair
// (006952A0) and clears +14h; otherwise resolve it (00521E30), observe it (006F34F0) and, when
// observed, hand it this->vtable[12Ch]() through its vtable[128h] and set its +71Ch = this.
// Then refresh the HUD panel (00549430) when this is the selected unit. Others go to the
// land-fort handler (00744BE0). All handled ids answer true.
bool command_building_handle_message_006f5460(Seg33bHost& host, Object building, Object message) {
    switch (at<std::uint8_t>(message, 0x10)) {
    case 0xD3:
        host.command_building_d3_006f4d10(building, at<std::uint32_t>(message, 0x1C), at<std::uint8_t>(message, 0x20));
        return true;
    case 0xD4:
        host.command_building_d4_006f38e0(building, at<std::uint32_t>(message, 0x1C), at<std::uint32_t>(message, 0x20));
        return true;
    case 0xD5:
        host.command_building_d5_006f29d0(building, at<float>(message, 0x1C), at<std::uint32_t>(message, 0x20),
                                          at<std::uint32_t>(message, 0x24));
        return true;
    case 0xD6:
        break;
    default:
        return host.land_fort_handle_message_00744be0(building, message);
    }
    for (std::int32_t i = 0; i < at<std::int32_t>(building, 0x77C); ++i) {
        Object entry = obj(at<std::uint32_t>(building, 0x778)) + i * 100;
        if (at<std::uint32_t>(entry, 0x14) != 0) continue;
        const std::uint16_t id = at<std::uint16_t>(obj(at<std::uint32_t>(message, 0x1C)), i * 2);
        if (id == 0) {
            Object target = obj(at<std::uint32_t>(entry, 0x14));
            if (target != nullptr) {
                host.observer_unregister_006952a0(target, entry);
                at<std::uint32_t>(entry, 0x14) = 0;
            }
            continue;
        }
        observer_member_set_target_006f34f0(host, entry, host.entity_handle_resolve_00521e30(id));
        Object unit = obj(at<std::uint32_t>(obj(at<std::uint32_t>(building, 0x778)) + i * 100, 0x14));
        if (unit != nullptr) {
            host.object_virtual_128(unit, host.object_virtual_12c(building));
            at<std::uint32_t>(obj(at<std::uint32_t>(obj(at<std::uint32_t>(building, 0x778)) + i * 100, 0x14)), 0x71C) =
                addr(building);
        }
    }
    const std::uint32_t hud = host.global_u32(kHudRoot);
    if (hud != 0 && host.hud_selected_unit_00644a60(obj(at<std::uint32_t>(obj(hud), 0x40))) == building) {
        host.hud_panel_refresh_00549430(obj(at<std::uint32_t>(obj(host.global_u32(kHudRoot)), 0x50)));
    }
    return true;
}

// 006F5610: the land-fort base (00745940), the powerup binding at +758h (008E35D0(this)), the
// nine vtables, the garrison vector +778h empty, the +784h list head (0059FD60) with size 0,
// the pad vector +794h empty, +7BCh = 1.0f and +7C0h = -00BD2F10(stream 1)(0, 1) (a shared
// RNG draw at construction), the paratrooper vector +7DCh empty, class id 1Ch, the section
// block +764h (00BD1860), +768h/+770h = 0, +774h = 0.0f, +7D4h = 0, +7D8h = 0.0f, +790h = 0,
// then the fixed-step node at +310h registered (00875A80(1)). Unwinds are not modelled.
Object command_building_construct_006f5610(Seg33bHost& host, Object memory, std::uint32_t argument) {
    host.land_fort_construct_00745940(memory, argument);
    host.powerup_binding_construct_008e35d0(memory + 0x758, memory);
    at<std::uint32_t>(memory, 0x0) = kCommandBuildingVtable;
    at<std::uint32_t>(memory, 0x10) = kCommandBuildingVtable10;
    at<std::uint32_t>(memory, 0x24) = kCommandBuildingVtable24;
    at<std::uint32_t>(memory, 0x170) = kCommandBuildingVtable170;
    at<std::uint32_t>(memory, 0x1E4) = kCommandBuildingVtable1E4;
    at<std::uint32_t>(memory, 0x310) = kCommandBuildingVtable310;
    at<std::uint32_t>(memory, 0x38C) = kCommandBuildingVtable38C;
    at<std::uint32_t>(memory, 0x72C) = kCommandBuildingVtable72C;
    at<std::uint32_t>(memory, 0x758) = kCommandBuildingVtable758;
    at<std::uint32_t>(memory, 0x778) = 0;
    at<std::uint32_t>(memory, 0x77C) = 0;
    at<std::uint32_t>(memory, 0x780) = 0;
    at<std::uint32_t>(memory, 0x788) = addr(host.list_buy_head_0059fd60(memory + 0x784));
    at<std::uint32_t>(memory, 0x78C) = 0;
    at<std::uint32_t>(memory, 0x794) = 0;
    at<std::uint32_t>(memory, 0x798) = 0;
    at<std::uint32_t>(memory, 0x79C) = 0;
    at<float>(memory, 0x7BC) = kOne;
    at<float>(memory, 0x7C0) = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    at<std::uint32_t>(memory, 0x7DC) = 0;
    at<std::uint32_t>(memory, 0x7E0) = 0;
    at<std::uint32_t>(memory, 0x7E4) = 0;
    at<std::uint32_t>(memory, 0xC4) = 0x1C;
    at<std::uint32_t>(memory, 0x764) = addr(host.critical_section_create_00bd1860());
    at<std::uint32_t>(memory, 0x768) = 0;
    at<std::uint32_t>(memory, 0x770) = 0;
    at<float>(memory, 0x774) = 0.0f;
    at<std::uint8_t>(memory, 0x7D4) = 0;
    at<float>(memory, 0x7D8) = 0.0f;
    at<std::uint8_t>(memory, 0x790) = 0;
    host.fixed_step_register_00875a80(memory + 0x310, 1);
    return memory;
}

// 006F5790.
Object land_fort_sub72c_006f5790(Object fort) { return fort != nullptr ? fort + 0x72C : nullptr; }

// 006F57C0.
bool land_fort_has_no_master_006f57c0(Object fort) { return at<std::uint32_t>(fort, 0x738) == 0; }

// 006F57F0: with a master (+738h) tail-jump to its vtable +5Ch with the same argument.
bool land_fort_master_is_kind_of_006f57f0(Seg33bHost& host, Object fort, std::int32_t kind) {
    Object master = obj(at<std::uint32_t>(fort, 0x738));
    if (master != nullptr) return host.object_is_kind_of_5c(master, kind);
    return false;
}

// 006F5980: with +15Ch set, the name at +160h or the empty name 00E19A60 when null; without,
// the class's name ([+538h]+54h).
Object land_fort_name_006f5980(Object fort) {
    if (at<std::uint32_t>(fort, 0x15C) != 0) {
        Object name = obj(at<std::uint32_t>(fort, 0x160));
        return name != nullptr ? name : obj(kLandFortEmptyName);
    }
    return obj(at<std::uint32_t>(obj(at<std::uint32_t>(fort, 0x538)), 0x54));
}

namespace {
// Find the node whose +8h is the unit in the list at registry+offset (first node at
// registry+offset+4h, next +4h) and erase it (004837D0, ECX = registry+offset).
bool erase_from_list(Seg33bHost& host, Object unit, std::size_t offset) {
    Object registry = obj(at<std::uint32_t>(unit, 0x30));
    for (Object node = obj(at<std::uint32_t>(registry, offset + 4)); node != nullptr;
         node = obj(at<std::uint32_t>(node, 0x4))) {
        if (obj(at<std::uint32_t>(node, 0x8)) == unit) {
            host.unit_list_erase_node_004837d0(registry + offset, node);
            return true;
        }
    }
    return false;
}
}  // namespace

// 006F59F0: the early lists (006D3620), then the registry's lists at +54h and +15Ch.
void land_fort_detach_world_lists_006f59f0(Seg33bHost& host, Object fort) {
    host.unit_detach_early_world_lists_006d3620(fort);
    erase_from_list(host, fort, 0x54);
    erase_from_list(host, fort, 0x15C);
}

// 006F5AA0: the land-fort lists (006F59F0), then the registry list at +168h.
void command_building_detach_world_lists_006f5aa0(Seg33bHost& host, Object building) {
    land_fort_detach_world_lists_006f59f0(host, building);
    erase_from_list(host, building, 0x168);
}

// 006F5AE0: the nine vtables restored, the fixed-step node unregistered (00875AE0), the
// section block destroyed (0041CC80 on +764h), the paratrooper vector resized to 0 (006F33A0)
// and freed, the pad buffer freed and cleared, the +784h list tidied (005A0B80), the garrison
// vector resized to 0 (006F4BA0) and freed, the powerup binding (008E9A70), the land fort
// (00745AA0).
void command_building_destruct_006f5ae0(Seg33bHost& host, Object building) {
    at<std::uint32_t>(building, 0x0) = kCommandBuildingVtable;
    at<std::uint32_t>(building, 0x10) = kCommandBuildingVtable10;
    at<std::uint32_t>(building, 0x24) = kCommandBuildingVtable24;
    at<std::uint32_t>(building, 0x170) = kCommandBuildingVtable170;
    at<std::uint32_t>(building, 0x1E4) = kCommandBuildingVtable1E4;
    at<std::uint32_t>(building, 0x310) = kCommandBuildingVtable310;
    at<std::uint32_t>(building, 0x38C) = kCommandBuildingVtable38C;
    at<std::uint32_t>(building, 0x72C) = kCommandBuildingVtable72C;
    at<std::uint32_t>(building, 0x758) = kCommandBuildingVtable758;
    host.fixed_step_unregister_00875ae0(building + 0x310);
    host.critical_section_destroy_owned_0041cc80(building + 0x764);
    pointer_vector_resize_006f33a0(host, building + 0x7DC, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(building, 0x7DC)));
    Object pads = obj(at<std::uint32_t>(building, 0x794));
    if (pads != nullptr) {
        host.free_00bf6989(pads);
        at<std::uint32_t>(building, 0x794) = 0;
    }
    host.list_tidy_005a0b80(building + 0x784);
    garrison_vector_resize_006f4ba0(host, building + 0x778, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(building, 0x778)));
    host.powerup_binding_destruct_008e9a70(building + 0x758);
    host.land_fort_destruct_00745aa0(building);
}

// 006F5C10: operator new(7E8h) (00BF55BE), memset 0, construct (006F5610; null block ->
// null), then 009553D0(descriptor) and +354h = descriptor on the result without a null test,
// as the image does. The unwind that frees the block on a throwing constructor is not modelled.
Object command_building_allocate_006f5c10(Seg33bHost& host, Object descriptor, std::uint32_t argument) {
    Object memory = host.operator_new_00bf55be(kCommandBuildingSize);
    host.memset_00bf79f0(memory, 0, kCommandBuildingSize);
    Object building = memory != nullptr ? command_building_construct_006f5610(host, memory, argument) : nullptr;
    host.unit_set_vehicle_class_009553d0(building, descriptor);
    at<std::uint32_t>(building, 0x354) = addr(descriptor);
    return building;
}

// 006F6740: the land-fort saved state (00748CC0), the garrison/pad adoption (006F5CC0), then
// +7B8h = the party (+54h).
void command_building_init_all_006f6740(Seg33bHost& host, Object building) {
    host.land_fort_load_saved_state_00748cc0(building);
    host.command_building_adopt_nearby_garrison_006f5cc0(building);
    at<std::uint32_t>(building, 0x7B8) = at<std::uint32_t>(building, 0x54);
}

// 006F2780: the land-fort scene init (007482B0), the powerup binding's properties (008ED720
// with the creation record +C0h), then the scene property bag ([+C0h]+8h, a fresh 008F2260
// lookup per field) copied as raw dwords: Level +770h (0), LevelUpSeconds +76Ch (10),
// CaptureRange +7A0h (500), CaptureValue +7A4h (1000), progress +7A8h = 0, LandingRange +7C4h
// (500), InferiorRange +7C8h (200), LandingPointRange +7CCh (500), MinShootingRange +7D0h
// (100); +7B0h = 2, +7B4h = -1; +790h = 1 when the effective game mode (004BCA50, asked
// up to four times) is 0, 1, 2 or 3.
void command_building_init_from_properties_006f2780(Seg33bHost& host, Object building) {
    host.land_fort_init_scene_007482b0(building);
    host.powerup_binding_init_008ed720(building + 0x758, obj(at<std::uint32_t>(building, 0xC0)));
    struct Field {
        const char* name;
        std::size_t offset;
        std::uint32_t fallback;
    };
    static constexpr Field kBefore[] = {
        {"Level", 0x770, 0},             // 00CFAE70
        {"LevelUpSeconds", 0x76C, 10},   // 00CFAE60
        {"CaptureRange", 0x7A0, 500},    // 00CFAE50
        {"CaptureValue", 0x7A4, 1000},
    };
    static constexpr Field kAfter[] = {
        {"LandingRange", 0x7C4, 500},
        {"InferiorRange", 0x7C8, 200},
        {"LandingPointRange", 0x7CC, 500},
        {"MinShootingRange", 0x7D0, 100},
    };
    auto read = [&](const Field& f) {
        Object bag = obj(at<std::uint32_t>(obj(at<std::uint32_t>(building, 0xC0)), 0x8));
        Object entry = host.property_bag_find_008f2260(bag, f.name);
        at<std::uint32_t>(building, f.offset) = entry != nullptr ? at<std::uint32_t>(entry, 0xC) : f.fallback;
    };
    for (const Field& f : kBefore) read(f);
    at<std::uint32_t>(building, 0x7A8) = 0;
    for (const Field& f : kAfter) read(f);
    at<std::int32_t>(building, 0x7B0) = 2;
    at<std::int32_t>(building, 0x7B4) = -1;
    Object game = obj(host.global_u32(kGameGlobals));
    if (host.effective_game_mode_004bca50(game) != 0) {
        game = obj(host.global_u32(kGameGlobals));
        if (host.effective_game_mode_004bca50(game) != 1) {
            game = obj(host.global_u32(kGameGlobals));
            if (host.effective_game_mode_004bca50(game) != 2) {
                game = obj(host.global_u32(kGameGlobals));
                if (host.effective_game_mode_004bca50(game) != 3) return;
            }
        }
    }
    at<std::uint8_t>(building, 0x790) = 1;
}

// 006F7670: once (+44h): a Lua state owner (00B66BD0, opened with 41h), run
// "Scripts\global\luaMW_init.lua" (00CE7CBC) and "Scripts\datatables\CommandBuildingGlobals.lua"
// (00CFB348) (00B69D40(path, 0), each path a pooled string freed after), take the globals
// (00B67980) and their CommandBuildingGlobals table, then read 15 numbers (00B66270, a float):
// HP*, Armor*, Repair* for Basic/Medium/Advanced/Expert at +4h..+10h, +18h..+24h,
// +2Ch..+38h, each divided by 100.0 (double 00D7A220) and rounded to float;
// MultiInvincibleTime +48h, SingleInvincibleTime +4Ch, GunRepairTime +40h as read; then the
// table is destroyed and the state closed (00B669A0). The image divides in x87 extended
// precision; this divides in double.
void command_building_globals_load_006f7670(Seg33bHost& host, Object tuning) {
    if (at<std::uint8_t>(tuning, 0x44) != 0) return;
    at<std::uint8_t>(tuning, 0x44) = 1;
    alignas(4) std::uint8_t owner[0x4DC] = {};
    host.lua_state_construct_00b66bd0(owner);
    host.lua_state_open_00b6a020(owner, 0x41);
    for (const char* path : {"Scripts\\global\\luaMW_init.lua", "Scripts\\datatables\\CommandBuildingGlobals.lua"}) {
        alignas(4) std::uint8_t name[8] = {};  // {length, data}
        host.native_string_assign_0041e870(name, path);
        host.lua_state_run_script_00b69d40(owner, name, 0);
        void* data = obj(at<std::uint32_t>(name, 0x4));
        if (data != nullptr) host.pool_release_00419cc0_00bd1510(data, at<std::uint32_t>(name, 0x0) + 1);
    }
    alignas(4) std::uint8_t globals[0x14] = {};
    alignas(4) std::uint8_t table[0x14] = {};
    alignas(4) std::uint8_t field[0x14] = {};
    Object g = host.lua_state_globals_00b67980(owner, globals);
    host.lua_get_by_name_00b67800(g, table, "CommandBuildingGlobals");  // 00CFB330
    host.lua_object_destruct_00b67700(globals);
    struct Field {
        const char* name;
        std::size_t offset;
        bool percent;
    };
    static constexpr Field kFields[] = {
        {"HPBasic", 0x4, true},          {"HPMedium", 0x8, true},        {"HPAdvanced", 0xC, true},
        {"HPExpert", 0x10, true},        {"ArmorBasic", 0x18, true},     {"ArmorMedium", 0x1C, true},
        {"ArmorAdvanced", 0x20, true},   {"ArmorExpert", 0x24, true},    {"RepairBasic", 0x2C, true},
        {"RepairMedium", 0x30, true},    {"RepairAdvanced", 0x34, true}, {"RepairExpert", 0x38, true},
        {"MultiInvincibleTime", 0x48, false}, {"SingleInvincibleTime", 0x4C, false},
        {"GunRepairTime", 0x40, false},
    };
    for (const Field& f : kFields) {
        Object value = host.lua_get_by_name_00b67800(table, field, f.name);
        const float number = host.lua_get_number_00b66270(value);
        at<float>(tuning, f.offset) = f.percent ? static_cast<float>(number / 100.0) : number;
        host.lua_object_destruct_00b67700(field);
    }
    host.lua_object_destruct_00b67700(table);
    host.lua_state_close_00b669a0(owner);
}

}  // namespace bsp::cc13_seg33b
