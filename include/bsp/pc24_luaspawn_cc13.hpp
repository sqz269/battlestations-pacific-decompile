#pragma once

// PC24 wave 2, packet pc24w2-luaspawn (census P6). The Lua spawn host runs on the main
// thread, whose x87 runs at 24-bit precision control: every FADD/FSUB/FMUL/FDIV rounds to
// a 24-bit significand (with the extended exponent) before the next op.
//
// kPc24LuaSpawnCc13Bound (OFF): src/lua_spawn_new.cpp spawn_member_position_00949300.
// 00949300 builds T = identity with row 3 = (o0, o1, o2, 1) and CALLs 00413920 at
// 009493E4 (out = T x M, row-major 4x4). Row 3 of out, per axis c (00413BEE-00413C1C for
// c = 0): ((o0*M0c + o1*M1c) + o2*M2c) + 1*M3c, each FMUL and FADDP at PC24, stored float.
// ON evaluates that per op through the cc13x87 helpers; OFF keeps the double chain
// rounded once. Expected movers when ON: Lua-spawned group member positions (last bit).

namespace bsp {

// ON (cc9-recon2): flipped with kPc24MissionLuaCc13Bound; 18 WINS-row pairs on afa0394fc.
inline constexpr bool kPc24LuaSpawnCc13Bound = true;

}  // namespace bsp
