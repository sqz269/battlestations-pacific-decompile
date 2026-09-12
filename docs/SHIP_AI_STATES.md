# The ship AI state family at vtable 00D21598 and the block its steps write

Addresses: 009F50E0 009F39C0 009F3DD0 009F3D00 009E1170 009E14C0 009E1610 009E1950 009E5770
009E59C0 009DAC20 009DAC80 009DADB0 009DAE20 009DAF50 009DB050 009DBF90 009DFFB0 009E0040
009ED6B0 009F4D10 009F4DA0 009E0270 009ECA20 009DA6E0 009F0EA0 009E04E0 009EF230 009F1420
009DDBC0 009DA0D0 009DA8D0 008162B0 00811940 00811960 00414C60 00438AA0 00438B10 00D21598
00D215C8 00D215F8 00D21628 00D21658 00D21688 00D216B8 00D219D0 00E08F70 00CECCA8

Packet `cc_ship_ai_states`, worker `agent/cc-ship-ai-states`, 2026-09-11 UTC. Ghidra was
read-only for this packet: no renames, comments, prototypes, function creation or saves.
Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every live query
verified both. Descriptive names are hypotheses, not recovered symbols. The literals in the
image are the nine state names (`cruise`, `stop`, `follow`, `land`, `movetopos`,
`moveonpath`, `attackmove`, `sub_attack`, `kamikaze_attack`) and `attackmove` again at
`00CECCA8`.

## Answer to the packet question

**The ship AI is one object with all nine states embedded in it, and its output never touches
`unit+0FC4h` / `unit+0FDCh`.** Each frame the controller picks the state that matches the
director's current command, runs that state only on a re-plan tick, and then runs a fixed
chain of per-frame routines over one control block. The last two links of that chain,
`009ED6B0` and `009F4D10`, are what reach the unit: they turn the state's desired throttle,
rudder and heading into a heading target and two distances and publish them into a
double-buffered 84-byte slot inside the unit. `docs/UNIT_AUTOPILOT_PAIR.md` carries that half.

### The three objects

`009F50E0` receives one pointer in ECX; call it `ai`. Two sub-objects matter:

| name | address | evidence |
| --- | --- | --- |
| `ai` | the update's `this` | `009F50E2 MOV ESI,ECX` |
| `brain` | `ai+58h` | `009F5231 ADD ESI,0x58` then `MOV ECX,ESI` for `009DA8D0` and `009F4DA0`; `009F4DF7` reads `[ESI+0AA8h]` there and the result's `+184h` is the player-controlled byte |
| `blk` | `ai+60h` = `brain+8h` | `009F5150 LEA EDI,[ESI+0x60]`; `009F50C0 LEA ECX,[ESI+8]` in `009F4DA0` reaches the same object from `brain` |

`009F39C0` is `brain`'s constructor. Its `this` is `brain`, not `ai`: it writes the state
objects at `brain+0B70h` and following and stores `brain` itself as each state's owner.

| field | meaning | evidence |
| --- | --- | --- |
| `brain+0AA8h` | the unit | `009E117B`, `009E11C2` (`+184h`), `009E1207` (`+994h`), `009F4DF7` |
| `brain+0AB8h` | the weapon director | `009E1354` and `009E1384` pass it to `008356E0`/`008356D0` |
| `ai+0B00h` | the unit again, through `ai` | `009F50E4`; `ai+0B00h` is `brain+0AA8h` |
| `ai+0B14h` | seconds until the next re-plan | `009F5112`, `009F51A8`, `009F5215` |
| `ai+0B18h` | seconds accumulated since the last one | `009F512E`, `009F5191` |
| `ai+2264h` | the active state object | `009F5177`, `009F518B`; `009F3E07` reads the same field |
| `blk+3FCh` | the unit, cached on the control block | `009F4D1B`, and the proof in `docs/UNIT_AUTOPILOT_PAIR.md` |

### The nine states

