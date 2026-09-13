# Battlestations Pacific reconstruction

A source-level reconstruction of `battlestationspacific.exe`, driven from an existing Ghidra
analysis. Recovered routines are rewritten as compilable MSVC Win32 C++ with their address,
evidence, original ABI and remaining uncertainty recorded next to them, and a runnable
executable target is assembled from those routines milestone by milestone.

The repository tracks no original executable, game assets, Ghidra database or built binaries.
Nothing here is a playable rebuild unless a ledger record or milestone document says so in
those words.

This file describes what is stable: where things live, what the words mean, and where the
current numbers are kept. It carries no counts or milestone claims of its own.

## Where the current state lives

| Question | Where to look |
| --- | --- |
| What is going on right now (owner, leases, index freshness, packets) | `python tools/bsp.py state` |
| Coverage counts with the caveats attached | `python tools/status.py` |
| Progress board: address bands by category, reconstruction and naming over time, harness split | `python tools/progress_board.py`, then open `local/progress_board.html` |
| Milestone plan and the next bounded work | [docs/ROADMAP.md](docs/ROADMAP.md) |
| The runnable executable: what it does on screen, its host coverage, how to run it | [docs/GAME_EXECUTABLE.md](docs/GAME_EXECUTABLE.md) |
| Packet ownership, states and dependencies | `config/parallel_work.json`, `python tools/bsp.py packets ready` |
| What a given address is, who documented it, who calls it | `python tools/bsp.py lookup <address>`, `docs-for`, `callers` |
| What shipped and when | `git log`; worker branches are `agent/<name>` |
| Library, template and compiler-generated code inventory | [reports/library_inventory/SUMMARY.md](reports/library_inventory/SUMMARY.md), [REVIEW.md](reports/library_inventory/REVIEW.md) |
| The first verified findings and the validation limits they set | [docs/BASELINE.md](docs/BASELINE.md) |

The raw internal function count in Ghidra is not the reconstruction denominator: it includes
compiler exception funclets, CRT and STL instantiations, stock Lua and zlib. The inventory
summary explains the corrected range.

## Vocabulary

Every reconstruction record states which of these it has reached, and the levels are not
interchangeable:

- **exported**: pseudocode and assembly pulled from Ghidra into ignored `exports/`.
- **reconstructed**: rewritten as C++ in `src/` with address, evidence and ABI notes.
- **build-tested**: compiles in the strict Win32 build.
- **fixture-tested**: passes a focused fixture, a real-device probe or an installed-asset check.
- **ABI-compatible**: proven to match the original calling convention and layout, not just behaviour.
- **game-validated**: compared against the original game running.

Descriptive names are hypotheses, never recovered symbols. Inventory tags describe code shape
and do not approve skipping a function. A *fragment* is a partial routine and is counted
separately from a function. Rules for all of this are in [AGENTS.md](AGENTS.md), which
`CLAUDE.md` imports.

## Repository layout

| Path | Purpose |
| --- | --- |
| `src/`, `include/bsp/` | Reconstructed, compilable C++ |
| `tests/` | Semantic tests, the optional original-code differential test, and Python tool tests |
| `cmake/`, `CMakeLists.txt`, `scripts/build.ps1` | Win32 build, dependency fetch, probe and executable targets |
| `tools/` | Export, lookup, ledger, annotation, coordination and verification scripts |
| `config/names/`, `config/reconstruction/`, `config/tags/` | Ledgers as JSON Lines shards, one 64 KB address band per file |
| `config/parallel_work.json` | Packet ledger: addresses, files, contract, state, validation |
| `config/target.json` | Ghidra project, program and install-path defaults; override with `--config` and a file under `local/` |
| `docs/` | One evidence document per subsystem or packet |
| `reports/` | Small retained evidence: per-packet JSON, validation runs, the library inventory |
| `third_party/` | Licenses and fetch notes for the pinned Lua, zlib and D3DX dependencies |
| `exports/`, `local/`, `build/` | Ignored: raw exports and index, local overrides and fixtures, build output |

Ledger shards, the lookup index and the reasons for the layout are described in
[docs/LEDGER_INDEX.md](docs/LEDGER_INDEX.md).

## Build and run

