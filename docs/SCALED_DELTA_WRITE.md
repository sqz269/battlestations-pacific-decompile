# The world tick's scaled delta, game+21F0h (packet `cc9_scaled_delta_write`)

Addresses: 004E4D45 (the writer's call site), 004C6E30 (the writer), 00914EF0 and 006DC1A0 (the
two gameplay-side readers), 0098759E, 00481640.

## 1. The writer and the value

`BSP_Game_OnMove` calls 004C6E30 at **004E4D45** with ECX = game and the frame delta's address:

```
004E4D3C  PUSH EAX            ; &delta
004E4D3D  MOV ECX,ESI         ; game
004E4D45  CALL 004C6E30
004E4D4A  FLD [ESI+21F0h]     ; read straight back into the frame clock
```

The call comes after the request drain (004E4D02) and before the fixed steps and Think (004E5133).
004C6E30 (`bsp::scale_frame_delta_004c6e30`, `src/game_frame_control.cpp`) stores its `scaled`
result at game+21F0h.
- In single view with no fast-forward, turbo, slow-motion or cinematic input, and a step under
  the single-step threshold, `scaled` is the delta unchanged. That holds for every harness run:
  0.05 s, no input.
- The host does not call 004C6E30. Its frame sets `frame_state.scaled_delta = raw_delta` ("no time
  dilation in this process"), which is the same value in these runs.
- The world tick state `WorldTickState::game.scaled_delta` (+21F0h) was never written, so it read 0.

`kScaledDeltaWriteBound` (`src/game_hosts_mission_frame.cpp`) writes it once per frame, right after
the frame's elapsed clock and its own request drain, from `frame_state.scaled_delta`. With the
switch on, the warning director's hold-and-restore workaround (packet cc9_warning_manager_tick) is
dropped, because the field already holds the director's argument.

`src/game_frame_control.cpp` is not touched. A harness jitter on the raw delta reaches this value
only through `frame_state.scaled_delta`.

## 2. Every reader the write feeds

These are the readers of `WorldTickState::game.scaled_delta` that this host calls:

| reader | site | use | gate in this host |
| --- | --- | --- | --- |
| 00987590 warning director | 0098759E | the 4 s accumulator +198h | open. It already received the argument through the workaround, so its value does not change. |
| 00914EF0 bot scheduler | 00914F36, 00915003, 00915058 | the retarget and think countdowns, and the active time | closed: `bots.enabled` (+1498h) and `slots_active` (+14A4h) have no writer in this host. The countdowns and the accumulator do not run. |
| 006DC1A0 marker update | 006DC1C3..006DC39C | each marker's update and pulse | no markers in the host's two groups |
| 00481640 entity manager | 00481652 | the sub-manager's +4h(delta) | closed: `world_gate.enabled` is false (the world object is a load record) |

Not fed by this write:
- `mission_events_run` in `src/world_entities.cpp` (004E4E50's positive-delta test), because the
  host drives 00987590 directly and not through `run_world_tick`;
- the decal manager, which the host hands its own argument;
- the menu screens' `interface_only_pump_delta` (004C414F), which reads a different struct
  (`InterfaceOnlyGameFields`).

**So the write alone moves nothing in these runs.** Every gameplay-side reader is gated shut by
state this host does not build. The bot scheduler needs +1498h, +14A4h and the periods at
+14A8h/+14ACh; their writers are not in this host, and binding them is a separate packet. The
write is still the image's, and it removes a latent 0 for the day those gates open.

## 3. Predictions (written before the pairs; the switch committed OFF)

One tree (main aba71a717 plus this), `local\bin\sd_off` against `local\bin\sd_on`,
`BSP_GUNNERY_RNG_STREAMS=1`, 1600x900. USN04 4700/4500 and USN02 9200/9000.
- **The bot scheduler.** No retargets and no think passes on either side: its `enabled` byte is
  clear. No `BotScheduler::*` row appears.
- **Markers.** No unit's state changes, and no `Markers::update_marker` row appears.
- **The warning scan.** `scan_proximity` stays at 55 in USN04 and 111 in USN02, now fed by the
  frame's write.
- **Deaths, hit records, releases.** Identical: a band of zero. The whole native table, the
  per-entity tables and every summary line are the same on both sides. A move would be the
  finding, and it would name a reader missing from section 2.

## 4. The pairs and the verdict

One tree (aba71a717 + de0ed0f47), `local\bin\sd_off` against `local\bin\sd_on`,
`BSP_GUNNERY_RNG_STREAMS=1`. All four logs show the 1600x900 fit line and the final COM release.

| | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | ---: | ---: | ---: | ---: |
| unimplemented total | 2,194,415 | 2,194,415 | 4,302,698 | 4,302,698 |
| `scan_proximity` | 55 | 55 | 111 | 111 |
| gunnery deaths / hit records | 41 / 727 | 41 / 727 | 22 / 440 | 22 / 440 |

**Every prediction held.**
- The whole native table, concrete and unimplemented rows both ways, is identical. The only
  exception is the harness counter `PlatformLoopCallbacks::pretranslate` (18 against 19 in USN04),
  which the standing rules ignore.
- The per-entity tables and every summary line are identical, so the clock offset is zero.
- No `BotScheduler::*` or `Markers::update_marker` row appears on either side.
- The warning scan keeps 55 and 111, now fed by the frame's write, not by the workaround.

**Verdict: ON.** The write is the image's (004E4D45). It moves nothing in these runs, because
every gameplay-side reader is gated shut by state the host does not build (section 2). With the
switch on, the warning director reads the frame's value directly. `kScaledDeltaWriteBound` is set
true.
