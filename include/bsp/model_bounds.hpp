#pragma once
#include "bsp/camera_transform.hpp"

namespace bsp {
// Separate bounds storage for the same model that owns CameraTransform. This is
// a new C++ interface, not the native object layout or its constructor. The
// caller supplies the model's actual local sphere and keeps both objects paired.
struct ModelBounds {
    std::array<float, 4> local_sphere{}; // native model+08: center XYZ, signed radius
    std::array<float, 4> world_sphere{}; // native model+13C
};

// Native ECX=matrix; ST0=float result; RET. Uses rows 0/1/2, ignoring their
// fourth elements and translation. Retains x87 arithmetic, float stores and
// carry-based selection (including unordered comparisons), not a spectral norm.
float maximum_basis_length_squared_007bb620(const CameraMatrix& matrix);

// Native ECX=source sphere; stack=destination,matrix; EAX=destination; RET8.
// Reuses the recovered affine point kernel. Radius is sqrt(max squared basis)
// stored float, multiplied by the original signed radius, then stored float.
// Numerical parity requires finite inputs/intermediates and x87 CW=0x007F or
// 0x027F (24/53-bit precision, round-to-nearest, masked exceptions). The first
// mode is used after D3D9 device creation. Its CRT masked-inexact path preserves
// the result and control word; it does not call matherr or change errno. NaN,
// domain diagnostics, other control words and full status-word parity are open.
// Source/result may alias; matrix/result overlap is unsupported. No validation
// or cache invalidation is added by this leaf.
void transform_sphere_007c1180(const std::array<float, 4>& source,
    const CameraMatrix& matrix, std::array<float, 4>& result);

// Native ECX=model; EAX=model+13C; RET. Either auxiliary bit 0x10 or 0x20
// accepts the cached sphere, even if world-valid bit2 is clear. On a miss,
// refresh world if needed, transform local bounds, copy the four float values,
// then OR both auxiliary bits. Local-sphere producers must clear both bits when
// bounds change; recovered transform edits already invalidate these shared bits.
const std::array<float, 4>& get_model_world_sphere_00b6e8c0(
    CameraTransform& transform, ModelBounds& bounds);
}
