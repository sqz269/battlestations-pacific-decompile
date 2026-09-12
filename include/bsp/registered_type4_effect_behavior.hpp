#pragma once

#include "bsp/registered_type4_effect.hpp"
#include "bsp/native_effect_jobs.hpp"

namespace bsp {

// Borrowed view of the actual generated-model-derived 7ACh SkinedWaterTracer
// produced by BAD6F0, including its existing node/count/material/geometry.
// This is not storage, an allocator, or a second lifetime/ownership domain.
struct RegisteredType4TracerView {
    void* actual_owner;
};

// Complete BA9820..BA984E; ECX actual tracer, RET. Store byte200=0, float7A8=1,
// then reload nullable actual+1B8 and store linked+24=0.1. No unlink/release.
void deactivate_registered_type4_tracer_00ba9820(RegisteredType4TracerView) noexcept;

// Complete direct setter bodies. All consume one stack word/RET4 except
// BA9A90, which stores TWO float words in order and returns with RET8.
void set_registered_type4_tracer_reciprocal_00ba9900(RegisteredType4TracerView, float) noexcept;
void set_registered_type4_tracer_texture_rate_00ba9920(RegisteredType4TracerView, float) noexcept;
void set_registered_type4_tracer_alpha_scale_00ba9940(RegisteredType4TracerView, float) noexcept;
void set_registered_type4_tracer_width_scale_00ba99c0(RegisteredType4TracerView, float) noexcept;
void set_registered_type4_tracer_width_scaler_00ba99e0(RegisteredType4TracerView, float) noexcept;
void set_registered_type4_tracer_width_offset_00ba9a00(RegisteredType4TracerView, float) noexcept;
void set_registered_type4_tracer_bone_length_00ba9a20(RegisteredType4TracerView, float) noexcept;
void set_registered_type4_tracer_height_scale_00ba9a40(RegisteredType4TracerView, float) noexcept;
void set_registered_type4_tracer_local_space_00ba9a60(RegisteredType4TracerView, std::uint8_t) noexcept;
void set_registered_type4_tracer_fade_in_00ba9a70(RegisteredType4TracerView, float) noexcept;
void set_registered_type4_tracer_width_endpoints_00ba9a90(RegisteredType4TracerView,
    float at_zero, float at_one) noexcept;

// REQUIRED real tracer and curve bindings; none has a successful fallback.
// Constructor must build the actual generated-model-derived owner and install
// its existing canonical NativeNodeBinding / GeneratedModelLifetimeRuntime and
// NativeRenderActualOwners associations, including current terminal virtuals.
class RegisteredType4TracerRuntime {
public:
    virtual ~RegisteredType4TracerRuntime() = default;
    // BAC660 overwrites ECX with actual pool0109049C; its incoming 7AC is not
    // a size parameter. Physical slots have 7B0 stride and pool ID at+7AC.
    virtual void* allocate_00bac660() = 0;
    // Native ECX raw slot, stack five actual borrowed arguments, RET14.
    // On throw, perform the real constructor's member unwind before returning.
    virtual void* construct_00bad6f0(void* actual_slot, void* actual_19f0_a8,
        void* actual_19ec, void* texture_98, void* texture_9c, void* definition_a4) = 0;
    // Constructor-allocation unwind only, actual pool slot; not owner teardown.
    virtual void return_slot_00bac2b0(void* actual_slot) noexcept = 0;
    // Current captured curve object's virtual08, one float word/RET4; x87
    // return rounded to float at each call site. Reload its CURRENT slot each
    // time; BACAA0's D63FFC slot08=BA9DA0 is a cached piecewise linear curve.
    virtual float sample_current_curve_08(void* actual_curve, float argument) = 0;
    // Complete native BAABB0 remains required. Six stack words/RET18, even
    // though current body does not read matrix argument1 or final argument6.
    virtual void update_00baabb0(RegisteredType4TracerView, const CameraMatrix&,
        float age, const float* first_world_translation,
        const std::array<float, 3>& captured_vector_68, float scaled_speed,
        float captured_subject_58) = 0;
    // BAA510 reads live tracer flags/link/fade clock; AL result. It is not a
    // simple active-bit test and must execute the real predicate.
    virtual std::uint8_t predicate_00baa510(RegisteredType4TracerView) = 0;
};

struct RegisteredType4EffectBehaviorBindings {
    // Same actual F87684 OS critical section and actual F8769C depth, borrowed
    // through the existing projection. No allocation or substitute mutex.
    SystemSingletonCriticalSection& actual_lock_00f87684;
    void* volatile& actual_game_00e188a8;
    RegisteredType4TracerRuntime& tracers;
    NativeEffectJobEvents& events;
    PointEffectChildEvents& children;
};

// Complete872020..87205B; ECX event/RET/AL completion. Pending1C blocks
// completion; timer28 and actual tracer201 are read without retaining anything.
std::uint8_t complete_registered_type4_effect_00872020(
    const NativeRegisteredType4EffectStorage&) noexcept;

// Complete872060..872073; ECX event/RET. Clear1C, reload current34 and tail
// deactivate if present. Does not clear active0C or change the sole prefix04.
void deactivate_registered_type4_effect_00872060(NativeRegisteredType4EffectStorage&) noexcept;

// Complete872790..872BC0 control/state behavior, ECX event, stack(delta,
// reference), RET8. Reference is not read. Borrowed subject is the existing
// PointEffectInstanceStorage, including its canonical current node110.
// The constructor, raw pool and tracer update/predicate remain required real
// bindings. On constructor throw, native state0 returns raw slot but DOES NOT
// decrement F8769C or leave F87684. Preserve that partial state.
void update_registered_type4_effect_00872790(NativeRegisteredType4EffectStorage&,
    float delta, void* actual_reference, RegisteredType4EffectBehaviorBindings);

// New C++ interfaces: not original MSVC object/vtable/SEH ABI replacements.
// Family dispatch/composition and game validation are outside these entries.
} // namespace bsp
