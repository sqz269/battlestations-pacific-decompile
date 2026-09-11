# The world's deferred-destroy drain, vtable slot 4

Addresses: 00904390, 009041A0, 00904C40, 004CB0B0, 009037F0, 00484540, 00CE7784 (data),
00CE7788 (data).

Packet `cc_entity_matrix`, worktree `agent/cc-entity-matrix`. Ghidra was **read-only**. Every
descriptive name is a hypothesis, not a recovered symbol. This packet takes the
`world_deferred_destroy` follow-up of `docs/WORLD_ENTITY_UPDATE.md`.

The reconstruction is in `include/bsp/world_deferred_destroy.hpp` and
`src/world_deferred_destroy.cpp`; machine-readable facts are in `reports/entity_matrix.json`.

## The world vtable has five slots

`00CE7784`, written by `004CB04C` in `BSP_World_Construct` and again by `00904C5F` in the
destructor's vptr reset, holds:

| Slot | Target | Role |
| --- | --- | --- |
| `0h` | 004CB0B0 | `BSP_World_ScalarDeletingDestructor` |
| `4h` | **00904390** | the drain, this packet |
| `8h` | 009035D0 | unread; no Ghidra function |
| `0Ch` | 00904BF0 | `BSP_World_UpdateEntities` (`docs/WORLD_ENTITY_UPDATE.md`) |
| `10h` | 004CB370 | tagged `CG_scalar_deleting_dtor_004cb370`, unread |

`00CE7798` is `48127C00`, not a code address, so the interface ends at slot `10h`. The world is
therefore **not** a member of the entity class family: in the entity vtables slot `4h` is
`0042B970` (`MOV EAX,ECX; RET`, an identity accessor) and slot `0Ch` is `0077D3F0`, neither of
which matches. The three small template forwarders that dispatch slot `4h` and compare its result
against `this+14h` (`004C2D60`, `004C2D90`, `0042EC20`) are consumers of the *entity* slot `4h`,
not of the world's.

## The drain

```
00904390  MOV ECX,[ECX+4] ; JMP 009041A0      ; re-point this at the chain header
```

`00904390` is `void __thiscall(world)`, body `00904390..00904397`. It has no logic: it hands the
`world+4h` chain header to `009041A0` and tail-jumps.

`009041A0` is `void __thiscall(header)`, plain `RET`, body `009041A0..00904203`:

```
009041A6  if (header->count_8 == 0) return
009041B0  loop: node = header->head_0
009041B2        prev = node->prev_34 ; next = node->next_38
          ; membership test, 009041B5..009041C2
009041B5        linked = prev != 0 || next != 0 || header->count_8 <= 1
          if (linked) {
009041C8        prev ? prev->next_38 = next : header->head_0 = next
009041DC        next ? next->prev_34 = prev : header->tail_4 = prev
009041EA        node->next_38 = 0 ; node->prev_34 = 0
009041F0        header->count_8 -= 1
          }
009041F4  node->vtable[0](1)                   ; scalar deleting destructor, free the storage
009041FC  if (header->count_8 != 0) goto loop
00904203  RET
```

The membership test is the part worth keeping. A node with neither link is still on the chain when
the count is 1, and is **not** on it when the count is above 1; in that case the loop destroys it
without writing the header at all. This is the inlined "remove if present" helper the node's own
destructor also uses, which is why the loop can safely re-read the head every iteration: a
destructor that unlinks its own node finds `prev == next == 0` and a count above 1 and declines.

Termination has no iteration bound natively. The loop repeats while the count is nonzero and only
the `linked` path decrements it, so a destructor that neither unlinks nor lowers the count would
spin. The reconstruction stops and reports `stalled` in that shape; that stop is not a native exit.

## The header at world+4h

| Offset | Field | Evidence |
| --- | --- | --- |
| `+0h` | head | `009041B0` reads it, `009041D3` writes it, `00904BF7` dereferences it in the update walk |
| `+4h` | tail | `009041E7` writes the removed node's predecessor |
| `+8h` | count | `009041A6` tests it, `009041F0` decrements it |

