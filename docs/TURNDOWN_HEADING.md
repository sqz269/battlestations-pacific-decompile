# Turndown heading and the aimdive pitch gains: both are the image's, and the row in force is open

Addresses: 009C44F0, 009C4551, 009C45B3, 009C4637, 009C4654, 009C467C, 009C46C9, 009C7800, 009C7EA0, 009C58D0, 009C5BF4, 009C5CAB, 009C5CD0, 009F9CE0, 009F9D22, 0072BBD0, 0072BC12, 008FBC80, 008FBCAC, 007ECF80, 007D66F0, 0099A880, 008AE030, 00895250

Packet `cc9_turndown_pitch`, 2026-09-22. It answers `docs/DIVE_BOMB_REATTACK.md` section 6 and the
pitch-gain item in `docs/FLYOVER_EXIT.md`. The report is `reports/turndown_heading.json`. All names
are hypotheses, not recovered symbols.

**Result.**
- **The turndown commands no heading.** The image's turndown is a roll-to-inverted and a pull, and
  it completes on pitch and bank alone. The host matches it term by term. So after a goaway, a
  split-S from behind ends with the aircraft pointed steeply down and still facing away, in the image
  as in the host.
- **The gains match SPNormal.** The aimdive pitch gains are the PilotBot row's
  `DiveBombAimPrecPullPlus/Minus`, read every tick. The host feeds the SPNormal values, 0.018 and
  0.025, which is the default row.
- **Which row is in force is not established.** It depends on a skill chain whose setter is unread.
  This host's own `GetDifficulty` of 0 makes the USN04 script request **Stun** for the launched
  strikers, with gains 0.03 and 0.06.
- **Nothing bounded differs, so nothing is bound and no run was made.**

## 1. The turndown, 009C44F0, read whole

The body runs 009C44F0-009C4736, `__thiscall(state, dt)`, RET 4.

- **Speed.** 009C450D-009C4524 set cmd+2B4h to 007C47F0 (LevelFlight x StallSpd), with
  cmd+2B0h = 0 and cmd+2D8h = 1.
- **Bank.** 009C4530-009C45A5 wrap the bank into (-π, π] (00BF857A fmod by 00CE3828, then ±2π)
  and fold it to |b|.
- **Unlatched** (state+1Ch clear, 009C45A9):
  - **Manual roll** while |b| < 0.8 (00CE3D40, double): roll = interp(0.5236 → 1, 0 → 0, π - |b|)
    x state+18h (009C45EA-009C460F). It goes on cmd+290h, with +294h = 1 and mode +2CCh = 0.
  - **Servo** once |b| ≥ 0.8: bank target cmd+2C4h = π (00D7A264, float) with mode 1
    (009C4637-009C464E).
  - **Latch:** state+1Ch is set when |b| > 2.618 (00D1FED0, float, 150°) (009C4654-009C465D).
  - **Pitch:** held level. While |pitch| < 0.349 (00CE398C, float, 20°), cmd+29Ch = 0 in mode 0;
    otherwise the pitch target cmd+2BCh = 0 in mode 2 (009C467C-009C46B6).
- **Latched** (009C46C9-009C4736): the bank target stays π in mode 1, and
  pitch = interp(0.5236 → 0, 0.0524 (00D0CBA0) → 1, π - |b|). That is a full pull within 3° of
  inverted, easing to none at 30°.

**The turndown writes no heading and no yaw.** It never compares a bearing to the target.

**009C7800, SetRollDirection.** It sets state+18h = (arg ≥ 0 ? 1.0 : -1.0) x
Uniform(0.8 (00CE74F8), 1.0) on stream 1. The argument is the side the transition rule picks: the
stored side, else from the bank, else random. **It is not the target's side.**

**009C7EA0, IsComplete.** It returns true when `pitch < -1.3` (00D1F98C, float, 74.5°), or when
`pitch < -1.0` (00D7A260) and `|bank| > 2.356` (00D20E80, 135°). **It has no heading term.**

**The host.** `dive_bomb_turndown_tick_009c44f0` and `dive_bomb_turndown_complete_009c7ea0`
(src/dive_bomb_task.cpp) reproduce every term above with the same constants. The host tick wires
roll, the π servo, speed and pitch the same way. No term differs.

**What this means for `docs/DIVE_BOMB_REATTACK.md` section 6.**
- **The split-S itself.** From behind at 1.4 km, the image rolls inverted and pulls, and it
  completes once the nose is 57-74° down. Until the nose passes vertical, the horizontal heading is
  still the original one. So **the image also hands the aimdive an aircraft still facing away**, as
  the host did: 2.64 rad at aimdive entry.
- **Heading reversal.** It is left to the aimdive's roll and pitch law. The turndown completing
  before it happens is the image's own behaviour, not a missing term.
- **What remains open.** Whether the image's aimdive recovers that geometry, where the host's does
  not, is still unknown. It rests on the same pitch loop as step 2.

## 2. The aimdive pitch gains

