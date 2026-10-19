# Repository layout proposal

Status: proposal only. Prepared by `oct18b-layout` on 2026-10-18 local time
(2026-10-19 UTC). This document authorizes no migration. This lane performed
read-only research, wrote ignored analysis files under its own `local/`, and
created this document. It did not change existing source, claim existing files,
write Ghidra, build, or run the game.

**Recommendation:** move existing source/header files and documentation into
directories in one coordinated change, preserving basenames and translation-unit
boundaries. Separate living guides from evidence; leave existing reports and
ledger shard locations alone. Defer semantic renaming and mixed-file splitting.
Prepare and rehearse the migration before a **45–90 minute global pause**, with
two hours reserved. These are planning estimates, not measured build/run timings.

## 1. Inventory and measurement boundaries

The inventory is pinned to fetched `origin/main` / this lane's initial HEAD,
`4890e900a3d304b2083efd9f2ac01cb35774c46f`. Count tracked blobs using
`git ls-tree -r --name-only <commit>`, not every file on disk. Ignore `build/`,
`local/`, shared exports and uncommitted work in other worktrees. Counts will
change before an approved migration; regenerate against its frozen base.

| Tree | Tracked files, recursive | Direct children | What is actually there |
| --- | ---: | ---: | --- |
| `src/` | 2,800 | 2,800 | 2,763 `.cpp`, 33 `.inc`, three private `.hpp`, one `.asm` |
| `include/bsp/` | 2,784 | 2,781 | 2,783 `.hpp`, one `.inc`; three headers already in `detail/` |
| `docs/` | 4,901 | 4,901 | 4,899 Markdown and two JSON files |
| `reports/` | 6,550 | 6,533 | 6,421 JSON, 116 text, nine Markdown, one diff, two BMP, one ZIP |
| `config/` | 493 | 6 | 432 JSONL, eight JSON, 53 text; already substantially structured |

`config/` contains 148 files under `names/`, 149 under `reconstruction/`, 137
under `tags/`, 53 under `win_orders/`, and six root files. Retain that structure:
the 64 KB shard identity is a useful address index, not a flat-layout problem.
The root files are `target.json`, `parallel_work.json`, `run_game.json`,
`seed_functions.json`, `lua_bindings.json`, and `fmod_ex_types.json`.

### Filename families

The following bins are disjoint, case-insensitive, and applied in table order.
The named rows require the entire stem or its underscore-delimited prefix;
`native_*` does not include `game_native_*`. The lineage remainder matches a
lane/packet marker anywhere in the stem after the explicit families are removed.

| Family | src | include/bsp | docs | reports |
| --- | ---: | ---: | ---: | ---: |
| `game_hosts_*` | 27 | 27 | 0 | 0 |
| `game_native_*` | 73 | 60 | 20 | 18 |
| `native_*` | 1,582 | 1,583 | 1,509 | 2,194 |
| `cc13_*` | 97 | 95 | 350 | 145 |
| `cc15_*` | 7 | 7 | 10 | 0 |
| Other leading `cc<number>` | 0 | 0 | 1,158 | 1,949 |
| `oct18b_*` | 5 | 5 | 24 | 12 |
| Other leading `oct<number>` | 5 | 9 | 185 | 195 |
| Other lane/packet/lineage marker | 144 | 154 | 228 | 413 |
| Semantic or unclassified remainder | 860 | 844 | 1,417 | 1,624 |
| **Total** | **2,800** | **2,784** | **4,901** | **6,550** |

Examples of semantic prefixes within `src/` are `gui` (80), `ship` (67), `sound`
(36), `unit` (32), `mission` (26), `plane` (23), `avoid` (22), `xlive` (22),
`scene` (19), `d3d9` (17), `lua` (17), `camera` (16), and `material` (14).
These counts are first-token observations, not authoritative subsystem totals.
`native_*` is especially broad: it covers rendering, storage, AI, containers,
platform work and library seams. It must not become one enormous directory.
Docs include 1,128 `CC12_*` files; reports include 1,344 `cc12_*` files.

There are two useful answers to “named after lanes or packets”:

| Measurement | src | include/bsp | docs | reports |
| --- | ---: | ---: | ---: | ---: |
| Opaque lineage/packet **prefix**, rather than a subsystem prefix | 140 | 143 | 1,971 | 2,706 |
| Any lane marker, including a suffix on a meaningful name | 255 | 266 | 2,307 | 3,095 |
| Additional packet-only marker, without a lane marker | 5 | 6 | 8 | 7 |

The prefix measurement recognizes `cc<number>`, `oct<number>[letter]`,
`sep<number>[letter]`, `orch<number>`, `goal*`, `band_<hex>`, `seg<number>*`,
`packet*`, `wave*`, and `codex/argon/elytra/sable/gpt/claude`, followed by `_`
or end of stem. The any-lane measurement recognizes those lane tokens at `_`
boundaries; packet markers additionally include `pass<number>` and
`batch<number>`. Thus **260 source files and 272 headers/fragments carry a lane
or packet marker**, while only 140 and 143 respectively start with an opaque
ownership label. These are reproducible lexical counts; they do not establish
that every remainder has a good name. For example, `render_fx_cc13.cpp` has both
a useful subject and provenance and should not be treated like `cc13_seg67b.cpp`.