`009F39C0` installs each state's vtable and its owner back-pointer, then registers it by name
through `00411E70` (`009F3ADF..009F3B60`). Every vtable in the family is twelve slots, 30h
apart, starting at `00D21598`. Slot `+0Ch` is the step and slot `+24h` returns the command
object the state serves; `009F3DD0` compares that against the director's current command.

| name | state object | vtable | step `+0Ch` | command `+24h` | evidence |
| --- | --- | --- | --- | --- | --- |
| `cruise` | `brain+0B70h` | `00D21598` | `009E1170` | `009DAC20` -> `00E08F70` | `009F3A1C`, `009F3AE0` |
| `stop` | `brain+0B80h` | `00D215C8` | `009E14C0` | `009DAC80` | `009F3A32`, `009F3AF8` |
| `follow` | `brain+0B8Ch` | `00D215F8` | `009E1610` | `009DADB0` | `009F3A42`, `009F3B0B` |
| `land` | `brain+0BE0h` | `00D21658` | `009E1950` | `009DAF50` | `009F3A6B` |
| `movetopos` | `brain+0C04h` | `00D21628` | `009E5770` | `009DAE20` | `009F3A85` |
| `moveonpath` | `brain+0C0Ch` | `00D21688` | `009E59C0` | `009DB050` | `009F3A95` |
| `attackmove` | `brain+0C18h` | `00D219D0` | not read | not read | `009F3AA6`, `009F3AB2` -> `009E8450` |
| `sub_attack` | `brain+2124h` | not read | not read | not read | `009F3AB7`, `009F3AC3` -> `009E4F90` |
| `kamikaze_attack` | `brain+21FCh` | `00D216B8` | not read | not read | `009F3ACE` |

`attackmove` is not a leaf: `009E8450` gives it its own vtable `00D219D0` and four embedded
sub-states at `00D2174C`, `00D2177C`, `00D217AC` and `00D2171C`, so the object runs from
`brain+0C18h` to `brain+2124h`. Its step was not read.

A state object is sixteen bytes: `+0h` the vtable, `+4h` the owner (`brain`), `+8h` and `+0Ch`
two floats seeded from `00CF5BFC` (-99.0f) at `009F3A22`/`009F3A27`.

### The frame sequence, 009F50E0

`__thiscall(ai)(float seconds)`, `RET 4`, body `009F50E0-009F5251`. No Ghidra function starts
here. Three gates stop it before any host call: `[ai+0B00h] == 0` (`009F50E4`),
`unit+5Dh != 0` (`009F50F2`) and **`unit+61h != 0` (`009F50FC`)**.

| # | call site | callee | this | argument |
| --- | --- | --- | --- | --- |
| 1 | `009F5106` | `009F3DD0` | `ai` | - |
| 2 | `009F5156` | `009E0270` | `blk` | the re-plan flag |
| 3 | `009F516C` | `009F1420` | `brain` | `ai+0B18h` |
| 4 | `009F5186` | `[ai+2264h]->vtable[0Ch]` | the state | `ai+0B18h` |
| 5 | `009F519E` | `[ai+2264h]->vtable[28h]` | the state | - |
| 6 | `009F51AE` | `009DDBC0` | `blk` | - |
| 7 | `009F51B7` | `009DA0D0` | `blk` | - |
| 8 | `009F51C6` | `009ECA20` | `blk` | seconds |
| 9 | `009F51D5` | `009DA6E0` | `blk` | seconds |
| 10 | `009F51E4` | `009F0EA0` | `blk` | seconds |
| 11 | `009F51F3` | `009E04E0` | `blk` | seconds |
| 12 | `009F51FA` | `009EF230` | `blk` | - |
| 13 | `009F5209` | `009ED6B0` | `blk` | seconds |
| 14 | `009F5227` | `009F4D10` | `blk` | seconds |
| 15 | `009F5239` | `009DA8D0` | `brain` | seconds |
| 16 | `009F5248` | `009F4DA0` | `brain` | seconds |

