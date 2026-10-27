#include "bsp/ocean_wave_field.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_flightrender_cc13.hpp"

#include <cmath>
#include <cstdint>

namespace bsp {
namespace {

namespace pc24 = cc13x87;

constexpr double kOceanGridScale = 63.0;       // 00D099A0
constexpr int kOceanGridMask = 0x3F;           // 00B96140 MOV EDI,3Fh
constexpr double kOceanCoverageDivisor = 255.0; // 00CE4B48

float grid_cell(const OceanWaveGridView& grid, int index) noexcept {
    // 00B961F0 / 00B96228 / 00B96258 / 00B96299: -0.0f minus the cell's first
    // float, i.e. its negation.
    const std::size_t i = static_cast<std::size_t>(index);
    if (grid.cells == nullptr || i >= grid.cell_count) return -0.0f;
    return -0.0f - grid.cells[i * 2u];
}

int clamp_cell(int value) noexcept {
    if (value < 0) return 0;
    return value > kOceanGridMask ? kOceanGridMask : value;
}

int wrap_next(int value) noexcept {
    // LEA ECX,[EAX+1]; AND ECX,8000003Fh; the negative fix-up; then clamped.
    int next = (value + 1) & static_cast<int>(0x8000003Fu);
    if (next < 0) next = ((next - 1) | ~kOceanGridMask) + 1;
    return clamp_cell(next);
}

std::uint32_t truncate_unsigned(double value) noexcept {
    // FISTP qword under RC = truncate (OR 0C00h), low dword read as unsigned.
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
}

} // namespace

float ocean_wave_grid_sample_00b960c0(const OceanWaveGridView& grid, float u,
                                      float v) noexcept {
    float fu;
    float fv;
    if constexpr (kPc24FlightRenderCc13Bound) {
        // pc24w1-sweep1-2: 00B960CF FMUL (DC C9, the listing shows the D8 form) and
        // 00B960DD FMUL, each rounded at PC24 before FSTP float.
        fu = static_cast<float>(pc24::x87_mul24(u, kOceanGridScale));
        fv = static_cast<float>(pc24::x87_mul24(kOceanGridScale, v));
    } else {
        fu = static_cast<float>(static_cast<double>(u) * kOceanGridScale);
        fv = static_cast<float>(kOceanGridScale * static_cast<double>(v));
    }
    const float floor_u = static_cast<float>(std::floor(static_cast<double>(fu)));
    const float floor_v = static_cast<float>(std::floor(static_cast<double>(fv)));
    const float tu = fu - floor_u;
    const float tv = fv - floor_v;
    const int cu = clamp_cell(static_cast<int>(std::lrint(floor_u)));
    const int cv = clamp_cell(static_cast<int>(std::lrint(floor_v)));
    const int nu = wrap_next(cu);
    const int nv = wrap_next(cv);
    const float a = grid_cell(grid, cv * 64 + cu);
    const float b = grid_cell(grid, cv * 64 + nu);
    const float c = grid_cell(grid, nv * 64 + cu);
    const float d = grid_cell(grid, nv * 64 + nu);
    if constexpr (kPc24FlightRenderCc13Bound) {
        // pc24w1-sweep1-2: 00B9627A..00B962C7, three lerps x + (y - x) * t, the
        // FSUBP, FMULP and FADDP each rounded at PC24 and each lerp stored to float.
        const auto lerp = [](float x, float y, float t) {
            return static_cast<float>(pc24::x87_add24(x, pc24::x87_mul24(pc24::x87_sub24(y, x), t)));
        };
        const float top = lerp(a, b, tu);
        const float bottom = lerp(c, d, tu);
        return lerp(top, bottom, tv);
    } else {
        const float top = static_cast<float>(static_cast<double>(a) +
            (static_cast<double>(b) - a) * tu);
        const float bottom = static_cast<float>(static_cast<double>(c) +
            (static_cast<double>(d) - c) * tu);
        return static_cast<float>(static_cast<double>(top) +
            (static_cast<double>(bottom) - top) * tv);
    }
}

float ocean_wave_field_sample_0078c890(const OceanWaveFieldState& field,
                                       const OceanWaveGridView& grid, float x,
                                       float z) noexcept {
    if (field.flat_f9) return 0.0f;                                   // 0078C89F
    const float kx = static_cast<float>(static_cast<double>(field.inv_tile_b4) * x);
    const float kz = static_cast<float>(static_cast<double>(field.inv_tile_b4) * z);
    const float u = static_cast<float>(static_cast<double>(kx) -
                                       std::floor(static_cast<double>(kx)));
    const float v = static_cast<float>(static_cast<double>(kz) -
                                       std::floor(static_cast<double>(kz)));
    const float h = ocean_wave_grid_sample_00b960c0(grid, u, v);      // 0078C928
    return static_cast<float>(static_cast<double>(h) * field.amplitude_24); // 0078C92D
}

float ocean_coverage_mask_00b9cf50(const std::vector<OceanCoverageRegion>& regions,
                                   float x, float z) noexcept {
    for (const OceanCoverageRegion& r : regions) {
        // 00B9CF73..00B9CF90: FCOMI/JB thrice and FCOMPI/JAE; an unordered
        // compare (NaN) leaves the region, so a NaN point gets 1.0.
        if (!(x >= r.x0) || !(z >= r.z0) || !(r.x1 >= x) || !(r.z1 >= z)) continue;
        float row_f;
        std::uint32_t col;
        float col_f;
        if constexpr (kPc24FlightRenderCc13Bound) {
            // pc24w1-sweep1-2: every op rounded at PC24. 00B9CFB3..00B9CFCA
            // (1 - (z - z0) * scale_z) * height; 00B9CFE4 width - 1;
            // 00B9D002..00B9D00F ((x - x0) * scale_x) * width.
            row_f = static_cast<float>(pc24::x87_mul24(
                pc24::x87_sub24(1.0, pc24::x87_mul24(pc24::x87_sub24(z, r.z0), r.scale_z)), r.height));
            col = truncate_unsigned(pc24::x87_sub24(r.width, 1.0));
            col_f = static_cast<float>(pc24::x87_mul24(
                pc24::x87_mul24(pc24::x87_sub24(x, r.x0), r.scale_x), r.width));
        } else {
            row_f = static_cast<float>(
                (1.0 - (static_cast<double>(z) - r.z0) * r.scale_z) * r.height);
            col = truncate_unsigned(static_cast<double>(r.width) - 1.0);
            col_f = static_cast<float>(
                (static_cast<double>(x) - r.x0) * r.scale_x * r.width);
        }
        const std::uint32_t col_try = truncate_unsigned(col_f);
        if (!(col_try > col)) col = col_try;                          // 00B9D03B JA
        std::uint32_t row;
        if constexpr (kPc24FlightRenderCc13Bound) {
            row = truncate_unsigned(pc24::x87_sub24(r.height, 1.0));  // 00B9D048 FSUBP
        } else {
            row = truncate_unsigned(static_cast<double>(r.height) - 1.0);
        }
        const std::uint32_t row_try = truncate_unsigned(row_f);
        if (!(row_try > row)) row = row_try;                          // 00B9D08D JA
        const std::uint32_t stride = truncate_unsigned(r.width);
        if (r.bitmap == nullptr) return 0.0f;
        const std::uint8_t byte = r.bitmap[row * stride + col];
        float q;
        if constexpr (kPc24FlightRenderCc13Bound) {
            q = static_cast<float>(pc24::x87_div24(byte, kOceanCoverageDivisor));  // 00B9D0C5 FDIV
        } else {
            q = static_cast<float>(static_cast<double>(byte) / kOceanCoverageDivisor);
        }
        if (0.0f > q) return 0.0f;                                     // 00B9D0D9
        return q > 1.0f ? 1.0f : q;                                    // 00B9D100
    }
    return 1.0f;                                                       // 00B9CFA0
}

} // namespace bsp
