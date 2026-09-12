# Objective unit list (packet `objective_unit_list`)

Addresses: 008cdd60, 008ce510, 008dfe50, 006de4c0, 006de3f0, 006de3d0, 006de3e0, 00647c20,
008df2b0, 008df400, 008df5d0, 008df9b0, 008dfc00, 008dee50, 008deeb0

Follow-up of `docs/MISSION_RESULT_DECISION.md`, which established the `Objective` record
(`operator new(2Ch)`, name `NativeString` at `+4h`/`+8h`, `kind` at `+18h`, `state` at `+1Ch`,
unit list at `+20h..+28h`), the per-slot `ObjectiveSet` at `game+21A4h + slot*4`, and
`Objectives_Add` 008cd440. This packet reads what the `+20h` list holds, who pushes and pops
it, and who walks it.

## The list record

`Objective::units` is an MSVC `std::list` whose object starts at `objective+20h`: `_Myhead` at
`+24h`, `_Mysize` at `+28h` (the constructor evidence is in `docs/MISSION_RESULT_DECISION.md`;
the run-time evidence is 008dee50's `MOV EDI,[ESI+0x24]` immediately followed by
`ADD ESI,0x20`, so `+20h` is the list object passed as `this` and `+24h` its head).

Its element is a **pointer to a 16-byte heap record**, not a unit pointer. Both producers
allocate it with `operator new(0x10)` and store the pointer into the node:

| Offset | Field | Producer |
| --- | --- | --- |
| `+0h` | `Entity* unit`, null for a position entry | 008dee67 `MOV dword ptr [EAX],ECX`; 008deec9 `MOV dword ptr [EAX],0x0` |
| `+4h` | `float x` | 008deecf `FSTP float ptr [EAX + 0x4]` |
| `+8h` | `float y` | 008deed5 `FSTP float ptr [EAX + 0x8]` |
| `+Ch` | `float z` | 008deedb `FSTP float ptr [EAX + 0xc]` |

**The two producers initialise disjoint halves.** `008dee50`, the unit push, writes `+0h` and
leaves `+4h..+Fh` uninitialised; `008deeb0`, the position push, writes `0` to `+0h` and the
three floats. The reader never touches the position of a record whose `+0h` is non-null, so
the uninitialised half is unobservable, but a reconstruction must not treat it as a zero
vector. Both then `push_back`: `_Buynode(_Myhead, _Myhead->_Prev, &record)`, `_Mysize++`
(008de200), `_Myhead->_Prev = node`, `node->_Prev->_Next = node` (008dee93..008dee9b).

`std::list` nodes here are `{_Next at +0h, _Prev at +4h, value at +8h}`: every walker reads
`node[2]` for the record and `*node` for the successor.

## `Objectives_AddUnit` 008cdd60 and `Objectives_RemoveUnit` 008ce510

`__fastcall(lua_State*)`, the shared binding prologue of `docs/LUA_BINDING_CORE.md`. The Lua
signature is `Objectives_AddUnit(party, playerSlot, objectiveName, target, ...)`, with the
same five-part shape in both bindings (the `LuaObject` call trace of 008ce510 matches
008cdd60 argument for argument; its only difference is the helper pair it dispatches to).

**Argument 0, the party, builds a slot mask.** Evidence, 008cde8f..008cdef2:

```
008cde8f  MOV ESI,0x18cc
008cdea4  MOV ECX,[00e188a8] / MOV ECX,dword ptr [ESI + ECX*0x1]   ; player k
008cdead  MOV EDI,dword ptr [ECX + 0x28]                           ; the player's party
008cdeba  CALL 0x00b66290 / CMP EDI,EAX / SETZ BL
008cded9  MOV EDX,0x1 / SHL EDX,CL / OR dword ptr [ESP + 0x18],EDX ; mask |= 1 << k
008cdeec  CMP ESI,0x18ec / JL
```

The eight player objects sit at `game+18CCh..+18E8h` and their party id at `player+28h`. The
loop runs only when argument 0 is an integer (008cde73 `IsInteger`); otherwise the mask stays 0.

