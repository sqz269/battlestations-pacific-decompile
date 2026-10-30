// cc13 leftover sweep C reconstructions (packet cc13-sweepC). See include/bsp/cc13_sweepC.hpp for
// the model and docs/CC13_SWEEPC.md for the evidence. Names are hypotheses, not recovered symbols.

#include "bsp/cc13_sweepC.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cstring>
#include <string.h>
#include <initializer_list>
#include <intrin.h>

namespace bsp::cc13_sweepC {

namespace {

// The engine's tracked lock: EnterCriticalSection, then the +18h recursion counter, and the
// reverse. A null section is skipped, as in every inlined copy.
void lock_enter(SweepCHost& host, void* section) {
    if (section != nullptr) {
        host.enter_critical_section(section);
        at<std::int32_t>(section, 0x18) += 1;
    }
}
void lock_leave(SweepCHost& host, void* section) {
    if (section != nullptr) {
        at<std::int32_t>(section, 0x18) -= 1;
        host.leave_critical_section(section);
    }
}

void* slot_at(void* block, int slot) {
    return add(ptr(block, 0x4C), static_cast<std::size_t>(slot) * kSlotStride);
}

// std::list<T*>::push_back on a list object whose head node pointer is at list+4 (006CA730 /
// 006CC760 inline it): buy a node before the head, grow the size, then link it.
void list_push_back(SweepCHost& host, void* list, void* value) {
    void* const head = ptr(list, 0x04);
    void* const node = host.list_buy_node_006bfbd0(list, head, ptr(head, 0x04), &value);
    host.list_incsize_006c89d0(list, 1);
    at<void*>(head, 0x04) = node;
    at<void*>(ptr(node, 0x04), 0x00) = node;
}

}  // namespace

// ---------------------------------------------------------------------------
// Segment 30.
// ---------------------------------------------------------------------------

// 006BF150-006BF1AB, __thiscall(ECX = block, [esp+4] = slot), RET 4. 00696350(ECX = block+7Ch
// owner, EDX = 4, 0), then the 006BD520 slot message is built in a 38h-byte frame buffer
// (ECX = buffer, block, slot) and 0077C7B0(ECX = owner, message, 0) broadcasts it. The image sets
// the EH state to 0 after the build and returns without a destructor call; none is made here.
void air_ops_publish_slot_006bf150(SweepCHost& host, void* block, int slot) {
    host.observer_notify_slot_00696350(ptr(block, 0x7C), 4, 0u);
    alignas(4) std::uint8_t message[0x38];
    void* const built = host.landing_slot_message_construct_006bd520(message, block, slot);
    host.session_owner_broadcast_0077c7b0(ptr(block, 0x7C), built, 0u);
}

// 006BF5C0, __fastcall(ECX = block) -> EAX. -1 when +1Ch or +1Dh is set, +7Ch is null or the
// owner's +5Dh is set; otherwise, under the +4 tracked lock, 0 when +9Ch and +38h are both zero
// and 1 when either is not.
std::int32_t air_ops_block_status_006bf5c0(SweepCHost& host, void* block) {
    std::int32_t status = -1;
    if (at<std::uint8_t>(block, 0x1C) == 0 && at<std::uint8_t>(block, 0x1D) == 0 &&
        ptr(block, 0x7C) != nullptr && at<std::uint8_t>(ptr(block, 0x7C), 0x5D) == 0) {
        void* const section = ptr(block, 0x04);
        lock_enter(host, section);
        status = (at<std::int32_t>(block, 0x9C) == 0 && at<std::int32_t>(block, 0x38) == 0) ? 0 : 1;
        lock_leave(host, section);
    }
    return status;
}

// 006BF650-006BF70F, __thiscall(ECX = block, [esp+4] = byte first, [esp+8] = byte second,
// [esp+0Ch] = int third), RET 0Ch. Compares +E0h, +E1h, +E4h in that order; on any difference
// stores +E4h, +E0h, +E1h (image order), builds the 88h message in a 28h-byte frame buffer
// (0075B430, then +20h/+21h/+24h from the block, +18h word 0, +1Ah byte 0, +4h = 1, +1Ch byte 0,
// vtable 00CF8660) and routes it through 0077C2A0(ECX = block+7Ch, message, 7, 0). No destructor
// call follows in the image.
void air_ops_block_set_orders_006bf650(SweepCHost& host, void* block, std::uint8_t first,
                                       std::uint8_t second, std::int32_t third) {
    if (at<std::uint8_t>(block, 0xE0) == first && at<std::uint8_t>(block, 0xE1) == second &&
        at<std::int32_t>(block, 0xE4) == third) {
        return;
    }
    at<std::int32_t>(block, 0xE4) = third;
    at<std::uint8_t>(block, 0xE0) = first;
    at<std::uint8_t>(block, 0xE1) = second;
    alignas(4) std::uint8_t message[0x28];
    host.session_message_construct_base_0075b430(message, kAirbaseOrdersMessageKind);
    at<std::uint8_t>(message, 0x20) = at<std::uint8_t>(block, 0xE0);
    at<std::uint8_t>(message, 0x21) = at<std::uint8_t>(block, 0xE1);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x00) = kAirbaseOrdersMessageVtable;
    at<std::int32_t>(message, 0x24) = at<std::int32_t>(block, 0xE4);
    host.session_route_message_0077c2a0(ptr(block, 0x7C), message, 7u, 0u);
}

// 006C77E0-006C794F, __fastcall(ECX = block). Under the +8 tracked lock, walks the 14h-record
// vector at +A8h (begin) / +ACh (end) by index and erases, by shifting the tail down one record,
// every record whose plane (word 0) is null or whose plane has +5Ch clear or +5Dh, +60h or +5Eh
// set. The debug-iterator checks call 00BF6713 at the same points as the image (unreachable for
// a consistent vector). Called once per 006CDC70 update (006CDC89).
void air_ops_block_compact_records_006c77e0(SweepCHost& host, void* block) {
    void* const section = ptr(block, 0x08);
    lock_enter(host, section);
    auto begin = [&] { return static_cast<std::uint8_t*>(ptr(block, 0xA8)); };
    auto end = [&] { return static_cast<std::uint8_t*>(ptr(block, 0xAC)); };
    auto size = [&] { return static_cast<std::uint32_t>((end() - begin()) / 0x14); };
    std::uint32_t index = 0;
    std::ptrdiff_t offset = 0;
    for (;;) {
        if (begin() == nullptr || index >= size()) break;                    // 006C7818 / 006C7839
        if (begin() == nullptr || index >= size()) host.invalid_iterator_00bf6713();  // 006C7866
        bool erase = false;
        void* const plane = *reinterpret_cast<void**>(begin() + offset);
        if (plane == nullptr) {                                              // 006C7875
            erase = true;
        } else {
            if (begin() == nullptr || index >= size()) host.invalid_iterator_00bf6713();  // 006C7898
            void* const p = *reinterpret_cast<void**>(begin() + offset);
            erase = at<std::uint8_t>(p, 0x5C) == 0 || at<std::uint8_t>(p, 0x5D) != 0 ||
                    at<std::uint8_t>(p, 0x60) != 0 || at<std::uint8_t>(p, 0x5E) != 0;
        }
        if (erase) {
            if (end() < begin()) host.invalid_iterator_00bf6713();          // 006C78CC
            std::uint8_t* pos = begin() + offset;
            if (end() < pos || pos < begin()) host.invalid_iterator_00bf6713();  // 006C78E3
            std::uint8_t* const last = end();
            std::uint8_t* src = pos;
            while ((src += 0x14) != last) {
                std::memcpy(pos, src, 0x14);
                pos += 0x14;
            }
            at<std::uint8_t*>(block, 0xAC) = end() - 0x14;
            --index;
            offset -= 0x14;
        }
        ++index;
        offset += 0x14;
    }
    lock_leave(host, section);
}

// 006CA730-006CA766, __thiscall(ECX = block, [esp+4] = squadron), RET 4. push_back of the
// squadron on the list object at block+B4h (head node pointer +B8h): 006BFBD0 buys the node
// before the head, 006C89D0(1) grows the size, then the head's prev and the old last node's next
// are linked to it. One caller, the Lua native at 0089FC54.
void air_ops_block_push_b8_006ca730(SweepCHost& host, void* block, void* squadron) {
    list_push_back(host, add(block, 0xB4), squadron);
}

// 006CC5C0-006CC67F, __thiscall(ECX = block, [esp+4] = slot), RET 4. Under the +0Ch tracked
// lock (a scoped lock object in the frame): a slot in state 2 goes to state 1 with timer +30h
// = 0, or 5.0 (00CE3850) with +34h cleared when +34h is set; its +4h class and +8h count go back
// to the stock (006CA770(class, count)), the slot index is removed from the launch list at
// block+10h (006C48F0(&slot)) and 006BF150 publishes the slot. Other states change nothing.
// Callers 006CCEE3 (006CCDA0) and 006CD45D (006CD350).
void air_ops_cancel_queued_slot_006cc5c0(SweepCHost& host, void* block, int slot) {
    void* const section = ptr(block, 0x0C);
    lock_enter(host, section);
    void* const record = slot_at(block, slot);
    if (at<std::int32_t>(record, 0x2C) == 2) {
        at<std::int32_t>(record, 0x2C) = 1;
        at<float>(record, 0x30) = 0.0f;
        if (at<std::uint8_t>(record, 0x34) != 0) {
            at<float>(record, 0x30) = kSlotCooldownSeconds;
            at<std::uint8_t>(record, 0x34) = 0;
        }
        host.air_ops_add_stock_006ca770(block, ptr(record, 0x04), at<std::int32_t>(record, 0x08));
        host.list_remove_006c48f0(add(block, 0x10), &slot);
        air_ops_publish_slot_006bf150(host, block, slot);
    }
    lock_leave(host, section);
}

// 006CC760-006CC7A7, __thiscall(ECX = block, [esp+4] = entity), RET 4. 00694A60(ECX = entity,
// EDX = block+C0h) registers the observer pair, then push_back of the entity on the list object
// at block+D4h (head node pointer +D8h, size +DCh). One caller, 007F1C57 (007F1C00).
void air_ops_push_ready_plane_006cc760(SweepCHost& host, void* block, void* entity) {
    host.observer_register_pair_00694a60(entity, add(block, 0xC0));
    list_push_back(host, add(block, 0xD4), entity);
}

// 006CD160-006CD1C0, __thiscall(ECX = block, [esp+4] = message), RET 4. The target is the
// entity the word handle at message+28h names: 0 for handle 0; below [00F89A10] it is
// [[00F89A54] + (id - [00F89A0C]) x 10h + 0Ch], otherwise [[00F89AA8] + (id - [00F89A60]) x 10h
// + 0Ch] (the engine's two handle tables, read inline). Then 006CCDA0(slot +20h, order +24h,
// target, player +2Ch, 1). The handle compare is signed (JGE) on a zero-extended word.
void air_ops_slot_order_message_006cd160(SweepCHost& host, void* block, void* message) {
    const std::uint16_t handle = at<std::uint16_t>(message, 0x28);
    const std::int32_t player = at<std::int32_t>(message, 0x2C);
    std::uint32_t target = 0;
    if (handle != 0) {
        const std::int32_t id = handle;
        std::uint32_t entry;
        if (id < static_cast<std::int32_t>(host.image_dword(0x00F89A10))) {
            entry = host.image_dword(0x00F89A54) +
                    ((static_cast<std::uint32_t>(id) - host.image_dword(0x00F89A0C)) << 4);
        } else {
            entry = host.image_dword(0x00F89AA8) +
                    ((static_cast<std::uint32_t>(id) - host.image_dword(0x00F89A60)) << 4);
        }
        target = *reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(entry) + 0x0C);
    }
    host.air_ops_slot_command_006ccda0(block, at<std::int32_t>(message, 0x20),
                                       at<std::int32_t>(message, 0x24),
                                       reinterpret_cast<void*>(static_cast<std::uintptr_t>(target)),
                                       player, 1u);
}

// 006CD6C0-006CD7DF, __thiscall(ECX = block, [esp+4] = message), RET 4 -> AL. Switch on the
// message byte +10h less 81h through the table at 006CD7E4 (9 entries):
//   81h 006C47F0(message); 82h 006CD160(message); 83h 006CA8E0([+20h], byte [+24h]);
//   84h 006C83B0(message); 85h every stock node of the list at block+40h (head +44h) whose +8h
//   class equals 007B8A80([+20h]) (0 for a zero id) takes +0Ch = [+24h]; 86h
//   [block+3Ch]->vtable[14h](006BDBF0(message)); 87h answers 0; 88h +E0h/+E1h bytes and +E4h
//   from +20h/+21h/+24h; 89h 006C7490([+20h], 0). Every handled case answers 1, any other 0.
// The 85h walk keeps the image's iterator checks (00BF6713 when the node is the head; the
// container identity test CMP EBX,EBX is always equal and never reports). The class lookup is
// made once per node, as in the image.
bool air_ops_handle_session_message_006cd6c0(SweepCHost& host, void* block, void* message) {
    switch (at<std::uint8_t>(message, 0x10)) {
    case 0x81:
        host.air_ops_apply_slot_setup_006c47f0(block, message);
        return true;
    case 0x82:
        air_ops_slot_order_message_006cd160(host, block, message);
        return true;
    case 0x83:
        host.air_ops_launch_slot_006ca8e0(block, at<std::int32_t>(message, 0x20),
                                          at<std::uint8_t>(message, 0x24));
        return true;
    case 0x84:
        host.air_ops_apply_slot_record_006c83b0(block, message);
        return true;
    case 0x85: {
        for (void* node = ptr(ptr(block, 0x44), 0x00); node != ptr(block, 0x44);
             node = ptr(node, 0x00)) {
            if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();       // 006CD745
            void* const node_class = ptr(node, 0x08);
            const std::uint32_t id = at<std::uint32_t>(message, 0x20);
            void* const wanted = id != 0 ? host.vehicle_class_get_007b8a80(id) : nullptr;
            if (node_class == wanted) {
                const std::int32_t count = at<std::int32_t>(message, 0x24);
                if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();   // 006CD769
                at<std::int32_t>(node, 0x0C) = count;
            }
            if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();       // 006CD776
        }
        return true;
    }
    case 0x86: {
        void* const object = ptr(block, 0x3C);
        host.object_vslot_14(object, host.message_resolve_unit_006bdbf0(message));
        return true;
    }
    case 0x88:
        at<std::uint8_t>(block, 0xE0) = at<std::uint8_t>(message, 0x20);
        at<std::uint8_t>(block, 0xE1) = at<std::uint8_t>(message, 0x21);
        at<std::int32_t>(block, 0xE4) = at<std::int32_t>(message, 0x24);
        return true;
    case 0x89:
        host.air_ops_launch_start_006c7490(block, at<std::int32_t>(message, 0x20), 0u);
        return true;
    default:  // 87h and anything outside 81h..89h
        return false;
    }
}

