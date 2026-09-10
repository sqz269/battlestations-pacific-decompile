# Battlestations Pacific reconstruction

An initialized reverse-engineering workspace targeting the existing `bsp.gpr` analysis of
`battlestationspacific.exe`. The output is a **32-bit C++ core library and subsystem probe**, not a playable
rebuild of the game. The current export/reconstruction counts and remaining validation boundaries
are recorded in [current status](reports/current_status.txt). Raw exports, reconstructed routines,
partial adapters and isolated probe results are tracked separately; no gameplay equivalence is established.

## Build and test

Requires Windows, Visual Studio 2022/2026 with the MSVC x86/x64 C++ tools, Windows SDK,
and Visual Studio's CMake component. First configuration downloads checksum-pinned Lua,
zlib and D3DX SDK dependencies; see [dependency details](third_party/README.md). Shader reflection uses
the installed x86 D3DX9_40 runtime. Python 3.10+ is used for the export tools; optional native
reference verification also uses pefile and Capstone.

```powershell
./scripts/build.ps1
```

Produces `build/win32/Release/bsp_core.lib`, `bsp_startup_probe.exe`, and test executables. The script selects Win32,
builds with warnings as errors, and runs CTest. It does not launch or modify the installed game.

Run `./build/win32/Release/bsp_startup_probe.exe` for the reconstructed random-subsystem
initialization, registration, draws, unregister, and cleanup path. It does not start the engine.

`./build/win32/Release/bsp_platform_probe.exe` exercises the reconstructed platform loop against
the real Windows thread-message queue. Its probe callbacks are explicitly separate from the
pending XLive and application-frame implementations; it does not create the game window.

`./build/win32/Release/bsp_d3d9_probe.exe` creates a real D3D9 device using the recovered
prefix of renderer initialization, then queries its swap chain and releases it. This partial
routine is tracked separately; resource setup and game rendering remain pending.
It also checks cached defaults, balanced renderer locking, surface binding and dynamic buffer
descriptions; see [resource ownership](docs/D3D9_RESOURCES.md),
[D3D9 startup evidence](docs/D3D9_STARTUP.md) and [renderer states](docs/D3D9_STATES.md).
The recovered non-indexed and indexed draw paths pass diagnostic triangle pixel readbacks;
see [draw evidence and remaining stream work](docs/D3D9_DRAW.md).

Pass the installed atlas DDS path to include shader-script and atlas validation:

```powershell
./build/win32/Release/bsp_d3d9_probe.exe 'I:/SteamLibrary/steamapps/common/Battlestations Pacific/interface/textures/menu_dxt1_2.dds'
```

The shader fixture derives the game root from that path and evaluates the installed
fundamentals, debug descriptor, include and dummy combiner using stock Lua5.1.1.
Without the path it explicitly skips asset-dependent shader checks. First build
downloads the checksum-pinned Lua source; see [dependency/license](third_party/README.md)
and [adapter scope](docs/SHADER_LUA_ADAPTER.md). Full material loading remains incomplete.
The installed-font check uses mounted streams and explicitly primes the priority-300
FileStore, then verifies that all three font resources open from that cache. See
[mounted stream evidence](docs/MOUNTED_RESOURCE_STREAMS.md) for native preload,
archive and ownership boundaries.
It also follows the recovered five-script startup preload order with flags
`0x32`, compares all cached bytes with installed files, and draws wrapped text
across three lines. A single synthetic MPKG fixture covers directory parsing,
stored/compressed entries and original-source reopening. See
[current integration evidence](reports/parallel_entry_validation.json);
native provider/context ownership and game startup remain incomplete.

The latest batch connects the startup provider manager and two package scans,
FileStore async request/completion and named material parameters. The existing
probe checks nested package discovery, an installed queued read with cached
completion, and both font draws through actual compiled parameter metadata.
See [validation and remaining limits](reports/startup_manager_validation.json).
Native outer-loop/effect-cache lifetime and a runnable game remain incomplete.

## Export the existing analysis

Open `C:/Users/sqz269/bsp.gpr` in Ghidra with the Ghidra MCP plugin enabled and open its
`/battlestationspacific.exe`. Defaults are in `config/target.json`. The exporter uses the same
loopback HTTP backend as the configured MCP bridge, checks the project/path/architecture/base,
and performs read-only requests.

