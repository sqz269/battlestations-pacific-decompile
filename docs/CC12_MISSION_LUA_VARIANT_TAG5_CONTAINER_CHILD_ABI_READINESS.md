# Raw mission Lua variant tag-5 child: 006EE960

The selected `006EE960..006EEA28` extent is 201 bytes. All live bytes match the
installed PE. Independent decoding produces 77 instructions, including one
unlisted NOP at `006EE9DF` after the first return. Ordinary entry flow reaches
the other 76 instructions / 200 bytes: six calls and two `RET 14h` exits.

Both normal exits return the current output pointer in EAX and write exactly
two DWORDs through it, consuming all five public argument words. Incoming EAX
is discarded; the output pointer is fetched from the stack late. Incoming EDX
is not locally read before definition, but can reach validation or loop children
unchanged. It is replaced before the special-path child call. No full transitive
EDX independence, Native ABI replacement or Source closure is established.

The body has a special path that calls `006EE890` and writes self-referential
header words, and a general loop involving `006EDA20` and `006EE5D0`. The latter
has a 637-byte metadata extent, above the 300-byte gate, and was not opened.
**Source closure remains held.** "Container", "owner" and "node" describe the
observed comparisons/links provisionally; no complete STL type is recovered.

Only this document and its JSON report are added. No Source, CMake, ledger,
Ghidra, build, test, probe, Original execution or new credit is included.

## Scope and full-span proof

- Published baseline: `6ed048390792b1c40bb1f1596a521a7cda02a62c`.
- Worker: `agent/cc12_lua_variant_copy_child_abi`; packet:
  `cc12_lua_variant_tag5_container_child_ABI_readiness`.
- Metadata checked before body capture: 201-byte extent, 76 instructions,
  15 blocks, 22 edges and six calls. The owned extent passes the 300-byte gate.
- Exact whole-span SHA-256:
  `bcf888af4677f8fd105b176f8b3ee00c4a824075d2185d3dba79ebf1b3a8d022`.