// 006CEDD0-006CEE53, __fastcall(ECX = site). Walks the occupant pointers at +34h (count +38h).
// An occupant that is non-null and whose +BF4h differs from site->vtable[4Ch]()+80h is
// unregistered (006952A0, ECX = occupant, EDX = site+20h) and erased by shifting the rest down;
// the walk stays on the same index after an erase. Then +1Ch byte = 0. The vtable call is made
// once per non-null occupant, after +BF4h is read.
void air_ops_site_release_mismatched_occupants_006cedd0(SweepCHost& host, void* site) {
    auto data = [&] { return static_cast<void**>(ptr(site, 0x34)); };
    auto count = [&] { return at<std::uint32_t>(site, 0x38); };
    void** it = data();
    if (it != data() + count()) {
        do {
            void* const occupant = *it;
            bool keep = true;
            if (occupant != nullptr) {
                void* const deck = at<void*>(occupant, 0xBF4);
                keep = deck == ptr(host.site_vslot_4c(site), 0x80);
            }
            if (keep) {
                ++it;
            } else {
                host.observer_unregister_pair_006952a0(*it, add(site, 0x20));
                for (void** move = it; move != data() + count() - 1; ++move) move[0] = move[1];
                at<std::uint32_t>(site, 0x38) -= 1;
            }
        } while (it != data() + count());
    }
    at<std::uint8_t>(site, 0x1C) = 0;
}

// 006CFFF0-006D0045, __thiscall(ECX = mother-ship site, flag, plane), RET 8. 00758B90 builds the
// elevator message in a frame buffer (ECX = buffer, flag, plane) and 0077C2A0(ECX = site+B8h,
// message, 5, 0) routes it. No destructor call follows (EH state 0 at return).
void mothership_site_route_elevator_message_006cfff0(SweepCHost& host, void* site,
                                                     std::uint32_t flag, void* plane) {
    alignas(4) std::uint8_t message[0x24];
    void* const built = host.elevator_message_construct_00758b90(message, flag, plane);
    host.session_route_message_0077c2a0(ptr(site, 0xB8), built, 5u, 0u);
}

// 006CE8A0-006CE901, __thiscall(ECX = dst, [esp+4] = src), RET 4 -> dst. +4h, +8h, +0Ch = 0,
// +10h byte = 0, vtable 00CF8900, +14h = src+14h, +10h byte = 1, and when the target is non-null
// 00694A60(ECX = target, EDX = dst) registers the pair.
void* follow_observer_member_copy_construct_006ce8a0(SweepCHost& host, void* dst, void* src) {
    at<std::uint32_t>(dst, 0x04) = 0;
    at<std::uint32_t>(dst, 0x08) = 0;
    at<std::uint32_t>(dst, 0x0C) = 0;
    at<std::uint8_t>(dst, 0x10) = 0;
    at<std::uint32_t>(dst, 0x00) = kFollowObserverVtable;
    void* const target = ptr(src, 0x14);
    at<void*>(dst, 0x14) = target;
    at<std::uint8_t>(dst, 0x10) = 1;
    if (target != nullptr) host.observer_register_pair_00694a60(target, dst);
    return dst;
}

// 006CE910-006CE990, __thiscall(ECX = occupant vector at site+20h, [esp+4] = plane), RET 4.
// 00694A60(ECX = plane, EDX = vector) registers the pair. When count +18h equals capacity +1Ch
// the capacity becomes 2 x capacity + 2, a new buffer of capacity x 4 bytes (00BF55BE) receives
// the old elements (each store skipped for a null destination, as the image tests), the old
// buffer +14h is freed (00BF6989) when non-null and replaced. Then the plane is stored at
// +14h[count] (skipped when that address is null) and the count grows.
void air_ops_site_push_occupant_006ce910(SweepCHost& host, void* occupants, void* plane) {
    host.observer_register_pair_00694a60(plane, occupants);
    std::uint32_t capacity = at<std::uint32_t>(occupants, 0x1C);
    if (at<std::uint32_t>(occupants, 0x18) == capacity) {
        capacity = capacity * 2 + 2;
        at<std::uint32_t>(occupants, 0x1C) = capacity;
        auto* const fresh = static_cast<void**>(host.operator_new_00bf55be(capacity * 4));
        void** out = fresh;
        for (std::uint32_t i = 0; i < at<std::uint32_t>(occupants, 0x18); ++i, ++out) {
            if (out != nullptr) *out = static_cast<void**>(ptr(occupants, 0x14))[i];
        }
        if (ptr(occupants, 0x14) != nullptr) host.operator_delete_00bf6989(ptr(occupants, 0x14));
        at<void*>(occupants, 0x14) = fresh;
    }
    void** const slot =
        static_cast<void**>(ptr(occupants, 0x14)) + at<std::uint32_t>(occupants, 0x18);
    if (slot != nullptr) *slot = plane;
    at<std::uint32_t>(occupants, 0x18) += 1;
}

// 006CEAD0-006CEB34, __thiscall(ECX = archive, [esp+4..8] = key, [esp+0Ch..20h] = observer
// member by value), RET 20h. archive->vtable[0Ch](key, {4, copy+14h}) writes the member's target
// as an object reference, then 006CDD70 destroys the by-value copy (callee-destroyed).
void save_writer_write_observer_ref_006cead0(SweepCHost& host, void* archive, ArchiveArg key,
                                             void* member_copy) {
    host.archive_write_vslot_0c(archive, key, ArchiveArg{4u, at<std::uint32_t>(member_copy, 0x14)});
    host.follow_observer_member_destroy_006cdd70(member_copy);
}

// 006CF0A0-006CF0F3, __fastcall(ECX = archive, EDX = member, [esp+4..8] = key), RET 8.
// archive->vtable[10h](key, {4, &value}) reads an object reference into a frame dword; when it
// differs from member+14h the old non-null target is unregistered (006952A0, ECX = old,
// EDX = member), +14h takes the new value and a non-null one is registered (00694A60).
// The frame dword is the slot of the entry PUSH ECX, so before the read it holds the archive
// pointer; a read that does not write it leaves that value, and it is used as is.
void save_reader_read_observer_ref_006cf0a0(SweepCHost& host, void* archive, void* member,
                                            ArchiveArg key) {
    std::uint32_t value = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(archive));
    host.archive_read_vslot_10(archive, key,
                               ArchiveArg{4u, static_cast<std::uint32_t>(
                                                  reinterpret_cast<std::uintptr_t>(&value))});
    void* const read = reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
    void* const old = ptr(member, 0x14);
    if (old == read) return;
    if (old != nullptr) host.observer_unregister_pair_006952a0(old, member);
    at<void*>(member, 0x14) = read;
    if (read != nullptr) host.observer_register_pair_00694a60(read, member);
}

// 006CF100-006CF173, __thiscall(ECX = site) -> site. vtable 00CF8948; the observer member at
// +4h (vtable 00CF89B4: +8h, +0Ch, +10h, +18h = 0, +14h byte = 1); the occupant vector at +20h
// (vtable 00CF899C: +24h..+2Ch = 0, +30h byte = 0, +34h..+3Ch = 0); +1Ch byte = 0;
// +40h = float([00F876A4] - 99999.0 (double 00CF89D0)): FLD float, FSUB double, FSTP float.
// Written in double, which matches the x87 result under the 53-bit precision control MSVC sets.
void* air_ops_site_construct_006cf100(SweepCHost& host, void* site) {
    at<std::uint32_t>(site, 0x00) = kAirOpsSiteVtable;
    at<std::uint32_t>(site, 0x08) = 0;
    at<std::uint32_t>(site, 0x0C) = 0;
    at<std::uint32_t>(site, 0x10) = 0;
    at<std::uint32_t>(site, 0x18) = 0;
    at<std::uint8_t>(site, 0x14) = 1;
    at<std::uint32_t>(site, 0x04) = kSiteObserverMemberVtable;
    at<std::uint32_t>(site, 0x24) = 0;
    at<std::uint32_t>(site, 0x28) = 0;
    at<std::uint32_t>(site, 0x2C) = 0;
    at<std::uint8_t>(site, 0x30) = 0;
    at<std::uint32_t>(site, 0x34) = 0;
    at<std::uint32_t>(site, 0x38) = 0;
    at<std::uint32_t>(site, 0x3C) = 0;
    at<std::uint32_t>(site, 0x20) = kSiteOccupantVectorVtable;
    const double stamp = static_cast<double>(host.fixed_step_clock_00f876a4()) - kSiteStampOffset;
    at<std::uint8_t>(site, 0x1C) = 0;
    at<float>(site, 0x40) = static_cast<float>(stamp);
    return site;
}

// 006CAC00-006CAD9F, __thiscall(ECX = block, [esp+4] = owner), RET 4 -> block. In image order:
// vtable 00CF88A0; the +10h list (004870F0 head at +14h, size +18h = 0); +1Ch/+1Dh bytes 0;
// +20h = 3; the embedded observer member at +24h (+28h..+30h = 0, +38h = 0, +34h byte 1,
// vtable 00CF6514); +3Ch = 0; the +40h list (00659350 head at +44h, size +48h = 0); +4Ch..+54h
// = 0 (the slot vector); +58h = 0Ch; the observer list at +5Ch (006C4FA0, vtable 00CF8888);
// +7Ch = owner; +80h, +88h..+90h = 0, +94h byte 0, +98h..+A0h = 0, +84h vtable 00CF868C;
// +A8h..+B0h = 0; the +B4h list (006BFA20 head at +B8h, size +BCh = 0); the observer list at
// +C0h (006C4FA0, vtable 00CF8888); +E0h/+E1h bytes 0, +E4h/+E8h = 0; the +ECh tree (006BFD60
// head at +F0h; head+15h byte 1, head's +4h, +0h and +8h point to itself; size +F4h = 0); the
// three tracked sections +4h, +8h, +0Ch from 00BD1860; +F8h = 0.0f.
void* air_ops_block_construct_006cac00(SweepCHost& host, void* block, void* owner) {
    at<std::uint32_t>(block, 0x00) = 0x00CF88A0u;
    at<void*>(block, 0x14) = host.list_head_004870f0(add(block, 0x10));
    at<std::uint32_t>(block, 0x18) = 0;
    at<std::uint8_t>(block, 0x1C) = 0;
    at<std::uint8_t>(block, 0x1D) = 0;
    at<std::uint32_t>(block, 0x20) = 3;
    at<std::uint32_t>(block, 0x28) = 0;
    at<std::uint32_t>(block, 0x2C) = 0;
    at<std::uint32_t>(block, 0x30) = 0;
    at<std::uint32_t>(block, 0x38) = 0;
    at<std::uint8_t>(block, 0x34) = 1;
    at<std::uint32_t>(block, 0x24) = 0x00CF6514u;
    at<std::uint32_t>(block, 0x3C) = 0;
    at<void*>(block, 0x44) = host.list_head_00659350(add(block, 0x40));
    at<std::uint32_t>(block, 0x48) = 0;
    at<std::uint32_t>(block, 0x4C) = 0;
    at<std::uint32_t>(block, 0x50) = 0;
    at<std::uint32_t>(block, 0x54) = 0;
    at<std::uint32_t>(block, 0x58) = 0x0C;
    host.observer_list_construct_006c4fa0(add(block, 0x5C));
    at<std::uint32_t>(block, 0x5C) = 0x00CF8888u;
    at<void*>(block, 0x7C) = owner;
    at<std::uint32_t>(block, 0x80) = 0;
    at<std::uint32_t>(block, 0x88) = 0;
    at<std::uint32_t>(block, 0x8C) = 0;
    at<std::uint32_t>(block, 0x90) = 0;
    at<std::uint8_t>(block, 0x94) = 0;
    at<std::uint32_t>(block, 0x98) = 0;
    at<std::uint32_t>(block, 0x9C) = 0;
    at<std::uint32_t>(block, 0xA0) = 0;
    at<std::uint32_t>(block, 0x84) = 0x00CF868Cu;
    at<std::uint32_t>(block, 0xA8) = 0;
    at<std::uint32_t>(block, 0xAC) = 0;
    at<std::uint32_t>(block, 0xB0) = 0;
    at<void*>(block, 0xB8) = host.list_head_006bfa20(add(block, 0xB4));
    at<std::uint32_t>(block, 0xBC) = 0;
    host.observer_list_construct_006c4fa0(add(block, 0xC0));
    at<std::uint32_t>(block, 0xC0) = 0x00CF8888u;
    at<std::uint8_t>(block, 0xE0) = 0;
    at<std::uint8_t>(block, 0xE1) = 0;
    at<std::uint32_t>(block, 0xE4) = 0;
    at<std::uint32_t>(block, 0xE8) = 0;
    void* const head = host.tree_head_006bfd60(add(block, 0xEC));
    at<void*>(block, 0xF0) = head;
    at<std::uint8_t>(head, 0x15) = 1;
    at<void*>(ptr(block, 0xF0), 0x04) = ptr(block, 0xF0);
    at<void*>(ptr(block, 0xF0), 0x00) = ptr(block, 0xF0);
    at<void*>(ptr(block, 0xF0), 0x08) = ptr(block, 0xF0);
    at<std::uint32_t>(block, 0xF4) = 0;
    at<void*>(block, 0x04) = host.critical_section_create_00bd1860();
    at<void*>(block, 0x08) = host.critical_section_create_00bd1860();
    at<void*>(block, 0x0C) = host.critical_section_create_00bd1860();
    at<float>(block, 0xF8) = 0.0f;
    return block;
}

