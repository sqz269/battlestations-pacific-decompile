# C051B7 admitted two-range body evidence

The saved owner at `00C051B7` was inspected through two independent metadata walks and a closing walk. The admitted physical capture contains **116 bytes in 43 complete instructions**, split between `00C051B7..00C0521E` (104 bytes, 37 instructions) and `00C05222..00C0522D` (12 bytes, six instructions). The three-byte gap `00C0521F..00C05221` remains excluded. Neither boundary cursor `C0521F` nor `C0522E` was queried.

This packet establishes bounded saved metadata and physical instruction effects. It does not reconstruct a Source implementation, authenticate the saved library name or prototype, or establish an ABI, provider, complete return behavior, runtime behavior, or gameplay equivalence.

## Admission and retained originals

The worktree froze actual Main `e9d50c01c94b957ed22dd54e391a8c0ade4c503c` and fourteen actual commit/tree/blob objects. The eight explicit positive inputs are the accepted C051B7 ENTRY and ENTRY_PRIMARY document/report pairs, `config/target.json`, and the exact three CLI sources `tools/bsp.py`, `tools/ghidra_export.py`, and `tools/ghidra_typed_flow.py`. Their working bytes, LF form, index blobs and committed blobs matched. Source subtree IDs were retained without reading Source bodies or earlier proof archives.

Phase one used 86 metadata requests at independently length-walked starts, with four redundant InstructionDB/code-unit views, complete owner ranges and separate default/effective flow fields. All paired original HTTP response bytes matched. Root reviewed the complete originals and admitted only the two saved ranges. All **668 initial Root artifacts**, including 658 metadata preimages, were copied and verified before the first phase-two command.

The subsequent paired byte captures and paired context-zero instruction text agree. All 43 closing original metadata responses equal their two initial originals. Across both phases, 141 Native commands retained original stdout/stderr, preexecution invocations and exact CLI-source observations before and after every command: 282 observations and 846 physical source reads. Six function-count checkpoints remained 64,730; every typed response remained modification 98 in project `bsp`, program `/battlestationspacific.exe`.

## Physical ranges

`00C051B7..00C0521E` SHA-256 `129e55689a2a866bc6d492e76bd3b4e0624a57273f2cfc2210c99f0d665c0aa2`:

```text
5657ff155822ce00ff35fc5ae1008bf8e8a4feffffffd08bf085f6754e68140200006a01e87af6ffff8bf085f65959743a56ff35fc5ae100ff353cde0901e8e4fdffff59ffd085c074186a0056e8effeffff5959ff153c22ce00834e04ff8906eb0956e8a94bffff
```

`00C05222..00C0522D` SHA-256 `0696a0a5b3e4feb323050302fcad557e9861683d8826b5425aa4a463d8341005`:

```text
57ff15bc22ce005f8bc65ec3
```

The concatenated evidence stream hashes to `862f25f769383b42aca28678bb4551b92091195851680db1811199c8805061d1`. It is **not** a contiguous 119-byte address range; no bytes were inserted for the gap.

| Address | Bytes | Physical instruction |
| --- | --- | --- |
| `00C051B7` | `56` | `PUSH ESI` |
| `00C051B8` | `57` | `PUSH EDI` |
| `00C051B9` | `ff155822ce00` | `CALL dword ptr [0x00ce2258]` |
| `00C051BF` | `ff35fc5ae100` | `PUSH dword ptr [0x00e15afc]` |
| `00C051C5` | `8bf8` | `MOV EDI,EAX` |
| `00C051C7` | `e8a4feffff` | `CALL 0x00c05070` |
| `00C051CC` | `ffd0` | `CALL EAX` |
| `00C051CE` | `8bf0` | `MOV ESI,EAX` |
| `00C051D0` | `85f6` | `TEST ESI,ESI` |
| `00C051D2` | `754e` | `JNZ 0x00c05222` |
| `00C051D4` | `6814020000` | `PUSH 0x214` |
| `00C051D9` | `6a01` | `PUSH 0x1` |
| `00C051DB` | `e87af6ffff` | `CALL 0x00c0485a` |
| `00C051E0` | `8bf0` | `MOV ESI,EAX` |
| `00C051E2` | `85f6` | `TEST ESI,ESI` |
| `00C051E4` | `59` | `POP ECX` |
| `00C051E5` | `59` | `POP ECX` |
| `00C051E6` | `743a` | `JZ 0x00c05222` |
| `00C051E8` | `56` | `PUSH ESI` |
| `00C051E9` | `ff35fc5ae100` | `PUSH dword ptr [0x00e15afc]` |
| `00C051EF` | `ff353cde0901` | `PUSH dword ptr [0x0109de3c]` |
| `00C051F5` | `e8e4fdffff` | `CALL 0x00c04fde` |
| `00C051FA` | `59` | `POP ECX` |
| `00C051FB` | `ffd0` | `CALL EAX` |
| `00C051FD` | `85c0` | `TEST EAX,EAX` |
| `00C051FF` | `7418` | `JZ 0x00c05219` |
| `00C05201` | `6a00` | `PUSH 0x0` |
| `00C05203` | `56` | `PUSH ESI` |
| `00C05204` | `e8effeffff` | `CALL 0x00c050f8` |
| `00C05209` | `59` | `POP ECX` |
| `00C0520A` | `59` | `POP ECX` |
| `00C0520B` | `ff153c22ce00` | `CALL dword ptr [0x00ce223c]` |
| `00C05211` | `834e04ff` | `OR dword ptr [ESI + 0x4],0xffffffff` |
| `00C05215` | `8906` | `MOV dword ptr [ESI],EAX` |
| `00C05217` | `eb09` | `JMP 0x00c05222` |
| `00C05219` | `56` | `PUSH ESI` |
| `00C0521A` | `e8a94bffff` | `CALL 0x00bf9dc8` |
| `00C05222` | `57` | `PUSH EDI` |
| `00C05223` | `ff15bc22ce00` | `CALL dword ptr [0x00ce22bc]` |
| `00C05229` | `5f` | `POP EDI` |
| `00C0522A` | `8bc6` | `MOV EAX,ESI` |
| `00C0522C` | `5e` | `POP ESI` |
| `00C0522D` | `c3` | `RET` |