### Partition segments, board categories, and ledger sources

`tools/progress_board.py:108` defines eleven `CATEGORIES`; its segment-start
mapping begins at line 114. `reports/library_inventory/candidate_partition.json`
contains 98 segments. Counting source-record addresses against the inclusive
segment ranges gives:

| Board category | Segments | Recon records in these address ranges |
| --- | ---: | ---: |
| core — startup/platform | 8 | 1,432 |
| files — files/VFS | 1 | 781 |
| lua — Lua/script API | 4 | 1,249 |
| scene — terrain/weather | 13 | 3,001 |
| units — weapons/damage | 22 | 5,565 |
| ai — AI/orders | 6 | 2,294 |
| mission — scoring/campaign | 5 | 1,061 |
| gui — menus/HUD | 22 | 3,755 |
| render — camera/effects | 12 | 4,084 |
| sound | 1 | 643 |
| net — network/online | 4 | 1,114 |
| Outside partition ranges | 0 | 967 |
| **Total** | **98** | **25,946** |

These are ledger rows (functions **and** fragments), not board game-code credit
or counts of translation units. Preserve the board's library exclusions.
Stock-library bindings merit their own directory, not new reconstruction credit.

Streaming only the reconstruction fields needed for aggregation found 23,873
function records and 2,073 fragment records. Their `source` values refer to:

| source root | Records |
| --- | ---: |
| `src/` | 25,066 |
| `docs/` | 637 |
| `include/` | 144 |
| `reports/` | 41 |
| `exports/` | 12 |
| null/empty | 46 |

There are 2,721 distinct nonempty source strings, plus the empty bucket. Of 2,615
distinct single-path `src/` strings, **778 span multiple board categories**.
Across all nonempty source strings, 789 span categories and 1,932 do not. These
are migration hints, not proof that files should be split: code in a single
contract can cross partition boundaries. Examples of concentrated lineage files:
`render_fx_cc13.cpp` has 450 source records, `water_weather_cc13.cpp` 385,
`cc13_seg67b.cpp` 378, and `scene_graph2_cc13.cpp` 336.

Do not assume `source` is one filename. Three existing strings contain two paths
separated by `; `, including
`src/compiled_material.cpp; src/material_parameter_bindings.cpp`. Preserve the
separator and remap each component. Record pre-existing unresolved references
separately: twelve export paths are absent in this worktree and require a check
against shared-export resolution, and seven rows name the currently absent
`src/native_online_lifetime.cpp`. A layout migration must introduce zero **new**
unresolved references; it must not silently repair historical evidence.

## 2. Proposed layout and assignment rules

Use the same subsystem vocabulary for C++ and public headers. Do not alter
namespaces, function identifiers, target membership, or compilation/link order.
Subdirectories beyond the first subsystem are optional, drawn only from a
committed rule table; keep the first migration shallow.

```text
src/
  game/entry/             game_main and other executable entry/application owners
  game/hosts/             game_hosts* harness/host implementations
  game/native/            game_native_* application adapters and their private .inc files
  core/                  platform/, math/, memory/, containers/ where established
  files/                 vfs/, archive/, streams/ where established
  lua/
  scene/
  units/
  ai/
  mission/
  gui/
  render/
  sound/
  net/
  library_bindings/       crt/, lua/, zlib/, fmod/, xlive/ where established
  probes/                standalone diagnostic executable sources/private headers
  legacy/mixed/          indivisible files without one established subsystem
include/bsp/
  <the same public-owner directories and existing detail/>
docs/
  README.md              short guide/evidence/plan index and lookup instructions
  guides/                living coordination, tooling, build, roadmap and policy guides
  evidence/<subsystem>/   reconstruction and verification notes; mixed/ allowed
  plans/                 proposals and future-work designs
reports/
  <existing files stay in place>
  library_inventory/     existing partition/inventory inputs stay in place
config/
  <existing structure stays in place>
  layout_map.json        approved migration map, created only during execution preparation
```

`bsp_core` remains the reusable CMake library. Its files occupy the appropriate
subsystems; it is **not** equated with the `core/` directory. `bsp_game` keeps
the visible `game/` entry/hosts/native grouping and its existing registrations.
Do not nest all rendering/files code under `core/` merely because it links into
`bsp_core`. Keep probe targets separate. No target is renamed or merged.

Apply the following **ordered** rules. Freeze the resulting assignments before
the maintenance window; the script consumes exact paths, not guesses at runtime.

1. An approved exact-path override wins. It records the owner category and a
   short contract/evidence reason. Overrides are reviewed during preparation,
   never invented during the pause. Preserve existing `include/bsp/detail/`.
2. Dedicated entry/application files go to `game/entry`; `game_hosts*` to
   `game/hosts`; `game_native_*` to `game/native`. Enumerate entry/probe targets
   from CMake, not a generic `main_*` filename match. A private `.inc` or `.hpp`
   follows its including translation unit. For multiple consumers, record its
   common owner in the override table and rewrite relative include edges.