// 006D1C20-006D1D1C, __thiscall(ECX = airfield, [esp+4] = flag), RET 4 -> airfield.
// 0095CC90(flag, 0) builds the unit base; 006CAC00(airfield) builds the block at +72Ch; the eight
// vtable pointers (+0h 00CF8C08, +10h 00CF8BEC, +24h 00CF8BE4, +170h 00CF8BE0, +1E4h 00CF8BD8,
// +310h 00CF8BC0, +38Ch 00CF8BBC, block +72Ch 00CF8BA0); a 160h-byte collision node from
// 00BF681B (vtable 00CE89E8, 004E6480, +4Ch = airfield, vtable 00CF8B18; null stays null) at
// +828h; +82Ch, the hangar vector +830h..+838h and the aim-point count +884h = 0; +C4h = 45h.
void* airfield_construct_006d1c20(SweepCHost& host, void* airfield, std::uint32_t flag) {
    host.unit_game_object_construct_0095cc90(airfield, flag, 0u);
    air_ops_block_construct_006cac00(host, add(airfield, 0x72C), airfield);
    at<std::uint32_t>(airfield, 0x000) = 0x00CF8C08u;
    at<std::uint32_t>(airfield, 0x010) = 0x00CF8BECu;
    at<std::uint32_t>(airfield, 0x024) = 0x00CF8BE4u;
    at<std::uint32_t>(airfield, 0x170) = 0x00CF8BE0u;
    at<std::uint32_t>(airfield, 0x1E4) = 0x00CF8BD8u;
    at<std::uint32_t>(airfield, 0x310) = 0x00CF8BC0u;
    at<std::uint32_t>(airfield, 0x38C) = 0x00CF8BBCu;
    at<std::uint32_t>(airfield, 0x72C) = 0x00CF8BA0u;
    void* node = host.operator_new_00bf681b(0x160);
    if (node != nullptr) {
        at<std::uint32_t>(node, 0x00) = 0x00CE89E8u;
        host.collision_node_init_004e6480(node);
        at<void*>(node, 0x4C) = airfield;
        at<std::uint32_t>(node, 0x00) = 0x00CF8B18u;
    }
    at<void*>(airfield, 0x828) = node;
    at<std::uint32_t>(airfield, 0x82C) = 0;
    at<std::uint32_t>(airfield, 0x830) = 0;
    at<std::uint32_t>(airfield, 0x834) = 0;
    at<std::uint32_t>(airfield, 0x838) = 0;
    at<std::uint32_t>(airfield, 0x884) = 0;
    at<std::uint32_t>(airfield, 0x0C4) = 0x45;
    return airfield;
}

// 006D3110-006D3190, __thiscall(ECX = descriptor, [esp+4] = flag), RET 4. operator new(8E4h)
// (00BF55BE), memset 0 over 8E4h bytes (00BF79F0, called before the null test, as in the image),
// 006D1C20(flag) when non-null, then 009553D0(ECX = instance, descriptor) and
// instance+354h = descriptor, both made whether or not the allocation succeeded.
void* airfield_allocate_006d3110(SweepCHost& host, void* descriptor, std::uint32_t flag) {
    void* memory = host.operator_new_00bf55be(0x8E4);
    std::memset(memory, 0, 0x8E4);
    void* const instance = memory != nullptr ? airfield_construct_006d1c20(host, memory, flag) : nullptr;
    host.unit_set_vehicle_class_009553d0(instance, descriptor);
    at<void*>(instance, 0x354) = descriptor;
    return instance;
}

// 006D2560-006D25E1, __thiscall(ECX = airfield) -> ST0. 0.0 (FLDZ) when +5Dh is set. Otherwise
// the sum over the 0Ch hangar records +830h (count +834h) of 00923BE0(record[0]), accumulated
// in a float (FADD float, FSTP float each step), divided by the record count (FIDIV) and
// rounded to float. With no record that is 0.0 / 0, which the x87 answers with NaN; the double
// division here gives the same. Each sum step is done in double, which can differ from the
// extended sum only in a double rounding before the float store.
float airfield_health_fraction_006d2560(SweepCHost& host, void* airfield) {
    if (at<std::uint8_t>(airfield, 0x5D) != 0) return 0.0f;
    float sum = 0.0f;
    std::int32_t count = 0;
    for (std::uint8_t* record = static_cast<std::uint8_t*>(ptr(airfield, 0x830));
         record != static_cast<std::uint8_t*>(ptr(airfield, 0x830)) +
                       at<std::uint32_t>(airfield, 0x834) * 0x0C;
         record += 0x0C) {
        ++count;
        sum = static_cast<float>(static_cast<double>(host.unit_health_00923be0(
                                     *reinterpret_cast<void**>(record))) +
                                 static_cast<double>(sum));
    }
    return static_cast<float>(static_cast<double>(sum) / static_cast<double>(count));
}

// 006D3200-006D3244, __thiscall(ECX = airfield, [esp+4] = float dt), RET 4. When +5Dh is clear,
// 006BA670(ECX = airfield+72Ch, dt). Then, game = [00E188A8]: when the current player
// [game + [game+18ECh] x 4 + 18CCh] has +28h equal to the airfield's +54h party,
// 00779C60(ECX = airfield, 1.0f).
void airfield_update_006d3200(SweepCHost& host, void* airfield, float dt) {
    if (at<std::uint8_t>(airfield, 0x5D) == 0) host.air_ops_block_tick_006ba670(add(airfield, 0x72C), dt);
    void* const game = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.image_dword(0x00E188A8)));
    const std::uint32_t index = at<std::uint32_t>(game, 0x18EC);
    void* const player = ptr(game, 0x18CC + static_cast<std::size_t>(index) * 4);
    if (at<std::int32_t>(player, 0x28) == at<std::int32_t>(airfield, 0x54)) {
        host.unit_set_float_00779c60(airfield, 1.0f);
    }
}

// 006D31A0-006D31F8, __thiscall(ECX = airfield). 00878340(airfield); every 0Ch hangar record
// (+830h, count +834h re-read each step) gets record[0]->vtable[1B8h](); +749h and +748h bytes
// = 0; then a tail jump to 006D2980 with ECX = airfield.
void airfield_restore_full_health_006d31a0(SweepCHost& host, void* airfield) {
    host.land_unit_reset_00878340(airfield);
    for (std::uint8_t* record = static_cast<std::uint8_t*>(ptr(airfield, 0x830));
         record != static_cast<std::uint8_t*>(ptr(airfield, 0x830)) +
                       at<std::uint32_t>(airfield, 0x834) * 0x0C;
         record += 0x0C) {
        host.object_vslot_1b8(*reinterpret_cast<void**>(record));
    }
    at<std::uint8_t>(airfield, 0x749) = 0;
    at<std::uint8_t>(airfield, 0x748) = 0;
    host.airfield_rebuild_aim_points_006d2980(airfield);
}

// 006D3620-006D366B, __thiscall(ECX = unit). 00928570(unit); then for each of the world's
// (unit+30h) singly linked lists at +30h (first node +34h) and +48h (first node +4Ch), the first
// node whose +8h is the unit (next at +4h) is erased through 004837D0(ECX = list, node).
void unit_detach_from_early_world_lists_006d3620(SweepCHost& host, void* unit) {
    host.unit_detach_00928570(unit);
    for (const std::size_t list_offset : {std::size_t{0x30}, std::size_t{0x48}}) {
        void* const list = add(ptr(unit, 0x30), list_offset);
        for (void* node = ptr(list, 0x04); node != nullptr; node = ptr(node, 0x04)) {
            if (ptr(node, 0x08) == unit) {
                host.list_erase_node_004837d0(list, node);
                break;
            }
        }
    }
}

// 006CF3E0, __thiscall(ECX = site) -> +44h airfield +72Ch, or null when +44h is null.
void* air_ops_site_airfield_block_006cf3e0(void* site) {
    void* const airfield = ptr(site, 0x44);
    return airfield != nullptr ? add(airfield, 0x72C) : nullptr;
}

// 006D1D30: MOV EAX,1 / RET.
std::uint32_t airfield_sensor_category_006d1d30() { return 1u; }

// 006D1DD0: TEST ECX,ECX; null -> 0, else LEA EAX,[ECX+310h].
void* unit_slot60_sub310_006d1dd0(void* unit) {
    return unit != nullptr ? add(unit, 0x310) : nullptr;
}

// 006D1DF0 and 006D1E30: MOV EAX,[ECX+360h] / RET.
void* unit_model_pointer_360_006d1df0(void* unit) { return ptr(unit, 0x360); }
void* unit_model_pointer_360_006d1e30(void* unit) { return ptr(unit, 0x360); }

// 006D1EB0: ECX = unit+38Ch, JMP [[unit+38Ch]] with the caller's stack argument in place.
std::uint32_t unit_set_skill_level_006d1eb0(SweepCHost& host, void* unit, std::uint32_t level) {
    return host.object_vslot_00(add(unit, 0x38C), level);
}

// ---------------------------------------------------------------------------
// Segment 50.
// ---------------------------------------------------------------------------

namespace {

// The common head every unit message constructor here writes after 0075B430(kind): +18h word 0
// and +1Ah byte 0 (each constructor orders its own stores; the callers below keep that order).
void message_head(void* message) {
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
}

}  // namespace

// 0080DC10-0080DC4A, __thiscall(ECX = message, [esp+4] = unit), RET 4: unit+1134h..+1148h =
// message+20h..+34h, six dwords in order.
void ship_repair_settings_message_apply_0080dc10(void* message, void* unit) {
    for (std::size_t i = 0; i < 6; ++i) {
        at<std::uint32_t>(unit, 0x1134 + i * 4) = at<std::uint32_t>(message, 0x20 + i * 4);
    }
}

// 0080DEC0-0080DF17, __thiscall(ECX = unit). 00BF681B(390h); when non-null 00939CB0(ECX = new,
// unit); unit+1018h = the result or 0.
void unit_create_motion_controller_0080dec0(SweepCHost& host, void* unit) {
    void* const memory = host.operator_new_00bf681b(0x390);
    void* const controller =
        memory != nullptr ? host.motion_controller_construct_00939cb0(memory, unit) : nullptr;
    at<void*>(unit, 0x1018) = controller;
}

// 0080DF80: MOV EAX,6 / RET (no stack argument).
std::uint32_t unit_return_six_0080df80() { return 6u; }

// 0080DF90-0080DFB9, __thiscall(ECX = unit, [esp+4] = level), RET 4. When +3B4h is non-null,
// [unit+3B4h]->vtable[18h](level); then 009565A0(ECX = unit, level).
void unit_skill_set_level_0080df90(SweepCHost& host, void* unit, std::uint32_t level) {
    if (ptr(unit, 0x3B4) != nullptr) host.object_vslot_18(ptr(unit, 0x3B4), level);
    host.unit_set_skill_level_009565a0(unit, level);
}

// 0080E0F0: ECX = [unit+1018h], JMP 0092D730.
float unit_body_axis_speed_0080e0f0(SweepCHost& host, void* unit) {
    return host.motion_body_axis_speed_0092d730(ptr(unit, 0x1018));
}

// 0080E130: MOV EAX,[ECX + category x 4 + 1134h] / RET 4. No bounds check.
std::uint32_t unit_repair_level_0080e130(void* unit, std::int32_t category) {
    return at<std::uint32_t>(unit, 0x1134 + static_cast<std::ptrdiff_t>(category) * 4);
}

// 0080E150: MOV EAX,[ECX+738h] / RET.
void* unit_weapon_director_0080e150(void* unit) { return ptr(unit, 0x738); }

// 0080E1B0: ECX = [unit+738h], JMP 0071BE40.
std::uint32_t unit_current_weapon_command_0080e1b0(SweepCHost& host, void* unit) {
    return host.director_current_command_0071be40(ptr(unit, 0x738));
}

// 0080E290-0080E2B6, __thiscall(ECX = unit). Walks the child chain from +48h through +44h; each
// node answering vtable[5Ch](22h) gets 0085AD00(ECX = node).
void unit_aim_turning_guns_to_rest_0080e290(SweepCHost& host, void* unit) {
    for (void* node = ptr(unit, 0x48); node != nullptr; node = ptr(node, 0x44)) {
        if (host.object_is_kind_of_vslot_5c(node, 0x22)) host.node_release_0085ad00(node);
    }
}

// 0080F960-0080F98D, __thiscall(ECX = message, [esp+4] = byte), RET 4 -> message. 0075B430(7Ah),
// +18h word 0, +1Ah 0, +1Ch 0, +20h = byte, +4h = 1, vtable 00D03310.
void* unit_message_shipyard_launch_construct_0080f960(SweepCHost& host, void* message,
                                                       std::uint8_t state) {
    host.session_message_construct_base_0075b430(message, 0x7A);
    message_head(message);
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint8_t>(message, 0x20) = state;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint32_t>(message, 0x00) = 0x00D03310u;
    return message;
}

