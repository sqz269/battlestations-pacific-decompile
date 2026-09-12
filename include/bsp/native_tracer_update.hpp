#pragma once
#include "bsp/registered_type4_effect_behavior.hpp"
#include "bsp/system_camera_axes.hpp"
#include <cstdint>

namespace bsp {
// A captured current virtual34 binding over the actual generated-model-derived
// owner. Its native identity DWORD is never dereferenced as a C++ vtable.
struct NativeTracerTransformTarget {
    void* context;
    void (__fastcall* invoke)(void* context, void* actual_owner, const CameraMatrix&);
};
class NativeTracerTransformAccess {
public:
    virtual ~NativeTracerTransformAccess() = default;
    // Pure, nonallocating canonical lookup at BAAEBA, matching the current-slot
    // capture. Must return a valid target; must not mutate the owner, floating
    // environment or application state. No successful fallback is supplied.
    virtual NativeTracerTransformTarget capture_current_34(
        void* actual_owner, std::uint32_t captured_native_table) noexcept = 0;
};
struct NativeTracerUpdateBindings {
    const CameraAxesCrtAccess& crt;
    NativeTracerTransformAccess& transforms;
};

// Complete BAA510 predicate; ECX actual tracer, native AL result, RET.
// Uses the existing actual CRT binding for the linked-point sqrt/fade path.
std::uint8_t predicate_native_tracer_00baa510(RegisteredType4TracerView,
    const CameraAxesCrtAccess&);

// Complete BAABB0 update over already constructed actual7ACh storage and its
// actual mesh, material and ring records. Original six stack words / RET18:
// matrix, age, position, direction, scaled speed, subject58. Matrix and the
// final word are marshalled but unread. Adds explicit current-slot/CRT bindings.
// Constructor, geometry production and application ownership remain separate.
void update_native_tracer_00baabb0(RegisteredType4TracerView,
    const CameraMatrix& matrix, float age, const float* position,
    const std::array<float,3>& direction, float scaled_speed, float subject58,
    NativeTracerUpdateBindings);

// Complete BAA670 ring insertion on native-valid existing storage. No allocation,
// ownership change or substitute linked-point container is introduced.
void append_native_tracer_point_00baa670(RegisteredType4TracerView,
    const float* position, const float* direction, const CameraAxesCrtAccess&);
// Complete BA9960: current material1BC then optional child254 geometry element0
// material; copy alpha word without setting the material lighting dirty flag.
void set_native_tracer_alpha_00ba9960(RegisteredType4TracerView, float alpha) noexcept;
// Complete B74390: four forward x87 stores at model08..14 then flags138 mask.
void set_native_generated_model_bounds_00b74390(void* actual_model,
    const float* four_words) noexcept;
}
