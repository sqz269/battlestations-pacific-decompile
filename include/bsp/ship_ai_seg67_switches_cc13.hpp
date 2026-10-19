#pragma once
// Packet cc13-seg67h: OFF switches for the seg67 mismatches the seg67f audit
// confirmed (docs/CC13_SEG67D.md adoption notes C, D, F and I). (G) and (H)
// live in bsp/cc13_ship_ai_commit_band.hpp.
namespace bsp {

// (I) 009E08EF. The torpedo crossing margin of 009E04E0 reads the own class's
// Length: 009E089A [ESI+3FCh], 009E08CB [EDX+538h], 009E08EF FLD [EAX+0A0h],
// times 0.6 [00CEFF98]. The host bound unit+9C8h, which in the image equals
// class+A0h only for a class without a model box (00811058).
// True: the margin reads the class Length (GameUnitsHost::unit_class_extents,
// the slot's class+A0h). False: unit+9C8h, as before.
inline constexpr bool kShipAiTorpedoMarginClassLengthBound = false;

} // namespace bsp
