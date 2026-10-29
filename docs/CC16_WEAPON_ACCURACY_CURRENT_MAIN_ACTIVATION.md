# Current GameHost native-range accuracy activation

The independent `kWeaponHitAccuracyNativeRangeBound` switch is accepted ON for
the actual-cell `FirepowerBinding` dispatcher after current-production build,
final-code, independent review and preregistered gameplay checks. This reuses
an existing reconstruction and adds zero native-body or vendor credit. The
existing PC24 cohort and other public interfaces retain their qualifications.

The OFF executable was actually built at `2b05bd3c5`; published `2c25911ef`
differs by thirteen metadata paths only. Their production-source equivalence
was verified. The ON executable was built at `2c182d11c`, whose sole production
change is this guard. The clean OFF runtime host is exactly `2c25911ef`.
Later correction/review/admission metadata changes no production code.

| Input | SHA256 |
| --- | --- |
| Current OFF executable | `8a83cf434cdd6d525696fb4574ce2db760b2c64dbff8781143b850653a1ba049` |
| Current ON executable | `7c22b561ae0f6cc38276d62e1468ceab358827ca043ff634842a8aa0aab9056a` |
| Shared current backend | `f11299db80cd48a355c9db2bf3de5ebc4707ba46d38d0ae5e8eb0da616f87fb3` |

The older copied 5c/c8 executables and F9 backend have separate receipts.
Their results are not relabelled as these current runs. The actual current
DLL, executables, sources, configuration, tools and immutable evidence copies
were pinned before execution: 68 native-case and 69 IJN-case file hashes.

## Source and final executable

All thirty critical OFF source blobs still match the reviewed 5c baseline.
Both current MSVC Win32 Release builds and existing suites pass: OFF 2/2
(14.20s), ON 2/2 (11.12s). The current core math object differs OFF/ON only in
COFF timestamp bytes 4/5. Four final functions match their actual COFF code
with relocation operands resolved; all three principal call targets match.
The independent review also resolves all seventeen dispatcher relocations and
the real converter tail helper.

The getter supplies the process-retained mutable `0109EEA4` address. The new
dispatcher receives that reference; its converter first reads `[ECX]` after
the unspilled FMUL. Getter/dispatcher offsets are `+1DF/+205` in the actual
caller. The live FMUL/MOV ECX/CALL sequence is `+ED/+EF/+F2`; no extra float
spill or intervening branch is introduced. The getter, canonical data owner,
ambient FP state and required lifetime contracts remain explicit.

## Current gameplay gates

Both cases were frozen at 4800 mission/5100 total frames, step 0.05, difficulty
1. IJN18 retains the exact existing `ijn18_l67_i18_d.txt`, its original
environment and stop-after-end 40. The native case retains its original three
native-mode flags and no plan. No seed, event forcing, trace, source, budget,
mask or WINS retime was introduced. The old evaluators and masks are unchanged;
an additional reviewed strict source-format observer is mandatory.

| Current OFF/ON case | Genuine samples per side | Actual accuracy calls per side | Completed ratings per side |
| --- | ---: | ---: | ---: |
| Native USN04 | 51 | 0 | 0 |
| IJN18 | 1 | 2,843,254 | 432,180 |

Native first logged sample is 93.60s/.4143. IJN18's `Recon2` sample is
67.05s/.4640 at line 15375, after its actual completed-return marker at line
5922. The complete formatted event sequences match. Both original and strict
checks pass, including pair, ordered, teardown, start, settings and provenance.
Equal absence cannot pass the strict sampled-event dimension.

All four logical games exited zero, serially, at Normal priority through
Slots8. Eight actual held-handle parent/child captures verify executable,
owner and parent identities. Every deadline is exactly manifest start plus
14400 seconds; no watchdog action occurred. Root and the independent reviewer
verified the 52 retained terminal/archive hashes and all frozen inputs.

## Interpretation and remaining work

These are bounded rounded **logged host observations**. They do not establish
global-first RNG, all-stream state/consumption, raw-bit parity or retail RNG
history. The previous MSD1600/USN02 absent-event rows remain unobserved; this
matrix does not retrofit their old results. The additive
[RNG correction](CC16_WEAPON_ACCURACY_RNG_CORRECTION.md) withdraws the inherited
feature-flag-as-draw claims and unqualified recommendation explicitly.

The accepted 8c2 fixture remains only the original post-spill window/direct
converter: eight plus eight checks, masked nearest PC24/PC53, synthetic
selector modes 0/1 and weight 1. It does not prove arbitrary interpolation or
the complete native sample across all inputs. Original binary ABI, FH/fault
transport and unrelated public paths remain unproved. No broad whole-function
or original-retail-game validation is claimed.

Exact independent review commit, raw receipts, source/code evidence, scope and
remaining obligations are in
[the activation report](../reports/cc16_weapon_accuracy_current_main_activation.json).
The full function-integration/activation goal remains active.
