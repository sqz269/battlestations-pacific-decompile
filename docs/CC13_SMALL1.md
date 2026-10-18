# cc13-small1: undone rows of segments 0-4, 14, 17, 18, 22 and 25

The packet covers the cc13 lead tree's `local/cc13_undone_small1.txt`: 156 functions with no
reconstruction record and no tag. Each row was checked against rule 1 with
`git grep` over src/include, `bsp.py lookup` and `docs-for`, and against
`tools/raw_ownership_cc13.py`. No body was reconstructed, wired or built.

| Group | Rows | Outcome |
|---|---|---|
| Template bodies (STL `_Tree`/vector/range, map constructors, iterator wrappers, insert-n) | 119 | Tagged `stl_shape`, `reviewed`, evidence `cc13small1_stl` |
| Template bodies in segs 2 and 14 (sort internals over (ptr, float) pairs, observer-handle ranges, 8-byte vector insert-n, ref-pointer add-ref) | 18 | Tagged `tmpl_shape`, evidence `cc13small1_tmpl` |
| Catch funclet 00444252 (`_free`, then a rethrow through 00BF6885) | 1 | Tagged `stl_shape`, evidence `cc13small1_catch` |
| 00403560 MSVC `vector constructor iterator` | 1 | Tagged `cg_array_ctor_helper`, evidence `cc13small1_cg` |
| Already reconstructed (the list was stale): 005D8B80, 005D8FC0 | 2 | src/cc13_seg62L.cpp (landed as 84ba2e0b2) |
| Leased by cc13-front7: 005DF510, 0061B0F0 | 2 | Not touched |
| cc9 lane, as earlier packets ruled | 13 | Not touched (see below) |

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

## Left alone (cc9 lane)

| Address | Reason |
|---|---|
| 0043FA00 (copy-construct-at of a Lua object) | Inside the cc9 span 0043F000-00443010 and unnamed |
| 00428420, 00428800 BSP_Explosion_NotifyBlastSites | Blast-site notification is cc9 gameplay (docs/CC13_SEG1.md, CC13_LEFT61.md); `src/blast_damage.cpp` calls 00428800 through a hook |
| 005CD0F0, 005CD9C0, 005CDB00, 005CDC50, 005CDD70, 005CDF30, 005CDF40 | In-game movie-screen wiring, left to cc9 (docs/CC13_FE_4.md, CC13_FE_10.md, CC13_SWEEPB.md) |
| 005D36B0 BSP_PromptResult_Surrender | On result 1 it calls 00771F20 BSP_Session_RequestPlayerSurrender, then tail-jumps to 004DB030; cc9's surrender path (CC13_FE_10.md) |
| 005DECA0 BSP_MultiGameLobby_RefreshMapIfChanged | Resets the mission statistics; the docs say "check with cc9 first" |
| 00565D80 BSP_HudLimboScreen_PollRespawnInput | Limbo respawn input, cc9 (CC13_FE_3.md, CC13_SWEEPB.md) |

The lead was told about these. If cc9 releases 005DECA0 or 005D36B0, both are
small enough to take later.

## Files

- Tag rows appended to `config/tags/00400000`, `00410000`, `00420000`,
  `00430000`, `00440000`, `00450000`, `00520000`, `00550000`, `00560000`,
  `00600000` and `00610000.jsonl` (139 rows).
- `config/tags/evidence.json`: four new ids.
