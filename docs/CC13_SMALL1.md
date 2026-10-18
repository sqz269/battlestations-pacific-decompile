# cc13-small1: undone rows of segments 0-4, 14, 17, 18, 22 and 25

The packet covers the cc13 lead tree's `local/cc13_undone_small1.txt`: 156 functions with no
reconstruction record and no tag. Each row was checked against rule 1 with
`git grep` over src/include, `bsp.py lookup` and `docs-for`, and against
`tools/raw_ownership_cc13.py`. The first pass (c9c34a3a8) only added tags. In a
second pass, cc9 released ten of the cc9-lane bodies under the gameplay terms,
and they are reconstructed in `src/cc13_small1.cpp` (see "Reconstructions").
Nothing is wired.

| Group | Rows | Outcome |
|---|---|---|
| Template bodies (STL `_Tree`/vector/range, map constructors, iterator wrappers, insert-n) | 119 | Tagged `stl_shape`, `reviewed`, evidence `cc13small1_stl` |
| Template bodies in segs 2 and 14 (sort internals over (ptr, float) pairs, observer-handle ranges, 8-byte vector insert-n, ref-pointer add-ref) | 18 | Tagged `tmpl_shape`, evidence `cc13small1_tmpl` |
| Catch funclet 00444252 (`_free`, then a rethrow through 00BF6885) | 1 | Tagged `stl_shape`, evidence `cc13small1_catch` |
| 00403560 MSVC `vector constructor iterator` | 1 | Tagged `cg_array_ctor_helper`, evidence `cc13small1_cg` |
| Already reconstructed (the list was stale): 005D8B80, 005D8FC0 | 2 | src/cc13_seg62L.cpp (landed as 84ba2e0b2) |
| Leased by cc13-front7: 005DF510, 0061B0F0 | 2 | Not touched |
| Released by cc9, reconstructed: 005CD0F0, 005CD9C0, 005CDB00, 005CDC50, 005CDD70, 005CDF30, 005CDF40, 005D36B0, 005DECA0, 00565D80 | 10 | `src/cc13_small1.cpp`, build-tested |
| cc9 keeps: 0043FA00, 00428420, 00428800 | 3 | Not touched |

These are bind-only under the 2026-10-16/17 library ruling.

## Evidence for the tagged rows

The rows already carry shape names from earlier sweeps. For every row whose
name did not start with a plain `STL_`, the callees were listed from the index:
- The iterator wrappers (00443C30/40, 00443E00/10, 0044A180/90, 0044AA20/30)
  each call one iterator next/previous row.
- The map constructors call only an allocate-empty-node row, which calls only
  operator new.
- The insert-n bodies call only the allocator, the range copy/fill rows, the
  xlen throw and `_free`.
- The seg 14 sort rows call only one another.
- The observer-handle ranges call only `BSP_Observer_RegisterPair` and
  `UnregisterPair`, or the handle constructor.

Some rows carry `BSP_` names (the soldier-loop iterators, the panel
palette/character/sequence maps, `BSP_LuaObjectVector_Tidy`,
`BSP_WeatherFloatMap_Construct`). They are still template instantiations whose
element types are game records, so they are tagged with the rest.

00441210 `BSP_LuaObjectVector_Tidy` is a host hook in existing reconstructions
(`cc13_gui1`, `cc13_seg52b`, `cc13_sweepA`). That is enough, so it does not
need a body of its own.

## Left alone (cc9 keeps)

| Address | Reason |
|---|---|
| 0043FA00 (copy-construct-at of a Lua object) | Inside the cc9 span 0043F000-00443010 and unnamed |
| 00428420, 00428800 BSP_Explosion_NotifyBlastSites | Blast-site notification is cc9 gameplay (docs/CC13_SEG1.md, CC13_LEFT61.md); `src/blast_damage.cpp` calls 00428800 through a hook |

## Reconstructions (released by cc9)

The code is in `include/bsp/cc13_small1.hpp` and `src/cc13_small1.cpp`. The host
is `Cc13Small1Host : Cc13SweepB11Host`. It reuses the chain's hooks for 00442190,
004D6480, 00BF65AC, 004BC410, 004BEC00, 004C43C0, 00425F40, 00506C80, 00771F20
and 004DB030, and adds the rest.

Rule 1: none of the ten had a body in src. cc9's files only call them through
hooks:
- 00565D80 from `frontend_seg12_17_cc13` and `seg23_cc13`;
- 005DECA0 from `frontend_menus`;
- 005CDB00 and 005CDD70 are declared in `scene_renderer_cc13.hpp`;
- 005CD0F0 is declared in `frontend_seg16_24_28_cc13.hpp`.

So there was nothing to verify-and-record, and no second copy exists. Each body
was transcribed from its full export listing.

