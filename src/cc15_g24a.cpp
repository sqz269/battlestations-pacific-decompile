// Game-code rows from the cc15-stlA audit (packet cc15-g24a). Evidence and
// ABI: include/bsp/cc15_g24a.hpp and docs/CC15_G24A.md.
#include "bsp/cc15_g24a.hpp"

#include <cstring>

namespace bsp::cc15_g24a {
namespace {

template <typename T>
T load(const std::uint8_t* object, std::uint32_t offset) {
    T value;
    std::memcpy(&value, object + offset, sizeof value);
    return value;
}

template <typename T>
void store(std::uint8_t* object, std::uint32_t offset, T value) {
    std::memcpy(object + offset, &value, sizeof value);
}

// Win32 pointer words.
std::uint8_t* ptr(std::uint32_t word) {
    return reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(word));
}
std::uint32_t word(const void* pointer) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
}
std::uint8_t* load_ptr(const std::uint8_t* object, std::uint32_t offset) {
    return ptr(load<std::uint32_t>(object, offset));
}

// (last - first) >> 2 with SAR, compared unsigned by the callers (JC/JNC).
std::uint32_t element_count(std::uint32_t first, std::uint32_t last) {
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(last - first) >> 2);
}

// The inlined VC9 vector erase(begin(), end()) shared by 0045DDA0 and 004AD380.
// `stale_first` is the first word the preceding loop read last (ECX). The
// tail move is (old last - erased last) elements, always 0 here.
template <typename Host>
void erase_all(std::uint8_t* vector_first_slot, std::uint32_t stale_first, Host& host) {
    const std::uint32_t erased_last = load<std::uint32_t>(vector_first_slot, 4);  // EBX
    if (stale_first > erased_last) host.invalid_parameter_00bf6713();
    const std::uint32_t first = load<std::uint32_t>(vector_first_slot, 0);        // EDI
    if (first > load<std::uint32_t>(vector_first_slot, 4)) host.invalid_parameter_00bf6713();
    if (first == erased_last) return;
    const std::int32_t tail =
        static_cast<std::int32_t>(load<std::uint32_t>(vector_first_slot, 4) - erased_last) >> 2;
    const std::uint32_t bytes = static_cast<std::uint32_t>(tail) * 4u;
    const std::uint32_t new_last = bytes + first;
    if (tail > 0) host.memmove_s_00bf67a7(ptr(first), bytes, ptr(erased_last), bytes);  // JLE skips
    store<std::uint32_t>(vector_first_slot, 4, new_last);
}

void release_string(std::uint32_t length, std::uint32_t data, MovieSplineAddPointHost& host) {
    if (data == 0) return;
    host.sized_pool_return_block_00bd1510(host.sized_pool_singleton_00419cc0(), ptr(data), length + 1u, 1u);
}

} // namespace

