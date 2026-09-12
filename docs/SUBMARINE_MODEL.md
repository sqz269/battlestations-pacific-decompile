# The submarine unit: depth bands, air supply, crush damage and the periscope

Addresses: 008531a0, 00852f10, 00854650, 00855420, 00855250, 008551c0, 008528b0, 00852c60,
00853630, 00853e10, 00852d30, 00936dc0, 004654d0, 008522c0, 00853090, 008530e0, and read-only
00852b90, 008527e0, 00852820, 00854230, 0087fa50, 009327f0, 009373c0, 0092bec0, 00813660,
0080e490, 0092d730, 00825f20, 008255b0, 0081ed40, 0095da00, 0077d1a0.

Packet `cc2_submarine_model`, worker `agent/cc2-submarine-model`, 2026-09-12. Ghidra was
**read-only** for this packet: no renames, no comments, no prototypes, no saves. Every
descriptive name below is a hypothesis except the ones marked **recovered**, which come from
string literals the binary itself uses as field names.

`reports/submarine_model.json` carries the machine-checkable call rows;
`include/bsp/submarine_model.hpp` and `src/submarine_model.cpp` carry the reconstruction.

## 1. What the binary itself calls these fields

Two overrides serialise and print the submarine's own state by name, so the names of eight
fields are **recovered data**, not guesses.

`00853E10` (main vtable slot `+A4h`, over the base `0081F980`) is the save/load hook. It opens a
section `_submarine` (`00D0BF28`), a nested section `airSupply` (`00D0BF1C`), and binds each
field *by address* to a typed archive entry (type 0 string, 1 int, 2 float, 3 bool):

| literal | address | type | instance offset | bound at |
| --- | --- | --- | --- | --- |
| `air` | `00D0BF18` | float | `+127Ch` | `00853EAF` |
| `needAir` | `00D0BF10` | bool | `+1281h` | `00853EE2` |
| `unlimitedAir` | `00D0BF00` | bool | `+1280h` | `00853F15` |
| `depthLevel` | `00D0BEF4` | int | `+1268h` | `00853F51` |
| `periscopeState` | `00D0BEE4` | int | `+122Ch` | `00853F8C`, stored back at `00853FA1` |
| `periscopeY` | `00D0BED8` | float | `+1230h` | `00853FB6` |
| `periscopeOut` | `00D0BEC8` | bool | `+1234h` | `00853FE9` |
| `sullyesztoEro` | `00D0BEB8` | float | `+1270h` | `0085401C` |

`00852D30` (slot `+A8h`, over `00818340`) pushes the same eight through a sink with the same
literals. `sullyesztoEro` is Hungarian for "submerging force"; the model node the boat looks up
is named `periszkop` and the controller vtable that holds the force model is preceded by the
literal `s_fizika` at `00D196E0`, so the whole subsystem carries the developer's own Hungarian
names. Treat all eight as recovered.

## 2. The class: id 8, instance `1288h`, a sibling of the ship unit

`docs/VEHICLE_CLASS_DESCRIPTORS.md` records the descriptor row: `MSubmarine`, class id `8`,
descriptor `840h`, instance `1288h`, constructor `00963DB0`, vtable `00D1AE38`, allocator
`008531A0`. This packet reads the instance side of that row.

`008531A0` (`__thiscall(descriptor, int flag)`, `RET 4`, body `008531A0`-`00853222`) is already
recorded as instruction-for-instruction identical to the ship's `006FE590` bar the size literal
and the leaf constructor. That leaf is `00852F10`.

`00852F10` is `__thiscall(instance, int flag)`, `RET 4`, body `00852F10`-`00852FAD`. It calls
`0081ED40` at `00852F18` with `ECX` still the instance (`MOV ESI,ECX` at `00852F16`; the
pseudocode drops the register argument and shows `BSP_UnitVehicleBase_Construct(param_2)`).
`0081ED40` is **level 5** of the chain in `docs/UNIT_INSTANCE_LAYOUT.md`, the same level the
ship's leaf `006FE460` calls. So the submarine unit is not derived from the ship unit: the two
are **siblings at level 6** over one shared five-level base, which is why the submarine's extra
fields start exactly at `1188h`, the ship's whole size, rather than inside it.

It writes the class id and nine leaf fields:

