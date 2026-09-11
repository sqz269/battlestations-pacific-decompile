#pragma once

#include "bsp/sound_system_owner.hpp"

namespace bsp {

// Genuine FMOD boundaries reached by native 00A85790. Its bank Sound::release
// and EventProject table slot0 both return FMOD_RESULT; only 2Bh is checked.
class SoundResourceCleanupFmodHost {
public:
    virtual ~SoundResourceCleanupFmodHost() = default;
    virtual FmodResult release_sound(void* bank) = 0;
    virtual FmodResult release_event_project(void* project) = 0;
    virtual void memory_get_stats(std::int32_t* current, std::int32_t* maximum) = 0;
    virtual void on_out_of_sound_memory(const char* message) = 0;
};

class SoundResourceCacheRemovalHost {
public:
    virtual ~SoundResourceCacheRemovalHost() = default;
    // Reload the CURRENT sound singleton's +54 owner (00A79910), then run
    // 00A85560 with this name. It may remove a cache record and change count/
    // accounting. No fallback or storage-only implementation is supplied.
    virtual void remove_from_current_cache(const NativeString& name) = 0;
};

struct SoundResourceCleanupContext {
    NativeStringStorage& strings;
    volatile std::uint32_t& actual_resource_bytes_00f8bbe4;
    SoundResourceCleanupFmodHost& fmod;
    SoundResourceCacheRemovalHost& cache;
};

// Concrete D5B118 resource projections defined in sound_resource_asset.hpp.
// Native retain/release ignore ECX manager, take resource stack, RET4. No null
// guards. These use the actual references_04 word with Windows interlocked ops.
SoundOwnedResource* retain_sound_resource_00a854c0(SoundOwnedResource&) noexcept;
void release_sound_resource_00a854e0(SoundOwnedResource&, SoundResourceCleanupContext&);
// Native ECX resource, MOV EAX,[ECX+28]; RET. No size query or recomputation.
std::uint32_t sound_resource_size_00a818b0(const SoundOwnedResource&) noexcept;

// 00A85790: native ECX resource, plain RET. Release bank then event project,
// remove from the CURRENT cache, clear handles, subtract current size from the
// shared byte counter, release pooled name, write CEB130. Borrowed subsound is
// never separately released. Failed FMOD results do not abort this sequence.
// An exception unwinds name/base only; remaining FMOD releases are not retried.
void destroy_sound_resource_00a85790(SoundOwnedResource&, SoundResourceCleanupContext&);
// 00A85AC0: ECX resource, stack flags, RET4/EAX original pointer. Destroy then
// free standard-new projection storage only for flags bit0. This is not native
// allocation or a whole object ABI. Storage remains allocated if destroy throws.
SoundOwnedResource* delete_sound_resource_00a85ac0(SoundOwnedResource*,
    std::uint32_t flags, SoundResourceCleanupContext&);

// Only the <=current-count shrink branch of 00A845A0; ECX native header owner+4,
// requested signed count stack, RET4. Domain: 0<=requested<=current count.
// Reverse record-storage destruction; no resource releases or accounting changes.
// Native decrements count before destruction; this vector projection clears the
// record then pops it, with no native allocator callback to observe that interval.
// Growth/allocation/record construction excluded; cleanup itself allocates nothing.
void shrink_sound_resource_cache_records_00a845a0_fragment(SoundResourceOwner&,
    std::uint32_t requested) noexcept;
// 00A84D00: ECX=&owner+4, RET. Shrink record storage to zero then free its
// allocation; never release resources. Also used by base-destructor unwind.
void destroy_sound_resource_cache_storage_00a84d00(SoundResourceOwner&) noexcept;

// 00A84C90: ECX owner, RET. For each CURRENT tail, subtract its size, release it,
// then reload count and destroy/pop the CURRENT tail when nonempty. A resource
// destructor may have already removed a record through 00A85560; preserve the
// observed reentrant ordering, including native repeated accounting/removal.
// Entries used for a size/release must hold nonnull live D5B118 resources.
void clear_sound_resource_cache_00a84c90(SoundResourceOwner&, SoundResourceCleanupContext&);

// 00A85500: ECX owner, RET. Set D5B1E8, clear resources, shrink records to zero,
// free record storage. Logical native capacity is left stale as in the image.
// On resource-clear exception, remaining record STORAGE is destroyed/freed;
// opaque resource releases are not retried by this native unwind path.
void destroy_sound_resource_owner_base_00a85500(SoundResourceOwner&,
    SoundResourceCleanupContext&);
// 00A85A50: set D5B210; nullsafe release +14, clear it only after release returns,
// then base cleanup. Base cleanup also runs if the +14 final release throws.
// This does not free the owner projection itself or its sound/FMOD system.
void destroy_sound_resource_owner_00a85a50(SoundResourceOwner&,
    SoundResourceCleanupContext&);

} // namespace bsp
