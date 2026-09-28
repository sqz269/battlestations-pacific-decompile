# How close a ship on `attackmove` goes (packet `cc9_attackmove_engagement_range`, a read)

Worker cc9-ships3, on main `b605b9eb9`. Ghidra was read-only. No code changed. Descriptive names
are hypotheses.

## 1. Answer

**The image holds a standoff, and the host already holds the same one.** Nothing is bound.

- **A surface ship on `attackmove` never leaves the approach sub-state** (`state+8h`,
  `009F3240`). The only exit is the engage sub-state (`state+14C0h`, `009E23B0`), which closes
  to an intercept point and latches a 250 m run. Its gate `009E85B0` opens only for a class
  with `KamikazeDamage` or `KamikazeBlastDamage` above zero. In this installation's
  `vehicleclasses.lua` (mtime 2026-05-09, modded) those are the Kaiten and the Kamikaze Boat.
  Myoko (Haguro, Nachi), Kuma (Jintsu, Naka), Shiratsuyu (Samidare, Murasame, Harusame) and Fubuki
  (Sazanami, Ushio) carry neither field. Only those two classes define the fields.
- **The approach picks a range from two damage curves, not from the gun range.** In mode 0,
  `009E6E80` seeds the standoff at 300 m past the longest range at which the target can still
  hurt this ship. It then scans 50 to 3000 m for the smallest ratio of the target's expected
  damage to this ship's (docs/SHIP_AI_APPROACH_CURVES.md).
- **The throttle keeps the ship on that ring.** Outside the standoff, `009E6A90` caps the throttle at
  `(d - 50) / 80`, where d is the range beyond the standoff, so the ship stops 50 m outside it.
  More than 50 m inside it, the ship backs astern at up to half throttle. Both caps apply only
  while the hull is within 15 degrees of the commanded heading.
- **The heading is a ring bearing.** It is one of sixty ring bearings, scored with the
  per-bearing firepower rating `0095EB40`. That rating is what favours a bearing on which more
  guns bear. It is a score term, not a hard broadside rule.
- **The host runs this whole chain concretely** (section 4). Haguro's chosen standoff on USN02
  is 1900 m at first and 2150 m by the end. It hovers at 1600..2050 m from Houston at near-zero
  speed and sinks her from 1878 m. That is the image's rule working, not a missing one.

So the kill rate at about 1.9 km is a gunnery question, which the lead has already assigned.

## 2. The image

### 2.1 The engage gate is a kamikaze gate

`009E85B0` (`BSP_ShipAi_AttackMoveEngageGate`, body `009E85B0-009E86B5`), first conjunct:

```
009E85B9  MOV EAX,[ECX+0AA8h]          ; the unit
009E85C7  MOV EAX,[EAX+538h]           ; its class record
009E85CD  MOVSS XMM1,[EAX+510h]
009E85D8  COMISS XMM1,XMM0 / JA 009E85EE
009E85DD  MOVSS XMM1,[EAX+514h]
009E85E5  COMISS XMM1,XMM0 / JBE 009E86AF   ; both <= 0 -> false
```

The two fields are stored by `BSP_ShipClass_ReadLuaFields` (`00831840`):

```
008319F6  PUSH 00D09CF4                ; "KamikazeDamage"
00831A1D  CALL 00B66330                ; read, default 0.0 (FLDZ at 00831A0A)
00831A22  FSTP dword [EDI+510h]
00831A3B  PUSH 00D09CE0                ; "KamikazeBlastDamage"
00831A67  FSTP dword [EDI+514h]
```

- **The two strings were read from the PE on disk.**
- **The byte scan is noisy.** It finds 1281 candidate writers of displacement 510h/514h across
  unrelated types. The only other float pair is `BSP_GameSettings_LoadFromLuaGlobals` at
  `00840378`/`008403B7`, which is a different object.
- **The other two conjuncts** are the avoid-zone test and "within 2000 m of the destination"
  (docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md).
