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

## Host methods

Nothing in this packet could be bound to the release request, and the reason is a lease, not the
evidence. `release_ordnance_007bbba0` lives in `src/game_hosts_units.cpp`, which was leased to
`agent/cc8-run-profile` under packet `cc8_torpedo_run_profile` for the whole of this packet's
window. The request lands in `GameUnitSlot`, and that struct is declared inside that same `.cpp`
with no header, so no other translation unit can observe that a release was requested. The one-line
hookup is recorded under "Follow-up packets" for whoever holds the file next.

What this packet did add, inside its own ownership, is the measurement that says where a round that
could swim actually stops.

| host method | file | what it does |
| --- | --- | --- |
| torpedo gate census | `src/game_hosts_gunnery.cpp` | one counter per conjunct of the gun loop's `want_fire`, restricted to guns whose bullet class derived a swim speed |

A gun counts as a torpedo gun when `swim_speed > 0`, which is the identical test the water crossing
uses to decide that a round swims instead of dying at the surface. A gun counted in the funnel is
exactly a gun whose shot could reach the swim model. These counters are **not** reconstructions and
no native address produces them.

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

Both mission runs of the instrumented build were still queued behind the machine-wide game lock
when this packet's window closed, so the funnel counters themselves are **unmeasured**. The gate
above is measured, from the baseline; the funnel is the instrument left in place to confirm it.

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

See the run table in `reports/torpedo_release_spawn.json` under `validation`.

## Follow-up packets

1. **The unapplied hookup.** In `src/game_hosts_units.cpp`, `release_ordnance_007bbba0` should stop
   logging unimplemented and instead drive channel C of the unit's actuator block: target 1, value
   1.0f, moving 1, `+11h` 1, guarded on the channel's enabled byte and the slot byte, which is
   exactly `007BBBA0`. That is a door command and by itself still spawns nothing.
2. **The real spawn.** Read the bomb platform's release slot in vtable `00CFE308` and bind the
   plane's torpedo platform to the gunnery host's shot creation, the way a ship's tube already is.
   `00730B80` proves the platform is a gun, so the two should share `0072F830`.
3. **The ship gate.** Act on whichever conjunct the torpedo gate census names.
