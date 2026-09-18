#pragma once
#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/sound_sample_cache.hpp"

namespace bsp {
struct NativeGlobalConfigSoundContext {
    SoundAuxiliaryTreeOwner* volatile& current_cache_00f8bbe8;
    SoundSampleCacheContext& cache;
    GameplayEffectComponentLifetime& lifetime;
};
// Full008DBE90..008DBF3A[171]. ECX embedded GlobalConfig+2C0 storage,
// stack signed index/name, AL1, RET8. The index is unchecked DWORD arithmetic.
// Capture old slot AFTER acquisition, publish new before retain/release, then
// release the CURRENT temporary. Existing source cache supplies its real
// lookup/creation contract; its host-map layout is not native STL storage.
// Supported C++ unwind from assignment releases the temporary; acquisition
// failure does not. No drop-in original FH3/SEH or register-ABI claim.
bool assign_global_config_sound_008dbe90(void* actual_effects, std::int32_t index,
    const NativeString& name, NativeGlobalConfigSoundContext&);

// 0087D7B0's four difficulty-vector end-insertion contracts (0045A120 ->
// 00459CE0). Reuse concrete checked storage: actual10h header, 1.5x growth,
// native x87 float fill and begin/capacity/end publication after old release.
// Opaque header0 survives. Valid owned storage and no structural mutation
// during allocation are required. No general native STL iterator interface.
void append_global_config_multiplier_storage(void* actual_vector, std::uint32_t value_bits);
} // namespace bsp