| store | field | value |
| --- | --- | --- |
| `00852F96` | `+C4h` class id | `8` (the ship leaf writes `7`) |
| `00852F6D` | `+1238h` | `0`, the seabed-scan step index |
| `00852F73` | `+127Ch` `air` | `DAT_00D7A24C` = `1.0f`, so a boat spawns with full air |
| `00852F7E` | `+1280h` `unlimitedAir` | `0` |
| `00852F84` | `+1281h` `needAir` | `0` |
| `00852F8A` | `+1235h` | `0`, the periscope auto-raise enable |
| `00852F90` | `+1238h` | (see above) |
| `00852FA0` | `+1274h` | `0.0f` |

The eight multiple-inheritance vptrs it installs, against `MDestroyer`'s set:

| vptr offset | submarine | `MDestroyer` |
| --- | --- | --- |
| `+0h` | `00D0BF80` | `00CFC3D0` |
| `+10h` | `00D0BF68` | `00CFC3B8` |
| `+24h` | `00D0BF60` | `00CFC3B0` |
| `+170h` | `00D0BF5C` | `00CFC3AC` |
| `+1E4h` | `00D0BF54` | `00CFC3A4` |
| `+310h` | `00D0BF3C` | `00CFC38C` |
| `+38Ch` | `00D0BF38` | — |
| `+72Ch` | `00D0BF34` | — |

### The overrides

Diffing `00D0BF80` against `00CFC3D0` over 120 slots gives the submarine's whole added
behaviour on the main interface. Ten slots differ and nothing else does:

| slot | submarine | base | what it is |
| --- | --- | --- | --- |
| `+0h` | `00853380` | `006FE570` | scalar deleting destructor |
| `+5Ch` | `00853050` | `006FE530` | `BSP_MSubmarine_IsKindOf`; every gate in the game that asks "is this a submarine" calls this with `8` |
| `+84h` | `00853590` | `00819880` | forwards to `00819880`; not read |
| `+A0h` | `00853630` | `00822C20` `BSP_UnitInstance_SEntityInit` | scene attach: the wake effects, the depth-band table, the periscope node |
| `+A4h` | `00853E10` | `0081F980` | save/load, section 1 above |
| `+A8h` | `00852D30` | `00818340` | state dump, section 1 above |
| `+DCh` | `00854650` | `008255B0` `BSP_UnitInstance_Update` | the per-frame update |
| `+130h` | `00853090` | `006FE620` | world-list registration; five `BSP_UnitList_PushBack` calls where the ship has its own count |
| `+134h` | `008530E0` | `006FE670` | world-list detach |
| `+164h` | `00852C60` | `00821E80` `BSP_UnitInstance_HandleMessage` | adds message kinds `A1h` and `A2h`, tail-jumps to the base for everything else |

Diffing the `+310h` sub-object vtable `00D0BF3C` against `00CFC38C` gives seven:

| slot | submarine | base | what it is |
| --- | --- | --- | --- |
| `+0h` | `00853030` | `006FE510` | not read |
| `+8h` | `00855420` | `00825F20` `BSP_UnitInstance_UpdateShipMotion` | the fixed-step motion tick |
| `+1Ch` | `00852B90` | `006DFD20` | `BSP_SensorCategory_Submarine`, `docs/SENSOR_TABLES.md` |
| `+20h` | `00852FB0` | `006FE4D0` | `MOV EAX,0Dh` / `RET`, a kind constant |
| `+28h` | `00853010` | `006FE4F0` | `SUB ECX,24h` / `JMP 00853380`, an adjustor thunk to the destructor |
| `+2Ch` | `00853040` | `006FE520` | not read |
| `+44h` | `00853380` | `006FE570` | destructor |

The `+1E4h` and `+170h` interface vtables repeat the same overrides at their own slot numbers
(`00852B90` sits at `00D0BF54 + 4h` as well as `00D0BF3C + 1Ch`), which is how `00852B90` gets
two apparent slot numbers.

### The two ticks

| routine | `this` | ABI | body | base it extends |
| --- | --- | --- | --- | --- |
| `00854650` | unit | `__thiscall(unit, float dt)`, `RET 4` | `00854650`-`008551B9`, SEH scope table `00C94748` | calls `008255B0` first at `00854676` |
| `00855420` | unit `+310h` | `__thiscall(unit+310h, float dt)`, `RET 4` | `00855420`-`00855937`, then a 13-entry jump table at `00855938` | calls `00825F20` first at `0085542F` |

Neither has a Ghidra function. `00855420`'s `this` is the `+310h` sub-object: `LEA EBP,[ESI-310h]`
at `00855775` recovers the unit, and `CMP byte ptr [ESI-248h]` at `0085575A` is the same
`unit+C8h` pose-dirty byte that `CMP byte ptr [EBP+0C8h]` reads at `00855782`. Its two tail calls
set `ECX = EBP`, so the air and crush models are methods on the **unit**, not on the sub-object.

