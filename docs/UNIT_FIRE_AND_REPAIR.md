# Ship fire, flooding and damage control

Addresses: 0093CA20 0093C770 0093C860 0093C520 0093C120 0093C210 0093BED0 0093C300 0093BD80
008160B0 00827450 0074E8F0 0074E9C0 0080E130 0081AD40 00827A40 00827960 00812A70 00939FE0
008ACD10 008ACF00 0088E320 0088E790 00891680 008918D0 008AD150 008AD330 008AD540 008AD6F0
008AD8D0 008ADA70 008ADC40 008C6DD0

Packet `cc2_fire_flooding`, read-only in Ghidra. Every descriptive name below is a hypothesis, not
a recovered symbol. This packet continues docs/UNIT_HIT_PATH.md, which established the tick driver
`0093CA20` and the first two of its five steps; the three steps that doc left `contract: unread`
are read here, the task record is located inside the unit instance, and the Lua bindings are
resolved. Health fields come from docs/UNIT_DAMAGE_AND_DEATH.md and are not restated.

## The repair task lives inside the unit instance at `+A20h`

`0093CA20` has no Ghidra function at its call site. The bytes at `008160B0..008160EA` were read
with `bsp.py ghidra bytes` and decoded with capstone (`local/decode_repair_site.py`); the enclosing
Ghidra candidate is `00815F30`, whose own body ends at `008160AC`, so the call site is in the gap
and is recorded `no_ghidra_function` in reports/unit_fire_flooding.json.

```
008160b0: mov  eax, [ecx + 0x538]        ; class/descriptor record
008160b6: cmp  byte ptr [eax + 0xd0], 0  ; class allows damage control
008160bd: je   0x8160ea
008160bf: cmp  byte ptr [ecx + 0x5c], 0  ; kEntityFlagActive must be set
008160c5: cmp  byte ptr [ecx + 0x5d], 0  ; kEntityFlagReleased must be clear
008160cb: cmp  byte ptr [ecx + 0x60], 0  ; kEntityFlagUnreadGate must be clear
008160d1: cmp  byte ptr [ecx + 0x5e], 0  ; kEntityFlagReleaseRequested must be clear
008160d7: fld  dword ptr [esp + 4]       ; dt
008160db: push ecx
008160dc: add  ecx, 0xa20                ; <-- the repair task
008160e2: fstp dword ptr [esp]
008160e5: call 0x93ca20
008160ea: ret  4
```

The `+A20h` offset is confirmed independently by `FailureRepairEnable`, which does
`LEA ECX,[ESI + 0xA20]` at `008AD699` before calling the one-line setter `00939FE0`, and by
`SetRepairEffectivity`, which writes `[ESI + 0xA48h]` at `008ADA11` -- the same slot the hull step
reads as the task's `+28h` (`A20h + 28h = A48h`). The four flag bytes already have constants in
`include/bsp/lua_binding_entity_lookup.hpp` and are not redeclared.

The task has no vtable of its own: `0093CA20` loads `*task` and calls **the unit's** `vtable[14h]`
through it, so `+00h` is a back-pointer to the owning unit.

## State fields

Repair task, embedded at `unit+A20h`. The constructor was not found, so every meaning is a
consumer's or a writer's use; the record is `contract: producer unread`.

