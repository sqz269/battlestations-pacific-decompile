#pragma once

// Packet cc13-subdive (docs/CC13_SUB_DIVE.md): the submarine depth level 008528B0.
// The switch is OFF; OFF is the host's behaviour before this packet, byte for byte.
//
// Record-only, no switch: 008528B0's A2h post. On a level change the image builds a
// stack message (0075B430(A2h) at 00852924) and calls 0077C7B0(ECX = the
// submarine, msg, 0) at 00852956. 0077C7B0 delivers only when
// [[00E188A8]+1FE4h] == 1 (0077C7B6 CMP, 0077C7C0 JNZ to the return), and then only
// to non-local peers through 00770B50 / 00783DC0; it never appends to the local
// session FIFO. In single player (game+1FE4h = 0) nothing is delivered; the A2h
// handler 00852C60 only re-calls 008528B0 on a remote peer. The hull move to
// bands[level] is the dive controller 00936DC0 reading +1268h every tick
// (kSubmarineDiveBound, ON). No shared-RNG (00BD2F10) draw is on the path.

namespace bsp::cc13_sub_dive {

// The kamikaze class test. 008528B0 (008528E5..00852908) and 00853630's stage 1
// (00853A31..00853A81) read class+510h KamikazeDamage and class+514h
// KamikazeBlastDamage (MOVSS/COMISS against 0.0, JA: true only when above and
// ordered, so NaN reads false; +514h only when +510h fails). True pins the level
// to 1 (periscope) in 008528B0, seeds +1268h = 1 in 00853630, and in the dive law
// 00936DC0 (00936E5C..00936E80) exempts the boat from the needAir / 008522C0
// surfacing override of the effective band. The host reads
// both keys from VehicleClass[type_id] once at the seed and keeps the result on
// the slot. False: the test reads false at both sites (the labelled substitution
// before this packet).
inline constexpr bool kCc13SubDiveKamikazeBound = true;  // ON: USNOS pair, all 12 Kaitens drown with no killer (SHIP_AI 307)

// The nearest-band seed stage (rev-convoy N1). 00853630 reaches 00853BE8 when
// neither `Dive` nor `TargetDive` is found (00853BE2/00853BE6), or for a holder
// that is not kind 1 (00853B0F). 00853BEB..00853C70: best = |hull world Y|; for
// i = 1..3, d = |float(Y - bands[i])|, and best > d (FCOMIP, JBE: equal or
// unordered keeps) takes best = d and +1268h = i. Band 0 is never compared by
// subtraction, and +1268h keeps the stage-1 (kamikaze) value unless a band beats
// |Y|. Independent of kCc13SubDiveKamikazeBound; the image runs both. False: the
// seed keeps the stage-1 value (the labelled substitution before this packet).
inline constexpr bool kSubSeedNearestBandCc13Bound = true;  // ON: USNOS/BSM01/JM06 pairs, seeds as predicted (SHIP_AI 307)

}  // namespace bsp::cc13_sub_dive
