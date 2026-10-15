#pragma once

#include "bsp/cc13_avoid1.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {

// Complete instruction-backed 0041BD10-0041BE81 over actual mutable producer
// backing. Original ECX layer, stack out/point, EAX out, RET8. New Source ABI:
// stdcall(out,point,layer,actual_mode,crt_access), EAX out, RET14h. The first two
// stack words retain native scratch roles; no native ABI/member proof follows.
//
// WorldToCell and four heights use accepted concrete raw kernels. Neighboring
// indices consume extended (cell+double1.0) through the existing BF7420 entry,
// whose actual mutable0109EEA4 address is read at each conversion. Vector length
// uses camera_vector_length_00419440 with the supplied borrowed CRT access.
// Its finite sqrt core is reconstructed; exceptional native diagnostics still
// depend on the actual state/handler. Current Source invalid-parameter policy
// behind CellHeight remains qualified versus original CRT identity.
//
// Point, output, layer fields and borrowed cell bytes MAY overlap. The kernel
// retains native reads/stores: point z then x before any output; first spacing
// read before first-gradient stores; first gradient before the fourth sample;
// second spacing read afterwards; sum written before length; final scaling uses
// saved sums even if a CRT handler changed output. No snapshot/admission guard
// restricts dimensions, vector shape, numeric values or native data aliases.
//
// Every touched address must be readable/writable when the original schedule
// reaches it, including returning-handler reload/read obligations. Source-owned
// call-frame/control slots and CRT access/provider backing must remain valid;
// their identity is not the original binary's frame/global/provider identity.
// Actual pointed-to CRT mode/state remains mutable at the existing load sites.
// This function creates no copied layer/grid or replacement service callback.
//
// Nonpositive OR unordered length selects +0 reciprocal, then all component
// multiplications still run. No early zero-vector return or extra repair.
// Semantic names are hypotheses. Existing typed normal/host and evidence remain
// separate. Evidence: docs/OCT15_RAW_LAYER_NORMAL.md and corresponding report.
float* __stdcall layer_normal_raw_kernel_0041bd10(float* out, const float* point,
    Cc13TerrainGridLayerNative* layer, const volatile std::uint32_t* actual_0109eea4,
    const CameraAxesCrtAccess* crt_access);

} // namespace bsp
