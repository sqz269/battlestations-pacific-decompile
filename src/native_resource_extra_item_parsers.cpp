#include "bsp/native_resource_extra_item_parsers.hpp"

#include "bsp/native_camera_group_resource.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource extra item parsers require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;

Word bits(const void* pointer) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}

void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(value);
}

void* at(void* base, Word offset) noexcept {
    return pointer(bits(base) + offset);
}

Word word(const void* base, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(pointer(bits(base) + offset));
}

void put(void* base, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(base, offset)) = value;
}

void read_current_slot20(void* item, void* handle,
    NativeResourceExtraItemReaderCalls& readers) {
    const Word current_profile = word(item);
    const Word captured_target = word(pointer(current_profile), 0x20);
    readers.read_item(captured_target, item, handle);
}
} // namespace

void* parse_native_animation_channels_item_00b8a910(
    void* handle, NativeResourceExtraItemReaderCalls& readers) {
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x14, 0x14});
    void* item = nullptr;
    if (allocation) {
        item = construct_native_resource_item_base_00b868b0(allocation);
        put(item, 0x00, kNativeAnimationChannelsItemProfile);
        put(item, 0x08, 0);
        put(item, 0x0c, 0);
        put(item, 0x10, 0);
    }

    // Native EH state is already -1 here. Reader failure owns no rollback.
    read_current_slot20(item, handle, readers);
    return item;
}

void* parse_native_bone_item_00b8a990(
    void* handle, NativeResourceExtraItemReaderCalls& readers) {
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x2c, 0x2c});
    void* item = nullptr;
    if (allocation) {
        item = construct_native_resource_item_base_00b868b0(allocation);
        put(item, 0x00, kNativeBoneItemProfile);
        put(item, 0x08, 0);
        put(item, 0x0c, 0);
    }

    // +10..+2B are deliberately untouched before the current reader call.
    read_current_slot20(item, handle, readers);
    return item;
}

void NativeResourceExtraItemParserCalls::renderer_hook(
    std::uintptr_t target, void* renderer) {
    remaining_.renderer_hook(target, renderer);
}

void* NativeResourceExtraItemParserCalls::parse_item(
    std::uintptr_t target, void* parser, void* handle) {
    if (target == 0x00b8a910u)
        return parse_native_animation_channels_item_00b8a910(handle, readers_);
    if (target == 0x00b8a990u)
        return parse_native_bone_item_00b8a990(handle, readers_);
    return remaining_.parse_item(target, parser, handle);
}

void NativeResourceExtraItemParserCalls::append_item(
    std::uintptr_t target, void* resource, void* item) {
    remaining_.append_item(target, resource, item);
}
} // namespace bsp
