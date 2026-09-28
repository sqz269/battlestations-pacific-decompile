# The ship brain's `sub_attack` state (packet `cc9_submarine_ai_states`)

Worker cc9-ships5, 2026-09-28. This continues docs/PLANNER_TASK_CHOICE.md section 16 (cc9-ships4's
handoff) and docs/SUBMARINE_MODEL.md section 16 (cc9-lua4's read). Code:
`include/bsp/ship_ai_sub_attack.hpp`, `src/ship_ai_sub_attack.cpp`, bound in
`src/game_hosts_ship_ai.cpp` behind `kShipAiSubAttackSelectBound` and `kShipAiSubAttackStatesBound`.
Every address below was read in the listing (`ghidra disasm`, or `disasm-raw` where Ghidra has no
function); constants were read from the PE on disk.

Object bases. `ai` is the object 009F3D00 receives. `brain` = `ai+58h` is the object
`BSP_ShipAi_BrainConstruct` 009F39C0 builds; its head is 009F1160 (same `ECX`, 009F39E5). The parent
state is `brain+2124h` (`ai+217Ch`); "approach" is parent+34h and "fire" parent+58h. Each sub-state
object holds the brain at `+4h`.

## 1. The producer of the attack subject, and the selector

**The attack subject is the brain's own unit, when that unit is a submarine.** The census of the
displacement (`scan-bytes '0C 0B 00 00'` and `'B4 0A 00 00'`, limit 4000, ship-AI range) finds one
writer, 009F1160:

```
009F11B0  MOV EAX,[EDI]; MOV EDX,[EAX+5Ch]; PUSH 8; MOV ECX,EDI; CALL EDX   ; unit->vtable[5Ch](8)
009F11C1  NEG AL; SBB EAX,EAX; AND EAX,EDI                                  ; submarine ? unit : 0
009F11C9  MOV [ESI+0AB4h],EAX
```

`brain+0AB4h` is `ai+0B0Ch`, the field 009F3D73 tests. Every other hit of either pattern is a reader
(009E4xxx, the fire tick) or `BSP_ShipAi_BrainPrePass`'s own `brain+0B0Ch`, a different field. The
same head also stores `+0AA8h` = the unit, `+0AACh` = `unit+538h` (the class), `+0AB0h` =
`unit+73Ch` (the navigator block) and `+0AB8h` = `unit+738h`.

**009F3D73..009F3D96:** with `[ai+0B0Ch]` non-null, `00779AA0` (ECX = that unit, read whole:
`[[unit+538h]+510h] > 0 || [+514h] > 0`, KamikazeDamage / KamikazeBlastDamage) picks
`kamikaze_attack` (ai+2254h) when true and `sub_attack` (ai+217Ch) otherwise; null takes attackmove.
So **every submarine brain given attackmove or artillery runs `sub_attack`**, and a submarine class
with a kamikaze warhead (this installation's `vehicleclasses.lua` lines 2106/2108, a 12 m class)
runs `kamikaze_attack`. The host previously answered "no producer" and ran attackmove for all.

Binding: `SyncBinding::select_state_for_command_009f3d00` with `kShipAiSubAttackSelectBound`. The
kamikaze test reads the two class keys through the stored settings (the same numbers 00831840
loads into `+510h`/`+514h`).

## 2. The parent, 009E4F90 and 009EAA90

009E4F90 (`__thiscall(parent, brain)`, `RET 4`, listing 009E4F90-009E50BD) installs vtable
00D218C0 then 00D2195C, the approach object (00D218F0) at +34h and the fire object (00D21920) at
+58h, each with `+4h` = brain, a reference node at `+8h` (vtable 00D21880) and `+1Ch` = 0. It sets
`+C8h` = 1.0 (00D7A24C), `+CCh` = `-00BD2F10(stream 1, 0, 1)` (009E5061), `+D4h` = approach, calls
approach `vtable[4]` (007B3DB0, a bare `RET`) and registers "approach" and "fire" (00CE6798) with
00411E70. **It initialises none of the fire object's fields past `+1Ch`.**

The parent's vtable 00D2195C: `+4h` enter = 009E50C0, a bare `RET` (no Ghidra function; `C3` then
`INT3` from 009E50C1), `+8h` exit = 007B3DC0 (`RET`), `+0Ch` step = 009EAA90, `+18h` = 009EAAD0 (the
debug text `"%s %d/%d, fwdt:%d, close:%d, Yturn:%d"`), `+28h` = 009DAA90 (the shared 5.0 interval).
Because enter does nothing, **the current sub-state survives leaving and re-entering `sub_attack`.**

**009EAA90:** `if ([brain+0B20h]) { 009EAA10(dt); [this+D4h]->vtable[0Ch](dt); }`.

**009EAA10 (listing read whole):** `FCOMI dt, cc; JC` - while `dt < cc`, `cc -= dt`. Otherwise
`cc = cc + (C8h - dt)` and then:
- current is approach (`CMP [ESI+D4h], ESI+34h`): `009E9D90` with **ECX = the approach object**;
  true switches to fire through `BSP_StateMachine_SetCurrentState` 007B6EE0 (old `vtable[8]`, store,
  new `vtable[4]`; nothing when equal);
- current is fire: `009EA9C0` with **ECX = the fire object**; true switches to approach.

## 3. The predicates and the range

**009E4A60** (range to target, `ST0`): `dx`, `dz` to the target (both poses refreshed through
00414DB0); `d = sqrt(dx*dx+dz*dz)` when `d2 > 1e-10` (00CE3820, double), else 0. Then
**`d - selfLength*0.5` is computed and stored to `[ESP+0Ch]` (009E4B26), a slot neither return
reads**: both exits load `[ESP+8]`. So the result is `d`, or `d - targetLength*0.5` for a ship target
(`vtable[5Ch](6)`), floored at 0. The own hull is not subtracted.

**009E9CE0** (torpedo range, `ST0`): the object's `+1Ch` caches the first tube (009E9BC0: the first
fore tube `brain+0AD0h`, else the first aft `brain+0AE0h`) through the reference node. No tube, or
no bot at `tube+39Ch` -> 500.0 (00CE397C). Else `length` = 100.0 (00CE3D08), or the target class
Length for a ship target, and `00901BA0(bot, length, 1)`: `00731020` on `[bot+58h]+3F4h` (the tube's
range) and `008387B0(7, length, range, [[00E1998C] + 14h*(bot+34h+1)])`, the TorpedoBot level row's
FireTargetAccuracy.

