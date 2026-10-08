# Native unit scene-base lifetime Source readiness

Status: **read-only readiness complete; no cohesive parent Source packet is ready**.
The four complete parents are accounted for, but the application does not yet own
their raw ID tables, pending-init registry, world links or complete virtual and
exception behavior. Source changes, reconstruction credit and ready wiring are **0**.

## Scope and physical evidence

Work started at accepted main `b25c2f49ca89e5f9ba5f2f2de5d4bdeadc5bb29b` and worker merge
`89b5b6e56c81ff4fdaa6b06c3b8a7d54dee8d1a1`. The only tracked outputs are this document and
[`cc12_native_unit_scene_base_lifetime_source_readiness.json`](../reports/cc12_native_unit_scene_base_lifetime_source_readiness.json). Ghidra batches verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 little-endian 32-bit.
The complete installed PE is retained. Its SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Original whole body | Bytes | Instructions | Calls | Incoming and return ABI |
|---|---:|---:|---:|---|
| `00925CE0..00925EF3` SceneNode construction | 532 | 127 | 5 | ECX actual root; EAX same root; RET |
| `00928630..0092878F` GameEntity construction | 352 | 81 | 7 | ECX root, two DWORD formals; EAX same root; RET 8 |
| `00925780..009258C2` SceneNode plain destruction | 323 | 94 | 13 | ECX actual root; RET; no root free |
| `009287B0..0092885B` GameEntity plain destruction | 172 | 42 | 4 | ECX actual root; RET; no root free |

All 1,379 physical bytes / 344 instruction starts match fresh Ghidra exports.
All 29 call sites, whole normal control flow, profiles and stack/EH-state writes
are retained. There is no x87 instruction in these parents; the existing matrix
copy callee has its own sixteen x87 load/store pairs.

## Whole construction contract

`00925CE0` builds observed `+00`, callback `+10`, then weak owner `+24`
(`00925D34`). It stamps the SceneNode profiles `D19120/D19104/D190FC`,
clears child head/tail/count `+48/+4C/+50`, and copies a stack identity matrix
to `+74` at `00925DCA`. It initializes the two native string headers at
`+154/+158` and `+15C/+160`, along with the physical flag/scalar stores.
A second distinct stack identity copy goes to the **same** `+74` at `00925EA0`.
Both copies and their positions in the store sequence belong to the whole body.

Raw resize `0041DD40` receives actual header `+154`, zero length and preserve=0.
The following nonnull-data arm rereads the current data and length for
`memcpy(data,null,length)`. The ordinary fresh-zero case avoids that arm, but
the physical arm is retained; this audit does not invent an unreachable assertion.
The body does not initialize world `+30`, hierarchy parent `+3C`, or link pairs
`+34/+38/+40/+44`. Those require an established preimage and later publication.

`00928630` first calls this SceneNode constructor. It replaces a transient
`CFF378` profile at `+170` with `D192BC`, installs root/callback/weak profiles
`D192E0/D192C8/D192C0`, and initializes the name header `+178/+17C`.
At `00928695` it calls **SceneRegistry** getter `00928240`, captures its actual
`+4` tracked section, enters it if nonnull, then increments depth `+18`.

At `009286B8`, only the **low byte** of formal 1 selects the registry:
nonzero means primary `F89A08`; zero means alternate `F89A5C`.
The full second DWORD is forwarded unchanged as the requested ID. Allocation
`009517C0` receives the same actual root, and AX is stored at `+174`.
Current semantic selectors use full `int != 0`: formal `0x100` therefore exposes
a real difference. This readiness packet records the difference without changing code.

After `+184=0` and `+180=9`, the constructor calls real vslot `+148`
with mask `1FF` and value `9` (`00928711`). Physical Original cell
`D192E0+148 = D19428` contains `00927D20`. Nine holder DWORDs `+1AC..+1CC`
become `8`; the current `F876A4` cell is loaded into `+1D0`. Pending-init
producer `00926BE0` runs at `00928760` **while the outer section remains held**.
The final byte `+1D4` is cleared, tracked depth is decremented before Leave,
and the same root returns. Original literal profile words do not provide Source dispatch.

