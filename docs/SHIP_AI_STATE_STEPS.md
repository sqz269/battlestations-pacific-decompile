# What one frame of each ship AI state step does

Addresses: 009DE050 009DA4E0 009DFF40 009E00A0 009E14C0 009E5770 009E8820 009E86F0 009E8450
009E8540 009DAE20 009E59C0 009E1610 009E1950 009E2020 009E23B0 009E26C0 009E4B90 009F3240
0071BFF0 007ADC60 0071C4F0 0071E430 00D219D0 00D21994 00D2171C 00D2174C 00D2177C 00D217AC
00D21688 00D216B8 00D20278 00D21530 00CE65D0 00D7A208 00E08F68 00E08F78

Packet `cc_ship_ai_state_steps`, worker `agent/cc-ship-ai-state-steps`, 2026-09-12 UTC. Ghidra was
read-only for this packet: no renames, comments, prototypes, function creation or saves. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every live query verified both.
Descriptive names are hypotheses, not recovered symbols. `docs/SHIP_AI_STATES.md` is the parent
packet; this one reads the step bodies it enumerated.

## Answer to the packet question

**`009DE050` is not a computation the nine steps share. It is the AI's one navigation-goal
writer, and a state step's whole job is to produce the two floats it takes.** A step never calls
`009DBF90` / `009DFFB0` / `009E0040` to steer toward a point; `cruise` is the only state that
uses the three setters, exactly as the parent packet's correction said. Every other state hands
`009DE050` a 2D goal, which forces `blk+1C4h` to `Navigate` (2) and lets the navigation arm of
`009ED6B0` (`docs/SHIP_AI_NAVIGATION_ARM.md`) do the steering. The states differ only in where
the goal comes from and in what they do when they arrive.

| state | where the goal comes from | arrival test | what arrival does |
| --- | --- | --- | --- |
| `stop` | the world origin `(0,0)`, but only while the unit is outside the world box | none | nothing; it holds heading and zero throttle every frame |
| `movetopos` | `brain+0B2Ch` / `brain+0B34h`, the AI goal vector | `state->vtable[2Ch]`, then a range test against the command's target entity | posts a `finished` message, ends the command, holds heading |
| `moveonpath` | the command's current waypoint, `slot->vtable[4](&tmp,-1)` | the same `vtable[2Ch]` | the same `finished` / end-command / hold sequence |
| `attackmove` | none of its own; it delegates to one of five embedded sub-states | none | hands the command back when the whole group can serve it |
| `follow` | `state+14h` / `state+18h`, a formation point the step itself maintains | none read | not read |
| `land`, `kamikaze_attack` | not projected | not read | not read |

The single answer to "which setters, with what values" is therefore: **none of the three, on any
state this packet read except `stop`**, which calls `009E0040` with the unit's own heading and
writes `blk+1D0h = 0` directly. The desired-throttle path that `docs/SHIP_AI_THROTTLE_TO_RING.md`
follows is fed by `cruise` and by the direct-control arm, not by the navigation states.

## 009DE050, the navigation goal setter

`__thiscall(blk)(const float* goal2d, char keep_mode, unsigned char final_leg)`, `RET 0Ch`, body
`009DE050-009DE1A6`, complete. Three stack arguments from the `RET 0Ch` and the loads at
`009DE063` (`[ESP+18h]`), `009DE087` (`[ESP+1Ch]`) and `009DE139` (`[ESP+20h]`); `ECX` is `blk`
directly (`009DE055 MOV ESI,ECX`, then `[ESI+1C4h]`), so callers do `ADD ECX,8` on `brain` first
(`009E580C`, `009E1509`, `009E22E8`, `009E5ADF`).

