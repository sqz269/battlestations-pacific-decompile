#pragma once
#include "bsp/camera_transform.hpp"
#include <string>
#include <vector>

namespace bsp {
struct CompiledMaterialPass;

// Native ECX transform, RET, EAX transform+60. Same cache and affine inverse
// schedule as00B6FCB0; returns the actual shared CameraTransform::view identity.
// This C++ interface is not the native object layout/calling convention.
const CameraMatrix& get_transform_inverse_world_00b6e0d0(CameraTransform&);

//00B42A7C..00B42E49 of00B42350. Writes VS then PS to existing shared words.
// BOTH stages use pass.vb.counts[0], even if VS register[0] is FF. Each enabled
// stage refreshes an invalid world before testing whether rows are2/3/4.
// Rows2 snapshot eight raw transposed words; rows3/4 copy sequentially via x87.
// No clear/resize or full-builder snapshot. Same-vector outputs are permitted:
// later PS writes win. Transform/metadata storage must not overlap outputs.
// Bounds failures are new host errors checked at the affected write; earlier
// writes and cache changes remain. No owner lookup or missing-owner defaults.
bool pack_material_world_constants_00b42a7c(const CompiledMaterialPass&,
    CameraTransform&, std::vector<float>& vertex_words,
    std::vector<float>& pixel_words, std::string& error);

//00B42EF9..00B4305C, AFTER visibility/LOD and BEFORE diffuse-color writes.
// VS only, register/count index1. Enabled unsupported rows still resolve the
// inverse cache. Uses the same copy/validation rules as the world fragment.
bool pack_material_inverse_world_constants_00b42ef9(const CompiledMaterialPass&,
    CameraTransform&, std::vector<float>& vertex_words, std::string& error);
}
