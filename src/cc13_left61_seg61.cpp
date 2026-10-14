// Packet cc13-left61: segment 61 leftovers (0092E0B0..00951F20). See
// include/bsp/cc13_left61.hpp and docs/CC13_LEFT61.md. Each body carries its
// address, the original ABI and every simplification next to it.

#include "bsp/cc13_left61.hpp"

#include <cstring>
#include <string.h>

namespace bsp::cc13_left61 {
namespace {

// 00D7A280, double 0.5.
constexpr double kHalf = 0.5;
// 00D7A208, float -0.0: the minuend of the image's absolute value.
constexpr float kNegativeZero = -0.0f;

std::uint8_t* first_of(Raw v) { return v.at<std::uint8_t*>(kVecFirst); }
std::uint8_t* last_of(Raw v) { return v.at<std::uint8_t*>(kVecLast); }
std::uint8_t* end_of(Raw v) { return v.at<std::uint8_t*>(kVecEnd); }

// The image's element count of a 14h-byte vector: a signed IMUL 66666667h/SAR 3
// division of the byte span; 0 when first is null.
std::uint32_t count20(std::uint8_t* first, std::uint8_t* to) {
    return first == nullptr ? 0u : static_cast<std::uint32_t>((to - first) / 0x14);
}

// The image's element count of a 10h-byte vector (SAR 4 of the byte span).
std::uint32_t count16(std::uint8_t* first, std::uint8_t* to) {
    return first == nullptr ? 0u : static_cast<std::uint32_t>((to - first) >> 4);
}

// |x| as the image writes it: COMISS x, 0; JBE takes -0.0 - x (also for NaN).
float image_abs(float x) { return x > 0.0f ? x : kNegativeZero - x; }

}  // namespace

// 0092E250. The x87 sums and differences are each stored to float before the
// 0.5 (double) product, which is stored to float again; float arithmetic gives
// the same values. The |matrix| copy keeps only the nine rotation entries
// (indices 0,1,2,4,5,6,8,9,10); the image leaves the fourth column of its stack
// copy unwritten, and 0042D0D0 does not read it, so it is zero here.
void transform_box_0092e250(const float* box, float* out, const float* matrix, Left61Host& host) {
    float centre[3];
    float extent[3];
    centre[0] = static_cast<float>(static_cast<float>(box[0] + box[3]) * kHalf);
    centre[1] = static_cast<float>(static_cast<float>(box[1] + box[4]) * kHalf);
    centre[2] = static_cast<float>(static_cast<float>(box[2] + box[5]) * kHalf);
    extent[0] = static_cast<float>(static_cast<float>(box[3] - box[0]) * kHalf);
    extent[1] = static_cast<float>(static_cast<float>(box[4] - box[1]) * kHalf);
    extent[2] = static_cast<float>(static_cast<float>(box[5] - box[2]) * kHalf);
    float world_centre[3];
    host.transform_affine_point_004142e0(centre, world_centre, matrix);
    float abs_matrix[12] = {};
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            abs_matrix[row * 4 + col] = image_abs(matrix[row * 4 + col]);
        }
    }
    float radius[3];
    host.transform_direction_0042d0d0(radius, extent, abs_matrix, 0);
    out[0] = world_centre[0] - radius[0];
    out[1] = world_centre[1] - radius[1];
    out[2] = world_centre[2] - radius[2];
    out[3] = world_centre[0] + radius[0];
    out[4] = radius[1] + world_centre[1];
    out[5] = radius[2] + world_centre[2];
}

// 0092FD50. Both vectors are released through operator delete when their
// storage is set, then first/last/end are zeroed (+14h vector first).
void two_vector_element_destroy_0092fd50(Raw element, Left61Host& host) {
    if (element.ptr(0x14) != nullptr) host.operator_delete_00bf65ac(element.ptr(0x14));
    element.at<std::uint32_t>(0x14) = 0;
    element.at<std::uint32_t>(0x18) = 0;
    element.at<std::uint32_t>(0x1c) = 0;
    if (element.ptr(0x04) != nullptr) host.operator_delete_00bf65ac(element.ptr(0x04));
    element.at<std::uint32_t>(0x04) = 0;
    element.at<std::uint32_t>(0x08) = 0;
    element.at<std::uint32_t>(0x0c) = 0;
}

// 00930520 / 00930570. 0CCCCCCCh is max_size() for 14h-byte elements; the
// xlen call (which throws) is followed by the allocation in the listing.
bool vector20_buy(Raw vector, std::uint32_t count, std::uint32_t xlen_target,
                  std::uint32_t allocate_target, Left61Host& host) {
    vector.at<std::uint32_t>(kVecFirst) = 0;
    vector.at<std::uint32_t>(kVecLast) = 0;
    vector.at<std::uint32_t>(kVecEnd) = 0;
    if (count == 0) return false;
    if (count > 0x0cccccccu) host.vector_xlen(xlen_target);
    auto* storage = static_cast<std::uint8_t*>(host.vector_allocate(allocate_target, count));
    vector.at<std::uint8_t*>(kVecFirst) = storage;
    vector.at<std::uint8_t*>(kVecLast) = storage;
    vector.at<std::uint8_t*>(kVecEnd) = storage + count * 0x14;
    return true;
}

bool vector20_buy_00930520(Raw vector, std::uint32_t count, Left61Host& host) {
    return vector20_buy(vector, count, 0x009302a0u, 0x0092c810u, host);
}

bool vector20_buy_00930570(Raw vector, std::uint32_t count, Left61Host& host) {
    return vector20_buy(vector, count, 0x00930310u, 0x0092c870u, host);
}

// 00930F20 / 00931070: MSVC9 vector::operator= for a POD 14h-byte element (the
// _Destroy calls are empty and not emitted). Branches in listing order: empty
// right -> clear(); right fits in size -> copy, last = first + right count;
// fits in capacity -> copy the first size() elements, _Ucopy the rest at last;
// otherwise operator delete the old storage, _Buy(right.size()), _Ucopy all.
Raw vector20_assign(Raw self, Raw right, const Vector20Targets& t, Left61Host& host) {
    if (self.p == right.p) return self;
    std::uint8_t* r_first = first_of(right);
    const std::uint32_t r_count = count20(r_first, last_of(right));
    if (r_first == nullptr || r_count == 0) {
        host.vector_clear(t.clear, self);
        return self;
    }
    std::uint8_t* mine = first_of(self);
    if (r_count <= count20(mine, last_of(self))) {
        host.range_copy(t.copy, r_first, last_of(right), mine);
        self.at<std::uint8_t*>(kVecLast) =
            first_of(self) + count20(first_of(right), last_of(right)) * 0x14;
        return self;
    }
    std::uint8_t* from;
    std::uint8_t* to;
    if (r_count <= count20(mine, end_of(self))) {
        from = r_first + host.vector_size(t.size, self) * 0x14;
        host.range_copy(t.copy, r_first, from, mine);
        to = last_of(self);
    } else {
        if (mine != nullptr) host.operator_delete_00bf65ac(mine);
        if (!t.buy(self, host.vector_size(t.size, right), host)) return self;
        to = first_of(self);
        from = first_of(right);
    }
    self.at<void*>(kVecLast) = host.vector_ucopy(t.ucopy, self, from, last_of(right), to);
    return self;
}

Raw vector20_assign_00930f20(Raw self, Raw right, Left61Host& host) {
    static constexpr Vector20Targets t{0x0092ff80u, 0x0092f010u, 0x0092c060u, 0x0092f8d0u,
                                       &vector20_buy_00930520};
    return vector20_assign(self, right, t, host);
}

Raw vector20_assign_00931070(Raw self, Raw right, Left61Host& host) {
    static constexpr Vector20Targets t{0x0092ffc0u, 0x0092f080u, 0x0092c0c0u, 0x0092f900u,
                                       &vector20_buy_00930570};
    return vector20_assign(self, right, t, host);
}

// 009311C0 / 00931200 / 009317D0 / 00931820: placement copy construction. The
// image's EH frame only covers the constructor call (state 0, no unwind
// action), so no catch is needed.
void construct_vector_009311c0(void* memory, const void* source, Left61Host& host) {
    if (memory != nullptr) host.vector_copy_construct(0x00930ce0u, memory, source);
}

void construct_vector_00931200(void* memory, const void* source, Left61Host& host) {
    if (memory != nullptr) host.vector_copy_construct(0x00930dd0u, memory, source);
}

void allocator_construct_009317d0(void* memory, const void* source, Left61Host& host) {
    if (memory != nullptr) host.vector_copy_construct(0x00930ce0u, memory, source);
}

void allocator_construct_00931820(void* memory, const void* source, Left61Host& host) {
    if (memory != nullptr) host.vector_copy_construct(0x00930dd0u, memory, source);
}

// 009319D0. The value is the 14h-byte temporary the image builds on its stack:
// dword +0 and +10h are zero, dwords +4..+0Ch are copied from uninitialised
// stack slots (009319DD..009319EB). The reconstruction passes zero there; the
// image's values are indeterminate.
void debris_entry_vector_resize_009319d0(Raw vector, std::uint32_t count, Left61Host& host) {
    const std::uint32_t value[5] = {0, 0, 0, 0, 0};
    host.debris_entry_vector_resize_00931530(vector, count, value);
}

// 00931B70 / 00931C10. Each step copy-constructs when the slot pointer is not
// null. The catch block (00931BCB) destroys [first, current) through
// allocator::destroy and rethrows (00BF6885(0, 0)).
void uninit_fill_n_vectors(std::uint8_t* first, std::uint32_t count, const void* value,
                           void* allocator, std::uint32_t copy_target, std::uint32_t destroy_target,
                           Left61Host& host) {
    std::uint8_t* current = first;
    try {
        for (; count != 0; --count) {
            if (current != nullptr) host.vector_copy_construct(copy_target, current, value);
            current += 0x10;
        }
    } catch (...) {
        for (std::uint8_t* p = first; p != current; p += 0x10) {
            host.allocator_destroy(destroy_target, allocator, p);
        }
        host.cxx_rethrow_00bf6885();
        throw;
    }
}

void uninit_fill_n_vectors_00931b70(std::uint8_t* first, std::uint32_t count, const void* value,
                                    void* allocator, Left61Host& host) {
    uninit_fill_n_vectors(first, count, value, allocator, 0x00930ce0u, 0x00930ec0u, host);
}

void uninit_fill_n_vectors_00931c10(std::uint8_t* first, std::uint32_t count, const void* value,
                                    void* allocator, Left61Host& host) {
    uninit_fill_n_vectors(first, count, value, allocator, 0x00930dd0u, 0x00930ef0u, host);
}

// 00931CB0 / 00931DE0: MSVC9 _Swap_move. An empty local vector (+4..+0Ch zero)
// is copy-constructed into each destination slot that is not null, then the
// slot's first/last/end are swapped with the source's (the swap is not guarded
// by the null test, as in the image). At the end the local's storage is
// released through operator delete when set. Catch block (00931D3E): destroy
// [dest, current) and rethrow. The local vector's own unwind action (EH
// state 1) is not modelled: it is never given storage here, so its _Tidy has
// nothing to free.
std::uint8_t* uninit_swap_move_vectors(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                       void* allocator, std::uint32_t copy_target,
                                       std::uint32_t destroy_target, Left61Host& host) {
    std::uint32_t empty[4] = {0, 0, 0, 0};  // [EBP-30h]: allocator byte, first, last, end
    std::uint8_t* const dest_first = dest;
    try {
        for (; first != last; first += 0x10, dest += 0x10) {
            if (dest != nullptr) host.vector_copy_construct(copy_target, dest, empty);
            for (std::size_t off = 4; off <= 0x0c; off += 4) {
                std::uint32_t& a = *reinterpret_cast<std::uint32_t*>(first + off);
                std::uint32_t& b = *reinterpret_cast<std::uint32_t*>(dest + off);
                const std::uint32_t moved = a;
                a = b;
                b = moved;
            }
        }
    } catch (...) {
        for (std::uint8_t* p = dest_first; p != dest; p += 0x10) {
            host.allocator_destroy(destroy_target, allocator, p);
        }
        host.cxx_rethrow_00bf6885();
        throw;
    }
    if (empty[1] != 0) host.operator_delete_00bf65ac(reinterpret_cast<void*>(static_cast<std::uintptr_t>(empty[1])));
    return dest;
}

