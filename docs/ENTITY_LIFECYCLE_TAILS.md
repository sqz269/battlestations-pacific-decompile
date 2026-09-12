# Entity lifecycle tails (packet `cc2_entity_lifecycle_tails`)

Addresses: 00927050, 009269B0, 0077A7E0, 008DFE50, 006DE3F0, 008DE6A0, 008DC3E0, 00694AF0,
00925780, 009287B0, 00928920, 00695760, 00903F30, 00924710, 004845A0, 006FE570, 00922FD0,
009273A0, 0042BAF0, 00826F10

Ghidra was **read-only** for this packet. Every descriptive name below is a hypothesis, not a
recovered symbol. This closes the four open questions of `docs/ENTITY_EVENT_QUEUES.md`, the three
follow-ups of `docs/OBJECTIVE_UNIT_LIST.md` and the unread predicate of `docs/RECON_SLOT_LISTS.md`.

## 1. The release: what `entity->vtable[0](1)` unregisters

`vtable[0]` is the scalar deleting destructor. For `MDestroyer` it is `00CFC3D0[0h] = 006FE570`,
`__thiscall(entity, int flags)`, `RET 4`, body `006FE570`-`006FE58D`: it calls the class body
`0081F3A0` and then `if (flags & 1) free(this)` at `006FE580`. Everything the release *undoes*
is in the base chain below that body.

**The chain.** `BSP_UnitOwnerEntity_Construct 0077EED0` calls `BSP_GameEntity_Construct 00928630`,
which calls `00925CE0`; the destructors mirror it. Each level rewrites its own vptr set first,
which is how a level is identified:

| level | destructor | vptrs it installs | evidence |
| --- | --- | --- | --- |
| GameEntity | `009287B0` | `+0h`=`00D192E0`, `+10h`=`00D192C8`, `+24h`=`00D192C0`, `+170h`=`00D192BC` | `009287CD`..`009287E1` |
| entity root | `00925780` | `+0h`=`00D19120`, `+10h`=`00D19104`, `+24h`=`00D190FC` | `009257A7`..`009257B3`; the `00925CE0` column of `docs/UNIT_INSTANCE_LAYOUT.md` |
| observer endpoint | `00695760` | `+0h`=`00CECCC8` | the same table's "`00CECCC8` then `00D19120`" |

**The rule table**, in execution order. "site" is the call instruction:

| # | site | in | step | gate |
| --- | --- | --- | --- | --- |
| 1 | `00928810` | `009287B0` | **entity id table release** `009516D0(registry, *(u16*)(entity+174h))`; the registry is `00F89A08` when `id < [00F89A10]`, else `00F89A5C` | always |
| 2 | `00928831` | `009287B0` | the entity **name** `NativeString` at `+178h` (length) / `+17Ch` (pointer) returned to the sized pool (`00419CC0` then `00BD1510`) | `entity[+17Ch] != 0` |
| 3 | `009257CF` | `00925780` | `00740270(entity[+168h])`: `InterlockedDecrement(ref+4h)` and, at zero, `ref->vtable[0]()`. `ECX = [00E1AEA0]` feeds the inner `007400C0` | always |
| 4 | `009257ED` | `00925780` | **destroy every child**: `while (entity[+50h] != 0) entity[+48h]->vtable[0](1)` — the recursive form `009035E0` of `docs/ENTITY_EVENT_QUEUES.md` inlined | `entity[+50h] != 0` |
| 5 | `009257FC` | `00925780` | **unlink from the world update chain**: `00903F30(ECX = *(entity[+30h] + 4h), entity)` over the `+34h`/`+38h` link pair, count at `head[+8h]` | always |
| 6 | `0092580C` / `00925820` | `00925780` | **unlink from the sibling chain** `00924710` over the `+40h`/`+44h` pair: `ECX = parent+48h` when `entity[+3Ch] != 0` (then `entity[+3Ch] = 0` at `00925811`), else `ECX = *(entity[+30h] + 8h)`, the world's root list | always |
| 7 | `00925835` | `00925780` | **unlink from the world list at `world+0Ch`**: `004845A0(ECX = entity[+30h] + 0Ch, entity)` walks `this[+4h]` through `+4h` for the node whose `+8h` is the entity and calls `004837D0` | parentless **and** `entity[+B8h] != 0` |
| 8 | `00925856` / `0092587E` | `00925780` | two more pooled strings: `+160h`/`+15Ch` and `+158h`/`+154h` | each pointer non-null |
| 9 | `00925891` | `00925780` | `00925540` `BSP_WeakOwner_Destroy` on the subobject at `entity+24h` | always |
| 10 | `0092589D` | `00925780` | `00695870` `BSP_CallbackOwner_Destroy` on the subobject at `entity+10h` | always |
| 11 | `009258AC` | `00925780` | **observer edges**: `00695760(entity)` takes the observer lock twice, and when `entity[+8h] != 0` calls `006953C0`, which walks the global edge table at `00E198E4` under the same lock; then `free(entity[+4h])` | `entity[+8h] != 0` for the detach |