// 0080F9F0-0080FA47, __thiscall(ECX = class) -> EAX. 1 when the vector +6E0h..+6E4h holds at
// least one 50h-byte element (signed division, as the image's magic multiply), or the vector
// +700h..+704h holds at least one 1Ch-byte element; a null begin counts as empty.
std::uint32_t vehicle_class_has_breakup_pieces_0080f9f0(void* vehicle_class) {
    const auto first = at<std::int32_t>(vehicle_class, 0x6E0);
    if (first != 0 && (at<std::int32_t>(vehicle_class, 0x6E4) - first) / 0x50 != 0) return 1u;
    const auto second = at<std::int32_t>(vehicle_class, 0x700);
    if (second != 0 && (at<std::int32_t>(vehicle_class, 0x704) - second) / 0x1C != 0) return 1u;
    return 0u;
}

// 0080FA50-0080FA8F, __thiscall(ECX = message, channel, float amount, byte), RET 0Ch -> message.
// 0075B430(9Eh); +18h word 0, +1Ah 0, +20h = byte, +4h = 1, vtable 00D0334C, +1Ch = amount,
// +24h = channel.
void* unit_message_fire_or_water_damage_construct_0080fa50(SweepCHost& host, void* message,
                                                           std::int32_t channel, float amount,
                                                           std::uint8_t flag) {
    host.session_message_construct_base_0075b430(message, 0x9E);
    message_head(message);
    at<std::uint8_t>(message, 0x20) = flag;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint32_t>(message, 0x00) = 0x00D0334Cu;
    at<float>(message, 0x1C) = amount;
    at<std::int32_t>(message, 0x24) = channel;
    return message;
}

// 0080FE10-0080FE40, __thiscall(ECX = message, value), RET 4 -> message. 0075B430(96h), +18h
// word 0, +1Ah 0, +1Ch 0, +20h = value, +4h = 1, vtable 00D033B0.
void* unit_message_torpedo_stock_construct_0080fe10(SweepCHost& host, void* message,
                                                    std::int32_t value) {
    host.session_message_construct_base_0075b430(message, 0x96);
    message_head(message);
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::int32_t>(message, 0x20) = value;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint32_t>(message, 0x00) = 0x00D033B0u;
    return message;
}

// 0080FEC0-0080FF20, __thiscall(ECX = message, unit), RET 4 -> message. 0075B430(6Fh), +4h = 1,
// vtable 00D03478, +18h word 0, +1Ah 0, +1Ch 0, then +20h..+34h = unit+1134h..+1148h.
void* unit_message_repair_settings_construct_0080fec0(SweepCHost& host, void* message, void* unit) {
    host.session_message_construct_base_0075b430(message, 0x6F);
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint32_t>(message, 0x00) = 0x00D03478u;
    message_head(message);
    at<std::uint8_t>(message, 0x1C) = 0;
    for (std::size_t i = 0; i < 6; ++i) {
        at<std::uint32_t>(message, 0x20 + i * 4) = at<std::uint32_t>(unit, 0x1134 + i * 4);
    }
    return message;
}

// 0080FF30-0080FF77, __thiscall(ECX = message, entity, const float3*), RET 8 -> message.
// 0075B430(70h), +18h word 0, +1Ah 0, +4h = 1, vtable 00D0348C, +1Ch word = entity+174h (its
// handle), +20h..+28h = the point (FLD/FSTP float copies).
void* unit_message_kamikaze_detonate_construct_0080ff30(SweepCHost& host, void* message,
                                                        void* entity, const float* point) {
    host.session_message_construct_base_0075b430(message, 0x70);
    message_head(message);
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint32_t>(message, 0x00) = 0x00D0348Cu;
    at<std::uint16_t>(message, 0x1C) = at<std::uint16_t>(entity, 0x174);
    at<float>(message, 0x20) = point[0];
    at<float>(message, 0x24) = point[1];
    at<float>(message, 0x28) = point[2];
    return message;
}

// 0080FF80-0080FFC2, __thiscall(ECX = message, value, const float3*), RET 8 -> message.
// 0075B430(90h), +18h word 0, +1Ah 0, +1Ch = value, +4h = 1, vtable 00D034A0, +20h..+28h = the
// point (float copies).
void* unit_message_hull_impact_construct_0080ff80(SweepCHost& host, void* message,
                                                  std::uint32_t value, const float* point) {
    host.session_message_construct_base_0075b430(message, 0x90);
    message_head(message);
    at<std::uint32_t>(message, 0x1C) = value;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint32_t>(message, 0x00) = 0x00D034A0u;
    at<float>(message, 0x20) = point[0];
    at<float>(message, 0x24) = point[1];
    at<float>(message, 0x28) = point[2];
    return message;
}

// 0080FFD0-0081000B, __thiscall(ECX = message, a, b, c), RET 0Ch -> message. 0075B430(93h),
// +18h word 0, +1Ah 0, +1Ch = a, +20h = b, +4h = 1, vtable 00D034C8, +24h = c.
void* unit_message_add_hull_torque_construct_0080ffd0(SweepCHost& host, void* message,
                                                      std::uint32_t a, std::uint32_t b,
                                                      std::uint32_t c) {
    host.session_message_construct_base_0075b430(message, 0x93);
    message_head(message);
    at<std::uint32_t>(message, 0x1C) = a;
    at<std::uint32_t>(message, 0x20) = b;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint32_t>(message, 0x00) = 0x00D034C8u;
    at<std::uint32_t>(message, 0x24) = c;
    return message;
}

// 00811150-00811172, __thiscall(ECX = unit, float, a, b, c), RET 10h: 00810630(ECX = unit+BD0h,
// float, a, b, c), the float passed through FLD/FSTP.
void unit_wake_point_at_distance_00811150(SweepCHost& host, void* unit, float distance, void* a,
                                          void* b, void* c) {
    host.wake_point_at_distance_00810630(add(unit, 0xBD0), distance, a, b, c);
}

// 00812960-008129D1, __fastcall(ECX = unit) -> EAX. When the director [unit+738h] is non-null,
// its vtable[2Ch]() is non-null and that object answers vtable[5Ch](1Ch), the routine tail-jumps
// to the director's vtable[2Ch] again and returns what it answers (a third call). Otherwise:
// 00414DB0 refreshes the pose when +C8h is clear, then 006F2C30(ECX = unit+FCh, EDX = unit+54h
// party, 2, &frame) answers. The frame dword is the entry PUSH ECX slot, so it starts as the
// unit pointer.
void* unit_landing_target_00812960(SweepCHost& host, void* unit) {
    void* const director = ptr(unit, 0x738);
    if (director != nullptr && host.object_vslot_2c(director) != nullptr &&
        host.object_is_kind_of_vslot_5c(host.object_vslot_2c(ptr(unit, 0x738)), 0x1C)) {
        return host.object_vslot_2c(ptr(unit, 0x738));
    }
    const std::uint32_t party = at<std::uint32_t>(unit, 0x54);
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    void* frame = unit;
    return host.nearest_landing_site_006f2c30(add(unit, 0xFC), party, 2u, &frame);
}

// 00813830-0081389A, __thiscall(ECX = unit). A 7Ah message in the frame (0075B430(7Ah), +18h word
// 0, +1Ah 0, +4h 1, +1Ch 0, vtable 00D03310, +20h byte 0), then 0077C2A0(ECX = unit, msg, 0, 0).
// No destructor call follows.
void unit_route_shipyard_launch_00813830(SweepCHost& host, void* unit) {
    alignas(4) std::uint8_t message[0x24];
    host.session_message_construct_base_0075b430(message, 0x7A);
    message_head(message);
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x00) = 0x00D03310u;
    at<std::uint8_t>(message, 0x20) = 0;
    host.session_route_message_0077c2a0(unit, message, 0u, 0u);
}

// 00813950-008139D1, __thiscall(ECX = unit, [esp+4] = message), RET 4. n = message+22h byte;
// with n != 0, walks the child chain (+48h, next +44h); the i-th child answering vtable[5Ch](22h)
// takes record i at message+24h + i x 44h: record+1Ch = message+20h byte, record+1Dh =
// message+21h byte, record+40h = child+4BCh, 00782790(ECX = record, message+0Ch tick), then
// child->vtable[18Ch](record). The walk stops once i reaches n or the chain ends.
void unit_apply_gun_sync_00813950(SweepCHost& host, void* unit, void* message) {
    if (at<std::uint8_t>(message, 0x22) == 0) return;
    std::int32_t index = 0;
    for (void* child = ptr(unit, 0x48); child != nullptr; child = ptr(child, 0x44)) {
        if (!host.object_is_kind_of_vslot_5c(child, 0x22)) continue;
        void* const record = add(message, 0x24 + static_cast<std::size_t>(index) * 0x44);
        at<std::uint8_t>(record, 0x1C) = at<std::uint8_t>(message, 0x20);
        at<std::uint8_t>(record, 0x1D) = at<std::uint8_t>(message, 0x21);
        at<void*>(record, 0x40) = add(child, 0x4BC);
        host.gun_sync_record_stamp_00782790(record, at<std::uint32_t>(message, 0x0C));
        host.object_vslot_18c(child, record);
        ++index;
        if (index >= static_cast<std::int32_t>(at<std::uint8_t>(message, 0x22))) break;
    }
}

// 00813CA0-00813DAE, __thiscall(ECX = message, unit, three unused dwords), RET 10h -> message.
// 0075B430(8Ch); +18h word 0, +1Ah 0, +4h = 0, vtable 00D02D94; +28h..+30h = unit+6A4h..+6ACh;
// +20h = unit->vtable[50h]() (float); +24h = 0092D700([unit+1018h], scratch)[1]; +34h..+3Ch =
// 0092D6A0([unit+1018h], scratch)[0..2]; +40h/+44h = unit+980h/+984h; +48h = 1 when 0.5
// (00CE3800) > unit+10D0h (COMISS, so NaN gives 0); +4Ah = unit+118Ch when (unit+170h)->vtable[0]()
// answers 2, else 0; +49h = [unit+1018h]+64h byte.
void* unit_state_message_build_00813ca0(SweepCHost& host, void* message, void* unit) {
    host.session_message_construct_base_0075b430(message, 0x8C);
    message_head(message);
    at<std::uint32_t>(message, 0x04) = 0;
    at<std::uint32_t>(message, 0x00) = 0x00D02D94u;
    at<float>(message, 0x28) = at<float>(unit, 0x6A4);
    at<float>(message, 0x2C) = at<float>(unit, 0x6A8);
    at<float>(message, 0x30) = at<float>(unit, 0x6AC);
    at<float>(message, 0x20) = host.object_vslot_50_float(unit);
    alignas(4) std::uint8_t scratch[0x10];
    at<float>(message, 0x24) = host.motion_angular_rate_0092d700(ptr(unit, 0x1018), scratch)[1];
    const float* const velocity = host.motion_velocity_0092d6a0(ptr(unit, 0x1018), scratch);
    at<float>(message, 0x34) = velocity[0];
    at<float>(message, 0x38) = velocity[1];
    at<float>(message, 0x3C) = velocity[2];
    at<float>(message, 0x40) = at<float>(unit, 0x980);
    at<float>(message, 0x44) = at<float>(unit, 0x984);
    at<std::uint8_t>(message, 0x48) = 0.5f > at<float>(unit, 0x10D0) ? 1 : 0;
    at<std::uint8_t>(message, 0x4A) =
        host.object_vslot_00_noarg(add(unit, 0x170)) == 2 ? at<std::uint8_t>(unit, 0x118C) : 0;
    at<std::uint8_t>(message, 0x49) = at<std::uint8_t>(ptr(unit, 0x1018), 0x64);
    return message;
}

namespace {
struct StateField {
    std::size_t offset;
    float range;
    std::uint32_t bits;
};
// The quantized fields, in stream order: positions x/z (24000.0f 00D02F64, 22 bits), heading
// (pi 00D7A264, 16), yaw rate (1.5f 00CE380C, 10), velocity x/z (50.0f 00D09290, 12), throttle
// and rudder (2.0f 00CE3958, 10). The Y components +2Ch and +38h are not sent.
constexpr StateField kStateFields[] = {
    {0x28, 24000.0f, 22}, {0x30, 24000.0f, 22}, {0x20, 3.14159274f, 16}, {0x24, 1.5f, 10},
    {0x34, 50.0f, 12},    {0x3C, 50.0f, 12},    {0x40, 2.0f, 10},        {0x44, 2.0f, 10},
};
constexpr std::size_t kStateFlags[] = {0x4A, 0x48, 0x49};
}  // namespace

// 00813DC0-00813EE4, __thiscall(ECX = message, stream), RET 4. 0075B480(stream), then each field
// through 004295C0(ECX = stream, value, 0, 1, range, bits), then the bytes +4Ah, +48h, +49h
// through 004290B0(ECX = stream, byte).
void unit_state_message_serialize_00813dc0(SweepCHost& host, void* message, void* stream) {
    host.session_message_write_header_0075b480(message, stream);
    for (const StateField& f : kStateFields) {
        host.bit_write_float_004295c0(stream, at<float>(message, f.offset), 0u, 1u, f.range, f.bits);
    }
    for (const std::size_t flag : kStateFlags) host.bit_write_bool_004290b0(stream, at<std::uint8_t>(message, flag));
}

// 00813EF0-00813FFC, __thiscall(ECX = message, stream), RET 4. 0075B4C0(stream), then the same
// fields in the same order through 004293F0(ECX = stream+4, &field, 0, 1, range, bits) and the
// three bytes through 00428D70(ECX = stream+4, &byte).
void unit_state_message_deserialize_00813ef0(SweepCHost& host, void* message, void* stream) {
    host.session_message_read_header_0075b4c0(message, stream);
    void* const cursor = add(stream, 4);
    for (const StateField& f : kStateFields) {
        host.bit_read_float_004293f0(cursor, &at<float>(message, f.offset), 0u, 1u, f.range, f.bits);
    }
    for (const std::size_t flag : kStateFlags) host.bit_read_bool_00428d70(cursor, &at<std::uint8_t>(message, flag));
}