Windows, Visual Studio with the MSVC x86 C++ tools, the Windows SDK and CMake. Python 3.10+
for the tools; `pefile` and Capstone for the optional native reference checks. The first
configure downloads checksum-pinned Lua, zlib and D3DX sources; see
[third_party/README.md](third_party/README.md).

```powershell
./scripts/build.ps1
```

The script configures Win32, builds with warnings as errors and runs CTest. It produces the
core library, the game executable target, the subsystem probes and the test executables under
`build/win32/Release/`. It never launches or modifies the installed game.

The game executable takes the install root and a frame limit on the command line and logs
which host methods ran concretely and which are still unimplemented records. Its switches, the
expected log and what each milestone draws are in
[docs/GAME_EXECUTABLE.md](docs/GAME_EXECUTABLE.md). Probes take their asset inputs on the
command line and skip the asset-dependent checks without them.

Native differential tests are enabled after `python tools/ghidra_export.py verify-seeds`
confirms the analysed binary matches the disk executable.

## Working with the Ghidra analysis

Ghidra is opened on the project and program named in `config/target.json`, with the Ghidra
MCP plugin's loopback HTTP backend running. The export and query tools check project, program,
architecture and image base before touching Ghidra.

- Read through capped commands, never whole files: `python tools/bsp.py show <address> [--asm]`,
  `lookup`, `range`, `callers`, `callees`, `docs-for`, `find`, `strings`, `scan-bytes`, and the
  live `python tools/bsp.py ghidra ...` family. `python tools/bsp.py --help` lists them all.
- `python tools/bsp.py snapshot` refreshes the function inventory only when Ghidra's count
  changed; `python tools/bsp.py index --if-stale` rebuilds the lookup index after snapshots
  or ledger edits. `tools/ghidra_export.py` does the underlying read-only export.
- Record findings with `python tools/bsp.py ledger add-name|add-function|add-fragment`. If a
  legacy monolithic `config/*.json` ledger reappears, run `ledger migrate` before committing.
- Ghidra writes go through the machine-global write lock: `tools/ghidra_annotate.py` applies
  reviewed names and evidence comments, `tools/ghidra_tag.py` applies the inventory tags built
  by `tools/build_tag_ledger.py`, `tools/ghidra_define_function.py` defines listing-only
  routines, and `tools/ghidra_flow_repair.py` repairs fall-through gaps. Old values are
  recorded before every edit; the tag tool also logs prior state under `local/` and can revert
  it. Defects the bridge cannot repair, and the Ghidra script that can, are in
  [docs/GHIDRA_LISTING_DEFECTS.md](docs/GHIDRA_LISTING_DEFECTS.md).
- `tools/callgraph_sweep.py` and `tools/partition_candidates.py` build the disk call graph and
  the candidate partition used to find nearby code. Segment labels are ownership hints, not
  module names.

## Working in parallel

Several harness agents work on the tree at once. Each one has its own git worktree on an
`agent/<name>` branch, claims address and file leases from a registry outside the worktrees,
and picks work from the packet queue; the integrator stays on `main` and merges.

- Procedure, lease and lock rules: [docs/COORDINATION.md](docs/COORDINATION.md).
- How packets are cut and why partition waves are not independence proofs:
  [docs/PARALLEL_WORK.md](docs/PARALLEL_WORK.md).
- What a worker checks before committing and an integrator checks again:
  [docs/WORKER_VERIFICATION_CHECKLIST.md](docs/WORKER_VERIFICATION_CHECKLIST.md),
  mechanised by `tools/verify_report_calls.py`.
- Integration of a worker branch: `tools/integrate_workers.py`; ledger merge conflicts:
  `tools/merge_resolve.py`; worktree lifecycle: `python tools/bsp.py worktree add|remove`.

Stage only owned files in a shared checkout, commit in batches with a message file, and never
use `git add -A`.

## Documents and reports

Each document in `docs/` covers one subsystem or packet, opens with the addresses it accounts
for, records what was verified and how, and usually ends with a follow-up packets section that
names the next bounded work. Find the documents for an address with
`python tools/bsp.py docs-for <address>` rather than by listing the directory.

`reports/` holds the machine-readable side of the same evidence: the JSON a packet produced,
validation runs, function definitions applied to Ghidra, and the library inventory. Reports are
retained when they are small and cited by a document or a packet record.
