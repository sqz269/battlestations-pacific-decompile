# The torpedo approach update (packet `cc8_torpedo_approach_update`)

Addresses: `009D3420` (`009D3420`-`009D3E3F`), `009D4A70` (`009D4A70`-`009D4B2F`), `009D3210`,
`009D15F0` (`009D15F0`-`009D2377`), `009D1500` (`009D1500`-`009D15C0`), `009D1360`
(`009D1360`-`009D14F3`), `009D0380` (`009D0380`-`009D066F`), `0099ACD0` (`0099AF53`-`0099AFAF`),
callees `009FADA0`, `007B93F0`, `00414DB0`, `0041BC20`, `00903BC0`, `007DF360`, `00414C60`,
`007BCC80`, `007BCFA0`, `00903860`.

`docs/TORPEDO_TASK_ARM.md` closed with "every input of the torpedo state machine is an output of
`009D3420`, a 2592-byte routine this packet did not read". It is read here. The one-line answer:
**`009D3420` is the torpedo run-in planner.** It measures the range and bearing to the target,
latches an in-range flag with hysteresis, and, on a timer, scans the 36 compass sectors around the
target for a terrain-free attack heading and stores the turn that reaches the nearest clear one.
It does **not** write the engage distance: that is `009D4A70`, the task's `+54h` cruise profile.

## (1) `009D3420` in full

`void __thiscall(BotApproachTorpedo* approach, float dt)`, `RET 4`. `ESI` = the approach object
throughout. The caller `009D486F` is the arm's step 2 with `ECX = task+3F8h`; the second caller is
`009D4A70` at its tail with `dt = 0`. The body ends `ADD ESP,0x8c` / `RET 4` at `009D3E3C`.

The approach object is embedded in the task at `task+3F8h`, so every offset below is a task offset
minus `3F8h`. `approach+0Ch` is the pilot control block `ctl` (`unit+9D4h`), `approach+4h` the
owner unit, `approach+CCh` the target entity, `approach+18h` the command block.

### Every field the routine writes

| offset | address | rule |
| --- | --- | --- |
| `+78h` | `009D3440` | `0.0f` when `ctl->+3ADh == 0`, the motion controller's dirty flag |
| `+80h` | `009D347A` | clamped down to `ctl->+39Ch` when it exceeds it |
| `+7Ch` | `009D3462`, `009D3482` | rescaled by the same clamp: `+7Ch = (+7Ch / old +80h) * ctl->+39Ch`, so the ratio between the two speeds survives |
| `+74h` | `009D34A9` | `ctl->+398h` when that is below the **double** `100.0` at `00D7A220` |
| `+132h` | `009D34CD` | `007B93F0(ECX = approach+4h, 0)`: does the aircraft still carry torpedo ordnance |
| `+90h` | `009D357E` | the 2D range from the unit to the target point; `0.0f` when the squared range is at or under the double `1e-10` at `00CE3820`, else `00BF7030` (sqrt) |
| `+94h` | `009D35C0` | `pi/2 - atan2(dz, dx)` (`00CE3830`, `00BF701A`), wrapped by `2*pi` (`00CE3828`) |
| `+131h` | `009D361E`, `009D3E21` | the in-range latch, below |
| `+A9h` | `009D36BC`, `009D36F3` | over-land: `0041BC20(ctl->+34Ch, unit.x, unit.z) > 10.0f` (`00CE38B8`), then overwritten outright by `007DF360(ECX = unit+C50h, target, target point)` when `unit+C50h` is set |
| `+30h`..`+53h` | `009D38A6`, `009D39E0` | the 36 sector flags |
| `+54h` | `009D39FD` | a wrap copy of `+30h` |
| `+58h` | `009D380E` and the loop | the count of clear sectors |
| `+ACh`, `+B0h` | `009D3A0C`, `009D3A1A` | the target position the plan was built for |
| `+64h` | `009D3BC8` | the sector the aircraft sits in, as seen from the target |
| `+5Ch` | `009D3C50`, `009D3C79` | the signed turn offset in radians |
| `+68h`, `+6Ch` | `009D3C53`, `009D3C56`, `009D3C85`, `009D3C88` | the forward and backward sector gaps |
| `+AAh` | `009D3C64`, `009D3C6E`, `009D3C7E` | no clear sector to turn to |
| `+F8h` | `009D3D2F`, `009D3D52`, `009D3D65` | the engagement estimate, clamped to `[0, 30]` |
| `+12Ch` | `009D3682`, `009D3795` | the replan countdown |
| `+134h` | `009D3E32` | `+= dt`, every path including both early-outs |