// 008144A0-00814514, __thiscall(ECX = unit). When [00E188A8]+1FE4h is not 2: a 91h message in
// the frame (0075B430(91h), +18h word 0, +4h 1, +1Ah 0, vtable 00D034B4) and 0077C2A0(ECX = unit,
// msg, 7, 0). Unit vtable slot 23Ch.
void unit_route_message_91_008144a0(SweepCHost& host, void* unit) {
    void* const game = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.image_dword(0x00E188A8)));
    if (at<std::int32_t>(game, 0x1FE4) == 2) return;
    alignas(4) std::uint8_t message[0x20];
    host.session_message_construct_base_0075b430(message, 0x91);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x00) = 0x00D034B4u;
    host.session_route_message_0077c2a0(unit, message, 7u, 0u);
}

// 00815010-0081502B, __thiscall(ECX = entity). Null -> 0; else entity->vtable[5Ch](6) true ->
// the entity, false -> 0.
void* entity_narrow_to_class6_00815010(SweepCHost& host, void* entity) {
    if (entity != nullptr && host.object_is_kind_of_vslot_5c(entity, 6)) return entity;
    return nullptr;
}

// 00815850-00815867, __thiscall(ECX = unit) -> EAX. s = unit+104Ch; s < 0 (signed JL) -> s;
// else 00810E90(ECX = unit) + s.
std::int32_t unit_torpedo_stock_total_00815850(SweepCHost& host, void* unit) {
    const std::int32_t spare = at<std::int32_t>(unit, 0x104C);
    if (spare < 0) return spare;
    return host.unit_torpedo_stock_part_00810e90(unit) + spare;
}

// 00815870-008158D0, __thiscall(ECX = ship, [esp+4] = Lua object, [esp+8] = key), RET 8.
// 00779BB0(ECX = ship, lua, key) first; then, when the key's text (key+4) is non-null and
// stricmp (CRT 00BF7FBF) matches "TorpedoStock" (00D09484): s = ship+104Ch; s < 0 pushes s,
// otherwise 00810E90() + s, through 00B664B0(ECX = lua, value).
void ship_get_property_00815870(SweepCHost& host, void* ship, void* lua, void* key) {
    host.unit_get_property_base_00779bb0(ship, lua, key);
    const char* const text = static_cast<const char*>(ptr(key, 0x04));
    if (text == nullptr || ::_stricmp(text, "TorpedoStock") != 0) return;
    const std::int32_t spare = at<std::int32_t>(ship, 0x104C);
    if (spare >= 0) {
        host.lua_push_integer_00b664b0(lua, host.unit_torpedo_stock_part_00810e90(ship) + spare);
        return;
    }
    host.lua_push_integer_00b664b0(lua, spare);
}

// 00817380-008173D2, __thiscall(ECX = unit, byte flag), RET 4. +1008h = flag; then for each part
// pointer in +FFCh (count +1000h, read once before the loop): 00815370(ECX = part, flag ? 0.0f
// : 1.0f (00D7A24C)).
void unit_broadcast_controlled_part_flag_00817380(SweepCHost& host, void* unit, std::uint8_t flag) {
    void** part = static_cast<void**>(ptr(unit, 0xFFC));
    at<std::uint8_t>(unit, 0x1008) = flag;
    void** const end = part + at<std::uint32_t>(unit, 0x1000);
    for (; part != end; ++part) host.part_set_controlled_00815370(*part, flag != 0 ? 0.0f : 1.0f);
}

// 00818300-0081833F, __thiscall(ECX = unit) -> EAX. Over the child chain (+48h, next +44h): a
// node answering vtable[5Ch](20h) whose +3F4h device has +80h == 7 adds its +448h.
std::int32_t unit_torpedo_barrel_count_00818300(SweepCHost& host, void* unit) {
    std::int32_t total = 0;
    for (void* node = ptr(unit, 0x48); node != nullptr; node = ptr(node, 0x44)) {
        if (host.object_is_kind_of_vslot_5c(node, 0x20) &&
            at<std::int32_t>(ptr(node, 0x3F4), 0x80) == 7) {
            total += at<std::int32_t>(node, 0x448);
        }
    }
    return total;
}

// 00819880-00819898, __thiscall(ECX = ship). 0092BD30(ECX = [ship+1018h]), 00818970(ECX = ship),
// then a tail jump to 0095D400 with ECX = ship.
void ship_on_killed_00819880(SweepCHost& host, void* ship) {
    host.motion_controller_on_killed_0092bd30(ptr(ship, 0x1018));
    host.ship_stop_wreck_effects_00818970(ship);
    host.unit_on_killed_base_0095d400(ship);
}

// ---------------------------------------------------------------------------
// Segment 63.
// ---------------------------------------------------------------------------

// 00967FB0-00967FE5, __thiscall(ECX = object) -> EAX. +10h (float, widened with FSTP double),
// the clock [00F876A4] (FSTP double), then FSUBR: clock - vtable[1Ch](); 1 when double(+10h) is
// greater (FCOMIP/JBE, so NaN gives 0).
std::uint32_t timer_not_elapsed_vslot_1c_00967fb0(SweepCHost& host, void* object) {
    const double limit = at<float>(object, 0x10);
    const double clock = host.fixed_step_clock_00f876a4();
    const double elapsed = clock - host.object_vslot_1c_double(object);
    return limit > elapsed ? 1u : 0u;
}

// 00967FF0-00968025: the same with vtable[20h].
std::uint32_t timer_not_elapsed_vslot_20_00967ff0(SweepCHost& host, void* object) {
    const double limit = at<float>(object, 0x10);
    const double clock = host.fixed_step_clock_00f876a4();
    const double elapsed = clock - host.object_vslot_20_double(object);
    return limit > elapsed ? 1u : 0u;
}

// ---------------------------------------------------------------------------
// Segment 64.
// ---------------------------------------------------------------------------

// 0096A8D0-0096A934: the stack shape of 006CEAD0 with a different member type. archive->
// vtable[0Ch](key, {4, copy+14h}), then 004B7EF0 destroys the by-value copy (callee-destroyed).
void save_writer_write_member_ref_0096a8d0(SweepCHost& host, void* archive, ArchiveArg key,
                                           void* member_copy) {
    host.archive_write_vslot_0c(archive, key, ArchiveArg{4u, at<std::uint32_t>(member_copy, 0x14)});
    host.member_destroy_004b7ef0(member_copy);
}

namespace {
// The reset all four copies inline: p = owner+offset; when non-null, InterlockedDecrement(p+4)
// (00CE2220) reaching 0 calls p->vtable[0](); then owner+offset = 0 (only when p was non-null).
void refptr_reset(SweepCHost& host, void* owner, std::size_t offset) {
    void* const object = ptr(owner, offset);
    if (object == nullptr) return;
    if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(object, 0x04)) == 0) {
        host.refcounted_destroy_vslot_00(object);
    }
    at<void*>(owner, offset) = nullptr;
}
}  // namespace

// 0096C4A0-0096C4CA (+8h; reached from the unwind funclet 00CAAA20).
void refptr_reset_08_0096c4a0(SweepCHost& host, void* owner) { refptr_reset(host, owner, 0x08); }
// 0096C4D0-0096C4FA (+0Ch; reached from the unwind funclet 00CAAA28).
void refptr_reset_0c_0096c4d0(SweepCHost& host, void* owner) { refptr_reset(host, owner, 0x0C); }
// 0096CD00-0096CD2A (+18h; no caller or vtable slot).
void refptr_reset_18_0096cd00(SweepCHost& host, void* owner) { refptr_reset(host, owner, 0x18); }
// 0096EC90-0096ECBA (+18h; no caller or vtable slot; byte-identical to 0096CD00 apart from its
// address).
void refptr_reset_18_0096ec90(SweepCHost& host, void* owner) { refptr_reset(host, owner, 0x18); }

// 0096CE00-0096CE60, __thiscall(ECX = object) -> AL. v = ([00E188D8]+170h)->vtable[0](); walks
// the list at +28h (head node pointer +2Ch) from head->next; 1 at the first node whose +8h is
// v, 0 at the head. The checked-iterator reports (00BF6713) are kept where the image makes them;
// the container identity test (CMP ESI,ESI) is always equal.
bool list_holds_current_player_key_0096ce00(SweepCHost& host, void* object) {
    void* const player = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.image_dword(0x00E188D8)));
    const std::uint32_t key = host.object_vslot_00_noarg(add(player, 0x170));
    void* const list = add(object, 0x28);
    void* node = ptr(ptr(list, 0x04), 0x00);
    void* const head = ptr(list, 0x04);
    for (;;) {
        if (node == head) return false;
        if (node == ptr(list, 0x04)) host.invalid_iterator_00bf6713();          // 0096CE3B
        if (at<std::uint32_t>(node, 0x08) == key) return true;
        if (node == ptr(list, 0x04)) host.invalid_iterator_00bf6713();          // 0096CE4A
        node = ptr(node, 0x00);
    }
}

// 00968060-00968084, __thiscall(ECX = box, [esp+4] = entity), RET 4 -> box. vtable 00D1AF24;
// +4h word = entity+174h, or 0 for a null entity.
void* hit_channel_entity_key_construct_00968060(void* box, void* entity) {
    at<std::uint32_t>(box, 0x00) = 0x00D1AF24u;
    at<std::uint16_t>(box, 0x04) = entity != nullptr ? at<std::uint16_t>(entity, 0x174) : 0;
    return box;
}

// 009681D0-009681F6, RET 4 -> EAX: 1 when entity+5Ch is set and +5Dh, +60h, +5Eh are clear.
std::uint32_t unit_is_active_for_hit_channel_009681d0(void* entity) {
    return at<std::uint8_t>(entity, 0x5C) != 0 && at<std::uint8_t>(entity, 0x5D) == 0 &&
                   at<std::uint8_t>(entity, 0x60) == 0 && at<std::uint8_t>(entity, 0x5E) == 0
               ? 1u
               : 0u;
}

// 00968230-0096825B, __thiscall(ECX = message, byte a, byte b), RET 8 -> message.
// 0075B430(E5h), +18h = a, +4h = 0, vtable 00D03324, +19h = b.
void* session_message_e5_construct_00968230(SweepCHost& host, void* message, std::uint8_t a,
                                            std::uint8_t b) {
    host.session_message_construct_base_0075b430(message, 0xE5);
    at<std::uint8_t>(message, 0x18) = a;
    at<std::uint32_t>(message, 0x04) = 0;
    at<std::uint32_t>(message, 0x00) = 0x00D03324u;
    at<std::uint8_t>(message, 0x19) = b;
    return message;
}

// 00968260-00968284, __thiscall(ECX = message, value), RET 4 -> message. 0075B430(E6h),
// +18h = value, +4h = 0, vtable 00D03338.
void* session_message_e6_construct_00968260(SweepCHost& host, void* message, std::uint32_t value) {
    host.session_message_construct_base_0075b430(message, 0xE6);
    at<std::uint32_t>(message, 0x18) = value;
    at<std::uint32_t>(message, 0x04) = 0;
    at<std::uint32_t>(message, 0x00) = 0x00D03338u;
    return message;
}

// 009683A0-009683F1, __thiscall(ECX = unit) -> EAX. Null -> 0. p = [00E188D8] (read after the
// virtual call). When unit->vtable[5Ch](0Fh) answers true the test is unit+9D4h == p, otherwise
// unit == p; equal -> 1. Otherwise o = unit+284h; null -> 0; 1 when o equals (p ? p+284h : 0).
std::uint32_t unit_is_current_player_or_ally_009683a0(SweepCHost& host, void* unit) {
    if (unit == nullptr) return 0u;
    const bool kind_f = host.object_is_kind_of_vslot_5c(unit, 0x0F);
    void* const player = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.image_dword(0x00E188D8)));
    const bool same = kind_f ? ptr(unit, 0x9D4) == player : unit == player;
    if (same) return 1u;
    void* const owner = ptr(unit, 0x284);
    if (owner == nullptr) return 0u;
    void* const player_owner = player != nullptr ? ptr(player, 0x284) : nullptr;
    return owner == player_owner ? 1u : 0u;
}

namespace {

// The inline checked index every matcher makes before key k: a null begin (+4h) or k at or past
// the dword count ((+8h - +4h) >> 2, signed shift then unsigned compare) reports through 00BF6713;
// the read still happens.
std::uint32_t checked_param(SweepCHost& host, void* params, std::uint32_t k) {
    const auto begin = at<std::int32_t>(params, 0x04);
    if (begin == 0 ||
        static_cast<std::uint32_t>((at<std::int32_t>(params, 0x08) - begin) >> 2) <= k) {
        host.invalid_iterator_00bf6713();
    }
    return static_cast<const std::uint32_t*>(ptr(params, 0x04))[k];
}

// The shared shape of the 48- and 67-instruction matchers: filter k is the object embedded at
// subscription+0Ch+k*10h; each must accept params[k], in order, stopping at the first refusal.
std::uint32_t match_keys(SweepCHost& host, void* subscription, void* params, std::uint32_t keys) {
    for (std::uint32_t k = 0; k < keys; ++k) {
        const std::uint32_t value = checked_param(host, params, k);
        if (!host.filter_accepts_vslot_00(add(subscription, 0x0C + k * 0x10), value)) return 0u;
    }
    return 1u;
}

}  // namespace

