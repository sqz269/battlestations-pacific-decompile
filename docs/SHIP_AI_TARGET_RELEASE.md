# Ship AI target release: when a dead attackmove target lets go

Packet cc9_target_release, 2026-09-23. Base: main 659eaf1cd. Switch `kShipAiTargetReleaseBound`
in `src/game_hosts_ship_ai.cpp`. Status: reconstructed, build-tested and run-compared on USN04
and USN01. Not ABI-compatible and not game-validated. Descriptive names are hypotheses.

Addresses: 009F1420 BSP_ShipAi_BrainPrePass, 009F1160 BSP_ShipAi_BrainRecordConstruct, 009E2FB0
(the command-target latch), 00521EA0 BSP_CommandTarget_ResolveObject, 00836920 (the weapon
director step) and its attackmove arm 00836B45..00836BEB, 0043F080 (the four-byte live test),
005457C0 (still hostile), 009F3240 (the attackmove approach step), 00926C80 and 00926390 (the
death queue and the destroy hook), 00923BE0.

## 1. Who writes [brain+0B20h]

A disp32 store census over .text found two stores against the brain:
- `89 ?? 20 0B 00 00`: 4 hits
- `C7 ?? 20 0B 00 00`: none
- `89 ?? ?? 20 0B 00 00`: none
- `C7 ?? ?? 20 0B 00 00`: 1 hit

The two brain stores:
- **009F1271** in BSP_ShipAi_BrainRecordConstruct, `MOV [ESI+0B20h],EBX` with EBX zeroed:
  the constructor.
- **009F1480** in BSP_ShipAi_BrainPrePass, `MOV [EDI+0B20h],ESI`. ESI was loaded at 009F1478
  from `[EDI+0B0Ch]`, which is the goal record (+0AF8h) field +14h.

The other three hits are in other classes: 00A996E3, 00A99A7A (joystick) and 0068CF43 (the
in-game interface).

The record field is written only by the latch 009E2FB0, from 00521EA0 applied to the active
command's descriptor (0071EB60). 00521EA0 returns the descriptor's cached object at +4h. When
that is null, it looks the id at +2h up in the handle tables (00F89A0C / 00F89A54 or
00F89A60 / 00F89AA8, 16-byte stride, entry +0Ch) and caches it.

So the brain holds whatever the command names. A dead but not yet removed entity still
resolves. **Nothing on the brain side clears the target on death.** It changes only when the
command changes.

## 2. What ends the command: the director's attackmove arm

00836920 runs once per unit per fixed step. When slot 0 holds the attackmove object
(`CMP EAX,0E08F78h` at 00836B45), the arm runs these tests in order:

1. 00521EA0 on director+58h. A null target raises stage 2 (00836B5C, then 00836BB2).
2. vtable[5Ch](41h), the nav point. True does nothing (00836B6B).
3. vtable[5Ch](1Ch), the command building. When +5Eh is set, or the target's side equals
   [[director+34h]+54h], it converts the order to `moveto` (00465080, then 0071ECF0 at
   00836BAD) and raises stage 2. Otherwise it does nothing.
4. Otherwise 0043F080, which is +5Ch set and +5Dh, +60h and +5Eh clear. False raises stage 2
   (00836BC9).
5. Otherwise 005457C0(ECX = [director+24Ch], target+54h), meaning "still hostile". False
   raises stage 2 (00836BE6).

Stage 2 is 0071D810(2). It sends MT_GAMEUNIT_CLEARCMD, and the receive side 00721A40 advances
the queue (docs/COMMAND_COMPLETION.md). The next director step's idle tail issues `cruise` or
`stop`, and the next 009F1420 re-latches the brain's target from the new command.

**The validity test for a dead unit.** vtable[5Ch] is IsKindOf, a class property, so a dead
plane still answers 5 and 6. What changes at death is the lifecycle bytes:
- The damage path's 00926C80 sets +60h and queues the entity at once.
- The 009273A0 flush's 00926390 sets +5Dh on the next frame (docs/ENTITY_DEAD_FLAG.md).

0043F080 therefore fails from the death on, and the arm releases a plane target at the first
director step after it.

There is a second, earlier guard. 009F3240, the attackmove approach step, reads target+5Dh at
009F3277. A set byte takes the hold arm 009E00A0 (hold heading and stop) and skips the whole
nested update. So the image never rates a dead target: no ring scan, no standoff choice.

## 3. Host against image

| Term | Image | Host before | Host now |
|---|---|---|---|
| brain target latch | 009F1480 from the command | the same | unchanged |
| attackmove arm 00836B45 | the five tests above | a record only; never ended an attackmove | bound through a target-facts source the ship AI host publishes to the commands host |
| building conversion to moveto | 00465080 + 0071ECF0 | not reached | record only, labelled; no building target reaches the arm in either mission |
| 0043F080 for a dead unit | false from the death on (+60h) | true forever: the host keeps a dead unit's scene bytes live | false once the gunnery host sets `sunk` |
| 009F3277, target+5Dh | set at the flush | always clear | set once the unit is dead |

