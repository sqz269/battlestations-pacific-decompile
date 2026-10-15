#pragma once

#include "bsp/scene_contents_hosts.hpp"
#include "bsp/system_camera_axes.hpp"

#include <array>
#include <cstdint>

namespace bsp {

// Required native sampling boundaries. Both callee bodies/listings were read.
// TerrainGridLayerRecord is the existing semantic .nav/producer record, not a
// native object overlay. The normal routine reads its existing cell_size field.
struct AvoidZoneLayerNormalHost {
    virtual ~AvoidZoneLayerNormalHost() = default;

    // 00417FA0: x87 (world+half_extent)/cell_size, float stores. For each c:
    // ordered 0>c selects +0; else ordered c>float(dimension-2) selects that
    // upper value; otherwise keep c, including equality and unordered values.
    // Existing game-host double arithmetic is not proved kernel-identical.
    // Original ECX layer, stack out/x/z, EAX out, RET 0Ch.
    virtual void world_to_cell_00417fa0(const TerrainGridLayerRecord& layer,
        std::array<float, 2>& out, float world_x, float world_z) = 0;

    // 0041BAE0: clamp indices to [0,dimension-1], use native 32-bit row index,
    // bounds-check the byte span, return byte*file_scalar rounded to float.
    // The invalid span path calls 00BF6713; a silent zero substitute is not
    // an exact binding. Original ECX layer, stack x/z, ST0 float, RET 8.
    virtual float cell_height_0041bae0(const TerrainGridLayerRecord& layer,
        std::int32_t x, std::int32_t z) = 0;
};

// Complete control-flow semantic projection of 0041BD10-0041BE81 through the
// required sampling bindings. Names are hypotheses. Native ECX layer; stack
// out_float3/point_float2; EAX out; RET 8. This is a new C++ API, not that ABI.
//
// Samples (x0,z0), (x1,z0), (x0,z1), (x1,z1), sums the two stored triangle
// gradients, then normalizes using the existing x87 vector-length kernel.
// Its finite sqrt core is reconstructed; native exceptional diagnostics still
// require the supplied actual CRT state/handler and are not validated here.
// x0/z0 use CVTTSS2SI; x1/z1 convert extended (cell+1.0) through the existing
// mode-dependent BF7420 entry. They are not computed by integer increment.
// A nonpositive OR unordered length selects +0 reciprocal, followed by all
// three multiplications (so exceptional inputs need not produce a zero vector).
//
// Keep the layer/sampling data stable and all borrowed CRT pointers live. Supply
// the actual mutable 0109EEA4 address; it is read at EACH neighboring conversion.
// Point, output and layer storage must not overlap in this typed projection.
// Before the fourth sample the output holds the first gradient; before length
// it holds the summed gradient. Hosts must not modify the point or output.
// Evidence and validation: docs/OCT15_LAYER_NORMAL.md and corresponding report.
std::array<float, 3>& avoid_zone_layer_normal_0041bd10(
    AvoidZoneLayerNormalHost& host, const TerrainGridLayerRecord& layer,
    std::array<float, 3>& out, const std::array<float, 2>& point,
    const volatile std::uint32_t* actual_0109eea4, const CameraAxesCrtAccess& crt);

} // namespace bsp