std::uint8_t* uninit_swap_move_vectors_00931cb0(std::uint8_t* first, std::uint8_t* last,
                                                std::uint8_t* dest, void* allocator, Left61Host& host) {
    return uninit_swap_move_vectors(first, last, dest, allocator, 0x00930ce0u, 0x00930ec0u, host);
}

std::uint8_t* uninit_swap_move_vectors_00931de0(std::uint8_t* first, std::uint8_t* last,
                                                std::uint8_t* dest, void* allocator, Left61Host& host) {
    return uninit_swap_move_vectors(first, last, dest, allocator, 0x00930dd0u, 0x00930ef0u, host);
}

// 009322F0 / 00932570: MSVC9 vector<vector<T>>::_Insert_n(where, count, value).
// The value is first copied into a local (_Tmp, [EBP-28h]); t.release_tmp
// releases it on the way out (operator delete for the vector copy, the string
// pool for the 0093B690 row copy). max_size() is 0FFFFFFFh;
// growth is capacity + capacity/2 unless that overflows (then 0), raised to the
// required size. Catch blocks: 0093245E destroys [new storage, current) through
// vector::_Destroy, deletes the new storage and rethrows; 009324E7 destroys
// [where + count, last + count) and rethrows. The _Tmp unwind action (EH state
// 0) on those paths is not modelled.
void insert_n_nested(Raw self, std::uint8_t* where, std::uint32_t count, const void* value,
                     const NestedVectorTargets& t, Left61Host& host) {
    std::uint32_t tmp[4];  // [EBP-28h]: allocator byte, first, last, end
    host.vector_copy_construct(t.copy_construct, tmp, value);
    std::uint8_t* first = first_of(self);
    std::uint32_t capacity = count16(first, end_of(self));
    if (count != 0) {
        if (0x0fffffffu - count16(first, last_of(self)) < count) host.vector_xlen(t.xlen);
        first = first_of(self);
        if (capacity < count16(first, last_of(self)) + count) {
            const std::uint32_t grow = capacity >> 1;
            capacity = (0x0fffffffu - grow < capacity) ? 0u : capacity + grow;
            if (capacity < count16(first, last_of(self)) + count) {
                capacity = count16(first, last_of(self)) + count;
            }
            auto* storage = static_cast<std::uint8_t*>(host.vector_allocate(t.allocate, capacity));
            std::uint8_t* current = storage;
            try {
                current = static_cast<std::uint8_t*>(
                    host.nested_uninit_move(t.uninit_move, first_of(self), where, storage, self.p));
                current = static_cast<std::uint8_t*>(host.nested_ufill(t.ufill, self, current, count, tmp));
                host.nested_uninit_move(t.uninit_move, where, last_of(self), current, self.p);
            } catch (...) {
                host.nested_destroy(t.destroy, self, storage, current);
                host.operator_delete_00bf65ac(storage);
                host.cxx_rethrow_00bf6885();
                throw;
            }
            std::uint8_t* old_first = first_of(self);
            const std::uint32_t new_count = count + count16(old_first, last_of(self));
            if (old_first != nullptr) {
                host.nested_destroy_range(t.destroy_range, old_first, last_of(self), self.p);
                host.operator_delete_00bf65ac(first_of(self));
            }
            self.at<std::uint8_t*>(kVecEnd) = storage + capacity * 0x10;
            self.at<std::uint8_t*>(kVecLast) = storage + new_count * 0x10;
            self.at<std::uint8_t*>(kVecFirst) = storage;
        } else {
            std::uint8_t* old_last = last_of(self);
            if (static_cast<std::uint32_t>((old_last - where) >> 4) < count) {
                host.nested_umove(t.umove, self, where, old_last, where + count * 0x10);
                try {
                    host.nested_ufill(t.ufill, self, last_of(self),
                                      count - static_cast<std::uint32_t>((last_of(self) - where) >> 4), tmp);
                } catch (...) {
                    host.nested_destroy(t.destroy, self, where + count * 0x10, last_of(self) + count * 0x10);
                    host.cxx_rethrow_00bf6885();
                    throw;
                }
                self.at<std::uint8_t*>(kVecLast) = last_of(self) + count * 0x10;
                host.nested_fill(t.fill, where, last_of(self) - count * 0x10, tmp);
            } else {
                std::uint8_t* const split = old_last - count * 0x10;
                self.at<void*>(kVecLast) = host.nested_umove(t.umove, self, split, old_last, old_last);
                host.nested_move_backward(t.move_backward, where, split, old_last);
                host.nested_fill(t.fill, where, where + count * 0x10, tmp);
            }
        }
    }
    t.release_tmp(tmp, host);
}

namespace {
// 00932541: the nested vector's local copy is freed through operator delete when set.
void release_vector_tmp(std::uint32_t* tmp, Left61Host& host) {
    if (tmp[1] != 0) host.operator_delete_00bf65ac(reinterpret_cast<void*>(static_cast<std::uintptr_t>(tmp[1])));
}
// 0093B8E9: the row's name goes back to the string pool (length + 1 bytes) when set.
void release_row_tmp(std::uint32_t* tmp, Left61Host& host) {
    if (tmp[2] != 0) {
        void* pool = host.string_pool_00419cc0();
        host.string_pool_return_00bd1510(pool, reinterpret_cast<void*>(static_cast<std::uintptr_t>(tmp[2])),
                                         tmp[1] + 1, 1);
    }
}
}  // namespace

void insert_n_nested_009322f0(Raw self, std::uint8_t* where, std::uint32_t count, const void* value,
                              Left61Host& host) {
    static constexpr NestedVectorTargets t{0x00930ce0u, 0x009301c0u, 0x0092c750u, 0x00931cb0u,
                                           0x009320d0u, 0x009316f0u, 0x00931f10u, 0x00932190u,
                                           0x00931af0u, 0x009321c0u, &release_vector_tmp};
    insert_n_nested(self, where, count, value, t, host);
}

void insert_n_nested_00932570(Raw self, std::uint8_t* where, std::uint32_t count, const void* value,
                              Left61Host& host) {
    static constexpr NestedVectorTargets t{0x00930dd0u, 0x00930230u, 0x0092c7b0u, 0x00931de0u,
                                           0x00932150u, 0x00931760u, 0x00931f30u, 0x00932200u,
                                           0x00931b40u, 0x00932230u, &release_vector_tmp};
    insert_n_nested(self, where, count, value, t, host);
}

void insert_n_record_009488f0(Raw self, std::uint8_t* where, std::uint32_t count, const void* value,
                              Left61Host& host) {
    static constexpr NestedVectorTargets t{0x00660540u, 0x00946a80u, 0x009416b0u, 0x009466a0u,
                                           0x00947900u, 0x00946d70u, 0x009479d0u, 0x00948080u,
                                           0x00945cf0u, 0x00946c00u, &release_row_tmp};
    insert_n_nested(self, where, count, value, t, host);
}

void insert_n_named_row_0093b690(Raw self, std::uint8_t* where, std::uint32_t count, const void* value,
                                 Left61Host& host) {
    static constexpr NestedVectorTargets t{0x0093ad30u, 0x0093b150u, 0x0093a1c0u, 0x0093b070u,
                                           0x0093b4f0u, 0x0093b300u, 0x0093b530u, 0x0093b620u,
                                           0x0093ad90u, 0x0093b460u, &release_row_tmp};
    insert_n_nested(self, where, count, value, t, host);
}

// --- 10h-byte named rows ---------------------------------------------------

namespace {

// The inlined row assignment of 0093AD90 / 0093ADF0 / 0093AEC0: dword +0, then
// the name through 0041DD40 and memcpy (length = the resized destination
// length) unless it is the same string, then the float at +0Ch (FLD/FSTP).
void named_row_assign(std::uint8_t* dst, const std::uint8_t* src, Left61Host& host) {
    Raw d{dst};
    Raw s{const_cast<std::uint8_t*>(src)};
    d.at<std::uint32_t>(0) = s.at<std::uint32_t>(0);
    if (d.field(4) != s.field(4)) {
        host.native_string_resize_0041dd40(d.field(4), s.at<std::uint32_t>(4), 1);
        if (s.at<std::uint32_t>(4) != 0) {
            std::memcpy(d.ptr(8), s.ptr(8), d.at<std::uint32_t>(4));
        }
    }
    d.at<float>(0x0c) = s.at<float>(0x0c);
}

// The inlined copy construction of 0093AD30 / 0093B070 / 0093B210: as the
// assignment, with the destination string zeroed first.
void named_row_construct(std::uint8_t* dst, const std::uint8_t* src, Left61Host& host) {
    Raw d{dst};
    Raw s{const_cast<std::uint8_t*>(src)};
    d.at<std::uint32_t>(0) = s.at<std::uint32_t>(0);
    d.at<std::uint32_t>(4) = 0;
    d.at<std::uint32_t>(8) = 0;
    if (d.field(4) != s.field(4)) {
        host.native_string_resize_0041dd40(d.field(4), s.at<std::uint32_t>(4), 1);
        if (s.at<std::uint32_t>(4) != 0) {
            std::memcpy(d.ptr(8), s.ptr(8), d.at<std::uint32_t>(4));
        }
    }
    d.at<float>(0x0c) = s.at<float>(0x0c);
}

void release_name(Raw row, Left61Host& host) {
    void* data = row.ptr(8);
    if (data != nullptr) {
        void* pool = host.string_pool_00419cc0();
        host.string_pool_return_00bd1510(pool, data, row.at<std::uint32_t>(4) + 1, 1);
    }
}

}  // namespace

// 0093A8B0. The length test comes first; equal lengths of zero match, and the
// unreachable "right empty" arm of the inlined compare returns 1 as in the image.
std::uint8_t* named_row_find_0093a8b0(std::uint8_t* first, std::uint8_t* last, const std::uint32_t* name) {
    for (; first != last; first += 0x10) {
        Raw row{first};
        const std::uint32_t length = name[0];
        if (row.at<std::uint32_t>(4) != length) continue;
        int order;
        if (row.at<std::uint32_t>(4) == 0) {
            order = length != 0 ? -1 : 0;
        } else if (length == 0) {
            order = 1;
        } else {
            order = _stricmp(static_cast<const char*>(row.ptr(8)),
                             reinterpret_cast<const char*>(static_cast<std::uintptr_t>(name[1])));
        }
        if (order == 0) return first;
    }
    return first;
}

std::uint32_t* named_row_find_iterator_0093a9e0(std::uint32_t* out, const std::uint32_t* name,
                                                std::uint32_t first_container, std::uint8_t* first,
                                                std::uint32_t last_container, std::uint8_t* last) {
    (void)last_container;  // the image passes only last.ptr on
    out[1] = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(named_row_find_0093a8b0(first, last, name)));
    out[0] = first_container;
    return out;
}

// 0093AC30 / 0093AFF0: return length + 1 bytes to the sized pool when the data
// pointer (+8) is set (pool = 00419CC0(data, length + 1, 1), then 00BD1510).
void named_row_release_name_0093ac30(Raw row, Left61Host& host) { release_name(row, host); }
void named_row_destroy_0093aff0(Raw row, Left61Host& host) { release_name(row, host); }

Raw named_row_copy_construct_0093ad30(Raw row, Raw source, Left61Host& host) {
    named_row_construct(row.p, source.p, host);
    return row;
}

// 0093AE60: the EH frame covers only the constructor (no unwind action).
void named_row_construct_0093ae60(std::uint8_t* memory, Raw source, Left61Host& host) {
    if (memory != nullptr) named_row_copy_construct_0093ad30(Raw{memory}, source, host);
}

void named_row_fill_0093ad90(std::uint8_t* first, std::uint8_t* last, Raw value, Left61Host& host) {
    for (; first != last; first += 0x10) named_row_assign(first, value.p, host);
}

std::uint8_t* named_row_copy_0093adf0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                      Left61Host& host) {
    for (; first != last; first += 0x10, dest += 0x10) named_row_assign(dest, first, host);
    return dest;
}

std::uint8_t* named_row_copy_backward_0093aec0(std::uint8_t* first, std::uint8_t* last,
                                               std::uint8_t* dest_end, Left61Host& host) {
    while (last != first) {
        last -= 0x10;
        dest_end -= 0x10;
        named_row_assign(dest_end, last, host);
    }
    return dest_end;
}