// 0045B420-0045B897. EBX is 0 throughout; the EH states 1/2/3 only cover the
// record's string while the list insert runs (unwind not modelled). The
// CMP ESI,ESI at 0045B450 always sets ZF, so the 00BF6713 at 0045B45C is dead.
void movie_spline_add_point_0045b420(std::uint8_t* owner, const float* position, float time,
                                     const float* in_tangent, const float* out_tangent,
                                     std::int32_t type, float alpha, std::uint8_t* comment,
                                     MovieSplineAddPointHost& host) {
    std::uint8_t* const list = owner + 0xF4;                       // ESI
    std::uint8_t* const head = load_ptr(owner, 0xF8);              // EBP
    std::uint8_t* const begin = load_ptr(head, 0);                 // [ESP+14h]
    alignas(4) std::uint8_t record[0x38];                          // [ESP+18h]
    alignas(4) std::uint8_t copy[8];                               // outgoing by-value slot

    // Inline insert before begin (0045B573-0045B59B / 0045B6E9-0045B711): the
    // head and its first node are re-read after 00465A40.
    const auto insert_front = [&](std::uint8_t* value) {
        std::uint8_t* const first = load_ptr(load_ptr(list, 4), 0);                         // EDI
        std::uint8_t* const node =
            host.list_buy_node_00459ea0(list, first, load_ptr(first, 4), value);            // EBP
        host.list_inc_size_00459900(list, 1);
        store<std::uint32_t>(first, 4, word(node));
        store<std::uint32_t>(load_ptr(node, 4), 0, word(node));
    };

    if (begin == head) {
        // 0045B46B-0045B4AD: the copy is assigned in place (both words zeroed
        // first; skipped if it were the argument itself), 0041DD40(length, 1),
        // and memcpy(data, source data, new length) when the length is not 0.
        store<std::uint32_t>(copy, 0, 0u);
        store<std::uint32_t>(copy, 4, 0u);
        if (copy != comment) {
            const std::uint32_t length = load<std::uint32_t>(comment, 0);
            host.native_string_resize_0041dd40(copy, length, 1u);
            if (length != 0) {
                host.memcpy_00bf7680(load_ptr(copy, 4), load_ptr(comment, 4), load<std::uint32_t>(copy, 0));
            }
        }
        std::uint8_t* const value = host.movie_timed_record_construct_00465a40(
            record, position, time, in_tangent, out_tangent, type, alpha, copy);
        insert_front(value);  // state 1
    } else {
        std::uint8_t* const head2 = load_ptr(list, 4);
        std::uint8_t* const first = load_ptr(head2, 0);
        if (first == head2) host.invalid_parameter_00bf6713();   // 0045B5F2
        const float begin_time = load<float>(first, 8);
        // 0045B5FE-0045B608: FCOMIP ST0 = begin time, ST1 = time; JBE (also
        // taken when unordered) goes to the search branch.
        if (begin_time > time) {
            host.native_string_copy_construct_00426060(copy, comment);
            std::uint8_t* const value = host.movie_timed_record_construct_00465a40(
                record, position, time, in_tangent, out_tangent, type, alpha, copy);
            insert_front(value);  // state 2
        } else {
            // 0045B718-0045B743: iterator = 00455BC0(time), then ++iterator
            // with the VC9 checks (null container, end of list).
            std::uint32_t found[2] = {0u, 0u};                   // [ESP+10h]
            host.entity_track_find_sample_before_00455bc0(owner, found, time);
            std::uint8_t* const container = ptr(found[0]);       // EBP
            if (container == nullptr) host.invalid_parameter_00bf6713();
            std::uint8_t* node = ptr(found[1]);
            if (node == load_ptr(container, 4)) host.invalid_parameter_00bf6713();
            node = load_ptr(node, 0);                            // EDI
            host.native_string_copy_construct_00426060(copy, comment);
            std::uint8_t* const value = host.movie_timed_record_construct_00465a40(
                record, position, time, in_tangent, out_tangent, type, alpha, copy);
            // The result iterator lands in the out-tangent argument slot
            // (ESP+88h at the push); nothing reads it.
            std::uint32_t result[2] = {0u, 0u};
            host.list_insert_0045b0a0(list, result, container, node, value);  // state 3
        }
    }

    // 0045B835 / 0045B59D: the record's NativeString (+4 length, +8 data).
    release_string(load<std::uint32_t>(record, 4), load<std::uint32_t>(record, 8), host);
    // 0045B858 / 0045B5C0: the by-value argument.
    release_string(load<std::uint32_t>(comment, 0), load<std::uint32_t>(comment, 4), host);
}

