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

// (C) 009EE586..009EE594. Every 009ED6B0 pass that reaches 009EE580 (every
// tick the station-keeping arm does not run) clears blk+38Ah, +388h and +389h
// before the path-pick gate. The host kept the latches across skipped ticks.
// True: the three station latches clear there. False: they persist.
inline constexpr bool kShipAiStationLatchClearBound = false;

// (D) 009EE6F1..009EE750. After the path-pick gate opens and before the output
// block and the tail, the parked byte blk+2FDh is computed:
//   2FDh = ([[blk+2F4h]+1Ch] == 0) ? 1
//        : ([blk+310h] >= [blk+30Ch] (signed SETGE) && !blk+2FCh && record+20h)
// The tail's parked stop 009EEF72 reads it, and 009DA610 reads the same byte.
// The host set it only at 009EF045. A sibling of (C), not the same switch:
// different fields, different consumers, different addresses.
// True: the byte is computed there into both host copies (tail.parked_2fd and
// goal.flag_2fd). False: the host's previous single write at 009EF045.
inline constexpr bool kShipAiParkedByteBeforeTailBound = false;

// (F) 009EF94E/009EF96F. Every turn-clearance refresh resets blk+37Ch to
// [00CE3D64] = 10000.0f (0x461C4000) before its lowerings. The host's
// kShipAiClearanceSentinel is 9999.0f. Of its readers only this store stands
// for 00CE3D64; the ShipAiClearanceBlock member default
// (ship_ai_clearance_profile.hpp:108) is the construction value, which no
// seg67 code writes, so it stays as it is.
// True: the refresh stores 10000.0f. False: 9999.0f, as before.
inline constexpr bool kShipAiClearanceRefreshSentinelBound = false;
inline constexpr float kShipAiClearanceRefreshSentinel = 0x1.388p+13f; // 10000.0f, 0x461C4000
static_assert(kShipAiClearanceRefreshSentinel == 10000.0f);

} // namespace bsp
