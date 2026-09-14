# The AA machine gun's commanded angles, and what `no_window` counts

Addresses: `00902920`, `00901C20`, `0085AD00`, `0085ABA0`, `007F5FC0`, `008FBCE0`, `0072C6A0`,
`008FDAF0`, `00438AA0`, `00414E10`, `00922E90`, `00961A68`. Constants `00D7A278` (FLT_MAX,
double), `00D7A248` (FLT_MAX, float), `00D7A280` (`0.5`), `00CF9058` (`9.81`), `00CEB5A8`
(`pi/4`), `00CE3958` (`2.0`), `00D7A264` / `00CE684C` (`+pi` / `-pi`).

Packet `cc7_aa_vertical_window`. Ghidra read-only: no rename, comment, prototype or save.

## 0. Verdict first

**The refusal is not the AA path's fault, and it is not a vertical-window refusal at all.**

Three separate claims, each established below:

1. **`no_window` is not an angle-refusal-while-aiming counter.** It is one tally per gun per
   tick of `gun_set_target_angles_0085aba0`'s return, and the host calls that
   **unconditionally**, including for a gun with no target, with the authored rest pair
   (`src/game_hosts_gunnery.cpp:1174-1175`, `1347-1354`). On IJN01 the AA column is
   dominated by **idle** guns. Proof: the PLANEGUN row reports `246000` refusals from
   `82` guns over a `3000`-tick run - `82 x 3000` exactly - with `0` assignments, and
   `angle_sets + refusals = 1385442 + 432558 = 1818000 = 606 guns x 3000 ticks` exactly.
   AAMACHINEGUN's `62952` is **7.1% of its 885000 gun-ticks**: an AA gun *accepts* its
   commanded pair on 92.9% of ticks.
2. **The authored AA windows allow aircraft engagement and the commanded vertical is nowhere
   near an edge.** Across all 1916 AAMACHINEGUN platforms in this installation, 1720 of 2335
   windows author `MinVertAngle = -5 deg` and 2235 author `MaxVertAngle` of 80/85/87/90 deg.
   The one AA engagement geometry IJN01 can produce commands a vertical of **+0.3 to +1.0 deg**.
3. **No AA gun on IJN01 is ever offered an aircraft.** All 31 enemy aircraft report
   `moved 0.00` for the whole 150 s mission; the closest one ever comes to an AA-carrying
   ship is **2950 m** horizontally, against an AAMACHINEGUN engagement range of **960 m**
   where the run log lets it be isolated. **The planes are the defect, not the AA path.**

The AA path does carry three genuine fidelity defects (section 5) and the `no_window` column
has a precise producer that is also a fidelity defect (section 6). None of them refuses an
angle in this mission's geometry, and none of them is why AA does not shoot.

## 1. Which bot drives an AA machine gun

`0072C6A0` fills six bot slots from the weapon descriptor sub-type at `[gun+3F4h]+80h`.
AAMACHINEGUN is `Function` category **1** (`docs/GUNNERY_TABLES.md`, row VA `00E0944C`,
store `00732857`). Sub-type 1 does **not** reach the AA flak bot:

| Slot | Gate | Class / tick | Site |
| --- | --- | --- | --- |
| `+390h` | `== 1`, gun `IsKindOf(22h)`, `00922E90(gun, 0Fh) == 0` | `AAGunnerBot` `008FE740` / **`00902920`** | `0072C736` |
| `+390h` | `== 1`, gun `IsKindOf(22h)`, `00922E90(gun, 0Fh) != 0` | `TailGunnerBot` `008FE9F0` / `008FFA20` | `0072C713` |
| `+394h` | `== 5` or `== 6` | `AAFlakBot` `008FEFD0` / `009030C0` | `0072C79E` |

`009031CF`, which the packet brief offered as the lead, is inside `009030C0`
(`BSP_GunBot_BallisticAimAndFireTick`, body `009030C0-0090341D`) - the **flak** bot, slot
`+394h`, sub-types 5 and 6. It is not on an AA machine gun's path. The split is confirmed by
the reconstruction already in the tree (`src/gun_bot_ticks.cpp:22-27`) and by
`docs/GUN_BOT_TICKS.md` section 2.