| unit | task | type | meaning | writer | reader |
| --- | --- | --- | --- | --- | --- |
| `+A20h` | `+00h` | ptr | owning unit | ctor unread | `0093C770`, `0093CA20` |
| `+A38h` | `+18h` | ptr | active-failure vector, begin | `0093BC30` | `0093C520` |
| `+A3Ch` | `+1Ch` | ptr | active-failure vector, end | `0093BC30` | `0093C520` |
| `+A40h` | `+20h` | ptr | active-failure vector, capacity | `0093BC30` | `0093B690` (the grow helper) |
| `+A44h` | `+24h` | int | repair priority, `0..4` | message `A0h` handler, unread | all five steps |
| `+A48h` | `+28h` | float | repair effectivity; the hull step's coefficient | `008ADA11` | `0093C770` |
| `+A4Ch` | `+2Ch` | float | fire damage per second | message `9Eh` selector `0`, handler unread | `0093C120` |
| `+A50h` | `+30h` | float | water damage per second | message `9Eh` selector `1`, handler unread | `0093C210` |
| `+A54h` | `+34h` | float | fire seconds remaining | `0093C120` | `0093CA20` |
| `+A58h` | `+38h` | float | water seconds remaining | `0093C210` | `0093CA20` |
| `+A5Ch` | `+3Ch` | u32 | cleared when the water timer expires | `0093C210` | unread |
| `+A60h` | `+40h` | u32 | cleared when the fire timer expires | `0093C120` | unread |
| `+A64h` | `+44h` | byte | "already reported repaired" | `0093CADC`, cleared in `0093BED0` | `0093CA20` |
| `+A65h` | `+45h` | byte | hull repair enabled | **no proven writer** (RepairEnable's direct arm writes `unit+378h`, the part-construction byte 0087BD19 sets and the subobject step 0093C860 tests per child, not this field; the `9Fh` handler is unread) | `0093C770` |
| `+A66h` | `+46h` | byte | failure repair enabled | `00939FE0` | `0093C520` |

Which timer is fire and which is water is **provisional**. The ordering evidence is: the completion
literal reads `Fire && Leak`; `0093CA20` tests `+34h` before `+38h`; `SetFireDamage` puts `0` in
the message's selector slot and `SetWaterDamage` puts `1`. The message `9Eh` handler that would
settle it is unread, so the assignment `+34h`/`+2Ch` = fire is a hypothesis.

Active-failure record, 16 bytes, stride proven by the `>> 4` element arithmetic in `0093C520` and `0093B690`. The producer
is `0093BC30`, called from all three sites below, so rule 4 of docs/WORKER_VERIFICATION_CHECKLIST.md
is satisfied for this record.

| offset | type | meaning | producer evidence |
| --- | --- | --- | --- |
| `+00h` | int | failure id or owning component; `5` for `EngineJam`, `-2` for a random pick, else the hit record's `+30h` | `0093BD80` `local_44 = 5`, `0093C300` `local_1c = 0xFFFFFFFE`, `0093BED0` `local_48 = [hit+30h]` |
| `+04h` | size | name length | `BSP_NativeString_Resize` |
| `+08h` | ptr | name characters (`"EngineJam"`, or the descriptor's name) | the `_memcpy` in `0093BD80` |
| `+0Ch` | float | seconds remaining | `local_38` / `local_3c` / `local_10` |

Unit fields outside the task:

| offset | type | meaning | evidence |
| --- | --- | --- | --- |
| `+344h` | vector | part-descriptor pointers; begin `+348h`, end `+34Ch`, 4-byte elements | `008AD025` `ADD ESI,0x344`, then `[ESI+4]`/`[ESI+8]`; count helper `0043EC50` |
| `+1018h` | ptr | parts object; its `+84h` float is the displacement divisor | `0080E490`, `0092BEB0` |
| `+10D4h` | ptr | leak manager; the peer packet owns the force model | `00891788` and `008919D8` `LEA ECX,[ESI+10D4h]` |
| `+114Ch` | float | repair time delay, clamped to `[0, 1e10]` | `008ADBB6..008ADBDD` |
| `+1134h + i*4` | int[6] | damage-control crew level per category | `0080E130`, `00827960`, `00812A70` |

Leak manager, read only as far as `GetLeaks` and `GetWaterLoad` need it:

| offset | type | meaning | evidence |
| --- | --- | --- | --- |
| `+14h` | int | element count | `0074E8F0`, `0074E9C0` |
| `+18h` | float* | per-element leak amount | `0074E9C0` |
| `+1Ch` | float* | per-element water amount | `0074E8F0` |

## Game-settings constants

All through `00424C40()`, the settings singleton. The default values are `contract: unread`; only
the offsets and their roles are established here.

| offset | used by | role |
| --- | --- | --- |
| `+3B4h` | `0093C770` | hull repair scale |
| `+3B8h` | `0093C860` | subobject repair scale |
| `+3C8h` | `0093C120` | fire damage divisor when the priority is `4` |
| `+3CCh` | `0093C210` | water damage divisor when the priority is `3` |
| `+3D0h` | `0093C520` | failure repair rate when the priority is `1` |
| `+3D4h` | `0093C770` | hull repair rate when the priority is `0` |
| `+3D8h` | `0093C860` | subobject repair rate when the priority is `2` |
| `+3DCh`, `+3E0h` | `0093BF5F`, `0093BF66` | fallback failure-chance numerator and denominator |
| `+3E8h`, `+3ECh` | `0093C320`, `0093C36A`, `0093C3B2` | failure descriptor vector, stride `14h` |
| `+46Ch` | `00827960` | maximum crew level per category |

Failure descriptor, stride `14h`: `+04h` name length, `+08h` name characters, `+0Ch` float
duration, `+10h` byte enabled. Read in `0093C300` (the enabled test) and `0093BED0`.

Literal constants: `1.0f` at `00D7A24C`; `0.0` (double) at `00D7A258`; `1e10` at `00CE4970`;
`0.0f` at `00D7A218`; `6.0` (double) at `00CE6628`; `10000.0` (double) at `00CE4BD8`; `1.5`
(double) at `00CE3D78`.

## The per-tick rules

`dt` is the driver's float argument. `modifier(unit)` is
`(00F88C30 && 00E0C978 && [00F88C30+ACh]) ? BSP_GameplayModifiers_ProductForUnit(3, unit) : 1.0f`;
the whole factor is skipped when `00F88C30` is null. The same three-line preamble opens all five
steps, at `0093C553`, `0093C12C` and `0093C21C` (the settings fetch) with the modifier product at
`0093C592`, `0093C16A` and `0093C25A`.

**1. Hull repair, `0093C770`** (`__thiscall(task, float dt)`, `RET 4`, `0093C770..0093C856`). As
docs/UNIT_HIT_PATH.md established, restated with the `+28h` writer now known:

```
base   = task.hull_repair_enabled ? (task.priority == 0 ? settings[3D4h] : 1.0f) : 0.0f
rate   = base * modifier(unit)
heal   = task.effectivity * dt * settings[3B4h] * unit.max_health * rate
00879810(unit, heal)                       ; ApplyHealthDelta, positive heals
if (unit.health > unit.max_health) 00877B90(unit, unit.max_health)
```

The effectivity at `+A48h` scales **only** the hull step; no other step reads it.

**2. Subobject repair, `0093C860`.** Unchanged from docs/UNIT_HIT_PATH.md.

**3. Failure repair, `0093C520`** (`__thiscall(task, float dt)`, `0093C520..0093C761`, SEH frame):

```
base = task.failure_repair_enabled ? (task.priority == 1 ? settings[3D0h] : 1.0f) : 0.0f
rate = base * modifier(unit)
i = 0
while (i < failure_count) {
    f = failures[i]
    f.remaining -= rate * dt
    if (f.remaining > 0) { i += 1; continue; }
    unit->vtable[19Ch](...)                ; named-state dispatch, contract: unread
    BSP_SessionMessage_ConstructBase(); BSP_Session_RouteMessage()
    failures[i] = failures[count-1]        ; swap-erase, name string moved by
    0093BA80(count - 1)                    ; BSP_NativeString_Resize + memcpy
}                                          ; i is NOT advanced: the swapped-in entry is retried
```

The swap-erase leaves `i` unchanged, so the entry moved into the hole is processed on the same
pass. `0093BA80` is the vector's shrink helper.

**4. Fire damage, `0093C120`** (`__thiscall(task, float dt)`, `0093C120..0093C20B`):

```
div = (task.priority == 4 ? settings[3C8h] : 1.0f) * modifier(unit)
before = task.fire_seconds
task.fire_seconds = before - dt
if (task.fire_seconds < 0) { task.fire_seconds = 0; task.u32_40h = 0; }
elapsed = before - task.fire_seconds
if (elapsed > 0) unit->vtable[1ACh](elapsed * task.fire_damage_per_second / div)
```

`vtable[1ACh]` is `kEntityVtableSlotAddDamage` (`include/bsp/unit_damage.hpp`; unit override
`0095DA00`). The priority **divides** the damage, so priority `4` reduces fire damage rather than
shortening the fire; the timer always runs down at real time.

**5. Water damage, `0093C210`** (`0093C210..0093C2FB`): the same body with `+38h`, `+30h`,
`settings[3CCh]`, priority `3`, clearing `+3Ch`.

**Completion, `0093CA20`.** After the five steps, when `task.priority` is `3` or `4`, both timers
are exactly zero and `+44h` is clear, it formats `"Ship Repaired Fire && Leak %s Hp %6f %f"`
through `004254B0` with the unit's `vtable[14h]` name and two floats (`unit+370h`, the current
health, and the result of `BSP_UnitInstance_GetHealth`), sets `+44h` and calls `00914100`. The
gate on priority `3`/`4` means the message only ever appears while damage control is set to fire
or flooding.

## How a failure starts

The producer call site `00827450` also has no Ghidra function; the enclosing candidate is
`00826D70`. Decoded from the raw bytes:

```
0082742b: mov    [esp + 0xd0], ebx
00827432: movss  xmm0, [esp + 0x14]       ; the damage this hit applied
00827438: comiss xmm0, [0xd7a218]         ; > 0.0f
0082743f: jbe    0x827455
00827441: fld    [esp + 0x14]
00827445: push   ecx
00827446: fstp   [esp]                    ; arg 2: damage
00827449: push   esi                      ; arg 1: the hit record
0082744a: lea    ecx, [edi + 0xa20]       ; the repair task
00827450: call   0x93bed0
```

`ESI` is the hit record of docs/UNIT_HIT_PATH.md: `0093BED0` reads its `+34h` (the hull segment
index, `-1` for no hull hit) and `+30h`. The two pushes and the callee's `RET 8` at `0093C118`
agree on two stack arguments.

`0093BED0` `BSP_ShipSystems_RollComponentFailure`:

```
gate: game(00E188A8)+1FE4h in {0,1}   and   hit+34h != -1   and   damage > 0
comp = 008782A0(hit+30h, hit+34h)                     ; null -> no roll
n = comp->desc[+28h];  d = comp->desc[+2Ch]
p = (n < 0 || d < 0) ? (settings[3DCh] * damage) / settings[3E0h]
                     : (n * damage) / d
if (00BD2F10(0, 1.0f) <= p) {
    row = 0093AA00(hit+30h)                            ; the failure descriptor, contract: unread
    if (row && !already_active) {                      ; 0093A5D0 over 0093A940/0093A910
        record = { hit+30h, row+4h (name), row+0Ch (duration) }
        BSP_Session_RouteMessage(0093A2C0(hit+30h, hit+34h, 1), 7, 0)
        0093BC30(&record)                              ; push onto the task's failure vector
        BSP_WarningManager_FireFailure(unit, name, duration)
        00913D80(unit, hit, owner)
        unit->vtable[21Ch](name, hit)
        task+44h = 0                                   ; allow a new "repaired" report
    }
}
```

Two other producers push the same record:

* `0093C300` `BSP_ShipSystems_PickRandomFailure` picks a uniformly random **enabled** row of the
  settings vector at `+3E8h` using `BSP_RandomThreads_NextU32`, and pushes it only when
  `0093A8B0(name)` returns the vector's end, i.e. that failure is not already active.
* `0093BD80` is a Lua binding that pushes `{ id 5, "EngineJam", arg2 or 1e10 }`. The duration
  defaults to `1e10` (`00CE4970`) when argument 2 is absent or not a number, which is the
  "until repaired" value.

**No in-engine producer of the fire or water timers was found in this packet.** The only writers
this packet proves are the two decrement steps; the Lua path writes them through session message
`9Eh`, whose handler is `contract: unread`. Whether a hit can ignite a fire without script is an
open question.

## Damage-control crew levels

`unit+1134h` is an `int[6]`. Index `0` is the unallocated pool; the five named categories draw
from it and give back to it, so the total is conserved.

`0081AD40(&name)` maps a name to the index, case-insensitively (`__stricmp` for `"all"`, then
`BSP_NativeString_EqualsCStringInsensitive`):

| index | names | evidence |
| --- | --- | --- |
| `0` | `"all"`, and any unrecognised name | `0081AD40..0081AF74`, the `__stricmp` branch and the default return |
| `1` | `"hull"` (`00D09600`) | `0081AD40` body, `00D09600` |
| `2` | `"firefighting"` | `0081AD40` body |
| `3` | `"engine and steering"`, `"engine"`, `"steering"` | `0081AD40` body, returns 3 |
| `4` | `"weapons"` (`00D095D4`) | `0081AD40` body, returns 4 |
| `5` | `"spec"` (`00D095CC`), `"runway"`, `"airsupply"` | `0081AD40` body, returns 5 |

This is **not** the same enum as the repair priority at `+A44h`, which the five tick steps read as
`0..4`; nothing in this packet connects a priority value to a category name, and the message `A0h`
handler is unread.

`00827960` raise one level, `__thiscall(unit, int index)`:

```
level[index] += 1
if (level[index] > settings[46Ch]) { level[index] = (int)settings[46Ch]; return; }
if (index == 0 || level[0] < 1) {
    for (j = 5; j > 0; --j) if (j != index && level[j] > 0) { level[j] -= 1; break; }
} else {
    level[0] -= 1
}
```

`00812A70` lower one level, `__thiscall(unit, int index)`; index `0` is ignored:

```
if (index == 0) return
level[index] -= 1
if (level[index] < 0) { level[index] = 0; return; }     ; nothing is returned to the pool
level[0] += 1
BSP_Session_RouteMessage(0080FEC0(unit), 0, 0)
```

`00827A40(unit, index, target)` steps with `00827960` while below the target and with `00812A70`
while above it. `0080E130(unit, index)` returns `level[index]`.

## The Lua bindings

All share the prologue of docs/LUA_BINDING_CORE.md; argument indices are the `LuaObject` argument
slots, slot `0` being the object table. Ret is the `BSP_LuaObject_ResultCount` value.

| address | name | arguments | effect | returns |
| --- | --- | --- | --- | --- |
| `008ACD10` | `AddFire` | unit, int, int, number | **none**: all four values are parsed and discarded (`FSTP ST0` at `008ACEA3`, the two `GetInteger` results unused) | 0 |
| `008ACF00` | `GetFire` | unit, int, int | scans `unit+344h` for a part descriptor with `+4h == arg1` and `+8h == arg2`; on a match pushes the constant `0.0f` (`FLDZ` at `008AD0E7`) | 1 on a match, 0 otherwise |
| `0088E320` | `SetFireDamage` | unit, number, [bool] | routes session message `9Eh` with the float at `+1Ch`, `!bool` at `+20h` and selector `0` at `+24h` | 0 |
| `0088E790` | `SetWaterDamage` | unit, number, [bool] | the identical message with selector `1` | 0 |
| `00891680` | `GetWaterLoad` | unit | `0074E8F0(unit+10D4h, ...)` then normalises | 7 numbers |
| `008918D0` | `GetLeaks` | unit | `0074E9C0(unit+10D4h, ...)` then normalises | 7 numbers |
| `008AD150` | `SetRepairLevel` | unit, string, int | `00827A40(unit, 0081AD40(string), int)` | 0 |
| `008AD330` | `RepairEnable` | unit, bool | owned by docs/LUA_BINDING_NAVIGATOR.md (packet cc_lua_navigator); two exclusive arms selected by the class test `vtable[5Ch](6)` at 008AD448..008AD45C (BL, the only write in the body): class 6 routes session message `9Fh` (vptr `00D03360`, bool at `+1Ch`), any other class writes the Lua boolean straight into `unit+378h` at 008AD4E8, so `RepairEnable(false)` on a non-class-6 entity removes it from subobject repair and never touches `task+45h` | 0 |
| `008AD540` | `FailureRepairEnable` | unit, bool | `00939FE0(unit+A20h, bool)`, i.e. `task+46h` | 0 |
| `008AD6F0` | `SetRepairPriority` | unit, int | routes session message `A0h`, vptr `00CF5C38`, int at `+1Ch` | 0 |
| `008AD8D0` | `SetRepairEffectivity` | unit, number | `unit+A48h = number` (`008ADA11`), the hull coefficient | 0 |
| `008ADA70` | `RepairAddTimeDelay` | unit, number | `unit+114Ch = clamp(unit+114Ch + number, 0, 1e10)` | 0 |
| `008ADC40` | `GetRepairLevel` | unit, string | `0080E130(unit, 0081AD40(string))` | 1 |
| `008C6DD0` | `CheatMaxRepair` | unit | `vtable[1B8h]` on the unit, or on each member of a group | 0 |

`AddFire` and `GetFire` are vestigial: neither reaches a unit method, and `GetFire` returns a
literal zero for any part that exists. A per-part fire intensity does not exist in this build.

`CheatMaxRepair` fans out by class (`vtable[5Ch]` is `unit_is_kind_of`, docs/UNIT_HIT_PATH.md):

| test | action |
| --- | --- |
| `is_kind_of(4)` | `unit->vtable[1B8h]()` |
| else `is_kind_of(18h)` | for `i` in `[0, unit+3CCh)`: `member = (i <= 4) ? [unit+3D0h + i*4] : null`, `member->vtable[1B8h]()` |
| else `is_kind_of(1Ah)` | for each pointer in the vector at `unit+394h` (begin `+398h`, end `+39Ch`): `->vtable[1B8h]()` |

The body of `vtable[1B8h]` is `contract: unread`.

### The water-load and leak readouts

`0074E8F0` and `0074E9C0` are the same routine over different arrays (`+1Ch` water, `+18h`
leaks). Both take six `float*` out parameters:

```
n = mgr[+14h];  bins[0..5] = 0;  pos = 0.0f
for (k = 0; k < n; ++k) { bins[(int)pos] += src[k]; pos += 6.0 / (float)n; }
*p6 = bins[0]; *p4 = bins[1]; *p2 = bins[2]; *p7 = bins[3]; *p5 = bins[4]; *p3 = bins[5];
```

The six elements are binned into six longitudinal zones by an index that advances `6/n` per
element, so each bin receives one sixth of the list. `(int)pos` reaching `6` on the last element is
not guarded; with `n` elements the final `pos` is exactly `6.0`, one past the array, and the loop
exits before using it only because the increment follows the store.

The binding then normalises (`00891793` onwards, and the identical sequence from `008919E7` in `GetLeaks`):

```
total = b0 + b1 + b2 + b3 + b4 + b5                       ; six FADDs, seeded with 0.0 at 00D7A258
push min(1.0f, total * 10000.0 * 1.5 / parts[+84h])
for (i = 0; i < 6; ++i) push min(1.0f, slot[i] * 1.5 * 10000.0 / parts[+84h])
```

`parts[+84h]` comes from `0092BEB0([unit+1018h])` and divides every value, so the readout is a
fraction of the hull's displacement, clamped to `1.0`.

The six per-zone results are pushed in **stack-slot** order, not bin order. The six pointers are
pushed at `0089176E..00891787` and the callee cleans them (`__thiscall`, and the caller executes
no `ADD ESP` before the `FLD` at `00891793`), so the slots at `S+10h..S+24h` hold, in ascending
order, `b2, b4, b0, b3, b1, b5`. That is the order of Lua results 2 through 7.

## Coverage

| routine | coverage |
| --- | --- |
| `0093CA20` | complete |
| `0093C520` | complete |
| `0093C120`, `0093C210` | complete |
| `0093C770`, `0093C860` | inherited from docs/UNIT_HIT_PATH.md; not re-read |
| `0093BED0` | partial: the gate, the probability and the record build are read; `0093AA00`, `0093A2C0`, `0093A5D0` and `00913D80` are `contract: unread` |
| `0093C300` | partial: the selection and the record build; `0093A8B0` is `contract: unread` |
| `0093BD80` | complete |
| `0074E8F0`, `0074E9C0` | complete |
| `0081AD40`, `00827A40`, `00827960`, `00812A70`, `0080E130`, `00939FE0` | complete |
| `008ACD10`, `008ACF00`, `008AD8D0`, `008ADA70`, `008ADC40`, `008AD150` | complete |
| `0088E320`, `0088E790`, `008AD6F0` | complete up to the routed message; the `9Eh`, `9Fh` and `A0h` handlers are `contract: unread` |
| `008AD330` | cited from packet cc_lua_navigator (docs/LUA_BINDING_NAVIGATOR.md), not reconstructed here; its two-arm structure is recorded in the binding table above |
| `00891680`, `008918D0` | complete |
| `008C6DD0` | partial: the fan-out is read, `vtable[1B8h]` is `contract: unread` |
| `008160B0..008160EA`, `00827400..00827455` | raw listing, `no_ghidra_function` |

## Host table

One row per native call site the reconstruction models as a virtual method of
`UnitFireFloodingHost` in `include/bsp/unit_fire_flooding.hpp`. Argument detail is in
reports/unit_fire_flooding.json.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `0093CA2B` | `0093C770` | `repair_hull` | task / dt / void | none |
| `0093CA3A` | `0093C860` | `repair_subobjects` | task / dt / void | none |
| `0093CA49` | `0093C520` | `repair_failures` | task / dt / void | none |
| `0093CA58` | `0093C120` | `apply_fire_damage` | task / dt / void | none |
| `0093CA67` | `0093C210` | `apply_water_damage` | task / dt / void | none |
| `0093CAAF` | `00923BE0` | `unit_get_health` | unit / none / float | completion |
| `0093CAC9` | `[vt+14h]` | `unit_display_name` | unit / none / const char* | completion |
| `0093CAD1` | `004254B0` | `log_line` | none / format, name, two doubles / void; `ADD ESP,0x18` at `0093CAD8` | completion |
| `0093C553`, `0093C12C`, `0093C21C` | `00424C40` | `game_settings` | none / none / settings | none |
| `0093C592`, `0093C16A`, `0093C25A` | `008E6430` | `gameplay_modifier_product` | none / kind `3`, unit / float | `00F88C30` non-null and `00E0C978` and `[00F88C30+ACh]` |
| `0093C62E` | `[vt+19Ch]` | `unit_named_state_dispatch` | unit / out, `0` / block | a failure expired |
| `0093C63B` | `0075B430` | `session_message_construct` | message / none / void | a failure expired |
| `0093C67D` | `0077C2A0` | `session_route_message` | session / message / void | a failure expired |
| `0093C72E` | `0093BA80` | `failure_vector_shrink` | vector / new size / void | a failure expired |
| `0093C1FA`, `0093C2EA` | `[vt+1ACh]` | `unit_add_damage` | unit / float / void | elapsed above zero |
| `0093BF1B` | `008782A0` | `resolve_component` | none / owner, segment / component | `hit+34h != -1` |
| `0093BF5F`, `0093BF66`, `0093BFBA` | `00424C40` | `game_settings` | none / none / settings | fallback chance, descriptor lookup |
| `0093BF98` | `00BD2F10` | `random_unit_float` | none / `0`, `1.0f` / float | after the probability |
| `0093BFC1` | `0093AA00` | `failure_descriptor_for` | none / owner / row | the draw passed |
| `0093C035` | `0093A5D0` | `failure_already_active` | set / none / bool | a row was found |
| `0093C067` | `00425F40` | `native_string_assign_header` | record+4h / row+4h / void | a new failure |
| `0093C07A` | `0093A2C0` | `failure_message_for` | none / owner, segment, `1` / message | a new failure |
| `0093C08E`, `0093C67D` | `0077C2A0` | `session_route_message` | session / message, `7`, `0` / void | a new failure |
| `0093C0A7` | `0093BC30` | `failure_vector_push` | task / record / void | a new failure |
| `0093C0C2`, `0093C4D1` | `00982C50` | `warning_fire_failure` | unit / name, duration / void | a new failure |
| `0093C0DC` | `00913D80` | `failure_side_effect` | unit / hit, owner / void | a new failure |
| `0093C0ED`, `0093C4E3` | `[vt+21Ch]` | `unit_on_failure` | unit / name, hit / void | a new failure |
| `0093C0FF` | `0093AC30` | `failure_record_destroy` | record / none / void | a new failure |
| `0093C351` | `00BD2FC0` | `random_next_u32` | none / none / u32 | random pick |
| `0093C43D` | `0093A8B0` | `failure_find_by_name` | vector / name / iterator | random pick |
| `0093C4B2` | `0093BC30` | `failure_vector_push` | task / record / void | not already active |
| `0089175A`, `008919AA` | `0080E490` | `unit_parts_object` | unit / none / parts | binding |
| `00891761`, `008919B1` | `0092BEB0` | `parts_displacement` | parts / none / float | binding |
| `0089178E` | `0074E8F0` | `leak_manager_water_zones` | `unit+10D4h` / six `float*` / void | binding |
| `008919DE` | `0074E9C0` | `leak_manager_leak_zones` | `unit+10D4h` / six `float*` / void | binding |
| `008AD2B6` | `0081AD40` | `repair_category_index` | none / name / index | binding |
| `008AD2BE` | `00827A40` | `set_repair_level` | unit / index, level / void | binding |
| `008AD69F` | `00939FE0` | `set_failure_repair_enabled` | `unit+A20h` / bool / void | binding |
| `008AD494`, `008AD831` | `0075B430` | `session_message_construct` | message / id `9Fh`, `A0h` / void | binding |
| `008AD4CD`, `008AD866` | `0077C2A0` | `session_route_message` | session / message, `7`, `0` / void | binding |
| `008C6EF0`, `008C6F52`, `008C6F9A` | `[vt+5Ch]` | `unit_is_kind_of` | unit / class id / bool | cheat |
| `008C6F00`, `008C6F7C`, `008C6FF2` | `[vt+1B8h]` | `unit_max_repair` | unit / none / void | cheat |

`0093CAF1` is a **tail call** (`JMP 00914100`) with `ECX = [[00E188A8] + 21A0h]` and the unit as
its one stack argument; it is recorded separately in the report because it is not a `CALL` site.

The site `008ADCxx` for `GetRepairLevel`'s `0080E130` was not transcribed and is omitted rather
than guessed; the callee is proven by `bsp.py ghidra callees 008ADC40`.

## Open questions

* The message `9Eh`, `9Fh` and `A0h` handlers. They are the only writers of the fire and water
  rates and timers, of the hull-repair enable byte and of the priority, and they would settle
  which timer is fire.
* The repair task's constructor, and therefore the initial priority, rates and enable bytes.
* Whether any native path other than script starts a fire or a leak.
* The repair priority enum's names; the crew-level category names are a different enum.
* `vtable[1B8h]` (max repair) and `vtable[19Ch]` (the named-state dispatch shared with the
  subobject step's `"destroyed"` branch).
* The settings defaults at `+3B4h`, `+3B8h`, `+3C8h`..`+3E0h`, `+3E8h` and `+46Ch`.

## Values from docs/GAMEPLAY_SETTINGS.md (packet cc2_gameplay_settings)

The settings constants above now have keys and installed values: `+3B4h` is
`BodyRepairTickPercentage` (0.1), `+3D4h` `BodyRepairMultiplier` (2), `+3ACh` `FireTickDamage`
(40), `+3B0h` `WaterTickDamage` (100), `+3BCh`/`+3C0h` the ignition fallback pair (3 and 10),
`+3DCh`/`+3E0h` both 100. The authored key names at `+3C8h` and `+3CCh` are crossed against
their consumers: `PumpRepairMultiplier` feeds the fire path and `FireRepairMultiplier` the water
path; both ship as 3, so no run can tell them apart. The second assignment block in the shipped
script is gated on `FailureDebug`, false in the installed `ScriptOptions.lua`.

## Correction from docs/GAMEPLAY_SETTINGS_TAIL.md (packet cc2_settings_tail)

The provisional timer order above is reversed. The session message `9Eh` arm at `0082203D`
inside `00821E80` is the only writer of both repair timers: selector 0 (`SetFireDamage`
0088E320) reaches `00939F90`/`0093A470`, which write `task+38h`; selector 1 (`SetWaterDamage`
0088E790) reaches `00939FA0`/`0093A4F0`, which write `task+34h`. So `+34h` is the **water**
timer and `+38h` the **fire** timer; `0093C120` (runs `+34h`) is the water step with
`+3C8h PumpRepairMultiplier` as its divisor and `0093C210` (runs `+38h`) the fire step with
`+3CCh FireRepairMultiplier`. The authored key names were right; the two step names were
swapped in Ghidra (now BSP_RepairTask_* per that packet) and the fire/water repair priorities
in the table above are exchanged accordingly.
