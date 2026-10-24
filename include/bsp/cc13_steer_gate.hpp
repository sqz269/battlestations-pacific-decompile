#pragma once

// Packet cc13-steergate (docs/CC13_STEER_GATE.md): the gun controller's commanded
// direction (approach+1Ch)+68h..+70h and the fine-aim steer gate that reads it.
//
// 009FC7C0 BSP_BotTaskGun_Tick (gun = task+314h, __thiscall, RET 4) steers through
// 009F9FC0 when, at 009FCCA0-009FCCB4,
//     gun+40h > 1 - (u . gun+68h)   (FCOMPI, JBE: unordered fails)   and   gun+4Ch <= 0,
// with u the unit lead direction (009FCB70-009FCB92). gun+68h is the COMMANDED direction:
// the state ticks write it through 009FABE0(heading, pitch) right after their cone store
// to gun+40h, in the task arm that runs before the gun tick (00999979). When all three
// components compare equal to 0.0 (009FC829-009FC855, UCOMISS/LAHF/TEST AH,44h/JP: a NaN
// counts as non-zero) the tick first copies row 2 (the forward) of the unit's world
// matrix (0042D7E0, 009FC857-009FC871). The tail zeroes +70h/+6Ch/+68h and the cone +40h
// (009FCE5A-009FCE69) on every path that reaches 009FCE26; the two early returns
// (gun+0 == 0, unit+2F4h == 0, to 009FCED9) leave them. The gun constructor 009FAAD0
// never writes +68h..+70h.
//
// The 28 callers of 009FABE0 (rel32 scan; no absolute references) are all bot state
// ticks, each with ECX = [approach+1Ch] and a cone store just before; the table is in
// the doc.

#include <array>

namespace bsp {

// True: every hosted 009FABE0 site stores the per-unit gun+68h direction
// (GameUnitSlot::gun_direction_68), and the gun tick's tail clears it with the cone.
// False: nothing is stored (the host's long-standing state).
inline constexpr bool kBotCommandedDirection68Bound = true;

// True: the fine-aim steer gate (009FCB96-009FCCAB) measures the lead against the
// stored commanded direction when a state wrote one this think, and against the unit's
// forward otherwise (009FC857). False: it always measures against the forward
// (SUBSTITUTION, labelled, at df_fine_aim_009f9fc0).
inline constexpr bool kSteerGateCommandedDirectionBound = true;

static_assert(!kSteerGateCommandedDirectionBound || kBotCommandedDirection68Bound,
              "the steer gate reads the direction kBotCommandedDirection68Bound stores");

// 009FABE0 __thiscall(gun, float heading, float pitch), RET 8: the normalised
// (cos a, tan pitch, sin a) with a = pi/2 - heading (+2pi when negative). This runs
// bsp::cc13_seg68::bot_set_direction_from_heading_pitch_009fabe0 on a scratch gun block
// with 00412E20 = native_projection_tangent_00412e20 and 00419440 as the float length
// the ledger records (each square stored as float, y2 + x2 then + z2, the sum stored as
// float, then its square root as float). Main-thread x87 at 24 bits is assumed (each
// x87 op rounds to float); at 53 bits the subtraction and products would round twice.
std::array<float, 3> bot_commanded_direction_009fabe0(float heading, float pitch);

// 009FCB96-009FCBBB: dot = (d[1] * u[1] + d[0] * u[0]) + d[2] * u[2], each step
// rounded to float (24-bit precision control), stored as float at 009FCBAF, then
// 1 - dot (FLD1 / FSUBRP) stored as float at 009FCBBB. u is the unit lead direction
// (009FCB70-009FCB92: each world component divided by the float length, FSTP float).
float steer_gate_one_minus_cos_009fcb96(const float direction_68[3], const float lead_unit[3]);

}  // namespace bsp
