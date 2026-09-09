# Frame clock reconstruction

`src/frame_clock.cpp` reconstructs timestamp subtraction00530890, concrete
clock initialization00bedbd0, update00bedc30 and fixed-step selection00bedb20.
Application frame00737a50 updates this clock and consumes its interval as
float32; the clock is now available to the startup probe. The application frame
and game object are still unimplemented.

## Arithmetic and state

A timestamp contains signed 64-bit ticks and signed 64-bit frequency. Subtraction retains
the left frequency. Different frequencies rescale the right ticks using low64
multiplication followed by signed division; subtraction itself wraps modulo2^64.
Unsigned magnitude division preserves the native `INT64_MIN / -1` result without
C++ signed overflow. The unequal-frequency zero-divisor path explicitly executes
an x86 unsigned DIV by zero, matching the native processor exception class.
Exception register/context identity and recovery are not claimed. Equal zero
frequencies do not divide. Exact destination aliases are supported; arbitrary
partially overlapping timestamp objects are outside the typed interface.

Initialization clears pause/update count, reads QPC frequency and origin, and
runs two concrete updates. Update increments the 64-bit count, remembers previous
elapsed time, obtains QPC or advances the synthetic counter, then subtracts the
origin and previous elapsed pair. It uses native x87 FILD/FILD/FDIVP, float32
spill, reload and FCOMIP ordering for the negative interval gate. Strictly
negative intervals restore previous elapsed time and produce zero interval;
unordered ratios skip rollback. A paused update only self-subtracts its interval.

Fixed-step selection sets the mode, samples QPC into the synthetic counter, and
computes low64(frequency * signed milliseconds) /1000. The observed startup
caller supplies 50ms. It does not reset origin or elapsed state, so the first
fixed update also includes time between the last update and mode selection.
The next consecutive fixed update has exactly the configured increment.

## Boundaries

Native clock methods use ECX=this; initialization/update take no stack arguments,
while fixed selection takes signed int32 milliseconds and RET4. Timestamp
subtraction uses ECX=left, stack destination/right, returns destination in EAX,
and RET8. The new C++ interfaces are semantic projections, not ABI replacements.

`FrameClock` owns ordinary typed fields and constructor defaults. Its five
timestamp defaults match the observed zero/one pairs, but its initialized fixed
fields are explicit host defaults: native construction leaves those fields
uninitialized until the setter runs. Native singleton publication, critical
section/lifetime registration, vtable overrides and destruction remain unported.
The concrete method calls here do not emulate arbitrary virtual overrides.

Native code ignores QPC failures; host functions return false at a failed call,
retaining preceding mutations. Callers must stop on false. Successful API paths
preserve operation order. No new frequency/range normalization is introduced.
Pause/resume setters, mode-dependent sampling and timestamp addition are
documented in CLOCK_CONTROLS but remain to be integrated.

## Evidence and validation

STARTUP_NEXT_HANDOFF contains complete disk/live byte hashes for subtraction,
initialization and update. CLOCK_CONTROLS records the setter's 61-byte body hash.
TIMESTAMP_INTEGER_HELPERS records the compiler helper semantics and hashes.
All analysis/export batches verified project `bsp`, program
`/battlestationspacific.exe`. Four descriptive names and evidence comments were
applied, preserving prior values in ignored annotation logs, and the project
was saved. Correct CRT helper names were retained.

Run `python tools/verify_timestamp_reference.py` to compare 358 bytes from three
isolated native bodies against the saved project and generate the ignored local
reference header. After `./scripts/build.ps1`, `bsp_startup_probe.exe` executes
the copied subtraction/helper bodies with two relocated calls and compares 13
result/return/alias cases. All 13 passed, including modulo-product overflow,
equal-frequency subtraction wrap, negative truncation and MIN/-1. Divide-zero
exception execution and arbitrary partial overlaps were not tested.

The same existing probe passed real QPC initialization/update and one fixed 50ms
state sequence covering consecutive fixed intervals, backward-time rollback and
paused update preservation. These clock checks are reconstruction checks, not
native clock differential comparisons. Both existing CTests passed; no test
target or broad suite was added. None of this establishes game/runtime parity.
