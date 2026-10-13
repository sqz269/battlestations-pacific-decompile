# CC13 VER 10: the squadron director and the air-ops return/landing path, checked against the bytes

Packet `cc13-ver10` (Claude, cc13 lineage, 2026-10-13). This is a verification pass over the chain that takes a squadron from
"ordnance spent" or "command finished" to landed and Dead:
- `0084E010`, the squadron step;
- `007F16D0`, the return resolution;
- `006C0840`, the site search;
- the land task construction;
- the stock return;
- `007F3970`, member removal and the squadron Kill;
- `KillReason`.

Inputs:
- docs/CC13_GPR_38.md 1 and 1.6;
- docs/CC13_VER_8.md;
- docs/SQUADRON_LAND_TASK.md 5go-5gq (cc9, read only).

Evidence levels:
- every image statement below is from the disk bytes (`bsp.py disasm-raw`), with Ghidra pseudocode as a guide only;
- the host statements are from `origin/main` at `a8c0791bc`;
- the log statements are from existing logs. No game run was made.

The model added here (section 2) is **reconstructed** and **build-tested**, and it is not wired.

Host files read: `src/game_hosts_units.cpp`, `src/game_hosts_script_orders.cpp` and `src/return_to_base.cpp`. All are cc9's, so
every host change below is an adoption note. `game_hosts_units.cpp` was leased to cc9-lua52 (`cc9_lua56_betty_rtb`) during this pass.

## 1. Verdicts, link by link

| link | image (bytes) | host | verdict |
| --- | --- | --- | --- |
| `007F16D0` null arms | `007F16E2` no class row; `007F16F0`-`007F16FB` `UCOMISS MinWaterSpd,[00D7A218]` (0.0), equal and ordered -> null; `007F1709`-`007F1720` head `IsKindOf(17h)` with `+C24h` clear -> null | `return_to_base.cpp:80-82` | faithful |
| `007F16D0` home arm | `007F1732` `006BCD20(+404h, DL=1)` (kind 9 `+1188h`, kind 45h `+72Ch`), `006C4790(block, squadron)`, `006BED30` (`+1Ch`, `+1Dh`, no owner, owner `+5Dh`) -> `007F1000(00E08FA0, +404h)` | `game_hosts_units.cpp:11868-11904` (runway/hangar failure = `+1Ch`/`+1Dh`, air_operations.hpp:500) | faithful (the `+B4h` list is labelled empty) |
| `007F16D0` site arm | `007F177E`-`007F17D2`: `006C0840(ECX = sq+54h, EDX = head, 0, 0047B850(head), 1)`; `0047B850` answers `IsKindOf(10h) \|\| IsKindOf(16h)` with a plain `RET`, so the `PUSH 1` is `006C0840`'s third argument; node+4 block; `006BC120` (owner null or `+5Dh`) refuses | `game_hosts_units.cpp:11905-11956` | faithful (`006BC120` is redundant after `006C0840`'s local-only `+5Dh` filter) |
| `006C0840` list walk | `006C0990`-`006C0ADB`: block, owner, owner `+5Eh`, local-only `+5Dh`, approach bit (`[block+20h] SHR 1 AND 1`), side (`[ESP+13h]` = `SETE` of `00927C90`, so not multiplayer), key and `accepts` take rule | `return_to_base.cpp:41-67` | faithful |
| `007F16D0` retreat arm | `004C7730(pos, side, 0, 0)`; x87 sums `0.0 + A` then `+B`, `+C`, `+D`, each stored to binary32, times the double 0.25 at `00D7A348` | `return_to_base.cpp:95-110` | faithful, x87 order included |
| `+369h` ReloadEnabled | writers: `007F2D09` (constructor, 1), `007F2216` (scene bag `ReloadEnabled`, type 3), `007F20F6` (kind-2 record `+124h`), `007F35A9` (`007F3500` clone, no caller). The capstone displacement census `local/v10_dispscan.py 369 mov,set` finds nothing else | `has_deck_369 = kLuaDeviceReloadEnabledBound` (true) | **faithful for every single-player squadron**. None of this installation's 259 `.scn` files contain `ReloadEnabled`; the positive control `HomeBase` is in 184 of them. The air-ops launch bag `006C5050` adds no `ReloadEnabled`. cc9's caveat "forces true" is not a divergence |
| `0084E010` B5 | see section 2 | `run_spent_ordnance_rtb_0084e010` (`:15236`) | resolve arm faithful; **the moveto-self fallback is missing (D1)** |
| land task construction | `009AFE70`: `+54h = U(0.9, 1.1) x tuning+4E4h`, `+58h = U(0.9, 1.1) x tuning+4E0h`, `+5Ch = class+18Ch x 0.75` (double `00CEC9D8`), `+60h = 0`, `+64h = 10.0` (`00CE38B8`), `+ACh = 0.5`, `+B0h = -U(0, 0.5)`, `+50h = 1` | `install_land_task_core_009b41c0` (`:12178-12194`); `+4E4h` is ApproachAngle and `+4E0h` ApproachPitch (game_tuning_singleton.hpp:358-359) | faithful (the draws are on per-unit streams, the standing substitution) |
| stock return gate | `007CE6CA`-`007CE6E4` (ESI = plane+310h): plane `+9E0h` clear, `+520h` clear, `+C00h` set; message C7h with `[[[plane+BF4h]+4]+7Ch]+174h` | `plane_stock_return_c7h_007cc8b0` (`:14175-14178`) | faithful |
| `009AE1B0` (dkz break-off) | base `0099C230`; no `+440h` target or target `+5Dh` -> true; `sq+369h && [00E17BF2]` -> false; `009ADD40(state)` false -> true; `+4B9h` set -> false; else true | `kamikaze_task_cc13.cpp:818-826`, feed `game_hosts_units.cpp:2611-2623` | faithful. The feed's `state->simulate` is the host's `+5Dh` torn-down byte: 0 at spawn (`:16085`), 1 at the `0092639B` flush (`:17385`) |
| `007F3970` | see section 3 | `kSquadronDeadOnLastMemberBound` | cc9's caveat has no effect; **KillReason source differs (D3)** |