- **The selector `009E86F0` reaches the engage member only through this gate** for a target
  that is neither kind 8 nor kind 1Ch.

### 2.2 The approach arm

`009F3240` runs `009F3090`'s seven calls, hands the approach point to `009DE050` with
`keep_mode = 1`, and writes `brain+1D8h = clamp(nested+1210h, -1, 1)` at `009F3635`
(docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md, docs/SHIP_AI_APPROACH_UPDATE.md). For an ordinary ship
target the approach point is the target's own position, so `nested+11E0h` is the range to the
target.

**The standoff seed, mode 0** (`009E71A5`):

```
009E71A5  LEA EBX,[ESI+13B0h]          ; the target's curve
009E71AE  CALL 00952530                ; longest range with a positive sample
009E71B3  FADD qword [00CE3CA8]        ; +300.0
009E71C1  FSTP dword [EDI]             ; nested+11E4h, then the 119-step scan
```

**The span radius** (`009E6FBD`) is `r = max(250.0, 00811A30(unit, 1.0) * 1.5)`, with `FLD1`,
`CALL 00811A30` and `FMUL qword [00CE3D78]` (1.5). The side byte is `nested+1204h = 2` when the
ship is at or beyond the standoff, and 0 inside it (docs/SHIP_AI_APPROACH_UPDATE.md).

**The arrival rule** in `009E6A90`, side 2:

```
009E6D42  FLD [ESI+11E0h] / FSUB [ESI+11E4h]      ; d = range - standoff
009E6D58  FLD qword [00CE4938]                    ; -50.0
009E6D70  FCOMI / JB 009E6DDE                     ; d < -50 -> back off
009E6D83  COMISS pi/12 (00D05AA8), |e| ; JBE      ; only when aligned
009E6D8E  FSUB qword [00CE3938] (50.0) ; FDIV qword [00CF1440] (80.0)
009E6DBF  CALL 00415620                           ; clamp((d-50)/80, 0, 2.0f)
009E6DD0  CALL 00415510                           ; min(limit, that)
009E6DF7  FCHS ; FSUB 50.0 ; FDIV qword [00D7A378] (40.0)
009E6E2A  CALL 00415620                           ; clamp((-d-50)/40, 0, 0.5f)
                                                  ; nested+1210h = -min(limit, that)
```

The constants were read from the image: `00CE4938` -50.0, `00CE3938` 50.0, `00CF1440` 80.0,
`00D7A378` 40.0, `00D05AA8` 0.2618, `00CE3800` 0.5f and `00CE3958` 2.0f.

**No gun range enters the chain.** The one class distance in `009F3240`,
`t = 2*([unit+494h] + 500 - nested+11E0h)` clamped to [0, 1000], goes to `brain+258h`/`+2C0h`
(`009F337B`, `009F3383`). Neither field has a reader.

## 3. Measured, this tree's build (main `b605b9eb9`)

The logs are `local\s3_tr_usn02.log` (plain) and `local\s3_tr_helm_usn02.log`
(`3135 takehelm Houston 1.0 EscapePoint`). Both ran USN02 9200/9000 with streams and the death
table on and `BSP_TORPEDO_TRACE` naming the DRKillers and the FinalShips (a diagnostic only).
The script is `local\s3_ranges.py`.

"rel" is the bearing of Houston off the bow: 0 is bow-on and ±90 is broadside.

**Helm run.** Houston is sunk at 208.26 s by Haguro's gunfire from 1878 m. Her first damage is
at 166.61 s. This reproduces docs/SCRIPTED_HELM.md 9.3.

| ship | standoff first/last | range at 156 s | closest | rel | speed |
| --- | --- | --- | --- | --- | --- |
| Haguro | 1900 / 2150 | 2040 | 1694 m at 188.5 s | -38 to -22 | -0.7..2.6 m/s, throttle toggles -0.625 / +1.0 |
| Samidare | 900 / 1450 | 2012 | 1658 m at 188.0 s | +16 to +28 | about 0 |
| Murasame | 900 / 1450 | 2503 | 1888 m at 195.0 s | -14 to +10, bow-on | 8.7 |
| Harusame | 900 / 1450 | 2735 | 2282 m at 186.5 s | +31 to +120, passes broadside | 8.7 |
| Jintsu | 1000 / 1650 | 2654 | 2148 m at 187.0 s | +13 to +76 | 9.3 falling to about 0 |

