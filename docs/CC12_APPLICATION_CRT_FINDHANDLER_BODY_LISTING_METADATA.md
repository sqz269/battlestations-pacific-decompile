# CC12 application CRT saved FindHandler body: physical listing and operands

The saved owner at `00C07623..00C07978` is a contiguous 854-byte physical body containing 260 complete x86-32 instructions. A manual opcode/ModRM/SIB/immediate decoder, actual detailed Capstone 5.0.7, and context-zero saved instruction text agree at all 260 established starts. This closes physical extent and operands with a qualified local CPU contract. The saved `FindHandler` name, imported EH types, handler-selection meaning, and original handler ABI remain unauthenticated.

The machine-readable result is [the report](../reports/cc12_application_crt_findhandler_body_listing_metadata.json). It contains every physical instruction, encoding field, numeric operand, saved text, and saved flow flag. Complete raw captures, producers, Root gate preimages, actual Git objects, and an audited supplied-ZIP replay are retained under `local/cc12_application_crt_findhandler_body_listing_metadata/`.

## Admission and physical evidence

The finite Git boundary is frozen commit `40ba8f8bcfd8aa77e81699c63c48734ac7eb11d7`. It contains only the immediate ENTRY and ENTRY_PRIMARY doc/report pairs, `config/target.json`, and the three existing CLI tools. Source subtree IDs are revision identity only; no Source contents or prior evidence archives were traversed.

The first pass derived each next query solely from the complete returned InstructionDB length inside the positive saved range. Its 260 direct responses were paired in five bounded batches. Root independently replayed all 520 raw responses, all four aliases, the complete owner range, 269 commands, and the exact contiguous boundary before admitting the body reads. The derived end `00C07979` and instruction interiors were never queried.

The admitted phase then read exactly 854 bytes twice, obtained context-zero text at those same 260 starts, and repeated their typed metadata. Closing raw responses equal both initial responses at every start. Across 283 Native commands, all six function-count checkpoints remained 64,730; typed modification epoch remained 98; project/program verification used `bsp.gpr` and `/battlestationspacific.exe`. There were no GPR writes, Native failures, fresh prototypes, decompiles, target-following, or neighboring/data queries.

The exact physical-byte SHA-256 is `81e644d42d247ba81c41ca69869f6c871666f51884cf0e23af28c2e5521cda66`. There are 30 direct E8 calls to 15 numeric addresses, 45 conditional jumps, five unconditional jumps, and one final RET. All in-body direct targets are established complete instruction starts. The only outbound jump is `00C07761 -> 00C07A75`. No outbound target bytes or metadata were read.

## Saved override boundary

| Site | Physical bytes and operation | Qualified result |
| --- | --- | --- |
| `00C07761` | `E9 0F 03 00 00`, JMP to `00C07A75` | Transfers without a return-address push. Saved `CALL_RETURN` does not make this an E8 call or establish target behavior. |
| `00C07795` | `E8 EB F0 FE FF`, CALL to `00BF6885` | Pushes encoded return site `00C0779A`. Saved `CALL_RETURN` does not establish nonreturn. The bytes at that next site are present and decoded. |

The complete physical extent has no gap. Flow annotations are retained as saved metadata separately from physical opcode effects.

## Local CPU contract

Every statement assumes the instruction is reached and completes without a fault. Addresses/register operations use 32-bit arithmetic. Each load or store samples its own instruction-time memory; possibly aliasing writes and opaque calls prevent substituting earlier values. There is no explicit segment override: EBP/ESP base operands default to SS, other base/no-base operands to DS, and implicit stack accesses to SS. LEA computes an address without reading that memory.

Before the first possible call, entry `ESP=T`, `EBP=B` gives frame value `F=T-4`; the initial pushes and `SUB ESP,0x2c` leave `ESP=T-0x3c` if the prefix completes. The signed comparison with `0x80` selects either a dword load or sign-extended byte load at the then-current `[ECX+8]`. The selected value is stored to `[F-8]`; the subsequent signed comparisons determine the local edge to the physical E8 at `00C07655`. These are numeric instruction effects, not an authenticated EH-state interpretation.

Every E8 has an independently decoded numeric target and return address. Whether it returns is unknown. At any resumed return site, all returned registers, flags, memory, and ESP are unconstrained here; this includes EBP and the frame identity. Later POP/ADD ESP instructions operate on the actual returned stack and do not prove callee preservation or argument cleanup.

If `00C07974` is reached with current `ESP=S`, `EBP=F`, POP EDI/ESI/EBX read successive words at `SS:[S]`, `SS:[S+4]`, and `SS:[S+8]`. LEAVE then uses current `F`, and RET reads its next EIP from `SS:[F+4]`, leaving ESP at `F+8`. Those operations do not prove restoration of any original entry or caller state.

The report also records encoded-width arithmetic/logic flag rules and all branch predicates. It makes no typed EH layout, global/TLS identity, provider, whole-function behavior, or default callee assumption.

## Verification and remaining limit

The finite package retains actual frozen/staged/final Git objects, exactly these two output paths, copied Root gate artifacts, all command streams, manual and Capstone results, and physically pinned replay/auditor code. The guard permits one supplied ZIP read, one declared receipt write, and one retained code execution after bootstrap. The distinct copied ZIP is replayed under the same restriction. Actual replay receipts and final commit/tree IDs accompany the handoff.

Two offline inspection mistakes are disclosed: a three-digit instead of four-digit capture filename, and an initial fixed-width hex-column parse that rejected the short final line. Neither changed evidence or caused an extra Native query. The final decoder passed on its first execution.

No C++ Source, build, new test, execution probe, Original, OS audio, runtime, or game validation belongs to this packet. Semantic handler reconstruction and original ABI compatibility remain separate work requiring bounded callee and frame evidence.