| Address | ABI | Body |
|---|---|---|
| 005CD0F0 BSP_MovCam_SpectateUnit | thiscall(screen, unit), RET 4 | 004CC460([00E198C4], 2Bh, unit); 005CC000(screen); 007A6910([screen+30h], unit) |
| 005CD9C0 HudMovieScreen_Enter | thiscall(screen), RET | 005CD1A0(screen, [00E188D8]) when byte +20h is 0; 005CC170(screen); game+61Dh = 1; park list, flag 1 |
| 005CDB00 HudMovieScreen_Exit | ECX unread, RET | 00B0D0B0([00F8D39C], 0.0f); 00A933F0(input, 1Eh, 0); game+61Dh = 0; 00A933F0(input, 3, 0); park list, flag 0 |
| 005CDC50 HudScreen38_ClearCameraMover | ECX unread, RET | 004BC410(game, null); park list, flag 1 |
| 005CDD70 BSP_HudEngineMovieScreen_Exit | ECX unread, RET | 00B0D0B0(.., 0.0f); a kind-4 cutscene player at game+38h is destroyed (0045D930) and freed (00BF65AC), and the slot cleared; park list, flag 0 |
| 005CDF30 / 005CDF40 | thiscall(screen, vector), RET 4 | 004D6790(vector, &screen+30h / +18h) |
| 005D36B0 BSP_PromptResult_Surrender | fastcall(result), RET | result 1: 00771F20(game+1EF0h, [game+18C8h], 1), then a tail jump to 004DB030(game) |
| 005DECA0 BSP_MultiGameLobby_RefreshMapIfChanged | thiscall(screen), RET | See the adoption note below |
| 00565D80 BSP_HudLimboScreen_PollRespawnInput | ECX unread, RET | Unless game+5D4h == 0Ch, the first pressed action of E1h, 8Dh, 8Ch, 8Eh, 8Fh tail-jumps to 006485A0([[00E198C4]+40h]) |

The "park list" step builds a local `std::vector<int>`, pushes 36h, 37h, 38h,
53h, 33h, 55h, 47h and 5Ah (one push_back each). Then the handler itself passes
the list and the flag to the widget park routine, and releases the storage. The SEH unwind states (00C73738, 00C73758, 00C73778,
00C73798, 00C74988), which release the locals on a throw, are not modelled.

### Adoption note: 005DECA0 and the mission statistics

005DECA0 returns early in any of these cases:
- the local player index game+18C8h is negative;
- the slot game+18ECh is outside 0..7;
- there is no current mission (game+5FCh);
- the mission's +914h byte is 0;
- screen+2D4h/+2D8h already hold this mission and the slot's party.

Otherwise it does the following, in order:
1. Stores the mission and party at screen+2D4h/+2D8h.
2. Fetches the lobby mission record with 005D8650.
3. Calls 004C89A0(game, record).
4. Calls **00626930(record)**. EDX holds the party but is not read.
5. Builds a local text block from mission+920h and the record's per-party
   string vector at +154h*party + B8h + 64h, with +18h set to (party == 1).
6. Publishes the block with 0057D060 and releases it with 004D2A80.

00626930 is the per-mission statistics reset. It has four call sites: 004DFD0A
(BSP_Game_LoadMissionScene), 0058BF21, 005C5735 and 005DED75, the lobby-side
call in this body. The reset clears, in this order:
- the strings 00E19798 (mission name, from record+08h) and 00E197A0
  (debriefing text, from record+38h);
- 00E196CC, 00E196DC and 00E196EC (vector erase 004954F0);
- 00E196FC, 00E19710 and 00E19724 (in-place reset 005F6190);
- 00E19738, 00E19748 and 00E19758 (vector erase 006226F0);
- 00E19768, 00E19778 and 00E19788 (vector erase 004954F0).

This is the table in `include/bsp/mission_briefing_start.hpp`.

**Fields cc9 models:** none concretely. `MissionStatsBinding` in
`src/game_hosts_mission.cpp` routes all fourteen effects to
`log.unimplemented` (`set_stats_mission_name`, `set_stats_debriefing_text`,
`clear_stats_container`). So a host that wires 005DECA0 would reset no modelled
state today.

For the WINS re-time: in the image, every lobby map change (and every party
change for the same map) clears the twelve statistic containers and both
strings. A re-time that reads stats after a lobby change would see them empty.

Other notes:
- Live in a normal mission? No. It is a multiplayer lobby path. Its caller is the
  lobby screen through the hook in `frontend_menus`.
- It makes no direct draw from the shared RNG 00BD2F10 or 00BD2F40. As for
  00565D80, a static call-graph walk (8 levels, 228 functions) reaches neither;
  virtual calls are not followed.

### RNG note: 00565D80

00565D80 makes no direct RNG draw. A static call-graph walk was run from
00565D80, 004C43C0 and 006485A0 (eight levels over the index's call table;
virtual calls are not followed). It reaches 199, 152 and 195 functions, and
none of them is 00BD2F10 or 00BD2F40.

00565D80 runs only in the limbo screen, and only when one of the five actions
was pressed this frame.

## Files

- Tag rows appended to `config/tags/00400000`, `00410000`, `00420000`,
  `00430000`, `00440000`, `00450000`, `00520000`, `00550000`, `00560000`,
  `00600000` and `00610000.jsonl` (139 rows).
- `config/tags/evidence.json`: four new ids.
- Second pass: `include/bsp/cc13_small1.hpp`, `src/cc13_small1.cpp` (registered in
  `cmake/startup.cmake`), and ten function records in `config/reconstruction/00560000`,
  `005c0000` and `005d0000.jsonl`.
