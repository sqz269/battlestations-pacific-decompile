#pragma once

#include "bsp/camera_frame_state.hpp"

namespace bsp {

// Borrow the live native constant word; MOVSS preserves its exact bit pattern.
// Both original entries use ECX for destination, return it in EAX, and RET 0.
// These typed interfaces add the explicit constant binding and are not ABI shims.
CameraPlaneSet* initialize_camera_plane_records_00b652d0(CameraPlaneSet& destination,
    const volatile std::uint32_t& live_one_00d7a24c) noexcept;

// Calls the record initializer, then reloads the same live constant, sets count
// to six, extracts planes from its diagonal matrix, and assigns flags seven.
CameraPlaneSet* construct_camera_plane_set_00b659d0(CameraPlaneSet& destination,
    const volatile std::uint32_t& live_one_00d7a24c);

} // namespace bsp
