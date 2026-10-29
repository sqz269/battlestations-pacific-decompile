# Independent current accuracy candidate admission review

The exact current ON candidate is suitable for the reviewed accuracy activation
cohort: independent static checks and all four preregistered current gameplay
gates pass. This is a bounded recommendation for these binaries, source and
inputs. Root owns the final guard/admission publication. This review changes
only this document and its JSON report.

## Current identity and provenance

Published OFF source is `2c25911ef30940f93ef3a56c6202198013b59b45`; its actual
build was `2b05bd3c5330aa42f5fd8baf83d77875e5d4cc54`. The thirteen paths between
those commits are exclusively docs/reports/name/reconstruction metadata;
production trees are equal. ON was built at
`2c182d11caa173b852404e625c680a0979720fcb`. Its entire OFF/ON diff is the
single `kWeaponHitAccuracyNativeRangeBound` false-to-true header change.
Staging 5ce4d9497 adds only correction documentation/reports after ON.

| Frozen artifact | SHA256 |
| --- | --- |
| Current OFF executable | 8a83cf434cdd6d525696fb4574ce2db760b2c64dbff8781143b850653a1ba049 |
| Current ON executable | 7c22b561ae0f6cc38276d62e1468ceab358827ca043ff634842a8aa0aab9056a |
| Fresh common XLive DLL | f11299db80cd48a355c9db2bf3de5ebc4707ba46d38d0ae5e8eb0da616f87fb3 |

All thirty critical OFF Git blobs equal base 5c2547156; ON differs among them
only in that guard. Both retained current builds pass their existing two
checks (14.20s OFF, 11.12s ON). No new build/test/probe/game was launched by
this review. The old F9 backend and old 5c/c8 game receipts are not current
candidate proof.

## Actual source, cell and linked instructions

Fresh read-only replay matches four COFF functions to the actual ON PE:
caller 101E7BF0, getter 103066B0, dispatcher 1037B080 and converter 104B5C70.
The caller's getter call at +1DF and new dispatcher at +205 resolve correctly;
the dispatcher's converter call also resolves correctly. OFF has zero getter
or new-overload references and one old-overload call; ON has one getter/new
call and no old-overload call. Complete core objects differ only in COFF
timestamp bytes 4/5.

All seventeen dispatcher relocations were additionally accounted: two real
calls, ten local selector/table targets and five exact constant operands.
Converter tail 104B5C88 resolves to the actual BF7456 helper at 104B5CE0,
whose COFF bytes also match the PE. No returning provider stub is used.

The canonical owner hashes the supported original PE, initializes the actual
loader-zero-filled page and exposes the mutable word at 0109EEA4. That zero
initial state is original image/BSS evidence, not a chosen conversion mode.
The checked handoff publishes permanent canonical storage after parent ACK.
Main passes its reference into ConvexBinding; rebinding/startup checks precede
use. The getter returns the bound address, not the word value. Canonical pages
outlive the process Dyn owner and CRT/atexit teardown.

Current linked dispatch spills the unchanged size interpolation result to
float at +79 before entering the corrected range window. Initial FCOMIP/JB
and COMISS/JB retain the unordered route. The live sequence is FMUL at +ED,
MOV ECX,[EBP+18] at +EF, CALL at +F2: no intervening branch or float spill.
The converter's first instruction reads CMP [ECX],0 at that conversion epoch.
There is no mode snapshot, CPU-feature inference, default mode or forced FP
control. Existing profile/fraction/function inputs and serialized profile
names/layout are unchanged. Observed four profile dimension/endpoint rows match
within each pair; this is not a full table or all-input proof.

## Narrow earlier proof and withdrawn claims

The current core source blob is exactly the blob exercised by 8c2fa31's
retained fixture. That fixture supplies eight original post-spill comparisons
and eight direct converter comparisons: stored weight1 only, synthetic mode
cells0/1, masked nearest PC24/PC53, finite0.7 and initial quiet-NaN inputs.
It proves neither arbitrary size interpolation nor whole 008383D0 at PC53,
original startup/cell custody, arbitrary inputs, original ABI/FH/fault transport
or game behavior. It was not rerun or enlarged here.

Correction 199569eef withdraws 048da363's copied substring-RNG claims and
unqualified recommendation. `lof_expiry_draw=1` is a feature flag. Historical
MSD1600/USN02 absent-event rows remain RNG-unobserved; no new result relabels
them. Only valid prior static/source and narrow fixture evidence is reused.

## Fresh current terminal gates

The clean runtime host is pinned OFF at 2c25911ef. Both frozen matrices retain
their original pending-creation flags and have separate fresh root execution
approval. Native freeze is `5d3e5a1db5d6c5cea659f8c7759b8810495942200413853dcfb1117a28c29a05`;
IJN freeze is `c693520e8d6661362766b7f9b22ad8649e3a2efd736f72e0cbe222bf41e7d9ad`.
All 68/69 input size/hash pins still match. Criteria, plans, original evaluator,
masks, scripts, budgets, environment and arguments were preserved.

Each pair uses difficulty1, mission4800/total5100, step0.05, Normal priority,
Slots8 and the same fresh backend. Native USN04 uses the three native mission
flags and no helm plan. IJN18 uses its frozen retained helm plan, bounds and
stop-after-end40; the plan contains select/takehelm/moveto orders, no forced
RNG/death event. Its later frame7000 orders lie beyond this prefix.

| Current case, each side | Genuine samples | Accuracy calls | Completed ratings | First sampled event |
| --- | ---: | ---: | ---: | --- |
| Native USN04 4800 | 51 | 0 | 0 | 93.60s / .4143 |
| Hot IJN18 4800 | 1 | 2,843,254 | 432,180 | Recon2, 67.05s / .4640 |

The independent parser splits the complete real producer format into unit,
time, draw, mode and c10, checks the literal source suffix and compares every
record. It does not copy the inherited substring matcher. Sentinel -1 never
counts as a sample and remains in sequence comparisons. All four actual lists
match the recorded strict results and their pair sequence exactly; both
current cases have zero sentinels. IJN's first hot call precedes completed
return line5922, and sample line15375 follows it. Source places that return
marker and rating increment after the actual 0095EB40 call returns.

All original and additional strict gates pass. Ordered counts are 762/762
and 636/636, with zero differences; teardown counts are 10749/10749 and
238/238, also zero differences. All four logical runs exited zero and were
serial. Eight actual held-handle Normal captures have correct parent chains,
OS principal, distinct launcher worktree identity, exact binary/path/log/tag
and slot0/core2. Deadlines equal manifest start+14400 seconds; actions are empty.
All 52 terminal/archive artifacts were independently rehashed.

The observed equality is a rounded logged plane-mode projection, not raw float
bits, the global first RNG draw or all-stream state/consumption parity. Current
helper entry/completed-return evidence does not supply arbitrary branch/input
coverage. No WINS retime, mask change, extra run or source substitution closes
a missing gate. Here the exact preregistered current gates all passed; original
ABI/FH, all-input/native startup equivalence and broader gameplay remain outside
this admission. New native/vendor body credit is zero.
