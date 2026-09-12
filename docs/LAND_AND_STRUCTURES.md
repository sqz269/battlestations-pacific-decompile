# Land units and structures (packet `cc2_land_and_structures`)

Addresses: 0074df10, 0074dcc0, 00747000, 00745940, 006f5c10, 006f5610, 004f2700, 004f2410,
006d3110, 006d1c20, 00848380, 00848080, 00743060, 007410c0, 007410b0, 00742400, 00743450,
007420b0, 00742c10, 007422a0, 0074cd10, 0074cda0, 006f57a0, 006d2510, 00846320, 006f7360,
004f0fb0, 004f0930, 004f0a00, 004e9900, 004e9910, 00844fc0, 0080f960, 0082226f

Every descriptive name here is a hypothesis, not a recovered symbol. Read-only analysis: this
packet renamed nothing in Ghidra and added ledger records only.

Three results dominate this packet.

1. **None of the six classes is a ship.** Every one of them stops at the level-4 base
   (`0095CC90`, class id `05`, whose own storage ends at `+72Ch`) and adds a small tail.
   The largest, `MAirfield` at `0x8E4`, is still less than half the `0x1188` ship instance of
   `docs/UNIT_INSTANCE_LAYOUT.md`. Any offset above `0x8E4` quoted for one of these classes is
   a ship offset and does not exist on them.
2. **`MLandVehicle` has no motion of its own.** It overrides neither the tick element's
   per-step slot (`+8h` stays the inherited base advance `00953CC0`) nor the per-frame update
   (`+0DCh` stays the base `00956600`), and it never reaches the ship motion controller
   `00825F20`. A land vehicle's pose is written every step by its `LandConvoy`, from
   `00742400`. A land vehicle whose `+738h` convoy pointer is null never moves.
3. **The shipyard does not receive message `7Ah`; it sends it.** `MT_VEHICLE_SHIPYARD_LAUNCH`
   writes `unit+1130h`, a ship-only offset. `00844FC0` sets the scene property
   `ShipYardLaunch` on the *new ship's* property bag at `008450AA`, and that becomes the
   launched ship's `+1130h`.

## 1. The six classes

`+C4h` carries the class id and the descriptor back-pointer at `+354h` is written by the
allocator, exactly as `docs/UNIT_INSTANCE_LAYOUT.md` established for `MDestroyer`. The
"allocator" column is the routine the vehicle-class descriptor reaches through its vtable slot
`+28h`; the leaf constructor is the most derived constructor it calls.

| class | id | parent id | allocator | instance | leaf ctor | main vtable | slots | element vtable at `+310h` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `MLandVehicle` | `19h` | `05` | `0074DF10` | `0x740` | `0074DCC0` | `00CFFDE0` | 129 (+0) | `00CFFD9C` |
| `LandConvoy` | `1Ah` | `02` | `004F2700` | `0x3CC` | `004F2410` | `00CEA570` | 100 (+0) | `00CEA528` |
| `MLandFort` | `1Bh` | `05` | `00747000` | `0x758` | `00745940` | `00CFF3F8` | 132 (+3) | `00CFF3B4` |
| `MCommandBuilding` | `1Ch` | `1Bh` | `006F5C10` | `0x7E8` | `006F5610` | `00CFB028` | 135 (+6) | `00CFAFE0` |
| `MAirfield` | `45h` | `05` | `006D3110` | `0x8E4` | `006D1C20` | `00CF8C08` | 131 (+2) | `00CF8BC0` |
| `MShipyard` | `46h` | `05` | `00848380` | `0x7A4` | `00848080` | `00D0B770` | 129 (+0) | `00D0B728` |

The parent-id column is not taken from the class-id table: it is the level the leaf constructor
calls. `0074DCC0`, `00745940`, `006D1C20` and `00848080` all call `0095CC90` (level 4, id `05`);
`006F5610` calls `00745940` at `006F5635` (so `MCommandBuilding` really derives from
`MLandFort`); `004F2410` calls `0077EED0` at `004F2434` (level 2, id `02`, the same level the
plane squadron uses). Six agreements with `docs/ENTITY_CLASS_IDS.md`.

