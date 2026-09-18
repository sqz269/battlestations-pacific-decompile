# `ctl+370h`, the pilot attack mode: a complete writer census, and why nothing lowers it

Addresses: `007ED3F0` (`007ED3F0`-`007ED3FA`, the plain setter), `007ED430` (`007ED430`-`007ED442`,
the raise-only setter), `007F0030` (the pilot control block's message dispatcher; the arm at
`007F005B`-`007F0068`, its tables at `007F01E8`/`007F0204`), `009A2810` (`009A2810`-`009A288A`, the
`closetoship` countdown), `009A2B00` (`009A2B00`-…, the `closetoship` vtable `+64h` arm, calling the
countdown at `009A2B56`), `0099B740` (`0099B740`-`0099B77A`), `0084DB50` (`0084DB50`-`0084DB8D`),
`008A4B10` (the Lua binding, calling the raise-only setter at `008A4C41`), `007ED610`
(`007ED610`-`007ED64C`, the flight-leader rotation), and the two consumers `009D3F60`
(`009D3F69`/`009D3F71`) and `009D4030` (`009D40CB`/`009D40D2`).

`coverage: reconstructed` for the two setters, the message arm and the countdown.
`coverage: census` for the writer set. The producer of message `BCh` is `contract: unread`.

**The result is negative and it is the point: no path a torpedo task runs ever lowers `ctl+370h`.**
Both routes to `0` belong to other objects, and neither fires on USN01. The previous packet named
this field as the gate; this one shows the gate cannot be opened from inside the torpedo task, and
says exactly what would have to happen instead.

## (1) The census, and why the earlier one was incomplete

`docs/TORPEDO_RELEASE_ORDERS.md` section (4) scanned `89 ?? 70 03 00 00` and `C7 ?? 70 03 00 00`
and concluded there were two writers on this object. Two things were missing.

**The store forms.** A dword-only scan cannot see a float or byte store. This packet scanned every
form that can reach a `+370h` field, disp8 and disp32, with `007ED3F4` and `007ED43C` as a positive
control so a silent miss would show up as a failed control rather than a clean negative:

| form | opcode | sites image-wide |
| --- | --- | --- |
| `MOV dword imm32` | `C7 /0` | 5 |
| `MOV dword r32` | `89 /r` | 12 |
| `MOV byte imm8` | `C6 /0` | 0 |
| `MOV byte r8` | `88 /r` | 0 |
| `MOVSS` | `F3 0F 11 /r` | 4 |
| `FSTP` / `FST` | `D9 /3`, `D9 /2` | 7 |
| `MOVSD` | `F2 0F 11 /r` | 0 |

Twenty-eight sites, both controls found. **Three are on the pilot control block**: `007ED3F4`,
`007ED43C` and `007F0068`. The other twenty-five are mission records, squadrons, ship AI, unit
health and the tuning loader, which share the offset by coincidence. There is no float or byte
writer, so the field is a dword everywhere.

**The call sites.** `python tools/bsp.py ghidra callers` under-reports. For `007ED3F0` it returns
two callers; a rel32 scan of the whole `.text`, plus an absolute-address search of `.text`,
`.rdata` and `.data`, returns **four** sites across the two setters:

| site | setter | mode | what it is |
| --- | --- | --- | --- |
| `0099B774` | `007ED3F0` | `1` | the flight leader's cruise-profile tail, `0099B740` |
| `0084DB86` | `007ED3F0` | `1` | `0084DB50`, a routine Ghidra has not defined; `PUSH 1` at `0084DB84` |
| `009A285E` | `007ED3F0` | `0` | a tail `JMP` from the `closetoship` countdown `009A2810` |
| `008A4C41` | `007ED430` | `2` | `008A4B10 BSP_LuaBinding_PilotStopCloseToShip` |

`0084DB86` is the one the caller query missed. Its routine is
`void __thiscall(this, arg)`, `0084DB50`-`0084DB8D`, `RET 4`, with `INT3` padding from `0084DB46`:
it calls `0071C130`, conditionally `007EFB60(this->+224h)` when `this->+16Ch` is the type constant
`00E08FA0`, then `009F6DD0(this->+38h, arg)`, and finally `007ED3F0(this->+224h, 1)`. It raises,
so it does not change the conclusion, but a census that misses a writer is not a census.

## (2) The third writer: the `BCh` message

