# The queued release orders and the pilot attack mode (packet `cc8_torpedo_release_orders`)

Addresses: `007BCBE0` (`007BCBE0`-`007BCC14`), `007EEF30`, `007ED3C0` (`007ED3C0`-`007ED3DD`),
`007C0D90`, `007B9140` (`007B9140`-`007B91A0`), `007ED3F0` (`007ED3F0`-`007ED3FA`), `007ED430`
(`007ED430`-`007ED442`), `0099B740` (`0099B740`-`0099B77A`), `0099B710` (`0099B710`-`0099B712`),
`009A2810`, `0099ACD0` (`0099AF2C`-`0099AFCC`), `00922F30`, `00922F80`, `007CE040`, `008A4B10`.

`docs/TORPEDO_APPROACH_UPDATE.md` ended with two gates and no answer for either: `unit+C58h`, the
queued release-order count that nothing in the repository raised, and `ctl->+370h`, the attack mode
that parked every ordered aircraft in `prepare`. Both are settled here.

**The one-line answer.** `unit+C58h` is not a magazine and not the player's fire button. It is a
per-tick permission budget that the **plane's own fixed-step tick** hands out: `007CE040` ->
`007C0D90` -> `007EEF30` -> `007BCBE0` writes `999` into every unit the pilot control block drives,
and `BSP_PilotBot_Tick` spends one of them per tick by offering it to each task. `ctl->+370h` is
raised to `1` by `0099B740`, the tail of eight of the ten task cruise profiles, but **only when the
task's unit is the lead of the flight**.

## (1) `unit+C58h`: the complete writer census

Offset `C58h` needs a `disp32`, so every base-register store form is byte-searchable. The scans
over `.text`, with their positive controls:

| pattern | form | result |
| --- | --- | --- |
| `C7 ?? 58 0C 00 00` | `MOV [reg+C58h], imm32` | `007BCC09`, `0099AFC2`, plus one `mod=01` false positive at `006CAC8A` (`MOV [ESI+58h], 0xC`) |
| `89 ?? 58 0C 00 00` | `MOV [reg+C58h], r32` | `007BCBFD`, `007D0038` |
| `83 ?? 58 0C 00 00` | `ADD`/`CMP [reg+C58h], imm8` | `0099AF53` (a `CMP`, the guard), `0099AFB6` (`ADD -1`) |
| `FF ?? 58 0C 00 00` | `INC`/`DEC dword [reg+C58h]` | none |
| `01 ?? 58 0C 00 00` | `ADD [reg+C58h], r32` | none |
| `29 ?? 58 0C 00 00` | `SUB [reg+C58h], r32` | none |
| `8D ?? 58 0C 00 00` | `LEA reg, [reg+C58h]` | `007D681E` in `BSP_Plane_ReadPropertyBag` |

`0099AFB6` and `0099AFC2` are the **positive controls**: both are sites this packet's predecessor
already read, and both appear, so the `83` and `C7` scans are known to hit the forms they look for.
The `89` scan hits twice, so it is live too. The `01` scan's form is known to occur in the image
(`005C24DA`, `006E4532`, `006E4734` with other displacements), so its empty result for `C58h` is
meaningful. The `FF` scan is the one negative this packet does **not** claim: no `INC dword ptr
[reg+disp32]` with a displacement ending `00 00` occurs anywhere in `.text`, so MSVC evidently never
emits that encoding here and the empty result proves nothing about `C58h`. It does not matter:
`007BCBE0` assigns rather than accumulates, so an increment would be out of character anyway.

So the census is:

| address | function | what |
| --- | --- | --- |
| `007BCBFD` | `007BCBE0` | `unit->+C58h = count`, the raise |
| `007BCC09` | `007BCBE0` | `unit->+C58h = 0`, the same routine's failure arm |
| `007D0038` | `BSP_PlaneUnitInstance_Construct` | `MOV [ESI+C58h],EBX`, the constructor's zero |
| `0099AFB6` | `BSP_PilotBot_Tick` | `ADD [EAX+C58h],-1`, the spend |
| `0099AFC2` | `BSP_PilotBot_Tick` | `MOV [EAX+C58h],0`, the tick's own clear |

`007D681E` takes a pointer to the field inside `BSP_Plane_ReadPropertyBag`. That routine is not
read here: `contract: unread`, and a store through that pointer would not show in the scans above.
It is a property-bag reader, so a write is unlikely but not excluded.