// Two-key matchers (one 48-instruction shape, checked by stripping addresses: 00968710,
// 009688C0, 00968940, 009689C0, 00968BD0, 00968C50, 00968D80, 00968E00): filters +0Ch, +1Ch.
std::uint32_t leak_subscription_matches_009688c0(SweepCHost& host, void* subscription, void* params) {
    return match_keys(host, subscription, params, 2);
}
std::uint32_t fire_subscription_matches_00968940(SweepCHost& host, void* subscription, void* params) {
    return match_keys(host, subscription, params, 2);
}
std::uint32_t repair_subscription_matches_009689c0(SweepCHost& host, void* subscription, void* params) {
    return match_keys(host, subscription, params, 2);
}
std::uint32_t target_subscription_matches_00968bd0(SweepCHost& host, void* subscription, void* params) {
    return match_keys(host, subscription, params, 2);
}
std::uint32_t ammo_type_subscription_matches_00968c50(SweepCHost& host, void* subscription, void* params) {
    return match_keys(host, subscription, params, 2);
}
std::uint32_t gui_subscription_matches_00968d80(SweepCHost& host, void* subscription, void* params) {
    return match_keys(host, subscription, params, 2);
}
std::uint32_t generate_subscription_matches_00968e00(SweepCHost& host, void* subscription, void* params) {
    return match_keys(host, subscription, params, 2);
}

// Three-key matchers (one 67-instruction shape: 00968810, 00968A40, 00968CD0): +0Ch, +1Ch, +2Ch.
std::uint32_t failure_subscription_matches_00968810(SweepCHost& host, void* subscription, void* params) {
    return match_keys(host, subscription, params, 3);
}
std::uint32_t zone_subscription_matches_00968a40(SweepCHost& host, void* subscription, void* params) {
    return match_keys(host, subscription, params, 3);
}
std::uint32_t stock_subscription_matches_00968cd0(SweepCHost& host, void* subscription, void* params) {
    return match_keys(host, subscription, params, 3);
}

// 009685B0, the hit matcher (vtable 00D1B74C slot 3). Keys 0..3 inline against +0Ch, +1Ch, +2Ch,
// +3Ch; keys 4..7 read through 009658B0(params, k) against the filters at +4Ch, +5Ch, +68h, +74h.
std::uint32_t hit_subscription_matches_009685b0(SweepCHost& host, void* subscription, void* params) {
    if (match_keys(host, subscription, params, 4) == 0) return 0u;
    constexpr std::size_t kTail[] = {0x4C, 0x5C, 0x68, 0x74};
    for (std::uint32_t i = 0; i < 4; ++i) {
        const std::uint32_t value = *host.vector_at_009658b0(params, 4 + i);
        if (!host.filter_accepts_vslot_00(add(subscription, kTail[i]), value)) return 0u;
    }
    return 1u;
}

// ---------------------------------------------------------------------------
// Segment 71.
// ---------------------------------------------------------------------------

// 00A371C0-00A371C7: CALL 004C1C50, MOV ECX,EAX, JMP 00A335D0.
void ai_globals_reload_00a371c0(SweepCHost& host) {
    host.ai_globals_load_00a335d0(host.ai_globals_owner_004c1c50());
}

// 00A37250-00A3729F, RET 14h: the 14h-byte Lua object is passed by value (the callee owns the
// copy). 00888AA0(ECX = &copy) -> entity; the answer is entity+16Ch, read before 00B67700
// destroys the copy.
std::uint32_t ai_group_from_lua_entity_00a37250(SweepCHost& host, void* lua_object_copy) {
    void* const entity = host.lua_object_to_entity_00888aa0(lua_object_copy);
    const std::uint32_t group = at<std::uint32_t>(entity, 0x16C);
    host.lua_object_destroy_00b67700(lua_object_copy);
    return group;
}

// 00A430E0-00A43129, RET 8 -> AL. 00A3EAC0(ECX = [00F8ABE8]) is called and its answer
// dropped, then called again: above 1 -> 1. Otherwise 00A4D5E4(00A3EAC0(), a, b, &out), the
// third 00A3EAC0 call made between the argument loads, and the answer is out != 0. The frame
// dword `out` is the entry PUSH ECX slot, so it starts as ECX; ECX is not otherwise read.
bool xlive_query_identifier_policy_00a430e0(SweepCHost& host, std::uint32_t ecx, std::uint32_t a,
                                            std::uint32_t b) {
    void* const xlive = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.image_dword(0x00F8ABE8)));
    host.xlive_user_index_00a3eac0(xlive);
    if (host.xlive_user_index_00a3eac0(xlive) > 1) return true;
    std::uint32_t out = ecx;
    const std::uint32_t user = host.xlive_user_index_00a3eac0(
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.image_dword(0x00F8ABE8))));
    host.xlive_check_privilege_00a4d5e4(user, a, b, &out);
    return out != 0;
}

// 00A43180-00A43213, __thiscall(ECX = client). When [00E188A8]+1FE4h is non-zero and
// 00A3E510(ECX = [00F8ABE8]) answers true: for each of the eight 118h-byte player records at
// game+748h (game re-read each pass), one with +9h clear and +8h set has its XUID (+80h, +84h)
// tested by client->vtable[24h](lo, hi), and 00771630(ECX = game+1EF0h, lo, hi, v) records the
// answer (taken as 0 or 1; the image stores whatever AL holds). v is the frame dword (the entry PUSH ECX slot, so its upper three bytes are those of
// the client pointer) with the answer stored in its low byte.
void xlive_refresh_participant_policy_00a43180(SweepCHost& host, void* client) {
    void* const game0 = reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(host.image_dword(0x00E188A8)));
    if (at<std::int32_t>(game0, 0x1FE4) == 0) return;
    if (!host.xlive_signed_in_00a3e510(reinterpret_cast<void*>(
            static_cast<std::uintptr_t>(host.image_dword(0x00F8ABE8))))) {
        return;
    }
    std::uint32_t frame = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(client));
    for (std::size_t offset = 0; offset < 0x8C0; offset += 0x118) {
        void* const game = reinterpret_cast<void*>(
            static_cast<std::uintptr_t>(host.image_dword(0x00E188A8)));
        void* const record = add(game, offset + 0x748);
        if (record == nullptr || at<std::uint8_t>(record, 0x09) != 0 ||
            at<std::uint8_t>(record, 0x08) == 0) {
            continue;
        }
        const std::uint32_t hi = at<std::uint32_t>(record, 0x84);
        const std::uint32_t lo = at<std::uint32_t>(record, 0x80);
        const bool allowed = host.object_vslot_24_xuid(client, lo, hi);
        frame = (frame & 0xFFFFFF00u) | (allowed ? 1u : 0u);
        host.session_set_participant_policy_00771630(
            add(reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.image_dword(0x00E188A8))),
                0x1EF0),
            lo, hi, frame);
    }
}

// ---------------------------------------------------------------------------
// Segment 73.
// ---------------------------------------------------------------------------

// 00A776B0, __thiscall(ECX = record, i), RET 4: +10h = i; +8h = [00E12EF0 + 4i] (the channel
// layout table {0, 1, 2, 6}, read through the host as image data; no bounds check).
void dialog_channel_record_set_speaker_layout_00a776b0(SweepCHost& host, void* record,
                                                       std::int32_t layout) {
    at<std::int32_t>(record, 0x10) = layout;
    at<std::uint32_t>(record, 0x08) =
        host.image_dword(0x00E12EF0u + static_cast<std::uint32_t>(layout) * 4u);
}

// 00A776D0: state +20h not 0 and not 3 -> 1.
std::uint32_t sound_stream_is_active_00a776d0(void* stream) {
    const auto state = at<std::int32_t>(stream, 0x20);
    return state != 0 && state != 3 ? 1u : 0u;
}

// 00A776F0: state +20h == 3.
std::uint32_t sound_stream_is_over_00a776f0(void* stream) {
    return at<std::int32_t>(stream, 0x20) == 3 ? 1u : 0u;
}

// 00A77700, RET 4: byte +4Eh = value.  00A77710: byte +4Eh.  00A77720: dword +50h.
void streamed_dialog_logical_set_byte_4e_00a77700(void* logical, std::uint8_t value) {
    at<std::uint8_t>(logical, 0x4E) = value;
}
std::uint8_t streamed_dialog_logical_get_byte_4e_00a77710(void* logical) {
    return at<std::uint8_t>(logical, 0x4E);
}
std::uint32_t streamed_dialog_logical_get_category_00a77720(void* logical) {
    return at<std::uint32_t>(logical, 0x50);
}

// 00A77730: logical = +0Ch; 1 when its byte +4Ch is set, or its stream +4h is non-null with a
// state other than 0 and 3.
std::uint32_t streamed_dialog_manager_is_dialog_playing_00a77730(void* manager) {
    void* const logical = ptr(manager, 0x0C);
    if (at<std::uint8_t>(logical, 0x4C) != 0) return 1u;
    void* const stream = ptr(logical, 0x04);
    if (stream == nullptr) return 0u;
    return sound_stream_is_active_00a776d0(stream);
}

// 00A77760, RET 4: +215h = paused; for the two logical channels at +8h and +0Ch, a non-null
// stream (+4h) gets 00A85C40(ECX = stream, +215h) (the byte re-read each time).
void streamed_dialog_manager_set_all_paused_00a77760(SweepCHost& host, void* manager,
                                                     std::uint8_t paused) {
    at<std::uint8_t>(manager, 0x215) = paused;
    for (std::size_t i = 0; i < 2; ++i) {
        void* const stream = ptr(ptr(manager, 0x08 + i * 4), 0x04);
        if (stream != nullptr) {
            host.sound_stream_set_paused_00a85c40(stream, at<std::uint8_t>(manager, 0x215));
        }
    }
}

// 00A777A0: y starts at 20h; for the two logical channels at +8h and +0Ch, a non-null stream
// gets 00A869B0(ECX = stream, 190h, y) and y becomes its answer (EAX carries it to the next push).
void streamed_dialog_manager_draw_debug_00a777a0(SweepCHost& host, void* manager) {
    std::uint32_t y = 0x20;
    for (std::size_t i = 0; i < 2; ++i) {
        void* const stream = ptr(ptr(manager, 0x08 + i * 4), 0x04);
        if (stream != nullptr) y = host.sound_stream_draw_debug_00a869b0(stream, 0x190, y);
    }
}

// 00A77840, RET 8: [manager + channel x 4 + 8h] byte +4Eh = value (no bounds check).
void streamed_dialog_manager_set_channel_byte_4e_00a77840(void* manager, std::int32_t channel,
                                                          std::uint8_t value) {
    at<std::uint8_t>(ptr(manager, 0x08 + static_cast<std::ptrdiff_t>(channel) * 4), 0x4E) = value;
}

// 00A77860, RET 4: d = float(+220h - level) (FLD/FSUB float, FSTP float); +224h = level (stored
// first); +228h = |float(d x 0.5 (double 00D7A280))| (the sign bit cleared with AND 7FFFFFFFh).
void streamed_dialog_manager_set_target_level_00a77860(void* manager, float level) {
    const float d = static_cast<float>(static_cast<double>(at<float>(manager, 0x220)) -
                                       static_cast<double>(level));
    at<float>(manager, 0x224) = level;
    float half = static_cast<float>(static_cast<double>(d) * 0.5);
    std::uint32_t bits;
    std::memcpy(&bits, &half, 4);
    bits &= 0x7FFFFFFFu;
    std::memcpy(&half, &bits, 4);
    at<float>(manager, 0x228) = half;
}

// 00A77B30, __thiscall(ECX = slot, value), RET 4 -> slot. Releases the old non-null pointer
// (InterlockedDecrement(+4) == 0 -> vtable[0]()), writes 0 then the new value; the new value is
// not add-ref'd here.
void* streamed_dialog_refptr_assign_00a77b30(SweepCHost& host, void* slot, void* value) {
    void* const old = ptr(slot, 0x00);
    if (old != nullptr) {
        if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(old, 0x04)) == 0) {
            host.refcounted_destroy_vslot_00(old);
        }
        at<void*>(slot, 0x00) = nullptr;
    }
    at<void*>(slot, 0x00) = value;
    return slot;
}

// 00A77B70: releases +10h as 0096C4A0 does, then stores 0 to +10h a second time (both stores are
// in the image).
void dialog_channel_record_release_ref_10_00a77b70(SweepCHost& host, void* record) {
    refptr_reset(host, record, 0x10);
    at<void*>(record, 0x10) = nullptr;
}

// 00A77EC0: delete[] of the 8-byte pair array at [owner]: 00BF7C6E(array, 8, [array-4],
// 00A77D10), operator delete (00BF6989) on array-4, [owner] = 0. Null does nothing.
void streamed_dialog_physical_pair_array_delete_00a77ec0(SweepCHost& host, void* owner) {
    void* const array = ptr(owner, 0x00);
    if (array == nullptr) return;
    void* const cookie = static_cast<std::uint8_t*>(array) - 4;
    host.eh_vector_destructor_iterator_00bf7c6e(array, 8, at<std::uint32_t>(cookie, 0), 0x00A77D10u);
    host.operator_delete_00bf6989(cookie);
    at<void*>(owner, 0x00) = nullptr;
}

// 00A77F80 -> record. The NativeString at +0h starts {0, null}, +10h = 0, then 0041DD40(0, 0)
// and, when its data (+4h) is non-null, a copy of the empty string 00CE3A0C of length [+0h]
// (00BF7680, the CRT memcpy); +8h = 0.
void* dialog_channel_record_construct_00a77f80(SweepCHost& host, void* record) {
    at<std::uint32_t>(record, 0x00) = 0;
    at<std::uint32_t>(record, 0x04) = 0;
    at<std::uint32_t>(record, 0x10) = 0;
    host.native_string_resize_0041dd40(record, 0, 0);
    if (ptr(record, 0x04) != nullptr) {
        std::memcpy(ptr(record, 0x04), "", at<std::uint32_t>(record, 0x00));
    }
    at<std::uint32_t>(record, 0x08) = 0;
    return record;
}