`docs/SHIP_MOTION.md` already listed `0085542F` as one of `00825F20`'s three call sites and named
it "the routine starting `00855420`, no Ghidra function"; this packet identifies it.

## 3. The depth state machine

### The band table, `00853630`

Four world-space Y values at `+1200h`..`+120Ch`, built once at scene attach by the loop at
`00853A90`..`00853AC5`:

```
for (i = 0, off = 80Ch; off < 81Ch; ++i, off += 4)
    bands[i] = (i < 1 || *(float*)(class + off) < 0.0f)
                 ? kDefaultBands[i]            // 00E0AD6C + off
                 : -0.0f - *(float*)(class + off);   // DAT_00D7A208 = -0.0f
```

`i == 0` always takes the default path (`00853A90` is `CMP EDI,EBX` with `EBX` zero and a `JLE`),
so `+80Ch` (`PeriscopeWave`, an effect handle, not a float) is never read as a number. The class
floats used are `+810h`, `+814h`, `+818h`. The default table is four dwords at `00E0B578`:
`{0.0, -20.0, -40.0, -80.0}`. The compiler indexes it with the same register as the class, so
`00853A63` loads `00E0B578` and `00853A7B` subtracts `80Ch` from it; any label Ghidra shows at the
resulting `00E0AD6C` is an artifact of that fold, not a real object. The Lua depth is positive
metres and the instance value is a negative world Y, which is what `-0.0f - depth` does
(`00853AA4`..`00853AAC`, `MOVAPS` of `DAT_00D7A208` then `SUBSS`).

| band | `depthLevel` | instance offset | class key | shipped default |
| --- | --- | --- | --- | --- |
| Surface | 0 | `+1200h` | none, always the default | `0.0` |
| Periscope | 1 | `+1204h` | `PeriscopeDepth` (`+810h`), legacy alias `SwimDepth1` | `-20.0` |
| Underwater | 2 | `+1208h` | `SwimDepth2` (`+814h`) | `-40.0` |
| Max depth | 3 | `+120Ch` | `SwimDepth3` (`+818h`) | `-80.0` |

The three near-identical gameplay settings `Submarine.SubmarinePeriscopeLevel` `20.0`,
`SubmarineMediumLevel` `50.0` and `SubmarineDeepLevel` `80.0` (`docs/GAMEPLAY_SETTINGS.md`
`+49Ch`..`+4A4h`) are **not** this table. They have a different producer and only the first has
a recorded reader (`BSP_Entity_IsSurfaceTarget`). Do not conflate them.

### The initial band

`00853630` picks `depthLevel` in three stages (`00853A26`..`00853BB3`):

1. `+1268h = (class.KamikazeDamage > 0 || class.KamikazeBlastDamage > 0) ? 1 : 0`. A kamikaze
   class (the human-torpedo type; `+510h`/`+514h` per `docs/SHIP_CLASS_FIELDS.md`) starts at
   periscope depth.
2. If `*(int*)(unit+C0h) + 4h == 1`, the scene property bag is consulted:
   `Dive` (`00D0B6A4`) sets `depthLevel` **and** teleports the entity, writing
   `bands[depthLevel]` into its local Y at `+A8h`, clearing `+C8h`/`+10Ch` and invalidating every
   child's pose. `TargetDive` sets `depthLevel` only.
3. Otherwise the nearest band wins: refresh the pose, then over `i = 1..3` take the `i` whose
   `|hullY - bands[i]|` is smallest, starting from `|hullY|` as band 0's error.

### The command, `008528B0`

`__thiscall(unit, int level)`, `RET 4`, body `008528B0`-`0085296E`:

```
level = clamp(level, 0, 3);
if (class.KamikazeDamage > 0 || class.KamikazeBlastDamage > 0) level = 1;
if (unit->depthLevel != level) {
    unit->depthLevel = level;
    // 0085291E: session message kind A2h, payload = level, sent through 0077C7B0
}
```

The kamikaze clamp means a human torpedo cannot be commanded off periscope depth at all. The
message is the multiplayer echo, and the submarine's own handler consumes it.

### The arms that reach it

