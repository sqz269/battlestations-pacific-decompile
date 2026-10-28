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

Fresh build, lint and the 13-row OFF identity gate are recorded below once
complete. Baseline is the preserved CC16 candidate executable with SHA-256
`1a30fa1d6c67b51bef537c69af3179dc8454a0c4ddf608229f97deb1d0c80bdb` and its
original `cc16-progress/local/codex_cc16_candidate_<row>.log` receipts.
