# CC13 sweep A3: segments 0..14 leftovers, third pass (cc13-sweepA3)

Packet cc13-sweepA3 (2026-10-16) takes the 34 bodies docs/CC13_SWEEPA2.md left
"for a later packet", smallest first. Excluded by the lead: 004A0D20 (a 72 KB
listing); 004C3A80 and 004C74C0 (cc9) were already outside that list. Source: `src/cc13_sweepA3.cpp`, header
`include/bsp/cc13_sweepA3.hpp`; nothing is wired. `SweepA3Host` extends
`cc13_sweepA2::SweepA2Host` (and so `SweepAHost`); sweepA and sweepA2
reconstructions are called directly where the image calls them (004518B0,
004423A0, 0045D930, 00525F80, 004CB160). Descriptive names are hypotheses,
not recovered symbols. Status of every reconstruction below: reconstructed
and build-tested (MSVC Win32, compiled into `bsp_core`); none is
fixture-tested, ABI-compatible or game-validated.

Outcome counts for this pass (18 of the 33 listed bodies other than
004A0D20 closed): 15 reconstructed, 1 existing implementation recorded,
1 out of scope (STL), 1 cc9 lane; 15 left (listed at the end).

## Method

- Each address was searched in `src/`, `include/` and `docs/` and with
  `bsp.py lookup`; none of the 15 had a reconstruction record or a body in
  `src/` (the hits were host hooks or descriptive notes).
- Bodies were read from the exported listing with the stack offsets tracked
  at every push and callee RET (the frame offsets are written in the
  comments where a slot is reused). x87 register forms were taken from the
  bytes (`local/sweepA3_dw.py b`): in 0051EAA0, DE F1 (0051EBAF) and DE F2
  (0051EBD0) are FDIVRP (1/divisor), DE EA (0051EBDE) is ST2 = ST2 - ST0 and
  DE E9 (0051ECFD) is ST1 = ST1 - ST0; in 005220C0 the FMULP/FADDP forms are
  commutative. Constants were read from the PE with their width.
- Every call the image makes happens at the same point through a named
  `SweepA3Host` / `SweepA2Host` / `SweepAHost` method (STL instantiations are
  host methods named by address); memcpy, memmove_s, strlen, atol and
  _stricmp are bound directly. Checked-iterator reports (00BF6713) are host
  calls at the image's points.
- Common simplification, stated once: SEH unwind actions are not modelled.
  Shared RNG: no reconstructed body calls 00BD2F10 directly.

## Reconstructed

