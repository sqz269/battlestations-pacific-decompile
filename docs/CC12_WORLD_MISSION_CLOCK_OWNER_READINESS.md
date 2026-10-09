# CC12 World mission-clock owner readiness

Baseline: `c0798f344901389adc6e9f1d503b185661adee3f`.
Read-only packet; no Source implementation and **zero new Original-function credit**.

## Decision

There is genuine mutable Source clock storage, but **no admitted common owner of
Native `00F876A4`**. `GameMissionFrameHost::Impl::fixed_clock.simulation_clock`
is written by the existing fixed-step projection. Reset `00874640` is not bound
to it, and the current World, frame, AI and unit hosts use separate time values.
Consequently this packet does not recommend or admit a production clock owner,
a new private timeline, or a binding of one of those copies as the World clock.

The supplied `00904600` consumer contract needs the same live float reloaded per
record. A by-value snapshot or a second independently advanced float cannot
establish that identity. This packet does not inspect, claim or reconstruct
`00904600` or `0042ED50`.

## Actual Native write authority

The complete current direct-xref response for `00F876A4` contains 226 references:
221 READ and five WRITE references. The five writes belong to two functions:

| Write | Authority | Effect |
| --- | --- | --- |
| `0087464C` | `00874640` | Store the caller's float bits into `F876A4`. |
| `00875C21` | `00875BB0` | Store the clock after subtracting the previous leftover. |
| `00875C80` | `00875BB0` | Publish the next simulation step before its callbacks. |
| `00875F26` | `00875BB0` | Add the final leftover back after the fixed-step loop. |
| `0087600B` | `00875BB0` | Store positive zero when the driver gate is closed. |

This inventories recognized direct references in the current analysis; it does
not prove the absence of unresolved indirect, alias, or external writers. The
report records code and constant bytes, not an initial or live `F876A4` value.
No PE/BSS zero-fill assumption is used as a clock initialization contract.

### Setter/reset `00874640`

The complete body is `00874640..00874690`: one stack float, `RET 4`; ECX is not
used. The original ABI is recovered from instructions, not Ghidra's current
`undefined(void)` prototype. Native behavior is:

1. Copy the input float bits to `F876A4` and `F876A8` with `MOVSS`.
2. Divide its x87 value by the binary64 step at `D7A270`; call `00BF7420` and
   store EAX at `F876B0`.
3. Reload `F876B0` with signed `FILD`; compute input minus count times step,
   and round/store the remainder to binary32 `F876AC`.
4. Subtract that stored remainder from the retained step and round/store the
   result to binary32 `F876B4`.

It does not write the stepping byte, buffer index or previous-buffer word.
This is a setter with arithmetic side effects, not an aggregate clear. For the
recovered zero inputs and zero conversion result, `F876B4` becomes the nonzero
step value; a value-initialized `FixedStepClock{}` is not that complete reset.
The conversion helper's general out-of-range/control-mode behavior is outside
this packet; no portable C++ cast replacement is admitted.

There are **six current call sites in five functions**, each verified to supply
positive zero through `FLDZ`, a stack slot, and `FSTP float [ESP]`:

| Call | Enclosing function | Bounded scheduling conclusion |
| --- | --- | --- |
| `004DAD52` | `BSP_Game_TeardownSessionState`, `004DA780` | Reset on this reached teardown path. |
| `004E0194` | `BSP_Game_LoadMissionScene`, `004DFB70` | First recovered reset site in mission load. |
| `004E1890` | `BSP_Game_LoadMissionScene`, `004DFB70` | Second recovered reset site in mission load. |
| `006B5A85` | `FUN_006B5960` | Reset on this reached path; broader caller role not recovered here. |
| `007714E7` | `FUN_007713A0` | Reset on this reached path; broader caller role not recovered here. |
| `00771FDC` | `FUN_00771FC0` | Reset on this reached path; broader caller role not recovered here. |

No whole-caller branch schedule or guarantee that both load sites run during
every load is inferred. The earlier `input_tick.hpp` comment saying one writer
and five read call sites is insufficient for the current writer inventory.

### Increment and gate schedule `00875BB0`

Its current caller is `004C40A0`. The driver discards incoming ECX by loading
`E188A8`; its float argument is callee-popped. The four gate tests require a
Game, the selected local-player slot, a positive signed short at slot `+10h`,
and, when Game `+1FE4h == 2`, session `+9Ch >= 8`.

A closed gate zeros only `F876AC`, `F876A4`, and `F876A8`. It does not perform the
setter's count/interpolation reset, clear the stepping flag or change buffers.
An open gate follows this publication order:

1. Subtract the previous `F876AC` from the clock and store `F876A4` as binary32.
2. Add the supplied delta separately to `F876AC` and `F876A8`, storing each as
   binary32. There is no wall-clock substitution or delta clamp in this slice.
3. For each admitted step, add the actual step constant to current `F876A4` and
   store it before the countdown, job waves and subsystem calls. Increment the
   DWORD step count; set the stepping byte and flip/save the buffer words.
