# The world's per-frame entity update walk

Addresses: 00904BF0, 009037F0, 004CB030, 004DE610, 00904600, 008255B0, 004C40A0, 00904390,
00903670, 00904C40 (world destructor), 00CE7784 (data).

Packet `cc_world_update`, worktree `agent/cc-unit-tick`. Ghidra was **read-only** for this packet:
no renames, comments, prototypes or saves. Every descriptive name below is a hypothesis, not a
recovered symbol, except the Lua binding name `AddMatrixInterpolator`, which is recovered from the
binding registration table and from the literal `luaMW_AddMatrixInterpolator failed:`
(`docs/WORLD_TIMED_ATTACHMENTS.md`).

`00904BF0` itself is already reconstructed as `bsp::update_world_entities_00904bf0` in
`include/bsp/unit_instance.hpp` and `src/unit_instance.cpp`. This packet did not redo it; it
settled the container the walk starts from, re-read the gates, and reconstructed the pass that
closes it. The new C++ lives in `include/bsp/world_entity_update.hpp` and
`src/world_entity_update.cpp`; machine-readable facts are in `reports/world_update.json`.

## The routine

`void __thiscall(world, float scaledDelta)`, `RET 4` at `00904C32`, body `00904BF0..00904C34`.
`this` is `[game+19CCh]`; the routine is slot `0Ch` of the world vtable `00CE7784`. Full listing
read.

```
00904bf2  EDI = ECX                 ; the world
00904bf4  EAX = [EDI + 4]           ; the chain header
00904bf7  ESI = [EAX]               ; the first entity
00904bfb  if (!ESI) goto tail
00904c00  loop: if (ESI->byte_5Ch != 0) {
00904c06      EDX = [ESI]                       ; the entity's primary vptr
00904c08      FLD [ESP+0Ch]                     ; the incoming delta
00904c0c      EAX = [EDX + 0DCh]
00904c12      PUSH ECX / MOV ECX,ESI / FSTP [ESP]
00904c18      CALL EAX                          ; entity->vtable[0DCh](scaledDelta)
          }
00904c1a  ESI = [ESI + 38h]         ; next sibling
00904c1f  if (ESI) goto loop
00904c21  tail: 00904600(world, scaledDelta)
00904c32  RET 4
```

### One container, not several

There is exactly one walk. `[world+4]` is **not** the head dword: `00904BF4` loads the field and
`00904BF7` dereferences it again, so the head is `[[world+4]]`.

`004CB030`, the world constructor, never writes `world+4h`. `004DE69C` runs `009037F0` on the
world immediately afterwards, and that routine allocates two zeroed `0Ch`-byte blocks through
`00BF681B` and stores them at `world+4h` (`00903822`) and `world+8h` (`0090383A`). So the entity
chain header is a small heap object with the head in its first dword, and the world holds two of
them. The second, at `world+8h`, is not touched by this walk.

The chain is intrusive: `entity+34h` and `entity+38h` are the sibling links
(`include/bsp/unit_instance.hpp`, `kUnitOffSiblingPrev` / `kUnitOffSiblingNext`) and `entity+30h`
is the parent, which is the world itself. Nothing here is an MSVC `std::list`; the interpolator
list of `docs/WORLD_TIMED_ATTACHMENTS.md` is the only `std::list` in the world object.

### The gates

The packet brief asked for the paused, dead and culled gates. **There is only one gate in this
body**, the byte at `entity+5Ch` (`00904C00`), and it is the same byte the two other walks over
this chain test:

| Walk | Address | Gate | Note |
| --- | --- | --- | --- |
| this update | 00904C00 | `entity+5Ch` | calls slot `0DCh` with the frame delta |
| world destructor | 00904CD8 | `entity+5Ch` | calls slot `0DCh` once with `FLDZ`, a zero delta |
| world destructor | 00904CA4 | `entity+5Eh` | the dead flag, a different byte |

So "dead" (`+5Eh`) is a teardown-time concept, not a per-frame one. The pause gate is upstream:
`004E50B0` in `BSP_Game_OnMove` requires `game+634h == 0 || game+635h != 0` and then
`game+5D4h == 0Dh && game+7184h == 0` before phase 18 runs at all
(`docs/GAME_SIMULATION_GATE.md`). There is **no culling test** anywhere on this path: a unit that
is off screen still runs its full `008255B0`.

`world+4ACh`, the ready byte `004CB098` sets to 1, is read by `00481640` in the entity-manager
forwarder, not by this walk.

