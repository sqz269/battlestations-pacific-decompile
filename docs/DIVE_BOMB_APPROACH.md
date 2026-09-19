# The dive-bomb approach states: what `moveto` and `follow` actually are

Packet `cc8_dive_approach`, worktree `agent/cc8-dive-approach`. Continues
`docs/BOMBER_AFTER_TASK.md` section 10, whose closing sentence is that the attack-mode gate is
faithful but "the state behind the gate" is not: `moveto`/`follow` are the states the image uses
to close the range on an attack profile, and this host ran no tick for either.

Every name here is a hypothesis, not a recovered symbol. Addresses are from
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

## 1. The arm has no per-state chain: it is one virtual tick

`009C8790 BSP_BotTaskDiveBomb_TickArm`, `__thiscall(task)(float dt)`, `RET 4`, body
`009C8790`-`009C88D3`, **read whole, 90 instructions**. In order:

| step | address | what it does |
| --- | --- | --- |
| 1 | `009C8794`-`009C87D2` | a bool: the current state `[ESI+310h]` is one of `+734h` aimdive, `+754h` aimglide, `+778h` flyabove, `+79Ch` turndown |
| 2 | `009C87EA` | `009C7A80(approach, dt, thatBool)`, the approach update |
| 3 | `009C87F9`-`009C8825` | `009BDE80(task+4F0h, task+4A4h - 100.0, task+4A4h, task+4ACh)` — **SetRanges on the MOVETO state, every tick** |
| 4 | `009C8834` | `009C83E0(task, dt)`, the transition |
| 5 | `009C883D`-`009C884C` | `MOV ECX,[ESI+310h] / MOV EAX,[ECX] / MOV EDX,[EAX+0Ch] / PUSH ECX / FSTP [ESP] / CALL EDX` |
| 6 | `009C884E`-`009C88C9` | the manual-release passthrough, excluded in `{5C4h, 734h, 754h, 778h, 79Ch, 7BCh}` |

