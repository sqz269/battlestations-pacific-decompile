#pragma once

#include "bsp/avoid_zone_layer_normal.hpp"
#include "bsp/cc13_avoid1.hpp"

#include <optional>

namespace bsp {

// Complete instruction-backed 00417FA0-0041803E, using the existing producer
// layout Cc13TerrainGridLayerNative. New Source ABI: ECX layer, EDX out, stack
// world_x/world_z, RET8, EAX out. Original: ECX layer, stack out/x/z, RET0Ch.
// Keeps x87 transforms, float stores, retained double2 and comparison branches.
// Native equality/unordered clamp inputs survive. No dimension/FP repair.
float* __fastcall layer_world_to_cell_kernel_00417fa0(
    const Cc13TerrainGridLayerNative* layer, float* out, float world_x, float world_z);

// Complete instruction-backed 0041BAE0-0041BB48. New Source ABI: ECX mutable
// layer, EDX x, stack z, RET4, ST0 float. Original stacks both indices, RET8.
// Signed clamps, DWORD-wrapped row index and unsigned span test are retained.
// On invalid span calls existing invoke_native_invalid_parameter_00bf6713;
// that entry uses current Source CRT policy and is qualified vs original CRT
// identity. If it returns, reload begin, read the byte using the OLD index,
// and load file_scalar late. No repeated check or zero/error substitution.
//
// Caller supplies readable live producer-shaped backing. A returning handler
// must leave the resulting byte address readable; it may repair the backing.
// The raw kernel does not require valid vector invariants or dimension>=2.
// Original faults and current Source handler failure/return remain possible.
float __fastcall layer_cell_height_kernel_0041bae0(
    Cc13TerrainGridLayerNative* layer, std::int32_t x, std::int32_t z);

// Concrete, nonowning normal-host adapter for valid semantic producer data.
// Admission requires dimension>=2, grid.size()==dimension^2 within DWORD span,
// and a valid live C++ vector. Cells are borrowed directly from grid.data().
// Scalar fields are copied bitwise into private native-shaped Source backing;
// ownership/proxy/vtable/capacity identity is not represented by that backing.
// It is used only by the two kernels above, never by native container services.
//
// Keep the SAME record object, scalars, vector storage, size and bytes stable
// from admission through all calls. Methods reject a different record object.
// Output must not overlap the admitted record/storage. Under these conditions
// every clamped height index is in the valid byte span, including indices from
// NaN/overflow conversions: no invalid-parameter dispatch is reachable.
// This const semantic adapter does not model handler-driven backing repair;
// use the mutable raw kernel boundary for that behavior. No cells are copied.
class AvoidZoneLayerSamplingHost final : public AvoidZoneLayerNormalHost {
public:
    static std::optional<AvoidZoneLayerSamplingHost> admit(
        const TerrainGridLayerRecord& layer) noexcept;

    const TerrainGridLayerRecord& admitted_layer() const noexcept { return *record_; }
    void world_to_cell_00417fa0(const TerrainGridLayerRecord& layer,
        std::array<float, 2>& out, float world_x, float world_z) override;
    float cell_height_0041bae0(const TerrainGridLayerRecord& layer,
        std::int32_t x, std::int32_t z) override;

private:
    explicit AvoidZoneLayerSamplingHost(const TerrainGridLayerRecord& layer) noexcept;
    const TerrainGridLayerRecord* record_;
    Cc13TerrainGridLayerNative backing_{};
};

// Names are hypotheses. These Source APIs are not native ABI replacements or
// production-member bindings. Evidence: docs/OCT15_LAYER_SAMPLING.md and report.

} // namespace bsp
