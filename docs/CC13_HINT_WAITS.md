# Hint-state waits in this installation's mission scripts (cc13-scripts1, item b for cc9)

Question: which mission-script reads of hint state (`IsHintActive`, `IsHintCritical`,
`IsHintStored`) and which timer chains keyed on hints can change mission flow (objectives,
spawns, phase changes, mission end) rather than only presentation? Context: the host has no
TriggerHint path yet, so `ShowHint` shows nothing and `IsHintActive` always answers "no hint".

## Summary

- 16 mission files contain 90 matches of `IsHint(Active|Critical|Stored)`: 33 native
  `IsHintActive`, 21 native `IsHintCritical`, **0 `IsHintStored`** anywhere in `scripts/`; the
  other 36 are the per-mission wrappers `luaJM{1,5,7,10}IsHintActive` (definitions and calls).
  (The brief's figure of 108 was not reproduced; this count is from `tools/hint_query_scan.py`, which
  separates native calls from wrapper names.)
- The 16 files are three copies of five IJN missions (`missions/ijn/JM/`, `missions/COTP-IJN/`,
  `missions/COTP-IJN/PRCPIJN/`, plus `PRCPIJN/prcpjm07.lua`) and
  `missions/COTP-USN/usn_01_battle_of_eastern_solomons.lua`. All have mtime 2024-07-13 08:26 in
  this installation (modded; the COTP/PRCP trees are campaign-mod content). The hint functions are
  identical across the copies after stripping comments and whitespace (`tools/lua_variant_compare.py`),
  except where noted (PRCP jm01 objectives, PRCP jm07/prcpjm07 objectives).
- **Three places can delay mission flow**; none can block it permanently, and none can change
  *what* happens, only *when*:
  1. jm07 adds primary objective 1 (airfields; recon levels, score, airfield movie, `Pri1Added`
     dialog) only on a `CheckObjectives` pass where no `IJN07*` or critical hint is active.
  2. jm01's message queue holds every queued dialog while a critical hint is active; three of the
     queued dialogs carry flow callbacks: `1st_Success` -> `luaJM1AddMicrotutObj` (adds primary 5),
     `MicrotutDone` -> `luaJM1SwapChange`, `Governor` -> `luaJM1EndFlag` (`Mission.Phase4End = true`,
     the mission-end flag).
  3. jm05's message queue holds queued dialogs while an `IJN05*` or critical hint is active; the
     queued `Map_hint1` dialog's callback `luaJM5EnableMap` re-enables the map input.
- Everything else is presentation: hiding or re-showing hints, random hint pacing, a dead
  jump-in-movie path.
- With no hints (the host today) every gate is **open**: the wrappers return false when
  `IsHintActive()` returns nil, so objectives and queued dialogs fire on the first pass instead
  of waiting. That is the same state the image reaches with the hints option off: TriggerHint
  (00690FD0) drops every trigger when the hints setting 00F8898C is 0 (docs/CC13_GUI_SCREENS_3.md,
  TriggerHint flow step 1), so `IsHintActive` is nil there too. The scripts are written to work
  with no hint on screen; the host's divergence is timing (earlier objectives and dialogs), not
  a stall.

## Native semantics relied on (from docs/CC13_GUI_SCREENS_3.md, not re-derived here)

- `IsHintActive()` (008D2850) with no argument: a table `{HintID = name, duration = seconds left}`
  while a hint is active, otherwise nothing (nil). With a name: the seconds left as an integer when
  the active hint matches **case-insensitively** (00435C40), otherwise nothing. A Lua integer 0 is
  true, so a `duration = 0` hint (no auto-dismiss) still reads as active.
- `IsHintCritical(name)` (008D26A0): the definition's `critical` byte (+114h), for any defined
  hint, not only the active one; false for an empty or undefined name.
- `ShowHint` (008D1F50) is skipped while game+624h (termination reason) is non-zero, else
  TriggerHint(name, 1).
- Critical hints in this installation (`scripts/datatables/hintsystemmessages.lua`, mtime
  2024-07-13 08:26): the global first-use tutorials (`MAP1STUSE`, `BASICPLANE*`, `BASICSHIP*`,
  `BASICSUB*`, `REPAIR1STUSE`, `ORDER1STUSE`, `SM1STUSE`, `PUM1STUSE`), `IJN03_REPAIR1STUSE_CLONE`,
  `IJN05_Hint01`, `IJN05_Hint05`, the `ijnair/ijnsea/usnair/usnsea` intro hints, and the multiplayer
  `ID_Hint_*` set. No `IJN01_*`, `IJN07_*`, `IJN10_*` or `USN01_*` hint is critical. The first-use
  tutorials are raised by the engine's own TriggerHint callers, not by these scripts.
