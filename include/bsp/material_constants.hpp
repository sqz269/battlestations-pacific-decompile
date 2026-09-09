#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {
// New semantic projections of parameter +8h/+Ch, +10h, +14h and +4Ch.
// Nonmatrices copy every source word, including a partial float4. Matrices
// require at least 16 words when selected; additional words are ignored.
struct MaterialConstantParameter {
    std::vector<std::uint32_t> source_words;
    bool matrix{};
    std::vector<std::int32_t> vertex_registers;
    std::vector<std::int32_t> pixel_registers;
};

// Native VS metadata record +0h/+4h, stride20h. Last matching record wins.
struct VertexConstantShape {
    std::int32_t start_register{};
    std::int32_t row_count{};
};

enum class MaterialConstantPackStatus {
    complete,
    selector_out_of_range,
    source_too_short,
    destination_too_short,
    shared_output_buffer
};

// Fragment 00b423c5..00b42693 of 00b42350, NOT an original function/ABI.
// Original parent: ECX pass, entry and unused override stack args, RET8.
// Selector is entry+10h -> +198h. Output vector sizes are word capacities;
// this function never resizes or clears them. Negative stage indices skip.
// For matrices, dst[4*r+c] = src[4*c+r]: VS writes only rows2/3/4 from
// shape lookup, while PS writes four rows regardless of those shapes.
//
// Bounds/selector errors are NEW interface statuses, not native error paths.
// All checks precede writes, so failure leaves both outputs untouched. The
// buffers must be distinct vector objects. Source words copy bit-for-bit;
// native x87 exceptional-value conversion/FP flags are not reproduced for
// matrices. Ordinary finite matrix values match the inspected packing.
// Evidence: docs/MATERIAL_CONSTANTS.md, reports/material_constants_evidence.json.
MaterialConstantPackStatus pack_material_parameter_constants_00b423c5(
    const std::vector<MaterialConstantParameter>& parameters,
    std::size_t selector,
    const std::vector<VertexConstantShape>& vertex_shapes,
    std::vector<float>& vertex_words,
    std::vector<float>& pixel_words);
}