**And `009031CF` does not call `008FBB00`.** The brief describes it as "an AA flak bot's call
into the intercept solver `008FBB00` taking `V0` at `+50h`". The listing says
`009031cf: CALL 0x00901c20`, and so does `00903219`; a byte census of the whole flak-bot body
`009030C0..00903417` finds **no** `CALL 0x008fbb00` at all. `008FBB00` is the **torpedo**
bot's solver, reached only at `0090025E` inside `008FFF20`. The `+50h` (`V0`) against `+0E4h`
(`WaterTravelSpeed`) contrast the brief draws is real, but it separates **two different
solvers** - `00901C20` for the flak and AA gunner bots, `008FBB00` for the torpedo bot - not
two call sites into one. The AA gunner bot's own call at `00902A74` also passes `+50h`:
`00902920`'s pseudocode loads `uVar7 = *(undefined4 *)(iVar1 + 0x50)` from the weapon
descriptor `[[gun+3F8h]+34h]` immediately before it (read from pseudocode, not the listing).

`00922E90(this, kind)` walks the entity parent chain, so the split is "does this gun sit under
an owner of kind `0Fh`", i.e. an aircraft. **On IJN01 both arms are live**: of the 295
category-1 guns, **273 are ship mounts** (AAGunnerBot) and **22 are aircraft tail guns**
(TailGunnerBot) - one each on 21 `JudySpawn`/`JillSpawn` and on `Dauntless1`.

## 2. What `00902920` commands, instruction by instruction

Body `00902920-009030B2`, `__thiscall(bot)(float dt)`. `bot+5Ch` is the gun
(`docs/GUN_BOT_TICKS.md` section 3). The commanded pair reaches `0085ABA0` at `00902FB0`.

| Step | Site | Instruction evidence | Rule |
| --- | --- | --- | --- |
| 1 | `00902A65`, `00902A74` | `CALL 00901C20` | `BSP_GunBot_InterceptSolution` gives the world aim point |
| 2 | `00902ACB` | `CALL 00414E10` | derived affine inverse of the gun node `[gun+3CCh]`'s world matrix |
| 3 | `00902AD9` | `CALL 008FDAF0` | that direction becomes the pair `(h, v)` in the gun's frame; `008FDAF0` writes `out[0] = h`, `out[1] = v` |
| 4 | `00902B38`-`00902EF7` | - | the swinging-error model: `bot+64h`/`+68h` the live error pair, `+6Ch`/`+70h` their rates, `+60h` the remaining time |
| 5 | `00902F3E` | `SUB ESP,8` / `FSTP [ESP+4]` / `FLD [ESP+20h]` / `FSTP [ESP]` / `CALL 00438AA0` | `h = AddWrappedAngle(h, bot+64h)`; `00438AA0` ends `RET 0x8` (`00438B0B`), so it cleans its own arguments |
| 6 | `00902F58` | same shape, `FLD [EBP]` | `v = AddWrappedAngle(v, bot+68h)` |
| 7 | `00902F62`-`00902F76` | `FLD [ESP+18h]` / `FLDZ` / `FCOMIP` / `JBE 00902F7A` / `FMUL double ptr [00D7A280]` | **a negative `v` is multiplied by `0.5`** |
| 8 | `00902FA2`-`00902FB0` | `SUB ESP,8` / `FSTP [ESP+4]` (the ST0 from step 7) / `FLD [ESP+1Ch]` / `FSTP [ESP]` / `CALL 0085ABA0` | `0085ABA0(gun, horz = step 5's result, vert = step 7's result)` |

Register/stack provenance for step 8, since the two slots are what the claim rests on. Let
`E` be `ESP` at `00902F2E`. `00438AA0` is `RET 8`, so after each call `ESP` is back at `E`:
`00902F43 FSTP [ESP+18h]` writes `E+18h` (the horizontal) and `00902F5D FSTP [ESP+1Ch]` writes
`E+1Ch` (the vertical). `00902F61 POP EDI` leaves `ESP = E+4`, so `00902F62 FLD [ESP+18h]`
reads `E+1Ch` - the **vertical** - which is what step 7 halves. `00902FA2 SUB ESP,8` leaves
`ESP = E-4`, so `00902FA9 FLD [ESP+1Ch]` reads `E+18h` - the **horizontal** - and it becomes
argument 0. The vertical, still on the FPU stack from step 7, becomes argument 1.

Argument order of `0085ABA0` cross-checked against `0085AD00`, which pushes
`[platform+94h]` as argument 0 and `[platform+90h]` as argument 1, and against
`src/vehicle_class_fields.cpp:701-709`, whose reader puts `RestAngles[1]` into `+94h`. A
rest pose of `{ DEG(-90), DEG(15) }` is only physical as (horizontal, vertical), so
argument 0 is the horizontal.

