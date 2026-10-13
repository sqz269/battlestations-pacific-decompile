#pragma once

// Packet cc13-ver8 (docs/CC13_VER_8.md): reference models for the mission-frame and
// objective-evaluation checks. Each function names the image range it follows. Names
// are hypotheses, not recovered symbols. These are new C++ interfaces, not drop-in
// binary replacements. The unit path's kill report gate is bsp::unit_death_00959450
// (include/bsp/mission_result.hpp), and the think walk's timed arm is
// bsp::entity_think_branch (include/bsp/entity_think_dispatch.hpp); neither is repeated.

namespace bsp {

// 007F3B10 (the squadron's vtable +80h, vtable 00D087C0, __thiscall): whether the
// squadron's death reaches the `kill` channel 009813A0 at 007F3B56. 007F3B19
// CMP [ESI+70h],1 / JNE; 007F3B2A world+4ACh; 007F3B33..007F3B4D the controller at
// +35Ch answering kind 17h with +36Ah set vetoes it. No clock gate, unlike 00959450.
// The squadron's cause is its last plane's +70h (007F3A2A Kill(plane+70h)).
bool squadron_death_reports_kill_007f3b10(int cause_70, bool world_active,
                                          bool controller_veto) noexcept;

// 007EDC30 (BSP_PlaneSquadron_AverageHealth, the squadron's vtable +110h): 0 when the
// initial count +3C8h is not above 0 (JLE); otherwise the float sum of 00923BE0 over
// the +3CCh current members (+3D0h array; each FADD stored as a float), FIDIV +3C8h,
// stored as a float. Members already removed count as 0.
float squadron_average_health_007edc30(const float* member_health_00923be0,
                                       int member_count_3cc, int initial_count_3c8) noexcept;

}  // namespace bsp