**`007BCBE0` is therefore the only raiser in the image.** `void __thiscall(unit, int count)`,
`RET 4`:

```
if (count > 0 && unit->+5Ch != 0 && 007B9140(unit, 0))   /* 007BCBE6, 007BCBEC, 007BCBF4 */
    unit->+C58h = count;                                  /* 007BCBFD */
else
    unit->+C58h = 0;                                      /* 007BCC09 */
```

It **assigns**. Every guard failure zeroes the field rather than leaving it, which is why a unit
that goes out of scene or loses its ordnance cannot keep a stale budget.

## (2) The two callers, and which paths reach them

`ghidra callers` gives `007BCBE0` exactly two, and a rel32 scan agrees.

### `007EEF30`, the issuer

`void __thiscall(ctl, unit)`. `ESI` is the **pilot control block**: it reads `+370h`, `+374h`,
`+378h`, `+390h`, `+3CCh` and `+3D0h`, the same fields `007ED3F0` and `007ED3C0` write, and its call
site loads `ECX` from `unit+9D4h`.

```
007EEF3B  007EE7F0(ctl, unit);                       /* contract: unread */
007EEF40  if (ctl->+390h > ctl->+374h)               /* FCOMIP / JBE */
007EEF54    for (i = 0; i < ctl->+3CCh; ++i) {
007EEF5C      unit_i = ((void**)(ctl + 3D0h))[i];
007EEF62      if (ctl->+378h != 0 || !007B8AD0(unit_i))
007EEF78        007BCBE0(unit_i, 0x3E7);             /* 999 */
            }
007EEF96  if (unit && 007B8AD0(unit) && ctl->+370h != 2) ...
```

**`ctl+3D0h` is an array of controlled units, count `ctl+3CCh`**, not a single target. That settles
a reading `docs/TORPEDO_APPROACH_UPDATE.md` left provisional: where `009D3420`'s no-ordnance branch
at `009D3D88` loads `ctl->+3D0h` and compares it with `approach+4h`, it is comparing against the
**first element**, the flight leader, so the test is "am I not the lead aircraft". See "Corrections".

`007B8AD0` is `BSP_Unit_LacksFollowTarget`, so the per-unit condition reads: issue when the force
flag `ctl+378h` is set, or when the unit **has** a follow target.

### `007ED3C0`, the clear

`void __fastcall(ctl)`, `007ED3C0`-`007ED3DD`: `ctl->+378h = 0` unconditionally, then
`007BCBE0(ctl->+3D0h[0], 0)` when `ctl->+3CCh > 0`. It clears the budget on the lead unit only.

### The path that reaches the issuer

```
007CE040  BSP_PlaneTickElement_FixedStep
  007C0D90(unit)                          /* the plane's own fixed step */
    007EEF30(unit->+9D4h, unit)           /* 007C0F01 */
      007BCBE0(ctl->+3D0h[i], 999)        /* 007EEF7F */
```