**Correction to the packet brief.** The brief lists `+98h` among this routine's writes. It is not:
the full write census of `[ESI + disp]` over the 649-instruction listing gives the twenty-six sites
above and no store to `+98h`. `009D3D12` **reads** `+98h` as the bias of the engagement estimate.
Its writer is `009D1360` at `009D14E7`. `+8Ch` likewise is read four times (`009D35FF`, `009D3774`,
`009D37FE`, `009D3E01`) and never written; see section (2).

### The in-range latch `+131h` (`task+529h`), the attackrun to aim flag

`009D35D4`-`009D361E` and `009D3770`-`009D3784`.

```
if (!latched)          latched = range < approach->+8Ch;
else if (ctl->+369h && [00E17BF2]) latched = 1;            /* never releases */
else                   latched = range < approach->+8Ch * 1.1;   /* 00CE3DF0 */
```

A plain hysteresis band: it closes at the engage distance and opens at 1.1 times it. `[00E17BF2]`
is a byte global, `0` in the image on disk.

### `+132h` (`task+52Ah`), the aim to goaway flag

`009D34C5`: `007B93F0` with `ECX = approach+4h` and the pushed argument `0`. It is simply **does
the aircraft still carry torpedo ordnance (kind `2Bh`)**. That settles what the transition rule's
step 11 and the entry chooser's row 2 in `docs/TORPEDO_TASK_ARM.md` mean: the aim state runs while
there is still a torpedo on the rack, and an empty aircraft goes to `done`.

### The altitude pair `+74h`/`+78h` and the speed pair `+7Ch`/`+80h`

Both come from the pilot control block, not from the target's geometry. `+74h` takes `ctl->+398h`,
which `009D4A70` fills from `Pilot/Torpedo`'s second altitude, and `+78h` is a margin the motion
controller's dirty flag clears. `+7Ch` and `+80h` are two commanded speeds selected by age, not by
range: `009D3C99`, `009D1509` and the arm's `009D4874` all test `+134h` against the double `15.0`
at `00CF3F20` and take `+7Ch` at or after fifteen seconds, `+80h` before.

**Correction to `docs/TORPEDO_TASK_ARM.md` section (1) step 4.** That table calls `+78h + +74h`
"the moveto ranges". The aim tick reads the same sum at `009D1631` as a floor on the **commanded
altitude**, alongside `[00CE3850] = 5.0` over water, `[00CE38C8] = 30.0` over land and the ground
height plus `5.0`. For this class the pair is an altitude band. The arm's own call is unchanged.

### `+134h`, the approach clock

`009D3E32`: `+134h += dt` on every path, including the no-target early-out. `009D05C1` resets it.
It is the only input of the speed switch, so the aircraft flies `+80h` for the first fifteen
seconds of an approach and `+7Ch` after.

### The sector scan, `009D3798`-`009D3A70`

This is the body of the routine and the part that depends on terrain rather than on tuning.

1. `009D3640`-`009D3682`: the replan timer. Below `dt`, `+12Ch -= dt` and the plan stands. At or
   above, `+12Ch += +128h - dt` and the plan is rebuilt this tick.
2. `009D374C`: with `ctl->+34Ch` (the terrain sampler) null the scan is skipped entirely.
3. `009D3752`-`009D37A9`: the plan also stands while the target has moved under the double `120.0`
   at `00D1F3F8` in both x and z since `+ACh`/`+B0h`.
4. `009D37EE`-`009D3808`: the scan cap is `+88h * 1.2` (`00CEC160`), held down by `+8Ch`.
5. `009D386D`: the ground under the target, floored at `20.0` (`00CE3930`) when it is
   below the double `20.0` at `00CE3D88`.