## 2. `0084E010`'s tail, and divergence D1

**Bytes, `0084E45D`-`0084E5C6`.** Two flags, `b4 = [ESP+11h]` and `b5 = [ESP+13h]`:
- `b4` starts from the head arm (`0084E047`-`0084E05C`: no command, a leader, an empty queue);
- `0084E4B1`: stage 2 with `0071BE60() <= 1` sets `b4` when the latch is clear and the command is not retreat / returntobase /
  land (`0084E4CC`-`0084E4DF`); otherwise it sets `b5`;
- `0084E4ED`: override stage 2 with the latch also sets `b5`.

Then:

1. **Resolve** (`0084E52F`) is taken in two cases:
   - when `+369h` is clear and any of these holds: `b4`, no command, or a command category other than 1 and 2;
   - otherwise, when `b5` is set.
2. **The resolve gate** (`0084E535`-`0084E54B`). With `+369h` and `[00E17BF2]` both set, or with `+368h` set, the step goes to
   `0084E595` and `007F16D0` is not called.
3. **A null `007F16D0` answer** (`0084E55B JE`) also goes to `0084E595`.
4. **No resolve.**
   - `b4` falls through to `0084E595` (`0084E58F`-`0084E593`).
   - Anything else returns.
5. **`0084E595`.** When `0071BE60() <= (stage == 2)`, the step calls `0071ECF0(00E08F68, 00465080(squadron, 0.0))`, a moveto on
   the squadron entity itself.

So the moveto-self is reached from **three** places:
- the idle arm (`b4` with ReloadEnabled set, which is every single-player squadron);
- a failed resolve gate;
- a null answer.

docs/SQUADRON_LAND_TASK.md 5gq names only the first.

**Moveto-self holds.** Take a command that is a moveto whose target resolves to the squadron (`bVar3`, `0084E05E`-`0084E078`;
pseudocode lines 52-93, not byte-read):
- the head raises it to stage 2 only when the leader's `+184h` is clear;
- with the leader's `+184h` set, the head raises it to stage 1 once the leader is within `[00CFE050]`.