// 0093B070: catch block 0093B115 destroys [dest, current) through 0093AFF0 and
// rethrows. Null destination slots are skipped but still advanced.
std::uint8_t* named_row_uninit_copy_0093b070(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                             Left61Host& host) {
    std::uint8_t* const dest_first = dest;
    try {
        for (; first != last; first += 0x10, dest += 0x10) {
            if (dest != nullptr) named_row_construct(dest, first, host);
        }
    } catch (...) {
        for (std::uint8_t* p = dest_first; p != dest; p += 0x10) named_row_destroy_0093aff0(Raw{p}, host);
        host.cxx_rethrow_00bf6885();
        throw;
    }
    return dest;
}

// 0093B210: catch block 0093B2B9 destroys [first, current) through 0093AFF0 and rethrows.
void named_row_uninit_fill_n_0093b210(std::uint8_t* first, std::uint32_t count, Raw value,
                                      Left61Host& host) {
    std::uint8_t* current = first;
    try {
        for (; count != 0; --count, current += 0x10) {
            if (current != nullptr) named_row_construct(current, value.p, host);
        }
    } catch (...) {
        for (std::uint8_t* p = first; p != current; p += 0x10) named_row_destroy_0093aff0(Raw{p}, host);
        host.cxx_rethrow_00bf6885();
        throw;
    }
}

// --- unit controller -------------------------------------------------------

// 0092F2E0. The copy is eight dword moves; the tail JMP 0092E5B0 runs with the
// caller's return address and the stack argument overwritten with 1.
void unit_controller_apply_network_state_0092f2e0(Raw controller, const std::uint32_t* state,
                                                  Left61Host& host) {
    for (std::size_t i = 0; i < 8; ++i) controller.at<std::uint32_t>(0x40 + i * 4) = state[i];
    controller.at<std::uint8_t>(0x60) = 1;
    host.unit_controller_network_correction_0092e5b0(controller, 1);
}

// --- simple 3D scene ---------------------------------------------------------

namespace {
constexpr std::uint32_t kSimple3dSceneVtable = 0x00d19724;  // slot 0 = 0093CC90
// 00F8D39C: the object 00B0D0D0 reads the render targets from.
constexpr std::uint32_t kRenderTargetsOwner = 0x00f8d39c;
// 00CE9A38 "X": the render command label.
constexpr char kSceneLabel[] = "X";
}  // namespace

void simple_3d_scene_construct_0093cb10(Raw scene) {
    scene.at<std::uint32_t>(0) = kSimple3dSceneVtable;
    scene.at<std::uint32_t>(0x04) = 0;
    scene.at<std::uint32_t>(0x08) = 0;
    scene.at<std::uint32_t>(0x0c) = 0;
    scene.at<std::uint32_t>(0x10) = 0;
    scene.at<std::uint32_t>(0x24) = 0;
}

// 0093CC20. +0Ch is a reference-counted object (count at +4, vtable slot 0
// releases); +10h and +8h are scene nodes; +4h the native scene. Each field is
// zeroed only when it was set.
void simple_3d_scene_release_0093cc20(Raw scene, Left61Host& host) {
    void* counted = scene.ptr(0x0c);
    if (counted != nullptr) {
        if (host.interlocked_decrement_ce2220(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(counted) + 4)) == 0) {
            host.object_release_slot0(counted);
        }
        scene.ptr(0x0c) = nullptr;
    }
    if (scene.ptr(0x10) != nullptr) {
        host.node_unlink_release_00b6dfa0(scene.ptr(0x10));
        scene.ptr(0x10) = nullptr;
    }
    if (scene.ptr(0x08) != nullptr) {
        host.node_unlink_release_00b6dfa0(scene.ptr(0x08));
        scene.ptr(0x08) = nullptr;
    }
    if (scene.ptr(0x04) != nullptr) {
        host.native_scene_release_00b72250(scene.ptr(0x04));
        scene.ptr(0x04) = nullptr;
    }
}

// 0093D250. The label is built in the outgoing argument area (0041E870 on
// [ESP]) and passed by value; 00B1F4D0 frees it. The global 00F8D39C is read
// before the queue singleton call, as in the image. The EH state that would
// destroy the label if 004C11F0 or 00B0D0D0 threw is not modelled.
void simple_3d_scene_submit_render_0093d250(Raw scene, Left61Host& host) {
    std::uint32_t label[2] = {0, 0};
    host.native_string_assign_0041e870(label, kSceneLabel);
    void* owner = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(kRenderTargetsOwner)));
    void* queue = host.render_queue_singleton_004c11f0();
    const std::uint32_t targets = host.render_targets_00b0d0d0(owner);
    host.render_queue_submit_scene_00b1f4d0(queue, scene.ptr(0x04), scene.ptr(0x08), targets, label);
}

// --- map overlay (debug drawer, no live reference) ----------------------------
// x87 chains run at the main thread's 24-bit precision control
// (docs/CC13_FIXTURE_PASS_1.md), so float arithmetic in the image's operation
// order reproduces them.

namespace {
float ov(Raw overlay, std::size_t off) { return overlay.at<float>(off); }
const float* overlay_rgba(Raw overlay) { return reinterpret_cast<const float*>(overlay.field(8)); }
}  // namespace

void map_overlay_draw_world_line_0093e510(Raw overlay, float x0, float y0, float x1, float y1,
                                          Left61Host& host) {
    const float sx = ov(overlay, 0x00), sy = ov(overlay, 0x04);
    const float ox = ov(overlay, 0x18), oy = ov(overlay, 0x1c);
    const float wx = ov(overlay, 0x20), wy = ov(overlay, 0x24);
    const float sy0 = oy - (y0 - wy) * sy;
    const float sx0 = ox + sx * (x0 - wx);
    const float sy1 = oy - sy * (y1 - wy);
    const float sx1 = ox + sx * (x1 - wx);
    host.debug_screen_line_00860030(overlay_rgba(overlay), sx0, sy0, sx1, sy1);
}

// 0093EAE0. The head is two strokes from the screen point back along the
// scaled, normalised direction by `size`, spread sideways by size * 0.5
// (00D7A280, double 0.5).
void map_overlay_draw_arrow_head_0093eae0(Raw overlay, const float* point, const float* direction,
                                          float size, Left61Host& host) {
    const float sx = ov(overlay, 0x00), sy = ov(overlay, 0x04);
    const float px = (point[0] - ov(overlay, 0x20)) * sx + ov(overlay, 0x18);
    const float py = ov(overlay, 0x1c) - (point[1] - ov(overlay, 0x24)) * sy;
    float scaled[2];
    scaled[0] = direction[0] * sx;
    scaled[1] = -direction[1] * sy;
    const float inverse = host.vector2_reciprocal_length_00419260(scaled);
    const float ux = scaled[0] * inverse;
    const float uy = inverse * scaled[1];
    const float back_x = size * ux;
    const float back_y = size * uy;
    const float half = static_cast<float>(size * kHalf);
    const float side_x = -uy * half;
    const float side_y = half * ux;
    const float base_x = px - back_x;
    const float base_y = py - back_y;
    host.debug_screen_line_00860030(overlay_rgba(overlay), px, py, base_x + side_x, base_y + side_y);
    host.debug_screen_line_00860030(overlay_rgba(overlay), px, py, base_x - side_x, base_y - side_y);
}

namespace {
// 0093EC80 / 0093EDC0: the head size is limited to half the squared scaled
// length, (b*b + a*a) * 0.5 with a = sx*dx, b = sy*-dy, as written in the image.
// The compare keeps `size` when it is not above the limit (also for NaN).
float limited_head_size(Raw overlay, float dx, float dy, float size) {
    const float a = ov(overlay, 0x00) * dx;
    const float b = ov(overlay, 0x04) * -dy;
    const float limit = static_cast<float>(static_cast<float>(b * b + a * a) * kHalf);
    return size > limit ? limit : size;
}
}  // namespace

void map_overlay_draw_arrow_mid_head_0093ec80(Raw overlay, const float* from, const float* to, float size,
                                              Left61Host& host) {
    const float dx = to[0] - from[0];
    const float dy = to[1] - from[1];
    size = limited_head_size(overlay, dx, dy, size);
    float dir[2] = {dx, to[1] - from[1]};
    float mid[2];
    mid[0] = static_cast<float>(static_cast<float>(from[0] + to[0]) * kHalf);
    mid[1] = static_cast<float>(static_cast<float>(from[1] + to[1]) * kHalf);
    map_overlay_draw_arrow_head_0093eae0(overlay, mid, dir, size, host);
    map_overlay_draw_world_line_0093e510(overlay, from[0], from[1], to[0], to[1], host);
}

void map_overlay_draw_arrow_0093edc0(Raw overlay, const float* from, const float* to, float size,
                                     Left61Host& host) {
    float dir[2] = {to[0] - from[0], to[1] - from[1]};
    size = limited_head_size(overlay, dir[0], dir[1], size);
    map_overlay_draw_arrow_head_0093eae0(overlay, to, dir, size, host);
    map_overlay_draw_world_line_0093e510(overlay, from[0], from[1], to[0], to[1], host);
}

void map_overlay_draw_world_circle_0093eeb0(Raw overlay, const float* centre, float radius,
                                            Left61Host& host) {
    float r[2];
    r[0] = radius / ov(overlay, 0x00);
    r[1] = radius / ov(overlay, 0x04);
    host.map_overlay_ellipse_0093e9a0(centre, r);
}

// --- sound overlay (0093FF10, no live reference) ------------------------------

// 0093F280. Read from the listing (the decompiler output mixes the constants
// of the branches). |value| is taken as -0.0 - value when value <= 0 (00D7A218
// 0.0, 00D7A208 -0.0). Below 100 / 200 / 500 (doubles 00D7A220, 00CE4D70,
// 00CE3840) the result is step * _ftol((value + bias) / step + 0.5) with step
// 5 / 10 / 50 (doubles 00D7A370, 00CE3DC0, 00CE3938) and bias +-step (floats
// 00CE3850/00CFBC84, 00CE38B8/00CE6848, 00CEB4D4/00CECA0C); below 1000 / 4000
// (00CE47A0, 00D19840) and above, 0093EEE0(value +- step, step) with step 100
// / 200 / 500 (floats 00CE3D08/00CE65D8, 00CE386C/00CE77E4, 00CE397C/00CE3980).
// round_up (CL) picks the + bias. The result is clamped to [1 (00D7A24C),
// 1e6 (00CE4C08 double, 00CE6A04 float)].
float sound_overlay_axis_step_0093f280(char round_up, float value, Left61Host& host) {
    if (!(value > 0.0f)) value = kNegativeZero - value;
    float result;
    auto fixed_step = [&](float plus, float minus, float step) {
        const float biased = value + (round_up != 0 ? plus : minus);
        const float scaled = static_cast<float>(biased / step);
        const auto steps = static_cast<std::int32_t>(static_cast<float>(scaled + kHalf));
        return static_cast<float>(static_cast<double>(step) * steps);
    };
    auto rounded_step = [&](float plus, float minus, float step) {
        const float biased = value + (round_up != 0 ? plus : minus);
        return host.round_to_step_0093eee0(biased, step);
    };
    if (100.0 > value) {
        result = fixed_step(5.0f, -5.0f, 5.0f);
    } else if (200.0 > value) {
        result = fixed_step(10.0f, -10.0f, 10.0f);
    } else if (500.0 > value) {
        result = fixed_step(50.0f, -50.0f, 50.0f);
    } else if (1000.0 > value) {
        result = rounded_step(100.0f, -100.0f, 100.0f);
    } else if (4000.0 > value) {
        result = rounded_step(200.0f, -200.0f, 200.0f);
    } else {
        result = rounded_step(500.0f, -500.0f, 500.0f);
    }
    if (1.0f > result) return 1.0f;
    if (result > 1000000.0) return 1000000.0f;
    return result;
}

// 0093F4B0: d = b->slot1C() - a->slot1C() (a's value stored to float first);
// -1 when d < 0, 1 when d > 0, else 0. `context` is unused.
int sound_overlay_compare_0093f4b0(void* context, void* const* a, void* const* b, Left61Host& host) {
    (void)context;
    void* left = *a;
    void* right = *b;
    const float va = host.object_float_slot1c(left);
    const float d = host.object_float_slot1c(right) - va;
    if (0.0f > d) return -1;
    if (d > 0.0f) return 1;
    return 0;
}