// 00A782E0, RET 4: 00A86710(ECX = +4h stream, name); then, unless name is the +8h string itself,
// +8h is resized to the name's length (0041DD40(len, 1)) and, for a non-empty name, the name's
// data copied over with the CRT memcpy (00BF7680) for +8h's new length.
void music_director_select_row_and_store_name_00a782e0(SweepCHost& host, void* director,
                                                      const void* name) {
    host.sound_stream_select_row_00a86710(ptr(director, 0x04), name);
    void* const stored = add(director, 0x08);
    if (stored == name) return;
    void* const source = const_cast<void*>(name);
    host.native_string_resize_0041dd40(stored, at<std::uint32_t>(source, 0x00), 1);
    if (at<std::uint32_t>(source, 0x00) != 0) {
        std::memcpy(ptr(stored, 0x04), ptr(source, 0x04), at<std::uint32_t>(stored, 0x00));
    }
}

// 00A78620. With [+0Ch]+4h stream present: 00A77FF0(ECX = stream, &local) gives its name; the
// log 004254B0("cMusicManager::EndDialogStream %s" 00D58EAC, data or "" (00F8BBD0)); the local
// string is released (pooled, len + 1 bytes) when its data is non-null; then a tail jump to
// 00A86F40 with ECX = the stream (re-read). Without a stream nothing happens.
void streamed_dialog_manager_end_dialog_stream_00a78620(SweepCHost& host, void* manager) {
    if (ptr(ptr(manager, 0x0C), 0x04) == nullptr) return;
    alignas(4) std::uint8_t local[8] = {};
    const void* const name = host.sound_stream_name_00a77ff0(ptr(ptr(manager, 0x0C), 0x04), local);
    const char* const text = static_cast<const char*>(ptr(const_cast<void*>(name), 0x04));
    host.log_printf_004254b0("cMusicManager::EndDialogStream %s", text != nullptr ? text : "");
    if (ptr(local, 0x04) != nullptr) {
        host.string_pool_release_00419cc0_00bd1510(ptr(local, 0x04), at<std::uint32_t>(local, 0) + 1);
    }
    host.sound_stream_request_stop_00a86f40(ptr(ptr(manager, 0x0C), 0x04));
}

// 00A78F80: [owner] non-null -> 00A780B0(ECX = it), operator delete (00BF65AC), [owner] = 0.
void streamed_dialog_logical_delete_00a78f80(SweepCHost& host, void* owner) {
    void* const object = ptr(owner, 0x00);
    if (object == nullptr) return;
    host.dialog_manager_destroy_00a780b0(object);
    host.operator_delete_00bf65ac(object);
    at<void*>(owner, 0x00) = nullptr;
}

// 00A790F0: 00A78EC0(ECX = records, 0), then operator delete (00BF6989) on [records], which is
// not cleared.
void dialog_channel_records_destroy_00a790f0(SweepCHost& host, void* records) {
    host.dialog_channel_records_resize_00a78ec0(records, 0);
    host.operator_delete_00bf6989(ptr(records, 0x00));
}

// ---------------------------------------------------------------------------
// Segment 74.
// ---------------------------------------------------------------------------

// 00A82E20 and 00A82E40 (byte-identical shapes): data = +4h; non-null -> 00419CC0(data, +0h + 1,
// 1) then 00BD1510 on the pool it answers. The fields are not cleared.
void native_string_destroy_00a82e20(SweepCHost& host, void* string) {
    if (ptr(string, 0x04) != nullptr) {
        host.string_pool_release_00419cc0_00bd1510(ptr(string, 0x04), at<std::uint32_t>(string, 0) + 1);
    }
}
void native_string_destroy_00a82e40(SweepCHost& host, void* string) {
    if (ptr(string, 0x04) != nullptr) {
        host.string_pool_release_00419cc0_00bd1510(ptr(string, 0x04), at<std::uint32_t>(string, 0) + 1);
    }
}

// 00A92A20-00A92A94, __thiscall(ECX = action, float dt), RET 4. Nothing unless byte +1h is set.
// 00A92370(action); c = +2Ch; with c non-null: b = (+28h byte set and +24h > 0.0), and
// 00A91A50(ECX = c, dt, a, b) with a = 1 when +20h byte is set and +1Ch > 0.0, else 0
// (COMISS against 0.0, so NaN gives 0).
void input_action_update_00a92a20(SweepCHost& host, void* action, float dt) {
    if (at<std::uint8_t>(action, 0x01) == 0) return;
    host.input_action_poll_00a92370(action);
    void* const control = ptr(action, 0x2C);
    if (control == nullptr) return;
    const std::uint32_t b = at<std::uint8_t>(action, 0x28) != 0 && at<float>(action, 0x24) > 0.0f ? 1u : 0u;
    const std::uint32_t a = at<std::uint8_t>(action, 0x20) != 0 && at<float>(action, 0x1C) > 0.0f ? 1u : 0u;
    host.input_control_update_00a91a50(control, dt, a, b);
}

// 00A943D0-00A9440F -> device. vtable 00CEB130, +4h = 1, vtable 00D5B638, then 100h zero bytes at
// +0Ch and at +10Ch (00BF79F0, the CRT memset).
void* input_device_construct_00a943d0(void* device) {
    at<std::uint32_t>(device, 0x00) = 0x00CEB130u;
    at<std::uint32_t>(device, 0x04) = 1;
    at<std::uint32_t>(device, 0x00) = 0x00D5B638u;
    std::memset(add(device, 0x0C), 0, 0x100);
    std::memset(add(device, 0x10C), 0, 0x100);
    return device;
}

// 00A9A0B0-00A9A0EF -> device. vtable 00CEB130, +4h = 1, the two 100h memsets, then vtable
// 00D5B850 (stored after the memsets here, unlike 00A943D0).
void* mouse_device_construct_00a9a0b0(void* device) {
    at<std::uint32_t>(device, 0x00) = 0x00CEB130u;
    at<std::uint32_t>(device, 0x04) = 1;
    std::memset(add(device, 0x0C), 0, 0x100);
    std::memset(add(device, 0x10C), 0, 0x100);
    at<std::uint32_t>(device, 0x00) = 0x00D5B850u;
    return device;
}

// 00A97B30-00A97B6D, __thiscall(ECX = backend). vtable 00D5B72C; operator delete (00BF65AC) of
// +E8h when non-null; +E8h, +ECh, +F0h = 0; tail jump to 00A90E00.
void native_input_backend_destroy_00a97b30(SweepCHost& host, void* backend) {
    at<std::uint32_t>(backend, 0x00) = 0x00D5B72Cu;
    if (ptr(backend, 0xE8) != nullptr) host.operator_delete_00bf65ac(ptr(backend, 0xE8));
    at<std::uint32_t>(backend, 0xE8) = 0;
    at<std::uint32_t>(backend, 0xEC) = 0;
    at<std::uint32_t>(backend, 0xF0) = 0;
    host.input_backend_base_destroy_00a90e00(backend);
}

// ---------------------------------------------------------------------------
// Segments 75, 77, 79.
// ---------------------------------------------------------------------------

// 00A9E970-00A9E9B0, __thiscall(ECX = pool, value), RET 4. +7800h word = 400h; for i in 0..3FFh:
// word +7000h[i] = 3FFh - i (the free-index stack) and dword +18h + i x 1Ch = value.
void string_map_node_pool_init_block_00a9e970(void* pool, std::uint32_t value) {
    const auto store = [pool](std::uint32_t offset, auto field) {
        _ReadWriteBarrier();
        std::memcpy(static_cast<unsigned char*>(pool) + offset, &field, sizeof(field));
        _ReadWriteBarrier();
    };
    store(0x7800u, std::uint16_t{0x400});
    for (std::uint32_t i = 0; i < 0x400; ++i) {
        store(0x7000u + i * 2u, static_cast<std::uint16_t>(0x3ffu - i));
        store(0x18u + i * 0x1cu, value);
    }
}

// 00AA6810: ECX = [widget+4Ch], JMP 00B6F960 (the caller's stack argument passes through).
void gui_widget_assign_node_name_00aa6810(SweepCHost& host, void* widget, const void* name) {
    host.node_assign_name_00b6f960(ptr(widget, 0x4C), name);
}

// 00AA7880: PUSH ECX, MOV ECX,00F8BC94, CALL 00AA75F0, RET: the incoming ECX is the block.
void gui_widget_pool_free_thunk_00aa7880(SweepCHost& host, void* block) {
    host.gui_widget_pool_free_00aa75f0(0x00F8BC94u, block);
}

// 00AA7CF0: MOV ECX,00F8BC94, JMP 00AA7A80.
void* gui_widget_pool_allocate_stub_00aa7cf0(SweepCHost& host, std::uint32_t size) {
    return host.gui_widget_pool_allocate_00aa7a80(0x00F8BC94u, size);
}

namespace {

// The GUI type-descriptor initializer shared by 00ACEA20/00ACEA70 and 00AD0780/00AD07D0: when the
// guard byte is clear it is set, +0Ch = the class name, 00AA8200(ECX = 00F8BC88) inits the base
// descriptor, +4h/+8h = its two dwords [00F8BC88]/[00F8BC8C], and +0h = the counter object's +4h,
// which is then incremented.
void gui_type_id_init(SweepCHost& host, void* descriptor, std::uint32_t guard, std::uint32_t name) {
    if (host.image_byte(guard) != 0) return;
    host.set_image_byte(guard, 1);
    at<std::uint32_t>(descriptor, 0x0C) = name;
    host.gui_type_base_init_00aa8200(0x00F8BC88u);
    at<std::uint32_t>(descriptor, 0x04) = host.image_dword(0x00F8BC88u);
    at<std::uint32_t>(descriptor, 0x08) = host.image_dword(0x00F8BC8Cu);
    void* const counter = host.type_id_counter_006fac20();
    const std::uint32_t id = at<std::uint32_t>(counter, 0x04);
    at<std::uint32_t>(counter, 0x04) = id + 1;
    at<std::uint32_t>(descriptor, 0x00) = id;
}

// 00AACE40's shape (data +0h, count +4h, capacity +8h): capacity = 2 x capacity + 2; a new
// block of capacity x 4 bytes (00BF55BE) takes the elements (each store skipped for a null
// destination, as tested); the old block is freed (00BF6989) when non-null; +0h = the new block.
void pointer_table_grow(SweepCHost& host, void* table) {
    const std::uint32_t capacity = at<std::uint32_t>(table, 0x08) * 2 + 2;
    at<std::uint32_t>(table, 0x08) = capacity;
    auto* const fresh = static_cast<void**>(host.operator_new_00bf55be(capacity * 4));
    void** out = fresh;
    for (std::uint32_t i = 0; i < at<std::uint32_t>(table, 0x04); ++i, ++out) {
        if (out != nullptr) *out = static_cast<void**>(ptr(table, 0x00))[i];
    }
    if (ptr(table, 0x00) != nullptr) host.operator_delete_00bf6989(ptr(table, 0x00));
    at<void*>(table, 0x00) = fresh;
}

}  // namespace

// 00AA9350-00AA938F -> descriptor. Guard 00F8BC80: +8h = "cGuiElement" (00D5C124), 00BEA780(ECX =
// 0109DB84), +4h = [0109DB84], +0h = the next id from 006FAC20's counter.
void* gui_element_type_id_init_return_00aa9350(SweepCHost& host, void* descriptor) {
    if (host.image_byte(0x00F8BC80u) == 0) {
        host.set_image_byte(0x00F8BC80u, 1);
        at<std::uint32_t>(descriptor, 0x08) = 0x00D5C124u;
        host.type_base_init_00bea780(0x0109DB84u);
        at<std::uint32_t>(descriptor, 0x04) = host.image_dword(0x0109DB84u);
        void* const counter = host.type_id_counter_006fac20();
        const std::uint32_t id = at<std::uint32_t>(counter, 0x04);
        at<std::uint32_t>(counter, 0x04) = id + 1;
        at<std::uint32_t>(descriptor, 0x00) = id;
    }
    return descriptor;
}

// 00ACEA20 / 00ACEA70: guard 00F8C138, "cGuiClipBox" (00D5D0E4).
void gui_clip_box_type_id_init_00acea20(SweepCHost& host, void* descriptor) {
    gui_type_id_init(host, descriptor, 0x00F8C138u, 0x00D5D0E4u);
}
void* gui_clip_box_type_id_init_return_00acea70(SweepCHost& host, void* descriptor) {
    gui_type_id_init(host, descriptor, 0x00F8C138u, 0x00D5D0E4u);
    return descriptor;
}

// 00AD0780 / 00AD07D0: guard 00F8C184, the FrameBox name at 00D5D120.
void gui_frame_box_type_id_init_00ad0780(SweepCHost& host, void* descriptor) {
    gui_type_id_init(host, descriptor, 0x00F8C184u, 0x00D5D120u);
}
void* gui_frame_box_type_id_init_return_00ad07d0(SweepCHost& host, void* descriptor) {
    gui_type_id_init(host, descriptor, 0x00F8C184u, 0x00D5D120u);
    return descriptor;
}

// 00ACC4A0-00ACC4DA: the +F0h ref-pointer release (as 0096C4A0), then +ECh = 0 unconditionally.
void gui_model_release_refs_00acc4a0(SweepCHost& host, void* model) {
    refptr_reset(host, model, 0xF0);
    at<std::uint32_t>(model, 0xEC) = 0;
}

