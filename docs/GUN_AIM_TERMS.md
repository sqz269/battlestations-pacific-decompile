# Gun aim terms left as substitutions: the fighter aim distortion, the stick damping, the dead plane's engine fire, the barrel count

Addresses:
- `009FA7E0` and `009FA620`, the dogfight gun's aim wander;
- `009FAAD0` (BSP_BotTaskGun_Construct) and `009FCCA0`-`009FCE2D` (the fine aim in
  BSP_BotTaskGun_Tick `009FC7C0`);
- `009F9FC0` (BSP_BotTaskGun_SetAimSticks);
- `007BCAA0` (the plane's destroyed hook), `007CAF10`, `007DE5B0` and `007DE600`;
- `00959450` (BSP_Unit_OnDestroyed);
- `0072E71A` and `0072AB80`.

Packet `cc9_gun_aim_terms`, 2026-09-23. Ghidra was read, not written. All names are hypotheses.
Nothing here is ABI-compatible or game-validated.

## 1. The aim distortion `009FA7E0`

**Where it is applied.** In the fine aim (`docs/FIGHTER_GUN_LEAD.md` section 1), the call is
at `009FCCD7`, with ECX = dogfight-gun + 4h and the tick's `dt`. It runs only on a tick where
the fine aim runs, after the lead angles (x/d, y/d) are formed and before `009F9FC0`.
- **Fighter owner.** At `009FCCDC`-`009FCD01` the owner (`[[gun]+2F4h]`) is tested with
  `IsKindOf(13h)`, MPlaneFighter. When it is a fighter, the angles get (+20h / div, +24h / div).
  `div` is tuning `+648h` Pilot/Dogfight/FighterAimMulVersusAI (default 1.8) or `+64Ch`
  FighterAimMulVersusPlayer (1.2). `+64Ch` is chosen when the target (`gun+74h`) has a `+DF4h`
  record whose slot entry `[+DF4h + [00F876B8]*1Ch + 9Ch]` is below 1.0 (`009FCD03`-`009FCD34`).
- **Other owners** add (+20h, +24h) undivided (`009FCD7C`-`009FCD87`).

**What it is: a bounded random walk in a unit disc.** The object is at gun+4h.
`BSP_BotTaskGun_Construct` `009FAAD0` sets its fields: accel `+0h` = 0.5 (`00CE3800`), scale
`+4h` = 0.02 (`00CE9BAC`) and radius `+8h` = 0. Its init `009FA620` then does:
- `pos (+0Ch, +10h) = unit(U(0.1, 1), U(-1, 1))`: two `00BD2F10` stream-1 draws, low bound 0.1 at
  `00D7A2F0`, normalised by `00419260`;
- `vel (+14h, +18h) = accel * unit(U(-1, 1), U(-1, 1))`: two more draws.

Each step `009FA7E0(dt)` does the following, with two stream-1 draws:

```
h = dt * 0.5                                  00D7A280
v += accel * unit(U(-1,1), U(-1,1))
p += h * v
if |p| > 1:  v = -p                           00D7A24C; the step ends
elif |p| < radius:  p += 3.0 * h * v, then p clamped to the unit disc    00D7A2B0
out (+1Ch, +20h) = (scale * p.x * 0.8, scale * p.y)                      00CE3D40
```

- The radius is 0, so the second branch never runs.
- The output is at most 0.016 rad horizontally and 0.02 rad vertically. For a fighter it is
  divided by 1.8, so about 0.009 and 0.011 rad: a slow jitter of half a degree on the fine aim.
- The object's +1Ch/+20h are dogfight-gun +20h/+24h, the values the fine aim reads.

## 2. The damping (`gun+4Ah`) in `009F9FC0`

**The setter is the tick itself.** `009FCE2D MOV byte [gun+4Ah], AL` stores the tick's local
flag at `[ESP+13h]`.
- It is 1 when the fine aim ran this tick (`009FCDAD`).
- It is cleared at `009FCDE9` when the fire branch (`009FCDB6`-`009FCE01`) re-aims instead. That
  branch adds 0.25 (`00D7A348`) to one aim angle and calls `009F9FC0` again.
- `009FCE39` saves the angles to `gun+54h`/`+58h` for the next tick. `009FCDE5` is the other
  clear the earlier doc saw.

**What it does in `009F9FC0`.** This is `009FA0E0`-`009FA197`, traced from the opcode bytes. The
Ghidra listing prints `fmul st(1),st(0)` as `FMUL ST1`, which reverses the operands. A symbolic
walk over the Capstone listing (`local\x87cs.py`) gives the result below.
- **Without damping,** per axis the magnitude is `min(|8e| / (Spd^2 / Accel), 1)`, and the stick
  is `sign(e)` times it.
- **With `gun+4Ah` set,** each axis also takes

  ```
  rate = (prev - e) / dt        prev = gun+54h (yaw) / +58h (pitch), last tick's angle
  t    = e / rate               the time the error takes to close at its current rate
  if t > 0:  cap = 0.35 * t / (Spd / Accel)          0.35 = 00D04690
  magnitude = min(magnitude, cap)
  ```

- So when the error is closing, the stick eases off in proportion to the time left to close. This
  is a lead-compensating brake that stops the overshoot.
- A diverging or static error (t <= 0) is not capped.

## 3. `007BCAA0`'s tail

From `docs/PILOT_SURFACE_CLIMBOUT.md`, the plane's `vtable[7Ch]`. Besides the squadron removal it:
- draws `unit+C14h = 00BD2F10(ECX=0, [00E18718], [00E18714])`, DeadEngineFireDelay
  `{3.0, 5.0}` from planepartclasses.lua;
- stores the slot `unit+1B0h` at `+748h` unless it is 8;
- calls `00959450` BSP_Unit_OnDestroyed. That clears `+520h`, runs `00878990` and reports the kill
  through `009813A0`. The host's gunnery kill funnel already covers the kill report
  (`docs/UNIT_DAMAGE_AND_DEATH.md`).

**What `C14h` drives.** This is `007CAF10`'s first block, for a dead aircraft outside net mode 2
when `007D7A80` is false:
- when `C14h < 2 * C3Ch`, it calls `007DE600`, which calls each engine's `vtable[0Ch]`;
- when the class has engines (`desc+140h > 0`) and either `C14h < C3Ch`, or `C10h > 0` and
  `-C14h * 0.6 < C3Ch - C10h`, it sets `C14h = -1`;
- then, if no engine is burning yet (`007DE5B0` scans the engine list for a set `+8h` byte), it
  raises `"enginefire"` and calls `007DE600` again.

`"enginefire"` routes message 101, which lights a random engine (`007BA170`, index `% desc+7Ch`,
handled at `007D0D49` by `007EB0B0`). That arm returns before the death tail, so it changes
neither the explosion timer `C10h` nor the removal.

**Decision: land the read, no switch.** Engine fire is visual and message-level. The dead
aircraft's throttle is already zero in the power-lost and delayed modes. No row the host prints
would move.

## 4. The barrel count `0072E71A`

`docs/GUN_MOUNT_POSITIONS.md` section 5 has the rule. `gun+448h = 0072AB80(gunClass) =
max(1, count)`, where count is the gun class's muzzle-offset list at `class+9Ch..+A0h` (12-byte
entries). That list is filled from the model's `"fire"` node group, and the host has the rule as
`gun_muzzle_count_0072ab80`. The host flatten fills `barrel_num` with the number of `Bullet`
records instead.