`0084E45D`'s raise from stage 1 to stage 2 needs `!bVar3` (bytes `0084E48A`-`0084E48E`). So a moveto-self with a live leader
stays at stage 1 and is not re-issued: the squadron loiters on itself.

**The host.** `run_spent_ordnance_rtb_0084e010` (`game_hosts_units.cpp:15236-15373`) runs only B5 (`st.ended && reload_open`).
Nothing in `src/` issues the moveto-self.

**D1, predicted effect.**
- **Every squadron whose command ends at stage 2 without the latch**, in every mission. In the image it holds over its own
  position under a moveto-self. In the host its rows are retired and it gets no order. This covers:
  - fighters (no latch, `IsKindOf(13h)`);
  - all squadrons under SetDeviceReloadEnabled(true): PRCP01 / usn_03, USN04/19, the JM missions;
  - targets killed under cc9-gunnery38's retest. `l56_R_p01r.log:81417` has Enterprise_sqn07 ending at 515.80 s and `:122242`
    has sqn10 ending at 779.38 s.
- **Under device reload**, a land or retreat command that reaches stage 2 also falls to moveto-self instead of re-resolving.
- **Kamikaze-head squadrons** (`007F1709` null) that latch: the image gives them a moveto-self. The host places `returntobase`,
  and its land intake refuses it (`007F16D0 did not answer land at site`).
- **USNOS Betties are not affected.** Reload is off, and `007F16D0` answers land at site Airfield3 (CC13_GPR_38 1.6).

**The model.**
- Files: `include/bsp/airops_ver_cc13.hpp` and `src/airops_ver_cc13.cpp`.
- `squadron_step_tail_plan_0084e4b1` gives b4/b5 and the outcome.
- `squadron_step_moveto_self_0084e595` is the room test.
- `squadron_step_issues_moveto_self` combines them with `007F16D0`'s answer.

**Adoption note (cc9).**
- Behind an OFF switch in `run_spent_ordnance_rtb_0084e010`, evaluate the plan per squadron each step:
  - the stage is 2 after the host's end-command or the retest end;
  - the latch is `st.latch_3c`;
  - the command is the leader's `attack_command_class`, and a retired command counts as stage 2 of that class.
- For `kMoveToSelfIfRoom`, and for `kResolve` with a null answer, issue `00E08F68` on each live member with the squadron entity as
  an object target. That is the same `issue_script_command` path as `issue_return_to_base_007f16d0`, with target kind 1 and the
  squadron unit.
- **RNG:** the moveto task's own construction draws, if any, on the members' streams.
- **Predicted, ON:**
  - PRCP01: sqn07 and sqn10 hold over the Japanese force after their retest ends instead of flying their task's exit. That means
    more AA exposure, and deaths can move.
  - Fighter CAP whose target dies holds in place.

**B5 runs once in the host.** `st.issued` latches forever. The image re-enters `b5` whenever a land, returntobase or retreat
command reaches stage 2, and re-resolves. This matters only when a land site dies under a landing squadron (Airfield3 destroyed
in USNOS). It is recorded, not predicted further.

## 3. `007F3970`, the squadron Kill, and divergence D3

**Bytes, `007F3970`-`007F3A59`, `__thiscall(squadron; plane, flag)`, `RET 8`.**
1. The cause for `007F2FD0` is 5 when the flag is set, else `plane+70h` (`007F3985`-`007F3991`).
2. `007F2FD0(cause, 1)` adds 1 to the squadron table's `PlaneKillReasons[cause]`.
3. The plane leaves `+3D0h` (compaction, count - 1, slot zeroed), `plane+9D4h = 0`, `007ED260`, and `+3ECh = 1`.
4. At count 0 (`007F3A16`), `007F3A2A` runs `Kill(squadron, plane+70h)`. **The cause is the last plane's `+70h`, not the
   flag's 5.** Then `+36Ah = (flag == 0 && plane+C41h)`.

