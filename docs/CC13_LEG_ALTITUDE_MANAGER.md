# The converging-leg altitude manager (packet cc13-legmgr1)

Reconstruct-only. The functions are `009FC000`, `009FC1C0` and `009FC260`, with the lock getter
`009FB010`. Code is in `include/bsp/leg_altitude_manager_cc13.hpp` and
`src/leg_altitude_manager_cc13.cpp`. The call rows are in `reports/cc13_leg_altitude_manager_calls.json`,
which `verify_report_calls` checks: 29 rows, 0 failed.

Status:
- **Exported:** yes.
- **Reconstructed:** yes.
- **Build-tested:** yes.
- **Fixture-tested:** no.
- **Game-validated:** no.
- **Wired into a host:** no. Section 6 has a suggested glue diff behind `kLegAltitudeManagerBound`, committed OFF.

All names are hypotheses, not recovered symbols.

## 1. Short answers

- **Data structure.** There is one process-wide registry, not one per path. `009BD400`
  `BSP_BotPathLegManager_Get` builds it lazily: `operator_new(10h)`, then constructor `009FAA40`, which sets vtable
  `00D21C80` and zeroes `+4h/+8h/+0Ch`.
  - `+4h` is a heap array of 10h-byte records `{unit*, x, y, z}`.
  - `+8h` is the record count.
  - `+0Ch` is the capacity. The array grows to `2*cap+2` (`009FC0EA`).
  - Each of the three members takes the critical section at `+4h` of a second lazy singleton, `009FB010`
    `BSP_BotPathLegLock_Get` at `[00F8A724]`. That singleton is 8 bytes, built by constructor `009FA9F0`
    (vtable `00D21C7C`, `+4h` = `BSP_CriticalSection_Create`). The members increment and decrement the
    depth counter at section`+18h`.
  - `009FB010` has no callers outside these three members. The thunk `009FB610` (`JMP 009FB010`) has
    no rel32 or absolute references.
- **Users.** These come from a rel32 + absolute-dword scan of the image file (`tools/image_ref_scan.py`).
  Ghidra's xrefs agree with the scan.
  - `009FC000` and `009FC1C0` have one caller each: `009FD050` (at `009FD07F` and `009FD0C5`).
  - `009FC260` has two callers:
    - `009BD6CD` in the moveonpath approach update `009BD4D0`;
    - `009BECBA` in the moveto refresh `009BEBA0`.
  - `009FD050` is called by:
    - moveonpath: `009BD52F` and `009BD642` (in `009BD4D0`), and `009BD138` (slot `+58h` `009BD120`);
    - moveto: `009BEC85` (in `009BEBA0`), and `009C32B8` (slot `+58h` `009C32A0`, in vtables `00D20B68`
      and `00D20BE0`). Ghidra has no function at `009C32A0`.
  - `009BEBA0` is called from `009C1C30`, `009C1DC0`, `009C3570` (the moveto approach update, at
    `009C358F`) and `009C3650` (the cruise profile, slot `+54h`).
  - So three plane tasks use the manager: **moveonpath** (kind 6), **moveto** (kind 7, vtable `00D20B68`)
    and its **attackmove** variant (vtable `00D20BE0`).
  - No landing approach and no squadron director calls any of these functions, directly or through
    `009FD050`.
  - **Moveto has no PATH_FM_SIMPLE gate.** Within 3000 m of its target point, every moveto refresh
    registers and asks for the altitude.
- **Separation law** (section 3). Every other registered plane whose x/z lies within the sum of the two
  turn radii contributes its stored height `b`. Each such height defines an exclusion band
  `(b - s, b + s)`, where `s = CircleAltDiff * 0.5` (15 m in this installation).
  - Starting from the steer height `y`, `down` is pushed to the bottom of any band containing it, and
    `up` to the top. This repeats until neither moves.
  - The image then takes `down`. The image's test is `(up - down) > (y - down)`, which reduces to
    `up > y`. Any band that contains `y` moves both `down` and `up` in the same step, so `up > y`
    whenever anything moved, and the `up` branch (`up` taken if it is below `Dynamics/Ceiling`) is
    effectively unreachable.
  - The result is floored at `clamp(y, -20, 120)`. A plane whose steer height is at or below 120 m is
    therefore never moved. A higher plane is moved down to no lower than 120 m.
  - The caller gets `y + w * (alt - y)`, where `w` is `009FD050`'s weight: 1.0 inside 1200 m, falling
    linearly to 0 at 3000 m.
  - The plane's own record stores `{x, alt, z}` (the unblended `alt`), so later planes separate
    against it.
