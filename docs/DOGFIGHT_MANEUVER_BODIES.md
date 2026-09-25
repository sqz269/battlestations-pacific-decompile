# Dogfight maneuver and avoid bodies (packet cc9_dogfight_maneuver_bodies)

2026-09-24. The dogfight task's unread state bodies are read here from the disk listing and bound.
Names are hypotheses. Offsets are relative to the state object S, with A = S+4h the approach,
U the unit, P the pilot plan block and T = A+B4h the target, unless written otherwise.

## 1. Inventory: what the E2 fighters reach

The counts are ticks per state, summed over the twelve US fighters in `local\SA_9000.log`, the
landed configuration of the substitution sweep.

| state or arm | ticks | host before this packet | disposition |
| --- | --- | --- | --- |
| follow (009C1FD0) | 24869 | follow law bound; the record name remains | already bound |
| moveto (009C18C0, dogfight profile) | 13816 | generic tick, speed slot and glide bound; heading 009F9E40 toward the target, as the image | already bound |
| aim (009A76E0) | 1411 | read and bound (docs/DOGFIGHT_ENGAGED.md) | already bound |
| **maneuver (009A8B20)** | **199** | **stand-in** (pursue arm only); enter 009A87F0 never ran | **bound**, `kDogfightManeuverBodyBound` |
| **avoid_roll / avoid_turn (009A7E80 / 009A80E0, via 009A7A50)** | **87** | **stand-in**: heading 90 degrees off the bearing, level | **bound**, `kDogfightAvoidBodiesBound` |
| **009A9970, the avoid choice** | 9 entries | always avoid_roll | **bound**, `kDogfightAvoidPickBound` |
| 0099DE8A gate (the planner's yaw base term) | every think with +2CCh != 2 | the host steered to the target bearing in any mode | bound, **landed OFF** (`kPlannerYawBaseModeGateBound`, section 4): it applies to every aircraft |
| attackrun (009A71E0) / prepare | 0 / 0 | partial | unreachable on E2 (squadron mode 1); left |

## 2. The bound bodies

The reads are in the scratch pseudo-C behind this packet, with every term's listing address. What
matters for the binding follows.

### 2.1 Maneuver: enter 009A87F0, tick 009A8B20, expire 009A86F0, threat 009A85B0

**Enter (009A87F0).**
- Clears the pursue latch +3Ch.
- The base heading is chosen three ways:
  - with no target, the current heading;
  - if 009A85B0(1) finds us in the target's nose cone (10 m < z < 1.5 × ShootDistance, lateral
    tangent < 1), the bearing to the target plus U(-0.5, 0.5);
  - otherwise **the bearing from the target to us, which extends away**.
- The heading vector (+20h, +24h) is that base plus U(-0.8, 0.8).
- The pitch is +30h = U(lo, hi), with:
  - hi = interp(100 → 0, 500 → climb angle; Ceiling - y);
  - lo = interp(200 → 0, 600 → -dive angle; unit+9B4h).
- The timer is +38h = U(0.8, 1.3) × ManeuverChangeTime.

**Expire (009A86F0).**
- Sets mode 0 and draws r = U(0, 1), doubled when the target's pilot targets us.
- With TurnAfterChance c > r, it sets +34h = FollowDist × approach+24h ×
  interp(0 → 0.25, 0.4 → -0.1; c - r).
- Otherwise it calls 009A8560.
- A negative +34h forces pursue on the next tick.

**Tick (009A8B20).**
1. The timer. At 0 it runs expire, then enter.
2. **Pursue**, while the latch is set, or d > +34h, or the plane is outside the map:
   - a re-aim countdown (+2Ch, period +28h = 1.0) recomputes the pitch point toward the aim point
     from +D8h (or climbs toward Ceiling - 50, at most 300 over 500 m, beyond 3 × ShootDistance);
   - the heading vector becomes the offset to the aim point;
   - it sets the latch, +34h = 0 and the timer to 10 s (16 outside the map).