// 0093FA70. The image scans four entries per step (an unrolled loop); the
// result, "some entry equals (x, y)", is the same. y += 12 (00CE42D0, double).
// Growth: capacity * 2, or 1 when that is below 2.
void sound_overlay_place_label_0093fa70(Raw array, float* out, float x, float y, float z,
                                        Left61Host& host) {
    for (;;) {
        const std::int32_t count = array.at<std::int32_t>(8);
        const float* entries = static_cast<const float*>(array.ptr(4));
        bool free_slot = true;
        for (std::int32_t i = 0; i < count; ++i) {
            if (entries[i * 3] == x && entries[i * 3 + 1] == y) free_slot = false;
        }
        if (free_slot) break;
        y = static_cast<float>(y + 12.0);
    }
    if (array.at<std::int32_t>(8) == array.at<std::int32_t>(0x0c)) {
        std::int32_t capacity = array.at<std::int32_t>(0x0c) * 2;
        if (capacity < 2) capacity = 1;
        host.array12_reserve_0093f590(array, capacity);
    }
    float* slot = static_cast<float*>(array.ptr(4)) + array.at<std::int32_t>(8) * 3;
    if (slot != nullptr) {
        slot[0] = x;
        slot[1] = y;
        slot[2] = z;
    }
    array.at<std::int32_t>(8) += 1;
    out[0] = x;
    out[1] = y;
}

// 0093FE90. The options are copied into the outgoing argument area for
// 00A83120 (sample = overlay+4Ch), then vtable +3Ch runs, then the by-value
// argument is destroyed inline: 0093F950(options+3Ch, 0) and _free of the
// array storage at options+3Ch. The unwind action that destroys the argument
// when a call throws (EH state 0) is not modelled.
void sound_overlay_apply_sample_settings_0093fe90(Raw overlay, std::uint8_t* options, Left61Host& host) {
    alignas(4) std::uint8_t copy[0x48];
    host.sound_options_copy_construct_0093fdf0(copy, options);
    host.sound_sample_apply_settings_00a83120(overlay.ptr(0x4c), copy);
    host.object_call_slot3c(overlay.p);
    host.sound_parameters_resize_0093f950(options + 0x3c, 0);
    host.free_00bf6989(*reinterpret_cast<void**>(options + 0x3c));
}

// --- spawn members -------------------------------------------------------------

namespace {
// 00E188A8: the game object pointer.
constexpr std::uint32_t kGamePointer = 0x00e188a8;
// 00E0CFB4: the float support-points total.
constexpr std::uint32_t kSupportPointsTotal = 0x00e0cfb4;
// Property keys: 00CE5804, 00CE8EE0, 00CF8838, 00CF882C, 00CFACBC, 00CE4780, 00CE8ED0.
constexpr char kKeyParty[] = "Party";
constexpr char kKeyRace[] = "Race";
constexpr char kKeySkill[] = "Skill";
constexpr char kKeyOwnerPlayer[] = "OwnerPlayer";
constexpr char kKeyBehaviour[] = "Behaviour";
constexpr char kKeyType[] = "Type";
constexpr char kKeyName[] = "Name";

std::uint8_t* game_object(Left61Host& host) {
    return reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(host.global_u32(kGamePointer)));
}

void return_pooled_string(char* data, std::uint32_t length, Left61Host& host) {
    if (data != nullptr) {
        void* pool = host.string_pool_00419cc0();
        host.string_pool_return_00bd1510(pool, data, length + 1, 1);
    }
}

// The inlined member assignment (009444C0, 00944540): +0 and +0Ch are dword
// moves, the name goes through 0041DD40 and memcpy unless it is the same string.
void spawn_member_assign(std::uint8_t* dst, const std::uint8_t* src, Left61Host& host) {
    Raw d{dst};
    Raw s{const_cast<std::uint8_t*>(src)};
    d.at<std::uint32_t>(0) = s.at<std::uint32_t>(0);
    if (d.field(4) != s.field(4)) {
        host.native_string_resize_0041dd40(d.field(4), s.at<std::uint32_t>(4), 1);
        if (s.at<std::uint32_t>(4) != 0) std::memcpy(d.ptr(8), s.ptr(8), d.at<std::uint32_t>(4));
    }
    d.at<std::uint32_t>(0x0c) = s.at<std::uint32_t>(0x0c);
}
}  // namespace

// 009420A0. Race = 2 - (party != 0) (NEG/SBB/ADD 2); Skill = 2 * (game+1FE4h
// != 0) + 1; Type only when the member's vehicle class (+0) is set, from its +70h.
void spawn_member_seed_property_bag_009420a0(Raw member, std::int32_t party, Left61Host& host) {
    host.property_bag_add_int_008f3710(member.ptr(0x0c), kKeyParty, party);
    host.property_bag_add_int_008f3710(member.ptr(0x0c), kKeyRace, party != 0 ? 1 : 2);
    const std::int32_t multiplayer = Raw{game_object(host)}.at<std::int32_t>(0x1fe4) != 0 ? 1 : 0;
    host.property_bag_add_int_008f3710(member.ptr(0x0c), kKeySkill, multiplayer * 2 + 1);
    host.property_bag_add_int_008f3710(member.ptr(0x0c), kKeyOwnerPlayer, 9);
    host.property_bag_add_int_008f3710(member.ptr(0x0c), kKeyBehaviour, 1);
    if (member.ptr(0) != nullptr) {
        host.property_bag_add_int_008f3710(member.ptr(0x0c), kKeyType,
                                           Raw{static_cast<std::uint8_t*>(member.ptr(0))}.at<std::int32_t>(0x70));
    }
}

// 00942130. The eight player records are game+18CCh..+18E8h (the image scans
// them four per step); a record counts when +8 is set, +9 is clear or +0Ah is
// set, and its team +28h equals the player's. The product with 0.5 (00D7A280,
// double) and the FIDIV are each stored to float.
float team_support_points_cap_00942130(std::int32_t player, Left61Host& host) {
    Raw game{game_object(host)};
    auto record = [&](std::int32_t index) {
        return Raw{reinterpret_cast<std::uint8_t*>(
            static_cast<std::uintptr_t>(game.at<std::uint32_t>(0x18cc + index * 4)))};
    };
    const std::int32_t team = record(player).at<std::int32_t>(0x28);
    std::int32_t count = 0;
    for (std::int32_t i = 0; i < 8; ++i) {
        Raw r = record(i);
        if (r.at<std::uint8_t>(8) != 0 && (r.at<std::uint8_t>(9) == 0 || r.at<std::uint8_t>(0x0a) != 0) &&
            r.at<std::int32_t>(0x28) == team) {
            ++count;
        }
    }
    if (count == 0) return 0.0f;
    float total;
    const std::uint32_t bits = host.global_u32(kSupportPointsTotal);
    std::memcpy(&total, &bits, sizeof total);
    const float half = static_cast<float>(total * kHalf);
    return half / static_cast<float>(count);
}

// 00943C00. The member-pointer null tests are on node+4 and node+8 (never null,
// even when operator new returned null), as in the image.
void* spawn_list_buy_node_00943c00(void* next, void* prev, const std::uint32_t* record, Left61Host& host) {
    auto* node = static_cast<std::uint8_t*>(host.operator_new_00bf681b(0x0c));
    const auto base = reinterpret_cast<std::uintptr_t>(node);
    if (node != nullptr) *reinterpret_cast<void**>(base) = next;
    if (base + 4 != 0) *reinterpret_cast<void**>(base + 4) = prev;
    if (base + 8 != 0) *reinterpret_cast<std::uint32_t*>(base + 8) = *record;
    return node;
}

// 00944130. The name is copied from the by-value argument (0041DD40 + memcpy),
// the bag is new(114h) + 008F41A0(0) (null when operator new fails), then
// 009420A0(party) seeds it, and the by-value name returns to the string pool.
// The unwind actions (EH states 1 and 2: the member name, the bag allocation)
// are not modelled.
Raw spawn_member_construct_00944130(Raw member, std::int32_t party, std::uint32_t vehicle_class,
                                    std::uint32_t name_length, char* name_data, Left61Host& host) {
    member.at<std::uint32_t>(4) = 0;
    member.at<std::uint32_t>(8) = 0;
    member.at<std::uint32_t>(0) = vehicle_class;
    host.native_string_resize_0041dd40(member.field(4), name_length, 1);
    if (name_length != 0) std::memcpy(member.ptr(8), name_data, member.at<std::uint32_t>(4));
    void* memory = host.operator_new_00bf681b(0x114);
    member.ptr(0x0c) = memory != nullptr ? host.property_bag_construct_008f41a0(memory, 0) : nullptr;
    spawn_member_seed_property_bag_009420a0(member, party, host);
    return_pooled_string(name_data, name_length, host);
    return member;
}

// 00944210. The two property lookups are not null-checked; the Type value is
// the property's +0Ch dword. Name length is strlen of the string value (0 for
// null); the copy runs when the resized data pointer is set.
void spawn_member_read_lua_table_00944210(Raw member, void* table, Left61Host& host) {
    host.lua_table_copy_into_bag_0043d8f0(table, member.ptr(0x0c));
    void* type = host.property_bag_find_008f2260(member.ptr(0x0c), kKeyType);
    member.ptr(0) = host.vehicle_class_get_or_create_00964790(Raw{static_cast<std::uint8_t*>(type)}.at<std::uint32_t>(0x0c), 1);
    void* name = host.property_bag_find_008f2260(member.ptr(0x0c), kKeyName);
    const char* text = host.property_string_value_008f0df0(name);
    const std::uint32_t length = text != nullptr ? static_cast<std::uint32_t>(std::strlen(text)) : 0u;
    host.native_string_resize_0041dd40(member.field(4), length, 0);
    if (member.ptr(8) != nullptr) std::memcpy(member.ptr(8), text, member.at<std::uint32_t>(4));
}

// 00944400. The count is the signed element difference (SAR 2).
void* pointer_range_copy_00944400(void* first, void* last, void* dest) {
    const std::int32_t count =
        static_cast<std::int32_t>(static_cast<std::uint8_t*>(last) - static_cast<std::uint8_t*>(first)) >> 2;
    const std::size_t bytes = static_cast<std::size_t>(count) * 4;
    if (count != 0) memmove_s(dest, bytes, first, bytes);
    return static_cast<std::uint8_t*>(dest) + bytes;
}

std::uint8_t* spawn_member_copy_backward_009444c0(std::uint8_t* first, std::uint8_t* last,
                                                  std::uint8_t* dest_end, Left61Host& host) {
    while (last != first) {
        last -= 0x10;
        dest_end -= 0x10;
        spawn_member_assign(dest_end, last, host);
    }
    return dest_end;
}

namespace {
// 00CE4ADC: the spawn manager's initial +0Ch float (bits D01502F9h, about -1e10).
constexpr std::uint32_t kSpawnManagerInitialBits = 0xd01502f9u;
// 00F89B3C: the spawn manager singleton.
constexpr std::uint32_t kSpawnManagerSingleton = 0x00f89b3c;

float member_key(const std::uint8_t* member) {
    return Raw{reinterpret_cast<std::uint8_t*>(
               static_cast<std::uintptr_t>(Raw{const_cast<std::uint8_t*>(member)}.at<std::uint32_t>(0)))}
        .at<float>(0xb0);
}

// The inlined copy of a record into a by-value argument or local: +0, the name
// through 0041DD40(length, 1) and memcpy, +0Ch.
RecordByValue record_copy(const std::uint8_t* src, Left61Host& host) {
    Raw s{const_cast<std::uint8_t*>(src)};
    RecordByValue copy{s.at<std::uint32_t>(0), 0, nullptr, 0};
    host.native_string_resize_0041dd40(&copy.length, s.at<std::uint32_t>(4), 1);
    if (s.at<std::uint32_t>(4) != 0) std::memcpy(copy.data, s.ptr(8), copy.length);
    copy.tail = s.at<std::uint32_t>(0x0c);
    return copy;
}

// Assignment from a by-value record (the string compare is always unequal).
void record_assign_from(std::uint8_t* dst, const RecordByValue& value, Left61Host& host) {
    Raw d{dst};
    d.at<std::uint32_t>(0) = value.head;
    host.native_string_resize_0041dd40(d.field(4), value.length, 1);
    if (value.length != 0) std::memcpy(d.ptr(8), value.data, d.at<std::uint32_t>(4));
    d.at<std::uint32_t>(0x0c) = value.tail;
}
}  // namespace