3. Files whose established contract is a stock import/ABI binding or qualified
   stock substitution go to `library_bindings/<library>`. Do not classify every
   `lua_*`, `fmod_*`, or `native_*` file as a binding; game-side script/audio
   owners remain `lua/` or `sound/`. Do not add vendor-library bodies.
4. A conservative semantic-prefix table assigns known families, optionally after
   stripping `native_`. Match full underscore-delimited terms and longer
   specific prefixes first. The initial family vocabulary is below. Explicit
   exceptions resolve overlaps, e.g. `shader_lua` is rendering and `ship_ai` is AI.
5. With no semantic match, a nonempty ledger source-address set wholly in one
   board category may supply that category. An address outside the partition or
   categories in disagreement cannot supply a single-category assignment.
6. Otherwise retain the whole file under `legacy/mixed/`. Headers follow their
   unique paired source; header-only files use their documented contract, then
   the same fallback. No file remains unassigned and no majority vote splits or
   relabels a mixed translation unit.

| Destination | Initial semantic families; match subject, not lane suffix |
| --- | --- |
| core | platform, win32, app/startup, clock, allocator, containers/tree/string, math/random |
| files | vfs, file_store, physical_file/directory, package/mpkg, archive/inflate, mounted/memory streams, resource_path/lookup |
| lua | Lua host/API, script natives and mission script bindings |
| scene | scene graph, terrain, weather, ocean, foliage, world/entity ownership |
| units | unit, ship, plane, weapon, gun/gunnery, damage, bullet, torpedo |
| ai | AI, planners, bot/coordinator, orders/commands, avoidance, ship_ai and pilot control |
| mission | mission state, objectives, campaign, scoring, unlock/savegame contracts |
| gui | GUI, frontend, menus, HUD, fonts/text layout |
| render | renderer/d3d9, camera, shader/material, texture/sampler, vertex/mesh, lighting/shadow/effects |
| sound | sound/audio owners and FMOD game integration |
| net | network/online, session messages, peers/matchmaking and XLive game services |

This vocabulary is the seed for a versioned **exact prefix table**, not permission
for a fuzzy keyword classifier. During preparation, expand it into ordered
prefix-to-directory entries and exact exceptions; unknowns have the deterministic
mixed fallback. Review unusually large mixed groups before approving the map.
For future work, choose the directory owning the new contract, use a subject
basename, and put packet/lane provenance in the report and commit trailer. If a
new contract crosses owners, place it in `legacy/mixed/` only with a written
ownership reason; promote an established shared primitive into `core/` later.

### Lane names and mixed files

**Keep all existing basenames in the first migration.** Moving and renaming and
splitting at once would obscure review and make old worker patches harder to
merge. For example, all 378 rows of `cc13_seg67b.cpp` are in segment 67 (AI),
so `src/ai/cc13_seg67b.cpp` is a defensible initial destination subject to its
contract check. In contrast, the six rows of `oct18b_nA1_early.cpp` span segments
1, 2, 6, 7, 8 and 9: retain it in `src/legacy/mixed/` absent an established
shared owner. Do not manufacture a subsystem name for that collection.

Later, rename a lineage file only if all its bodies implement one describable
contract, the paired header can follow, and callers/evidence can be remapped in
a small independent change. Use a subject plus an address suffix when necessary
to distinguish contracts. A mixed file needs a separately reviewed split by
function ownership first. Keep dated packet/evidence report basenames: those
names correctly identify historical work rather than a current code owner.

### Documentation and reports

Keep root `AGENTS.md`, `CLAUDE.md` and `README.md` in place. Move living guides
such as COORDINATION, TOOLING, ROADMAP, PARALLEL_WORK, LEDGER_INDEX and the worker
checklist under `docs/guides/`, with a reviewed explicit guide allowlist. Every
other existing document defaults to `docs/evidence/<owner>/` (or `mixed/`),
except an explicit plan/proposal allowlist under `docs/plans/`. This proposal
stays at its requested path now; an approved later map may move it to plans.
Place the two documentation JSON files beside their evidence owners.

Create a short `docs/README.md`: links to living guides, the eleven subsystem
evidence directories, plans, and `bsp.py docs-for <address>`. Generate a bounded
per-subsystem evidence index if useful, rather than one 4,900-entry landing page.
Do not leave thousands of redirect stubs at old paths. Use the retained path map
to resolve old references, including those outside the repository.

**Leave the 6,550 existing reports byte-for-byte and path-for-path unchanged.**
They are immutable audit artifacts, often named after packets, and include
embedded source/diagnostic snapshots. This is already more than a directory
listing problem: lexical scans found 218,029 `src/` mentions in 3,948 report
files and 338,997 `include/bsp/` mentions in 3,395. Blindly replacing those would
rewrite recorded evidence. The inventory subdirectory is a live tool input.
For new reports adopt `reports/<subsystem>/<packet>/<artifact>`; a cross-owner
packet uses `reports/mixed/<packet>/`. Add an index/manifest when needed without
retroactively moving old evidence. Consumers of historical path fields must
resolve via the map; do not interpret archived code/include snapshots as live
source references. A later report-only reorganization needs its own proposal.