### There is no gravity term anywhere on this path

Both bodies were dumped in full from the listing and scanned:

| Function | Extent dumped | `FSQRT` | `00CF9058` (`9.81`) | `00CEB5A8` (`pi/4`) |
| --- | --- | --- | --- | --- |
| `00902920` | `00902920`..`009030AD`, 552 instructions | none | none | none |
| `00901C20` | `00901C20`..`00902277`, 421 instructions | none | none | none |

`00901C20`'s only float constants are `00CE3958` (`2.0`, the speed floor), `00CE3D78`,
`00CE47A0`, `00CEB4B8` (the time-of-flight clamp), `00D7A238`, `00D7A23C`, `00D7A24C`,
`00D7A280`, `00D7A328`. Its ledger comment, from packet `cc2_gun_bot_ticks`, already reads
"a closed-form lead solution with no gravity and no loop"; this packet confirms it from the
whole listing rather than the head.

**So the AA gunner bot points at the lead point directly.** It adds no superelevation.

## 3. The authored windows in this installation

Files read (**never modified**):

| File | Size | mtime |
| --- | --- | --- |
| `scripts/datatables/autoload/vehicleclasses.lua` | 3349364 | **2026-05-09 21:52:13** |
| `scripts/datatables/classtables/arcade/deviceclasses.lua` | 433194 | **2026-05-09 22:37:38** |
| `scripts/datatables/classtables/arcade/bulletclasses.lua` | 75048 | **2026-05-09 23:04:46** |

`autoload/deviceclasses.lua` selects `ArcadeTable` unless `GameMode == 1`, so the arcade
table is the one a default run reads. This installation is modded (BSPRM/AlterBSP) against an
untouched 2024-07-13 datatables bulk; every number below is **this installation's**, not retail.

**78 device classes** carry `Function = "AAMACHINEGUN"`, and **all 78** author
`HorzRotSpeed = VertRotSpeed = 4.0`. Neither of `0085ABA0`'s two rate gates (`0085ACA5`,
`0085ACB9`) can therefore refuse an AA mount.

**1916 platforms** in `vehicleclasses.lua` point a `Gun[1]` at one of those devices, carrying
**2335 windows** between them:

| `MinVertAngle` | count | | `MaxVertAngle` | count |
| --- | --- | --- | --- | --- |
| `-5 deg` | **1720** | | `80 deg` | **1005** |
| `0 deg` | 192 | | `87 deg` | 630 |
| `-10 deg` | 76 | | `85 deg` | 560 |
| `-2 deg` | 72 | | `90 deg` | 40 |
| everything else (-90..+35) | 275 | | everything else (0..70) | 100 |

**An AA mount in this installation elevates to 80-90 degrees.** The data is not the
constraint; a window that admits 87 degrees admits any aircraft.

Note on parsing, because it changed the answer: the windows are emitted with their keys in
**alphabetical** order - `MaxHorzAngle`, `MaxVertAngle`, `MinHorzAngle`, `MinVertAngle` - and
angles appear both as `DEG(80)` and as the bare radian literal `1.396263`. `RestAngles` has
two authored forms, `= { DEG(0),DEG(10) }` on one line and a multi-line `[1]`/`[2]` table.
A first pass that assumed Min-before-Max and the single-line rest form produced a window
census that was wrong in both directions; the numbers above come from a brace-depth walk
(`local/aa_windows2.py`, `local/aa_rest_census.py`).

### Inverted horizontal windows are faithful, not a host bug

80 of the 2335 AA windows author `MinHorzAngle > MaxHorzAngle` (for example
`VehicleClass[6]` platform 2, `h[165, -75]`). The host's `gun_arc_contains_007f5fc0` tests
`min <= h && h <= max`, which no `h` satisfies, so those windows are dead in the host. **They
are dead in the native too.** `007F5FC0 BSP_GunPlatform_AnglesInTraverseWindow` (body
`007F5FC0-007F609E`) clamps the horizontal into `[-pi, +pi]` (`00D7A264` / `00CE684C`,
`007F601F`-`007F6037`) and then runs four one-sided comparisons with an epsilon:

```
007f603f FADD ST0,ST5      ; h + eps
007f6041 FLD [EDX+4]       ; min_horz     007f604a JC  -> fail
007f604c FSUB ST0,ST4      ; h - eps
007f604e FLD [EDX+8]       ; max_horz     007f6055 JC  -> fail
007f605b FLD [EDX+0Ch]     ; min_vert     007f6064 JC  -> fail
007f606a FLD [EDX+10h]     ; max_vert     007f6071 JC  -> fail
```

A plain four-sided box with **no wrap-around**. This is a **negative result**: do not
"fix" the host to wrap an inverted window. Authors who need the stern sector write it as two
explicit halves, as `Oglala` platform 5 does with `h[25, 180]` and `h[-180, -25]`.

## 4. Is the target even airborne? No - and it never can be

From the IJN01 run (`--frames 3200 --press-start-frame 30 --menu-select IJN01
--mission-frames 3000 --mission-frame-seconds 0.05`, exit 0):

- **31 enemy aircraft**, every one reporting `moved 0.00` over the whole 150 s: 8 `A7M` at
  `z = 3500..3800`, 9 `JudySpawn` at `z = 4000+`, 12 `JillSpawn` at `z = 3000+`. They never
  take off, never move, and never take or deal damage.
- The **closest** an enemy aircraft ever comes to an AA-carrying ship is **2950 m**
  horizontally (`LST1` -> `JillSpawn12`). The `nearest` column is a 3-D distance between aim
  points (`src/game_hosts_gunnery.cpp:807-822`) and reports `4235 m` for `LST1`, so the
  aircraft are also about 3 km up.
- The AAMACHINEGUN engagement range, isolated on the four units that carry **only**
  category 1 (`Vestal 1:7`, `Neosho 1:7`, `Medusa 1:7`, `B-17 1:6`), is **960 m**. The
  largest `range` column on any AA-carrying ship is 3000 m and that belongs to `Lexington`'s
  HEAVYARTILLERY, not to its AA.

**2950 m against 960 m. No AA gun on this mission can ever be offered an aircraft.** The
sibling `plane_unit_tick` packet owns the reason the aircraft never fly; this packet's part
of the answer is that until they do, the AA path has nothing to shoot at.

### The only AA engagement IJN01 can produce, and its commanded vertical

The only enemies inside any AA gun's reach are the two Ko-hyoteki midget submarines `No18`
and `No19`. Three ships have one inside 1600 m: `Cassin` (966 m), `Downes` (1126 m) and
`Oglala` (1277 m). `Oglala` carries four AA mounts, so take it as the worked case.

Run-log facts: `Oglala` is `VehicleClass[234]` at `x = 500.0, z = -3300.0`, heading
`0.4886 rad`; `No18` is `VehicleClass[83]` at `x = -160.0, z = -4393.7`. Authored `Height` is
`5.0` for `Oglala` and `4.0` for the Minisub, and `unit_aim_point` raises both by their own
`Height` (`src/game_hosts_gunnery.cpp:201-205`), so the segment falls by 1 m over 1277 m:

```
horizontal 1277.4 m,  dy -1.00 m,  slant 1277.4 m
direct line  asin(dot(unit_delta, up))   = -0.045 deg
+ gravity pre-estimate, V0 = 600 / 800 / 1000 m/s:  +1.00 / +0.56 / +0.36 deg
commanded want_vert                      = +0.95 / +0.52 / +0.31 deg
```

Against `Oglala`'s four AA platforms:

| Platform | Device | Windows |
| --- | --- | --- |
| 2 `AA right` | 42 | `h[0, 170]  v[-5, 85]` |
| 3 `AA left` | 42 | `h[-170, 0]  v[-5, 85]` |
| 4 `AA brutal` | 40 | `h[-110, 0]  v[-2, 80]` and `h[0, 110]  v[-2, 80]` |
| 5 `AA high` | 40 | `h[25, 180]  v[-5, 85]` and `h[-180, -25]  v[-5, 85]` |

**The commanded vertical clears every one of those windows by at least 1 degree at the bottom
and 79 degrees at the top.** The vertical axis is not what refuses, in the only AA geometry
this mission has.

The horizontal convention was calibrated against the run rather than assumed. With
`forward = (sin h, 0, cos h)` and `right = (cos h, 0, -sin h)`, `Cassin` (at `442.4, -3637.9`,
heading `0.5498`) firing at `No18` computes `want_horz = 187.1 deg` and a slant of `966.5 m`;
the run log's first-shot line reports `horz 187.4 deg, range 970 m`. The same convention puts
`Oglala`'s target at `-176.9 deg` (`No18`) and `+179.0 deg` (`No19`), both inside platform 5's
windows.