Raw spawn_manager_construct_00945820(Raw manager, Left61Host& host) {
    void* head = host.spawn_list_buy_head_node_00943b90(manager);
    manager.ptr(4) = head;
    manager.at<std::uint32_t>(8) = 0;
    host.global_u32(kSpawnManagerSingleton) =
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(manager.p));
    manager.at<std::uint32_t>(0x0c) = kSpawnManagerInitialBits;
    return manager;
}

// 00945C30. The unwind action for the bag allocation (EH state 1) is not modelled.
Raw spawn_member_construct_from_table_00945c30(Raw member, std::int32_t party, void* table,
                                               Left61Host& host) {
    member.at<std::uint32_t>(4) = 0;
    member.at<std::uint32_t>(8) = 0;
    member.at<std::uint32_t>(0) = 0;
    void* memory = host.operator_new_00bf681b(0x114);
    member.ptr(0x0c) = memory != nullptr ? host.property_bag_construct_008f41a0(memory, 0) : nullptr;
    spawn_member_seed_property_bag_009420a0(member, party, host);
    spawn_member_read_lua_table_00944210(member, table, host);
    return member;
}

void record_fill_00945cf0(std::uint8_t* first, std::uint8_t* last, const std::uint8_t* value,
                          Left61Host& host) {
    for (; first != last; first += 0x10) spawn_member_assign(first, value, host);
}

// 00945DA0: the EH frame covers only the constructor call (no unwind action).
void record_construct_00945da0(void* memory, const void* source, Left61Host& host) {
    if (memory != nullptr) host.record_copy_construct_00660540(memory, source);
}

// 00945E00. tmp = 00660540(a); a = b; b = tmp; tmp's name returns to the pool.
// The unwind action for tmp is not modelled.
void record_swap_00945e00(std::uint8_t* a, std::uint8_t* b, Left61Host& host) {
    RecordByValue tmp;
    host.record_copy_construct_00660540(&tmp, a);
    spawn_member_assign(a, b, host);
    record_assign_from(b, tmp, host);
    return_pooled_string(tmp.data, tmp.length, host);
}

// 00945EE0. The child is the right one (2 * hole + 2) unless its key is greater
// than the left one's (FCOMIP, JBE keeps the right one, also on NaN). When the
// last right child equals `bottom`, the left child (bottom - 1) moves up. Then
// the value is copied into the by-value argument of 00944540(first, hole,
// original hole, copy), and this function's own by-value name returns to the
// string pool. The unwind action for the by-value argument is not modelled.
void spawn_member_adjust_heap_00945ee0(std::uint8_t* first, std::int32_t hole, std::int32_t bottom,
                                       const RecordByValue& value, Left61Host& host) {
    const std::int32_t top = hole;
    std::int32_t child = hole * 2 + 2;
    while (child < bottom) {
        if (member_key(first + child * 0x10) > member_key(first + (child - 1) * 0x10)) --child;
        spawn_member_assign(first + hole * 0x10, first + child * 0x10, host);
        hole = child;
        child = child * 2 + 2;
    }
    if (child == bottom) {
        spawn_member_assign(first + hole * 0x10, first + (bottom - 1) * 0x10, host);
        hole = bottom - 1;
    }
    RecordByValue copy{value.head, 0, nullptr, 0};
    host.native_string_resize_0041dd40(&copy.length, value.length, 1);
    if (value.length != 0) std::memcpy(copy.data, value.data, copy.length);
    copy.tail = value.tail;
    spawn_member_push_heap_00944540(first, hole, top, copy.head, copy.length, copy.data, copy.tail, host);
    return_pooled_string(value.data, value.length, host);
}

// 009460B0: MSVC9 random-access _Rotate. The gcd of the element counts gives
// the number of cycles; each cycle lifts the hole value into a local copy
// (released to the pool afterwards), shifts the cycle, and drops it at the end.
// The unwind action for the local copy is not modelled.
void record_rotate_009460b0(std::uint8_t* first, std::uint8_t* middle, std::uint8_t* last,
                            Left61Host& host) {
    const std::int32_t count = static_cast<std::int32_t>(last - first) >> 4;
    const std::int32_t shift = static_cast<std::int32_t>(middle - first) >> 4;
    std::int32_t cycles = count;
    for (std::int32_t factor = shift; factor != 0;) {
        const std::int32_t rest = cycles % factor;
        cycles = factor;
        factor = rest;
    }
    if (cycles < count && 0 < cycles) {
        std::uint8_t* hole = first + cycles * 0x10;
        do {
            const RecordByValue held = record_copy(hole, host);
            std::uint8_t* next = hole;
            std::uint8_t* next1 = hole + shift * 0x10 == last ? first : hole + shift * 0x10;
            while (next1 != hole) {
                spawn_member_assign(next, next1, host);
                next = next1;
                const std::int32_t left = static_cast<std::int32_t>(last - next1) >> 4;
                next1 = shift < left ? next1 + shift * 0x10 : first + (shift - left) * 0x10;
            }
            record_assign_from(next, held, host);
            return_pooled_string(held.data, held.length, host);
            --cycles;
            hole -= 0x10;
        } while (0 < cycles);
    }
}

// 009462B0. *dest = *first, then 00945EE0(first, 0, last - first, copy of the
// value through 00660540), then the by-value name returns to the pool. The
// unwind action for the by-value argument is not modelled.
void spawn_member_pop_heap_009462b0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                    const RecordByValue& value, Left61Host& host) {
    spawn_member_assign(dest, first, host);
    RecordByValue copy;
    host.record_copy_construct_00660540(&copy, &value);
    spawn_member_adjust_heap_00945ee0(first, 0, static_cast<std::int32_t>(last - first) >> 4, copy, host);
    return_pooled_string(value.data, value.length, host);
}

void record_destroy_00946450(Raw record, Left61Host& host) {
    return_pooled_string(static_cast<char*>(record.ptr(8)), record.at<std::uint32_t>(4), host);
}

// 00946480: catch block destroys [dest, current) through 00946450 and rethrows.
std::uint8_t* record_uninit_copy_00946480(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                          Left61Host& host) {
    std::uint8_t* const dest_first = dest;
    try {
        for (; first != last; first += 0x10, dest += 0x10) {
            if (dest == nullptr) continue;
            Raw d{dest};
            Raw s{first};
            d.at<std::uint32_t>(0) = s.at<std::uint32_t>(0);
            d.at<std::uint32_t>(4) = 0;
            d.at<std::uint32_t>(8) = 0;
            host.native_string_resize_0041dd40(d.field(4), s.at<std::uint32_t>(4), 1);
            if (s.at<std::uint32_t>(4) != 0) std::memcpy(d.ptr(8), s.ptr(8), d.at<std::uint32_t>(4));
            d.at<std::uint32_t>(0x0c) = s.at<std::uint32_t>(0x0c);
        }
    } catch (...) {
        for (std::uint8_t* p = dest_first; p != dest; p += 0x10) record_destroy_00946450(Raw{p}, host);
        host.cxx_rethrow_00bf6885();
        throw;
    }
    return dest;
}

// 00946570. hole = count / 2 (signed); for each hole down to 1, element
// hole - 1 is copied into the by-value argument of 00945EE0(first, hole - 1, count).
void spawn_member_make_heap_00946570(std::uint8_t* first, std::uint8_t* last, Left61Host& host) {
    const std::int32_t count = static_cast<std::int32_t>(last - first) >> 4;
    std::int32_t hole = count / 2;
    while (hole > 0) {
        const RecordByValue copy = record_copy(first + (hole - 1) * 0x10, host);
        --hole;
        spawn_member_adjust_heap_00945ee0(first, hole, count, copy, host);
    }
}

// 00946610: each test swaps when the second key is greater than the first
// (FCOMIP, JBE skips also on NaN): (mid, first), (last, mid), (mid, first).
void spawn_member_median3_00946610(std::uint8_t* first, std::uint8_t* mid, std::uint8_t* last,
                                   Left61Host& host) {
    if (member_key(mid) > member_key(first)) record_swap_00945e00(mid, first, host);
    if (member_key(last) > member_key(mid)) record_swap_00945e00(last, mid, host);
    if (member_key(mid) > member_key(first)) record_swap_00945e00(mid, first, host);
}

std::uint8_t* record_uninit_copy_009466a0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* dest,
                                          Left61Host& host) {
    return record_uninit_copy_00946480(first, last, dest, host);
}

namespace {
// 00CFACAC "ResourceUsage", 00CF882C "OwnerPlayer".
constexpr char kKeyResourceUsage[] = "ResourceUsage";

// A member's "ResourceUsage": 0 when absent; the property's +0Ch as a float
// when its type (+4) is 1, else its +0Ch int converted (CVTSI2SS).
float member_resource_usage(const std::uint8_t* member, Left61Host& host) {
    void* bag = Raw{const_cast<std::uint8_t*>(member)}.ptr(0x0c);
    if (!host.property_bag_contains_0048e9f0(bag, kKeyResourceUsage)) return 0.0f;
    Raw property{static_cast<std::uint8_t*>(host.property_bag_find_008f2260(bag, kKeyResourceUsage))};
    return property.at<std::int32_t>(4) == 1 ? property.at<float>(0x0c)
                                             : static_cast<float>(property.at<std::int32_t>(0x0c));
}

std::uintptr_t addr(const void* p) { return reinterpret_cast<std::uintptr_t>(p); }

// The checked-iterator walk shared by 009467B0 and 00946870, with the image's
// _invalid_parameter_noinfo tests in their places. `body` runs for each member
// after the "iterator below last" test.
template <class Body>
void walk_members_checked(Raw members, Left61Host& host, Body body) {
    std::uint8_t* it = static_cast<std::uint8_t*>(members.ptr(4));
    if (addr(it) > addr(members.ptr(8))) host.invalid_parameter_00bf6713();
    for (;;) {
        std::uint8_t* last = static_cast<std::uint8_t*>(members.ptr(8));
        if (addr(members.ptr(4)) > addr(last)) host.invalid_parameter_00bf6713();
        if (members.p == nullptr) host.invalid_parameter_00bf6713();
        if (it == last) break;
        if (members.p == nullptr) host.invalid_parameter_00bf6713();
        if (addr(it) >= addr(members.ptr(8))) host.invalid_parameter_00bf6713();
        body(it);
        if (addr(it) >= addr(members.ptr(8))) host.invalid_parameter_00bf6713();
        it += 0x10;
    }
}
}  // namespace

float members_resource_usage_009467b0(Raw members, Left61Host& host) {
    float sum = 0.0f;
    walk_members_checked(members, host, [&](std::uint8_t* member) {
        sum = member_resource_usage(member, host) + sum;
    });
    return sum;
}

float members_resource_usage_for_player_00946870(Raw members, std::int32_t player, Left61Host& host) {
    float sum = 0.0f;
    walk_members_checked(members, host, [&](std::uint8_t* member) {
        void* bag = Raw{member}.ptr(0x0c);
        std::int32_t owner = -1;
        if (host.property_bag_contains_0048e9f0(bag, kKeyOwnerPlayer)) {
            owner = Raw{static_cast<std::uint8_t*>(host.property_bag_find_008f2260(bag, kKeyOwnerPlayer))}
                        .at<std::int32_t>(0x0c);
        }
        if (owner == player) {
            if (addr(member) >= addr(members.ptr(8))) host.invalid_parameter_00bf6713();
            sum = member_resource_usage(member, host) + sum;
        }
    });
    return sum;
}

