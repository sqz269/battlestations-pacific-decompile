#include "bsp/profile_seg2_cc13.hpp"

namespace bsp::profile_seg2_cc13 {

// 0042B3C0 / 0042B3F0, __thiscall(ECX = matrix, stack angle), RET 4.
// bsp.py show 0042b3c0 / 0042b3f0: the respective 00B64640/00B646E0
// builder writes a 40h temporary, then 004134F0 copies it into ECX.
// Names are hypotheses; these context-bearing C++ interfaces do not claim
// the original register/stack ABI. Moved from profile_seg2_cc13.cpp so the
// shared setters have their own implementation unit. The live 0042B450/490
// callers retain the SUBSS negation and return-this behavior themselves.
void matrix_set_rotation_x_0042b3c0(Raw matrix, float angle, MatrixNegatedRotationHost& host) {
    Mat16 temp{};
    host.build_rotation_x_00b64640(temp, angle);
    host.matrix_copy_004134f0(matrix, temp);
}

void matrix_set_rotation_y_0042b3f0(Raw matrix, float angle, MatrixNegatedRotationHost& host) {
    Mat16 temp{};
    host.build_rotation_y_00b646e0(temp, angle);
    host.matrix_copy_004134f0(matrix, temp);
}

} // namespace bsp::profile_seg2_cc13