// 0045DDA0-0045DE4E. The loop bound re-reads +6Ch each pass; the slot is
// re-read from +68h before the dereference.
void engine_movie_reset_0045dda0(std::uint8_t* movie, EngineMovieResetHost& host) {
    std::uint32_t first = 0;  // ECX at the exit
    for (std::uint32_t i = 0;; ++i) {
        first = load<std::uint32_t>(movie, 0x68);
        if (first == 0) break;
        if (i >= element_count(first, load<std::uint32_t>(movie, 0x6C))) break;
        if (first == 0 || i >= element_count(first, load<std::uint32_t>(movie, 0x6C))) {
            host.invalid_parameter_00bf6713();  // 0045DDCA
        }
        std::uint8_t* const track = load_ptr(load_ptr(movie, 0x68), i * 4u);
        if (track != nullptr) {
            host.movie_entity_track_destroy_0045b220(track);
            host.free_00bf65ac(track);
        }
    }
    erase_all(movie + 0x68, first, host);
    store<float>(movie, 0x0, host.global_float_00d7a24c());
    store<float>(movie, 0x4, host.global_float_00ce3800());
}

// 00480A90-00480B26. Main-thread x87 (PC 24-bit): FILD of the remainder,
// FDIV by the double, FSTP float. The remainder (< 5A1C9h) and the on-disk
// divisor 369097.0 are exact floats, so one float division is the 24-bit
// result. The running sum is FLD float + FADD float, FSTP float.
std::uint32_t weighted_pick_00480a90(std::uint32_t seed, std::uint8_t* weights, WeightedPickHost& host) {
    const std::uint32_t remainder = (seed * 0x7B913u + 0x67933u) % 0x5A1C9u;
    const float threshold =
        static_cast<float>(static_cast<std::int32_t>(remainder)) / static_cast<float>(host.global_double_00ce6428());
    float sum = 0.0f;
    if (host.vector_size_0043ec30(weights) == 0) return 0;
    std::uint32_t i = 0;
    do {
        const std::uint32_t first = load<std::uint32_t>(weights, 4);
        if (first == 0 || i >= element_count(first, load<std::uint32_t>(weights, 8))) {
            host.invalid_parameter_00bf6713();  // 00480AE8
        }
        sum = load<float>(load_ptr(weights, 4), i * 4u) + sum;
        if (sum > threshold) return i;  // FCOMIP ST0 = sum, ST1 = threshold; JA
        ++i;
    } while (i < host.vector_size_0043ec30(weights));
    return 0;
}

// 004872D0-00487359. VC9 list::remove: the iterator's container is the list
// itself (EDI = EBX), so the null/foreign-container checks never fire; the
// end node is re-read each pass.
void list_node_remove_child_004872d0(std::uint8_t* parent, std::uint8_t* child,
                                     ListNodeRemoveChildHost& host) {
    std::uint8_t* const list = parent + 0x0C;  // EBX
    std::uint8_t* const container = list;      // EDI
    std::uint8_t* node = load_ptr(load_ptr(parent, 0x10), 0);
    for (;;) {
        std::uint8_t* const end = load_ptr(list, 4);
        if (container == nullptr || container != list) host.invalid_parameter_00bf6713();
        if (node == end) return;
        if (container == nullptr) host.invalid_parameter_00bf6713();
        if (node == load_ptr(container, 4)) host.invalid_parameter_00bf6713();
        if (load_ptr(node, 8) == child) {
            if (node == load_ptr(container, 4)) host.invalid_parameter_00bf6713();
            std::uint8_t* const next = load_ptr(node, 0);
            if (node != load_ptr(list, 4)) {
                store<std::uint32_t>(load_ptr(node, 4), 0, word(next));
                store<std::uint32_t>(load_ptr(node, 0), 4, load<std::uint32_t>(node, 4));
                host.free_00bf65ac(node);
                store<std::uint32_t>(list, 8, load<std::uint32_t>(list, 8) - 1u);
            }
            node = next;
        } else {
            if (node == load_ptr(container, 4)) host.invalid_parameter_00bf6713();
            node = load_ptr(node, 0);
        }
    }
}

