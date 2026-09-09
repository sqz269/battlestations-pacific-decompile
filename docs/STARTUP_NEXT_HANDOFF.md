# Startup dependency handoff

Audit on 2026-09-09 against the current sources and saved `bsp` project,
`/battlestationspacific.exe`, x86 Windows, image base 00400000. The target was
verified before each live analysis batch. No program annotations were changed.

## Current boundary

The native route remains CRT entry -> WinMain 008f81f0 -> random registration ->
application constructor 00737970 -> initialization 0073d410 -> platform dispatch
00bea800 -> concrete loop 00bec1a0. Empty-queue frames call platform virtual +20h,
00bece70, which calls application virtual +10h, 00737a50, followed by cursor/focus
work 00becb20(false). Application frame passes a clock-derived float delta to
004e4a40 with game object `[application+14h]` in ECX.

Only the random subsystem and projected message loop are implemented on that
entry route. CMake still produces a static core and three diagnostic probes;
startup_probe initializes random state only. Native application construction,
initialization, window creation, XLive pretranslation, application frame,
game update, shutdown and destruction are not assembled into a game executable.
The separate renderer/material probes have advanced substantially but do not
close those startup dependencies. PLATFORM_LOOP and STARTUP_RANDOM contain
historical counts/next-step statements; GAME_FRAME supersedes their early frame
analysis. In particular, the concrete loop slot is already resolved.

## Next independently implementable engine dependency: frame clock

Application frame 00737a50 calls clock singleton 01090ab0 virtual +8h at
00737b01, then virtual +1Ch at 00737b0e. Initialization 0073d410 constructs it
through 00bedfb0 when absent. That constructor calls singleton base 00bede00,
installs vtable 00d68d50 and invokes initialization 00bedbd0. Live table entries:

| Slot | Address | Evidence-backed role |
| --- | --- | --- |
| +4 | 00bedbd0 | Read QPC frequency/start value and perform two updates |
| +8 | 00bedc30 | Advance clock and calculate frame interval |
| +1C | 00bee070 | Return pointer to interval at this+40h |
| +20 | 00bee080 | Used by rendering worker; not audited here |
| +24 | 00bedb20 | Called by application initialization; not audited here |

00bee070 is four bytes, `LEA EAX,[ECX+40h]; RET`. It is not currently in the
function inventory: attempting a normal exporter batch stopped at that address.
Read-only raw disassembly and disk/live byte comparison establish this getter;
the parent can create/name it in a coordinated annotation batch.

00bedc30 takes this in ECX, no stack parameters, returns with plain RET. It uses
one arithmetic dependency, 00530890, plus QueryPerformanceCounter. Its fields:

| Offset | Observed representation/use |
| --- | --- |
| +4 | Float accumulated by game update, initialized zero by constructor |
| +8 | 64-bit update counter, incremented with ADD/ADC |
| +10 | 16-byte starting timestamp/frequency pair |
| +20 / +30 | Current / previous elapsed timestamp/frequency pairs |
| +40 | Current frame interval pair |
| +60 | 64-bit QueryPerformanceFrequency result |
| +68 | Paused gate: replace interval by interval minus itself and return |
| +69 | Fixed-counter mode gate |
| +70 / +78 | 64-bit increment / synthetic counter in fixed-counter mode |

On an ordinary update, increment the counter, copy current to previous, query
QPC (or advance the synthetic counter), subtract the starting timestamp, then
subtract previous elapsed time to obtain the interval. Convert the interval's
signed int64 numerator/denominator with x87 FILD/FDIVP and spill to float32 for
the negative test. Only a strictly negative result restores current from previous
and recomputes a zero interval. Preserve the assembly's comparison behavior for
NaN; do not replace this with a broad nonpositive clamp. Pause does not query QPC
or increment the update counter.

Initialization 00bedbd0 zeros +68 and the update counter, reads frequency and
counter, writes the starting pair, and calls virtual +8 twice. Constructor
00bedfb0 initializes five pairs with zero numerator and denominator one, clears
+68/+69, then invokes initialization. It does not initialize the fixed-counter
fields +70..+7F in the observed constructor; recover their setter before exposing
that mode. Base 00bede00 publishes the singleton under the existing manager's
optional critical section and registers its lifetime. A typed clock can recover
arithmetic/Win32 behavior before the native singleton manager is reconstructed,
but must explicitly retain that ownership boundary.

## Smallest arithmetic prerequisite

00530890 receives left pair in ECX, destination then right pair on the stack,
returns destination in EAX and `RET 8`. A pair is signed 64-bit ticks followed
by signed 64-bit frequency. If frequencies match, subtract ticks modulo 2^64
and retain the left frequency. Otherwise multiply right ticks by left frequency
with the native low-64-bit multiply helper 00bf7df0, signed-divide by right
frequency through 00bf7d40, and subtract from left ticks modulo 2^64. Output
frequency is left frequency. Pseudocode incorrectly declares a void return;
assembly retains the destination in EAX in both branches.

This helper and the QPC update form a bounded next implementation. Verify native
integer helper exceptional behavior before claiming divide-by-zero or signed
overflow parity; ordinary C++ signed multiplication risks undefined overflow.
Use exact x87 spill points for delta conversion. The application call site also
spills/reloads the ratio as float32 before passing it to GGame::OnMove.

## Byte evidence and limits

These inclusive ranges match the installed executable byte-for-byte in the saved
project. SHA256 values cover exactly those ranges:

| Range | SHA256 |
| --- | --- |
| 00bedfb0..00bee04b | 1028ad2523d9b8e1057b5d6a03c405d8423a114343d4130251eb10057febd753 |
| 00bedbd0..00bedc27 | b612bf106a4c1720cd7048c10c021a623787f18fc1f91df067d27d8234937fab |
| 00bedc30..00beddb3 | 79a978ad1f7ad4e56891cc0626cfa01e9f60b96af08732c32a217dbd6e42a05c |
| 00530890..00530917 | 225903975901288525f2d40f026081b5620248dd93106930fd06e6217d2b9b42 |
| 00bee070..00bee073 | 9b5251af6794c76b2f519f4601eefe7c7f18622726ef5633ff8713cbcdf488fe |

No C++ changed and no build or new test was required for this audit. Clock
reconstruction alone will not run an engine frame: profiling helpers, input,
game state construction/update, resources and complete renderer lifecycle remain
real dependencies. The stored Ghidra body of 004e4a40 remains truncated; use the
existing bounded CFG evidence in GAME_FRAME and GHIDRA_BODY_REPAIR instead of
treating its two-instruction assembly export as the complete game update.
