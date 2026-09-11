#pragma once
#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/sound_resource_runtime.hpp"
#include "bsp/sound_sample_cache.hpp"
#include "bsp/sound_sample.hpp"

namespace bsp {
// Concrete VFS/FMOD/resource-loader composition for D5B460 sample factories
// and D5B074 sample final release. Bind the canonical current sound owner,
// matching sound-system state, current sample cache and shared accounting word.
class SoundSampleRuntime final : public SoundSampleCacheHost, public SoundSampleHost,
    public GameplayEffectComponentLifetime {
public:
    SoundSampleRuntime(SoundSystemOwner* volatile&,
        SoundAuxiliaryTreeOwner* volatile&, std::uint32_t& actual_resource_bytes,
        VfsMountContext&, const VfsCandidateRegistrations&, SoundResourceRuntime&,
        FmodConfigurationLibrary&, NativeStringStorage&, const char* live_e17bf0,
        const char* live_f8bbec);
    SoundSampleCacheContext& sample_cache_context() override { return cache_; }
    SoundSampleContext& sample_context() noexcept { return sample_; }
    void* construct_sample_00a84d70(void*, const NativeString&) override;
    void* unknown_cache_create_slot_04(SoundAuxiliaryTreeOwner&, NativeString&) override;
    void unknown_cache_erase_slot_08(SoundAuxiliaryTreeOwner&, SoundAuxiliaryTreeOwner::Tree::iterator) override;
    void zero_references_slot_00(void*) override;
    bool resolve_name_00bdf4c0(NativeString&) override;
    std::unique_ptr<SoundSampleScanner> open_scanner_00bef2e0(const NativeString&) override;
    SoundResourceOwner& current_resource_owner_00a79910() override;
    SoundOwnedResource* load_resource_00a84740(SoundResourceOwner&, const NativeString&,
        void* actual_options, bool clone, bool load_if_missing) override;
    void* current_event_system_00f8bbd8_48() override;
    SoundAuxiliaryTreeOwner& current_sample_cache_00f8bbe8() override;
    void release_resource_00a854e0(SoundOwnedResource&) override;
private:
    SoundSystemOwner* volatile& current_owner_;
    SoundAuxiliaryTreeOwner* volatile& current_cache_;
    VfsMountContext& mounts_;
    const VfsCandidateRegistrations& registrations_;
    SoundResourceRuntime& resources_;
    NativeStringStorage& strings_;
    SoundSampleCacheContext cache_;
    SoundSampleContext sample_;
};
}