## 5. The divergence from `src/game_hosts_gunnery.cpp`

Three, at the `want_horz` / `want_vert` site. None of them refuses an angle here; all three are
fidelity defects that will matter the moment an aircraft is in reach.

### 5.1 The bot split is computed and thrown away

`src/game_hosts_gunnery.cpp:1170-1172` calls `gun_bot_slots_for_subtype_0072c6a0(gun.category,
true, false)` and immediately writes `(void)slots;`. Two consequences: the `owner_of_kind_0f`
argument is hard-coded `false`, so the **22 aircraft tail guns on IJN01 are driven as if they
were ship AA mounts** when `0072C713` gives them `TailGunnerBot` / `008FFA20`; and the
selected class never reaches the aim arm, which runs one generic path for every category
except torpedo.

### 5.2 The host superelevates where `00902920` does not

`src/game_hosts_gunnery.cpp:1239` takes `else if (gun.muzzle_speed > 0.0f)` for **every**
non-torpedo category, which is `006DF520`'s **artillery** pre-estimate:

```
s     = distance * 9.81 / v^2                 006DF8BF
pitch = min(asin(s) * 0.5, pi/4)              00CEB5A8
```

and then replaces `pitch` with `00955630`'s ballistic arc minus the direct line
(`1281-1290`), so `want_vert = asin(dot(unit_delta, up)) + pitch`. Section 2 shows
`00902920` and `00901C20` contain no gravity constant, no `FSQRT` and no `pi/4` clamp
anywhere in their 552 and 421 instructions. **For category 1 the elevation must be the direct
line to the lead point, with no ballistic term.** At 1277 m the host's extra term is
`+0.3` to `+1.0 deg`; at an AA mount's real working range against a diving aircraft it is
larger, and it is not what the native commands.

`arc_unsolved = 0` for the whole run, so the `s > 1` refusal in that arm never fires on IJN01
and is not masking anything here.

### 5.3 A negative vertical is not halved

`00902F62`-`00902F76` multiplies a negative commanded vertical by `[00D7A280] = 0.5` before
`0085ABA0`. The host has no such step. With `MinVertAngle = -5 deg` on 1720 of 2335 windows,
this is the difference between a depression of `-8 deg` being refused and `-4 deg` being
accepted, so it will change outcomes as soon as an AA mount is asked to look down.

A fourth difference is deliberate and is **not** proposed as a fix: the host models none of
the `bot+64h`/`+68h` swinging-error pair (`00902B38`-`00902EF7`), which is RNG-driven. A
deterministic host is the right call for a validation run.

## 6. Where `no_window` actually comes from, and the one prescription worth wiring

The host calls `gun_set_target_angles_0085aba0` once per gun per tick with no target gate
(`1347`), seeding `want_horz` / `want_vert` from `gun.rest_horz` / `gun.rest_vert` (`1174-1175`),
and counts a refusal at `1353`. A gun whose **rest pose** is outside its own windows therefore
refuses on every tick of the mission, with no target anywhere near it. The IJN01 column:

| Category | guns | ticks | refusals | refusals / 3000 |
| --- | --- | --- | --- | --- |
| PLANEGUN | 82 | 3000 | 246000 | **82.0 - every gun, every tick** |
| AAMACHINEGUN | 295 | 3000 | 62952 | 21.0 |
| HEAVYARTILLERY | 45 | 3000 | 42774 | 14.3 |
| LIGHTARTILLERYFLAK | 72 | 3000 | 26832 | 8.9 |
| DEPTHCHARGE | 10 | 3000 | 30000 | **10.0 - every gun, every tick** |
| CATAPULT | 4 | 3000 | 12000 | **4.0 - every gun, every tick** |
| LIGHTARTILLERY / MEDIUMARTILLERY | 13 / 13 | 3000 | 0 / 0 | 0 |

The native has no such counter and does not make the call. `008FBCE0`'s idle timer reaches
the rest arm **once** per idle period - `008FBD4B` pins `bot+54h` to `00D7A248` before
`008FBD59` tests `vtable[5Ch](22h)` and `008FBD61` calls `0085AD00` - and `0085AD00` itself
refuses to command anything when the platform has no authored rest angle:

