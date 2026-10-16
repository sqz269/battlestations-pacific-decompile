# Raw avoid-zone winding and x87 math

This packet reconstructs complete physical bodies at 0041A200, 00419260 and
00438B10. Descriptive names remain hypotheses. The new interfaces operate on
actual raw storage and use genuine current Win32 UCRT `_CIatan2`/`_CIsqrt`
entries with their hidden x87 operands. Existing typed implementations in
`ship_ai_lateral_record.cpp`, `native_vector2_math.cpp` and `unit_rudder.cpp`
are preserved. Current CRT service is an explicit boundary; it does not finish
the Native private CRT dispatch, state or exception reconstruction.

| Original | Complete body | Original physical ABI | New Source entry |
|---|---|---|---|
| BSP_AvoidZone_DeriveCornerGeometryAndWinding, 0041A200 | 729B through 0041A4D8; 726 typed bytes plus skipped 3B padding | ECX actual zone; one stack DWORD, low-byte flag; AL bool; RET4 | `derive_native_avoid_zone_0041a200(void*, void* unused_edx, uint32_t)` fastcall |
| BSP_Vector2f_ReciprocalLength, 00419260 | 117B through 004192D4 | ECX two-float vector; no stack args; ST0 result; RET | `reciprocal_length_native_vector2_00419260(const float*, void* unused_edx)` fastcall |
| BSP_Math_SubtractWrappedAngle, 00438B10 | 110B through 00438B7D | two stack float words; ST0 result; RET8 | `subtract_native_wrapped_angle_00438b10(float, float)` stdcall |

EDX placeholders are Source adapters only and are not Original arguments.
Original annotation proposals are pointer-thiscall plus byte flag/bool result
for derive, one consumed ECX pointer for reciprocal, and stdcall two floats for
angle. Live derive/angle prototypes were still `undefined name(void)` at capture;
the saved reciprocal prototype already described ECX/ST0 correctly. No Ghidra
mutation occurred here. There are no `noexcept` declarations: absence of a
Native EH prologue does not establish the current CRT handler's C++ contract.

## Whole derive body

Owner+0 holds the actual DWORD pointer-array base, owner+4 its DWORD count.
Each cell points to record storage: x/z +0/+4, outgoing unit +8/+C, bisector
right normal +10/+14, outgoing length +18, signed turn +1C. The producer's
full record is 24h, including clearance+20. This entry allocates/frees nothing
and performs no virtual dispatch. No owner, record, count or alias repairs are
introduced.

The literal schedule preserves base/count and current-record reloads, previous
and next wrap arms, modulo-DWORD end arithmetic, x87 coordinate snapshots,
MOVSS next-coordinate snapshots, every binary32 spill, cutoff comparison and
unconditional division by the selected outgoing length. The outgoing squared
length retains both squares extended until its single sum spill, unlike the
reciprocal child's separate square spills. Unordered or <=double-cutoff selects
zero length; outgoing normalization still divides by that zero.

Two inlined heading sequences call atan2 with ST0=x/ST1=y, spill to float,
subtract from float-rounded pi/2 stored as double, spill, compare via FLDZ and
FCOMIP, then conditionally add float-rounded 2*pi with another float store.
There is no Native CALL to the higher-level heading helper. The wrapped-angle
call consumes outgoing then incoming heading as two stack words. Signed-turn,
normal and accumulation stores retain their exact later record reloads.

Total turn is rounded to binary32 after each addition. Final COMISS/JBE accepts
nonpositive or unordered sum with AL1. Initial end==base also returns AL1,
including count*4 wrap. Positive sum returns AL0. If the flag's low byte is
nonzero, the signed-index reversal swaps pointer cells, reloading base between
the two stores, and calls the same complete public derive entry with flag0.
Its result is discarded; the original pass still returns AL0. Recursion is
bounded by that zero flag. EAX high bits carry no return contract.

The real JMP at 0041A49B skips `8D 49 00` at 0041A49D..49F. Those three bytes
are retained explicitly. The complete body has no SEH registration or C++
throw/catch/EH prologue. Ambient x87/MXCSR state is not reset. Raw aliases
observe the native load/store sequence, including changes reachable during a
CRT handler call; no typed reference snapshot replaces those reloads.

## Math children and immutable data