```powershell
python tools/ghidra_export.py snapshot
python tools/ghidra_export.py seed
python tools/ghidra_export.py verify-seeds
./scripts/build.ps1
python tools/status.py
python -m unittest discover -s tests -p test_exporter.py
```

- `snapshot`: internal-function inventory, external symbols, segments, entry points,
  program metadata, and current disk PE identity under `exports/bsp/`.
- `seed`: pseudocode, assembly, and provenance for 16 starting functions, including CRT startup
  and the likely WinMain. Existing completed exports are skipped; use `--force` after Ghidra edits.
- `verify-seeds`: compares eight math/PRNG functions' complete byte ranges with the disk PE.
  On a match, creates ignored `local/seed_reference.hpp` for the optional native differential test.
- `status`: reports exports and reconstruction coverage without connecting to Ghidra, including
  names, compiler EH funclets and inventory categories separately. Names do not establish
  replaceable library behavior or reduce the reconstruction denominator.

Library and generated-code tagging: `python tools/build_tag_ledger.py` derives `config/ghidra_tags.json`
from the inventories under `reports/library_inventory/` (stock Lua 5.1.1 and zlib 1.2.1 names,
RTTI vtable slots, CRT leftovers, telemetry/physics blocks, STL instantiations and compiler-generated
helpers). `python tools/ghidra_tag.py` previews it; `--apply` renames only Ghidra-default `FUN_`
functions, creates the table-referenced entry points Ghidra missed, sets `Inventory:` bookmarks, and
logs prior state to `local/ghidra-tags-<stamp>.json` (`--revert <log>` restores it). Tag names such as
`STL_inst_*`, `CG_*`, `TRIV_body_*`, `DYN_physics_*` and `TELEMETRY_*` are inventory hypotheses;
library names with a `__prov` suffix are medium-confidence source matches. Take a fresh `snapshot` and
re-export with `--force` after applying tags.

Candidate partition for parallel work: `python tools/callgraph_sweep.py` disassembles the disk
executable over the snapshot's function ranges with Capstone and writes `exports/bsp/callgraph.json`
and `datarefs.json` (direct calls, tail jumps and .rdata/.data immediates only). `python
tools/partition_candidates.py` then cuts the remaining `FUN_` candidates into disjoint link-order
address segments with string-derived labels, script-binding and vtable counts, strong dependencies
and leaf-first waves; see `reports/library_inventory/candidate_partition.md`. Segment labels are
ownership hints, not recovered module names, and re-run both after every fresh `snapshot`.

