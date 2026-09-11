#include "bsp/sound_resource_cleanup.hpp"
#include "bsp/sound_resource_asset.hpp"
#include "bsp/sound_resource_cache.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cassert>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Sound resource cleanup requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(LONG) == sizeof(std::uint32_t));

struct ResourceNameAndBaseCleanup {
    SoundOwnedResource& resource;
    NativeStringStorage& strings;
    ~ResourceNameAndBaseCleanup() noexcept {
        // Normal A858A2..A858CC and CB5F68 -> CB5F60 unwind: name first,
        // reference base second. Native headers remain stale after release.
        destroy_native_string_header_0041dd20(&resource.name_14, strings);
        resource.native_vtable_00 = 0x00ceb130u;
    }
};

struct CacheStorageCleanup {
    SoundResourceOwner& owner;
    ~CacheStorageCleanup() noexcept {
        // Normal A8553F/free and CB5F20 -> A84D00 unwind only release storage.
        destroy_sound_resource_cache_storage_00a84d00(owner);
    }
};

struct OwnerBaseCleanupOnUnwind {
    SoundResourceOwner& owner;
    SoundResourceCleanupContext& context;
    bool armed = true;
    ~OwnerBaseCleanupOnUnwind() noexcept {
        if (armed) destroy_sound_resource_owner_base_00a85500(owner, context);
    }
};

void checkpoint(FmodResult result, SoundResourceCleanupFmodHost& fmod) {
    if (result == FmodResult::err_memory)
        fmod.on_out_of_sound_memory(kOutOfSoundMemoryMessage);
}
} // namespace

SoundOwnedResource* retain_sound_resource_00a854c0(SoundOwnedResource& resource) noexcept {
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(&resource.references_04));
    return &resource;
}

void release_sound_resource_00a854e0(SoundOwnedResource& resource,
    SoundResourceCleanupContext& context) {
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(&resource.references_04)) == 0)
        delete_sound_resource_00a85ac0(&resource, 1, context);
}

std::uint32_t sound_resource_size_00a818b0(const SoundOwnedResource& resource) noexcept {
    return resource.size_28;
}

void destroy_sound_resource_00a85790(SoundOwnedResource& resource,
    SoundResourceCleanupContext& context) {
    resource.native_vtable_00 = 0x00d5b118u;
    ResourceNameAndBaseCleanup cleanup{resource, context.strings};
    if (resource.fsb_bank_0c) {
        std::int32_t before{}, after{};
        context.fmod.memory_get_stats(&before, nullptr);
        checkpoint(context.fmod.release_sound(resource.fsb_bank_0c), context.fmod);
        context.fmod.memory_get_stats(&after, nullptr);
        // Native 004254B0 is exactly RET. Its log arguments are unused.
    }
    if (resource.event_project_08) {
        std::int32_t before{}, after{};
        context.fmod.memory_get_stats(&before, nullptr);
        checkpoint(context.fmod.release_event_project(resource.event_project_08), context.fmod);
        context.fmod.memory_get_stats(&after, nullptr);
    }
    context.cache.remove_from_current_cache(resource.name_14);
    const auto size = resource.size_28; // Native reads before handle clearing.
    resource.event_project_08 = nullptr;
    resource.fsb_bank_0c = nullptr;
    resource.fsb_subsound_10 = nullptr;
    context.actual_resource_bytes_00f8bbe4 = context.actual_resource_bytes_00f8bbe4 - size;
}

SoundOwnedResource* delete_sound_resource_00a85ac0(SoundOwnedResource* resource,
    std::uint32_t flags, SoundResourceCleanupContext& context) {
    destroy_sound_resource_00a85790(*resource, context);
    if (flags & 1u) delete resource;
    return resource;
}

void shrink_sound_resource_cache_records_00a845a0_fragment(SoundResourceOwner& owner,
    std::uint32_t requested) noexcept {
    assert(requested <= owner.records_04.size());
    while (requested < owner.records_04.size()) {
        // Do not move the std::list-bearing record: MSVC list move construction
        // can allocate a sentinel during teardown. Native count-before-free
        // observability is outside this standard-container storage projection.
        clear_sound_resource_cache_record_storage_00a842e0(owner.records_04.back());
        owner.records_04.pop_back();
    }
}

void clear_sound_resource_cache_00a84c90(SoundResourceOwner& owner,
    SoundResourceCleanupContext& context) {
    while (!owner.records_04.empty()) {
        const auto size = sound_resource_size_00a818b0(*owner.records_04.back().resource_28);
        owner.total_resource_size_10 -= size;
        // Reload the current tail before release and again after it returns.
        release_sound_resource_00a854e0(*owner.records_04.back().resource_28, context);
        if (!owner.records_04.empty()) {
            clear_sound_resource_cache_record_storage_00a842e0(owner.records_04.back());
            owner.records_04.pop_back();
        }
    }
    shrink_sound_resource_cache_records_00a845a0_fragment(owner, 0);
}

void destroy_sound_resource_cache_storage_00a84d00(SoundResourceOwner& owner) noexcept {
    shrink_sound_resource_cache_records_00a845a0_fragment(owner, 0);
    std::vector<SoundResourceCacheRecord>{}.swap(owner.records_04);
}

void destroy_sound_resource_owner_base_00a85500(SoundResourceOwner& owner,
    SoundResourceCleanupContext& context) {
    owner.native_vtable_00 = 0x00d5b1e8u;
    CacheStorageCleanup storage{owner};
    clear_sound_resource_cache_00a84c90(owner, context);
}

void destroy_sound_resource_owner_00a85a50(SoundResourceOwner& owner,
    SoundResourceCleanupContext& context) {
    owner.native_vtable_00 = 0x00d5b210u;
    OwnerBaseCleanupOnUnwind cleanup{owner, context};
    if (auto* resource = owner.error_resource_14) {
        release_sound_resource_00a854e0(*resource, context);
        owner.error_resource_14 = nullptr;
    }
    cleanup.armed = false;
    destroy_sound_resource_owner_base_00a85500(owner, context);
}
} // namespace bsp
