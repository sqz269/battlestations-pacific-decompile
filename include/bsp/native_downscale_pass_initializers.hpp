#pragma once
#include "bsp/native_render_pass_initializers.hpp"

namespace bsp {
struct NativeDownscale2x2OffsetConstants {
    const volatile float& unsigned_bias_00ce3978;
    const volatile double& one_and_half_00ce3d78;
    const volatile double& negative_half_00cec9e0;
};
struct NativeDownscalePassInitializationContext {
    NativeRenderPassInitializationContext& common;
    void* const volatile& actual_configuration_0109cf04;
    NativeDownscale2x2OffsetConstants offsets;
    const char* downscale4x4_name_00d620e8;
    const char* downscale2x2_name_00d620fc;
    const char* inverse_size_name_00d620d8;
};

// Complete B4CD30[174]. Original ECX unused, unsigned width/height and output
// stacked, RET0Ch. Four float4 records, only X/Y written: (-1.5/w,-1.5/h),
// (-0.5/w,-1.5/h), (-1.5/w,-0.5/h), (-0.5/w,-0.5/h). Preserve Z/W preimages.
// Original FILD/FADD-float unsigned conversion, x87 stack/spills/reloads and
// single-precision stores are retained, including caller precision/rounding.
// New context supplies the actual readonly constants; no native ABI claim.
void write_native_downscale2x2_offsets_00b4cd30(std::uint32_t width,
    std::uint32_t height,void* actual_four_float4_records,
    const NativeDownscale2x2OffsetConstants&) noexcept;

// Prepare block with context.common before entry and preserve it through all
// native/host survivors, exactly as for R77. The same five-state caller cleanup
// is reused; no prior+08/+0C releases, whole-owner clearing or fallback objects.
// Existing220h/90h receiver profile/count must already be initialized.
// Both native entries take ECX owner and four stack words, RET10h.
// Full B544F0[485]: register borrowed float4 at+210 BEFORE filling it; bind
// input holder texture; capture current config ONCE and compute signed x87
// reciprocals of+24/+28. Clear+218/+21C only; +10..20F stays untouched. Create
// holder using passed output dimensions/format, mode0, then bind primary color0.
void initialize_native_downscale4x4_pass_00b544f0(void*,std::size_t,
    NativeDepthDownscalePassArguments,const NativeDownscalePassInitializationContext&,
    NativeRenderPassInitializationBlock&);
// Full B546F0[434]: register FOUR borrowed float4s at+10 BEFORE populating XY;
// bind input texture, capture current config ONCE, read height then width and
// invoke full B4CD30. Create mode0 output holder, publish and bind color0.
void initialize_native_downscale2x2_pass_00b546f0(void*,std::size_t,
    NativeDepthDownscalePassArguments,const NativeDownscalePassInitializationContext&,
    NativeRenderPassInitializationBlock&);

// Complete initializer source is distinct from numeric execution evidence.
// Native FH3/SEH/failure, unmasked FP exceptions, private stack aliases and
// application/gameplay behavior remain separate validation obligations.
} // namespace bsp
