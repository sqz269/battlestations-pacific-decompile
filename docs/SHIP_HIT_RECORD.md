# The ship hit-record handler, and who owns the three `vtable[ECh]` bodies

Addresses: 00826F10 007BBCF0 0042BAF0 008777D0 0092D1F0 0092CED0 0080FA50 0080FF80 0080FFD0
004155B0 00BD2F10 0064B170 009553D0 00C32000

Packet `cc2_ship_hit_record`, read-only in Ghidra. Every descriptive name below is a hypothesis,
not a recovered symbol. docs/UNIT_HIT_PATH.md's two damage formulas and its hit-record field table
are cited, not restated; docs/UNIT_FIRE_AND_REPAIR.md's `0093BED0` is cited, not restated.

## 1. The headline: the three handlers are one class hierarchy, not three classes

`vtable[ECh]` is the hit handler docs/PROJECTILE_IMPACT.md's dispatcher `009239A0` calls in a loop
at `00923A70`. Scanning `.rdata` for each handler address and subtracting `ECh` lands every hit
exactly on a vtable base that docs/ENTITY_CLASS_IDS.md already names, so the owners are settled:

| handler | vtables | class ids | the family |
| --- | --- | --- | --- |
| `0042BAF0` `BSP_Entity_HitNotHandledStub` | 54 | the root `00` (`00D19120`, `00D1920C`), `02` (`00D03E80`), `03` (`00D11138`) and 50 more | everything that is not a unit: projectiles, effects, scene nodes |
| `008777D0` `BSP_UnitInstance_ApplyHitRecord` | 7 | introduced at `04` (`00D0DF70`), inherited by `05` (`00D1A698`), `19` `MLandVehicle`, `1B` `MLandFort`, `1C` `MCommandBuilding`, `45` `MAirfield`, `46` `MShipyard` | land units and structures |
| `00826F10` | 9 | introduced at `06` (`00D09678`), inherited by `07` `MDestroyer`, `08` `MSubmarine`, `09` `MMothership`, `0A` `MCruiser`, `0B` `MCargo`, `0C` `MLandingShip`, `0D` `MBattleship`, `0E` `MTorpedoBoat` | **the ship family** |
| `007BBCF0` | 9 | introduced at `0F` (`00D05F20`), inherited by `10`..`13` the bombers and fighter, `14`..`16` the recon planes, `17` `MPlaneKamikaze` | **the plane family** |

Method: `python tools/bsp.py scan-bytes "<addr little-endian>" --section .rdata`. Every hit minus
`ECh` is a known vtable base and every known base appears once, so no hit is a stray immediate.
`python tools/bsp.py ghidra xrefs 00826F10` returns exactly the same nine addresses, all `[DATA]`,
and no code reference: the handler is reached only virtually.

The class ids line up with docs/UNIT_INSTANCE_LAYOUT.md's constructor chain, which installs
`00D0DF70` at level 3 (`0087B670`), `00D1A698` at level 4 (`0095CC90`), `00D09678` at level 5
(`0081ED40`) and `00CFC3D0` at level 6 (`006FE460`). So the three bodies are a base and two
overrides on the same single-inheritance spine:

```
class 04                008777D0   introduces the handler
  class 05                "        land units and structures keep it
    class 06            00826F10   ships override it, then call it
    class 0F            007BBCF0   planes override it
```

### The corrected chain

`00826F10` ends with `PUSH ESI / MOV ECX,EDI / CALL 0x008777D0` at `008277F9`, so for a ship the
base handler is **called by the override**, not reached from `007BBCF0`. For a plane, `007BBCF0`
calls `008777D0` at `007BBDB1`, which docs/UNIT_HIT_PATH.md already recorded. Both overrides are
pre-passes: every side effect runs first and the shared damage formula runs last.

```
009239A0 step 5  -->  vtable[ECh]
                        ship    00826F10  --> 008777D0 --> vtable[1ACh] --> 0095DA00 --> 0087D730 --> 00879070
                        plane   007BBCF0  --> 008777D0 --> (the same tail)
                        other   0042BAF0  --> XOR AL,AL; RET 4, and the dispatcher walks entity+3Ch
```

