# The ordnance release request and what it actually reaches

Addresses: 007BBBA0, 007EABC0, 007DE3A0, 007DE1E0, 007C6820, 007D5D20, 007C0D90, 007CE040,
007C1D80, 00730B80, 0072F830, 00730160, 008568E0, 00CFE308, 00D0862C, 00CFD99C.

Packet `cc8_torpedo_release_spawn`, owner `agent/cc8-torpedo-drop`, tree base `95c5e3e9c`.

## The headline correction

The packet was dispatched on the premise that the object at `unit+DECh` is the plane's ordnance
device, that its tick consumes `dev->+11h`, and that this consumption spawns the projectile. The
first two halves of that premise are wrong and the third does not exist.

`unit+DECh` holds a 0x80-byte block of **three animated 0..1 actuator channels**. Its tick steps
those channels and clears `+11h` when none of them is still moving. It reads no ordnance, calls no
spawn, and touches no projectile. `+11h` is a bookkeeping "some channel is still animating" flag,
raised by every writer that moves a channel and lowered by the tick when all three come to rest.
`BSP_Unit_RequestOrdnanceRelease` at `007BBBA0` is, on this evidence, a **door-open command**, not
a release: it drives one channel of that block toward 1.0 and bumps the counter `unit+C20h`.

This section replaces, for this one claim, the `contract: unread` note that earlier torpedo packets
left on `unit+DECh`. It does not contradict anything those packets asserted; they recorded the
writes correctly and declined to name the consumer.

## The block at `unit+DECh`

Producer, exhaustively. A capstone sweep from every indexed function start over `.text`, collecting
every instruction with a memory operand whose displacement is `0xDEC`, returns 37 references, of
which 4 are stores. Three of the stores (`008051A5`, `00807115`, `008076D4`) are in the recon slot
code and write a different object; `00806ED0` is the matching destructor. The one store that fills
the plane's block is `007D61B4` `MOV [ESI+0xDEC], EAX` inside `BSP_Plane_ReadPropertyBag`
`007D5D20`. Immediately before it:

```
007d618e: push 0x80          ; sizeof == 0x80
         call 0xbf681b       ; operator new
007d61a8: push esi           ; the unit
007d61a9: mov ecx, eax
007d61ab: call 0x7eabc0      ; the constructor
007d61b4: mov dword ptr [esi + 0xdec], eax
```

`007EABC0` `BSP_PlaneActuatorBlock_Construct`, `undefined4* __thiscall(this, unit)`. It runs the
base constructor `00876020` under base vtable `00CFD99C`, installs vtable `00D0862C`, and writes
the unit backpointer at `+1Ch`. Rule 4 is satisfied: the layout below is taken from this producer,
not inferred from readers.

### Layout

| offset | meaning | evidence |
| --- | --- | --- |
| `+00h` | vtable, `00D0862C` | `007EABDD` |
| `+11h` | aggregate "a channel is moving" | set by every channel writer, cleared at `007DE417` |
| `+1Ch` | the owning unit | `007EABE3`, from the constructor argument |
| `+28h` | channel B base | constructor `+28h = 0`, `+30h = 1` |
| `+44h` | channel A base | constructor `+44h = 0`, `+4Ch = 1` |
| `+60h` | channel C base, the ordnance bay | constructor `+60h = 0`, `+68h = 1` |
| `+78h`, `+7Ch` | two floats, both `-1.0f` | `007EAC5x`, unread |

Each channel is the same six-field record, from `007DE1E0`:

| offset in channel | meaning |
| --- | --- |
| `+0` | byte, the channel is present and enabled |
| `+1` | byte, the commanded end state, 1 extend and 0 retract |
| `+4` | float, the current value, clamped to `[0.0, 1.0]` |
| `+8` | byte, moving |
| `+0Ch` | float, the rate per second |
| `+18h` | the frame stamp `DAT_00F876A4` |

`DAT_00D7A24C` is `1.0f` (`00 00 80 3F`), `DAT_00D7A260` is `-1.0f` (`00 00 80 BF`) and
`DAT_00D7A218` is `0.0f`; all three read from the image.

Weak corroboration, worth recording but not evidence: the partition keywords for segment 48, which
holds both the constructor and the tick, are `travelspeed, gears, baydoor, wings, state, windsound,
timeout, targst`. Three animated channels in a segment whose strings include `baydoor`, `gears` and
`wings` is the shape this layout predicts. It does not establish which channel is which, and the
channel names in this document stay as offsets for that reason.