The reciprocal leaf retains all 117 bytes of its native schedule, including
the square/sum/sqrt spills and FUCOMIP/LAHF/TEST AH,44h/JP selection. Its only
call is now the real current `_CIsqrt` entry. It does not borrow
`CameraAxesCrtAccess`. The angle leaf retains all 110 native bytes, repeated
x87 FADD/FSUB/FSTP wraps and RET8. This also preserves x87 control/status
behavior that the existing typed SSE2 wrap loops do not establish. No fmod,
finite-input guard, iteration bound or ordinary double bridge is added.
Infinities and sufficiently large finite angles can fail to make progress.

All read-only operands reference exact immutable bit payloads checked against
live Native memory and the installed executable:

| Native data | Bit payload | Role |
|---|---|---|
| 00CE3820 | 3DDB7CDFD9D7BDBB | double squared-length cutoff, 1e-10 |
| 00CE3828 | 401921FB60000000 | float-rounded 2*pi carried as double |
| 00CE3830 | 3FF921FB60000000 | float-rounded pi/2 carried as double |
| 00CE3D18 | C00921FB60000000 | negative float-rounded pi carried as double |
| 00CE3D28 | 400921FB60000000 | positive float-rounded pi carried as double |
| 00D7A218 | 00000000 | float +0 |

Native BF701A uses descriptor E15C20, `__cintrindisp2` C07EE0, classifier
C081C7, a finite FPATAN/quadrant path and C07FAA restoration/exception routing.
Native BF7030 retains ST0 while spilling a double and enters its classifier
and sqrt worker. Both have native policy/state beyond their tiny entries,
including 0109DD78 and the `__87except` family. These library names/evidence
remain intact, and neither Native library entry is newly marked reconstructed.

Source calls the actual current UCRT x87 intrinsics directly, with no extra
operand spill or invented mutable CRT globals. Local x86 `ucrtbase.dll`
10.0.26100.9711 and SDK 10.0.26100.0 import library provide them. The current
atan2 dispatcher preserves both inputs in 80-bit cells around a policy query;
current sqrt retains ST0 and reaches FSQRT. Current CRT errno, matherr,
diagnostics, dispatch globals and exception behavior remain its own policy.

## Verification and integration boundary

Guarded CLI queries verified existing `C:/Users/sqz269/bsp.gpr`, project `bsp`,
program `/battlestationspacific.exe`, x86 base00400000. Fresh live captures of
all three complete bodies and three data ranges match the installed game PE.
Typed metadata records derive's exact split ranges and no-return=false.

Strict MSVC Win32 `/MD /O2 /W4 /WX /EHsc /fp:strict /permissive-` compilation
passed. COFF comparison verifies **956 physical bytes, 884 fixed bytes and all
18 bindings**: eight real CALL REL32 operands and ten read-only data operands.
Internal calls resolve to defined actual public raw functions, including the
recursive derive call. Data bindings prove the actual section payloads and
read-only characteristics. External calls resolve only to real `__CIatan2`
and `__CIsqrt` symbols. Every unmasked byte, RET and skipped padding matches.

One ignored combined fixture links the actual Source object and real CI imports.
It compares complete captured Native bodies rebound to those same current CRT
services against Source for two full positive-winding flag variants and five
math cases. It checks false return after recursive reversal, low-byte-only flag
handling, pointer/owner/position/clearance retention, DWORD-wrapped initial end,
signed-pi boundary, repeated wrap, zero/finite/SNaN reciprocal inputs, result
bits and x87 exception/TOP/CW observations. Opposing x87 and MXCSR rounding
modes expose typed SSE substitution. All checks passed. The emitted executable
imports `_CIatan2` and `_CIsqrt` from `api-ms-win-crt-math-l1-1-0.dll`.

Ignored helpers under `local/goal15i_avoid_winding/` support the primary's final
registered verification: `verify_coff.py --object <registered.obj> --output
<receipt.json>` and `probe_registered.cmd <actual bsp_core.lib>`. No fixture
framework, target or callback provider was added. The worker repository build
and its two existing CTests passed. The new module was unregistered in that
build; its own strict compilation, byte proof and fixture are separate.

The eight exact body CALL rows are in the report. Derive incoming physical
census contains only self 0041A4BE and constructor0041D063. The constructor
calls clip at0041D053 before conditionally invoking derive with flag1; clip
does not itself call derive. Existing names, typed Sources and earlier ledger
records are retained. Primary registration, final archive linkage, annotation
and export are separate. The controlled fixture does not establish Original
CRT/global/EH identity, unmasked-exception behavior, actual game adoption or
gameplay correctness.
