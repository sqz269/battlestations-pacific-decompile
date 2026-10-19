// Open game-named rows, packet cc13-gamerows (marker cc13_gamerows). docs/CC13_GAMEROWS.md.
#include "bsp/cc13_gamerows.hpp"

#include <cstring>

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

// An FLD/FSTP float copy: the bits pass through the x87 stack, which quiets a signalling NaN
// (sets mantissa bit 22). A plain float assignment compiles to MOVSS and would keep it signalling.
void fld_fstp(void* dst, const void* src) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, src, 4);
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0) bits |= 0x00400000u;
    std::memcpy(dst, &bits, 4);
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
    // 0082DF2F-0082DF5E: seven FLD/FSTP float pairs (an SNaN comes out quiet, as in the image).
    for (std::uint32_t off = 0x18; off <= 0x30; off += 4) {
        fld_fstp(static_cast<std::uint8_t*>(record) + off, static_cast<const std::uint8_t*>(source) + off);
    }
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

// ===== Batch 4 =====

namespace {

std::uint8_t* bytes(void* p) noexcept { return static_cast<std::uint8_t*>(p); }

// {data, count, capacity}.
std::uint8_t*& data_of(void* v) noexcept { return at<std::uint8_t*>(v, 0x0); }
std::int32_t& count_of(void* v) noexcept { return at<std::int32_t>(v, 0x4); }
std::int32_t& capacity_of(void* v) noexcept { return at<std::int32_t>(v, 0x8); }

// MUL by the element size with SETO/NEG/OR: 0xFFFFFFFF when the product overflows.
std::uint32_t new_array_size(std::uint32_t n, std::uint32_t size) noexcept {
    const std::uint64_t product = static_cast<std::uint64_t>(n) * size;
    return product > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<std::uint32_t>(product);
}

// The shared push_back(const dword&) of 0086A910 / 0086A950 / 0086AAA0 / 0086AE90.
template <class Reserve>
void push_back_dword(void* v, const std::uint32_t* value, Reserve reserve) {
    if (count_of(v) == capacity_of(v)) {
        std::int32_t n = capacity_of(v) + capacity_of(v);   // ADD EAX,EAX
        if (!(n > 1)) n = 1;
        reserve(n);
    }
    auto* slot = reinterpret_cast<std::uint32_t*>(data_of(v) + 4u * static_cast<std::uint32_t>(count_of(v)));
    if (slot != nullptr) *slot = *value;
    count_of(v) += 1;
}

// The unsigned grow of 008661B0, inlined by 00866690: capacity = 2*capacity + 2 when larger.
void pointer_vector_regrow(GameRowsHost& host, void* v, std::uint32_t n) {
    auto& capacity = at<std::uint32_t>(v, 0x8);
    if (!(n > capacity)) return;
    capacity = n;
    auto* const block = static_cast<std::uint32_t*>(host.operator_new_00bf55be(new_array_size(n, 4)));
    auto* const old = at<std::uint32_t*>(v, 0x0);
    if (old != nullptr) {
        for (std::uint32_t i = 0; i < at<std::uint32_t>(v, 0x4); ++i) block[i] = at<std::uint32_t*>(v, 0x0)[i];
        host.free_00bf6989(at<std::uint32_t*>(v, 0x0));
    }
    at<std::uint32_t*>(v, 0x0) = block;
}

// The 12-byte point vector's last element (007B0E60 / 007B2B90): {first, last} at `pair`.
void last_point(GameRowsHost& host, const void* pair, float* out) {
    const auto first = reinterpret_cast<std::uintptr_t>(at<const void*>(pair, 0x0));
    const auto last = reinterpret_cast<std::uintptr_t>(at<const void*>(pair, 0x4));
    // IMUL 2AAAAAABh / SAR 1 / add the sign bit: a signed division of the byte span by 12.
    const auto size_of = [&] {
        return first == 0 ? std::int32_t{0}
                           : static_cast<std::int32_t>(static_cast<std::int32_t>(last - first) / 12);
    };
    const std::int32_t index = size_of() - 1;
    if (!(static_cast<std::uint32_t>(index) < static_cast<std::uint32_t>(size_of()))) host.invalid_parameter_00bf6713();
    const float* const point = reinterpret_cast<const float*>(first + 12u * static_cast<std::uint32_t>(index));
    fld_fstp(out + 0, point + 0);                                      // FLD/FSTP x3
    fld_fstp(out + 1, point + 1);
    fld_fstp(out + 2, point + 2);
}

}  // namespace