6. `009D38A6`-`009D3A2A`: for each of 36 sectors, bearing `pi/2 - i*pi/18` wrapped by `2*pi`, march
   out from `420.0` (`00D2143C`) in steps of `120.0`. Past the double `450.0` at `00D1F4C0` the
   test is a terrain slope, `(height - ground_at_target) / (radius - 400.0)` against a limit that
   starts at `0.06` (`00CF0A2C`) and grows by a step that itself grows by `0.02` (`00D7A2F8`);
   inside it the test is `00903BC0` at `009D39D3` between the target point and the probe. A failure clears the
   sector's byte, decrements `+58h` and stops that sector.
7. `009D3A3E`-`009D3A70`: with no clear sector the start radius drops by the double `80.0`
   (`00CF1440`), the slope step resets to `0.015` (`00D21438`) and the whole scan runs again, down
   to the floor at the double `50.0` (`00CE3938`).
8. `009D3B93`-`009D3C56`: the bearing from the target back to the aircraft gives the home sector
   (`00BF7420` at `009D3BC0` truncates, `009D3BCE` clamps to `23h`). Walk up from `home+1` and down from `home`
   to the nearest clear sector; the cheaper walk wins. The forward count divides by the **double
   `-18.0`** at `00D21430` and the backward count by `+18.0` at `00CEE930`, so `+5Ch` is a signed
   turn in radians.

`+AAh` goes up when every sector is clear, when none is, or when both walks are zero.

The aim tick consumes exactly this: `009D1D16` writes the commanded heading as the bearing `+94h`
plus the turn offset `+5Ch`. That is the whole point of the scan.

### The engagement estimate `+F8h`, `009D3C93`-`009D3D6D`

```
speed = (+134h >= 15.0) ? +7Ch : +80h;
d = max(0, +90h - speed);
eta = +98h + 2*d / (unit->vtable[38h]() + +70h);
+F8h = eta < 0 ? 0 : (eta > 30.0 ? 30.0 : eta);
```

`[00CE7630]` is the double `30.0` and `[00CE38C8]` the float `30.0` it stores instead.

### The two early-outs

`009D3506`: **`approach+CCh == 0`** jumps to `009D3E21`, which clears `+131h` and advances `+134h`
and returns. Nothing else is written, so `+90h` and `+94h` keep whatever the constructor left.

`009D3624`: `+132h == 0` (no torpedoes left) takes `009D3D72`-`009D3E1F`. That branch can only
**clear** the latch, never set it: both stores are `AND byte [ESI+131h],AL`. It measures
`00414C60` between the target point and `ctl->+3D0h`'s position and clears the latch when that
exceeds `+8Ch`.

## (2) Where the engage distance `+8Ch` comes from: `009D4A70`

`009D3420` never writes `+8Ch`. Its one producer in the image is the tail of `009D4A70`
`BSP_BotTaskTorpedo_UpdateCruiseProfile`, the torpedo task's vtable slot `+54h`, at
`009D4AC4`-`009D4AD8`:

```
task->+484h = max(task->+484h, tuning->+434h * task->+41Ch);
```

`tuning+434h` is `Pilot/Torpedo/AttackDist`, default `2200`, from `docs/BOT_TASKS.md`'s tuning
table and `docs/GAME_TUNING_SINGLETON.md`. `task+41Ch` is the speed ratio; it was not read:
`contract: unread`. `009D4A70` is also the second caller of `009D3420` (`dt = 0`) and it parks the
plan position at `[00CF87D0] = 999999.0` and negates a positive `+12Ch`, both of which force the
next approach tick to replan.

`ghidra callers 009d4a70` returns none: it is reached only through the vtable, so who runs slot
`+54h` per tick is still `docs/BOT_TASKS.md`'s open question.

## (3) `009D0380`, the approach reset

`009D0380`-`009D066F`, `RET 8`, one caller `009D2DA0`. It is a **constructor-side reset**, not a
second per-tick producer of the flag pair. At `009D0579`-`009D05F0` it writes `+128h = 1.0`
(`00D7A24C`), `+12Ch = -00BD2F10(0, 1)` (a negated uniform draw, so the first tick replans),
`+130h = BL`, `+131h = BL` at `009D05B5`, `+134h = [00CFDEB0]`, and interpolates `+7Ch` between its
old value and `+80h` by a second random draw between `0` and `[00CE3CB4]`. The randomised replan
phase is what keeps a squadron's five aircraft from scanning on the same frame.