The five unit-kind allocators are one routine written five times: `operator new(size)`,
`memset(0, size)`, the leaf constructor when the allocation succeeded,
`009553D0 BSP_Unit_SetVehicleClass`, then `[instance+354h] = descriptor`. They inherit the
null-allocation fault `docs/UNIT_INSTANCE_LAYOUT.md` records for `006FE590`: on a null
`operator new` the instance register is zeroed and `009553D0` is still called.

### What each class adds beyond the level-4 base

| class | own fields, from the leaf constructor |
| --- | --- |
| `MLandVehicle` | `+72Ch`/`+730h`/`+734h` = 0 (`0074DD0B`, `0074DD15`, `0074DD1F`): a `std::vector` triple, read as begin/end by `0074DB70` at `0074DC2E`/`0074DC34`. `+738h` = the convoy, `+73Ch` = the slot index: both left zero by the `memset` and written by the convoy at `00743450`. Seven vptrs, no eighth. |
| `LandConvoy` | `+310h` tick registration (`00875890`, `004F2449`), `+344h` route/formation record (`0049F9B0`, `004F2459`), `+398h`/`+39Ch`/`+3A0h` member vector = 0, `+3C8h` = 0 (byte). Six vptrs (no `+38Ch`). |
| `MLandFort` | `+72Ch` owned-ref slot (`00809270`, `00745978`), `+738h` master fort, `+73Ch` section record, `+740h`/`+744h` retained point effects, `+748h` effect lifetime, `+750h` = `0.0f` roll clock. Eight vptrs, the eighth on the `+72Ch` subobject. |
| `MCommandBuilding` | `+758h` listener node (`008E35D0`, `006F5649`), `+764h` critical section (`00BD1860`, `006F572D`), `+770h`/`+774h`, `+778h`/`+77Ch`/`+780h` garrison slot array (stride `0x64`, occupant at `slot+14h`), `+784h`/`+788h`/`+78Ch` `std::list` (head node from `0059FD60`), `+790h`/`+794h`/`+798h`/`+79Ch`, `+7BCh` = `1.0f`, `+7C0h` = `-uniform(0,1)`, `+7D4h`, `+7D8h`, `+7DCh`/`+7E0h`/`+7E4h`. Ten vptrs; it re-vptrs the fort's `+72Ch` subobject to `00CFAFD8`, which proves `+72Ch` is an inherited base subobject and not a member. |
| `MAirfield` | `+72Ch` air-operations block (`006CAC00`, `006D1C59`), `+828h` collision node (`operator new(0x160)`, `006D1CAA`), `+82Ch` shape, `+830h`/`+834h`/`+838h` sub-object vector (stride `0xC`), `+83Ch..+883h` six inline vec3 slots with the count at `+884h`, direction/offset pairs at `+8A8h` and `+8CCh`. Eight vptrs. Not one field shares an offset with `MLandFort`: every `MAirfield` field is above `MLandFort`'s whole instance. |
| `MShipyard` | `+72Ch` (`00CF85D4` then `00D0B718`), and three vectors at `+770h`, `+780h` (stride `0x10`, the slot vector) and `+790h`. Eight vptrs. |

**`+72Ch` now holds three unrelated things.** `docs/AIR_OPERATIONS.md` cautions that the ship's
`+72Ch` is the owned-ref slot of `docs/UNIT_INSTANCE_SUBOBJECTS.md` while `MAirfield`'s is the
air-ops block. `MLandFort` is a third case: its `+72Ch` is built by the *same* `00809270` as the
ship's (call at `00745978`, against `0081ED7B` for the ship), so the ship and the fort share that
subobject and the airfield does not.

### The gun holder is not the unit

`docs/UNIT_GUNNERY_PASS.md` reads the gun holder through `unit->vtable[140h]()` and records the
base as `0047F320`, `MOV EAX,ECX; RET` -- the unit holds its own guns. Two of these classes
redirect it, and both redirect it to `+738h`:

| class | `vtable[140h]` | body |
| --- | --- | --- |
| base, `MAirfield`, `MShipyard` | `0047F320` | `MOV EAX,ECX; RET` |
| `MLandFort`, `MCommandBuilding` | `006F57A0` | `EAX = [ECX+738h]; if (!EAX) EAX = ECX; RET` -- the master fort, else itself |
| `MLandVehicle` | `0074CDA0` | `EAX = [ECX+738h]; RET` -- the convoy, with **no fallback** |

So a fort section's guns are served through its master fort and a convoy member's through its
convoy. A land vehicle outside a convoy returns a null holder to the gunnery pass.

