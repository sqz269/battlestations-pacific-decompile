#include "bsp/native_scene_record_map_iterator.hpp"

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
    return *reinterpret_cast<const volatile std::uint8_t*>(address + 0x19u);
}

void publish(U iterator, U node) noexcept {
    *reinterpret_cast<volatile U*>(iterator + 4u) = node;
}
} // namespace

void __fastcall decrement_native_scene_record_map_iterator_004678f0(
    void* actual_iterator) {
    const U iterator = reinterpret_cast<std::uintptr_t>(actual_iterator);
    if (word(iterator) == 0)
        invoke_native_invalid_parameter_00bf6713();

    // The initial handler may change the iterator. Read its node afterward.
    const U current = word(iterator, 4);
    if (nil(current) != 0) {
        const U right = word(current, 8);
        publish(iterator, right);
        if (nil(right) != 0)
            invoke_native_invalid_parameter_00bf6713();
        return;
    }

    U candidate = word(current);
    if (nil(candidate) == 0) {
        U right = word(candidate, 8);
        while (nil(right) == 0) {
            candidate = right;
            right = word(candidate, 8);
        }
        publish(iterator, candidate);
        return;
    }

    U parent = word(current, 4);
    while (nil(parent) == 0) {
        const U current_node = word(iterator, 4);
        const U parent_left = word(parent);
        if (current_node != parent_left)
            break;
        publish(iterator, parent);
        parent = word(parent, 4);
    }
    // The native tail check reads the CURRENT iterator.node; it does not test
    // the captured current value or automatically publish parent on failure.
    if (nil(word(iterator, 4)) != 0) {
        invoke_native_invalid_parameter_00bf6713();
        return;
    }
    publish(iterator, parent);
}

void* __fastcall decrement_native_scene_record_map_iterator_00468200(
    void* actual_iterator) {
    decrement_native_scene_record_map_iterator_004678f0(actual_iterator);
    return actual_iterator;
}

void* __fastcall decrement_native_scene_record_map_iterator_004688e0(
    void* actual_iterator) {
    decrement_native_scene_record_map_iterator_004678f0(actual_iterator);
    return actual_iterator;
}
} // namespace bsp
