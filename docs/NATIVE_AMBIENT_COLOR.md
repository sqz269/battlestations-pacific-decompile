# Native ambient color

This packet reconstructs complete 4FB850 (159 bytes) and B84C60 (4 bytes).
Names describe their observed behavior; they are not recovered symbols.
B84C60 takes ECX owner and returns owner+8 in EAX without dereferencing the
owner. It supplies a borrowed payload address and adds no lifetime operations.

4FB850 originally takes ECX destination and one stack source-float4 pointer,
returns the destination in EAX, and uses RET4. The new fastcall interface takes
destination/source in ECX/EDX and explicitly borrows the actual CE4B48 double
scale and mutable0109EEA4 conversion-mode storage on the stack, using RET8.
It is not a drop-in replacement for native callers.

The first FLD reads R before loading the actual double scale (original255.0).
The original DC C9 multiplies ST1 by ST0, then FXCH places scaled R on top
while retaining the scale beneath it. Green and blue each load their current
source float and multiply by the retained ST1 scale. Alpha multiplies the
remaining scale directly by source+0C, consuming the final scale at conversion.
There is no float4 snapshot or intermediate float/double narrowing.

Each channel calls the existing complete ST0 provider
native_crt_truncate_st0_00bf7420 with the actual mutable mode address. It reads
the current DWORD separately on every call: zero tail-calls the complete
BF7456 x87 converter; nonzero uses the native FSTP-double/CVTTSD2SI path.
The color converter clamps the signed low EAX result to0..255 and writes
R,G,B,A at destination bytes2,1,0,3. EDX's high integer word is not consulted.
Those ordered writes can change later source loads or the borrowed mode/scale
storage through overlap. The scale is read once; the mode is read four times.

The implementation adds no validation, saturation before conversion, floating
control policy, exception recovery, or rollback. A hardware exception or
access fault can leave already-published bytes and native x87 state intact.
The explicit raw source and destination must be valid for the accesses actually
reached. Getter arithmetic retains x86 address wrapping without dereferencing.

The strict worker Win32 build, both existing CTests and eight original seed
checks passed. One ignored original-body probe compares eight paired cases
against the complete worker library: finite values through both dispatch paths,
source overlap, a red write that changes the following conversion mode, retained
scale despite scale-storage aliasing, masked NaNs/infinities and low-EAX behavior,
and a blue-load access fault preserving the prior red/green stores. It compares
978 observable bytes and retains all16 complete624-byte raw results, including
FXSAVE state. The getter also matches original address wrapping.

Whole linked COFF sections and relocations, two exact archive objects and17
immutable original/source code stages were verified. Only the live x87 registers,
control/status/tag and MXCSR fields are compared; raw instruction/data pointers
and inactive register contents are preserved as evidence without an equality
claim. No unmasked floating exception dispatch was tested. See the audit report
for the isolated sealed artifact path and hashes.

The source is integrated into main. Its strict Win32 build and both existing
CTests passed, followed by independent primary original-body replay against
the actual main library. No permanent tests, owner lifecycle, gameplay or
native caller ABI are claimed.


## Primary validation

The primary reviewed the source and fixture and verified 51 immutable worker
pins plus 14 literal current source/header matches. Both actual archive
objects were extracted from the frozen main library
`348707f9e0d0ebd36608df65194c67df071da59b351cc9197f2dc42509ddb362`.
Their code/directive sections and relocations match the worker objects;
compiler debug/checksum metadata and anonymous-namespace scope hashes are
recorded separately. The fixture never recompiles either provider.

Six fresh guarded Ghidra/installed-PE spans cover 320 bytes, and all eight
native seed ranges matched. The unchanged eight-case probe passed again:
978 compared observable bytes, 16 preserved raw results, 171 complete linked
COFF sections and 17 complete code postimage stages. An independent parser
reproduced every compared byte from the raw results. The original converter,
getter and full original dispatch/fallback execute without provider bridges;
only the scale and mode data operands relocate in the private original image.

The existing Ghidra names and prior comments were preserved, reviewed
evidence was appended and saved, and both exports and complete function
records were refreshed. The tracked audit pins the read-only primary bundle
at `local/ambient_primary/`. Runtime limits listed above remain unchanged.
