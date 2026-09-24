#include "bsp/unit_instance_step11.hpp"

#include "bsp/unit_kind_query.hpp"

#include <array>

// Packet cc9_unit_instance_step11, docs/UNIT_INSTANCE_STEP11.md.
//
// The host these runners serve builds no scene model (00928860 is a record), no
// sound emitters and no effect handles (006FE590 is a record), so every handle
// the routines test is null and every null test takes the image's own skip.
// Each adapter below answers the handle tests with "absent". The methods behind
// those tests are unreachable under that state and are empty.

namespace bsp {

std::size_t unit_prune_finished_effects_0081c050(UnitEffectPruneHost& host) {
    std::size_t erased = 0;
    std::size_t index = 0;
    // 0081C090..0081C09A: begin and count are re-read after every element.
    while (index < host.effect_count()) {
        if (host.effect_finished(index)) { // 0081C072, 00865FE0
            host.erase_effect(index);      // 0081C082, 0081B010; the walk stays put
            ++erased;
        } else {
            ++index;                       // 0081C089
        }
    }
    return erased;
}

namespace {

// The +FFCh array this host never fills.
class EmptyEffectArray final : public UnitEffectPruneHost {
public:
    std::size_t effect_count() override { return 0; }
    bool effect_finished(std::size_t) override { return false; }
    void erase_effect(std::size_t) override {}
};

// 008252C0's host. The emitters at +BB4h/+BB8h/+BBCh and the object at +BC0h
// are absent, so only the settings read and the kind tests are reachable.
class EngineAudioAdapter final : public UnitEngineAudioHost {
public:
    EngineAudioAdapter(const UnitStep11Inputs& in, const EngineSoundSmoothRates& rates)
        : in_(in), rates_(rates) {}
    float settings_rate(std::size_t block_offset) override {
        // settings+5BCh + i*24h, +8h. The record index is the block's distance
        // from +5BCh in 24h steps.
        const std::size_t index = (block_offset - kUnitAudioSettingsDefault) / 0x24;
        return index < kEngineSoundRecordCount ? rates_.rate[index]
                                               : kEngineSoundSmoothRateDefault;
    }
    bool is_kind_of(int class_id) override { return unit_is_kind_of(in_.class_id, class_id); }
    float reference_speed() override { return 0.0f; } // only behind emitter C
    float forward_speed() override { return 0.0f; }   // only behind emitter C
    void set_emitter_parameter(std::size_t, const char*, float) override {}
    void stop_audio_target(float) override {}

private:
    const UnitStep11Inputs& in_;
    const EngineSoundSmoothRates& rates_;
};

// 00956600's host. The descriptor at +354h is not built, so its damage table
// is empty; the animation owner at +360h is absent (+70Ch clear) and there is
// no scene node at +4A4h.
class TimerAdapter final : public UnitTimerHost {
public:
    explicit TimerAdapter(const UnitStep11Inputs& in) : in_(in) {}
    std::size_t damage_record_count() override { return 0; }
    UnitDamageRecord damage_record(std::size_t) override { return {}; }
    std::size_t descriptor_anchor_count() override { return 0; }
    float health() override { return 0.0f; }
    const std::array<float, 3>& descriptor_anchor(std::size_t) override { return zero_; }
    void announce_damage_record(std::size_t, const std::array<float, 3>&) override {}
    void spawn_damage_effect(std::size_t, const std::array<float, 3>&) override {}
    bool animation_target_ready() override { return false; }
    void animation_pre_step_00b78670() override {}
    void animation_step() override {}
    int effective_game_mode() override { return 0; }
    bool mission_reveal_byte() override { return false; }
    bool global_intensity_override() override { return in_.global_intensity_override; }
    void set_visibility_factor(float) override {}

private:
    const UnitStep11Inputs& in_;
    std::array<float, 3> zero_{};
};

// 00834E90's host and the one its three tail routines share. No steering or
// propeller nodes (+107Ch, +106Ch) and no wave, spray or cavitation handles
// (+9E8h, +9ECh, +A00h..+A0Ch, +B44h..).
class PropellerAdapter final : public UnitPropellerHost, public UnitTimedSubUpdateHost {
public:
    PropellerAdapter(UnitStep11State& state, const UnitStep11Inputs& in)
        : state_(state), in_(in) {}
    // UnitPropellerHost
    float sample_controller_speed() override { return in_.controller_speed; }
    void steering_node_local_position(std::size_t, float out[3]) override {
        out[0] = out[1] = out[2] = 0.0f;
    }
    float propeller_node_local_x(std::size_t) override { return 0.0f; }
    void set_steering_node_transform(std::size_t, float, const float[3]) override {}
    void spin_propeller_node(std::size_t, float) override {}
    void start_cavitation_effect(std::size_t) override {}
    void stop_cavitation_effect(std::size_t) override {}
    void sub_update_00834820(float delta) override {
        UnitWaveSubUpdateState wave{};
        wave.effect = nullptr;          // +9E8h
        wave.ocean_available = true;    // [00E188A8]+19F0h, the host's ocean
        wave.timer = state_.bow_wave_timer;
        unit_update_bow_wave_00834820(wave, *this, delta);
        state_.bow_wave_timer = wave.timer;
    }
    void sub_update_00834cc0(float delta) override {
        UnitWaveSubUpdateState wave{};
        wave.effect = nullptr;          // +9ECh
        wave.timer = state_.stern_wave_timer;
        unit_update_stern_wave_00834cc0(wave, *this, delta);
        state_.stern_wave_timer = wave.timer;
    }
    void sub_update_00834a70(float delta) override {
        UnitSpraySubUpdateState spray{}; // +A00h..+A0Ch all null
        spray.timer = state_.spray_timer;
        spray.throttle = in_.throttle;
        // SUBSTITUTION: class+650h/+654h (the spray points) are not carried by
        // the host's class block. With every slot null the walk only skips
        // (00834AE8), so the count changes nothing and is left at zero.
        unit_update_spray_00834a70(spray, *this, delta);
        state_.spray_timer = spray.timer;
    }
    // UnitTimedSubUpdateHost: unreachable behind the null handles.
    void refresh_unit_pose() override {}
    OceanVec3 transform_by_unit_pose(const OceanVec3& p) override { return p; }
    float water_height(float, float) override { return 0.0f; }
    void effect_resume(UnitTimedEffect*) override {}
    void effect_stop(UnitTimedEffect*) override {}
    void effect_set_point(UnitTimedEffect*, const OceanVec3&) override {}
    float unit_virtual_38() override { return 0.0f; }
    OceanVec3 unit_world_matrix_row2() override { return {}; }
    void effect_store_float(UnitTimedEffect*, std::size_t, float) override {}

private:
    UnitStep11State& state_;
    const UnitStep11Inputs& in_;
};

// 00815AA0's host: no effect groups anywhere on the unit.
class EffectIntensityAdapter final : public UnitEffectIntensityHost {
public:
    EffectIntensityAdapter(const UnitStep11Inputs& in, std::size_t parts)
        : in_(in), parts_(parts) {}
    bool global_intensity_override() override { return in_.global_intensity_override; }
    std::size_t part_count() override { return parts_; }
    bool group_present(int, std::size_t) override { return false; }
    void group_set_scalar(int, std::size_t, float) override {}

private:
    const UnitStep11Inputs& in_;
    std::size_t parts_;
};

} // namespace

void run_engine_audio_008252c0(UnitStep11State& state, const UnitStep11Inputs& in,
    const EngineSoundSmoothRates& rates, float scaled_delta) {
    if (!state.audio_initialised) {
        // 00822CBE, SEntityInit: MOV byte [ESI+9C4h],1, unconditionally.
        state.audio.enabled = true;
        state.audio_initialised = true;
    }
    state.audio.gate_5d = in.gate_5d;
    state.audio.throttle = in.throttle;
    EngineAudioAdapter host(in, rates);
    unit_update_engine_audio_008252c0(state.audio, host, scaled_delta);
}

void run_unit_timers_00956600(UnitInstanceState& unit, UnitStep11State& state,
    const UnitStep11Inputs& in, float scaled_delta) {
    UnitTimerState timers{};
    timers.age = unit.age;
    timers.clamped_countdown = unit.clamped_countdown;
    timers.damage_scan_timer = unit.fire_countdown;
    timers.damage_scan_mark = 0.0f; // +364h, read only behind a non-empty table
    // +2F4h is the same cell as intensity_scale. SUBSTITUTION: +2F8h's writers
    // (0077F030 in the owner constructor, the setter 00779C60 from 006D3200 and
    // 007BC550, and 00953B55) are not reconstructed, so the host holds the
    // target at the current scale and 009569F8's chase takes its "equal" arm.
    timers.fade = unit.intensity_scale;
    timers.fade_target = unit.intensity_scale;
    timers.local_intensity_override = unit.intensity_override;
    timers.animation_gate = false;  // +70Ch
    timers.animation_extra = false; // +714h
    timers.has_scene_node = false;  // +4A4h
    TimerAdapter host(in);
    unit_update_timers_00956600(timers, unit.pose, host, scaled_delta);
    unit.age = timers.age;
    unit.clamped_countdown = timers.clamped_countdown;
    unit.fire_countdown = timers.damage_scan_timer;
    unit.intensity_scale = timers.fade;
    state.fade = timers.fade;
    state.fade_target = timers.fade_target;
}

void run_propellers_00834e90(UnitStep11State& state, const UnitStep11Inputs& in,
    float scaled_delta) {
    UnitPropellerState prop{};
    prop.throttle = in.throttle;
    prop.steering = in.steering;
    prop.use_alternate_inputs = in.alternate_inputs;
    // +FC4h/+FDCh are only read when +61h is set, which nothing sets.
    // SUBSTITUTION: +1030h (the propeller load) is not carried; it feeds only
    // the per-propeller term, which no node reaches.
    prop.load = 0.0f;
    prop.steering_angle = state.steering_angle;
    for (std::size_t i = 0; i < kUnitPropellerCount; ++i) prop.rates[i] = state.propeller_rates[i];
    // SUBSTITUTION: [+538h]+69Ch/+6A0h/+6A4h are not read from the class block;
    // like the load they only reach the per-propeller loop.
    const UnitPropellerClassBlock class_block{};
    PropellerAdapter host(state, in);
    unit_update_propellers_00834e90(prop, class_block, host, scaled_delta);
    state.steering_angle = prop.steering_angle;
    for (std::size_t i = 0; i < kUnitPropellerCount; ++i) state.propeller_rates[i] = prop.rates[i];
}

UnitEffectIntensityResult run_effect_intensity_00815aa0(UnitInstanceState& unit,
    UnitStep11State& state, const UnitStep11Inputs& in, float gate) {
    UnitEffectIntensityState intensity{};
    intensity.published_latch = state.effect_latch;
    intensity.intensity_override = unit.intensity_override;
    intensity.intensity_scale = unit.intensity_scale;
    EffectIntensityAdapter host(in, unit.part_count);
    const UnitEffectIntensityResult result =
        publish_unit_effect_intensity_00815aa0(intensity, host, gate);
    state.effect_latch = intensity.published_latch;
    return result;
}

std::size_t run_prune_effects_0081c050() {
    EmptyEffectArray host;
    return unit_prune_finished_effects_0081c050(host);
}

} // namespace bsp
