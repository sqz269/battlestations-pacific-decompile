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
// for 00CE3D64. The ShipAiClearanceBlock member default
// (ship_ai_clearance_profile.hpp:108) is the construction value: in the image
// 009DFCB0 (ECX = blk+1C4h) stores self+1B8h = blk+37Ch = 0.0f at 009DFDA9.
// That value is unobservable: 009DFCB0 also leaves +374h = 0.0f (self+1B0h),
// every other +374h writer stores -1.0f, so the first 009EF910 call always
// falls through its timer (009EF93D JA, not taken for 0 - dt or -1 - dt) and
// stores 37Ch at 009EF96F; 009F4D10 makes that call (009F4D87) before
// 009F50E0 reaches 009F4DA0 -> 009F3F80's reader at 009F416E (009F5227,
// then 009F5248, straight-line). So the default is left as it is.
// True: the refresh stores 10000.0f. False: 9999.0f, as before.
inline constexpr bool kShipAiClearanceRefreshSentinelBound = false;
inline constexpr float kShipAiClearanceRefreshSentinel = 10000.0f; // 00CE3D64
static_assert(kShipAiClearanceRefreshSentinel == 0x1.388p+13f);    // 0x461C4000

// (D') 009ED7E6..009ED7F3 sets [ESP+4Fh] = (blk+1C4h == 3); 009EE756
// CMP [ESP+4Fh],0 / JNE 009EF206. In mode 3 (written by the attack-move
// approach step at 009F3375) the image still runs the 009ED6B0 prologue, the
// 009EE586 clears, the 009EE59B gate, the path pick 009EE5BA..009EE66C, the
// output prelude 009EE671..009EE6CB (A90h = 1, A94h/A98h/A9Ch = path x, 0,
// path z, and 1F0h = max(1F0h, 009D9E50([blk+2F4h]))) and the 2FDh writes
// 009EE6F1..009EE750, then skips the rest of the output block 009EE765..009EEAA2
// and the tail 009EEAAB..009EF1FE and goes straight to 009EF206: 009DE5B0(dt),
// AL = [ESP+37h] = 0 (set at 009EDA2F).
// True: the host does the same for NavigateAstern. False: the host runs the
// output block and the tail for mode 3 as for mode 2.
inline constexpr bool kShipAiNavigateAsternSkipBound = false;

// 009EE59B/009EE59F. When blk+1C4h is neither 2 nor 3 (Rudder or Heading),
// the path gate JZ 009EF206 still reaches 009EF206: 009DE5B0(dt), then
// AL = [ESP+37h] = 0. The host's closed-gate return
// (kShipAiNavTailGateBookkeeping, packet cc9_ship_ai_tails_2) skipped it.
// A sibling of kShipAiNavTailGateBookkeeping, which only chooses the
// bookkeeping record.
// True: the closed gate makes the same arm-final call before returning false.
// False: it returns without it, as before.
inline constexpr bool kShipAiNavGateClosedArmFinalBound = false;

} // namespace bsp
