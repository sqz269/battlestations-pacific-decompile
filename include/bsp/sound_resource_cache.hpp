#pragma once

#include "bsp/sound_system_owner.hpp"

namespace bsp {

// Required services reached by the reconstructed loader. Each operation is
// meaningful; there are no default resource, metadata, or message-pump stubs.
// Calls are serialized by the owning game thread. Resolve/create/services may
// recursively load resources; the loader retains no vector references over them.
class SoundResourceCacheHost {
public:
    virtual ~SoundResourceCacheHost() = default;
    virtual void pump_load_events_00beccd0() = 0;
    // Return the native resolver's owned result using the supplied storage.
    virtual NativeString resolve_00a82ea0(SoundResourceOwner&,
        const NativeString& normalized_request, SoundResourceLoadOptions&,
        NativeStringStorage&) = 0;
    virtual SoundOwnedResource* create_00a835b0(SoundResourceOwner&,
        const NativeString& canonical, SoundResourceLoadOptions&) = 0;
    virtual SoundOwnedResource* retain_00a854c0(SoundResourceOwner&,
        SoundOwnedResource&) = 0;
    // Native resource virtual +C; D5B118 dispatches to 00A818B0.
    virtual std::uint32_t resource_size_vslot_0c(SoundOwnedResource&) = 0;
    virtual std::array<std::uint32_t, 5> query_metadata_00bdd340(
        const NativeString& canonical) = 0;
};

// Full normal 00A84740 behavior, ECX=owner, four stack args, RET10. A cached
// nonnull hit always retains regardless of clone. A newly created pointer is
// retained only when clone=true. Null creations are recorded, and later loads
// can append more records for those aliases. The cache adopts the created
// reference; no extra reference is added by record copying or destruction.
// NativeString path temporaries use the supplied pool; standard containers
// project the native 2Ch records/list storage. This is not the native ABI or
// allocation-failure/SEH implementation. See docs/SOUND_RESOURCE_CACHE.md.
SoundOwnedResource* load_sound_resource_00a84740(SoundResourceOwner&,
    const NativeString& path, SoundResourceLoadOptions&, bool clone,
    bool load_if_missing, SoundResourceCacheHost&,
    NativeStringStorage& = crt_string_storage());

// Full normal 00A84530: ECX=&owner+4, stack source-record pointer, RET4.
// Minimum 64 records; double only when count==capacity. Source must be outside
// the destination vector, as in the native loader. Scalar resource is copied
// without retain. Host capacity may exceed this explicit native capacity.
void append_sound_resource_cache_record_00a84530(SoundResourceOwner&,
    const SoundResourceCacheRecord&);

// Full storage destruction 00A842E0, ECX=record, plain RET. Releases aliases
// before name storage, with no game-resource release or scalar metadata reset.
// This reusable projection clears its C++ fields; the native name header is
// dead after destruction and is left dangling by the original pool release.
void clear_sound_resource_cache_record_storage_00a842e0(
    SoundResourceCacheRecord&) noexcept;

// Full normal 00A84680; ECX=destination, stack source, returns destination,
// RET4. Copies name, clears/repopulates aliases, then copies metadata/resource.
// The allocator word represented by field_08 is not assigned by this routine.
SoundResourceCacheRecord& copy_sound_resource_cache_record_00a84680(
    SoundResourceCacheRecord& destination, const SoundResourceCacheRecord& source);

// Full normal 00A85560; ECX=owner, stack name, RET4. Searches all aliases,
// subtracts the first match's nonnull resource size, copies the live last entry
// over that match, then destroys/pops the live last entry. Does not release the
// resource. The normalized temporary is DISCARDED: search uses the input copy.
// The size callback must preserve the matched record's storage, just as the
// native EBP pointer must remain live; count/last-record are reloaded afterwards.
void remove_sound_resource_cache_entry_00a85560(SoundResourceOwner&,
    const NativeString& path, SoundResourceCacheHost&,
    NativeStringStorage& = crt_string_storage());

} // namespace bsp