- **Registration and release.**
  - `009FD050(d, point)` registers inside `0 <= d < 3000` and releases otherwise.
    - Registration ignores a unit that already has a record. The first point stays until `009FC260`
      overwrites it.
  - The tasks' slot `+58h` passes `d = -1` to release.
  - `009FC000` and `009FC1C0` also prune every null record and every record whose unit has `unit+5Dh`
    set (the out-of-action byte). `009FC260` prunes only null records.
- **RNG.** None. No function in the manager draws from `00BD2F10`. No `#lg` key is needed.
- **JM06.** For a plane with no other registered plane within the radius sum, `009FC260` returns `y`
  exactly (or `-20 * w + (1-w) * y` for `y < -20`). A lone Catalina on its last 3000 m is therefore not
  moved by the manager. Its altitude changes only if another moveonpath, moveto or attackmove plane is
  registered close by. Whether JM06 has such a plane in the image is not established here (open
  question 1).

## 2. Layout

| Offset | Meaning | Evidence |
|---|---|---|
| mgr+0h | vtable `00D21C80` | `009FAA44` |
| mgr+4h | record array | `009FC04E`, `009FC31C` |
| mgr+8h | count | `009FC0E1`, `009FC378` `ADD [ESI+8],-1` |
| mgr+0Ch | capacity | `009FC0E1`-`009FC0EA` |
| rec+0h | unit pointer (0 = null) | `009FC063`, `009FC33E` |
| rec+4h/+8h/+0Ch | x, y, z | `009FC394`-`009FC3A0`, `009FC3C6`, `009FC749`-`009FC764` |
| unit+5Dh | out-of-action byte | `009FC06D`, `009FC1FD` |
| [unit+538h]+268h | TurnCircleRadius | `009FC2DF`, `009FC384` |
| tuning+378h | `Pilot/MoveTo/CircleAltDiff` (30 in this installation) | `009FC3F9` |
| tuning+210h | `Dynamics/Ceiling` (1500 in this installation) | `009FC6AD` |

The installation's values come from `scripts/datatables/planeglobals.lua` (mtime 2024-10-29; the install
is modded). Its comment on `CircleAltDiff` (Hungarian) says this is the height difference kept between
squads circling near each other, so that they do not collide.

## 3. The functions

### `009FC000` BSP_BotPathLegManager_Register

ABI: `__thiscall(manager, unit*, const float* point)`, `RET 8`. It has an SEH frame.

1. Lock.
2. Scan the records:
   - a null or out-of-action record is swap-removed (the last record is copied over it, the count drops,
     and the same slot is re-read);
   - the unit's own record jumps to the unlock (`009FC077`), leaving the stored point unchanged.
3. Append `{unit, point}` unless the unit is null (`009FC0BA`).

### `009FC1C0` BSP_BotPathLegManager_Remove

ABI: `__thiscall(manager, unit*)`, `RET 4`. It has no SEH frame.

One swap-remove pass. A record goes if it is the unit's own, null, or out of action.

### `009FC260` BSP_BotPathLegManager_Altitude

ABI: `__thiscall(manager, unit*, const float* point, float weight)`, `RET 0Ch`. The result comes back in ST0.

- **Null unit.** Unlock and return `FLDZ` (`009FC2B4`-`009FC2D6`).
- **Collect** (`009FC336`-`009FC3E4`, x87):
  - the unit's own record is remembered and skipped;
  - null records are swap-removed;
  - any other record is kept when `(float)(px - rx)^2 + (float)(pz - rz)^2 < ((float)(R_other + R_self))^2`,
    and its y goes into a scratch array (`operator_new(count*4)`, freed at `009FC6E7`).
  - The comparison is `FCOMIP ST0,ST1` with ST0 = r², and `JBE` skips. It is strict.