```
blk+1C8h = 1 ; blk+1CCh = 0                              ; 009DE067, 009DE071
if (blk+1C4h == 2 || blk+1C4h == 3) {                    ; 009DE05F, 009DE07D
    moved = |goal - (blk+1DCh, blk+1E0h)|^2              ; 009DE0A0..009DE0C9, one float store
    if (moved > 2500.0f)   blk+314h = 0 ; blk+2FDh = 0 ; blk+2FEh = 0   ; 00D20278
    planned = |goal - (blk+1E8h, blk+1ECh)|^2            ; 009DE0F6..009DE11F
    replan = ((double)planned > 6400.0)                  ; 00D21530, a double
} else {
    009DA4E0(blk)                                        ; 009DE082, drop the path plan
    if (keep_mode) goto store                            ; 009DE087
    blk+1C4h = 2 ; replan = true                         ; 009DE091, 009DE133
}
store:
blk+1DCh = goal.x ; blk+1E0h = goal.z                    ; 009DE137, 009DE143
blk+1E4h = final_leg                                     ; 009DE14C
blk+1C4h = 2                                             ; 009DE152
if (replan && !keep_mode) {                              ; 009DE15C, 009DE15E
    blk+1E8h = goal.x ; blk+1ECh = goal.z                ; 009DE165, 009DE171
    blk+1F0h = 00414C60(goal - (blk+184h, blk+188h))     ; 009DE17C..009DE193
}
```

Three facts this settles for the rest of the chain.

* **`blk+1C4h = 2` has a writer.** `include/bsp/ship_ai_states.hpp` says "no site that writes 2 or
  3 was read"; `009DE050` writes 2 at `009DE091` and again at `009DE152`, unconditionally. Every
  ship AI state except `cruise` therefore puts the block into `Navigate` every time it steps, which
  is what opens the navigation arm of `009ED6B0` at all.
* **`blk+1E4h` is the final-leg flag** the navigation arm reads at `009EE6DE`, where it combines
  with the last-waypoint local into the byte at `[ESP+43h]`. The value comes from `007ADC60`.
* **`blk+1F0h` is seeded here**, with the straight-line distance from the block's pose
  (`blk+184h` / `blk+188h`) to the goal. `009EE6B5..009EE6CB` raises it to the planner's path
  length and `009EE856` scales the turn-lead taper by `1.5 *` it, so this seed is what a ship's
  turn lead tapers against on the first frames of a new goal.

`009DA4E0` (`__fastcall(blk)`, `RET 0`, body `009DA4E0-009DA58D`, complete) is the path-plan reset
the parent packet left as `contract: unread` on `ShipAiSetterHost`. It releases the two owned path
objects through their own `vtable[0](1)` (`009DA4E9`, `009DA524`) and clears fourteen fields plus
the three bytes at `+2FCh`, `+2FDh`, `+2FEh`; `+254h` and `+2BCh` take `[00CF58EC]`.

### 0071BFF0 and 007ADC60, where `final_leg` comes from

`0071BFF0`: `__thiscall(director)(int index)`, `RET 4`, body `0071BFF0-0071C00F`, complete.
Returns `[director + 1A4h + 4*index] + 10h` when `index <= 9` and the slot is non-null, otherwise
`[director+1A4h] + 10h`. So the director holds ten command slots at `+1A4h` and the routine hands
back a sub-object 10h into one of them.

`007ADC60`: `__thiscall(slot)()`, `RET 0`, body `007ADC60-007ADCBD`, complete. Reads a list at
`slot+4h` whose count is `list->vtable[0Ch]()`, a mode at `slot+0Ch`, a cursor at `slot+8h` and a
reverse byte at `slot+10h`.

```
if (slot+4h == 0) return true                            ; 007ADC6C..007ADC71
if (list->vtable[0Ch]() <= 0) return true                ; 007ADC7A
if (slot+0Ch != 1) return false                          ; 007ADC87..007ADC95
if (slot+10h) { if (slot+8h == count-1) return true ; return false }   ; 007ADC96..007ADCB2
return (slot+8h == 0)                                    ; 007ADCB4
```

So it is "this goal is the command's final leg": true for a command with no path, and for a
mode-1 path sitting on its last waypoint (or its first when the reverse byte is clear).

## 009E14C0, the `stop` step

`__thiscall(state)(float)`, `RET 4`, body `009E14C0-009E1605`, complete. vtable `00D215C8` slot
`+0Ch`, the state at `brain+0B80h`. The float argument is never read.

Host methods in call order, for milestone 2o:

