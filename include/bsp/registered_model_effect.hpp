#pragma once

#include "bsp/live_effect_event_registry.hpp"
#include "bsp/native_node_parenting.hpp"
#include "bsp/point_effect_instance.hpp"
#include "bsp/random_threads.hpp"

namespace bsp {

// Actual20h event, sharing the canonical1Ch event prefix. The model pointer is
// the actual2DCh particle-model slot, never a host companion. Padding retains
// its allocation preimage. Definition14 and point10 are borrowed, without retain.
struct RegisteredModelEffectStorage {
    NativeRegisteredEffectPrefixStorage prefix;
    NativeNodeStorage* model_1c;
};

// Required application callees; no successful placeholder implementation exists.
// AF74A0 is a derived particle-model constructor, not the188h generated-model
// constructor. Its geometry/material/emitter bodies remain outside this packet.
class RegisteredModelEffectCallees {
public:
    virtual ~RegisteredModelEffectCallees() = default;
    // Perform the actual singleton getter, then return its live8h owner. The
    // factory reads byte04 AFTER this call; a cached suppression boolean is wrong.
    virtual const volatile void* call_0051f6b0() = 0;
    // Actual F8D2D0 pool,2E0h stride: return raw2DCh object storage with its
    // trailing pool ID intact. Native incoming ECX=2DC is overwritten by thunk.
    virtual void* allocate_model_slot_00af6b60() = 0;
    // ECX slot, stack selected variant, RET4; return SAME actual slot. Complete
    // the native constructor/member unwind before returning/throwing. Publish
    // its stable NativeNodeBinding in parenting.nodes.scenes before return.
    // The caller must provide live CameraMatrix storage at actual slot+298.
    virtual NativeNodeStorage* construct_model_00af74a0(void* actual_slot,
        void* actual_selected_variant) = 0;
    // Only the raw allocation unwind after AF74A0 throws. Do not destroy the
    // model again: AF74A0 already unwinds members. Same F8D2D0 pool, no free().
    virtual void return_model_slot_00af62f0(void* actual_slot) noexcept = 0;
};

struct RegisteredModelEffectRuntime {
    LiveEffectEventRegistryBindings& registry;
    RandomThreads& random;
    NativeNodeParentingRuntime& parenting;
    RegisteredModelEffectCallees& callees;
    // Borrow actual immutable D5DA50 table words through+38. Used to validate
    // the current virtual38 before executing the recovered B6DB10 implementation.
    const volatile std::uint32_t* actual_model_table_00d5da50;
};

// Complete8742A0..87442E ordinary body and its two native unwind states, through
// required real callees above. ECX event; stack(actual34h Particle definition,
// actual point instance); EAX same event; RET8. The producer is86BC80/871D00.
// Requires nonempty valid variant storage and a successful nonnull model for
// normal return. A null model/native invalid-pointer path is rejected, not SEH
// emulated. No retain of the model or point and no model cleanup on later throw.
RegisteredModelEffectStorage& construct_registered_model_effect_008742a0(
    RegisteredModelEffectStorage&, void* actual_definition,
    PointEffectInstanceStorage&, RegisteredModelEffectRuntime&);

// ECX definition, stack actual point, RET4. Reads the current0051F6B0 owner+04;
// suppressed returns null before allocation. Otherwise actual20h allocation,
// constructor and allocation unwind. No executable application binding is
// claimed until all required particle-model callees are implemented and tested.
RegisteredModelEffectStorage* create_registered_model_effect_0086b1c0(
    void* actual_definition, PointEffectInstanceStorage&, RegisteredModelEffectRuntime&);

// Complete874430..87449A, ECX event, RET. Unregister then base tables only.
// Does NOT stop, release, detach, clear or destroy model1C, or change ref04.
void destroy_registered_model_effect_00874430(RegisteredModelEffectStorage&,
    LiveEffectEventRegistryBindings&);
// Complete8745F0..87460D, ECX event, stack flags, RET4. Bit0 returns event
// allocation only, after destructor. Without bit0 typed storage stays live for
// native byte inspection/reuse. No native ABI/vtable/SEH compatibility claim.
RegisteredModelEffectStorage* delete_registered_model_effect_008745f0(
    RegisteredModelEffectStorage&, std::uint32_t flags, LiveEffectEventRegistryBindings&);

} // namespace bsp