**Plain run.** Houston survives with 6037 health, having taken 900. Haguro holds near-zero
speed. The range falls from 2040 to 1571 m by 206.5 s because Houston closes on her. Murasame
is still closing bow-on at 1496 m at 212 s.

Haguro is sunk by Exeter at 206.66 s in both runs.

**What the numbers show:**
- The cruisers stop inside about 2.2 km and hold.
- The destroyers close toward their 900..1450 m standoffs.
- Nobody goes to point-blank range.

## 4. The host, term by term

| image term | host | state |
| --- | --- | --- |
| engage gate `009E85B0`, `[[unit+538h]+510h]`/`+514h` | answers 0 | the image's value for every class in USN02, USN01, USN04 and JM06's ships; wrong only for a Kaiten or a Kamikaze Boat on `attackmove` |
| engage step `009E23B0` | record | unreachable while the gate is shut |
| standoff choice `009E6E80` with the curves `0095F080` | concrete (`choose_standoff_range`, `curve_refresh_own`, `curve_target_block_1238`) | bound |
| its target-kind-8 arm `009E6EFC` | record, false | the image's answer for a ship target |
| span radius `009E6FBD`, `00811A30` | concrete (`unit_class_turn_circle_radius_0082e960`) | bound |
| ring scan `009E76D0`, bearing ratings `0095EB40`/`009E5DA0`, commit `009E5E90` | concrete | bound |
| arrival and back-off `009E6A90` | concrete (`limit_throttle`) | bound |
| `[unit+494h]` term to `brain+258h` | record, 0 | no reader in the image either |
| frame-state probe length `nested+11F0h` = `max(class+500h * 10, 00811A30(unit, 1.0) * 1.5)` (`009F1D1E..009F1D6D`) | `00811A30` at `009F1D3C` is a record answering 0, so the host keeps `class+500h * 10` | **a divergence, but not a range.** It is only the avoid-zone probe length of `009E6640`, and USN02 records `hits=0`. See section 5 |

## 5. Follow-ups (not bound here)

1. **`009F1D3C`: the probe length.** The host already has the image's `00811A30` as
   `GameUnitsHost::unit_class_turn_circle_radius_0082e960(index, 1.0f)`. Using it in
   `ApproachPointBinding::unit_turn_radius_00811a30` (`src/game_hosts_ship_ai.cpp`, about line
   2643) would give the frame state the image's longer probe. The effect is limited to slots
   that an avoid zone blocks. That needs its own OFF switch and pairs, near land or zones.
2. **The phase-2 FinalShips have no ship AI.**
   - `GameShipAiHost::register_units` sizes the controllers once, from the units present at
     registration.
   - Nachi, Sazanami, Naka and Ushio are generated at phase 2 (about mission frame 3089), so
     they have no ship AI rows.
   - Their four `NavigatorAttackMove` orders are recorded, but the ships sit at their spawn
     points: (4200, -7500), (4700, -7500), (-4200, -7500) and (-4700, -7500).
   - They fire 0 shots, with the nearest target at 3020..4092 m. See the unit table in both
     logs.
   - In the image, Nachi and Sazanami would join the attack on Houston. This matters more to
     Houston's survival than any range rule. It is a host registration gap, not an image
     switch.
3. **Kamikaze classes.** A binding of the engage gate to the class's two fields, plus the
   `009E23B0` step, which is already projected in `src/ship_ai_attackmove_substates.cpp`,
   matters only for a mission that orders an AI Kamikaze Boat or Kaiten to `attackmove`.

## no_ghidra_function

None. Every address named here lies inside a Ghidra function.