Node links are `node+34h` prev and `node+38h` next, which are `kUnitOffSiblingPrev` and
`kUnitOffSiblingNext` in `bsp/unit_instance.hpp`. `009037F0` allocates this header and a second
one at `world+8h` as two zeroed `0Ch`-byte blocks (`docs/WORLD_ENTITY_UPDATE.md`).

## What it frees

Slot `0` of each node with the argument `1`. In MSVC a scalar deleting destructor's low flag bit
means "run the destructor and free the storage", so the drain destroys and releases every entity on
the world's chain, in head-to-tail order, and leaves an empty header behind. It frees nothing else:
the header object itself survives, and `world+8h` is not touched.

## Who calls it in a frame

**Nothing found calls it.** The evidence:

* Ghidra's xrefs to `00904390` are one data reference, `00CE7788`, the vtable slot. There is no
  code reference and no thunk.
* `009041A0`'s only caller is `00904390`.
* `00CE7784` itself is referenced twice, by the constructor and by the destructor's vptr reset, so
  the vtable is never used as a type tag or copied into another object.
* A byte search for the world load followed immediately by a slot-`4h` dispatch
  (`CC 19 00 00 8B 01 8B 50 04 FF D2` and `CC 19 00 00 8B 01 FF 50 04`, the two encodings MSVC
  emits in this image) returns nothing. All fifty sites of the bare `8B 01 8B 50 04 FF D2` sequence
  were checked for a world receiver; the three in world-adjacent functions dispatch on
  `[00F88C20]` (`BSP_Game_Render`, `004CA489`) or on an argument object (`004C2D67`, `004C2D97`).

The drain is reachable only through `world->vtable[4]()`, and no static site performs that
dispatch. The honest reading is that it is a base-interface method with no caller in the shipped
image. It is **not** on the teardown path either: the world destructor `00904C40` does its own
passes and then frees the header directly (see below). A caller could still exist behind a cached
base pointer that no byte pattern matched; that is the residual uncertainty.

## The world destructor does not use the drain

`00904C40`, Ghidra body `00904C40..00904E10`, is what `004CB0B0` calls. Its entity teardown is its own:

1. `00904C5F` resets the vptr to `00CE7784` and clears `world+4ACh` and `DAT_00E0AF20`.
2. `00904C73..00904C92` walks `[[world+8]]` through `+44h` calling `00926D90(node, 7)`.
3. `00904C9B..00904CC6` walks `[[world+4]]` through `+38h`; a node with `+5Eh == 0` and
   `+6Ch == 0` whose parent is absent or dead gets `00922FD0(node)`.
4. Three identical rounds at `00904CD8`, `00904D13` and `00904D50`: walk `[[world+4]]` calling
   `vtable[0DCh](0.0f)` on every node whose `+5Ch` is set, then `00904600(world, 0.0f)`, then
   `00874D00(0)`. `00874D00` `BSP_Game_RunExtraFixedStep` is also called once before step 3.
5. `00904D82..00904D9D` frees the two headers with `_free`: `world+8h` first, then `world+4h`.
6. `00904DA8` onwards releases `world+4A8h` through `[00CE2220]` and drains the `world+4B4h` list.

So the destructor runs three zero-delta simulation rounds to let entities retire themselves and
then discards the headers, rather than calling slot `4h`. This confirms the `entity+5Eh` dead flag
as teardown-time, as `docs/WORLD_ENTITY_UPDATE.md` already recorded.

## Host methods, in native call order

| # | Call site | Native | Host method | Owner |
| --- | --- | --- | --- | --- |
| 1 | 00904390 | 009041A0 | the drain applied to `[world+4]` | this packet |
| 2 | 009041B2 | — | `prev_sibling` = `node+34h` | this packet |
| 3 | 009041B9 | — | `next_sibling` = `node+38h` | this packet |
| 4 | 009041CB | — | `set_next_sibling(prev, next)` | this packet |
| 5 | 009041DF | — | `set_prev_sibling(next, prev)` | this packet |
| 6 | 009041F4 | node vtable `0h` | `destroy_node`, the scalar deleting destructor with flag 1 | this packet |

## Reconstruction

