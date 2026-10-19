// Open game-named rows, packet cc13-gamerows (marker cc13_gamerows). docs/CC13_GAMEROWS.md.
#include "bsp/cc13_gamerows.hpp"

namespace bsp::cc13_gamerows {
namespace {

template <class T>
T& at(void* base, std::uint32_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}

template <class T>
const T& at(const void* base, std::uint32_t off) noexcept {
    return *reinterpret_cast<const T*>(static_cast<const std::uint8_t*>(base) + off);
}

// A fresh head node: the isnil byte at `isnil` set, its three links (+0h, +4h, +8h) to itself,
// size +8h of the map = 0.
void init_map(void* map, void* head, std::uint32_t isnil) {
    at<void*>(map, 0x4) = head;
    at<std::uint8_t>(head, isnil) = 1;
    at<void*>(at<void*>(map, 0x4), 0x4) = at<void*>(map, 0x4);
    at<void*>(at<void*>(map, 0x4), 0x0) = at<void*>(map, 0x4);
    at<void*>(at<void*>(map, 0x4), 0x8) = at<void*>(map, 0x4);
    at<std::uint32_t>(map, 0x8) = 0;
}

// erase(begin, end): first = {map, [head] (the leftmost node)}, last = {map, head}; the result
// iterator is a stack temporary.
template <class Erase>
void erase_all(void* map, Erase erase) {
    void* result[2] = {nullptr, nullptr};
    void* const head = at<void*>(map, 0x4);
    erase(map, result, map, at<void*>(head, 0x0), map, head);
}

}  // namespace

void* idle_record_construct_0082e1f0(GameRowsHost& host, void* record) {
    // 0082E20E-0082E22B: the map at +0h.
    init_map(record, host.tree_buy_head_node_004858d0(record), 0x15);
    // 0082E232-0082E260: the map at +0Ch.
    void* const anims = static_cast<std::uint8_t*>(record) + 0xC;
    init_map(anims, host.string_float_map_blank_node_00443e20(anims), 0x19);
    return record;
}

void idle_record_destruct_0082e280(GameRowsHost& host, void* record) {
    // 0082E2A1-0082E2CE: +0Ch first.
    void* const anims = static_cast<std::uint8_t*>(record) + 0xC;
    erase_all(anims, [&](void* m, void* r, void* c0, void* f, void* c1, void* l) {
        host.tree_erase_range_00444b10(m, r, c0, f, c1, l);
    });
    host.free_00bf65ac(at<void*>(anims, 0x4));
    at<std::uint32_t>(anims, 0x4) = 0;
    at<std::uint32_t>(anims, 0x8) = 0;
    // 0082E2D1-0082E301: then +0h.
    erase_all(record, [&](void* m, void* r, void* c0, void* f, void* c1, void* l) {
        host.tree_erase_range_00492a00(m, r, c0, f, c1, l);
    });
    host.free_00bf65ac(at<void*>(record, 0x4));
    at<std::uint32_t>(record, 0x4) = 0;
    at<std::uint32_t>(record, 0x8) = 0;
}

void* idle_record_copy_construct_0082e5a0(GameRowsHost& host, void* record, const void* source) {
    host.tree_copy_construct_004990b0(record, source);                       // 0082E5C3
    host.string_float_map_copy_construct_0049dec0(static_cast<std::uint8_t*>(record) + 0xC,
                                                  static_cast<const std::uint8_t*>(source) + 0xC);  // 0082E5D7
    at<std::uint32_t>(record, 0x18) = at<std::uint32_t>(source, 0x18);       // 0082E5DC
    return record;
}

void* traffic_record_assign_0082df00(GameRowsHost& host, void* record, const void* source) {
    // 0082DF0D-0082DF2C: the plain fields.
    at<std::uint32_t>(record, 0x0) = at<std::uint32_t>(source, 0x0);
    at<std::uint32_t>(record, 0x4) = at<std::uint32_t>(source, 0x4);
    at<std::uint32_t>(record, 0x8) = at<std::uint32_t>(source, 0x8);
    at<std::uint8_t>(record, 0xC) = at<std::uint8_t>(source, 0xC);
    at<std::uint32_t>(record, 0x10) = at<std::uint32_t>(source, 0x10);
    at<std::uint32_t>(record, 0x14) = at<std::uint32_t>(source, 0x14);
    // 0082DF2F-0082DF5E: seven FLD/FSTP float pairs (a value copy; an SNaN would come out quiet).
    for (std::uint32_t off = 0x18; off <= 0x30; off += 4) at<float>(record, off) = at<float>(source, off);
    // 0082DF35-0082DF7B: the map at +34h, unless it is the source's.
    void* const map_a = static_cast<std::uint8_t*>(record) + 0x34;
    const void* const src_a = static_cast<const std::uint8_t*>(source) + 0x34;
    if (map_a != src_a) {
        erase_all(map_a, [&](void* m, void* r, void* c0, void* f, void* c1, void* l) {
            host.tree_erase_range_00492a00(m, r, c0, f, c1, l);
        });
        host.tree_copy_0048d080(map_a, src_a);
    }
    // 0082DF80-0082DFA2: the map at +40h, unless it is the source's.
    void* const map_b = static_cast<std::uint8_t*>(record) + 0x40;
    const void* const src_b = static_cast<const std::uint8_t*>(source) + 0x40;
    if (map_b != src_b) {
        erase_all(map_b, [&](void* m, void* r, void* c0, void* f, void* c1, void* l) {
            host.tree_erase_range_00444b10(m, r, c0, f, c1, l);
        });
        host.scalar_tree_copy_00499680(map_b, src_b);
    }
    return record;
}

float virtual_return_zero_float_009a7a40(void*) noexcept {
    return 0.0f;  // D9 EE C3: FLDZ; RET
}

}  // namespace bsp::cc13_gamerows
