# First missing Dyn body batch, 2026-10-28

Reviewed inputs: `c8ef5e03d` (ring initializer) and `814db0b3c` (radix entries),
based on accepted main `d0a2656fe`. The user goal is full integration and
activation with required gameplay gates. This batch establishes two compiled
bodies; their real caller/provider and original register-ABI integration remains
open and receives no runtime activation credit.

| Address | Compiled source | Original boundary |
|---|---|---|
| `00C33510` | `src/cc16_dyn_ring_init.cpp` | `00C33510..00C33645`, ESI receiver, RET |
| `00C37180` | `src/cc16_dyn_radix_entries.cpp` | `00C37180..00C37202`, ESI receiver plus stack object, RET4 at `00C37200` |

## Review and registration

The integrator independently read both complete assembly listings and source
interfaces. `verify_report_calls.py` passed all four rows in each report before
integration. The ring's two ordered resizes, wrapping arithmetic, signed loop
and unconditional endpoint writes are preserved. The radix append preserves
capacity publication, two-DWORD copy order, untouched new-row first DWORD and
low16 index store. Allocation and free remain required real external providers;
no vendor body, fallback allocator or new native null guard was introduced.

`CMakeLists.txt` registers both sources directly in the existing `bsp_core`
target-source list. The common `./scripts/build.ps1` passed the MSVC Win32
Release build and both existing suites (`reconstructed_math`, `tool_tests`).
Both dedicated object files are present in `bsp_core.dir/Release`; their hashes
are recorded in the reports. The build log is `local/cc16_dyn_batch1_build.log`.
No tests were added.

The new interfaces are not native ESI calling-convention adapters. Their
function symbols are absent from the game map because no game host calls them
yet; compiling them into the core library does not establish activation. The
reports preserve the workers' earlier checkpoint validation separately from
the common integration build proof.

## Metadata and saved analysis

The two `metadata_only_no_function_body_verified_oct24` placeholders were
replaced through `bsp.py ledger add-function` with
`reconstructed_build_tested_unwired` body records. Original values remain in
git history. Names are descriptive hypotheses:

- `BSP_DynRing_InitializeIndices_00C33510`
- `BSP_DynRadixEntries_AppendObject_00C37180`

`ghidra_annotate.py --apply` verified the existing `bsp.gpr` program
`/battlestationspacific.exe`, acquired the write lock, preserved existing plate
comments and recorded prior names/comments in
`local/ghidra-annotations-20261029T023246Z.json`, then saved the project.
Both affected exports were refreshed. No library symbol was renamed.

## Baseline runtime checkpoint

The fresh accepted `d0a2656fe` build was saved before this source batch, with
executable SHA-256
`574B8A0F84A8FB24284E832207A4A353D3A7C06C70B6140B4D61DBFEF2B773FD`.
The receipt is `local/function_goal_d0_baseline_receipt.json`.

- USN04 reference completed 4,800 mission frames and exited 0.
- USN04 milestone-D completed 1,600 mission frames and exited 0, released 179
  preload rows and retained only the documented `error.dds` texture allowance.

These runs used the eight-slot launcher at Normal priority in the active console
session. Their logs/manifests are `local/function_goal_d0_usn04_ref.log` and
`local/function_goal_d0_usn04_msd.log`. They verify the baseline, not execution
of the new unwired functions. Late-plane drawing is not established.

## Next dependencies

`00C4D5A0` calls the ring initializer; its source recovery is active in a disjoint
worker. The radix parent `00C55D30` is the next reviewed source packet. Real
manager/object/heap custody, native adapters and original root reachability or
qualified dead-chain treatment remain explicit obligations. The wider goal
also retains partial bodies, PC24 debt and unreferenced in-house candidates;
the old partition-pool zero does not close those obligations.