So `dev+60h`/`+61h`/`+64h`/`+68h`, the four bytes `007BBBA0` guards on and writes, are exactly
channel C's enabled byte, target byte, value and moving byte. The apparent float compare
`dev->+64h != DAT_00D7A24C` is "the bay is not already fully open".

### The tick, `007DE3A0`

`void __thiscall(this, float dt)`, `RET 4` at `007DE41D`. Raw listing `007DE3A0`-`007DE41D`, **no
Ghidra function**: `FUN_007DE2E0`'s stored body ends at `007DE398` and INT3 padding runs
`007DE399`-`007DE39F`, so both containment checks agree the code at `007DE3A0` is a separate body.
It is slot 3 (`+0Ch`) of vtable `00D0862C`.

```
step(+44h);  step(+28h);  step(+60h)          ; each via 007DE1E0 with dt
if (+4Ch == 0 && +30h == 0 && +68h == 0) +11h = 0
```

Each `step` call is guarded by `enabled == 0 || value == 1.0f` (`CMP BYTE [edi],0` / `JE` to the
call, then `UCOMISS` `+4h` against `00D7A24C`, `LAHF`, `TEST AH,0x44`, `JP` past the call). The
`JP` is taken when the two differ, so the call runs when the channel is disabled or already at the
top of its travel, which is where `007DE1E0` does its own settle test and clears the moving byte.

`007DE1E0` `BSP_PlaneActuatorChannel_Step` integrates `value += rate * (+1 if target else -1) * dt`,
clamps into `[0, 1]`, stamps `+18h`, and sets `+8`. When the value already sits at the end the
target asks for, it clears `+8` and returns without moving anything.

Nothing in either routine reads an ordnance count, a bullet class or a mount, and neither calls a
spawn. Rule 3: every call site in both bodies was read, and there are two, both to `007DE1E0`'s
family.

### Who drives channel C

