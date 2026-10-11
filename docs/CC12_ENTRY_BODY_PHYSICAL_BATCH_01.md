# CC12: three conditional byte-analysis packets

The three scoped captures, prewritten manual analyses and first fresh detailed
decoders are complete and independently reviewed. The worker publications were
merged into Main as each packet closed. This batch adds documentation and
evidence; it adds no C++ reconstruction or runtime validation.

| Packet | Captured bytes / units | Native commands | Observed count / epoch | Worker commit |
|---|---:|---:|---|---|
| [C04EF3](CC12_C04EF3_ENTRY_PHYSICAL_01.md) | 7 / 1 | 8 | 65052 / 7572 | `f3040c0cfc838337224a81cbe1782f40028a7ec8` |
| [C04FD5](CC12_C04FD5_BODY_PHYSICAL_01.md) | 9 / 4 | 21 | 65052 / 7572 | `3483cb234c63fe54c28332a6868acf0d98eec832` |
| [C11C21](CC12_C11C21_BODY_PHYSICAL_01.md) | 49 / 20 | 48 | 65052 / 7524 | `8bbf6b8d5bba069ada7684e3a8382bbef85a2690` |

The 77 worker-origin Native commands were launched by Root through fully reviewed
worker drivers. Root authored no additional Native query. Each capture used its
own cooperative reservation and dynamic guards; the reservations did not freeze
the GUI or auto-analysis. Paired byte reads agree within each capture. Before and
after every Native command, the three CLI files were physically read and bound,
totaling 462 physical file reads. No capture was rerun. The two observed epochs
remain separate; count agreement does not make either a permanent baseline.

The three fresh Capstone executions decoded 65 bytes into 25 units. Complete
manual records were written and pinned before their respective first imports.
Root read the decoder and driver sources before launch, then checked retained
actual detail records against the manual records without another import or
decoder execution. The first independent freeze, capture, decoder and publication
reviews passed. These are conditional byte interpretations, not context-verified
physical-body admissions, binary replacement contracts or runtime proof.

## Physical findings and limits

`C04EF3` begins with `83 25 a8 ee 09 01 00`: under default x86-32 operand/address
sizes, `AND dword DS:[0109EEA8], 0`. Normal completion reads the old value and writes
zero; CF/OF/SF clear, ZF/PF set and AF is undefined. The DS base, actual linear
target and descriptive global/function identities are unverified. The instruction
has no LOCK prefix. Only seven of the eight saved owner bytes were analyzed;
`C04EFA` and end-plus-one `C04EFB` remain unqueried. The accepted incoming CALL at
`BF6707` still numerically targets `C04EF3` and encodes return offset `BF670C`.

`C04FD5` contains PUSH zero, relative CALL, POP ECX and near RET. The CALL at
`C04FD7` has displacement -117, next/return offset `C04FDC` and numeric target
`C04F67`. The callee is opaque. POP does not establish ECX=0 across that CALL,
and no stack-balance or return-value contract follows. `C04FDE` remains unqueried.

`C11C21` has 20 units covering its 49 saved owner bytes. Both JNEs encode `C11C47`;
the direct CALLs encode `C11B5E` and `BFBA09`, with return offsets `C11C3A` and
`C11C46`. The indirect CALL at `C11C49` uses memory offset `CE2218` and return
offset `C11C4F`. Its saved effective external association with EnterCriticalSection
is metadata; the bytes do not authenticate that provider. The saved `__amsg_exit`
companion has `no_return=false`; its name does not establish termination. All
flow overrides are NONE, and default/effective flags remain separately recorded.
`C11C52` remains unqueried.

All interpretations assume default x86-32 code mode. Per-address CPU context was
not queried. Full-ESP PUSH/POP/CALL/RET equations additionally require SS.B=1 and
normal completion; code mode alone does not establish SS.B. Applicable segment
bases, rights and live mappings remain unverified. Faults, callee effects, ABI,
startup and gameplay were not validated. The original C11 manual remains intact;
its separately pinned stack qualification is a scope clarification, not a failure.

## Provenance and integration

Each finite input freeze used actual commit
`f7fb43633f7082f188a13abe75042d41c29c1e8f`, 16 saved Git objects and ten LF/index/blob
bindings. Across packets these are 48 object bindings and 30 file bindings, not a
unique-object census. The freezes include eight core inputs and two supplemental
prior Root-batch inputs. Source tree identities were recorded without reading
their bodies: src `01614b9fcb7660481e1e7130a4563ee378655337`, include
`ae7196607ba2e3915302f2c5fa5f1299555c51dc`.

Root publication reviews verified 437 EF, 632 FD and 792 C11 selected original
bindings, plus six actual final Git objects and two public LF bindings per packet.
A separate completion review retained 737 completed publication/delivery originals,
including FD publication copies and post-handoff streams. Counts describe selected
bindings and may overlap; they are not a whole-history inventory.

Main received each closed packet through the Root integration branch. Subsequently
observed upstream `d37f8ee90ce57302f2488fac58c206c2db8ca232` was merged without conflicts.
The pre-publication integrated Main/Root commit is
`9dad241a3d05f0838a4d29100bad80c7f8871be9`, with src
`f2efed3eb4188625f71dd64fdd7090411bac58a2` and include
`8bd7c8a84efd0d38ea851bf38c972eb8e9a792bc`. Those newer identities reflect other
harness work and do not replace the frozen capture inputs. This batch does not
claim to review or validate that other work. Final commit/push bindings are kept
in the Root private handoff because a report cannot bind its own future commit.

## Incident scopes and retained evidence

Historical first failures, stops and minimal corrections remain separate from
the present captures. Current Root inspection retained one argument rejection,
two missing-path inspection errors and four display truncations. These occurred
outside Native/decoder/helper execution. Original tool returns are retained, but
separate full inspection stdout/stderr and input prepins are unavailable. Named
bounded follow-ups do not turn those originals into first-pass successes.

Worker preparation retained EF's two inspection process failures/two truncations,
FD's setup failure/one truncation, and C11's argument rejection/three truncations.
The fresh captures, fresh decoders, publication helpers and Root verification
helpers had no process/comparison failures. Complete helper execution streams and
prepins are retained; this does not imply complete stdio for earlier inspections.

The [batch report](../reports/cc12_entry_body_physical_batch_01.json) embeds Root
review receipts, worker handoffs, reservations, authorization pins and incident
records. Finite private originals and the publication manifest remain under the
Root worktree's `local/cc12_entry_body_physical_batch_01/`; worker originals remain
in their named managed worktrees. No archive or guarded final-proof helper ran.
No game Source bodies or absence census, listing/context/provider/prototype/xref
query, GPR write, build/test/probe, game/OS/audio runtime, ABI or gameplay validation
was added. The continuing reconstruction goal remains active and incomplete.
