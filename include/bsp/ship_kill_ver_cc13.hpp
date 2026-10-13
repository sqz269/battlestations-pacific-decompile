#pragma once

// Packet cc13-ver5 (docs/CC13_VER_5.md): reference models of the ship kill chain pieces
// that the library did not carry. Every name is a hypothesis, not a recovered symbol.
// Reconstructed and build-tested only; not ABI-compatible and not game-validated.
// The rest of the chain is already modelled: 00879070 / 00877B90 / 00827A90 / 00958A30 in
// src/unit_damage.cpp, 00959450 in src/mission_result.cpp, 009329C0 in
// src/ship_hydro_forces.cpp.

namespace bsp {

// 00958DA7..00958DB1 (the no-parent arm of 00958A30) and 00827AB0..00827ABE: COMISS
// 0.0, [unit+370h] with JB skipping the death. A health at or below zero kills; an
// unordered (NaN) health sets CF and does NOT kill. bsp::unit_is_dead answers true for a
// NaN, which is the only difference (docs/CC13_VER_5.md D5).
bool unit_health_kills_00958da7(float health) noexcept;

// 009864D2..009864F1 (entityKilled; the same test sits at 0098688B for shipLanded and
// 00986B67 for hpEvent): 00923810(entity, 1) is the +3Ch scene parent. With no parent
// the event fires; with one, only when the parent answers IsKindOf(44h), the Landscape.
bool entity_event_parent_gate_009864d2(bool has_scene_parent,
                                       bool parent_is_landscape) noexcept;

// 00E0CF04[entity+70h], the KillReason strings 00929800 and 00986480 publish: 0 none,
// 1 harm, 2 soft, 3 sell, 4 exitzone, 5 landed, 6 editor, 7 a null entry. Any other
// cause reads past the table in the image; this returns nullptr for it.
const char* kill_reason_name_00e0cf04(int cause) noexcept;

// 00825001..00825044: the wreck's new inertia. 00C37F10 returns f(1 / inverse) per axis
// with no zero test (FLD1 / FDIVRP, so a zero inverse gives +inf), 0082500C..00825041
// multiply each by the double 2.0 at 00D7A308 (exact), and 00C37E70 stores the result
// (its own UCOMISS / JNP maps a zero inertia to a zero inverse and 1/I otherwise, so a
// zero inverse comes back as zero through +inf).
struct WreckInertia {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};
WreckInertia wreck_inertia_00825006(float inverse_x, float inverse_y,
                                    float inverse_z) noexcept;

}  // namespace bsp
