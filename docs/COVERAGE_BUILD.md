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
ignored. Copy census inputs/PDBs into the primary's ignored `local/` artifact
directory before eventual worktree removal. One live compiler/process sample
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

## Smoke and reference recipes

No game run is authorized for this worker until the primary announces that the
original timing/thread-histogram run has finished. Run one coverage case at a
time, at Normal priority, and never alongside an original-game run. Use the
shared wrapper and honor `C:/Users/sqz269/.bsp/DRAIN`.

The accepted USN04 reference environment contains exactly
`BSP_DEATH_TABLE=1` and `BSP_GUNNERY_RNG_STREAMS=1`. Use an isolated child shell
with that environment and no unrecorded `BSP_*` overrides. Keep the source-data
preservation checks. Never pass `--hardware-probe-commit`, change power settings,
or stop another owner's processes.

After the launch gate opens, first run an untraced and traced smoke with the
same coverage EXE and arguments. Each output directory must be new. Example:

```powershell
$dr = 'J:/PROG/battlestations-pacific-decompile-codex-coverage/local/tools/dynamorio/DynamoRIO-Windows-11.91.20749/bin32/drrun.exe'
$exe = 'build/coverage-pdb/Release/bsp_game.exe'
$env:BSP_DEATH_TABLE = '1'
$env:BSP_GUNNERY_RNG_STREAMS = '1'
$smoke = @('--frames','620','--press-start-frame','30','--menu-select','USN04',
           '--mission-frames','300','--mission-frame-seconds','0.05','--difficulty','1')
./tools/run_game.ps1 -Exe $exe -Log local/coverage/smoke-untraced.log `
  -WaitSeconds 7200 -NoForceStop -- @smoke
$drLogs = Join-Path (Get-Location) 'local/coverage/smoke-drcov'
New-Item -ItemType Directory -Path $drLogs | Out-Null
./tools/run_game.ps1 -Exe $exe -Log local/coverage/smoke-traced.log `
  -WaitSeconds 7200 -NoForceStop -TraceExecutable $dr `
  -TraceArguments @('-follow_children','-t','drcov','-logdir',$drLogs) -- @smoke
```

Check actual mission entry/start frame, completion/exit code and both process
logs before the longer case. `-WaitSeconds` is the slot-acquisition wait; it is
not a game runtime timeout. If startup/renderer fails, run `query session` and
retain the actual failure before attributing it to source or tracing.

Then use the revision-2 reference window (no trajectory CSV):

```powershell
$drLogs = Join-Path (Get-Location) 'local/coverage/usn04-4800-drcov'
New-Item -ItemType Directory -Path $drLogs | Out-Null
./tools/run_game.ps1 -Exe $exe -Log local/coverage/usn04-4800.log `
  -WaitSeconds 7200 -NoForceStop -TraceExecutable $dr `
  -TraceArguments @('-follow_children','-t','drcov','-logdir',$drLogs) -- `
  --frames 5000 --press-start-frame 30 --menu-select USN04 `
  --mission-frames 4800 --mission-frame-seconds 0.05 --difficulty 1
```

Record that this rebuilt start uses `--menu-select USN04`; the original's direct
scene path is a window/setup difference, not resolved by this packet. Coverage
attribution must retain process/module bases and PDB identity, count undefined
original code and EH funclets separately, and separate game from vendor code.
Keep drcov logs/PDBs under ignored output paths. Delete intermediate drcov logs
only after the census JSON has been written and inspected.