```
0085ad33 FLD [ECX+94h]              ; the rest horizontal
0085ad46 FLD double ptr [00D7A278]  ; FLT_MAX
0085ad4e FUCOMIP / LAHF / TEST AH,0x44
0085ad56 JNP 0085ad74               ; equal -> return, command nothing
0085ad58 FLD [ECX+90h]              ; otherwise 0085ABA0(+94h, +90h)
```

`src/vehicle_class_fields.cpp:701-709` is the producer: `rest_angle_a` is seeded to
`FLT_MAX` (`00D7A248`) and the reader returns early when the `RestAngles` key is nil, so
`+94h` stays `FLT_MAX`. `docs/GUN_AIMING.md:178-181` already records the rule -
"a platform without `RestAngles` never re-centres" - and the gunnery host does not apply it:
`src/game_hosts_gunnery.cpp:491-492` defaults both rest angles to `0.0f`.

**49 of the 1916 AA platforms author no `RestAngles`, and 36 of those have windows that
exclude dead-ahead** (`VehicleClass[11]` platforms 22-25 and 28-29, `h[10, 170]` or
`h[-170, -10]`; `VehicleClass[6]` platform 2; and 30 more). Every instance of those on a
mission refuses forever at `(0, 0)`.

### Prescription (the integrator owns the host; this packet publishes no module)

1. **Carry the `FLT_MAX` sentinel into the gunnery host.** `gun.rest_horz` must come back as
   `FLT_MAX` - or the row must carry a `has_rest_angles` flag - when `p.RestAngles` is nil,
   matching `src/vehicle_class_fields.cpp:701`. This is a change at
   `src/game_hosts_gunnery.cpp:326-327` and `491-492`, not in `bsp::` code.
2. **Do not command the rest pair on a gun with no target on every tick.** Mirror
   `0085AD00`: if the gun has no target, skip `gun_set_target_angles_0085aba0` when the rest
   horizontal is the sentinel, and otherwise command it once on the idle-timer arm rather
   than per tick. Either change alone removes the whole idle contribution to `no_window` and
   makes the column mean what its name says.
3. **Branch the aim arm on the bot class** at `src/game_hosts_gunnery.cpp:1170-1239`. Use the
   `GunBotSlotAssignment` that is already computed instead of discarding it, and pass the real
   `owner_of_kind_0f` so an aircraft tail gun takes `008FFA20` rather than the ship path.
4. **For `GunBotClass::kAAGunner`, drop the gravity term**: `want_vert = asin(dot(unit_delta,
   up))` with no `pitch`, and no `00955630` call. Then apply `if (want_vert < 0.0f)
   want_vert *= 0.5f;` (`00902F6C`, `00D7A280`) before `gun_set_target_angles_0085aba0`.
5. **Change no commanded angle to move a counter.** Items 3 and 4 are the native's rule; item
   1 and 2 are the native's idle behaviour. None of them will make AAMACHINEGUN shoot on
   IJN01, because of section 4.

No header or source is published. Every rule above is either a one-line change inside
`src/game_hosts_gunnery.cpp`, which the integrator owns, or already declared -
`gun_set_target_angles_0085aba0` and `gun_traverse_allowed_007f5fc0` in
`include/bsp/gun_aiming.hpp`, `gun_bot_slots_for_subtype_0072c6a0` and `GunBotClass` in
`include/bsp/gun_bot_ticks.hpp`, `rest_angle_a` in `include/bsp/vehicle_class_fields.hpp`.
Publishing a module here would redeclare recovered interfaces for no new pure rule.

## 7. What is proven and what is assumed

**Proven from the listing (whole-body dumps, not heads):**

- `00902920` and `00901C20` contain no `FSQRT`, no `00CF9058` and no `00CEB5A8`, over
  `00902920..009030AD` (552 instructions) and `00901C20..00902277` (421 instructions).
- `00902F6C` halves a negative commanded vertical with `[00D7A280]`.
- `00902FB0` calls `0085ABA0` with argument 0 = the step-5 horizontal and argument 1 = the
  step-7 vertical; the stack slots were resolved from `00438AA0`'s `RET 0x8` at `00438B0B`
  and the `POP EDI` at `00902F61`, not from the decompiler.
- `007F5FC0` is a four-sided box test with a `[-pi, +pi]` clamp and no wrap-around.
- `0085AD00` returns without calling `0085ABA0` when `[platform+94h] == FLT_MAX`, and
  `008FBD4B`/`008FBD61` reach it once per idle period.

