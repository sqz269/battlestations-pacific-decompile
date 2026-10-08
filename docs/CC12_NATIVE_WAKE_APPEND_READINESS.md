# Native wake append readiness: 00810190 and 00810160

Read-only assessment, 2026-10-08. **00810190 is not ready for a faithful raw Source append packet. The first missing Source dependency is the raw ECX/x87 vector-length provider at 0042B2F0.** Its entire 77-byte, 31-instruction body is recovered and is a bounded next child. The existing public typed 3D helper and public typed 2D helper should remain intact.

The proposed address 00810160 is a separate 43-byte, 15-instruction sample-copy leaf. The append body is 00810190..0081062E: 1,183 bytes, 303 instructions, six calls. The copy leaf is independently Source-eligible after explicit authorization; 24 is its data size, not its code size. This assessment implements neither function.

## Evidence and scope

The exact bodies of append, copy, 0042B2F0 and 00414C60 were captured before and after analysis from the existing `bsp.gpr` / `/battlestationspacific.exe` project. Each CLI live query verifies project/program identity. All four bodies match the original installed PE, whose SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Independent Capstone decoding agrees with every Ghidra instruction start: 303 + 15 + 31 + 26. Seven constants and the three contiguous mutable reset cells are also pinned before/after. The cells lie beyond `.data` raw bytes and inside its virtual size; their initial zeroes are a loader projection, not a read-only PE constant.

Twenty worker source/config/tool inputs are unchanged. Existing worker and primary production objects, exact archive members, whole libraries, executable bytes, build registrations and source inputs are physically retained. This is inspection of actual compiled artifacts; no build, relink, probe execution, old fixture rerun, Ghidra mutation or new Source took place. Full bodies, relocations, selected executable bindings, call sites, x87 depth accounting and pins are in [the report](../reports/cc12_native_wake_append_readiness.json). Its ignored evidence family is `local/cc12_native_wake_append_readiness_evidence`; the sealed ZIP and its hash are recorded in the report.

## Native input and write contract

Append receives wake in ECX, followed on the caller stack by world-position pointer, heading float and yaw-rate float; it returns with `RET 0xC`. It preserves EBX, ESI, EDI and EBP. Ghidra's zero-parameter prototype is incomplete and is not the ABI contract.

| Raw input | Required preimage |
| --- | --- |
| `wake+0x008` | Forty actual 0x18-byte samples. Each has X/Y/Z at +0/+4/+8, heading +0xC, segment +0x10 and yaw rate +0x14. |
| `wake+0x3C8` | Actual head index; the bounded ring contract requires 0..39. Native code does not validate it. |
| `wake+0x3CC` | Actual byte flag. Every nonzero byte follows the set arm, not only byte 1. |
| `wake+0x3D0` | Three actual mutable residual floats. |
| Position pointer | Three readable float cells, read again after residual mutation on the decay path. Native load/store order matters if these alias wake storage. |
| `F87574/78/7C` | Three live mutable reset cells, read individually for equality and re-read individually when resetting residual. |

There are no native `appends`, `advances`, `merges` or `written` counters. Prefix bytes, padding and samples outside the selected writes must survive. The original routine re-reads head between several stores; a future implementation must retain the proven order rather than normalize the object into the current typed trail first.

## Whole append behavior

| Native span | Established behavior and boundary |
| --- | --- |
| 00810190..00810233 | Form position plus residual with x87 arithmetic and float spills. Form `(dx,+0,dz)` against current head; square/sum and spill to float. Return only for ordered `16 > squared_distance`. Equality at 16 and unordered comparisons pass. No external write precedes this gate. |
| 00810235..00810271 | Compare residual against each actual reset cell using FUCOMIP/LAHF/TEST; any unequal or unordered component enters decay. All three must be ordered equal to skip it. |
| 00810273..008102CD | Raw 0042B2F0 measures actual residual. Native `_CIsqrt` measures gate distance; multiply its float-spilled result by double 0.25 and spill again. `FCOMI travelled,length; JB` selects scaling for ordered less **or unordered**. Otherwise copy current reset-cell bits to residual in X/Y/Z order. |
| 008102CF..00810329 | `FSUBR ST(1); FDIVRP ST(1)` forms `(length-travelled)/length`, spilled to float. Interleaved x87 residual loads/stores apply it. Re-read position/residual and rebuild all point components, including the original duplicate float spill/reload sequence. |
| 0081032A..0081041E | Derive previous and two-back indices with wrap. Store head position, zero segment, heading and yaw. The vector passed to 0042B2F0 is `(newX-prevX,+0,newZ-prevZ)`; **Y is not part of this leg length**. |
| 00810422..00810476 | Use two-back X/Z to call intrinsic `_CIatan2` with ST0=delta-X, ST1=delta-Z. Spill angle to float, subtract from the pinned promoted-float half-pi in double precision, spill; add the pinned promoted-float two-pi once only for ordered negative. Store previous heading before testing segment length. |
| 00810476..008104C4 | Ordered old segment greater than new leg sets flag=1 and replaces segment. Ordered <= or unordered leaves that arm. If flag was zero, replace segment directly; if nonzero, enter the merge-span test. |
| 008104C6..00810583 | Raw 00414C60 measures planar span to two-back. Merge only for ordered `55 > span`; equality and unordered skip. Merge writes two-back segment, overwrites previous sample, calls 00810160 from next-head sample into current-head sample, then sets head to previous. It flows into the normal tail; the overwritten zero previous segment rejects advancement there. |
| 00810585..0081062E | Advance only for ordered previous segment > double 50 and flag=0. Wrap head at 39, write the new head before its sample fields, then set position, zero segment, heading and yaw with native re-reads. Return restores the native frame. |

