#include "bsp/matrix_orthogonal_inverse.hpp"

#include <cmath>

// Instruction-by-instruction derivation and the numerical verification:
// docs/MATRIX_ORTHOGONAL_INVERSE.md. Every address in a comment below is a
// native instruction in 00B63D50..00B63F09.

namespace bsp {
namespace {

// 00B63D70..00B63DA6 (row 0), repeated verbatim for rows 1 and 2 at
// 00B63DBB..00B63DF1 and 00B63E06..00B63E3C. Each of the three products is
// spilled through `FSTP float ptr [ESP+8]` and reloaded, so each is rounded to
// float32; the two additions happen in the x87 register and only the sum is
// spilled, at `FSTP float ptr [ESP+10h]` / `[ESP+14h]` / `[ESP+8]`.
float row_squared_length(const float* row) {
    const float xx = row[0] * row[0];
    const float yy = row[1] * row[1];
    const float zz = row[2] * row[2];
    return static_cast<float>((static_cast<double>(xx) + static_cast<double>(yy)) +
                              static_cast<double>(zz));
}

// One column of the inverse basis: source row `row` over its own squared
// length, one correctly rounded float division per component. 00B63E3D..
// 00B63E5D, 00B63E5E..00B63E7B, 00B63E7C..00B63E99.
void store_inverse_column(float* destination, const float* row, float squared_length) {
    destination[0] = row[0] / squared_length;  // FSTP float ptr [ECX+00h/04h/08h]
    destination[4] = row[1] / squared_length;  // FSTP float ptr [ECX+10h/14h/18h]
    destination[8] = row[2] / squared_length;  // FSTP float ptr [ECX+20h/24h/28h]
}

}  // namespace

void build_orthogonal_scaled_affine_inverse_00b63d50(CameraMatrix& destination,
                                                     const CameraMatrix& source) {
    const float* const s = source.data();
    float* const d = destination.data();

    const float squared_length_0 = row_squared_length(s + 0);
    const float squared_length_1 = row_squared_length(s + 4);
    const float squared_length_2 = row_squared_length(s + 8);

    // The transpose: source row r becomes destination column r.
    store_inverse_column(d + 0, s + 0, squared_length_0);
    store_inverse_column(d + 1, s + 4, squared_length_1);
    store_inverse_column(d + 2, s + 8, squared_length_2);

    // 00B63E9A..00B63F03. The native negates the first term with FCHS and then
    // subtracts the other two, keeping the whole dot product in the x87 stack
    // and rounding once at each `FSTP float ptr [ECX+30h/34h/38h]`. It reads
    // the destination columns it has just written, not the source rows.
    const double translation_x = static_cast<double>(s[12]);  // source +30h
    const double translation_y = static_cast<double>(s[13]);  // source +34h
    const double translation_z = static_cast<double>(s[14]);  // source +38h
    for (int column = 0; column < 3; ++column) {
        const double projected = -(static_cast<double>(d[column]) * translation_x) -
                                 (translation_y * static_cast<double>(d[4 + column])) -
                                 (translation_z * static_cast<double>(d[8 + column]));
        d[12 + column] = static_cast<float>(projected);
    }

    // 00B63E55 XORPS XMM0,XMM0 feeds the three zeros at 00B63EE2/EEA/EEF;
    // 00B63EF4 loads 00D7A24C = 1.0f for the store at 00B63EFE. The source's
    // own fourth column is never read.
    d[3] = 0.0f;
    d[7] = 0.0f;
    d[11] = 0.0f;
    d[15] = 1.0f;
}

float matrix_orthogonal_row_residual(const CameraMatrix& source) {
    const float* const s = source.data();
    double squared[3];
    for (int row = 0; row < 3; ++row) {
        const double x = static_cast<double>(s[row * 4 + 0]);
        const double y = static_cast<double>(s[row * 4 + 1]);
        const double z = static_cast<double>(s[row * 4 + 2]);
        squared[row] = x * x + y * y + z * z;
        if (!(squared[row] > 0.0)) return 1.0f;  // the native would divide by zero
    }

    double worst = 0.0;
    for (int i = 0; i < 3; ++i) {
        for (int j = i + 1; j < 3; ++j) {
            const double dot = static_cast<double>(s[i * 4 + 0]) * static_cast<double>(s[j * 4 + 0]) +
                               static_cast<double>(s[i * 4 + 1]) * static_cast<double>(s[j * 4 + 1]) +
                               static_cast<double>(s[i * 4 + 2]) * static_cast<double>(s[j * 4 + 2]);
            const double residual = std::fabs(dot) / std::sqrt(squared[i] * squared[j]);
            if (residual > worst) worst = residual;
        }
    }
    return static_cast<float>(worst);
}

}  // namespace bsp