Step 7's gate is the counter `009245A0` maintains: it adds to `entity+B4h`/`+B8h` up the parent
chain and calls `BSP_UnitList_PushBack` on the `0` -> non-zero edge and `004845A0` on the
non-zero -> `0` edge, so `+B8h` **is** the membership flag for the `world+0Ch` list.

Step 11 is the single step that undoes the objective-set and marker-manager registrations of
section 4 and the recon-slot pairs of `docs/RECON_SLOT_LISTS.md` (`00805240 BSP_Recon_DestroySlot`
calls the same `00695760`). No consumer has to unregister itself.

**What the release does not do.** The Lua self table is **not** cleared on the unit path.
`00928920` is the only destructor found that nils an entry: it fetches the Lua globals
(`[00E188A8]+1A0Ch`), reads `thisTable` (`00CE7494`) and calls `00B67350(thisTable, &entity[+178h])`
— the entity's own name is the key — before tail-calling `009287B0`. Its **only** caller is the
scalar deleting destructor `00888ED0`, the script-entity branch of `BSP_LuaBinding_CreateScript`
`00898750`. The unit branch (`0077EED0` -> `00928630`) never reaches it, so a unit's
`thisTable[name]` survives the release. `docs/MISSION_ENTITY_LUA_ATTACH.md` establishes that every
mission entity is given one by slot 39; nothing on the unit path takes it back.

Also absent from the chain: any tick-registry removal. The four lists the release touches are the
world update chain (`+34h`/`+38h`), the sibling chain (`+40h`/`+44h`), the `world+0Ch` list and the
observer edges; the fixed-step fan-out of `docs/FIXED_STEP_FANOUT.md` walks the first of those.
The spatial index is `contract: unread` here — no call in the chain was identified as one.

Coverage: `complete` for `009287B0`, `00925780`, `00928920`, `00695760`, `00903F30`, `00924710`,
`004845A0` and `006FE570`. `partial` above `009287B0`: the unit's own levels `0081F3A0` and up were
not read, and Ghidra's stored body for `0081F3A0` (`0081F3A0`-`0081F56B`) is **short** — the real
body continues past `0081F56B` (raw bytes at `0081F56C`: `C7 86 D0 0B 00 00 80 94 D0 00`, a vptr
store), so its listing is not a safe basis for a "nothing else happens" claim.

## 2. `00927050` and the gate bytes

`00927050` is **not a teardown site**. It is `__fastcall(ECX = entity)`, `RET`, body
`00927050`-`0092739D`, and it is the **spawn-descriptor apply**: virtual slot `+9Ch` in 31
vtables, the same slot `docs/MISSION_ENTITY_LUA_ATTACH.md` records for `00928A00` (10 vtables), and
`00928A00` also calls it directly at `00928A1E`. It dispatches on the descriptor kind at
`*(entity[+C0h] + 4h)` (`00927086`):

* **kind 1**, a scene property bag: `Race` -> `entity[+58h]`, `Party` -> `entity[+54h]`, `GuiName`
  -> `vtable[0Ch]`, `SetHierarchy` -> `004F01F0`, `HierarchyParent` -> a name lookup through
  `BSP_EntityRegistry_FindEntityByName`, `HierarchyMatrix` -> `BSP_Matrix_Copy4x4X87`, then
  `vtable[98h](world = [00E188A8]+19CCh, parent)`.
* **kind 3**, a mission Lua table: opens key `_entity` and reads `active` (`00CE4A08`),
  `deadMeat` (`00D190D8`) and `invincible`.

