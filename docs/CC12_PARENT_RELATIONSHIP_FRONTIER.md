# CC12 parent relationship publication frontier

**Unready: Source 0, ready Source packets 0.** Exactly one previously named
frontier was selected: `009258F0`, provisionally
`BSP_SceneNode_AttachToParents`, with one genuine placement caller witness
from `00928860`. No Source, build, fixture, native execution, shared metadata
or Ghidra mutation was performed. No second frontier or root census was searched.

Baseline: `e5e9828543b83f2339e0063e2db179bcfcb56e11`, new named worktree
`cc12_parent_relationship_frontier`. Earlier parent36, removal46 and all
accepted historical families remain immutable and unreplayed.

## Whole native evidence and physical ABI

The complete `[009258F0,009259FB)` body is **267 bytes / 88 instructions /
three CALLs**, SHA256
`887cc8a74fa4d6673d8b7019b7295d31e51728b5fb54b3188b7d3538c2b6e10a`.
Whole hex and all instructions are in the sealed proposal. Live Ghidra and
installed PE match before/after. Each supported query batch verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` and PE SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Entry ECX is the actual subject. The three entry stack DWORDs are hierarchy
parent at ESP+4, world node at ESP+8, and local matrix pointer at ESP+0Ch.
Incoming EDX is not a formal input. The body saves EBX/ESI, retains subject
in ESI and zero in EBX. The unlatched arm additionally saves/restores EDI.
The sole return is `RET 0Ch` at 009259F8, consuming exactly those three
stackwords; ordinary return ESP is entry ESP+16.

The initial byte comparison at 009258F6 tests subject+BC against zero.
Any nonzero byte returns before publishing a relationship or reading the
world/hierarchy/matrix arguments. EAX/ECX/EDX remain entry values on this
arm; saved nonvolatiles are restored. Final flags are CMP8 value minus zero:
CF0/OF0/AF0/ZF0, with PF/SF from the original nonzero byte. This is the
physical byte gate, without imposing Boolean normalization.

The completed unlatched arm ends with CMP EDI,EBX where both are zero:
CF0/PF1/AF0/ZF1/SF0/OF0, `mask 0x8D5 = 0x44`. AF is defined here.
Latch stores, POPs and RET preserve those flags. Completed EAX/ECX/EDX are
callee-defined/volatile; void pseudocode does not justify a zero or subject
return. All of these facts are static evidence, not new execution. Native
x87 and other callee effects remain unexpanded; no blanket FP/DF claims are made.

## What is actually published

The unlatched arm first stores worldNode into subject+30 at **00925906**,
then reads `[worldNode+4]`. There is no unlatched null-world guard or rollback.
The pointed header has `{head0, tail4, count8}` and the subject itself is an
intrusive element with prev+34/next+38. This differs from the accepted raw
list's `{count0, head4, tail8}` root and separately allocated 12-byte nodes.

At **00925935**, the body stores hierarchyParent into subject+3C. This is
distinct from the world-node +30 relationship. A non-null hierarchy parent
owns the embedded head+48/tail+4C/count+50 child chain; the subject uses
prev+40/next+44. A null hierarchy parent instead appends to the separate
header at `[worldNode+8]`, also `{head0, tail4, count8}`.

After both relationships/chains are published, pending +B4/+B8 values are
conditionally dispatched to 009245A0. Both fields are cleared **before**
that call. The local matrix is then copied into subject+74 through native
004134F0; validity bytes +C8/+10C are cleared; each child from subject+48
and child+44 is passed to 0042ED50. Only after this work does 009259EE store
the attachment latch +BC=1. Failure/EH behavior and those dependent effects
are not admitted. No inline prefix is proposed as a whole Source body.

| Native CALL | Operand bytes | Physical caller inputs | Unexpanded prerequisite |
|---|---|---|---|
| 009259B6 -> 009245A0 | `[199,203)` | ECX subject; stacked prior+B8/prior+B4 | actual pending-pair ownership/cleanup |
| 009259C4 -> 004134F0 | `[213,217)` | ECX subject+74; one matrix pointer stackword | native register/stack/x87 copy binding |
| 009259E2 -> 0042ED50 | `[243,247)` | ECX actual child | real recursive child pose invalidation |

No dependency body was recovered or substituted with a provider. Matrix
recovery requires a separately authorized Astra/x87 packet if pursued.

There is **no direct store initializing worldNode+24's category-1
`{count,head,tail}` owned-node root**. The intrusive chains above do not supply
that root's producer ownership. Unknown side effects of the three unexpanded
callees are not asserted. Actual parent construction and registration through
the placement caller's virtual +130 slot remain separate prerequisites.

## One bounded publication caller

The actual `[009288CD,009288E1)` witness is **20 bytes / 10 instructions**,
SHA256 `bd2917e0cf4892c8253e335f0dbb91844dac553c279496d9f79c2452ccbaddb4`:

```text
8b4c24285055518bcee815d0ffff5d84db5b7412
```

It loads ECX from caller ESP+28, pushes EAX, EBP and that loaded ECX,
sets receiver ECX from ESI and calls 009258F0 at 009288D6. The target's own
physical argument reads identify those supplied values as hierarchy, world
and matrix. The earlier origins and EH frame ownership of EAX/EBP/ESI are
not independently admitted by this partial witness. Existing forwarding,
other-caller and vtable census comments remain historical context only.

Bounded caller pseudocode names its lock manager/EnterCriticalSection/
LeaveCriticalSection phase, old-parent virtual +134 before attachment, and
registration virtual +130 afterward when the world relationship changes
and is non-null. That whole caller, its EH/lock owner, exact virtual targets,
latch reset, observer ordering and ownership are not reconstructed here.

## Concrete readiness boundary

The native publication is established, but current admitted Source does not
construct this actual subject/world/hierarchy owner phase, intrusive headers,
pending pair or pose subtree. The current World constructor returns a new
typed `WorldObjectLayout` and delegates actual owner creation/postconstruction
to a host; semantic attachment and native-light contracts use distinct
interfaces/addresses. They do not bind this whole body or its physical fields.
Accepted append/erase in the current canonical domain likewise do not produce
these intrusive subjects or establish category-1 parent+24 ownership.

Required next evidence is genuine physical parent/header construction and
stable lifetime; actual subject latch/pair/pose phase; exact independent
cleanup/matrix/invalidation bindings; and bounded real registration/detach
targets with owning category-root publication. Fabricated partial parents,
manual +30 stores and fake class constructors cannot replace that evidence.
Root alone may register a Source packet after these prerequisites are ready.
Original private heap/CRT, whole world/class closure, EH/failure and gameplay
remain unbound.

## Complete immutable inventory

The selected graph is **12 nodes + 11 edges = 23/24**, including named
incomplete callees, placement lock/import/virtual frontiers, and owner
constructor/header phases. Only one whole frontier and one partial caller
witness were inspected; protected duplicate-string/type4 streams were not
queried. All 9,150 prior actual artifact pins remain unchanged. There are
23 prepared inputs, four supplemental Source-context pins and 19 actual
frozen files, including historical receipt/manifest copies by exact path.
Old Source/metadata original-path associations remain historical/unconsumed.

A post-only proposal script used a reserved Python keyword and stopped
before any phase/query. Its file is preserved; a new corrected helper produced
the proposal. No successful prepare, query, bookend, Native or old recipe was
replayed. The final `local/pr30` inventory is **143 artifacts + two seals**:

| Artifact | SHA256 |
|---|---|
| `proposal.json` | `488632235ed777eea0089fa5e4f1c61970c5f9b4b5cdd928c83b243b85c7cc86` |
| `readiness_receipt.json` | `25632b2c425db3213a87b0da640cdf8dbc194dd9a48e54d86e530210918224f3` |
| `artifact_manifest.json` | `e5841e384ad9365621792aef6672a440e328a8ba5a4e4362cecf41eecfc9efad` |

Only the two exact root output paths are excluded. All helpers, stopped
attempts, whole bytes/instructions, caller evidence and historical metadata
copies are included. Source and ready credit remain zero.
