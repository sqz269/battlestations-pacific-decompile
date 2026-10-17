# CC13_VERIFY9: application host labels and the rest of the approach chain

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify9 on 2026-10-17 against
base d05a3550e. It covers two things. Part A is `src/game_hosts.cpp`, the application shell host
(startup, frame loop, shutdown). Part B is what CC13_VERIFY7 left uncovered:
`src/ship_ai_approach_update.cpp` 009E6E80, 009E9190, 009E76D0, 009F1F47, 009F2124 and 009F239A
(`:652-1058` and `:1196-1352`), and the 009E76D0 driver body in `ship_ai_ring_scan.cpp:328-405`.
No code was changed and no ledger records were added. Every line number below is against that base.
`ship_ai_approach_update.cpp` and `ship_ai_ring_scan.cpp` have not changed since CC13_VERIFY7's base
44242d5c7, so its line numbers still apply. "Image" means battlestationspacific.exe, read with
`bsp.py disasm-raw` (disk bytes) and `bsp.py ghidra decompile`. Constants were read from the PE with
`tools/pe_const_read.py`, and with an exact-bit comparison script
(`local/verify9_constbits.py` in the worker tree; it is not committed).

Excluded, as briefed: the +490h max-range sites `ship_ai_approach_update.cpp:728` and `:1039`
(cc9-ships56). Earlier reports are not repeated here. That covers everything in the CC13_VERIFY1..7
REAL lists, and from VERIFY7 the 1-ulp `kApproachThrottleSeedX1` and the ring-scan winner, verdict and
normalise helpers.

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, width with no effect, ordering within a step, presentation,
  NaN-only branch polarity, or 1-ulp precision.
- MISMATCH REAL: wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but the premise no longer holds.
- UNRESOLVED.

## Summary: MISMATCH REAL (sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | `ship_ai_approach_update.cpp:801` takes the side arm when +1208h is CLEAR and the mode is 0; `game_hosts_ship_ai.cpp:6452-6458` therefore skips the mode-0 write of +1204h | 009E73C9 `TEST BL,BL` / `JE 009E7400`, 009E73D0 `CMP [+1234h],0` / `JNE 009E7400`: the side arm needs +1208h SET. 009F275B..009F2789 write +1204h for mode 0 | yes: every ship attackmove on a ship target (the latch picks mode 0 for it), on all four ship rows JM08 / USN01 / USN04 / USN13 | none directly; the bearing and throttle changes shift later draws on 00BD2F10 |

### R1 The 009E6E80 side arm is inverted

The tail of 009E6E80 picks a side weight for the 60 slot scorers and may write the throttle gate byte
nested+1204h. The image keeps the byte +1208h in BL (loaded at 009E7345 and not changed across the
00419010 call):

    009E73C9  TEST BL,BL
    009E73CE  JE   009E7400          ; +1208h clear -> general arm
    009E73D0  CMP  [ESI+1234h],0
    009E73D7  JNE  009E7400          ; mode != 0    -> general arm
    009E73D9  ...                    ; side arm: weight pi or 0, +1204h = 0 / 2

