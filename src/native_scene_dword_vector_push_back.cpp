#include "bsp/native_scene_dword_vector_push_back.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_scene_dword_vector_insert.hpp"

#include <cstdint>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == sizeof(U));
U address(const void* p) noexcept { return reinterpret_cast<U>(p); }
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
} // namespace

void push_back_native_scene_dword_vector_0046c450(void* actual_vector16,
    const NativeSceneDwordVectorPushBackFrame& frame) {
    const U tree = address(actual_vector16);
    const U begin = read(tree, 4);
    const U size = begin == 0 ? 0 : sar2(read(tree, 8) - begin);
    if (begin != 0) {
        const U capacity = sar2(read(tree, 12) - begin);
        if (size < capacity) {
            const U end = read(tree, 8);
            const U value_address = read(address(frame.actual_parameter4));
            const U value = read(value_address);
            write(end, 0, value);
            write(tree, 8, end + 4u);
            return;
        }
    }
    const U end = read(tree, 8);
    if (begin > end) invoke_native_invalid_parameter_00bf6713();
    const U value_address = read(address(frame.actual_parameter4));
    // Keep unused fast-path metadata untouched; even here, capture the late
    // value before inspecting the reached child's stable binding metadata.
    const auto& child = *frame.child_frame;
    const U child_parameters = address(child.actual_parameters16);
    write(child_parameters, 12, value_address);
    write(child_parameters, 8, end);
    write(child_parameters, 4, tree);
    write(child_parameters, 0, address(frame.actual_private_output8));
    (void)insert_native_scene_dword_vector_one_0046b4a0(actual_vector16, child);
}

} // namespace bsp