The host has no separate flush, so the death itself stands in for both +60h and +5Dh.

## 4. Predictions

These were written to local/rl_predictions.txt before the runs.

1. Every escort holding attackmove on the dead Val releases at the first director step after the
   death. This held. Lexington did not release, because by then its slot 0 held moveto or
   moveonpath.
2. The AI command tick would issue a new attackmove within seconds. This was WRONG. No new order
   came, and all eight escorts stayed in `stop` to the end.
3. No scan rates a dead target. This held, because there are no scans after the release.
4. First standoffs would be unchanged. This held. Last standoffs would no longer be 300. This
   held, but for another reason: the last choice is the one made before the death.
5. Decisions after the death would move substantially. This held, and gunnery moved (below).
6. USN01 would be identical. This held.

## 5. Measurement

Runs used `BSP_GUNNERY_RNG_STREAMS=1` on both sides, at 4700/4500 frames for USN04 and
3200/3000 for USN01. The control is the same source with the switch false. Logs:

- local/rl_ctl_usn04.log and local/rl_trt_usn04.log
- local/rl_ctl_usn01.log and local/rl_trt_usn01.log

On this base, D3A Val #1.1 dies at **162.71 s** in both runs. Other packets merged since
cc9_ship_traffic moved the gunnery baseline, so this is no longer 156.20 s. Both runs are
identical up to the first arm line.

| Ship | Released | Standoff choices ctl / trt | Standoff first-last ctl | Standoff first-last trt | Heading changes ctl / trt |
|---|---|---|---|---|---|
| Northampton-class01 | 162.76 s | 402 / 153 | 2250-300 | 2250-2250 | 25 / 9 |
| Northampton-class02 | 162.76 s | 402 / 153 | 2250-300 | 2250-2250 | 139 / 135 |
| Fletcher-class01 | 162.76 s | 402 / 153 | 1450-300 | 1450-1450 | 76 / 47 |
| Fletcher-class02 | 162.76 s | 402 / 153 | 1450-300 | 1450-1450 | 99 / 57 |
| Fletcher-class03 | 162.76 s | 402 / 153 | 1550-300 | 1550-1550 | 97 / 9 |
| Fletcher-class04 | 162.76 s | 402 / 153 | 1450-300 | 1450-1450 | 95 / 51 |
| York-class01 | 162.76 s | 402 / 153 | 1850-300 | 1850-1850 | 31 / 30 |
| York-class02 | 162.76 s | 402 / 153 | 1850-300 | 1850-1850 | 138 / 17 |

Per ship, every decision row is identical until one fixed step after the death. That step logs
`attackmove arm 00836b45: ... target=22 target not live (0043f080) at 162.76 s; stage 2, queue
1 -> 0`. The ship then goes to `state=stop`, from step 3260. It makes no further approach
decision, so the choices stop at 153 and the last standoff stays the pre-death one.

In the control, 249 more choices rate a dead target to the end, which is where the 300 m seed
came from. The hold arm at 009F3277 was never reached: the release lands one step after the
death, before any approach step sees the flag.

| Result | Control | Treatment |
|---|---|---|
| queued hits | 253 | 241 |
| damage | 18515.8 | 18217.5 |
| deaths | 24 | 24 |
| Lexington | sunk at 185.81 s | afloat, 141.9 HP |

In the control, the gunnery table credits Lexington's sinking to Fletcher-class01. The last
logged impact before the death is bullet 15 (took 14.9, health 19.1); which gun fired it was not
traced. In the treatment, the stopped escorts shoot down D3A Val #1.1|.-3 at 176.46 s
and |.-4 at 179.41 s. Both survive in the control. Seven other aircraft rows change killer or
time.

USN01 is identical apart from the arm's own record. It ran 23,328 times and released nothing,
because every attackmove target there stays live and hostile.

Decision: landed with `kShipAiTargetReleaseBound` on. The release is the image's director arm
and is explained per ship. The idle after it is the host's reconstructed idle tail.

## 6. Step 4, read-only

- **The 1.0 floor is the image's answer.** 0095EB40's category gate at 0095EBD7..0095EC1E
  skips categories 0, 5, 0Ah and 0Bh, so an aircraft's category 0 guns and its category 10 bomb
  never count. Only its one category 1 device does. Ship Armour is 50 (Fletcher,
  VehicleClass[23] in this installation's vehicleclasses.lua). The small-calibre damage the log
  shows is 12.0, so the excess clamps to 0 and the weight takes the 1.0 floor.
- **The plane positions against the 1100 m keep radius** were not checked; there was no room in
  this turn.

## 7. Open

- After the release, the AI coordinator issued no new attackmove in either run: 54 attackmove
  choices and 50 suppressed duplicates in both. It is unverified whether the image's
  coordinator re-tasks released escorts. If it does, the 60 s idle in the treatment is a host gap
  in the AI tick, not in this arm.
- In the control on this base, the kill credit for Lexington goes to Fletcher-class01, which
  is friendly fire. Worth a gunnery look independently of this packet.
- The arm's building-to-moveto conversion is not issued.