`007F0030` is the pilot control block's message dispatcher. `007F003B` reads the id byte at
`msg+10h`, `007F003F` subtracts `4Bh`, `007F0042` rejects anything above `73h`, and
`007F004D`/`007F0054` index a byte table at `007F0204` into a jump table at `007F01E8`. Decoding
both tables over the whole `4Bh`..`BEh` range gives six live arms; two matter here:

```
id BCh -> 007F005B   CL = msg->+20h; NEG CL; SBB ECX,ECX; AND ECX,2
                     ctl->+370h = CL ? 2 : 0                        /* 007F0068 */
id BEh -> 007F0077   007ED610(ctl, msg->+1Ch)   the flight-leader rotation
```

So **message `BCh` with a zero payload sets the mode to `0`**. That is the second route down, and
the only one that does not require a `closetoship` task. Nothing in the image builds that message
with an immediate `BCh`: scans for `MOV byte [reg+10h],BCh`, `MOV dword [reg+10h],BCh`, `PUSH 0BCh`
and `MOV reg,0BCh` return no site that survives inspection, so the id reaches the message from a
table or an enum the sender copies. **`contract: unread` — the producer of message `BCh`.**

## (3) The other route down, and why a torpedo bomber never takes it

`009A2810`, `void __thiscall(task, float dt)`, `RET 4`:

```
009A2819  if (task->+550h > 0.0f) {                  /* COMISS against 00D7A218 */
009A2827      if (task->+43Ch != 0) return;          /* the latch stops it dead */
009A2833      t = task->+550h - dt
009A283F      task->+550h = t
009A284B      if (t > 0.0f) return                   /* FLDZ/FCOMIP/JC */
009A285E      JMP 007ED3F0(task->+404h, 0)           /* the mode goes to hold */
          }
009A2869  if (task->+404h->+370h == 2) {             /* only from the forced mode */
009A287A      task->+43Ch = 1
009A2881      task->+550h = 2.0f                     /* 00CE3958 */
          }
```

Its only caller is `009A2B56`, inside `009A2B00`. A rel32 and absolute-address scan of `009A2B00`
returns one reference, the `.rdata` dword at `00D1F54C`. The `closetoship` task's vtable is
`00D1F4E8` (`docs/BOT_TASKS.md`), so `00D1F54C` is its slot **`+64h`**, the per-kind heading arm.

So the countdown runs only inside a `closetoship` task, and it only arms after the mode is already
`2`, which only `008A4B10 BSP_LuaBinding_PilotStopCloseToShip` sets. The sequence the game intends
is: Lua stops a close-to-ship order, the mode goes to `2`, the `closetoship` arm sees `2` with an
expired timer and arms two seconds, and at expiry the mode drops to `0`.

**An ordered torpedo bomber has a kind `Eh` task and no `closetoship` task**, so none of that runs.
Its mode goes to `1` on the flight leader's first cruise tick and stays there.

## (4) The consumers, checked against the listing

Both tests were read rather than trusted, because an inverted comparison here would be silent:

```
009D3F69  CMP dword [EAX+370h],0      009D3F71  JNZ 009D3F9F
          -> falls through at 0, sets task->+310h = task+740h (prepare) and
             tail-jumps its enter at 009D3F9D

009D40CB  CMP dword [EAX+370h],0      009D40D2  JNZ 009D40E8
          -> falls through at 0, calls 009D3E40(task, task+740h) = SetState(prepare)
```

Both are `== 0`, so the reconstruction in `src/torpedo_task_arm.cpp` is faithful. `009D49A0` arms
`prepare+98h` only in `prepare` (`009D49C2`), and `009D4132` sends a completed `goaway` back to
`aim` while `task+52Ah` holds, never to `prepare`. The attack cycle therefore cannot reach the
release countdown on its own.

## ABI

| address | ABI | evidence |
| --- | --- | --- |
| `007ED3F0` | `void __thiscall(ctl, int mode)`, `RET 4` | `007ED3FA` |
| `007ED430` | `void __thiscall(ctl, int mode)`, `RET 4`; raises only, signed `<` | `007ED434`/`007ED43A` |
| `007F0030` | `char __thiscall(ctl, msg*)`, `RET 4` | `007F0074`, `007F0089` |
| `009A2810` | `void __thiscall(task, float dt)`, `RET 4` | `009A288A` |
| `009A2B00` | `void __thiscall(task, float dt)`, `RET 4`; `closetoship` vtable `+64h` | `00D1F54C` |
| `0084DB50` | `void __thiscall(this, arg)`, `RET 4` | `0084DB8D`, `INT3` from `0084DB46` |
| `007ED610` | `void __thiscall(ctl, int index)`, `RET 4` | `007ED64C` |