The callers:
- `007BCAEB` (OnDestroyed, flag 0);
- `007F3CE7` (`007F3BA0` TickAdvance, flag 0, after `Kill(plane, 5)` for a member with `+900h == 1` and `+904h` set);
- `008A2249` (Lua `SquadronLandAndKill`, flag 1, after `Kill(plane, 5)` at `008A223F`).

**cc9's caveat ("a member removed by landing, 007F3970 argument 1, is not modelled") has no outcome effect.**
- Argument 1 comes only from the Lua native `SquadronLandAndKill`. Its effect beyond flag 0 is `PlaneKillReasons` and `+36Ah`.
- No script in this installation reads `PlaneKillReasons`, and the host does not publish it.
- A landed plane's `+70h` is already 5 (`Kill(plane, 5)` precedes the removal on every path), so the squadron's KillReason is
  "landed" with either flag.
- The host's stock return (`:14251-14261`) removes the member from the record and kills it with cause 5. The `Dead` hold in
  `publish_unit_deaths_00929800` (`game_hosts_script_orders.cpp:4962-4984`) releases once no other member is unsunk.
- Measured, `cc9-gunnery36/local/g39_bb0_os.log`:
  - `plane #1.5` lands at 320.40 s, is held, and is published at 333.99 s with its wingman's landing (`:69805-69806`,
    `KillReason=landed`);
  - `Airfield1_sqn17` lands at 1441.88 s and is published at 1929.11 s (`:444843`).

**D3, the KillReason source.**
- The host publishes the fused squadron unit's `KillReason` from `kill_cause_00926d90(index)`, which is the wing-0 plane's own
  cause.
- The image uses the cause of the **last** plane removed.
- They differ when causes are mixed. Example: wing-0 is shot down (harm) and the wingmen land (landed) or exit (exitzone). The
  image says landed/exitzone and the host says harm.
- `Dead` itself is identical.
- Scripts that read `KillReason` compare it with `"exitzone"`: messagesender.lua:253 for `Exiters`, chg_1/chg_7, jm02. None of
  those registers a squadron as far as read, so this is low impact.
- **Adoption:** record the cause of the last member removed on the squadron record, and publish that.

## 4. Divergence D2: `SquadronLandAndKill` (`008A20E0`) has no host body

- **The script.** PRCP01's script `PRCPUS/usn_03_battle_of_santa_cruz.lua:1189` (`luaUS03EndReconMovie`, phase 3) calls it for
  every Enterprise squadron past the first two (`luaGetSlotsAndSquads`, commandhelpers.lua:2486).
- **The image** (`008A220F`-`008A2249`):
  1. `007F1B70` on every carrier and airfield frees the launch slot.
  2. Then, from the last plane down: `Kill(plane, 5)` and `007F3970(plane, 1)`.
  3. At count 0 the squadron is Killed with cause 5.
- **The host.**
  - `src/mission_lua_host.cpp:203` only registers the name.
  - `src/air_operations.cpp:227` holds an unwired reference model. Its `squadron_remove_plane_007f3970` also ignores `+C41h` in
    the `+36Ah` result and does not model the squadron Kill.
- **Measured.** `cc9-lua52/local/l55_E1_p01e.log:47186` has `host MissionLuaNative::SquadronLandAndKill [008a20e0]
  UNIMPLEMENTED`, `calls=1`. The order file had launched Enterprise_sqn02 and Enterprise_sqn03 at frames 6600 and 6620, so three
  squadrons were active. None of the other PRCP01 logs in that tree reach the call.
- **Predicted effect.** On a PRCP01 order file with three or more Enterprise squadrons up at phase 3:
  - the image deletes the extras (planes Dead with KillReason landed, slots freed);
  - the host keeps them attacking.
- **Ownership.** The dispatch is in `game_hosts_lua.cpp`, leased to cc13-fix3 during this pass. This is an adoption note.

## 5. Records

- Model: `src/airops_ver_cc13.cpp` and `include/bsp/airops_ver_cc13.hpp`, registered in `cmake/startup.cmake`.
- Scripts:
  - `local/v10_dispscan.py`: capstone displacement census of `.text`;
  - `local/v10_scnscan.py`: scene byte search.