void dword_vector_reserve_0086a280(GameRowsHost& host, void* vector, std::int32_t n) {
    if (n < 1) n = 1;                                                  // 0086A286
    if (capacity_of(vector) >= n) return;                              // 0086A292
    auto* const block = static_cast<std::uint32_t*>(host.operator_new_00bf55be(4u * static_cast<std::uint32_t>(n)));
    for (std::int32_t i = 0; i < count_of(vector); ++i) {              // 0086A2AC..0086A2C7
        std::uint32_t* const dst = block + i;
        if (dst != nullptr) *dst = reinterpret_cast<std::uint32_t*>(data_of(vector))[i];
    }
    host.free_00bf6989(data_of(vector));                               // 0086A2CC
    data_of(vector) = reinterpret_cast<std::uint8_t*>(block);
    capacity_of(vector) = n;
}

void dword_vector_resize_zero_fill_0086aa10(GameRowsHost& host, void* vector, std::int32_t n) {
    if (n > capacity_of(vector)) dword_vector_reserve_0086a280(host, vector, n);   // 0086AA1E
    for (std::int32_t i = count_of(vector); i < n; ++i) {              // 0086AA30..0086AA44
        auto* const slot = reinterpret_cast<std::uint32_t*>(data_of(vector) + 4u * static_cast<std::uint32_t>(i));
        if (slot != nullptr) *slot = 0;
    }
    while (n < count_of(vector)) count_of(vector) += -1;               // 0086AA50 (trivial destroys)
    count_of(vector) = n;
}

void dword_vector_destroy_0086b630(GameRowsHost& host, void* vector) {
    dword_vector_resize_zero_fill_0086aa10(host, vector, 0);
    host.free_00bf6989(data_of(vector));
}

std::uint32_t* dword_vector_at_00872e70(void* vector, std::int32_t index) noexcept {
    return reinterpret_cast<std::uint32_t*>(data_of(vector) + 4u * static_cast<std::uint32_t>(index));
}

void dword_vector_push_back_0086a910(GameRowsHost& host, void* vector, const std::uint32_t* value) {
    push_back_dword(vector, value, [&](std::int32_t n) { host.dword_vector_reserve_0086a220(vector, n); });
}

void dword_vector_push_back_0086a950(GameRowsHost& host, void* vector, const std::uint32_t* value) {
    push_back_dword(vector, value, [&](std::int32_t n) { dword_vector_reserve_0086a280(host, vector, n); });
}

void particle_resource_pointers_push_back_0086aaa0(GameRowsHost& host, void* vector, const std::uint32_t* value) {
    push_back_dword(vector, value,
                    [&](std::int32_t n) { host.particle_resource_pointers_reserve_0086a4d0(vector, n); });
}

void particle_resource_pointers_push_back_0086ae90(GameRowsHost& host, void* vector, const std::uint32_t* value) {
    push_back_dword(vector, value,
                    [&](std::int32_t n) { host.particle_resource_pointers_reserve_0086a4d0(vector, n); });
}

void qword_vector_push_back_0086a990(GameRowsHost& host, void* vector, const float* pair) {
    if (count_of(vector) == capacity_of(vector)) {
        std::int32_t n = capacity_of(vector) + capacity_of(vector);
        if (!(n > 1)) n = 1;
        host.qword_vector_reserve_0086a350(vector, n);
    }
    auto* const slot = reinterpret_cast<float*>(data_of(vector) + 8u * static_cast<std::uint32_t>(count_of(vector)));
    if (slot != nullptr) {
        fld_fstp(slot + 0, pair + 0);                                  // FLD/FSTP pairs
        fld_fstp(slot + 1, pair + 1);
    }
    count_of(vector) += 1;
}

void qword_vector_clear_0086aae0(GameRowsHost& host, void* vector) {
    if (capacity_of(vector) < 0) host.qword_vector_reserve_0086a350(vector, 0);   // 0086AAEB
    while (count_of(vector) > 0) count_of(vector) += -1;               // 0086AB00
    count_of(vector) = 0;
}

void qword_vector_destroy_0086aed0(GameRowsHost& host, void* vector) {
    qword_vector_clear_0086aae0(host, vector);
    host.free_00bf6989(data_of(vector));                               // 0086AF03
}

void pointer_vector_reserve_00865e90(GameRowsHost& host, void* vector, std::uint32_t n) {
    pointer_vector_regrow(host, vector, n);
}

void pointer_vector_grow_008661b0(GameRowsHost& host, void* vector) {
    const std::uint32_t capacity = at<std::uint32_t>(vector, 0x8);
    pointer_vector_regrow(host, vector, capacity + capacity + 2u);     // LEA EAX,[EAX+EAX+2]
}

void pointer_vector_push_back_00866690(GameRowsHost& host, void* vector, std::uint32_t value) {
    if (at<std::uint32_t>(vector, 0x4) == at<std::uint32_t>(vector, 0x8)) pointer_vector_grow_008661b0(host, vector);
    at<std::uint32_t*>(vector, 0x0)[at<std::uint32_t>(vector, 0x4)] = value;   // 008666FC
    at<std::uint32_t>(vector, 0x4) += 1;
}

