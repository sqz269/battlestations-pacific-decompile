# Segment 96 engine core: perf timers, profiler overlay, allocation report, VFS helpers (cc13-render5)

Worker cc13-render5 (Claude lineage cc13), second packet in tree `cc13-render4`. Names are
hypotheses, not recovered symbols. "Unreferenced" means no rel32 CALL/JMP and no absolute dword
equal to the address anywhere in the image (scan `local/r5_rel32.py`). Reconstructions are in
`src/engine_core_cc13.cpp` / `include/bsp/engine_core_cc13.hpp` and are **build-tested only**
(`scripts/build.ps1`, both existing CTests pass); nothing here is fixture-tested, ABI-compatible or
game-validated.

## Triage

Segment 96 (`00BD9220-00BE4410`, keywords startup, perftime, alloc, duplicate, rejected, srch) was
already mostly reviewed: 142 reconstruction records and 148 names before this packet, almost all
VFS (cc12/orch `native_vfs_*`, `docs/VFS_*`, `docs/NATIVE_VFS_*`), the frame job pool, the
allocation-statistics lifecycle and the stream readers. 104 functions were unnamed:

| group | treatment |
|---|---|
| perf counter / profiler (`00BE3240..00BE3BF0`) | named; 7 reconstructed |
| allocation report and platform/allocation vtable leaves | named; report reconstructed |
| stream vtable leaves and operator wrappers | named |
| VFS helpers already described in VFS docs but unnamed | named from those docs and the bodies; content-package removal reconstructed |
| STL allocators, iterators, tree/list init/destroy | named, not reconstructed |

Totals: **104 ledger names** (48 of them unreferenced), **9 reconstruction records** (8 complete,
1 partial), Ghidra names/comments applied. No Ghidra functions were defined.

Segment 95's shared gameplay RNG (`00BD2F10 BSP_Random_UniformFloatRange`, `00BD2FC0
BSP_RandomThreads_NextU32`) is already named and reconstructed (`src/native_particle_object_state.cpp`,
`src/random_threads.cpp`); this packet did not touch segment 95.

## Performance counter and profiler

| address | name | ABI | coverage |
|---|---|---|---|
| `00BE3310` | `BSP_PerfCounter_CalibrateTicksPerSecond` | plain RET, ST0 | complete |
| `00BE3360` | `BSP_PerfCounter_TicksToSeconds` | ECX int64*, RET, ST0 | complete |
| `00BE32B0` | `BSP_ProfilerCounterRecord_End` | ECX record, RET | complete |
| `00BE3300` | `BSP_ProfilerCounterRecord_Reset` | ECX record, RET | complete |
| `00BE3380` | `BSP_Profiler_ClearHistory` | ECX profiler, RET | complete |
| `00BE33E0` | `BSP_Profiler_FormatPerfTimes_Unreferenced` | ECX profiler, RET | complete (returns the lines the image discards) |
| `00BE3BF0` | `BSP_Profiler_DrawOverlay` | ECX profiler, RET | complete (renderer append as output quads) |
| `00BE37A0`, `00BE3800`, `00BE3780`, `00BE3BC0` | destructor family (singleton `[0109DB50]`) | | named only |
| `00BE3240` | `BSP_PerfCounter_Query_Unreferenced` | | named only |

**The tick scale `[0109DB48]` is ticks per second.** It was listed as an open question in
`docs/APP_FRAME_GAME_STATE.md`, `docs/APP_RUN_FRAME.md` and `docs/GAME_EXECUTABLE.md` ("zero in the
image, unit unproven"). Its only writer is the static initializer at `00CD8F30` (`CALL 00BE3310;
FSTP qword [0109DB48]; RET`), and `00BE3310` returns (QPC after `Sleep(100)` - QPC before) * 10
through `__allmul` (`00BF7DF0`). So every converted profiler value is seconds:
`BSP_Profiler_EndFrame` and `00BE3360` divide ticks by an estimated counter frequency. This
matches the overlay's full scale `profiler+4 = 0.1` (seconds) and its grid step of 1/60.

**`profiler+8h` is the bar height.** `include/bsp/app_frame_game_state.hpp` names it
`reference_frame_seconds` and says it is not read; `00BE3BF0` reads it at `00BE3C14` (first bar
y = 0.8 - h), `00BE3DCF` (quad height) and `00BE3E69` (pitch h * 1.3). Its value 0.0222 is a
height in the overlay's 0..1 space. That header belongs to another lane; the correction is recorded
here only.

`00BE3BF0` (called from `BSP_Game_Render` at `004CA694` when `game+2191h` is set), from the
listing:

- slots 1 .. `[0109CF18] + [00E15118] - 1`; per slot the 20 history floats (`+18h`, slot i at
  i*20) and 20 start-offset floats (`+1Ch`) are summed with an FSTP to float after every add,
  then divided by the double 20.0 (`00CE3D88`) and stored to float. `current[slot]` (`+20h`)
  receives the average for every slot;
- when the average is >= 0 (`FCOMI` / `JC`, so NaN and negatives skip), one renderer vt+D4h call
  (`BSP_Renderer_AppendDebugRecord40`, cc12's record-40 append) with texture 0, x = 0.05
  (`00CE7638`) or, when byte `+10h` is set, the start-offset average (the local persists into later
  slots), y, width = average / `+4h`, height = `+8h`, colour `+24h[slot]`, uv (0,0)-(1,1);
- the label `"%s %.2f ms count:%d"` (`00D68694`) with the slot name from the 1Ch string table at
  `0109D038` (std::string: pointer form when capacity `+14h` >= 10h), average * 1000 and the
  counter record's hit count (`+14h` array, `+24h`) is formatted into a stack buffer; the following
  `CALL 004C14C0` (`BSP_DiagnosticSink_GetOrCreate`) takes no arguments and its result is unused, so
  the text never leaves the function in this build;
- y decreases by h * 1.3 (`00D21298`, the double of 1.3f) per slot, drawn or not;
- then grid columns: width 0.001565 (`00D68690`), height (1 - y) - 0.2 (`00CE3D10`), colour
  `701F1F1Fh` (bytes stored at `00BE3EA9..00BE3EB5`), x from 0.05 stepping (1/60) / `+4h`
  (`00D68688`) while 1.0 > x, at least one column.

`00BE33E0` uses the same sum but starts its history pointer at slot 0 (`MOV ESI,[ECX+18h]`, no
`+50h`), so its line `PERFTIME[i]` reports slot i-1. The function is unreferenced.

`00BE3380` sizes: both rings `(n*5 + 0FAh) * 10h` bytes = (n + 50) * 20 floats, the current
array `n*4 + 200` bytes, n = `[00E15118]`: the constructor's capacity of static counters plus 50.

## Allocation report

`00BE3FD0 BSP_AllocationStats_LogStartupReport`, cdecl `(const char* caption)`, plain RET, callers
`BSP_Game_TeardownSessionState` and `004CCCC0`. With `[0109CEFC]` (written by
`BSP_AllocationStats_BaseConstruct`) non-null it logs a separator, the optional caption, `"Alloc at
startup:%8dK "` of stats `+08h`, calls stats vt+08h twice (first result dropped) and logs `"Phy
allocated:%8dK"`. The logger `004254B0` is an empty body in this build, so the report is inert at
run time. Kilobytes are signed / 1024 truncated toward zero.

