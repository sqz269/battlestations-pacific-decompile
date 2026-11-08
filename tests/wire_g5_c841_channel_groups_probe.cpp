// One ownership case: crossing the native outer-table growth boundary keeps
// the first group's actual DSP allocation, name and FMOD handle together.
#include "bsp/sound_configuration.hpp"
#include <cstdio>
#include <stdexcept>

static void require(bool value) {
    if (!value) throw std::runtime_error("chain841 channel-group ownership failed");
}

int main() {
    bsp::SoundConfigurationState state;
    int handle{};
    auto& first = bsp::sound_cc13::append_default_sound_configured_group_00a7faf0(state);
    require(&first == &state.channel_groups_128.back() && first.name.empty()
        && first.channel_group == nullptr && first.dsps.empty());
    first.name = "retained group";
    first.channel_group = &handle;
    const auto word = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&handle));
    bsp::sound_cc13::append_sound_group_effect_pointer_00a7b2d0(first.dsps.native_header(), word);
    auto* const actual_dsp_slots = first.dsps.native_header().data;
    for (int i = 1; i != 17; ++i)
        bsp::sound_cc13::append_default_sound_configured_group_00a7faf0(state);
    require(state.channel_groups_128.size() == 17 && state.channel_group_capacity_130 == 32);
    auto& retained = state.channel_groups_128.front();
    require(retained.name == "retained group" && retained.channel_group == &handle
        && retained.dsps.native_header().data == actual_dsp_slots
        && retained.dsps.back() == &handle);
    while (state.channel_groups_128.size() != 1)
        bsp::sound_cc13::pop_back_sound_configured_group_00a7f110(state);
    require(state.channel_groups_128.front().dsps.native_header().data == actual_dsp_slots);
    bsp::sound_cc13::pop_back_sound_configured_group_00a7f110(state);
    bsp::sound_cc13::pop_back_sound_configured_group_00a7f110(state); // empty is a no-op
    require(state.channel_groups_128.empty() && state.channel_group_capacity_130 == 32);
    std::puts("chain841 channel-group relocation, storage retirement and empty pop passed");
}