**The host cannot bind this.** No host path loads a unit or device model's node tree. The
gunnery host builds guns from the Lua class rows alone. What it needs:
- the device class's model name, from its `Model` key;
- a reader for that model's node names (the model path `docs/MODEL_REACHES_UNIT.md` traces);
- a count of the nodes in the gun's `"fire"` group.

With that list, `barrel_num = gun_muzzle_count_0072ab80(n)`, and `gun_muzzle_local_offset_00730799`
already takes the offsets. **Landed as a read, no switch.**

## 5. What the dogfight worker calls

The units host applies these behind its existing `kFighterGunLeadBound`, in the dogfight hunks
owned by cc9-dogfight-engaged. This packet implements the accessors only.

```cpp
// include/bsp/game_hosts_gunnery.hpp, GameGunneryHost
void fighter_aim_distortion_009fa7e0(std::size_t unit_index, float dt,
                                     bool owner_is_fighter, float out[2]);
// include/bsp/gun_aim_terms.hpp
float gun_aim_damped_cap_009f9fc0(float e, float e_prev, float dt, float spd,
                                  float accel) noexcept;
```

**Application point, in `df_fine_aim_009f9fc0`'s caller (the fine-aim block):**
1. After forming `angles = (x/d, y/d)`, and only when the fine aim runs, call
   `gunnery->fighter_aim_distortion_009fa7e0(unit, dt, unit_is_kind_of(owner, 0x13), add)`.
   Then set `angles += add`.
