# Avoid-zone group query follow-up

Addresses: `0041B4B0`, `0041B580`, `0041B7E0`.

These are distinct, previously unreconstructed function entries. All three are
complete control-flow semantic projections using existing geometry, boundary and
tangent implementations. Names are hypotheses. No other routine is reconstructed
or renamed by this packet. Ghidra annotations are left to the primary integrator.

| Entry and inclusive body | Proposed name | Original ABI | Coverage |
| --- | --- | --- | --- |
| `0041B4B0-0041B4D6` | `BSP_AvoidZoneGroup_AnySegmentHit` | ECX group; stack toward/from; AL result; `RET 8` at `0041B4D4` | complete |
| `0041B580-0041B677` | `BSP_AvoidZoneGroup_WeightedBoundaryDirection` | ECX group; stack point/slack/full-weight-distance/out; ST0 minimum distance; `RET 10h` at `0041B675` | complete through existing required bindings |
| `0041B7E0-0041B83E` | `BSP_AvoidZoneGroup_SlackTangent` | ECX group; stack out/point/slack; EAX out; `RET 0Ch` at `0041B83C` | complete through existing required bindings |

`0041B4B0` supplies scratch point/zone/edge outputs to the existing group segment
query. The two input stack slots are reused as scratch output slots after their
pointer values have been pushed. The callee's AL result survives the wrapper's
stack restoration, so the decompiler's `void` signature loses its useful result.
The semantic wrapper returns the existing `AvoidZoneGroupHit::hit` field.

`0041B580` clears the output vector, starts the minimum at `FLT_MAX`, and walks
the zones in native order. For each it obtains a distance and boundary direction,
then computes `00419010(full_weight_distance, 1, slack, 0, distance)`. Only an
ordered positive weight contributes. The sum is not normalized. Equal interpolation
endpoints preserve the existing callee's constant-one arm; this wrapper adds no
range repair or endpoint ordering. Empty groups return `FLT_MAX` and positive zeros.

The minimum branch at `0041B648` is `JA`: replace on equality or unordered, as
well as on a smaller value. This differs from the decompiled `distance <= minimum`
expression for NaNs. A NaN distance replaces the running minimum, and a later
ordered value replaces a NaN minimum. Equality also preserves the later value's
signed-zero bits. Weight comparison `FCOMI` / `JBE` at `0041B60E-0041B612`
skips zero, negative and unordered weights.

The multiply at `0041B618` has bytes `D8 C9`, so ST0 is multiplied by ST1;
there is no DC-form destination ambiguity. Both products are separately stored
to float at `0041B61A` and `0041B622`, before the additions and their float stores
at `0041B62C` and `0041B635`. A small x87 assembly block preserves that schedule
and avoids a fused multiply-add or a retained product across an addition.
The x87 DC-form scan of the candidate body found no DC register-form sites.
`00D7A248` bytes `FF FF 7F 7F` establish the float maximum sentinel.

`0041B7E0` clears only the output's valid byte, then folds each existing per-zone
slack tangent through the existing optional-vector merge in zone order. It uses
the already declared `Cc13AvoidZoneOptionalVector`; no competing layout is added.
An empty group or all-invalid contributions leave the caller's x/z and padding
untouched. A valid result still leaves padding untouched. Merge order matters
because the existing merge can reverse a contributing tangent before summing.

| Containing function | Call site | Native callee | Contract read and reused |
| --- | --- | --- | --- |
| `0041B4B0` | `0041B4CC` | `004179D0` | Full group segment-hit body; toward/from, three scratch outputs, AL boolean, callee `RET 14h` |
| `0041B580` | `0041B5D0` | `0041AEA0` | Full body; point/slack/out-direction, ST0 distance, `RET 0Ch`; existing boundary host bindings |
| `0041B580` | `0041B5FF` | `00419010` | Full body; five float stack arguments, ST0 result, `RET 14h`; existing x87 interpolation and clamp |
| `0041B7E0` | `0041B815` | `0041B290` | Full body and existing C++; zone tangent, out/point/slack, `RET 0Ch`; returned EAX is the temporary consumed by the merge |
| `0041B7E0` | `0041B81E` | `0041B700` | Full listing and existing C++; ECX accumulator, EDX addend, bare `RET`; invalid preservation and direction alignment |

The live `bsp.py` commands verified the configured `bsp` project, program
`/battlestationspacific.exe`, x86 language and image base before each query/export.
The configuration names `C:/Users/sqz269/bsp.gpr`. Native body spans were checked
with live `proto`; decompilation and assembly were read in full for the candidates.
Live xrefs reported no references to any of the three entries. Existing nearby
arc-hit, distance and merge reconstructions are reused without modification.

The vector/zone collections are borrowed semantic snapshots, not native arrays.
They and the point must remain stable during host calls; output and point must
not alias. Native loops reload the group pointer/count after each callee, whereas
the snapshot loops require stable membership. Polygon validity and host/CRT
requirements remain those of the reused interfaces. There are no invented global
values or unresolved-call stubs.

Validation is recorded in `reports/oct15_avoid_tangent.json`. `./scripts/build.ps1`
passed on source commit `2f362246a`, including `reconstructed_math` and `tool_tests`
(2/2); native differential fixtures were not configured in this isolated worktree.
All five direct call rows and the staged diff check passed. Compilation
and existing checks establish source/build evidence only. No new tests, original
byte differential execution, native ABI replacement, game wiring or gameplay
validation is claimed; floating-point exception/status equivalence is not claimed.