## VFS helpers

All are inside the cc12/orch VFS domain; this packet only names what was unnamed, using the
existing documents as the primary evidence:

- search groups (`docs/VFS_SEARCH_REGISTRATION.md`): `00BDC230 BSP_VFS_FindSearchGroup`,
  `00BDB2E0 BSP_VfsSearchGroup_SelectValueList`, `00BE10A0`/`00BE13E0` payload construct/copy,
  `00BE1A20` node, `00BE1AC0` erase, and the unregister path `00BE1B90
  BSP_VFS_UnregisterSearchGroupValue` (erases the first equal value, then the group when both its
  lists are empty) with the unreferenced directory wrapper `00BE2210`;
- extension prefixes (`docs/NATIVE_VFS_EXTENSION_PREFIX_BE.md`): `00BDC3F0`, `00BDCBB0`,
  `00BDCEA0`, `00BDB7C0`, `00BE0B90`, `00BE0370`, `00BDF700`;
- aliases: `00BDF590 BSP_VFS_AddAlias` (both names normalized, pushed onto manager+94h, the vector
  `BSP_VFS_ApplyFirstAlias` reads) and `00BDF940 BSP_VFS_ClearAliases`, both called from
  `004ED910`;
- `00BDD020 BSP_VFS_RemoveContentPackageName`: erases matching names from the manager+48h string
  vector; its callers log "Remove active content package (%s)" and "Damaged content %s", so that
  member (described only as a string vector in `docs/NATIVE_VFS_MANAGER_PACKET_MAP.md`) holds active
  content-package names. Reconstructed as a list operation (partial: the erase helper `0043EA00`
  and the in-place iteration are modelled by value);
- `00BDD520 BSP_VFS_ExistsInFlaggedMounts` (manager vtable +0Ch): `BSP_VFS_ExistsInMounts` with a
  visitor (`00D683A4`) whose callback `00BD9130` asks a provider only when the mount's `+0Ch` byte
  is set. `00BD9130` and `00BD9160` (segment 95) were defined and named in the follow-up packet: `BSP_VFS_FlaggedExistsMountCallback` (stores the provider's answer at visitor+4 only when mount+0Ch is set; RET 8) and `BSP_VfsFlaggedExistsVisitor_ReadResultByte`;
- `00BE0AE0 BSP_FileBlockScope_Construct` (the frontend "open load block" that
  `src/game_hosts_menu.cpp` logs as unimplemented): refcounted object, name at `+14h`, then
  `BSP_VFS_EnterFileBlock`. EDX is a register input forwarded to `BSP_FileBlock_PrepareIdentifier`
  and was not traced; provisional.

## Open questions

- The unit fix for `[0109DB48]` and the `+8h` field name should be carried into
  `include/bsp/app_frame_game_state.hpp` and the three docs that list them as open, by their owner.
- The writer of the profiler name table `0109D038` (xref shows only the overlay; probably a block
  copy in a static initializer).
- The meaning of mount `+0Ch` (the flag `00BDD520` filters on).
- The static initializer `00CD8F30` is not a Ghidra function (it sits after `00CD8A80`); left for
  the define pass.

## Handoff

Packet complete; nothing in flight. Follow-up done (2026-10-12 06:09Z): the two profiler corrections were appended to
`docs/APP_FRAME_GAME_STATE.md`, `docs/APP_RUN_FRAME.md`, `docs/GAME_EXECUTABLE.md` and as comments in
`include/bsp/app_frame_game_state.hpp`; the two segment-95 visitor callbacks are named.
