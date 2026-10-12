# CC13 unlabelled draws batch (packet cc13-draws1)

Addresses: 009C859E, 009C8515, 009C8531, 009C855B, 009C8587, 009C85A3, 009C85AA, 009C6CAE, 009C6D1A, 009C6D31, 009C6D3B, 009C628C, 009C7800, 009C7839, 009C579E, 009C5786, 009C6114, 0099C765, 00999912, 00999936, 0099B740, 00BD2F10, 00BF7420

Status: **exported and reconstructed; build-tested with every switch OFF** (the default). It is not
pair-tested or game-validated. The deliverable is the unified diff in section 6, written for cc9 to
apply to `src/game_hosts_units.cpp`. It was applied in this tree, built with `./scripts/build.ps1`,
reverted, and the copy in this doc passes `git apply --check` against origin/main
`50fb69620` and `011ac8e97` (the file is identical in both). Nothing in cc9's files is changed on this branch.

Source rows: `docs/CC13_RNG_STREAM_CENSUS.md` sections 10.2 and 11.2/11.3. All image reads below are
disk-byte listings (`bsp.py disasm-raw`). Constants were read from the PE on disk.

## 1. Conventions

- Every change has its own `static constexpr bool k...Bound = false;`, declared in one block after
  `kAttackDistDrawBound`. With all six OFF the host behaves exactly as before. Because these bodies are
  `if constexpr` in non-template code, the OFF build still compiles them.
- Every draw goes through `GameUnitsHost::Impl::release_altitude_draw_00bd2f10` with a per-site key
  (`<unit>#db_side`, `#db_roll`, `#db_ag1c`, `#db_ad1c`). This is the units host's own LCG, the
  same one `st_draw`/`lb_draw` use. With `BSP_GUNNERY_RNG_STREAMS=1` each key is its own stream;
  unset, all units-host draws share one sequence. Nothing draws from stream 0 or from the gunnery
  host's generator. In the image all four draws are on stream 1 (`ECX = 1`).
- `00BD2F10(low, high)` takes `[ESP] = low`, `[ESP+4] = high` and `ECX` = the stream, and returns
  ST0. The host helper returns `low + (high - low) * u`. That matches the distribution; the
  sequence is not the image's.

## 2. 009C859E: the side of the roll into the dive

### Image (009C8515-009C85C9, BSP_BotTaskDiveBomb_UpdateStateTransition, flyabove -> turndown arm)

```
009c8515  MOV EAX,[ESI+798h]           ; flyabove+20h, the latched side
009c851d  JNE 009C85B8                 ; non-zero: use it
009c8529  MOVSS xmm0,[[ESI+3FCh]+C68h] ; the unit's bank
009c8531  COMISS xmm0,[00CE398C]       ; +0.3490659 (20 deg)
009c8538  JBE ...                      ; bank > +20 deg -> side +1
009c855b  MOVSS xmm1,[00D1FBC0]        ; -0.3490659
009c8563  COMISS xmm1,xmm0 / JBE ...   ; -20 deg > bank -> side -1
009c8587  FLD [00D05B54] (1024.0) ; [ESP+4] = 1024.0, FLDZ -> [ESP] = 0.0, ECX = 1
009c859e  CALL 00BD2F10                ; U(0, 1024) on stream 1
009c85a3  CALL 00BF7420                ; _ftol2: truncate (CVTTSD2SI on this runtime)
009c85aa  CMP EAX,200h / SETGE DL / LEA EDX,[EDX+EDX-1]   ; >= 512 -> +1, else -1
009c85c1  CALL 009C7800 (sign)         ; turndown's SetRollDirection, which draws 009C7839
```

An unordered bank (NaN) fails both `JBE`s and takes the coin; the reconstruction's `!(>)`/`!(<)`
does the same.

