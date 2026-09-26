# The warning manager's per-frame tick (packet `cc9_warning_manager_tick`)

Addresses: 00982540, 0096D540, 00968550, 00977990, 00977690, 00977820, 00965810.

The object is the one `docs/MISSION_EVENTS_UPDATE.md` reads whole: the in-mission warning and
radio-chatter director at `game+21E0h` (also held at [00F8A0C4]). Its per-frame routine is 00987590
(`BSP_WarningManager_Update`, reconstructed as `bsp::update_mission_events_00987590`). The host
recorded three of its calls under names from an older reading (`MissionEvents::pre_pass`,
`poll_zones`, `poll_triggers`). This packet binds them under their own names.

## 1. Order and what each routine touches

00987590, `__thiscall(this, float scaledDelta)`, calls in this order (the table in
`docs/MISSION_EVENTS_UPDATE.md`):

| site | routine | reads | writes | reaches |
| --- | --- | --- | --- | --- |
| 009875A2 | 00982540 `PumpInputChannel` | the `input` channel of the +F8h channel map; the 30h-byte input records | nothing on the manager | `BSP_MissionLuaHost_CallNamedThreadSafe` for each subscription whose action was pressed |
| 009875D9 | 00977990 `ScanProximity`, only when +198h crossed 4.0 | the world lists [game+19CCh]+64h and +13Ch; each entity's +54h, +5Ch..+60h, +348h/+34Ch and transform | 00975D00's per-entity record (+4h deadline, +8h bound entity); `other+9h = 1` on release | nothing outside its records |
| 009875E0 | 0096D540 `PollPrompt` | +19Ch, input action 3 | +19Ch cleared | `[game+1ED4h]` vtable +5Ch(54h) and +390h = 1; the Lua callback |
| 009875E7 | 00968550 `UpdateDeadlines` | +174h/+178h, +17Ch/+180h against 00F876A4 | the two flags | `[[[00E198C4]+C4h]+60h]` vtable +34h(0) when the collision flag clears |

The queue walk that follows (retire one expired warning, pick the highest-priority ready one, hand
it to the voice manager through 00974070) is already the reconstruction's.

**Why 00977990 was never reached.** The accumulator +198h adds the director's argument
(`FLD [EBP+8]; FADD [EDI+198h]` at 0098759E) and fires strictly above 4.0 (00CE3D34). The
reconstruction reads the delta from `WorldTickState::game.scaled_delta`, and the host never writes
that field. `update_warning_manager_00987590` received the frame's delta and dropped it. So the
accumulator stayed at 0. This is a host bug, not the image's behaviour.

**The same field is read by the bot scheduler 00914EF0** (`bsp::update_bot_scheduler_00914ef0`,
its retarget countdowns) and by the marker update 006DC1A0. Both run from this host with it at 0.
That is out of this packet's scope, and it is reported, not fixed: fixing it would move gameplay.

## 2. What raises entries

- **009DA8D0**, the ship AI's warning timer (`docs/SHIP_AI_TAILS.md` section 2), on
  [brain+AA8h] = the manager:
  - **00977690 `ReportTorpedo(this, entity)`.** The guards, in listing order: +D0h clear; the local
    slot in [0,8); the clock above 4.0; the entity's +5Ch set and +5Dh, +60h, +5Eh clear; its
    suppress byte from 0077EDF0 on this+D4h clear; 00965810; and not kind 8.
    - 00965810 answers true when the entity is the controlled unit [00E188D8], or when it is a
      plane (kind 0Fh) whose +9D4h is the controlled unit.
    - **So only the player's own unit gets a torpedo warning.**
    - It then builds a 84h-byte warning "torpedo" (00974150) and reports it through 009763E0.
  - **00977820(this, entity).** 00975D00's per-entity record holds a deadline at +0h. When the
    deadline has passed, it becomes now + 30.0 (00CE7630, double), and a point effect is spawned.
    The effect is `BSP_PointEffect_CreateWithParentMatrix(definition this+190h, parent entity+4A4h,
    identity scaled by 1.0 (00D7A24C))`, with `effect+9 = 1`. It is render-side only.