`007C0D90` has one caller, `BSP_PlaneTickElement_FixedStep`, which this repository already
reconstructs. Its body walks the linked list at `unit+48h` (next at `+44h`) looking for a device of
class `25h` (the weapon-device class of `docs/ORDNANCE_KIND_IDENTITY.md`) that holds bomb-family
ordnance `2Ah` and whose descriptor answers `2Ch`, `2Bh` or `33h`. `2Bh` is `Torpedo`, so a torpedo
bomber matches. On a match it latches a stack byte, and after the walk it sets `unit+C25h = 1` at
`007C0EE2` and calls the issuer unless the descriptor answers `33h`. `coverage: partial` - the six
vtable slots the walk uses (`+5Ch`, `+210h`, `+1FCh`, `+220h`, `+1F0h` and the descriptor's `+8h`)
are named, not read.

**The player's fire button does not reach `unit+C58h`.** Both callers of the only writer are on the
plane's AI tick, and the field is read only by `BSP_PilotBot_Tick`. The manual release the arm's
step 8 and the tick's own guard use is a different mechanism: the object embedded at `unit+72Ch`
answering its `vtable[38h]`, which `docs/TORPEDO_TASK_ARM.md` already records. `unit+C58h` gates
that guard but does not carry it.

## (3) `007B9140` and `unit+5Ch`, the two shared guards

`char __thiscall(unit, int arg)`, `RET 4`, `007B9140`-`007B91A0`:

```
if (unit->vtable[5Ch](0x17)) return 1;                      /* 007B9148 */
for (i = 0; i < unit->+994h; ++i)                           /* 007B915B */
    if (((void**)(unit + 974h))[i]->vtable[210h](0x2A, arg)) return 1;   /* 007B917B */
return 0;
```

`17h` is `MPlaneKamikaze` and `2Ah` is `MBomb` in `docs/ORDNANCE_KIND_IDENTITY.md`, and that doc's
descriptor table shows `Torpedo` answering `2Bh`, `2Ah` **and** `29h`. So the routine reads: **can
this unit drop ordnance at all** - a kamikaze plane is its own weapon, anything else needs a
bomb-family device in the array at `unit+974h`. A torpedo bomber passes on its torpedo.

`unit+5Ch` is the **scene-node enabled byte**. The disp8 store scan `C6 46 5C ??` finds
`00922F4B` in `BSP_SceneNode_Enable` writing `1` and `00922F9B` in `BSP_SceneNode_Disable` writing
`0`, with `BSP_SEntity_InitAll` toggling both. `coverage: partial` - the scan covers the `ESI` base
only, which is the base both scene-node routines use; other bases were not enumerated because the
offset is too common for the result to be readable.

## (4) `ctl->+370h`, the attack mode

Three values, and this packet finds a writer for each.

| value | name | writer | meaning |
| --- | --- | --- | --- |
| `0` | hold | `009A2869`-`009A285E` (`009A2810`, the `closetoship` timer) and the block's initial state | `009D3F60` row 1 and `009D4030` step 6 send an engaged task to `prepare`; it never reaches `attackrun` or `aim` |
| `1` | attack | `0099B774` in `0099B740` | the normal attack path |
| `2` | forced | `007ED43C` from `008A4B10 BSP_LuaBinding_PilotStopCloseToShip` | `009D3210` short-circuits to engaged at `009D3232` |

The `89 ?? 70 03 00 00` and `C7 ?? 70 03 00 00` scans return fifteen and four sites, but only two
are on the pilot control block: `007ED3F4` (the plain setter `007ED3F0`) and `007ED43C` (the
raise-only setter `007ED430`). The rest sit on ship AI, squadron and controller objects that happen
to use the same offset. The `83 ?? 70 03 00 00` hits are all `CMP` (`reg` field 7 in the modrm), not
stores: `009A548F`, `009A57D9`, `007EEFA5` and the rest are the readers.

### `0099B740`, the one producer of mode 1

`void __fastcall(task)`, `0099B740`-`0099B77A`, the tail of eight of the ten task vtable `+54h`
cruise profiles, including the torpedo's `009D4A70`:

```
if (task->+2FCh && task->+2F4h && task->+2F4h == *(void**)(task->+2FCh + 3D0h)
    && task->vtable[38h]())
    007ED3F0(task->+2FCh, 1);                    /* 0099B76C, 0099B772, 0099B774 */
```

`task+2FCh` is the pilot control block and `task+2F4h` the unit. The third test compares the unit
with the **first element** of the control block's unit array, so **only the flight leader's task
raises the mode**, and because the mode lives on the shared block the whole flight follows it.

The torpedo task inherits the base for vtable slot `+38h`. That body, `0099B710`, is two
instructions, `MOV AL,1` / `RET`, with no Ghidra function (`INT3` padding from `0099B713`). So for
this class the authorisation is unconditional and the lead test is the whole gate.

`009A2810`, the `closetoship` task's countdown at `task+550h`, tail-calls `007ED3F0(task->+404h, 0)`
when the timer runs out, and re-arms the timer with `[00CE3958]` when it finds the mode at `2`. It
is the only route back down to `0`.

## ABI summary

| address | ABI | evidence |
| --- | --- | --- |
| `007BCBE0` | `void __thiscall(unit, int count)`, `RET 4` | `007BCC05` and `007BCC14` |
| `007EEF30` | `void __thiscall(ctl, unit)` | `007C0EFA MOV ECX,[EBP+9D4h]` / `007C0F00 PUSH EBP` |
| `007ED3C0` | `void __fastcall(ctl)`, `RET 0` | `007ED3DD RET` |
| `007C0D90` | `void __fastcall(unit)`, `RET 0` | `007C0F0D RET` after `ADD ESP,0x20` |
| `007B9140` | `char __thiscall(unit, int)`, `RET 4` | `007B9153`, `007B9197`, `007B91A0` |
| `007ED3F0` | `void __thiscall(ctl, int mode)`, `RET 4` | `007ED3FA RET 4` |
| `007ED430` | `void __thiscall(ctl, int mode)`, `RET 4` | `007ED442 RET 4` |
| `0099B740` | `void __fastcall(task)`, `RET 0` | `0099B77A RET` |
| `0099B710` | `char __fastcall()`, `RET 0` | `0099B712 RET`, `INT3` from `0099B713`; **no Ghidra function** |

## Host methods

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `007C0DB8`-`007C0ED3` | the device walk | `unit_carries_droppable_device_007c0d90` | `unit / - / bool` | always |
| `007C0EE2` | - | `set_release_pending_c25` | `unit / bool / void` | the walk matched |
| `007EEF3B` | `007EE7F0` | `pre_issue_hook_007ee7f0` | `ctl / unit / void` | always |
| `007EEF40`, `007EEF54` | - | `read_issue_inputs` | `ctl / - / fields` | always |
| `007EEF69` | - | `controlled_unit_count` | `ctl / - / int` | the gate passed |
| `007EEF6F` | `007B8AD0` | `unit_lacks_follow_target_007b8ad0` | `unit / - / bool` | per controlled unit |
| `007BCBEC`, `007BCBF4` | `007B9140` | `read_set_inputs` | `unit / - / fields` | per issued unit |
| `007BCBFD`, `007BCC09` | - | `write_release_order_count` | `unit / int / void` | per issued unit |

`007EE7F0`, the device walk's six vtable slots, and the scene-node pair `00922F30`/`00922F80` are
this packet's **contracts**, logged through the unimplemented-host mechanism when the host wiring
lands.

## The host wiring

`src/torpedo_release_orders.cpp` is registered in `cmake/startup.cmake` and the issue path runs
where the native runs it. Four changes in `src/game_hosts_units.cpp`:

1. `run_release_order_issue_007c0d90` at the end of the plane's fixed step, matching `007CEA8D`,
   which sits after the think at `007CE865` and the latch at `007CE96F`.
2. `run_attack_mode_tick_0099b740` at the head of the torpedo arm, where `009D4A70`'s tail runs it.
   Only the first ordered torpedo aircraft carries `torpedo_is_flight_lead`, and it publishes the
   mode to the whole flight because the native mode lives on the shared control block.
3. The approach update's `read_control_block` and the transition rule's three readers take that
   live mode instead of the constant `0` they carried.
4. The census reports, per aircraft, the lead flag, the issue ticks, the orders raised, the peak
   and remaining `unit+C58h`, the arming offers, the attack mode and the drop countdown.

`ctl+374h` and `ctl+390h`, the pair whose comparison at `007EEF40` gates the whole issue, are read
in section (5) below. The host was wired before they were read and it logs them through the
unimplemented-host mechanism with the gate taken as open; section (5) shows that is the right
answer for a five-aircraft flight, for a reason rather than by assumption. Replacing the stub with
`flight_armed_fraction_007ee7f0` is left for the next packet, because
`src/game_hosts_units.cpp` passed to `agent/cc8-bank-inputs` as soon as this packet released it.

## (5) The issue gate: `ctl+374h` and `ctl+390h`

Both are **floats**, so the integer store scans of section (1) were the wrong form for them. The
float scans, `F3 0F 11 ?? <disp32>` for `MOVSS` and `D9 ?? <disp32>` for the x87 stores, find the
producers at once.

### `ctl+374h`, the armed fraction, from `007EE7F0`

`void __thiscall(ctl, unit)`, the hook `007EEF30` calls first at `007EEF3B`, writing `+374h` at
`007EE891` and `007EE883`:

```
ctl->+3ECh = 0;
armed = 0; total = 0;
for (i = 0; i < ctl->+3CCh; ++i) {
    u = ((void**)(ctl + 3D0h))[i];
    if (u && u->+5Ch) {                    /* the scene-node enabled byte */
        total += 1.0f;
        n = 007C1F60(u);
        if (u == unit) --n;                /* the caller is about to spend one */
        if (n > 0) armed += 1.0f;
    }
}
ctl->+374h = total > 0 ? armed / total : 0;
```

`007C1F60(unit)` walks the same device list at `unit+48h` that `007C0D90` walks, keeps the devices
of class `25h` holding ordnance `2Ah`, and **sums `006E3500` over them**: the remaining round count.
So `ctl+374h` is the fraction of the flight's enabled aircraft that still have a round, with the
calling aircraft's own next round already deducted.

### `ctl+390h`, the threshold, from `0079CBD0`

The only writer on the control block is `0079CD36`, inside a block at `0079CD05`-`0079CD36` that
seeds five fields at once when `ctl->+3E8h` is zero:

```
v = *(float*)(unit->+538h + A0h) * 0.95;   /* 0079CCE5, FMUL double [00CEFFB0] */
ctl->+384h = 0; ctl->+388h = 0;
ctl->+38Ch = v; ctl->+398h = v; ctl->+394h = v; ctl->+390h = v;
```

`+394h` and `+398h` are the cruise and second altitudes that `009D4A70` later overwrites with
`Pilot/Torpedo/CruisingAlt`, which is how the block is identified. `unit+538h` is the class
descriptor and `+A0h` a field of it: `contract: unread`. `0079CBD0` has no callers and is reached
through a vtable.

### What that means for the runs

The gate is `ctl->+390h > ctl->+374h`. For USN01's flight of five loaded torpedo bombers every
aircraft is enabled and carries a round, and the caller deducts its own, so `+374h = 4/5 = 0.8`.
The gate is therefore open for any descriptor value above about `0.842`, and closed only if the
descriptor field is smaller than that. The rule also **throttles a large formation**: at twenty
loaded aircraft `+374h` is `0.95`, and `0.95 > 0.95` is false, so orders stop going out until some
of the flight has spent its load.

So the wiring's assumption is supported rather than merely stated, with one residual unknown named:
the class descriptor field at `unit+538h`, `+A0h`. `coverage: partial` for `0079CBD0`, of which only
the seeding block is read.

## Corrections

### Correction to `docs/TORPEDO_APPROACH_UPDATE.md` (packet `cc8_torpedo_approach_update`)

That doc's section (1) describes `009D3420`'s no-ordnance branch as measuring the distance between
the approach's target point and "`ctl->+3D0h`'s position", and its `TorpedoApproachControl` carries
`has_designated_target_3d0` as though the field were a single target. `ctl+3D0h` is an **array of
the units the control block drives**, with the count at `ctl+3CCh`: `007EEF5C` walks it with
`LEA EDI,[ESI+3D0h]` and `ADD EDI,4`, and `0099B757` compares a task's own unit against its first
element. `009D3D88` loads that first element, so the branch compares the approach's target point
against the **flight leader's** position and the `!=` test at `009D3D8E` asks whether this aircraft
is not the leader.

That doc's open question 2 asks who writes `ctl->+370h`; section (4) above answers it. Its
follow-up 1 asks who raises `unit+C58h`; sections (1) and (2) answer that.

### Correction to `docs/PILOT_BOT_TICK_GATES.md` (packet `cc2_pilot_bot_tick_gates`)

Its "Open questions" list `unit+C58h` and `007B9140` as "named here only by the role they play in a
gate" and "not read beyond what is quoted". Both are read here: `007B9140` is the droppable-ordnance
capability test and `unit+C58h` is the queued release-order budget `007BCBE0` assigns.

## `no_ghidra_function`

| start | end (inclusive) | name |
| --- | --- | --- |
| `0099B710` | `0099B712` | the base task vtable `+38h`, `MOV AL,1` / `RET`; `INT3` padding from `0099B713`; Ghidra's enclosing candidate is `0099B6E0 TRIV_body_0099b6e0` |

## Validation

`./scripts/build.ps1` (MSVC Win32, `/W4 /WX`) succeeds; `ctest` passes both existing suites
(`reconstructed_math`, `tool_tests`). No test was added.

| run | result |
| --- | --- |
| USN01 before this packet | five aircraft, `transitions=1..3`, `states[moveto=239..434 prepare=865..1060]`, `releases=0`, `unit+C58h` zero on all 6495 arm ticks |
| USN01 after | five aircraft, `transitions=1..4`, `states[moveto=239..434 attackrun=865..1059]`, `releases=0` |
| USN01 orders | per aircraft `issue_ticks=2597 raised=~12981 peak_C58h=999 left=999 arm_offers=1298 blocked_0099af53=1 attack_mode_370=1`; the leader's `raised_at_tick=0` |
| USN01 approach | unchanged: `ticks=1299 no_target=0 replans=130`, `engage 8Ch=2200.0`, closest range `2906.0` (Mav2) |
| USN02 after | `shots=734 first_shot=1.40 s`, `hull=180 part=0 deaths=2 total_damage=18525.6`, `projectiles created=734` - identical to the milestone 2t baseline |
| USN02 torpedo census | `no ordered aircraft carries torpedo ordnance (kind 2Bh), so 0099A170 builds no kind Eh task` |

**Both gates this packet set out to find now pass, and the state changes to prove it.** With
`ctl+370h` live the entry chooser stops sending an engaged task to `prepare` and sends it to
`attackrun` instead: every aircraft that spent 865 to 1060 ticks in `prepare` before now spends
them in `attackrun`. With `unit+C58h` live the arming loop at `0099AF81` offers `009D49A0` its slot
1298 times per aircraft instead of never; only the very first tick is refused at `0099AF53`, before
the plane's fixed step has run.

**Still no release, and the gate is now the range itself.** `009D49A0` arms `prepare+98h` only when
the task's state **is** `prepare` (`009D49B2`), and the rule now correctly keeps these aircraft in
`attackrun`. To reach `aim`, and from there `goaway` and `done` where the drop lives, step 10 of
`009D4030` needs `task+529h`, the in-range latch, and `009D3774` closes that latch only when the
range falls **inside** `approach+8Ch`. The engaged predicate `009D3210` admits 2.2 times that
distance, which is why the rule leaves `moveto` at 4840, but the latch needs 2200 and the closest
any of the five came in the 150-second window was **2906**. The aircraft are not being blocked by a
missing rule; they simply do not arrive. `docs/TORPEDO_APPROACH_UPDATE.md`'s own pilot-attack
census says the same thing from the other side: `closed_mean=-321.6 m`, with two of the five
opening range rather than closing.

So the remaining work is the flight path, not the release logic: the sector plan at `approach+5Ch`
is zero on every tick because `ctl+34Ch`, the terrain sampler, is null in this host, so the aim
heading has no planned run-in, and the `moveto` state's own steering is what carries the aircraft.

## Follow-up packets

1. **Why the five USN01 aircraft do not close inside 2200.** This is now the only thing between an
   ordered torpedo flight and a drop. The `moveto` state's steering and the missing sector plan
   (`ctl+34Ch` is null, so `approach+5Ch` stays zero) are the two candidates.
2. **The class descriptor field at `unit+538h`, `+A0h`**, the one number behind `ctl+390h`, and the
   rest of `0079CBD0`. Replacing the host's issue-gate stub with `flight_armed_fraction_007ee7f0`
   belongs with it, once `src/game_hosts_units.cpp` is free again.
3. **`007C0D90`'s device walk** and the six vtable slots it uses, and `007EE7F0`.
4. **`008A4B10 BSP_LuaBinding_PilotStopCloseToShip`**, the one route to attack mode `2`, and whether
   a mission script on USN01 takes it.
5. **`007D681E`**, the `LEA` that takes a pointer to `unit+C58h` inside
   `BSP_Plane_ReadPropertyBag`, the one site the store scans cannot cover.

## Correction to section (4) from `docs/TORPEDO_ATTACK_MODE.md` (packet `cc8_torpedo_attack_mode_lowering`)

Appended, not rewritten. Section (4)'s three values and its readings of `0099B740` and the Lua
binding hold, but its writer census is incomplete twice over.

* It scanned only `89 ?? 70 03 00 00` and `C7 ?? 70 03 00 00`. A census over every store form that
  can reach a `+370h` field, disp8 and disp32, with the two known sites as a positive control,
  finds a **third writer on this object**: `007F0068`, the `BCh` arm of the control block's message
  dispatcher `007F0030`, which sets the mode to `2` or **`0`** from the byte at `msg+20h`. There is
  no float or byte writer anywhere.
* It used `ghidra callers`, which under-reports. An exhaustive rel32 scan of the two setters finds
  **four** call sites, not three: the missing one is `0084DB86`, inside the undefined routine
  `0084DB50`-`0084DB8D`, which raises the mode to `1`.

So the sentence "`009A2810` … is the only route back down to `0`" is wrong. The `BCh` message is a
second route, and the only one that does not need a `closetoship` task. Section (4) is otherwise
confirmed, including that `009A2810` arms only from mode `2` and belongs to the `closetoship`
task's vtable `+64h` arm.