| Address | Function | ABI | Notes |
|---|---|---|---|
| 0051EAA0 | `hud_binoculars_step_input_axes_0051eaa0` | `__fastcall(ECX value*, EDX limit*; divisor, dt, active*, rising*, falling*, keep_limit)`, RET 18h | Input bank b = [004BEC00()+4]; analog = 00A92090(b+15C0h) or (b+2A00h); toggle = 004C43C0([00E188A8], E0h) or rising and falling; axis b+15E4h == 0 handles the toggle (limit > 0.25f lowers it by 0.75 unless keep_limit; otherwise limit 1.0f and the binoculars model [[00E198C4]+84h]+20h slot +34h(0)); otherwise analog mapping r + (1 - r)(diff + 1) * 0.5 or 0042AC60 steps (0.5 / -0.5 * dt); then rising/falling move by dt * 5.0 clamped at limit. |
| 004DD7C0 | `game_complete_leaderboard_read_004dd7c0` | `__fastcall(ECX game)` -> EAX, plain RET | Overlapped read state game+7058h: 8000FFFFh idle, 3E5h pending, 80004005h failure (00A4D42E / 00A4D422), else results +7054h: no views -> prompt "FE_pc.xsm_requireslive" (00531B00 with 2, text, 2, 0, 0, "", 0.0f, 0, "" by value, 1); views -> [[00E198AC]+6Ch]+14Ch, 004DCA80 and per 30h row 004BCD20 + 004C5E60 / 004C5E20; both end in 00564F60 and 004D9CA0. |
| 004CCE50 | `game_begin_engine_movie_004cce50` | `__thiscall(ECX game)`, plain RET | game+38h = 00460B80(operator new(74h)); path = stem of [game+5FCh]+90Ch before the last "." (00467CF0, 00469840) + ".ema"; 004602C0; state 5 -> 00B72030, list [00E188B4] slot 0, 0045D930 + free; else 0045DB40 and 004CC460([00E198C4], 2Dh, 0). |
| 0045ECA0 | `movie_spline_read_point_0045eca0` | stdcall-shaped (tokenizer, owner), RET 8 | Point { PointType, ActivateTime, Comment, PointPos, optional Alpha (peek + 00438E10), InTangent, OutTangent } through 008D9930 / 008D9980 / 008D9AD0 / 008D9B40, then 0045B420(owner, pos, time, in, out, type, alpha, comment by value). |
| 0045F600 | `engine_movie_stop_0045f600` | `__thiscall(ECX movie)`, plain RET | +4 = +0, +60h = 4; dialog pause 00A77760(1); sound level reset; [00F8BBCC] +4(0.0f); 00735B50; 0046B730 and luaPrecacheUnits when [00E18680]; 004518B0; twelve class-table resets; collectgarbage; 004C3CB0; luaStageInit check, then the "4_" + short-name file block (00BE0A30 / 00BDCB30) around luaStageInitMulti / luaStageInit; HUD model hides, 00A77760(0), 00B72030, 004B46B0. |
| 00526720 | `unit_pick_cycle_target_00526720` | `__thiscall(ECX screen)`, plain RET | 0071C060(2, player) gate; refresh (00526180) or rebuild (00525170, 00525F80); advance +D4h modulo the list, first unit accepted by player +16Ch gets 0077D600 with the attackmove (00E08F78) or default (00E08EF8) command; none -> clear list (00524DC0) and +D4h = -1; 004254B0 debug line. |
| 004D32A0 | `game_nil_script_added_globals_004d32a0` | `__fastcall(ECX game)`, plain RET | Walks the Lua globals (00B67080 / 00B67190 / 00B66420); functions whose name is not in the set game+1930h (004C7F10) are collected (004CDC20) and, last first, nilled with name + " = nil" through 00B66C60; set emptied (004CEC60), vector freed (00427110, 00BF6989). |
| 004D2BB0 | `game_destroy_world_004d2bb0` | `__thiscall(ECX game)`, plain RET / tail JMP 0060D3E0 | The world teardown: input context 1Dh, hidden records, please-wait screen, the owned world slots in the image's order, the [00F87560] vector, point effects +19D8h / +19E4h, effect manager, listener reset to identity, +19F4h / +19F8h / +19ECh, 00AD7B90, 004C7DD0, 004D2B10, the eight participant slots, 0060D3E0. |
| 00487470 | `traffic_runtime_remap_side_templates_00487470` | `__fastcall(ECX runtime)`, plain RET | Mode A (+16Ch == 0) / B; soldier templates (+F4h) remapped through the config tree +40h with 004B1400; vehicle templates (+E4h) through +4Ch with 0047B3C0(00965480). docs/SCENE_TRAFFIC_RUNTIME.md describes the same remap; the vehicle "empty name" rule is narrower than that doc says (a null compare matches only a null or empty class name). |
| 0050B440 | `tactical_library_rebuild_entry_list_0050b440` | `__thiscall(ECX screen)`, plain RET | Clears +444h (00505DE0); the 99999 selection guard (00A9CA00, +28h text, atol); then pushes the entries (004FB090 / [+5B0h] for id -1) of every record id not seen before (0050AD00, 00441F30). The found byte is never reset. |
| 005220C0 | `unit_line_of_sight_from_viewer_005220c0` | stdcall-shaped (unit), RET 4, AL | Rays from the viewer [[00E188A8]+1ED4h] (009043A0 with 004B4B00's vehicle ignored, mask 44h): direct, then four lifted by 4..16 (only the last counts), then for kind 6 one lifted by [[unit+538h]+A8h] * 0.5; a ray passes with no hit, a self hit or a hit within 8 units. |
| 00456DD0 | `movie_track_bind_entities_00456dd0` | `__thiscall(ECX track)`, plain RET | 00925180 for +120h (squadron -> +3D0h, +11Ch, byte +61h) or an unshown warning string; per ReferPath action, +14h = 00925180(+18h) and another unshown warning string; the track's own iterator +114h/+118h with its checks. |
| 00443090 | `device_class_resolve_from_lua_00443090` | `__fastcall(ECX id, DL retain)`, EAX, plain RET | docs/NATIVE_DEVICE_CLASS_RESOLUTION_ORCH4.md's schedule: cached slot (+ InterlockedIncrement when DL), else DeviceClass[id].Type -> Got = true -> seven Type branches (allocation sizes and vptrs as that doc lists), class slot +8 reads the row, +6Ch = id, publish; unmatched -> 0. |
| 0045D480 | `movie_action_type_construct_0045d480` | `__thiscall(ECX record; 8 strings by value, 6 flags)`, RET 58h | Zeroes the eight strings, assigns them with the flags interleaved as the image stores them, releases the eight by-value arguments. |
| 0044B5B0 | `panel_sequence_describe_properties_0044b5b0` | `__thiscall(ECX sequence, sink)`, RET 4 | Property writes through the sink's +0Ch with the 00BD5680 default checks, the "sequence" and "panelStates" groups (+4 / +8). The tag-3 frame slots keep the previous entry's upper bytes, as in the image. |

## Existing implementation, recorded ledger-only

| Address | Implementation | Outcome |
|---|---|---|
| 004E5540 | `bsp::game_on_init` (src/game_entry.cpp) | Verified against the listing: the startup callback, controller and handler, the forced-logo check, the eight switch tests (with the dead CMP -1 and the repeated skipBriefings), the logo / title / scenario arms and their call order. Simplifications: it returns the game+5D4h value instead of storing it (the image stores 1 before the logo allocation and 0Ah before the 004D3ED0 push), and it reads game+7168h once. |

## Out of scope (vendor library), recorded by name

- 00460750: an STL vector insert-n instantiation for the movie records (sole
  caller 00460A20 STL_Vector_InsertOne; callees allocate 0045C220, xlen throw
  0045FCA0, uninitialised copy 0045FBA0, destroy range 004601B0, the record
  copy constructor 0045F110).

## cc9 lane (skipped)

- 004C7AA0 (closest-border-zone geometry, caller 0094BFF0):
  docs/CC13_SEGS_0_9_11.md line 125 leaves it to cc9
  (`include/bsp/plane_retreat_task.hpp`).

## Adoption notes

None: no reconstruction here replaces code in a gameplay host file. For the
record, cc9's engine-movie model in `include/bsp/game_hosts_scene_contents.hpp`
labels 0045F600's tail and 0046B730's kill pass as partial; 0045F600 is now
reconstructed here in full and can be compared with that model when cc9
picks it up.

## Left for a later packet

<!-- skipped-list -->
- 004D80D0 BSP_Game_UpdateMultiplayerInterface (359 instructions): not attempted (context budget); it calls 004C3A80 (cc9 lane).
- 0046B730 (272 instructions): not attempted; the engine-movie kill pass, register-mangled pseudocode, partly modelled by cc9 (game_hosts_script_orders.hpp).
- 00526180 (274 instructions): not attempted; the in-place cycle-list refresh with inlined observer-handle vector erase and sort (00526720 calls it through a host hook).
- 00487A70 (260 instructions, vtable slot at 00CE6854): not attempted; x87-heavy debris integration with matrix builds and ref-counted point effects.
- 004602C0 BSP_EngineMovie_ParseFile (367 instructions): not attempted (context budget).
- 00447510 BSP_GameDynamicsList_Add (377 instructions): not attempted (context budget).
- 004E2200 BSP_Game_RunConsoleCommandQueue (378 instructions): not attempted (context budget).
- 004E3AA0 BSP_Game_OnInit (387 instructions): not attempted (context budget).
- 004EA080 BSP_SceneClass_ReadStockLists (415 instructions): not attempted (context budget).
- 004507D0 BSP_PanelSequence_ReadProperties (434 instructions): not attempted (context budget).
- 0045DE50 BSP_MovieAction_Create (524 instructions): not attempted (context budget).
- 004E8BF0 BSP_SceneHeader_Assign (531 instructions): not attempted (context budget).
- 00459020 BSP_MovieEntityTrack_Update (588 instructions): not attempted (context budget).
- 0051CDE0 BSP_BriefingScreen_Enter (1269 instructions): not attempted (context budget).
- 0051FA50 BSP_CheatMenuScreen_BuildLines (1679 instructions): not attempted (context budget).