| caller | site | what it passes |
| --- | --- | --- |
| `004654D0` | `00465506`, `00465524`, `00465542`, `00465560` | the mission/entity command. `__thiscall(command, entity)`, `RET 4`, body `004654D0`-`00465569`. `ESI = [ESP+8]` is the entity and `EDI = ECX` is the command; the gate is `entity->vtable[5Ch](8)`, then `command+2Ch` (a native string) is matched case-insensitively against `Surface` → 0, `Periscope` → 1, `Underwater` → 2, `Maxdepth` → 3. `ECX` at every `008528B0` site is `ESI`, the entity. Its only reference is the arm-table dword at `00CE5530`. |
| `00852C60` | `00852C81` (kind `A2h`), `00852C9F` and `00852CB2` (kind `A1h`) | the message handler, below |
| `0081F980` | `0082018D` | the base save/load hook, not read by this packet |
| `00893F40` | `008940A5` | not read |
| `009E4B90` | `009E4BA5` | ship AI: `SetDepthLevel(2)` on entry to a state |
| `009E4EE0` | `009E4F00`, `009E4F2B`, `009E4F3D` | ship AI: `periscopeState != 2 && periscope != 0` → 1, else a second gate → 1, else 0. A broken periscope (state 2) forces the AI to surface. |
| `009EA8DE`, `009EA949` | — | ship AI, not read |

`004654D0`'s four literals are recovered command names. `python tools/bsp.py find` misses them
because they are plain `.rdata` C strings at `00CE5490`, `00CE5418` and their neighbours rather
than ledger entries.

### The message handler, `00852C60`

`__thiscall(unit, msg)` returning `AL`, `RET 4`, body `00852C60`-`00852CB9`. No Ghidra function.
It switches on the byte at `msg+10h`:

| kind | arm | rule |
| --- | --- | --- |
| `A1h` | `00852C8B` | step. `msg+20h` byte zero → deeper: `if (level < 3) SetDepthLevel(level+1)`. Non-zero → shallower: `if (level > 0) SetDepthLevel(level-1)`. Returns 1 either way, even when clamped. |
| `A2h` | `00852C7D` | absolute. `SetDepthLevel(*(int*)(msg+20h))`, the dword payload. Returns 1. |
| anything else | `00852C74` | rewrites `[ESP+4]` and `JMP 00821E80`, the base handler for kinds `4Bh`..`A0h`. |

`A1h`/`A2h` sit immediately above the base's `A0h` ceiling, so the submarine owns exactly the two
kinds the base leaves free.

### The periscope

`00853630` looks the node up by name: kamikaze classes get `+1214h = 0` and no periscope at all;
everything else gets `FUN_0071AD50("periszkop")`. On success it stores `periscopeState = 0` and
`periscopeY = localMatrix[13]`, the node's rest local Y, then creates a `6Ch` object at `+1224h`
owned by the unit and reads the `periscope_sight` marker list into `+1218h`..`+1220h`
(default `{0, 10.0, 0}`, `DAT_00CE38B8`).

`periscopeState` has three values:

| value | meaning | set by | cleared by |
| --- | --- | --- | --- |
| 0 | stowed / lowering | `00853CAB` at attach, `008549EA`-region and `00854ECF` | — |
| 1 | raising / raised | `00854F48` | `00854ECF` on repair, or falling to the lowering arm |
| 2 | **broken, repairing** | `009373C0` at `009373E9`, and `009327F0` at `009327F7` | `00854ECF` when the deadline passes |

`00854650` runs the state machine each frame:

1. `00854B00` clears `periscopeOut` unconditionally, before anything else. It is a per-frame
   output, not a latch.
2. `00854E44`: if `periscopeState == 2`, accumulate the repair. `dt` is scaled by
   `settings.FailureRepairMultiplier` (`+3D0h`, installed `3`) when `unit+A44h == 1`, otherwise
   raw. `+1258h += dt`, `+125Ch += 2*dt`. When `+1258h >= +1254h` the repair completes:
   `BSP_UnitInstance_GetPartsObject(unit)` then `0092BEC0`, which zeroes `+1254h`, restores the
   node's visibility to `1.0` and re-enables its collision; `+122Ch` becomes 0.
3. `00854ED7`: the auto-raise. Only when `+1235h` is set **and** `depthLevel == 1`, and only when
   the hull Y is strictly inside `bands[1] ± 2.5` (`00CE3DE0`, a double), does `+122Ch` become 1.
   Both bounds are `JBE` skips, so the window is open at both ends. The first bound needs the
   assembly: `00854F10` is `D8 E9`, `FSUBR ST(0),ST(1)`, so it is `bands[1] - 2.5` and not
   `2.5 - bands[1]` as the operand order in a linear listing suggests. The second bound re-reads
   the hull Y through `00427EB0 BSP_EntityPose_GetWorldPositionRefreshed` at `+4h`.
   `+1235h` itself is written only by two reflection setters, `00464320`
   (`__thiscall(this, bool)`) and `004649B0` (a string form comparing against `00CE53CC`), so it
   is a Lua/mission-settable property, default 0 from the constructor.