**009E9D90** (approach -> fire): target present and `R + 150.0 > range` (00CE3DD8, a double; the
listing's `FXCH` makes it `R+150` against `range`). **009EA9C0** (fire -> approach): no target, or
`range > R + 400.0` (00CE3D90, a double). Hysteresis of 250 m.

## 4. "approach", 009E4B90 (00D218F0 `vtable[0Ch]`)

`SetDepthLevel(2)` on `[brain+0AB4h]`; with a target, `this+20h` = 009E4A60 (no reader found) and
`009DE050(brain+8, &{target+FCh, target+104h}, 0, 1)`; without, `009E00A0` (hold heading, stop).

## 5. "fire", enter 009EABE0 and step 009E9EB0

**009EABE0** (00D21920 `vtable[4]`; no Ghidra function, 009EABE0-009EAC9C, `JMP 009E9E50` at
009EAC98 then `INT3` at 009EAC9D): `+5Ch` = 10.0; `+20h` = fore + aft ready tubes (009E9910 /
009E9A50 at horizon 10); 009E9DE0; `+24h` = 100.0; `+60h` = U(30, 50) (stream 1); `+3Ch` = 10.0;
`+30h` = `+38h` = `+48h` = 1; `+40h` = `+44h` = 0; `+64h` = -1.0; `+6Ch` = 0; `+68h` = 0; then
**009E9E50**: `+58h` = R * U(0.8, 1.0), `+54h` = `+58h` * U(0.3, 0.5), two stream-1 draws around
009E9CE0. `+34h` is never seeded (LABELLED: the host starts it at 0).

**009E9DE0**: `+4Ch` / `+50h` = fore / aft tubes with a barrel timer `<= +5Ch` (00727D30) that are
operational (00729F10); `+28h` / `+2Ch` = the least barrel timer (00729920) over the operational
fore / aft tubes (3600.0 when none); `+20h` = the sum, and `+24h` = 0 when the sum dropped.

**009E97B0** (once, `brain+0AC8h`): each child of the unit answering `vtable[5Ch](24h)` with Function
7 goes to the fore list when its node's forward axis (`[gun+3CCh]+110h`, row 2 of the world matrix)
has a positive dot with the hull's (`+0ECh..+0F4h`), else aft; `brain+0AECh` = the least
`[[gun+3F8h]+34h]+0E4h` (WaterTravelSpeed).

**009E9EB0** (`vtable[0Ch]`, body 009E9EB0-009EA9B7, no Ghidra function), in order:
1. No target: return. 009E9DE0; `+24h += dt`; `range` = 009E4A60; `t` = 00419010(`+54h`->0,
   `+58h`->1, range).
2. The close latch `+40h`: set to 1.0 when `50.0 > range` (00CE3938, double; `+48h` = 0, `+44h` =
   0); while set, cleared when `range > 80.0` (00CE5444; `+48h` = 1), else `+44h += dt`.
3. The end choice, when `+34h += dt` exceeds 5.0 (00CE3850). With the latch set: after
   `0.6 * +60h` s, if the firing end (the heading, turned by pi via 00438AA0 when `+30h` is the
   stern) is more than 3pi/4 (00D20E80) off the target, toggle. Bow (`+30h`=1): no ready bow tube
   for over 3 s (`+24h > 3.0`) with the bow reload longer than the stern's, toggle; else with a ready
   stern tube and the target more than 2pi/3 (00D2017C) off the bow, toggle. Stern: the mirror, with
   the bow taken back when the target is within pi/3 (00D05AAC) of the bow and a bow tube is ready.
   A toggle zeroes `+34h`, flips `+30h` and, when `5.0 > +24h`, forces the band flip below when the
   old end and `+38h` agree.
4. The band, when `+3Ch += dt` exceeds 3.0 or forced: closing (`+38h`=1) flips out when `+54h >
   range`; opening flips in when `range > +58h`; each flip redraws the band (009E9E50).
5. The aim: the target's x/z led by its velocity (`vtable[34h]`) for `min((range - 50)/+0AECh, 12)`
   seconds (00CEB4B8) when `range > 50`; the bearing of the normalised (00419510) offset
   (the 00414EB0 sequence inline); for the stern end, that bearing minus pi; `err` = heading minus
   bearing (00438B10).
6. The speed: closing, 00419010(`+54h`->0.25, `+58h`->3.0, range); opening, (`+58h`->-0.25,
   `+54h`->-3.0). Less `clamp(-closingRate / class MaxSpeed (+500h), -2, 2)`, negated for the stern
   end. Negative: clamp to [-0.5, -0.25] and turn the bearing by pi; else clamp to [0.25, 1.0].
   A limit 00419010(25 deg->1.0, 60 deg->0.5, |err|).
7. A heading error above 1.6 rad (00CE3D48) with the band position on the wrong side (0.8 and 0.1
   margins, 00CE3D40 / 00D7A3A0) holds a reverse for 3.0 s (`+64h`): speed +/-0.5 the other way and
   the bearing turned by pi. The speed is clamped to [-limit, limit], 009E0040 takes the bearing, and
   a throttle sign change needs three ticks (`+6Ch`) before the inline 009DBF90 stores it.
8. The depth. Latch set: a submarine target above us -> 009E4C70 (AllowMaxDepth ? 3 : 2), else
   009E4EE0; any other target -> AllowMaxDepth ? 3 : 2; then `periscopeState` = 0 unless it is 2.
   Latch clear: the chosen end has a ready tube and `1.2 > |err|` (00CE3814) -> 009E4EE0 (periscope
   depth when the boat has a periscope and it is not broken) and 009E4D90(1) (raise the periscope when
   at periscope depth and the target is not a submarine); else `SetDepthLevel(2)`.

## 6. Host substitutions (LABELLED in the code)

- Tube lists: the gunnery host's category-7 list stands for the child walk; a tube's node axis is
  the hull forward turned by the gun's horizontal angle (as the gunnery host builds it for
  008FFF20), so fore means `cos(horz) > 0` when the lists are first built.
