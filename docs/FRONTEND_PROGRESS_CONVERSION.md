# Loading progress uses an x87 conversion, not a clock

`report_loading_progress` now reconstructs the floating schedule of0057BEC0
and requires the existing BF7420 ST0 conversion service. The previous extra
integer `tick` argument, `kLoadingProgressDeadScale` and `last_tick` field names
were incorrect. The multiplication is live and the conversion consumes ST0.

## Evidence

Read-only Ghidra batches on2026-09-11 verified project `bsp`,
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 LE32,
base00400000 through `tools/bsp.py ghidra` before each batch. No Ghidra edits
were made. The current function is `BSP_LoadingScreen_ReportProgress`, a
descriptive hypothesis. Its whole body is0057BEC0..0057BF0F inclusive bytes;
the final instruction is `RET4` at0057BF0D. Original ABI: stdcall with one
stack float, no ECX input. All26 instructions exist and the flow audit reports
zero gaps, so no missing-function/free-gap repair is needed.

| Native instruction | Meaning |
|---|---|
| BEC1..BEC9 | Read singleton00E194B4; null exits without floating/conversion work |
| BECB/BECF | Load old+30 through x87 and spill float32 to the local; preserves native exception/NaN effects |
| BED5 | Load ORIGINAL incoming progress into ST0 |
| BED9 | Push old local onto ST0; incoming is now ST1 |
| BEDD FCOMIP ST0,ST1 | Compare old with incoming, pop old, leave incoming in ST0 |
| BEDF JBE / BEE1 / BEE9 | Select incoming in XMM0 for less/equal/unordered; old only for strict greater |
| BEEF FMUL qword[CEF258] | Multiply ORIGINAL incoming ST0 by actual double128.0 |
| BEF5 MOVSS [screen+30],XMM0 | Store the independently selected float BEFORE conversion |
| BEFA CALL BF7420 | Consume scaled incoming ST0 and return native signed low EAX |
| BEFF..BF08 | Re-read current+2C and update only when signed old is less than EAX |

CEF258's saved bytes are `00 00 00 00 00 00 60 40`. For finite ordinary input,
the two results are `+30 = selected_max(old_progress,incoming)` and
`+2C = signed_max(old_units,convert(incoming*128))`. The conversion operand is
not the selected maximum. For example old_progress0.75, incoming0.125 and
old_units-1 produce float0.75 and integer16. Equal zeros preserve the incoming
sign. Unordered comparisons select the incoming raw float bits, so neither
`std::max` nor the previous `if (incoming > old)` implements the native result.

## Existing conversion boundary

BF7420 reads actual0109EEA4 at conversion time. Its nonzero arm spills ST0 to
double and uses CVTTSD2SI EAX; the zero arm jumps to the existing BF7456 x87
conversion returning EDX:EAX. Their out-of-range/NaN behavior can differ.
This packet does not reinterpret, duplicate or rename either library kernel.
The existing full dispatch wrapper is currently private to gui_group_bounds.cpp;
the shared public BF7456 owner only covers its zero arm.

`LoadingProgressCrtAccess` therefore requires the actual scale pointer, actual
0109EEA4 pointer and an ST0 conversion function. At invocation ECX supplies
that global pointer, ST0 supplies the live scaled input, and EAX returns the
native low word. The service must read the current global itself and preserve
the established hardware/exception effects. There is no C++ cast, integer
clock proxy, cached dispatch mode or default fallback. A direct original
BF7420 entry may ignore ECX while reading the SAME bound native global.

The new progress kernel reproduces the caller's x87/SSE schedule without a
float/double spill of the scaled ST0 operand between FMUL and conversion.
Missing services throw for a nonnull screen; a null screen bypasses validation.
The existing `LoadingScreen` remains a semantic projection and this is not a
binary replacement. Host integration still must provide the actual conversion
service; no unrelated frontend host is silently supplied.

## Callers and consumers

The live graph has25 call sites in6 callers:004E3AA0 (12),004E4000 (1),
0051DDA0 (1),005CAAF0 (1),00686380 (8),006898C0 (2). Checked representative
sites004E4130..413A,006866C5..66CF and00689914..9924 load a progress float and
place that one float on the stack. They pass no independent time value.
Current C++ source has no concrete call to the old three-argument helper;
frontend manager and entry hosts expose their existing one-float abstract
methods, which are unchanged. No caller migration or additional caller lease
was needed.

The related reset was leased for the narrow field-name correction. At0057CE83
the begin-loading path reads00E194B4;0057CE89 calls0057C990 with that SAME
pointer. Reset stores+30=0 atC99A,+2C=-1 atC99F and+50=0 atC9A2. Consequently
`LoadingScreen.progress_units` and `LoadingHintRotation.progress_units` name
the same recovered native+2C field; the existing projections are not claimed
to be unified actual storage by this change. Hint tick0057C4C0 reads float+30
for travel and samples clock01090AB0/+20 against timestamp+38 for elapsed time.
There is no+2C read in that tick, and no additional consumer is inferred.

## Verification and limits

MSVC Win32 `/W4 /WX /fp:strict`, `scripts/build.ps1`, both existing CTests and
all8 seed byte matches passed. The ignored `local/frontend_progress_probe.cpp`
executes the saved original80-byte0057BEC0 body and unchanged28-byteBF7420 /
117-byteBF7456 library bodies in an isolated allocation with relocated data
operands. All three spans were independently byte-equal to the installed PE.
The library bytes are the required conversion service for the host comparison;
there is no alternate fixture conversion implementation.

One focused fixture compares16 runs across both actual dispatch modes, covering
decreasing reports, a fractional unit, both zero signs, previous/incoming quiet
NaN, incoming signaling NaN and infinity. Final float bits, signed units and
the complete x87 status word match; TOP is balanced and null-screen conversion
is skipped. These checks establish the selected caller behavior with masked
default FP exceptions; they do not claim all unmasked-trap or concurrent-global
scenarios, an installed host binding, ABI compatibility, rendering or gameplay.
No permanent tests were added. The report preserves old ledger names/comments
and the superseded clock/dead-multiply interpretation for review.