4. `00854F52`: if `periscopeState == 1` the mast extends at `dt * 5.0` (`00D7A370`) toward
   `class.PeriscopeMoveRange + periscopeY`; otherwise it retracts at `dt * 3.0` (`00D7A2B0`)
   toward `periscopeY`. Either way the new local Y goes through `0042AC60` and is written to the
   node with `node->vtable[2Ch]`.
5. `00855045`: `periscopeOut = 1` only on the extending arm, only once the mast's Y reaches
   `class.PeriscopeMoveRange + periscopeY - 1.0` (`00D7A210`, a double `1.0`).

`00852B90 BSP_SensorCategory_Submarine` reads `periscopeOut` to choose between `PeriscopeOut` (3)
and `PeriscopeIn` (2). `docs/SENSOR_TABLES.md` noted "the four depth words and the periscope byte
are produced elsewhere and are not read here" — this is that elsewhere.

### The break, and the reporter that is never called

`009373C0 BSP_UnitController_ClearShapeCollisionBits` (body `009373C0`-`00937438`, reached from
the contact callback `009377E0` when contact bit 1 is set) is the periscope break:

```
if (controller->+394h) {
    SetVisibilityFactor(0.0, 0);               // hide the mast
    unit->periscopeState = 2;
    unit->+125Ch = 0.0f;
    unit->+1258h = DAT_00F876A4;                            // the clock snapshot
    unit->+1254h = settings.PeriscopeRepairTime + DAT_00F876A4;
    DYN_physics_00C47FC0(2);                   // drop the mast's collision
}
```

`settings.PeriscopeRepairTime` is `+4C4h`, default 30, installed **60**
(`docs/GAMEPLAY_SETTINGS.md`), and that doc already records `009373C0` as one of its two readers.
`009327F0` (`__fastcall(unit)`, `RET 0`, body `009327F0`-`0093283A`, no Ghidra function, no
references at all) is a second copy of the same four stores without the visibility and physics
calls. `00813660` is a third consumer of `periscopeState` and the caller of the repair completion
at `0081378A`; it was not read.

**`00977500 BSP_WarningManager_ReportSubmarinePeriscopeBroken` has no callers.** `ghidra xrefs`
finds none, and a scan of the whole `.text` section for the absolute value `00977500` finds none
either, so it is not reached through a table. Neither break site reports. The
`submarineperiscopebroken` warning string is present and the reporter is complete and dead: the
player is told about a broken periscope only by the mast disappearing. The three sibling
reporters are live: `00977050` and `009771E0` from `00855375` and the critical arm of
`00855250`, `00977370` from `0085523C` in `008551C0`.

## 4. Air supply, `00855250`

`__thiscall(unit, float dt)`, `RET 4`, body `00855250`-`00855415`. Coverage: complete.
Thresholds are `docs/GAMEPLAY_SETTINGS.md` names with installed values.

```
surfaceY = bands[1] + 3.0;                 // 00D7A2B0, a double
if (surfaceY < 0.0f) surfaceY = -4.0f;     // 00CEE4E0 = 0.0f, 00CF1430 = -4.0f
if (!unit->poseClean) RefreshWorld();

if (hullY <= surfaceY) {                                     // submerged
    if (!unlimitedAir) {
        old = air;
        air = old - dt / class.AirRunOutTime;                // +838h, default 120.0
        if (air <= AirNeedLimit && old > AirNeedLimit)        // 0.16
            ReportSubmarineAirCritical();                    // 009771E0
        else if (air <= AirWarningLimit && old > AirWarningLimit)   // 0.35
            ReportSubmarineAirLow();                         // 00977050
    }
} else {
    air += dt / class.AirReloadTime;                         // +83Ch, default 5.0
}

if (air > 1.0f) air = 1.0f;                                  // DAT_00D7A24C
else if (air < 0.0f && !unit->+5Dh) { air = 0.0f; DestroyAndBroadcast(1); }

if (!needAir) { if (air < AirNeedLimit) { needAir = true; return; } }
if (needAir && air > AirEnoughLimit) needAir = false;        // 0.5
```

Four things worth naming:

