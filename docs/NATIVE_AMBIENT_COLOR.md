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

Main integration/build remains separate. No permanent tests, owner lifecycle,
gameplay or native caller ABI are claimed.