| # | call site | callee | method |
| --- | --- | --- | --- |
| 1 | `009E14E1` | `00414DB0` | refresh the unit pose, only when `unit+0C8h` is 0 (`009E14D7`) |
| 2 | `009E14F3` | `0071C4F0` | `[00E188A8]` world-bounds test on `&unit+0FCh` |
| 3 | `009E1518` | `009DE050` | set the goal to `(0,0)`, `keep_mode = 0`, `final_leg = 1` |
| 4 | `009E1534` | `[unit]->vtable[50h]` | the unit's current heading |
| 5 | `009E153C` | `009E0040` | `SetDesiredHeading(that heading)` |
| 6 | `009E1570` | `0092D730` | the body-axis speed of `[unit+1018h]` |

Steps 3 and 4 to 6 are exclusive. `0071C4F0` (`__thiscall(world)(const float* pos3)`, body
`0071C4F0-0071C547`, complete) returns **0 when the position is inside** the box
`[+711Ch, +7128h]` by `[+7130h, +7124h]` and 1 otherwise, so `AL != 0` at `009E14FA` means the
unit is **outside the world**. That is the only case in which `stop` moves: it navigates to the
world origin. Inside the box it holds heading and stops:

```
blk+1C8h = 0 ; blk+1D0h = 0.0f ; blk+1CCh = 0            ; 009E1549, 009E154F, 009E1557
if (state+8h) {                                          ; 009E155D, a BYTE latch
    v = |0092D730([unit+1018h])|                         ; built as -0.0f - v (00D7A208), not fabs
    if ((double)v < 0.4) state+8h = 0                     ; 00CE65D0 = (double)0.4f, JBE 009E15AF
}
if (state+8h == 0) { brain+3FCh = 0 ; brain+3F8h = -1 ; brain+3F4h = 1 }   ; 009E15BB..009E15D2
else               { brain+3FCh = 1 ; brain+3F8h =  3 ; brain+3F4h = 1 }   ; 009E15E4..009E15FB
```

Those three fields are `blk+3F4h`, `blk+3F0h` and `blk+3ECh` (the writes use `brain`, and
`blk = brain+8h`). Two readers were located in the per-frame chain: `009DA6E0` (chain step 9)
tests `blk+3F4h` at `009DA6E6` and then queries a class byte at `+242h`, and `009F0EA0` (chain
step 10) tests `blk+3F0h` at `009F1052` with `JL` and compares against another entity's `+54h`
at `009F105C`, which `docs/ENTITY_COMMAND_ARMS.md` reads as a side id. `blk+3F0h = -1` is
therefore "no side filter" and 3 a side; `blk+3ECh` has no reader in what was scanned.

`state+8h` is used here as a byte, not as the `-99.0f` float the constructor stores at
`009F3A22`. The low byte of `-99.0f` (`C2C60000`) is zero, so the latch reads false on the first
frame either way.

## 009E5770, the `movetopos` step

`__thiscall(state)(float)`, `RET 4`, body `009E5770-009E59B7`, complete. vtable `00D21628` slot
`+0Ch`, the state at `brain+0C04h`. Its command getter `009DAE20` is `MOV EAX,0E08F68h; RET`,
which is the `movetopos` row of milestone 2n's `kShipAiCommandStates`. The float argument is
never read. `ESI = state+4` throughout, so `[ESI]` is `brain`.

| # | call site | callee | method |
| --- | --- | --- | --- |
| 1 | `009E57A5` | `00414DB0` | refresh the unit pose when `unit+0C8h` is 0 |
| 2 | `009E57ED` | `0071BFF0` | command slot 0 of `[brain+0AB8h]` |
| 3 | `009E57F4` | `007ADC60` | is that command on its final leg |
| 4 | `009E580F` | `009DE050` | goal `(brain+0B2Ch, brain+0B34h)`, `keep_mode = 0`, `final_leg` |
| 5 | `009E5821` | `state->vtable[2Ch]` | the state's arrival test on the same goal, `contract: unread` |
| 6 | `009E5847` | `00521EA0` | resolve the command target from `director+58h` |
| 7 | `009E585F` | `target->vtable[5Ch]` | entity-kind predicate with `1Ch`, `contract: unread` |
| 8 | `009E5874` | `00414DB0` | refresh the target's pose when `target+0C8h` is 0 |
| 9 | `009E58B9` | `00414C60` | planar length of `unit.xz - target.xz` |
| 10 | `009E58EF` | `0041E870` | assign the message text `"finished"` (`00D09FD8`) |
| 11 | `009E595C` | `00984300` | `(unit, &payload, 00E08F68, &text)`, `__stdcall` `RET 10h` |
| 12 | `009E597C` / `009E5983` | `00419CC0` / `00BD1510` | release the text buffer |
| 13 | `009E5997` | `0071E430` | `(director, 00E08F68, 1)`, end the command |
| 14 | `009E599E` | `009E00A0` | hold heading and stop |