## Whole destruction and ownership constraints

`009287B0` stamps GameEntity profiles and zero-extends the current low WORD ID
from `+174`. It compares that value, signed, with **current** primary count
`F89A10`, choosing primary on JL and alternate otherwise (`009287F5/00928809`).
ID release `009516D0` occurs first. This body takes no ID lock and does not clear
the ID; a caller lock or failure rollback cannot be invented from that absence.
It releases name `+178/+17C` through actual pool getter `00419CC0` and
`00BD1510`, then calls the SceneNode destructor.

`00925780` stamps SceneNode profiles and first releases `+168` through
`00740270`, using the actual global `E1AEA0`. Its child loop repeatedly reads
the **current** count `+50` and head `+48`; a nonnull child receives slot0 with
deleting flag `1` at `009257ED`. The next iteration reloads both fields.
There is no cached bound or synthesized progress rule for a null head.

At `009257F5/F8`, the destructor unconditionally reads root.world `+30`
and `[world+4]` before calling `00903F30`. A freshly zeroed, merely constructed
root is therefore **not proven valid for immediate normal destruction**.
If parent `+3C` exists it unlinks through `parent+48` and clears `+3C`;
otherwise it unlinks through `[world+8]`. Only the parentless case with `+B8`
nonzero removes from the separate embedded `world+0C` list through `004845A0`.
The pointed chain headers are head/tail/count; the embedded list is count/head/tail.

Second string `+15C/+160` is released before first string `+154/+158`.
For every nonnull string, the block, length+1 and argument `1` are captured
**before** the actual pool getter, then passed to raw pool release.
Actual weak `+24` is destroyed next (`00925891`), callback `+10` next
(`0092589D`), observed root `+00` last (`009258AC`). Neither plain destructor
frees the allocation root.

## Current Source providers and missing owners

| Domain | Current evidence and admission boundary |
|---|---|
| Weak handle and physical pool | One `GameSingletonHost::weak_owners` domain binds actual shared manager `01090AA0`; `GameNativeWeakPoolProcess` owns `0109CE90/0109CE94` and shares the physical pool head `E188B4`. Startup `00CD8A60` precedes consumers. The actual weak view must be the root's `+24` identity. |
| Observer/callback | `GameObserverRuntime` owns `E198DC/E198E0/E198E4` and dispatch initialization. Concrete `CF7E64` edges are supported; unknown deleting profiles are rejected. This is not complete root/child scalar dispatch. |
| Native strings/matrix | Actual eight-byte-header resize, permanent string process manager/pool/gate `01090AA0/AA8/AA4`, raw pool release and complete matrix helper exist. |
| ID tables | Real `0x54` raw storage and `00951660/009517C0/009516D0` APIs exist, but no application producer owns/constructs canonical pair `F89A08/F89A5C`. Semantic vector tables and borrowed field views do not establish that ownership. |
| Registry/pending-init | `00928240/00928080` SceneRegistry and `00924810/00924100` pending-init lock are distinct unresolved owners. Pending-init uses owner `F899CC`, head `F899D0`, count `F899D4`. Existing Native owners cover destroy `F899A8` / kill `F899B4`, with different `009248D0/F899E8` lock. |
| Application pending list | `GameMissionLuaHost::push_pending_entity` deduplicates IDs into a semantic vector. It is not same-root-pointer publication by `00926BE0`. |
| World/hierarchy | Current `GameWorldHost::construct` and unlink projections use vectors/indices. They do not own the raw pointed headers, list nodes and memberships required by these parents. Raw `004845A0` alone does not establish those producers. |
| Profiles/effects | Actual role vslot `00927D20`, scalar profiles `00925F00/009289E0`, child deletion and weak secondary deletion need complete Source providers. The current semantic role method omits part of notification behavior. `00740270/007400C0`, actual `E1AEA0` ownership and the `F876A4` binding also remain unresolved. |