* The breathing line is not the periscope band. `bands[1] + 3.0` is computed and then
  **discarded whenever it is below zero** (`00855266`..`00855286`: `FCOMIP` of `0.0` against the
  sum, `JBE` keeps the sum, the fall-through loads `-4.0`). Every shipped periscope depth is
  around `-20`, so the sum is around `-17` and the line is the flat **`-4.0` metres** in
  practice. The sum only survives for an unusually shallow class whose periscope depth is
  above `-3`. A boat at ordered periscope depth is therefore deep in air-consuming territory:
  it must come nearly to the surface to breathe.
* Both warnings are **edge-triggered on the way down only**, and the critical edge suppresses the
  low edge on the same step. The pseudocode's shape is
  `if (air > crit || old <= crit) { low edge } else { critical }`, which is exactly
  "critical when `air <= crit && old > crit`".
* Running out of air is fatal, not a slow damage: `vtable[70h]` is
  `0077D1A0 BSP_UnitInstance_DestroyAndBroadcast(1)`. The guard `unit+5Dh` is the death flag, so
  a boat already sinking does not drown twice. The slot is loaded and `air` is stored as 0
  **before** the indirect call is made, so the store survives whatever the destroy path does.
* `needAir` is the hysteresis latch, not a warning: it arms below `0.16` and disarms above `0.5`,
  and the `return` after arming means the disarm test is skipped on the arming step. Its consumer
  is the force model, section 6.

Air is normalised `0..1`, not seconds: the constructor seeds `1.0` and both rates are
`dt / duration`. `AirRunOutTime` installed for a class is the seconds-to-empty from full.

## 5. Crush depth, `008551C0`

`__thiscall(unit, float dt)`, `RET 4`, body `008551C0`-`0085524F`. Coverage: complete.

```
acc = unit->+1284h + dt;
unit->+1284h = acc;
if (acc > 1.0f) {
    if (!unit->poseClean) RefreshWorld();
    if (hullY < -settings.SubmarineDamageDepth) {          // +4B0h, installed 70.0
        unit->vtable[1ACh](settings.SubmarineDepthDamage * acc);   // +4ACh, installed 9.0
        ReportSubmarineDepthDamage();                      // 00977370
    }
    unit->+1284h = 0.0f;
}
```

The depth limit is a **global gameplay setting, not a class field**: every submarine in the game
is crushed below the same 70 metres, and `SwimDepth3` only decides how deep the player can order
the boat, not where it starts taking damage. A class whose `SwimDepth3` is shallower than 70
therefore never crushes at all under its own commands.

`vtable[1ACh]` of `00D0BF80` is `0095DA00 BSP_UnitInstance_AddDamage(float)`. The accumulator
makes this a once-per-second pulse, and the damage is multiplied by the accumulated interval
rather than the frame `dt`, so the rate is 9 per second regardless of frame rate but the pulse
lands on the first step past each whole second.

## 6. How the depth target reaches the hull physics

This is a **contract** with the unit controller and the DYN physics body, both owned elsewhere
(`docs/SHIP_MOTION.md`, `docs/UNIT_CONTROLLER.md`). The producer is on the submarine side; the
consumer is not.

`00936DC0` is the submarine's controller force model: `__thiscall(controller, float dt)`, body
`00936DC0`-`009373B4`, its only reference the vtable dword at `00D196EC`. `docs/SHIP_MOTION.md`
establishes that `00937440`, the dword at `00D196F0`, is the **ship's** force model reached at
`00826A6D` as `controller->vtable[0](dt)`; `00936DC0` is the neighbouring slot of the same shape
and the literal `s_fizika` sits at `00D196E0` immediately before it. `controller+1Ch` is the unit
and `unit+1018h` is the controller: `00855480` passes `unit+1018h` to
`0092D730 BSP_UnitController_GetBodyAxisSpeed` and `00854EC1` passes it to `0092BEC0`, which
reads `+1Ch` as the unit.

The sequence, with the register-input caveat below:

1. Read the body's linear velocity and speed through `DYN_physics_00C31F40` / `00C31F20` /
   `00C32000`, then `BSP_UnitController_ApplyHydroForces(dt)`.
2. Resolve the **effective** band from the commanded one:
   * `needAir` or `008522C0` true, and the class is not kamikaze → band 0. The boat surfaces
     itself. `008522C0` (`__fastcall(unit)` → bool, `RET 0`, body `008522C0`-`008522F7`) answers
     "`unit+1279h` is set and some child answers `IsKindOf(28h)` with `child+490h >= 0`". Class
     `28h` is `MCatapult` (`docs/ENTITY_CLASS_IDS.md`), so a seaplane-carrying boat surfaces for
     a pending launch.
   * `unit+5Dh` (dead) → band 3. A destroyed boat is driven to maximum depth: this is how a
     wreck sinks.