**Argument 1, a player slot, replaces the mask.** Evidence, 008cdf46..008cdff1: the slot
indexes the same player array; when `player+8h != 0` and `player+9h == 0` the binding rereads
argument 1 and sets `mask = (uint16)(1 << slot)`, discarding the party mask entirely. A slot
that fails the two-byte test leaves the party mask standing.

**Argument 2 is the objective name** (008cdffa `PUSH 0x2`, then `GetString`), and arguments 3
upward are the targets (008ce075 `MOV ESI,0x3`, loop bound `count()`). Each target is either
an entity table (008889C0) or a Vector3 table (0088B840); anything else is expanded by
iterating it as a Lua table and pushing each element (00B67080 / 00B67190 / 00B66420), so a
table of units is accepted in one argument.

**Dispatch.** Evidence, 008ce2ec..008ce455:

```
008ce2ec  MOV EBP,0x21a4
008ce2f1  MOV EAX,0x1 / SHL EAX,CL / TEST ECX,EAX / JZ            ; mask bit for this slot
008ce39f  MOV ECX,[00e188a8] / MOV ECX,dword ptr [ECX + EBP*0x1]  ; the set at game+21A4h+slot*4
008ce3ae  CALL 0x008df9b0                                         ; entity target
008ce42d  CALL 0x008df5d0                                         ; Vector3 target
008ce448  ADD EBP,0x4 / CMP EBP,0x21c4 / JL
```

so bit k of the mask selects the objective set of player slot k and every target is added to
that set. `Objectives_RemoveUnit` is the same loop into 008dfc00 and 008ddd30.

## The four set-level helpers

| Address | ABI | Role |
| --- | --- | --- |
| 008df9b0 | `__thiscall(ObjectiveSet*, const NativeString* name, Entity* unit)` | add an entity |
| 008df5d0 | `__thiscall(ObjectiveSet*, const NativeString* name, Vector3 pos)` | add a position |
| 008dfc00 | `__thiscall(ObjectiveSet*, const NativeString* name, Entity* unit)` | remove an entity |
| 008ddd30 | `__thiscall(ObjectiveSet*, const NativeString* name, Vector3 pos)` | remove a position |