Ledgers and lookups: reviewed names, reconstruction records and inventory tags are sharded JSON Lines
under `config/names/`, `config/reconstruction/` and `config/tags/` (one 64 KB address band per file).
`python tools/bsp.py index` builds an ignored SQLite index; `python tools/bsp.py state` is the one-screen
orientation card; `lookup <address>` and `show <address> [--asm]` (plus `range`, `callers`, `callees`,
`docs-for`, `segment`, `find`) answer with capped output so no ledger, export or docs directory is ever
read whole; `ghidra count|proto|xrefs|callers|callees|bytes|decompile|disasm|export` are capped live
queries; `snapshot` refreshes only when Ghidra's function count changed. `python tools/bsp.py ledger
add-name|add-function|add-fragment` appends records; `ledger migrate` folds any legacy monolithic
`config/*.json` ledger into shards. See [ledger and index](docs/LEDGER_INDEX.md).

Several harness agents can work at once: one git worktree per agent (`python tools/bsp.py worktree add
<name>`), address/file leases in a registry outside the worktrees (`bsp.py lease claim|release|list|check`),
a dependency-aware packet queue (`bsp.py packets ready|done|depend`), and a machine-global Ghidra write
lock taken by the annotate and tag tools. See [coordination](docs/COORDINATION.md).

The completed import has been [reviewed](reports/library_inventory/REVIEW.md).
The [roadmap](docs/ROADMAP.md) now separates stock-source reuse, compiler machinery
and game-specific contracts. Provisional tags and in-house Dyn code remain in
scope when needed. [zlib 1.2.1](docs/ZLIB_DEPENDENCY.md) now compiles; its game
stream wrappers and archive integration remain pending.

[Parallel work assignments](docs/PARALLEL_WORK.md) use explicit function families
and file ownership. The corrected [candidate partitions](reports/library_inventory/candidate_partition.md)
help locate code; their waves do not establish independent implementation tasks.

The native test executes five math routines and the PRNG seed/refill/integer routines in its
own process: 455 math comparisons plus one stream case covering 1,500 random values and final state.
The PRNG's one relative call is relocated to the copied refill routine. It does not execute game startup.
The reference bytes are locally generated, not required for the ordinary semantic test.
Run `verify-seeds` again whenever the input executable or saved analysis changes.

Export selected functions:

```powershell
python tools/ghidra_export.py decompile --addresses 008f81f0 00737970
```

An explicit `all` command exports all non-thunk internal functions and resumes completed exports:

```powershell
python tools/ghidra_export.py all
```

That is a large, long-running job; the initial setup exports only the seed set. Individual failures
are recorded in `last_export_failures.json`, and the process exits unsuccessfully if any fail.
Keep the same project and binary open throughout an export. Cached files belong to the snapshot;
after replacing the analyzed binary, use a new `--output` directory and a fresh snapshot.

## Repository layout

| Path | Purpose |
| --- | --- |
| `src/`, `include/bsp/` | Reconstructed, compilable C++ |
| `tests/` | Semantic tests and optional original-code differential tests |
| `tools/ghidra_export.py` | Read-only inventory/export and seed byte checks |
| `config/reconstruction.json` | Address-based reconstruction ledger |
| `config/ghidra_names.json` | Descriptive function names and supporting evidence |
| `config/ghidra_tags.json` | Generated library / compiler-generated / template tag ledger (see `reports/library_inventory/SUMMARY.md`) |
| `docs/BASELINE.md` | Verified initial findings and validation limits |
| `docs/STARTUP_RANDOM.md` | Startup path, PRNG/thread layout and current limits |
| `docs/FRAME_CLOCK.md` | QPC/fixed frame clock, native timestamp comparison and ownership limits |
| `docs/TEXT_INPUT_QUEUE.md` | Platform event storage and remaining text-input policy |
| `docs/PHYSICAL_FILE.md` | Physical asset reader and VFS/pooling boundaries |
| `docs/FONT_GEOMETRY.md` | Glyph quad prefix, native comparison and text rendering boundaries |
| `docs/FONT_MATERIAL_DRAW.md` | Installed bilinear glyph draw with supplied mount/camera inputs |
| `docs/VFS_MOUNT_LOOKUP.md` | Ordered lookup, native search lists and installed font resolution |
| `docs/PROVIDER_FACTORY_STARTUP.md` | Physical factory/mount reconstruction and FileStore integration boundary |
| `docs/ARCHIVE_PROVIDER_ENTRY.md` | Memory-backed FileStore and the separate MPKG reader dependencies |
| `docs/MOUNTED_RESOURCE_STREAMS.md` | Mounted read-only streams, aliasing and cache-backed font loading |
| `docs/ZLIB_DEPENDENCY.md` | Pinned stock zlib and game raw-DEFLATE wrapper evidence |
| `docs/FONT_RESOURCE_OWNERSHIP.md` | Font image ownership, loading order and reload boundaries |
| `docs/FONT_MATERIAL_BINDINGS.md` | Font texture slots, shader selection and native ownership boundaries |
| `docs/TEXTURE_IMAGE_LOADING.md` | Shared image-info/loading route and actual recreation metadata |
| `docs/OCCLUSION_QUERY_CALLERS.md` | Frame-end polling and flare cached-sample consumers |
| `docs/WINDOW_CREATION.md` | Recovered Win32 setup and remaining native lifecycle |
| `docs/DEFAULT_SURFACES.md` | Default color/depth capture and native reset boundaries |
| `docs/SURFACE_RESET_LIST.md` | Borrowed surface registration, removal and reset traversal |
| `docs/PLATFORM_LOOP.md` | Concrete Windows vtable, message loop, and exit behavior |
| `docs/ROADMAP.md` | Next milestones toward a game rebuild |
| `reports/` | Small, retained baseline evidence |
| `exports/`, `local/`, `build/` | Ignored generated analysis, local fixtures, and builds |

No original executable, game assets, Ghidra database, or generated binaries are tracked.
There is no game executable target yet. Reconstructing startup, object layouts, subsystem
interfaces, data loaders, rendering, and gameplay remains substantial work.