The gate between the two halves is `009E5823`/`009E5829`: **only an arrival on the command's
final leg finishes outright**. Anything else falls into the `00E08F68` arm, which this packet
projects for the first time:

```
if ([brain+0AB8h]+54h != 00E08F68) return                ; 009E5837
target = 00521EA0([brain+0AB8h]+58h)                     ; 009E5844, 009E5847
if (!target) return                                      ; 009E584E
if (!target->vtable[5Ch](1Ch)) return                    ; 009E585B..009E5863
d    = unit.xz - target.xz                               ; 009E5891..009E58B5
dist = 00414C60(&d)                                      ; 009E58B9, rounded to float32 at 009E58C0
r    = (float80)(int)[target+7A0h] - [unit+9C8h]         ; 009E58CE FILD, 009E58D4 FSUB
if (r <= dist) return                                    ; 009E58DA FCOMIP, 009E58DE JBE
```

So a `moveto` issued against a **unit** keeps running while the ship is farther than
`target+7A0h - unit+9C8h` from it, and finishes as soon as it is inside that range even without
reaching the goal point. `target+7A0h` is an integer (`FILD`), not a float. The subtraction and
the compare are one 80-bit chain with no intermediate store.

## 009E59C0, the `moveonpath` step

`__thiscall(state)(float)`, `RET 4`, body `009E59C0-009E5C90`. No Ghidra function; the boundary is
in the table below, and the vtable slot `00D21688+0Ch` holds `009E59C0`. **Partial**: the head, the
goal source and the arrival gate are read; `009E5B17-009E5C8D` is the same `finished` block
`movetopos` has and was not transcribed instruction by instruction.

```
if (state+8h == 0 && !007ADC30(0071BFF0(director,0))) goto body   ; 009E59DE..009E59FF
if (007ADC60(0071BFF0(director,0))) { 009E00A0(&state->owner); return }  ; 009E5A15..009E5A20
state+8h = 0                                             ; 009E5A39
body:
slot = 0071BFF0(director, 0)                             ; 009E5A46
p    = slot->vtable[4](&tmp, -1)                          ; 009E5A50..009E5A59
goal = (p[0], p[2])                                       ; 009E5A5B, 009E5A5F
pos  = (unit+0FCh, unit+104h)                             ; 009E5A87, 009E5A9D
final = 007ADC60(0071BFF0(director, 0))                   ; 009E5AB3
if (!final) brain+308h = 1.0f                             ; 009E5AC0..009E5ACA, 00D7A24C
009DE050(blk, &goal, 0, final)                            ; 009E5AE2
if (state->vtable[2Ch](&goal)) { ... the target range test and the finished block ... }
```

It is `movetopos` with the goal taken from the command's waypoint list instead of from
`brain+0B2Ch`. `007ADC30` (`009E59F8`) is a second predicate on the same slot and was not read:
`contract: unread`.

## 009E8820 and 009E86F0, the `attackmove` step

`009E8820`: `__thiscall(state)(float seconds)`, `RET 4`, body `009E8820-009E88F9`, complete. No
Ghidra function; the boundary is in the table below, and `00D219D0+0Ch` holds `009E8820`. The
state's command getter `009E8540` is `MOV EAX,0E08F78h; RET`, the `attackmove` object milestone
2n already names.