- **The Lua route**: the channel dispatchers of `docs/MISSION_EVENTS_UPDATE.md` ("Channel
  producers").
- **00980380** writes the two sound deadlines (+174h, +17Ch). The host has no caller of it.
- **+19Ch**, the prompt callback, has no writer established in any doc. The host never sets it.

## 3. Who reads the result

- **The voice manager.** The queue walk hands the selected warning to 00974070, and 005B71D0 gates
  it on [[00E198C4]+A4h]. That is audio.
- **The HUD.** 0096D540 raises the prompt flag on [game+1ED4h]. `src/hud_warning_screen.cpp` is
  screen 29h's pick and does not read the manager.
- **Mission Lua**, through the `input` channel (00982540) and the prompt (0096D540). This is the
  one path that could reach gameplay. Here the `input` channel is empty, because 0097E360's
  parser is unbound (`MissionEvents::command_event_block` is its existing record), and the prompt
  has no writer.
- **The point effect** from 00977820 is render-side.
- No gunnery, ship-AI or unit field reads the manager.

## 4. The binding

`kWarningManagerTickBound` (`src/game_hosts_mission_frame.cpp`):
- **00982540** becomes `WarningManager::pump_input_channel` (done). The empty channel is the named
  record `WarningManager::input_channel_subscriptions` (0097E360), once per call.
- **00977990** becomes the named record `WarningManager::scan_proximity`: its world lists are not
  built (004DE610 is a load record).
- **0096D540** becomes `WarningManager::poll_prompt`, through `bsp::poll_warning_prompt_0096d540`
  on the host's `bsp::WarningManagerState`.
- **00968550** becomes `WarningManager::update_deadlines`, through `bsp::update_warning_deadlines`.
  A collision expiry records `WarningManager::stop_collision_sound`.
- **The accumulator** takes the director's argument for the call alone. The world field is
  restored after it, so the bot scheduler and the markers are untouched.
- A summary line reports the manager's counters: `summary mission warning manager ...`.

Off, all four are the old records, and the accumulator reads the unwritten field.

## 5. The contract with the gunnery worker (009DA8D0)

Declared in `include/bsp/game_hosts_mission_frame.hpp`, in namespace `bsp::game`:

```cpp
void game_warning_report_torpedo_00977690(std::size_t unit);   // zero-based unit index
void game_warning_torpedo_effect_00977820(std::size_t unit);
```

- Both work on the live mission frame host's manager and are no-ops before it exists.
- `report_torpedo` applies every guard of section 2 the host can answer:
  - +D0h;
  - the clock (`bsp::warning_report_clock_allows`);
  - the four unit bytes;
  - the suppress byte, which has no entries here;
  - 00965810, the controlled unit (a plane's +9D4h owner is not modelled);
  - kind 8.
  An accepted report then records `WarningManager::report_torpedo_queue` (009763E0), because the
  warning object and its voice clips are not built.
- `torpedo_effect` keeps the 30 s per-unit deadline and records
  `WarningManager::torpedo_effect_spawn` when it fires.
- They count only in the warning-manager summary line, so a gunnery pair moves no gunnery or
  ship-AI line through them.
- The timer calls them with [brain+AA8h] as the manager. There is one manager, so the entries take
  the unit only.

## 6. Predictions (written before the pairs; the switch committed OFF)

One tree (main 6e7e38a66 plus this), `local\bin\wm_off` against `local\bin\wm_on`,
`BSP_GUNNERY_RNG_STREAMS=1`, 1600x900. USN04 4700/4500 and USN02 9200/9000.
- **Rows.** The three `MissionEvents::*` records (one per director call: 4,500 and 9,000) are
  replaced by:
  - `WarningManager::pump_input_channel`, `poll_prompt` and `update_deadlines`, concrete at the
    same count;
  - the record `WarningManager::input_channel_subscriptions`, at the same count;
  - the record `WarningManager::scan_proximity`, once each time the accumulator passes 4.0.
    At 0.05 s a frame it fires about every 81 frames: about 55 in USN04 and 111 in USN02.
  - The unimplemented total falls by twice the call count, less the scans.
- **Entries raised.** No torpedo reports and no effects: nothing calls the two entries until the
  gunnery worker binds 009DA8D0. No deadline expires, because nothing writes them. No prompt fires.
  The summary line reads `scans=~55` (USN04) or `~111` (USN02), and zeros elsewhere.
- **What the screen shows.** Nothing new: no warning reaches the voice manager, and the prompt flag
  is not raised.
- **Gameplay.** No gameplay row, per-entity row or summary line moves except the new
  warning-manager line and the rows above. A move would be the finding.
