# CC13 plane target-reference freeze: OFF integration gate

This lands the dormant source packet `4d0350e9e` after merging current main
`5ca4639f0` (source candidate `381792703`, followed by evidence only). The clean
merge `d6f9da5be` changes only `include/bsp/approach_ref_freeze_planes_cc13.hpp`
and `src/game_hosts_units.cpp` relative to main. No edits from the dirty
`cc13-aimfreeze3` worktree were imported. Base latch `a8046b1ef` was already on
main with its switches OFF.

## Source and original-code review

The configured saved Ghidra project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 image base `00400000`. Live queries use the
`bsp.py` client verification of project, program, language and base. No Ghidra
annotations, prototypes or saved analysis were changed for this integration.

Original `009FADA0` is a `__thiscall` target-reference update (ECX = embedded
reference, stack = float step, `RET 4` at `009FAF85`). Its death head tests
target state `+5Dh`, transforms the body offset `sub+28h` through target `+CCh`
at `009FADE4`, stores `sub+1Ch`, and clears `+14h/+18h/+41h` before the regular
lead tail. `009FAF90` is the observer callback (`__thiscall`, one stack object,
`RET 4` at `009FB003`): its transform uses `sub+1Ch`, then clears
`+40h/+18h/+14h/+41h`. These are different transform inputs.

The caller index and assembly confirm the packet's three embedded references:
kamikaze `009AF0CB` uses approach `+68h`, rocket `007B7902` uses `+58h`, and
depth charge `009A6043` uses `+7Ch`. Kamikaze tests the resulting `+80h` at
`009AF0D0..009AF0DE`; depth charge tests its `+94h` before the update at
`009A6005`, then reads the point through vtable slot zero at `009A606E`.
Existing descriptive names remain hypotheses, not recovered symbols.

`kApproachRefFreezePlanesCc13Bound` remains `false`. Dive, level-bomb, torpedo
and strafe freeze switches also remain `false`. With the new switch OFF,
kamikaze/rocket keep their existing early false return for a missing/dead
target and their existing live `aim_point_009fada0` call. Depth charge uses the
original adapter branch, including its point update before the live test.
Drop-kamikaze keeps task zero. The larger private census arrays are zeroed
ordinary host storage; task names are constant string pointers. No new dynamic
static initialization, RNG draw, callback registration or provider is added.
The OFF gate below checks simulation order/startup state, not wall-clock speed.

The possible ON behavior remains explicitly provisional: one reference per
shooter rather than per approach, target pose sampled at the host call, and
notify order inferred from observed flags. This integration does not accept
the ON behavior or establish original binary ABI replacement or visual parity.

## Validation receipts

Exact tested source: clean commit `93b9a5517d62ef38f0cd0876c1357acf9b5314b0`.
`./scripts/build.ps1` passed the Win32 Release build and all three existing
CTest groups (reconstructed math, CC13 leaf differential, tool tests).
`pc24_lint.py --diff origin/main...HEAD` reported zero findings across the two
changed implementation files; `git diff --check` was clean. No test cases were
added. The preserved candidate executable SHA-256 is
`4692bcf9bfcd2da95cea5cbd0872213a3deb5368b13333000e6a8ac58646430e`.

Baseline is the preserved CC16 candidate executable with SHA-256
`1a30fa1d6c67b51bef537c69af3179dc8454a0c4ddf608229f97deb1d0c80bdb` and its
original `cc16-progress/local/codex_cc16_candidate_<row>.log` receipts. All 13
baseline manifests were checked before comparison. The original executable,
logs, manifests and input archives were preserved; the baseline was not rerun.

The new runs used the existing `cc15-gui4/local/gui4_a1_jobs.txt` 13-row matrix,
the same frame limits and order files, step `0.05`, start press at frame 30,
death table and gunnery RNG streams enabled, and the same per-row bounds aids.
At most three candidate runs were active, all Normal priority, through
`run_game.ps1 -Slots 8`. Run manifests identify the exact candidate hash and
clean source above. Launch arguments, environment, order-file hashes and
difficulty matched the corresponding baseline receipts.

All 13 new candidate runs exited 0. Every `pair_diff.py` comparison exited 0.
Ordered dialog/message/Lua and teardown comparisons had zero index differences
and zero length differences, using only the existing pointer and worker-ID
masks from the CC15 comparators. Every mission began at frame 36 with the same
first RNG draw. Both sides had RNG streams enabled and no freeze census output.
The line counts below are equal on both sides.

| Mission | First RNG | Dialog/message/Lua lines | Teardown lines |
| --- | ---: | ---: | ---: |
| ESMP08 | 0.4933 | 2486 | 638 |
| JM06 | 0.1578 | 1262 | 241 |
| USN21 | 0.6207 | 1887 | 413 |
| IJN18 | 0.4640 | 7013 | 249 |
| BSM01 | 1 | 526 | 166 |
| USN12 | 0.1307 | 1457 | 360 |
| LOMP10 | 0.1119 | 4181 | 244 |
| BSM09 | 0.0949 | 1178 | 328 |
| PRCP01 | 0.3605 | 1050 | 290 |
| USN01 | 0.04169 | 930 | 1935 |
| USN02 | 1 | 559 | 203 |
| LOMP06 | 1 | 1717 | 166 |
| USN04 | 0.4143 | 762 | 248 |

The build-directory XLive DLL has SHA-256
`75e45ff775053b495370eb5de1d7a28788dba1bad22d7b5b6f310fa658647a91`, versus
baseline `ddf2c2a86d96f37819147c38991a74ef5c5946ce625e7b4e7914f8382aa13f87`.
These 32768-byte DLLs differ in exactly six bytes, confined to the four-byte
`IMAGE_FILE_HEADER.TimeDateStamp` at file offset `0xF8` and
`IMAGE_DEBUG_DIRECTORY.TimeDateStamp` at `0x1B04` (values 1792360310 versus
1793194228). Clearing exactly those two fields makes the entire files identical.
Code, imports, 31 exports, entry point, runtime data, IAT, resources, relocation
and load-config bytes are identical. Debug type 13 contains identical COFF
group data; neither DLL has CodeView or path metadata. Both exact DLL hashes
remain in the per-run input receipts. This provenance comparison added no
runtime output mask and required no game rerun.

Evidence resides under
`J:/PROG/battlestations-pacific-decompile-cc13-aimfreezekz/local/`:
`codex_aimfreeze_off_build.txt`, `codex_aimfreeze_off_lint.txt`,
`codex_aimfreeze_off_build_receipts.json`,
`codex_aimfreeze_off_baseline_preflight.json`,
`codex_aimfreeze_off_xlive_proof.json`,
`codex_aimfreeze_off_gate_summary.json` (13/13 accepted), the launcher/evaluator
scripts, and `codex_aimfreeze_off_<row>.log` with manifests and archived inputs.
Each `codex_aimfreeze_off_pair_<row>.json` has the raw pair report, with adjacent
`.txt` and `.ordered.txt` receipts. There were no retries or added comparison
masks. All three partition launchers exited 0 and all candidate processes ended.

Acceptance is for dormant OFF integration against current main on these 13
rows. All freeze switches stay OFF; ON gameplay behavior, original binary ABI
compatibility and visual equivalence remain outside this receipt.
