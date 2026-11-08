// Focused Win32 probe for 007D9F60's newly hosted free-flight path.
// Link against Release/bsp_core.lib with /link /MANIFEST:EMBED.
#include "bsp/plane_angular_velocity.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <float.h>
#include <limits>

int main() {
    unsigned int previous = 0;
    _controlfp_s(&previous, _PC_24, _MCW_PC);
    bsp::PlaneAdvanceInputs input;
    input.committed_pose.m[0] = input.committed_pose.m[5] =
        input.committed_pose.m[10] = input.committed_pose.m[15] = 1.0f;
    input.committed_pose.m[12] = 10.0f;
    input.committed_pose.m[13] = 20.0f;
    input.committed_pose.m[14] = 30.0f;
    input.step = 0.5f;
    input.linear_velocity[0] = 4.0f;
    input.linear_velocity[1] = -2.0f;
    input.linear_velocity[2] = 6.0f;
    input.external_velocity[0] = 2.0f;
    input.external_velocity[1] = 1.0f;
    input.external_velocity[2] = -4.0f;
    input.angular_velocity[1] = 0.5f;
    input.bank_angle = 0.3f;
    input.bank_yaw_gain = 2.0f;
    input.difficulty = 0.75f;
    input.level_blend_input = 4.0f;
    input.blend_curve_x0 = 3.0f;
    input.blend_curve_x1 = 5.0f;
    bsp::NativeAdvanceMatrixOps ops;
    const auto result = bsp::rotate_and_translate_007d9f60(input, ops);
    // Check the analytic position from both velocity contributions independently
    // of the orientation update. Host dispatch is verified by source inspection.
    if (result.live_pose.m[12] != 13.0f || result.live_pose.m[13] != 19.5f ||
        result.live_pose.m[14] != 31.0f || !result.rotation_applied ||
        result.levelling_applied || result.attached_branch_unmodelled ||
        result.attached_lifted ||
        std::fabs(result.bank_yaw_angle - -0.11082008f) > 0.000001f ||
        std::fabs(result.live_pose.m[2]) < 0.01f) {
        std::fputs("free-flight pose publication failed\n", stderr);
        return 1;
    }
    // 007D9F95..007D9FA0 copies the boundary pose and returns on zero dt,
    // before consuming even nonfinite velocity/attitude inputs.
    input.step = 0.0f;
    input.angular_velocity[1] = input.linear_velocity[0] =
        input.external_velocity[0] = input.bank_angle = std::numeric_limits<float>::quiet_NaN();
    const auto stopped = bsp::rotate_and_translate_007d9f60(input, ops);
    if (std::memcmp(stopped.live_pose.m, input.committed_pose.m,
                    sizeof input.committed_pose.m) != 0 || stopped.rotation_applied ||
        stopped.bank_yaw_angle != 0.0f) {
        std::fputs("zero-step pose publication failed\n", stderr);
        return 1;
    }
    std::puts("007D9F60 free-flight and zero-step probe passed");
    return 0;
}
