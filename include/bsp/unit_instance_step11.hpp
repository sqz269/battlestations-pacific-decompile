#pragma once
// The unit instance's per-frame presentation sub-updates, run from a host that
// builds no scene model, no sound emitters and no effect handles for its units.
// Packet cc9_unit_instance_step11, docs/UNIT_INSTANCE_STEP11.md.
//
// 008255B0 (update_unit_instance_008255b0) calls six routines this packet
// wires:
//   step 4   00815AA0  publish_unit_effect_intensity_00815aa0 (unit_controller)
//   step 6   0081C050  unit_prune_finished_effects_0081c050 (here)
//   step 7   008227E0  NOT bound: the damage-smoke spawner, see the doc
//   step 11  008252C0  unit_update_engine_audio_008252c0 (unit_motion)
//            00956600  unit_update_timers_00956600 (unit_motion)
//            00834E90  unit_update_propellers_00834e90 (unit_motion), which
//                      ends with 00834820, 00834CC0 and 00834A70 (unit_timers)
//
// The runners below take only what the routines read. Every host term that is
// not the image's own value is named as a substitution at its field.

#include "bsp/unit_controller.hpp"
#include "bsp/unit_instance.hpp"
#include "bsp/unit_motion.hpp"
#include "bsp/unit_timers.hpp"

#include <cstddef>

namespace bsp {

// ---------------------------------------------------------------------------
// 0081C050, __thiscall(unit), RET 0, body 0081C050..0081C09F.
//
// Walks the effect-pointer array at unit+FFCh (begin at +FFCh, element count at
// +1000h). For each element, 00865FE0(effect) is asked whether the effect has
// finished; a finished element is erased through 0081B010 (vector erase at the
// iterator, which leaves the walk on the same position), anything else steps
// the iterator by 4. The loop re-reads begin and count after every element.
// ---------------------------------------------------------------------------
struct UnitEffectPruneHost {
    virtual ~UnitEffectPruneHost() = default;
    virtual std::size_t effect_count() = 0;               // [unit+1000h]
    virtual bool effect_finished(std::size_t index) = 0;  // 00865FE0 on [begin+index*4]
    virtual void erase_effect(std::size_t index) = 0;     // 0081B010
};

// Returns how many elements it erased.
std::size_t unit_prune_finished_effects_0081c050(UnitEffectPruneHost& host);

// ---------------------------------------------------------------------------
// The engine-sound records 0083B5E0 fills at settings+5BCh + i*24h from
// ShipGlobals["Sounds"][name], i = 0 Ship, 1 TBoat, 2 Submarine, 3 Plane
// (jump table 00842954 at 0084043C; names 00CEB79C, 00D0A780, 00CEB7B0,
// 00D0A638). 008252C0 reads only +8h of the first three: EngineSoundSmoothRate
// (key 00D0A5F8), stored at 008405D5, default 0.2f from 00CE54A0 through
// 00B66330 when the key is absent.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kEngineSoundRecordCount = 4;
inline constexpr float kEngineSoundSmoothRateDefault = 0.2f; // 00CE54A0
struct EngineSoundSmoothRates {
    float rate[kEngineSoundRecordCount]{kEngineSoundSmoothRateDefault,
        kEngineSoundSmoothRateDefault, kEngineSoundSmoothRateDefault,
        kEngineSoundSmoothRateDefault};
    bool loaded{false};
};

// The per-unit cells these routines own that UnitInstanceState does not carry.
struct UnitStep11State {
    UnitEngineAudioState audio{};
    float fade{0.0f};          // +2F4h in 00956600 (the same cell as intensity_scale)
    float fade_target{0.0f};   // +2F8h
    float steering_angle{0.0f};// +1088h
    float propeller_rates[kUnitPropellerCount]{}; // +108Ch
    float bow_wave_timer{0.0f};   // +9F8h
    float stern_wave_timer{0.0f}; // +9FCh
    float spray_timer{0.0f};      // +A10h
    float effect_latch{0.0f};     // +9D0h
    bool audio_initialised{false};
};

// What the host supplies each call.
struct UnitStep11Inputs {
    int class_id{-1};           // +C4h
    bool gate_5d{false};        // +5Dh
    float throttle{0.0f};       // +980h, the ring's published throttle
    float steering{0.0f};       // +984h
    bool alternate_inputs{false}; // +61h, no writer in .text (docs/UNIT_AUTOPILOT_PAIR.md)
    float controller_speed{0.0f}; // 0092D730 on +1018h, already evaluated by the host
    bool global_intensity_override{false}; // DAT_00F87152
};

// Step 11, in native order (00825D4D..00825D83). Each returns after running
// the image's routine against the host state described in the header.
void run_engine_audio_008252c0(UnitStep11State& state, const UnitStep11Inputs& in,
    const EngineSoundSmoothRates& rates, float scaled_delta);
void run_unit_timers_00956600(UnitInstanceState& unit, UnitStep11State& state,
    const UnitStep11Inputs& in, float scaled_delta);
void run_propellers_00834e90(UnitStep11State& state, const UnitStep11Inputs& in,
    float scaled_delta);

// Step 4 and step 6.
UnitEffectIntensityResult run_effect_intensity_00815aa0(UnitInstanceState& unit,
    UnitStep11State& state, const UnitStep11Inputs& in, float gate);
std::size_t run_prune_effects_0081c050();

} // namespace bsp
