#pragma once
#include "bsp/system_camera_axes.hpp"

namespace bsp {
// Native 00B63F10: ECX destination, EDX eye, stack target pointer followed by
// three world-up float words, RET10h, EAX destination. This is a new typed ABI.
// Copies the up words without extra FP conversions, then preserves the native
// x87/SSE operations, spill boundaries, near-parallel branches and store order.
// Uses the shared length/cross kernels and the caller's actual CRT access;
// missing CRT bindings throw before reading vectors or changing destination.
// No geometric input validation or substitute direction/up policy is added.
CameraMatrix& build_camera_look_at_00b63f10(CameraMatrix& destination,
    const CameraAxis& eye, const CameraAxis& target, const CameraAxis& world_up,
    const CameraAxesCrtAccess& crt);
// Native-owner variant: reload actual D7A24C at B64232, after the builder's
// length/CRT callbacks. The legacy overload uses its installed constant bits.
CameraMatrix& build_camera_look_at_00b63f10(CameraMatrix& destination,
    const CameraAxis& eye, const CameraAxis& target, const CameraAxis& world_up,
    const CameraAxesCrtAccess& crt, const volatile std::uint32_t& one_00d7a24c);
}