**The +798h source.** `+798h` is flyabove `+20h`, with the fly-over state at task+778h (its
`+18h/+19h/+1Ah` are the transition's `+790h/+791h/+792h`). There are three writers:
- `009C61A9`, the constructor, writes 0;
- `009C628C`, the enter, writes 0;
- `009C6D31`/`009C6D3B` in the fly-over tick write -1/+1.

There is no other `98 07 00 00` displacement in 009C. The tick arm is `009C6CAE`-`009C6D3B`, reached
only past `009C6B75` (`|a0| > float(0.05)`, qword `00D7A270`). Here `a0 = [ESP+68h]` and
`b0 = [ESP+74h]` are the first components of 007F0280's two outputs at `009C6B3A`. Its gate is
`+19h != 0`, `+18h != 0` (BL from `009C6B7F`) and `+20h == 0`. Then:
- `d = float(|a0| - b0)` (`009C6CEC`-`009C6CF4`);
- `|d|` is formed as `d > 0 ? d : -0.0 - d` (`00D7A208` is `-0.0`);
- if `|d| > 0.01` (`00D7A238`, `COMISS`/`JBE`), `+19h = 0` (`009C6D23`);
- otherwise `+20h = a0 < 0 ? +1 : -1` (`009C6D2C` `JB`).

This matches `docs/CC13_AUDIT_PLANE_BOT_STATES_3.md` item 18 and `docs/DIVE_BOMB_TASK.md` near
line 5076.

### Host before

`dive_bomb_transition_inputs` passes `flyabove_turn_side_798 = 0`, both thresholds `0.0` and
`random_turn_side = 1`. So the side is the sign of the bank, and +1 at a bank of exactly 0.

### Diff (three switches)

- `kDiveTurnSideLatchBound`:
  - a new slot field `db_flyabove_side_20`, cleared on the fly-over enter (`009C628C`);
  - latched in the fly-over tick next to the existing 007F0280 call (the `kNearFieldProbeBound`
    block, which already has `pr` and `b18`), by the rule above;
  - fed to `flyabove_turn_side_798`.

  **Not modelled:** the `+19h = 0` half (`009C6D23`). This host derives `+19h` in the state feed
  each think, so a clear in the tick would not persist, and audit9's fly-over diff owns that feed.
  With the latch on and the clear off, a non-converged probe tick can still turn down where the
  image would hold one more think. That is the existing `+19h` difference; this switch does not
  add it.
- `kDiveTurnSideBankBound`: `+-0.3490658700466156f` in place of the two `0.0f`.
- `kDiveTurnSideCoinBound`: the coin is drawn **lazily** in the binding's `write_turn_direction`.
  `read_transition_inputs` keeps this think's inputs in `transition_in_`. When 009C7800 runs with
  `+798h == 0` and the bank inside both thresholds (exactly the coin arm of
  `dive_bomb_next_state_009c83e0`), the binding draws `U(0, 1024)` on `#db_side`. If
  `trunc < 0x200`, it negates the roll. The placeholder side is +1 and 009C7800 multiplies the
  magnitude by `+-1.0` (`00D7A24C`/`00D7A260`), so the negation is exact.

  Draw count: one per coin-arm turndown, as in the image. No draw happens on ticks where the
  transition takes another edge. Order: the image draws the coin before the magnitude; here the
  magnitude is drawn first, because it is an argument of `write_turn_direction`. With keyed
  streams the two keys are independent, so only the unkeyed sequence sees the swap.

## 3. 009C7839: the turndown roll magnitude

**Image.** In 009C7800-009C7846 the side is `[00D7A24C] = 1.0` if `sign >= 0`, else
`[00D7A260] = -1.0`. Then `00BD2F10(0.8 at 00CE74F8, FLD1)` on `ECX = 1`, `FMUL` by the side, and
`FSTP` to turndown `+18h`.

**Callers of `uniform_between`.** Fixing it in place is right:
- `git grep uniform_between` finds one declaration (`include/bsp/dive_bomb_task.hpp:1543`), one
  caller (`src/dive_bomb_task.cpp:1255`, this roll) and one implementer (the units host's
  `DiveBombArmBinding`).
- Nothing else depends on it returning `low`.
- `reports/dive_bomb_task.json` lists 009C6114, 009C579E, 009C859E and 009C85A3 against
  `uniform_between` too, but those were never wired through it.

**Diff.** `kDiveTurnRollDrawBound`: `uniform_between` draws on `#db_roll`. Its comment says that a
second caller needs its own key. A site-local draw would need an interface change in
`dive_bomb_task.cpp`, and it would buy nothing while there is one caller.

**Nit for the report.** `reports/dive_bomb_task.json` calls `00BF7420` "LIBCRT rand". It is the
`_ftol2` truncation (ledger `LIBCRT_unmatched_00bf7420`, `CVTTSD2SI`). The coin is one draw, not
two.

## 4. 009C579E aimglide re-arm, and 009C6114 aimdive re-arm

**Image, aimglide.**
- `009C5786` loads 0.5 (`00CE3800`) into `[ESP+4]` and 0.2 (`00CE54A0`) into `[ESP]`, with
  `ECX = EBX = 1`.
- `009C579E` calls `00BD2F10`; `009C57A6` stores `+1Ch`.
- `009C5786` is the fall-through of the salvo loop. It runs whenever the `009C5755` `JBE` passes,
  whether or not a round was released. In `dive_bomb_aimglide_release_009c5777` that is the
  `gate_reached == 6` path (`out.rearm_timer_1c = in.rearm_draw`).

**Image, aimdive.**
- `009C60F6` FLD1 goes to `[ESP+4]`, 0.5 (`00CE3800`) to `[ESP]`, with `ECX = 1`.
- `009C6114` draws; `009C6119` stores `+1Ch`.
- This happens only inside the release block that begins at `009C60F1`.

**Host before.** `in.rearm_draw` is `kAimGlideRearmLow` (0.2, unlabelled) and `kAimDiveRearmLow`
(0.5, labelled).

**Diff.** The draws are lazy, in the binding's `apply_aimglide_result`/`apply_aimdive_result`:
- `kAimGlideRearmDrawBound` overwrites `db_aim_rearm_1c` with `U(0.2, 0.5)` on `#db_ag1c` when
  `r.gate_reached == 6`;
- `kAimDiveRearmDrawBound` overwrites it with `U(0.5, 1.0)` on `#db_ad1c` when `r.released`.

Drawing in the input builders would draw every tick. The aimdive pin was not on the packet list. It
is included because it has the same shape, takes three lines and has its own switch.

## 5. 0099C765 bot-task base phase: left out

**What it feeds.** The constructor 0099C6F0 writes `+304h = 1.0` (`0099C75D`, `00D7A24C`) and
`+308h = -U(0, 1)` (`0099C765` on `ECX = 1`, `FCHS`, `0099C76C`). The only readers are the load
variant 0099C8B0 and `BSP_PilotBot_Update` (`00999912`-`00999936`, `009999B5`-`009999BB`), which
is a 1 Hz countdown:

```
if (dt >= t) { t = t + period(+304h) - dt;  CALL 0099B740 }   ; then the per-tick vtable[64h]
else         { t = t - dt }                                    ; per-tick vtable[64h] only
```

So the phase only staggers when 0099B740 (BSP_BotTask_AbandonIfStale) runs within each second. The
first update after construction always runs it, because `t <= 0`. For the dive bomb, 0099B740 is an
idempotent `squadron+370h = 1` for the flight leader.

**Why it is left out.** The host calls 0099B740 every tick at four sites:
`run_dive_bomb_attack_mode_tick_0099b740`, the level-bomb, strafe and torpedo copies, at about
`game_hosts_units.cpp` 18977, 21621, 22288 and 25422 before this diff. A phase draw does nothing
until that 1 Hz cadence is modelled. The cadence is a structural packet across four task hosts, not
a draw fix. Its only gameplay effect would be how long a Lua-raised `+370h = 2` (007ED430) survives
before the next think resets it to 1: up to one second in the image, one tick in the host. If cc9
wants it, model the countdown first and then key the phase as `<unit>#task308`. Per-construction
draw count matters only for the unkeyed sequence.

## 6. The diff

Base: origin/main `50fb69620` (rechecked on `011ac8e97`), `src/game_hosts_units.cpp` only, 11 hunks, all switches OFF.

```diff
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..bcf500c5e 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -1237,6 +1237,7 @@ struct GameUnitSlot {
     float db_glide_bearing_max{0.0f};     // the census arm A needs either way
     int db_aimglide_pull_outs{0};
     bool db_flyabove_bank_latch_1c{false};
+    int db_flyabove_side_20{0};   // flyabove+20h = task+798h, 0 / +1 / -1
     float db_flyabove_dead_band_t{0.0f};
     float db_flyabove_along_track{0.0f};
     float db_flyabove_cross_track{0.0f};
@@ -4534,6 +4535,9 @@ struct GameUnitsHost::Impl {
             }
         }
         in.flyabove_turn_side_798 = 0;
+        if constexpr (kDiveTurnSideLatchBound) {
+            in.flyabove_turn_side_798 = slot.db_flyabove_side_20;  // 009C8515
+        }
         in.aimdive_alive_74d = slot.db_aim_alive_19;
         in.aimdive_pull_out_74c = slot.db_aim_pull_out_18;
         // 009C86B2/009C86BF. Packet cc8_dive_flyover, the ONE line the
@@ -4582,6 +4586,12 @@ struct GameUnitsHost::Impl {
         in.unit_bank_c68 = slot.plane_bank_angle_c68;
         in.bank_high_00ce398c = 0.0f;
         in.bank_low_00d1fbc0 = 0.0f;
+        if constexpr (kDiveTurnSideBankBound) {
+            in.bank_high_00ce398c = 0.3490658700466156f;   // 00CE398C, 20 deg
+            in.bank_low_00d1fbc0 = -0.3490658700466156f;   // 00D1FBC0
+        }
+        // The coin is drawn lazily in write_turn_direction (kDiveTurnSideCoinBound),
+        // so it is drawn only when 009C7800 runs on the coin arm, as in the image.
         in.random_turn_side = 1;
         return in;
     }
@@ -6240,6 +6250,21 @@ struct GameUnitsHost::Impl {
     // Packet cc9_fighter_gunfire_rate: approach+B4h = uniform(0.6, 0.8) * class+268h
     // (009C3F63-009C3F97), pinned at 0.6 before. OFF: unmeasured (runs blocked).
     static constexpr bool kAttackDistDrawBound = true;
+    // Packet cc13-draws1 (docs/CC13_UNLABELLED_DRAWS_BATCH.md). Each is OFF until
+    // its pair runs. The draws use release_altitude_draw_00bd2f10 with a per-site
+    // key, never stream 0; the image draws all four on stream 1 (ECX = 1).
+    // 009C6CAE-009C6D3B: the fly-over latches its roll side +20h (task+798h).
+    static constexpr bool kDiveTurnSideLatchBound = false;
+    // 009C8531/009C855B: the bank thresholds +-20 deg (00CE398C, 00D1FBC0).
+    static constexpr bool kDiveTurnSideBankBound = false;
+    // 009C8587-009C85B6: the coin trunc(U(0, 1024)) >= 512 -> +1, else -1.
+    static constexpr bool kDiveTurnSideCoinBound = false;
+    // 009C7839: the turndown roll magnitude U(0.8, 1.0), pinned at 0.8 before.
+    static constexpr bool kDiveTurnRollDrawBound = false;
+    // 009C579E: the aimglide re-arm U(0.2, 0.5), pinned at 0.2 before.
+    static constexpr bool kAimGlideRearmDrawBound = false;
+    // 009C6114: the aimdive re-arm U(0.5, 1.0), pinned at 0.5 before.
+    static constexpr bool kAimDiveRearmDrawBound = false;
     // Packet cc9_fighter_gun_lead (docs/FIGHTER_GUN_LEAD.md): the gun's lead
     // point +5Ch from 009FC931-009FC9F6 (two passes of the target's vtable[48h]
     // 00954650, both arms, t = d / 007C2610), the envelope against it, and the
@@ -18860,12 +18885,33 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         }
                         bsp::DiveBombTransitionInputs read_transition_inputs(
                             void*) override {
-                            return owner_.dive_bomb_transition_inputs(slot_);
+                            transition_in_ = owner_.dive_bomb_transition_inputs(slot_);
+                            return transition_in_;
                         }
                         void set_dive_bomb_state(void*, bsp::DiveBombState) override {
                             ++slot_.dive_bomb_transitions;
                         }
                         void write_turn_direction(void*, float roll) override {
+                            if constexpr (GameUnitsHost::Impl::kDiveTurnSideCoinBound) {
+                                // 009C8587-009C85B6: with +798h 0 and the bank inside
+                                // both thresholds, the side is the coin: 00BD2F10(0.0,
+                                // 1024.0 at 00D05B54) on ECX = 1, 00BF7420 truncates,
+                                // 009C85AA CMP 200h / SETGE gives +1, else -1. The
+                                // host side was the placeholder +1, so the roll is
+                                // magnitude * +1 and -1 flips it exactly. The image
+                                // draws the coin before 009C7839's magnitude; the
+                                // keys are separate, so only the unkeyed sequence
+                                // (BSP_GUNNERY_RNG_STREAMS unset) sees the order swap.
+                                const bsp::DiveBombTransitionInputs& t = transition_in_;
+                                if (t.flyabove_turn_side_798 == 0 &&
+                                    !(t.unit_bank_c68 > t.bank_high_00ce398c) &&
+                                    !(t.unit_bank_c68 < t.bank_low_00d1fbc0)) {
+                                    const float coin = owner_.release_altitude_draw_00bd2f10(
+                                        slot_.row.name + "#db_side", 0.0f,
+                                        bsp::dive_bomb_constant::kTurnCoinRange);
+                                    if (static_cast<int>(coin) < 0x200) roll = -roll;
+                                }
+                            }
                             slot_.db_turn_roll_18 = roll;  // 009C7800
                         }
                         bsp::DiveBombAimDiveReleaseInputs read_aimdive_inputs(
@@ -18876,6 +18922,14 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         void apply_aimdive_result(
                             void*, const bsp::DiveBombAimDiveReleaseResult& r) override {
                             slot_.db_aim_rearm_1c = r.rearm_timer_1c;
+                            if constexpr (GameUnitsHost::Impl::kAimDiveRearmDrawBound) {
+                                // 009C6114 on ECX = 1: U(0.5 at 00CE3800, FLD1),
+                                // drawn only on a release, stored by 009C6119.
+                                if (r.released) {
+                                    slot_.db_aim_rearm_1c = owner_.release_altitude_draw_00bd2f10(
+                                        slot_.row.name + "#db_ad1c", 0.5f, 1.0f);
+                                }
+                            }
                             slot_.db_aim_pull_out_18 = r.pull_out;
                             if (r.released) owner_.note_dive_bomb_release(slot_);
                         }
@@ -18886,6 +18940,16 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         void apply_aimglide_result(
                             void*, const bsp::DiveBombAimGlideReleaseResult& r) override {
                             slot_.db_aim_rearm_1c = r.rearm_timer_1c;
+                            if constexpr (GameUnitsHost::Impl::kAimGlideRearmDrawBound) {
+                                // 009C579E on ECX = EBX = 1: U(0.2 at 00CE54A0, 0.5 at
+                                // 00CE3800), stored by 009C57A6. 009C5786 is reached
+                                // whenever the 009C5755 gate passes, salvo or not,
+                                // which is gate 6 in the reconstruction.
+                                if (r.gate_reached == 6) {
+                                    slot_.db_aim_rearm_1c = owner_.release_altitude_draw_00bd2f10(
+                                        slot_.row.name + "#db_ag1c", 0.2f, 0.5f);
+                                }
+                            }
                             slot_.db_glide_travel_20 = r.travel_accumulator_20;
                             // Packet cc8_dive_glide's census: which gate of
                             // 009C5689-009C5755 stopped this call, and the lead
@@ -18916,7 +18980,17 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                             // that list, so the count comes from there.
                             return owner_.dive_bomb_rounds_remaining(slot_);
                         }
-                        float uniform_between(float low, float) override { return low; }
+                        float uniform_between(float low, float high) override {
+                            // The one caller is the turndown roll magnitude
+                            // (dive_bomb_task.cpp, 009C7839, U(0.8, 1.0) on ECX = 1).
+                            // A second caller needs its own key.
+                            if constexpr (GameUnitsHost::Impl::kDiveTurnRollDrawBound) {
+                                return owner_.release_altitude_draw_00bd2f10(
+                                    slot_.row.name + "#db_roll", low, high);
+                            }
+                            (void)high;
+                            return low;
+                        }
                         bool approach_has_bomb_ordnance(void*) override {
                             return slot_.db_has_bomb_d1;
                         }
@@ -18949,6 +19023,7 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
 
                         GameUnitsHost::Impl& owner_;
                         GameUnitSlot& slot_;
+                        bsp::DiveBombTransitionInputs transition_in_{};  // this think's
                     };
 
                     // 0099A170 builds a kind 8 task for an ordered aircraft
@@ -22745,6 +22820,7 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 // are recomputed every tick by the state feed.
                                 unit_.db_flyabove_bank_latch_1c = false;
                                 unit_.db_flyabove_latch_tick = -1;
+                                unit_.db_flyabove_side_20 = 0;   // 009C628C
                                 if constexpr (kAimErrorDrawBound) {
                                     // 009C6276: every fly-over enter re-draws.
                                     unit_.db_aim_time_error_c8 = owner_.redraw_aim_error(
@@ -23887,6 +23963,26 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 const float w[3] = {30.0f, 0.0f, 0.0f};  // 00CE38C8
                                 const bsp::NearFieldProbeResult pr = nf_probe_007f0280(ext, w);
                                 slot_entry_108 = bsp::near_field_flyover_slot_009c6b75(pr, b18);
+                                if constexpr (GameUnitsHost::Impl::kDiveTurnSideLatchBound) {
+                                    // 009C6CAE-009C6D3B, behind 009C6B75's
+                                    // |a0| > float(0.05) (qword 00D7A270): with
+                                    // +19h, +18h set and +20h still 0, a probe
+                                    // whose |a0| is within 0.01 (00D7A238) of b0
+                                    // latches +20h = a0 < 0 ? +1 : -1. The arm's
+                                    // other half (009C6D23, +19h = 0 when not
+                                    // within) is not modelled: this host derives
+                                    // +19h in the state feed every think.
+                                    const float a0 = pr.out_a[0];
+                                    const float abs_a0 = std::fabs(a0);
+                                    if (static_cast<double>(abs_a0) > static_cast<double>(0.05f) &&
+                                        unit_.db_flyabove_ready_19 && b18 &&
+                                        unit_.db_flyabove_side_20 == 0) {
+                                        const float d = abs_a0 - pr.out_b[0];   // 009C6CF0
+                                        if (!(std::fabs(d) > 0.01f)) {          // 009C6D1A JBE
+                                            unit_.db_flyabove_side_20 = a0 < 0.0f ? 1 : -1;
+                                        }
+                                    }
+                                }
                                 if (slot_entry_108 != 0.0f) {
                                     ++unit_.nf_flyover_slot_writes;
                                     if (slot_entry_108 < unit_.nf_flyover_slot_min) unit_.nf_flyover_slot_min = slot_entry_108;
```

## 7. Deliberately left out

- `0099C765`: section 5.
- `009C6D23`, the `+19h = 0` half of the latch arm: section 2. It is a deterministic gate, not a
  draw, and it belongs to the fly-over feed that audit9's diff edits.
- `0070C247`, the flak FlyTime-expiry burst: census 10.7 item 1. It changes AA lethality and is
  cc9's call, not a draw pin.
- `00900830`/`0090083E`, the torpedo AngleErr cadence: drawn per launch, not per aim pass. Only the
  last value before launch reaches the torpedo, so the distribution per shot is already the
  image's.
- The other NOT-MODELLED rows in census 11.2 are not in this packet's list:
  - task-state constructors;
  - `008FBF5C`;
  - `00856132`;
  - `007AB94A`;
  - the presentation reclassifications.

## 8. Applying and pairing

1. `git apply` the block in section 6 on top of current main. Run `git apply --3way` if
   cc9-gunnery36's edits have shifted the context.
2. Build. Nothing changes while all six switches are OFF.
3. Suggested pairs, one switch at a time, on a dive-bomb mission (USN04, JM05):
   - `kDiveTurnRollDrawBound` and `kAimGlideRearmDrawBound`/`kAimDiveRearmDrawBound` are
     distribution-only;
   - `kDiveTurnSideBankBound` with `kDiveTurnSideCoinBound` changes the turndown side for banks
     inside +-20 deg;
   - `kDiveTurnSideLatchBound` needs `kNearFieldProbeBound` (ON) and only fires when the probe sees
     a unit.
