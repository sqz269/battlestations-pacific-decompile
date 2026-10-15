#include "bsp/native_scene_dword_iterator.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
U address(const void* p) noexcept { return reinterpret_cast<std::uintptr_t>(p); }
U read(U p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(p + offset);
}
void write(U p, U offset, U value) noexcept {
    *reinterpret_cast<volatile U*>(p + offset) = value;
}
} // namespace

void* advance_native_scene_dword_iterator_00467a80(
    void* actual_iterator8, const void* actual_delta4) {
    const U iterator = address(actual_iterator8);
    if (read(iterator) == 0) invoke_native_invalid_parameter_00bf6713();
    U scaled = read(address(actual_delta4));
    const U current = read(iterator, 4);
    const U owner = read(iterator);
    scaled += scaled;
    scaled += scaled;
    const U proposed = current + scaled;
    if (proposed > read(owner, 8) || proposed < read(owner, 4))
        invoke_native_invalid_parameter_00bf6713();
    const U current_after_diagnostic = read(iterator, 4);
    write(iterator, 4, current_after_diagnostic + scaled);
    return actual_iterator8;
}

std::uint32_t unequal_native_scene_dword_iterators_00467d50(
    const void* actual_left_iterator8, const void* actual_other_parameter4) {
    const U left = address(actual_left_iterator8);
    const U owner = read(left);
    const U other = read(address(actual_other_parameter4));
    if (owner == 0 || owner != read(other))
        invoke_native_invalid_parameter_00bf6713();
    const U left_current = read(left, 4);
    const U right_current = read(other, 4);
    return (left_current & 0xffffff00u) | static_cast<U>(left_current != right_current);
}
} // namespace bsp
