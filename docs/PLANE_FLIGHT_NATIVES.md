# Plane-side natives: vtable[34h], the fixed step's predicates, the rate law and the commit

Addresses: 007BBB70, 007CE040, 007CE9FD, 007CEA02, 007CEA1C, 007CEA29, 007CEAB8, 007CEB00,
007D1314, 007B84D0, 007DA710, 007BB920, 007CD930.

Packet `cc9_plane_flight_natives`. Every name is a hypothesis, not a recovered symbol.

## 1. vtable[34h] in the aim tick

The plane unit vtable `00D05F20` slot `+34h` is `007BBB70 BSP_Unit_CopyVectorAC8`
(`FLD [ECX+AC8h..AD0h]; FSTP [out]; RET 4`). `unit+AC8h..AD0h` is the **world linear
velocity** (`docs/PLANE_FOLLOW_HOLD_ARM.md`: `ctl+18h`; `docs/PLANE_FOLLOW_PHASE_A.md` reads the
leader's velocity through the same slot).

So `009A76E0`'s head-on flag is `local z > 1 && dot(own velocity, target velocity) < 0`. The
binding had used the forward rows. `kDogfightHeadOnVelocityBound` (true) uses
`plane_world_velocity` for both.

## 2. The fixed step's release-issue predicates (007CE9FD-007CEB31)

The stage, as `docs/TORPEDO_ISSUE_TIMING.md` already binds its arithmetic, with
`ESI = unit+310h` and `EDI = unit`:

| site | predicate | width | read |
| --- | --- | --- | --- |
| `007CEA02` | `[[00E188A8]+1FE4h] == 2` skips | dword | the session mode, 0 for none (`docs/GAME_SESSION_POLLS.md`); single player is 0 |
| `007CEA0F` | `unit+9E0h` (`[ESI+6D0h]`) | byte | already bound |
| `007CEA1C` | `unit+C3Ah` | byte | written only by the plane state-message arm (`007D1314`, after `unit+C39h`, then `vtable[70h](1)`), zeroed by the constructor (`007D0136`); `007B84D0` is a one-line setter with no caller or vtable reference |
| `007CEA29` | `unit+5Dh` | byte | the scene `simulate` byte |
| `007CEA33` | `unit+900h` in {7, 6, 4, 5} | dword | already bound |
| `007CEAB8` | `* class+1F4h` | float | `BombDelay` (`007D2318`); in this installation 0, 0.1, 0.3, 0.4 or 0.5 |
| `007CEB00` | `unit+974h[i]->vtable[1FCh]()` over `unit+994h` weapon slots | byte return | **contract**: the slots' `+1FCh` is not the gun vtable's (that one takes arguments), so the slot object is a sub-object not identified here |

`kPlaneFixedStepPredicatesBound` (true):
* session mode 0, `+C3Ah` clear (exact: no plane state message in this host), `+5Dh` from
  `simulate`, and the interval scale from `BombDelay` in place of 1.0;
* the device walk stays unimplemented.

## 3. The rate law and the commit

**`007DA710`**: `docs/PLANE_CONTROL_RATE_LAW.md` and `docs/PLANE_CONTROL_TARGETS.md` read the
free-flight arm in full (the flag from `007DA380` is 1 in the air). The host's
`plane_control_targets_007da710` and `plane_control_axis_step_007da710` are those rules, and the
host always passes controller mode 0. Only the ground arm (`007DA542`) is unread, and this host
never runs it. **Host against image: no term differs on the path the host runs.** The call was
still logged through `record`, which writes to the unimplemented table.
`kPlaneControlRateLawBound` logs it as implemented. No behaviour changes.

**`007BB920`**, read from the listing (`007BB920`-`007BB998`, plain `RET`):
1. `unit+61h` set skips (`007BB923`). It has no writer (`GameUnitsHost::unit_flag_0061`).
2. `unit+A14h` clear skips.
3. Flight state not in {7, 6, 4, 5}: `cmd+10h` (`unit+A0Ch`, air brake) = 1.0 (`007BB94C`).
   **Missing in the host.**
4. `IsKindOf(17h)` and `unit+C24h` clear: `cmd+0Ch` (`unit+A08h`, throttle) = 1.0 (`007BB972`).
   **Missing.** `unit+C24h` is `PilotFires`, set once at load by `007CD930` when any gun's weapon
   group entry `+0Ch` is set (`docs/ATTACK_GATE_TAILS.md`). That authored weapon-group byte is
   also the enable the gun-trigger loop reads, which answers "the weapon-group enable byte's
   producer": authored class data, per weapon group (`PilotFires`: 113 + 88 true, 30 + 20
   false in this installation's vehicleclasses.lua).
5. `007BB6E0(unit+9FCh)` quantizes, then `unit+A14h = 0`.

`kPlaneCommitCommandBound` (true):
* binds item 3 and logs the commit as implemented;
* item 4 stays a contract, logged as `Plane::pilot_fires_c24` for `IsKindOf(17h)` planes,
  because the host does not load PilotFires.

**`unit->vtable[1FCh](gunFire)` at `007CE98D`** is not read this packet.

## 5. Predictions, written before runs X0/A1/A1T/B1/CD1

All runs are USN04 at the E2 parameters. X0 is this tree with the four new switches off. Each
treatment turns on one item; A1T is A1 plus `kDogfightThrottleBound`.

1. **A1 (head-on velocity).** Head-on aim ticks fall sharply for the Yorktown fighters: they
   chase from behind, so the velocities are mostly aligned. The stand-in head-on speed rarely
   runs, and the aim speed command becomes `(d - 300) + target speed` most of the time.
   * Fighter paths change a little, and through them recon and AA (RNG-coupled).
   * Dive-bomb rows should not change unless the recon picture does.
2. **A1T.** With few head-on ticks, the direct throttle cut rarely applies, so the Yorktown
   leader and `.-2` **survive**. If they still drown, the stall is not the head-on cut.
3. **B1 (predicates).** Only aircraft with release requests (`+C20h`) are affected: torpedo
   Kates and dive-bomb Vals. The interval becomes `0.9 x BombDelay` instead of 0.9 s. For a Val
   or Kate with BombDelay 0, successive issues run on consecutive steps. Expect release timing
   in the dive-bomb and torpedo rows to change for multi-release aircraft. The unimplemented
   total falls by about 4 x 213,420.
4. **CD1 (rate law and commit).** The rate law changes only the log. The air-brake override
   fires only outside flight states 4-7: aircraft on deck or launching, if any are in those
   states when a command is committed. Expect no or few rows to move. The unimplemented total
   falls by about 213,338 + 106,645.

## 6. Runs

All runs are USN04 at the E2 parameters. Each ran from its own freshly copied binary. X0 is the
shared control.

| run | switch on | log | unimplemented total | rows vs X0 |
| --- | --- | --- | --- | --- |
| X0 | none | `local\X0_9000.log` | 20,568,744 | - |
| A1 | head-on velocity | `local\A1_9000.log` | 20,568,744 | identical |
| A1T | head-on velocity + throttle wiring | `local\A1T_9000.log` | - | the two Yorktown drownings return |
| B1 | fixed-step predicates | `local\B1_9000.log` | 18,581,064 (-1,987,680) | identical |
| CD1 | rate law + commit | `local\CD1_9000.log` | 19,861,461 (-707,283) | identical |

"Identical" means every summary row, all 8 water contacts, the killed_by table, the 365
dive-bomb rows and every dogfight row. The refills counter is ignored. B1 and CD1 together
remove 2,694,963 unimplemented calls per run: 4 x 496,920, plus 471,538 and 235,745.

**A1.** The head-on counts are unchanged, 195/183/169 in both runs. Classifying by velocity
gives exactly the ticks the forward rows gave. The flags are genuine: Yorktown's fighters fly
out from the carrier and meet the incoming Vals head-on, and fly forward, so velocity and nose
agree. Prediction 1 was wrong.

**A1T, the stall.**
* Head-on ticks are 213/197/181 (minimum throttle 0.463/0.458/0.390), identical to HT.
* The leader and `.-2` still drown, at |v| 50.94 and 51.18.
* The vtable[34h] substitute was **not** the cause. Neither is the per-think re-seed
  (`docs/PLANE_GUNFIRE.md`).
* The stall happens after aim hands the fighters back to moveto or follow, following about 200
  ticks of the direct-throttle arm.
* Still unread: why a fighter that leaves aim at that speed cannot recover in this host's
  follow and moveto.
* `kDogfightThrottleBound` stays off. Prediction 2 was wrong.

**B1.** The four predicates are bound, and the device walk remains the one unimplemented site.
No row moves. Why the BombDelay scale changes nothing on USN04 was not traced.

**CD1.** No row moves. No `IsKindOf(17h)` plane commits a command in USN04, so the PilotFires
contract never logs. The air-brake override's count was not censused.

## 7. Decisions

* **Land:** `kDogfightHeadOnVelocityBound`, `kPlaneFixedStepPredicatesBound`,
  `kPlaneControlRateLawBound` and `kPlaneCommitCommandBound`, all true and all neutral on USN04.
* **Throttle wiring:** stays off.
* **Contracts:**
  * the device-busy walk (`007CEB00`, slot class unidentified);
  * the commit's PilotFires throttle override (PilotFires not loaded);
  * `unit->vtable[1FCh](gunFire)` at `007CE98D`, unread.