// 009469F0. The manager is captured at entry; each pass re-reads [00F89B3C] and
// reports when it changed or is null. Nodes are {next, prev, request}; the
// request's +78h is its team and the request itself is the member vector
// 009467B0 sums.
float queued_resource_usage_for_team_009469f0(std::int32_t team, Left61Host& host) {
    auto manager_now = [&] {
        return reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(host.global_u32(kSpawnManagerSingleton)));
    };
    std::uint8_t* const manager = manager_now();
    Raw m{manager};
    auto* node = *static_cast<std::uint8_t**>(m.ptr(4));
    float sum = 0.0f;
    std::uint8_t* current = manager;
    for (;;) {
        std::uint8_t* head = static_cast<std::uint8_t*>(Raw{current}.ptr(4));
        if (manager == nullptr || manager != current) host.invalid_parameter_00bf6713();
        if (node == head) break;
        if (manager == nullptr) host.invalid_parameter_00bf6713();
        if (node == m.ptr(4)) host.invalid_parameter_00bf6713();
        Raw request{static_cast<std::uint8_t*>(Raw{node}.ptr(8))};
        if (request.at<std::int32_t>(0x78) == team) {
            if (node == m.ptr(4)) host.invalid_parameter_00bf6713();
            sum = members_resource_usage_009467b0(request, host) + sum;
        }
        if (node == m.ptr(4)) host.invalid_parameter_00bf6713();
        node = static_cast<std::uint8_t*>(Raw{node}.ptr(0));
        current = manager_now();
    }
    return sum;
}

// 00946C80: catch block destroys [first, current) through 00946450 and rethrows.
void record_uninit_fill_n_00946c80(std::uint8_t* first, std::uint32_t count, const std::uint8_t* value,
                                   Left61Host& host) {
    std::uint8_t* current = first;
    try {
        for (; count != 0; --count, current += 0x10) {
            if (current == nullptr) continue;
            Raw d{current};
            Raw s{const_cast<std::uint8_t*>(value)};
            d.at<std::uint32_t>(0) = s.at<std::uint32_t>(0);
            d.at<std::uint32_t>(4) = 0;
            d.at<std::uint32_t>(8) = 0;
            if (d.field(4) != s.field(4)) {
                host.native_string_resize_0041dd40(d.field(4), s.at<std::uint32_t>(4), 1);
                if (s.at<std::uint32_t>(4) != 0) std::memcpy(d.ptr(8), s.ptr(8), d.at<std::uint32_t>(4));
            }
            d.at<std::uint32_t>(0x0c) = s.at<std::uint32_t>(0x0c);
        }
    } catch (...) {
        for (std::uint8_t* p = first; p != current; p += 0x10) record_destroy_00946450(Raw{p}, host);
        host.cxx_rethrow_00bf6885();
        throw;
    }
}

// 00946DD0. step = (count + 1) / 8 (signed, CDQ/AND 7/SAR 3).
void spawn_member_median_00946dd0(std::uint8_t* first, std::uint8_t* mid, std::uint8_t* last,
                                  Left61Host& host) {
    const std::int32_t count = static_cast<std::int32_t>(last - first) >> 4;
    if (count > 40) {
        const std::int32_t step = (count + 1) / 8;
        const std::int32_t s = step * 0x10;
        spawn_member_median3_00946610(first, first + s, first + 2 * s, host);
        spawn_member_median3_00946610(mid - s, mid, mid + s, host);
        spawn_member_median3_00946610(last - 2 * s, last - s, last, host);
        spawn_member_median3_00946610(first + s, mid, last - s, host);
        return;
    }
    spawn_member_median3_00946610(first, mid, last, host);
}

// 00946EC0. The size test is ((last - first) & ~0Fh) > 10h (signed). The value
// is a copy of last[-1] through 00660540, and last[-1] is also the destination.
void spawn_member_pop_heap_00946ec0(std::uint8_t* first, std::uint8_t* last, Left61Host& host) {
    if (static_cast<std::int32_t>(static_cast<std::uint32_t>(last - first) & 0xfffffff0u) > 0x10) {
        RecordByValue value;
        host.record_copy_construct_00660540(&value, last - 0x10);
        spawn_member_pop_heap_009462b0(first, last - 0x10, last - 0x10, value, host);
    }
}

namespace {
// The unit walk of 00946F00 / 00946FC0: game+19CCh -> +58h list {+4 next, +8
// unit}; a unit counts when +5Ch is set and +5Dh, +60h, +5Eh are clear; a kind
// 0Fh unit (vtable +5Ch) counts only when it leads its flight, through its
// +9D4h owner.
template <class Visit>
void walk_live_units(Left61Host& host, Visit visit) {
    Raw game{game_object(host)};
    auto* node = static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(game.ptr(0x19cc))}.ptr(0x58));
    for (; node != nullptr; node = static_cast<std::uint8_t*>(Raw{node}.ptr(4))) {
        auto* unit = static_cast<std::uint8_t*>(Raw{node}.ptr(8));
        Raw u{unit};
        if (u.at<std::uint8_t>(0x5c) == 0 || u.at<std::uint8_t>(0x5d) != 0 || u.at<std::uint8_t>(0x60) != 0 ||
            u.at<std::uint8_t>(0x5e) != 0) {
            continue;
        }
        if (host.unit_is_kind_slot5c(unit, 0x0f)) {
            if (!host.unit_is_squadron_flight_leader_007b8ad0(unit)) continue;
            unit = static_cast<std::uint8_t*>(u.ptr(0x9d4));
        }
        visit(Raw{unit});
    }
}
}  // namespace

// 00946F00. The cost is subtracted when it is above 0 (COMISS, JBE skips also
// NaN). The final difference runs on the double of the running value (FSUBR
// double) and is stored to float.
float player_support_points_00946f00(std::int32_t player, Left61Host& host) {
    float points = team_support_points_cap_00942130(player, host);
    walk_live_units(host, [&](Raw unit) {
        if (host.unit_loss_counting_slot_009ffd20(unit.p) != player) return;
        const float cost = unit.at<float>(0x304);
        if (cost > 0.0f) points = points - cost;
    });
    void* manager = reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(kSpawnManagerSingleton)));
    return points - host.spawn_manager_player_usage_00946970(manager, player);
}

// 00946FC0. The cost test is UCOMISS/LAHF/TEST AH,44h/JNP: it skips only an
// exact 0 (NaN is subtracted). The team is unit+54h.
float team_available_resources_00946fc0(std::int32_t team, Left61Host& host) {
    float total;
    const std::uint32_t bits = host.global_u32(kSupportPointsTotal);
    std::memcpy(&total, &bits, sizeof total);
    float points = static_cast<float>(total * kHalf);
    walk_live_units(host, [&](Raw unit) {
        const float cost = unit.at<float>(0x304);
        if (cost != 0.0f && unit.at<std::int32_t>(0x54) == team) points = points - cost;
    });
    return points - queued_resource_usage_for_team_009469f0(team, host);
}

// 009473B0. Written from the MSVC9 <algorithm> source, checked against the
// decompiler output and the nine swap sites in the listing (00947587..00947657:
// argument order as in the source). The image unrolls the pivot-extension scan
// four entries per step; the result is the same. pred is key(a) > key(b), as in
// 00946610's compares.
void spawn_member_unguarded_partition_009473b0(std::uint8_t** out, std::uint8_t* first, std::uint8_t* last,
                                               Left61Host& host) {
    auto pred = [](const std::uint8_t* a, const std::uint8_t* b) { return member_key(a) > member_key(b); };
    std::uint8_t* mid = first + ((static_cast<std::int32_t>(last - first) >> 4) / 2) * 0x10;
    spawn_member_median_00946dd0(first, mid, last - 0x10, host);
    std::uint8_t* pfirst = mid;
    std::uint8_t* plast = pfirst + 0x10;
    while (first < pfirst && !pred(pfirst - 0x10, pfirst) && !pred(pfirst, pfirst - 0x10)) pfirst -= 0x10;
    while (plast < last && !pred(plast, pfirst) && !pred(pfirst, plast)) plast += 0x10;
    std::uint8_t* gfirst = plast;
    std::uint8_t* glast = pfirst;
    for (;;) {
        for (; gfirst < last; gfirst += 0x10) {
            if (pred(pfirst, gfirst)) {
            } else if (pred(gfirst, pfirst)) {
                break;
            } else {
                record_swap_00945e00(plast, gfirst, host);
                plast += 0x10;
            }
        }
        for (; first < glast; glast -= 0x10) {
            if (pred(glast - 0x10, pfirst)) {
            } else if (pred(pfirst, glast - 0x10)) {
                break;
            } else {
                pfirst -= 0x10;
                record_swap_00945e00(pfirst, glast - 0x10, host);
            }
        }
        if (glast == first && gfirst == last) {
            out[0] = pfirst;
            out[1] = plast;
            return;
        }
        if (glast == first) {
            if (plast != gfirst) record_swap_00945e00(pfirst, plast, host);
            plast += 0x10;
            record_swap_00945e00(pfirst, gfirst, host);
            pfirst += 0x10;
            gfirst += 0x10;
        } else if (gfirst == last) {
            glast -= 0x10;
            pfirst -= 0x10;
            if (glast != pfirst) record_swap_00945e00(glast, pfirst, host);
            plast -= 0x10;
            record_swap_00945e00(pfirst, plast, host);
        } else {
            glast -= 0x10;
            record_swap_00945e00(gfirst, glast, host);
            gfirst += 0x10;
        }
    }
}

// 00947680. The candidate's key is stored to float and compared with FCOMI:
// greater than the first key -> rotate it to the front; otherwise walk back
// while it is greater than the previous key (unguarded; NaN stops at once).
// The rotate runs only when the insertion point differs from the candidate.
void spawn_member_insertion_sort_00947680(std::uint8_t* first, std::uint8_t* last, Left61Host& host) {
    if (first == last) return;
    for (std::uint8_t* next = first + 0x10; next != last; next += 0x10) {
        const float value = member_key(next);
        std::uint8_t* at;
        if (value > member_key(first)) {
            at = first;
        } else {
            at = next;
            while (value > member_key(at - 0x10)) at -= 0x10;
        }
        if (at != next && next != next + 0x10) record_rotate_009460b0(at, next, next + 0x10, host);
    }
}

// 00947750. Each step copies last[-1] into the by-value argument and calls
// 009462B0(first, last - 1, dest = last - 1).
void spawn_member_sort_heap_00947750(std::uint8_t* first, std::uint8_t* last, Left61Host& host) {
    while ((static_cast<std::int32_t>(last - first) >> 4) > 1) {
        const RecordByValue value = record_copy(last - 0x10, host);
        spawn_member_pop_heap_009462b0(first, last - 0x10, last - 0x10, value, host);
        last -= 0x10;
    }
}

// 009480B0. The partition halves are compared by their byte spans masked to
// whole elements (signed), as in the listing; the make_heap step runs only when
// the masked span is above one element.
void spawn_member_sort_009480b0(std::uint8_t* first, std::uint8_t* last, std::int32_t ideal,
                                Left61Host& host) {
    std::int32_t count = static_cast<std::int32_t>(last - first) >> 4;
    while (count > 32) {
        if (ideal < 1) {
            if (static_cast<std::int32_t>(static_cast<std::uint32_t>(last - first) & 0xfffffff0u) > 0x10) {
                spawn_member_make_heap_00946570(first, last, host);
            }
            spawn_member_sort_heap_00947750(first, last, host);
            return;
        }
        std::uint8_t* mid[2];
        spawn_member_unguarded_partition_009473b0(mid, first, last, host);
        ideal = ideal / 2;
        ideal += ideal / 2;
        const auto low = static_cast<std::int32_t>(static_cast<std::uint32_t>(mid[0] - first) & 0xfffffff0u);
        const auto high = static_cast<std::int32_t>(static_cast<std::uint32_t>(last - mid[1]) & 0xfffffff0u);
        if (low < high) {
            spawn_member_sort_009480b0(first, mid[0], ideal, host);
            first = mid[1];
        } else {
            spawn_member_sort_009480b0(mid[1], last, ideal, host);
            last = mid[0];
        }
        count = static_cast<std::int32_t>(last - first) >> 4;
    }
    if (count > 1) spawn_member_insertion_sort_00947680(first, last, host);
}

