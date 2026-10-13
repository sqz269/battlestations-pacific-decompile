# CC13 pass over segments 49, 64 and 78 (packet cc13-sys6)

Packet cc13-sys6, lineage cc13. Ranges: segment 49 (`007F8400..0080D9B0`), segment 64
(`00968E80..0097AAF0`) and segment 78 (`00AACF10..00AC9640`). Lease: those three ranges minus the two
single addresses cc9 held (`0080A000`, `0080AA20`, packet cc9_g39_rocket). Every name is a descriptive
hypothesis, not a recovered symbol. Code: `include/bsp/engine_seg49_64_78_cc13.hpp`,
`src/engine_seg49_64_78_cc13.cpp` (registered in `cmake/startup.cmake`).

## Scope and counts

The index listed 359 functions in the three ranges with a reviewed ledger name and neither a
reconstruction record nor a tag. After this pass 193 remain.

| Group | Count | Action |
| --- | --- | --- |
| Segment 64 Lua listener subscriptions: filters, Load/Save/Matches slots, constructors, destructors | 55 | reconstructed or modelled, recorded |
| Segment 64 Warning objects, WarningManager calls, variant pick, manager archive, ship-noise effect | 43 | reconstructed, recorded |
| Segment 78 GUI leaves (type ids, Icon state records, named-value array, text, scroll, Movie, Section, layer backup) | 42 | reconstructed, recorded |
| Segment 49 profile, unlocks, bonus texts, nose art, race table, rain setter, owned pointer | 16 | reconstructed, recorded |
| Existing reconstructions without records (`warning_owner.cpp` 4, `race_config.cpp` 5) | 9 | backfilled after reading the decompile |
| `009763E0` (`mission_events.cpp`) | 1 | backfilled as `partial_projection_defect_open` (below) |
| Gameplay code (recon, sensors, repair zones, rockets, turn-path planner, entity path, MRT gun, bomb platform, two warning-manager scans) | 90 | left to cc9 (list below) |
| Remaining cc13 bodies (rain, a few profile and GUI layout routines) | 24 | open (below) |
| `STL_*` / `TMPL_*` / string-release / pointer-table-grow copies and thunks | 79 | not tagged (below) |

166 records in all: 156 against `src/engine_seg49_64_78_cc13.cpp`, 10 against existing files. The
naming-ledger evidence of the 156 now ends with a pointer to the reconstruction, applied to Ghidra with
`tools/ghidra_annotate.py` (prior values in `local/ghidra-annotations-20261013T071006Z.json` and
`...T071330Z.json` of the worker tree), and their exports were refreshed.

## Status

| Item | Exported | Reconstructed | Build-tested | Validated |
| --- | --- | --- | --- | --- |
| Subscription and Warning bodies (segment 64) | yes | yes; Warning fields are a projection | yes | no |
| GUI leaves (segment 78) | yes | yes | yes | no |
| Profile / nose art / race / small leaves (segment 49) | yes | yes | yes | no |
| Backfilled `warning_owner.cpp`, `race_config.cpp` helpers | yes | earlier packets | yes | no |
| `009763E0` in `mission_events.cpp` | yes | partial, with a defect | yes | no |

Nothing is ABI compatible or game-validated. No game runs were made. `tools/verify_report_calls.py`
checks JSON call-site reports; this packet has none, so it was not run.

## Method

Each body was read from the export (comment headers stripped by a local script), then from the
listing wherever the pseudocode lost an argument, a register or an x87 comparison. Host methods are
named by the callee they stand for. Two families are projections and say so in the header:
`WarningObject` flattens every Warning class into one struct (so recon `old_level` and hit
`area_exclusion`, both at `+84h`, are separate members), and the subscription filters are `std::set`
members where the image keeps `{vtable, allocator, head, size}`.

### Segment 64: Lua listener subscriptions

The 23 classes `0097E360` builds (table in `bsp/band_0095_cc13.hpp`) share a base `{vtable, callback
NativeString}` and filters of five kinds, each a vtable whose slot 0 is the predicate "empty or holds the
value": entity ids (`00D1B670`, predicate `00979140`, 16-bit ids), ints (`00D1B674`, `00979320`), bools
(`00D1B678`, `00979510`), case-insensitive strings (`00D1B73C`, `0097BC70` over `004C7F10`) and the
`{min, max}` range (`00D1AF44`, `00968440`; unordered compares fail). Field offsets of every Load slot come
from the `LEA EDX,[ESI+N]` before each loader call.