## (4) `009D15F0`, the aim tick. `coverage: partial`

`void __thiscall(BotStateTorpedoAim* state, float dt)`, `RET 4` at `009D2377`. `state+4h` is the
approach; the command block is `approach+18h`.

What is established: the routine's inputs, its eleven command-block writes, and the heading
producer. What is not: the four `BSP_Math_InterpolateClamped` chains (thirteen calls to `00419010`)
that fold the bank at `unit+C64h`, the altitude at `unit+100h` and the time to target into the
commanded throttle. Those are transcribed only to their constants.

| site | field | value |
| --- | --- | --- |
| `009D1D16` | `cmd->+2C0h` | `AddWrappedAngle(approach->+94h, turn)` - the bearing plus the sector turn offset `+5Ch`, shaped by the lead |
| `009D1D1E` | `cmd->+2CCh` | the constant `2` |
| `009D1D2E` | `cmd->+2C8h` | the throttle, the product of the four interpolation chains, capped at `[00CE3814]` |
| `009D1D02` | `cmd->+2E8h` | `[00D06874]` |
| `009D1EDD` | `cmd->+2BCh` | the commanded altitude |
| `009D1EE5` | `cmd->+2D0h` | the constant `1` |
| `009D1F4A`, `009D1F55` | `cmd->+278h`, `+27Ch` | `[00D7A24C] = 1.0` and `1` |
| `009D1F5C`, `009D1F64` | `cmd->+2A8h`, `+2ACh` | `0` and `1` |
| `009D1F6B` | `cmd->+2D8h` | `0` |
| `009D1C5B` | `approach->+60h` | the same commanded heading |
| `009D2021` | `approach->+130h` | the aim-solution byte |
| `009D22B6`, `009D22C4` | `approach->+A4h` | `0FFh` at or past `[00CE3958]`, else `3` |
| `009D22A9` | `(approach->+1Ch)->+40h` | `0` |
| `009D2344`, `009D2354` | `state->+24h`, `+28h`, `+2Ch` | the state's own exit flags |

The altitude floor at `009D1613`-`009D16C6` is `max(over_land ? 30.0 : 5.0, +78h + +74h,
00903860(unit position) + 5.0)`. The tick contains no `007BBBA0` and no store to `state+98h`,
confirming `docs/TORPEDO_TASK_ARM.md` section (3).

## (5) `009D1500` and `009D1360`

`float __fastcall(approach)`, `009D1500`-`009D15C0`. The time-to-target metric:

```
speed = (+134h >= 15.0) ? +7Ch : +80h;
t = +90h / speed;
if (t <= 1.0) return t;
if (speed < 600.0f /* 00CE4BC4 */) return (+90h - speed) / 600.0 + 1.0;   /* 00D20198, 00D7A210 */
return t;
```

`void __fastcall(approach)`, `009D1360`-`009D14F3`, the committed hook. It is the **torpedo run
time** and the one writer of `+98h`:

```
v0   = unit->vtable[38h]();             /* 009D1379 */
tf   = 007BCC80(unit->+100h);           /* 009D139D, a sqrt of the altitude */
+A0h = tf * v0;                          /* 009D13A9, the lead while it falls */
vt   = 007BCFA0();                       /* 009D13BF, the water run speed */
td   = max(0, (v0 - vt) / 80.0f);       /* 00CF1440 */
dd   = td * (vt + v0) * 0.5f;           /* 00D7A280 */
leg  = min(+90h, speed) - +A0h;
total = tf + (leg < 0 ? 0 : dd <= leg ? td + (leg - dd)/vt : td * (leg/dd));
+98h = max(0, +9Ch + total);
```

So the ETA the approach update stores at `+F8h` is the flight time to the release point plus the
torpedo's own run time, which is what a torpedo bomber needs to lead a moving ship.

## ABI summary

