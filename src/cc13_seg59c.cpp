// Segment 59 leftovers, packet cc13-seg59b (marker cc13_seg59c). See include/bsp/cc13_seg59c.hpp
// and docs/CC13_SEG59C.md. Not bound into bsp_game.

#include "bsp/cc13_seg59c.hpp"

namespace bsp::cc13_seg59c {

void damage_control_note_unit_in_slot_list_0090e6c0(Seg59cHost& host, void* manager, void* unit) {
    // 0090E6C5 MOVZX EBX,word [unit+174h]; 0090E6CC the slot, unsigned compare against 7.
    const std::uint16_t id = at<std::uint16_t>(unit, kUnitIdOffset);
    const std::uint32_t slot = at<std::uint32_t>(unit, kUnitSlotOffset);
    if (slot >= kSlotCount) return;                                   // 0090E6D9 JA
    void* list = add(manager, slot * kSlotRecordStride + kSlotUnitListOffset);  // 0090E6E5
    for (std::uint32_t i = 0;; ++i) {
        const auto* first = at<const std::uint16_t*>(list, 0x4);
        if (first == nullptr) break;                                   // 0090E6F5
        const auto* last = at<const std::uint16_t*>(list, 0x8);
        if (i >= static_cast<std::uint32_t>(last - first)) break;      // 0090E700 JNC
        // 0090E702..0090E711: the checked operator[] repeats both tests.
        if (first == nullptr || i >= static_cast<std::uint32_t>(last - first))
            host.invalid_parameter_00bf6713();
        if (at<const std::uint16_t*>(list, 0x4)[i] == id) return;     // 0090E719 / 0090E71D
    }
    // 0090E724: push_back of the argument slot, which now holds the zero-extended id.
    host.vector_u16_push_back_007265e0(list, &id);
}

float zero_float_008fb520(const void*) noexcept { return 0.0f; }

void scoring_noop_00905370(void*) noexcept {}

void construct_world_noop_00920ec0(void*) noexcept {}

namespace {

using EraseFn = void (Seg59cHost::*)(void*, void*, void*);

void clear_vector(Seg59cHost& host, void* object, std::size_t off, EraseFn erase) {
    void* vec = add(object, off);
    // first at +4h, last at +8h of the vector; both checks are first <= last.
    if (at<std::uintptr_t>(vec, 0x4) > at<std::uintptr_t>(vec, 0x8)) host.invalid_parameter_00bf6713();
    if (at<std::uintptr_t>(vec, 0x4) > at<std::uintptr_t>(vec, 0x8)) host.invalid_parameter_00bf6713();
    (host.*erase)(vec, at<void*>(vec, 0x4), at<void*>(vec, 0x8));
}

// The map's head node is at map+4h; begin is head->_Left (node+0h). The checked-iterator
// tests (container non-null, iterator not at end before dereference) report through 00BF6713
// and cannot fire on a well-formed map, so they are kept only as the end test.
template <class Visit>
void for_each_node(Seg59cHost& host, void* map, Visit visit) {
    void* head = at<void*>(map, 0x4);
    for (void* node = at<void*>(head, 0x0); node != at<void*>(map, 0x4);
         node = host.tree_next_00581290(head, node)) {
        if (!visit(node)) return;
    }
}

}  // namespace

void clear_three_vectors_008facc0(Seg59cHost& host, void* object) {
    clear_vector(host, object, 0x14, &Seg59cHost::vector_erase_range_008fa1a0);   // 008FACC7
    clear_vector(host, object, 0x24, &Seg59cHost::vector_erase_range_008fa220);
    clear_vector(host, object, 0x04, &Seg59cHost::vector_erase_range_008f9fd0);
}

bool map_all_node_floats_zero_009080a0(Seg59cHost& host, void* map) {
    bool all_zero = true;
    for_each_node(host, map, [&](void* node) {
        // 009080DF MOVSS / UCOMISS [00D7A218] = 0.0f; unordered goes to 00908104 (false).
        const float v = at<float>(node, kNodeFloat20Offset);
        if (!(v == 0.0f)) {
            all_zero = false;
            return false;
        }
        return true;
    });
    return all_zero;
}

std::int32_t map_sum_node_int_1f4_00908120(Seg59cHost& host, void* map) {
    std::uint32_t sum = 0;
    for_each_node(host, map, [&](void* node) {
        sum += at<std::uint32_t>(node, kNodeInt1F4Offset);            // 00908167
        return true;
    });
    return static_cast<std::int32_t>(sum);
}

}  // namespace bsp::cc13_seg59c