- Ledger: `0084E010` was leased to cc9-lua52 (single address), so no function record was written for it. The tail model is
  recorded here only.

## 6. Open

- The touchdown geometry and the landing sequencer (`006CD240`) were not re-read. Both have their own cc9 pairs
  (SQUADRON_LAND_TASK 5c, 5ag), and carrier recovery is measured end to end in 5gp.
- `+228h`'s readers were not traced.

## 7. D2 wired: `SquadronLandAndKill` (`008A20E0`), packet cc13-fix4

- **Commit** `53377c98e`, switch `kLuaSquadronLandAndKillBound` (`include/bsp/game_hosts_lua.hpp`), committed **OFF**. The
  row is not handled while it is off, so the native stays `UNIMPLEMENTED` and nothing changes byte for byte.
- **Binding.** `GameMissionLuaHost::run_squadron_land_and_kill_008a20e0` (`src/game_hosts_lua.cpp`) runs the existing model
  `squadron_land_and_kill_008a20e0` (`src/air_operations.cpp`) rather than a second copy. The model now takes the narrow
  `SquadronLandAndKillHost` (its two call sites, `release_squadron_slot` and `kill_plane`), which `AirOperationsHost` derives
  from, so no unrelated virtual had to be stubbed.
  - `007F1B70` -> `006C65B0` runs on every deck in the registry with the squadron's slot handle (`squadron_unit + 1`, the
    entity id, the same value the stock return passes at `007F1D4E`), **before** the kills, in the image's order.
  - Each live member, last to first (at most five readable, `008A2232`), is killed through the gunnery host with cause 5,
    skipping a member already dead (`00926D90`'s `+5Fh` return).
  - `kKillListenerHarmOnlyBound` (cc13-ver8): the cause 5 is recorded in `lua_kill_cause_`, so once that switch is on
    these deaths do not report `kill` (`007F3B19 CMP [unit+70h],1`). With it off they report as every Lua Kill does today.
  - `007F3970`'s removal and the squadron Kill (`007F3A2A`) are the units host's existing paths: the death sweep
    (`007BCAA0 -> 007F3970`) removes each dead member on the next motion step, and the fused squadron slot's `Dead` is held
    until the record's live count is 0.
- **Adoption note (cc9, `src/game_hosts_units.cpp`, leased to cc9-lua52).** The published `KillReason` of these planes
  will be `harm`, not `landed`. The gunnery kill ignores its cause, and the units host's `note_kill_cause` (`:2625`) is
  private to its `Impl`. **Missing:** a public `GameUnitsHost::note_kill_cause_00926d90(std::size_t unit, int cause)` that
  forwards to `Impl::note_kill_cause`; the binding's `kill_plane` should then call it with 5 before the gunnery kill, as
  the stock return does at `:14259`. No script in this installation was found to read the KillReason of a PRCP01
  Enterprise squadron, so this is presentation/scoring only.
- **Prediction when ON.** On PRCP01 order files with three or more Enterprise squadrons up when `luaUS03EndReconMovie`
  runs (`usn_03_battle_of_santa_cruz.lua:1189`), the extras are deleted at that call: their planes go `Dead` (KillReason
  `harm` until the adoption note lands, `landed` after), the squadron goes `Dead` once its last plane is swept, and its
  launch slot drops to cooldown (state 1, 5.0 s), so the deck can launch again. Today those squadrons keep attacking.
  This may explain part of cc9's SQUADRON_LAND_TASK 5gp item 3. The campaign copy of the same script,
  `COTP-USN/usn_03_battle_of_santa_cruz.lua:1189`, makes the identical call, so the campaign mission that loads that file changes
  the same way; no other `.lua` in this installation calls the native.
- **RNG.** The binding draws nothing. The deleted planes stop their per-tick draws and the freed slot can launch earlier,
  so on PRCP01 every draw after the call (shared stream `00BD2F10` and the per-unit streams) can shift. Pair PRCP01 runs
  only option-on against option-off from the call onwards.
- **Status.** Reconstructed (model) and wired; build-tested; not game-validated (no run, by rule).