Step 5 is the whole of the state dispatch: **one indirect call through the current state's
vtable slot `+0Ch`**, with `ECX` the state and `dt` the single stack argument (the `PUSH ECX` only
reserves the slot, `FSTP [ESP]` overwrites it). There is no per-state chain in the image. This
host's `run_dive_bomb_task_arm_009c8790` flattens that dispatch into a chain of `if (ctx.current
== ...)` blocks, and the two it never wrote were `kMoveTo` and `kFollow` — so a dive bomber in the
approach flew whatever command the previous state had left in the plan block.

## 2. Which class is at `task+4F0h` and `task+52Ch`, proved from the constructors

`009C7710 BSP_BotTaskDiveBomb_Construct` does **not** build the states. It calls
`009C73A0(approach = task+3F8h, unit, target)` at `009C7751`, and that body builds them inside the
approach: `009C73F8 LEA ECX,[ESI+0F8h]` then `009C7436 CALL 009C2AC0`, and `009C7441 LEA
ECX,[ESI+134h]` then `009C7451 CALL 009C2980`. With the approach at `task+3F8h`, `+F8h` is
`task+4F0h` and `+134h` is `task+52Ch` — the two states `009C83E0`'s approach edges select
(`docs/BOMBER_AFTER_TASK.md` 10.1).

| state | constructor | vtable write | tick, vtable `+0Ch` |
| --- | --- | --- | --- |
| `moveto` `task+4F0h` | `009C2AC0` | `009C2B32 MOV [ESI],0D20AECh` | `00D20AF8` = **`009C18C0`** |
| `follow` `task+52Ch` | `009C2980` | `009C29B5 MOV [ESI],0D20AB8h` | `00D20AC4` = **`009C1FD0`** |

Nothing later overwrites either: `009C7710` rewrites only `[task]` = `00D20E18`, `[task+3F8h]` =
`00D20E10` and `[task+4DCh]` = `00D20E0C`.

The two tables read out of `.rdata`, with the base-class defaults `007B3DB0`/`007B3DC0`/
`007B3DE0`/`007B3DF0`/`007B45E0` naming the slots:

```
00D20AB8  009c2a60 009bed80 009bde40 009c1fd0 007b3de0 009be590 009a4860     ; follow, 7 slots
00D20AD4  009c2b70 009bdeb0 0042b120 0042b130 0042b140 006935c0              ; the moveto observer
00D20AEC  009c3d10 007b3db0 007b3dc0 009c18c0 007b3de0 007b3df0 007b45e0 009c1850  ; moveto, 8
```

**RETRACTION, of the ledger and of another document.** The ledger entry on `009C18C0` says
"Vtable 00D20AEC+0Ch, **shared by moveto and follow**", and section 2 of
`docs/TORPEDO_MOVETO_TICK.md` says "Both states share this body (`00D20AEC+0Ch` and
`00D20B24+0Ch`)". The two addresses it cites do both hold `009C18C0`, but neither is the follow's:
`00D20B24` is a third, moveto-derived table (`+0Ch` = `009C18C0`, `+1Ch` = `009C1BC0` instead of
`009C1850`). The follow state's table is `00D20AB8` and its tick is `009C1FD0`, the formation
body. The ledger entry is corrected by this packet.

The moveto table has eight slots and the follow table seven: `+1Ch`, the speed slot `009C1850`
that `009C18C0` calls at `009C1999`, exists only on the moveto family. That is itself a proof
that the follow state cannot be running `009C18C0`.

## 3. What the dive-bomb task feeds the move-to tick

`009C2AC0(this, owner, target, near, far, mode)` stores the last three at `+30h`, `+34h`, `+38h`
(`009C2B17`, `009C2B22`, `009C2B41`), and `009BDE80` is the three-store setter that refreshes
them. Both dive-bomb call sites were read:

| site | `+30h` | `+34h` | `+38h` |
| --- | --- | --- | --- |
| constructor `009C7436` | `approach+A8h - 100.0` | `approach+A8h` | `approach+B4h` |
| the arm, every tick, `009C8825` | `approach+ACh - 100.0` | `approach+ACh` | `approach+B4h` |

(`00D7A220` = `100.0`, read as a double at the width of the `FSUB qword` that loads it.) The
per-tick pair wins from the first tick on. `approach+ACh` is `ctl->+398h`, which `009C7A80`
copies in at `009C7A96` and the cruise profile holds at `Pilot/DiveBomb/BeginAltRange/1`; this
agrees with `docs/DIVE_BOMB_TASK.md` lines 391-396, derived there independently.

The state's `+2Ch` target is the **task's own target**: `009C8C70 BSP_BotTask_MakeDiveBomb` passes
`EDX` to `009C7710` as arg2, `009C773B` forwards it to `009C73A0` as arg2, `009C73BF` parks it in
`EBP` and `009C7434 PUSH EBP` makes it `009C2AC0`'s `target`. So step 1's separation is the
aircraft-to-target planar range, the quantity this host already keeps as `db_planar_bc`.

## 4. `009C18C0` to its end, and what it commands

`__thiscall(this, float dt)`, `RET 4`, body `009C18C0`-`009C1BB3`. `this+4h` is the approach and
`(this+4h)+4h` the unit.

1. `009C18CB`-`009C1984`. With a target at `+2Ch`, refresh both poses (`00414DB0`), take
   `dx = target+FCh - unit+FCh` and `dz = target+104h - unit+104h`, and set the separation to
   `sqrt(dx²+dz²)` when the square exceeds the double `[00CE3820]` = `1e-10`, else `0`. The
   target's `+100h` is parked at `[ESP+20h]` for step 5.
2. `009C198A`-`009C1999`. `this->vtable[+1Ch](separation)` — **unconditional**, before either
   early return. For `00D20AEC` that is `009C1850`, which writes `cmd+2B4h` from
   `009BECD0(ctl+3A0h, 007C47F0(), speed)`, clears `cmd+2B0h` and raises `cmd+2D8h`.
3. `009C19A1`. `CMP byte [unit+0C25h],0` — non-zero returns after four command writes.
4. `009C19E7`. `+2Ch == 0` takes the `009C1B69` no-target arm and returns.
5. `009C19F1`-`009C1B5B`, the glide slope:

```
base   = max(this+34h + targetY, this+30h)            009C19F1-009C1A1F
margin = 1400.0 [00D1F8D0] - unit+100h, floored at 50.0 [00CEB4D4, gate 00CE3938]
denom  = separation - 1000.0 [00CE47A0], clamped into [50.0, 2000.0 [00CFFD60, gate 00CF0DD8]]
t      = InterpolateClamped(0.05, 0.35, 0.4, 1.6, margin/denom)          009C1AF3
009C1B17  009FBA50(base, this+38h, separation, t)
009C1B1C  009F9E40(&targetWorldPos)        ; the heading, at the target point
009C1B2D  approach+1Ch -> +40h = Pilot/AutoStrafeAngle/Angle_MoveTo
009C1B45  0099B630 -> 009A1A20 -> 009FABE0 ; three tail calls, contract: unread
```

The denominator's floor is implicit and was nearly missed: `009C1A89`'s `JA` leaves `XMM0`
holding the `50.0f` that `009C1A3B` loaded for the margin, so one constant floors both. Every
constant above was read at the width of the instruction that loads it — the five gates are
`FLD double ptr`, the five values `MOVSS`/`FLD float ptr`.

**So for a dive bomber the move-to tick is a glide from `BeginAltRange/1` above the target down
to that altitude at `approach+B4h`, at the LevelFlight speed, steering at the target.** It is the
same command shape the attack-run tick issues at `009C43ED` — `009FBA50(approach+ACh +
approach+50h, approach+B4h, ., .)` — which is why the image can gate entry into the attack on the
in-range latch and still have its bombers arrive on profile.

## 5. What was bound

New file pair `include/bsp/move_to_glide.hpp` + `src/move_to_glide.cpp`: `009C18C0` step 1's
`move_to_planar_distance_009c1950` and step 5's `move_to_glide_009c18c0`, pure over explicit
inputs, with the eleven constants named by their data addresses. The torpedo arm open-codes the
same arithmetic inline in `src/game_hosts_units.cpp`; that hunk belongs to another packet and was
left alone rather than refactored.

`run_dive_bomb_move_to_tick_009c18c0()` in the dive-bomb arm, dispatched for `kMoveTo`, runs
steps 2, 5 and the heading. Labelled substitutions, all inherited or stated:

* `009BECD0`'s shaping of the speed against the distance is unread, so the desired speed is
  `007C47F0`'s LevelFlight × StallSpd product unshaped (the same substitution
  `docs/TORPEDO_MOVETO_TICK.md` carries).
* `009F9E40`'s body is unread, so the heading written is this host's own `db_bearing_c0`, the
  `pi/2 - atan2` convention `009C7B8A` was read for, through the mode-2 pair the attackrun and
  flyabove ticks already use.
* `unit+0C25h` has no host field; these aircraft are never under it.
* the strafe angle and the three tail calls are not bound.

`run_dive_bomb_follow_tick_009c1fd0()` for `kFollow`. **NOT bound and said so:** `009C1FEA CALL
009BFD70` (the station point, bound) then `009BFEE0` and `009BEE30`, about 2900 instructions of
station-keeping law that are unread (`docs/PLANE_FORMATION.md` section 6). This host places the
member on its station instead — the geometry is the image's, the path to it is not — which is
what the done and prepare states already do, and those reach this *same* body through `009C7278`.
`kFollow` is unreachable in this host anyway: `009C777E` and `009C8419` both choose moveto when
`007B8AD0` says the unit lacks a follow target, and `unit_has_no_follow_target()` is hardcoded
true.

## 6. PREDICTION, written before the runs

Both runs are USN04, `--frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames
4800 --mission-frame-seconds 0.05`, on a build in which `approach+B4h` and `approach+B8h` are
still main's **1100.0** (`cc8-dive-heading` holds a change that will move them to draws around
780 / 2080; these windows did not run with it).

* **A**, the new baseline: the moveto binding compiled in, `dive_bomb_transition_inputs` still
  reading the pinned constant `2`. Since `engaged` is pinned true, no bomber should enter
  `moveto` at all and `db_moveto_tick_ticks` should be **0** for every squadron; the run should
  reproduce the accepted main numbers, 1 dive-bomber water contact and 2 `movieval` releases.
  If `moveto` ticks are non-zero here, the pinned constant is not doing what 10.7 says.
* **B**, wired to `slot.db_attack_mode_370`. `movieval` (the flight lead) spent 1493 ticks in
  `moveto` commanding nothing; with the glide bound it should arrive at `approach+B4h` = 1100 m
  at about `BeginAltRange/1` = 1000 m rather than at whatever altitude it drifted to, and should
  then take the attack chain instead of `goaway`. I predict for `movieval`: releases **>= 2**, no
  water contact, `db_moveto_tick_ticks` of order 1000-1500. For the six `D3A Val` bombers, whose
  failure in 10.9 was `aimdive -> goaway` at 240 m instead of `aimdive -> aimglide` at 600 m, I
  predict the entry altitude rises toward 1000 m and the water contacts fall from 7 toward 0-1.
  The risk I can name: the glide commands a DESCENT to 1000 m from up to 11 km out, and if the
  aircraft start below 1000 m it will command a climb they do not have the range to complete.

## 7. The break-off range is 3-D, and its far point is the aim point

`docs/BOMBER_AFTER_TASK.md` 10.10 read both endpoints of `009C8A90`'s range arm and left the fix
undone for want of the aim point's x and z. Nothing new is needed: this host's aim point already
**is** the commanded target's position — the labelled substitution `db_planar_bc` and
`db_bearing_c0` already run on — so the x and z differences are the two it computes anyway and
only the vertical term was missing. `db_aim_point_3d` adds `dy = target.y - unit.y` to the same
squared sum, and both break-off feeds (`dive_bomb_transition_inputs` and the arm binding's
`should_break_off`) now read it instead of `db_planar_bc`. Both of the old errors pushed the same
way — planar, and to a different point — so the break-off used to fire later and lower than the
image's.

**A second CORRECTION to 10.10, and it makes the fix smaller than 10.10 thought.** 10.10's
error 2 says `009C7B43`-`009C7BAA` builds `approach+BCh` "from the target entity's `+100h`/`+104h`,
not from the approach's `+4Ch`/`+54h` aim point". Read whole:

```
009c7b03  cmp dword [esi+48h],0 / je 009c7e91   ; the latched target gates the block
009c7b14  mov edi,[esi+4]                        ; EDI is the UNIT, not the target
009c7b17  refresh the unit's pose
009c7b27  eax=[esi] / edx=[eax] / lea ecx,[esp+18h] / push ecx / ecx=esi / call edx
                                                 ; approach vtable SLOT 0 = 009C40A0, the aim point