## 3. Path dependency register

Line numbers below are for the pinned commit. Re-run the scan at the frozen base.
The scan was read-only and aggregated content rather than dumping ledgers/docs.

### Build and C++

| Consumer | Exact mechanism / required treatment |
| --- | --- |
| `CMakeLists.txt:14–25,249–286,322` | Explicit `add_library`, `target_sources`, probes/tests and deferred game registrations. 387 occurrences of tracked source paths, 387 distinct. Rewrite exact path tokens, retaining command and source order. |
| `cmake/startup.cmake:503–505,2304–2368` | Deferred executable and `target_sources` registry: 2,382 occurrences, 2,376 distinct tracked source paths. Preserve repeated registrations, target membership and deferred include ordering. It is loaded through `CMAKE_PROJECT_INCLUDE`, not merely a root CMake include. |
| `cmake/native_crt_seh4_safe_handler.cmake:7,13` | Explicit assembly filename and deferred source registration. Remap the `.asm` path as well as `.cpp`. |
| `cmake/lua.cmake:25,37` | `${lua511_SOURCE_DIR}/src/...` belongs to fetched stock Lua. **Exclude** from repository source-path replacement. Its documentation reference at line 24 does need remapping. |
| `scripts/build.ps1:20,45,56` | Uses `build/win32`, injects startup registry, builds and then runs CTest. Build paths and script location stay fixed. |
| `scripts/clean_verify.ps1` | Same source-root configure/registry mechanism in `build/clean-verify`; do not change build-directory policy or invoke it during this research lane. |
| Quoted public includes | 12,114 `#include "bsp/..."` directives, 2,784 distinct spellings: src 8,764, headers 3,217, tests 128, tools 5. Derive each new spelling from the mapped `include/`-relative destination. Keep `target_include_directories(... PUBLIC include)` (`CMakeLists.txt:15`). |
| Private relative includes | Resolve against the **old including file**, map both endpoints, then recompute the relative spelling. `src/game_native_renderer_application.cpp:172–194` includes renderer and mission `.inc` fragments from the same flat directory. Do not treat fragments as new compilation units. |
| Generated/external headers | `camera_reference.hpp`, `seed_reference.hpp`, GUI/timestamp/leaf reference headers and Lua headers come from existing local/dependency include roots. Exclude them unless their actual owner is mapped. The three private headers in `src/` must follow their consumers. |

### Discovery, indexing, verification and integration tools

| Consumer | Exact mechanism / required treatment |
| --- | --- |
| `tools/bsp.py:189,296` | Both input digest and docs index use `(ROOT / 'docs').glob('*.md')`. Change **both** to recursive discovery. Database stores repository-relative paths and address mentions; rebuild `local/bsp_index.sqlite`, then verify `docs-for` links and counts. Query implementation is at lines 762–768. |
| `tools/bsp.py:243–256`; `tools/ledger.py:19,42,89` | Loads sharded reconstruction `source` into recon/functions tables; partition is explicitly `reports/library_inventory/candidate_partition.json`. Keep shard and partition paths fixed; invalidate/rebuild the index after source rewrites. |
| `tools/progress_board.py:108,153,175,188,225,267,297,328` | Categories use segment starts/keywords, **not directories**. History uses `origin/main` and recursive Git pathspec `src`, plus config shard history. Its `.cpp` filter already accepts nested files. Preserve historical paths and first-address dates; validate the migration commit does not create false first-body credit. `build(now=...)` allows a frozen comparison clock. |
| `tools/wire_readiness.py:68,145,149–150,236,294,340,386` | Reads ledger sources and the built map, recursively scans `.cpp` and `.hpp`, and chooses roots by **basename** `game_host*`, `game_main.cpp`, `game_startup.cpp`, `game_app.cpp`. Directory moves work if source fields are rewritten and basenames stay. Host labels are also basenames. Do not broaden its existing omission of `.inc` in a path-only migration; that would change counts. |
| `tools/integrate_workers.py:170` | Duplicate-declaration preflight scans `include/bsp/*.hpp` non-recursively. Make recursive. Do **not** use the full integrator for this doc-only lane: it builds, applies Ghidra names and refreshes snapshots. |
| `tools/merge_resolve.py:92,118–134,167–171,190–246` | Exact startup-registry special case; ordered deferred includes; append-only reconstruction union. Ledger path rewrites are edits of existing rows, not appends. Old-path and new-path copies must be normalized before union; a generic append union is insufficient and deliberately refuses some such conflicts. Keep shards and startup registry at their existing locations. |
| `tools/ledger_backfill.py:53,94,123` | Nonrecursive `src/*.cpp` scanner must recurse. CMake text matching uses full relative path and remains valid after lists change. The `docs/GAME_EXECUTABLE.md` exclusion must use its new path. |
| `tools/const_width_sweep.py:275,325,446` | Switch loading already scans headers recursively; **both** `--all` dispatch paths still use `include/bsp/*.hpp`. Make both recursive or `--all` silently skips the moved headers. |
| `tools/native_vfs_device_fixture.py:76,91,120` | Hard-coded two VFS sources need mapping. Its header freeze already recurses; retain the public include root. |
| `tools/oct18_tb_contract_probe.py:55,79` | Hard-coded read of `src/leftovers_cc13.cpp`; remap it. `/I include` stays. |
| `tools/verify_report_calls.py:159,282,419–429` | Accepts supplied JSON/Markdown paths and checks native call claims; no flat docs/report discovery and no live C++ source dereference. Nested document arguments work. Remap invocations/checklist references, not historical report snapshots. |
| `tools/verify_camera_reference.py:12,65–66`; `verify_cc13_leaf_reference.py:62,137–138` | Read target config and write local reference headers/report audits. Keep locations; existing report outputs do not move. |
| `tools/verify_gui_geometry_reference.py:16,95–99`; `verify_timestamp_reference.py:11,64–65`; `tools/ghidra_export.py:218,251–263` | Target config, local reference outputs and seed verification are independent of source directories. They are not migration steps requiring Ghidra writes or new seed generation. Preserve baseline seed availability. |
| `tools/check_ledger_bodies.py:69–72,97–100` | Reads names shards and asks Ghidra about address bodies; no C++ discovery. Unchanged because shard/tool paths stay. |
| `tools/pair_diff.py:5,22`; `tools/pair_export.py:21` | Comparison consumes supplied logs. Documentation pointers move; log syntax/normalization and executable paths do not. No noise-rule changes to force a passing migration pair. |
| `tools/workspace.py:16–39` | Shared exports resolve from the main checkout through `git-common-dir`, with `BSP_EXPORTS_DIR` override. Keep that mechanism. Do not create export junctions or move exports. |

