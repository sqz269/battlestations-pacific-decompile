# FindHandler first direct-call owner: two-range physical listing

The saved owner at `00C07AC1` contains 13 complete instructions in two admitted ranges: `00C07AC1..00C07AE4` (36 bytes) and `00C07AEC..00C07AF7` (12 bytes). Paired exact byte reads, manual physical fields, actual detailed Capstone 5.0.7 and paired context-zero saved text agree. The intervening seven bytes `00C07AE5..00C07AEB` were neither read nor classified. The saved `_inconsistency` name does not establish service behavior or original ABI.

The [report](../reports/cc12_findhandler_first_direct_call_body_metadata.json) retains every opcode, ModRM, displacement, immediate, operand, numeric target, saved text and saved flow record. The range SHA-256 values are `ce9461cead0d4ec6030b48fc53ba1eb36847b6417a940aafd7fbbe0761cdad97` and `dabbfac2550f0bc32ae9ada9f3779129bf88197b198aa383f4a98f1af907d857`.

## Gate and physical operations

The finite Git boundary is commit `b7493b31b7fee4e1cb7dd7c432661d856b65bf52`: the immediate ENTRY and ENTRY_PRIMARY doc/report pairs, `config/target.json` and the three CLI files. Actual Git commit, root, selected parent trees and eight blob payloads are retained with working-LF agreement. Source subtree IDs identify the revision only. Source contents and prior evidence archives were not traversed.

Before body bytes or text were read, Root checked two independent metadata walks. Each next start came from that pass's complete exact InstructionDB length and stayed wholly inside its explicit range. All four aliases, complete owner ranges and 26 original raw responses agree pairwise. Root admitted only the two exact byte ranges, paired context-zero at the 13 established starts, closing same-start metadata and count/identity bookends. All 13 closing responses equal both originals. Across 39 Native commands and 183 retained HTTP responses, epoch remains `98` and six count checkpoints remain `64,730` with the intended project/program. No prototype, decompile, data, target, gap or outside-endpoint query was made.

| Address | Physical operation | Bounded observation |
| --- | --- | --- |
| `C07AC1` | `6A 08` | Push sign-extended immediate dword `8`. |
| `C07AC3` | `68 78 35 E0 00` | Push immediate dword `00E03578`; no pointer identity established. |
| `C07AC8` | `E8 33 01 00 00` | Direct call to numeric `C07C00`, encoded return `C07ACD`. |
| `C07ACD` | `FF 35 70 E1 09 01` | Push the current dword at default `DS:[0109E170]`. |
| `C07AD3` | `E8 06 D5 FF FF` | Direct call to numeric `C04FDE`, encoded return `C07AD8`. |
| `C07AD8` | `59` | Pop a current stack dword into ECX. |
| `C07AD9` | `85 C0` | Test current EAX against itself; EAX is unchanged. |
| `C07ADB` | `74 16` | Jump to `C07AF3` if ZF is set. |
| `C07ADD` | `83 65 FC 00` | Read-modify-write zero to current default `SS:[EBP-4]`. |
| `C07AE1` | `FF D0` | Call instruction-time EAX; encoded return `C07AE3`. |
| `C07AE3` | `EB 07` | Jump to `C07AEC` across the unqueried gap. |
| `C07AEC` | `C7 45 FC FE FF FF FF` | Write dword `FFFFFFFE` to current default `SS:[EBP-4]`. |
| `C07AF3` | `E9 7D FF FF FF` | Jump with rel32 `-131` to numeric `C07A75`; no return-address push. |

At `C07AF3`, saved default flow is `UNCONDITIONAL_JUMP`, override is `CALL_RETURN` and effective flow is `CALL_TERMINATOR`. The physical opcode is E9. The annotation establishes neither a CALL instruction nor target nonreturn. Automatic target labels returned by the admitted metadata endpoint are retained as metadata; no target query was issued.

## Qualified local CPU contract

Each effect requires that the observed encoding be reached and complete fault-free under the x86-32 interpretation. There is no explicit segment, operand-size or address-size prefix. No-base memory defaults to current DS; EBP-based memory and implicit stack accesses default to current SS. These defaults do not authenticate live segment selectors, bases or limits.

**Every ESP equation additionally requires actual 32-bit SS stack-address size at that point.** Decoder mode and operand width do not establish `SS.B`; after each opaque call the condition must be imposed anew. A 16-bit stack uses SP semantics.

With entry `ESP=T` and that stack condition, the initial pushes write `8` at `SS:[T-4]` and `00E03578` at `SS:[T-8]`. The first E8 writes return `C07ACD` at `SS:[T-12]`. On any resumed return, every register including EBP and ESP, all segments, EFLAGS and memory are unconstrained. The next PUSH samples current `DS:[0109E170]`; it does not identify a global, TLS value or provider. The subsequent direct call creates another opaque boundary. At a resumed `C07AD8` with current `ESP=U` and actual 32-bit stack size, POP ECX reads current `SS:[U]` and leaves `ESP=U+4`; this proves no argument cleanup or earlier stack identity.

TEST EAX,EAX sets ZF for zero, SF from bit 31 and PF from even parity of the low byte; it clears CF/OF and leaves AF undefined. If the adjacent JZ uses those flags, zero selects `C07AF3`. The other path reaches a dword AND with zero at current `SS:[EBP-4]`. This remains a read-modify-write requiring the memory access; it sets ZF/PF, clears SF/CF/OF and leaves AF undefined. EAX is unchanged before the indirect call.

At any return from CALL EAX, all returned state is again unconstrained. The later MOV uses its then-current EBP and SS, so it need not write the address used by the earlier AND. Values `0` and `FFFFFFFE` establish no EH state or typed frame. Each memory access is a separate instruction-time sample; aliasing and opaque calls prevent assuming prior values remain stable.

No RET appears in the admitted bytes. That does not establish whether an outbound target can return. Saved names `_inconsistency`, `__SEH_prolog4`, `BSP_CRT_DecodePointerCurrentState` and `terminate`, along with their flags, authenticate no library, handler, EH/FH3/TLS/type, provider, independence, preservation, convention or original ABI meaning.

## Retained proof and failure distinction

The finite proof retains 383 original metadata preimages and all 392 copied Root artifacts, including the Root gate's initial failed producer and complete streams. That offline gate initially allowed guessed endpoint names instead of the actual pinned `list_project_files` and `get_current_program_info` endpoints. Root corrected the whitelist before admission. This one Root gate failure caused no Native query or GPR mutation. Worker Native captures and the physical decoder passed first execution without failure.

Full physical helper bodies, all actual frozen/final Git objects, exactly these two output blobs and original command/HTTP streams accompany the package. Each strict supplied-ZIP replay permits one physical ZIP read, one reviewed local helper execution and one exclusive receipt write; it denies other audit events and executes no archive-contained code. A distinct physical ZIP copy receives its own replay. Final commit/tree IDs and receipt pins accompany the handoff outside the self-referential report.

This packet contains no Source change, build, test, probe, GPR mutation, Original/game/OS/audio or runtime execution. It closes the physical listing of the two saved owner ranges, with no whole-owner semantic or original ABI credit.
