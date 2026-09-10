#pragma once
#include <string>

namespace bsp {
struct CompiledMaterialPass;
struct InstanceRenderEntry;
struct MaterialEntryConstantState;
class MaterialLighting;

// Native00B42350 byte range[00B42E4A,00B42EF9): visibility VS then PS,
// then cLODValue PS. Uses each live stage's bindings and actual shared words.
// actual_stream_threshold is required only for selected PS semantic44. It must
// point to the current threshold resolved from entry+8: index at owner+50,
// owner+4+16*index when nonzero, native0.995 only when the actual index is zero.
// It has no fallback here; resolve the native owner before calling this stage.
bool write_material_visibility_lod_00b42e4a(const CompiledMaterialPass&,
    const InstanceRenderEntry&, const float* actual_stream_threshold,
    MaterialEntryConstantState&, std::string& error);

// Native range[00B4305D,00B430CF): diffuse PS then VS. Call AFTER the
// inverse-world fragment[00B42EF9,00B4305D), never as part of the scalar stage.
// actual_lighting must be the current entry.section+20 material's lighting;
// it is required only when a stage selects semantic47. The native getter is
// evaluated separately at each selected stage, copying DWORDs in source order.
bool write_material_diffuse_00b4305d(const CompiledMaterialPass&,
    const MaterialLighting* actual_lighting, MaterialEntryConstantState&,
    std::string& error);

// Both are bounded semantic APIs, not the original builder ABI. FF skips a
// stage; register counts are not gates. Missing required input/short storage
// returns false without undoing earlier writes. No buffer resize or clearing;
// overlapping selected registers retain native ordered last-writer behavior.
}
