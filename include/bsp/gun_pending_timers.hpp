#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {

// 0072AD40, body 0072AD40-0072ADB2, `void __thiscall(this, float dt)`, RET 4.
// Its only caller is BSP_Gun_FixedStepTick at 0072D18C, which passes
// `this = gun+114h` (LEA ECX,[ESI+0x114] at 0072D183) and the fixed step's dt
// (0072D17E loads it from [ESP+48h], 0072D189 stores it as the argument). It is
// the first thing the gun runs each step, before the reload and fire work.
//
// The object at gun+114h owns a vector of 12-byte records: begin at its +0Ch
// (gun+120h), count at its +10h (gun+124h). 0072AD40 computes the end as
// `begin + 12*count` through the `LEA EAX,[EAX+EAX*2]` / `LEA EAX,[ESI+EAX*4]`
// pair at 0072AD47 and 0072AD4C, and returns immediately when the range is
// empty (0072AD51).
//
// The rule is an age-and-unordered-erase sweep:
//
//   for each record, front to back:
//     record.countdown -= dt            (0072AD5D..0072AD6A, stored back by FST)
//     if record.countdown < 0:
//       overwrite this record with the LAST record  (0072AD82..0072AD8F)
//       count -= 1                                  (0072AD92, ADD [ECX+10h],-1)
//       re-examine the same slot, do not advance    (0072AD95 jumps past the
//                                                    ADD EDX,0Ch)
//     else:
//       advance one record                          (0072AD97)
//
// Three details the listing pins that a paraphrase would lose:
//
// * The comparison is `FCOMI` of 0.0 against the decremented value followed by
//   `JBE` (0072AD6F/0072AD73), so a record survives when `0.0 <= value` - the
//   erase is on a STRICTLY negative countdown, and a countdown of exactly 0
//   stays.
// * `JBE` is also taken when the comparison is unordered, so **a NaN countdown
//   never expires**. `value < 0.0f` in C++ is likewise false for NaN, which
//   reproduces it exactly; do not rewrite the test as `!(value >= 0.0f)`.
// * The decremented value is stored to the stack and reloaded before both the
//   write-back and the compare (0072AD62/0072AD66), which rounds it to float
//   precision. The write-back happens before the test and so also lands on
//   records that are about to be erased - harmless, since they are overwritten.
//
// What the 12-byte record MEANS is not established. 0072AD40 copies the first
// eight bytes without reading them, so only the countdown at +8h has a proven
// interpretation. The producer that pushes records was not found: the only
// `LEA reg,[reg+114h]` in the whole image that lands in this gun segment is the
// tick call site itself (a scan for `8d 8e 14 01 00 00` returns one gun hit,
// 0072D183), so whatever fills the list reaches it another way. The segment's
// keywords - `barreldelaytime`, `nextfirebarrel`, `loadtime`, `throwa`,
// `throwb` - make a queue of delayed per-barrel events the obvious hypothesis,
// but it is a hypothesis and nothing here tests it.
//
// Descriptive names are hypotheses, not recovered symbols.

// One record of the list at gun+120h. 12 bytes, matching the native stride.
struct GunPendingTimerRecord {
    std::uint32_t payload[2]{0u, 0u};  // +0h..+7h: copied on erase, never read
    float countdown{0.0f};             // +8h: the only field 0072AD40 interprets
};

static_assert(sizeof(GunPendingTimerRecord) == 12,
    "0072AD40 strides the list by 12 bytes (LEA EAX,[EAX+EAX*2] then *4)");

// 0072AD40. Ages every countdown by `dt` and removes the records that go
// strictly below zero, by overwriting each with the list's last element and
// shrinking - so the order is not preserved. Returns how many were removed.
std::size_t gun_age_pending_timers_0072ad40(std::vector<GunPendingTimerRecord>& records,
                                            float dt) noexcept;

}  // namespace bsp