void vector14_reserve_0072a3b0(GameRowsHost& host, void* vector, std::int32_t n) {
    if (n < 1) n = 1;
    if (capacity_of(vector) >= n) return;                              // 0072A3C5
    std::uint8_t* const block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(20u * static_cast<std::uint32_t>(n)));
    for (std::int32_t i = 0; i < count_of(vector); ++i) {              // 0072A3F5..0072A42A
        std::uint8_t* const dst = block + 20u * static_cast<std::uint32_t>(i);
        if (dst == nullptr) continue;
        const std::uint8_t* const src = data_of(vector) + 20u * static_cast<std::uint32_t>(i);
        at<std::uint32_t>(dst, 0x0) = kVector14ElementVtable;
        at<std::uint16_t>(dst, 0x4) = at<std::uint16_t>(src, 0x4);
        at<std::uint32_t>(dst, 0x8) = at<std::uint32_t>(src, 0x8);
        at<std::uint32_t>(dst, 0xC) = at<std::uint32_t>(src, 0xC);
        at<std::uint32_t>(dst, 0x10) = at<std::uint32_t>(src, 0x10);
    }
    for (std::int32_t i = 0; i < count_of(vector); ++i)                // 0072A435..0072A44C
        host.element_virtual_destroy_slot0(data_of(vector) + 20u * static_cast<std::uint32_t>(i), 0);
    host.free_00bf6989(data_of(vector));                               // 0072A451
    data_of(vector) = block;
    capacity_of(vector) = n;
}

void vector14_resize_0072b6d0(GameRowsHost& host, void* vector, std::int32_t n) {
    if (n > capacity_of(vector)) vector14_reserve_0072a3b0(host, vector, n);   // 0072B6DF
    for (std::int32_t i = count_of(vector); i < n; ++i) {              // 0072B6F8..0072B710
        std::uint8_t* const e = data_of(vector) + 20u * static_cast<std::uint32_t>(i);
        if (e == nullptr) continue;
        at<std::uint32_t>(e, 0x0) = kVector14ElementVtable;
        at<std::uint32_t>(e, 0xC) = 0;
        at<std::uint32_t>(e, 0x10) = 0;
    }
    while (n < count_of(vector)) {                                     // 0072B717..0072B731
        count_of(vector) += -1;
        host.element_virtual_destroy_slot0(data_of(vector) + 20u * static_cast<std::uint32_t>(count_of(vector)), 0);
    }
    count_of(vector) = n;
}

void vector_c_push_back_0072a6d0(GameRowsHost& host, void* vector, const std::uint32_t* element) {
    auto& count = at<std::uint32_t>(vector, 0x4);
    auto& capacity = at<std::uint32_t>(vector, 0x8);
    if (count == capacity) {                                           // 0072A6D9
        capacity = capacity + capacity + 2u;
        std::uint8_t* const block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(12u * capacity));
        for (std::uint32_t i = 0; i < count; ++i) {                    // 0072A700..0072A727
            std::uint8_t* const dst = block + 12u * i;
            if (dst == nullptr) continue;
            const std::uint8_t* const src = data_of(vector) + 12u * i;
            at<std::uint32_t>(dst, 0x0) = at<std::uint32_t>(src, 0x0);
            at<std::uint32_t>(dst, 0x4) = at<std::uint32_t>(src, 0x4);
            at<std::uint32_t>(dst, 0x8) = at<std::uint32_t>(src, 0x8);
        }
        if (data_of(vector) != nullptr) host.free_00bf6989(data_of(vector));   // 0072A72D
        data_of(vector) = block;
    }
    std::uint8_t* const dst = data_of(vector) + 12u * count;           // 0072A740
    if (dst != nullptr) {
        at<std::uint32_t>(dst, 0x0) = element[0];
        at<std::uint32_t>(dst, 0x4) = element[1];
        at<std::uint32_t>(dst, 0x8) = element[2];
    }
    count += 1;
}

void native_string_free_storage_0086b290(GameRowsHost& host, void* object) {
    void* const text = at<void*>(object, 0xC);
    if (text == nullptr) return;
    void* const pool = host.pool_singleton_00419cc0();                 // 0086B2A1 (its pushes are 00BD1510's)
    host.pool_return_block_00bd1510(pool, text, at<std::uint32_t>(object, 0x8) + 1u, 1);
}