Steps 3 to 6 run only on a re-plan tick; step 7 runs instead on every other frame. The tick is
`ai+0B18h + seconds >= ai+0B14h` (`009F5121..009F513C`, `JB` at `009F5140`). After the state
steps, `ai+0B18h` is cleared and `ai+0B14h` becomes `state->vtable[28h]() * 0.05f`
(`00D0DE84`, `009F51A0`). A true return from step 1 or step 13 clears `ai+0B14h`, which forces
the next frame to be a re-plan tick.

**A ship AI state therefore does not run every frame.** `009E1170`'s three calls into the
setters, which `docs/CRUISE_COMMAND.md` read, happen on re-plan ticks only; the per-frame work
is steps 8 to 14 over `blk`.

### 009F3DD0, which state

`__fastcall(ai)`, `RET 0`, body `009F3DD0-009F3E29`, complete.

```
director = [ai+0B00h]->vtable[114h]()                 ; 009F3DE2
command  = 0071BE40(director)                         ; 009F3DE6
if ([ai+0B00h]+184h) command = 00E08F70               ; 009F3DF3, 009F3DFC
else if (command == 0) return false                   ; 009F3E05
if ([ai+2264h]->vtable[24h]() == command) return false ; 009F3E12
009F3D00(ai, command); return true                     ; 009F3E1B
```

`00E08F70` is the authored `Cruise` object of `docs/CRUISE_COMMAND.md`: a player-controlled
ship is always put into the `cruise` state whatever the director holds.

### The control block and the three setters

`009DBF90`, `009DFFB0` and `009E0040` are called with `ECX = &state->owner` (`009E1362`,
`009E1392`, `009E13A1` all do `MOV ECX,ESI` with `ESI = state+4` from `009E1181`). Each does
`MOV EAX,[ECX]; ADD EAX,8`, so the block they write is `brain+8h`, which is `blk`. The field
table of `docs/CRUISE_COMMAND.md` is confirmed and extended here:

| offset on `blk` | meaning | writer |
| --- | --- | --- |
| `+1C4h` | steering mode: 0 rudder, 1 heading, 2 and 3 navigation | `009DFFB0`, `009E0040` |
| `+1C8h` | a hold flag; while non-zero `009ED6B0` keeps `+1CCh` | `009DBF90` clears it |
| `+1CCh` | the requested ahead/astern direction, 1 or 2 | `009DBF90`, `009ED83B`, `009ED851` |
| `+1D0h` | desired throttle, clamped to `[-1,+1]` | `009DBF90` |
| `+1D4h` | desired rudder, clamped to `[-1,+1]` | `009DFFB0` |
| `+1D8h` | desired heading, stored unclamped | `009E0040` |
| `+324h` | the heading target `009F4D10` publishes | `009ED6B0` |
| `+32Ch` | the first distance `009F4D10` publishes | `009ED6B0` |
| `+330h` | the second distance `009F4D10` publishes | `009ED6B0` |
| `+35Ch` | the latched ahead/astern direction | `009ED6B0` |
| `+3E0h` | the distance used while the ship is making way | not read |
| `+3FCh` | the unit | not read; see `docs/UNIT_AUTOPILOT_PAIR.md` |

`009ED6B0`'s direct-control arm is the converter. Registers are established by filtering the
listing: `ECX = 0` at `009ED767`, `EDI = 2` at `009ED7EC`, `EBP = 1` at `009ED7F7`.