Points the listing settled:

- `009721C0` (entity sets) reads each key as a handle first and, when that gives null, again as an int,
  keeping the low word. Neither destination local is cleared per key, so a key the reader leaves alone
  inherits the previous key's value; `009722D0`/`00972390` have the same uncleared local.
- `0096E520` writes only the low byte of the boolean's value word (`0096E58B`); the rest is stale stack.
- `0096ACE0` (kill) evaluates all three filters before combining them; the other Matches slots stop at
  the first failure. Its log call goes to the empty body `004254B0`.
- `00972450` reads "party" only when `has_key` finds it; `0096E790` writes it unconditionally.

### Segment 64: Warnings

- `009741E0` builds the recon id: "recon", and for a change from level <= 1 to >= 2,
  "identified" + `OWN`/`ENEMY`/`NEUTRAL`/`UNKNOWN` (table `00E0B080`, relation from `00803CE0`) +
  "ship"/"plane"/"other" from the detector's kind. The player flag compares the detector (its squadron
  `+9D4h` for a plane) with `[00E188D8]`.
- `009768F0` reads "state" and the three handles through one local (`[ESP+20h]`) that is never cleared.
  `00976C60` pre-loads its handle local with a stack address (`00976C74`) and links the result
  unconditionally.
- `0096ADB0` treats two area-excluded hits within 5000 (`00D1AF84`) as duplicates.
- `00979B50` draws from `00BD2FC0` with `ECX = 1`, the thread generator's stream 1, not the gameplay
  stream 0 of `00BD2F10`. The reconstruction routes it through `WarningVariantHost`. With several tied
  variants that have been used, the draw is modulo ties - 1, so the last of them is never picked; an
  empty variant list divides by zero at `00979D1A`.
- `0096F090` is the session handler for the hit-feedback message: it rewrites its two stack arguments
  with message bytes `+18h`/`+19h` and jumps to `0096EF60` (`report_hit_feedback_0096ef60` in
  `options_hardware_cc13.cpp`).

**Defect found in `src/mission_events.cpp`.** `report_walk` (the model of `009763E0`) returns after the
first expired entry it retires (`WarningReportOutcome::RetiredExpired`). The native retires it, unlinks
and frees the node, and continues with the next node (`0097653C JMP 00976470`); the applied-queue walk
does the same through `0096C220`. So one Report call can retire several entries and still queue the
incoming warning. The file is not this packet's; the record carries the defect and the owner should fix
the projection.

### Segment 78: GUI