## 2. Land motion: the convoy is the motion controller

The element interface is the six-slot contract of `docs/TICK_ELEMENT_OVERRIDES.md`, installed at
`instance+310h`.

| class | `+0h` | `+4h` restore+extrapolate | `+8h` per step | `+0Ch` commit | `+10h` fixed-step callback |
| --- | --- | --- | --- | --- | --- |
| ship `00CFC38C` | `006FE510` | `00811AB0` | `00825F20` | `006D1FC0` | `0042BBA0` |
| base `00D1A654` | `00959C10` | `0042BB70` | `00953CC0` | `006D1FC0` | `0042BBA0` |
| `MLandVehicle` | `0074DD80` | `0042BB70` | `00953CC0` | `006D1FC0` | `0042BBA0` |
| `MLandFort` | `00745A80` | `0042BB70` | `00953CC0` | `006D1FC0` | `0042BBA0` |
| `MCommandBuilding` | `006F5880` | `0042BB70` | `00953CC0` | `006D1FC0` | **`006F7360`** |
| `MAirfield` | `006D1DA0` | `0042BB70` | **`006D2510`** | `006D1FC0` | `0042BBA0` |
| `MShipyard` | `00846BD0` | `0042BB70` | **`00846320`** | `006D1FC0` | `0042BBA0` |
| `LandConvoy` | `004F2550` | **`00743060`** | **`007410C0`** | **`007410B0`** | `0042BBA0` |

`0042BB70` and `0042BBA0` are the base no-ops (`C2 04 00`). The three land *unit* classes
override nothing in the pose slots; the convoy overrides all three. That is the whole answer to
"how does a land vehicle move".

### Rule 2a: the convoy's arc along its path

`+344h` holds the path interface. Its producer is the convoy's deferred-reference pass
`007420B0` (vtable `+0A0h`): when the property bag at `+C0h` is in state 1 it reads the scene
key `"Path"` (`00CEA738`, referenced at `007420D6`), resolves it with
`BSP_EntityRegistry_FindEntityByName`, converts it with `BSP_Entity_PathInterfaceForKind` and
stores the result at `+344h`. The arc length is `[[convoy+344h]+28h]`.

| field | offset | meaning | producer |
| --- | --- | --- | --- |
| stop flag | `+3A8h` | byte; when non-zero the arc does not advance | `00743450` clears it |
| reverse | `+3A9h` | byte; selects the direction scale `-1.0f` (`00D7A260`) over `+1.0f` (`00D7A24C`) | scene key `"Reverse"` |
| speed | `+368h` | float, path units per second | scene key `"Speed"` |
| live arc | `+3ACh` | float, wrapped into `[0, L)` | scene key `"Offset"` seeds it |
| committed arc | `+3B0h` | float, the arc of the last completed step | `007410B0` |
| odometer | `+3B4h` | float, `+= |speed| * t`, never wrapped | zeroed at `00742C58` |

Slot `+4h` `00743060` (body `00743060`-`00743154`, `RET 4`): when `+3A8h` is clear, advance the
live arc by `t * speed * dir`, wrap it into `[0, L)` with **two loops** (add `L` while negative
at `007430DB`, subtract `L` while `>= L` at `007430FE`), add `t * |speed|` to the odometer, then
`ADD ECX,-310h` and call `00742400` with the convoy as `this`.

Slot `+8h` `007410C0`: when `+3A8h` is clear, `+3ACh = +3B0h + dt * speed * dir`. This is the
slot that restores the committed arc; `docs/TICK_ELEMENT_OVERRIDES.md` puts "restore the
committed pose" in `+4h` for the unit, and for the convoy the two roles sit the other way round.
Slot `+0Ch` `007410B0`: `+3B0h = +3ACh`.

So per frame: `+4h` extrapolates the live arc and re-places every member; `+0Ch` commits;
`+8h` re-derives the live arc from the committed one, discarding the interpolation drift.

### Rule 2b: the formation, `00742400`

`00742400` is `__fastcall(convoy)`, body `00742400`-`00742927`. The convoy is a
`Rows x Columns` block riding the path.

