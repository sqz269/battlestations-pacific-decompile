// Packet cc13-ver5. docs/CC13_VER_5.md. Disk bytes read with bsp.py disasm-raw.
#include "bsp/ship_kill_ver_cc13.hpp"

namespace bsp {

bool unit_health_kills_00958da7(float health) noexcept {
    // COMISS 0.0, health: CF = (0.0 < health) or unordered; JB 00958BC5 skips.
    const bool skip = (0.0f < health) || (health != health);
    return !skip;
}

bool entity_event_parent_gate_009864d2(bool has_scene_parent,
                                       bool parent_is_landscape) noexcept {
    // 009864D7 TEST EAX,EAX / JE 00986505: no parent goes on; 009864EB PUSH 44h,
    // vtable[5Ch], 009864F1 JNZ 00986505: a Landscape parent goes on; else return.
    return !has_scene_parent || parent_is_landscape;
}

const char* kill_reason_name_00e0cf04(int cause) noexcept {
    // 00E0CF04: 00CE43EC, 00D0E35C, 00D0E354, 00D19080, 00D19074, 00D0E34C, 00D1906C, 0.
    static const char* const kNames[] = {
        "none", "harm", "soft", "sell", "exitzone", "landed", "editor", nullptr};
    if (cause < 0 || cause > 7) return nullptr;
    return kNames[cause];
}

WreckInertia wreck_inertia_00825006(float inverse_x, float inverse_y,
                                    float inverse_z) noexcept {
    // 00C37F13..00C37F2F: FLD [+54h], FLD1, FLD ST0, FDIVRP ST2 -> 1 / x; then
    // FDIVR ST1 for y and FDIV [+5Ch] on the remaining 1.0 for z. Each is stored as a
    // float, so the doubling of a float by an exact 2.0 needs no wider intermediate.
    const float ix = 1.0f / inverse_x;
    const float iy = 1.0f / inverse_y;
    const float iz = 1.0f / inverse_z;
    WreckInertia out;
    out.x = ix * 2.0f;
    out.y = iy * 2.0f;
    out.z = iz * 2.0f;
    return out;
}

}  // namespace bsp
