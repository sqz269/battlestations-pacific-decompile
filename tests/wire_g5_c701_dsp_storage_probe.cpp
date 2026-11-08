// Focused synthetic ownership probe: growth crosses both native thresholds,
// outer-vector relocation preserves the inner allocation, and explicit native
// teardown followed by C++ destruction does not release storage twice.
#include "bsp/sound_configuration_shutdown.hpp"
#include <cstdio>
#include <stdexcept>

static void require(bool value) {
    if (!value) throw std::runtime_error("chain701 DSP storage probe failed");
}

struct Fmod final : bsp::SoundConfigurationShutdownFmodHost {
    int* handles;
    int next{8};
    bool removing{true};
    explicit Fmod(int* values) : handles(values) {}
    bsp::FmodResult dsp_remove(void* dsp) override {
        require(removing && dsp == &handles[next]);
        removing = false;
        return {};
    }
    bsp::FmodResult dsp_release(void* dsp) override {
        require(!removing && dsp == &handles[next]);
        --next;
        removing = true;
        return {};
    }
    bsp::FmodResult channel_group_release(void* group) override {
        require(next == -1 && removing && group == &handles[17]);
        return {};
    }
    void memory_get_stats(std::int32_t*, std::int32_t*) override {
        require(false);
    }
};

int main() {
    int handles[18]{};
    bsp::SoundConfigurationState state;
    auto& groups = state.channel_groups_128;
    groups.reserve(1);
    groups.emplace_back();
    groups.back().channel_group = &handles[17];
    for (std::size_t i = 0; i != 17; ++i) {
        const auto word = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&handles[i]));
        bsp::sound_cc13::append_sound_system_dsp_pointer_00a7b310(
            state.system_dsps_134.native_header(), word);
        if (i < 9) bsp::sound_cc13::append_sound_group_effect_pointer_00a7b2d0(
            groups.back().dsps.native_header(), word);
    }
    require(groups[0].dsps.native_header().capacity == 12);
    require(state.system_dsps_134.native_header().capacity == 32);
    auto* const backing = groups[0].dsps.native_header().data;
    groups.emplace_back(); // Relocates the first record, whose array is move-only.
    require(groups[0].dsps.native_header().data == backing && groups[0].dsps.size() == 9);
    require(groups[0].dsps.back() == &handles[8]);
    require(state.system_dsps_134.back() == &handles[16]);
    Fmod fmod(handles);
    bsp::release_sound_configured_group_effects_00a7aaf0(groups[0], fmod);
    bsp::destroy_sound_configured_group_storage_00a7bfe0(groups[0]);
    require(groups[0].dsps.native_header().data == nullptr && groups[0].dsps.empty());
    state.system_dsps_134.reset(); // Storage only: EventSystem owns these objects.
    require(state.system_dsps_134.native_header().data == nullptr && state.system_dsps_134.empty());
    std::puts("chain701 DSP growth, relocation, reverse release and teardown passed");
}
