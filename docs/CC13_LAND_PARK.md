# Land task started in land/park (packet cc13-landpark)

Static spec for cc9-physics29, which owns the switch at the refusal site in
`GameUnitsHost::Impl::install_land_task_core_009b41c0` (`src/game_hosts_units.cpp`,
"not airborne: the task would start in land/park (unbound)"). Nothing here changes code; the
park state's enter, tick, done test and rule arms are already hosted (section 3). Read live
against `/battlestationspacific.exe` on 2026-10-27; names are hypotheses.

## 1. What the image does at 009B32AA-009B32B8

`009B3240 BSP_BotTaskLand_Construct` (`__thiscall(task)(bot, target)`, `RET 8`), called from
`009B41C0 BSP_BotTask_MakeLand`:

| Address | Instruction | Meaning |
|---|---|---|
| `009B3265` | `CALL 0099C6F0` | base task constructor (bot, 3) |
| `009B3283` | `CALL 009B2E50` | approach composite at task+3F8h; its `009AFE70` stores the plane at approach+4 (task+3FCh) and takes the angle/pitch draws; `009C2980` takes the follow stagger draw |
| `009B3294`-`009B32A0` | vtables `00D1FFA0`, `00D1FF94` (+3F8h), `00D1FF90` (+4B0h) | |
| `009B3288`-`009B32B4` | `ECX = [task+3FCh]+72Ch`, `CALL [[ECX]+38h]` | the plane's airborne predicate; the host reads it as `+900h == 7` (docs/SQUADRON_LAND_TASK.md, LABELLED) |
| `009B32B8` | `JNZ 009B32C2` | airborne: leader test |
| `009B32BA` | `LEA ECX,[ESI+620h]` | **not airborne: the start state is land/park, task+620h** |
| `009B32C2`-`009B32D7` | `007B8AD0` leader ? `+4C4h` (moveto) : `+500h` (follow) | airborne arms |
| `009B32DD` | `MOV [ESI+310h],ECX` | current state |
| `009B32E3`-`009B32E8` | `CALL [[ECX]+4]` | the state's enter, directly (no `009B3680` switch, so no exit runs) |
| `009B32ED` | `CALL 009F9980(approach, task)` | approach+18h = task+4, +1Ch = task+314h, +20h = task+38Ch |

So the image installs the task (with all of its construction draws) for a grounded plane, and
only the start state differs.

### The land/park state object (task+620h = approach+228h)

Built in `009B2E50` at `009B2FA1`-`009B2FC5`:

| Field | Value at construction |
|---|---|
| +0h | vtable `00D1FF60` |
| +4h | the approach (back pointer) |
| +8h, +0Ch, +10h, +14h | 0 |
| +18h (byte) | 0, the done byte |
| +1Ch, +20h, +24h, +28h | not written by the constructor |

Vtable `00D1FF60`:

| Slot | Target | Role |
|---|---|---|
| +0h | `009B2E30` | scalar deleting destructor |
| +4h | `009B21A0` | **enter** (`009B21A0`-`009B21BA`): +1Ch = 0.0, +18h = 0, +28h = 3.0 (`00CE3854`) |
| +8h | `009B21C0` | exit, `RET` only |
| +0Ch | `009B22C0` | **tick** (`009B22C0`-`009B2C68`, `__thiscall(park, float dt)`, `RET 4`) |
| +10h | `007B3DE0` | base predicate, true |
| +14h | `007B3DF0` | base no-op |
| +18h | `007B45E0` | base predicate, false |

+20h/+24h (the taxi target x/z) are first written by the tick, right after it has compared
them with the new target when the plane is off a holder (`009B23F1`-`009B2454`). The constructor leaves them as the allocation left them; the host's
default is 0.0 (uncertain: the first off-holder tick may count one retarget).

## 2. Is it hosted?

Yes, all of it, in the per-unit land binding of `src/game_hosts_units.cpp`, under
`kLandParkStateBound` (ON, packet cc9_land_park_taxi, docs/SQUADRON_LAND_TASK.md 5s):

| Image | Host |
|---|---|
| `009B21A0` enter | `land_enter_park_009b21a0()` (fields `land_park_1c`, `land_done_18`, `land_park_timer_28`) |
| `009B21C0` exit | nothing to do (`RET`) |
| `009B22C0` tick | `run_land_park_tick_009b22c0(dt)`, dispatched from `land_state == LandTaskState::kPark` (0x620) |
| `009B21D0` done test | `land_park_done_009b21d0(...)` |
| `009B3770` park arm | `land_state_rule_009b3cf0()`: in park with the done byte set, `land_enter_abort_009b0980()` |

