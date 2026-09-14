#pragma once
#include "bsp/native_logical_buffer_mapping.hpp"
#include <cstdint>

namespace bsp {
// Borrow the actual CE3800 and D7A24C cells. Native MOVSS loads each exactly
// once, only for a nonzero record count. No synthesized .5/1 defaults are used.
struct NativePostEffectGeometryConstants {
    const volatile std::uint32_t* actual_depth_00ce3800;
    const volatile std::uint32_t* actual_one_00d7a24c;
};

// Complete B4D2D0..B4D4B2 normal body. Original ECX unused; four stack DWORDs
// output, first record, record count, input; RET10h; preserves ESI/EDI. This new
// C++ interface adds explicit constant cells and does not claim native ABI.
// Each 36-byte input produces six 28-byte vertices. All address arithmetic wraps
// as native DWORD arithmetic; input/output may overlap in the original access
// order. No struct-wide load, bounds clamp, allocation or retention is added.
// MOVSS preserves words; eight FLD/FSTP pairs retain the native conversions and
// order, including observable NaN/denormal behavior in the admitted FP domain.
// Caller provides valid accessed storage, a free x87 slot and nontrapping x87
// controls. CW/MXCSR are not changed. Hardware-fault timing and native exception
// routing are unclaimed. A zero count dereferences neither input nor constants.
void write_native_post_effect_stream_geometry_00b4d2d0(void* actual_output,
    std::uint32_t first_record, std::uint32_t record_count, const void* actual_input,
    const NativePostEffectGeometryConstants&) noexcept;

struct NativePostEffectStreamGeometryContext {
    NativeLogicalBufferMappingContext& mapping;
    const volatile std::uint32_t* actual_logical_profile_00d61d6c;
    NativePostEffectGeometryConstants constants;
};

// Complete B4D4C0..B4D4FA. Native ECX actual24h receiver, ONE stacked input
// pointer, RET4. Current +18 stream and +20 count feed concrete B49980; after
// map, a fresh +20 >>1 feeds the writer; after writing a fresh +18 is unmapped
// by B49A80. Native count*3 wraps; odd counts leave the last three mapped
// vertices untouched. No automatic unlock is added if a provider throws.
// The explicit context must be the same actual renderer/synchronization/physical
// mapping domain used by the current D61D6C streams. Numeric native slots select
// existing concrete implementations; they are never invoked as host callbacks.
void populate_native_post_effect_stream_geometry_00b4d4c0(void* actual_receiver,
    const void* actual_input, NativePostEffectStreamGeometryContext&);
} // namespace bsp
