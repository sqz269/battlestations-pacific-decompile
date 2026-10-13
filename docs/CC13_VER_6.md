# CC13 VER 6: strike release gates, image against host (cc13-ver6)

Packet cc13-ver6 was a read-only verification, done for cc9. Its target was the strike release gap:

- USN21: the torpedo task releases 0-4 of 43-91 aircraft, and the dive task 0-20 of 92-128.
- USN01: ConSBD3's dive never releases.

Method:

- Every gate was compared with the disk bytes (`bsp.py disasm-raw`) and with the host source.
- x87 operand order was read from the stack, and one DC/D8 FMUL form was read from the bytes (`009C5586` is
  `D8 C9`).
- Existing logs only; no new runs.
  - USN21: `J:\PROG\battlestations-pacific-decompile-cc9-ships44\local\s46_e_u21d.log`.
  - USN01: `J:\PROG\battlestations-pacific-decompile-cc9-physics1\local\p6u1_gj.log` and `p6u1_g.log`.
  - USN04: `J:\PROG\battlestations-pacific-decompile-cc9-gunnery36\local\g38_auoff_usn04.log`.

Status of the C++ added here:

- `src/strike_release_ver_cc13.cpp` and `include/bsp/strike_release_ver_cc13.hpp` are reconstructed and
  build-tested.
- They are not wired into the host. `src/game_hosts_units.cpp` is leased by cc9, so every host change below is
  an adoption note.
- Nothing here is game-validated.

## Findings

| # | where | image | host | symptom |
| --- | --- | --- | --- | --- |
| 1 | `game_hosts_units.cpp:37618` | - | the dive-task summary's `bombs_spawned` sums `torpedo_drops_spawned` | USN21 "dive bombs_spawned=0" is false; the per-aircraft lines show `bombs_spawned=1` for each release (`s46_e_u21d.log:395884`) |
| 2 | `0099AF53`-`0099AFC2` | if the free-flight test, `unit+5Ch` or `007B9140` fails, C58h is zeroed; otherwise each task's vt[+24h] is offered, and the first true answer spends one order | spent only inside the torpedo arm (`:26785`); no zeroing; the dive task's `009C8200` is never called, so `dive_bomb_rounds_pending` (task+424h) is never raised | USN21 dive: the second round (`rounds_left=1` on every releasing Val) would leave through the `009C884E` passthrough in goaway/done. Conditional on the Val's `007C0D90` arm, see caveat |
| 3a | `009D49CE`-`009D49DA` -> `0099B6A0` | +52Ah set, not prepare and not engaged: tail jump to the base slot, which calls `007BBBA0` (release) when the task has a squadron and the aircraft is in free flight, and returns 1 | `torpedo_task_arm.cpp:128-133`: `consumed=false`, no release | wingmen never drop on the flight's order |
| 3b | `007EEF96`-`007EEFEE` | after or instead of the 999 loop: when the issuer leads its flight, mode != 2, `unit+1B0h != 8` and `!00927F10`, each other member gets `007BCBE0(m, 1)` | `torpedo_release_orders.cpp:143` returns before it | the same wingman orders, independent of the 390h/374h gate |
| 4 | `009C6EB1`, `009C6F17` read frame `[ESP+28h]` | `A` = R, the 3-second lead range written at `009C63A6` (the only writer in `009C62B0`-`009C6F97`) | `:25642` passes `db_planar_bc` (\|aim - pos\|) | USN01 ConSBD3 and USN21 Vals reach aimglide higher than the image: about 80 m on the ConSBD3 run-in by a kinematic model. That shortens, but does not remove, the ceiling-blocked stretch |
| 5 | `009B83D0` | the level-bomb slot +24h raises task+424h; in prepare it calls `009B8100` | no model; `:23233` says the passthrough needs a "player-held aircraft" | none on AI rows: the level-bomber branch of `007C0D90` never calls `007EEF30` |

The shared predicate: `(unit+72Ch)->vt[38h]` is `0074E210 BSP_PlaneControlMode_IsFreeFlight`, which tests
`[ECX+1D4h] == 7`, i.e. unit+900h == 7.

- The MPlaneBomber constructor stores vtable `00D065B0` at unit+72Ch (`007D776B`). Its slot 38h, at
  `00D065E8`, is `0074E210`. MPlaneFighter's `00D06898` has the same slot.
- The host sets +900h = 7 for an airborne aircraft (`game_hosts_units.cpp:15554`, `:14142`).
- So the gate holds for AI aircraft. The host comment at `:23233` reads it as a player test, which is wrong.

Slot +24h, read from `.rdata`:

| table | slot +24h | function |
| --- | --- | --- |
| base `00D05708` | `0099B6A0` | the base release request |
| dive `00D20E18` | `009C8200` | dive override |
| level bomb `00D20210` | `009B83D0` | level-bomb override |

A scan for the dword `A0 B6 99 00` finds `0099B6A0` in 20 tables.

### Caveat on finding 2

An order reaches a flight only when `007C0D90` takes the single-drop arm to `007EEF30`. The test is at
`007C0E1F`-`007C0E65`: an IsKindOf(10h) bomber whose bullet answers none of 2Ch, 2Bh or 33h takes the rack
walk instead, and that walk never issues orders. Resolved:

- `MPlaneDiveBomber` (12h) has parent 0Fh, not 10h (`src/entity_class_ids.cpp`). So a dive bomber takes
  the single-drop arm `007C0EE0` and does reach `007EEF30`. This assumes USN21 class 113 is built as
  MPlaneDiveBomber.
- `007EEF30` gives orders to the wing and not to the issuing leader: the 999 loop skips the leader unless
  ctl+378h is set, and the leader block writes to the others only.
- So the task+424h passthrough fires for wingmen: those in follow/moveto/goaway/done, and a wingman's own
  remaining round after its drop.