// 00ACBEC0-00ACBF6A, RET 8. +F4h..+FCh = scale, +100h..+108h = euler (float copies). A frame
// block holds {euler copy from +100h..+108h, 0, 0, 0}; 00B64470(ECX = frame matrix, EDX = the
// three zeros, the euler copy, this+F4h) composes the matrix and [+ECh]->vtable[38h](matrix)
// takes it (the vtable read from the +ECh object before the compose).
void gui_model_set_transform_vectors_00acbec0(SweepCHost& host, void* model, const float* scale,
                                              const float* euler) {
    at<float>(model, 0xF4) = scale[0];
    at<float>(model, 0xF8) = scale[1];
    at<float>(model, 0xFC) = scale[2];
    at<float>(model, 0x100) = euler[0];
    at<float>(model, 0x104) = euler[1];
    at<float>(model, 0x108) = euler[2];
    float block[6] = {at<float>(model, 0x100), at<float>(model, 0x104), at<float>(model, 0x108),
                      0.0f, 0.0f, 0.0f};
    alignas(4) std::uint8_t matrix[0x40];
    void* const out = host.matrix_compose_00b64470(matrix, &block[3], &block[0],
                                                   static_cast<const float*>(add(model, 0xF4)));
    host.object_vslot_38(ptr(model, 0xEC), out);
}

// 00ACF1D0-00ACF24E, RET 4 (vtable 00D5D160 slot). +18h/+1Ch = position (float copies),
// 00AA7220(box); when the state word +ECh is not FFFFh, i = its sign-extended value: the
// checked index into the 2Ch-byte records at +F4h..+F8h (00BF6713 when the begin is null or i is
// at or past the count, unsigned compare), record i +0Ch/+10h = position, then box->vtable[88h]().
void gui_frame_box_set_position_00acf1d0(SweepCHost& host, void* box, const float* position) {
    at<float>(box, 0x18) = position[0];
    at<float>(box, 0x1C) = position[1];
    host.gui_widget_position_changed_00aa7220(box);
    const std::uint16_t word = at<std::uint16_t>(box, 0xEC);
    if (word == 0xFFFF) return;
    const auto index = static_cast<std::int32_t>(static_cast<std::int16_t>(word));
    const auto begin = at<std::int32_t>(box, 0xF4);
    if (begin == 0 ||
        static_cast<std::uint32_t>(index) >=
            static_cast<std::uint32_t>((at<std::int32_t>(box, 0xF8) - begin) / 0x2C)) {
        host.invalid_iterator_00bf6713();
    }
    void* const record = add(ptr(box, 0xF4), static_cast<std::size_t>(index) * 0x2C);
    at<float>(record, 0x0C) = position[0];
    at<float>(record, 0x10) = position[1];
    host.object_vslot_88(box);
}

// 00AD0680-00AD070F, RET 8. Checked index of `state` into the +F4h records (as above); record
// word 0 takes the texture as a ref pointer: when it differs, store, add-ref the new non-null one
// (InterlockedIncrement +4h) and release the old non-null one (decrement, vtable[0] at zero).
// Then, when state equals the sign-extended +ECh word, box->vtable[80h](state).
void gui_frame_box_set_state_texture_00ad0680(SweepCHost& host, void* box, std::uint32_t state,
                                              void* texture) {
    const auto begin = at<std::int32_t>(box, 0xF4);
    if (begin == 0 ||
        state >= static_cast<std::uint32_t>((at<std::int32_t>(box, 0xF8) - begin) / 0x2C)) {
        host.invalid_iterator_00bf6713();
    }
    void* const record = add(ptr(box, 0xF4), static_cast<std::size_t>(state) * 0x2C);
    void* const old = ptr(record, 0x00);
    if (old != texture) {
        at<void*>(record, 0x00) = texture;
        if (texture != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(texture, 0x04));
        if (old != nullptr && host.interlocked_decrement_00ce2220(&at<std::int32_t>(old, 0x04)) == 0) {
            host.refcounted_destroy_vslot_00(old);
        }
    }
    if (static_cast<std::int32_t>(state) ==
        static_cast<std::int32_t>(static_cast<std::int16_t>(at<std::uint16_t>(box, 0xEC)))) {
        host.object_vslot_80(box, static_cast<std::int32_t>(state));
    }
}

// 00AD0D00-00AD0D75, RET 4 -> dst. +0h dword copy; the NativeString at +4h assigned unless it is
// the source's own (0041DD40(len, 1) then the CRT memcpy for the new length when the source is
// non-empty); +0Ch..+18h float copies; +1Ch..+28h dword copies.
void* gui_record_2c_assign_00ad0d00(SweepCHost& host, void* dst, const void* src_const) {
    void* const src = const_cast<void*>(src_const);
    at<std::uint32_t>(dst, 0x00) = at<std::uint32_t>(src, 0x00);
    void* const d = add(dst, 0x04);
    void* const s = add(src, 0x04);
    if (d != s) {
        host.native_string_resize_0041dd40(d, at<std::uint32_t>(s, 0x00), 1);
        if (at<std::uint32_t>(s, 0x00) != 0) {
            std::memcpy(ptr(d, 0x04), ptr(s, 0x04), at<std::uint32_t>(d, 0x00));
        }
    }
    for (std::size_t off = 0x0C; off <= 0x18; off += 4) at<float>(dst, off) = at<float>(src, off);
    for (std::size_t off = 0x1C; off <= 0x28; off += 4) {
        at<std::uint32_t>(dst, off) = at<std::uint32_t>(src, off);
    }
    return dst;
}

// 00AD2D50-00AD2D96 -> icon. 00AB5C60(icon); +140h..+142h bytes = 1; vtable 00D5BCE0; +138h =
// 30.0f (00CE38C8); +13Ch = 0; +60h = 0Ch.
void* gui_anim_icon_construct_00ad2d50(SweepCHost& host, void* icon) {
    host.gui_icon_construct_00ab5c60(icon);
    at<std::uint8_t>(icon, 0x140) = 1;
    at<std::uint8_t>(icon, 0x141) = 1;
    at<std::uint8_t>(icon, 0x142) = 1;
    at<std::uint32_t>(icon, 0x00) = 0x00D5BCE0u;
    at<float>(icon, 0x138) = 30.0f;
    at<std::uint32_t>(icon, 0x13C) = 0;
    at<std::uint32_t>(icon, 0x60) = 0x0C;
    return icon;
}

// ---------------------------------------------------------------------------
// Segments 90, 91.
// ---------------------------------------------------------------------------

// 00B87170 / 00B871A0 / 00B871D0: MOV EAX,ECX; [EAX] = the factory vtable; RET.
void* plain_node_factory_destruct_00b87170(void* factory) {
    at<std::uint32_t>(factory, 0) = 0x00D63210u;
    return factory;
}
void* model_factory_destruct_00b871a0(void* factory) {
    at<std::uint32_t>(factory, 0) = 0x00D63218u;
    return factory;
}
void* group_factory_destruct_00b871d0(void* factory) {
    at<std::uint32_t>(factory, 0) = 0x00D63220u;
    return factory;
}

namespace {
// 00B87180 / 00B871B0 / 00B871E0 (one shape): vtable 00D63208 (the base), operator delete
// (00BF65AC) when flags bit 0 is set; RET 4 -> this.
void* factory_scalar_delete(SweepCHost& host, void* factory, std::uint32_t flags) {
    at<std::uint32_t>(factory, 0) = 0x00D63208u;
    if ((flags & 1u) != 0) host.operator_delete_00bf65ac(factory);
    return factory;
}

// 00B87710 / 00B87750 (one shape, differing in the growth call): count +4h == capacity +8h ->
// grow to max(capacity + 10h, 10h) (signed); then data[count] = *item (skipped for a null
// address) and count += 1.
void item_array_push_back(SweepCHost& host, void* array, const std::uint32_t* item, bool hierarchy) {
    std::int32_t capacity = at<std::int32_t>(array, 0x08);
    if (at<std::int32_t>(array, 0x04) == capacity) {
        capacity += 0x10;
        if (capacity <= 0x10) capacity = 0x10;
        if (hierarchy) {
            host.hierarchy_item_array_reserve_00b87350(array, capacity);
        } else {
            host.resource_item_array_reserve_00b872f0(array, capacity);
        }
    }
    auto* const slot = static_cast<std::uint32_t*>(ptr(array, 0x00)) + at<std::int32_t>(array, 0x04);
    if (slot != nullptr) *slot = *item;
    at<std::int32_t>(array, 0x04) += 1;
}
}  // namespace

void* plain_node_factory_scalar_delete_00b87180(SweepCHost& host, void* factory, std::uint32_t flags) {
    return factory_scalar_delete(host, factory, flags);
}
void* model_factory_scalar_delete_00b871b0(SweepCHost& host, void* factory, std::uint32_t flags) {
    return factory_scalar_delete(host, factory, flags);
}
void* group_factory_scalar_delete_00b871e0(SweepCHost& host, void* factory, std::uint32_t flags) {
    return factory_scalar_delete(host, factory, flags);
}

// 00B876F0: PUSH ECX, ECX = 0109022C, CALL 00B17AF0, RET: the incoming ECX is the slot.
void return_native_88_byte_pool_slot_00b876f0(SweepCHost& host, void* slot) {
    host.native_pool_return_00b17af0(0x0109022Cu, slot);
}

void resource_item_array_push_back_00b87710(SweepCHost& host, void* array, const std::uint32_t* item) {
    item_array_push_back(host, array, item, false);
}
void hierarchy_item_array_push_back_00b87750(SweepCHost& host, void* array, const std::uint32_t* item) {
    item_array_push_back(host, array, item, true);
}

// 00B877F0 / 00B87800: PUSH 0, CALL 00B873C0 / 00B87410 with ECX = the array, RET.
void resource_item_array_clear_00b877f0(SweepCHost& host, void* array) {
    host.resource_item_array_resize_00b873c0(array, 0);
}
void hierarchy_item_array_clear_00b87800(SweepCHost& host, void* array) {
    host.hierarchy_item_array_resize_00b87410(array, 0);
}

// 00B88B00, RET 4 -> node: +0h = src+0h; 00B887D0(ECX = node+4h, src+4h).
void* resource_node_copy_construct_00b88b00(SweepCHost& host, void* node, const void* src) {
    at<std::uint32_t>(node, 0x00) = *static_cast<const std::uint32_t*>(src);
    host.resource_name_copy_00b887d0(add(node, 0x04), static_cast<const std::uint8_t*>(src) + 4);
    return node;
}

// 00B88CB0, RET 8 -> node: +0h = *key; 00B887D0(ECX = node+4h, name).
void* resource_node_construct_from_00b88cb0(SweepCHost& host, void* node, const std::uint32_t* key,
                                            const void* name) {
    at<std::uint32_t>(node, 0x00) = *key;
    host.resource_name_copy_00b887d0(add(node, 0x04), name);
    return node;
}

// 00B88D00, RET 14h -> node: +0h/+4h/+8h = a/b/c; +0Ch = keyed_name+0h; 00B887D0(ECX = node+10h,
// keyed_name+4h); +20h = flag; +21h = 0.
void* resource_node_construct_00b88d00(SweepCHost& host, void* node, std::uint32_t a,
                                       std::uint32_t b, std::uint32_t c, const void* keyed_name,
                                       std::uint8_t flag) {
    at<std::uint32_t>(node, 0x00) = a;
    at<std::uint32_t>(node, 0x04) = b;
    at<std::uint32_t>(node, 0x08) = c;
    at<std::uint32_t>(node, 0x0C) = *static_cast<const std::uint32_t*>(keyed_name);
    host.resource_name_copy_00b887d0(add(node, 0x10), static_cast<const std::uint8_t*>(keyed_name) + 4);
    at<std::uint8_t>(node, 0x20) = flag;
    at<std::uint8_t>(node, 0x21) = 0;
    return node;
}

// 00B89FE0 -> instance or 0: 00BF681B(3Ch); non-null -> 00B89F20(ECX = block, resource).
void* resource_create_instance_00b89fe0(SweepCHost& host, void* resource) {
    void* const block = host.operator_new_00bf681b(0x3C);
    return block != nullptr ? host.resource_instance_construct_00b89f20(block, resource) : nullptr;
}

// 00B8A200, RET 4 -> camera or 0: 00B71930(ECX = 458h); non-null -> 00B71A80(ECX = block, arg).
void* camera_pool_create_camera_00b8a200(SweepCHost& host, std::uint32_t arg) {
    void* const block = host.camera_pool_allocate_00b71930(0x458);
    return block != nullptr ? host.camera_construct_00b71a80(block, arg) : nullptr;
}

// 00B999A0, RET 4 -> ST0. s = float((p.y*n.y + p.x*n.x) + p.z*n.z) (FMUL, FMUL, FADDP, FMUL,
// FADDP, FSTP float), then float(s - plane+0Ch). cc13-fixture6: modelled per x87 operation at
// 24-bit precision control, the precision of the main thread (no reference in the image; judged
// with its render family). n is copied through FLD / FSTP DWORD first; that only quiets a
// signalling NaN, which the products then propagate the same way.
float plane_signed_distance_00b999a0(const void* plane, const float* point) {
    using namespace cc13x87;
    const auto* const n = static_cast<const float*>(plane);
    const double sum = x87_add24(x87_add24(x87_mul24(point[1], n[1]), x87_mul24(point[0], n[0])),
                                 x87_mul24(point[2], n[2]));
    const float s = static_cast<float>(sum);
    return static_cast<float>(x87_sub24(s, n[3]));
}

// 00AACE40, 00ACBE50, 00ACD250, 00ACEFC0: one 35-instruction shape (address-stripped compare).
void pointer_table_grow_00aace40(SweepCHost& host, void* table) { pointer_table_grow(host, table); }
void pointer_table_grow_00acbe50(SweepCHost& host, void* table) { pointer_table_grow(host, table); }
void pointer_table_grow_00acd250(SweepCHost& host, void* table) { pointer_table_grow(host, table); }
void pointer_table_grow_00acefc0(SweepCHost& host, void* table) { pointer_table_grow(host, table); }

}  // namespace bsp::cc13_sweepC
