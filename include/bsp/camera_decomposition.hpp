#pragma once

#include "bsp/camera_projection.hpp"

namespace bsp {

// Native ECX=writable XYZ, RET. Captures Y/X/Z before writes; squared length
// has one float spill. Ordered <=1e-10 or unordered selects float(1e-5),
// otherwise genuine current CRT sqrt is float-spilled. Divides X/Y/Z in order.
void __fastcall normalize_camera_basis_0042b260(std::array<float, 3>&);

// Native ECX=matrix, EDX=output X, stack=output Y,output Z, RET8. Captures and
// normalizes the three basis rows before writing X, then Z, then Y. All output
// references may alias each other or any matrix float; input storage must be
// writable when overlapped. Translation/last-column elements are ignored.
// Preserves x87 instructions/spills and the caller FP environment. Uses the
// canonical staged asin and genuine current CRT atan2/sqrt, whose original
// dispatch globals/diagnostics/exception policy are not reconstructed here.
void __fastcall extract_camera_matrix_angles_0042d2e0(const CameraMatrix&,
    float& output_x, float& output_y, float& output_z);

} // namespace bsp