The two entity helpers share one body shape. Each replicates to the remote peer first when
`game+1FE4h == 1` and the owning player has a session at `player+50h` (008df9b0 reads the
set's own slot at `set+14h`), then classifies the entity through its `vtable+5Ch` type query
and expands groups: selector 5 is a plain unit and goes straight through; selector 18h is a
squadron and the helper walks the pointer pair at `entity+398h`/`entity+39Ch`; selector 1Ah,
45h and 46h route to 006D6010 and 00848410. The formation branch walks `entity+3CCh` entries
and **passes null for every member past index 5** (`if (index < 5) member else 0`), which is
one way a position-shaped record with a null unit reaches the list from an entity argument.

## `008df2b0`, the per-unit push

`__thiscall(ObjectiveSet* this, const NativeString* name, Entity* unit)`. This is the only
caller of 008dee50 and the routine that actually reaches the list.

```
if (unit[+5Dh] || unit[+5Eh]) return;                 // 008df2d?, a dead or removed unit
for (node = *[this+28h]; node != [this+28h]; node = *node) {
    if (!NativeString_EqualsInsensitive(node[2]->name, *name)) continue;
    if (!00694af0(...)) BSP_Observer_RegisterPair(...);        // 008df338 / 008df345
    008dee50(node[2], unit);                                   // 008df354 MOV ECX,[ESI+8]
    if (node[2][+18h] == 2 && node[2][+1Ch] != 1) return;       // hidden and not completed
    006de3d0(markers, "SzurkeNyil", unit);                      // 008df39e
    008de6a0(...);                                              // 008df3d7
    return;
}
```

`MOV ECX,dword ptr [ESI + 0x8]` at 008df354 is what fixes the objective as the `this` of
008dee50: `ESI` is the set's list node and `+8h` its value. The position twin 008df5d0 has
the same shape with 008deeb0 and the same `+18h`/`+1Ch` gate.

**The marker call at add time is a stub.** `006de3d0` is one instruction, `RET 0x8`, and
`006de3e0` (the position twin, called from 008df5d0) is the same. The objective's markers are
therefore not bound when a unit is added; only 008dfe50 binds them.

## `008dfe50`, the completed/failed walk

The ledger records this as `__stdcall(Objective*)`. The listing contradicts the `__stdcall`
half: ECX is saved in the prologue at 008dfe74 (`MOV dword ptr [ESP+0x14],ECX`) and read back
at 008e002d as the `this` of the removal call, so the ABI is
`__thiscall(ObjectiveSet* this, Objective* objective)`, `RET 4` at 008e005a. The `Objective*`
stack argument and the `objective+18h == 2` early return are unchanged.

```
if (objective->kind == 2) return;                     // 008dfe6e, hidden objectives
std::vector<Entity*> live;                            // zeroed at 008dfe80..008dfe88
for (node = *[objective+24h]; node != [objective+24h]; node = *node) {
    rec = node[2];
    if (rec->unit != 0) {                             // 008dfeb9 CMP dword ptr [EAX],EBX
        00647c20(&live, &rec->unit);                  // 008dfedd, vector push_back
        006de3f0(markers, "SzurkeNyil", rec->unit);   // 008dff3b
    } else {
        006de4c0(markers, "SzurkeNyil", rec->x..z);   // 008dffd2, three FLD/FSTP from rec+4h
    }
}
for (u in live) 008dfc00(this, &objective->name, u);  // 008e0035
free(live);                                           // 008e0040
```

The marker manager is `game+21D4h` (008dff35, 008dffcc). `00647c20` is a `push_back` on a
4-byte element vector whose `_Myfirst`/`_Mylast`/`_Myend` are at `+4h`/`+8h`/`+Ch` of the
object it is called on; the growth path is 00647A10. The second argument of 008dfc00 is
`objective+4h`, the objective's own name (008e0028 `MOV EDX,[ESP+0x48]; ADD EDX,0x4`).

So the walk **empties the objective**: every live unit is handed back to the set-level
removal, and the position entries only have their marker call made. Its two callers are
`BSP_ObjectiveSet_SetCompleted` 008e20d0 and `BSP_ObjectiveSet_SetFailed` 008e2200 and
nothing else. `006de4c0` (`RET 10h`) creates a lookup key from (name, position) through
006DE150, resolves it with 006DBD40 and destroys the object in the resulting slot; `006de3f0`
does the same for a unit through 006DE000 and then continues past 006de42c with work this
packet did not read, so its insert half is `contract: unread`.

## How completion consults the list

It does not. `Objectives_Completed` 008bd340 and `Objectives_Failed` 008bd900 reach
008e20d0 / 008e2200, which `docs/MISSION_RESULT_DECISION.md` establishes as walking the
**objective** list of the set, comparing the key against `objective+4h`/`+8h` and writing
`objective+1Ch` - the only field either writes. The `+20h` unit list is read exactly once in
the whole binary, by 008dfe50, after that state write. The unit list is the HUD marker set and
the units the objective still holds; it is never a completion test. Nothing walks it to decide
whether an objective is done.

## Host table

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| 008cdea4 | `[00e188a8]+18CCh+k*4` | `player_party` | -/int slot/int | argument 0 is an integer |
| 008cdf51 | `[00e188a8]+18CCh+s*4` | `player_slot_active` | -/int slot/bool | argument 1 is an integer |
| 008ce39f | `[00e188a8]+21A4h+k*4` | `objective_set` | -/int slot/ptr | mask bit k set |
| 008df338 | 00694af0 | `observer_pair_registered` | -/objective,unit/bool | name matched |
| 008df345 | 00694a60 | `observer_register_pair` | -/objective,unit/void | not already registered |
| 008df39e | 006de3d0 | `marker_bind_unit_on_add` | markers/name,unit/void | empty body, `RET 8` |
| 008dff3b | 006de3f0 | `marker_rebind_unit` | markers/name,unit/void | `rec->unit != 0` |
| 008dffd2 | 006de4c0 | `marker_clear_position` | markers/name,pos/void | `rec->unit == 0` |
| 008e0035 | 008dfc00 | `set_remove_unit` | set/name,unit/void | per collected live unit |
| 008dfedd | 00647c20 | `std::vector::push_back` | vector/&unit/void | `rec->unit != 0` |
| 008df9b0 `vtable+5Ch` | virtual | `entity_kind_is` | entity/int selector/bool | five selectors |

`006de150`, `006de000`, `006dbd40`, `006d6010`, `00848410`, `008de6a0`, `008dc3e0` and the
`vtable+5Ch` implementations were not read; they are contracts, named by address (rule 1).

## A naming collision to be aware of

`include/bsp/local_player_unit_lists.hpp` declares `kObjectiveSetTreeOffset = 0x18`,
`kObjectiveSetHeadOffset = 0x1c` and `kObjectiveSetSizeOffset = 0x20`, cited to 008DDF09 and
008DDF90. Those addresses belong to `BSP_SzurkeNyil_ContainsUnit` and an STL instantiation,
not to the objective set at `game+21A4h + slot*4`: 008ddf93 reads `[EDI+0x20]` as a size on a
`SzurkeNyil` marker object. The sets read here put their `std::list<Objective*>` object at
`+24h` with `_Myhead` at `+28h` (008df2e5 `MOV EAX,[ECX+0x28]` next to 008df2ec
`LEA EDI,[ECX+0x24]`, repeated at 008df418/008df425 and in 008df5d0). The three constants are
therefore a different record's layout under a misleading name; this packet does not touch that
header and declares its own constants.

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 008cdd60 | `BSP_LuaObjectives_AddUnit` | exported, analyzed, reconstructed, build-tested |
| 008ce510 | `BSP_LuaObjectives_RemoveUnit` | exported, analyzed, reconstructed, build-tested |
| 008dfe50 | `BSP_Objective_RefreshUnitMarkers` (existing) | exported, analyzed, reconstructed, build-tested |
| 008dee50 | `BSP_Objective_PushUnitEntry` | exported, analyzed, reconstructed, build-tested |
| 008deeb0 | `BSP_Objective_PushPositionEntry` | exported, analyzed, reconstructed, build-tested |
| 008df2b0 | `BSP_ObjectiveSet_AddUnitToNamedObjective` | exported, analyzed, reconstructed, build-tested |
| 008df5d0 | `BSP_ObjectiveSet_AddPositionToNamedObjective` | exported, analyzed |
| 008df400 | `BSP_ObjectiveSet_RemoveUnitFromNamedObjective` | exported, analyzed |
| 008df9b0 | `BSP_ObjectiveSet_AddUnitExpandingGroups` | exported, analyzed |
| 008dfc00 | `BSP_ObjectiveSet_RemoveUnitExpandingGroups` | exported, analyzed |
| 006de4c0 | `BSP_MarkerManager_ClearPositionMarker` | exported, analyzed |
| 006de3f0 | `BSP_MarkerManager_RebindUnitMarker` | exported, analyzed |
| 006de3d0 | `BSP_MarkerManager_BindUnitMarkerStub` | exported, analyzed |
| 00647c20 | `STL_PointerVector_PushBack_00647c20` | exported, analyzed |

Coverage: `complete` for the 16-byte record, its two producers, the list splice, the
argument contract of both bindings and the 008dfe50 walk. `partial: 008df9b0 008dfc00` - the
1Ah/45h/46h entity branches were read as dispatch only. `contract: unread` for 006de3f0's
insert half (008de42c..), 006de150, 006de000 and 006dbd40.

## Follow-ups

- `008dfe50` is named `RefreshUnitMarkers` in the ledger, but its loop hands every live unit
  to the removal helper and its only callers are the completed and failed transitions. Whether
  006de3f0's unread tail rebinds a marker or only drops it decides whether the name should
  become a clear. The name is left alone here and the corrected ABI appended as evidence.
- `008de6a0` at 008df3d7, the second call of the add path, and `008dc3e0` at 008df49e, the
  list erase of the remove path.
- `player+8h` and `player+9h`, the two bytes `Objectives_AddUnit` tests before accepting an
  explicit slot.