- Three ledger names were wrong and are replaced (ledger and Ghidra): `00AB1F60` sets the Pivot
  (`+18h`, and the current state's `+2Ch`), now `BSP_GuiIcon_SetPivot`; `00AB4630` assigns the
  ShaderName at `+104h`, now `BSP_GuiIcon_SetShaderName`; `00AC4AD0` is a writer helper whose value
  carries tag 1 (integer), now `BSP_GuiWriter_WriteIntUnlessDefault`.
- `00AB6D30` sign-extends the byte (`MOVSX`) before comparing it with `A2h`..`B2h`, so the rejection list
  never matches and every byte goes to `00AB6D00`. The ledger evidence describes the list as effective.
- `00AC8EE0` moves the content widget `[this+118h]`, not the scroll area: `MOV ECX,[ESI+118h]` before
  `00AA7D00`, which the pseudocode drops.
- `00AB3C10` assigns the Icon state's texture pointer without touching its reference count; the copy
  constructor `00AB3A70` retains it.
- The Icon/Line/Scrollbar type-id initializers are instances of `gui_widgets_cc13::gui_type_initialize`.

### Segment 49

- `007FCBC0` appends the record `{a, b, c, 0}` after the dedupe walk; it complements
  `scoring_bonus_record_007fc9f0` in `scoring_bodies.hpp`.
- `007F8690` (message E3h) applies the nose-art selection to the server's own player record
  (`game+748h + slot*118h`, `007F81C0`) when `[game+1FE4h] == 1`, otherwise sends it.

## For cc9

Gameplay-state code this packet did not touch (named, no record, no tag):

- Recon and detection: `008034E0`, `00803860`, `008039C0`, `00803B50`, `00803BE0`, `00803CE0`,
  `00803D40`, `00803DC0`, `00803E40`, `00803F30`, `008040E0`, `00804150`, `008042B0`, `00804300`,
  `00804350`, `00804620`, `00804C30`, `00804D20`, `00804E10`, `00804E70`, `00804F10`, `00805020`,
  `00805430`, `00805490`, `00805680`, `00805A70`, `00805BE0`, `00805C60`, `00805CF0`, `00805F60`,
  `00806400`, `00806480`, `008065B0`, `00806840`, `008069A0`, `00806A60`, `00806B10`, `00806CD0`,
  `00806EA0`, `00806EC0`, `00806F00`.
- Sensor classes: `00807B40`, `00807B60`, `00807BD0`, `00807C60`, `00807CF0`, `00807D40`, `00807D80`,
  `00807E60`, `00807F10`, `00807F60`, `00808080`, `008080A0`, `00808200`, `00808E40`, `00808EA0`,
  `008091D0`.
- Repair zones: `00809380`, `00809740`, `00809820`, `00809880`, `00809940`, `00809A80`, `00809BC0`,
  `00809C10`, `00809C50`.
- Rockets (around cc9_g39_rocket): `00809CB0`, `00809F20`, `00809F50`, `00809FB0`, `0080AAD0`,
  `0080AB40`, `0080AC60`, `0080ACB0`, `0080ACF0`, `0080AD40`, `0080AD80`, `0080ADE0`.
- Turn-path planner and entity path: `0080B310`, `0080B890`, `0080C8C0`, `0080CA60`, `0080CFB0`,
  `0080D690`, `0080D700`, `0080D770`.
- `00803330` (multiple bomb platform setup), `00803480` (MRT gun network message).
- Warning-manager scans modelled by `game_hosts_mission_frame.cpp` behind cc9 switches: `00977820`
  (collision sound) and `00977990` (proximity scan). Their records belong with that work.
- The `009763E0` defect above, if cc9 relies on `report_warning_009763e0`.

## Open (cc13)

- Rain: `00800450`..`00800EC0` (the 532-byte, 2-per-page pool; a 532/128 instance is already
  reconstructed in `dyn_body_creation.cpp`, not as a template), `00800B20`, `008019A0`, `00801B70`,
  `00801DB0`, `00802250`, `00801400` (spawn step, draws stream 0 of `00BD2F10` every frame, so any wiring
  needs a presentation stream; see `bsp/scene_weather_cc13.hpp`) and `008023B0` (543 lines).
- Profile: `007FB520` (content ids, 218 lines), `007FBA30`, `007FC490`, `007FCD90` (rank award).
- GUI: `00AAE770` (Movie loaded hook, also listed as unimplemented by the movie player host),
  `00AB4CE0`, and the x87 layout bodies with register inputs `00AB9240`, `00AC0610`, `00AC0970`,
  `00AC0BD0`, `00AC10A0`.
- Not tagged: the `STL_*`, `TMPL_*`, `BSP_NativeString_Release*`, `BSP_PointerTable_Grow_*`,
  `BSP_PointerArray_Grow_*`, `Mit_PoolAllocator_532_2_FreeThunk` copies and the thunks in the three
  ranges. Tagging needs a new evidence id in the shared `config/tags/evidence.json`, which is better
  done by one sweep than by a worker branch.

## Handoff

Branch `agent/cc13-sys6`, merged with `origin/main` before the final build. The record tables used to
write the ledger are the worker tree's `local/sys6_tab*.tsv` (address, C++ symbol, evidence, optional
status) and `local/sys6_records.py`; `local/sys6_names.py` appends the reconstruction pointer to the
naming ledger. A next pass should start with the rain model (`00802250`/`00801B70`, then `008019A0`),
then `007FBA30` and `007FCD90`, and leave the x87 layout bodies for an x87-capable worker.
