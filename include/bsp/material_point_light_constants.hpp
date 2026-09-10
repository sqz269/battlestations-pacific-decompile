#pragma once
#include <string>
#include <vector>

namespace bsp {
struct CompiledMaterialPass;
struct GeneratedModelPointLightLinks;
class GeneratedModelLifetime;

// Native ECX model, EAX model+164 borrowed array header, RET. Returns the SAME
// live ordered light list used by lifetime unlink and building-instance writes.
const std::vector<GeneratedModelPointLightLinks*>& borrowed_point_lights_00b6dc50(
    const GeneratedModelLifetime&) noexcept;

// Fragment [00B43160,00B43403) of00B42350, not an original callable ABI.
// Resolve the actual model/list at this stage, after preceding builder calls.
// Current pass.vb.registers[45] gates the entire branch. Writes (min(count,4),
// +0,+0,+0), then reloads vb.registers[46] and writes that many interleaved
// position_radius/color float4 pairs. pb/counts/end_register are not consulted.
// No resize, tail clear, W replacement, or private output staging. Later data
// writes win over an overlapping count register; all source bits are retained.
// Each position float4 is captured before its stores; color words are read and
// written one at a time, then the next actual light pointer is fetched.
//
// Host domain: count0..INT_MAX; selected light pointers are valid borrowed
// owners. Metadata/list headers and source objects retain valid C++ storage.
// Bounds/null errors are new checked-interface failures, not native branches.
// Count-range/count-size failure writes nothing; data-range failure retains the
// count write; null-light failure retains the count and earlier light writes.
// A missing count register succeeds without examining count, lights or output.
// A missing data register/zero count does not validate or write the data range.
// Evidence: docs/MATERIAL_POINT_LIGHT_CONSTANTS.md.
bool write_material_point_light_constants_00b43160_fragment(
    const CompiledMaterialPass&,
    const std::vector<GeneratedModelPointLightLinks*>& actual_model_point_lights,
    std::vector<float>& vertex_words, std::string& error);
}