3. **Mode 0, the heading servo.**
   - Heading error under 50 degrees: heading hold (+2C0h, mode 2) and pitch +30h (mode 2).
   - Under 125 degrees: mode 1 with bank ±1.3.
   - Otherwise: mode 1 with a bank drawn from U(lo, hi), where lo is the larger of an
     altitude term and a speed/LevelFlight term, and hi = interp(300 → 1.5, 700 → 2.0; height).
4. **Mode 1, roll-in.**
   - While wsub(bank, target) >= 25 degrees (a **signed** test): bank target +2C4h in mode 1, and
     the pitch target in mode 2.
   - Below it, mode 2, with +1Ch = ±1.
5. **Mode 2, pull.**
   - Pitch slot +1.0 with pitch mode 0.
   - Bank target ±1.5 (or upright / inverted) in mode 1.
   - Back to mode 0 once the heading error is under 30 degrees (or under 60 with the pitch error
     under 30).
6. **Every tick.** Throttle 1.0 and air brake 0, both active, and +2D8h = 0. Then
   approach+DCh = 1.5 and +E0h = 0.1.

### 2.2 Avoid: enters 009A7DE0 / 009A8020, common 009A7A50, ticks 009A7E80 / 009A80E0

**Enters.**
- Both: latch +24h = 0 and timer = U(0.75, 1.5) × AvoidTime.
- avoid_roll: +1Ch = ±U(0.8, 1.0), with the sign of the bank.
- avoid_turn: +20h = U(0.75, 1.0) and +1Ch = ±U(70, 100 degrees).

**Common (009A7A50).**
- The timer runs down. It ends early (-0.1) when the target is no longer ahead enough:
  - either +F4h / max(|+ECh|, |+F0h|) < 0.1;
  - or the listing's own-minus-target point, taken through the target's full inverse, gives a
    ratio under 0.1.
- It sets throttle clamp(r, 0, 1), air brake clamp(-r, 0, 1) and +2D8h = 0, with
  r = interp(-15 → 1, -45 → -1; pitch).
- The latch: with no target it is set; otherwise it sets when the target is more than 5 m behind.
  - Unlatched: approach+DCh = 0 and +E0h = 1.
  - Latched: DCh = 1 and E0h = 0.5, and a timer above 2 becomes U(1, 2).

**avoid_roll tick.**
- Roll slot = +1Ch with heading mode 0.
- Unlatched: yaw slot = -sgn(+F8h) with yaw mode 0, and pitch slot = -sgn(+FCh) with pitch
  mode 0.
- Latched: pitch target 0.5 in mode 2.

**avoid_turn tick.**
- Unlatched:
  - with y = +FCh < 0.3, pitch slot +1 in mode 0; otherwise -1;
  - roll slot = interp of x / y (three arms), in heading mode 0.
- Latched:
  - the pitch target is (A+8)+1E4h, then overwritten by pitch mode 0;
  - +20h decays by 0.5 × dt;
  - bank target +1Ch × (1 - dt) in mode 1;
  - pitch slot = interp(20 → 0.1, 50 degrees → +20h; |bank|) in mode 0.

### 2.3 The avoid choice: 009A9970 with 009A7F70 and 009A83B0

- **Roll weight** wr = interp(-40 → 0, 10 degrees → 1; pitch) × interp(100 → 0.2, 300 → 1;
  unit+9B4h).
- **Turn weight** wt = interp(40 → 1, 70 degrees → 0.4; |90 degrees - |wrap(bank)||) ×
  interp(100 → 0, 300 → 1; unit+9B4h).
- **Choice.** r = U(0, wr + wt), and roll when wr >= r. Below 100 m, roll is always picked.

### 2.4 The planner's yaw base term, 0099DE8A

- It is reached only through `CMP ECX,2` on plan+2CCh.
- Any other mode leaves the base numerator at 0, so the plane holds no heading.
- The per-think reset 0099B450 seeds +2CCh = 1, so a state that does not write mode 2 gets no
  heading hold. The host used the bearing to the commanded target in every mode.