Torpedo bullets answer 2Bh, so finding 3 does not depend on any of this.

## Gates verified equal to the image

- **Torpedo aim tick, the five-flag chain.**
  - `009D1F75`-`009D2287`: the speed switch at `00CF3F20`, and the envelope `interp(0.5,1,1,+84h;|cos|)*speed`
    (`00419010` is stdcall `RET 14h`).
  - The aim-solution byte (`+200`), and the lead flag: envelope > F14 and F14+slack > +A0h.
- **`009FA3A0` and `007BB110`.**
  - The release timer and its guard match `torpedo_first_release.cpp`.
  - The re-seed draw uses ECX=1, stream 1.
- **Dive aimglide release chain, `009C5689`-`009C5755`.**
  - The rearm gate, the 30-degree bearing and the ceiling: `[ESP+1Ch]+50 > [ESP+20h]`, where
    `[ESP+1Ch] = row+40h * approach+A8h`, written at `009C5493`.
  - The 120 m lateral gate and the lead window.
- **Dive aimglide pitch, `009C5484`-`009C55DF`.**
  - `atan((C-H)/max(|range-throw|,1))`, then min with +1ECh.
  - `p1` over ratio (0.75, 1.15) and `p2` over height (0.7C, 1.1C).
  - Then `max(p1,p2)`.
- **Flyabove altitude arm `009C6E10`-`009C6F91` and `009FB800`.** Everything matches except the divisor
  (finding 4).
- **Flyabove BL, span, leave and dive test, `009C64EE`-`009C680E`.**
  - These match.
  - The old-style-bombing byte ctl+3A8h has one writer, the Lua binding at `0089EB39`, and this installation's
    scripts never call it.
- **Dive break-off `009C8A90`.**
  - The host's form drops the attacking-state term.
  - It is only consulted from attacking states (`dive_bomb_task.cpp:107`), so the two are equivalent there.
- **Level-bomb release and abort flags, `009B639A`-`009B6428`.** These match `plane_level_bomb_task.cpp:263-274`.

- **Level-bomb release tick `009B59F0`-`009B5AF0`.** It matches `lb_release_tick_009b59f0`:
  - the one-shot `007BBBA0` and `+2Ch -= 1`;
  - the bank and pitch holds;
  - the countdown that runs only while +18h is set and unit+C25h is clear, else re-seeding 5.0.
- **Approach+A8h.** `009C7AB3`-`009C7AF6` lowers it each tick to `ctl+39Ch - 0.05*A8`.
  `dive_bomb_decay_dive_altitude_009c7a94` already does this.

## Host fixes landed (OFF)

Both landed under HOST FIX TERMS, each with its own switch:

| sha | switch | finding |
| --- | --- | --- |
| `fe1dbc68e` | `kTorpedoArmFallthroughReleaseBound` (`torpedo_task_arm.hpp`) | 3a |
| `80da020b3` | `kReleaseOrderLeaderBlockBound` (`torpedo_release_orders.hpp`) | 3b |

- With the switch OFF, behaviour is unchanged byte for byte.
- With the switch ON but no host change, behaviour is also unchanged: the new inputs default to false, and the
  new host virtuals default to skipping the block.
- The feeds belong in `game_hosts_units.cpp`, so they are adoption notes.
- Correction to `fe1dbc68e`'s message: its RNG line names stream 1 and the `009FA3A0` reseed. That is wrong for
  this path. A wingman drop through `0099B6A0` never runs `009FA3A0`, and the fix adds no draws of its own.

## What the logs say stops the strikes

- **USN21 torpedo.** No aircraft opens the lead flag.
  - Cabot_sqn04 died in aim at F14 = 1044.8 m. Its envelope is at most +7Ch, about 975 m
    (`s46_e_u21d.log:397060-397076`).
  - Intrepid_sqn02 died in attackrun at 4345 m.
  - That is attrition before the gate, not a gate divergence. Findings 3a and 3b add wingman drops only when a
    leader drops.
- **USN21 dive.** Most aircraft die in attackrun.
  - The rest enter aimglide around 1080 m with a ceiling of about 208 m, and every call is blocked at the
    ceiling (`:395877-395883`). They start the flyabove at about 1388 m.
  - With the reference already at the 0.8 cap, finding 4 does not change that.
- **USN01 ConSBD3.**
  - The flight hands over from flyabove to aimglide at 453-468 m, against a ceiling+50 of 284-297 m
    (`p6u1_gj.log`, hand-overs).
  - |.-4 got below the ceiling after 38 calls and was then held by the lead window.
  - Finding 4 lowers the hand-over by roughly 45-80 m (`local/ver6_flyabove_sim.py`, kinematic model).

## Uncertainty

- Finding 3b's party test: the name `00927F10 BSP_PartySlot_IsAiHeld` is a hypothesis. Which side's slot
  answers false in single player was not read.
- The predicted counts are bounds, not run results: findings 3a and 3b add at most (wing size - 1) drops per
  leader release.

## Handoff

- **For cc9 (`game_hosts_units.cpp` adoption).**
  - Call `torpedo_offer_release_order_009d49a0` at `:26822` in place of `torpedo_arm_drop_009d49a0`. Raise
    request_ordnance_release on `release_007bbba0`.
  - Add `release_orders_leader_block_007eef96` after the 999 loop in `torpedo_issue_release_orders_007c0d90`.
  - Feed the flyabove altitude arm `db_flyabove_lead_range`.
  - Run the 0099AF53 spend over the dive task, using the existing `dive_bomb_arm_drop_009c8200` and
    `release_order_spend_0099af53`.
  - Fix the summary sum at `:37618`.
- **Open.**
  - Whether `00927F10` answers false for the player's side in single player.