009c7b34  [esp+24h] = aim[0] - [edi+FCh]         ; dx
009c7b40  [esp+28h] = aim[1] - [edi+100h]        ; dy, COMPUTED AND THEN UNUSED
009c7b4d  [esp+2Ch] = aim[2] - [edi+104h]        ; dz
009c7b5a  dx*dx + dz*dz, sqrt above 00CE3820     ; -> approach+BCh
```

`EDI` is `approach+4h`, the unit, exactly as it is everywhere else in `009C7A80`; the target
entity is not in the expression at all, and the far point is the **same aim point**
`009C8A90` uses, through the **same** `009C40A0` call. So the two ranges share both endpoints
and differ only in whether the vertical term is summed — and the image computes `dy` here at
`009C7B40` and then deliberately drops it. 10.10's error 1 (planar versus 3-D) and its reading of
the sign stand; its error 2 is retracted.

One refinement to 10.10's provenance, which does not change its conclusion. The dive-bomb
approach's vtable is written **three times**: `009C3EE2` (the base, `00D20C48`), then `009C740B`
in the approach constructor (`00D20E08`), then `009C7767` in the *task* constructor
(`00D20E10`) — so the table live when `009C8AFD` dereferences `[task+3F8h]` is `00D20E10`, not
the `00D20C48` 10.10 names. All three carry `009C40A0` at slot 0, so the aim point stands. The
ledger entry on `009C40A0` names only the first write and should name the last.

## 8. The spent-member latch arm, read whole — and a correction to 10.4

`009C7C31`-`009C7CFE`, read from the listing:

```
009c7c31  cmp byte [esi+D1h],0        ; sets the flags 009c7c3e consumes
009c7c38  mov byte [esi+D0h],al       ; the latch store; MOV writes no flags
009c7c3e  jne 009c7d04                ; still has bombs -> done
009c7c44  eax=[esi+0Ch] / cmp byte [eax+369h],0 / je 009c7c5d
009c7c50  cmp byte [00E17BF2],0 / jne 009c7d04      ; the hold arm's own guard
009c7c5d  edi=[eax+3D0h]              ; the FLIGHT LEADER
009c7c63  cmp edi,[esi+4] / je 009c7d04             ; the leader itself is exempt
009c7c6c  refresh the leader's pose
009c7c7c  fld [edi+FCh] -> [esp+10h] ; fld [edi+104h] -> [esp+14h]   ; the LEADER's x,z
009c7c82  edx=[esi] / edx=[edx] ; 009c7c9b call edx  ; approach vtable SLOT 0 = 009C40A0
009c7c9d  fld [eax] -> [esp+8] ; fld [eax+8] -> [esp+0Ch]            ; the AIM POINT's x,z
009c7cae  [esp+18h] = aimX - leaderX ; 009c7cba [esp+1Ch] = aimZ - leaderZ
009c7cc6  call 00414C60               ; BSP_Vector2f_LengthWithCutoff
009c7ccb  fld [esi+B8h] / fcompi / jbe 009c7cfc     ; al = 0 when R <= that range
009c7cfe  and byte [esi+D0h],al       ; an AND: it can only CLEAR
```

**CORRECTION to `docs/BOMBER_AFTER_TASK.md` 10.4.** That section calls the measured quantity the
"2-D range to the leader" and summarises the rule as "a spent wing member further than `R` from
its flight leader has its in-range latch forcibly cleared". The unit's own position is **not in
the expression**. `009C7C9B` calls slot 0 of the approach's vtable, which is `009C40A0`, the aim
point getter — the same call 10.10 identifies in `009C8A90` — and the subtractions at `009C7CAE`
and `009C7CBA` are `aimPoint - leader`. The rule is: **a spent wing member's latch is cleared
when the flight leader is further than `approach+B8h` from that member's own aim point**, i.e.
when the leader has left the area of the target. Everything else in 10.4 — the exemption for the
leader, the `AND`, the `+D1h` fall-through past a flags-preserving `MOV`, the guard shared with
the hold arm — is confirmed as written.

Bound as a pure insertion into `bsp::dive_bomb_in_range_latch_009c7c31`, in the image's order,
with the leader resolved through the squadron registry's `member_units[0]` (the same two-site
convention `0099B757` and `009C7C7C` agree on) and the aim point standing in as the commanded
target's position. With no squadron, `leader_known` stays false and no arm runs — which is what
the image does when `[eax+3D0h]` has no member.

## 9. Run A: the moveto binding is inert while the constant is pinned

`local/approach_a_unwired.log`, USN04, the arguments of section 6, one clean shutdown
(`native renderer final COM release`, one line). The moveto and follow ticks are compiled in;
`dive_bomb_transition_inputs` still reads the pinned constant `2`.

**Prediction A confirmed exactly.** Zero `db moveto` rows in the whole log, no `moveto` bucket in
any dive-bomber state census, one mission-wide `plane water contact` (`movieval`, `|v|=68.72`),
`movieval` `releases=2 rounds_left=0`, `done=303`, `approach_returns=0`. The log is 6 835 436
bytes against the accepted baseline `attackmode_before.log`'s 6 835 437 — the same run.

So the dispatch is correctly gated: with `engaged` pinned true, no dive bomber ever enters the
approach, and adding the two ticks changes nothing. That is the control this packet needed before
any of the three behaviour changes could be read.

(The `attack mode` row already reports `lead=1 mode_370=1`: the feed `cc8_attack_mode` built is
live and correct, and only the single read site is pinned. `approach+BCh=278.5 m` against
`approach+B8h=1100.0 m` at the end, `ticks without latch=1527`.)

## 10. Run A2 prediction, and an honest note about when it was written

Section 6's plan had the wiring measured before sections 7 and 8 were written. They were written
during run A's wait instead, so the order of windows changed: **A2** is sections 7 and 8 compiled
in with the read site still pinned, which isolates them against A, and **B** is A2 plus the one
read-site line, which keeps "the same binary apart from that line" intact for the wiring verdict.
Same number of windows.

**This prediction was written after run A2 was launched and before its log was read**, which is
weaker than section 6's and is stated as such.

From run A's own numbers: the break-off threshold is `thr=100.0` (`SafeDist` × `speed_ratio_41c`
= 100 × 1.0) and `movieval` is already at `d = 278.5 m` planar when it goes spent, so
`100.0 <= d` is **already true** with the planar feed. The 3-D range can only make it true
earlier, never later. So section 7 should move *when* a spent bomber reaches `done`, not whether
— exactly what 10.10 predicted — and `movieval`'s `done` count should go **up** from 303, with
its water contact still present. I do not expect the mission's release or contact totals to
change.

Section 8's arm needs a spent, non-leader bomber whose leader is more than `approach+B8h` =
1100 m from its aim point. In run A every aircraft converges on the same target and the leader
ends 278 m from it, so I expect the arm to be **inert** in this window: `movieval` is the lead
and exempt, `movieval|.-3` is spent but its leader is close. A null result here is a null
result about USN04, not about the rule.

## ABI

| address | signature | notes |
| --- | --- | --- |
| `009C8790` | `void __thiscall(ECX = task)(float dt)`, `RET 4` | read whole, 90 instructions |
| `009C18C0` | `void __thiscall(ECX = state)(float dt)`, `RET 4` | read whole for the dive bomb's inputs |
| `009C1FD0` | `void __thiscall(ECX = state)(float dt)`, `RET 4` | not read past its two formation calls |
| `009C2AC0` | `State* __thiscall(ECX = this)(Approach*, void* target, float near, float far, float speedRange)` | the last argument is a DISTANCE, not a mode; `009BDE80`'s ledger entry already said so |
| `009C2980` | `State* __thiscall(ECX = this)(Approach*, float)` | body read only to its vtable stores |
| `009BDE80` | `void __thiscall(ECX = state)(float, float, float)` | three stores, nothing else |
| `009C1850` | `void __thiscall(ECX = state)(float speed)`, `RET 4` | vtable `00D20AEC+1Ch` |
| `009C7710` | `Task* __thiscall(ECX = this)(Unit*, void* target)`, `RET 8` | kind 8, size `7E0h` |
| `009C73A0` | `Approach* __thiscall(ECX = this)(Unit*, void* target)` | builds the states inside the approach |
| `009C40A0` | `float* __thiscall(ECX = approach)(float out[3])`, `RET 4` | copies `+4Ch`/`+50h`/`+54h`; no Ghidra function |

## Uncertainty

* The three tail calls of `009C18C0` (`0099B630`, `009A1A20`, `009FABE0`) and the strafe-angle
  write are read as call sites only; nothing behind them is bound.
* `009BECD0`'s shaping of the desired speed against the distance is unread, so the speed this
  host commands in `moveto` is the unshaped `007C47F0` product. Inherited, not introduced.
* `009F9E40`'s body is unread. The heading written is this host's own `db_bearing_c0`. It is the
  right *quantity* — a bearing at the target point — by the call's argument, but not the right
  *code*.
* `unit+0C25h` has no host field, so `009C18C0`'s step-3 early return is not modelled.
* `009C1FD0` past `009BFD70` is unread, about 2900 instructions. `kFollow` is unreachable in this
  host, so nothing measured here bears on it.
* The aim point `approach+4Ch`/`+50h`/`+54h` still has no producer read, in this packet or in
  `docs/DIVE_BOMB_TASK.md`. Everything here that needs the aim point uses the host's standing
  substitution, the commanded target's position, and says so at the use site.
* `approach+B4h` and `approach+B8h` are main's `1100.0` in every window here. `cc8-dive-heading`
  holds a change that makes them draws around 780 and 2080; these numbers do not carry across it.

## Coverage

| item | status |
| --- | --- |
| `009C8790` the arm's dispatch shape | read whole, host corrected |
| `009C18C0` steps 1, 2, 5 | reconstructed, build-tested, measured |
| `009C18C0` steps 3, 4 | read, deliberately not bound |
| `009C1FD0` | identified; its law not read, host substitution stated |
| `009BDE80` dive-bomb call site | read whole, bound |
| `009C7710` / `009C73A0` construction | read for the state classes and the vtables |
| `009C8A90`'s range endpoints | both read; the feed corrected |
| `009C7C31`-`009C7CFE` spent-member arm | read whole, bound |

## 11. Why the follow tick and the done tick are the same six lines of host code

`009C7270 BSP_BotStateDiveBombDone_Tick` is a **five-instruction thunk**:

```
009c7270  fld dword [esp+4] / push ecx / fstp dword [esp] / call 009c1fd0 / ret 4
```

It forwards `dt` and its own `this` and does nothing else, so the dive-bomb done and prepare
states literally run the follow tick. That is the evidence behind the two near-identical host
bodies `run_dive_bomb_done_prepare_tick_009c7270` and `run_dive_bomb_follow_tick_009c1fd0`: they
are two call sites of one image body, not two reconstructions of one rule, and they differ only
in which counters and `record` labels they keep. The `plan_mode_26c = 2` both write is
`009C1FE2 MOV byte [ECX+26Ch],BL` with `BL = 2` and `ECX = (state+4h)->+18h`, the command block —
a byte, inside `009C1FD0`, which is why it belongs to both.

## 12. Run A2: both corrections are arithmetically confirmed and behaviourally null on USN04

`local/approach_a2_breakoff_latch.log`, same arguments, one clean shutdown. Sections 7 and 8
compiled in, the read site still pinned.

Comparing every `divebomb`, `db aim exit` and `plane water contact` row of A against A2: 156 rows
each, and **all 22 differing entries are the printed `d` on the 11 `db aim exit` rows**. Nothing
else moved — not one state count, altitude, release, water contact or latch decision.

| aircraft | `alt` | `d` in A (planar) | `d` in A2 (3-D) | `sqrt(d_A² + alt²)` |
| --- | --- | --- | --- | --- |
| `movieval\|.-3` | 205.4 | 204.2 | **289.6** | 289.6 |
| `movieval` | 180.6 | 191.9 | **263.5** | 263.5 |
| `movieval\|.-2` | 164.2 | 189.1 | **250.5** | 250.5 |

The vertical term is exactly the aircraft's altitude above a sea-level aim point, to the digit, on
every row. So section 7's change does what the listing says it does.

**And it changes no decision here, because the threshold is 100.0 m** (`SafeDist` ×
`speed_ratio_41c`) and *both* ranges are already two to three times that whenever a bomber is
spent. `movieval` still breaks off at tick 1809 and still reaches `done` with 303 ticks. So the
prediction of section 10 was right in direction and too strong in degree: I said `movieval`'s
`done` count should go **up**, and it did not move at all. 10.10's "it moves WHEN a spent bomber
reaches `done`" is not observable on USN04 either; the honest statement is that the feed is now
the quantity `009C8B14`-`009C8B3B` measures, and this mission cannot tell the two apart.

Section 8's spent-member arm is **inert in this window**, as predicted: `movieval` is its
squadron's lead and exempt, and `movieval|.-3` is spent but its leader is 192 m from the aim
point, far inside `approach+B8h` = 1100 m. A null about USN04, not about the rule.

(Read the `bomb_4c9` column with care — it is the PRE-arm sample, so the rows above print
`bomb_4c9=1` on the very transition the break-off fired for.)