- Installed PE SHA-256:
  `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
  The selected bytes are at file offset `002EE960h` and RVA `002EE960h`.
- Capstone 5.0.7 consumes every selected byte. Both returns (`006EE9DC`,
  `006EEA26`) encode `C2 14 00`. All owned direct branch targets are instruction
  starts inside the extent. No outside switch/table/data access was inspected.
- Ghidra omits only the one-byte `90` NOP at `006EE9DF`. The preceding return
  ends ordinary flow, and every branch entering the following loop targets
  `006EE9E0`. A local graph allowing each child to return reaches all 76 listed
  instructions and does not reach this NOP. No after-call continuation is missing.
  The 201 raw bytes and 200 ordinary-entry code bytes remain separate quantities.
- The configured existing `C:/Users/sqz269/bsp.gpr` exists and was reused.
  Each live CLI query verifies project `bsp`, `/battlestationspacific.exe`,
  `x86:LE:32:default`, and image base `00400000` before access. Live/snapshot
  counts both remain 64,729. No save, reimport, flow repair or flag edit occurred.
- Children received selected metadata fields only. No Native child, caller,
  handler, profile, table, string or adjacent body was read. Shared captures are
  resolved with `tools/workspace.py`; no worktree export junction exists.

## Inputs, frame and incoming registers

Let S be entry ESP and R entry ECX. The five DWORD arguments are named by their
observed use, not recovered source types:

| Slot | Name | Observed role |
| --- | --- | --- |
| `S+4` | O | Output pointer, loaded late on each return path. |
| `S+8` | F | First pair's owner word; initially captured in EDI. |
| `S+Ch` | N | First pair's node word; captured in EBX after the first validation call/bypass. |
| `S+10h` | L | Last pair's owner word; read from the current stack slot at checks. |
| `S+14h` | E | Last pair's node word; read from the current stack slot at comparisons. |

The prologue reserves eight local bytes and saves EBX, EBP, ESI, EDI. Steady ESP
is `S-18h`. The local area is `S-8..S-1`; saved EBX, EBP, ESI, EDI occupy `S-Ch`,
`S-10h`, `S-14h`, `S-18h`. The local bytes are neither initialized nor read by
owned instructions; their address is passed to `006EE5D0` on each loop iteration.

| Incoming state | Complete owned classification |
| --- | --- |
| ECX | Used as R at `006EE96D`; ESI retains R across children, requiring compatible nonvolatile preservation. |
| EAX | Overwritten by `[R+4]` at `006EE96F` before any owned consumption. The parent's duplicate output pointer in EAX is unused. |
| EDX | No owned read before definition. Validation calls and the first `006EDA20` may receive the incoming value. `006EE9A4` defines it from a current header word before `006EE890`; `006EEA00` defines it as the local-buffer address before `006EE5D0`. |
| EDI / ESI | Incoming values are read only for PUSH preservation, then replaced by F / R. The parent's incoming EDI=D is not an owned semantic input. |
| EBX / EBP | Similarly saved. EBP is replaced by a nested/header word before the first child; EBX is replaced by N only after the first validation call/bypass. That first call can therefore see incoming EBX, without establishing a hidden contract. |
| Arithmetic flags | Defined by entry `SUB ESP,8`, then local TEST/CMP operations before conditional use. No incoming arithmetic-flag dependency is encoded. |
| x87 state | No owned x87 operation, environment access, initialization or reset. Child independence/preservation is unproved. |

All register restorations consume current saved frame words. Output stores and
child writes can alias those slots; the exact interleaving below matters for
arbitrary raw backing. Normal preservation is conditional on compatible children
and uncorrupted saved slots.

## Initial comparisons and returning validation calls

First capture F from `[S+8]` and TEST it. Then, **before acting on that null
test**, copy ECX to ESI, load H0=`[R+4]`, and load B0=`[H0]`. The intervening MOVs
preserve the TEST flags. Null/invalid R or H0 can therefore fault before a null-F
validation call. There is no receiver/header null guard here.

If F is zero or differs from R, call `00BF6713` at `006EE97A`, with no pushed
argument. If it returns, continue: load current N from `[S+Ch]` into EBX and
compare it with previously captured B0 in EBP. A mismatch goes to the loop.

On equality, load current L from `[S+10h]`, TEST it, and capture H1=`[R+4]` in EBP.
If L is zero or differs from R, call `00BF6713` at `006EE996`, again with no
pushed argument. After its return/bypass, compare current E=`[S+14h]` with H1.
An equality selects the special path; a mismatch selects the loop.

The owner checks invoke a child and have ordinary continuations. They do not
themselves enforce a return, throw or abort. If the validation helper returns,
the first node/header comparisons still use values captured before that helper,
while their stack comparison words are read afterward. A rewrite must not turn
the helper's name into an unconditional no-return rule or silently revalidate
the cached words after it returns.

## Special path and repeated header reloads

The observed structural selection is N==B0 and E==H1 after the validation
call/bypass sequence. If mismatching owner checks return, those comparisons can
still select this path. Assuming each intervening child preserves required
registers, the exact operations are:

| Site | Owned operation |
| --- | --- |
| `006EE9A1..9AA` | Reload H2=`[R+4]`; read K=`[H2+4]` into EDX; push K; set ECX=R; call `006EE890`. |
| `006EE9AF..9B2` | Reload H3=`[R+4]`; store H3 at `[H3+4]`. |
| `006EE9B5..9BF` | Reload H4=`[R+4]`; store zero at `[R+8]`; then store H4 at `[H4]`. |
| `006EE9C1..9C4` | Reload H5=`[R+4]`; store H5 at `[H5+8]`. |
| `006EE9C7..9CA` | Reload H6=`[R+4]`; capture W=`[H6]` in ECX. |
| `006EE9CC..9DE` | Load current O=`[S+4]`; perform the interleaved output/restore sequence; `RET 14h`. |

At `006EE890`, ECX=R and EDX equals the pushed K. No incoming EDX value is used
by the owned setup. There is no caller argument cleanup, so the observed frame
requires four-byte callee cleanup from that child. No child ABI is proved from
its generic metadata signature.

There is no one cached header across these stores. A store through one header
can alias `[R+4]` and change the next header reload. H4 is captured **before**
the receiver `+8=0` store and used afterward. W is captured before output
publication. No local check ensures a child left a valid header or count.

## General loop and current argument slots

At `006EE9E0`, EDI and EBX hold the cached current F and N. Test F; if zero,
or unequal to freshly read `[S+10h]`, call `00BF6713` at `006EE9EA`. If it returns,
continue without reloading F/N. Compare cached N against current `[S+14h]`.
Equality selects the loop's output/return sequence.

Otherwise one iteration does the following in order:

1. Set ECX=`S+8`, the address of the actual first-pair argument cells, and call
   `006EDA20` at `006EE9F9` with no pushed argument. The body does not perform
   its own increment or traversal. That child's full reads/writes are unopened.
2. Push **cached N**, then **cached F**, retaining their pre-call register values
   if the child respects its nonvolatile contract. They need not equal argument
   cells the child may already have changed.
3. Set EDX=`S-8`, push that local-buffer address, set ECX=R, and call `006EE5D0`
   at `006EEA07`. The three callee argument words are `(&local8,F,N)`; EDX also
   holds `&local8`. Normal balance requires `RET 0Ch` or equivalent callee
   cleanup, not the parent's five-word `RET 14h` contract.
4. After both children return, reload N from current `[S+Ch]` into EBX, then
   F from current `[S+8]` into EDI. Repeat the loop checks, which freshly read
   last-owner/node slots on each pass.

The local-buffer contents are ignored by owned instructions. The publicly
returned pair comes from the reloaded first-pair words when equality ends the
loop, not from the loop child output area. The initial special-path test is
never retried inside the loop. There is no owned iteration bound, monotonicity
check, tree/list integrity check or proof of progress/termination. Children can
change first-pair cells, last-pair cells, receiver storage or aliases; the exact
capture/reload order is the established contract.

## Exact output writes and return ABI

Both epilogues first capture current O from its public stack slot, then return
that same pointer in EAX. Neither reads prior output contents or checks O for
null, alignment or capacity. Each performs two separate DWORD stores at offsets
0 and 4, with no owned output-relative access outside those eight bytes.

| Path | Exact epilogue order before `ADD ESP,8; RET 14h` |
| --- | --- |
| Special | Load O; POP EDI; `[O]=R` using live ESI; POP ESI; POP EBP; `[O+4]=W` using captured ECX; POP EBX. |
| Loop | Load O; `[O]=cached F` using EDI; POP EDI; POP ESI; POP EBP; `[O+4]=cached N` using EBX; POP EBX. |

This closes the parent call site's 20-byte cleanup requirement and establishes
the normal owned output write extent. It does not establish all children's
nonvolatile behavior or Native ABI compatibility. If O aliases saved slots,
the first store affects a different restoration window on the two paths; the
second store precedes EBX restoration on both. Stores may also alter argument
or return-address words. Caching O occurs before those stores, and no atomic
pair publication or general overlap safety is claimed. A fault on the second
store can leave the first store and prior child/receiver effects visible.

## Footprint, failure and accepted-parent qualification

The direct receiver-relative footprint is repeated DWORD reads at `R+4` and,
only on the special path, one zero DWORD store at `R+8`. There is no direct
receiver `+0` access or larger receiver-layout evidence. Header-relative reads
are `+0` and `+4`; special-path writes are `+4`, `+0`, `+8`, each through the
fresh/captured bases detailed above. General-loop node words are compared and
passed, not directly dereferenced by the loop's own instructions. Children can
expand all of these footprints. No complete receiver/header/node class is proved.

No owned allocation, free, FS access, exception-frame installation, handler,
unwind-state store, catch or rollback is present. Invalid receiver/header memory
can fault before validation. Output O is not accessed until after any selected
child and header mutation sequence. A child can throw, terminate, fault or fail
to return; later normal-path stores then are not established. The existing
outer/child EH and Native CRT behavior remain separate contracts.

Accepted parent `00886920` supplies R=P, O=&its uninitialized local8, F=P,
N=B=`[A]`, L=P, E=A, with A captured from `[P+4]` and P checked nonzero. If those
header/first-node words remain stable until this body's comparisons, both owner
validation calls are bypassed and N/B0 and E/H1 match. This selects the special
path. Its first reached child receives EDX freshly defined from current `[H2+4]`,
so incoming EDX is eliminated on that **conditional** path. The comparison can
change under frame/storage alias mutation; arbitrary raw inputs and the general
loop retain pass-through uncertainty. No parent Native body was reopened.

## Current Source and smallest next dependencies

Exact address searches and selected reconstruction records find no Source for
`006EE960`, `006EE890`, `006EDA20` or `006EE5D0`.

| Child | Metadata bytes / instructions / calls | Next bounded question |
| --- | --- | --- |
| `006EE890` | 150 / 46 / 5 | First useful parent-path dependency: ECX receiver, pushed K also in EDX, full footprint/reload/lifetime and `RET 4` requirement. |
| `006EDA20` | 99 / 38 / 2 | In-place first-pair argument-cell updates, incoming EDX, progress and nonvolatile preservation. |
| `00BF6713` | 16 / 9 / 1 | Actual incoming-state and return/failure contract behind three validation sites. |
| `006EE5D0` | 637 / 215 / 14 | **Above 300-byte gate.** Primary must explicitly admit a full Astra body scope before Native reading; three arguments/local output and `RET 0Ch` requirement remain unproved. |

Metadata sizes and names do not establish any child's behavior. In particular,
the name `STL_xlen_throw_006ee5d0` is not proof that the call never returns; this
owned body has a normal continuation that reloads the pair and loops. No partial
prefix of its 637 bytes was read or accepted as a body.

The address-bound `00BF6713` ledger row points to the current
`NativeInputDeviceRuntime::invalid_parameter_00bf6713` method, whose body calls
`_invalid_parameter_noinfo()`. The row explicitly says it was backfilled from a
matching Source name and reverified nothing. Another current container adapter
delegates to the same host CRT function. These are Source service boundaries,
not proof of the Original register ABI or unconditional termination. The input
runtime's historical report includes a returning-host-handler fixture, which
was not rerun and is not Original execution evidence for this packet.

All 54 parent input pins replay at their stated revision. Current BF name and
reconstruction shards differ and are separately pinned; selected `00BF6713`
records are checked independently. Its reconstruction row is unchanged, and
its names-shard row is absent at both revisions. Current Source/header/report bytes and
historical Source snapshots at the input report's declared code revision are
pinned separately. Its aggregate Source-tree hash is not silently treated as
an individual current-file attestation. The inherited stale string receipt
remains qualified and supplies no new ABI proof here.

The JSON includes the complete 201-byte decode, the unlisted NOP distinction,
normal local-flow model, child metadata, exact calls/stores/register schedules,
and canonical/physical pins. Startup, gameplay and binary replacement remain
unproved.