- The maneuver's modes 1 and 2 and both avoid ticks rely on it.

### 2.5 Stand-ins, labelled in the source

- Every 00BD2F10 draw is taken at its midpoint, the convention of this dogfight binding. That makes
  the avoid choice "roll when wr >= wt".
- The aim point is the target origin.
- unit+9B4h is the height above the water surface.
- 0071C4F0 (outside the map) answers false.
- 009A85B0's T+9C0h / 007B9140 clause is skipped.
- 007BBC10 "the target's pilot targets us" means the target's dogfight target or commanded target
  is this unit.
- approach+24h is 1.0.
- The plan+268h bit-2 timer decay never runs (+268h is only stored as 0).
- The throttle, air-brake and +2D8h writes of both bodies stay behind the existing
  `kDogfightThrottleBound` (OFF).
- (A+8)+1E4h is the class climb angle.

## 3. Predictions, written before the pair

Three runs, E2 9000, same tree, `BSP_GUNNERY_RNG_STREAMS=1`:
- DM0: all four switches off, the SA configuration;
- DM1: all four on;
- DM2: all on except the planner gate.

**Off side (DM0).** Equal to SA: fighter fire ticks 222, fighter hits 101, 37 deaths, 553 hit
records, the Lexington moved 6517.06 m.

**Maneuver.**
- The fighters now extend away from a target that does not target them, then pursue.
- Expire with no threat gives +34h = -30 m, which forces pursue at once.
- Maneuver ticks change by -50% to +200%. Mode 1/2 ticks appear, up to half of maneuver ticks.

**Avoid.**
- Level fighters above 300 m pick roll (wr 0.8 against wt 0.4).
- Turn is picked only when banked near knife-edge or nose-down: 0-40% of avoid entries.
- Avoid ticks change by ±50%.

**Planner gate.**
- Zeroed ticks are above 0. Every state that leaves +2CCh at its reset value 1 with a commanded
  target loses its heading hold.
- If this is large on Vals or Kates, their paths change broadly. Judged against DM2.

**Headline rows (DM1).**
- Fighter fire ticks 100-400.
- Fighter hits 50-180.
- Deaths 28-42, and US fighter losses 0-3.
- Releases 0/0.
- The Lexington's path is unchanged apart from coupling (5.5-7.5 km).
- No mission end.
- Any switch whose rows move beyond this, without the read explaining it, lands OFF.

## 4. The runs, measured

`local\DM0_9000.log` (all off), `local\DM1_9000.log` (all on) and `local\DM2_9000.log` (bodies on,
planner gate off). Binaries `local\dm0`/`dm1`/`dm2`; the module directory was checked in every log.

| row | DM0 off | DM1 all on | DM2 bodies only | prediction | verdict |
| --- | --- | --- | --- | --- | --- |
| off side = SA | 222 fire ticks, 101 hits, 37 deaths, 553 hit records, 6517.06 m | - | - | equal | held |
| planner yaw base zeroed ticks | 0 | **67436** | 0 | above 0 | held in sign |
| Japanese deaths (Val / Kate / movieval) | 16 / 16 / 3 | 16 / **0** / 3 | 16 / 16 / 3 | 28-42 total | DM1 **missed** (19); DM2 held (36) |
| US fighter losses | 2 | 0 | 1 | 0-3 | held |
| hit records / damage | 553 / 8001.4 | 332 / 4301.5 | 552 / 7821.6 | - | - |
| fighter fire ticks / hits | 222 / 101 | 177 / 86 | 216 / 104 | 100-400 / 50-180 | held |
| maneuver ticks (sum) | 199 | - | **29** | -50% to +200% | **missed**, lower (explained below) |
| maneuver mode ticks 0 / 1 / 2 | - | - | 19 / 0 / 10 | modes 1/2 up to half | mode 1 never holds a tick (below) |
| avoid ticks / roll : turn entries | 87 / all roll | - | **19** / 18 : 9 | +-50% / turn 0-40% | ticks **missed**, lower; turn 33% held |
| releases | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | held |
| Lexington moved | 6517.06 m | 6710.43 m | 6184.98 m | 5.5-7.5 km | held |
| mission end | none | none | none | none | held |