| # | call site | callee | method |
| --- | --- | --- | --- |
| 1 | `009E883F` | `[unit]->vtable[5Ch]` | entity-kind predicate with `9`, `contract: unread` |
| 2 | `009E8873` | `0070D060` | group member `i`: `[group + 34h*i + 18h]`, body `0070D060-0070D06D` |
| 3 | `009E888C` | `[member]->vtable[5Ch]` | the same predicate on each member |
| 4 | `009E88BD` | `[unit]->vtable[114h]` | the director, `contract: unread` |
| 5 | `009E88C1` | `0071E430` | `(director, 00E08F78, 1)`, end the command |
| 6 | `009E88D8` | `009E86F0` | pick the sub-state |
| 7 | `009E88F0` | `[state+1508h]->vtable[0Ch]` | run the chosen sub-state's step |

The step's whole body is a group check: if the unit and **every** member of `[unit+284h]` answers
the `vtable[5Ch](9)` predicate, the state gives the command back to the director (steps 4 and 5)
and does nothing else. Otherwise it runs steps 6 and 7. The two pushes at `009E88B6` and
`009E88B8` belong to `0071E430`, not to the getter: `0071E430` is `RET 8` (`0071E469`) and there
is no `ADD ESP` after `009E88BD`.

`009E86F0`: `__thiscall(state)(float seconds)`, `RET 4`, body `009E86F0-009E881A`, complete. This
is the sub-state selector and the one place in the family where the argument seconds is used.

```
if (seconds < state+1500h) { state+1500h -= seconds; return }   ; 009E8706 FCOMI, 009E8708 JC
state+1500h += (state+14FCh - seconds)                          ; 009E870A FSUBR, 009E871C FADDP
target = [brain+0B20h]                                          ; 009E8714
if (!target) {                                                  ; 009E8724
    if ([state+1508h] == state+8h) return                       ; 009E87F7
    [state+1508h]->vtable[8]() ; [state+1508h] = state+8h        ; 009E8804, 009E8806
    (state+8h)->vtable[4]() ; return                             ; 009E8813
}
if (target->vtable[5Ch](8)) {                                    ; 009E8733
    if (00852860(target))  want = [brain+0B28h] ? state+14CCh : state+14E0h  ; 009E873B..009E876C
    else if ([state+1508h] != state+14C0h) want = state+8h       ; 009E877B, 009E8783
    else goto tail
} else if (target->vtable[5Ch](1Ch)) want = state+8h             ; 009E8799, 009E87B6
else if ([state+1508h] != state+8h && != state+14C0h) want = state+8h  ; 009E87A2..009E87B4
else goto tail
007B6EE0(state+1504h, want)                                      ; 009E87BD
tail:
if ([state+1508h] == state+8h && 009E85B0(state+8h))             ; 009E87C5, 009E87CD
    007B6EE0(state+1504h, state+14C0h)                           ; 009E87E3
```

`00852860` and `009E85B0` are `contract: unread` and carry address-named host methods.
`007B6EE0` was read: `__thiscall(machine)(member)`, `RET 4`, body `007B6EE0-007B6F0A`. It returns
at once when `machine+4h` is already `member`, otherwise calls the old member's `vtable[8]`,
stores the new one at `machine+4h` and calls its `vtable[4]`. `state+1504h` is that machine
(`009E84FF` stores vtable `00D05704` there) and `state+1508h` is its `+4h`, so the no-target arm
of `009E86F0` is the same routine written out inline.

### The five sub-states `009E8450` builds

`009E8450` (`__thiscall(state)(brain)`, body `009E8450-009E8533`) writes the composite. The doc it
corrects said four; there are five, and the fifth is the one the object starts on.

| state offset | vtable | step, vtable `+0Ch` | constructed at |
| --- | --- | --- | --- |
| `+8h` | `00D21994` | `009F3240` | `009E8486`, by `009E5CA0`, which nests another object at `+10h` through `009E5530` |
| `+14C0h` | `00D2174C` | `009E23B0` | `009E8491` |
| `+14CCh` | `00D2177C` | `009E26C0` | `009E84A7` |
| `+14E0h` | `00D217AC` | `009F3670` | `009E84B7` |
| `+14F4h` | `00D2171C` | `007B3DD0` | `009E84CA`; `009E850F` makes it the initial member |