**The command.** At 009C5C9F-009C5CF7, `pitch = clamp(error x gain, -1, 1)`.
- The gain is `[EBP+64h]` when the error is positive (009C5CAB) and `[EBP+68h]` otherwise
  (009C5CD0).
- EBP = `[EDI+14h]` (009C5BF4), where EDI = `[state+4]` is the approach. So the gains are
  `(approach+14h)->+64h/+68h`.
- **The error** is metres of signed along-track miss:
  `gain x (cos(bearing) x distance - lead)`, from 009C5C9B (`docs/AIMDIVE_RESPONSE.md`).
- **The clamp** saturates beyond 1/0.018 = 55.6 m and 1/0.025 = 40 m. So the host's ±200 m swing
  runs the stick bang-bang for most of the dive.

**The producer chain:**
1. **Row pointer.** approach+14h = `[00F8A30C] + index x 248h + 0Ch`, written once by the approach
   base constructor at 009F9D22 (`BSP_BotApproach_ConstructSpeedReference`). The fields are read
   live each tick, so the row is authoritative, and robot_config.hpp names +64h/+68h as row
   +70h/+74h, `DiveBombAimPrecPullPlus`/`Minus`.
2. **Index.** It is `[[unit+DF4h]+34h]`, the skill index of the plane's bot object. That object is
   allocated at 007D66F0 (0BCh bytes, constructed by 0099A880). The base 0072BBD0 seeds +34h = 1 at
   0072BC12 (a seed, not authoritative). `BSP_GunBot_Attach` 008FBC80 then sets it from
   `unit->vtable[12Ch]()` (008FBCAC), which returns unit+390h. unit+390h defaults to 1 (0095CCCC,
   per `docs/UNIT_GUNNERY_PASS.md`).
3. **Row order.** The rows are Stun 0, SPNormal 1, SPVeteran 2, MPNormal 3, MPVeteran 4, Elite 5,
   per `luamw_init.lua` and `docs/UNIT_GUNNERY_PASS.md`.
4. **The script.** In USN04, `usn_19_coralus.lua` sets `Mission.SkillLevel` from `GetDifficulty()`
   (lines 68-76: 0 gives SKILL_STUN, 1 SPNORMAL, 2 SPVETERAN). Every launched striker gets
   `SetSkillLevel(launchedStriker, Mission.SkillLevel)` (lines 1412-1595). For a squadron that
   broadcasts `vtable[128h](level)` to its members (007ECF80).
5. **The host.** Its `GetDifficulty` returns 0, because game+6ACh has no writer here
   (src/game_hosts_script_orders.cpp). It records the 18 `SetSkillLevel` calls but does not apply
   them.

**Authored values**, this installation's `robots.lua`, PilotBot, both floats at the row:

| row | PullPlus (+64h) | PullMinus (+68h) |
|---|---|---|
| Stun (0) | 0.03 | 0.06 |
| **SPNormal (1), the host's** | **0.018** | **0.025** |
| SPVeteran (2) | 0.0 | 0.0 |
| MPNormal (3) | 0.014 | 0.018 |
| MPVeteran (4) | 0.012 | 0.016 |
| Elite (5) | 0.0 | 0.0 |

**Why nothing is bound.** Two links of the chain are unread:
- a member plane's `vtable[128h]` body, and whether it re-applies `SetSkillIndex` to the bot;
- whether the approach, which captures the row pointer ONCE at construction, is built before or
  after the script's `SetSkillLevel`. The script calls `PilotSetTarget` first, at line 1411.

Depending on those links, the image's strikers fly SPNormal (the host's) or Stun, and SPVeteran at
difficulty 2 would remove pitch feedback entirely. **So the gain comparison is: the host matches the
default row, and the row in force is not established.** Binding Stun from the host's difficulty of 0
would rest on two unread links, so `kTurnDownHeadingBound` is not introduced.

## 3. Predictions and runs

No bounded term differs, so there is no treatment, and none of the requested runs would be a
before/after pair.

**Predictions, recorded for whoever binds the skill chain.**
- **Stun gains** (0.03/0.06) saturate at 17-33 m instead of 40-56 m. That makes the aimdive more
  bang-bang, so the swing should not shrink.
- **SPVeteran** (0/0) holds pitch at 0 through the aimdive. The dive then follows the turndown's exit
  attitude and roll alone.
- **Neither** changes the turndown's hand-over, which is fixed by section 1.

## 4. Decision

* **No binding.** `kFlyoverSpeedBound` stays true. `kAimDiveTailBound` and
  `kHullAimOffsetEnabled` stay false.
* **Next read**, in order:
  1. a member plane's `vtable[128h]` body;
  2. the order in which the dive-bomb approach is constructed relative to the script's
     `SetSkillLevel`;
  3. the image's game+6ACh writer, `MissionStart::set_effective_difficulty` at 0058BF58, recorded
     only.
  Together these fix which PilotBot row the USN04 Vals fly, and so the aimdive's gains, power and
  brake values.
