# Battlestations Pacific reconstruction

An initialized reverse-engineering workspace targeting the existing `bsp.gpr` analysis of
`battlestationspacific.exe`. The first output is a **32-bit C++ math library**, not a playable
rebuild of the game. The saved program reports 62,514 functions; its internal-function iterator
exports 62,072 functions. Five small math functions have been reconstructed so far.

## Build and test

Requires Windows, Visual Studio 2022/2026 with the MSVC x86/x64 C++ tools, Windows SDK,
and Visual Studio's CMake component. Python 3.10+ is used for the export tools; no pip packages.

```powershell
./scripts/build.ps1
```

Produces `build/win32/Release/bsp_core.lib` and test executables. The script selects Win32,
builds with warnings as errors, and runs CTest. It does not launch or modify the installed game.

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
- `verify-seeds`: compares the five rebuilt functions' complete byte ranges with the disk PE.
  On a match, creates ignored `local/seed_reference.hpp` for the optional native differential test.
- `status`: reports exports and reconstruction coverage without connecting to Ghidra.

The native test executes only these five call-free math routines in its own process and compares
455 bounded input cases, including aliased vector outputs. It does not execute game startup.
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
| `docs/BASELINE.md` | Verified initial findings and validation limits |
| `docs/ROADMAP.md` | Next milestones toward a game rebuild |
| `reports/` | Small, retained baseline evidence |
| `exports/`, `local/`, `build/` | Ignored generated analysis, local fixtures, and builds |

No original executable, game assets, Ghidra database, or generated binaries are tracked.
There is no game executable target yet. Reconstructing startup, object layouts, subsystem
interfaces, data loaders, rendering, and gameplay remains substantial work.