- **Bands.** The listing is unrolled by four at `009FC44D`-`009FC60A`, with the remainder at
  `009FC614`-`009FC688`. The stack was traced through the `FXCH`/`FSTP STn` rotation:
  - `sep = (float)(tuning+378h * 0.5)`, where the 0.5 is the qword `[00D7A280]`;
  - for each band, `lo = (float)(b - sep)` and `hi = (float)(b + sep)`;
  - `down = lo` when `lo < down && down < hi`;
  - `up = hi` when `lo < up && up < hi`. This test uses the same `lo`/`hi`, after the `down` update.
  - The pass repeats while any band moved (CL). DL records that anything moved at all.
- **Choice** (`009FC692`-`009FC6DB`). The bytes `D8 E1` / `D9 CA` / `DE E1` were read from the image:
  - `FSUB ST0,ST1` gives `y - down`;
  - `FXCH ST2`;
  - `FSUBRP ST1` gives `up - down` (Intel `DE E0+i`: ST(i) = ST(0) - ST(i));
  - `FCOMIP`, then `JA` selects `down`.
  - Otherwise `up` is chosen when `up < Ceiling` (`FCOMIP` + `JNC`), and `down` when it is not.
  - Nothing moved: `alt = y`.
- **Floor** (`009FC6F0`-`009FC73C`, SSE + x87):
  - `f = y`;
  - `f = -20.0f` (`[00CF180C]`) when `-20 > y`;
  - otherwise `f = 120.0f` (`[00D05804]`) when `y > 120.0` (qword `[00D1F3F8]`);
  - `alt = f` when `f > alt`.
- **Write-back** (`009FC742`-`009FC764`), only if the unit's record was found:
  - the record becomes the point;
  - then its y becomes `alt`.
- **Return** (`009FC769`-`009FC78A`): `(float)(y + weight * (alt - y))`. The `FSUBP ST(2)` /
  `FMULP ST(2)` / `FADDP` sequence was traced.

### Order and precision

Record order matters only to the band scan. Swap-removal is kept exactly. The capacity growth is not
modelled; `std::vector` keeps the same order.

The image's x87 intermediates (the squared distances, the choice differences and the blend) are computed
here in double. Every float store is rounded at the same place as in the image.

The control word in force at these calls (D3D may set 24-bit precision) is not established. Under a
24-bit control word, `(up - down) > (y - down)` could round differently from `up > y` when the operands
are near-equal. This is uncertainty 2.

## 4. Uncertainty

1. **The `up` branch.** It is reachable only when `up == y` after something moved. Two cases allow this:
   - a band edge equal to `y` exactly (the strict tests then move neither bound);
   - a precision effect.

   The branch is reconstructed as listed; it is not simplified.
2. **x87 precision control.** As above.
3. **Unit liveness.** In the image, a registered unit that is deleted without passing through `+58h`
   leaves a dangling pointer. Only `unit+5Dh` and null are pruned. The reconstruction takes the unit's
   identity from the host and does not model a dangling pointer.
4. **Names.** `009FC000`, `009FC1C0`, `009FC260` and `009FB010` are named from behaviour only. There is
   no string anchor.

## 5. Ledger and Ghidra

- `ledger add-name` records were added for:
  - `009FC000` BSP_BotPathLegManager_Register
  - `009FC1C0` BSP_BotPathLegManager_Remove
  - `009FC260` BSP_BotPathLegManager_Altitude
  - `009FB010` BSP_BotPathLegLock_Get
- They were applied with `ghidra_annotate.py --apply`. The old values were `FUN_009fc000`,
  `FUN_009fc1c0`, `FUN_009fc260` and `FUN_009fb010`, none with a comment. The before-image is
  `local/ghidra-annotations-20261012T111837Z.json` in the worker tree.
- No function was created.
- `bsp.py ghidra proto 00BF9DC8` showed `void __cdecl _free(void * _Memory)` both before and after.
- 'Non-Returning Functions - Discovered' was not touched.
- Out of lease and not renamed:
  - `009FAA40` (manager constructor);
  - `009FA9F0` (lock constructor);
  - `009FD050` (the moveonpath doc reconstructs it as `moveonpath_leg_weight_009fd050`);
  - `009C32A0` (moveto slot `+58h`, no Ghidra function).

## 6. Host glue (suggested, not applied)

This diff replaces the pass-through in the moveonpath glue (docs/CC13_MOVEONPATH_TASK.md section 8)
behind `kLegAltitudeManagerBound`, which is committed OFF. It stacks on that glue and cannot stand alone,
because it edits the glue's `Binding` overrides and the lines next to `kMoveOnPathTaskBound`.