`0042BAF0` returning `AL = 0` is what makes the dispatcher's `while` loop walk up the parent chain,
so the stub is not a no-op: it is the "ask my parent" answer.

## 2. `00826F10`'s rule table

`__thiscall bool (ShipEntity* this /*ECX, kept in EDI*/, HitRecord* hit /*[ESP+4], kept in ESI*/)`,
`RET 4` at `00827819`, body `00826F10`-`0082781B`, SEH frame (handler `00C918C8`, state dword at
`[ESP+0D0h]` cycling `-1, 0, 1, 2, 3, 4` for the five routed messages). `coverage: complete`; the
listing is contiguous with no gaps (624 instructions, `00826F10`..`00827819`).

The return value is `008777D0`'s (`1`) on every path but the veto, where `AL` is still the `0` the
class test left.

| # | site | rule |
| --- | --- | --- |
| R0 | `00826F38`..`00826F59` | **Depth-charge veto.** When `hit+4h` is non-null and the shot passes `vtable[5Ch](2Ch)` `MDepthCharge` and the hull fails `vtable[5Ch](8)` `MSubmarine`, return `0` at once. A depth charge only hurts a submarine; against anything else the dispatcher retries on the parent. |
| R1 | `00826F62` | `hit+34h == -1` jumps straight to the part loop (R11). The hull pass is skipped whole. |
| R2 | `00826F6B`..`00826F93` | **Armour source.** `hit+0Ch >= 0` takes `this+368h`; a negative selector calls `[this+538h]->vtable[24h]()`. |
| R3 | `00826FA3` | `hullDamage = 00470510(hit, armour)`, the formula in docs/UNIT_HIT_PATH.md. |
| R4 | `00826FAC`..`0082703E` | **Hull-segment part damage.** Gated on `hit+30h == 0Dh`, `hullDamage > 0` and `Mass >= 100.0` (`00D7A220`). Direction is `shot->vtable[34h](&buf)` or `{0,0,0}`; call `0092D1F0(this+1018h, hit+34h, &dir, hullDamage)`. This uses the **unscaled** damage. |
| R5 | `00827043`..`008270B7` | **Difficulty scaling.** `hullDamage` is recomputed from scratch, then: `game+1FE4h != 0` picks row `2`; otherwise `this+54h == [game+18CCh + game+18ECh*4]+28h` picks row `game+6ACh`; otherwise nothing is scaled. The row indexes the float vector at `00432650()+20h`/`+24h`, bounds-checked into `00BF6713`. |
| R6 | `008270BE` | `this+10D0h = 0.0f`. `contract: unread` (the field's consumer is outside this packet; `10D4h` is docs/UNIT_FIRE_AND_REPAIR.md's leak manager, so `10D0h` sits just below it). |
| R7 | `008270C6` | Everything to `0082742B` needs `hit+4h`. `weapon = shot->vtable[108h]()` is taken here even when the branches below are skipped. |
| R7a | `008270E5`..`00827339` | **Roll torque.** Gated on `vtable[5Ch](2Bh)` `MTorpedo`, `Mass > 500.0` (`00CE3840`) and `game+1FE4h` in `{0,1}`. See the formula below. Routes message `93h`. |
| R7b | `0082733C`..`0082738E` | **Flooding, every hit.** `hit+50h = weapon->vtable[10h]()`, then `0080FA50(&msg, 1, weapon->vtable[10h](), 1)` routed. The virtual is called twice. |
| R7c | `0082738E`..`0082742B` | **Fire, on a roll.** `chance = hullDamage > 0 ? weapon->vtable[18h]() : 0.0f`; `draw = 00BD2F10(0.0f, 1.0f)`; on `chance > draw`, `hit+4Ch = weapon->vtable[14h]()` and `0080FA50(&msg, 0, weapon->vtable[14h](), 1)` routed. |
| R8 | `00827432`..`00827450` | `hullDamage > 0.0f` runs `0093BED0(this+A20h, hit, hullDamage)`, the component-failure roll of docs/UNIT_FIRE_AND_REPAIR.md. |
| R9 | `00827455`..`00827497` | **The local player's damage arrow.** Only when `this == [00E188D8]` and `004704E0(hit)` is non-null: refresh the shooter's pose (`00414DB0`) when `owner+C8h` is clear, then `0064B170([[00E198C4]+78h]+18Ch, owner+FCh)`. |
| R10 | `0082749C`..`0082757B` | **Hull impact effect.** `hullDamage > 0.0f` and the hull fails `vtable[5Ch](8)`: `n = trunc(clamp(hullDamage / 10.0, 0, 63))`; on `n != 0`, transform `hit+8h..10h` by `00414E10(this)` and route message `90h` built by `0080FF80(&msg, n, &local)`. A submarine shows none. |
| R11 | `00827582`..`008277F3` | **The part loop**, `for (i = 0, off = 0; i < hit+40h; ++i, off += 10h)`. |
| R11a | `008275A0`..`008275BD` | `partDamage = 004705C0(hit, this+368h, i)`. The part pass never takes the descriptor's virtual armour, only the instance's own. |
| R11b | `008275C1`..`0082765B` | When `entry+0h == 0Dh`: `partDamage <= 0` ends the iteration; otherwise `Mass >= 100.0` calls `0092D1F0(this+1018h, entry+4h, &dir, partDamage)`. A mass below the floor falls through to R11c. |
| R11c | `00827663`..`008277C1` | Same effect rule as R10 with `partDamage`, but through a **cached** inverse: when `this+10Ch` is clear, refresh the pose, set the flag and build `this+110h` from `this+CCh` with `00B63D50`. The `90h` message is built inline (`0075B430(90h)`, vtable `00D034A0`, `+4h=1`, `+1Ch=n`, `+20h..28h` the local point) instead of through `0080FF80`, and the vtable is restored to `00CE4974` after the route. |
| R12 | `008277F9` | `008777D0(this, hit)` and return its result. |