4. After each wave/subsystem sequence, reload `F876AC`, subtract the step and
   store it. Zero `F876B4` per step. Re-test the freshly loaded remainder.
5. On leaving an entered loop clear the stepping byte. A no-step path bypasses
   that clear. Add the remaining accumulator to current `F876A4` and store it.
6. When delta and remainder are both above zero, run the interpolation wave
   with the remainder; only afterwards copy the current remainder to `F876B4`.

Thus observers inside a step see a different phase of the same clock from
observers after the leftover is added back. Advancing a host-local clock once
per rendered frame does not reproduce the reset, gate or intermediate schedule.
The packet pins the clock-affecting windows and does not recover the full job
scheduler or each callback's ownership/concurrency contract.

## Float and x87 qualification

`D7A270` is **not** the ordinary binary64 C++ literal `0.05`:

| Native storage | Bytes, little-endian | Exact value |
| --- | --- | --- |
| binary64 `D7A270` | `00 00 00 A0 99 99 A9 3F` | `0.05000000074505806`, `0x1.99999a0000000p-5` |
| binary32 `D0DE84` | `CD CC 4C 3D` | The same value when promoted to binary64. |

The existing driver's `static_cast<float>(kFixedSimulationStepSeconds)` yields
the same numeric step. No step-value mismatch is claimed. The header's standalone
binary64 literal has different bits from Native `D7A270`, so using that double
directly in a future Native calculation would require correction.

Native uses x87 loads, retained stack values, comparison flags and explicit
binary32 store/reload points. Portable arithmetic and `const volatile float&`
alone do not establish identical control-word, exception, NaN, evaluation or
calling-convention behavior. Volatile access is not a synchronization/lifetime
mechanism. This read-only packet supplies no native execution, compiler-output,
ABI, floating-point differential, or gameplay validation.

## Current Source storage and missing composition

| Current Source | What actually exists | Why it is not an admitted common owner |
| --- | --- | --- |
| `FixedStepClock::simulation_clock`, `in_mission_subsystem_tick.hpp:106` | Mutable aggregate field; both clock helpers write it. | New Source layout/interface; reset helper and shared consumer binding are absent. |
| `GameMissionFrameHost::Impl::fixed_clock`, `game_hosts_mission_frame.cpp:305,966` | The frame host passes this actual field owner to the fixed-step driver. | Gate/context remains a host projection; no `00874640` reset binding or public live-cell provider was found. |
| Frame host `world_clock`, lines `264,2886..2897` | Separately adds raw delta and copies time to frame/World/result views. | A different storage identity and write schedule from `fixed_clock.simulation_clock`. |
| World host `clock`, `game_hosts_world.cpp:74,342` | Separately adds scaled delta before the World pass. | Existing matrix host receives this copy; it does not alias the fixed-step clock. |
| AI `clock_seconds`, `game_hosts_ai.cpp:1144,6105` | Adds each supplied AI step. | Another independently advanced field. |
| `GameUnitsHost::mission_clock`, `game_hosts_units.cpp:15789` | Returns `summary.simulated_seconds` by value. | A summary input, not access to the common float cell. |
| `UnitGenericInputContext::mission_clock_00f876a4` | Existing `const volatile float&` consumer, read at `unit_generic_input_phase.cpp:153`. | No production context construction was found in `src/` or `include/`; the interface does not create or identify its owner. |

`GameMissionFrameHost` heap-owns its `Impl` via `std::make_unique` and destroys it
with the host. That establishes where the existing candidate field lives, not
that it outlives every proposed World/reference consumer or has all Native write
authority. No fresh owner lifetime is inferred from a numeric original address.

`MissionSceneLoadHost::reset_render_scene` is an abstract `00874640(0)` hook;
`mission_scene_load.cpp` calls it at both projected load positions. The inspected
Source has no concrete override implementing this reset over the candidate
clock. `mission_load_path.cpp` still labels that address as a renderer-owned row
with no source body. These names do not change the verified clock writes.

`GameStartupHost::ClockServices` is the existing actual frame-timer owner for
publication `01090AB0`/profile `D68D50`. It is a different object and time source;
its presence does not establish an `F876A4` alias or mission reset authority.

## Admission boundary

A future binding needs one established mutable cell, the setter and driver
writing that cell on their actual paths, and all admitted consumers borrowing
that same live cell for their full lifetimes. Existing owners must be reconciled
before selecting one; adding another clock or copying between clocks at frame
boundaries does not meet the contract. No new provider/signature is recommended
as ready by this packet. A supplied external live reference remains an explicit
unresolved application-composition precondition.

This packet changed only this document and its report. Verification comprises
13 live-Ghidra/installed-PE windows totaling **565 bytes**, all five recognized
writes, six zero-reset call sites, both exact constants, 15 physical/LF Source
pins, JSON decoding and diff checks. No Source, ledger, CMake, GPR or Original
credit changed; no build, test, probe or game run was performed.
