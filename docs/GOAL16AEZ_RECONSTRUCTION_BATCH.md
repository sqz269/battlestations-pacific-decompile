# GOAL16A/E and GOAL15Z reviewed integration

This batch reconstructs 21 complete functions covering 378 Native bytes / 154 instructions. Seventeen complete entry bodies (118 bytes) are exact; four complete interfaces (260 Native bytes) have explicit calling-convention or current-service qualifications. Complete new Source code totals 364 bytes / 153 instructions.

| Packet | Functions | Native bytes | Evidence |
|---|---:|---:|---|
| [GOAL16A](GOAL16A_TAGGED_LEAF_B4_02.md) | 16 | 154 | 13 exact bodies / 73B; one raw copy and two literal x87 transfers use new interfaces / 81B |
| [GOAL16E](GOAL16E_TAGGED_LEAF_B6_16E.md) | 4 | 45 | Four exact raw-read/address/signed-delta bodies, including the full int32 divide-by-12 domain |
| [GOAL15Z](GOAL15Z_TRAFFIC_DWORD_RESIZE.md) | 1 | 179 | Complete raw traffic resize with six genuine current child/service calls; one focused returning-diagnostic/shrink fixture |

The primary merged current Main `942d144a5e613174d332d44a6d8fe8d01126e63b`, then ran the normal MSVC Win32 build at Source commit `551c94f483e8bd0ce7f3ff670e43930543513cbb`. It exited zero in 19.199 seconds; all three existing CTests passed (7.99-second test interval). An earlier combined build also passed all three checks. No new tests were added.

Before publication, Main `c471d52ad` added only `config/parallel_work.json` readiness metadata and `reports/cc12_native_crt_strcspn_readiness.json`. Those two changes were merged after validation; no Source, header, CMake or build script changed. The exact validated Source snapshot above remains the build evidence.

Three independent reviews accepted the frozen author pins. Primary verification compared every executable section and relocation target in the three new modules and twelve real traffic providers, checked complete symbol identities with bidirectional mappings for compiler-generated path hashes, and verified actual complete members and relevant public bindings through both production archive linker dictionaries. All fifteen final registered objects remain unchanged from the reviewed combination. Earlier GOAL15O and GOAL16B tracked blobs and their distinct CMake registrations are preserved.

All 21 Ghidra prototypes, names and evidence comments were applied under the write lock to the existing `bsp.gpr` / `/battlestationspacific.exe`, saved and independently queried. Original thiscall auto-ECX was not duplicated; Source-only unused EDX was not added to Original metadata. Complete typed bodies total 378 bytes and all have no-return false. Correct existing `STL_Vector_Resize_00495ae0` and `STL_Vector_Size_00b6bdd0` names remain. Old values/comments, final typed responses and prototypes are retained; affected exports, snapshot and index are refreshed. Names and formal type interpretations remain provisional.

GOAL16A initially collided with an existing filename. The collision was repaired before the worker commit: the old published files/providers/registration were restored and the new module moved to `_b4_16a`. Superseded evidence remains. Its supplemental strict compile used C++20; its actual registered build and the primary builds used C++17, with identical complete emitted code. An earlier summary calling its strict compile C++17 was incorrect. GOAL16E retains its rejected 52-byte C++20/helper form and failed C++17 build; its final accepted strict17 and registered17 forms are exactly 45 bytes.

The three new GOAL16A interfaces omit original stack-argument reads/RET4 cleanup. Their physical argument-cell aliases, stack faults, register/fault context and x87 saved-FIP/code placement are qualified. Literal x87 ordering and inherited state are retained; there is no floating-point execution claim. GOAL16E has static whole-body evidence, including all-input arithmetic analysis, with no Native execution.

GOAL15Z runtime evidence covers only its first returning shrink diagnostic and negative-suffix erase composition. Growth, no-op, other diagnostics, copies, allocation/free/overflow and C++ exception propagation through naked frames are unexecuted. Current SDK/private Native CRT, EH identities, full faults/flags/unwind/binary ABI and larger caller/game closure remain unproved. Existing child fixtures do not establish those parent paths. The primary and independent reviewers ran no new fixture or captured Native code.

Tagged-but-unreconstructed range sweeps supply bounded candidates, while dependency-closing traffic work continues independently. Worker inclusive intervals were 2,197.992 seconds (A), 1,089.655 seconds (E), and a conservative 2,509.135 seconds (Z), including classification, setup, repair/rejections, builds and handoff. They overlap and must not be summed as batch wall time. Counts and bytes are reported together; these observations do not establish a controlled throughput speedup.

[Machine-readable report](../reports/goal16aez_reconstruction_batch.json) records the exact author pins, independent-review hashes, build snapshots, artifact proofs and remaining evidence limits. Frozen local evidence is under `local/goal16a_root/`, `local/goal16e_root/` and `local/goal15z_root/`.
