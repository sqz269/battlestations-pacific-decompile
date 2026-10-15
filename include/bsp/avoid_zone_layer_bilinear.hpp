#pragma once

#include "bsp/avoid_zone_layer_sampling.hpp"

namespace bsp {

// Complete instruction-backed 0041BC20-0041BD02 through the accepted raw
// WorldToCell/CellHeight kernels. New Source ABI: stdcall(layer,x,z), RET0Ch,
// ST0 float. Original: ECX layer, stack x/z, RET8, ST0 float. This API is not
// a native member/ABI replacement. Names are hypotheses.
//
// Hardware CVTTSS2SI forms both base indices; LEA +1 wraps each DWORD. Samples
// h11,h01,h10,h00 in that order. Retains float stores of u fraction, heights,
// both row blends, v fraction and final result; other intermediates stay x87.
// No double/C++ float-to-int replacement or extra clamp/finite-value guard.
//
// Supply live mutable Cc13TerrainGridLayerNative producer-shaped backing. The
// accepted CellHeight kernel's qualified current-CRT invalid-parameter service
// may fail or return; its backing reload, cached index, late scale and fault
// obligations remain. This raw entry does not impose the semantic adapter's
// positive-dimension/valid-vector/stable-data admission restrictions.
//
// The accepted normal adapter has private backing admitted only for its two
// sampling methods. This raw entry adds no semantic-record adapter or new
// access to that private backing. Actual cell ownership and CRT state remain
// with the caller. No runtime wiring or new-function fixture proof is supplied.
// Evidence: docs/OCT15_LAYER_BILINEAR.md and reports/oct15_layer_bilinear.json.
float __stdcall layer_sample_bilinear_kernel_0041bc20(
    Cc13TerrainGridLayerNative* layer, float world_x, float world_z);

} // namespace bsp
