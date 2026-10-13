#pragma once

// Packet cc13-ver8 (docs/CC13_VER_8.md): reference models for the mission-frame and
// objective-evaluation checks. Each function names the image range it follows. Names
// are hypotheses, not recovered symbols. These are new C++ interfaces, not drop-in
// binary replacements.

namespace bsp {

// 00959450 (BSP_Unit_OnDestroyed, __thiscall, ECX = unit): whether the unit's death
// reaches the `kill` channel 009813A0 at 00959507.
//  00959460 MOVSS clock [00F876A4]; 00959468 COMISS [00D7A24C] = 1.0f; JBE past it
//    (a clock at or below 1.0, or unordered, does not report);
//  00959475 CMP [EDI+70h],1 / JNE past it (only cause 1, "harm");
//  0095948A CMP [[game+19CCh]+4ACh],0 / JE past it (the world's active byte);
//  009594C4..009594FE: the controller at +538h answering kind 17h with +C41h set, or
//    kind 6 with +100Ah set, vetoes the report (0091BDA0 at 0095950C still runs).
bool unit_death_reports_kill_00959450(int cause_70, float clock, bool world_active,
                                      bool controller_veto) noexcept;

// 007F3B10 (the squadron's vtable +80h, vtable 00D087C0, __thiscall): the same report
// at 007F3B56. 007F3B19 CMP [ESI+70h],1 / JNE; 007F3B2A world+4ACh; 007F3B33..007F3B4D
// the controller at +35Ch answering kind 17h with +36Ah set vetoes it. No clock gate.
// The squadron's cause is its last plane's +70h (007F3A2A Kill(plane+70h)).
bool squadron_death_reports_kill_007f3b10(int cause_70, bool world_active,
                                          bool controller_veto) noexcept;

// 00929460's timed arm for one live, eligible entity (009294AB..009294E5): when +1DCh is
// set and +1E0h > 0 (COMISS / JBE), +1E0h becomes delay - step (FSUB, stored as a float)
// BEFORE the think is called, and the think fires when the stored value is <= 0
// (FLDZ / FCOMIP / JB: an unordered value waits). A delay the think re-arms through
// SetWait (008982C9) is therefore first decremented on the next pass. Returns true when
// the think fires; `delay_1e0` is updated in place as the image does.
bool think_timed_arm_00929460(bool armed_1dc, float& delay_1e0, float step) noexcept;

// 007EDC30 (BSP_PlaneSquadron_AverageHealth, the squadron's vtable +110h): 0 when the
// initial count +3C8h is not above 0 (JLE); otherwise the float sum of 00923BE0 over
// the +3CCh current members (+3D0h array; each FADD stored as a float), FIDIV +3C8h,
// stored as a float. Members already removed count as 0.
float squadron_average_health_007edc30(const float* member_health_00923be0,
                                       int member_count_3cc, int initial_count_3c8) noexcept;

}  // namespace bsp
