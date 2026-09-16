#include "bsp/native_animation_channels_resource_reader.hpp"

#include "bsp/native_camera_group_resource.hpp"
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/native_resource_node_traversal.hpp"
#include "bsp/native_resource_reader_references.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native AnimationChannels reader requires MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* pointer(U value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p, U offset = 0) noexcept { return pointer(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void put(void* p, U offset, U value) noexcept { *static_cast<volatile U*>(at(p, offset)) = value; }
std::int32_t signed_word(U value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, 4);
    return result;
}
bool named(void* child_handle, const char* expected) {
    const auto* name = static_cast<const char*>(pointer(word(pointer(word(child_handle)), 0x14)));
    return name && _stricmp(name, expected) == 0;
}
} // namespace

void reserve_native_animation_channel_pointers_00b76710(void* header, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (signed_word(word(header, 8)) >= requested) return;
    const U bytes = static_cast<U>(requested) * 4u;
    void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    U destination = bits(allocation);
    for (U i = 0; signed_word(i) < signed_word(word(header, 4)); ++i, destination += 4u) {
        if (destination) put(pointer(destination), 0, word(pointer(word(header)), i * 4u));
    }
    singleton_lifetime_free(pointer(word(header)));
    // Native B76761..69 publication follows the returning _free call.
    put(header, 0, bits(allocation));
    put(header, 8, static_cast<U>(requested));
}

void resize_native_animation_channel_pointers_00b774e0(void* header, std::int32_t requested) {
    if (requested > signed_word(word(header, 8)))
        reserve_native_animation_channel_pointers_00b76710(header, requested);
    for (U i = word(header, 4); signed_word(i) < requested; ++i) {
        const U destination = word(header) + i * 4u;
        if (destination) put(pointer(destination), 0, 0);
    }
    while (requested < signed_word(word(header, 4))) put(header, 4, word(header, 4) - 1u);
    put(header, 4, static_cast<U>(requested));
}

void destroy_native_animation_channel_pointers_00b77a10(void* header) {
    resize_native_animation_channel_pointers_00b774e0(header, 0);
    singleton_lifetime_free(pointer(word(header)));
}

void* construct_native_animation_channel_group_00b78c40(
    void* group, const void* name, NativeStringRawPoolContext& strings) {
    construct_native_resource_item_base_00b868b0(group);
    put(group, 0, 0x00d62ed4u);
    void* const target_name = at(group, 8);
    void* const channels = at(group, 0x10);
    put(target_name, 0, 0); put(target_name, 4, 0);
    put(channels, 0, 0); put(channels, 4, 0); put(channels, 8, 0);
    try {
        if (target_name != name) {
            resize_native_string_header_0041dd40(target_name, strings, word(name), true);
            if (word(name) != 0) {
                const U count = word(target_name);
                const void* const source = pointer(word(name, 4));
                void* const destination = pointer(word(target_name, 4));
                std::memcpy(destination, source, count);
            }
        }
        resize_native_animation_channel_pointers_00b774e0(channels, 11);
        // Native constructor repeats the null stores after resize.
        for (U i = 0; signed_word(i) < signed_word(word(group, 0x14)); ++i)
            put(pointer(word(channels)), i * 4u, 0);
        put(group, 0x1c, 0); // XORPS/MOVSS positive float zero.
    } catch (...) {
        try {
            destroy_native_animation_channel_pointers_00b77a10(channels);
            destroy_native_string_header_0041dd20(target_name, strings);
            destroy_native_resource_item_base_00b86890(group);
        } catch (...) { std::terminate(); }
        throw;
    }
    return group;
}

void read_native_animation_channels_00b8ad80(void* item, void* handle,
    NativeResourceStreamReadContext& context, NativeAnimationChannelBodyCalls& channels) {
    void* current_group = nullptr; // EDI, preserved across children.
    while (native_resource_node_has_remaining_00715bf0(handle)) {
        U child;
        create_native_resource_child_00bea680(handle, &child, context);
        int state = 0;
        U name[2];
        void* allocation = nullptr;
        try {
            if (named(&child, "AnimationGroupName")) {
                read_native_resource_handle_string_00bea010(&child, name, context);
                state = 1;
                allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x20, 0x20});
                state = 2;
                current_group = allocation
                    ? construct_native_animation_channel_group_00b78c40(allocation, name, context.strings)
                    : nullptr;
                state = 1; // The completed group has no reader rollback from here.
                void* const array = at(item, 8);
                const U capacity = word(array, 8);
                if (word(array, 4) == capacity) {
                    auto requested = signed_word(capacity * 2u);
                    if (requested <= 1) requested = 1;
                    reserve_native_instance_entry_pointers_00b1c500(
                        *static_cast<NativeRenderPointerArrayStorage*>(array), requested);
                }
                const U destination = word(array) + word(array, 4) * 4u;
                if (destination) put(pointer(destination), 0, bits(current_group));
                put(array, 4, word(array, 4) + 1u);
                state = 0;
                destroy_native_string_header_0041dd20(name, context.strings);
            } else if (named(&child, "ChannelAnimation")) {
                channels.read_channel_00b8aad0(item, &child, current_group);
            } else {
                skip_native_resource_node_00be9c40(&child, context);
            }
            state = -1;
            release_native_structured_node_handle_00be9ed0(&child, context.streams);
        } catch (...) {
            try {
                if (state == 2) singleton_lifetime_free(allocation);
                if (state >= 1) destroy_native_string_header_0041dd20(name, context.strings);
                if (state >= 0) release_native_structured_node_handle_00be9ed0(&child, context.streams);
            } catch (...) { std::terminate(); }
            throw;
        }
    }
}

void NativeAnimationChannelsReaderCalls::read_item(std::uintptr_t target, void* item, void* handle) {
    if (target == 0x00b8ad80u) read_native_animation_channels_00b8ad80(item, handle, context_, channels_);
    else remaining_.read_item(target, item, handle);
}
} // namespace bsp