`BSP_Plane_DriveActuatorChannels`, raw listing from `007C6820`, **no Ghidra function**
(`FUN_007C6760`'s body ends at `007C6819`, INT3 padding `007C681A`-`007C681F`).
`__thiscall(ECX = plane, float dt)`. Coverage: **partial**, read for `007C6A78`-`007C6BD4`.

```
007c6a9a: if (unit+DF0h || unit+C25h || unit+C20h) bl = unit+9C3h[[00F876B8]*8]  else bl = 0
007c6aca: if (vtable[5Ch](0x2f))  { +64h = 1.0f; +61h = 1; +68h = 1 }            ; force the bay open
          else                    { +61h = bl;  +68h = 1 }                        ; hold it at the slot byte
          if (+68h) +11h = 1
```

This is the same per-slot byte `unit+9C3h[[00F876B8]*8]` and the same counter `unit+C20h` that
`007BBBA0` guards on and raises. The release request therefore reaches this block only as a
door command, and this step re-asserts channel C's target every frame from the ordnance state.

## What the release request actually reaches

| step | address | effect |
| --- | --- | --- |
| the request | `007BBBA0` | channel C target 1, value 1.0f, moving 1, `+11h` 1; `unit+C20h += 1` on every path |
| the plane fixed step | `007CE078`-`007CEA8D` inside `007CE040` | spends a pending counter and calls `007C0D90` with the unit in ECX |
| the order issue | `007C0D90` | walks the device list at `unit+48h`, sets `unit+C25h = 1`, calls `BSP_PilotControl_IssueReleaseOrders` |
| the actuator tick | `007DE3A0` | steps the channel, clears `+11h` when it stops |

**The spawn is not on this chain, and this packet did not reach it.** What the packet did establish
about where it must be: the bomb platform is a **gun**. `BSP_MultipleBombPlatform_Construct`
`00730B80` calls `BSP_Gun_Construct` before installing vtable `00CFE308`, and
`BSP_Plane_ReloadBombPlatforms` `007C1D80` reloads every device whose `vtable[5Ch]` answers `25h`
through `vtable[200h]`, which vtable `00CFE308` fills with `0085AD80`. A plane's torpedo is
therefore a gun row in the same sense a ship's tube is, and the projectile it makes should come out
of `BSP_Gun_Fire` `00730160` into `BSP_Gun_SpawnShotAndEffects` `0072F830`, which is the path the
gunnery host already implements. Reading the platform's own release slot is the follow-up.

Coverage for this section: **partial**. `007C0D90`'s six vtable slots are named, not read, and that
was already its recorded state.

## The rest of the chain

Every hop below was read to its end.

| address | routine | what it does |
| --- | --- | --- |
| `007BBBA0` | `BSP_Unit_RequestOrdnanceRelease` | channel C open, `unit+C20h += 1` on every path |
| `007CEA82` | inside `BSP_PlaneTickElement_FixedStep` `007CE040` | spends one pending release, then calls `007C0D90` at `007CEA8D` |
| `007C0D90` | `BSP_Plane_TickReleaseOrderIssue` | walks `unit+48h`, sets `unit+C25h = 1`, calls `007EEF30` |
| `007EEF30` | `BSP_PilotControl_IssueReleaseOrders` | walks the controlled-unit array at `ctl+3D0h` and calls `007BCBE0` with 999 for each |
| `007BCBE0` | `BSP_Unit_SetQueuedReleaseOrders` | assigns `unit+C58h` when the unit is enabled and `007B9140` holds, else zero |
| `0099AFB6` | inside `BSP_PilotBot_Tick` `0099ACD0` | `ADD [EAX+0xC58], -1` after a bot task's `vtable[24h]` at `0099AF9B` answers |

The spawn is **not** on this chain and this packet did not reach it. The bot task's `vtable[24h]`
release slot and the bomb platform's own release slot in vtable `00CFE308` are both unread.

What is established about where the spawn must be: the bomb platform is a **gun**.
`BSP_MultipleBombPlatform_Construct` `00730B80` calls `BSP_Gun_Construct` at `00730B88` before
installing vtable `00CFE308`, and `BSP_Plane_ReloadBombPlatforms` `007C1D80` reloads every device
whose `vtable[5Ch]` answers `25h` through `vtable[200h]`, which that vtable fills with `0085AD80`.
A plane's torpedo is therefore a gun row in the same sense a ship's tube is, and the projectile it
makes should come out of `BSP_Gun_Fire` `00730160` into `BSP_Gun_SpawnShotAndEffects` `0072F830`.

## Host methods

| host method | file | native | kind |
| --- | --- | --- | --- |
| `plane_actuator_channel_step_007de1e0` | `src/torpedo_release_spawn.cpp` | `007DE1E0` | reconstruction |
| `plane_actuator_block_step_007de3a0` | `src/torpedo_release_spawn.cpp` | `007DE3A0` | reconstruction |
| `ordnance_release_request_007bbba0` | `src/torpedo_release_spawn.cpp` | `007BBBA0` | reconstruction |
| `release_ordnance_007bbba0` | `src/game_hosts_units.cpp` | `007BBBA0` | binding |
| the block step, per fixed step | `src/game_hosts_units.cpp` | `007DE3A0` | binding |
| `GameGunneryHost::release_ordnance_drop` | `src/game_hosts_gunnery.cpp` | `0072F830` | **substitution** |
| the water-entry limit | `src/game_hosts_gunnery.cpp` | `008568E0` | partial reconstruction |
| torpedo gate census | `src/game_hosts_gunnery.cpp` | none | instrumentation |

Three substitutions, each labelled at its address in the source:

1. **The release geometry.** The native mount node and the platform's release slot are unread, so
   the round leaves from the plane's origin along its forward axis at the plane's own forward
   speed. A drop inherits the aircraft's velocity, so `projectile_launch_velocity_006E8430` is not
   used and no muzzle speed is applied. Aiming, barrel timers and the stock decrement are not
   reproduced.
2. **The channel's enabled byte and rate.** The native writer of `+60h` and of the channel rate is
   unread. Channel C is enabled on first request with a nominal 1.0 per second travel. The request
   forces the value to 1.0f on the same frame regardless, so the rate only governs how long the bay
   takes to close.
3. **The third guard.** `007BBBA0`'s per-slot byte at `unit+9C3h[[00F876B8]*8]` lives on the unit,
   not on the block, and this host has no such byte, so it is not applied. The refusal count
   therefore under-counts what the image would refuse.

The water-entry limit applies only `008568E0`'s first test, the `MaxWaterHitVel` speed limit at
`classDesc+0DCh`. The second, `shot[+0Ch] < -classDesc[+0ECh]`, needs a depth field this host's
shot record does not carry and is not applied.

The gate census is not a reconstruction and no native address produces it. A gun counts as a
torpedo gun when `swim_speed > 0`, the identical test the water crossing uses, so a gun in the
funnel is exactly a gun whose shot could reach the swim model.

## Why `swims_started` is zero, including for ships

The host's water crossing at `src/game_hosts_gunnery.cpp` requires `shot.position[1] <= 0` while
the previous position was above zero, and then a positive `swim_speed` on the round's bullet class.
`torpedo_ranges_derived=39` says 39 gun rows did derive a swim speed, so the bullet classes are
authored and read correctly. The USN01 baseline nevertheless reports `water=0`: **no projectile of
any kind crossed the sea surface**, so the swim branch is never reached. The gate is upstream of
the swim model entirely.

The baseline's own per-category table names it without needing a new run. Columns are guns,
assigns, shots, no_window, arc_blocked:

| function | guns | assigns | shots | no_window | arc_blocked |
| --- | --- | --- | --- | --- | --- |
| AAMACHINEGUN | 154 | 449 | 3814 | 85436 | 0 |
| FLAK | 21 | 64 | 24 | 10312 | 0 |
| TORPEDO | 15 | 0 | 0 | 0 | 0 |
| BOMBPLATFORM | 81 | 0 | 0 | 234327 | 0 |
| DEPTHCHARGE | 10 | 0 | 0 | 30000 | 0 |

**The next gate is target assignment.** `assigns` counts `set_bot_fire_target_00727F10`, and it is
zero over all 15 torpedo guns in USN01 while AAMACHINEGUN takes 449. With `have_target` false on
every tick, every conjunct of `want_fire` downstream is vacuous, which is why the torpedo rows of
the funnel added here are all zero and why `no_window` is zero as well: those guns are never even
aimed. `00729BC0`'s slot test, `length3(delta) <= row.max_range`, is the pre-filter in front of the
assignment and is the first thing to read. Note that BOMBPLATFORM does take 234327 aim ticks with
zero assigns, so the two categories fail differently and should not be treated as one bug.

That gate is why the ship half of the packet is unchanged by this work: nothing here assigns a
target to a torpedo gun, so ship tubes still never fire. The air drop added here bypasses the
assignment entirely, which is faithful to the native only in the sense that a dropped torpedo is
not an aimed shot. Ship tubes remain a follow-up.

## Corrections

Appended, not rewritten.

* `docs/BOT_TASK_STATES.md`, "The ordnance release": the device at `unit+DECh` is the plane's
  actuator block `007EABC0`, not an ordnance device, and `dev->+11h` is an aggregate "a channel is
  moving" flag cleared at `007DE417`, not a release request that something later consumes.
* `docs/TORPEDO_FIRST_RELEASE.md` line 93 and `docs/PLANE_GROUND_OPS.md` line 132 describe the same
  structure at two different channels: `+44h`/`+45h`/`+48h`/`+4Ch` is channel A and
  `+60h`/`+61h`/`+64h`/`+68h` is channel C of one block, with `+11h` shared between them. Both
  entries are correct as written; this names what they were looking at.

## no_ghidra_function

Inclusive end addresses, each the final RET.

| start | end | name |
| --- | --- | --- |
| `007DE3A0` | `007DE41D` | `BSP_PlaneActuatorBlock_Step` |
| `007C6820` | unread past `007C6C2F` | `BSP_Plane_DriveActuatorChannels`, partial |

## Validation

Both missions, 3200 frames, `--mission-frames 3000` at `0.05` s, through `tools/run_game.ps1` on
this worktree. Only this tree's own before-run is a reference.

### USN01, before and after

| | before | after |
| --- | --- | --- |
| release requests `007BBBA0` | 1, logged unimplemented | 1, executed |
| bay requests accepted | not modelled | 1 |
| torpedoes dropped | 0 | **1** |
| drop refusals | not modelled | 0 |
| water crossings | 0 | **1** |
| water-entry breakups | not modelled | **1** |
| `swims_started` | 0 | 0 |
| projectiles created | 3838 | 3839 |
| entity impacts | 26 | 26 |
| total damage | 229.5 | 229.5 |
| torpedo hits, torpedo damage | 0, 0 | 0, 0 |

A torpedo now leaves the aircraft and reaches the sea. It does not swim, and the reason is
measured, not guessed:

```
gunnery: torpedo drop 1 by ConTBD1 at 700 m, speed 0.0 m/s, bullet 63, swim 30.9 m/s
water entry broke the round up at 117.2 m/s, MaxWaterHitVel 100.0 m/s (008568E0)
```

**The next gate is release altitude.** The AI releases at 700 m with the aircraft's forward speed
reading zero, so the round is a pure free fall: `sqrt(2 * 9.81 * 700)` is 117.2 m/s, which is
exactly the entry speed logged, and `008568E0`'s `MaxWaterHitVel` at `classDesc+0DCh` is 100.0 m/s
for this round. The image's rule destroys it. Two separate things have to change before this
torpedo swims, and they are independent:

1. The release altitude. 700 m is not a torpedo run. Whatever sets the bomber's run-in height is
   the thing to read; this packet did not touch it.
2. The aircraft's forward speed. `unit_forward_speed_0092d730` returns 0.0 for `ConTBD1` at the
   release instant, so the round inherits no velocity from the plane. That is a host gap, not an
   image fact, and it is upstream of this packet.

Even at zero forward speed, a release below about 510 m would enter under the 100 m/s limit, so
altitude alone is enough to move this number.

### USN02, after only

No USN02 before-run exists on this tree: the machine-wide game lock was held by other agents
through the window in which a baseline build was still installed, and by the time the lock came
free the instrumented build was in place. **No before/after attribution is claimed for USN02.**

| | after |
| --- | --- |
| torpedo guns | 71 |
| torpedo gate: ticks / targeted / accepted / settled / window / sent / shots | 207795 / 54869 / 54869 / 49666 / 45059 / 45059 / 44 |
| `swims_started` | **44** |
| water crossings | 372 |
| torpedoes dropped | 0 |
| water-entry breakups | 0 |
| projectiles created | 562 |
| entity impacts | 168 |

Two things follow, and the second corrects the packet brief.

* `drops=0` is expected: the run log reports that no ordered aircraft in USN02 carries torpedo
  ordnance, so no release request is ever made and the air-drop path is never entered.
* **Ship torpedoes already swim.** 44 swims out of 44 torpedo-gun shots, with the water-entry
  breakup count at zero. The claim that `swims_started` is zero "even for ships" is a USN01 fact,
  not a general one: USN01 simply has no ship torpedo engagement, which is the same
  `assigns = 0` gate documented above. Nothing in this packet caused the 44, and the only way this
  packet's changes could have moved that number is downward, through the new water-entry limit,
  which fired zero times.

## Follow-up packets

1. **The real spawn, to replace substitution 1.** Read the bot task's `vtable[24h]` that
   `0099AF9B` calls and the bomb platform's release slot in vtable `00CFE308`. Those two give the
   mount node, the launch velocity and the stock decrement that this packet substituted.
2. **The channel enabler, to replace substitution 2.** Find the writer of `+60h` and of the channel
   rate at `+6Ch` on the block at `unit+DECh`. The plane property bag `007D5D20` is the obvious
   place to look, since it is where the block is built.
3. **The ship gate.** `assigns = 0` over 15 torpedo guns. Read `00729BC0`'s slot test and whatever
   fills the TORPEDO category's preference list, then act on it. BOMBPLATFORM fails differently
   and needs its own look.
4. **The second water-entry limit.** `008568E0`'s `shot[+0Ch] < -classDesc[+0ECh]` needs a depth
   field on the host's shot record before it can be applied.
5. **Channels A and B.** Channel A at `+44h` is driven by `BSP_Plane_GroundRollStep` at `007CBFC8`
   and channel B at `+28h` from `unit+72Ch` and `unit+900h == 6`. Naming what each one actuates,
   most likely gear and flaps, would finish the block.

## Correction: "the next gate is target assignment" (packet cc8_torpedo_gun_assignment)

Appended, not a rewrite. The measurement in "Why `swims_started` is zero, including for ships" is
correct: `assigns` really is 0 over all 15 TORPEDO-category guns in USN01. The implication drawn
from it, that target assignment is a gate to be fixed, overstates it. Neither category can be
assigned in USN01, and in both cases that is the image's own behaviour.

* Category 0Ah `BOMBPLATFORM`, which is the category a torpedo bomber's torpedo is actually
  mounted in (24 of USN01's 39 swim-capable guns), has an **empty preference row** at `00E0A1F0`,
  all zeros in the image. The rank test inside `00863990` therefore refuses every candidate for it
  on every tick. A bomb platform is never a gunnery target holder, which is consistent with its
  ordnance being released by the bot-task chain rather than aimed.
* Category 7 `TORPEDO`, the other 15 and all ship tubes, is cut out of the recon sweep at
  `008651F5` (`CMP ESI,7 / JE 00865442`), so its only candidate source is a director command or
  fire target. USN01 gives its 5 torpedo-carrying units neither, across 15000 pass ticks, so
  `00863990` is called 0 times for that category. USN02 gives 14 of its 29 owners a command
  target, and there the same code assigns 55960 times and fires 46 torpedoes.

The gate that actually keeps an aerial torpedo out of the swim is the one this document already
measures: a 700 m release, a 117.2 m/s entry and a 100.0 m/s `MaxWaterHitVel`.
Full evidence: `docs/TORPEDO_GUN_ASSIGNMENT.md`.
