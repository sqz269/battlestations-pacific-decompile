#include "bsp/native_scene_dword_vector_insert.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_dword_fill_alias.hpp"
#include "bsp/native_scene_dword_move_nonzero.hpp"
#include "bsp/native_scene_dword_vector_allocation.hpp"
#include "bsp/native_scene_dword_vector_ranges.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U address(const void* p) noexcept { return reinterpret_cast<std::uintptr_t>(p); }
void* pointer(U p) noexcept { return reinterpret_cast<void*>(p); }
U read(U p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(p + offset);
}
void write(U p, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(p + offset) = value;
}
U sar2(U bits) noexcept {
    return (bits >> 2u) | ((bits & 0x80000000u) ? 0xc0000000u : 0u);
}
U size_from_captured_begin(U tree, U begin) noexcept {
    return begin == 0 ? 0 : sar2(read(tree, 8) - begin);
}
} // namespace

void insert_native_scene_dword_vector_n_0046afb0(void* actual_vector16,
    const NativeSceneDwordVectorInsertNFrame& frame) {
    const U parameters = address(frame.actual_parameters16);
    const U value_address = read(parameters, 12);
    const U tree = address(actual_vector16);
    const U value = read(value_address);
    write(parameters, 12, value);
    const U begin = read(tree, 4);
    U capacity = begin == 0 ? 0 : sar2(read(tree, 12) - begin);
    const U count = read(parameters, 8);
    if (count == 0) return;
    U size = size_from_captured_begin(tree, begin);
    if (0x3fffffffu - size < count)
        throw_native_scene_dword_vector_length_0046a400();
    size = size_from_captured_begin(tree, begin);
    if (capacity < size + count) {
        const U half_capacity = capacity >> 1u;
        capacity = 0x3fffffffu - half_capacity < capacity
            ? 0 : capacity + half_capacity;
        size = size_from_captured_begin(tree, begin);
        if (capacity < size + count) {
            size = size_from_captured_begin(tree, begin);
            capacity = size + count;
        }
        void* const allocation = allocate_native_scene_dword_vector_00466ce0(capacity, 0);
        const U new_begin = address(allocation);
        const U current_begin = read(tree, 4);
        const U position = read(parameters, 4);
        void* const prefix_end = move_native_scene_dword_range_00468f60(
            pointer(current_begin), pointer(position), allocation);
        void* const filled_end = fill_native_scene_dword_count_00469950(
            prefix_end, count, pointer(parameters + 12u));
        const U current_position = read(parameters, 4);
        const U current_end = read(tree, 8);
        (void)move_native_scene_dword_range_00468f60(
            pointer(current_position), pointer(current_end), filled_end);
        const U old_begin = read(tree, 4);
        const U old_size = size_from_captured_begin(tree, old_begin);
        const U new_size = count + old_size;
        if (old_begin != 0) singleton_lifetime_free(pointer(old_begin));
        const U capacity_end = new_begin + capacity * 4u;
        const U new_end = new_begin + new_size * 4u;
        write(tree, 4, new_begin);
        write(tree, 12, capacity_end);
        write(tree, 8, new_end);
        return;
    }

    const U old_end = read(tree, 8);
    const U position = read(parameters, 4);
    const U tail_count = sar2(old_end - position);
    const U scaled = count * 4u;
    const bool short_tail = tail_count < count;
    write(parameters, 8, scaled);
    if (short_tail) {
        (void)move_native_scene_dword_range_00468f60(
            pointer(position), pointer(old_end), pointer(position + scaled));
        const U current_end = read(tree, 8);
        const U remaining = count - sar2(current_end - position);
        (void)fill_native_scene_dword_count_00469950(
            pointer(current_end), remaining, pointer(parameters + 12u));
        const U current_scaled = read(parameters, 8);
        const U end_before_add = read(tree, 8);
        write(tree, 8, end_before_add + current_scaled);
        const U end_after_add = read(tree, 8);
        (void)fill_native_dword_range_00468440(pointer(position),
            pointer(end_after_add - current_scaled), pointer(parameters + 12u));
        return;
    }

    const U source_tail = old_end - scaled;
    void* const moved_end = move_native_scene_dword_range_00468f60(
        pointer(source_tail), pointer(old_end), pointer(old_end));
    write(tree, 8, address(moved_end));
    (void)copy_backward_native_scene_dword_range_00468460(
        pointer(position), pointer(source_tail), pointer(old_end));
    const U current_scaled = read(parameters, 8);
    (void)fill_native_dword_range_00468440(pointer(position),
        pointer(position + current_scaled), pointer(parameters + 12u));
}

void* insert_native_scene_dword_vector_one_0046b4a0(void* actual_vector16,
    const NativeSceneDwordVectorInsertOneFrame& frame) {
    const U parameters = address(frame.actual_parameters16);
    const U owner = read(parameters, 4);
    const U tree = address(actual_vector16);
    const U begin = read(tree, 4);
    U index = 0;
    if (begin != 0) {
        const U end = read(tree, 8);
        if (sar2(end - begin) != 0) {
            if (begin > end) invoke_native_invalid_parameter_00bf6713();
            if (owner == 0 || owner != tree) invoke_native_invalid_parameter_00bf6713();
            index = sar2(read(parameters, 8) - begin);
        }
    }
    const U value_address = read(parameters, 12);
    const U position = read(parameters, 8);
    const auto& child = *frame.child_frame;
    const U child_parameters = address(child.actual_parameters16);
    write(child_parameters, 12, value_address);
    write(child_parameters, 8, 1);
    write(child_parameters, 4, position);
    write(child_parameters, 0, owner);
    insert_native_scene_dword_vector_n_0046afb0(actual_vector16, child);
    const U new_begin = read(tree, 4);
    if (new_begin > read(tree, 8)) invoke_native_invalid_parameter_00bf6713();
    write(parameters, 8, new_begin);
    const U inserted = new_begin + index * 4u;
    if (inserted > read(tree, 8) || inserted < read(tree, 4))
        invoke_native_invalid_parameter_00bf6713();
    const U output = read(parameters);
    write(output, 4, inserted);
    write(output, 0, tree);
    return pointer(output);
}
} // namespace bsp
