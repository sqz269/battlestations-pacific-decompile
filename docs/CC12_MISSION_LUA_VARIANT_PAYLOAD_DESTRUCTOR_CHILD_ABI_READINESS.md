# Raw mission Lua variant payload cleanup: 00884AF0

The complete `00884AF0..00884B54` span is 101 bytes / 29 instructions. It stores
`00D0E6F4h` at receiver offset zero, calls the previously audited `00886920` with
that same receiver in ECX, then reads a block at receiver `+10h`. A nonzero block
selects a late size read at `+0Ch`, wrapped DWORD increment, and the pool
getter/return pair. Its sole normal exit is plain `RET` at `00884B54`.

All 101 live bytes match the installed PE. The live listing contains every
decoded instruction; there is no omitted continuation or adjacent-byte need.
The selected body does not clear the block/size fields or free the receiver.
Its children can alter aliased storage and retire nested payloads.

**Source closure remains held.** The `00886920` child has complete earlier
readiness evidence but no current Source implementation. This edge closes the
readiness cycle `00886920 -> 006EE960 -> 006EE890 -> 00884AF0 -> 00886920`;
it does not prove a terminating input graph or independently closed Source leaf.
"Payload destructor" and the fixed header word's possible table role are
descriptive hypotheses, not recovered symbols or class/layout proof.

This packet changes only this document and its JSON report. It adds no Source,
CMake, ledger or Ghidra edits, builds, tests, probes, Original execution or credit.

## Complete extent and target

- Published baseline: `9a45da88d25a3c86c41fe71ead4dce8224da18c3`.
- Worker: `agent/cc12_lua_variant_copy_child_abi`; packet:
  `cc12_lua_variant_payload_destructor_child_ABI_readiness`.
- Metadata checked before body reads: 101 bytes, 29 instructions, three blocks,
  three edges and three calls. This passes the 300-byte admission gate.
- Full independent decode: 29 instructions / 101 bytes, three direct calls,
  one conditional branch, no local loop, and one `C3` return. The ordinary owned
  graph, permitting children to return, reaches all 29 instructions / 101 bytes.
- Whole-span SHA-256:
  `40a084311f56d31bed50c8487a75be912da83ec178507028bab016b5775db2c3`.
