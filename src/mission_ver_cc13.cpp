// Packet cc13-ver8, docs/CC13_VER_8.md. Read from the disk bytes (bsp.py disasm-raw);
// no Ghidra mutation was made for these models.
#include "bsp/mission_ver_cc13.hpp"

namespace bsp {

bool unit_death_reports_kill_00959450(int cause_70, float clock, bool world_active,
                                      bool controller_veto) noexcept {
    // 00959468 COMISS / JBE: report only when clock > 1.0f; NaN fails as well.
    if (!(clock > 1.0f)) return false;
    if (cause_70 != 1) return false;     // 00959475
    if (!world_active) return false;     // 0095948A
    return !controller_veto;             // 009594FC TEST AL,AL / JNE 0095950C
}

bool squadron_death_reports_kill_007f3b10(int cause_70, bool world_active,
                                          bool controller_veto) noexcept {
    if (cause_70 != 1) return false;     // 007F3B19
    if (!world_active) return false;     // 007F3B2A
    return !controller_veto;             // 007F3B46 CMP [ESI+36Ah],0 / JNE 007F3B5B
}

bool think_timed_arm_00929460(bool armed_1dc, float& delay_1e0, float step) noexcept {
    // 009294B4..009294C5: the armed delay must be above zero, else the untimed arm.
    if (!armed_1dc || !(delay_1e0 > 0.0f)) return false;
    // 009294C7..009294D7: FSUB, FSTP to a float, FST into +1E0h.
    delay_1e0 = delay_1e0 - step;
    // 009294DD FLDZ / FCOMIP ST(1) / JB 00929534: waits while 0 < remaining.
    return !(0.0f < delay_1e0) && delay_1e0 == delay_1e0;
}

float squadron_average_health_007edc30(const float* member_health_00923be0,
                                       int member_count_3cc, int initial_count_3c8) noexcept {
    if (initial_count_3c8 <= 0) return 0.0f;   // 007EDC3B JLE 007EDC8A, FLDZ
    float sum = 0.0f;                          // 007EDC3D..007EDC49
    for (int i = 0; i < member_count_3cc; ++i) {
        sum = sum + member_health_00923be0[i]; // 007EDC5F FADD, 007EDC6F FSTP float
    }
    // 007EDC7B FIDIV dword [+3C8h], 007EDC82 FSTP float. SUBSTITUTION (labelled): the
    // quotient is taken in double, not the x87's extended precision, before the float
    // rounding.
    return static_cast<float>(static_cast<double>(sum) / static_cast<double>(initial_count_3c8));
}

}  // namespace bsp