## Host methods

| host binding | native | site |
| --- | --- | --- |
| `pilot_control_set_attack_mode_007ed3f0` | `007ED3F0` | `007ED3F4` |
| `pilot_control_raise_attack_mode_007ed430` | `007ED430` | `007ED43C` |
| `pilot_control_attack_mode_from_message_007f0068` | `007F0030` arm `BCh` | `007F0068` |
| `closetoship_attack_mode_countdown_009a2810` | `009A2810` | `009A285E` |

Both lowering bindings are wired and both are inert on USN01, by construction rather than by
omission: the countdown needs `torpedo_closetoship_task_installed`, which no ordered aircraft sets
because it has no `closetoship` task, and the message arm needs a pending `BCh`, which nothing
produces. The census reports whether either ever fired, so the moment a producer appears the
numbers move without another packet touching this code.

## Corrections

### Correction to `docs/TORPEDO_RELEASE_ORDERS.md` section (4)

Appended, not rewritten. That section's table is right about the three values and about
`0099B740` and the Lua binding, but its writer census is incomplete in two ways. It scanned only
the two dword store forms, and it used `ghidra callers`, which under-reports. The complete census
is section (1) above: a third writer on this object at `007F0068` (message `BCh`, which sets `2` or
**`0`**), and a fourth call site at `0084DB86` inside the undefined routine `0084DB50`, which
raises to `1`. Its sentence "`009A2810` … is the only route back down to `0`" is therefore wrong:
the `BCh` message is a second route, and the one that does not need a `closetoship` task.

### Correction to `docs/TORPEDO_GOAWAY_RELEASE.md`

Appended, not rewritten. Its follow-up 1 asks what lowers `ctl+370h`. The answer is that nothing a
torpedo task runs does, and both external routes are named above.

## no_ghidra_function

| address | inclusive end | what it is |
| --- | --- | --- |
| `0084DB50` | `0084DB8D` | raises the attack mode to 1; `INT3` padding from `0084DB46` |
| `009A2B00` | (not read to its end) | the `closetoship` vtable `+64h` arm; only its call to `009A2810` at `009A2B56` was needed, and the rest is `coverage: partial` |

## Validation

* `build-tested`: `./scripts/build.ps1`, Win32 `/W4 /WX`, clean; `ctest` 2/2.
* Tree base `9ac3fcbb7`. `main` moved again before this packet, so the USN02 baseline is
  this tree's own before-run rather than the figure quoted in the brief.

**USN01 after**, 3200 frames, 3000 mission frames at 0.05 s. The mode column is arm ticks:

| aircraft | mode now | raised at tick | hold | attack | forced | prepare entries | first prepare tick | first empty-queue tick | coincide | releases |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Mav1 | 1 | 0 | 0 | 1299 | 0 | 0 | -1 | 1 | no | 0 |
| Mav2 | 1 | -1 | 1 | 1298 | 0 | 1 | 1 | 1 | yes | 0 |
| Mav3 | 1 | -1 | 1 | 1298 | 0 | 1 | 1 | 1 | yes | 0 |
| Mav4 | 1 | -1 | 1 | 1298 | 0 | 0 | -1 | 1 | no | 0 |
| Mav5 | 1 | -1 | 1 | 1298 | 0 | 0 | -1 | 1 | no | 0 |

The mode is at `0` for exactly one arm tick on four of the five aircraft, and never on the
flight leader, which raises it on tick 0. Two aircraft use that single tick to enter
`prepare`. Neither lowering binding fired: `lowered_009A285E` and `message_007F0068` are 0
everywhere, which is the native's behaviour on this mission and not a host gap.

State sequences after:

* `Mav1`: moveto=188 attackrun=614 goaway=126 aim=371, transitions 195
* `Mav2`: moveto=191 attackrun=599 goaway=151 aim=357 prepare=1, transitions 186
* `Mav3`: moveto=139 attackrun=624 goaway=170 aim=365 prepare=1, transitions 184
* `Mav4`: moveto=268 attackrun=586 goaway=87 aim=358, transitions 176
* `Mav5`: moveto=234 attackrun=594 goaway=99 aim=372, transitions 197

Torpedo task summary after:

* summary mission torpedo task: aircraft=5 releases=0 blocked_engaged_009d3210=1020 blocked_arm_009d49a0=0
* summary mission torpedo task: no release. 009D15F0 writes the aim-complete byte (5 of 5) and 009D3150 is computed from goaway+24h=700.0 (true for 1, furthest 766.5 m). The gate is ctl+370h, the attack mode: 009D3F69 and 009D40CB send an engaged task to prepare only while it is 0, and 009D49A0 arms prepare+98h only in prepare. 0099B774 raised it to 1 and it stayed there for 6486 of 6490 arm ticks; prepare was entered 2 times. Neither route back to 0 is one a torpedo task runs: 009A285E needs the closetoship task's countdown after a Lua PilotStopCloseToShip, and 007F0068 needs the BCh message on the control block. closest approach 285.2 against 8Ch=2200.0, 6490 offers, aim ran 1823 ticks

Ordnance: summary mission gunnery ordnance units_with torpedo=14 general_bomb=11 drop_kamikaze=0 paratrooper=0 (of 41 units with guns); summary mission gunnery torpedo_ranges_derived=39 swims_started=0 snaps=0 bullet_ranges_derived=336 base_tick_timers_live=0 expired=0

**USN02**, against this tree's own merged-tree before-run:

```
before: summary mission gunnery damage queued_hits=126 dispatched=126 hit_records=126 hull=125 part=0 fires=0 floods=0 attributions=126 deaths=3 kill_credits=3 total_damage=13673.7 first_hit=41.85 s
after:  summary mission gunnery damage queued_hits=126 dispatched=126 hit_records=126 hull=125 part=0 fires=0 floods=0 attributions=126 deaths=3 kill_credits=3 total_damage=13673.7 first_hit=41.85 s
```

The second USN01 run added tick recording only, which cannot change behaviour, so the
USN02 after-run from the previous build still applies.

### The next gate, by address and value

**`unit+C58h`, the queued release-order count, is `0` on arm tick 1** - the only tick
`ctl+370h` is still `0`. `009D49A0` therefore takes its no-order arm at `0099AF53`,
`prepare+98h` is never set, and by tick 2 the mode is `1` and `prepare` is unreachable for
the rest of the mission. This is measured, not inferred: for Mav2 and Mav3 the first
`prepare` tick and the first empty-queue tick are both arm tick 1. The order issuer
`007C0D90` raises `unit+C58h` later in the mission, by which time the window has closed.


## Follow-up packets

1. **The `BCh` message producer.** It is the only route to mode `0` that does not need a
   `closetoship` task, and it is the shortest path to a torpedo release.
2. **`task+424h`, the manual passthrough budget.** `009D4956` releases through `007BBBA0` outside
   `aim`/`attackrun`/`prepare`, which the aircraft now occupy for real time. A census of every store
   form at `+424h` finds thirty sites and **none that raises it**: the torpedo band has only
   `009D3EBB`, `009D3ED3` and `009D495B`, which decrement or clear. Its positive value comes from
   outside the image's store sites, so the order layer is the place to look.
3. **`ctl+3D0h`, the flight array.** `0099B740` raises the mode only when the task's unit is
   `ctl->+3D0h[0]`, and `007ED610` rotates that array. When the array is populated relative to the
   first cruise tick decides whether a `prepare` window exists at all.

## Correction from docs/PLANE_SQUADRON_ENTITY.md

Appended by packet `cc8_plane_squadron_entity`. The text above is left as written, and its
decoding of the `BCh` and `BEh` arms is unaffected.

This document calls `007F0030` "the pilot control block's message dispatcher". `ghidra xrefs
007f0030` returns exactly one reference, the `.rdata` dword at `00D08924`. That is offset `+164h`
of `00D087C0`, the **plane squadron's primary vtable** (`docs/ENTITY_CLASS_IDS.md` row 18,
`PlaneSquadronGen`). So the "pilot control block" this dispatcher serves is the squadron object
itself, and the document's `ctl` is the squadron throughout.

The same identification settles the `ctl+3D0h` reading in the follow-up section: `ctl+3D0h` **is**
the flight array with count `ctl+3CCh`, exactly as this document says, and
`docs/AI_COMMAND_LIFETIME.md`'s competing reading of it as a carrier link is the one that was
wrong. Two further slots of the same vtable belong with `007F0030`: `+114h` is `007ECFD0`
(`MOV EAX,[ECX+348h]; RET`, the squadron's AI command block) and `+128h` is `007ECF80`, a loop over
`+3D0h` bounded by `+3CCh` that calls the same slot on every member plane. Neither has a Ghidra
function; both are decoded in `docs/PLANE_SQUADRON_ENTITY.md` section 4.
