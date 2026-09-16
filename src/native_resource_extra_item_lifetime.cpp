#include "bsp/native_resource_extra_item_lifetime.hpp"

#include "bsp/native_camera_group_resource.hpp"
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/native_resource_extra_item_parsers.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource extra item lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;

static_assert(sizeof(void*) == 4);

Word bits(const void* pointer) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* base, Word offset = 0) noexcept {
    return pointer(bits(base) + offset);
}
Word word(const void* base, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(base, offset));
}
void put(void* base, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(base, offset)) = value;
}
NativeRenderPointerArrayStorage& groups(void* item) noexcept {
    return *static_cast<NativeRenderPointerArrayStorage*>(at(item, 8));
}

void destroy_group_vector(void* item) {
    auto& array = groups(item);
    resize_native_instance_entry_pointers_00b1c770(array, 0);
    singleton_lifetime_free(array.data_00);
}

void delete_current_group(void* group,
    NativeResourceExtraItemLifetimeContext& context) {
    const Word profile = word(group);
    const auto* const actual = context.group_profile_00d62ed4;
    if (profile != 0x00d62ed4u || actual == nullptr || actual[1] != 0x00b78d00u)
        throw std::runtime_error(
            "Current AnimationChannels group terminal is outside the reconstructed domain");
    context.group_deletes.delete_vslot04(group, profile, 1);
}

void give_back_bone_name(void* item, NativeStringRawPoolContext& strings) {
    auto* const data = pointer(word(item, 0x0c));
    if (data == nullptr) return;
    const Word bytes = word(item, 8) + 1u;
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,
        strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, bytes,
        strings.actual_small_returns_disabled_01090aa4);
}
} // namespace

void destroy_native_animation_channels_item_00b8a680(void* item,
    NativeResourceExtraItemLifetimeContext& context) {
    put(item, 0, kNativeAnimationChannelsItemProfile);
    int state = 1;
    try {
        while (word(item, 0x0c) != 0) {
            const Word cell_address =
                word(item, 8) + word(item, 0x0c) * 4u - 4u;
            void* const cell = pointer(cell_address);
            void* const group = pointer(word(cell));
            if (group != nullptr) {
                delete_current_group(group, context);
                put(cell, 0, 0);
            }
            const Word current_count = word(item, 0x0c);
            if (current_count != 0) put(item, 0x0c, current_count - 1u);
        }
        state = 0;
        auto& array = groups(item);
        resize_native_instance_entry_pointers_00b1c770(array, 0);
        singleton_lifetime_free(array.data_00);
        state = -1;
        destroy_native_resource_item_base_00b86890(item);
    } catch (...) {
        try {
            if (state >= 1) destroy_group_vector(item);
            if (state >= 0) destroy_native_resource_item_base_00b86890(item);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
}

void* delete_native_animation_channels_item_00b8a760(void* item,
    Word flags, NativeResourceExtraItemLifetimeContext& context) {
    destroy_native_animation_channels_item_00b8a680(item, context);
    if ((flags & 1u) != 0) singleton_lifetime_free(item);
    return item;
}

void destroy_native_bone_item_00b8a8a0(void* item,
    NativeStringRawPoolContext& strings) {
    try {
        give_back_bone_name(item, strings);
    } catch (...) {
        destroy_native_resource_item_base_00b86890(item);
        throw;
    }
    destroy_native_resource_item_base_00b86890(item);
}

void* delete_native_bone_item_00b8af10(void* item, Word flags,
    NativeStringRawPoolContext& strings) {
    destroy_native_bone_item_00b8a8a0(item, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(item);
    return item;
}

NativeResourceExtraItemReferences::NativeResourceExtraItemReferences(
    NativeAdoptedSubstreamDispatch& other,
    NativeResourceExtraItemLifetimeContext& context,
    const volatile Word* animation_profile,
    const volatile Word* bone_profile) noexcept
    : other_(other), context_(context), animation_profile_(animation_profile),
      bone_profile_(bone_profile) {}

std::uint8_t NativeResourceExtraItemReferences::source_is_open(
    std::uintptr_t entry, void* source) {
    return other_.source_is_open(entry, source);
}

Word NativeResourceExtraItemReferences::source_seek(std::uintptr_t entry,
    void* source, Word low, Word high, Word origin) {
    return other_.source_seek(entry, source, low, high, origin);
}

void NativeResourceExtraItemReferences::source_read(std::uintptr_t entry,
    void* source, void* destination, Word requested, Word* actual) {
    other_.source_read(entry, source, destination, requested, actual);
}

void NativeResourceExtraItemReferences::source_write(std::uintptr_t entry,
    void* source, const void* bytes, Word requested, Word* actual) {
    other_.source_write(entry, source, bytes, requested, actual);
}

void NativeResourceExtraItemReferences::source_zero_reference(
    std::uintptr_t entry, void* item, std::uintptr_t captured_profile) {
    if (entry != 0x00bd30e0u ||
        (captured_profile != kNativeAnimationChannelsItemProfile &&
         captured_profile != kNativeBoneItemProfile)) {
        other_.source_zero_reference(entry, item, captured_profile);
        return;
    }
    if (item == nullptr) return;
    const Word current_profile = word(item);
    if (captured_profile == kNativeAnimationChannelsItemProfile) {
        if (current_profile != kNativeAnimationChannelsItemProfile ||
            animation_profile_ == nullptr ||
            animation_profile_[1] != 0x00b8a760u)
            throw std::runtime_error(
                "Current AnimationChannels item terminal is outside the reconstructed domain");
        delete_native_animation_channels_item_00b8a760(item, 1, context_);
        return;
    }
    if (current_profile != kNativeBoneItemProfile || bone_profile_ == nullptr ||
        bone_profile_[1] != 0x00b8af10u)
        throw std::runtime_error(
            "Current Bone item terminal is outside the reconstructed domain");
    delete_native_bone_item_00b8af10(item, 1, context_.strings);
}
} // namespace bsp
