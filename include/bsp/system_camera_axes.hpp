#pragma once
#include "bsp/camera_frame_state.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct FrameClock;
using CameraAxis = std::array<float, 3>;

// Actual Win32 __87except record. For unary sqrt, argument2 is uninitialized
// native stack storage: the handler must not read it for operation 5.
struct CameraAxesCrtException {
    std::int32_t type;
    const char* name;
    double argument1;
    double argument2;
    double result;
};
using CameraAxes87Except = void (__cdecl *)(std::int32_t operation,
    CameraAxesCrtException* exception, std::uint16_t* saved_control_word);

// Borrowed actual CRT state/handler, not a new runtime mode or a default math
// policy. The pointed-to global is loaded at the original sqrt branch points.
// Handler projects native __87except 00C27489, including its error/SEH/matherr/
// errno effects and possible result changes. Both pointers must remain valid.
struct CameraAxesCrtAccess {
    const volatile std::uint32_t* dispatch_bypass_0109dd78;
    CameraAxes87Except except_00c27489;
};

// Shared recovered kernels. Length keeps the native ECX vector / ST0 result
// schedule and adds the actual CRT binding in EDX. Both pointers must be valid.
// Cross retains ECX destination, EDX left, stack right, RET4 and EAX destination;
// its first destination store precedes later source reads (do not assume alias
// safety). The pointed-to vectors each contain three float words.
float __fastcall camera_vector_length_00419440(const float*, const CameraAxesCrtAccess*);
float* __fastcall camera_vector_cross_004f9b30(float*, const float*, const float*);

// Native ECX=camera, no stack args, EAX=&camera+440 / +44C. New typed ABI.
// Both refresh the same CameraFrameState.axis_y/axis_x pair and the actual
// CameraState.projection.valid_flags bit100. World refresh uses the same
// CameraState.transform cache consumed by the matrix getters.
const CameraAxis& get_camera_axis_y_00b70ea0(CameraFrameState&, const CameraAxesCrtAccess&);
const CameraAxis& get_camera_axis_x_00b70fe0(CameraFrameState&, const CameraAxesCrtAccess&);

// Interior 00B46C50..00B46CB3: patches initialized c31.xyz then c32.xyz. Capacity
// is in float words and must be at least131; padding c31.w/c32.w is untouched.
// Captures the actual timer slot after loading Y.x, before storing c32.x.
// Returns that borrowed owner for the immediately following time segment.
// The slot read has no callback or service side effect. No output/cache changes
// occur for invalid output capacity. Other exceptions preserve earlier stores.
FrameClock* write_system_camera_axes_00b46c50(CameraFrameState&, float* prefix,
    std::size_t capacity, FrameClock* const volatile& global_01090ab0,
    const CameraAxesCrtAccess&);
}
