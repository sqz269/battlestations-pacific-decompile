# October 24 board pool audit

Baseline: `oct24-tail/local/output/oct24_board_before.json`, generated
2026-10-24 08:32 UTC, origin/main `515837c10`: 20,726 target game functions,
20,714 reconstructed and 12 remaining inside partition segments. This audit
uses the board's historical source/ledger union, game exclusions and live
reference predicate, rather than treating every fragment as a missing body.

The four independent Mit lifecycle bodies are reconstructed in the already
built `src/cc15_dyn_live.cpp`, with declarations in
`include/bsp/cc15_dyn_live.hpp`. They reuse the established 38h-byte pool
layout. The other eight rows retain their existing credit and ownership.
No target denominator, wire identifier, gameplay host, or switch is changed.

## All twelve baseline rows

| Address | Segment | Result and evidence | Original ABI / remaining boundary |
| --- | --- | --- | --- |
| `00407590` | 0 | Added `pool36_construct_00407590`; assembly `00407590..00407680`, global `0109EA0C`, concrete table `00D7A1DC`. | No register/stack inputs; EAX is global pool; plain RET, ESI/EDI preserved. New C++ interface supplies pool, shared registry head and allocator services. |
| `00407690` | 0 | Added `pool36_destroy_global_00407690`; assembly `00407690..00407741`. | No inputs, plain RET; ESI preserved. Supplied pool/head/CRT replace absolute globals. |
| `0040F910` | 0 | Added `pool20_construct_0040f910`; assembly `0040F910..0040FA00`, global `0109ED28`, concrete table `00D7A1E4`. | Same constructor ABI and interface boundary as `00407590`. |
| `0040FA10` | 0 | Added `pool20_destroy_global_0040fa10`; assembly `0040FA10..0040FAC1`. | Same destructor ABI and interface boundary as `00407690`. |
| `00922DD0` | 60 | Classified, skipped cc9 gameplay ownership. Live listing is exactly `XOR DL,DL; JMP 00922C80`. `00922C80` already has a function reconstruction in `src/entity_mission_seg60_argon.cpp`; the `PlaneSquadronCommandGateHost::is_surface_00922dd0` member is an abstract dependency, not a body. | Preserves ECX entity, forces DL=false, then inherits `00922C80`'s register ABI/return. Owner can supply the actual forwarding implementation and ledger record. No function credit added here. |
| `009C7240` | 67 | Skipped cc13 and cc9 ownership. Existing `dive_bomb_done_prepare_enter_009c7240` in `src/game_hosts_units.cpp` is recorded as `partial_projection_build_tested`. Live body `009C7240..009C725B`, four instructions, tail target `009BED80`. | Existing evidence records state+9Ch=0 and state+98h=-1 before the follow enter tail. Owner must close the full follow-entry contract; no promotion from fragment. |
| `009C7270` | 67 | Skipped cc13 and cc9 ownership. Existing `dive_bomb_done_prepare_tick_009c7270` in `src/game_hosts_units.cpp` is a partial projection. Live body `009C7270..009C727F`, target `009C1FD0`. | State receiver and dt forwarded to follow tick, RET 4. Retain partial status until owner accepts full base/receiver behavior. |
| `009D9CC0` | 67 | Skipped cc13 ownership. Existing `ShipAiPathPlanBlock` in `include/bsp/ship_ai_path_planner.hpp` has `layout_documented` fragment credit. Live body `009D9CC0..009D9D32`, 25 instructions. | ECX plan, EAX same plan, RET; 68h native plan block. Documented storage is not a complete constructor implementation. |
| `009EACA0` | 67 | Skipped cc13 and cc9 ownership. Existing constructor arm of `ship_ai_admit_torpedo_track_009f0ad0`, `src/ship_ai_torpedo_response.cpp`, is a semantic fragment. Live body `009EACA0..009EADDF`, with observer and refresh calls. | Thiscall track with half_length, length, horizon, entity, observation, speed error and kind. Owner must preserve observer lifetime, full record construction and native failure transport. |
| `009EC770` | 67 | Skipped cc13 ownership. `src/ship_ai_avoidance_request.cpp` already has `ship_ai_avoidance_party_accepted_009ec770` with a complete-host-build-tested fragment and a later listing-verification fragment. | Thiscall record plus side, RET 4, AL result. This is a possible owner-reviewed function-ledger backfill, not permission for this worker to promote it. |
| `00A9DF40` | 75 | Classified, retained partial. `GuiListboxRuntime::GuiListboxRuntime` in `src/gui_listbox_runtime.cpp` intentionally projects FC row/selection/cache/listener/layout state. Full native constructor `00A9DF40..00A9E066` calls UIContext with type 0Bh and creates two sentinels. `docs/GUI_LISTBOX_RUNTIME.md` explicitly excludes native base, remaining derived storage, pools and SEH. | ECX storage, EAX same storage, RET. Full native base/member allocation and unwind closure remain open; canonical owner fixtures do not establish this complete body. No cc15 object/GUI ownership changed. |
| `00BF3ACC` | 97 | Classified as an existing saved-listing continuation, not an independent callable body. Full 399-byte parent is reconstructed by `query_native_physical_file_date_00bf3a80`, `src/native_physical_file_date.cpp`; both addresses' naming evidence explicitly preserve this split. | Entry depends on the `00BF3A80` frame/shared SEH prologue and branch state. A separate callable ABI would be false. Keep fragment credit and saved Ghidra split intact; any future board distinction must be explicit. |

