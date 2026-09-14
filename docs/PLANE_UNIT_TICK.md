# Why a plane unit never ticks

Addresses: 007CE040, 007C6500, 0074E0F0, 0074E2D0, 0084C920, 007CFD20, 007D7720, 007DD9B0,
00951B60, 00951C40, 00951D20, 0087B670, 00875890, 00953CC0, 00825F20, 007CEC30, 0074E210,
007C1430, 007C6340, 00727BD0, 00E092C8, 00E19BF8, and read-only 007CC2F0, 007CBFA0, 007CBA50,
0085DC80, 007DC830, 007DB680, 00863990, 008633D0, 008624C0, 008053C0, 00806480.

`bsp_game.exe` on `IJN01` creates seven `A7M` fighter units with four category-0 `PLANEGUN` guns
each, and every column of their report row is zero. This document establishes that two separate
mechanisms produce those zeroes, that only one of them is a gap in the reconstruction, and what
the host would have to do, smallest first, to move each one.

Summary:

| observation | cause | native or host |
| --- | --- | --- |
| plane does not move | the host's per-unit motion loop skips every dispatch whose entry is `007CE040` | **host gap**, `src/game_hosts_units.cpp:1976` |
| `PLANEGUN` takes 0 assignments over 82 guns | category 0's authored preference row holds one class id, `61h`, which is outside the `0..60h` id space, so its rank row is empty for every class | **native rule**, `00E092C8` |
| plane `range` reads 0 | the report column is `best_range`, written only *after* a candidate survives the rank gate; the plane's `category_ranges[0]` is derived and never printed | **host reporting artifact**, `src/game_hosts_gunnery.cpp:824` |
| plane is never shot at (`taken 0`) | the host's recon-contact stand-in admits only `IsKindOf(06h)` ship-base units; the native scan visits seven plane leaf class ids | **host gap**, `src/game_hosts_gunnery.cpp:774` |

## 1. How a plane is ticked natively

### 1.1 The receiver is the same object a ship uses

Every unit instance embeds a tick-registration node at `unit+310h`. `0087B670`
`BSP_UnitTickableEntity_Construct`, level 3 of the unit constructor chain, builds it: `0087B69C
LEA EBX,[this+310h]`, then `0087B6A9 CALL 00875890 BSP_TickRegistration_Construct` with that node
as `ECX`, the unit as the payload and group 0. `00875890` has exactly nine call sites, and the
unit one is shared by ships, planes and every other unit class. **A plane is not a different kind
of tickable object; it is the same node with a different vtable.**

The node's vtable has six slots, whose roles `include/bsp/tick_element_overrides.hpp` and
`docs/TICK_ELEMENT_OVERRIDES.md` already fix. The three families differ only in the overrides:

| slot | role | generic unit `00D1A654` | ship `00CFC38C` | plane `00D068DC` |
| --- | --- | --- | --- | --- |
| `+0h` | destroy | `00959C10` | `006FE510` | `007DDA60` |
| `+4h` | PlacePose (interpolation) | `0042BB70` (base stub, `RET 4`) | `00811AB0` | **`007C6500`** |
| `+8h` | AdvanceSim (fixed step) | `00953CC0` | `00825F20` | **`007CE040`** |
| `+0Ch` | CommitPose | `006D1FC0` | `006D1FC0` | **`007BEEE0`** |
| `+10h` | unused | `0042BBA0` | `0042BBA0` | `0042BBA0` |
| `+14h` | predicate | `0042BBB0` | `0042BBB0` | `0042BBB0` |

Slot bytes read from the image: `00CFC38C` = `10 e5 6f 00 | b0 1a 81 00 | 20 5f 82 00 | c0 1f 6d
00`; `00D1A654` = `10 9c 95 00 | 70 bb 42 00 | c0 3c 95 00 | c0 1f 6d 00`; `00D068DC` = `60 da 7d
00 | 00 65 7c 00 | 40 e0 7c 00 | e0 ee 7b 00`. Both the plane and the ship tables continue
`00D068EC` / `00CFC39C` = `a0 bb 42 00 | b0 bb 42 00`, the two base stubs.

A plane therefore overrides **three** slots where a generic unit overrides one and a ship two.

### 1.2 Nine plane tick vtables, not five

`docs/PLANE_FLIGHT.md` states that the base plane table plus five derived tables carry `007C6500`
at `+4h` and `007CE040` at `+8h`. A whole-image byte census of the dword `40 e0 7c 00`
(`007CE040`) returns **nine** hits, all in `.rdata`, all at a vtable `+8h`, and a census of the
pair `00 65 7c 00 40 e0 7c 00` returns the same nine. There is no `.text` hit, and the call graph
gives `007CE040` zero direct callers: it is reached only through slot `+8h`.

Each installer was read and each carries the class id it writes to `unit+C4h`:

| ctor | class id `+C4h` | class | tick vtable | install site |
| --- | --- | --- | --- | --- |
| `007CFD20` `BSP_PlaneUnitInstance_Construct` | inherited | plane base | `00D05EDC` | `007CFDA0` |
| `007D7720` | `10h` | `MPlaneBomber` | `00D065F4` | `007D7757` |
| `007DD9B0` | `13h` | `MPlaneFighter` | `00D068DC` | `007DD9E7` |
| `00951B60` | `12h` | `MPlaneDiveBomber` | `00D19CE4` | `00951B97` |
| `00951C40` | `11h` | `MPlaneTorpedoBomber` | `00D19FBC` | `00951C77` |
| `00951D20` | `17h` | `MPlaneKamikaze` | `00D1A294` | `00951D57` |
| **`0074E0F0`** | **`14h`** | **`MReconPlane`** | **`00D0002C`** | **`0074E127`** |
| **`0074E2D0`** | **`16h`** | **`MLargeReconPlane`** | **`00D002C4`** | **`0074E305`** |
| **`0084C920`** | **`15h`** | **`MSmallReconPlane`** | **`00D0BA3C`** | **`0084C955`** |

The three bold rows are the ones the earlier census missed. They are a second class tree:
`0074E0F0` calls `007CFD20` at `0074E0FA` and then rewrites five vtable pointers and
`this+72Ch`; `0074E2D0` and `0084C920` each call `0074E0F0` (`0074E2D8`, `0084C928`) and rewrite
only `+310h` and `+C4h`. The eight concrete ids `10h 11h 12h 13h 14h 15h 16h 17h` are exactly the
eight rows of `kUnitMotionDispatches` that carry entry `0x007ce040`.

### 1.3 What `007CE040` does per step

`__thiscall(node, float step)`, Ghidra body `007CE040`-`007CF172`, 1095 instructions. `ESI` is the
node, `EDI` the unit (`007CE0ED LEA EDI,[ESI-310h]`). Most of the body is damage, fire, collision,
sound and effects. The simulation is:

1. `007CE08F CALL 00953CC0` - the level-4 unit tick that a generic unit uses as its whole `+8h`.
   Inside it, `unit->vtable[+1F0h]` (the plane's is `0095DC40`) and, when `unit+520h` is clear,
   `unit->vtable[+1D8h]` (the plane's is `007C6C30`).
2. `007CEC30`-`007CECBA`, the motion dispatch: three mutually exclusive arms and a shared commit.

The dispatch listing, read directly:

```
007cec30  MOV EDX,[ESI+41Ch]        ; node+41Ch = unit+72Ch, an EMBEDDED sub-object's vtable
007cec36  MOV EAX,[EDX+38h]
007cec39  LEA ECX,[ESI+41Ch]        ; ECX = unit+72Ch, so the sub-object is embedded, not a pointer
007cec3f  CALL EAX
007cec41  TEST AL,AL / JZ 007cec75
007cec45  CMP byte [ESI+6D0h],0     ; unit+9E0h, the airborne-time freeze
007cec4e  FLD step / FADD [ESI+5F8h] / FSTP   ; unit+908h += step
007cec6e  CALL 007cc2f0             ; free-flight arm
007cec75  MOV EAX,[EDI+900h]        ; EDI is the unit: unit+900h
007cec7b  CMP EAX,4 / 007cec80 CMP EAX,5
007cec92  CALL 007cbfa0             ; ground-roll arm
007cec99  CMP dword [ESI+5F0h],6    ; node+5F0h = unit+900h, the SAME field
007ceca0  JNZ 007cecbf              ; no arm: the commit below is skipped too
007cecaf  CALL 007cba50             ; surface arm
007cecb4  LEA ECX,[ESI+364h]        ; unit+674h
007cecba  CALL 0085dc80             ; re-orthonormalise the pose basis (NOT a commit)
```

**Correction.** That last line read "commit this step's pose", and a packet was written on the
strength of it before the body was read. `0085DC80` is `BSP_Matrix_OrthonormalizeBasisRows`, a
general Gram-Schmidt over one row-major 4x4's three basis rows, called from 54 sites across the
binary and already named in the ledger. It takes only the matrix pointer in `ECX` - no step, no
velocity, no control axis - and on an already-orthonormal basis it does nothing at all. It cannot
turn a plane; it is the tidy-up that keeps a basis from drifting. `docs/PLANE_POSE_COMMIT.md` has
the rule and the evidence.

The free-flight gate is not opaque. `00D06130+38h` and `00CFFFE8+38h` (the two `unit+72Ch`
vtables, base-plane and recon-plane) both hold `0074E210`, whose four instructions on disk are

```
0074e210  33 c0              XOR EAX,EAX
0074e212  83 b9 d4 01 00 00 07  CMP dword [ECX+1D4h],7
0074e219  0f 94 c0           SETZ AL
0074e21c  c3                 RET
```

`ECX` is `unit+72Ch` from `007CEC39`, so `[ECX+1D4h]` is `unit+900h`. **All three arms and the
"no arm" case are decided by one field, `unit+900h`, the flight state:**

| `unit+900h` | arm | call |
| --- | --- | --- |
| `7` | free flight | `007CEC6E` `007CC2F0` |
| `4` or `5` | ground roll | `007CEC92` `007CBFA0` |
| `6` | water surface | `007CECAF` `007CBA50` |
| anything else (`0`, `1`, `2`, `3`) | **none**, and `007CECBA`'s pose commit is skipped | - |

`007C1430 BSP_Plane_SetFlightState` is the general writer (`007C145B MOV [ECX+900h],EAX`);
`007C6340 BSP_Plane_ChooseSpawnFlightState` is the writer on the create path (`007C63F4` stores
`6`); `007C7110 BSP_Plane_BeginFlying` stores `7` at `007C7183`.

This supersedes `docs/PLANE_FLIGHT.md`'s "surface | `unit+5F0h == 6`": `ESI` at `007CEC99` is the
node, not the unit, so `ESI+5F0h` is `unit+900h`. `include/bsp/plane_ground_ops.hpp`'s
`select_motion_arm_007cec30(bool control_mode_gate, int flight_state)` already has the corrected
two-input form; the listing above is the independent confirmation, and it further collapses
`control_mode_gate` to `flight_state == 7`.

## 2. Where the reconstruction drops it

### 2.1 The exact site

`src/game_hosts_units.cpp:94-116` declares `kUnitMotionDispatches`, keyed on the descriptor's
`allocate_instance`. Eight rows resolve to entry `0x007ce040` with
`UnitMotionCoverage::unresolved`:

```
{0x008091d0, 0x00d0002c, 0x007ce040, UnitMotionCoverage::unresolved},   // MReconPlane 14h
{0x0084ca50, 0x00d0ba3c, 0x007ce040, UnitMotionCoverage::unresolved},   // MSmallReconPlane 15h
{0x0074e540, 0x00d002c4, 0x007ce040, UnitMotionCoverage::unresolved},   // MLargeReconPlane 16h
{0x007ddae0, 0x00d068dc, 0x007ce040, UnitMotionCoverage::unresolved},   // MPlaneFighter 13h  <- A7M
{0x00956390, 0x00d19ce4, 0x007ce040, UnitMotionCoverage::unresolved},   // MPlaneDiveBomber 12h
{0x009564e0, 0x00d19fbc, 0x007ce040, UnitMotionCoverage::unresolved},   // MPlaneTorpedoBomber 11h
{0x00956240, 0x00d1a294, 0x007ce040, UnitMotionCoverage::unresolved},   // MPlaneKamikaze 17h
{0x007d7850, 0x00d065f4, 0x007ce040, UnitMotionCoverage::unresolved},   // MPlaneBomber 10h
```

The per-unit motion loop is `src/game_hosts_units.cpp:1920`. For a plane slot:

- `1926` `if (!slot.motion_dispatch.runs_ship_base())` is **true** (`unresolved` is neither
  `direct_ship_body` nor `ship_base_fragment`);
- `1927` `if (slot.motion_dispatch.entry == 0x00953cc0u)` is **false** (it is `0x007ce040`);
- `1975` `if (slot.motion_dispatch.entry != 0)` is true, so
- **`1976` `host.record_motion_phase("unreconstructed_phase", 0x007ce040);`**
- `1980` `continue;`

That single `continue` is the whole answer to "the plane exists and then does nothing". The unit
is not filtered out of creation, it fails no registration test, and there is no plane branch: the
loop records the address it would need and skips the slot. `007DDAE0` (the `A7M`'s allocator,
`0E94h` bytes at `007DDAF8`) resolves correctly, so the dispatch row is found - it is the *body*
behind it that the host does not run.

### 2.2 The reconstruction of that body already exists and has no caller

`include/bsp/plane_flight.hpp:307` declares `PlaneFlightHost` with one method per native call site
of `007CE040`'s motion path, and `src/plane_flight.cpp:150` implements
`run_plane_fixed_step_007ce040(PlaneFlightHost&, float)`. `include/bsp/plane_ground_ops.hpp`
declares `PlaneGroundOpsHost` and `run_ground_roll_step_007cbfa0`. A repository-wide grep for
`PlaneFlightHost` and `PlaneGroundOpsHost` finds **only the declarations and definitions**: no
class in `src/game_hosts_*.cpp` derives from either, and neither entry point is called. The plane
tick is reconstructed and unbound, not missing.

### 2.3 What is *correctly* not done

`src/game_hosts_ship_ai.cpp:61` `has_ship_navigation_class(kind)` lists only the eight ship-family
`VehicleClassKind` leaves, so `4187`'s `register_units` builds no navigation block, no avoid
search and no controller for a plane. That is right: `include/bsp/vehicle_class.hpp:40-48` gives
planes their own kinds `0Fh`..`17h`, and the ship-family `virtual+210h` brain is not theirs. A
plane is not a ship with the brain switched off; it has no reconstructed brain of its own at all
(see §5, step 6).

## 3. The gunnery side: why `PLANEGUN` takes zero assignments

**This one is native, and it is not a defect.**

`00E092C8` is file-initialized `.data` holding twelve authored preference rows of `61h` dwords on
a `184h` stride. The first row, category 0 `PLANEGUN`, reads on disk:

```
00e092c8  61 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00e092d8  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

`00727BD0 BSP_GunneryTargetRanks_Build` (body `00727BD0`-`00727C24`) inverts those rows into the
rank table at `00E19BF8`. Its listing, read in full:

```
00727bd6  MOV EBP,0xe19bf8          ; rank row cursor
00727bdb  MOV ESI,0xe092c8          ; preference cursor
00727be0  XOR EAX,EAX / MOV ECX,0x61 / MOV EDI,EBP / LEA EDX,[EAX+1] / REP STOSD   ; zero the row
00727bf3  MOV ECX,[EAX]             ; the authored class id
00727bf5  TEST ECX,ECX / JZ 00727c05  ; 0 is a hole, not an id, and consumes no rank
00727bf9  ADD ECX,EBX               ; EBX = category * 0x61
00727bfb  MOV [ECX*4 + 0xe19bf8],EDX ; rank_table[category*0x61 + id] = rank
00727c02  ADD EDX,1
00727c0d  ADD EBP,0x184 / 00727c13 ADD EBX,0x61 / 00727c16 CMP EAX,0xe0a4f8 / JL
```

There is no bound on the write. Category 0's single entry `61h` writes rank 1 to
`rank_table[0*61h + 61h]`, which is index `61h` - the first dword of **row 1**, not row 0. The
next outer iteration begins with the `REP STOSD` at `00727BEC`, which zeroes all `61h` dwords of
row 1 before filling it. The stray write never survives, and **row 0 ends the pass entirely zero.**

`61h` is one past the class-id space `0..60h`, so even without the aliasing it could never match a
unit. The consumer (`src/game_hosts_gunnery.cpp:799`, from `00863990`'s chain) rejects any
candidate whose rank is 0, so a category-0 gun has no admissible target class at all. Seven `A7M`
units with four `PLANEGUN` guns each and 82 `PLANEGUN` guns on the mission therefore take exactly
zero assignments, and would in the shipped game too.

The native code confirms the intent rather than contradicting it. `008624C0`'s stance apply has a
plane arm at `00862558` (`src/unit_gunnery_pass.cpp:117`): *a plane keeps categories 0 and 1
enabled whatever the director says*. So the pass does attach to a plane, category 0 is enabled,
the guns are in `unit+394h` and their range is in `unit+430h` - and the authored target list is
deliberately empty. `PLANEGUN` is the fixed forward battery the airframe aims; category 1
`AAMACHINEGUN`, whose row does list all seven plane class ids, is the turret a gunner swings. The
unit-side auto-target pass drives the second and not the first.

**The `A7M` therefore has no AI-driven weapon at all on this mission**: all four of its guns are
category 0. It would fire only under pilot control (`00519520
BSP_PlanePilotView_BuildPlayerCommand`, the player's command block at `unit+9FCh`..`+A10h`) or
through a plane AI that points the airframe.

### 3.1 The `range 0` column is a reporting artifact

`src/game_hosts_gunnery.cpp:824` writes `state_.row.best_range` **inside**
`score_candidate_00863990`, after the rank gate at `799` has already returned false. `nearest`
(line `821`) is in the same position. For a unit all of whose guns are category 0, both columns
are structurally unreachable and print 0 whatever the guns' ranges are.

The plane's ranges are in fact derived: `src/game_hosts_gunnery.cpp:634`
`store_category_range(category, range)` fills `state.category_ranges[0]` from `00956C20` over the
`PLANEGUN` devices' `max_range`, and `655` copies it to `state.row.category_ranges[0]`. The
"`range` still reads 0 even after a fix that gave 674 guns a derived engagement range" in the
packet brief is this column, not a missing derivation. It is not proof that the `PLANEGUN` kind
resolves to 0 in `006E9890`'s table - that has to be read off `row.category_ranges[0]`, which
nothing prints.

## 4. Why nothing shoots *at* the plane

`taken`, `hits` and `dealt` are zero for the `A7M` from the other side as well, and that **is** a
host gap.

The gunnery pass gets its candidate set from `recon_contact_count_008053c0`,
`src/game_hosts_gunnery.cpp:763`. The host has no recon slot object, so the method is a documented
stand-in, and its class filter is

```
if (!owner_.units.unit_is_kind_of(i, bsp::kUnitGunneryKindShipBase)) continue;   // 06h
```

`docs/RECON_SLOT_LISTS.md` §1(a) gives the native membership rule: `00806480`'s lazily built
singleton (`00F874C8`, constructed by `00805F60`) holds **twenty-two** class ids, and the scan
visits the world registry's per-class list for each and only for each. Seven of the twenty-two are
plane leaves - `10h MPlaneBomber`, `11h MPlaneTorpedoBomber`, `12h MPlaneDiveBomber`, `13h
MPlaneFighter`, `15h MSmallReconPlane`, `16h MLargeReconPlane`, `17h MPlaneKamikaze`. §1(b)'s
class test is `vtable[5Ch](2)`, `IsKindOf(02h)`, the base of every scene entity - not `06h`.

The stand-in is therefore strictly narrower than the rule it stands in for, and it is the reason
no ship's 295 `AAMACHINEGUN` or 60 `FLAK` guns ever see an `A7M`. Both of those categories rank
every plane class (`00E0944C` row 1: `17h 11h 12h 15h 16h 10h 13h ...`; `00E09A5C` row 5: `17h 11h
12h 15h 16h 14h 10h 13h 0Eh`), and `008633D0`'s mask bit 0 is the plane bit, so the rest of the
chain is already built for air targets. Nothing downstream of the contact list has to change.

(`14h MReconPlane` is a concrete class with its own allocator `008091D0` and yet is **not** in the
twenty-two. Either it never spawns as a leaf or it is deliberately untargetable; this packet did
not settle which.)

## 5. The host prescription, smallest first

| # | change | native evidence | what it makes measurable |
| --- | --- | --- | --- |
| 1 | In `recon_contact_count_008053c0`'s stand-in, replace `IsKindOf(06h)` with the native membership rule: leaf class id in `00806480`'s twenty-two, and `IsKindOf(02h)`. One predicate, no new module. | `docs/RECON_SLOT_LISTS.md` §1(a)(b); `00806480`, `00805F60`, `008074D5`..`008074FA` | The seven `A7M` become candidates for every category-1 and category-5 gun. `taken`, `hits`, and the 295 `AAMACHINEGUN` / 60 `FLAK` guns' `shots` leave zero for the first time, and the plane arms of `008633D0` (mask bit 0) and `00863990` (the loitering-plane penalty) are exercised at all. The planes are static, so it is a fixed-target measurement - but it is the whole air-defence chain. |
| 2 | Print `row.category_ranges[]` beside `best_range` in the gunnery report. | `00956C20`; `src/game_hosts_gunnery.cpp:634`, `655`, `824` | The `A7M`'s `PLANEGUN` engagement range becomes visible, settling whether `006E9890`'s kind table gives `PLANEGUN` a non-zero `+60h` or not. Today the column cannot distinguish the two. |
| 3 | Bind the eight `entry == 0x007ce040` rows to the existing `bsp::run_plane_fixed_step_007ce040`, with a `PlaneFlightHost` whose three arm methods only record, and seed `unit+900h` from `007C6340`'s create-path choice. | `007CE040`; `007CEC30`-`007CECBA`; `0074E210`; `007C6340` at `007C63F4`; `007C1430` at `007C145B` | Which arm each plane takes, and the count of `007CECBA` pose commits. Proves the plane tick runs at all. Until `unit+900h` is seeded it is provably `None` for every plane, and even the pose commit is skipped - the current zero is the *correct* output of the reconstructed selector for a state-0 plane. |
| 4 | Give the ground-roll arm a body with the existing `bsp::run_ground_roll_step_007cbfa0`. | `007CBFA0` -> `007DCCF0` -> `007DB680` | The cheapest arm that moves a plane, and the one `docs/PLANE_GROUND_OPS.md` already covers. It still bottoms out in `007DB680`, so the motion is only as real as step 5. |
| 5 | Read `007DB680`'s unread body `007DB760`-`007DC82A` and `007DC830`'s `007DCA17`-`007DCB47`. A packet, not a host change. | `docs/PLANE_FLIGHT.md` coverage section | Real altitude and speed. This is the "full flight model" answer: **a plane cannot hold altitude until the core law is read**, because every arm routes into it. |
| 6 | A plane AI. | **`0099ACD0 BSP_PilotBot_Tick`** | **This row was wrong and is corrected.** It said "nothing in the ledger names one", on the strength of no `Plane*Ai`/`Plane*Think` name existing. None does - because the AI is not named `Plane*`. It is `PilotBot`, vtable `00D1F348` slot `+0Ch`, `void __thiscall(bot, float dt)`, and the plane unit hangs off `bot+50h`. `docs/PILOT_COMMAND_PATH.md` (2026-09-12) had already recovered the command path, and `docs/PLANE_AI_CONTROL.md` proves by a complete writer census that the bot is the only non-player producer. The lesson is the method: a name search across one naming convention is not a census, and this row presented one as the other. |

Steps 1 and 2 are one predicate and one log line each, touch no plane code, and are the only two
that move a counter this week. Steps 3-6 are the honest scope of "air combat": **the host has no
plane tick, no flight law body and no plane AI, and the pieces that do exist (`plane_flight.cpp`,
`plane_ground_ops.cpp`, `plane_squadron.cpp`) are pure rules with no host behind them.**

Every one of those edits is in `src/game_hosts_units.cpp` and `src/game_hosts_gunnery.cpp`, which
the integrator owns. This packet publishes no code (see §7).

## 6. Run-time evidence, and a negative result

The claims in §1, §3 and §4 are static readings of the image and of host source; §2's are readings
of host source. The run-time evidence for the zeroes themselves is the packet brief's `IJN01`
measurement (78 units, 606 guns, seven `A7M` rows all zero, `PLANEGUN` 0 assignments over 82
guns), which is consistent with all four rows of the summary table.

**A fresh run could not be taken in this session.** `./scripts/build.ps1` succeeded (exit 0), and
`build/win32/Release/bsp_game.exe --frames 60 --log local/plane_run2.log --game-root "<install>"`
exits `0xC0000005` at a fixed point: the last log line is always

```
host Phase 5 load_game_settings [008d8190] concrete
settings resolution=2560x1440 index=24 fullscreen=0 vsync=1 antialias=0 shader_model=2 ...
```

i.e. it faults in the next step, `sound_->core.startup()` / `Phase 5 sound_system_initialize
0073DAFD` (`src/game_hosts.cpp:1378`-`1381`). It reproduces with `--menu-select IJN01
--mission-frames 120`, with a bare `--frames 60`, and with `--settings-personal-root` pointed at an
empty directory, so it is not the options file. **The main checkout's own
`build/win32/Release/bsp_game.exe` crashes identically at the same line**, and no other
`bsp_game.exe` was running, so this is the session's environment rather than this worktree or this
build. Treat every "would become measurable" in §5 as a prediction, not an observation.

## 7. Proven, assumed, and not published

**Proven.**
- The nine plane tick vtables and their nine installers, each with the class id it writes to
  `unit+C4h`: byte census of `40 e0 7c 00` and of the `+4h`/`+8h` pair over the whole image, plus
  the disassembly of each installer.
- `007CE040` has no direct caller and no `.text` reference; the nine `.rdata` slots are its only
  references.
- The three-way slot comparison of `00D1A654`, `00CFC38C` and `00D068DC`, from image bytes.
- The motion dispatch `007CEC30`-`007CECBA` in full, from the listing, including that `ESI` is the
  node and `EDI` the unit, that the surface arm's `ESI+5F0h` is `unit+900h`, and that a plane in
  any other state runs no arm and skips the pose commit.
- `0074E210` as the free-flight gate, from its four bytes on disk, and `ECX = unit+72Ch` from
  `007CEC39 LEA ECX,[ESI+41Ch]`, giving `unit+900h == 7`.
- `00E092C8` row 0 and `00727BD0`'s full body, hence the empty category-0 rank row.
- Every host site in §2 and §4, from the source in this worktree.

**Assumed or partial.**
- `coverage: partial` for `007CE040` - only `007CE040`-`007CE094` and `007CEC30`-`007CECBF` were
  read here; `007CE094`-`007CEC30` and `007CECBF`-`007CF172` (damage, fire, collision, sound,
  effects) are unread, as `docs/PLANE_FLIGHT.md` already states.
- `007C6500`, `007CC2F0`, `007CBFA0`, `007CBA50`, `0085DC80`, `007DC830` and `007DB680` are read
  here as **contracts only**, from `docs/PLANE_FLIGHT.md` and `docs/PLANE_GROUND_OPS.md`. Their
  bodies were not re-read in this packet.
- Whether `00D06130` is the base plane's `unit+72Ch` vtable is taken from
  `include/bsp/plane_flight.hpp`'s `free_flight_gate_00d06130_38`. What was verified directly is
  that `00D06130+38h` and `00CFFFE8+38h` both hold `0074E210`, and that `00CFFFE8` is what
  `0074E13B` writes to `unit+72Ch` for the recon-plane tree.
- `008091D0`, `0074E540`, `0084CA50`, `007DDAE0`, `00956390`, `009564E0`, `00956240` and
  `007D7850` are taken as the eight descriptor `allocate_instance` thunks from
  `kUnitMotionDispatches`; only `007DDAE0`'s head was read (`0E94h` at `007DDAF8`). The
  ctor-to-allocator pairing is inferred from the vtable each row names, which was verified.
- Why `14h MReconPlane` is absent from the twenty-two scanned ids: **not established**.
- `PLANEGUN`'s actual derived `+60h` from `006E9890`: **not established** - see step 2. Nothing in
  this process prints it.
- Whether anything other than the pilot fires a `PLANEGUN`: **not established**. No plane AI name
  exists in the ledger.

**No module published.** `include/bsp/plane_flight.hpp` already declares `PlaneFlightHost` and
`run_plane_fixed_step_007ce040`, and `include/bsp/plane_ground_ops.hpp` already declares
`select_motion_arm_007cec30` and `run_ground_roll_step_007cbfa0`. Every rule this packet could
express is one of those, and the one refinement found - that the free-flight gate collapses to
`flight_state == 7` - belongs in `select_motion_arm_007cec30`'s owner, not in a second copy. There
is no new pure rule with explicit inputs here, so there is no header, no source, and no
`cmake/startup.cmake` line. The result is scoping evidence.

## 8. Corrections

- **Was** (`docs/PLANE_FLIGHT.md`, "The per-step law"): "`007D7757` installs `00D065F4`, `007DD9E7`
  `00D068DC`, `00951B97` `00D19CE4`, `00951C77` `00D19FBC` and `00951D57` `00D1A294`, and all five
  tables carry `007C6500` at `+4h` and `007CE040` at `+8h`", and in Coverage, "`complete`: ... the
  five plane tick vtables and the proof that all share `007CE040`".
  **Is:** nine tables carry the pair, not six. The three additional ones are `00D0002C`
  (`0074E0F0`, `MReconPlane` `14h`), `00D002C4` (`0074E2D0`, `MLargeReconPlane` `16h`) and
  `00D0BA3C` (`0084C920`, `MSmallReconPlane` `15h`), a second class tree that calls `007CFD20` and
  then overrides `+310h`.
  **Evidence:** a whole-image byte census of the dword `40 e0 7c 00` returns exactly nine hits, all
  `.rdata`, all at a vtable `+8h`: `00d00034 00d002cc 00d05ee4 00d065fc 00d068e4 00d0ba44
  00d19cec 00d19fc4 00d1a29c`; the same nine for the pair `00 65 7c 00 40 e0 7c 00` at the `+4h`
  offsets. Installers: `0074E127 MOV dword [ESI+310h],0xd0002c` preceded by `0074E0FA CALL
  007cfd20` and followed by `0074E145 MOV dword [ESI+0C4h],0x14`; `0074E305`/`0074E323` = `0x16`
  after `0074E2D8 CALL 0074e0f0`; `0084C955`/`0084C973` = `0x15` after `0084C928 CALL 0074e0f0`.
  `src/game_hosts_units.cpp`'s `kUnitMotionDispatches` independently carries all eight concrete
  plane rows.

- **Was** (`docs/PLANE_FLIGHT.md`, dispatch table): "surface | `unit+5F0h == 6`".
  **Is:** `unit+900h == 6`. All three arms read one field.
  **Evidence:** `007CE0ED LEA EDI,[ESI-310h]` makes `ESI` the node and `EDI` the unit;
  `007CEC99 CMP dword [ESI+5F0h],6` is therefore `unit+310h+5F0h` = `unit+900h`, the same field
  `007CEC75 MOV EAX,[EDI+900h]` reads two instructions earlier for the ground arm.
  `include/bsp/plane_ground_ops.hpp:136` already carries the corrected single-input form; this is
  an independent confirmation from the listing.

- **Was** (`docs/PLANE_FLIGHT.md`, dispatch table): "free flight | `(*(unit+72Ch))->vtable[+38h]()`
  is true", left as an opaque virtual, and `include/bsp/plane_flight.hpp:313`'s
  `free_flight_gate_00d06130_38`.
  **Is:** `unit+72Ch` is an **embedded** sub-object, not a pointer, and both `+72Ch` vtables in the
  plane trees resolve slot `+38h` to `0074E210`, which is `return this->+1D4h == 7` - that is,
  `unit+900h == 7`.
  **Evidence:** `007CEC39 LEA ECX,[ESI+41Ch]` (an `LEA`, so `ECX` is the sub-object's address, and
  `007CEC30 MOV EDX,[ESI+41Ch]` loads its first dword as the vtable); `0074E13B MOV dword
  [ESI+72Ch],0xcfffe8` writes a vtable pointer directly into the field; `00D06168` and `00D00020`
  (both `+38h`) hold `0074E210`; `0074E210`'s disk bytes `33 c0 83 b9 d4 01 00 00 07 0f 94 c0 c3`,
  and `310h + 41Ch + 1D4h = 900h`.

- **Refutation of a reading of the packet brief:** "their unit `range` still reads 0 even after a
  fix that gave 674 guns a derived engagement range" does **not** show that `PLANEGUN` failed to
  get a range. `src/game_hosts_gunnery.cpp:824` writes `best_range` only after the rank gate at
  `799`, which a category-0 gun can never pass, so the column is structurally 0 for any unit whose
  guns are all category 0 - regardless of `category_ranges[0]`, which `634` and `655` do fill.

## 9. Follow-up packets

1. **`plane_recon_contact_membership`** - replace the `IsKindOf(06h)` stand-in in
   `recon_contact_count_008053c0` with `00806480`'s twenty-two-id membership plus `IsKindOf(02h)`,
   and re-run `IJN01`. Owner: whoever holds `src/game_hosts_gunnery.cpp`. Coordinate with the
   `cc7-recon-slot` / `cc7-recon-finish` packets, which own `008053C0`'s area.
2. **`plane_motion_arm_binding`** - bind the eight `0x007ce040` dispatch rows to
   `bsp::run_plane_fixed_step_007ce040`, seed `unit+900h` from `007C6340`, and report the arm.
   Needs no new reconstruction; needs `src/game_hosts_units.cpp`.
3. **`plane_flight_core_law_body`** - `007DB680`'s `007DB760`-`007DC82A` and `007DC830`'s
   `007DCA17`-`007DCB47`. The blocker for any real plane motion. `gpt-6-astra`: the body is x87
   throughout.
4. **`plane_tick_effect_body`** - `007CE040`'s `007CE094`-`007CEC30` and
   `007CECBF`-`007CF172`: damage, fire, collision, sound and the `powerlost` / `explosion` /
   `Collided` effect strings. Overlaps the cockpit/camera and resource territory the peer
   orchestrators hold; read their contracts first.
5. **`plane_gun_trigger_producer`** - who writes `unit+9F8h`/`+9F9h`, the two bytes `007DC84F`
   copies into the flight controller as `ctl+4h`/`ctl+5h`, and whether anything but
   `00519520 BSP_PlanePilotView_BuildPlayerCommand` fires a `PLANEGUN`. Settles whether an AI
   plane can shoot at all.
6. **`recon_plane_class_14h`** - why `14h MReconPlane` has an allocator (`008091D0`), a
   constructor (`0074E0F0`) and a tick vtable (`00D0002C`) but is absent from `00806480`'s
   twenty-two scanned class ids.

## no_ghidra_function

`0074E210`, the free-flight gate, has no Ghidra function; the enclosing candidate Ghidra offers is
`0074E190 BSP_SensorCategory_Air`, which is a different routine. Bounds taken from the disk bytes:

| address | name | end_address | final instruction |
| --- | --- | --- | --- |
| `0074E210` | `BSP_PlaneControlMode_IsFreeFlight` | `0074E21C` | `0074E21C RET` (`c3`), followed by `cc cc cc` padding and the next entry at `0074E220` |

`007C6500 BSP_PlaneTickElement_AdvancePose` (`007C6500`-`007C675D`) also has no Ghidra function;
its bounds are `docs/PLANE_FLIGHT.md`'s, not re-derived here.

## Integration result: planes are contacts, and two bottlenecks remain

Step (1) of the prescription is implemented - the recon-contact stand-in in
`src/game_hosts_gunnery.cpp` admitted only `IsKindOf(06h)` ship bases, and now admits plane bases
too. Measured on `IJN01`, with per-gate counters added so a zero can be attributed rather than
guessed:

```
contacts considered=673920 side=133068 invisible=0 dead=2728
         kind=461922 admit_ship=31378 admit_plane=44824
```

**44824 plane contacts are admitted where there were none.** The `USN02` control is byte-identical
(`created=273 entity_impacts=167 water=96 expired=29 deaths=2 total_damage=16472.1`) with
`admit_plane=0` and `kind=0`, which also confirms that mission genuinely carries no aircraft.

**The per-category table does not move**: AAMACHINEGUN stays at 26 assignments and 0 shots, FLAK at
0. So the contact gate was necessary and is not sufficient, and there are two further bottlenecks,
both downstream:

1. **From 44824 plane contacts, AA takes only 26 assignments.** The gate is
   `score_candidate_00863990` - the category mask, the authored rank row, or the range. An AA
   machine gun's derived range is 960 m, so most of the approach may simply be out of reach, which
   would be faithful. Unmeasured either way.
2. **Those 26 assignments produce 10452 window refusals and no shot.** `arc_blocked` is 0, so it is
   the commanded angle and not the firing window - the same shape as the torpedo defect, where
   `009003DD` commands a hard `0.0f` vertical and `0085AB50` -> `007F6190` snaps the heading.

**A correction to my own reading.** I first judged this change a no-op from the per-category table
alone and hypothesised that `kUnitGunneryKindPlaneBase = 0x0F` was the wrong constant, because this
document cites the native scan as visiting plane leaves under `IsKindOf(02h)`. The counters refute
that: `0x0F` admits the planes. Acting on the hypothesis would have broken a working predicate.