```
if (blk+1C4h != 2) {                                   ; 009ED7D8
  latch the ahead/astern direction from blk+1D0h       ; 009ED802..009ED8A5
      |blk+1D0h| <= 0.05  (00D7A270)  -> stopped, and blk+1D0h = 0
  if (blk+35Ch == stopped)                             ; 009ED8BD
      v = 0092D730([unit+1018h])                       ; 009ED8D1
      blk+32Ch = (v / [[unit+538h]+508h]) * v * 0.5 + [unit+9C8h]
      blk+330h = blk+32Ch                              ; 009ED8EC, 009ED902, 009ED910
  else
      blk+32Ch = blk+3E0h ; blk+330h = blk+3E0h + 2000.0 (00CF0DD8)
  if (blk+1C4h == 0)                                   ; 009ED938
      h = unit->vtable[50h]() ; blk+324h = h           ; 009ED95D, 009ED976
      if astern: blk+324h = wrap(h + pi)               ; 009ED99E
      blk+324h = wrap(blk+324h -/+ 00811940(unit))     ; 009ED9C1, 009ED9E5, 009ED9EC
  else
      blk+324h = blk+1D8h                              ; 009ED947
}
if (blk+3F5h) return 1                                 ; 009ED9FF, 009EDA25
```

So in rudder mode the AI's own heading target is just the unit's heading corrected by the
current yaw rate; the rudder value at `+1D4h` is not used anywhere in this arm. The bearing to
a goal is computed in the navigation arm, `009EDA26-009EF228`, which this packet did not
project: it takes a waypoint through `009E3C00`, writes `blk+32Ch` from
`BSP_Vector2f_LengthWithCutoff` and `blk+324h` from `BSP_Geometry_HeadingAngle` (decompiler
lines 584 and 595 of the saved pseudocode; the call sites were not transcribed), then limits the turn against `blk+3D0h`.

### 009E5770, the movetopos step

`__thiscall(state)(float)`, `RET 4`, body `009E5770-009E59B7`, no Ghidra function. It reads
the unit's position from `unit+0FCh` and `unit+104h` (`009E57AA`, `009E57C2`) and the goal
from `brain+0B2Ch` and `brain+0B34h` (`009E57D0`, `009E57B4`), asks
`009DE050(blk, &out, 0, flag)` for the navigation goal (`009E580F`, body unread), and tests
arrival through the state's own `vtable[2Ch]` (`009E5817`, `009E5821`). Its second arm, the
only pure rule here, is the range test against another entity:

```
d = length2d(goal.xz - unit.xz)                        ; 009E5891..009E58B9, 00414C60
r = (float)[other+7A0h] - [unit+9C8h]                  ; 009E58CE, 009E58D4
continue only while r > d                              ; 009E58DA FCOMPI, 009E58DE JBE
```

`brain+0B2Ch..+0B38h` is the AI's goal vector and its valid flag: the cruise step sets the
flag at `009E11F2` and `009F4DA0` skips its whole body when it is set (`009F4DAF`).
`brain+0B38h` and `unit+0B38h` are different fields that happen to share an offset; do not
conflate them.

### 008162B0, the command-availability predicate

`__thiscall(entity)(name, target)`, `RET 8` (`008162CD` and `00816406`), body
`008162B0-00816408`. The two stack arguments are proved by the `RET 8` and the loads at
`008162B1` and `008162B6`. Its structure is in `include/bsp/ship_ai_states.hpp`; the callees
`00779D50`, `00803510`, `00827F70`, `00852820`, `00465020` and `0080F750` are
`contract: unread` and each has its own host method named by address. The one recovered fact
about its shape: `00816346` compares the name against `"attackmove"` at `00CECCA8` and a
**different** name returns available immediately, so everything from `00816353` on is the
`attackmove` special case.

## Coverage