| address | ABI | evidence |
| --- | --- | --- |
| `009D3420` | `void __thiscall(approach, float dt)`, `RET 4` | `009D3E3C ADD ESP,0x8c` / `RET 4`; caller `009D486F` `PUSH ECX` + `FSTP [ESP]` |
| `009D4A70` | `void __fastcall(task)` | `009D4B2F RET` with no cleanup |
| `009D15F0` | `void __thiscall(state, float dt)`, `RET 4` | `009D2377` |
| `009D1500` | `float __fastcall(approach)` | result in `ST0`, `009D15C0 RET` |
| `009D1360` | `void __fastcall(approach)` | `009D14F3 RET` |
| `009D0380` | `RET 8`, two pushed arguments | `009D066D RET 0x8` |
| `009FADA0` | `void __thiscall(approach+B4h, float dt)` | `009D34B1 PUSH ECX` / `009D34B2 LEA ECX,[ESI+0xb4]` / `009D34B8 FSTP [ESP]` |
| `007B93F0` | `char __thiscall(unit, int)` | `009D34C0 MOV ECX,[ESI+4]` / `009D34C3 PUSH 0` |
| `007DF360` | `char __thiscall(unit+C50h, target, point)` | `009D36E6`-`009D36EE`, two pushes |
| `0041BC20` | `float __thiscall(terrain, float x, float z)` | `009D3699 SUB ESP,8` for two floats, `MOV ECX,EBX` |

## Host methods

One row per native call site the reconstruction models.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `009D34BB` | `009FADA0` | `tick_approach_subobject_009fada0` | `approach+B4h / dt / void` | always |
| `009D34C5` | `007B93F0` | `unit_has_torpedo_ordnance_007b93f0` | `approach+4h / 0 / bool` | always |
| `009D34DE`, `009D363B`, `009D3DA4` | `00414DB0` | `unit_world_xz`, `target_world_xz` | `entity / - / xz` | pose dirty |
| `009D3517`, `009D36E4`, `009D3DC8` | `approach->vtable[0]` | `approach_target_point` | `approach / out / point*` | always |
| `009D36A9`, `009D386D`, `009D3A42` | `0041BC20` | `terrain_height_0041bc20` | `ctl+34Ch / x, z / float` | `ctl+34Ch != 0` |
| `009D39D3` | `00903BC0` | `segment_blocked_00903bc0` | `- / two points / bool` | radius under 450 |
| `009D36EE` | `007DF360` | `target_reachable_007df360` | `unit+C50h / target, point / bool` | `unit+C50h != 0` |
| `009D3D01` | `unit->vtable[38h]` | `unit_speed_vtable38` | `unit / - / float` | always |
| `009D3430` and the block reads | - | `read_control_block` | `ctl / - / fields` | always |
| `0099AF53` | - | `PilotBot::queue_release_order` (unimplemented) | `unit+C58h` | every tick |

`009FADA0`, `0041BC20`, `00903BC0`, `007DF360`, `007BCC80`, `007BCFA0` and `00903860` are this
packet's **contracts**, logged through the unimplemented-host mechanism.

## Corrections

### Correction to `docs/TORPEDO_TASK_ARM.md` (packet `cc8_torpedo_task_arm`)

1. Section (1) step 4 calls `approach->+78h + approach->+74h` "the moveto ranges". The aim tick
   reads the same sum at `009D1631` as a floor on the commanded **altitude**, next to the
   over-water `5.0` and over-land `30.0` constants and the ground height. For the torpedo class the
   pair is an altitude band.
2. The "Follow-up packets" entry 1 lists `+98h` among `009D3420`'s writes. `009D3420` only reads
   it; `009D1360` at `009D14E7` writes it.
3. The Validation section names `009D3420` as the blocking gate on USN01 because
   `approach+8Ch`/`+90h` were both `0.0f`. Both halves of that are now explained: `+90h` was zero
   because the host had no approach object to run, and `+8Ch` is not `009D3420`'s output at all but
   `009D4A70`'s. See the Validation section below for the gate that replaces it.

### Correction to the reconstruction `src/torpedo_task_arm.cpp`'s host binding

`src/game_hosts_units.cpp` filled `TorpedoEngagedInputs::engage_range_scale` with `1.0f`. The
constant is `[00D05AC8]`, the **double `2.2`**, multiplied in at `009D324F`. With `1.0` the
predicate refused a range the native admits, which is why the first run after wiring `009D3420`
still reported every tick blocked. Corrected to `kTorpedoEngageRangeScale_00d05ac8`.

## `no_ghidra_function`