| field | offset | scene key |
| --- | --- | --- |
| rows (ranks along the path) | `+354h` | `"Rows"` (`00CFF344`) |
| columns (files across it) | `+358h` | `"Columns"` |
| row gap | `+35Ch` | `"RowGap"` |
| column gap | `+360h` | `"ColumnGap"` |
| authored HP | `+364h` | `"HP"` (`00CE6750`) |
| member vector | `+398h`/`+39Ch`/`+3A0h` | built by `00743450` |
| live member count | `+3BCh` | written once at `00743B54` |

The pass allocates a `Rows`-entry pointer array (`00742409`), walks
`i` over `[0, Rows * Columns)` (the loop bound `IMUL` at `00742458`), and for each `i` takes
`g = i / Columns` and `lane = i % Columns`:

* Once per `g`, build a `0x40`-byte matrix: group arc `= wrap(+3ACh + g * dir * RowGap, L)`,
  wrapped by a **single** conditional here (`00742581`-`007425A9`: add `L` once if negative,
  subtract `L` once if `>= L`), not the loop `00743060` uses;
  sample the path with `007B03C0(arc, matrix+30h, matrix+20h, 0)` at `007425B9`, which is
  `BSP_CameraPath_SampleLocalCubic` plus a direction transform, so `row3` is the position and
  `row2` the tangent; negate `row2` when reverse (`007425BE`); set `row1` from the global
  up-axis triple `00F8758C`..`00F87594`; orthonormalise with `0085DC80`.
* The lateral offset is `(lane - (Columns - 1) * 0.5) * ColumnGap` along `row0`
  (`00742622`..`00742644`, with `1.0` at `00D7A210` and `0.5` at `00D7A280` as doubles).
* Then one of two placement laws, selected by `+3C8h` at `0074265C`.

`+3C8h` is not an authored flag. `00742C10` (vtable `+0A4h`, no Ghidra function,
`00742C10`..`0074305C`, plain `RET`) sets it at `00742C70` to
`(pathEntity->+3Ch) != (convoy->+3Ch)`: **the path and the convoy have different scene
parents**. It also caches the convoy's own parent at `+3A4h` (`00742C81`).

| `+3C8h` | law | what it does |
| --- | --- | --- |
| `0` (same parent) | terrain snap, `007426E4`..`007428CE` | sample the parent's height field: `0087FA20(x, z)` and `0087FB90(x, z)`, both `[parent+3D0h]->vtable[24h/...]` forwarders, so the authored parent must be a `Landscape` (`44h`); rebuild the frame against the surface normal with `0085DAD0`; write the member's local matrix `+74h`..`+B0h` directly; clear `+C8h`/`+10Ch`; walk the child chain `+48h`/`+44h` with `BSP_SceneNode_InvalidateSubtreePose`; then `[member+4A4h]->vtable[38h](member+74h)` |
| non-zero | world pose, `00742662`..`007426DF` | `0085DAD0`, then `00741E90(member, matrix)`, the shared world-pose setter that `00929CB0 BSP_TickableGameEntity_PlaceStepPose` also uses; then the same `+4A4h` call |

There is **no leak** in the cleanup, contrary to what the decompiler shows. `ghidra disasm` drops
ten bytes after the `_free` at `00742908`; the raw bytes `0074290D`-`00742916` are
`ADD ESP,4` and `MOV [EDI+ESI*4],0`, so the loop frees every group matrix and then frees the
array at `00742922`. The decompiler's `return` inside that loop is the gap, not the code.

### Rule 2c: the roster, `00743450`

`00743450` (vtable `+09Ch`, body `00743450`-`00743B6F`) is the convoy's mission-attach pass. It
calls `BSP_MissionEntity_AttachLuaSelfWithResourceUsage`, clears `+3A8h`, reads the eight scene
keys above plus `"Reverse"`, `"Speed"` and `"Offset"`, fills a `Rows * Columns` slot map with
`-1`, and then walks two nested authored lists: `"Type" + n` (`Type1`, `Type2`, ... , built with
`BSP_NativeString_ConcatInt`, accepted only at property type tag 6) and, inside each,
`"Position" + n`.

For each occupied position it creates one member:

