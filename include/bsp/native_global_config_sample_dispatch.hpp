#pragma once

#include "bsp/global_config.hpp"

namespace bsp {
class SoundSampleRuntime;

// The caller admits this union before dispatch: complete Source 7Ch samples
// with current D5B074 from the retained canonical runtime/factory/cache, with
// the same strings, resources and allocator; or actual raw objects with
// compatible callable current-process tables whose current table word differs
// from D5B074. A numeric stamp alone proves neither provenance nor lifetime,
// and a numeric profile identity is not a callable table.
// Stop accepts only the admitted raw domain; Source D5B074 samples have no
// valid stop slot. Retain the canonical services through every callback, which
// must be nonthrowing. The caller owns all reference operations and slot writes.
// Receivers are non-null and live at entry. Zero is called only after the
// caller's real actual+4 decrement reaches zero.
class NativeGlobalConfigSampleDispatch final : public GlobalConfigEffects {
public:
    explicit NativeGlobalConfigSampleDispatch(SoundSampleRuntime& canonical_samples) noexcept;
    NativeGlobalConfigSampleDispatch(const NativeGlobalConfigSampleDispatch&) = delete;
    NativeGlobalConfigSampleDispatch& operator=(const NativeGlobalConfigSampleDispatch&) = delete;
    NativeGlobalConfigSampleDispatch(NativeGlobalConfigSampleDispatch&&) = delete;
    NativeGlobalConfigSampleDispatch& operator=(NativeGlobalConfigSampleDispatch&&) = delete;

    void stop_slot_08(void* actual_object, std::uint32_t flag) noexcept override;
    void zero_references_slot_00(void* actual_object) noexcept override;

private:
    SoundSampleRuntime& canonical_samples_;
    NativeGlobalConfigCurrentDispatch current_dispatch_;
};
} // namespace bsp
