#include "bsp/plane_bow_wave.hpp"

namespace bsp {
void bind_plane_bow_wave_points_007d4812(PlaneBowWaveDefinition* records,
    std::size_t count, PlaneBowWaveMarkerHost& host) {
    for (std::size_t index = 0; index < count; ++index) {
        records[index].model_point = host.first_model_marker_point(
            "wave", static_cast<std::uint32_t>(index + 1));
    }
}

namespace {
struct TemporaryEffect {
    PlaneBowWaveEffectHost& host;
    std::uint32_t instance;
    ~TemporaryEffect() {
        if (instance != 0) host.release_effect(instance);
    }
};
}

void create_plane_bow_waves_007d5890(const PlaneBowWaveDefinition* records,
    std::size_t count, PlaneBowWaveEffectHost& host) {
    for (std::size_t index = 0; index < count; ++index) {
        const PlaneBowWaveTransform identity{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f};
        TemporaryEffect temporary{host,
            host.create_effect(records[index].effect_definition, identity, 0, 0)};
        // Native copies the model point after creation and before byte +9h.
        const PlaneBowWaveInstance instance{temporary.instance, records[index].model_point};
        if (temporary.instance != 0) host.set_effect_byte_09(temporary.instance, 0);
        host.append_bow_wave(instance);
    }
}
}