The `+5Dh` write is the `deadMeat` property:

```
009272E3  CALL 0x00bd68d0          ; ReadFieldOrDefault(_entity, "deadMeat", default 0)
009272E8  CMP byte ptr [ESP+0xf],0x0
009272ED  JZ 0x009272fa
009272EF  MOV byte ptr [ESI+0x5d],0x1
009272F3  MOV dword ptr [ESI+0x70],0x1
```

So an entity the mission script declares `deadMeat` is **born** with the torn-down byte already
set, at creation time, and also gets the dword at `+70h`. The neighbouring `active` test is the
same shape: `00927295 CMP byte ptr [ESP+0xe],0` and, when clear, `0092729F CALL 00922F80`
`BSP_SceneNode_Disable`.

**The gate bytes, consolidated.** Writers found by reading the bodies plus an image-wide scan for
immediate byte stores to `+5Ch`..`+60h`, `+6Ch` and `+70h` (`local/gate_bytes.py`; the scan sees
only immediate stores, so register-sourced writes such as `00922FDE` were added by hand):

| byte | set by | cleared by | meaning |
| --- | --- | --- | --- |
| `+5Ch` | `00922F30` `BSP_SceneNode_Enable` | `00922F80` `Disable`; `00922FE4` (kill); `009274DA` (pending flush) | the active test every per-frame pass makes |
| `+5Dh` | `00922FE1` (kill); `009274CE` (pending flush); **`009272EF` (`deadMeat`, at creation)** | nothing found | torn down |
| `+5Eh` | `00922FDE` (kill); `009274D2` | nothing found | destroyed |
| `+5Fh` | `009274D6` | nothing found | removed (`009263C0`'s subject) |
| `+6Ch` | `00922FE8` (kill, dword `1`); incremented by `00903627` | nothing | expiry-pass counter, released at `3` |
| `+70h` | `009272F3` (`deadMeat`) | nothing found | set only alongside `+5Dh`; no reader identified |

`00922FD0` `BSP_SceneNode_Kill` is `+5Eh=1`, `+5Dh=1`, `+5Ch=0`, `+6Ch=1`, then a recursion over
children through `+48h`/`+44h` whose `+6Ch` is still clear, then `JMP vtable[84h]`.
`009273A0` `BSP_EntityEventQueues_FlushPending` is `+5Dh=1`, `+5Eh=1`, `+5Fh=1`, `+5Ch=0` at
`009274CE`-`009274DA`. Those two are the teardown sites; `00927050` is a third **writer** of
`+5Dh` but a creation path, not a teardown.

The `+5Dh`/`+5Eh` pair is what `008DF2B0` tests before accepting a unit into an objective
(`008DF2CD`/`008DF2DC`), so a `deadMeat` entity is refused by the objective add path from the
moment it is created.

## 3. The re-entrancy question: no `vtable[ECh]` handler reaches `00926E80`

`00923A74` is the `vtable[ECh]` dispatch (`MOV EDX,[EAX+0xec]; PUSH EBX; MOV ECX,ESI; CALL EDX`),
the hit handler; it returns a bool and, when false, the drain's callee `009239A0` walks up
`entity+3Ch` and retries.

Scan (`local/vt_ech_scan2.py`, `local/vt_ech_reach.py`): vtable bases taken from the code's own
data references, keeping those with at least `3Ch` consecutive `.text` slots — **564 vtables, 163
distinct functions at slot `+ECh`**. `81` of them have no Ghidra function, so the index's call
graph is blind to them and their bodies were scanned raw for `E8`/`E9` targets. Forward search
from each handler, eight levels, over the Ghidra call edges plus the scanned ones:

**None of the 163 reaches `00926E80`.**

The producers stay on the projectile and explosion side: `00819A20`, `0084BAD0`
`BSP_Explosion_ApplyRadialDamage` and `0084BC60` `BSP_Projectile_OnImpact`, whose own ancestors
(19 functions, depth 8) are the projectile ticks and sweeps. Spot checks of the entity handlers:
`00CFC3D0` and `00D09678` have `+ECh = 00826F10` (the ship hit handler, 22 direct callees, none a
producer), `00D1A698` and `00D0DF70` have `008777D0` `BSP_UnitInstance_ApplyHitRecord` (three
callees: `008E6430`, `00470510`, `004705C0`), and `00D03E80`/`00D19120` have `0042BAF0`, which is
`XOR AL,AL; RET 4`.

So the drain's re-entrancy hazard is real in the listing but **not reachable through the hit
path**. The residual uncertainty is virtual dispatch inside a handler, which no static scan
follows; `00826F10` is the one worth watching because it re-dispatches through `vtable[5Ch]` and
`vtable[24h]`.

## 4. The objective marker tails, and `008DFE50` is a clear

### `006DE3F0` only drops

`__thiscall(MarkerManager* this, const NativeString* name, Entity* unit)`, `RET 8`, body
`006DE3F0`-`006DE4BA`. `006DE000` is an `operator[]`-shaped red-black-tree lookup on the map at
`this+14h` keyed by the unit pointer (`[node+1Dh]` is `_Isnil`, the key at `node+0Ch`); it returns
the per-unit container, whose `_Myhead` is at `+4h`.

```
006DE40A  006de000(this+14h, &unit)          ; the per-unit marker container
006DE411  006dbd40(that, name)               ; -> slot
006DE418  if (*slot) { (*slot)->vtable[0](1); *slot = 0; }     ; 006DE424 / 006DE426
006DE447  for (it = begin; it != end; 006d7d20(&it))           ; the loop head
006DE47A      if (it->node[+14h] != 0) return;                 ; 006DE47E -> the epilogue
006DE49F  if (00694af0(ECX = unit, EDX = this))                ; a pair exists
006DE4AC      006952a0(ECX = unit, EDX = this)                 ; BSP_Observer_UnregisterPair
```

There is **no insert and no rebind**: the only object written is the slot, and it is written with
`0`. The loop is a "nothing left" test, and the routine's second half drops the observer pair
between the unit and the marker manager when the unit has no remaining entry with `+14h` set.
Its name is therefore wrong and is replaced with `BSP_MarkerManager_DropUnitMarker`.

That settles the first follow-up of `docs/OBJECTIVE_UNIT_LIST.md`: `008DFE50`'s loop hands every
live unit to the removal helper **and** its per-unit call only drops a marker, so the routine is a
clear. It is renamed `BSP_Objective_ClearUnitMarkersAndEmpty`.

### `008DE6A0` at `008DF3D7` is `std::set<Entity*>::insert`

`__thiscall(_Tree* this, pair<iterator,bool>* result, const Entity** key)`, `RET 8`, body
`008DE6A0`-`008DE758`. Node layout `_Left +0h`, `_Parent +4h`, `_Right +8h`, key `+0Ch` (4 bytes),
`_Isnil +11h`. Found: `result = {existing, false}` at `008DE74E`; absent: `008DE010` inserts and
`result = {new, true}` at `008DE715`.

At the call site `this = EBX + 18h`, and `EBX` is the `ObjectiveSet` (`008DF2D2` stores `ECX` into
the local at `E-18h`, `008DF330` reloads it), so the add path ends by inserting the unit into a
`std::set<Entity*>` at **`ObjectiveSet+18h`**. The out-parameter is the routine's own 12-byte local
block (`008DF3CF LEA ECX,[ESP+0x14]`), and the key is the address of the `unit` argument
(`008DF3CA LEA EAX,[ESP+0x30]`); the pair is discarded.

### `008DC3E0` at `008DF49E` erases every matching entry

`__thiscall(Objective* this, Entity* unit)`, `RET 4`, body `008DC3E0`-`008DC498`. It walks the
`std::list` at `this+20h` (`_Myhead` at `+24h`, `_Mysize` at `+28h` — the `Objective::units` layout
of `docs/OBJECTIVE_UNIT_LIST.md`) and, for each node whose 16-byte record has `record[+0h] == unit`:

```
008DC439  free(record); [node+8h] = 0        ; the second store is at 008DC441, inside a
                                             ; run the Ghidra listing omits after the free
008DC460  prev->_Next = next; next->_Prev = prev
008DC46B  free(node); _Mysize -= 1           ; the decrement is 008DC473 ADD [EBP+8],-1
008DC477  continue with the saved successor
```

It does **not** stop at the first match, and a null `unit` would match every position record.
`008DC43E`-`008DC447` and `008DC470`-`008DC476` are the two instruction runs `ghidra disasm` drops
after a `_free`-class call; they were read from the PE bytes.

### `009269B0`'s tail from `00926A56` is the catch block

`__thiscall(list* this, iterator where, iterator first, iterator last)`, `RET 1Ch`, body
`009269B0`-`00926AC3`. The insert loop is `009269F0`-`00926A54` and its normal exit is the
`JZ 00926AB1` at `00926A01`; **nothing jumps to `00926A56`**, which is reached only through the
frame handler pushed at `009269B5`. The tail is the strong-guarantee rollback: while the saved
copy of `first` (`[EBP-1Ch]`/`[EBP-18h]`, stored at `009269DD`) differs from the advanced one, it
rebuilds `where`, decrements it (`00923CB0`), erases that node (`00781170`) and advances the saved
copy (`00778C10`); then `00926AAC PUSH 0; PUSH 0; CALL 00BF6885` rethrows.

`0077A7E0` is the checked-iterator `operator!=`: `__thiscall(it* this, const it* other)`, `RET 4`,
body `0077A7E0`-`0077A808` — `if (this->_Mycont == 0 || this->_Mycont != other->_Mycont) 00BF6713;`
then `SETNZ` on `this->_Ptr != other->_Ptr`. It is library code, not entity code, and the branch is
not a normal-flow path at all.

## 5. `00694AF0` and `player+8h`/`+9h`

`00694AF0` is `__fastcall(ECX = subject endpoint, EDX = callback owner) -> bool`, `RET`, body
`00694AF0`-`00694B8C`. It is a locked wrapper: `00694280 BSP_ObserverLockOwner_Get`, enter the
`CRITICAL_SECTION` at `owner+4h` through `[00CE2218]` with the recursion count kept by hand at
`cs+18h`, `006949D0 BSP_Observer_FindPair(ECX, EDX)`, leave, and return whether a pair was found.
So it is `BSP_Observer_IsPairRegistered`, the predicate `006DE49F` and `008DF338` gate on and the
one `docs/RECON_SLOT_LISTS.md` left unread.

The argument order matters and was read from both call sites. At `006DE49F` `ECX` is the unit and
`EDX` the marker manager; at `008DF338` `ECX = EBP` is the **unit** (`008DF2C9 MOV EBP,[ESP+0x24]`,
the second argument, the same pointer whose `+5Dh`/`+5Eh` are tested) and `EDX = EBX` is the
**`ObjectiveSet`**, not the objective. That matches `006949D0`'s recorded `ECX` = first endpoint,
`EDX` = callback owner.

`player+8h` / `+9h`: the test is exact but its producer was not found.

```
008CDF51  MOV EAX,[EDX + EAX*0x4 + 0x18cc]
008CDF58  CMP byte ptr [EAX + 0x8],0x0     ; zero -> reject the explicit slot
008CDF5E  CMP byte ptr [EAX + 0x9],0x0     ; non-zero -> reject the explicit slot
```

so an explicit slot is accepted only when `player[+8h] != 0 && player[+9h] == 0`. The same pair is
read the same way by `BSP_Game_AssignPartyPlayerSlots 004C3840` (`004C38A2`/`004C38A8` and
`004C396F`/`004C3975`), `BSP_Game_LoadMissionScene`, `BSP_LuaObjectives_Completed 008BD340`,
`BSP_LuaObjectives_Failed 008BD900` and `BSP_LuaObjectives_RemoveUnit 008CE510`, and `+9h` alone by
`BSP_Game_CheckMultiplayerPlayerCount`. An image-wide scan for immediate byte stores to `[reg+8h]`
or `[reg+9h]` within `200h` bytes of a `0x18CC` reference (`local/pw.py`) produced one candidate,
`008C8980`, which is a `NativeString` assign on an unrelated object. **The producer is
`contract: unread`**, so the two bytes are recorded as a gate and left unnamed (rule 4).

## Corrections to earlier documents

| document | was | is | evidence |
| --- | --- | --- | --- |
| `docs/ENTITY_EVENT_QUEUES.md` | "`009269B0`'s tail from `00926A56`: an unread branch through `0077A7E0`" | the `catch(...)` rollback of a `std::list` range insert, reached only through the frame handler, ending in a rethrow; `0077A7E0` is the checked `operator!=` | no jump in `009269B0` targets `00926A56`; `00926AAC PUSH 0; PUSH 0; CALL 00BF6885` |
| `docs/ENTITY_EVENT_QUEUES.md` | "`00927050`'s `+5Dh` write at `009272EF`, a third teardown site" | the `deadMeat` mission-Lua property applied at creation; `00927050` is virtual slot `+9Ch`, the spawn-descriptor apply | `009272E3` `ReadFieldOrDefault(..., "deadMeat")` then `009272EF`/`009272F3`; `00927050` at `+9Ch` of 31 vtables, the slot `00928A00` occupies in 10 |
| `docs/ENTITY_EVENT_QUEUES.md` | "does any `vtable[ECh]` handler reach `00926E80`?" | no: 163 distinct `+ECh` handlers over 564 vtables, none reaches it in eight static levels | `local/vt_ech_reach.py`; `0042BAF0` is `XOR AL,AL; RET 4`, `008777D0` has three callees, `00826F10` has 22 and no producer among them |
| `docs/ENTITY_EVENT_QUEUES.md` | "What the release does beyond `vtable[0](1)` ... is `contract: unread`" | section 1's eleven-step table | `009287B0`, `00925780`, `00695760` read complete |
| `docs/OBJECTIVE_UNIT_LIST.md` | `008DFE50` `BSP_Objective_RefreshUnitMarkers`; `006DE3F0` `BSP_MarkerManager_RebindUnitMarker`, insert half `contract: unread` | both are clears: `006DE3F0` destroys the slot's marker and may drop the observer pair, and never inserts | `006DE418`-`006DE426`; `006DE47E` returns on the first surviving entry; `006DE4AC` `006952A0` |
| `docs/OBJECTIVE_UNIT_LIST.md` | host row `008DF338 / 00694AF0 / observer_pair_registered / -/objective,unit/bool` | the arguments are `(ECX = unit, EDX = ObjectiveSet)`; there is no objective pointer in the call | `008DF2C9` `MOV EBP,[ESP+0x24]`; `008DF2D2` stores `this` at `E-18h` and `008DF330` reloads it into `EBX` |

## State per address

| Address | Name | State | Coverage |
| --- | --- | --- | --- |
| `00927050` | `BSP_MissionEntity_ApplySpawnDescriptor` | exported, analyzed, reconstructed, build-tested | partial: the kind-1 branch read as a key list; the kind-3 branch complete |
| `009269B0` | `STL_list_InsertRange_009269b0` (existing) | analyzed | complete |
| `0077A7E0` | `STL_ListIterator_NotEqual_0077a7e0` | analyzed | complete |
| `008DFE50` | `BSP_Objective_ClearUnitMarkersAndEmpty` (replaces `RefreshUnitMarkers`) | analyzed | complete |
| `006DE3F0` | `BSP_MarkerManager_DropUnitMarker` (replaces `RebindUnitMarker`) | analyzed, reconstructed, build-tested | complete |
| `008DE6A0` | `STL_SetPtr_Insert_008de6a0` | analyzed | complete |
| `008DC3E0` | `BSP_Objective_EraseUnitEntries` | analyzed, reconstructed, build-tested | complete |
| `00694AF0` | `BSP_Observer_IsPairRegistered` | analyzed | complete |
| `00925780` | `BSP_Entity_DestructBase` | analyzed, reconstructed, build-tested | complete |
| `009287B0` | `BSP_GameEntity_Destruct` | analyzed, reconstructed, build-tested | complete |
| `00928920` | `BSP_ScriptEntity_Destruct` | analyzed | complete |
| `00695760` | `BSP_ObserverEndpoint_Destruct` | analyzed | complete |
| `00903F30` | `BSP_World_UnlinkFromUpdateChain` | analyzed, reconstructed, build-tested | complete |
| `00924710` | `BSP_Entity_UnlinkFromSiblingChain` | analyzed, reconstructed, build-tested | complete |
| `004845A0` | `BSP_UnitList_Remove` | analyzed | complete |

`0042BAF0` and `00826F10` are named in the report's `no_ghidra_function` list only; Ghidra has no
function at either and this packet did not annotate them.

## Open questions

| question | why it is open |
| --- | --- |
| the unit's own destructor levels above `009287B0` | `0081F3A0`'s Ghidra body ends at `0081F56B` but the real body continues; the levels between `MDestroyer` and `GameEntity` were not read |
| does anything take a unit's `thisTable[name]` back? | only `00928920` nils one and units do not reach it; either the entry leaks or a path outside the destructor clears it |
| the producer of `player+8h` / `player+9h` | six readers agree on the gate; no writer was found |
| the spatial index | no step in the release chain was identified as a spatial-index removal |
| `entity+70h` | written `1` only beside `+5Dh` in `00927050`; no reader found |
| virtual dispatch inside a `vtable[ECh]` handler | the reachability result is static only; `00826F10` re-dispatches through `vtable[5Ch]` and `vtable[24h]` |

## Correction from docs/UNIT_DESTRUCTOR_LEVELS.md (packet cc2_unit_destructor_levels)

- **Was:** open question: the unit's own destructor levels above 009287B0 were not read
  **Is:** four levels read and named - 0077E380 (level 2), 0087A410 (level 3), 00959940 (level 4), 0081F3A0 (level 5) - and level 6 has no destructor body at all: 006FE570 calls the level-5 body
  **Evidence:** each level rewrites exactly the vptr set docs/UNIT_INSTANCE_LAYOUT.md records for its constructor: 0081f3c2..0081f408 = 00D096xx, 0095995f..00959991 = 00D1A6xx, 0087a435..0087a45d = 00D0DFxx, 0077e3a0..0077e3be = 00D03Exx
- **Was:** 0081F3A0's Ghidra body ends at 0081F56B but the real body continues
  **Is:** still true after the integrator's repair; the real body ends at the plain RET at 0081F8AD and the remaining truncation is at the free at 0081F567
  **Evidence:** disasm-raw 0081f56c on the PE bytes: MOV dword ptr [ESI+0xBD0],0xD09480 and on to the RET at 0081f8ad; Ghidra's proto still reports body 0081f3a0-0081f56b
- **Was:** the producer of player+8h / player+9h was not found
  **Is:** player+8h is written 1 at 004BB47D in BSP_Game_ClaimParticipantRecord 004BB440, the claim flag of the 0x118-byte participant record (eight at session+748h); player+9h has no writer anywhere in .text
  **Evidence:** 004BB446 LEA ESI,[ECX+0x748]; 004BB450 CMP byte ptr [ESI+8],BL; 004BB458 ADD ESI,0x118; 004BB45E CMP EAX,8; 004BB47D MOV byte ptr [ESI+8],1. For +9h: the only store in .text is the uncalled out-of-line setter at 004B5684, and a scan of every C6/88 byte store to [reg+9h] plus every disp32 byte store landing on field 9 of an eight-slot 0x118 array based at session+748h or session+1008h found nothing else
- **Was:** open question: no step in the release chain was identified as a spatial-index removal
  **Is:** none exists. The unit owns no node of its own; its grid presence is per part (docs/SPATIAL_INDEX.md's 00710B6D row) and the detach 00710B80 is reached only from 00951F40, which no destructor level calls. The entity+1C4h moving-entity node belongs to the class whose +0h vtable is 00D19500, not to the unit's branch
  **Evidence:** the grep above; 00929E30 is the only vtable slot in the image holding the detach wrapper (00D19584, slot offset 84h of the vtable at 00D19500) and it tests [this+384h] as a byte while level 3 uses +384h as a vector end pointer
- **Was:** entity+70h: written 1 only beside +5Dh in 00927050; no reader found
  **Is:** read at 0077E3EB by the level-2 destructor and forwarded as the +20h payload of a kind-4Fh session message
  **Evidence:** 0077E3EB MOV EAX,[ESI+0x70]; 0077E3EF LEA ECX,[ESP+0x18]; 0077E3F3 CALL 00779780 (which stores the argument at msg+20h at 0077979A); 0077E3FD MOV ECX,ESI; 0077E3FF CALL BSP_Session_RouteMessage 0077C2A0, gated on [00E0AF20], [00E188A8] and [[00E188A8]+1FE4h] == 1
- **Was:** the Lua self table is not cleared on the unit path, established from levels 0 and 1 alone
  **Is:** confirmed for the whole seven-level chain
  **Evidence:** no reference to 00ce7494, 00b67350 or [00E188A8]+1A0Ch in any of the six listings