- Operational tubes: the three bytes of 00729F10 have no producer, as for the firepower rating.
- The TorpedoBot FireTargetAccuracy row is this installation's robots.lua table the torpedo
  standoff already uses (factored into `torpedo_bot_fire_target_accuracy`).
- `periscopeState` (`unit+122Ch`) is a field of the controller: the units host models no periscope.
  `unit+1214h` is taken as present for every non-kamikaze class. AllowMaxDepth is the seed 1: only
  bsm_07 and bsm_11 call `luaMW_NavigatorAllowMaxDepth` in this installation.
- The target's velocity is `neighbour_world_velocity` (the hull heading times 0092D730).
- The parent countdown draw is made at the brain seed for submarines only.

## 7. Reach, from an OFF run with `BSP_SUB_ATTACK_SHADOW=1`

`local\s5_shadow_jm06.log` and `local\s5_shadow_lomp06.log` (worktree cc9-ships5, pair_diff against
the plain OFF runs: gameplay identical). The shadow prints 009E4A60 and 009E9CE0 while a submarine
runs attackmove.

| mission | boat | first attackmove | target | range then | R | tubes fore/aft |
| --- | --- | --- | --- | --- | --- | --- |
| JM06 | Narwhal-class Submarine 01 | 8.90 s | PlayerSub 03 | 2661 m | 833.4 | 4/2 |
| JM06 | PlayerSub 02 | 63.10 s | US Tanker 01 | 1993 m | 919.8 | 6/0 |
| JM06 | PlayerSub 03 | 63.10 s | US Cargo Transport 02 | 1683 m | 919.8 | 6/0 |
| LOMP06 | Narwhal | 10.50 s | Yugiri | 2660 m | 852.8 | 4/2 |

