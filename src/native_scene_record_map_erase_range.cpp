#include "bsp/native_scene_record_map_erase_range.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_record_map_clear.hpp"
#include "bsp/native_scene_record_map_increment.hpp"

#include <cstdint>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U address(const void* value) noexcept { return reinterpret_cast<std::uintptr_t>(value); }
U read(U location, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(location + offset);
}
void write(U location, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(location + offset) = value;
}
} // namespace

void* erase_native_scene_record_map_range_0046b570(void* actual_map,
    const NativeSceneRecordMapEraseRangeFrame& frame, NativeStringRawPoolContext& strings) {
    const U parameters = address(frame.actual_parameters20);
    U owner = read(parameters, 4);
    const U tree = address(actual_map);
    const U initial_head = read(tree, 4);
    const U initial_minimum = read(initial_head);
    if (owner == 0 || owner != tree) invoke_native_invalid_parameter_00bf6713();
    U node = read(parameters, 8);
    if (node == initial_minimum) {
        const U last_owner = read(parameters, 12);
        const U captured_head = read(tree, 4);
        if (last_owner == 0 || last_owner != tree) invoke_native_invalid_parameter_00bf6713();
        if (read(parameters, 16) == captured_head) {
            // Exact existing Source schedule for the native inlined AD10/reset.
            clear_native_scene_record_map_0046b460(actual_map, strings);
            const U head = read(tree, 4);
            const U minimum = read(head);
            const U output = read(parameters);
            write(output, 0, tree);
            write(output, 4, minimum);
            return reinterpret_cast<void*>(output);
        }
    }
    for (;;) {
        if (owner == 0 || owner != read(parameters, 12))
            invoke_native_invalid_parameter_00bf6713();
        if (node == read(parameters, 16)) break;
        increment_native_scene_record_map_iterator_00467980(
            reinterpret_cast<void*>(parameters + 4u));
        // Do not inspect child metadata/backing until this call is reached.
        const auto& child = *frame.child_frame;
        const U child_arguments = address(child.actual_parameters12);
        write(child_arguments, 8, node);
        write(child_arguments, 4, owner);
        write(child_arguments, 0, address(frame.actual_private_output8));
        (void)erase_native_scene_record_map_iterator_0046a470(actual_map, child, strings);
        node = read(parameters, 8);
        owner = read(parameters, 4);
    }
    const U output = read(parameters);
    write(output, 0, owner);
    write(output, 4, node);
    return reinterpret_cast<void*>(output);
}
} // namespace bsp