`state+14FCh` is `[00D7A24C]` = 1.0f (`009E84F0`), so the selector re-picks at most once a second.
`state+1500h` is seeded to `-00BD2F10(0.0f, 1.0f)` (`009E84F8`, `FCHS` at `009E84FD`), a negative
value, so the first step always re-selects. Three of the five steps are among the callers of
`009DE050`: `009F3240`, `009E23B0` and `009E26C0`.

`009E23B0` was read in pseudocode only. Two facts survive that reading with the registers the
decompiler lost: it writes `brain+3F8h = [unit+54h]` and `brain+3FCh = 1` or `0`, the same
avoidance-request block the `stop` step writes, and its two arms are `009DE050(blk, &goal, 0, 0)`
followed by `009DA610(&goal)` on one side and `009DFF40(brain, heading)` on the other, with the
heading built from `atan2` and `00CE3830`.

## 009E2020, the `kamikaze_attack` step

`00D216B8+0Ch` holds `009E2020`, which confirms the vtable row `docs/SHIP_AI_STATES.md` left
blank. **Not projected**; the call inventory only: `009DFF40` at `009E228A`, then
`brain+3FCh = 1` at `009E22D8`, `009DE050(blk, &goal, 0, 0)` at `009E22EB`, `009DA610` at
`009E22FB` and `009DA4E0` at `009E2365`. It steers with `009DFF40`, the heading setter that also
drops the path plan, rather than with `009E0040`.

## 009E1610, the `follow` step

**Partial**: read in pseudocode with the goal source and the two latches identified, not
projected. Body `009E1610-009E18C2`. It runs only when `[unit+284h]` has a leader at `+14h` that
answers `vtable[5Ch](6)`. It keeps a formation point at `state+14h`/`state+18h` and a direction at
`state+1Ch`/`state+20h`, latches "making way" in `state+2Ch` against `0092D730` and `00D7A260`,
and latches "out of station" in `state+8h` with a hysteresis pair built from `0082E850()` scaled
by `[00CE3DE0]` (in at one radius squared, out at twice it). It then calls
`009DE050(blk, &state+14h, 0, 1)` and hands a seven-field request to `009DA3B0`.

## 009E1950, the `land` step

**Not projected.** Body `009E1950-009E201A`, 1739 bytes of geometry: `006AC220` twice, `00BD2F10`,
`006F2E60`, `006F2FB0`, `006AC5D0`, two `atan2` calls, `006BC0C0`, two `00438B10`, `009DA4E0`,
`00605070`, `00419010`, `0082ADC0`, `00417B10`, and `009DE050` at `009E1EB6` with `keep_mode = 0`
and `final_leg = 0` (`009E1EAC`, `009E1E9A`).

## Coverage