void point3_ptr_array_clear_007b21a0(GameRowsHost& host, void* array) {
    std::uint8_t* const v = bytes(array) + 0x4;                        // first +4h, last +8h of v
    auto first = [&] { return at<std::uint8_t*>(v, 0x4); };
    auto last = [&] { return at<std::uint8_t*>(v, 0x8); };
    for (;;) {
        if (first() == nullptr || ((last() - first()) >> 2) == 0) return;   // 007B21AB / 007B21B7
        std::uint8_t* const end = last();
        if (first() > end) host.invalid_parameter_00bf6713();         // 007B21BE
        std::uint8_t* const back = end - 4;
        if (back > last() || back < first()) host.invalid_parameter_00bf6713();   // 007B21C8..007B21D4
        if (!(back < last())) host.invalid_parameter_00bf6713();       // 007B21DE
        void* const point = at<void*>(back, 0x0);
        if (point != nullptr) host.free_00bf65ac(point);               // 007B21EF
        if (first() == nullptr || ((last() - first()) >> 2) == 0) continue;
        at<std::uint8_t*>(v, 0x8) = last() - 4;                        // 007B220D: pop_back
    }
}

void point3_ptr_array_erase_at_007b1b40(GameRowsHost& host, void* array, std::uint32_t index) {
    auto first = [&] { return at<std::uint8_t*>(array, 0x8); };
    auto last = [&] { return at<std::uint8_t*>(array, 0xC); };
    if (first() == nullptr || !(index < static_cast<std::uint32_t>((last() - first()) >> 2)))
        host.invalid_parameter_00bf6713();                             // 007B1B5B
    std::uint8_t* const slot = first() + 4u * index;
    void* const point = at<void*>(slot, 0x0);
    if (point != nullptr) {
        host.free_00bf65ac(point);                                     // 007B1B75
        at<void*>(slot, 0x0) = nullptr;
    }
    if (first() > last()) host.invalid_parameter_00bf6713();           // 007B1B89
    std::uint8_t* const where = first() + 4u * index;
    if (where > last() || where < first()) host.invalid_parameter_00bf6713();   // 007B1B92..007B1B9B
    const std::int32_t tail = static_cast<std::int32_t>(last() - (where + 4)) >> 2;
    if (tail > 0) {
        const std::uint32_t n = 4u * static_cast<std::uint32_t>(tail);
        host.memmove_s_00bf67a7(where, n, where + 4, n);              // 007B1BB9
    }
    at<std::uint8_t*>(array, 0xC) = last() - 4;                        // 007B1BC1
}

void point3_ptr_array_insert_at_007b2850(GameRowsHost& host, void* array, std::uint32_t index,
                                         const float* point) {
    float* const p = host.point3_ptr_array_new_point_slot20(array);   // 007B285C, vtable+20h
    fld_fstp(p + 0, point + 0);                                        // FLD/FSTP x3
    fld_fstp(p + 1, point + 1);
    fld_fstp(p + 2, point + 2);
    std::uint8_t* const v = bytes(array) + 0x4;
    std::uint8_t* const begin = at<std::uint8_t*>(v, 0x4);
    if (begin > at<std::uint8_t*>(v, 0x8)) host.invalid_parameter_00bf6713();   // 007B287F
    std::uint8_t* const where = begin + 4u * index;
    if (where > at<std::uint8_t*>(v, 0x8) || where < at<std::uint8_t*>(v, 0x4)) host.invalid_parameter_00bf6713();
    void* result[2] = {nullptr, nullptr};
    void* value = p;
    host.vector_insert_one_007b26f0(v, result, v, where, &value);      // 007B28AE
}

void point_list_last_point_007b0e60(GameRowsHost& host, const void* object, float* out) {
    const void* const vector = at<const void*>(object, 0x8);          // 007B0E61
    last_point(host, static_cast<const std::uint8_t*>(vector) + 0x4, out);
}

void point3_vector_last_point_007b2b90(GameRowsHost& host, const void* object, float* out) {
    last_point(host, static_cast<const std::uint8_t*>(object) + 0xC, out);
}

std::uint32_t air_ops_block_stock_node_for_class_006ca5d0(GameRowsHost& host, const void* block,
                                                         std::int32_t n) {
    // The list's container pointer is block+40h, never null, so the binary's null and
    // owner checks (006CA5E0..006CA5E9, 006CA5FA, 006CA614) cannot fire and are not modelled.
    const void* const head = at<const void*>(block, 0x44);
    const void* node = at<const void*>(head, 0x0);                     // begin
    for (std::int32_t i = 0; node != head && n != i; ++i) {            // 006CA5F0 / 006CA5F4
        node = at<const void*>(node, 0x0);
    }
    if (node == head) host.invalid_parameter_00bf6713();               // 006CA622: dereferencing end
    return at<std::uint32_t>(node, 0xC);
}

}  // namespace bsp::cc13_gamerows
