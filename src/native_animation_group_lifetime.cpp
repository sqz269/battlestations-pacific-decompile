#include "bsp/native_animation_group_lifetime.hpp"

#include "bsp/native_camera_group_resource.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native animation group lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
constexpr U group_profile = 0x00d62ed4;
constexpr U channel_profile = 0x00d632b0;
constexpr U group_delete = 0x00b78d00;
constexpr U channel_delete = 0x00b8ad60;
static_assert(sizeof(void*) == 4);

U bits(const void* p) noexcept {
    return static_cast<U>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(U value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p, U offset = 0) noexcept { return pointer(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept {
    return *static_cast<const volatile U*>(at(p, offset));
}
void put(void* p, U offset, U value) noexcept {
    *static_cast<volatile U*>(at(p, offset)) = value;
}
std::int32_t signed_word(U value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}

void unwind_group(void* group, int state, NativeStringRawPoolContext& strings) noexcept {
    try {
        if (state >= 2)
            destroy_native_animation_channel_pointers_00b77a10(at(group, 0x10));
        if (state >= 1)
            destroy_native_string_header_0041dd20(at(group, 8), strings);
        if (state >= 0)
            destroy_native_resource_item_base_00b86890(group);
    } catch (...) {
        std::terminate();
    }
}

void unwind_channel(void* channel, int state, NativeStringRawPoolContext& strings) noexcept {
    try {
        if (state >= 1)
            destroy_native_string_header_0041dd20(at(channel, 8), strings);
        if (state >= 0)
            destroy_native_ref_counted_base_00bd30f0(channel);
    } catch (...) {
        std::terminate();
    }
}
} // namespace

void resize_native_animation_keys_00b8a3c0(void* header, std::int32_t requested) {
    if (signed_word(word(header, 8)) < requested)
        reserve_native_animation_keys_00b76680(header, requested);

    const U initial_count = word(header, 4);
    if (signed_word(initial_count) < requested) {
        U offset = initial_count * 0x28u;
        U remaining = static_cast<U>(requested) - initial_count;
        do {
            const U destination = word(header) + offset;
            if (destination != 0) {
                for (U i = 0; i != 10; ++i)
                    put(pointer(destination), i * 4u, 0);
            }
            offset += 0x28u;
            --remaining;
        } while (remaining != 0);
    }

    while (requested < signed_word(word(header, 4)))
        put(header, 4, word(header, 4) - 1u);
    put(header, 4, static_cast<U>(requested));
}

void destroy_native_animation_channel_00b8a7d0(void* channel,
    NativeStringRawPoolContext& strings) {
    int state = 1;
    try {
        void* const keys = at(channel, 0x1c);
        resize_native_animation_keys_00b8a3c0(keys, 0);
        singleton_lifetime_free(pointer(word(keys)));

        state = 0;
        destroy_native_string_header_0041dd20(at(channel, 8), strings);

        state = -1;
        destroy_native_ref_counted_base_00bd30f0(channel);
    } catch (...) {
        unwind_channel(channel, state, strings);
        throw;
    }
}

void* delete_native_animation_channel_00b8ad60(void* channel, U flags,
    NativeStringRawPoolContext& strings) {
    destroy_native_animation_channel_00b8a7d0(channel, strings);
    if ((flags & 1u) != 0)
        singleton_lifetime_free(channel);
    return channel;
}

void destroy_native_animation_channel_group_00b78530(void* group,
    NativeStringRawPoolContext& strings, NativeRefCountedDeleteCalls& deletes) {
    put(group, 0, group_profile);
    int state = 2;
    try {
        while (word(group, 0x14) != 0) {
            const U data = word(group, 0x10);
            const U count = word(group, 0x14);
            auto* const captured_cell = static_cast<volatile U*>(
                pointer(data + count * 4u - 4u));
            void* const captured_channel = pointer(*captured_cell);
            if (captured_channel != nullptr) {
                const U captured_profile = word(captured_channel);
                deletes.delete_vslot04(captured_channel, captured_profile, 1);
                *captured_cell = 0;
            }
            const U count_after_delete = word(group, 0x14);
            if (count_after_delete != 0)
                put(group, 0x14, count_after_delete - 1u);
        }

        state = 1;
        void* const channels = at(group, 0x10);
        resize_native_animation_channel_pointers_00b774e0(channels, 0);
        singleton_lifetime_free(pointer(word(channels)));

        state = 0;
        destroy_native_string_header_0041dd20(at(group, 8), strings);

        state = -1;
        destroy_native_resource_item_base_00b86890(group);
    } catch (...) {
        unwind_group(group, state, strings);
        throw;
    }
}

void* delete_native_animation_channel_group_00b78d00(void* group, U flags,
    NativeStringRawPoolContext& strings, NativeRefCountedDeleteCalls& deletes) {
    destroy_native_animation_channel_group_00b78530(group, strings, deletes);
    if ((flags & 1u) != 0)
        singleton_lifetime_free(group);
    return group;
}

void NativeAnimationDeleteCalls::delete_vslot04(void* owner, U captured_profile,
    U flags) {
    const volatile U* table = nullptr;
    U expected = 0;
    if (captured_profile == group_profile) {
        table = context_.actual_group_profile_00d62ed4;
        expected = group_delete;
    } else if (captured_profile == channel_profile) {
        table = context_.actual_channel_profile_00d632b0;
        expected = channel_delete;
    } else {
        throw std::invalid_argument("unbound native animation lifetime profile");
    }
    if (table == nullptr || table[1] != expected)
        throw std::invalid_argument("unbound native animation lifetime slot04");

    if (captured_profile == group_profile) {
        (void)delete_native_animation_channel_group_00b78d00(
            owner, flags, context_.strings, *this);
    } else {
        (void)delete_native_animation_channel_00b8ad60(
            owner, flags, context_.strings);
    }
}
} // namespace bsp