No baseline row lies in cc13 segment 40b or 58b; both reservations were
nevertheless respected. The additional current-source-only gap `006FF6E0` is not
one of these twelve: board history already credits its earlier address-named
definition. It was not reconstructed or reclassified again.

## Pool lifecycle contract

The constructor publishes `[previous=0,next=old_head]`, updates old head+4 and
the shared `00E188B4` head, then installs the concrete table and initializes
the real Win32 critical section. It sets recursion/count/table to zero,
first-free to -1, capacity to 20h and reserves 80h bytes via `00BF55BE`.
The image's post-allocation count/table reloads and destination-null skip are
retained, followed by old-table free and final publication of the reserve.
Allocation returning null is not converted to a fabricated exception.

The two FH3 records are `00E04824` and `00E04678`, each with three unwind
states. Their maps at `00E0480C` / `00E04660` select, in reverse order:

- vector cleanup at pool+28h (`00CC8773 -> 0040C600` and
  `00CC8633 -> 0040FEE0`), which conditionally frees the table;
- tracked critical-section cleanup at pool+0Ch
  (`00CC8768` / `00CC8628 -> 00402F70`);
- registry base unlink (`00CC8760` / `00CC8620 -> 00403970`).

The C++ constructor catches allocation/provider C++ exceptions, performs
these established cleanup actions in order and rethrows. This does not claim
compatibility with original FH3 frame metadata or Windows SEH transport.

Destruction reinstalls the concrete table, deletes every page using
`00BF65AC`, then frees the table using `00BF6989`. Each page iteration reloads
the table and count after the callback. It drains only a **signed positive**
explicit recursion depth, deletes the critical section, stamps the
`00D7A0C0` base table and unlinks both neighbors/head. Native stale pool fields
and links remain stale; no extra reset or second-destruction guarantee is
introduced. The two CRT releases are separate supplied services.

| Caller | Direct CALL sites checked against live listing |
| --- | --- |
| `00407590` | `00407620 -> 00BF55BE`; `0040765D -> 00BF6989` |
| `00407690` | `004076AE -> 00BF65AC`; `004076CB -> 00BF6989` |
| `0040F910` | `0040F9A0 -> 00BF55BE`; `0040F9DD -> 00BF6989` |
| `0040FA10` | `0040FA2E -> 00BF65AC`; `0040FA4B -> 00BF6989` |

These are in-house Mit pool bodies, which the board explicitly includes in
the game target. CRT library bodies and compiler EH funclets receive no new
reconstruction credit.

## Validation

- Every live batch used the configured `bsp` project at
  `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; the client
  verifies project, program, language and image base before queries/exports.
- Pseudocode was checked against all four complete assembly listings because
  the original globals overlap and lock/allocator/registry ordering matters.
- `python tools/ghidra_export.py verify-seeds`: eight seeds matched disk bytes.
- `python tools/verify_report_calls.py local/output/oct24_pool_calls.json`:
  eight direct CALL rows checked, zero failures.
- The changed translation unit compiled through `scripts/build.ps1` to
  `build/win32/bsp_core.dir/Release/cc15_dyn_live.obj` (14,506 bytes). Full
  Win32 build/existing CTests and final board recount remain pending.
- Lifecycle evidence is source/listing/build evidence. The existing registered
  suites do not provide a focused lifecycle execution or differential claim.
- Naming evidence was appended with `bsp.py ledger add-name`, then applied via
  `ghidra_annotate.py --apply` under the write lock. The project was saved and
  all four affected exports refreshed. Prior annotation values are recorded in
  `local/ghidra-annotations-20261024T084529Z.json`. Shard preservation checks
  found four added function rows, four appended name notes and zero unowned
  changes.
- No game runs, runtime switch changes, native ABI installation or gameplay
  validation were performed. Explicit host storage/services remain required.