Additional executable documentation-path consumers are
`tools/template_shapes/st1_build.py:18`, `st1_report.py:11`, `st1_retag.py:17`,
and `stlr_review.py:16` (all under `tools/template_shapes/`). Remap those exact
document constants, including report/tag provenance pointers they generate.
`cmake/native_data_placement.cmake:5` and `tools/fmod_sdk_types.py:340` also carry
documentation references. Retain data-placement linker options and safety policy.

The remaining tool hits in the scan are descriptive comments, docstrings or
historical labels rather than source discovery: `build_tag_ledger.py`,
`callgraph_indirect.py`, `frame_slot_census.py`, `gun_arc_pre_estimate_compare.py`,
`merge_resolve_tags_check.py`, `motion_trace_compare.py`, `pe_section_of.py`,
`raw_ownership_cc13.py`, `run_game.ps1`, `sample_main_thread.py`,
`stack_frame_walk.py`, `x87_dc_form_scan.py`, `x87_sym_walker.py`, and
`xlive_stub/xlive_stub.cpp`. Their current-document links can be remapped; do not
reinterpret historical statements. Template-shape decision `.txt` files and
`stlr/{README.txt,stlr2_final.py,stlr_export.py,stlr_feat.py}` also contain
documentation labels or unchanged `config/tags/*.jsonl` discovery. `status.py`
only globs shared exported functions. Tests contain both real `bsp/` includes
and deliberately synthetic `src/...` fixtures (`test_coordination_transactions`
and `test_merge_resolve_refusals`): rewrite mapped real paths, not synthetic
examples merely because they begin with `src/`.

### Structured references, documentation and external state

* **Reconstruction shards:** remap every operational `source` component by exact
  map lookup, including documentation/header destinations. Keep address, kind,
  name, status, other evidence fields, shard membership and record order. Empty
  values and existing unresolved source strings remain baseline exceptions.
* **Other config:** `config/parallel_work.json:39` and many later packet
  `output_files`, source/implementation fields, contract links and validation
  pointers name old paths. Apply map-aware string-token rewriting throughout
  operational config, including name/tag evidence pointers where they are current
  document links. Do not treat every config string as a filename or alter packet
  IDs, addresses, ownership, status or recorded code snippets. The root partition
  reference stays unchanged. Config has 29,774 `src/` and 69,799 `docs/` lexical
  mentions: rewrite by identified field/token, never by arbitrary substring.
* **Docs and root instructions:** rewrite Markdown links, reference links,
  HTML links and live inline path references. There are 10,152 `docs/` mentions
  across 1,242 documents; 6,537 `src/` mentions across 1,666. A moved Markdown
  file's relative links must be recalculated even when its target does not move.
  Preserve URL fragments; check anchors. Root `README.md:22–28`,
  `AGENTS.md:6,46,79` and `CLAUDE.md:12` refer to guides that will move. Update
  their links and flat-layout instructions. Local absolute links, ignored
  handoffs, editor tasks, open terminals and coordinator briefs need an owner
  notification/map lookup; they cannot all be fixed in a Git commit.
* **Archived reports:** preserve their old paths and embedded original content.
  Where a consumer actually dereferences an archived source path, resolve it
  through `layout_map.json` first. Existing `verify_report_calls` does not do
  that dereference. Do not fail validation on exempt historical path text.
