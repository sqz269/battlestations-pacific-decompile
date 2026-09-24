# Plane substitution sweep (packet cc9_plane_substitution_sweep)

2026-09-24. This packet inventories the labelled stand-ins that the aircraft packets of lineage
cc9 left in `src/game_hosts_units.cpp`, then closes the ones that one read can settle. Names are
hypotheses. The full 47-row inventory, with line numbers, was taken read-only. This page keeps the
terms named in the brief and every term this packet changed.

## 1. The terms and their disposition

| term | where | what the image does | disposition |
| --- | --- | --- | --- |
| Squadron +3A4h enable bits "taken as set" | 009A1865/194F/1972 | **Reads the squadron field.** `BSP_PlaneSquadronTickableEntity_Construct` passes ECX = squadron+37Ch at 007F2CA2 to 007F2BD0, which stores `[ECX+28h] = 0FFh` at 007F2C13. That is squadron+3A4h = 0FFh. **Who changes it:** only luaMW_SquadronEnable{Terrain,Vehicle,Gunfire}Avoidance (0089FE50, 008A0000, 008A01B0) set or clear bits 1/2/4, and no USN04 script calls them. | **Confirmed.** The value is the image's; the label is removed. No row change. |
| pilot+2E4h enable mask | 009C87A3/009C8855, 009D4865/009D48FF | Set to 0FFh per tick; no store clears a bit (docs/ATTACKER_EVASION.md 6) | Confirmed earlier. |
| unit+B04h "taken as zero" | 007DF62D (the vehicle arm's frame) | **No writer found.** A store census over FSTP, MOVSS, MOV and MOVUPS at disp32 AFCh-B08h finds no writer. The only plane reader is 007DF62D, so the field keeps its zeroed allocation. | **Confirmed**, with the block-copy caveat. No row change. |
| unit+C7Ch "as the flight-path angle" | 0099F1C0's pitch | It is the elevation of the vector at unit+AC8h (007C1CA8), and +AC8h is the world velocity (vtable[34h] 007BBB70, docs/FIGHTER_GUN_LEAD.md) | **Confirmed**: the host term is exact. |
| unit+BC4h (0099BB00) "not modelled" | 0099CAB0 yaw pass | **Writer.** 0099BB00 is a max-latch, read at 0099BB00-0099BB28. **Consumer.** The rate law reads it at 007DA92C and multiplies it into the yaw target, YawSpd × latched yaw × BC4h × mode factor (007DA926-007DA95F). This is before its relax at 007DA967-007DA9E3: when it is not 1.0 and dt > 0, it moves toward 1.0 by 0.5 × dt, clamped at 1.0. The earlier doc's frame slots at 007DAA18/007DAA1F hold sin(bank), not BC4h. | **Closed**, behind `kPlaneYawGainBc4Bound`. |
| Torpedo done/prepare tick: time to target = 0 | 009D2A44 through 009D1500 | The range over the reference speed, which is the metric the aim tick already reconstructs (`torpedo_time_to_target_009d1500`) | **Closed**, behind `kTorpedoArmTimeToTargetBound` (plumbing). |
| Fighter lead: target acceleration | 00954650 arc arm | **Read.** unit+654h = (+648h - +6BCh) / dt per fixed step, where +648h = (position - previous position) / dt (007CEE05-007CEE5B). | **Bound but landed OFF**, behind `kFighterLeadAccel654Bound`. The finite differences are kept per plane in the fixed step, and the lead reads the target's (zero for the first two steps). The pair moves the fighter rows beyond the read (section 3). |
| gunFire +BC9h for lastGunState +C35h | 0099EC40's firing list | **Only two literal stores** of +C35h: the constructor 007D01A1, and a clear at 007CC33E when the plane dies (+5Eh). **The setter** is reached another way, most likely the `lastGunState` property write, which is unread. | **Open.** Blocked on the property-table setter. |
| Kind-13h (+DF4h) timer scaling | 0099EFA5-0099F038 | Scales AvoidTime/WaitTime for kind-13h (Fighter) planes | **Open.** It is reached only for a flagged fighter, and in E2 only Val wingmen flag (docs/ATTACKER_EVASION.md 5.4). No E2 row effect; not read. |
| 007D7A40 after a changed pitch or yaw | 0099BF30 tail | Writes 1.2f to [ctl+10h]+0C8h | **Open.** The reader of +0C8h is unknown (also open in docs/PLANE_CONTROL_AUTHORITY.md). |
| Model box from Width/Height/Length | 007DF4F0 (class+50h, class+500h) | The mesh's local bounds | **Open.** Needs the model bounds, which the host does not load. |
| Sea surface for the avoid-zone layer ctl+34Ch | 0099F1C0, 0041BC20 | **What USN04 has.** 0041DF40 always returns a layer: it primes with the first. USN04's `.nav` holds three 240×240 layers at 100 m. | **Open.** The layer records live in the scene-contents host, and binding 0041BC20 needs plumbing outside this packet's hunks. Over open water the stand-in probably equals the layer (unverified). |
| Sea surface for the ground under unit+9B4h | vehicle and terrain arms | Height above the ground or sea | **Open.** Same blocker. USN04 is open water. |
| Squadron class = the member's class | 009BECD0 | [squadron+35Ch]+190h | No row effect on USN04 (one class per squadron); left. |
| Torpedo/dive neutral answers | the inventory's row 24 | TorpFlikFlakTime draw, 009FADA0, 007DF360 and others | **Open.** Each needs its own read: 00901610's PilotBot context, 009FADA0, 007DF360. |
| Controlled unit's party for the player's | 009A186E | [[00E188A8]+5FCh]+908h | No row effect on USN04 (the controlled carrier is the player's party). Left. |
| Player-record rebind, 0080E290, suppress tails | 00780120 4Bh arm, 0077F360 | docs/SCRIPTED_HELM.md 6.1 | **Open.** Their consumers are ship AI or unread. |
| Dogfight maneuver/avoid stand-ins (009A8B20, 009A7A50, 009A9970) | fighter states | Unread bodies | **Open.** These are the largest likely effect, but three large bodies; a packet of their own. |

## 2. Predictions, written before the pair

The pair is E2 9000 with the three new switches off (S0) against on (S1), from the same tree
(main 2026-09-24 plus this packet), with `BSP_GUNNERY_RNG_STREAMS=1`. The confirmed terms change
no code path.

- **Off side.** It equals the current tree's idle reference.
- **BC4h.**
  - Raised only by terrain yaw bands that cover stick 0 with m > 0.4, which means Kates on their
    low runs.
  - yaw_gain_raises: 0-500. It relaxes at 0.5 per second, so each raise lasts at most about 1-2 s.
  - Kate paths shift slightly, so the AA picture moves by coupling.
- **Torpedo time to target.** It enters only the done/prepare tick of a Kate that has finished or
  is preparing. Releases stay 0 in the idle scenario.
- **Target acceleration.** It changes the fighters' arc-lead only by the difference between a
  per-step and a per-think difference. Fighter hits stay within ±25% of the off side.
- **Headline rows:**
  - releases 0/0;
  - Japanese deaths 28-42;
  - hit records within ±15% of the off side;
  - Lexington path unchanged (the aircraft do not steer it);
  - no mission end.

## 3. The pair, measured

`local\S0_9000.log` (all three switches off, binary `local\ss0`) against `local\S1_9000.log` (on,
`local\ss1`), plus one split run `local\SA_9000.log` (only the target acceleration on,
`local\ss2`), E2 9000, `BSP_GUNNERY_RNG_STREAMS=1`.

| row | S0 off | S1 all on | SA acceleration only | prediction | verdict |
| --- | --- | --- | --- | --- | --- |
| off side = current idle reference | 549 hits, 35 deaths, 6905.23 m | - | - | equal | held (matches the scripted-helm packet's unset row) |
| BC4h raises | 0 | **0** | - | 0-500 | held at the edge: no terrain yaw band on E2 reaches m > 0.4 |
| torpedo done/prepare time-to-target calls | 0 | 0 | 0 | only on finished/preparing Kates | held: the done/prepare tick never runs on E2 |
| hit records | 549 | 553 | 553 | within +-15% | held |
| Japanese / all deaths | 35 / 35 | 35 / **37** | 35 / 37 | 28-42 | held in count, but two US fighters now die |
| fighter fire ticks / hits | 129 / 80 | **222 / 101** | 222 / 101 | within +-25% | **missed**: fire ticks +72% |
| releases (torpedo / bomb) | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | held |
| torpedo task releases (commanded) | 7 | 5 | 5 | - | coupled |
| Lexington moved | 6905.23 m | 6517.06 m | 6517.06 m | unchanged | **missed**: coupled through the air battle |
| mission end | none | none | none | none | held |

**Split.** S1 and SA agree on every summary row except the yaw-gain census, so every movement comes
from the target acceleration. The time-to-target term is never reached on E2 and BC4h never rises.

**The acceleration term moves rows its read does not explain.**
- The two US fighter deaths: Lexington-class01_sqn01 .-3 at 107.90 s is credited to its own flight
  leader, a friendly-fire kill, and the leader at 414.87 s to movieval .-2.
- The +72% fire ticks.

The read predicts a change of arc lead, not of this size. Either the per-step second difference of
the host's positions is noisier than the image's, or the image's fighters have the same exposure.
This pair cannot tell which.

**Switch states landed:**
- `kTorpedoArmTimeToTargetBound` ON: faithful plumbing, not reached on E2.
- `kPlaneYawGainBc4Bound` ON: its read is complete, and it is inert on E2.
- `kFighterLeadAccel654Bound` **OFF**, for the reason above. Its next step is a trace of |unit+654h|
  per step against the gun-tick difference on the sqn01 flight.

## 4. The acceleration term, traced (packet cc9_fighter_accel_friendly_fire)

**The trace.** `kFighterAccelTraceEvery` is a diagnostic, 0 in the landed build. `local\FAT1_9000.log`
is the SA configuration with it at 20. On every lead tick it computes both accelerations: the
target's per-fixed-step +654h, and the gun tick's own velocity difference. It then runs 00954650
on each and compares the two lead points. The trace does not perturb the run: FAT1 equals SA on
every headline row.

| fighter | lead ticks | mean gun-tick dt | mean / max \|a654\| | mean / max \|aold\| | arc arm, a654 / aold | mean lead-point separation |
| --- | --- | --- | --- | --- | --- | --- |
| Lexington sqn01 | 354 | 0.100 s | 4.74 / 22.57 | 4.66 / 22.52 | 252 / 249 | 0.07 m |
| sqn01 .-2 | 378 | 0.100 s | 2.45 / 19.83 | 2.39 / 19.73 | 195 / 194 | 0.05 m |
| sqn01 .-3 | 378 | 0.100 s | 5.14 / 22.13 | 5.03 / 21.94 | 270 / 270 | 0.08 m |
| Yorktown sqn02 .-3 | 180 | 0.100 s | 1.92 / 5.53 | 1.88 / 5.52 | 102 / 99 | 0.08 m |

The sampled lines are 3-11 cm apart at 780-920 m range.

**Mechanism.**
- The term is a real lead change that the read supports, and it is tiny.
- It is not a units or timing mismatch:
  - +648h and +654h are per 0.05 s step, and the gun tick differences over 0.1 s;
  - the magnitudes agree within 2%;
  - the arc arm is taken on the same ticks.
- It is not a term the image zeroes: 00954650's arc arm reads +654h whenever the target turns.
- **Divergence explains the 72% fire-tick move** (129 to 222, and 358 with the hold also on).
  A centimetre-scale change of the aim point flips one envelope or burst-clock decision. After
  that, the engagement and every later burst diverge. It is the same kind of coupling as the
  shared random stream (memory: shared-rng-stream-couples-pairs), not an error in the term.

**One caveat stays.** The image's +648h/+6BCh are filled by a snapshot that no store census finds:
the difference block 007CED7E-007CEE5B only reads the previous-position and previous-velocity
fields. A block copy is the likely writer. The host takes the previous values as the previous
fixed step's, which is what the read implies.

**Switch state: `kFighterLeadAccel654Bound` ON,** with this corrected read. The consequence to know:
the E2 reference now carries SA-like fighter rows, including a friendly-fire loss of sqn01 .-3 to
its own leader, which the image's own hold does not prevent (docs/FIGHTER_GUN_LEAD.md 5.2).