// 00489DD0-00489E5F. The CMP ESI,ESI at 00489E14 always sets ZF, so the
// 00BF6713 at 00489E1F is dead. A null front child would spin forever, as in
// the image.
void list_node_destruct_00489dd0(std::uint8_t* node, ListNodeDestructHost& host) {
    store<std::uint32_t>(node, 0x0, kListNodeVtable);
    std::uint8_t* const parent = load_ptr(node, 0x8);
    if (parent != nullptr) {
        host.list_node_remove_child_004872d0(parent, node);  // 00489E05
        store<std::uint32_t>(node, 0x8, 0u);
    }
    std::uint8_t* const list = node + 0x0C;
    for (;;) {
        std::uint8_t* const head = load_ptr(list, 4);
        if (load_ptr(head, 0) == head) break;
        std::uint8_t* const head2 = load_ptr(list, 4);
        std::uint8_t* const front = load_ptr(head2, 0);
        if (front == head2) host.invalid_parameter_00bf6713();  // 00489E31
        std::uint8_t* const child = load_ptr(front, 8);
        if (child != nullptr) host.scalar_deleting_destructor(child, 1u);
    }
    host.list_tidy_00485290(list);
}

// 004A5AA0-004A5B25. A null front object would spin forever, as in the image.
void breakup_registry_destroy_unit_entries_004a5aa0(void* manager, void* unit,
                                                    BreakupDestroyUnitEntriesHost& host) {
    std::uint8_t* const map = static_cast<std::uint8_t*>(manager) + 0x1C;  // EDI
    const std::uint32_t key = word(unit);                                  // [ESP+1Ch]
    for (;;) {
        const std::uint8_t* vector = host.map_subscript_004a5750(map, &key);
        std::uint32_t first = load<std::uint32_t>(vector, 4);
        if (first == 0 || element_count(first, load<std::uint32_t>(vector, 8)) == 0) break;
        vector = host.map_subscript_004a5750(map, &key);                   // ESI
        first = load<std::uint32_t>(vector, 4);
        if (first == 0 || element_count(first, load<std::uint32_t>(vector, 8)) == 0) {
            host.invalid_parameter_00bf6713();  // 004A5AE4
        }
        std::uint8_t* const object = load_ptr(load_ptr(vector, 4), 0);
        if (object != nullptr) host.scalar_deleting_destructor(object, 1u);
    }
    std::uint32_t found[2] = {0u, 0u};   // [ESP+8]
    host.map_find_0048cf20(map, found, &key);
    std::uint32_t result[2] = {0u, 0u};  // [ESP+10h]
    host.map_erase_00491f40(map, result, found[0], found[1]);
}

// 004AD380-004AD47D. Each pass re-reads +18h for the bound, the element and
// the null store; +1Ch is re-read for every bound.
void ptr_vector_owner_destruct_004ad380(std::uint8_t* object, PtrVectorOwnerDestructHost& host) {
    store<std::uint32_t>(object, 0x0, kPtrVectorOwnerVtable);
    std::uint32_t first = 0;  // ECX at the exit
    for (std::uint32_t i = 0;; ++i) {
        first = load<std::uint32_t>(object, 0x18);
        if (first == 0) break;
        if (i >= element_count(first, load<std::uint32_t>(object, 0x1C))) break;
        if (first == 0 || i >= element_count(first, load<std::uint32_t>(object, 0x1C))) {
            host.invalid_parameter_00bf6713();  // 004AD3D3
        }
        std::uint8_t* const element = load_ptr(load_ptr(object, 0x18), i * 4u);
        if (element != nullptr) host.scalar_deleting_destructor(element, 1u);
        const std::uint32_t again = load<std::uint32_t>(object, 0x18);
        if (again == 0 || i >= element_count(again, load<std::uint32_t>(object, 0x1C))) {
            host.invalid_parameter_00bf6713();  // 004AD3FD
        }
        store<std::uint32_t>(load_ptr(object, 0x18), i * 4u, 0u);
    }
    erase_all(object + 0x18, first, host);
    std::uint8_t* const storage = load_ptr(object, 0x18);
    if (storage != nullptr) host.free_00bf65ac(storage);
    store<std::uint32_t>(object, 0x18, 0u);
    store<std::uint32_t>(object, 0x1C, 0u);
    store<std::uint32_t>(object, 0x20, 0u);
}

} // namespace bsp::cc15_g24a