Static whole-CFG x87 depth accounting covers every instruction. Append has zero net x87 stack change and a maximum of four occupied slots from an empty entry when the recovered leaf bodies are included. 3D length needs four local slots and returns one; 2D length needs three and returns one; copy needs one and returns zero. These counts exclude the private interiors of CRT math, for which only their intrinsic input/output contracts were used. They are static evidence, not a floating-point execution test.

The constants are float `CE6454=16`, double `D7A348=0.25`, double `CE3830=1.5707963705062866`, double `CE3828=6.2831854820251465`, double `D09438=55`, double `CE3938=50`, and helper double `CE3820=1e-10`. The report contains exact bytes and writable/section facts.

## Actual available providers and the first gap

| Dependency | Actual evidence | Readiness |
| --- | --- | --- |
| Raw 0042B2F0 | Original 77 bytes/31 instructions: ECX points at three floats, PUSH/POP preserves ECX, ST0 returns length. Its existing public `force_event_vector_length_0042b2f0` compiles to 116 bytes/37 instructions with a stack argument, MOVSS captures, changed product/read order, SSE COMISD and ordinary stack-argument `_sqrt`. | **First missing raw Source provider**, at append calls 00810273 and 00810416. The typed implementation is real but does not supply this raw x87/ABI/event contract. Preserve it and add a separately authorized raw child. |
| 00414C60 | Existing private `length_kernel` in `src/vector_helpers.cpp` is 66 bytes/26 instructions and equals the whole original except its two genuine relocations: cutoff and `_CIsqrt`. Its public cdecl bridge is 12 bytes/5 instructions. Five generic bridge byte patterns appear in each executable; resolving the real REL32 target selects exactly the bridge that reaches the unique kernel. | Math body already exists. A later append must bind the actual calling interface; do not label it missing or substitute a guessed generic wrapper. |
| `F87574/78/7C` owner | `bsp::game::game_native_geometry_globals()` is compiled and registered. Its six-byte getter has a real DIR32 relocation to a writable 12-byte uninitialized process object. `GameNativeGeometryGlobals` exposes a mutable array reference and preserves process lifetime. Both physical objects and exact archive members are retained. | Genuine Source storage is available. No existing append binding or application consumer is admitted by this audit. Do not replace it with literal zeroes or claim the owner is missing. |
| BF7030 / BF701A | Actual production executables import `_CIsqrt` / `_CIatan2`; their physical FF25 import thunks and IAT bindings are retained alongside ordinary `sqrt`/`atan2` bindings. | Genuine current intrinsic bindings are available. Original CRT dispatch policy, internal globals, diagnostics and whole-program initialization are outside this packet; no CRT interior parity is claimed. |
| 00810160 | Entire original 43-byte/15-instruction leaf, no helper calls/globals. It copies six floats via six ordered FLD/FSTP pairs. | Independently bounded Source candidate. Existing typed append uses MOVUPS + MOVQ bit copying; `native_unit_wake_copy` reconstructs 00815680, a different whole-ring copy. Neither is this leaf. |

The raw 3D child must retain the initial Y/X/Z x87 loads, ST-register multiplication/addition order, float squared-length spill, ordered/unordered `FCOMI` cutoff, intrinsic `_CIsqrt`, and both float store/reload pairs on the sqrt arm. The cutoff arm returns positive zero using XORPS/MOVSS/FLD. Squared length <= cutoff **or unordered** follows zero; do not use an ordinary C++ predicate as proof of identical signaling-NaN or FP-event behavior. The child does not need entity constructors, world settings or append implementation.

00810160 receives destination in ECX and source on the stack, moves destination to EAX, loads source into ECX, processes offsets 0/4/8/0xC/0x10/0x14 in order, and returns destination with `RET 4`. Its contract is neither snapshot copying nor memmove: partial overlap can propagate earlier stores, and even self-copy can quiet signaling NaNs and alter floating-point status. Native append supplies selected real ring records. A future raw leaf must preserve these semantics rather than reuse a six-word integer copy.

## Existing typed append is not admission evidence

The actual current typed append body is 1,169 bytes/261 instructions. Its existing linked body and four ordinary `_sqrt` calls plus one ordinary `_atan2` call are pinned. Source lines 124..140 compare/reset residual to literal zero despite the existing genuine mutable owner. Line 157 includes a Y delta in previous-leg length. Lines 38..46 explicitly omit native length cutoffs, and lines 184..188 use a typed record assignment whose compiled copy is MOVUPS/MOVQ. Ordinary formulas also do not retain all original float spills, unordered branches, alias order or native stack contracts. These differences establish why current typed compiled behavior cannot certify the raw append. No typed Source is changed in this packet.

The next bounded packet should implement only a separate raw 0042B2F0 provider, with exact source/artifact/ABI evidence and a proportionate native differential check after authorization. After that child is integrated, re-evaluate the copy leaf and append bindings. This report does not admit raw append, entity-constructor completion, executable startup, game behavior or gameplay validation.