**The bodies (DM2) are explained by their reads.**
- **Maneuver ends fast.** With TurnAfterChance 1.0 (SPNormal) above the midpoint draw 0.5, expire
  sets +34h = FollowDist × interp(…; 0.5) = -30 m. The next tick pursues, and 009A9BD0 returns
  the fighter to aim as soon as it is on target. So maneuver ticks fall from 199 to 29, and aim
  ticks rise (sqn01 564 to 825, .-3 492 to 1341).
- **Mode 1 holds no tick.** The roll-in test is signed (0.436 > wsub(bank, target)), so a
  positive bank target drops straight into mode 2, as the read notes.
- **Avoid ends fast.** 009A7A50 ends the timer (-0.1) as soon as +F4h / max(|+ECh|, |+F0h|)
  < 0.1, that is once the target is no longer well ahead. So avoid ticks fall from 87 to 19.
- **The choice** takes turn on 9 of 27 entries: banked or nose-down fighters, as the weights say.
- **The headline rows stay within coupling.**

**The planner gate (DM1) moves rows its binding cannot own.**
- With 0099DE8A gated on mode 2, 67436 plan ticks lose the heading base term. The Kates never
  reach the fleet: Kate deaths go from 16 to 0 and hit records from 553 to 332.
- **The gate is the image's.** 0099DE8A is reached only with +2CCh == 2, and 0099B450 reseeds 1
  each think.
- **But the host's fallback was standing in for per-state heading writes.** The fallback was the
  bearing to the commanded target in every mode. The only such writer identified,
  009AC190/009AC40B, belongs to Kamikaze/gotowards (docs/PILOT_TASK_HEADING_ARM.md), not to a
  general arm. So the host's torpedo (and other) states that never write +2C0h with mode 2 were
  steering on that fallback.
- Landing the gate needs those states' own heading writes first.

**Switch states landed:**
- `kDogfightManeuverBodyBound` ON.
- `kDogfightAvoidBodiesBound` ON.
- `kDogfightAvoidPickBound` ON.
- `kPlannerYawBaseModeGateBound` **OFF**. Its next step is a census of which host states reach
  the planner with +2CCh != 2 and a target (a per-state count of the 67436), then each state's
  heading write.

**Bodies without a Ghidra function** (`ghidra proto --brief` returned no function):

| body | start | exclusive end |
| --- | --- | --- |
| avoid_roll tick | 009A7E80 | 009A7F6C |
| avoid_turn enter | 009A8020 | 009A80DE |
| avoid_turn tick | 009A80E0 | 009A83AB |

009A7DE0, 009A7A50 and 009A7F70 now have Ghidra functions.

## Correction from docs/PLANNER_HEADING_WRITES.md (integrator, 2026-09-25)

`kPlannerYawBaseModeGateBound` is now **ON** (main fab4dbebe), landed together with
`kPilotStateHeadingWritesBound`. Section 4's DM1 result was not the gate's: the host had never run
the two heading writes the image's torpedo states make, so its bearing fallback was steering the
Kates. The image steers torpedo moveto through 009F9E40 at 009C1B23 (mode 2) and torpedo
attackrun by AddWrapped(approach+94h, +20h) at 009D0927 with mode 2 at 009D092D. Torpedo
done/prepare (009D2720) writes +2C4h = 0 with mode 1, which is the reseed. Pair PHW0/PHW1 (E2
9000, both OFF / both ON): Kate/Val/movieval deaths 16/16/3 on both sides, hit records 592 -> 588,
Lexington 6,751 -> 5,929 m, no mission end. The switch table above (line 20) is superseded by this
section.