On attackmove the OFF ranges fall to R+150 at about 125 s (Narwhal, JM06), about 105 s (PlayerSub
02), about 85 s (PlayerSub 03); in LOMP06 the Narwhal is still at 2035 m at 45 s.

## 8. Predictions, written before any ON run

**Step 1, `kShipAiSubAttackSelectBound` alone** (the states stay records, so a selected boat is not
driven):
- JM06 3200/3000: `sub_attack select` lines for the Narwhal at about 8.9 s and PlayerSub 02 and 03 at
  about 63 s, and no submarine `attackmove` step after them; the three stop steering (the last blk
  values hold), so their tracks and the fight move: pair_diff exit 3.
- LOMP06 1200/1000: the Narwhal selects at about 10.5 s and stops steering; exit 3.
- USN02 9200/9000 and USN04 4700/4500 carry no submarine: exit 0 or 1, deaths identical.
- Verdict rule: this flag cannot flip on its own (it removes the attackmove drive and puts nothing
  in its place); it flips with step 2.

**Step 2, both flags:**
- The same selects. Each boat opens in approach: `SetDepthLevel(2)` (the PlayerSubs leave their
  periscope band -13 for -40; the Narwhal's level likewise goes to 2) and steers straight at the
  target's position.
- JM06: switches to fire when `range < R+150`: PlayerSub 03 first (about 75-90 s), PlayerSub 02
  (about 95-110 s), the Narwhal last or not at all by 150 s (about 110-140 s). In fire, a bow tube
  ready with `|err| < 1.2` puts the boat back at depth 1 with its periscope raised; the band makes
  it run in to 30-50 % of the outer radius and open out again. No boat is expected inside 50 m
  (the close latch) before 150 s. Gameplay moves (exit 3); the torpedo hit count and the deaths
  among the US transports are expected to change, direction not predicted.
- LOMP06: the Narwhal stays in approach for the whole 50 s (range > 1003 m), at depth level 2;
  exit 3 on its track.
- USN02 and USN04: identity (exit 0 or 1, deaths identical).
- Verdict rule: a boat that never enters the state, a switch at a range that contradicts
  `R+150`/`R+400`, or any movement on USN02/USN04 keeps both flags OFF.
