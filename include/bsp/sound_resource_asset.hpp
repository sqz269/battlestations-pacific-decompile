#pragma once

#include "bsp/sound_system_owner.hpp"
#include "bsp/vfs_candidates.hpp"
#include "bsp/vfs_mounts.hpp"

#include <array>
#include <optional>

namespace bsp {

// Typed ownership projection of native 2Ch resource D5B118. This is not its
// binary layout. Native +1C/+20/+24 are unwritten on the event-project branch.
// +10 is borrowed from +0C; only the bank and event project own FMOD references.
// No implicit resource or NativeString cleanup: use the recovered destructor.
struct SoundOwnedResource {
    std::uint32_t native_vtable_00{0x00d5b118};
    std::uint32_t references_04{1};
    void* event_project_08{};
    void* fsb_bank_0c{};
    void* fsb_subsound_10{};
    NativeString name_14;
    std::optional<std::uint32_t> pcm_length_1c;
    std::optional<float> frequency_20;
    std::optional<float> duration_24;
    std::uint32_t size_28{};
};

// 00A823FA..00A82424 constructs precisely 6Ch zero bytes, then cbsize at0 and
// VFS byte length at4. Remaining FMOD 4.18.04 fields are zero, not guessed types.
struct SoundFmodCreateInfo41804 {
    std::uint32_t cbsize{0x6c};
    std::uint32_t length{};
    std::array<std::uint32_t, 25> zero_fields{};
};
static_assert(sizeof(SoundFmodCreateInfo41804) == 0x6c);

// Only genuine installed FMOD entry points. The game's branch/mode/ownership
// logic is implemented below, not delegated to a generic asset-load callback.
class SoundResourceAssetFmodHost {
public:
    virtual ~SoundResourceAssetFmodHost() = default;
    virtual void memory_get_stats(std::int32_t*, std::int32_t*) = 0;
    virtual FmodResult create_sound(void* system, const char* data,
        std::uint32_t mode, void* extra_info, void** sound) = 0;
    virtual FmodResult sound_get_length(void* sound, std::uint32_t*, std::uint32_t unit) = 0;
    virtual FmodResult sound_get_num_subsounds(void* sound, std::int32_t*) = 0;
    virtual FmodResult sound_get_subsound(void* sound, std::int32_t index, void**) = 0;
    virtual FmodResult sound_get_mode(void* sound, std::uint32_t*) = 0;
    virtual FmodResult sound_get_defaults(void* sound, float* frequency,
        float* volume, float* pan, std::int32_t* priority) = 0;
    virtual FmodResult sound_set_3d_min_max_distance(void* sound, float, float) = 0;
    virtual FmodResult sound_set_variations(void* sound, float, float, float) = 0;
    virtual FmodResult sound_set_loop_points(void* sound, std::uint32_t start,
        std::uint32_t start_unit, std::uint32_t end, std::uint32_t end_unit) = 0;
    virtual FmodResult sound_set_mode(void* sound, std::uint32_t mode) = 0;
    virtual FmodResult event_system_load(void* event_system, const char* path,
        void* load_info, void** project) = 0;
};

// The separately reconstructed resource deleting destructor is needed only for
// a completed but invalid constructor. Constructor unwind releases the name.
class SoundResourceAssetLifetimeHost {
public:
    virtual ~SoundResourceAssetLifetimeHost() = default;
    virtual void delete_resource_00a85ac0(SoundOwnedResource&, std::uint32_t flags) = 0;
};

struct SoundResourceAssetContext {
    VfsMountContext& mounts;
    const VfsCandidateRegistrations& registrations;
    NativeStringStorage& strings;
    SoundResourceAssetFmodHost& fmod;
    void* system;       // sound-system +44
    void* event_system; // sound-system +48
    SoundResourceAssetLifetimeHost& lifetime;
};

// 00A82EA0: hidden return/name/options stack, RET0C; owner/options unused.
// Uses canonical VFS resolution; failure returns literal sound/gui/error.fsb.
// Returned NativeString owns supplied string storage and must be released.
NativeString resolve_sound_resource_00a82ea0(const NativeString&,
    VfsMountContext&, const VfsCandidateRegistrations&, NativeStringStorage&);

// Fresh object only. 00A83450 ECX=this, name/options stack, RET8/EAX=this.
// Uses recovered substring/equality behavior for .fsb and .fev, including
// short-name signed-start clamping. Callers own the actual eventual teardown.
SoundOwnedResource& construct_sound_resource_00a83450(SoundOwnedResource&,
    const NativeString&, const SoundResourceLoadOptions&, SoundResourceAssetContext&);

// 00A835B0 owner ECX unused, name/options stack, RET8. Checks VFS existence,
// allocates the resource, constructs it, accepts only +10 or +8 nonnull;
// otherwise invokes deleting-dtor +4 with flags1 and returns null.
SoundOwnedResource* create_sound_resource_00a835b0(const NativeString&,
    const SoundResourceLoadOptions&, SoundResourceAssetContext&);

// Game-side load branches; native A823F0 ECX=this/options stack RET4 and
// A82720 ECX=this RET0. These use the existing canonical VFS memory adapters.
// Host guards reject unavailable/incomplete file data and FMOD output queries
// that would leave native stack outputs indeterminate. See the evidence doc.
void load_sound_bank_00a823f0(SoundOwnedResource&,
    const SoundResourceLoadOptions&, SoundResourceAssetContext&);
void load_sound_event_project_00a82720(SoundOwnedResource&, SoundResourceAssetContext&);

} // namespace bsp
