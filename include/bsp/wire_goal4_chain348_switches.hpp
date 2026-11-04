#pragma once

namespace bsp {
// Goal 4 chain 348: the ship-AI host asks a submarine's own vtable+214h / +218h
// (00852FD0 / 00852FF0, vtable 00D0BF80 from 00852F10) for its navigation layer
// and avoid-zone group, both indexed by depthLevel (unit+1268h), instead of the
// surface ship's 006DFD80 / 006DFD90 class+560h slot. False: every ship-AI
// site reads class+560h as before.
inline constexpr bool kWireGoal4Chain348Bound = true;
}
