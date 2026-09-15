#include "bsp/native_ship_model_pointer_binding.hpp"

#include "bsp/native_game_resource_lists.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native ship model pointer binding requires MSVC Win32.
#endif

namespace bsp {
namespace {

using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);

Word address(const void* value) noexcept {
    return reinterpret_cast<Word>(value);
}

void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}

Word read(Word base, Word byte_offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(base + byte_offset);
}

void write(Word base, Word byte_offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(base + byte_offset) = value;
}

Word arithmetic_shift_two(Word value) noexcept {
    std::int32_t signed_value;
    std::memcpy(&signed_value, &value, sizeof(signed_value));
    return static_cast<Word>(signed_value >> 2);
}

void invalid(const SingletonLifetimeCallbacks& callbacks) {
    callbacks.invalid_parameter(callbacks.context);
}

} // namespace

void bind_native_ship_model_pointers_0082d700(
    void* actual_ship_class, const SingletonLifetimeCallbacks& callbacks) {
    const Word ship = address(actual_ship_class);
    const Word captured_source = read(ship, 0x50u) + 0x54u;
    Word iterator = read(captured_source, 4u);

    if (iterator > read(captured_source, 8u)) invalid(callbacks);

    for (;;) {
        const Word current_source = read(ship, 0x50u) + 0x54u;
        const Word current_end = read(current_source, 8u);
        if (read(current_source, 4u) > current_end) invalid(callbacks);
        if (captured_source != current_source) invalid(callbacks);
        if (iterator == current_end) return;

        if (iterator >= read(captured_source, 8u)) invalid(callbacks);
        const Word value = read(iterator);

        const Word destination = ship + 0x6bcu;
        const Word captured_destination_begin = read(destination, 4u);
        Word destination_size = 0;
        if (captured_destination_begin != 0) {
            destination_size = arithmetic_shift_two(
                read(destination, 8u) - captured_destination_begin);
        }

        bool appended = false;
        if (captured_destination_begin != 0) {
            const Word destination_capacity = arithmetic_shift_two(
                read(destination, 0x0cu) - captured_destination_begin);
            if (destination_size < destination_capacity) {
                const Word current_destination_end = read(destination, 8u);
                write(current_destination_end, 0, value);
                write(destination, 8u, current_destination_end + 4u);
                appended = true;
            }
        }

        if (!appended) {
            const Word position = read(destination, 8u);
            if (captured_destination_begin > position) invalid(callbacks);
            Word output[2];
            insert_native_game_type54_one_0071b2c0(
                pointer(destination), output, pointer(destination), pointer(position),
                &value, callbacks);
        }

        if (iterator >= read(captured_source, 8u)) invalid(callbacks);
        iterator += 4u;
    }
}

} // namespace bsp
