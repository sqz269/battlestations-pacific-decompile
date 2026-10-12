#include "bsp/band_0085_cc13.hpp"

namespace bsp::cc13_band0085 {

namespace {

// 00855C20: x when x > 0 (COMISS + JBE), otherwise -0.0f - x (SUBSS from the
// -0.0f at 00D7A208), so -0.0 and NaN take the subtraction.
float abs_by_negative_zero(float x) noexcept {
    return x > 0.0f ? x : (-0.0f - x);
}

// 0085BF90 per-axis term. d and e are stored to float slots before the
// compare; the square is stored to a float slot too.
float axis_distance_squared(float p, float lo, float hi) noexcept {
    const float d = static_cast<float>(static_cast<double>(lo) - static_cast<double>(p));
    if (d > 0.0f) {
        return static_cast<float>(static_cast<double>(d) * static_cast<double>(d));
    }
    const float e = static_cast<float>(static_cast<double>(p) - static_cast<double>(hi));
    if (e > 0.0f) {
        return static_cast<float>(static_cast<double>(e) * static_cast<double>(e));
    }
    return 0.0f;  // MOVSS from the zeroed XMM0
}

}  // namespace

int vector3_all_abs_less_than_00855c20(const float v[3], float eps) noexcept {
    // FCOMI eps, |v[i]| + JBE: fails on eps <= |v[i]| and on unordered.
    for (int i = 0; i < 3; ++i) {
        if (!(eps > abs_by_negative_zero(v[i]))) {
            return 0;
        }
    }
    return 1;
}

bool aabb_overlaps_0085bf20(const float min_a[3], const float max_a[3], const float min_b[3],
                            const float max_b[3]) noexcept {
    // Axis order x, z, y as at 0085BF2B..0085BF70; `>` is false on NaN, which
    // matches the JA/JBE fall-through.
    static constexpr int kAxes[3] = {0, 2, 1};
    for (int axis : kAxes) {
        if (min_a[axis] > max_b[axis] || min_b[axis] > max_a[axis]) {
            return false;
        }
    }
    return true;
}

float aabb_point_distance_squared_0085bf90(const float point[3], const float box_min[3],
                                           const float box_max[3]) noexcept {
    const float x = axis_distance_squared(point[0], box_min[0], box_max[0]);
    const float y = axis_distance_squared(point[1], box_min[1], box_max[1]);
    const float z = axis_distance_squared(point[2], box_min[2], box_max[2]);
    // FLD y; FADD x; FADD z; FSTP float at 0085C07F..0085C08C.
    return static_cast<float>((static_cast<double>(y) + static_cast<double>(x)) + static_cast<double>(z));
}

int unit_is_damaged_00859c60(float max_health_36c, float health_370) noexcept {
    return max_health_36c > health_370 ? 1 : 0;  // FCOMIP + JBE
}

std::uint32_t unit_hangar_block_offset_00851e40(bool is_mothership, bool is_airfield,
                                                bool is_shipyard) noexcept {
    if (is_mothership) {
        return 0x1188u;  // param_1 + 0x462 dwords
    }
    if (is_airfield || is_shipyard) {
        return 0x72Cu;  // param_1 + 0x1CB dwords
    }
    return 0u;
}

bool objective_is_active_008da130(std::int32_t state_1c) noexcept {
    return state_1c == 0;
}

bool objective_is_shown_008da140(std::int32_t kind_18, std::int32_t state_1c) noexcept {
    return kind_18 != 2 || state_1c == 1;
}

void powerup_slot_start_cooldown_008e4b00(PowerupSlotRecord& record, const PowerupClassTimes& cls,
                                          float now_f876a4) noexcept {
    record.deferred_14 = cls.kind_8 == 1;
    if (!record.deferred_14) {
        record.end_time_10 = static_cast<float>(static_cast<double>(cls.cooldown_80) + static_cast<double>(now_f876a4));
        record.ready_time_0c = static_cast<float>(static_cast<double>(cls.delay_7c) + static_cast<double>(now_f876a4));
    }
    record.subject_4 = 0;
    record.pending_0 = false;
}

float powerup_slot_cooldown_fraction_008e4bb0(const PowerupSlotRecord& record, float cooldown_80,
                                              float now_f876a4) noexcept {
    if (record.deferred_14) {
        return 1.0f;  // FLD1
    }
    if (now_f876a4 > record.end_time_10) {
        return 0.0f;  // FLDZ
    }
    // FSUBP ST(1): end - now; FDIV dword class+80h; FSTP float.
    return static_cast<float>((static_cast<double>(record.end_time_10) - static_cast<double>(now_f876a4)) /
                              static_cast<double>(cooldown_80));
}

void landscape_dilate_mask_to_blocks_0087fa90(std::uint8_t* const* rows, std::int32_t row_count,
                                              std::int32_t column_count, std::int32_t block) noexcept {
    if (block <= 1) {
        return;
    }
    for (std::int32_t r0 = 0; r0 <= row_count - block; r0 += block) {
        for (std::int32_t c0 = 0; c0 <= column_count - block; c0 += block) {
            bool any = false;
            // The image's inner break leaves only the column loop; the row
            // scan continues. The result is the same.
            for (std::int32_t r = r0; r < r0 + block && !any; ++r) {
                for (std::int32_t c = c0; c < c0 + block; ++c) {
                    if (rows[r][c] != 0) {
                        any = true;
                        break;
                    }
                }
            }
            if (any) {
                for (std::int32_t r = r0; r < r0 + block; ++r) {
                    for (std::int32_t c = c0; c < c0 + block; ++c) {
                        rows[r][c] = 1;
                    }
                }
            }
        }
    }
}

std::optional<std::string_view> voice_language_name_008d57a0(const VoiceLanguageRow* table,
                                                             std::int32_t count, std::int32_t index,
                                                             bool owns_dl_content_62) noexcept {
    if (count < index) {
        return std::string_view("english");
    }
    if (index < 0 || index == count) {
        return std::nullopt;  // the image reads [00F88974] + index*20h + 10h here
    }
    const VoiceLanguageRow& row = table[index];
    if (row.has_voice_10 == 0) {
        return std::string_view("english");
    }
    if (owns_dl_content_62) {
        return std::string_view("authentic");
    }
    if (row.voice_folder_14 == nullptr) {
        return std::string_view("");
    }
    return std::string_view(row.voice_folder_14);
}

}  // namespace bsp::cc13_band0085
