#include "bsp/point_effect_entry_array.hpp"
#include "bsp/point_effect_instance.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Point-effect entry arrays require MSVC Win32.
#endif

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(PointEffectReferenceArray) == 0x0c);
static_assert(offsetof(PointEffectReferenceArray, begin) == 0);
static_assert(offsetof(PointEffectReferenceArray, count) == 4);
static_assert(offsetof(PointEffectReferenceArray, capacity) == 8);

std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

RenderCommandReference** slot(RenderCommandReference** begin,
    std::uint32_t index) noexcept {
    // Native LEA is a wrapping DWORD computation, including the subsequent
    // null-destination check; do not use C++ arithmetic on a null pointer.
    return reinterpret_cast<RenderCommandReference**>(
        reinterpret_cast<std::uintptr_t>(begin) + index * 4u);
}

void release_then_clear(RenderCommandReference** captured_slot) noexcept {
    RenderCommandReference* const reference = *captured_slot;
    if (reference) {
        release_render_command_reference(*reference);
        *captured_slot = nullptr;
    }
}

} // namespace

void reserve_point_effect_entry_array_008670a0(
    PointEffectReferenceArray& array, std::int32_t requested_capacity) {
    volatile auto& actual = array;
    if (requested_capacity < 1) requested_capacity = 1;
    if (actual.capacity >= requested_capacity) return;

    const auto bytes = static_cast<std::uint32_t>(requested_capacity) * 4u;
    auto** const replacement = static_cast<RenderCommandReference**>(
        singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes}));
    for (std::uint32_t i = 0; signed_word(i) < actual.count; ++i) {
        auto** const destination = slot(replacement, i);
        if (destination) {
            auto** const source = slot(actual.begin, i);
            *destination = nullptr;
            RenderCommandReference* const reference = *source;
            if (reference) {
                *destination = reference;
                retain_render_command_reference(*reference);
            }
        }
    }
    for (std::uint32_t i = 0; signed_word(i) < actual.count; ++i)
        release_then_clear(slot(actual.begin, i));

    singleton_lifetime_free(actual.begin);
    // Returning-free tail 0086716F..00867181 is absent from saved Ghidra
    // flow. Live bytes and independent disk decoding prove these stores.
    actual.begin = replacement;             // 0086717C
    actual.capacity = requested_capacity;   // 0086717E; count is untouched
}

void resize_point_effect_entry_array_008672a0(
    PointEffectReferenceArray& array, std::int32_t requested_count) {
    volatile auto& actual = array;
    if (requested_count > actual.capacity)
        reserve_point_effect_entry_array_008670a0(array, requested_count);

    auto index = static_cast<std::uint32_t>(actual.count);
    while (signed_word(index) < requested_count) {
        auto** const destination = slot(actual.begin, index);
        if (destination) *destination = nullptr;
        ++index;
    }
    while (requested_count < actual.count) {
        actual.count = signed_word(static_cast<std::uint32_t>(actual.count) - 1u);
        const auto removed_index = static_cast<std::uint32_t>(actual.count);
        release_then_clear(slot(actual.begin, removed_index));
    }
    actual.count = requested_count;
}

void erase_point_effect_entry_array_unordered_00867210(
    PointEffectReferenceArray& array,
    RenderCommandReference** const* position) noexcept {
    auto** const destination = *position;
    volatile auto& actual = array;
    const auto initial_count = static_cast<std::uint32_t>(actual.count);
    auto** const initial_tail = slot(actual.begin, initial_count - 1u);
    if (destination != initial_tail)
        assign_render_command_reference(*destination, *initial_tail);

    // 0086725F reloads both fields after the replaced entry's terminal
    // callback. Do not release the earlier captured tail or predecrement.
    const auto current_count = static_cast<std::uint32_t>(actual.count);
    release_then_clear(slot(actual.begin, current_count - 1u));
    actual.count = signed_word(static_cast<std::uint32_t>(actual.count) - 1u);
}

void append_point_effect_entry_array_00867320(
    PointEffectReferenceArray& array, RenderCommandReference* const* source) {
    volatile auto& actual = array;
    const auto capacity = actual.capacity;
    if (actual.count == capacity) {
        const auto doubled = signed_word(static_cast<std::uint32_t>(capacity) * 2u);
        reserve_point_effect_entry_array_008670a0(array, doubled > 1 ? doubled : 1);
    }

    const auto index = static_cast<std::uint32_t>(actual.count);
    auto** const destination = slot(actual.begin, index);
    if (destination) {
        *destination = nullptr;
        RenderCommandReference* const reference = *source;
        if (reference) {
            *destination = reference;
            retain_render_command_reference(*reference);
        }
    }
    actual.count = signed_word(static_cast<std::uint32_t>(actual.count) + 1u);
}

} // namespace bsp
