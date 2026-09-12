#pragma once

#include "bsp/registered_model_effect.hpp"

namespace bsp {

// Required real application calls. These borrow actual objects; no companion
// count, copied model, successful update, or successful cleanup is supplied.
class RegisteredModelEffectBehaviorCallees {
public:
    virtual ~RegisteredModelEffectBehaviorCallees() = default;
    // Complete AF6DD0 is required: ECX actual model; stack(float delta, byte
    // reference+198==3); RET8. Includes transforms, initialization, random values,
    // x87 fixed steps, emitters, geometry and bounds. No part is skipped here.
    virtual void call_00af6dd0(NativeNodeStorage& actual_model, float delta,
        std::uint8_t reference_mode_is_3) = 0;
    // ECX actual28h emitter; RET, EAX actual30h container. Read body: lazily
    // allocates/B053D0-constructs if emitter10 is null, then reloads emitter10.
    // AF6BE0 calls only after a nonnull test, but preserves the actual call.
    virtual void* call_00aff690(void* actual_emitter) = 0;
    // ECX actual6Ch state; stack full DWORD; RET4, AL result. Read body:
    // current definition64 virtual1C(state, argument), then conditional actual
    // point-light unlink/node release under72B740 lock. Other native callers
    // pass1; B05070 passes0 and IGNORES the returned byte before removing row.
    virtual std::uint8_t call_00b04f00(void* actual_state,
        std::uint32_t argument) = 0;
};

// Complete AF6BE0..AF6C46 through existing0051F6B0 and required AFF690.
// ECX actual2DCh model; RET, AL bool. Reads its real tail and emitter storage.
std::uint8_t complete_particle_model_00af6be0(NativeNodeStorage&,
    RegisteredModelEffectCallees&, RegisteredModelEffectBehaviorCallees&);
// Complete872010..872017, D0DE18 virtual08: load model1C then AF6BE0.
std::uint8_t complete_registered_model_effect_00872010(RegisteredModelEffectStorage&,
    RegisteredModelEffectCallees&, RegisteredModelEffectBehaviorCallees&);

// Complete872740..872761, D0DE18 virtual28. ECX event; stack(delta, actual
// reference node); RET8. The reference must cover+198; it is NOT the114h point.
// Preserve FLD before the mode comparison and FSTP before required AF6DD0.
void update_registered_model_effect_00872740(RegisteredModelEffectStorage&, float delta,
    const void* actual_reference_node, RegisteredModelEffectBehaviorCallees&);

// Complete B05070..B05100 through required B04F00. ECX actual container, RET.
// Signed live-count loop; definition+28 filter; captured state backing per row;
// reload row backing/count after cleanup; swap only WORD id and float+4, with
// native x87 copy in the forward direction. Recheck the replacement row.
void stop_particle_emitter_states_00b05070(void* actual_container,
    RegisteredModelEffectBehaviorCallees&);
// Complete AFF570..AFF57C: null emitter10 returns; otherwise B05070.
void stop_particle_emitter_00aff570(void* actual_emitter,
    RegisteredModelEffectBehaviorCallees&);
// Complete AF5F20..AF5F50: capture model194 before clearing1A4, then capture
// count198/end once; visit every raw emitter pointer, including null preconditions.
void stop_particle_model_00af5f20(NativeNodeStorage&,
    RegisteredModelEffectBehaviorCallees&);
// Complete871FE0..871FE7, D0DE18 virtual30: load model1C then AF5F20.
// Does not clear event.active_0c: the established PointEffectChildEvents
// dispatcher captures virtual30, clears that byte, then invokes this body.
// No retain/release, detach, model destruction, or reference-count change.
void deactivate_registered_model_effect_00871fe0(RegisteredModelEffectStorage&,
    RegisteredModelEffectBehaviorCallees&);

// These new C++ interfaces require valid actual storage/lifetimes and caller
// synchronization, as native code does. They do not emulate access violations,
// original vtables/ABI/SEH or implement the separate AF74A0 model constructor.
} // namespace bsp
