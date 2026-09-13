# Native unit observer prefix producer

Packet `orch6_unit_observer_endpoint_s`, base `9cc35911`, represents only the
actual observer-prefix stores inside existing unit constructors. It adds no
complete native constructor, allocation, callback implementation or runtime
unit. Every source reconstruction in this packet is a **partial projection**.
Ghidra access was read-only against `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; the root separately defined the two short
deleting adjustment thunks.

## Two roles in the same owning unit

`NativeUnitObserverPrefixStorage` is the first 20h bytes of the native unit,
with two existing `NativeObserverOwnerStorage` members. It adds no competing
definition of that 10h layout and has no default initialization or destructor.

| Unit offset | Existing raw member | Producer / initial value |
|---|---|---|
| 00h | `observed_00.native_vtable_00` | 00925CFF: CECCC8 |
| 04h | observed edge data | 00925D0A: zero |
| 08h | observed edge count | 00925D0D: zero |
| 0Ch | observed edge capacity | 00925D10: zero |
| 10h | `callback_10.native_vtable_00` | 00925D13: CE3CD4 |
| 14h | callback edge data | 00925D1A: zero |
| 18h | callback edge count | 00925D1D: zero |
| 1Ch | callback edge capacity | 00925D20: zero |

The producer is `00925CE0..00925EF3`, which saves the original ECX in ESI at
00925CFB and sets EBX to zero at 00925CFD. These stores are inlined; no shared
base-constructor call occurs between them. The next actual subobject
constructor is 00925490 at 00925D34 for the distinct weak owner at unit+24h.
The byte store at unit+20h, its padding, the weak owner and every subsequent
entity field are outside these source initialization fragments. Calling a
fragment on an owner with live arrays would discard ownership; these are
construction-stage operations, not reset methods.

Other inspected CE3CD4 producers (0051E950, 005178B0, 0068A8A0) likewise inline
its stores at their own callback-base offsets. They are larger, unrelated
constructors, not a recovered callable constructor for this unit prefix.
This is a bounded finding about the inspected paths, not an exhaustive claim
that no such helper can exist anywhere in the executable.

## Construction stages and table identity

Two later store projections replace only the two table words:

| Source stage | Native envelope | Observed / callback table | Coverage |
|---|---|---|---|
| `initialize_unit_observed_prefix_00925cff` | 00925CFF..00925D12 | CECCC8 / untouched | Partial: four owner stores; excludes native frame bookkeeping and the rest of 00925CE0 |
| `initialize_unit_callback_prefix_00925d13` | 00925D13..00925D22 | untouched / CE3CD4 | Partial: four owner stores; rest of 00925CE0 omitted |
| `publish_scene_observer_tables_00925d44` | 00925D44..00925D50 | D19120 / D19104 | Partial: two stores; rest of 00925CE0 omitted |
| `publish_game_entity_observer_tables_00928662` | 00928662..0092866E | D192E0 / D192C8 | Partial: two stores; rest of 00928630 omitted |

00928630 calls the actual scene constructor at 00928651 before its table
stores. Existing intermediate class constructors and the final leaf then
publish their own table pairs. Source publication does not execute those
constructors, repair their missing fields or skip an actual lifecycle stage.
All native addresses stored in the source are evidence identities, not
callable process vtables.

The 21 supported creator identities reuse `unit_world_registration`'s existing
map. Each actual leaf constructor's adjacent `MOV [ESI],primary` and
`MOV [ESI+10h],callback` instructions were checked in the live function body
and against installed bytes. No callback table is calculated by subtracting
a guessed distance from the primary table. Each row below is a **partial
two-store projection**; all other instructions of its constructor are omitted.

| Creator | Constructor | Primary store | Observed / callback table |
|---|---|---|---|
| `006FE590` | `006FE460` | `006FE46D` | `00CFC3D0` / `00CFC3B8` |
| `006FB430` | `006FB300` | `006FB30D` | `00CFB738` / `00CFB71C` |
| `0074BE00` | `0074BB00` | `0074BB2C` | `00CFFA30` / `00CFFA18` |
| `006EB290` | `006EB160` | `006EB170` | `00CFA778` / `00CFA75C` |
| `006DFEF0` | `006DFC90` | `006DFC9D` | `00CF90B0` / `00CF9094` |
| `008531A0` | `00852F10` | `00852F27` | `00D0BF80` / `00D0BF68` |
| `00857E20` | `00857CD0` | `00857CDD` | `00D0C648` / `00D0C630` |
| `00758D30` | `00758550` | `00758598` | `00D01630` / `00D01614` |
| `008091D0` | `0074E0F0` | `0074E0FF` | `00D00070` / `00D00058` |
| `0084CA50` | `0084C920` | `0084C92D` | `00D0BA80` / `00D0BA68` |
| `0074E540` | `0074E2D0` | `0074E2DD` | `00D00308` / `00D002F0` |
| `007DDAE0` | `007DD9B0` | `007DD9BF` | `00D06920` / `00D06908` |
| `00956390` | `00951B60` | `00951B6F` | `00D19D28` / `00D19D10` |
| `009564E0` | `00951C40` | `00951C4F` | `00D1A000` / `00D19FE8` |
| `00956240` | `00951D20` | `00951D2F` | `00D1A2D8` / `00D1A2C0` |
| `007D7850` | `007D7720` | `007D772F` | `00D06638` / `00D06620` |
| `006D3110` | `006D1C20` | `006D1C68` | `00CF8C08` / `00CF8BEC` |
| `00848380` | `00848080` | `008480B1` | `00D0B770` / `00D0B754` |
| `0074DF10` | `0074DCC0` | `0074DCCF` | `00CFFDE0` / `00CFFDC8` |
| `00747000` | `00745940` | `0074597D` | `00CFF3F8` / `00CFF3E0` |
| `006F5C10` | `006F5610` | `006F564E` | `00CFB028` / `00CFB00C` |

`publish_unit_leaf_observer_tables_for_creator` performs only that selected
pair of stores and preserves both existing edge arrays. Its unsupported
creator path returns false without writes; it supplies no default leaf.
Every primary slot+4 points to the verified identity getter 0042B970. The
initial CECCC8 table instead selects the verified null getter 00522E90, so
initializing the base prefix alone does not establish a finished unit getter.

Secondary callbacks are distinct from the obstacle-node CF5C94 callbacks.
Across these 21 tables, slot+4 targets are 007C6E90, 0080DFC0 and 00952050;
slot+8 targets are 0042B120 and 008455A0. Only 0042B120 has the already proven
RET4/no-op contract here. The other targets are recorded by address with
behavior **unread**, and 0080DFC0/00952050 currently lack function definitions.
The source does not dispatch or replace any of those callbacks.

## Concrete canonical-identity mapping

At this base, `GameUnitsHost::Impl` keeps stable `unique_ptr<GameUnitSlot>`
owners. `world_list_push_back_00484540` stores `&unit` in each existing list
node; the first member of `GameUnitSlot` is semantic `GameUnitRow`, not a
native observer table. Casting `GameUnitSlot*` to `NativeObserverOwnerStorage*`
would read unrelated metadata. S does not edit this host.

The integration contract is to embed **one** `NativeUnitObserverPrefixStorage`
member in that same existing stable slot. `NativeUnitObserverAlias` is a
borrowed pair of the existing canonical unit identity and a reference to that
member. It owns no memory and creates no second unit or endpoint registry.
The root's host binding can spell the relationship directly:

```cpp
// Existing owning slot gains one raw prefix member, initialized by its
// corresponding source construction stages. This is an integration sketch.
NativeUnitObserverAlias alias{&slot, slot.observer_prefix};
auto& first = alias.prefixes.observed_00;
auto& callback_owner = alias.prefixes.callback_10;
```

The world lists continue to hold `alias.canonical_unit` (`&slot`). Observer
registration, the obstacle node's owner+14 cell, notification arguments and
identity-getter comparisons all use **the same `&first`**. A result that must
return to a world-list consumer is resolved through the existing slot and
this binding, not reinterpreted as the semantic object and not substituted
with `canonical_unit` during observer equality. A reverse lookup can compare
`&slot.observer_prefix.observed_00` while traversing the existing slot owners;
it does not require a second authoritative unit map.

The secondary base is `&alias.prefixes.callback_10`, exactly 10h after this
raw prefix's first base. It is not necessarily the semantic slot's address
plus 10h: the raw member's offset within the semantic slot is a source layout
choice. Both aliases must remain stable for all registered edges, queued
dispatch entries and active callbacks, and must use the same application
observer lifetime/publication owners. Metadata table pairs cannot be invoked
as C++ vtables.

For a source slot-zero deleting callback, resolve the same canonical owner
through this binding before invoking its whole-unit deleting provider.
Subtracting 10h identifies the raw prefix only; it does not recover the
allocation base of a semantic `GameUnitSlot` that embeds that prefix.

## Destruction, notification and deleting identity

The complete game-entity destructor 009287B0 restores D192E0/D192C8 at
009287CD/009287D3, performs its own cleanup, then calls 00925780 at 00928847.
The latter restores D19120/D19104 at 009257A7/009257AD and performs substantial
entity, parent-list, array and weak-owner cleanup before the observer tails.
S does not replace either complete destructor.

| Native site / function | Actual receiver / target | Contract established from body |
|---|---|---|
| 0092589D / 00925780 | ECX=unit+10h -> 00695870 | Restore CE3CD4, nested locked count, callback detach00695530 if needed, free its backing array; do not free unit |
| 009258AC / 00925780 | ECX=unit -> 00695760 | Restore CECCC8, nested locked count, observed detach006953C0 if needed, free its backing array; do not free unit |
| 00925F03 / 00925F00 | unit -> 00925780 | Whole entity scalar deletion; flags bit0 gates operator delete00925F10 -> 00BF6989; EAX=unit, RET4 |
| 009289E3 / 009289E0 | unit -> 009287B0 | Whole game-entity scalar deletion; flags bit0 gates operator delete009289F0 -> 00BF6989; EAX=unit, RET4 |
| 009258E3 / 009258E0 | callback base minus10h -> 00925F00 | Exact eight-byte SUB ECX,10h; JMP deleting method, preserving stack flags |
| 009287A3 / 009287A0 | callback base minus10h -> 009289E0 | Same adjustment to actual whole game entity |

The two endpoint cleanup operations are different: observed detach matches
`edge.first_04`, while callback detach matches `edge.callback_owner_08`.
Do not call callback destruction twice or apply its array owner to the
primary prefix. The existing base has callback cleanup; the separate observed
lifetime S packet supplies primary cleanup. Neither cleanup resets dangling
array fields after freeing. Full owning deletion must free the existing unit
allocation once, never the address of an embedded source prefix member.

The raw scalar-delete tails contain `ADD ESP,4` at 00925F15/009289F5 after
their allocation-release calls, then return the original ESI and execute
RET4. Those two ADD instructions currently lack Ghidra body membership;
the retained live bytes and installed disassembly establish the cleanup.
S does not reconstruct these whole deleting routines or repair their flow.

The secondary D19104/D192C8 slot-zero words select the adjustment thunks,
not standalone callback-base deletion. Root saved and exported both bodies;
`reports/observer_endpoint_function_definitions.json` records that operation.
The two base tables CECCC8/CE3CD4 have their own standalone deleting methods;
those do not justify freeing a member of the larger unit directly.

Notification belongs to the independently reconstructed R path: immediate
009263C0 calls conditional00925C40 after its flag stores, and queued009273A0
calls direct00696330 after its own presence sample. Delivered first/getter
identities must be the same alias registered earlier. Those notifications do
not replace the native later destructor chain. S adds no notification or
whole-unit deletion shortcut.

## Verification and limits

The focused ignored probe executes 25 original store envelopes (335 original
bytes: 330 owner-store bytes plus five bytes of native frame bookkeeping).
There are no relocations. The fixture supplies the established ESI/EBX inputs,
adds one RET per fragment and balances the first fragment's native PUSH EDI
with one harness POP EDI. It compares all 128 bytes of each before/prefix/after
fixture, including the untouched other prefix and surrounding bytes. This is
store-fragment evidence, not a full constructor or native ABI test. One
nonallocating creator confirms the unsupported path leaves storage unchanged.
All 25 paired cases passed: 3,200 compared bytes per side, one unsupported
input, and zero mismatches.
The MSVC Win32 Release build and both existing CTests passed.
`verify_report_calls.py` checked all 15 direct call/tail-jump context rows
with zero failures, including the root-defined secondary adjustment thunks.

Exact build/probe results, bytes, source, executable, objects and every
linker-searched library are retained in `local/unit_observer_proof_manifest.json`.
The source is added to this worker build through ignored
`local/unit_observer_before.cmake`; root owns the production source-registry
append. No new broad test suite or tracked runtime fixture is added.

No game run, complete unit constructor/destructor, arbitrary callback,
allocator, FH3/SEH, OOM or concurrency equivalence is claimed. The borrowed
alias layout is a concrete source integration contract; no runtime binding
has been installed by this worker.