### The delta

`004C40A0` loads `game+21F0h` with `FLD`/`FSTP` around the `PUSH ECX` slot (`004C40B9`), so the
world receives the **scaled** frame delta of `docs/GAME_FRAME_CONTROL.md`. It is passed on to the
per-entity virtual unchanged at `00904C08`, and again to `00904600` at `00904C21`.

`00904600` never reads it. The body has no access to the argument slot at `ESP+2FCh`; it works
entirely off the mission clock `DAT_00F876A4`. Passing the delta there is dead code.

### What runs after the walk

`00904C2B` calls `00904600` `BSP_World_UpdateMatrixInterpolators`, documented in full in
`docs/WORLD_TIMED_ATTACHMENTS.md`. Nothing else follows: the epilogue is two pops and `RET 4`.

The world's deferred-destroy drain, vtable slot `4` (`00904390`, `MOV ECX,[ECX+4]; JMP 009041A0`),
runs on the **same** `world+4h` chain header, but it is not called from here. Neither is the
activation flush `00903670`, which `004E4A40` calls separately at phase 20 with the same ECX.

## Host methods for one unit pass, in native call order

What an executable has to implement to get one unit from the in-mission tick through its own
update and out again. `address` is the call site; `owner` is the packet that owns the contract.

| # | Call site | Native | Host method | Owner |
| --- | --- | --- | --- | --- |
| 1 | 004C40AD | 00875BB0 | `step_fixed_simulation_00875bb0` | `docs/IN_MISSION_SUBSYSTEM_TICK.md` |
| 2 | 004C40B4 | 004C3CB0 | `build_local_player_unit_lists_004c3cb0` | `docs/LOCAL_PLAYER_UNIT_LISTS.md` |
| 3 | 004C40CE | 00904BF0 | `update_world_entities_00904bf0` | this packet |
| 4 | 00904BF4 | — | `chain_head` = `[[world+4]]` | this packet |
| 5 | 00904C00 | — | `entity_active` = `entity+5Ch` | this packet |
| 6 | 00904C18 | 008255B0 | `update_entity` (vtable `0DCh`) | `docs/UNIT_INSTANCE_UPDATE.md` |
| 7 | 0082584B | 0092D730 | `controller_body_axis_speed` | `docs/UNIT_CONTROLLER.md` |
| 8 | 0082586B | 00815AA0 | `publish_effect_intensity` | `docs/UNIT_CONTROLLER.md` |
| 9 | 008259DF | 0092BE80 | `controller_update` (empty body) | `docs/UNIT_CONTROLLER_UPDATE.md` |
| 10 | 00825D5A | 008252C0 | `update_engine_audio` | `docs/UNIT_TIMED_SUBUPDATES.md` |
| 11 | 00825D6C | 00956600 | `update_unit_timers` | `docs/UNIT_TIMED_SUBUPDATES.md` |
| 12 | 00825D7E | 00834E90 | `update_propellers` | `docs/UNIT_TIMED_SUBUPDATES.md` |
| 13 | 00825DB4 | 00815370 | `set_effect_group_scalar` per part | `docs/UNIT_CONTROLLER.md` |
| 14 | 00904C1A | — | `next_sibling` = `entity+38h` | this packet |
| 15 | 00904C2B | 00904600 | `update_matrix_interpolators` | `docs/WORLD_TIMED_ATTACHMENTS.md` |
| 16 | 00904AE3 | vtable `88h` | `entity_set_local_matrix` | `docs/WORLD_TIMED_ATTACHMENTS.md` |
| 17 | 00904B09 | 0042ED50 | `entity_invalidate_subtree_pose` | this packet |
| 18 | 00904B2A | vtable `0D8h` | `entity_refresh` | `docs/WORLD_TIMED_ATTACHMENTS.md` |
| 19 | 004C40DD | 00447B80 | `update_game_dynamics_00447b80` | `docs/GAME_DYNAMICS_LIST.md` |

Steps 7 through 13 are the calls inside `008255B0` that this packet touched; the full twelve-step
order of that routine is in `docs/UNIT_INSTANCE_UPDATE.md` and is not repeated here.

## Reconstruction

`include/bsp/world_entity_update.hpp` adds the chain-header constants and the interpolator model.
It does **not** redeclare `WorldEntityUpdateHost` or `update_world_entities_00904bf0`; those stay
in `bsp/unit_instance.hpp`. The existing reconstruction takes a `std::vector<bool>` of gate bytes
in chain order, which is a faithful projection of the walk now that the chain header is settled:
the head indirection and the `+38h` link are traversal, not behaviour.