3. `targetY = bands[effective]`. If the unit has a `+740h` owner whose class is `8` (or
   `00927F10` agrees) **and** `targetY < unit->+123Ch`, clamp `targetY` up to `+123Ch` through
   `00415510` and set the gain `local_44` to `1.5` (`00CE380C`). `+123Ch` is the seabed clearance
   of section 7.
4. `error = targetY - hullY`. When `effective != 0` or `error <= 1.0`, evaluate a curve
   (`00419010`, bounds `0.5` and `class.UpDownStopTime` `+82Ch`) on
   `error * gain / clampedVerticalSpeed` and blend the current vertical velocity toward
   `class.UpSpeed` (`+830h`) or `class.DownSpeed` (`+834h`); otherwise the commanded rate is 0.
   `unit+1278h` is set to `effective == 0` in the same branch.
5. If the seabed clamp engaged, two more curve evaluations of `+1244h` and `+1240h` are written
   into the order-ring slots at `unit + 844h + unit[97Ch]*20h` and `+848h`, so the scan also
   steers the dive planes.
6. If `unit+126Ch` is set (the one-shot seed from `00853630`), `sullyesztoEro = -0.0 - vy`,
   `+1274h = 0` and `+126Ch` clears.
7. `sullyesztoEro` is slewed toward the commanded rate at `class.UpDownAccel` (`+824h`) per
   second, with a speed-dependent term on the decreasing side.
8. **The contract:** the body's velocity Y has `sullyesztoEro` subtracted from it, and the result
   is written back through `DYN_physics_00C37E50` and `DYN_physics_00C37E20`. Nothing else in the
   submarine module touches the physics body.

Coverage of `00936DC0` is **partial**: steps 1-6 are read from the pseudocode and cross-checked
against the field offsets, but the decompiler reports `unaff_EBX` and `unaff_retaddr` for the
slew in step 7, so the exact sign and the speed term of `009372A0`-`009373B4` are **not
established**. The reconstruction covers steps 2 and 3 only and says so.

## 7. The seabed-clearance scan, `00855420`

The motion tick spends its own body on a rotating scan and only then runs the air and crush
models. Per step, with `state = unit->+1238h`:

1. Call `00825F20 BSP_UnitInstance_UpdateShipMotion(dt)` first, `ECX` unchanged.
2. When `state == 0` (`00855440`), publish and reset the previous sweep:
   `+123Ch = +1248h`, `+1240h = +124Ch` then `+124Ch = 0`, `+1244h = +1250h` then `+1250h = 0`,
   `+1248h = +120Ch`. So `+123Ch`/`+1240h`/`+1244h` are the double-buffered outputs the force
   model reads and `+1248h`/`+124Ch`/`+1250h` are the accumulators, seeded from the deepest band.
3. Read the body-axis speed (`0092D730` on `unit+1018h`) and derive a distance-over-speed time
   from `class.MaxSpeed` (`+500h`), `class.Length` (`+A0h`) and `class.Width` (`+A4h`).
4. `state` selects one of 13 jump-table arms at `00855938`, each producing an `(x, z)` offset
   from `{-0.0, ±Width, ±2*Width, 0, Length}` combinations: a ring of sample points around the
   hull footprint. Three arms (`00855643`, `00855680`, `008556A9`) are pure x87 and compute
   rotated offsets. `state > 0Ch` resets it to 0 at `008556BB`.
5. Transform the point to world space (`004142E0 BSP_Vector3f_TransformAffinePoint` against the
   pose at `unit+CCh`) and walk the provider list `[00E188A8] + 19CCh` → `+34Ch`, `next` at
   `+4h`, object at `+8h`. For each, `0087FA50(object, x, z)` — a two-instruction forwarder,
   `__thiscall(this, float, float)`, `RET 8`, to `this->+3D0h->vtable[28h]` — samples a height.
   A zero result skips the provider.
6. `+1248h = max(+1248h, sample + class.Height + 3.0)` (`class+A8h`). So the published clearance
   is the shallowest floor under the footprint raised by the boat's own height and a three-metre
   margin, expressed as a world Y.
7. Two further comparisons bucket the sample into `+124Ch` or `+1250h` by `state` ranges
   (`state < 2`, `4 <= state < 7`, else) — the pitch and roll terms the force model reads.
