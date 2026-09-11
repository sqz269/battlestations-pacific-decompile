#include "bsp/fixed_step_countdown.hpp"

// 008079B0, __stdcall void(float), body 008079B0..00807A41.
// docs/FIXED_STEP_COUNTDOWN.md carries the evidence for every line here.

namespace bsp {

bool step_recon_countdown_008079b0(ReconCountdown& countdown, float step) noexcept
{
    // 008079B0..008079BA: acc = acc - step, stored back as a float.
    countdown.remaining = countdown.remaining - step;

    // 008079C0..008079CA: FLDZ, FCOMI ST0,ST1, JBE. The early exit is taken
    // when 0.0 <= acc and on unordered, so the pass runs only for acc < 0.
    if (!(countdown.remaining < 0.0f)) {
        return false;
    }

    // 008079CE..008079D4: the 3.0 is a double added in the x87 register, and
    // the sum is rounded to float when it is stored to the stack slot.
    const float reloaded = static_cast<float>(
        static_cast<double>(countdown.remaining) + kReconRefreshPeriodSeconds);

    // 008079DC..008079ED: JBE again, so a non-positive reload clamps to zero
    // and the pass runs again on the next step.
    countdown.remaining = (reloaded > 0.0f) ? reloaded : 0.0f;
    return true;
}

void arm_recon_refresh_00803b80(ReconCountdown& countdown) noexcept
{
    countdown.remaining = kReconRefreshExpireNow; // 00803B88, 00807A5E
}

void clear_recon_countdown_006f23c0(ReconCountdown& countdown) noexcept
{
    countdown.remaining = 0.0f; // 006F23C3, 006F4E33
}

bool run_recon_refresh_008079b0(ReconCountdown& countdown, float step,
                                ReconRefreshHost& host)
{
    if (!step_recon_countdown_008079b0(countdown, step)) {
        return false; // 008079CA JBE 00807A3D
    }

    // 008079F7..00807A36: EDI walks 00F874BC to 00F874C8 in steps of four.
    for (std::size_t index = 0; index < kReconSlotCount; ++index) {
        if (!host.slot_present(index)) {
            continue; // 00807A04
        }
        host.rebuild_slot_008073c0(index); // 00807A08
        if (host.slot_dirty(index)) {      // 00807A0D
            host.publish_recon_report_00806b10(index, host.recon_lua_instance()); // 00807A24
        }
        host.clear_slot_dirty(index);      // 00807A29, unconditional
    }
    return true;
}

void force_recon_refresh_00807a50(ReconCountdown& countdown, ReconRefreshHost& host)
{
    arm_recon_refresh_00803b80(countdown);          // 00807A5E
    run_recon_refresh_008079b0(countdown, 0.0f, host); // 00807A66, FLDZ argument
}

bool ensure_recon_slot_008053c0(bool slot_present) noexcept
{
    return !slot_present; // 008053E0 TEST EAX,EAX / 008053E2 JNZ 00805414
}

} // namespace bsp
