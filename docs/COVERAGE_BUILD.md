# Optimized coverage build and serialized tracing

This is the build/runner part of `CODEX_BRIEF_coverage_run.md` revision 2.
Execution coverage establishes which code was reached in a particular run; it
does not establish correctness. Motion comparison and direct-scene runner work
were deferred by revision 2 and are not part of these changes.

## Optional symbols, unchanged default

`BSP_COVERAGE_PDB` defaults to `OFF`. When enabled, the root CMake directory adds
`/Zi` to compilation and `/DEBUG:FULL /OPT:REF /OPT:ICF /INCREMENTAL:NO` to
executable/DLL links. It keeps the Release optimization level. Explicit REF/ICF
preserve the optimization choices that `/DEBUG` would otherwise change.
The option applies before Lua/zlib/core/game targets are created so their
optimized code and inline records can contribute to the full program PDB.

The generated Release projects were checked: the normal build has no compiler
debug format and linker debug information is false; coverage has compiler
`ProgramDatabase`, linker `DebugFull`, REF/ICF true and incremental linking false.
Both retain `MaxSpeed` optimization. No default compile/link flag was changed.

Two independent build directories use the same source commit:

- `build/coverage-normal`: `BSP_COVERAGE_PDB=OFF`.
- `build/coverage-pdb`: `BSP_COVERAGE_PDB=ON`.

Build-source commit: `9c79eedff6231a0140c6dc401f5305e8526bc4fd`.
Both configurations use Win32 Release, the existing `cmake/startup.cmake`
registry, and `BSP_CL_MP=1`. Each build uses at most two MSBuild nodes; two
concurrent builds therefore admit at most four `/MP1` compilers. Build shells
run at Normal priority.

Actual configure/build commands, run from this worktree with the VS 2026 bundled
CMake executable assigned to `$cmake` and its installation assigned to `$vs`:

```powershell
$registry = (Resolve-Path cmake/startup.cmake).Path
& $cmake -S . -B build/coverage-normal -G 'Visual Studio 18 2026' -A Win32 `
  "-DCMAKE_GENERATOR_INSTANCE=$vs" "-DCMAKE_PROJECT_INCLUDE=$registry" `
  -DBSP_CL_MP=1 -DBSP_COVERAGE_PDB=OFF
& $cmake -S . -B build/coverage-pdb -G 'Visual Studio 18 2026' -A Win32 `
  "-DCMAKE_GENERATOR_INSTANCE=$vs" "-DCMAKE_PROJECT_INCLUDE=$registry" `
  -DBSP_CL_MP=1 -DBSP_COVERAGE_PDB=ON
& $cmake --build build/coverage-normal --target bsp_game bsp_xlive_stub --config Release --parallel 2
& $cmake --build build/coverage-pdb --target bsp_game bsp_xlive_stub --config Release --parallel 2
```

## Build measurements

Both final target builds completed with exit 0. The initial command named the
stub target `xlive_stub`; its actual CMake target is `bsp_xlive_stub`. Both game
EXEs had already linked before that secondary-target error. The corrected
commands above completed successfully; the initial and final logs are retained.

| Measure | Normal | Coverage |
| --- | ---: | ---: |
| EXE bytes | 7,497,728 | 7,498,240 |
| `.text` RVA | `1000` | `1000` |
| `.text` virtual bytes | 5,884,939 | 5,884,939 |
| `.text` raw bytes | 5,885,440 | 5,885,440 |
| `.rdata` virtual bytes | 1,326,060 | 1,326,268 |
| `.rdata` raw bytes | 1,326,080 | 1,326,592 |
| `.reloc` virtual bytes | 205,384 | 205,400 |
| Program PDB bytes | none | 92,614,656 |

All 36,087 common map function-symbol names have identical RVA sets, with no
added/missing function-symbol names. This includes vendor/template/alias names;
it is **not** a count of reconstructed game functions. Every section keeps its
RVA. `.text` bytes are not identical: 11,109 differ. All changed bytes lie inside
the same HIGHLOW relocation operand fields; bytes outside those fields match.
There are 58,597 such fields in each image at identical positions; 5,858 values
change (5,852 by +208, six by +212). Thus function layout and nonrelocated code
bytes match, while the coverage image's data addresses/debug metadata differ.
Do not claim full-image or byte-identical executable equality.

Normal EXE SHA-256:
`dd486dbf0ea2cb426c7374b68561382c5d1840e9239e1cdd9f3aed02bedeb461`.
Coverage EXE SHA-256:
`128fba34c171517a605d0188c00fac76f8edf020b9a6dbad51da6d192c9c6d93`.
Coverage program PDB SHA-256:
`6545e1ea9eefd884b969264f15e74b8f714eaac90bdab4268746089a403e61b4`.

