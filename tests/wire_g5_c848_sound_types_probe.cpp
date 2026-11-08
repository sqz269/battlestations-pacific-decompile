// Synthetic ownership check for the loader's independent temporary record.
#include "bsp/sound_cc13.hpp"
#include "bsp/sound_configuration.hpp"
#include <cstdio>

int main() {
    bsp::SoundConfigurationState state;
    int group_handles[2]{};
    bsp::SoundConfiguredType source{
        "sound-type-name-with-owned-storage", {&group_handles[0], nullptr, &group_handles[1]}};
    constexpr int capacities[] = {1, 2, 4, 4, 8};
    for (int i = 0; i != 5; ++i) {
        bsp::sound_cc13::append_sound_configured_type_00a7feb0(state, source);
        if (state.types_38.size() != static_cast<std::size_t>(i + 1)
            || state.type_capacity_40 != capacities[i]
            || source.name != "sound-type-name-with-owned-storage"
            || source.by_listener.size() != 3) return 1;
    }
    source.name.clear();
    source.by_listener[0] = nullptr;
    for (const auto& stored : state.types_38) {
        if (stored.name != "sound-type-name-with-owned-storage"
            || stored.by_listener.size() != 3
            || stored.by_listener.data() == source.by_listener.data()
            || stored.by_listener[0] != &group_handles[0]
            || stored.by_listener[1] != nullptr
            || stored.by_listener[2] != &group_handles[1]) return 2;
    }
    std::puts("c848: growth and independent record storage retain borrowed group identities");
}