| Routine | Coverage |
| --- | --- |
| `009DE050` | complete |
| `009DA4E0` | complete |
| `009DFF40` | complete |
| `009E00A0` | complete |
| `0071BFF0` | complete |
| `007ADC60` | complete |
| `0071C4F0` | complete |
| `007B6EE0`, `0070D060` | complete |
| `009E14C0` | complete |
| `009E5770` | complete |
| `009E8820` | complete |
| `009E86F0` | complete |
| `009E8450` | complete for the sub-state table; `009E5CA0` and `009E5530` read only for the vtable they install |
| `009E59C0` | partial: `009E59C0-009E5B16` projected. `009E5B17-009E5C8D`, the target range test and the `finished` block, are read as a call list only |
| `009E1610` | partial: pseudocode only, no projection. Nothing between `009E1610` and `009E18C2` is reconstructed |
| `009E23B0` | partial: pseudocode only. `009E23B0-009E26BD` is not projected |
| `009E2020` | not projected; call inventory only, `009E2020-009E23AD` |
| `009E1950` | not projected; call inventory only, `009E1950-009E201A` |
| `009E26C0`, `009E4B90`, `009F3240`, `009F3670`, `007B3DD0` | not read. Named here only as vtable slots and as `009DE050` call sites |
| `0071E430` | partial: `0071E430-0071E48E` read, the `director+30h` dispatch. The `+54h` arm from `0071E489` is unread |

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `include/bsp/ship_ai_states.hpp`: "009ED6B0 tests the field against 2 and 3 for its navigation arm; no site that writes 2 or 3 was read" | `009DE050` writes `blk+1C4h = 2` at `009DE091` and again unconditionally at `009DE152`. Every state step except `cruise`'s therefore forces `Navigate` every time it runs | `009DE091 MOV dword [ESI+1C4h],2`, `009DE152` the same, reached on both arms |
| `include/bsp/ship_ai_states.hpp` `ShipAiSetterHost::on_steering_mode_change_009da4e0`: "Body unread" | `009DA4E0` is the path-plan reset: it releases `blk+244h` and `blk+2ACh` through `vtable[0](1)` and clears fourteen fields and three bytes | body `009DA4E0-009DA58D`, read complete |
| `docs/SHIP_AI_STATES.md`: "`attackmove` ... four embedded sub-states at `00D2174C`, `00D2177C`, `00D217AC` and `00D2171C`" | Five. The fifth is at `state+8h` with vtable `00D21994` and step `009F3240`, built by `009E5CA0`, and it is the one the selector treats as the default | `009E8479 LEA ECX,[ESI+8]; 009E8486 CALL 009E5CA0`, `009E5CD0 MOV dword [ESI],0D21994h`, `00D219A0` = `009F3240` |
| `docs/SHIP_AI_STATES.md`: the `attackmove` row's step and command are "not read" | step `009E8820` (`00D219DC`), command getter `009E8540` returning `00E08F78` | `00D219D0+0Ch`, `009E8540 MOV EAX,0E08F78h; RET` |
| `docs/SHIP_AI_STATES.md`: the `kamikaze_attack` row's step is "not read" | `009E2020`, and `009E5770`'s sibling `moveonpath` step is `009E59C0` | `00D216C4` = `009E2020`, `00D21694` = `009E59C0` |
| `docs/SHIP_AI_STATES.md`: "A state object is sixteen bytes: ... `+8h` and `+0Ch` two floats seeded from `00CF5BFC` (-99.0f)" | `+8h` is used as a **byte** latch by `009E14C0` (`CMP byte [EDI+8],BL`) and by `009E59C0` and `009E1610`. The float seed is still what the constructor writes; the low byte of -99.0f is 0, so the latch starts false | `009E155D`, `009E15B4`, `009E59DE`, `009E5A39` |
| `docs/SHIP_AI_STATES.md`: `009E5770`'s coverage "partial: ... the `00E08F68` arm at `009E5837-009E5891` and the arrival message `009E58E6-009E59B7` are not projected" | complete; both arms are projected here | the tables above |

## no_ghidra_function