Ghidra's decompile reads the same way: `if ((char)iVar3 == '\0' || param_1[0x48d] != 0) { general }
else if (0.0 <= local_124) { param_1[0x481] = 2; } else { param_1[0x481] = 0; local_100 = pi; }`.
The host's condition at `:801` is `!state.flag_1208 && state.mode_1234 == free_0`, the reverse on the
flag.

+1208h is always clear. `game_hosts_ship_ai.cpp:6870-6873` records that the vtable[22Ch] call at
009F31F3 is 006DFDB0, `XOR AL,AL; RET`. So in the image 009E6E80 never takes the side arm and never
writes +1204h. In mode 0 the gate byte comes from 009F275B..009F2789 instead:

    009F275B  CMP  byte [EBP+1208h],0 ; clear
    009F2772  CALL 009E5E30           ; target present and IsKindOf(6)
    009F2779  JNE  009F278B           ; -> +1204h = 0
    009F277B  CMP  byte [00E0E2FC],AL ; NavigatorLandAttackerStop's byte
    009F2781  JNE  009F278B           ; -> +1204h = 0
    009F2783  MOV  [EBP+1204h],1

The host skips that mode-0 write on purpose (`game_hosts_ship_ai.cpp:6452-6458`). Its comment says
009E6E80 rewrites the byte "because +1208h is always clear", and that is the inverted reading.

The latch 009F1F47 (`:1196-1222`) sets mode 0 for every ship target outside the submarine-hold case,
and for any target that is neither a ship nor a building. The differences are:

- Ship target. The image's gate is 0, so 009E6A90 applies no gate. The host's gate is 2 whenever
  goal range >= standoff (offset >= 0). With the heading error under 15 degrees (009E6D83), that
  caps the throttle at clamp((dist - 50) / 80, 0, 2) (`:633-639`), so host ships slow and hold at
  the standoff ring where the image's keep going. Inside the standoff the host's gate is 0, the same
  as the image's.
- Inside the standoff (offset < 0). The image's side weight is ((c + c*c) * 0.5 * pi + pi) * 0.5 with
  c = min(|offset|, 1) (009E7441..009E7469). The host's is pi. The two agree for |offset| >= 1 and
  differ within one turn radius inside the standoff. The weight feeds all 60 009E6870 slot scores and
  so the bearing that 009E76D0 picks.
- A target that is neither a ship nor a building. The image's gate is 1, so the limit drops to 0 once
  standoff > goal - 80 (009E6D24), unless 00E0E2FC is set. The host's gate is 0 or 2 by the offset's
  sign.

The fix has two parts: drop the `!` at `:801`, and let `run_lander_side_and_bearing` write mode 0 as
009F275B..009F2789 do.

## Part A: `src/game_hosts.cpp`

Census pattern:
`substitut|approximat|stub|simplif|not in the struct|host constant|record-only|placeholder|in place of|no producer|stand-in|fixed value|unimplemented|record\(|not modell|for now|TODO|Bound = false`,
plus `instead|not yet|neutral|skip|omit|not reconstruct|borrow|diagnostic`. The file has no compile-time
switches. It has eleven `log_.unimplemented` call sites and a handful of labels. None of it is gameplay:
the file hosts the application shell. The gameplay entry from here is `menu_->frame` (`:1140`).

| Site | Label / call | Verdict |
|---|---|---|
| `:999-1003` | `RendererHost::present_frame` unimplemented when `device_` is null | MATCH (dead). `create()` (`:985`) runs before the loop. A null device means startup failed, and the run stops there. |
| `:1066`, `:1075`, `:1179`, `:1189` | four profiler arms unimplemented when `profiler_` is null | MATCH (dead). `profiler_` is constructed at `:2710`, before `frame_host_` (`:2784`). The image has no such test because 004C1DD0 creates its singleton on demand. |
| `:1121-1126` | "The simulation spine the branch falls through to at 004e4d32 is not reconstructed", and `unimplemented("GameFrameControl::simulation_spine", "004e4d32")` on every frame | STALE LABEL. The spine is reconstructed and runs. `game_frame_control.cpp:332` covers the timing block 004E4D3E..004E4D94 and the drain. `mission_state_frame.cpp` covers 004E4DF4..004E551E. `GameMissionFrameHost::run_mission_frame_004e4a40` (`game_hosts_mission_frame.cpp:2986`) drives them from `menu_->frame` (`game_hosts_menu.cpp:1862-1865`). The call only inflates the unimplemented count by one per frame. |
| `:1127-1139` | close policy: `CloseRequestPolicy::online_owner` / `confirmation_owner` unimplemented | MISMATCH MINOR (UI path, never reached by a headless run). The online publication 00F8ABE8 stays null (`:2524-2527`), so `process_window_close_transport_004ca2f0` (`window_close_transport.cpp:75`) returns `missing_online_owner` on every close request. The request stays latched, and clicking the window's X never exits, even in states 1/2/4 where 004CA2F0 exits directly. The prompt owner also exists as a body (`RetainedWindowClosePromptCalls`, `window_close_transport.cpp`), but `bind_window_close_prompt` (`game_hosts.hpp:436`) has no caller. |
| `:1174-1176` | `update_loading_queue` unimplemented on every frame | MISMATCH MINOR (front end only). 00737A50 calls 004FDE20 then 00509190 on every frame. Both bodies exist (`native_loading_queue_owner.cpp:62`, `frontend_seg12_17_cc13.cpp:1316`), but nothing calls them. The image creates the queue singleton on the first frame through 004FDE20; the host never creates it. Only front-end previews post jobs (005072C0, 0050A950, 00504D20, 005055C0), and no host reconstructs those. `mission_scene_load.hpp:13-16` records that missions post nothing. |
| `:2505-2513` | power policy 00BED223 "gated until the actual normal-exit restore caller is established" | MATCH (labelled, deliberate). The premise holds: the body is concrete (`platform_control_messages.cpp:14`, `GameWindowHost::initialize_power_policy_after_render_entry_cache` `:887`), and no caller restores it on a normal exit. Note for the run harness: in the image, BED223 sets the active power scheme's VideoTimeoutAc/Dc to 0 (BED258/BED25E) and turns the screensaver off (SPI 11h, BED271). So the image never lets the display sleep, and that sleep is the cause of the renderer-init failures. The host leaves the user's power scheme alone and uses `SetThreadExecutionState` instead. |
| `:2524-2527` | Phase 5 `online_manager_initialize` 0073DC7C unimplemented, publication kept null | MATCH (labelled). The premise holds: the 00A40DF0 records say `application_adoption_open`. See the close-policy row for the one effect seen. |
| `:2539` | Phase 4 `renderer_resources` 00B14A10 unimplemented | STALE LABEL. 00B14A10 is reconstructed (`native_render_resources_construction.cpp:167`, recorded `...cold_constructor_terminal_verified`). An application wrapper exists, `GameNativeRendererApplication::construct_render_resources` (`game_native_renderer_application.cpp:541`), but no caller uses it. The effect is presentation only: the 6ACh resource owner with the `noise.dds` / `black.tga` fallback textures is never built. |
| `:2133-2139` | `application_construct`: "subsystem pointers at +4h..+14h are not reconstructed" | MATCH (labelled; pointers with no host reader). Not checked further. |
| `:2843-2846` | `singleton_teardown` 00737F80 unimplemented, "remains unbound" | MISMATCH MINOR (shutdown only). The premise holds, but `app_shutdown.cpp` holds the 42-step `shutdown_application`, including 00737F80 (`:20-23`), and nothing calls it. |
| `:490-491`, `:546-548`, `:2116-2125` | `--ai-drive` "diagnostic stand-in", `--order`, `--affinity-core` | MATCH (harness options, not image behaviour). |

Outside the file: `game_hosts_menu.cpp:1866-1869` keeps `kMenuPumpYieldsToMissionFrameBound` OFF with
the label "OFF: the second pass stays". Its own comment says that pumps every level-1 screen twice
per mission frame. It was not checked here. If a level-1 screen's frame script advances a timer, the
double pump would be script-visible.

## Part B: `src/ship_ai_approach_update.cpp` and `src/ship_ai_ring_scan.cpp`

These ranges have no substitution labels, so the pass is the unlabelled arithmetic. All six
approach entry points are live: `game_hosts_ship_ai.cpp` calls them at `:5983`, `:6192`/`:6215`,
`:6249`, `:6351`, `:6366` and `:6390`. The ring-scan driver `ship_ai_ring_scan_009e76d0` has no
caller outside its file.

### 009E6E80 `ship_ai_approach_choose_standoff_009e6e80` (`:652-843`)

| Step | Image | Verdict |
|---|---|---|
| override bytes, tune +1Ch, modes 1/3, kind-8 + 00827F70 (`:655-680`) | 009E6E8F..009E6F18 | MATCH, with one MINOR. The 009E6ECA `COMISS`/`JB` arm takes NaN to "continue", but the host's `!(override_range < 0)` stores the NaN as the standoff. The comment at `:664-665` says what the image does; the code does not do it. NaN-only. |
| mode 4 (`:682-697`) | 009E6F29..009E6FB3, 009E718A | MATCH: 1000 / r - 300, leader -200, high = low + 250, `current > high || low > current` -> stream-1 draw (low, high). |
| mode 2 (`:698-725`) | 009E700B..009E7124 | MATCH in flow; MINOR in constants. `kApproachTargetRangeHigh` 0.85, `kApproachLeaderShrink` 0.8 and `kApproachLeaderFloor` 0.9 are exact decimals in the header (`ship_ai_approach_update.hpp:200-202`), but 00CF0B58 / 00CE3D40 / 00D7A390 hold the widened floats 0.8500000238418579 / 0.800000011920929 / 0.8999999761581421. The relative error is about 3e-8, so the band edges `low` / `high` can land one float ulp apart. The edges feed the stream-1 draw (009E711D). |
| cruise band (`:726-735`) | 009E7138..009E719E | excluded (+490h). Its two constants carry the same widening: `kApproachCruiseLow` 0.92 vs 0.9200000166893005 at 00D04308, `kApproachCruiseHigh` 0.98 vs 0.9800000190734863 at 00D219C8. |
| 119-step scan (`:736-768`) | 009E71A5..009E72E8 | MATCH: seed `curve_base + 300`, x = 50, step = 25, weight 00419010(0, 2, 1, 1, primary / reference), floor `max(1, secondary)`, ratio, score, 119 steps. MINOR: the 009E72AE `FCOMIP`/`JB` skips the update when the compare is unordered, but `:762` `!(best < score)` takes a NaN score. NaN-only. |
| common tail (`:771-784`) | 009E6FBD..009E733D | MATCH: 00811A30(1.0) * 1.5, knee 250, clearance clamp through 00415510. MINOR: at 009E6FE4 `FCOMPI`/`JBE` an unordered turn radius keeps itself (009E72ED), but `:775` replaces it with 250. NaN-only. |
| offset, bias, span weight (`:786-798`) | 009E733F..009E73C5 | MATCH. Under 53-bit the host does `(goal - standoff) / turn` in one double expression where the image stores once, so the difference is 1 ulp at most. |
| side arm / general arm (`:800-836`) | 009E73C9..009E746D | **MISMATCH REAL: R1.** The general arm's arithmetic (with `kApproachSideWeightDcFormBound` ON) matches the bytes; 00CE3D28 is the float pi widened, the same value as `kApproachHalfTurn`. |
| slot scorers (`:838-842`) | 009E7473..009E74C2 | MATCH: 60 calls with {side, span, +11DCh, tune +4h}. |

### 009E9190 `ship_ai_approach_refresh_avoidance_009e9190` (`:849-959`)

| Step | Image | Verdict |
|---|---|---|
| modes 1/3 (`:854-863`) | 009E96D8..009E9720 | MATCH: 00419010(0, 0, pi, 0, 00438B10(angle, 0)) into each slot +3Ch. |
| timer and redraw (`:865-870`) | 009E91CA..009E9211 | MATCH, with one MINOR: at 009E91E4 `FCOMPI`/`JBE` an unordered timer skips the redraw, but `:868` redraws. NaN-only. The draw is stream 1 (2.0, 3.0) on both sides. |
| candidate walk (`:871-906`) | 009E9217..009E93A6 | Candidate source already reported (VERIFY2 R2). Otherwise MATCH: IsKindOf(5), skip the target, distance against (+494h + 200)^2 with JBE, duplicate walk on record +14h, a 124h record from 009E8360, append. Two MINOR precision points. The image stores dx and dz to float at 009E92A1 / 009E92B1 before squaring, while the host keeps them in double (`:884-885`). And it squares the radius unrounded from the x87 stack (009E92D5..009E92E1), while the host rounds it to float first (`:888`). Each is 1 ulp at most. The image also refreshes the unit's pose (009E927E) inside the loop. That is a lazy cache, and the host reads the position once (`:873`): MATCH. |
| accumulation (`:917-937`) | 009E93B0..009E9574 | MATCH: seed 00F87574 (zero), 009E6170 with radius 300, erase on false, 009E6240, products float-stored and then summed. |
| strength and heading (`:939-953`) | 009E95B5..009E966E | MATCH: (x^2 + y^2) + z^2 in that order, `> 1` with JBE. atan2 is float-stored at 009E95FF, then pi/2 minus it, with the wrap. 00419010(0, 0, tune +0Ch, tune +08h, length). MINOR (shared helper `ship_ai_approach_heading_from_delta` `:101`): the wrap at 009E9617 `FCOMPI`/`JBE` leaves a NaN heading alone, but the host adds 2*pi to it. NaN-only. |
| slot weights (`:955-958`) | 009E9674..009E96C1 | MATCH: signed 00438B10(angle, heading), 00419010(0, strength, pi, 0, diff). |

### 009E76D0 `ship_ai_approach_select_slot_009e76d0` (`:965-1058`)

| Step | Image | Verdict |
|---|---|---|
| phase (`:971-972`) | 009E76D0..009E76F4 | MINOR, no effect: `kApproachPhaseRate` is 0.2 in the header (`:240`), while 00CE3D10 holds 0.20000000298023224. Its only reader is the discarded wobble. |
| scores, maximum, normalise (`:974-998`) | 009E76F9..009E7808 | MATCH: running maximum seeded 0 with JBE; normalise only when 1.0 > maximum, dividing by the maximum left on the x87 stack. |
| accept/reject (`:999-1011`) | 009E780A..009E79AA | MATCH in fields: keep clears +40h; reject zeroes +2Ch / +38h / +3Ch and writes -0.0 - tune +4h into +30h. MINOR: `kApproachSlotKeepThreshold` is exact 0.85 (`hpp:243`), while 00CF0B58 is the widened float 0.8500000238418579. A score of exactly 0.85f is rejected by the image and kept by the host. |
| winner (`:1014-1025`) | 009E79CA..009E7C28 | MATCH: totals as avoid + normalized + penalty + bearing + evade, strict `>`, first maximum wins. |
| wobble (`:1027-1029`) | 009E7C3E..009E7C78 | MATCH. sin * 0.0 goes through 00438AA0 and is popped. 00438AA0 is a pure wrap. |
| goal override (`:1031-1052`) | 009E7C72..009E7EAE | MATCH in flow: 00438B10(+11ECh, heading), window 00419010(0.2, 400, 1.2, 50, abs turn), flip by pi through 00438AA0. MINOR at `:1047` (next to the excluded +490h read at `:1039`): the image float-stores speed - window at 009E7E71 before the double compare at 009E7E7D, but the host compares the unrounded double. The ring-scan copy (`ship_ai_ring_scan.cpp:324`) rounds it correctly. Edge only. |
| commit (`:1054`) | 009E7EB4..009E7ECB | MATCH. |

### 009F1F47 `ship_ai_approach_mode_latch_009f1f47` (`:1196-1253`)

MATCH. Ship branch 009F1F47..009F1FFE: scale 2.1 / 1.9 by the previous mode, turn radius times the
scale against the range as a double with JBE, mode 1 with the 00415510 minimum into +11F0h, else
mode 0. Other branch 009F2003..009F211A: no building gives 0. Same side gives EDI. A troop lander
gets mode 3 or 4 by `r + max(300, 2t) >= range` (009F20A4..009F20B2 JB), then the timer reset when
0 > +121Ch. Otherwise the same side or side 2 gives EDI, and everything else 0.

STALE LABEL: `include/bsp/ship_ai_approach_update.hpp:54-58` and `:62` say mode 1 is stored at 009F2022
and 009F2112 and that "Value 2 ... is never assigned ... its producer is unknown". On the path into
009F2003, EDI is 2 (009F1E25 `MOV EDI,2`, before the `JE 009F2003` at 009F1E30). So 009F2022 and
009F2112 store mode 2: a building target on the unit's own side, or a neutral building that is
not a troop lander's. The code at `:1229` and `:1250` already does that. The labels `free_0` "no
engagement geometry" (`:60`) and `standoff_4` "the ordinary approach mode" (`:64`) are also wrong.
Mode 0 is the ordinary ship-on-ship approach, and mode 4 is a troop lander's approach to a hostile
building. These labels are the reasoning behind the R1 skip in `game_hosts_ship_ai.cpp:6452-6458`.

### 009F2124 `ship_ai_approach_retarget_head_009f2124` (`:1259-1266`)

MATCH: 009F2124..009F2161. Latch +11D6h; the timer becomes 1.0 only when 1 > timer, ordered, and the
store is at 009F2161.

### 009F239A `ship_ai_approach_retarget_ring_009f239a` (`:1269-1352`)

MATCH: 009F239A..009F2727. The goal is stored to +1228h. The zone comes from 004178F0 on the goal xz,
which is `[ESP+70h]` written at 009F1CF6 from brain +B2Ch / +B34h. The origin is 009E6120, the same
goal, with y = max(50, class +A8h) + goal y (00415550). The reach is max(R - 600, R * 0.6f) (+490h,
excluded), or the capture range - 600 for a mode-2 building (009F2505..009F2513). The far point is
dir * 1e5 plus the goal, and 00416DD0 takes four pointer arguments, of which the host passes the first
three; the fourth (`[ESP+88h]`) has no reader in the loop. The probe is hit + dir * 100 at y 60.
00904400(44h, origin, probe, segment, 0) rejects a slot. 00414C60 (x^2 + z^2 with the 1e-10 cutoff,
bit-identical to the host's helper) gives the reach and best tests, and the point is
hit + dir * 10 with y = dir_y * 10 + 0.

### `ship_ai_ring_scan.cpp:328-405`, the unwired 009E76D0 driver

The flow matches 009E76D0 as described above. It has no host caller, so none of this is live. It
has two NaN-only polarity slips that the approach-update copy does not have. `:356`
`!(scores[i] <= best)` lets a NaN score replace the maximum, where the image's JBE at 009E776E skips.
`ship_ai_ring_scan_reverse_goal_009e76d0` (`:325`) returns true for an unordered compare, where
009E7E81 JBE skips. Both are MINOR. `kShipAiRingScanOneFifth` is the exact widened value.

## Constant widths beyond the brief

The constant-width sweep checks each constant against its float and double readings with a
tolerance, so it passes a header double written as the decimal 0.85 when the image holds the widened
float 0.8500000238418579. An exact-bit comparison over `include/bsp/*.hpp` finds 28 such widened
doubles. In the approach header there are seven: the five above plus `kApproachPhaseRate` and
`kApproachTurnDeadband` (-0.1 vs -0.10000000149011612 at 00CE3928, read by 009E5E90). There are also
these value differences in live gameplay headers. Their load sites were NOT read, so each is a
candidate, not a verdict:

| Header | Constant | Header value | Image | Note |
|---|---|---|---|---|
| `torpedo_aim_tick.hpp:66` | `kProbeGateY0` | 0.872665f | 0.8726646304 (float, 00D05B40) | about 6 ulp; torpedo aim 009D19B9 (URGENT class, but precision only) |
| `torpedo_aim_tick.hpp:68` | `kProbeGateY1` | 0.349066f | 0.3490658700 (float, 00CE398C) | about 4 ulp |
| `torpedo_aim_tick.hpp:88` | `kPitchClampHi` | 0.872665f | 0.8726646304 (double, 00D057E0) | the same DEG(50) rounding |
| `pilot_controls.hpp:84` | `kPilotPitchHalfRange` | 0.5236f | 0.5235987902 (float, 00CEC724) | about 20 ulp |
| `pilot_command_path.hpp:108` | `kPilotCmdPitchZeroAngle` | 0.1745329f | 0.1745329350 (float, 00CE3990) | 1 ulp |
| `plane_ground_ops.hpp:105` | `kPlaneGroundYawTurnNumerator` | 0.872665f | 0.8726646304 (double, 00D057E0) | DEG(50) rounding |
| `ship_ai_sector_scan.hpp:235` | `kShipAiSectorSideBias` | 0.43633234500884056f | 0.4363323152 (double, 00D1F6C0) | 1 float ulp |

`kPitchClampLo` (1 ulp) also shows up in the comparison, but it is already known from the
constant-width sweep work.

## Uncovered

- Nothing in the briefed ranges.
- Not checked: the load sites of the cross-header constant candidates above, and the binding side of
  the 009E9190 / 009F239A host calls in `game_hosts_ship_ai.cpp` (CC13_VERIFY2's file).