// 009481A0. Keys 00D19968 ownHorizontal, 00D19958 enemyHorizontal, 00D1994C
// ownVertical, 00D1993C enemyVertical, 00D19928 formationHorizontal go to
// block+0..+10h; each read passes {0, key}, {2, &block[i]} and {2, block[i]}
// (the current value as the default). Then +14h = [0]^2 and +18h = [1]^2, each
// value reloaded from float before the FMUL ST0. The reader lives in a 14h-byte
// frame slot ([ESP+0Ch] of the frame). The unwind actions (reader, by-value
// object) are not modelled.
void spawn_request_read_exclude_radius_009481a0(float* block, void* object, Left61Host& host) {
    static const char* const kKeys[5] = {"ownHorizontal", "enemyHorizontal", "ownVertical", "enemyVertical",
                                         "formationHorizontal"};
    if (!host.lua_object_is_nil_00b65fb0(object)) {
        alignas(4) std::uint8_t copy[0x14];
        host.lua_object_copy_construct_00b66fa0(copy, object);
        alignas(4) std::uint8_t reader[0x14];
        host.lua_reader_construct_004425c0(reader, copy);
        for (int i = 0; i < 5; ++i) {
            std::uint32_t value_bits;
            std::memcpy(&value_bits, &block[i], sizeof value_bits);
            const std::uint32_t fallback[2] = {2, value_bits};
            const std::uint32_t destination[2] = {2, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&block[i]))};
            const std::uint32_t key[2] = {0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(kKeys[i]))};
            host.lua_reader_read_field_or_default_00bd68d0(reader, key, destination, fallback);
        }
        block[5] = block[0] * block[0];
        block[6] = block[1] * block[1];
        host.lua_reader_destroy_00441a20(reader);
    }
    host.lua_object_destruct_00b67700(object);
}

// 00948BB0. In order: the dword vector at +0CCh (storage +0D0h) through operator
// delete, the name at +0B8h to the string pool (not cleared), the vector at
// +8Ch (storage +90h), the string at +84h, the callback owner at +10h, then the
// member vector at +0 (00946D70 over [+4, +8), operator delete of +4). The
// unwind states (3, 1, 0) only matter if one of these calls throws and are not
// modelled.
void spawn_request_destroy_00948bb0(Raw request, Left61Host& host) {
    if (request.ptr(0xd0) != nullptr) host.operator_delete_00bf65ac(request.ptr(0xd0));
    request.at<std::uint32_t>(0xd0) = 0;
    request.at<std::uint32_t>(0xd4) = 0;
    request.at<std::uint32_t>(0xd8) = 0;
    return_pooled_string(static_cast<char*>(request.ptr(0xbc)), request.at<std::uint32_t>(0xb8), host);
    if (request.ptr(0x90) != nullptr) host.operator_delete_00bf65ac(request.ptr(0x90));
    request.at<std::uint32_t>(0x90) = 0;
    request.at<std::uint32_t>(0x94) = 0;
    request.at<std::uint32_t>(0x98) = 0;
    return_pooled_string(static_cast<char*>(request.ptr(0x88)), request.at<std::uint32_t>(0x84), host);
    host.callback_owner_destroy_00695870(request.field(0x10));
    if (request.ptr(4) != nullptr) {
        host.member_destroy_range_00946d70(request.ptr(4), request.ptr(8), request.p);
        host.operator_delete_00bf65ac(request.ptr(4));
    }
    request.at<std::uint32_t>(4) = 0;
    request.at<std::uint32_t>(8) = 0;
    request.at<std::uint32_t>(0x0c) = 0;
}

// 009496A0. The singleton is cleared first. Each queued request gets 009442A0
// (also when null), then, when set, 00948BB0 and operator delete; the walk
// reports through 00BF6713 when the node is the head. The list is then cleared
// and its nodes and head freed.
void spawn_manager_destroy_009496a0(Raw manager, Left61Host& host) {
    host.global_u32(kSpawnManagerSingleton) = 0;
    auto* node = *static_cast<std::uint8_t**>(manager.ptr(4));
    while (node != manager.ptr(4)) {
        void* request = Raw{node}.ptr(8);
        host.spawn_request_release_members_009442a0(request);
        if (request != nullptr) {
            spawn_request_destroy_00948bb0(Raw{static_cast<std::uint8_t*>(request)}, host);
            host.operator_delete_00bf65ac(request);
        }
        if (node == manager.ptr(4)) host.invalid_parameter_00bf6713();
        node = static_cast<std::uint8_t*>(Raw{node}.ptr(0));
    }
    Raw head{static_cast<std::uint8_t*>(manager.ptr(4))};
    auto* entry = static_cast<std::uint8_t*>(head.ptr(0));
    head.ptr(0) = head.p;
    Raw{static_cast<std::uint8_t*>(manager.ptr(4))}.ptr(4) = manager.ptr(4);
    manager.at<std::uint32_t>(8) = 0;
    while (entry != manager.ptr(4)) {
        auto* next = static_cast<std::uint8_t*>(Raw{entry}.ptr(0));
        host.operator_delete_00bf65ac(entry);
        entry = next;
    }
    host.operator_delete_00bf65ac(manager.ptr(4));
    manager.ptr(4) = nullptr;
}

// 0094C830. The message is a 5Ch-byte frame local; it is destroyed by 00766DA0
// on the way out (the unwind action is not modelled).
void spawn_manager_send_spawn_request_0094c830(Raw manager, const std::uint32_t (&args)[8], Left61Host& host) {
    alignas(4) std::uint8_t message[0x5c];
    const std::uint32_t ctor_args[7] = {args[0], args[1], args[3], args[2], args[4], args[5], args[6]};
    host.spawn_message_construct_00947bc0(message, ctor_args);
    Raw game{game_object(host)};
    const std::int32_t mode = game.at<std::int32_t>(0x1fe4);
    if (mode == 0 || mode == 1) {
        host.spawn_manager_handle_message_0094b600(manager.p, message, args[7]);
    } else {
        host.session_send_to_secondary_00770af0(game.field(0x1ef0), message);
    }
    host.session_message_destroy_00766da0(message);
}

void ordered_map_delete_subtree_0094ca10(void* tree, void* node, Left61Host& host) {
    if (node == nullptr) return;
    Raw n{static_cast<std::uint8_t*>(node)};
    ordered_map_delete_subtree_0094ca10(tree, n.ptr(4), host);
    ordered_map_delete_subtree_0094ca10(tree, n.ptr(8), host);
    host.object_delete_slot0(node);
}

// 0094CA40. The root (+4) subtree is deleted inline: left and right through
// 0094CA10, then the root's deleting destructor.
void ordered_map_destruct_0094ca40(Raw tree, Left61Host& host) {
    tree.at<std::uint32_t>(0) = 0x00d2505cu;
    void* root = tree.ptr(4);
    if (root != nullptr) {
        Raw r{static_cast<std::uint8_t*>(root)};
        ordered_map_delete_subtree_0094ca10(tree.p, r.ptr(4), host);
        ordered_map_delete_subtree_0094ca10(tree.p, r.ptr(8), host);
        host.object_delete_slot0(root);
    }
    tree.at<std::uint32_t>(0) = 0x00d24f84u;
}

namespace {
// 00D19B8C "used": the archive section name.
constexpr char kUsedSection[] = "used";

std::uint32_t ptr_bits(const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); }

// Unlink a {next, prev} entry and insert it after the sentinel (next = +0).
void relink_after(std::uint8_t* entry, std::uint8_t* sentinel) {
    Raw e{entry};
    Raw{static_cast<std::uint8_t*>(e.ptr(0))}.ptr(4) = e.ptr(4);
    Raw{static_cast<std::uint8_t*>(e.ptr(4))}.ptr(0) = e.ptr(0);
    e.ptr(4) = sentinel;
    e.ptr(0) = Raw{sentinel}.ptr(0);
    Raw{static_cast<std::uint8_t*>(e.ptr(0))}.ptr(4) = entry;
    Raw{static_cast<std::uint8_t*>(e.ptr(4))}.ptr(0) = entry;
}
}  // namespace

// 00951820. The read destination is the caller's archive argument slot (the
// image reuses [ESP+20h]); only its low word, the id, is used. For each id:
// when +50h is 0 the free list is rebuilt first; the entry (id 0: the +10h
// pointer, else +4Ch + (id - +4) * 10h) is tagged DEEEDEEEh, moved to the +3Ch
// list and +50h drops; then the entry for the id (always the computed one) is
// tagged 0, moved to the +1Ch list and +50h rises.
void entity_id_table_load_used_00951820(Raw table, void* archive, Left61Host& host) {
    const std::uint32_t section[2] = {0, ptr_bits(kUsedSection)};
    host.archive_open_slot04(archive, section);
    // The slot starts as the archive pointer (it is the argument slot).
    std::uint32_t slot = ptr_bits(archive);
    for (std::int32_t i = 0;; ++i) {
        const std::uint32_t key[2] = {1, static_cast<std::uint32_t>(i)};
        if (!host.archive_has_slot14(archive, key)) break;
        const std::uint32_t destination[2] = {1, ptr_bits(&slot)};
        host.archive_read_slot10(archive, 1, i, destination);
        const auto id = static_cast<std::uint16_t>(slot);
        if (table.at<std::int32_t>(0x50) == 0) host.entity_id_table_rebuild_free_list_00951720(table.p);
        auto computed = [&] {
            return static_cast<std::uint8_t*>(table.ptr(0x4c)) +
                   (static_cast<std::int32_t>(id) - table.at<std::int32_t>(4)) * 0x10;
        };
        std::uint8_t* entry = id == 0 ? static_cast<std::uint8_t*>(table.ptr(0x10)) : computed();
        Raw{entry}.at<std::uint32_t>(0x0c) = 0xdeeedeeeu;
        relink_after(entry, table.field(0x3c));
        table.at<std::int32_t>(0x50) -= 1;
        entry = computed();
        Raw{entry}.at<std::uint32_t>(0x0c) = 0;
        relink_after(entry, table.field(0x1c));
        table.at<std::int32_t>(0x50) += 1;
    }
    host.archive_close_slot08(archive);
}

// 00951940. The walk from the +10h entry skips a leading run of consecutive
// ids exactly as the listing does (00951949..0095199B); every entry from there
// whose next is set is written as {1, id}. Then every unit of game+19CCh
// (+4 -> first, +38h next) with +5Eh set and vtable +5Ch(1) true whose word
// +174h lies in [+4, +4 + +8) is written.
void entity_id_table_save_used_00951940(Raw table, void* archive, Left61Host& host) {
    auto id_of = [](std::uint8_t* e) { return static_cast<std::uint32_t>(Raw{e}.at<std::uint16_t>(8)); };
    auto next_of = [](std::uint8_t* e) { return static_cast<std::uint8_t*>(Raw{e}.ptr(4)); };
    std::uint8_t* e = static_cast<std::uint8_t*>(table.ptr(0x10));
    std::uint8_t* n = next_of(e);
    if (n != nullptr && next_of(n) != nullptr && id_of(e) + 1 == id_of(n)) {
        std::uint32_t previous = id_of(e);
        e = n;
        n = next_of(e);
        while (n != nullptr) {
            const std::uint32_t current = id_of(e);
            if (current != previous + 1) break;
            e = n;
            n = next_of(e);
            previous = current;
        }
    }
    const std::uint32_t section[2] = {0, ptr_bits(kUsedSection)};
    host.archive_open_slot04(archive, section);
    std::int32_t index = 0;
    while (next_of(e) != nullptr) {
        const std::uint32_t value[2] = {1, id_of(e)};
        host.archive_write_slot0c(archive, 1, index, value);
        e = next_of(e);
        index += 1;
    }
    Raw game{game_object(host)};
    auto* unit = *static_cast<std::uint8_t**>(Raw{static_cast<std::uint8_t*>(game.ptr(0x19cc))}.ptr(4));
    for (; unit != nullptr; unit = static_cast<std::uint8_t*>(Raw{unit}.ptr(0x38))) {
        if (Raw{unit}.at<std::uint8_t>(0x5e) == 0 || !host.unit_is_kind_slot5c(unit, 1)) continue;
        const auto id = static_cast<std::int32_t>(Raw{unit}.at<std::uint16_t>(0x174));
        if (table.at<std::int32_t>(4) <= id && id < table.at<std::int32_t>(8) + table.at<std::int32_t>(4)) {
            const std::uint32_t value[2] = {1, static_cast<std::uint32_t>(id)};
            host.archive_write_slot0c(archive, 1, index, value);
            index += 1;
        }
    }
    host.archive_close_slot08(archive);
}