| step | native | what |
| --- | --- | --- |
| 1 | `BSP_VehicleClass_GetOrCreate` | resolve the authored type to a descriptor |
| 2 | `descriptor->vtable[28h](0)` | allocate the member instance -- the same slot the class table uses |
| 3 | `member->vtable[98h](convoy+3Ch, convoy+30h, convoy+74h)` | parent the member to the convoy's parent and seed its pose |
| 4 | `convoy->vtable[10h]()` + `00742A70` | build the member's name from the convoy's name and the index |
| 5 | `member+58h = convoy+58h`, `member+54h = convoy+54h` | inherit the party |
| 6 | `00922DE0(convoy+C0h)` | give the member its own property-bag reference at `+30h` |
| 7 | `member+738h = convoy`, `member+73Ch = slot index` | the back pointer |
| 8 | `00743350` | push the member into `+398h` |

`member+738h` is the field `docs/RECON_SLOT_LISTS.md` already reads from the other side
(`kReconLandVehicleConvoyBackPointerOffset`, consumer `00805680`); this packet supplies its
producer.

Death runs the same link backwards. A member's `vtable[080h]` `0074CD10` loads
`ECX = [member+738h]`, and when it is non-null calls `007422A0(convoy, member)` before
tail-jumping the base `00951FB0`. `007422A0` finds the member in `+398h`, nulls its slot,
decrements `+3BCh`, and when the count reaches zero calls `BSP_MissionEntity_Kill(2)` on the
convoy. The convoy's own `vtable[080h]` `00742180` runs it the other way: null every member's
`+738h`, then kill each member.

## 3. The shipyard launch, end to end

`MT_VEHICLE_SHIPYARD_LAUNCH` (`7Ah`) is built by `0080F960` (vptr `00D03310`, `+20h` = the byte
argument) and applied inside `BSP_UnitInstance_HandleMessage` at `0082226F`:
`NEG CL; SBB ECX,ECX; AND ECX,5; MOV [EDI+1130h],ECX`, i.e. `unit+1130h = byte ? 5 : 0`.

`+1130h` is a **ship** field, named `shipYardLaunch` by the property-reflection tables
(string side `00818653`, pointer side `lea edx,[esi+1130h]` at `0081FDB5`). Its readers settle
what `5` means: `0081DE31` compares it and returns immediately when it is positive, skipping the
ship motion body, and the two HUD control passes `0064B97D` and `0067C736` skip when it is
non-zero. There is no decrement anywhere, so `5` is a sentinel, "launching from a shipyard:
suppress player motion and controls", and `0` is normal.