- Hint definitions' `LuaCommand` (run by TriggerHint before showing): 90 non-empty, all only
  `luaSetNarrativeParam`/`luaSetNarrativeSticks` (button-icon text parameters). Presentation only.

## Per-site table

Line numbers: J = `missions/ijn/JM/`, C = `missions/COTP-IJN/`, P = `missions/COTP-IJN/PRCPIJN/`.

| File | Line (J / C / P) | Call (function) | What it waits for | Gates flow | Effect |
| --- | --- | --- | --- | --- | --- |
| jm01 | 1392 / 1391 / 1516 | `IsHintActive("IJN01_Hint08")` (`luaJM1CheckObjectives`) | orders hint not on screen | no | while primary 5 (retreat order) is pending, queues the orders hint again through `luaJM1OrdersHint` -> `luaJM1ShowHintorDlg("IJN01_Hint08", 1)` on each pass it is not already up (TriggerHint then drops repeats: its storingrule is `missionunique`); the objective check itself does not depend on it |
| jm01 | 1641 / 1640 / 1855 | `IsHintActive("IJN01_Hint03")` (`luaJM1SwapChange`) | swap-sticks hint up | no | hides it after the player's swap input (input listener) |
| jm01 | 1667 / 1666 / 1881 | `IsHintActive("IJN01_Hint03")` (`luaJM1ResetSwapListener`, `luaDelay` 15 s) | same | no | hides it when the 15 s timer ends |
| jm01 | 3935-3943 / 3931-3939 / 4406-4414 | `IsHintActive()`, `IsHintCritical(HintID)` (`luaJM1IsHintActive`) | active hint whose name contains `IJN05` or that is critical | (predicate) | true only for critical hints in practice: the `IJN05` test is a copy from jm05 and jm01 shows only `IJN01_*` hints |
| jm01 | 3987 / 3983 / 4458 | `luaJM1IsHintActive()` (`luaJM1ShowHintorDlg`) | no critical hint, no dialog, empty queue | **yes (delay)** | otherwise the dialog/hint goes to `Mission.MessageQ`. 61-64 call sites; flow-bearing dialogs: `1st_Success` (line 1379) -> adds primary 5; `MicrotutDone` (1576) -> `luaJM1SwapChange`; `Governor` (3614) -> `luaJM1EndFlag` sets `Mission.Phase4End` (mission end) |
| jm01 | 4012 / 4008 / 4483 | `luaJM1IsHintActive()` (`luaJM1DlgHintQCheck`, polled every 2 s by the `luaJM1MessageQ_think` script started at line 1031) | same | **yes (delay)** | dequeues one item per 2 s poll only when no critical hint or dialog is active and `Mission.StopMessages` is clear |
| jm01 | 4065, 4068 / 4060, 4063 / 4535, 4538 | `IsHintActive()`, `IsHintCritical(HintID)` (`luaJM1HideActHint`) | active non-critical hint | no | hides it before the queue shows the next hint |
| jm03 | 2871, 2874 / 2878, 2881 / 2882, 2885 | `IsHintActive()`, `IsHintCritical(HintID)` (`luaJM3HideActHint`) | active non-critical hint | no | hides it before map hints 1-5 (`luaJM3OnMap`, `luaJM3MapHint1CleanUp`) |
| jm05 | 4670-4678 / 4684-4692 / 4965-4973 | `IsHintActive()`, `IsHintCritical(HintID)` (`luaJM5IsHintActive`) | active `IJN05*` or critical hint | (predicate) | true for every jm05 mission hint (all are `IJN05_*`) |
| jm05 | 4728 / 4742 / 5023 | `luaJM5IsHintActive()` (`luaJM5ShowHintorDlg`) | no `IJN05*`/critical hint, no dialog, empty queue | **yes (delay)** | otherwise queued (`important` puts it at the head). Flow-bearing: `Map_hint1` (line 3227) -> `luaJM5EnableMap` (`ForceEnableInput(IC_MAP_TOGGLE, true)` then the map hint) |
| jm05 | 4754 / 4768 / 5049 | `luaJM5IsHintActive()` (`luaJM5DlgHintQCheck`, 2 s poll, started at line 1074) | same | **yes (delay)** | as jm01; `Mission.HintsShowed` is written but never read |
| jm05 | 4807, 4810 / 4821, 4824 / 5102, 5105 | `IsHintActive()`, `IsHintCritical(HintID)` (`luaJM5HideActHint`) | active non-critical hint | no | hide before next queued hint / map hint 1 |
| jm05 | 4821 / 4835 / 5116 | `luaJM5IsHintActive()` (`luaJM5AddJumpinMovie`) | no `IJN05*`/critical hint | no (dead) | would play a jump-in movie or retry via `luaDelay(..., 5)`; every caller is commented out in all three copies |
| jm05 | 4832 / 4846 / 5127 | `luaJM5IsHintActive()` (`luaJM5AddDelayedJumpinMovie`) | same | no (dead) | the 5 s retry of the above; skips the movie if still blocked |
| jm07 | 2671-2679 / 2686-2694 / 2860-2868 (also prcpjm07 2860-2868) | `IsHintActive()`, `IsHintCritical(HintID)` (`luaJM7IsHintActive`) | active `IJN07*` or critical hint | (predicate) | |
| jm07 | 733 / 739 / — | `luaJM7IsHintActive()` (`luaJM7CheckObjectives`, phase 1 or 2) | no `IJN07*`/critical hint (in practice `IJN07_Hint01` sub sighted, `IJN07_Hint02` Fuso sighted, or a first-use tutorial) | **yes (delay)** | adds primary 1, forced recon on the three airfields, score, `luaJM7AFMovie`, dialog `Pri1Added`. `AFMovieOK` then unlocks secondary 2 |
| jm07 (PRCP) | — / — / 847 (jm07 and prcpjm07) | same | same | no | the guarded branch's body is entirely commented out in both PRCP copies; the gate has no effect |
| jm10 | 1448-1454 (J and C) | `IsHintActive()`, `IsHintCritical(HintID)` (`luaJM10IsHintActive`) | active `IJN10*` or critical hint | (predicate) | |
| jm10 | 1479 (J and C) | `luaJM10IsHintActive()` (`luaJM10RndHint`, called from `luaJM10_think` at line 562) | no hint, no dialog, `HintDisplayInterval` elapsed | no | picks and shows the next random tip (`luaPickRnd`) |
| usn_01 (COTP-USN) | 865 | `IsHintActive("USN01_Hint_Switch_unit")` (`US01Think`) | switch-unit hint up | no | hides it once the selected unit has payloads (name differs in case from the definition `USN01_Hint_Switch_Unit`; the native compares case-insensitively) |
| usn_01 | 1750 | `IsHintActive("USN01_Hint_Turbo")` (`luaUS01RemoveTurboHint2`, `luaDelay` 20 s) | turbo hint still up | no | removes the turbo listener, hides it, chains `luaUS01MapHint` after 10 s. With no hint the listener stays and the map hint is reached only through the turbo-input path |
| usn_01 | 1773 | `IsHintActive("USN01_Hint_Map")` (`luaUS01RemoveMapHint2`, `luaDelay` 20 s) | map hint still up | no | removes the map listener and hides it |