Nothing here is a drop-in binary replacement. The routines are `__thiscall` on objects whose full
layouts are not recovered.

## Corrections

### To `include/bsp/world_construct.hpp`

`kWorldActivationChainHeadOffset = 0x04` carries the comment `memset only`. That is wrong:
`009037F0`, called from `004DE69C` immediately after the constructor, stores a heap pointer there.
The dword at `world+4h` is a pointer to a `0Ch`-byte chain header, and the entity chain head is
its first field. `world+8h` receives a second, identical header from the same routine.

### To `docs/UNIT_INSTANCE_UPDATE.md`

That doc lists `00904600` as "read, not analyzed; described as an external contract" and records
under uncertainties that "`00904600`'s per-attachment work was not read past its head". It is now
read in full; see `docs/WORLD_TIMED_ATTACHMENTS.md`. Two specifics correct the earlier summary:

* The list at `world+4B0h` is not a list of *attachments* with deadlines. It is a list of matrix
  interpolators registered from Lua, each holding a captured local matrix and a pose offset.
* `DAT_00F876A4` is used as an absolute clock against a per-record **start** time plus a duration,
  not as a deadline stored in the record.

### To `docs/GAME_WORLD_ENTITIES.md`

That doc says of `00903670` that "the chain starts at `[this+4]`". The chain starts at
`[[this+4]]`; `[this+4]` is the header object. The observable behaviour it describes is unchanged.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `world_deferred_destroy` | 00904390, 009041A0, 00904C40 | docs/WORLD_DEFERRED_DESTROY.md | The world vtable slot 4 drain over the same `world+4h` header, who calls it in a frame, and what `world+8h`'s second header holds |
| `entity_refresh_virtual` | 00955970, 00CFC3D0+0D8h | docs/ENTITY_REFRESH_VIRTUAL.md | Slot `0D8h`, the refresh the interpolator pass ends each record with. `00955970` has no Ghidra function; it reads `entity+4A4h`, `entity+3Ch` and the pose block at `+0CCh` and calls the scene node's vtable `34h` |
| `entity_local_matrix_slot` | 00CFC3D0+88h, entity+74h | docs/ENTITY_LOCAL_MATRIX.md | Slot `88h`, the setter the interpolator writes through, and the rest of the `entity+74h` local 4x4's producers |

## no_ghidra_function

none. Every address named by this packet has a Ghidra function:

| Address | Ghidra body |
| --- | --- |
| 00904600 | 00904600 - 00904BE2 |
| 00905080 | 00905080 - 009051DC |
| 008ADE00 | 008ADE00 - 008AE025 |
| 009037F0 | 009037F0 - 00903848 |
| 0042ED50 | 0042ED50 - 0042ED75 |
| 00815370 | 00815370 - 008153DD |
| 00815AA0 | 00815AA0 - 00815D15 |
| 009045C0 | 009045C0 - 009045FE |

`00904600`'s stored body ends at `00904BE2` but contains one flow gap, `00904BCA..00904BD5`, which
Ghidra leaves undisassembled because it treats the `_free` call at `00904BC5` as non-returning.
The bytes are on disk and are read in `docs/WORLD_TIMED_ATTACHMENTS.md`; the integrator should run
`python tools/ghidra_flow_repair.py 00904600 --apply` before re-exporting.

`00955970`, the `MDestroyer` target of vtable slot `0D8h`, has **no** Ghidra function. It is not
named by this packet; the follow-up above owns it.

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 00904BF0 | BSP_World_UpdateEntities (evidence appended) | analyzed, reconstructed elsewhere, build-tested |
| 009037F0 | BSP_World_AllocateEntityChains | analyzed, documented |
| 004CB030 | BSP_World_Construct (evidence appended) | analyzed |
| 0042ED50 | BSP_SceneNode_InvalidateSubtreePose | analyzed, reconstructed, build-tested |

Nothing in this packet is fixture-tested, ABI-compatible or game-validated.

## Uncertainties

* `world+8h`'s header has no reader in anything this packet read. Its purpose is unknown.
* `009037F0`'s second stack byte is never used by the body; only `AL`, the first, reaches
  `world+4A4h`. Both call-site values are `1`.
* The entity vtable slots `88h` and `0D8h` are read from their call sites and from `00955970`'s
  head. Slot `88h`'s body was not read at all.
* `world+4A8h`, zeroed by `009037F0`, has no reader in this packet.
