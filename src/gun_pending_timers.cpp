// 0072AD40, the gun's base tick. See include/bsp/gun_pending_timers.hpp and
// docs/GUN_BASE_TICK.md.
#include "bsp/gun_pending_timers.hpp"

namespace bsp {

std::size_t gun_age_pending_timers_0072ad40(std::vector<GunPendingTimerRecord>& records,
                                            float dt) noexcept {
    // 0072AD4F/0072AD51: begin == end returns without touching the x87 stack.
    std::size_t erased = 0;
    std::size_t i = 0;
    while (i < records.size()) {
        GunPendingTimerRecord& record = records[i];

        // 0072AD5D..0072AD6A. The native rounds through a stack slot before
        // storing; assigning a float here has the same effect.
        record.countdown = record.countdown - dt;

        // 0072AD6F/0072AD73: FCOMI 0.0 against the value, JBE. Survives when
        // 0.0 <= value, and also when the compare is unordered - so a NaN is
        // kept. `value < 0.0f` is false for NaN, which matches; writing this as
        // !(value >= 0.0f) would erase NaN records instead.
        if (record.countdown < 0.0f) {
            // 0072AD82..0072AD8F copies the last record over this one, then
            // 0072AD92 drops the count. 0072AD95 skips the advance, so the
            // record just swapped in is examined next - including the case
            // where it is itself already expired.
            record = records.back();
            records.pop_back();
            ++erased;
            // i is deliberately not advanced.
        } else {
            ++i;  // 0072AD97: ADD EDX,0Ch
        }
    }
    return erased;
}

}  // namespace bsp