## Conditional local effects

Every PUSH, POP, CALL and RET independently renews the condition **actual current SS.B = 1** for its stated ESP-based 32-bit stack formula. Code address size does not establish stack address size. Each operation remains conditional on successful segment, memory and control-transfer checks. Earlier stack predicates and earlier register values are not carried across opaque calls.

The five direct calls encode numeric offsets `C05070`, `C0485A`, `C04FDE`, `C050F8`, and `BF9DC8`. Two calls resolve actual current EAX independently; three read a dword through actual current DS at `CE2258`, `CE223C`, or `CE22BC`. No callee, cell or computed target was queried. Every returned register, flag, segment and stack state remains opaque afresh. The FF /6 pushes read current DS dwords at `E15AFC` twice and `0109DE3C` once, without establishing pointer or argument meanings or equal values across calls.

Under uninterrupted successful local execution, the three conditional branches test their preceding TEST result. At `C051E6`, the two intervening POP ECX instructions preserve EFLAGS and ESI; each pop still reads its own current stack value. The OR at `C05211` performs a genuine DS dword read-modify-write at current ESI + 4, using sign-extended imm8 FF. Successful completion writes FFFFFFFF and gives CF=0, OF=0, SF=1, ZF=0, PF=1, with AF undefined. The architectural source read remains present. The following MOV stores current EAX to DS:[current ESI] without an explicit operand-data read. Neither operation proves a structure, valid memory or atomicity.

The final POP EDI and POP ESI read current stack dwords; they do not establish restoration of original registers or a balanced stack. For uninterrupted successful final MOV EAX,ESI / POP ESI / RET, EAX retains the value read from ESI at `C0522A`; its meaning and earlier history remain unknown. DS/SS aliases, cell/stack/code aliases, accessibility, exceptions and later fetch validity remain unresolved.

## Saved metadata differs from physical evidence

At `C0521A`, bytes `E8 A9 4B FF FF` encode a near **CALL** to numeric `BF9DC8`, with encoded return offset **C0521F inside the excluded gap**. Saved default flow is UNCONDITIONAL_CALL, while saved effective flow is CALL_TERMINATOR under CALL_RETURN. Both saved fallthrough fields are null. These saved annotations do not prove physical nonreturn, convert the E8 into a jump, or erase its encoded return offset.

At `C051B9`, `C0520B` and `C05223`, default target arrays are empty while effective arrays carry saved EXTERNAL offsets `0000016b`, `00000164` and `00000184`. These are retained observations, not authenticated values read from the three physical DS cells. Every default/effective flow flag, target array and fallthrough remains independently recorded in the JSON report.

## Verification and limits

All 43 complete manual opcode, ModR/M, displacement, immediate, access, register, flag and transfer records were saved before a fresh actual Capstone 5.0.7 detailed x86-32 decode. The decoder worked on the two ranges separately. Every recorded comparison passed on its first execution; the manual records were not revised afterward.

The reviewed finite helper is pinned at `5cb2df771723d08c9527b9f9ecffa76da15047cbc6eb86b27dc797e2fcdf5bb8` (67003 bytes). Core replay passed on both the original ZIP and a physically separate byte-identical copy. Each strict audit permitted one archive read, one execution of the exact helper code object and one exclusive receipt write; `denied` was asserted empty. The final delivery contract adds the worker commit, twenty actual Git objects in the initial/final union, and exact LF/index/working/committed identities for all eight inputs and two outputs.

Current Native, capture-producer, decoder and pure-replay failures are zero. Inspection history separately retains **two display truncations** (both commands exited zero) and **one rejected apply_patch context match** before any helper execution. Complete original stdout was unavailable for those display-only invocations; original command/warning evidence and separately labeled later full recaptures are retained. No recapture is presented as an original. Prior ENTRY core history, its later writer failure, and PRIMARY's independent validation and inspection history remain nested under their original scopes.

No Source or ledger edit, GPR mutation, build, new test, probe, game, OS or audio/runtime experiment belongs to this packet. The full structured evidence and all per-instruction uncertainties are in [`cc12_c051b7_direct_target_body_metadata.json`](../reports/cc12_c051b7_direct_target_body_metadata.json); private originals and finite replay artifacts are under `local/cc12_c051b7_direct_target_body_metadata/`.