Checks:
- Applied in this worktree on top of the moveonpath glue, `./scripts/build.ps1` passed (build and both
  ctest suites), and the change was then reverted.
- `git apply --check` against origin/main `63256d78f`:
  - with the moveonpath glue applied first: clean;
  - alone: fails at `pilot_order_bindings.hpp:68`, as expected.

The pieces:
- `GameUnitsHost::Impl` gains one `bsp::LegAltitudeManager`. The image has one per process; here it is
  one per host. It also gains a `LegUnits` adapter:
  - a record's unit is slot index + 1, and 0 stays null;
  - `unit+5Dh` is replaced by the host's death predicate (`plane_death_c3a`, `plane_death_removed`,
    gunnery `unit_dead`). This is a LABELLED substitution;
  - `[unit+538h]+268h` is `plane_turn_circle_radius`.
- The three overrides keep their records. With the switch ON they also call the manager.
  - `009FC260` receives `Pilot/MoveTo/CircleAltDiff` and `Dynamics/Ceiling` from `plane_globals()`.
    When the globals are not loaded, it uses the image-less defaults 30 / 1500.
- LABELLED: the moveto and attackmove refresh (`moveto_refresh_009beba0`, cc9's lane) still only records
  `009FD050`. With the switch ON, moveonpath planes therefore separate only against each other.
  Binding moveto is a separate cc9 change, using the same manager member.

```diff
diff --git a/include/bsp/pilot_order_bindings.hpp b/include/bsp/pilot_order_bindings.hpp
index a8359222a..3e4bd3465 100644
--- a/include/bsp/pilot_order_bindings.hpp
+++ b/include/bsp/pilot_order_bindings.hpp
@@ -68,6 +68,11 @@ inline constexpr bool kMoveToCircleSteerBound = true;
 // 009BD990 (src/moveonpath_task_cc13.cpp). Committed OFF: the plane keeps the
 // task it already had while its director holds the path command.
 inline constexpr bool kMoveOnPathTaskBound = false;
+// Packet cc13-legmgr1 (docs/CC13_LEG_ALTITUDE_MANAGER.md): the converging-leg
+// manager 009BD400 (009FC000 / 009FC1C0 / 009FC260) behind the moveonpath task's
+// leg calls. Committed OFF: registration and release stay records and the
+// altitude is the steer point's own height.
+inline constexpr bool kLegAltitudeManagerBound = false;
 // 008A44EF. `retreat`.
 inline constexpr std::uint32_t kPilotOrderClassRetreat = 0x00E08F90u;
 // 008A4900. `land`, SCENE_COMMAND_TYPES row 22.
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index 4b9ea270b..52a9e8d15 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -74,6 +74,7 @@
 #include "bsp/torpedo_release_spawn.hpp"
 #include "bsp/bot_tasks.hpp"
 #include "bsp/moveonpath_task_cc13.hpp"
+#include "bsp/leg_altitude_manager_cc13.hpp"   // packet cc13-legmgr1
 #include "bsp/game_hosts_ship_ai.hpp"
 #include "bsp/unit_hull_extents.hpp"
 
@@ -6346,6 +6347,29 @@ struct GameUnitsHost::Impl {
     // head of the next motion step, which is the same frame boundary.
     int squadron_leaves_{0};
     int squadron_promotions_{0};
+    // Packet cc13-legmgr1 (docs/CC13_LEG_ALTITUDE_MANAGER.md), kLegAltitudeManagerBound:
+    // the process-wide converging-leg manager 009BD400 (one per host here) and its
+    // two unit reads. A record's unit is slot index + 1. unit+5Dh stands as the
+    // host's death predicate (plane_death_c3a / removed / gunnery dead), a
+    // LABELLED substitution; [unit+538h]+268h is plane_turn_circle_radius.
+    struct LegUnits final : bsp::LegAltitudeUnitHost {
+        explicit LegUnits(const Impl& h) : h_(h) {}
+        const GameUnitSlot* slot(bsp::LegUnitId id) const {
+            return id != 0 && id - 1 < h_.slots.size() ? h_.slots[id - 1].get() : nullptr;
+        }
+        bool out_of_action_5d(bsp::LegUnitId id) const override {
+            const GameUnitSlot* s = slot(id);
+            return s == nullptr || s->plane_death_c3a || s->plane_death_removed ||
+                (h_.gunnery != nullptr && h_.gunnery->unit_dead(s->process_index));
+        }
+        float turn_circle_radius_268(bsp::LegUnitId id) const override {
+            const GameUnitSlot* s = slot(id);
+            return s != nullptr ? s->plane_turn_circle_radius : 0.0f;
+        }
+        const Impl& h_;
+    };
+    bsp::LegAltitudeManager leg_manager_cc13_;
+    LegUnits leg_units_cc13_{*this};
     void squadron_leave_on_death_007bcaa0() {
         for (const auto& owned : slots) {
             GameUnitSlot& s = *owned;
@@ -29496,7 +29520,10 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                     // 0099A170's arm (0099A24A) and ticked through its vtable+64h
                     // 009BD990. The squadron slot stands for ctl (unit+9D4h).
                     // LABELLED substitutions: the converging-leg manager 009BD400
-                    // is not modelled (no separation; its altitude is the input);
+                    // runs only under kLegAltitudeManagerBound (packet cc13-legmgr1,
+                    // docs/CC13_LEG_ALTITUDE_MANAGER.md; OFF: no separation, its
+                    // altitude is the input), and then only moveonpath planes
+                    // register (the moveto refresh still records 009FD050);
                     // slot +54h runs once, at the install (no re-activation hook);
                     // the follow state is the moveto task's (same vtable
                     // 00D20AB8); the circle state, unreachable live, is a record.
@@ -29507,16 +29534,34 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 return pb_.owner_.release_altitude_draw_00bd2f10(
                                     pb_.unit_.row.name + "#mp" + key, lo, hi);
                             }
-                            void leg_register_009fc000(const std::array<float, 3>&) override {
+                            void leg_register_009fc000(const std::array<float, 3>& p) override {
                                 pb_.owner_.record("BotPathLegManager::register", 0x009fc000u);
+                                if (!bsp::kLegAltitudeManagerBound) return;
+                                pb_.owner_.leg_manager_cc13_.register_009fc000(
+                                    pb_.owner_.leg_units_cc13_, leg_id(), p);
                             }
                             void leg_remove_009fc1c0() override {
                                 pb_.owner_.record("BotPathLegManager::remove", 0x009fc1c0u);
+                                if (!bsp::kLegAltitudeManagerBound) return;
+                                pb_.owner_.leg_manager_cc13_.remove_009fc1c0(
+                                    pb_.owner_.leg_units_cc13_, leg_id());
                             }
                             float leg_altitude_009fc260(const std::array<float, 3>& p,
-                                                        float) override {
+                                                        float w) override {
                                 pb_.owner_.record("BotPathLegManager::altitude", 0x009fc260u);
-                                return p[1];
+                                if (!bsp::kLegAltitudeManagerBound) return p[1];
+                                bsp::LegAltitudeTuning t;
+                                if (pb_.owner_.lua.plane_globals_loaded()) {
+                                    const bsp::GameTuningBlock& g = pb_.owner_.lua.plane_globals();
+                                    t.circle_alt_diff_378 = g.pilot_move_to_circle_alt_diff;
+                                    t.ceiling_210 = g.dynamics_ceiling;
+                                }
+                                return pb_.owner_.leg_manager_cc13_.altitude_009fc260(
+                                    pb_.owner_.leg_units_cc13_, t, leg_id(), p, w);
+                            }
+                            // The record's unit pointer: process index + 1 (0 is null).
+                            bsp::LegUnitId leg_id() const {
+                                return static_cast<bsp::LegUnitId>(pb_.unit_.process_index + 1);
                             }
                             bsp::MoveOnPathApproachInputs approach_inputs() override {
                                 bsp::MoveOnPathApproachInputs in;
```

## 7. Open questions

1. Does JM06's Catalina share its last 3000 m with any other registered plane? Moveto and attackmove
   planes register too, so in the image a fighter cover flight on moveto near the Catalina would push it
   down to no lower than 120 m. The host's moveto refresh (`src/game_hosts_units.cpp`,
   `moveto_refresh_009beba0`) still records `009FD050` and does not register. Until cc9 wires that, the
   suggested glue separates moveonpath planes only against each other. That substitution is labelled in
   the glue.
2. Is the x87 precision control at these calls 24-bit or 53-bit? A same-binary trace at `009FC6A2` would
   settle it.
