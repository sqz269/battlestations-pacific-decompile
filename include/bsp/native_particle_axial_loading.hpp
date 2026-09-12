#pragma once
#include "bsp/native_particle_type_loading.hpp"
#include "bsp/camera_projection.hpp"

namespace bsp {
class NativeStringStorage;

// Complete B64640/B646E0 arithmetic against actual64-byte matrix storage.
// Original ECX destination, EDX float angle pointer, RET/EAX destination.
// Added stack operands borrow CURRENT native globals. Separate x87 FSIN/FCOS
// spills, SSE subtraction, stores and aliased global reloads are retained.
// These new C++ interfaces are not original binary entry points.
void* __fastcall build_native_particle_rotation_x_00b64640(void*, const float*,
    const volatile float* negative_zero_00d7a208, const volatile float* one_00d7a24c);
void* __fastcall build_native_particle_rotation_y_00b646e0(void*, const float*,
    const volatile float* negative_zero_00d7a208, const volatile float* one_00d7a24c);

// Current-global overload of the established GUI rotation provider. Retains
// native store-before-one-load alias behavior; the existing two-argument GUI
// entry remains unchanged. Complete B64780 body with borrowed global operands.
void build_gui_rotation_z_00b64780(CameraMatrix&, const float&,
    const volatile float* negative_zero_00d7a208, const volatile float* one_00d7a24c);

// Complete B05D00. Original ECX actual A4h Axial definition; RET, no result.
// Reads heading+88/elevation+84, constructs (RotZ(0)*RotX(base-scale*elevation))
// *RotY(scale*heading), then transforms (0,current-one,0) into +94/+98/+9C.
// Uses shared matrix/affine providers, not a geometry or scene abstraction.
void refresh_native_axial_particle_axis_00b05d00(void*, NativeParticleTypeLoadingBindings&);

// Complete B062F0. Original ECX definition, stack nonnull C-string, RET4.
// Allocates the actual8h temporary NativeString in the supplied string domain;
// Center/Bottom/Top/Left/Right map to 0/1/2/3/4 at+A0, unmatched maps to0.
void set_native_axial_particle_alignment_00b062f0(void*, const char*, NativeStringStorage&);

// Complete B064A0. Original ECX actual A4h definition, stack TextBuffer,
// RET4/AL=true, including EOF before opening/closing brace. Sparse publication:
// FollowDirection+80 byte, elevation+84, heading+88, Length+8C, Width+90,
// axis+94..9C and alignment+A0. Common properties use the existing providers.
// Replaced curve pointers are not released by native; parse status is ignored.
// initial_builder_kind_0c supplies incoming native stack residue; that word is
// reused across lines instead of inventing a kind for rejected curve syntax.
// Same actual parameter/string domains, no owner overlays or fallback parser.
// Host exception unwind follows owned temporaries; original FH3 ABI unclaimed.
bool load_native_axial_particle_definition_00b064a0(void*, void*, NativeParticleTypeLoadingBindings&);
} // namespace bsp