2. In the stick law, per axis, when the previous tick's fine aim ran (the image's `gun+4Ah`), take
   `magnitude = min(magnitude, gun_aim_damped_cap_009f9fc0(e, e_prev, dt, Spd, Accel))`. `e_prev`
   is that axis's angle saved at the end of the last fine-aim tick (`gun+54h`/`+58h`).
3. Set the `gun+4Ah` flag to "the fine aim ran this tick", and clear it on the fire branch's
   re-aim.

**Substitutions:**
- The wander is created on the first call, not when the dogfight task is built.
- The FighterAimMulVersusPlayer arm is never taken: no human-held targets.
- The draws are on the shared stream by default, with key `Draw::aim_wander` = 10 under the
  option.

## 6. Pairs

No pair was run. No switch in this packet changes host behaviour:
- the two aim terms have no caller until the dogfight worker wires them (section 5);
- the engine fire and the barrel count are reads (sections 3 and 4).

A pair would compare identical binaries. The pair belongs to the dogfight worker's wiring. Its
predictions:
- fighter hits change little, since the jitter is about 0.5 degrees;
- fire-tick overshoot falls with the damping, which should raise fighter hits;
- each fighter uses two more shared-stream draws per fine-aim tick, so option-off rows shift
  through the coupling.

## 7. The wiring (packet `cc9_fighter_aim_wiring`)

This is applied in the units host's fine aim, `df_fine_aim_009f9fc0` and its call in
`df_gun_tick_009fc7c0`, behind `kFighterGunLeadBound`. The new sub-switch is
`kFighterAimTermsBound`.
- **Distortion.** On a fine-aim tick, after `(e_h, e_v) = (x/d, y/d)`, the fine aim calls
  `fighter_aim_distortion_009fa7e0(unit, dt, IsKindOf(13h), add)` and adds `add` to the angles.
- **Damping.** When the previous tick's fine aim ran (`+4Ah`), each axis takes
  `min(magnitude, gun_aim_damped_cap_009f9fc0(e, prev, dt, Spd, Accel))`.
- **Saved angles.** When the fine aim runs, the angles it used are saved as `prev`, as at
  `009FCE39`/`009FCE44`. The flag is then "the fine aim ran this tick" (`009FCE2D`).
- **The fire branch's re-aim.** `009FCDB6`-`009FCE01` adds +0.25, calls `009F9FC0` a second
  time and clears the flag at `009FCDE9`. It runs only when `007B96D0` BSP_Unit_TargetFinderBusy
  answers true (`unit+C50h` non-null and `007DEDB0` true). The host's gun tick passes
  `finder_busy = false` (`DogfightGunInputs`), so that branch never runs here. The flag therefore
  follows the image exactly under that existing substitution. It is not a new one.
- **Census.** The `fighter gun lead` line now also prints `distortion_ticks` and `damped_ticks`.

### Predictions, written before the pair

E2 9000, RNG option on both sides, `kFighterAimTermsBound` off against on, same tree
(main `7d4095eaa` plus this wiring).
- **distortion_ticks** equals **fine_aim_ticks** for every fighter. **damped_ticks** is a large
  share of them, because the fine aim runs on consecutive ticks while a target stays in the cone.