## Timer chains keyed on hint state

Only these timers re-test hint state: jm01 `luaDelay(luaJM1ResetSwapListener, 15)`; jm05
`luaDelay(luaJM5AddDelayedJumpinMovie, 5)` (dead); usn_01 `luaDelay(luaUS01RemoveTurboHint2, 20)`
and `luaDelay(luaUS01RemoveMapHint2, 20)`; and the two 2-second queue scripts (jm01, jm05). The
many `luaDelay(lua...Hint, N)` calls elsewhere only *show* a hint after a delay and do not read
hint state.

## What this means for the host

- No mission stalls for want of hints: every gate is open when `IsHintActive()` returns nil.
- Timing divergence from an image run with hints on: jm07 primary 1 and the jm01/jm05 queued
  dialogs (including jm01's mission-end flag via `Governor`) fire up to one hint lifetime earlier
  (hint seconds from `duration`, `-1` meaning the screen computes it). Compare such runs with the
  image's hints option off, or wire TriggerHint first.
- When TriggerHint lands, `IsHintActive` must return nil with no active hint, a table whose
  `HintID` is a string without an argument, and a number (0 allowed) for a case-insensitive name
  match. A table without `HintID` would make the wrappers call `string.find(nil, ...)` (a Lua
  error inside the 2 s queue think or `CheckObjectives`), and `luaJMxHideActHint` would pass nil to
  `IsHintCritical`; returning `false` instead of nil is harmless to every site listed here.

## Status

Read from this installation's scripts only (no script executed). Exported/reconstructed C++:
none for this item. Not game-validated. Tools: `tools/hint_query_scan.py` (call sites, critical list),
`tools/lua_variant_compare.py` (copy comparison).