### The roll-torque formula (`00827126`..`00827339`)

```
raw   = 00470510(hit, 0.0f)                       ; hull damage with no armour at all
d     = shot->vtable[34h](&buf)                   ; the impact direction, three floats
len   = sqrtf(d.x*d.x + d.y*d.y + d.z*d.z)        ; 00BF7030
dx    = d.x / len ;  dz = d.z / len               ; d.y is never read again
axis  = [[this+1018h]+2Ch] + 20h                  ; three floats, reached through 00C32000
sign  = sgn(axis.z*dx - axis.x*dz)                ; -1, 0 or +1; exactly zero kills the torque
mass  = [this+538h]+B0h                           ; the class Mass
term  = (mass == 0) ? 0 : powf(|mass|, 1 / settings+594h)    ; FYL2X / F2XM1 / FSCALE
scale = sign * settings+590h * raw * term         ; settings is 00424C40
0080FFD0(&msg, axis.x*scale, axis.y*scale, axis.z*scale) ; message 93h, then 0077C2A0(this,&msg,7,0)
```

`|mass|` is formed as `-0.0f - mass` (the `00D7A208` literal), not a negation. The sign comes from
the horizontal cross product of the hull axis and the impact direction, so it says which side the
torpedo struck; the torque is applied **about the hull's own axis**, which is a roll.

### The constants

| address | type | value | read at |
| --- | --- | --- | --- |
| `00D7A218` | float | `0.0f` | the `> 0` tests at `00827438`, `008274A2` |
| `00D7A220` | double | `100.0` | the `Mass` floor for part damage, `00826FCA`, `008275E3` |
| `00CE3840` | double | `500.0` | the `Mass` floor for roll torque, `008270F7` |
| `00CE3DC0` | double | `10.0` | damage per effect unit, `008274CE`, `00827689` |
| `00D099A0` / `00D099A8` | double / float | `63.0` | the effect-count ceiling, `008276A4` / `008274C6` |
| `00D19620` | float | `-10000.0f` | the destroyed-part sentinel inside `0092D1F0` |