Current game units retain a partial `0x20` observer-prefix owner and semantic
`ShipAiWakeTrail`. The application instance callback still returns scene metadata.
These do not become complete raw scene/unit roots through a cast or shadow copy.
Existing wake and lower leaves remain useful, but do not close parent ownership.

## Exception and ABI qualification

Normal-body EH states are Scene construction `-1→0→1→4`, GameEntity construction
`-1→0→1→2`, Scene destruction `4→3→2→1→0→-1`, GameEntity destruction `1→0→-1`.
The saved FS chain in GameEntity construction is entry ESP-12; the decompiler's
guard profile `CE37FC` is a distinct local and is not the previous exception chain.
Static stack/branch analysis visits all instructions and balances all normal returns,
using explicitly recorded external-callee argument-cleanup assumptions.

Handler entries `00CA6CBA`, `00CA6FAE`, `00CA6C72`, `00CA6FD6` lack exact current
function starts. Their nearest named funclets are only routing hints. No handler,
table or funclet body was freshly analyzed outside the four-address lease.
Lock-guard cleanup, partial-base cleanup, ID/publication rollback and fault behavior
remain open. Normal-body EH state numbers do not prove those exception paths.

## Next cohesive frontier

1. Read complete registry/pending-init lifetime candidates `00928080`, `00928240`,
   `00924100`, `00924810`, `00926BE0`; recover actual CRT initializer extents and
   registration for the raw ID pair and pending-init owner, including teardown.
   Physical routing sites are `00CD39A0/F899CC`, write xref `00CD39AF/F899D0`,
   `00CD3A8C/F89A08`, `00CD3AA0/F89A5C`, with shutdown ranges `00CDF4C0/D0`.
   The nearest `00CD3940` label owns an accepted 37-byte body; it does not own
   these later bytes merely because a scan prints that name.
2. Establish actual world and hierarchy producer/withdrawal closure at
   `009037F0`, `009258F0`, `00928860`, `00903F30`, `00924710`, using existing
   raw `004845A0` only within its real root/node/allocation contract.
3. Close functional profiles, residual lifetime effects and all four exception
   entries above. Then review a real ordinary owner whose allocation, publication,
   borrowers, quiescence, unwind and freeing form one coherent lifecycle.

These are readiness frontiers, not newly ready Source packets. No scratch root,
tiny prelude, fake profile, public opaque lifecycle callback or isolated header
clear qualifies as a reconstructed parent or a real application producer.

## Artifacts and verification limits

The retained historical worker archive has 1,905 members. Sixteen selected whole
core objects match unique complete members; five application objects are correctly
absent from that core archive. All 21 whole COFF objects retain complete sections,
symbols, ordered relocations and 608 bounded function extents, including separate
compiler/EH helpers. Fourteen API groups are selected for readiness comparison.
The archive SHA-256 is `46b565d40915f9e1472d75d9b0157328ec5ee3cc003968c31555a0d8ee57c8e8`.

All 3,990 physical pin records
and 4,029 current Source inventory records rehashed successfully.
Actual compiler/linker files and historical compile/read records are retained.
These are historical objects plus current dependencies, not fresh compiler-consumed
preimages. No normal build was needed for the two documentation outputs; no tests,
probes, Original/new entry calls, Ghidra mutations or application wiring occurred.
There is no final-link/COMDAT, loaded-target, full ABI or gameplay acceptance claim.

Evidence: `local/cc12_native_unit_scene_base_lifetime_source_readiness_evidence.zip`
(66,252,450 bytes; 4,071 entries).
Every ZIP CRC and every member SHA-256 passed. ZIP SHA-256:
`88f23f7f926b7a7c3cbeae75ce21523232b367e5cd450d6ecde2257f9e80d678`.
Manifest SHA-256: `b6d619f321dee019de9691c3c10455bb8a9daffac51c66791e7db423e1da500f`.
Final tracked document/report are generated after sealing and intentionally remain
outside the ZIP to avoid a circular hash dependency. Detailed parent ordering,
provider qualification, frontier and per-file evidence are in the linked report.