* **Git continuity:** use `git mv`, but remember Git records snapshots, not an
  explicit rename operation. Review with `git diff -M --summary`; use
  `git log --follow -- <newpath>` and `git blame -M -C -- <newpath>`. For a
  10,000-file move, raise rename limits per invocation when needed, e.g.
  `git -c diff.renameLimit=30000 diff -M ...` and
  `git -c merge.renameLimit=30000 merge ...`. Never rewrite old commits.
  The map is the reliable historical crosswalk when rename heuristics fail.
* **Live leases/queues:** `tools/coordination.py:39–52,177–226,400` stores paths
  outside Git in `C:/Users/sqz269/.bsp/leases.jsonl` and `waitlist.jsonl`
  (environment/config can override). An ordinary commit cannot rewrite these.
  Require owners to release/refresh their own leases and queued old resources;
  never bulk edit the shared registry or let an old-path lease imply new-path
  ownership. Reclaim mapped paths only after the migration has landed.
* **In-flight branches:** observed live samples had 30–31 active leases across
  12–14 owners, one sample with 73 leased path entries and three waiters; Git
  registered 109 worktrees. Worktree count is not active worker count. Examples
  ahead of fetched origin were `agent/oct18b-nB` (27 commits),
  `agent/oct18b-render` (13), `agent/oct18b-render-a1` (12),
  `agent/cc13-undef` (2), and `agent/cc15-vcp6` (1), sampled before this proposal
  was written. Zero commits ahead does not prove a clean working tree. These
  are dated observations, not a list to act on later. Re-inventory all branches
  and dirty worktrees at the pause; local `main` is stale and is not the base.

The coordinator brief describes a `C:/Users/sqz269/.bsp/DRAIN` run guard. At the
pinned commit, `tools/run_game.ps1` has no literal `DRAIN` check; verify the guard
on the frozen head rather than assuming it is implemented there. Explicit
acknowledgements from all lanes are required regardless. Do not change any
running tests, registry files or pause marker during proposal research.

For reproducible coverage, the future preparer should save a bounded hit catalog
under `local/`: scan tracked text for `src/`, `include/bsp/`, `docs/`, their
Windows-separator equivalents, `glob/rglob`, and quoted/joined directory names.
Review CMake, tools, scripts, tests, config, docs, root instructions and report
schemas. Partition operational pointers from historical snapshots; record each
exception. The catalog plus a link/source resolver catches representations a
simple prefix grep misses. No claim that all lexical mentions are live links.

## 4. Deterministic map and migration procedure

### Map design — not a populated map in this proposal

Create `config/layout_map.json` only as part of an approved implementation. Its
schema should include the following; this is a schema sketch, not an executable
map or current file:

```json
{
  "schema_version": 1,
  "base_commit": "<full frozen origin/main hash>",
  "rules_version": "<classification rules hash>",
  "moves": [],
  "unchanged_roots": ["reports", "config/names", "config/reconstruction", "config/tags"],
  "historical_reference_policy": "preserve archived artifacts; resolve old paths through moves"
}
```

Each `moves` entry has `old`, `new`, `old_sha256`, `category`, `reason`, and
`build_targets` (a list, possibly empty). All paths are repository-relative POSIX
paths. Sort by `old`; require one old path per entry, unique destinations under
Windows case folding, no absolute/parent traversal paths, no destination alias
to an unrelated unchanged file, and no reparse points. Byte hashes guard drift.
Paired headers/fragments are individual entries, not implicit recursive moves.
Do not use globs in the committed map. Keep classification rules/overrides
versioned in the migration tool or alongside its schema, with the hash recorded.

The map is the single source for `old -> new`, public include spellings, private
include-edge resolution, CMake tokens, structured pointers, document links,
historical lookup, and inverse rollback. It need not map unchanged files.
Review full coverage: every tracked source/header/doc is either moved or on an
explicit unchanged allowlist. Existing reports/config shards do not move.

Design a future `tools/layout_migrate.py` with `plan`, `check`, `apply`, `verify`
and `resolve` operations; these commands **do not exist as a result of this lane**.
`check` must emit no changes, reject base/hash drift and collisions, and show
counts by owner plus exact planned non-move edits. `apply` must refuse an
unacknowledged pause or a changed base, and write its log under `local/`.
Re-running on a completed mapping must be a verified no-op, not a second move.

### Prepare outside the global pause

1. Obtain approval for this scope. Build the migration tool and recursive-scan
   fixes in an isolated worktree. Generate and review a complete provisional map;
   use exact overrides or the mixed fallback for every ambiguity. No file split,
   basename rename, new build target, compiler flag or library-credit change.
2. Rehearse against a pinned main snapshot. Record how long a reconfigure/full
   source rebuild, existing CTests, board/index refresh and the required mission
   comparison actually take. Run the existing coordination/merge resolver tests
   (`python -m unittest discover -s tests -p 'test_*.py'`) when changing their
   consumers. Add at most a focused regression for a demonstrated missed case.
3. Save baseline manifests: source/header file hashes, include-resolution edges,
   per-target ordered source registrations/properties, ledger identities and
   source values, doc/address index associations, board JSON and wire-readiness
   JSON. Retain an OFF executable/log with the exact USN04 argument vector and
   frozen settings used for the migration comparison. Baselines must ultimately
   correspond to the final frozen base, not an earlier rehearsal commit.