`native_land_state_entries.cpp` (`enter_native_land_park_009b21a0`) is a separate native-ABI
reconstruction of the same enter, not wired to the host. Ledger: `009B21A0`
BSP_BotStateLandPark_Enter, `009B21C0` BSP_BotStateLandPark_Exit, `009B22C0`
BSP_PlaneBot_ApproachBaseStep (source `src/game_hosts_units.cpp`).

So no new reconstruction and no `kPlaneLandParkStateCc13Bound` switch is needed: physics29's
switch can install the task and start it in the hosted park state.

## 3. What physics29's arm must do (the image's order)

At the refusal site, instead of refusing:

1. Run the whole install exactly as for an airborne plane, up to and including the construction
   draws (`#l54`, `#l58`, `#lb0`, `#l74`), `land_mode_50 = 1`, the site/deck fields,
   `land_task_installed`, `bot_task_ctor_seeds_0099c6f0`, `land_profile_activation_pending` and
   `land_installed_at`. The image takes those draws before it picks the state, so skipping them
   would shift the shared stream (00BD2F10).
2. Instead of `kMoveTo`/`kFollow`: `land_state = LandTaskState::kPark`, then the enter's writes
   directly (the image calls vtable[4] with no exit and no `009B3680`):
   `land_park_1c = 0.0f`, `land_done_18 = false`, `land_park_timer_28 = 3.0f`.
   `land_enter_park_009b21a0()` itself is a member of the per-unit binding, not callable from
   `Impl`; if physics29 prefers to call it, the binding's first think can do it, but it then
   also runs the `009B3680` bookkeeping (state-change log), which the image does not.
   The leader test `007B8AD0` is not evaluated on this arm (`009B32B8` jumps past it).
3. Leave `land_park_target_20/24` at 0.0 (see the uncertainty above).

## 4. Fields the park tick reads and writes (009B22C0, as hosted)

Reads: the deck (`land_deck_plus_one`, `land_site_plus_one`), the owner's death, plane
`+BF8h`/`+BF4h` (holder contact), `+900h` (control mode), `+910h`, the hangar/lift points
(`006CF420`/`006CF520`, or `006D0120`/`006D00E0` on a carrier), the tuning block
(AirField/MoveSpd +184h, Pilot/Landing/ParkVelocity +4E8h, the runway yaw-turn rows), the
speed (vtable[38h]) and the heading.

Writes: bank target 0 (+2CCh = 1), pitch 0 active (+2D0h = 0), park +20h/+24h (target),
+28h (carrier clock), the done byte +18h, plane `+910h` (queue), `+C00h` (hangar, `007B96C0`),
the speed request, and the path transitions `007C16F0` (4 -> 5) / `007C1680` (5 -> 4) and
`007B9000` (`+C01h` = 2) on a retarget.

## 5. Transitions out of park

- **Done byte -> land/abort** (`009B3770`, `009B37A7`), the only rule arm out of park. The done
  test `009B21D0` sets it, off the path (`+900h != 5`) and with the byte clear, when:
  `+28h < 0`; or the plane is not resting on a holder (`+BF8h` clear or `+BF4h` = 0); or, on a
  holder, the target z is behind the plane (and, on a carrier, the plane is not at the lift,
  `006CFF70`); or the heading is more than pi/2 (`00CE3830`) off the runway heading
  (holder+88h). The tick also sets it at once when the plane rests on a holder whose owner is
  dead (`009B22DC`-`009B2313`).
- Park does not leave by the `+900h` 4/5 arm (`009B3D0B`-`009B3D3B` skips park and abort).
- From land/abort, the image pushes a takeoff task at the first ground abort (`0099B0BE`,
  docs/SQUADRON_LAND_TASK.md 5bp); the host's abort arm returns to standby (+66Ch) or park
  (+66Dh) instead.

For a recalled on-deck squadron, then, a plane at rest on its holder and facing the runway
heading taxis to the hangar/lift and stays in park; a plane not on a holder (+BF8h clear) is
done on its first tick and goes to abort, where the image re-launches it.
