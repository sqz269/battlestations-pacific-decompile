# The ship AI navigation block's constructor, and the five fields the arm tail reads

Addresses: 009E4330, 0082E960, 009DFCB0, 009E44CF, 009E4537, 009E453F, 009E4568, 009E45A9, 009E4659

Packet `cc_ai_nav_block_ctor`. Read-only analysis of `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. No Ghidra mutation. Reconstructed in
`include/bsp/ship_ai_nav_block_ctor.hpp` and `src/ship_ai_nav_block_ctor.cpp`;
evidence in `reports/ship_ai_nav_block_ctor.json`. Every descriptive name is a
hypothesis, not a recovered symbol.

The pseudocode of `009E4330` cannot carry this reading on its own: five of its
six calls take register inputs the decompiler drops, and every float is x87. The
stored Ghidra listing was read instruction by instruction over
`009E4330-009E46C2`.

## What 009E4330 is

`__thiscall(blk)(unit*)` returning `blk`, `RET 4` at `009E46C0`, body
`009E4330-009E46C2`. Its only caller is `BSP_ShipAi_BrainRecordConstruct`
`009F1160` at `009F118D`, and `009F1180 LEA ECX,[ESI+8]` sets the receiver, so
the navigation block is the brain record's **inline sub-object at `brain+8h`**:
it is never allocated on its own, and there is exactly one per ship AI brain.
The argument is the brain's owner unit; `009F1192` stores the same pointer at
`brain+0AA8h`. `009E4354` installs the vtable at `00D21854`, whose `009E46D0`
entry is the deleting destructor.

So the answer to "which caller and when" is: once, while the brain record is
constructed, before any state object exists.

## Every field it writes

Offsets are relative to `blk` = `brain+8h`. The two `_memset` runs cover
`blk+4h..blk+44h` (`009E4363`) and `blk+46h..blk+145h` (`009E4379`); the byte
just past each range is written one instruction *before* the call, so
`blk+45h` and `blk+146h` survive as 1.

### Constants

| field | site | value |
| --- | --- | --- |
| `blk+0h` | `009E4354` | the vtable `00D21854` |
| `blk+45h`, `blk+146h` | `009E435F`, `009E4372` | 1 |
| `blk+148h`, `blk+14Ch`, `blk+170h` | `009E4381`..`009E4391` | 0.0f |
| `blk+158h`, `blk+15Ch` | `009E43C4`, `009E43BC` | `1.0e11f`, the float at `00D21528` |
| `blk+160h`, `blk+164h`, `blk+168h` | `009E43AA`..`009E43B6` | 0 |
| `blk+3E8h`, `blk+3E9h`, `blk+3EAh` | `009E43E1`..`009E43D5` | 0 |
| twelve records at `blk+808h`, stride `2Ch` | `009E43F0`..`009E43FF` | `+0h` = 1, `+14h` = 0, `+24h` = 0 |
| `blk+0A18h`, `blk+0A1Ch` | `009E4471`, `009E4463` | 0 |
| `blk+0A20h` | `009E4469` | `-1.0f`, the float at `00D7A260` |
| three records at `blk+0A24h`, `+0A44h`, `+0A64h` | `009E4401`..`009E4449` | `+0h` = 1, `+14h` = -1, `+18h` = 0, `+1Ch` = 0 |
| `blk+0A90h` | `009E4477` | 0 |
| `blk+0A84h`, `blk+0A88h`, `blk+0A8Ch` | `009E4673`..`009E4683` | 0.0f |
| `blk+400h`, `blk+604h` | `009E4653`, `009E4659` | 0 |
| `blk+3F0h` | `009E468B` | 3 |
| `blk+3ECh`, `blk+3F4h`, `blk+3F5h` | `009E4695`..`009E46A3` | 1, 1, 0 |
| `blk+148h` | `009E4669` | `-uniform(0.0f, 1.0f)`, `009E465F` then `FCHS` |

The twelve `2Ch`-byte records at `blk+808h` are the obstacle sectors: the same
base and stride `009E0270` uses (`009E02FA LEA EAX,[ESI+818h]` is `sector[0]+10h`,
docs/SHIP_AI_SECTOR_SCAN.md section 7). They start immediately after the inline
neighbour array, `blk+608h..blk+807h`, whose count is the `blk+604h` this
routine clears; the count and the sector table are cleared together.

### Derived from the unit and its class

`unit->n` below is `[unit+538h]`, the vehicle class descriptor.

```
blk+3FCh = unit                                                  ; 009E447D
blk+3F8h = (unit && unit->vtable[5Ch](8)) ? unit : 0             ; 009E448E, 009E44A2
blk+168h = [unit->n + 570h]                                      ; 009E44B4
blk+3C8h = 0082E960(unit->n, 1.0f)                               ; 009E44C4, 009E44CF
blk+3E4h = [unit+9C8h] * 0.45                                    ; 009E44DB, double 00CF1748
blk+3D4h = max([unit->n+500h] * 1.5, [unit+9C8h] * 0.4)          ; 009E4519 JBE, 009E4537
blk+3D8h = 1.5 * blk+3D4h                                        ; 009E4533, 009E453F
blk+3CCh = 0082E960(unit->n, 0.9f)                               ; 009E4555, 009E4568
blk+3D0h = ([unit->n+4F8h] >= 4 deg/s) ? [unit->n+4F8h] : 4 deg/s; 009E458D JBE, 009E45A9
blk+340h = blk+3C8h + blk+3C8h                                   ; 009E45B1, 009E45BB
blk+1B4h = blk+3E4h / (blk+3CCh * 1.5)                           ; 009E45C1..009E45D1
blk+1B8h = sqrt(blk+3CCh^2 - [unit+9C8h]^2 * 0.25)               ; 009E45D7..009E4606
blk+260h = blk+2C8h = unit                                       ; 009E4600, 009E460C
blk+318h = (blk+340h >= 250.0) ? blk+340h : 250.0f               ; 009E4625 JBE, 009E4648
```

Class id 8 is `MSubmarine` (docs/ENTITY_CLASS_IDS.md), so `blk+3F8h` is the
submarine view of the owner and is null for every surface ship.

The three `FCOMIP`/`JBE` pairs all compare the *threshold* against the value and
branch to keeping the value, so an unordered compare (a NaN input) keeps the
value rather than the threshold. `009E4587` and `009E461F` load the threshold as
a `double` and `009E4593`/`009E462B` substitute a `float` at a different address;
both pairs hold the same number.

`blk+1B4h` and `blk+1B8h` divide and take a square root with no guard. The
radicand is negative whenever the hull is longer than twice the cruise turning
circle, which no shipped ship row reaches.

### The call at 009E43CC, and what the defaults are

`009E43A4 LEA ECX,[ESI+1C4h]` then `009E43CC CALL 009DFCB0`, so `009DFCB0`'s
`param_1[0x80..0x88]` stores land on `blk+3C4h..blk+3E4h`:

| field | site in 009DFCB0 | default |
| --- | --- | --- |
| `blk+3C4h` | `009DFE60` | `10.0f` (`00CE38B8`) |
| `blk+3C8h` | `009DFE88` | `400.0f` (`00CFD710`) |
| `blk+3CCh` | `009DFE90` | `400.0f` |
| `blk+3D0h` | `009DFE50` | `0.15f` (`00CE7818`) |
| `blk+3D4h` | `009DFE78` | `100.0f` (`00CE3D08`) |
| `blk+3D8h` | `009DFE70` | `100.0f` |
| `blk+3DCh` | `009DFEA0` | `400.0f` |
| `blk+3E0h` | `009DFEA8` | `1000.0f` (`00CE3804`) |
| `blk+3E4h` | `009DFE98` | `50.0f` (`00CEB4D4`) |

`009E4330` then overwrites `+3C8h`, `+3CCh`, `+3D0h`, `+3D4h`, `+3D8h` and
`+3E4h`, and `009E0270` overwrites `+3C4h` at `009E02BA`. Only `blk+3DCh` and
`blk+3E0h` reach the first tick with these authored values.

## 0082E960, where the two turn distances come from

`__thiscall(descriptor)(float throttle)`, `RET 4` at `0082E97F`, ST0 result,
body `0082E960-0082E981`, complete:

```
0082E96B  denominator = 0082E890(throttle)
0082E970  FMUL float ptr [ESI + 520h]
```

`class+520h` is `MaxSpeed / MaxRotAngle`, the minimum turn radius in metres,
derived once per class by `00828F20` (docs/SHIP_AI_CLASS_FIELD_0524.md). `0082E890`
is the two-segment clamped interpolation over the gameplay settings singleton's
`+438h..+44Ch`, which docs/UNIT_RUDDER_CURVE.md traced to
`ShipGlobals["Navigator"]["TurnMultipliers"]` in `shipglobals.lua`. The authors'
own comment on those rows says the multiplier is "at what throttle setting the
turning-circle radius given in the vehicleclass is multiplied by how much", so
`0082E960(class, m)` is the turning-circle radius at throttle fraction `m`.

The installed points are `(0.0, 0.4)`, `(0.5, 1.5)` and `(1.0, 2.0)`, so:

| call site | throttle | multiplier | field |
| --- | --- | --- | --- |
| `009E44C4` | `1.0f` (`FLD1` at `009E44A0`) | `2.0` | `blk+3C8h` |
| `009E4555` | `0.9f` (`00CE3860`) | `1.9` | `blk+3CCh` |

Both call sites `PUSH ECX` only to make room; `FSTP float ptr [ESP]` one
instruction later overwrites that dword with the float argument, and ECX still
holds the descriptor at the `CALL`. The decompiler renders this as a one-integer
call and drops the receiver, which is why the listing was needed.

## The values a destroyer gets

`Length`, `MaxSpeed` and `MaxRotAngle` read from
`scripts/datatables/autoload/vehicleclasses.lua` in the installed game,
read-only. Every value that depends on `[unit+9C8h]` uses the Lua `Length`,
which is what the fallback branch of its producer assigns; see the caveat below.

| field | DeRuyter 1935, row 20 | Fubuki 1944, row 73 |
| --- | --- | --- |
| `MaxSpeed`, `MaxRotAngle`, `Length` | 16.4622, 0.122173, 171 | 19.5489, 0.139626, 118 |
| `class+520h` | 134.745 m | 140.009 m |
| `blk+3C8h` | 269.490 m | 280.018 m |
| `blk+3CCh` | 256.015 m | 266.017 m |
| `blk+3D0h` | 0.122173 rad/s | 0.139626 rad/s |
| `blk+3D4h` | 68.400 m | 47.200 m |
| `blk+3D8h` | 102.600 m | 70.800 m |
| `blk+3E4h` | 76.950 m | 53.100 m |
| `blk+340h`, `blk+318h` | 538.980 m | 560.036 m |
| `blk+1B4h` | 0.20038 rad | 0.13307 rad |
| `blk+1B8h` | 241.317 m | 259.392 m |
| `blk+604h` | 0 | 0 |

For both ships the hull term wins the `blk+3D4h` maximum, so the stop radius is
0.4 hull lengths and the restart radius 0.6 of one, not the speed term. The yaw
floor never binds: both classes turn faster than 4 degrees per second.

`[unit+9C8h]` is a *unit* field, not a class field. Its two producers are
`0081106E` in `FUN_00810F60` and `0081FA4D` in `FUN_0081F980`, which both write

```
unit+9C8h = 2.0 * max([box+3Ch], -[box+30h])        ; box = [class+50h]
unit+9CCh = 2.0 * max(-[box+28h], [box+34h])
```

and, when the class carries no box, copy `descriptor+A0h` and `descriptor+A4h`
straight in. `descriptor+A0h` is the Lua key `Length`
(docs/SHIP_CLASS_FIELDS.md), so `[unit+9C8h]` is a full hull length in metres.
The doubled half-extent and the copied full `Length` can only agree if the box
holds signed extents about the model origin; that layout is a hypothesis this
packet did not close, which is the caveat on the four rows above.

## Every other writer of the five fields

Method: `python tools/bsp.py scan-bytes` over `.text` for every store-shaped
opcode family at the `disp32` forms of `3C8h`, `3CCh`, `3D4h`, `3D8h` and
`604h`, then again at `204h`, `208h`, `20Ch`, `210h`, `214h` and `440h` for the
`blk+1C4h` sub-object base. `reports/ship_ai_nav_block_ctor.json` carries the
per-field hit lists.

| field | writers |
| --- | --- |
| `blk+3C8h` | `009E44CF` (this routine), `009DFE88` (`009DFCB0`, the default) |
| `blk+3CCh` | `009E4568`, `009DFE90` |
| `blk+3D4h` | `009E4537`, `009DFE78` |
| `blk+3D8h` | `009E453F`, `009DFE70` |
| `blk+604h` | `009E4659`, `009F0E69` in `BSP_ShipAi_NeighbourListAdd`, `009F114E` in `BSP_ShipAi_NeighbourListRefresh` |

So the four float fields really are write-once per ship, and the hysteresis pair
and the two turn distances can never change after construction. `blk+604h` is
not: it is the inline neighbour count, and `009F0E69 ADD dword ptr [ESI+604h], imm8`
and `009F114E SUB [ESI+604h], EBP` move it every frame. The keep-clear gate the
arm tail hangs off it is therefore live, not dead, and the constructor's zero is
only the empty starting state.

Every other hit at those displacements inside the ship AI band is `D9 /0 FLD`,
a read. `009EEA8F LEA ECX,[ESI+3C8h]` feeds `00415510 BSP_Math_MinFloatByRef`,
whose body writes only its own stack frame, so it too is a read.

## The commanded throttle, 009E6A90

The brief asked for `009E6A90` whole only if its existing projection was
incomplete. It is complete, and nothing was re-read or re-reconstructed here:

* docs/SHIP_AI_APPROACH_UPDATE.md line 299 states `__thiscall(nested)()`, `RET 0`,
  body `009E6A90-009E6E78`, **complete**, and lines 301-325 transcribe the whole
  slow-down rule including the `9999.0f` sentinel arm and the final clamp;
  its routine table at line 358 records `complete (009E6A90-009E6E78)`.
* docs/SHIP_AI_RING_SCAN.md line 263 records the same range as
  `complete, verified only; projected by the previous packet`, and line 222
  names the sentinel arm `009E6AC8..009E6B17`.
* `python tools/bsp.py lookup 009e6a90` reports the reconstruction
  `ship_ai_approach_limit_throttle_009e6a90`, status `semantic_build_tested`,
  in `src/ship_ai_approach_update.cpp`.

The commanded heading at `nested+120Ch` becomes the commanded throttle at
`nested+1210h`: `009E6AB5`/`009E6ABB` take the wrapped heading error,
`009E6ADE` tests the sentinel and `009E6B12` replaces it with
`interp(pi/6 -> 1.0, 70 deg -> 0.5)` of the absolute error, and the final clamp
bounds it to plus or minus the mode limit. What the executable still lacks is
the wiring, not the rule: `src/game_hosts_ship_ai.cpp` records `009E76D0` and
`009E6A90` rather than running them, which is the existing
`ship_ai_ring_scan_runtime` follow-up.

## Coverage

| routine | coverage |
| --- | --- |
| `009E4330` `009E4330-009E46C2` | whole |
| `0082E960` `0082E960-0082E981` | whole |
| `009DFCB0` `009DFCB0-009DFEB4` | partial: the `blk+3C4h..blk+3E4h` window only (`009DFE50`, `009DFE60`, `009DFE70`, `009DFE78`, `009DFE88`, `009DFE90`, `009DFE98`, `009DFEA0`, `009DFEA8`). `009DFCB0-009DFE4F` and `009DFEB0-009DFEB4` are not projected |
| `009E0270`, `009DE2F0`, `009E6A90`, `0082E890` | not re-read; owned by docs/SHIP_AI_SECTOR_SCAN.md, docs/SHIP_AI_APPROACH_UPDATE.md and docs/UNIT_RUDDER_CURVE.md |
| `00810F60`, `0081F980` | read for the `unit+9C8h` producer only; the rest of both bodies is not projected |

## Corrections

1. **`blk+3CCh` is not a hull radius.** docs/SHIP_AI_CLEARANCE_PROFILE.md line 57
   annotates `blk+3CCh = 0082E960(unit->538h, 0.9f)` as "the hull radius". It is
   the class turning-circle radius at 0.9 throttle. Evidence: `0082E970 FMUL
   float ptr [ESI+520h]` with ESI the descriptor; `class+520h` is
   `MaxSpeed / MaxRotAngle` in metres (docs/SHIP_AI_CLASS_FIELD_0524.md line 102);
   the authors' comment on the three Lua rows (docs/UNIT_RUDDER_CURVE.md lines
   242-248) calls the multiplier the turning-circle factor.
   docs/SHIP_AI_NAVIGATION_ARM_TAIL.md line 156 already called the same field
   "the class turn distance", so the two docs disagreed.

2. **`class+520h` has two readers, not one.** docs/SHIP_AI_CLASS_FIELD_0524.md
   line 104 and its `+520h` table row, and include/bsp/ship_ai_throttle_ring.hpp
   line 129, all say `0082E850` is its only reader. `0082E970 FMUL float ptr
   [ESI+520h]` in `FUN_0082E960` (body `0082E960-0082E981`) is a second, and
   ESI is the descriptor at all three of its call sites (`009E44BA`, `009E4545`,
   `00811A45`). The claim that `00828F20` is its only writer is unaffected.

3. **`[unit+9C8h]` has producers, and it is a length.** docs/GAME_EXECUTABLE.md
   line 6124 calls it "the unit radius that has no producer anywhere";
   docs/SHIP_AI_PATH_PLANNER.md line 40 and
   include/bsp/ship_ai_navigation_arm_tail.hpp's `hull_radius_9c8` repeat
   "radius". `0081106E` and `0081FA4D` write it, from the model box at
   `[class+50h]` doubled, or from `descriptor+A0h` (`Length`) when there is no
   box. It is a full hull length: 171 m for the DeRuyter, 118 m for the Fubuki.

4. **`unit+9CCh` is the full beam, not the half width.**
   docs/SHIP_AI_SECTOR_SCAN.md section 7 calls it "the hull half width". The
   same producer doubles the box half-extent and, in the fallback, copies
   `descriptor+A4h`, the neighbour of the full `Length`. The sector arithmetic
   `w/2.2` and `w/4` is unchanged; only its reading is.

5. **"nothing writes them" is true of four fields, not five.** The brief, quoting
   docs/GAME_EXECUTABLE.md milestone 2q, says every one of the five is an input
   the arm tail reads and nothing writes. `blk+604h` is written every frame by
   `009F0E69` and `009F114E`, and all four float fields are written once more,
   earlier, by `009DFCB0` through the `blk+1C4h` base.

6. **`009E4330` does not call `008E6430`.** docs/SHIP_AI_CLEARANCE_PROFILE.md
   line 353 lists `009DFCB0`, `0082E960` and `008E6430` as its callees. Its six
   callees are `0082E960`, `009DFCB0`, `009E0270`, `00BD2F10`, `00BF7030` and
   `00BF79F0`. `008E6430` is reached from `00811A30`, a different caller of
   `0082E960`, at `00811A59`.

7. **The `ship_ai_nav_block_fields` follow-up is two-thirds answered.**
   docs/SHIP_AI_NAVIGATION_ARM.md asks `009E4330` for initial values for
   `blk+19Ch`, `blk+3C8h`, `blk+3D0h` and `blk+1F0h`. `blk+3C8h` and `blk+3D0h`
   are `009E44CF` and `009E45A9`, above. `blk+19Ch` is written during
   construction but by `009DE2F0` at `009DE465`, reached through
   `009E46A9` -> `009E0270` -> `009E02C2`. `blk+1F0h` gets nothing: neither
   memset range covers it, no store in `009E4330` or `009DFCB0` targets it, and
   `009F1160` and `009F39C0` call no memset, so it holds whatever the brain's
   allocation left until `009DE198`, `009EE6CB` or `009DA1C9` writes it at run
   time.

## Uncertainties

1. The null test at `009E4483` guards only the vtable call. `009E44A8` and
   `009E44BA` dereference the unit pointer unconditionally, so the native
   routine faults on a null unit. The projection records the null case only in
   `blk+3F8h`.
2. `009E466F PUSH 1` supplies a float parameter. Read as a float that is the
   denormal `1.4e-45`, not `1.0f`. Whether `009F5156` passes a real float was
   not checked; `009E0270` belongs to docs/SHIP_AI_SECTOR_SCAN.md.
3. The writer scan is a `disp32` scan from the two bases any reader uses. It
   cannot see a store through a third base with a short displacement, nor a bulk
   copy over the block.
4. The fields with no traced reader keep placeholder names carrying only their
   offset: `blk+14Ch`, `blk+164h`, `blk+170h`, `blk+400h`, `blk+0A18h`,
   `blk+0A1Ch`, `blk+0A84h..blk+0A8Ch`, `blk+0A90h`.
5. `class+520h` is taken from the existing `ShipClassAiDerivedMotion` projection
   rather than re-derived.
6. No run-time evidence. `bsp_game.exe` never constructs a ship AI brain, so
   docs/WORKER_VERIFICATION_CHECKLIST.md rule 6 does not bind here.

## Follow-up packets

| id | scope |
| --- | --- |
| `ship_ai_nav_block_seed_defaults` | `009DFCB0` whole, `009DFCB0-009DFEB4`. Only the `blk+3C4h..blk+3E4h` window is projected here; the rest seeds `blk+1C4h..blk+3C3h`, the steering and pose fields docs/SHIP_AI_STATES.md models by hand. |
| `unit_hull_extents_00810f60` | `FUN_00810F60` and `FUN_0081F980` whole, and the record at `[class+50h]`. This packet established that they produce `unit+9C8h` and `unit+9CCh`, but did not read the box's own producer, so the `+28h..+3Ch` layout is a hypothesis. Closing it settles corrections 3 and 4 and fixes the wording in four docs and one header. |
| `ship_ai_nav_block_memo_records` | The three `20h`-byte records at `blk+0A24h`, `+0A44h` and `+0A64h` seeded with `+0h` = 1 and `+14h` = -1. docs/SHIP_AI_CLEARANCE_PROFILE.md reads the first two back through `009D57E0` and `00415D70` without naming the record. |
| `ship_ai_nav_block_ctor_runtime` | Wire `ship_ai_nav_block_ctor_009e4330` into `bsp_game.exe`. Nothing in the executable constructs a ship AI brain today. |

## no_ghidra_function

none. Every routine read or named here has a Ghidra function whose body range
the bridge reports: `009E4330-009E46C2`, `0082E960-0082E981`,
`009DFCB0-009DFEB4`, `009E0270-009E04D9`, `009DE2F0-009DE5A0`,
`00810F60-008110CF`, `0081F980-008206EB`, `00811A30-00811AAA`,
`009DDBC0-009DDCBF`, `0082E850-0082E881`, `009F1160-009F1413`,
`009F0D20-009F0E83`, `009F0EA0-009F115D`. `python tools/verify_report_calls.py
reports/ship_ai_nav_block_ctor.json` checked all nine call rows: 0 failed.