| Start | Inclusive end | Evidence for the boundary |
| --- | --- | --- |
| `009E59C0` | `009E5C90` | `MOV EAX,FS:[0]` / `PUSH -1` SEH prologue at `009E59C0`, the target of `00D21694` = vtable `00D21688 + 0Ch`; the previous function ends with the `RET 4` at `009E59B5` (`009E5770`'s body end `009E59B7`) and eight `INT3` at `009E59B8..009E59BF`; the last `RET 4` is at `009E5C8E`, followed by fifteen `INT3` at `009E5C91..009E5C9F` and the next Ghidra function `009E5CA0` (body `009E5CA0-009E5CEE`). An earlier `RET 4` at `009E5A36` is an internal exit, not the end. |
| `009E8820` | `009E88F9` | `PUSH ECX; PUSH EBX; PUSH EBP; MOV EBP,ECX` prologue at `009E8820`, the target of `00D219DC` = vtable `00D219D0 + 0Ch`; the previous Ghidra function `009E86F0` has body end `009E881A` with its `RET 4` at `009E8818`, followed by five `INT3` at `009E881B..009E881F`; the last `RET 4` is at `009E88F7`, followed by six `INT3` at `009E88FA..009E88FF` and the next Ghidra function `009E8900` (body `009E8900-009E8BE3`). A second `RET 4` at `009E88CB` is the hand-back exit. |

`009DAE20` (`MOV EAX,0E08F68h; RET`, `009DAE20-009DAE25`) and `009E8540`
(`MOV EAX,0E08F78h; RET`, `009E8540-009E8545`) also have no Ghidra function; both are five-byte
constant getters followed by `INT3` padding and are read from the disk bytes.

## Follow-up packets

| packet | addresses / files | why |
| --- | --- | --- |
| `ship_ai_attackmove_substates` | `009F3240`, `009E23B0`, `009E26C0`, `009F3670`, `009E5CA0`, `009E5530`, `00852860`, `009E85B0`, `00D21994` | The five sub-state bodies the selector switches between, and the three predicates that decide which. Three of them call `009DE050`, so each produces a navigation goal of its own and the contract is already fixed by this packet. |
| `ship_ai_follow_and_land` | `009E1610`, `009E1950`, `009DA3B0`, `009DA610`, `0082E850`, `00CE3DE0` | The two states read here in pseudocode only. `follow` maintains a formation point and hands a seven-field request to `009DA3B0`; `land` is 1739 bytes of geometry with its own `009DE050` call. |
| `ship_ai_avoidance_request` | `blk+3ECh`, `blk+3F0h`, `blk+3F4h`, `009DA6E0`, `009F0EA0`, `0080E160` | The three-field block `stop` and `009E23B0` write and the two chain steps read. `009F1052`'s `JL` against another entity's `+54h` says the middle field is a side filter; the other two have no reader yet. |
| `entity_command_end` | `0071E430` `0071E489..`, `0071D810`, `0071D9E0`, `00984300`, `0041E870` | What "end the command" actually does. Two of the three arms dispatch on `director+30h`; the third walks `director+54h`. The `finished` message `00984300` posts is the same one `movetopos` and `moveonpath` build. |
| `ship_ai_arrival_test` | `009E5770+vtable2C`, `009E59C0+vtable2C`, `00D21634`, `00D21694` | The per-state arrival predicate at vtable slot `+2Ch`. Two call sites are read; no callee body is, and it is the test that decides when a `moveto` ends. |

## Uncertainties

1. `state->vtable[2Ch]` is called with the goal in both `movetopos` and `moveonpath` and no body
   was read. "Arrival test" is inferred from the call site and from what a true return leads to.
2. `target+7A0h` is called a range because `009E58CE` subtracts the unit's `+9C8h` from it and
   compares against a planar distance. It is an integer field and no writer was read.
3. The `keep_mode != 0` arm of `009DE050` is projected from the listing; every call site checked
   passes zero, so nothing exercises it.
5. `009E8820`'s `vtable[5Ch](9)` is read as a capability the whole group must share, from the
   loop's shape. What `9` and `1Ch` and `8` select is not established; `1Ch` is the value
   `movetopos` uses on a command target that turns out to be a unit.
6. The x87 chains in `009DE050` and `009E86F0` are reconstructed with a single rounding at each
   named store, as `docs/X87_CONTROL_WORD.md` requires. The C++ projection uses float variables,
   which rounds each operation; the two agree on every value read here but are not identical.

## Correction from docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md

Packet `cc_ai_attackmove_substates` (main 7db7b9d3) read the five sub-state steps whole and
corrects two claims above:

| what this doc said | what is true | evidence |
| --- | --- | --- |
| three of the five sub-state steps call `009DE050` (`009F3240`, `009E23B0`, `009E26C0`) | four do: `009F3670` calls it at `009F3920`. `007B3DD0`, the initial member, is a COMDAT-folded empty virtual (`RET 4`) shared by twenty vtables, never usefully stepped | `009F3920`, `007B3DD0` |
| `009F3240`'s `009DE050` call passes `final_leg = 1` | it passes `keep_mode = 1` and `final_leg = 0` at `009F332B`: `009DE050` reads its first stack argument as the goal (`009DE063`, `009DE0A0`) and its second as the keep byte (`009DE087`), and the `PUSH 1` at `009F3308` is the middle argument | `009F3308`, `009F332B`, `009DE087` |

`009E23B0`'s two arms are selected by the latch byte at `sub+8h`, and `brain+3FCh` follows the
arm: 1 on the goal arm (`009E2669`), 0 on the heading arm (`009E25CC`).
