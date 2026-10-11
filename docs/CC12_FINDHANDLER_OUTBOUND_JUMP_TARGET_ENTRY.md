# FindHandler outbound jump target: exact entry

The exact saved entry at `00C07A75` is `6A 08`, one complete two-byte x86-32 PUSH of sign-extended immediate `8`. Paired exact bytes, paired context-zero text, manual physical fields and actual detailed Capstone 5.0.7 agree. The remaining owner bytes were not read. The [report](../reports/cc12_findhandler_outbound_jump_target_entry.json) retains the physical fields, complete saved metadata, raw command evidence and validation limits.

The positive edge comes solely from the accepted immediate BODY and PRIMARY pair: `00C07AF3:E9 7D FF FF FF`, signed displacement `-131` from numeric next `00C07AF8` to `00C07A75`. This is a near JMP, with no return-address push and no change to current ESP. Manual decoding and detailed Capstone separately check those supplied five bytes without a new source-edge query.

## Admission and metadata

The finite boundary is actual Git commit `5bf637888ab41abff1da77d0335c7a467c59dfa3`. Only the immediate BODY and PRIMARY doc/report pairs, `config/target.json` and the three CLI files supply the eight positive Git inputs. Their actual blob payloads, parent directories, root and commit are pinned, with working-LF equivalence. Source subtree IDs identify the revision only; Source contents and previous evidence archives were not traversed.

Root's metadata gate checked both original direct responses before byte/text admission. All four aliases identify an exact two-byte InstructionDB at `00C07A75`. Saved function-at and containing-owner entries agree there, named `terminate`, nonthunk and `no_return=false`. Its complete saved extent is 44 bytes: `00C07A75..00C07A94` (32) and `00C07A9C..00C07AA7` (12). This is metadata, not a decoded owner body. Saved flow is `NONE`/`FALL_THROUGH`, numeric next `00C07A77`, with no direct targets.

Root admitted only this two-byte unit, paired context-zero entry text, closing exact-entry metadata and count/identity bookends. Closing raw metadata equals both originals byte for byte. Eleven Native commands and 39 original HTTP responses preserve epoch `98` and four function-count checkpoints at `64,730` for `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. No query was made at `00C07A77`, the remaining owner, its gap, any target, prototype or provider.

## Qualified local effect

These statements assume the observed encoding is reached and completes fault-free under the x86-32 interpretation. Opcode `6A` has one immediate byte `08`, sign-extended to dword `00000008`, and no prefix, ModRM, SIB or displacement field. It performs an implicit stack write through current SS. It does not locally change EFLAGS or the other general registers.

**The entry's ESP equation requires an actual 32-bit SS stack-address size, imposed anew at this instruction.** Decoder mode and operand width do not authenticate live `SS.B`. Prior opaque calls leave all registers, EBP/ESP, segments, flags and memory unconstrained. A 16-bit stack uses SP semantics.

With that stack condition and entry `ESP=T`, PUSH leaves `ESP=T-4` modulo 2^32 and writes dword `8` to the resulting current SS stack address. If the supplied E9 and this entry execute consecutively, current `ESP=S` at the E9 remains `S` on entry; PUSH then writes at `SS:[S-4]`. The jump contributed no additional return-address word. This establishes no call argument, original caller frame, earlier stack identity or preservation contract. Numeric next `00C07A77` was not queried.

Saved `terminate`, its flags and its owner ranges establish no library service, nonreturn, provider, handler, EH/FH3/TLS/type or original ABI identity. The local JMP and PUSH effects do not constrain the behavior of the remaining owner.

## Retained proof and failure provenance

All 156 original metadata preimages and 167 available Root artifacts are copied and pinned. Root recorded two offline gate failures and three offline inspection failures, with no Native or GPR effect. The first gate producer is an exact reconstruction from the original tool-call code, as Root disclosed. **The first gate failure's separate local stdout/stderr were overwritten by attempt two.** Root reports its complete original tool output remains in the conversation; this packet does not claim or synthesize separate original local streams for that attempt. Attempt two's actual local streams, the successful gate's streams and the failure-history/preservation producer are copied unchanged.

Worker Native captures and the manual/detailed decoder passed first execution without failure. Full helper bodies and actual frozen/final Git objects accompany the finite ZIP. Each strict replay permits one physical ZIP read, one reviewed pinned local helper execution and one exclusive receipt write; it tracks denied events, rejects other events and executes no archive-contained code. The pure helper compares already-captured detailed Capstone data with manual fields; it creates/imports no Capstone instance within the audit. A distinct physical ZIP copy is verified and replayed separately. Final Git IDs, copy identities and receipt pins accompany the handoff outside the self-referential report.

Exactly this document and its report are committed. No Source change, build, test, probe, GPR mutation, Original/game/OS/audio or runtime execution belongs to this packet. Exact entry closure gives no whole-owner or original ABI credit.