The program PDB is `build/coverage-pdb/Release/bsp_game.pdb`, paired with that
directory's EXE; use that pair for attribution. Both full build directories are
ignored. Final binaries, maps and evidence were copied with SHA-256 verification
to the main checkout's `local/coverage-census-20261030/bsp/`; its artifact
manifest binds each copy to its source. One live compiler/process sample
recorded both CMake/MSBuild trees and their two active `cl.exe` descendants at
Normal priority (`local/build-priority-sample.json`).

The ignored `local/measure_coverage_builds.py` reads each PE's section layout and
`.text` bytes and compares public/static function symbol RVAs in the two map
files. It wrote `local/coverage-build-comparison.json`, including EXE/map/PDB
hashes and CodeView identity. `local/coverage-text-relocation-comparison.json`
records the relocation comparison. These artifacts live in
`J:/PROG/battlestations-pacific-decompile-codex-coverage-build/`.
An inline function absent from the map is not
classified as absent from the PDB; attribution must also inspect inline sites.

## DynamoRIO provenance and supported CLI

The primary downloaded the official Windows release into its own ignored
directory, without installation or PATH changes:

`J:/PROG/battlestations-pacific-decompile-codex-coverage/local/tools/dynamorio/DynamoRIO-Windows-11.91.20749/`.

Version: `cronbuild-11.91.20749`. Release ZIP SHA-256:
`9ef2d8f990a6acb4d303385fec0105bd45e54d36e92269494ac8d45e966c3844`.
The primary's `local/tools/dynamorio/download-receipt.json` records the download
and matching GitHub asset digest.

Use `bin32/drrun.exe` for this x86 target. The downloaded binary's help confirms
`drrun [options] [DR options] -t <tool> [tool options] -- <app and args>`.
Official deployment documentation says child following is enabled by default;
the recipe states `-follow_children` explicitly. Do not use `-no_follow_children`
for this two-process bootstrap. No global DR registration is required.
[DynamoRIO deployment documentation](https://dynamorio.org/page_deploy.html).

`drcov` writes a separate coverage log per process. `-logdir` selects the private
output directory, and binary logs are the default. Preserve the bootstrap and
actual game-child logs until attribution identifies them; tracing only the
bootstrap is insufficient.
[Official drcov documentation](https://dynamorio.org/page_drcov.html).

## Optional launcher prefix

`tools/run_game.ps1` now accepts `-TraceExecutable` and `-TraceArguments`.
When supplied, it launches exactly:

```text
<TraceExecutable> <TraceArguments...> -- <bsp_game.exe> <unchanged game arguments...>
```

The ordinary path is unchanged when no tracer is supplied. The serialized
wrapper still owns the slot, instance tag, affinity selection, DRAIN check and
bootstrap/child drain handling. The manifest adds the tracer executable hash,
prefix arguments and complete launch argv only for traced runs. Its root exit
code is the tracer's result; also inspect the game log and child coverage logs.
`tools/run_replay.py` does not yet consume the new tracer manifest block, so
repeat traced runs with the explicit prefix recipe rather than assuming replay
will retain tracing.

## Completed smoke and reference runs

The primary opened the launch gate after the original timing/thread-histogram
run finished. Four cases then ran serially: untraced/traced 300-frame smoke,
followed by untraced/traced 4,800-frame reference. All used the exact coverage
EXE above. Every case returned root/launcher exit 0, drained naturally, and
matched on actual mission-entry frame and requested simulated frame count.
The real runtime/options/save inventories remained unchanged.

[COVERAGE_BSP_RUNS.md](COVERAGE_BSP_RUNS.md) records the measured table, existing
frame-marker proof, exact arguments, private game/options paths, write-route
audit, process/module identities and raw drcov record counts. Its archived
`commands/launch-case.ps1` is the precise executed recipe; it asserts the real
save directories already exist, copies settings privately, removes unrelated
`BSP_*` variables, and uses only `BSP_DEATH_TABLE=1` and
`BSP_GUNNERY_RNG_STREAMS=1`. The wrapper retains slot/DRAIN handling,
`-WaitSeconds 7200` and `-NoForceStop`. The wait is for slot acquisition, not
an application runtime timeout.

Long-run combat totals differ between traced and untraced cases; the cause is
unknown. Matching startup, frame count and exit validates the census window,
not deterministic gameplay equivalence. Rebuilt `--menu-select USN04` also
remains distinct from original direct-scene startup. Function attribution must
retain process/module bases and PDB identity and distinguish game/vendor code,
undefined original code, and EH funclets. Raw drcov logs remain in the frozen
worktree for the census worker; no worktree or log cleanup occurred here.
