#include "bsp/native_wreck_effect_providers.hpp"
#include "bsp/sound_retained_factory.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
class ActualWreckReferences final : public VoiceReferenceHost {
public:
    explicit ActualWreckReferences(NativeRenderActualOwners& owners) noexcept
        : owners_(owners) {}
    void retain_reference(void*) override {
        throw std::logic_error("native adoption must not retain its replacement");
    }
    void release_reference(void* captured) noexcept override {
        // VoiceReferenceHost's canonical release domain is nonthrowing.
        // A missing/mismatched actual owner is a violated required contract.
        release_native_render_actual_owner(owners_, captured);
    }
private:
    NativeRenderActualOwners& owners_;
};
} // namespace

void stop_native_wreck_effects_008674c0(NativeLiveEffectManagerStorage& manager,
    void* filter, NativeWreckEffectStopAccess& access) {
    const volatile auto& header = manager.effects_10;
    const auto count = static_cast<std::uint32_t>(header.count_04);
    auto cursor = reinterpret_cast<std::uintptr_t>(header.data_00);
    const auto end = cursor + static_cast<std::uintptr_t>(count) * 4u;
    while (cursor != end) {
        auto& cell = *reinterpret_cast<void* volatile*>(cursor);
        if (!filter || cell == filter)
            access.call_008673b0(cell);
        cursor += 4u;
    }
}

void* volatile& adopt_native_wreck_effect_00484620(void* volatile& cell,
    void* replacement, NativeRenderActualOwners& owners) noexcept {
    ActualWreckReferences references(owners);
    return adopt_sound_reference_0054d510(cell, replacement, references);
}
} // namespace bsp
