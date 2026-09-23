#pragma once

// The ocean wave field's two leaves under 0078CF20 (docs/OCEAN_HEIGHT.md), read by
// packet cc9_ocean_waves: 0078C890 BSP_OceanWaveField_SampleHeight and 00B9CF50
// BSP_OceanWaveField_CoverageMask. Semantic C++ interfaces for MSVC Win32, not
// binary replacements; names are hypotheses. docs/OCEAN_WAVE_FIELD.md.
//
// The field is the 6BCh-byte object 00BA6FB0 allocates and 00BA6C40 constructs
// (base 00BA1400), stored at world+A8h by the world constructor 0078DAA0 at
// 0078DB03. Its height grid is the 64x64 inverse-FFT output of the spectrum
// object at field+BCh (00B95750 seeds it from a local Mersenne state with 1105h;
// 00B95D30 advances it in time and runs 00B95AA0), which is simulation state no
// file carries. Its amplitude field+24h is stored once, 0.0f, by the base
// constructor (00BA19C6 XORPS, 00BA19C9 MOVSS), and a store census found no
// other writer, so 0078C890 returns h * 0.0f.

#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {

// 0078C890's inputs on the field.
struct OceanWaveFieldState {
    bool flat_f9{false};        // +F9h, 00BA6C40's third argument, [[game+5FCh]+0C20h]
    float inv_tile_b4{0.01f};   // +B4h, 1 / tile, 00B9A4C0 at 00B9A4F1
    float amplitude_24{0.0f};   // +24h, 0.0f from 00BA19C9
};

// The grid 00B960C0 samples at the field+BCh spectrum object: 64 x 64 cells of
// 8 bytes whose first float is negated (SUBSS from the -0.0f at 00D7A208).
// The host does not run the spectrum, so its callers pass a labelled zero grid.
struct OceanWaveGridView {
    const float* cells{nullptr};   // interleaved pairs; nullptr reads as 0.0f
    std::size_t cell_count{0};
};

// 00B960C0 `float __thiscall(spectrum, float u, float v)`, RET 8: u and v times
// the double 63.0 at 00D099A0, floor through 00BF85B0, the integer cells clamped
// to [0, 3Fh] and their +1 neighbours wrapped with AND 8000003Fh, then a bilinear
// blend of the four negated cells in lerp form (a + (b - a) * t).
float ocean_wave_grid_sample_00b960c0(const OceanWaveGridView& grid, float u, float v) noexcept;

// 0078C890: 0.0f when +F9h is set (FLDZ at 0078C89F); otherwise the two
// products k*x and k*z stored to float, their fractional parts formed against the
// 00BF85B0 floor in double and stored to float, the grid sample, and FMUL by
// [field+24h] at 0078C92D, rounded to float once.
float ocean_wave_field_sample_0078c890(const OceanWaveFieldState& field,
                                       const OceanWaveGridView& grid,
                                       float x, float z) noexcept;

// One 30h-byte region record of field+620h (count +624h), built by 00BA0C00.
struct OceanCoverageRegion {
    const std::uint8_t* bitmap{nullptr};   // +4h
    float x0{0.0f};                        // +10h
    float z0{0.0f};                        // +14h
    float x1{0.0f};                        // +18h
    float z1{0.0f};                        // +1Ch
    float scale_x{0.0f};                   // +20h
    float scale_z{0.0f};                   // +24h
    float width{0.0f};                     // +28h
    float height{0.0f};                    // +2Ch
};

// 00B9CF50: the first region with x0 <= x, z0 <= z, x <= x1 and z <= z1 (the
// FCOMI / JC chain 00B9CF73..00B9CF90); none returns 1.0f (FLD1 at 00B9CFA0).
// Inside one: col = min(trunc(float((x - x0) * sx * W)), trunc(W - 1)) and
// row = min(trunc(float((1 - (z - z0) * sz) * H)), trunc(H - 1)), both compared
// UNSIGNED (JA), the byte at bitmap[row * trunc(W) + col] divided by the double
// 255.0 at 00CE4B48, stored to float, and clamped to [0, 1].
float ocean_coverage_mask_00b9cf50(const std::vector<OceanCoverageRegion>& regions,
                                   float x, float z) noexcept;

} // namespace bsp