**Proven from a run** (`local/ijn01.log`, exit 0, 3000 mission ticks at 0.05 s):
every number in sections 0, 4 and 6, including `angle_sets + refusals = 606 x 3000` exactly
and PLANEGUN's `82 x 3000`.

**Proven from the authored data** (parsed by brace depth, `local/aa_windows2.py`,
`local/aa_rest_census.py`, `local/aa_oglala.py`, `local/aa_plane_reach.py`): the 78 device
classes, the 1916 platforms, the 2335 windows and their bounds, the 49/36 rest-angle counts,
the `Oglala` and `Minisub` geometry.

**Assumed or partial, labelled as such:**

- `00902920` **coverage: partial.** The error model `00902B38`-`00902EF7` (steps 4 above,
  steps 2-7 of `docs/GUN_BOT_TICKS.md` section 6.4) was read from pseudocode only; this packet
  filtered the listing for the gravity constants and for the `00902F3E`-`00902FB0` command
  tail, not for the error integration. The claim "no gravity" is a whole-body byte census and
  does not depend on that gap.
- `00901C20` **coverage: census only.** Its body was scanned for gravity constants, `FSQRT`
  and its float-constant set. Its solve was not re-derived; the contract is
  `docs/GUN_BOT_TICKS.md`'s and the existing ledger comment's.
- **Which two AA guns held a target is not established.** The run reports 148 AA assignments,
  which at the 2.000 s director think time over 150 s is exactly `2 x 74`, so two AA guns
  were re-assigned every think cycle. The log prints per-gun rows only for guns with
  `shots > 0`, so those two guns cannot be named from it, and `62952 = 20 x 3000 + 2952` does
  not resolve cleanly into idle and targeted ticks. **The claim that AA's refusals are
  dominated by idle guns is proven by the exact `82 x 3000` / `10 x 3000` / `4 x 3000` rows
  and by AA's 92.9% acceptance rate; the residual 2952 is not attributed.** Attributing it
  needs a per-gun counter in the host, which this packet does not own.
- `0085ABA0`'s argument order is settled by three independent readings (section 2), not by one.
- `008633D0` / `00863990` / `00864FE0` admission and `006E9890` range derivation are read as
  **contracts** from merged packets; this packet did not re-derive them. The 960 m AA range is
  an observation from the run log, not a derivation.

**Refuted:** the packet brief's framing that "AA guns are now admitted a target and then
refused on the commanded angle, every time", and `docs/BULLET_ENGAGEMENT_RANGE.md:527-529`'s
"26 assignments and `no_window = 10452`, so the targets sit outside the guns' vertical
windows". An AA gun accepts its commanded pair on 92.9% of ticks, `no_window` counts idle
guns, and the AA vertical is never within 79 degrees of an upper window edge on this mission.
Also refuted by this packet's own first pass: the AA windows are **not** a low-elevation
dead zone (1720 of 2335 reach 80-90 degrees), and the 80 inverted horizontal windows are
**not** a host bug.

## 8. Follow-up packets

1. **`plane_unit_tick`** (already open) - the 31 aircraft never move. Until one does, no
   AA measurement on IJN01 means anything. Re-run this packet's checks after it lands.
2. **`gun_idle_rest_arm`** - wire items 1 and 2 of section 6, then re-read every category's
   `no_window`. The PLANEGUN `82 x 3000`, DEPTHCHARGE `10 x 3000` and CATAPULT `4 x 3000`
   rows are *all* idle-rest artefacts and should go to zero or near it; whatever survives is
   a real refusal and is worth a packet of its own.
3. **`tail_gunner_bot_008ffa20`** - 22 of IJN01's 295 category-1 guns are aircraft tail guns
   taking `0072C713`'s arm. `008FFA20`'s rule is section 6.1 of `docs/GUN_BOT_TICKS.md`,
   marked complete; it needs a host branch, not new reading.
4. **The 80 inverted horizontal windows** need no code change (section 3), but the 36 AA
   platforms with no `RestAngles` and a dead-ahead gap are worth reporting to whoever owns the
   installation's mod tables: in the native those mounts simply never re-centre.

## Correction to `docs/BULLET_ENGAGEMENT_RANGE.md` (packet `cc7_bullet_engagement_range_kinds`)