8. Advance: `state = (state < 0Ch) ? state + 1 : 0`.
9. `0085591C`: `00855250(unit, dt)`. `0085592B`: `008551C0(unit, dt)`. Both with `ECX = EBP`,
   the unit, and the tick's own float argument forwarded from the stack.

Coverage of `00855420` is **partial**: the sequence, the buffers, the provider walk and the
clearance formula are established; the 13 geometry arms are read but the exact sample ring is
**not reconstructed**, and the `+124Ch`/`+1250h` bucketing rule is read from the branch structure
only. Unread as behaviour: `00855643`-`008556BB`.

## 8. The tuning keys, `00854230`

`BSP_SubmarineClass_ReadLuaFields`, slot `+8h` of the descriptor vtable,
`__thiscall(descriptor, LuaObject* row)`, `RET 4`, chaining to `00831840`
`BSP_ShipClass_ReadLuaFields` first. Thirteen keys into `+80Ch`..`+83Ch`:

| key | offset | default | used by |
| --- | --- | --- | --- |
| `PeriscopeWave` | `+80Ch` | none, integer-guarded effect handle | `00854D4C` (the wake effect spawn) |
| `PeriscopeDepth` | `+810h` | `DAT_00D7A260` = `-1.0`, so unset falls to `-20.0` | band 1, the air surface line, the auto-raise window |
| `SwimDepth1` | `+810h` | the legacy alias, read only when `PeriscopeDepth` left the slot negative (`00854332`) | — |
| `SwimDepth2` | `+814h` | `-1.0` → `-40.0` | band 2 |
| `SwimDepth3` | `+818h` | `-1.0` → `-80.0` | band 3 |
| `PeriscopeMoveRange` | `+81Ch` | `DAT_00CE38B8` = `10.0` | the mast travel, `00854FAF` and `00855039` |
| `UpDownAccel` | `+824h` | `DAT_00CE3868` = `0.25` | the `sullyesztoEro` slew rate |
| `UpDownRotation` | `+828h` | — | not read by this packet |
| `UpDownStopTime` | `+82Ch` | `DAT_00CE3850` = `5.0` | the depth curve bound |
| `UpSpeed` | `+830h` | `DAT_00CE3814` = `1.2` | the rise rate |
| `DownSpeed` | `+834h` | `DAT_00CE3814` = `1.2` | the dive rate |
| `AirRunOutTime` | `+838h` | `DAT_00D05804` = `120.0` | `00855250` |
| `AirReloadTime` | `+83Ch` | `DAT_00CE3850` = `5.0` | `00855250` |

`+820h` is in the range but no key writes it.

**No installed class values are quoted here.** The per-class Lua rows live in the packed archives
and reading them is not in this packet's scope; the defaults above are the binary's own and are
the values any class that omits a key actually gets. The installed *global* values are quoted in
sections 4 and 5 from `docs/GAMEPLAY_SETTINGS.md`.

## 9. Consumers outside this packet

| reader | field | note |
| --- | --- | --- |
| `00852B90` | `+1200h`..`+120Ch`, `+1234h` | the sensor category, `docs/SENSOR_TABLES.md` |
| `008527E0` | `+1204h` | the torpedo-bot engagement gate, `docs/GUN_BOT_REMAINDER.md` |
| `00852820` | `+1204h` | the depth-charge depth gate |
| `00650532`, `0065048D`, `006500F2` and neighbours | `+1200h`, `+1268h`, `+127Ch`, `+122Ch` | the submarine and periscope HUD screens, another orchestrator's packet |
| `009E4EE0`, `009E4B90`, `009F29B4` region | `+122Ch`, `+1268h`, `+1270h`, `+1274h`, `+1278h`, `+1279h` | the ship AI |
| `00936DC0` | `+123Ch`, `+1240h`, `+1244h`, `+1268h`, `+126Ch`, `+1270h`, `+1274h`, `+1278h`, `+1281h`, `+5Dh` | the force model, section 6 |
| `00813660` | `+122Ch` | a part-damage handler, not read |

## 10. What `bsp_game.exe` still lacks for a submarine mission

Reconstructed here as pure rules: the band table, the initial band, the command clamp and step,
the air model, the crush model and the effective-band resolution. Analysed but not reconstructed:
the seabed geometry ring, the `sullyesztoEro` slew, the periscope mast pose and the wake effects.
Not started: the `6Ch` object at `+1224h`, the `00852970` tick it receives, and the periscope
camera. None of the reconstruction is ABI-compatible; it is a behavioural projection.
