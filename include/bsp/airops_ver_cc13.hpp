#pragma once
// Packet cc13-ver10 (docs/CC13_VER_10.md): the squadron command block step's
// tail, 0084E4B1-0084E5C6 of 0084E010 (BSP_PlaneSquadronCommandBlock_Step,
// __fastcall, ECX = the +348h command block; vtable 00D0BD98 slot +7Ch). Read
// from the disk bytes (disasm-raw 0084E45D, length 170h). Reference model only:
// nothing here is wired, and every name is a hypothesis, not a recovered symbol.
//
// The tail decides between three outcomes once the stage raises above it
// (0084E45D-0084E4AC: stage 1 -> 2, override stage 1 -> 2) have run:
//  - resolve: 007F16D0(&out) on the squadron (0084E552); a non-null answer is
//    issued (0071D880 clear, 0071ECF0 issue, +228h = 1) and the step returns;
//  - moveto self: 0071ECF0(00E08F68, 00465080(squadron, 0.0)) (0084E595-0084E5C6),
//    reached from the latch-clear arm, from a failed resolve gate, and from a
//    null 007F16D0 answer;
//  - nothing.
#include <cstdint>

namespace bsp {

struct SquadronStepTailInputs {
    int stage_48{0};                  // block+48h, after 0084E45D's raise
    int override_stage_50{0};         // block+50h, after 0084E4A4's raise
    int filled_slots_71be60{0};       // 0071BE60(): queue slots in use
    bool latch_3c{false};             // [block+38h]+3Ch, the spent-ordnance latch
    std::uint32_t command_54{0};      // block+54h, the current command class (0 = none)
    int command_category_0c{0};       // command class vtable[0Ch]() (read only when asked)
    bool head_arm{false};             // 0084E047-0084E05C: +54h == 0, +3D0h set, 0071BE60() == 0
    bool reload_enabled_369{true};    // squadron+369h ReloadEnabled (constructor 1, 007F2D09)
    bool device_reload_e17bf2{false}; // [00E17BF2], SetDeviceReloadEnabled
    bool flag_368{false};             // squadron+368h (only 007F2C60's zeroing writes it)
};

enum class SquadronStepTailOutcome : std::uint8_t {
    kNothing,       // 0084E5CB
    kResolve,       // 0084E54D: call 007F16D0; on null take kMoveToSelfIfRoom
    kMoveToSelfIfRoom,  // 0084E595: moveto self when 0071BE60() <= (stage == 2)
};

struct SquadronStepTailPlan {
    bool b4{false};  // [ESP+11h], the latch-clear / idle arm
    bool b5{false};  // [ESP+13h], the latched / finished-return arm
    SquadronStepTailOutcome outcome{SquadronStepTailOutcome::kNothing};
};

// 0084E4B1-0084E593.
SquadronStepTailPlan squadron_step_tail_plan_0084e4b1(const SquadronStepTailInputs& in) noexcept;

// 0084E595-0084E5A6: the moveto-self test, `0071BE60() <= (block+48h == 2)`.
bool squadron_step_moveto_self_0084e595(int filled_slots, int stage) noexcept;

// The whole tail with 007F16D0's answer supplied: true when the step issues
// moveto on the squadron itself.
bool squadron_step_issues_moveto_self(const SquadronStepTailInputs& in,
                                      bool rtb_answer_nonnull) noexcept;

}  // namespace bsp