| Routine | Coverage |
| --- | --- |
| `009F3DD0` | complete |
| `009DBF90` | complete |
| `009DFFB0`, `009E0040` | complete apart from `009DA4E0` and `00605070`, both unread |
| `009F50E0` | complete as a sequence; thirteen of its sixteen callees are unread bodies |
| `009F39C0` | complete for the state table; the base constructor `009F1160`, `009E8450` and `009E4F90` are read only for the objects they install |
| `009ED6B0` | partial: `009ED6B0-009EDA25`, the prologue, the direct-control arm and the early return. `009EDA26-009EF228`, the navigation arm and the tail, are read in pseudocode for the fields they write and are not projected |
| `009E5770` | partial: the field reads and the range test `009E5891-009E58DE`. `009E58E6-009E59B7`, the message the arrival builds, and the `00E08F68` arm at `009E5837-009E5891` are not projected |
| `008162B0` | complete as a control-flow projection; every callee is `contract: unread` |
| `009F4D10` | see `docs/UNIT_AUTOPILOT_PAIR.md` |
| `009E14C0`, `009E1610`, `009E1950`, `009E59C0` | not read. They are named here only as vtable slots |
| `00D219D0` `attackmove` step, `sub_attack` | not read |

## The probe

`src/ship_motion_probe.cpp --moveto X,Z` drives the reconstructed chain every step:
`009DBF90` and `009E0040` on `blk`, then the direct-control arm, then `009F4D10` through
`00811960`, then the promotion of `00825F2C`. Two stand-ins carry the run and neither is
recovered: the bearing to the goal (the unprojected navigation arm) and the hop from the
published slot to the order ring (no reader of the slot was found). The run is therefore
evidence about the published slot, not about the shipped steering law.

`VehicleClass[20]` (DeRuyter class 1935, `MaxSpeed` 16.4622, `MaxRotAngle` 0.122173),
`--throttle 1 --moveto 3000,3000`, 0.05 s steps, class-built hull body:

| steps | seconds | heading (deg) | distance at start | minimum distance | inside 2000 |
| --- | --- | --- | --- | --- | --- |
| 280 | 14.0 | 26.82 | 4242.64 | 4102.46 | no |
| 4000 | 200.0 | 109.27 | 4242.64 | 1328.26 | yes, at step 3074 |