- **Fighter hits and bursts:** within ±25% of the control. V1b had 50 hits and 16 bursts.
  - The damping trims the stick while the error closes, which should reduce the overshoot and
    raise the time on target.
  - The distortion adds a jitter of about half a degree that works the other way.
  - Direction: hits up slightly.
- **Trigger ticks:** within ±15%.
- **Val deaths to fighters:** unchanged, or plus or minus 1.
- **Kate and Lexington rows:** flat. The fighters' new draws are on their own keys under the
  option, and Kates are not fighter targets before their runs. Any Kate or Lexington movement
  would come from a changed Val kill.

### The pair

`localC_9000.log` (sub-switch off) against `localT_9000.log` (on). E2 9000, RNG option on
both sides, same tree.

| quantity | off | on |
| --- | --- | --- |
| fine-aim ticks, all fighters | 87 | 315 |
| distortion / damped ticks | 0 / 0 | 315 / 311 |
| fighter bursts | 5 | 10 |
| fighter fire ticks | 147 | 276 |
| category 0 (fighter) shots / hits | 875 / 32 | 1646 / 71 |
| Val deaths to fighters | 2 (Val #3.1, #3.1\|.-3) | 2 (Val #3.1\|.-2, #3.1\|.-4) |
| Kates shot down by fighters | 0 | 2 (Kate #8.1\|.-2, #8.1\|.-3, by Lexington-class01_sqn01) |
| category 10 hits / Yorktown damage taken | 8 / 1883 | 4 / 982 |
| deaths / total damage | 34 / 9216.2 | 33 / 8242.4 |

**Against the predictions:**
- distortion_ticks equals fine_aim_ticks for every fighter. damped_ticks is one less per
  engagement, because the first fine-aim tick of an engagement has no previous tick. **Held.**
- The direction held for fighter hits and bursts, but **the magnitude did not.** Hits rose 122%
  and bursts doubled, against the ±25% predicted. Fire ticks rose 88%, against ±15%.
- Val deaths to fighters stayed at 2, but they are different Vals. **Held** within ±1.
- **The Kate rows are not flat, and that prediction failed.** Lexington-class01_sqn01's leader
  went from 0 fine-aim ticks to 146 and shot down two Kates of #8.1. So one torpedo fewer
  reaches Yorktown.

**The read explains the size.** The fine aim runs only while the lead error sits inside the
strafe cone (`009FCCAB`). The damped cap slows the stick as the error closes, so the aircraft
stops overshooting out of the cone. The fine aim then keeps running, 87 to 315 ticks. Firing
follows the fine aim, so bursts, fire ticks and hits follow it. The distortion is only half a
degree and does not offset that. The one host term that could inflate the effect,
`finder_busy = false`, predates this packet and is the same on both sides.

**Decision:** `kFighterAimTermsBound` lands ON.

## 8. Barrel count: what the host can reach

- **The node table is readable.** `src/native_resource_hierarchy_parser.cpp` reconstructs
  `00B7F100` (Hierarchy) and `00B7EB90` (Item). `include/bsp/structured_hierarchy.hpp` carries
  each node's `parent`, `name`, `resources`, matrix and bounds. `src/installed_model_probe` and
  `src/structured_resource_probe.cpp` already open installed `.mmod` files through the host's
  structured reader. `MMOD_FORMAT_NOTES.md` in this installation describes the same Hierarchy
  and Item chunks.
- **The device has no model.** `deviceclasses.lua` in this installation has no `Model` or `Mesh`
  key, so a gun's model cannot be reached from its device row.
- **The producer queries a group, not the node list.** `007325A0` builds the string `"fire"` and
  asks "the model" through `00718870`, flags 0 and 1. It then copies that group's element list,
  12-byte points (`docs/GUN_MOUNT_POSITIONS.md` section 4).
- **What is missing:**
  1. which model `007325A0` queries, which is the class loader's model source;
  2. `00718870`, the named-group lookup, and the layout of a group's element list. This is
     probably one of the structured resource payloads next to `GroupParams`, but it is unread.

  With those two reads, the count is `gun_muzzle_count_0072ab80(elements)` and a flatten field.
  **Not reachable with the current loaders. No code.**