That document's "Integration result" section, at the bullet beginning "**AA machine guns are
admitted but never fire**", concludes from `no_window = 10452` that "the targets sit outside
the guns' vertical windows" and that this is "the same shape of defect the torpedoes had".
**Both clauses are wrong.** `no_window` is a per-gun-per-tick counter that includes guns with
no target at all, so it cannot support a statement about targets; and the AA vertical windows
reach 80-90 degrees while the commanded vertical on this mission is under 1 degree. The
torpedo defect was a real angle refusal (`MinVertAngle == MaxVertAngle == 0` against a
computed depression); this is not. The document's other IJN01 observations - the inert `A7M`
fighters, and FLAK/TORPEDO/DEPTHCHARGE needing their own admission traces - stand.

## Measured confirmation from the recon-contact fix (integration)

This packet argued from distance that no aircraft is reachable; the recon-contact fix measured it.
Admitting plane bases at the kind gate added **44824 plane contacts** on IJN01 and moved
AAMACHINEGUN's assignment count by **zero** - the same 26 this packet reproduced at 522 mission
ticks with the kind gate still shut. The binding constraint is the **range test downstream** of it,
2950 m horizontal to the nearest aircraft against a 960 m derived AA range, not the kind gate. The
recon fix is correct against the native scan and is behaviourally inert on this mission.

The `angle_sets + refusals == guns x mission_ticks` test was run on the integrator's tree and passes
to the digit: `230442 + 72558 = 303000 = 606 x 500`.

### Targeted-tick counters

Four counters were added to split the ticks on which a gun actually held a target out of the
per-gun-per-tick tallies. On the same run:

```
targeted refusals=12058  no_accept=12058  no_settle=1828  no_window=0
```

**These are global, not per-category**, so they do not test this packet's category-1 prediction of
roughly 450 and its falsifier is untouched. What they do show is the idle-domination reading
generalised: **12058 of 72558 refusals held a target, so 83% of all refusals are idle gun
instances.** `no_accept` is *identical* to `angle_refusals_targeted`, but that is **definitional, not a
confirmation**: `accepted` is the return value of the very call that increments `angle_refusals`, so
on a targeted tick the two predicates are the same event. An earlier revision of this section
presented the match as evidence that the reasoning held; it is a tautology and carries no
information beyond `angle_refusals_targeted`. If the instrumentation is ever trimmed, that is the
counter to drop. `no_window = 0` against per-category `arc_blocked` of 222 and 952 satisfies the
`want_fire_no_window <= arc_blocked` self-test, and both are 0 for category 1 as predicted.

### Ordering constraint on the prescription

**Items 3 and 4 must land together.** Dropping the artillery gravity pre-estimate for category 1
without first branching on the bot class would strip superelevation from *every* non-torpedo
category, including the HEAVYARTILLERY and LIGHTARTILLERYFLAK that are currently the only guns
landing hits on this mission. This is the one ordering in the prescription that is not safe to
split, and it is the same half-applied failure that made the torpedo solver-speed fix read as a
regression before its companion steps landed.

### The ceiling that settles the falsifier without another run

`set_bot_fire_target` fires once per gun per director think cycle, and the logged think time is
2.000 s. The 500-tick run is 25 s, so 12.5 cycles, and 26 assignments over 12.5 cycles is **2.08 AA
gun-instances holding a target at any moment**. The 3000-tick run agrees independently: 148 over 75
cycles is **1.97**. So the absolute ceiling on category 1's targeted refusals is `2 x 500 = 1000`,
and that assumes both guns refuse on *every* tick they hold a target.

**1000 against a column of 10452 is 9.6%**, so **at least 90.4% of AA's `no_window` is idle gun
instances**. The same ceiling at 3000 ticks gives 6000 against 62952, the same share. This packet's
falsifier asked whether category 1's targeted refusals could be anywhere near 10452; they cannot
reach a tenth of it, so the idle-domination reading stands and the global 12058 was correctly not
treated as a refutation.

The global figure corroborates it. Only three categories took any assignment, so only three can
contribute targeted refusals at all - PLANEGUN's 41000, TORPEDO's, DEPTHCHARGE's and CATAPULT's are
idle by construction. With AA capped at 1000, the 12058 is almost entirely HEAVYARTILLERY and
LIGHTARTILLERYFLAK, which hold targets continuously and have mounts the superstructure genuinely
blocks - which is also why those are the only two categories with non-zero `arc_blocked`.

**The one genuinely new number is `no_settle = 1828.`** With the 1174 `arc_blocked` (222 + 952) it
decomposes **3002 targeted-and-accepted ticks that still produced no shot**. That is the thread for
whoever asks why HEAVYARTILLERY lands 29 shots rather than hundreds.
