#include "bsp/world_entity_update.hpp"

#include <cmath>
#include <cstring>

#include "bsp/native_camera_matrix_math.hpp"

namespace bsp {
namespace {

// 00D7A208 and 00D7A24C. The builders compute their negated entry as
// `-0.0f - sin`, which differs from `-sin` only for a zero sine, where it
// yields -0.0f instead of +0.0f. The bit pattern reaches the multiply, so the
// reconstruction keeps the native expression.
constexpr float kNegativeZero = -0.0f; // 00D7A208
constexpr float kOne = 1.0f;           // 00D7A24C

void identity(float dst[16]) noexcept {
    std::memset(dst, 0, sizeof(float) * 16);
    dst[0] = kOne;
    dst[5] = kOne;
    dst[10] = kOne;
    dst[15] = kOne;
}

} // namespace

void matrix_interpolator_rotation_x_00b64640(float dst[16], float angle) noexcept {
    // 00B64640. Row 0 is the identity row; the sine pair sits in rows 1 and 2.
    const float s = std::sin(angle);
    const float c = std::cos(angle);
    std::memset(dst, 0, sizeof(float) * 16);
    dst[0] = kOne;
    dst[5] = c;
    dst[6] = s;
    dst[9] = kNegativeZero - s;
    dst[10] = c;
    dst[15] = kOne;
}

void matrix_interpolator_rotation_y_00b646e0(float dst[16], float angle) noexcept {
    // 00B646E0. Row 1 is the identity row.
    const float s = std::sin(angle);
    const float c = std::cos(angle);
    std::memset(dst, 0, sizeof(float) * 16);
    dst[0] = c;
    dst[2] = kNegativeZero - s;
    dst[5] = kOne;
    dst[8] = s;
    dst[10] = c;
    dst[15] = kOne;
}

void matrix_interpolator_rotation_z_00b64780(float dst[16], float angle) noexcept {
    // 00B64780. Row 2 is the identity row.
    const float s = std::sin(angle);
    const float c = std::cos(angle);
    std::memset(dst, 0, sizeof(float) * 16);
    dst[0] = c;
    dst[1] = s;
    dst[4] = kNegativeZero - s;
    dst[5] = c;
    dst[10] = kOne;
    dst[15] = kOne;
}

float matrix_interpolator_phase_00904670(float clock, float start, float duration) noexcept {
    // 00904670: FLD elapsed, FDIV [ESI+24h]. 00904681: FCOMIP 0.0f against the
    // quotient, JBE to the upper clamp. 00904B75: COMISS quotient against 1.0f,
    // JBE keeps it. Both JBEs are taken on an unordered compare.
    float phase = (clock - start) / duration;
    if (!(0.0f > phase)) {
        if (phase > kMatrixInterpolatorPhaseCeiling) {
            phase = kMatrixInterpolatorPhaseCeiling;
        }
    } else {
        phase = 0.0f;
    }
    return phase;
}

void matrix_interpolator_compose_009046b5(const MatrixInterpolatorRecord& record, float phase,
                                          float out[16]) noexcept {
    // 009046B8..00904751: the translation row is scaled, the angles are scaled
    // and then negated. Native evaluation order is translation first, then the
    // x, y and z angles, each through its own builder.
    float translate[16];
    identity(translate);
    translate[12] = phase * record.translation[0]; // 009046EB, +30h of the block
    translate[13] = record.translation[1] * phase; // 00904721
    translate[14] = record.translation[2] * phase; // 00904751

    float rotation_x[16];
    float rotation_y[16];
    float rotation_z[16];
    // 009047AB, 00904889, 00904991: FCHS on the scaled angle.
    matrix_interpolator_rotation_x_00b64640(rotation_x, -(phase * record.rotation[0]));
    matrix_interpolator_rotation_y_00b646e0(rotation_y, -(record.rotation[1] * phase));
    matrix_interpolator_rotation_z_00b64780(rotation_z, -(record.rotation[2] * phase));

    // 00904AC3..00904AD8, four chained 00413920 calls. The left operand of the
    // first is the z rotation and the right operand of the last is the record's
    // captured base matrix; each call's destination is the next call's left.
    float stage1[16];
    float stage2[16];
    float stage3[16];
    multiply_native_camera_matrices_00413920(rotation_z, nullptr, stage1, rotation_y);
    multiply_native_camera_matrices_00413920(stage1, nullptr, stage2, rotation_x);
    multiply_native_camera_matrices_00413920(stage2, nullptr, stage3, translate);
    multiply_native_camera_matrices_00413920(stage3, nullptr, out, record.base);
}

void add_matrix_interpolator_00905080(std::vector<MatrixInterpolatorRecord>& records,
                                      std::uint32_t entity, const float translation[3],
                                      const float rotation[3], float duration,
                                      const float entity_local_matrix[16], float clock) noexcept {
    // 009050[9E..B6]: the node is bought and spliced in front of the head
    // sentinel, that is, appended at the back. Every field below is written
    // through that node in this order.
    MatrixInterpolatorRecord record;
    record.entity = entity;                     // 009050D3
    record.translation[0] = translation[0];     // 009050F8
    record.translation[1] = translation[1];     // 00905103
    record.translation[2] = translation[2];     // 00905111
    record.rotation[0] = rotation[0];           // 00905138
    record.rotation[1] = rotation[1];           // 00905146
    record.rotation[2] = rotation[2];           // 00905154
    record.duration = duration;                 // 00905178
    std::memcpy(record.base, entity_local_matrix, sizeof(record.base)); // 0090519D
    record.start_time = clock;                  // 009051CF
    records.push_back(record);
}

MatrixInterpolatorPassResult run_matrix_interpolator_pass_00904600(
    MatrixInterpolatorHost& host, std::vector<MatrixInterpolatorRecord>& records, float clock) {
    MatrixInterpolatorPassResult result{};

    std::size_t index = 0;
    while (index < records.size()) {
        const MatrixInterpolatorRecord& record = records[index];
        ++result.visited;

        // 0090462F loads the clock once per iteration and 0090464B subtracts the
        // record's start time; both values are held across the whole body.
        const float elapsed = clock - record.start_time;

        // 0090465C, the first read of entity+5Ch.
        if (host.entity_active(record.entity)) {
            const float phase = matrix_interpolator_phase_00904670(clock, record.start_time,
                                                                   record.duration);
            float composed[16];
            matrix_interpolator_compose_009046b5(record, phase, composed);
            host.entity_set_local_matrix(record.entity, composed);  // 00904AE3
            host.entity_invalidate_subtree_pose(record.entity);     // 00904AF7..00904B13
            host.entity_refresh(record.entity);                     // 00904B2A
            ++result.applied;
        }

        // 00904B39, the second read of the same byte. The refresh above runs a
        // virtual on the entity between the two reads, so they are kept apart.
        if (host.entity_active(record.entity) && !(elapsed > record.duration)) {
            // 00904B54: JA to the erase only when elapsed is strictly greater.
            ++index;
            continue;
        }

        // 00904B95..00904BD1: unlink, free, _Mysize -= 1, then back to the top
        // of the walk with the successor already in hand.
        records.erase(records.begin() + static_cast<std::ptrdiff_t>(index));
        ++result.retired;
    }

    return result;
}

} // namespace bsp
