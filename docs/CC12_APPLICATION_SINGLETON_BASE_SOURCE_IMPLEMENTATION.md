# Dormant Application singleton-base Source implementation

`construct_native_application_singleton_base_00bea810` is implemented as an uncalled,
explicitly bound Source interface in the existing `native_render_service_base`
header/TU. This mechanical placement reuses that module's genuine generic-base
provider and exact private Source guards; it needs no build-list change. The two
old files remain byte-for-byte prefixes of their new versions.

The context contains only references to actual manager `01090AA0` and Application
`E1AE90` cells. Bindings are fixed, distinct and disjoint from receiver/context/guard;
manager authority must be shared with `GameNativeStringProcess`/`GameSingletonHost`.
No actual Application-cell producer is created by accepting a parameter. The caller
retains stable aligned storage with valid uint32 access at +0 and its enclosing
1Ch preimages/domains. This interface allocates nothing and directly writes no
bytes at +04..+1B. Original ABI is ECX receiver, no stack arguments, EAX original
receiver, RET; this explicit-context C++ API is not an original binary replacement.

State0 arms before D68BC4. Genuine 415350 supplies the first manager's raw +10
section. The exact eight-byte local guard holds CE37FC at +0 and the actual native
section pointer at +4. Conditional actual Enter precedes the current physical
depth increment at +18. State1 arms only after that completes (including the
skipped null-section branch). Original receiver publication is unconditional.
The second genuine getter returns before reloading current E1AE90 for genuine
BD0C30, including null. Later validator effects do not change that selected argument.

Normal release decrements the current depth and calls actual Leave on the first
captured section. State1 stays armed through Leave. Only successful release disarms
both Source guards and returns the original receiver, without a final publication
reload or normal base reset. Actual manager results/vector, captured section/count,
cells, receiver and selected provider domains must stay valid; there is no
null-manager fallback, projected critical section, EH-stack alias or concurrent
invalidation allowance.

Before state1, ordinary C++ escape uses genuine 412430. After state1, genuine
411EE0 receives the actual local guard before 412430, and base reset completes only
if guard cleanup returns. A normal-Leave escape may decrement/Leave again; no safe
release disposition or balanced count is inferred. The existing second escaping
MSVC C++ cleanup exception terminates. This is ordinary Source cleanup, not proof
of original FH3/EBP, hardware-fault handling or current external runtime behavior.
The emitted base-armed byte precedes D68BC4; MSVC /EHsc places its EH state0
selector after that non-C++-throwing store and before the first provider call.
Successful-exit disarm stores are elided. These compiler details are retained,
not presented as original instruction ordering or hardware-fault equivalence.

Publication and registration/provider partial effects persist. No clear, restore,
unregister, free, retry, extra retain or counter compensation is added. A null or
different later publication is not retirement evidence. The genuine getter's lazy
allocation policy concerns its saved manager allocation, never this borrowed receiver.

Fresh normal `scripts/build.ps1` runs before and after passed all three existing
checks. No tests were added. The closed Source snapshot retains 4228
Source/build-input paths, including 4210 src/include files, actual
Git blobs/trees and raw working bytes. The accepted Source3356 manifest is pinned
exactly; all 3356 inputs match the baseline after LF normalization, with six raw
EOL variants separately retained. Four historical artifact pins in that manifest
are provenance only and supply no new build credit.

Complete Core1990/App80 COFF censuses, all old executable-section bytes/symbolic
relocations, compiler-proven header consumers and every selected TU's physical full
index are retained. 174357 old code sections are exact;
2 sections were added, with no changed or removed
old code. New normal/cleanup bodies, handler tables, provider calls, all-section
references, positive actual provider definitions and direct edges, current CRT/Win32
linker-input bytes and the final game map are retained. Whole-object hash drift is
reported separately from code equality. No private-name normalization is used for
this same-worktree comparison; any Root cross-worktree correspondence needs finite
positive function-definition bijections. The new entry has no caller in the frozen
Source corpus and is absent from the linked game map. The portable supplied-ZIP proof passed after physical copy,
with one permitted ZIP open and other I/O/process/network effects denied.

The generic reset retains its positive historical reconstructed/build/fixture scope
(generic profile/canaries among three complete leaves and ten parent pairs; five
merged library-only replays). The guard retains its positive strict Win32 and
original-byte FH3 fixture record (38,274 DWORDs, 128 snapshots and seven diagnostic
shutdown domains), with its declared bridges/runtime qualifications. These results
were neither downgraded nor transferred to an Application frame.

Canonical raw owner/E1AE90 production, BEA970/737970 outer construction, callable
profiles, +14 consumer/domain lifetime, independent E1AE78 custody, startup activation,
qualified destruction/retirement, Original FH3 and current external ABI remain closed.
No Native/GPR query/mutation/export/repair or original-game/OS/audio/ABI probe occurred.

Baseline: `f93506fb497fe36fbe51294c19349b5289254b2f`. Detailed report:
`reports/cc12_application_singleton_base_source_implementation.json`.
Physical proof: `local/cc12_application_singleton_source_cpp/`, including
`cohort_roster.json`, before/after captures and full indices, `evidence.zip`,
`portable_replay.py` and `replay_copied.json`.