- Installed PE SHA-256:
  `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
  RVA and file offset are `00484AF0h`.
- Capstone 5.0.7 consumes exactly the selected span, ending at `00884B54`.
  No bytes before or after that extent, or table contents at `00D0E6F4`, were read.
- Each live CLI query verifies existing project `bsp`, program
  `/battlestationspacific.exe`, language `x86:LE:32:default` and base `00400000`.
  The configured `C:/Users/sqz269/bsp.gpr` exists and was reused. Live/snapshot
  function counts remain 64,729.
- External children received metadata queries only. Earlier reports and current
  Source pins supply qualified contracts; no Native child/caller/handler/profile,
  string, table or adjacent body was opened. The 637-byte `006EE5D0` remains out
  of scope. Shared exports use `tools/workspace.py`, without a worktree junction.
- No no-return, flow, body, prototype or other Ghidra property was changed or
  repaired. No project write/save was performed.

## Receiver, registers and stack

Let S be entry ESP, P entry ECX, B the late DWORD `[P+10h]`, W the conditional
late DWORD `[P+0Ch]`, and Z=`(W+1) mod 2^32`. The accepted parent `006EE890`
delivers P=`N+1Ch`, where N is its retained current node. There are no explicit
stack arguments to this body and no read from the caller's argument area.

| Incoming state | Owned classification |
| --- | --- |
| ECX | P, pushed into a local slot and retained in ESI. It is not changed before the `00886920` call, which receives that same P. |
| EAX | Overwritten by `FS:[0]` at `00884AF7` before any incoming-value read. The old FS word reaches the first child in EAX; that child's admitted body discards it. |
| EDX | No owned read or write in all 29 instructions. It can pass into children; no hidden-input or transitive-independence claim follows. |
| EBX / EBP / EDI | No owned read or write. Their incoming values can reach children; whole-call preservation depends on those child contracts. |
| ESI | Saved at `00884B06`, assigned P, used for late field reads, then replaced by W/Z on the pool path. Restored from its current saved slot on normal exit. |
| Arithmetic flags | No incoming conditional use. The only branch consumes the owned `TEST EAX,EAX` after the first child. The following state-store MOV preserves those flags. |
| x87 | No owned x87 instruction, status/environment operation or reset; child behavior remains unproved. |

Under the admitted parent delivery, EBX carries its opaque R, EBP the retained
node N, incoming ESI the already captured next node L, and incoming EDI the
parent's V=`N+Ch`. This body saves L before replacing ESI with P, and restores
that saved word for its parent. It makes no semantic local use of R/N/V through
EBX/EBP/EDI. Passing those values into children is not proof of their independence.

The complete local frame is:

| Address | Role |
| --- | --- |
| `S-4` | Unwind state, initially `FFFFFFFFh`. |
| `S-8` | Handler address `00C970DBh`. |
| `S-Ch` | Saved FS head; registration installed at this address. |
| `S-10h` | Local receiver word: first `PUSH ECX`, then explicitly stored again at `00884B09`. |
| `S-14h` | Saved incoming ESI; steady body ESP. |

The repeated local receiver store is `[ESP+4]=ESI` at steady ESP. It is not a
public argument write, and normal owned flow never reloads that slot. It remains
observable to aliases or exceptional machinery. No saved EBX/EBP/EDI words are
created by this body.

`00886920` is called without pushed arguments at `00884B1B`; its previously
audited plain returns satisfy the local no-argument balance requirement. The
pool path pushes `1`, Z, B before the getter. Admitted Source/ledger evidence
describes `00419CC0` as no native arguments/plain RET, so these are pending words
for `00BD1510`, whose admitted `RET 0Ch` removes all three. They are not three
arguments to the getter despite the pseudocode call spelling.

At normal exit, `[ESP+8]` supplies the current saved FS head. The body pops ESI,
restores `FS:[0]`, adds `10h` to ESP to discard the local word and three EH words,
then executes plain `RET`. Final ESP is S+4. This closes the accepted parent's
zero-argument stack-balance requirement, subject to compatible children and
uncorrupted saved slots; it does not establish binary ABI execution.

## Exact normal schedule and direct footprint

1. Push state -1 and handler `00C970DB`, capture and register the old FS head,
   push ECX, save ESI, retain P in ESI and explicitly publish P to the local slot.
2. At `00884B0D`, write DWORD `00D0E6F4h` to `[P]`. There is no receiver-null,
   existing-header or ownership check. The EH state is still -1 during this write.
3. Set state zero at `00884B13`. At `00884B1B`, call `00886920` with unchanged
   ECX=P and ESI=P. Its complete earlier audit describes tag-driven raw cleanup,
   including a normal tag `FFFFFFFFh` store at P+4 and conditional P+8 clearing.
   Those are admitted child effects, not stores in this 101-byte body.
4. Only after that child returns, at `00884B20`, load B=`[P+10h]`; TEST B. State
   remains zero through the read/test, then becomes -1 at `00884B25`.
5. If B is zero, branch directly to the common epilogue. `[P+0Ch]` is not read.
6. Otherwise read W=`[P+0Ch]` into ESI at `00884B2F`, push literal 1, increment
   ESI to Z with DWORD wrap, and push Z then captured B. Call getter `00419CC0`
   at `00884B39`, put its EAX in ECX, then call `00BD1510` at `00884B40` with
   pending `(B,Z,1)`. No pool-null guard, size clamp or intervening field reload
   is encoded. The admitted pool contract does not read the last word.
7. Restore the local registration/ESI and return. There is no subsequent owned
   receiver-field store or receiver-free call.

The selected receiver footprint is one DWORD store at `P+0`, an unconditional
post-child DWORD read at `P+10h`, and a conditional DWORD read at `P+0Ch`.
It directly reaches through P+13h: a minimum 20-byte address extent, not a
complete class or allocation size. Under P=`N+1Ch`, those addresses become the
node header store N+1Ch, late block read N+2Ch and conditional size read N+28h.
The unopened table address supplies only an immediate stored DWORD.

The first child runs before either field read, so its mutations to B/W or aliases
are observable. B is captured before the W read; both B and Z are then on the
stack before the getter. Later getter/pool-helper mutations do not cause a block,
size or receiver reload. On the pool path ESI no longer holds P; the saved local
P word still exists but is never used on normal owned flow. No owned store clears
P+0Ch/P+10h after a possible block return. Repeat safety therefore needs separate
child and lifetime contracts; the selected body alone does not prove it.

## EH, failure and normal result

Handler `00C970DB` was not opened. State is -1 through registration setup, local
receiver publication and the fixed-header store; it is 0 during `00886920` and
the following B read/test, then -1 for the W read, size arithmetic and pool pair.
The numeric state and local P publication do not prove which cleanup runs on a
fault, ownership transfer, rollback, catch behavior or second-failure handling.

Null/unmapped P can fault on the initial fixed-header store after registration
installation. A child can partially retire nested storage before the late B/W
reads; those reads can fault or observe its side effects. Aliases can merge P,
its block, nested objects, the parent's node or local frame storage. There is no
owned overlap, extent, alignment or overflow guard. Normal saved-slot restoration
does not establish exceptional restoration, leak freedom or arbitrary alias safety.

The B-zero path leaves EAX=0 at return because the late B load is the last EAX
definition. The B-nonzero path passes through the pool-return helper's EAX value
without another local definition. There is no uniform receiver-return contract.
The accepted parent ignores that value and performs its own later field reads
and node-free sequence; this packet does not reclassify that parent behavior.

## Current Source and qualified dependency cycle

Exact address searches find no current Source or reconstruction/name admission
for `00884AF0` or `00886920`. The child evidence is:

| Direct child | Current metadata bytes / instructions / calls | Qualified status |
| --- | --- | --- |
| `00886920` | 168 / 46 / 5 | Existing full readiness audit has 59 decoded instructions, six calls and two plain returns; the metadata undercount remains qualified. No Source implementation. |
| `00419CC0` | 192 / 51 / 5 | Admitted pool getter Source with explicit publication, semantic-domain/raw-manager and lifetime contexts; no new Native ABI validation. |
| `00BD1510` | 95 / 32 / 2 | Admitted pool-return Source with explicit pool/block/size/live gate; current CRT/locking/EH boundary remains. |

No Native child body was reopened. The previously accepted `00886920` schedule
is reused from its pinned report, including its hidden post-free continuations,
raw tag/payload ordering and local plain-return contract. Connecting the existing
cycle requires valid recursive data and mutually compatible lifetime/context
contracts. A static call cycle can process distinct nested objects; it does not
assert repeated pointer identity. It neither proves dynamic termination nor removes the other readiness
holds inside `006EE960`, including unopened over-gate `006EE5D0`.

All 62 accepted parent input pins replay exactly at their recorded historical
revision. At this baseline 60 are unchanged. The two BF00 ledger shards each add
only the separately admitted `00BF63A6` record; no existing record is removed or
changed, and selected `00BF65AC`/`00BF6713` records are byte-identical. This is
explicit shard drift, not 62 unchanged current files. The report records 64
current canonical/physical pins, including the immediate parent document/report.

Current pool Source/header/report files match the previously inspected contracts
exactly. Their receipts lack individual historical Source artifact hashes; old
fixture/aggregate evidence is not promoted to current Source or Original ABI.
Inherited stale string and other indirect receipt qualifications remain intact.
The JSON embeds the full decode, call/frame/reload schedule and evidence hashes.
No startup, binary replacement, runtime fixture or gameplay claim is added.
