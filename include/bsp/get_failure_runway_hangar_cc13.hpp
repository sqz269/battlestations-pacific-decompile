#pragma once

// Packet cc13-getfail (census S13). Header-only switch.

namespace bsp {

// kGetFailureRunwayHangarCc13Bound (OFF). Lua GetFailure(entity, name) (0088F1D0)
// calls the entity's vtable+198h (0088F331). The ship base answers with 008135C0,
// which has no "RunwayFailure" or "HangarFailure" arm. Two classes override the slot
// and test those two names first, runway with _stricmp (00BF7FBF) and hangar with
// 00425850 (case-blind):
//   MMothership, vtable 00D01630, slot +198h = 007589D0 (disk dword): reads
//     unit+11A4h / unit+11A5h, which is air-ops block+1Ch / +1Dh (block at +1188h),
//     answers 1, and falls through to 008135C0 for other names. 007583A0 sets these
//     bytes on 72h / 74h, which SetFailure's 00758750 sends.
//   MAirfield, vtable 00CF8C08, slot +198h = 006D2350 (disk dword 00CF8DA0): reads
//     unit+748h / +749h, the same block+1Ch / +1Dh (block at +72Ch, 006D1C20),
//     answers 1, and falls through to 00953E60 (InferiorFailure) for other names.
// The host keeps the block flags as AirOpsDeck::runway_failure / hangar_failure,
// looked up by entity id. Only mothership_apply_failure_message writes them, on
// IsKindOf(09h), which matches the image.
// For an airfield the image never makes them nonzero. The byte and dword stores at
// disp32 744h..749h are:
//   006D31E9/EF (006D31A0 restore, AL = 0)
//   006D0D67/82 (006D0D50)
//   006D0DA7/C2 (006D0D90)
//   0081C57E: 0081C550 stores AL at +749h. That is a ship-class object at +660h, not the
//     airfield, and 0081C550 has no references.
// The disp8 +1Ch/+1Dh stores in 006B0000..006D4340 are 006CAC37/3A (006CAC00 ctor,
// EBX = 0); the rest are message and site constructors. So an airfield answers false
// either way.
// ON: on IsKindOf(09h) or IsKindOf(45h), the two names read the deck's flags (false
// when the deck is not registered) before the ship and inferior arms. A carrier, still
// a ship, takes the deck arm, as 007589D0 runs before 008135C0.
// OFF: both names answer false.
// Not changed: Periscope (labelled), and the unmatched-name stale stack byte.
// Movers: none in this installation's mission Lua, because no script calls GetFailure
// with either name. Scripts that read GetFailure(carrier, "RunwayFailure") /
// "HangarFailure" after a SetFailure would change.
// Lua only: the switch covers 0088F1D0 alone. shipglobals.lua's FailureName rows also
// name RunwayFailure and HangarFailure (the section path, 0093BED0 / 0093C300). A
// native caller of vtable+198h with those names is not covered here.
inline constexpr bool kGetFailureRunwayHangarCc13Bound = false;

}  // namespace bsp