// 0094C900. Each vector is a 10h-byte frame object {allocator, first, last,
// end} zeroed before its push_back; they are freed (operator delete of
// `first`, then zeroed) in the order a5, a4, a1. The unwind actions are not
// modelled.
void spawn_manager_send_single_0094c900(Raw manager, const std::uint32_t (&args)[8], Left61Host& host) {
    std::uint32_t v1[4] = {0, 0, 0, 0};
    std::uint32_t v4[4] = {0, 0, 0, 0};
    std::uint32_t v5[4] = {0, 0, 0, 0};
    host.vector_push_back_00947fd0(v1, &args[1]);
    host.vector_push_back_00442190(v4, &args[4]);
    host.vector_push_back_00442190(v5, &args[5]);
    const std::uint32_t forwarded[8] = {args[0], ptr_bits(v1), args[2], args[3], ptr_bits(v4), ptr_bits(v5),
                                        args[6], args[7]};
    spawn_manager_send_spawn_request_0094c830(manager, forwarded, host);
    std::uint32_t* const vectors[3] = {v5, v4, v1};
    for (std::uint32_t* v : vectors) {
        if (v[1] != 0) host.operator_delete_00bf65ac(reinterpret_cast<void*>(static_cast<std::uintptr_t>(v[1])));
        v[1] = 0;
        v[2] = 0;
        v[3] = 0;
    }
}

// 0094EEF0. The temporary std::string (1Ch bytes: +4 buffer, +14h size, +18h
// capacity) starts empty with capacity 0Fh; after the push_back its heap
// buffer is freed when the capacity is 10h or more. The loop end is recomputed
// from the list each pass. The unwind action for the temporary is not modelled.
void append_entry_texts_0094eef0(Raw list, void* strings, Left61Host& host) {
    static const char kEmpty[1] = {0};
    for (auto* entry = static_cast<std::uint8_t*>(list.ptr(0));
         entry != static_cast<std::uint8_t*>(list.ptr(0)) + list.at<std::int32_t>(4) * 8; entry += 8) {
        const char* text = static_cast<const char*>(Raw{entry}.ptr(4));
        if (text == nullptr) text = kEmpty;
        alignas(4) std::uint8_t temp[0x1c] = {};
        Raw t{temp};
        t.at<std::uint32_t>(0x18) = 0x0f;
        t.at<std::uint32_t>(0x14) = 0;
        t.at<std::uint8_t>(4) = 0;
        host.std_string_assign_00408720(temp, text, static_cast<std::uint32_t>(std::strlen(text)));
        host.string_vector_push_back_00849170(strings, temp);
        if (t.at<std::uint32_t>(0x18) >= 0x10) host.operator_delete_00bf65ac(t.ptr(4));
    }
}

// 009510F0. Floats: +1Ch 10.0 (00CE38B8), +18h 40.0 (00CE685C), +24h 5.0
// (00CE3850), +28h/+2Ch/+30h 0.0.
Raw traffic_record_construct_009510f0(Raw object, std::uint32_t unused, std::uint32_t value, Left61Host& host) {
    (void)unused;
    host.base_construct_0049f9b0(object.p);
    object.at<std::uint32_t>(0) = value;
    object.at<std::int32_t>(4) = -1;
    object.at<std::int32_t>(8) = -1;
    object.at<float>(0x1c) = 10.0f;
    object.at<float>(0x18) = 40.0f;
    object.at<std::uint32_t>(0x10) = 0;
    object.at<std::uint8_t>(0x0c) = 0;
    object.at<std::uint32_t>(0x14) = 1;
    object.at<float>(0x2c) = 0.0f;
    object.at<float>(0x30) = 0.0f;
    object.at<float>(0x24) = 5.0f;
    object.at<float>(0x28) = 0.0f;
    return object;
}

void traffic_record_vector_destroy_00951450(Raw vector, Left61Host& host) {
    host.traffic_record_vector_clear_00951160(vector.p);
    if (vector.ptr(4) != nullptr) host.operator_delete_00bf65ac(vector.ptr(4));
    vector.at<std::uint32_t>(4) = 0;
    vector.at<std::uint32_t>(8) = 0;
    vector.at<std::uint32_t>(0x0c) = 0;
}

// --- effect entities -------------------------------------------------------------

namespace {
// 00D09B84 "Effect", 00D19B68 "Period", 00D0D404 "Delay".
constexpr char kKeyEffect[] = "Effect";
constexpr char kKeyPeriod[] = "Period";
constexpr char kKeyDelay[] = "Delay";

void release_counted(void*& slot, Left61Host& host) {
    auto* object = static_cast<std::uint8_t*>(slot);
    if (object == nullptr) return;
    if (host.interlocked_decrement_ce2220(reinterpret_cast<volatile long*>(object + 4)) == 0) {
        host.object_release_slot0(object);
    }
    slot = nullptr;
}

// The image's inlined counted-pointer assignment: nothing when equal; else
// store, InterlockedIncrement (00CE221C) the new one, release the old one.
void assign_counted(void*& slot, void* value, Left61Host& host) {
    void* old = slot;
    if (old == value) return;
    slot = value;
    if (value != nullptr) {
        host.interlocked_increment_ce221c(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(value) + 4));
    }
    if (old != nullptr &&
        host.interlocked_decrement_ce2220(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(old) + 4)) == 0) {
        host.object_release_slot0(old);
    }
}

// The "Effect" handle step of 0094F910 / 0094FC60: Find (no Contains test),
// 00870CD0(&tmp, property+0Ch, 0), counted assignment into the slot, then the
// temporary handle released (its slot is not cleared; it is a local).
void take_effect_definition(void* bag, void*& slot, Left61Host& host) {
    Raw property{static_cast<std::uint8_t*>(host.property_bag_find_008f2260(bag, kKeyEffect))};
    void* temp = nullptr;
    void** handle = host.effect_handle_acquire_00870cd0(&temp, property.at<std::uint32_t>(0x0c), 0);
    assign_counted(slot, *handle, host);
    if (temp != nullptr &&
        host.interlocked_decrement_ce2220(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(temp) + 4)) == 0) {
        host.object_release_slot0(temp);
    }
}

// A property's float: the +0Ch float when its type is 1, else the int converted;
// `fallback` when the bag does not contain the key.
float bag_float(void* bag, const char* key, float fallback, Left61Host& host) {
    if (!host.property_bag_contains_0048e9f0(bag, key)) return fallback;
    Raw property{static_cast<std::uint8_t*>(host.property_bag_find_008f2260(bag, key))};
    return property.at<std::int32_t>(4) == 1 ? property.at<float>(0x0c)
                                             : static_cast<float>(property.at<std::int32_t>(0x0c));
}
}  // namespace

// 0094F830. Vtables 00CE8BD0 (+0), 00CE8BB4 (+10h), 00CE8BAC (+24h), 00CE8BA8
// (+170h); the live effect gets 00867B10 and byte +9 = 1 when set, then +1E8h
// and +1E4h are released and 009287B0 runs. Unwind states not modelled.
void effect_entity_destruct_0094f830(Raw entity, Left61Host& host) {
    entity.at<std::uint32_t>(0) = 0x00ce8bd0u;
    entity.at<std::uint32_t>(0x10) = 0x00ce8bb4u;
    entity.at<std::uint32_t>(0x24) = 0x00ce8bacu;
    entity.at<std::uint32_t>(0x170) = 0x00ce8ba8u;
    void* effect = entity.ptr(0x1e8);
    if (effect != nullptr) {
        host.point_effect_stop_children_00867b10(effect);
        Raw{static_cast<std::uint8_t*>(effect)}.at<std::uint8_t>(9) = 1;
    }
    release_counted(entity.ptr(0x1e8), host);
    release_counted(entity.ptr(0x1e4), host);
    host.game_entity_destruct_009287b0(entity.p);
}

void effect_entity_activate_0094f910(Raw entity, Left61Host& host) {
    host.entity_activate_009277e0(entity.p);
    Raw descriptor{static_cast<std::uint8_t*>(entity.ptr(0xc0))};
    if (descriptor.at<std::int32_t>(4) != 1) return;
    take_effect_definition(descriptor.ptr(8), entity.ptr(0x1e4), host);
}

// 0094FB50. countdown = +38h - seconds (stored to float, also back into the
// argument slot); FLDZ/FCOMIP/JC returns while 0 < countdown (and on NaN).
// Otherwise +38h = +34h; with a definition (+3Ch): the entity (updater -
// 1E4h) pose is refreshed when its +0C8h is clear, 00868420(&tmp, game+19ECh,
// definition with an added reference, entity+0CCh, 0, 0), 004845D0(+40h,
// &tmp), tmp released, and the live effect's byte +9 = 1 when set.
void periodic_effect_update_0094fb50(Raw updater, float seconds, Left61Host& host) {
    const float countdown = updater.at<float>(0x38) - seconds;
    updater.at<float>(0x38) = countdown;
    if (0.0f < countdown || countdown != countdown) return;
    void* definition = updater.ptr(0x3c);
    updater.at<float>(0x38) = updater.at<float>(0x34);
    if (definition == nullptr) return;
    Raw entity{updater.p - 0x1e4};
    if (entity.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_00414db0(entity.p);
    const std::uint32_t kind = Raw{game_object(host)}.at<std::uint32_t>(0x19ec);
    void* passed = nullptr;
    if (updater.ptr(0x3c) != nullptr) {
        passed = updater.ptr(0x3c);
        host.interlocked_increment_ce221c(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(passed) + 4));
    }
    void* temp = nullptr;
    void** created = host.point_effect_create_00868420(&temp, kind, passed,
                                                       reinterpret_cast<const float*>(entity.field(0xcc)), 0, 0);
    host.handle_assign_004845d0(updater.field(0x40), created);
    if (temp != nullptr &&
        host.interlocked_decrement_ce2220(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(temp) + 4)) == 0) {
        host.object_release_slot0(temp);
    }
    void* live = updater.ptr(0x40);
    if (live != nullptr) Raw{static_cast<std::uint8_t*>(live)}.at<std::uint8_t>(9) = 1;
}

// 0094FC60. Period default 1.0 (00D7A24C), Delay default 0.
void periodic_effect_entity_activate_0094fc60(Raw entity, Left61Host& host) {
    host.entity_activate_009277e0(entity.p);
    Raw descriptor{static_cast<std::uint8_t*>(entity.ptr(0xc0))};
    if (descriptor.at<std::int32_t>(4) != 1) return;
    entity.at<float>(0x218) = bag_float(descriptor.ptr(8), kKeyPeriod, 1.0f, host);
    entity.at<float>(0x21c) =
        bag_float(Raw{static_cast<std::uint8_t*>(entity.ptr(0xc0))}.ptr(8), kKeyDelay, 0.0f, host);
    take_effect_definition(Raw{static_cast<std::uint8_t*>(entity.ptr(0xc0))}.ptr(8), entity.ptr(0x220), host);
}

// 00944540. Parent index (hole - 1) / 2 with signed truncation (CDQ/SUB/SAR).
// The loop stops when the parent's key is not greater than the value's (FCOMIP,
// JBE: also on NaN). The value then fills the hole and its by-value name
// returns to the string pool. The unwind action for the by-value name is not
// modelled.
void spawn_member_push_heap_00944540(std::uint8_t* first, std::int32_t hole, std::int32_t top,
                                     std::uint32_t value_class, std::uint32_t value_length,
                                     char* value_data, std::uint32_t value_bag, Left61Host& host) {
    auto key = [](std::uint32_t vehicle_class) {
        return Raw{reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(vehicle_class))}.at<float>(0xb0);
    };
    while (top < hole) {
        const std::int32_t parent = (hole - 1) / 2;
        std::uint8_t* p = first + parent * 0x10;
        if (!(key(Raw{p}.at<std::uint32_t>(0)) > key(value_class))) break;
        spawn_member_assign(first + hole * 0x10, p, host);
        hole = parent;
    }
    Raw slot{first + hole * 0x10};
    slot.at<std::uint32_t>(0) = value_class;
    host.native_string_resize_0041dd40(slot.field(4), value_length, 1);
    if (value_length != 0) std::memcpy(slot.ptr(8), value_data, slot.at<std::uint32_t>(4));
    slot.at<std::uint32_t>(0x0c) = value_bag;
    return_pooled_string(value_data, value_length, host);
}

}  // namespace bsp::cc13_left61