The shipyard's own build pass is the creator `00844FC0` (body `00844FC0`-`0084559D`), reached
from the per-step tick `00846320` (spawn at `00846612`) and from `00846D90` (spawn at
`00846F89`, reached from `vtable[164h]` `00847030`). It builds a scene property bag
(`008F41A0` at `00845014`), sets `Type`, `Skill`, `Race`, `Party` (key `00CE5804`, set at
`0084507F` from the shipyard's own `+54h`), `OwnerPlayer`, `WingCount`, `VelocitySI`, `State`,
`Equipment`, `Dive`
and **`ShipYardLaunch`** (`008F3940` at `008450AA`), takes the launch pose from the chosen slot
and snaps it to the water with `BSP_GameWorld_SampleWaterHeight` at `008451FB`, then dispatches
by class to one of the six scene unit creators (`004F0AD0`, `004F0860`, `004F05F0`, `004F06C0`,
`004F0790`, `004F0520`), and finishes with `00922E20`, `BSP_Entity_IssueCommand 0077D600`,
`00694A60` and a session broadcast at `0084551B`.

That closes the loop: the shipyard writes `ShipYardLaunch` into the *new ship's* bag, and the
new ship's `+1130h` carries it. The `7Ah` message is the network/serialised form of the same
bit; its senders are three call sites, not two -- `0081386D`, `009CFC1B` and `00760511` (in
`FUN_007604C0`, which targets the local player's own unit slot from
`DAT_00E188A8+18CCh + idx*4`).

The launch spot is `vtable[100h]` `00844A10`: it walks the `+780h` slot vector (stride `0x10`,
free when `elem+4`'s `+5Ch` is set and `+5Dh`/`+5Eh`/`+60h` are clear), picks one with
`BSP_Random_UniformFloatRange`, and turns it into a world pose through `EntityPose_RefreshWorld`,
`BuildOrthogonalScaledAffineInverse`, `Multiply4x4` and `TransformAffinePoint`. The slots are
scene-authored, so the launch position is authored per shipyard and not derived from geometry.

## 4. The fort and airfield ticks

### `MLandFort`

The fort's element table inherits the per-step slot, so **nothing fort-specific runs in the
fixed-step pass**. Its work is in the per-frame update `vtable[0DCh]` `00745BE0`
(`00745BE0`-`00745F9F`, `RET 4`, one float): call the base `00956600`, tick the `+72Ch`
subobject with `00809C10(this+72Ch, dt)`, run two identical effect timers (`+748h -= dt`,
`+750h += dt`; at `+748h <= 0` stop the effects retained at `+740h`/`+744h`), and then, when the
fort is alive and `[+538h]+170h > 0` and `+750h > 0`, roll
`uniform(0,1) < ((maxHp - hp) / maxHp) * [desc+170h] * [00D7A358]` at `00745D47`. On success it
picks a random entry of `[+538h]+150h`, spawns a point effect, picks a random row of `[desc+28h]`
(stride 12) for `00745130`, reloads `+750h` and plays a sound with `0077C2A0(name, 4, 0)`.

That is a damage-effect pass, not repair. **No health regeneration and no crew field exists in
any routine this packet read on `MLandFort`** -- coverage note: the interior of `vtable[0A0h]`
`007482B0` (`007482D0`-`00748986`) was not read.

The destruction rule is `vtable[1A8h]` `00747400` (`00747400`-`007474DB`). Already dead: return.
Has a parent at `+71Ch`: play the named effect `"InferiorFailure"` (`00CF0B74`) through
`vtable[194h]` and **do not die** -- a section of a bigger fort cannot die on its own. Root fort
with `+50h` set and no already-dead ancestor in the `+48h`/`+44h` chain: `vtable[1B4h]` then
`vtable[70h](1)`. Parts go through the base `00958A30`, which `docs/UNIT_PARTS.md` shows reading
`vtable[204h]` and the *owner's* descriptor.

The fort's three added slots are one interface, "play or retain a numbered damage effect":
`+204h` `006F57F0` (no Ghidra function, `006F57F0`..`006F580A`) forwards `IsKindOf` to `+738h`;
`+208h` `007460E0` and `+20Ch` `00745FA0` create point effects from the descriptor lists
`[+538h]+144h/+148h` and `+154h/+158h`. Their only in-class caller is `vtable[164h]` `00744BE0`
on session message `D1h`.

### `MAirfield`

The airfield **does** override the per-step slot: `006D2510` (no Ghidra function,
`006D2510`..`006D2553`, `RET 4`, `ECX = unit+310h`). Per step it calls the inherited base advance
`00953CC0(dt)`, then `unit->vtable[1A8h]()` = `006D40F0`, and then, only when `byte[unit+5Dh]`
is clear, `006CDC70(unit+72Ch, dt)` -- the air-operations block update of
`docs/AIR_OPERATIONS.md`. So the airfield's per-step work is exactly "advance the base, run the
destruction/state slot, then run air operations".

Beyond the air-ops block the airfield owns a hangar list, not runway geometry. `vtable[0A0h]`
`006D3C10` reads the scene keys `"RunwayWidth"` (`00CF8AE8`) and `"RunwayLength"` (`00CF8AD8`)
and hands both to `006BF0D0`, which writes them into the air-ops block at `block+B0h`/`+B4h`;
`vtable[0A4h]` `006D5220` (`006D5220`-`006D600E`) resolves the marker entities
`"AirBaseLookAtPos"`, `"AirBaseCameraPos"` and `"CaptainCameraPos"` and walks `"Hangar %d"`
(`00CF8F08`) sub-bags for `"Object"`, `"EntryPath"` (`00CF8EF4`) and `"ExitPath"` (`00CF8EE8`)
into the `0xC`-stride vector at `+830h`. `vtable[100h]` `006D3250` picks a random index over
`+884h` and returns the inline vec3 at `+83Ch + i*0xCh`, a park-slot picker.

**The plane does not call an airfield accessor for its runway.** `006BF0D0`'s only callers are
`006D3C10` and `006D5220`, both airfield methods, and `docs/PLANE_GROUND_OPS.md` has the plane
reach its surface through `(unit+BF4h)->+4h->+3Ch->vtable[28h](unit)`, a held launch-spot object.
The `EntryPath`/`ExitPath` list is a plausible producer for the taxi paths of ground state `5`
("Runway on Path") but the consumer side was not traced; that link is open.

The airfield's two added slots `+204h` `006D0D50` (`..006D0D8D`) and `+208h` `006D0D90`
(`..006D0DCD`) are a two-arm event handler pair over the byte flags `+748h`/`+749h`
(`+204h` tests ids `72h` then `74h`, `+208h` tests `73h` then `75h`). They are **not** the same
interface as `MLandFort`'s added trio: different arity and different fields.

### `MCommandBuilding` and the `+10h` slot

`MCommandBuilding` is the only class in this packet with a fixed-step callback. Its leaf
constructor registers the element node at `006F5767` with
`BSP_FixedStepCallback_Register 00875A80(this+310h, repeating = 1)`, and
`00874DE0 BSP_FixedStepCallbackList_Run` invokes `node->vtable[10h](0.05f)`. In the element table
`00CFAFE0` slot `+10h` is `006F7360` (no Ghidra function, `006F7360`..`006F7665`, `RET 4`) where
every other table in the image carries the no-op `0042BBA0`. That makes `+10h` the
fixed-step-callback entry of the element interface, distinct from the three wave slots.

`006F7360` gates on `[00E188A8]+1FE4h != 2` and `+7D4h`, scans the `+778h` garrison slot array
for any occupant with `short[unit+174h] == 0`, and when all are complete calls `006F4110`,
routes a session message and calls `006F4300`. It then runs either the capture countdown
(`+7C0h`, reloaded from `+7BCh`, when the party at `+54h` is 2) or the level clock
(`+774h += step`; at `+774h > (float)+76Ch` with `+770h < 3` it routes message `D4h` with
`Level + 1`), and finally decays `+7D8h` toward zero. `+7C0h`'s constructor seed of
`-uniform(0, 1)` is a phase stagger: every command building starts its capture countdown at a
different offset so they do not all fire on the same step.

The critical section at `+764h` is entered and left in seven paired regions
(`006F1EE1`/`006F1F00`, `006F1F39`/`006F1F59`, `006F2A6A`/`006F2BAA`, `006F2DE4`/`006F2E3E`,
`006F2FCD`/`006F2FF3`, `006F34B4`/`006F34D5`, `006F39B4`/`006F3A03`) because the fixed-step pass
and the frame update both touch the slot list. Its death slot `vtable[1A8h]` `006F1F80` is a
single `RET`: a command building does not die. Ownership changes instead, through `vtable[1B0h]`
`006F3270`, which records the new party at `+7B0h`/`+7B4h` and tail-jumps `006F2940`.
`vtable[218h]` `006F5960` returns `CaptureRange * CaptureRange`; `vtable[0A0h]` `006F2780` reads
`CaptureRange`, `CaptureValue`, `LevelUpSeconds`, `Level`, `MinShootingRange` (`+7D0h`),
`InferiorRange` (`+7CCh`), `LandingRange` and `LandingPointRange`.

## 5. What the scene creators read

| creator | class | scene keys in the body | shape |
| --- | --- | --- | --- |
| `004F0930` `AirField` | `45h` | `"Type"` only (`00CE4780` at `004F0938`) | byte-for-byte the shared creator `004F0520`, retargeted: both bodies are `0xC1` bytes and differ only in the low half of seven `rel32` displacements |
| `004F0A00` `Shipyard` | `46h` | `"Type"` only | the same shared creator |
| `004F0FB0` `LandFort` | `1Bh` | `"Stationary"` (through `0048E9F0`) and `"Type"` | the one creator that branches: when `Stationary` is set **and** the found property's byte at `+0Ch` is non-zero it takes the instance from `00748C40(descriptor)` at `004F0FFE` instead of `descriptor->vtable[28h](0)` at `004F1016`, and it never calls `004E6B30`. `00748C40` (`..00748CB5`) allocates a different, `0x1AC`-byte class through `00748A40` (vptrs `00CFF678`/`00CFF65C`/`00CFF654`) and calls `0084F7F0`. So `Stationary` swaps the `0x758` fort for a lightweight static prop -- which is how 72,615 authored `LandFort` entities, the highest count of any scene class, stay affordable |
| `004F2700` `LandConvoy` | `1Ah` | none; it takes the entity name as an argument and copies it into `+154h`/`+158h` | not a unit creator: no `009553D0`, no `+354h` descriptor write. The convoy's properties are read later, by `00743450` and `007420B0` |

The two stock registrars are per-party hidden-unit walks:

* `004E9900 BSP_SceneAirField_RegisterStock`: `004E96D0` then a tail jump to `006BCF40`, which
  reads `"Party"` and then `"PlaneStock %d"` for `d` in `1..12`, each requiring property type tag
  6 and a non-null value, re-reads `"Type"` per entry and calls
  `BSP_VehicleClass_MarkPartyRequired` and `BSP_SceneEntity_RegisterHiddenStockUnit`.
* `004E9910 BSP_SceneShipyard_RegisterStock`: a 16-byte thunk (`004E9910`-`004E991F`) into
  `008433C0` (`..00843455`), which reads `"Party"` at `008433C7` and then `"Stock 1"`..`"Stock
  20"` at `008433E5`, with the same type-tag-6 filter and the same two registration calls.

The descriptor side adds nothing for either structure. `0095FF50` is exactly
`BSP_VehicleClass_ConstructBase 00749050` plus two vptr writes (`00D1A9DC`, and `00D1A9D8` at
`+6Ch`), so a `Shipyard` row in `vehicleclasses.lua` can carry only base VehicleClass fields;
everything shipyard-specific is authored in the scene bag (`Party`, `Stock %d`, `Type`,
`Hidden`), not in the class row. `0095FEE0` is the same shape for `AirField`.

## Corrections to earlier documents

| document | was | is | evidence |
| --- | --- | --- | --- |
| `docs/TICK_ELEMENT_OVERRIDES.md` | slot `+10h`: "no override in any of the seven tables", wave "none" | `MCommandBuilding`'s element table `00CFAFE0` overrides `+10h` with `006F7360`, and `+10h` is the slot the fixed-step callback list invokes | `00CFAFE0` bytes `60 73 6f 00` at `+10h`; `00874DE0` calls `(*(code **)(*node + 0x10))(DAT_00D0DE84)`; the registration is `00875A80` at `006F5767` |
| `docs/SCENE_ENTITY_FACTORY.md` | the multiplayer stock walk `0046BF70` reads `NumSlots`, `Slot %d`, `Stock %d` and `AlliedList` for `Shipyard` | the `Shipyard` branch reads only `Stock %d`, `Type` and `Hidden`; `NumSlots`/`Slot %d` belong to the `AirField`/`MotherShipGen` branch and `AlliedList`/`JapanList` to the `SpawnPoint` branch | branch string refs: `Shipyard` `0046C1D7` with `0046C215`, `0046C247`, `0046C259`, `0046C268`; `AirField` `0046BFB8` with `0046C0E9`, `0046C10A`, `0046C065`; `SpawnPoint` `0046C2DA` with `0046C3A6`, `0046C30C` |
| `docs/UNIT_MESSAGE_ARMS.md` | `7Ah` senders `00813830`, `009CFBA4` | three senders: `0081386D`, `009CFC1B` and `00760511` in `FUN_007604C0` | all four refs to the arm vptr `00D03310`; `FUN_007604C0` sets the message target from `DAT_00E188A8+18CCh + idx*4` |
| `docs/AIR_OPERATIONS.md` | the `+72Ch` caution names two cases, the ship's owned-ref slot and the airfield's air-ops block | three cases: `MLandFort`'s `+72Ch` is built by the same `00809270` as the ship's | `00745978` with `ECX = this+72Ch` against `0081ED7B` for the ship |

## Open questions

* Who consumes the airfield's `"EntryPath"`/`"ExitPath"` hangar list. It is the obvious producer
  for the plane's `Runway on Path` taxi state but the consumer was not found.
* Whether a `LandConvoy` is ever authored under a parent that is not a `Landscape`. The terrain
  branch reaches the height field through `parent+3D0h` with no class test, so a non-landscape
  parent would fault.
* `MLandFort` `vtable[0A0h]` `007482B0` interior (`007482D0`-`00748986`), which is where a repair
  or crew rule would live if one exists.
* `MShipyard` `vtable[0A0h]` `00849A30` past `00849AB3`, and `vtable[0A4h]` `00849F70`.
* Which of the shipyard's three vectors (`+770h`, `+780h`, `+790h`) is the stock list and which
  the hangar list; only `+780h` is proven to be the slot vector.
* `MAirfield`'s `+748h`/`+749h`: which is the runway failure flag and which the hangar failure
  flag, and whether class ids `72h`-`75h` are runway and hangar parts.