| Routine | Coverage |
| --- | --- |
| `009041A0` | complete |
| `00904390` | complete; two instructions, folded into the drain's call contract |
| `00904C40` | partial: `00904C40..00904E0C` read for the entity teardown and the two `_free` calls. `00904E0C` onward, and the SEH funclet, are unread |

## Corrections

### To the Ghidra tag on `00904C40`

The function is tagged `cg_vector_deleting_dtor` and named `CG_vector_deleting_dtor_00904c40`.
It is the world's plain destructor: it is *called by* `004CB0B0 BSP_World_ScalarDeletingDestructor`,
it resets the vptr to `00CE7784` at `00904C5F`, and it takes no count or element-size argument. A
vector deleting destructor is not called by a scalar one. The correct name is
`BSP_World_Destructor`. This packet leaves the tag alone (Ghidra was read-only) and records the
correction for the integrator.

### To the shape of the `0Ch` list header

Two different `0Ch`-byte list objects exist and are easy to confuse, because `009037F0` allocates
the world's two headers with the same `PUSH 0Ch` that `00484540` uses for its cells:

| Kind | Layout | Nodes | Producer |
| --- | --- | --- | --- |
| the world's entity chain, `world+4h` | `+0h` head, `+4h` tail, `+8h` count | intrusive, links at `node+34h`/`+38h` | `009041A0`, `00904BF0` |
| the cell lists at `world+30h`, `+48h`, `+54h`, `+60h`, `+6Ch`, `+0B4h` | `+0h` count, `+4h` head, `+8h` tail | separately allocated `0Ch` cells, `+0h` prev, `+4h` next, `+8h` payload | `00484540`, pushed by `006FE620` |

The two field orders are exact reverses of each other. `docs/SCENE_UNIT_CREATORS.md` describes the
second kind as "five intrusive lists of that node"; they are not intrusive, they allocate a cell
per membership, and they are a different header layout from `world+4h`.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `world_vtable_slots_8_10` | 009035D0, 004CB370 | docs/WORLD_VTABLE_TAIL.md | The two unread world virtuals. `009035D0` has no Ghidra function |
| `world_second_chain` | world+8h, 00926D90 | docs/WORLD_SECOND_CHAIN.md | What the `world+8h` header holds. Its only reader found so far is the destructor's step 2, which walks it through `+44h`, the scene-graph sibling link, and calls `00926D90(node, 7)` |
| `world_destructor_tail` | 00904C40 from 00904E0C | docs/WORLD_DESTRUCTOR.md | The rest of `00904C40` and its SEH funclet, plus the `world+4B4h` list drain |
| `unit_cell_lists` | 00484540, 004837D0, 006FE620 | docs/UNIT_CELL_LISTS.md | The five-plus-one cell lists on the world node and their removal path. Overlaps packet `cc2-*`'s unit instance layout |

## no_ghidra_function

| Start | Inclusive end | Evidence for each boundary |
| --- | --- | --- |
| 009035D0 | unresolved | Named here only as world vtable slot `8h`. `FUN_009035A0` is the enclosing candidate; this packet did not read the body and does not claim the end |

`00904390`, `009041A0` and `00904C40` all have Ghidra functions:

| Address | Ghidra body |
| --- | --- |
| 00904390 | 00904390 - 00904397 |
| 009041A0 | 009041A0 - 00904203 |
| 00904C40 | 00904C40 - 00904E10 |
| 004CB0B0 | present, not re-read by this packet |

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 009041A0 | BSP_EntityChain_DestroyAll | analyzed, reconstructed, build-tested, one focused test |
| 00904390 | BSP_World_DestroyAllEntities | analyzed, reconstructed, build-tested |

Nothing in this packet is fixture-tested, ABI-compatible or game-validated.

## Uncertainties

* No caller for slot `4h` was found; a dispatch through a cached base pointer would not have been
  caught by the byte searches listed above.
* The node's slot `0h` contract is taken from the MSVC convention and from the `PUSH 1`, not from
  reading an entity destructor.
* `00904C40`'s steps 3 and 6 name `00922FD0` and `[00CE2220]` from their call sites only.
* No run-time evidence: `bsp_game.exe` does not reach the drain, which is consistent with finding
  no caller, but absence of a run does not prove absence of a caller (checklist rule 6).