None. Every routine this packet read has a Ghidra function: `009D3420` (`009D3420`-`009D3E3F`),
`009D4A70` (`009D4A70`-`009D4B2F`), `009D1500` (`009D1500`-`009D15C0`), `009D1360`
(`009D1360`-`009D14F3`), `009D0380` (`009D0380`-`009D066F`), `009D3210` (`009D3210`-`009D3265`),
and `009D15F0` (`009D15F0`-`009D2379`), which a previous session defined.

## Validation

`./scripts/build.ps1` (MSVC Win32, `/W4 /WX`) succeeds; `ctest` passes both existing suites
(`reconstructed_math`, `tool_tests`). No test was added.

| run | result |
| --- | --- |
| USN01 before | five ordered aircraft, `arm_ticks=1299 transitions=0 states[moveto=1299] releases=0`, `blocked_engaged_009d3210=6495` |
| USN01 after | five aircraft, `transitions=1..3`, `states[moveto=239..434 prepare=865..1060]`, `releases=0`, `blocked_engaged_009d3210=1671` |
| USN01 approach census | `ticks=1299 no_target=0 replans=130` per aircraft; `engage 8Ch=2200.0`; `range 90h` non-zero from tick 1, minimum `2906.0` (Mav2) to `3979.4` (Mav1) |
| USN02 after | `shots=734 first_shot=1.40 s`, `hull=180 part=0 deaths=2 total_damage=18525.6`, `projectiles created=734` - identical to the milestone 2t baseline |
| USN02 torpedo census | `no ordered aircraft carries torpedo ordnance (kind 2Bh), so 0099A170 builds no kind Eh task` |

**The approach update moves the state machine.** With `009D3420` producing a live `+90h` and
`009D4A70` producing `+8Ch = 2200`, `009D3210`'s range clause `+8Ch * 2.2 = 4840 > +90h` passes
once the aircraft closes inside 4840, the transition rule's step 6 fires, and all five aircraft
leave `moveto` for `prepare` after 239 to 434 arm ticks. `blocked_engaged_009d3210` falls from
6495 to 1671, the ticks before they close.

**No release, and the next gate is `unit+C58h` at `0099AF53`.** The rule parks in `prepare` because
step 6 re-fires while `ctl->+370h == 0`. `prepare`'s tick `009D2720` releases only with
`prepare+98h > 0`, and the one positive writer of `prepare+98h` is `009D49A0`, the task vtable's
`+24h`, which `BSP_PilotBot_Tick` reaches only through

```
0099af53  if (unit->+C58h <= 0) goto end;
0099af81  for (task in bot->+58h) if (task->vtable[24h]()) { unit->+C58h -= 1; break; }
```

The arming loop is wired here and reports `the queued release-order count is 0 on every one of
6495 ticks`, so `009D49A0` is never offered, `prepare+98h` stays at `-1.0f` and `007BBBA0` is never
reached. **The raiser of `unit+C58h` is the contract this packet leaves open.**

The aim state was never entered (`aim_ticks=0` on all five), because the entry chooser `009D3F60`
sends an engaged task with `ctl->+370h == 0` to `prepare` before it can test `+529h`. The aim tick
reconstruction is therefore reconstructed and build-tested but not run-exercised.

## Follow-up packets

1. **`unit+C58h`**: who raises the queued release-order count. It is now the single gate between
   an ordered torpedo aircraft and its drop. The player's fire button and the AI order may share
   it; `007B9140` and the `unit+5Ch` byte in the same guard are also unread.
2. **`ctl->+370h`**, the pilot control block's three-valued attack mode. It decides `prepare`
   against the `attackrun`/`aim` pair in both `009D4030` step 6 and `009D3F60` row 1, and `== 2`
   short-circuits `009D3210`. Nothing in this repository writes it.
3. **The throttle chain of `009D15F0`**, the four `BSP_Math_InterpolateClamped` folds between
   `009D1B00` and `009D1D2E`.
4. **`0041BC20` and `00903BC0`**, the terrain height sampler and the segment-blocked test. Without
   them the sector scan cannot run at all, so `+5Ch` stays zero and the run-in has no planned
   heading.
5. **`task+41Ch`**, the speed ratio the attack-distance clamp multiplies by, and who runs the task
   vtable's `+54h` slot per tick.
6. **`009FD0E0`**, the `task+38Ch` sub-object, which calls `007BBBA0` on a path independent of the
   task states.
