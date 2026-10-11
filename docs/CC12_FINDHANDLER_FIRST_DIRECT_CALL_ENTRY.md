# FindHandler first direct-call target: exact entry only

The exact saved entry at `00C07AC1` contains `6A 08`: one complete two-byte x86-32 `PUSH` of sign-extended immediate `8`. Two admitted byte reads, a manual physical-field decode, actual detailed Capstone 5.0.7 and context-zero saved text (`PUSH 0x8`) agree. No remaining instruction of the callee was read. The [report](../reports/cc12_findhandler_first_direct_call_entry.json) retains the physical fields, metadata and explicit validation limits.

The positive caller comes solely from the accepted immediate FindHandler BODY and PRIMARY reports. At `00C07655`, `E8 67 04 00 00` encodes signed displacement `1127`, return site `00C0765A` and numeric target `00C07AC1`. The packet manually recomputes those fields and runs actual detailed Capstone on those five supplied bytes. It makes no new query at the caller.

## Admission and retained metadata

The finite boundary is actual commit `022395d820ddaf58360b6c551b7d7657417e6a2b`: the immediate BODY and PRIMARY doc/report pairs, `config/target.json` and the three CLI source files. Actual Git blob, commit, root and parent-directory payloads are pinned. Source subtree IDs are revision identity only; Source contents and earlier evidence archives were not traversed.

Before any code bytes or saved text were read, Root independently checked both original direct metadata captures, all four aliases, the complete owner ranges, the finite actual Git objects and all four phase-one Native commands. Both responses identify an exact `InstructionDB` of length two and a saved `_inconsistency` owner at `00C07AC1`, with complete ranges `00C07AC1..00C07AE4` (36 bytes) and `00C07AEC..00C07AF7` (12 bytes). These 48 bytes are an owner metadata extent, not a decoded body. Saved flow is `NONE`/`FALL_THROUGH`, with numeric fall-through `00C07AC3` and no direct targets.

Root then admitted only paired exact two-byte reads, context-zero entry text and closing exact-entry/count checks. The closing raw metadata equals both original raw responses byte for byte. All three have epoch `98`; all four count checkpoints remain `64,730`. The ten Native commands verify the intended `bsp.gpr` and `/battlestationspacific.exe`. Complete command streams, wrappers and original HTTP response base64 are retained; the second phase passively records every HTTP response. No Native query or decoder failed. No prototype, next-address, body, neighbor, data or target query was made.

## Qualified CPU effect

These statements assume the instruction is reached and completes fault-free under the x86-32 interpretation. No operand-size prefix is present. Opcode `6A` has one signed immediate byte, `08`, which becomes the 32-bit value `00000008`; there is no ModRM, SIB or displacement field. The instruction performs one implicit stack write through SS. It does not alter EFLAGS or the other general registers locally.

**The ESP algebra additionally requires an actual 32-bit SS stack-address size.** Byte-decoder mode and operand width do not establish live `SS.B`. With that explicit condition and entry `ESP=T`, the entry leaves `ESP=T-4` modulo 2^32 and writes dword `8` at the resulting SS stack address. Its numeric next EIP is `00C07AC3`; no bytes at that site were read. A 16-bit stack uses SP semantics instead.

With the same conditions, if the physical caller E8 is reached with `ESP=S`, it first writes the encoded return `00C0765A` at `SS:[S-4]` and enters the numeric target with `ESP=S-4`. The entry PUSH then writes `8` at `SS:[S-8]`, leaving `ESP=S-8`. Each write uses its own instruction-time address and memory; this does not prove earlier memory remains unaliased or unchanged.

Nothing here establishes whether the remaining callee runs or returns. At any resumed `00C0765A`, all returned registers, EBP, ESP, EFLAGS and memory remain unconstrained. The saved name, `no_return=false`, ranges and flow do not establish service behavior, handler selection, EH/TLS/FH3/type identity, argument meaning, cleanup, provider, whole-function preservation or original ABI compatibility.

## Finite proof and limits

The retained package includes the Root gate producer/receipt and all 136 original metadata preimages as distinct copies, all actual finite Git objects, both final output blobs, physical decoders and full replay/auditor bodies. The supplied-ZIP audit allows one physical ZIP read, one pre-reviewed local helper execution and one exclusive receipt write. The replay does not execute archive-contained code. A distinct physical ZIP copy receives its own constrained replay; final commit/tree IDs and receipt pins accompany the handoff.

Nonexecuted helper drafts are preserved. During offline preparation, the context endpoint label and POST method were aligned with the actual retained response, and the auditor preloaded the standard ZIP filename codec before restricting imports. Those corrections changed no capture and caused no additional Native query. Any replay failure remains in a separate retained attempt stream.

This packet changes exactly this document and its report. It includes no C++ Source change, build, test, probe, GPR mutation, Original/game/OS/audio or runtime execution. Physical entry closure provides no whole-callee or original ABI credit.