4. Inventory **all** branches/worktrees/dirty states, not just leases. Agree which
   work will integrate before the pause and which parked work will rebase. Assign
   one integrator and one publication owner; stop other pushes during the window.
   Do not commandeer other lanes' worktrees, files or leases.

### During the approved global pause

1. Coordinator stops dispatch/refill, asks existing lanes to drain, and collects
   explicit stopped/clean-or-preserved-work acknowledgements. Respect the current
   drain/run policy. Finish and integrate completed packets onto **origin/main**;
   every other open branch must rebase onto that pre-move head or be recorded as
   parked with an owner and preservation hash. Owners settle old-path leases and
   queues. “Integrated” means on origin/main, not merely pushed to an agent ref.
2. Fetch, freeze base `C`, retain a backup ref, and prohibit concurrent publication
   and analysis changes for this window. Regenerate the final map against `C`;
   repeat baseline captures if `C` changed since rehearsal. If classification,
   tool behavior, dirty work or missing ownership is unresolved, postpone before
   applying any moves. All active lanes must be based on `C` before moving.
3. Validate every original hash, then `git mv -- <old> <new>` from the map. Use
   native tooling within the verified worktree; no recursive delete or cross-shell
   move. Stage only paths in the reviewed manifest, not `git add -A`.
4. Rewrite **only allowed tokens** automatically:
   * Public includes use mapped destination minus `include/`; private quoted
     includes resolve both old endpoints and recompute their relative path.
     Preserve include order, comments, generated headers and dependency includes.
   * Tokenize all affected CMake files, including deferred registrations and
     assembly/source properties. Rewrite repository path tokens without sorting
     commands/sources, changing targets or deduplicating existing registrations.
   * Decode JSONL records, transform only `source` components where mapped, and
     preserve all other field values and row order. Handle semicolon lists;
     include doc/header sources. Prefer a string-span patch to preserve existing
     formatting; verify semantic equivalence irrespective of formatting.
   * Remap operational packet/config pointers and current documentation links;
     resolve links relative to the old and new document locations. Keep anchored
     links and reports as specified above. Apply recursive scanner fixes and the
     root guide links. Write the new docs index and approved populated map.
5. Run static invariants below, create the single candidate migration commit
   `M` on the integrator branch, then run build/CTest and semantic validations.
   If a correction is necessary, amend the **unpublished** candidate so the landed
   migration remains one commit. Do not merge an old-path branch into this diff.
6. Coordinator explicitly opens the maintenance validation phase for the planned
   USN04 comparison after all other runs are quiescent. Do not bypass an existing
   DRAIN marker automatically; the coordinator owns its state and restores the
   pause as appropriate. Run via `tools/run_game.ps1`, write separate log files,
   keep at most two runs, and retain all settings OFF in both versions.
7. After every gate passes, confirm origin still equals `C`, fast-forward publish
   the one migration commit, verify remote contains `M`, then release the pause.
   If origin moved, stop and rebase/revalidate; never force-push main. All lanes
   refresh tool code and the map before reclaiming new-path resources.

### Acceptance gates

* **File/content invariants:** same inventory cardinalities by kind except new
  map/tool/index files; no lost/doubled bodies, empty accidental files or Windows
  path collisions. Every move resolves; inverse-normalizing allowed path edits
  reproduces original source/header content. `.inc` and assembly ownership stays
  intact. `git diff --check` passes and rename review covers all moves.
* **Build graph:** per-target ordered source lists and source properties equal
  the baseline after inverse mapping, including membership in `bsp_core`,
  `bsp_game`, probes and tests. Configure from the new tree, and run
  `./scripts/build.ps1`; require its Win32 build **and CTest** to pass. Do not rely
  on stale project files or old object paths. Retain the baseline seed/reference
  setup so previously enabled native differential tests remain enabled; no silent
  reduction in discovered tests. Run the existing Python tool tests as applicable.
* **Ledger/config:** exact same row identities/counts, addresses, kinds, names,
  statuses and non-path semantics. Each source is exactly `map(old)` component by
  component. Baseline unresolved references remain accounted for; zero new
  dangling operational paths. Shard locations and partition data are unchanged.
* **Documentation:** recursive index finds all baseline Markdown documents,
  preserving each address-to-document association modulo the map. Validate
  Markdown destinations/anchors, root guide links and representative `bsp.py
  docs-for` queries. Classify historical exceptions explicitly; do not “fix” saved
  report content to satisfy a naive stale-path grep.
* **Progress board:** equal reconstructed/name/fragment totals, target/pool
  denominator, per-category/per-segment totals, and first-address attribution.
  Use `progress_board.build(now=frozen_epoch)` with `MAIN` set to `C` for the
  baseline and `M` for the candidate in a local comparison driver. The ordinary
  CLI fetches and uses origin/main (`:153,427–438`), so simply running it on an
  unpublished branch does **not** test candidate history. Normalize head/hash
  and migration activity metadata, not category or credit changes; compare at
  the same clock so rolling 24-hour fields cannot drift spuriously. After
  publication, also run the normal board command with JSON output.
