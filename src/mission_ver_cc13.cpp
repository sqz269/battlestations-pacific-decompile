// Packet cc13-ver8, docs/CC13_VER_8.md. Read from the disk bytes (bsp.py disasm-raw);
// no Ghidra mutation was made for these models.
#include "bsp/mission_ver_cc13.hpp"

namespace bsp {

bool squadron_death_reports_kill_007f3b10(int cause_70, bool world_active,
                                          bool controller_veto) noexcept {
    if (cause_70 != 1) return false;     // 007F3B19
    if (!world_active) return false;     // 007F3B2A
    return !controller_veto;             // 007F3B46 CMP [ESI+36Ah],0 / JNE 007F3B5B
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