## 3. The two thresholds are the class `Mass`

`this+538h` was already settled: `009553D0` `BSP_Unit_SetVehicleClass` is its ref-counted setter,
recorded by docs/SCENE_UNIT_CREATORS.md and docs/UNIT_INSTANCE_LAYOUT.md, and `this+354h` is the
non-owning back-pointer `006FE590` writes at `006FE5FC` to the same descriptor. This packet adds
only which field of it the handler reads.

docs/VEHICLE_CLASS_FIELDS.md attributes descriptor `+B0h` to the Lua key `Mass` (default `1.0f`,
written at `0096043A`), which makes the two thresholds readable:

* a hull lighter than `100.0` never loses individual parts to a hit — `0092D1F0` is skipped in
  both R4 and R11b, so all its damage lands on the shared health pool;
* only a hull heavier than `500.0` can be rolled by a torpedo;
* a stock `Mass` of `1.0f` fails both, so a class row that forgets `Mass` gets neither.

The same field is the base of the torque's `pow(|Mass|, 1 / settings+594h)` term, so `Mass` sets
both the gate and the scale.

## 4. The callees this packet read

| address | ABI | what it does | coverage |
| --- | --- | --- | --- |
| `0092D1F0` | `__thiscall(parts, uint index, const float dir[3], float damage)`, `RET 0Ch` | subtracts `damage` from the part-health vector at `parts+310h`/`+314h`; requires `parts+34Ch+index` to be a non-negative signed byte and the slot above `-10000.0f`; on reaching zero it parks `-10000.0f` in the slot and routes message `99h` built by `0092CED0` through `[parts+1Ch]`. **The `dir` argument is never read** — three arguments on the cleanup, two used. | complete |
| `0092CED0` | `__thiscall(msg, int partIndex)`, `RET 4` | message `99h`, vtable `00D033EC`, `+1Ch = 0`, `+20h = partIndex` | complete |
| `0080FA50` | `__thiscall(msg, int channel, float amount, char flag)`, `RET 0Ch` | message `9Eh`, vtable `00D0334C`, `+1Ch = amount`, `+20h = flag`, `+24h = channel` | complete |
| `0080FF80` | `__thiscall(msg, int count, const float point[3])`, `RET 8` | message `90h`, vtable `00D034A0`, `+1Ch = count`, `+20h..28h = point` | complete |
| `0080FFD0` | `__thiscall(msg, float x, float y, float z)`, `RET 0Ch` | message `93h`, vtable `00D034C8`, `+1Ch..24h = the torque` | complete |
| `004155B0` | `__fastcall(const float* v /*ECX*/, const float* lo /*EDX*/, const float* hi /*stack*/)`, `RET 4`, returns `ST0` | `max(lo, min(v, hi))` | complete |
| `00BD2F10` | `(float lo, float hi)`, `RET 8`, returns `ST0` | forwards both to `00BD2ED0` (with the caller's `ECX`, `1` at this site) and calls `00BD2E60` on the result: a uniform draw. The two callees are `contract: unread`. | partial |
| `0064B170` | `__thiscall(widget, const float worldPoint[3])`, `RET 4` | takes the camera at `game+19FCh`, transforms the point into its frame (`00B6E0D0`, `004142E0`, `0042B260`), reduces it to a bearing (`0042CF10`, `00BF701A`) and stores `[00CE380C]` into one of `widget+14h`, `+18h`, `+1Ch` by sector: a directional damage indicator | partial, the sector arithmetic `0064B1BE`..`0064B28F` is read for shape only |
| `00C32000` | `LEA EAX,[ECX+8]; RET` | a four-byte accessor, **not** a dynamic physics import despite the `DYN_physics_00c32000` name and `block_dyn` tag | complete |
| `009553D0` | `__thiscall(unit, descriptor)` | `BSP_Unit_SetVehicleClass`, already in the ledger: the ref-counted `unit+538h` setter. Read here only to confirm which pointer the handler dereferences; no name added | already recorded |

`00470510`, `004705C0`, `004704E0`, `00414DB0`, `00414E10`, `004142E0`, `00B63D50`, `0075B430`,
`0077C2A0`, `00432650`, `00424C40`, `0093BED0` and `008777D0` are `contract: as documented
elsewhere` and were not re-read beyond the argument setup at these sites.

## 5. A hit ignites fire and starts flooding with no script involved

docs/UNIT_FIRE_AND_REPAIR.md closes with "**No in-engine producer of the fire or water timers was
found in this packet** ... the Lua path writes them through session message `9Eh`, whose handler is
`contract: unread`. Whether a hit can ignite a fire without script is an open question."

It can. Message `9Eh` has exactly three constructors in the image
(`python tools/bsp.py scan-bytes "68 9e 00 00 00" --section .text`):

| site | function | `message+24h` |
| --- | --- | --- |
| `0080FA51` | `0080FA50`, called only from `00826F10` (twice) and `00827B90` | the caller's `channel` argument |
| `0088E4AC` | `0088E320` `BSP_LuaBinding_SetFireDamage` | `EBP`, set by `XOR EBP,EBP` at `0088E343` — **`0`** |
| `0088E919` | `0088E790` `BSP_LuaBinding_SetWaterDamage` | `EBP`, set by `MOV EBP,1` at `0088E7AF` — **`1`** |

All three write the same vtable `00D0334C` and the same field layout. So `+24h` is the channel,
`0` is fire and `1` is water, and the ship hit handler is an in-engine producer of both:

* **flooding on every hit that has a weapon** (R7b), at the rate `weapon->vtable[10h]()`;
* **fire on a roll** (R7c), with the chance from `weapon->vtable[18h]()` and the rate from
  `weapon->vtable[14h]()`, and only when the hit got through the armour.

The `9Eh` handler itself is still `contract: unread`, so what it does with the float is the Lua
path's business as before; this packet proves only that the hit path sends the identical message.
`weapon` is `shot->vtable[108h]()`, whose concrete class is `contract: unread`: the three float
getters are named here by the channel their result ends up in, not by a body that was read.

## 6. What docs/UNIT_HIT_PATH.md already covers, and what is new

Already covered and not restated: the two damage formulas `00470510` / `004705C0` and their armour
and falloff arithmetic; the hit-record field table for `+04h`, `+0Ch`, `+14h`, `+24h`, `+28h`,
`+2Ch`, `+34h`, `+38h`, `+3Ch`, `+40h`, `+48h`; the `vtable[1ACh]` -> `0095DA00` -> `0087D730` ->
`00879070` tail; the `00879810` correction; `0087BCC0`'s health block; the repair tick.

New here, and ship-only: the depth-charge veto; the difficulty table; the roll torque; the
per-part health subtraction and its `Mass` floor; the fire and flooding messages; the damage
arrow; the two impact-effect paths and their different inverse-matrix strategies; the record
fields `+08h`..`+10h` (the impact point), `+30h` (the hull segment kind), `+4Ch` and `+50h` (the
fire and flood rates the handler **writes**); `unit+538h`, `unit+10Ch`/`+110h`, `unit+10D0h`.

## 7. Corrections to earlier documents

1. **docs/UNIT_HIT_PATH.md**, the chain table's level 0: `007BBCF0` is described as the unit
   instance's handler with an unread producer. It is the **plane** family's override (nine vtables,
   class ids `0F`..`17`), and `00CFC3D0`, the vtable docs/UNIT_INSTANCE_LAYOUT.md gives for
   `MDestroyer`, carries `00826F10` at `+ECh`, not `007BBCF0`. A shell hit on a destroyer never
   enters `007BBCF0`. Evidence: the `.rdata` scan in section 1 and `ghidra xrefs 00826F10`.
2. **docs/UNIT_HIT_PATH.md**, level 1: `008777D0` is reached from `00826F10`'s tail at `008277F9`
   for a ship and from `007BBDB1` for a plane, so it is not a single-producer step.
3. **docs/UNIT_HIT_PATH.md**, the hit-record table's `+0Ch` row: "negative selects the
   `[unit+354h]->vtable[24h]()` armour source" omits that `008777D0` also gates that branch on
   `this->vtable[5Ch](6)` at `008777ED`, i.e. on the entity being class `06`-derived. In
   `00826F10` the same selection reads `[this+538h]`, the other pointer to the same descriptor,
   with no class test.
4. **docs/UNIT_FIRE_AND_REPAIR.md**, the `0093BED0` call-site block: "the call site `00827450` has
   no Ghidra function; the enclosing candidate is `00826D70`". Ghidra now has `00826F10`
   (`00826F10`-`0082781B`) and the site is inside it.
5. **docs/UNIT_FIRE_AND_REPAIR.md**, the open question quoted in section 5 above, is answered.
6. **docs/UNIT_FORCE_CHANNEL.md**: "`0080FFD0` is the local constructor for the message class
   whose vtable is `00D034C8`. **It has no callers in the image.** ... nothing in this image
   produces such a message locally." `00826F10` produces one: `0080FFD0` at `00827312`, routed by
   `0077C2A0(this, &msg, 7, 0)` at `00827329`. The producer is the torpedo roll torque of R7a, so
   a hull's force and torque do **not** come only from `009329C0`'s hydrodynamic model and
   `00937440`'s rudder term. The earlier reading was taken before Ghidra had a function at
   `00826F10`, so the call site was inside unrecognised bytes and the caller scan could not see it.
7. **The ledger record for `00C32000`** (`DYN_physics_00c32000`, tag `block_dyn`) describes a
   dynamic physics import. The four bytes on disk are `8D 41 08 C3` — `LEA EAX,[ECX+8]; RET`.
8. **The ledger tag on `0092D1F0`** is `stl_probable`. The body is the per-part health
   subtraction; only its bounds checks look like container code.
9. **Scope**, not a correction: `0077CE60` is not called by `00826F10`. It is step 7 of the
   dispatcher `009239A0` in docs/PROJECTILE_IMPACT.md, which runs after the handler returns.

## 8. Open questions

* The `9Eh` handler. Until it is read, "the hit path sets the fire and water damage" is a claim
  about the message, not about `repairTask+34h`/`+38h`.
* `weapon = shot->vtable[108h]()`: the concrete class, and whether `vtable[10h]`/`[14h]`/`[18h]`
  are plain getters. Calling each twice in R7b/R7c is only safe if they are.
* `this+10D0h`, cleared on every hull hit and never read here.
* `[[this+1018h]+2Ch]+20h`, the roll axis. The cross product only makes sense for a horizontal
  hull axis, but the producer of that field was not read.
* `00827B90`, the other caller of `0080FA50`, was not read.
* No run-time evidence: rule 6 of docs/WORKER_VERIFICATION_CHECKLIST.md does not apply, because
  `bsp_game.exe` has no host for a hit record.

## 9. Reconstruction

`include/bsp/ship_hit_record.hpp` and `src/ship_hit_record.cpp`. The pure rules are the veto, the
`004155B0` clamp, the effect-count truncation, the difficulty row and multiply, the roll-direction
sign, the roll torque and `0092D1F0`'s subtraction. `ShipHitRecordHost` has one method per native
call site and `apply_ship_hit_record_00826f10` runs R0..R12 in order. The two damage formulas come
from `bsp/unit_hit_path.hpp` unchanged; `HitRecord` and `HitPartEntry` are reused from there and
`ShipHitRecordView` carries only the fields this handler adds. Build: Win32, warnings as errors,
clean; the existing `reconstructed_math` test passes and no test was added.
