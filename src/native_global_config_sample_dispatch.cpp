#include "bsp/native_global_config_sample_dispatch.hpp"
#include "bsp/sound_sample_runtime.hpp"

#include <cstring>

namespace bsp {
NativeGlobalConfigSampleDispatch::NativeGlobalConfigSampleDispatch(
    SoundSampleRuntime& canonical_samples) noexcept
    : canonical_samples_(canonical_samples) {}

void NativeGlobalConfigSampleDispatch::stop_slot_08(void* actual_object,
    std::uint32_t flag) noexcept {
    current_dispatch_.NativeGlobalConfigCurrentDispatch::stop_slot_08(actual_object, flag);
}

void NativeGlobalConfigSampleDispatch::zero_references_slot_00(
    void* actual_object) noexcept {
    std::uint32_t profile;
    std::memcpy(&profile, actual_object, sizeof(profile));
    if (profile == 0x00d5b074u) {
        canonical_samples_.SoundSampleRuntime::zero_references_slot_00(actual_object);
        return;
    }
    current_dispatch_.NativeGlobalConfigCurrentDispatch::zero_references_slot_00(actual_object);
}
} // namespace bsp
