#include "bsp/native_scene_integer_set_increment.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#include <cstdint>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);

U word(U address, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(address + offset);
}

std::uint8_t nil(U address) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(address + 0x11u);
}

void publish(U iterator, U node) noexcept {
    *reinterpret_cast<volatile U*>(iterator + 4u) = node;
}
} // namespace

void __fastcall increment_native_scene_integer_set_iterator_00466ae0(
    void* actual_iterator8) {
    const U iterator = reinterpret_cast<std::uintptr_t>(actual_iterator8);
    if (word(iterator) == 0)
        invoke_native_invalid_parameter_00bf6713();

    // The initial handler may change the iterator. Load node after it returns.
    const U current = word(iterator, 4);
    if (nil(current) != 0) {
        // Native POP ESI/JMP reports and returns without another publication.
        invoke_native_invalid_parameter_00bf6713();
        return;
    }

    U candidate = word(current, 8);
    if (nil(candidate) == 0) {
        U left = word(candidate);
        while (nil(left) == 0) {
            candidate = left;
            left = word(candidate);
        }
        publish(iterator, candidate);
        return;
    }

    U parent = word(current, 4);
    while (nil(parent) == 0) {
        const U current_node = word(iterator, 4);
        const U parent_right = word(parent, 8);
        if (current_node != parent_right)
            break;
        publish(iterator, parent);
        parent = word(parent, 4);
    }
    publish(iterator, parent);
}

void* __fastcall increment_native_scene_integer_set_iterator_00467b30(
    void* actual_iterator8) {
    increment_native_scene_integer_set_iterator_00466ae0(actual_iterator8);
    return actual_iterator8;
}

void* __fastcall increment_native_scene_integer_set_iterator_00468340(
    void* actual_iterator8) {
    increment_native_scene_integer_set_iterator_00466ae0(actual_iterator8);
    return actual_iterator8;
}
} // namespace bsp
