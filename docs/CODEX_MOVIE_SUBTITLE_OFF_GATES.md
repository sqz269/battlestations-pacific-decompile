# Subtitle/VFS OFF acceptance freeze, 2026-10-28

This packet evaluates the cumulative default-OFF subtitle loader and genuine
VFS borrowing (609ef74158,4561c11dc) against exact accepted source
409689e5e813d4aaaeb656abeb827d06a5842016. Candidate source merge:
f49e064d5e393c5ad8d0dfdd54d686b5669b2585. Both standard MSVC Win32 Release builds
passed their existing two CTest checks. No source or baseline-tree changes are
permitted during the matrix. The switch remains OFF.

The earlier 9b baseline was rejected for this gate because later accepted
GUI/input source changed. No old 9b executable or run log is reused. Source 409
has its own clean isolated baseline tree and a fresh build.

Read-only frozen executable copies:

- Baseline SHA-256 b121e96c18f08ac2be198c38761caa4bc2834f973be9f22c88c9b6f68f2f85c3.
- Candidate SHA-256 00b1d4eb8120171a3adc48aaa9cdceae90379d6e7f748d9dd9ca004a53350d52.
- Every run explicitly uses the SAME common XLive DLL,
  SHA-256 e41d690635ce050161ee0f6d376e10ba210339ab5c1970f82055a2e67bf1c80d.

Both original per-build DLLs are preserved. Win32 PE32 inspection found all 31
export names/ordinals/RVAs and the complete .text section identical. The common
backend is the baseline409 DLL; no runtime backend differs across either side.
Full source/build/input/backend receipts are recorded in
reports/cc16_movie_subtitle_off_gates.json and local frozen artifacts.

## Exact thirteen-row matrix

These are precisely the a1base rows from cc15-gui4's archived matrix. Frame
budgets, plan contents and bounds flags are preserved; outer frames are each
budget+300, mission step 0.05 s, start press at frame 30 and stop-after-end 40
only when a plan is present.

| Row | Mission frames | Plan | Flag |
|---|---:|---|---|
| esmp08 | 140000 | esmp08_p12_e8_rb.txt | (none) |
| jm06 | 120000 | jm06_s45_j6_k.txt | bounds |
| usn21 | 80000 | usn21_nd1_s46_u21_f.txt | bounds |
| ijn18 | 64000 | ijn18_l67_i18_d.txt | bounds |
| bsm01 | 40000 | bsm01_l69_b1_d.txt | (none) |
| usn12 | 36000 | usn12_l68_u12_e8.txt | (none) |
| lomp10 | 36000 | lomp10_p3_l10_b.txt | bounds |
| bsm09 | 36000 | bsm09_l68_b9_d1.txt | (none) |
| prcp01 | 30000 | prcp01_p9_p01_k.txt | (none) |
| usn01 | 26000 | usn01_l69_u1_a.txt | (none) |
| usn02 | 16000 | usn02_l68_u2_a.txt | (none) |
| lomp06 | 16000 | lomp06_l70_l6_c.txt | bounds |
| usn04 | 4800 | (none) | (none) |

Three external Normal-priority PowerShell partitions own disjoint logs and
process environments. Each runs baseline then candidate for its assigned rows.
At most three logical runs overlap, using the existing Slots8 launcher.
`BSP_DEATH_TABLE=1` and `BSP_GUNNERY_RNG_STREAMS=1`; only the original bounds rows
set `BSP_SEARCHER_BOUNDS_INIT=1`. Inherited BSP overrides are cleared independently in
each process. No power/hardware-probe flags, priority lowering or retries are
introduced. All binaries, matrix/plans and comparison support are archived.
Each launch records the existing manifest and its archived file inputs.

## Acceptance contract and scope

Every pair must have valid exact binary/backend/input/environment provenance,
clean frozen host metadata, exits 0/0, existing pair_diff exit 0, zero ordered
dialog/message/Lua and teardown differences, matching mission-start frame and
first RNG, and RNG logging enabled. Only the existing pointer/worker masks are
used. Failures stop further launches and remain available without retries,
retiming or new masks. Runtime callback counts and absence of new schedule
callbacks are recorded explicitly. OFF identity is not callback/playback proof.

The existing VFS CLI will separately check the installed us_outro.mws read and
a nonexistent virtual path. The installed file's hash/extent is archived.
That check concerns the actual VFS provider and full-extent read, not movie
callback reachability, subtitle playback or a hash of the returned VFS buffer.
No synthetic Lua result will be presented as native runtime equivalence.

Runtime results are pending. No ON, new native-function body, ABI or full
gameplay-completion credit follows from this OFF gate. Evidence is bounded to
exact common source409; later main changes require explicit integration review.
