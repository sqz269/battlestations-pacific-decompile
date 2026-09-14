# The gun's base tick: `0072AD40`

`BSP_Gun_FixedStepTick` runs this first, before any reload or fire work. It was the
second-most-called unimplemented host method in a full mission - 1,818,000 calls across 3000 ticks
of `IJN01` - which is what drew attention to it. The body is 114 bytes and calls nothing.

## ABI and call site

`void __thiscall(this, float dt)`, body `0072AD40`-`0072ADB2`, `RET 4`. One caller, `0072D18C`.

The caller sets both arguments in the three instructions before the call:

```
0072d17e  FLD  float ptr [ESP + 0x48]     ; the fixed step's dt
0072d182  PUSH ECX
0072d183  LEA  ECX,[ESI + 0x114]          ; this = gun+114h
0072d189  FSTP float ptr [ESP]            ; dt as the stack argument
0072d18c  CALL 0x0072ad40
```

So `this` is a sub-object at **gun+114h** owning a vector of 12-byte records: begin at its `+0Ch`
(gun+120h), count at its `+10h` (gun+124h). The stride is pinned by the address arithmetic at
`0072AD47`/`0072AD4C`, `LEA EAX,[EAX+EAX*2]` then `LEA EAX,[ESI+EAX*4]` - count times three times
four. An empty range returns at `0072AD51` without touching the x87 stack.

## The rule

An age-and-unordered-erase sweep:

```
for each record, front to back:
  record.countdown -= dt                       0072AD5D..0072AD6A, stored back by FST
  if record.countdown < 0:
    overwrite this record with the LAST record 0072AD82..0072AD8F
    count -= 1                                 0072AD92, ADD [ECX+10h],ESI with ESI = -1
    re-examine the same slot, do not advance    0072AD95 jumps past the ADD EDX,0Ch
  else:
    advance one record                          0072AD97
```

Three details the listing pins that a paraphrase would lose:

* The test is `FCOMI` of `0.0` against the decremented value followed by `JBE`
  (`0072AD6F`/`0072AD73`), so a record survives when `0.0 <= value`. The erase is on a **strictly**
  negative countdown; a countdown of exactly zero stays.
* `JBE` is also taken when the compare is unordered, so **a NaN countdown never expires**. The C++
  `value < 0.0f` is false for NaN and reproduces this exactly - rewriting it as `!(value >= 0.0f)`
  would erase NaN records instead, which is the opposite behaviour.
* The decremented value goes through a stack slot and is reloaded before both the write-back and
  the compare (`0072AD62`/`0072AD66`), rounding it to float precision. The write-back at `0072AD6A`
  is a non-popping `FST` and happens **before** the test, so it also lands on records that are
  about to be erased. Harmless, since they are immediately overwritten.

Erasing by swapping in the last element means **the list order is not preserved**, and the swapped-in
record is examined in the same pass - so a run of expired records at the tail is cleared in one
sweep. When the expired record *is* the last one, the copy is a self-assignment and the recomputed
end (`0072AD9A`..`0072ADA8`) equals the cursor, so the loop exits.

## What is NOT established

**What the 12-byte record means.** `0072AD40` copies the first eight bytes on erase without ever
reading them, so only the countdown at `+8h` has a proven interpretation. The payload could be two
32-bit fields, a pointer and an index, or anything else.

**The producer was not found.** A byte scan for `8d 8e 14 01 00 00` (`LEA ECX,[ESI+0x114]`) across
the image returns twelve hits, of which exactly one is in this gun segment - `0072D183`, the tick
call site itself. So whatever pushes records reaches the list another way, and finding it is the
follow-up packet. The segment's keyword set - `barreldelaytime`, `nextfirebarrel`, `loadtime`,
`throwa`, `throwb` - makes a queue of delayed per-barrel events the obvious hypothesis, but nothing
here tests that and it is recorded as a hypothesis only.

Descriptive names are hypotheses, not recovered symbols.

## Status

Reconstructed in `src/gun_pending_timers.cpp`, build-tested, and bound in
`src/game_hosts_gunnery.cpp`, which moves `Gun::base_tick_0072ad40` from `UNIMPLEMENTED` to
`concrete`. It is **not** game-validated, and cannot be: with no producer the list is always empty,
so the sweep is a no-op. The run confirms exactly that and nothing more -
`base_tick_timers_live=0 expired=0` - and `IJN01` at 500 ticks is otherwise unchanged to the digit,
`shots=74 first_shot=0.25 s` and `hull=3 total_damage=300.0`, which is the regression control.

The two counters exist so the always-zero is stated rather than assumed: a non-zero
`base_tick_timers_live` is the first sign that a producer has appeared.