* **Wire readiness:** rebuild the local index from unchanged shared analysis
  inputs and use the newly built map. Run `python tools/wire_readiness.py --limit
  10`, preserve `local/output/wire_readiness.json`, and compare to baseline.
  Require identical summary, ready ABI and category counts, roots, reachable-file
  count, address sets, per-address state/caller/callee/ABI results; normalize only
  mapped paths. Do not accept unchanged headline totals hiding row substitutions.
* **USN04 OFF pair:** compare the pre-move OFF build from `C` with the post-move
  OFF build from `M`, using the same frozen USN04 arguments, frame budget, input,
  jitter and runtime settings. “OFF pair” means **OFF before versus OFF after**,
  not a feature-toggle experiment. Run
  `python tools/pair_diff.py <before_OFF.log> <after_OFF.log> --json <result.json>`
  and require **exit 0**, with complete native tables. No exit-1 exception, new
  masks or tolerance changes. Preserve the exact command/logs for review.

No build, CTest, board regeneration, wire-readiness run or USN04 pair was performed
for this proposal. Passing these future gates would establish a path migration's
static/build/fixture and one-pair behavior checks, not general ABI or gameplay
equivalence across all missions.

### Older branches and rollback

Active branches should integrate or rebase **before** the move; afterwards each
owner rebases onto `M` before resuming. Git rename detection can carry ordinary
body edits, but cannot reliably rewrite CMake strings, include spellings or
newly appended ledger/config paths. Run map normalization on the resulting diff,
check no mapped old file was resurrected, refresh leases, then use the ordinary
build/verification/integration path. Preserve the branch tip before rebasing.

For a parked branch, port only its commits not already integrated, preferably
into a fresh worktree based on new main. Do not merge an entire stale tree or
rewrite published worker history. With a source conflict, resolve against the
mapped current owner. With ledger conflicts, map old paths in base/ours/theirs
before evaluating equivalent rows; never resurrect both spellings through a
union. `merge_resolve.py` is not automatically sufficient for this transition.
Retain the map indefinitely for archived refs and late branches.

Before publication, abort by discarding only the isolated migration worktree's
owned candidate changes or retaining that worktree for diagnosis; do not reset a
shared main checkout. After publication, pause lanes again and `git revert M`
as one inverse migration commit. If later work exists at new paths, first port
that work through the inverse map and resolve conflicts; a revert is not promised
to be conflict-free. Rebuild/reindex and repeat the same acceptance checks before
resuming. Never reset/force-push main or restore stale shared registries/Ghidra
state. Retain the old executable and logs as evidence, not as a substitute for
validating the reverted tree.

## 5. Options, costs, and recommendation

| Option | Benefit | Cost and risk | Estimated global pause |
| --- | --- | --- | --- |
| Full semantic tree, rename/split lane files, move all reports | Most uniform final appearance | Several days of ownership analysis, major evidence churn, many-to-many mappings, highest history/merge risk; not a mechanical path-only change | Several hours or multiple windows; insufficient evidence for a tighter estimate |
| **Directories for existing source/headers and docs; preserve basenames; reports stay** | Immediate navigation benefit, one stable map, reusable subsystem vocabulary | About one working day to prepare/rehearse, plus review; CMake/include/ledger/tool rewrites and coordinated worker reset required | **45–90 minutes target; reserve 120** after a timed rehearsal |
| New work uses directories, gradual opportunistic moves | Small individual changes, little initial disruption | Scanner fixes still required immediately; thousands of old files persist, repeated branch conflicts and dual conventions, no single complete migration checkpoint | 5–15 minutes initially, then recurring windows |
| Docs only first; leave source/header/report paths | Separates living instructions from thousands of evidence notes with less build risk | Still needs recursive indexing and extensive links/config pointers; leaves both C++ directories flat and requires a second coordinated migration | 20–40 minutes after rehearsal |

Choose the second option. It separates filesystem organization from reconstruction
and gives a later user-approved execution a finite, reversible mapping. If the
rehearsal cannot meet the pause budget or open branches cannot drain, postpone;
docs-only is a reasonable explicitly approved smaller step, not an automatic
fallback with a different scope.

Estimated pause allocation: drain/final integrations and freeze 10–20 minutes;
final map/check/apply/static validation 5–10; reconfigure/build/CTest 10–25;
index/board/wire/link checks 5–10; USN04 pair and publication 10–20. Work in the
window can overlap only where dependencies permit. Finishing a long-running
packet, unexpected build failures or non-quiescent workers can exceed the
reserve; resolve those before starting or abort the unpublished candidate.

The three main risks are **concurrent old-path work and lease/queue resurrection**,
**silent tooling omissions or misresolved path rewrites** (especially private
fragments and multi-path ledger fields), and **evidence/history distortion or
runtime drift** from combining moves with semantic cleanup or build-graph changes.
The frozen map, explicit pause, unchanged archived reports and before/after gates
address those risks without claiming broader reconstruction progress.