The ship does turn toward the point and does close inside the 2000 of `00CF0DD8`, but it does
not stop there: with a pure heading chase and this hull's turn radius it orbits the goal at
about 1330 units. The published slot holds `+44h` the limited heading, `+40h` 0.0 and `+48h`
2000.0 for the whole run, because the direct-control arm takes its distances from `blk+3E0h`,
whose producer is unread. The slot is promoted on all 4000 steps.

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/CRUISE_COMMAND.md`: "blk = `[state]+8`" | `blk` is `[state+4] + 8`, that is `brain+8h`, a sub-object of the AI controller and not of the state. The setters receive `&state->owner`, not the state | `009E1181 LEA ESI,[EDI+4]`, `009E1362 MOV ECX,ESI`, `009DBF90 MOV EAX,[ECX]; ADD EAX,8`; `009F3A19` stores `brain` into `state+4` |
| `docs/CRUISE_COMMAND.md`: "`unit+61h` chooses between an autopilot pair and the ring's own pair, and the autopilot pair is where a cruise-driven ship's order arrives" | The first half holds; the second is wrong. `unit+61h` is also the ship AI controller's third bail-out gate, so the AI does not run at all while the pair is in use. A cruise-driven ship's order arrives in the unit's 84-byte AI order slot instead | `009F50FC CMP byte [EAX+61h],0; JNE 009F524F` against `008266C1` |
| `docs/CRUISE_COMMAND.md` follow-up `ship_ai_state_machine`: "the whole `moveto`/`follow`/`stop` set follows the same shape as `cruise`" | Only partly. `cruise` is the one state that calls the three setters directly. `movetopos` sets a navigation goal through `009DE050` and never calls them | `009E580F` against `009E1367`/`009E1376`/`009E1397` |
| `docs/CRUISE_COMMAND.md`: `009E1170` "the cruise step" is reached every frame | It is vtable slot `+0Ch` and `009F5186` calls it only on a re-plan tick, at most once every `state->vtable[28h]() * 0.05f` seconds | `009F511A..009F5148`, `009F5177..009F5186` |

## no_ghidra_function

| Start | Inclusive end | Evidence for the boundary |
| --- | --- | --- |
| `009F50E0` | `009F5251` | Thirteen `INT3` at `009F50D3..009F50DF` after `009F4DA0`'s body end `009F50D2`; `PUSH ECX; PUSH ESI; MOV ESI,ECX` prologue at `009F50E0`; the only exit is `RET 4` at `009F5251`; `INT3` at `009F5254..009F525F` and the next function `009F5260`. |
| `009E5770` | `009E59B7` | `PUSH -1; MOV EAX,FS:[0]` SEH prologue at `009E5770`, the target of `00D21634` = vtable `00D21628 + 0Ch`; `RET 4` at `009E59B5` after the SEH unlink at `009E59AB`; eight `INT3` at `009E59B8..009E59BF` and the next vtable target `009E59C0`. |

`009E59C0` (`moveonpath`) likewise has no Ghidra function; its body was not read, so no
boundary is claimed for it. `009F4D10`, `009F3DD0`, `009ED6B0`, `009E14C0`, `009E1610`,
`009E1950`, `009E0270`, `009ECA20`, `009DA6E0`, `009F0EA0`, `009E04E0`, `009EF230`,
`009F1420`, `009DDBC0`, `009DA0D0`, `009DA8D0`, `009F4DA0`, `009F39C0` and `008162B0` all
have Ghidra functions.

## Follow-up packets

| packet | addresses / files | why |
| --- | --- | --- |
| `ship_ai_navigation_arm` | `009ED6B0` `009EDA26..009EF228`, `009E3C00`, `009D9E50`, `00415510`, `00415550`, `00415620`, `blk+3D0h`, `blk+3E0h` | The bearing-to-waypoint rule and the turn limiter: the half of the converter that actually steers a ship toward a point. It writes the same `blk+324h`/`+32Ch`/`+330h` this packet published, so the contract is already fixed. |
| `unit_ai_order_slot_reader` | `unit+0A98h`, `unit+0AECh`, `+40h`, `+44h`, `+48h`, `00825F20` `00825F7C..00826D6B`, `00811890` | The one hop left in the chain: who reads the promoted slot and turns a heading target and two distances into the order ring's `+148h`/`+14Ch`. `docs/UNIT_AUTOPILOT_PAIR.md` states the negative result this packet reached. |
| `ship_ai_state_steps` | `009E14C0`, `009E1610`, `009E1950`, `009E59C0`, `00D219D0`'s step, `009E8450`, `009E4F90` | The five state steps this packet enumerated but did not read: `stop`, `follow`, `land`, `moveonpath` and the composite `attackmove` with its four sub-states at `00D2171C`, `00D2174C`, `00D2177C`, `00D217AC`. |
| `ship_ai_goal_vector` | `brain+0B2Ch`, `brain+0B34h`, `brain+0B38h`, `009DE050`, `009F4DA0` | Who writes the AI's goal vector, and what `009F4DA0` computes on the frames no state has set the flag. |
| `entity_command_availability` | `008162B0`'s callees `00779D50`, `00803510`, `00827F70`, `00852820`, `00465020`, `0080F750`, and `007582B0` | The predicate's six unread callees and its single caller. |

## Uncertainties

1. `blk+32Ch` and `blk+330h` are called distances here because `009ED6B0` builds the first
   from a speed squared over a class field plus a unit length and the second by adding 2000.0,
   and because the navigation arm writes the first from a planar length. Nothing read in this
   packet consumes them, so the units are an inference.
2. `blk+3E0h` has no writer in what was read, so the published `+40h` is 0.0 in the probe.
3. `009F4DA0`'s body was read only at its head and tail. What it computes on a frame with no
   AI goal is not established, and it is the last call of every frame.
4. The `attackmove` object's extent, `brain+0C18h` to `brain+2124h`, is inferred from the two
   constructor call sites and from the next state's offset, not from a size field.
5. `009E5770`'s `vtable[2Ch]` arrival test differs per state; only the `movetopos` call site
   was read, never a callee body, so the test itself is `contract: unread`.
