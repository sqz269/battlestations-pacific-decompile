# Application material state-cache provider

The renderer application's existing `CompilerOwnersGraph` now retains
`NativeMaterialStateCacheContext{registry}` and exposes that same instance in
`GameNativeMaterialCompilerOwners`. This supplies the genuine canonical state-cache
dependency required by `NativeMaterialEffectNativeChildren` for the recovered
B46950/B45EE0 -> B5F6A0 -> B26500/B265C0/B26680 route. Obtaining the view performs
no native cache operation, registration or reference-count change.

The context borrows the graph's existing actual-owner registry, also used by the
pass lifetime and D61A2C/D61A34/D61A3C state companions. Existing constructor/copy
registration and native hit/miss credit schedules are unchanged. The application,
registry, actual renderer arrays/owners and all borrowed contexts must outlive
their consumers and retained failed operations. The ready-only getter and existing
live-companion/drain guards remain intact; they do not authorize discarding an
unresolved caller frame. No new cleanup, rollback, retry or lifetime admission is
introduced.

The new source view is 48 bytes, with alignment 4 on the installed MSVC 14.51.36231
Win32 target. Its previous slots remain 0/4/8/12/28/32/40, with `state_cache` at 44.
The full emitted borrowed return and application getter establish those slot
offsets. An isolated `/c` no-link record exports size 48, alignment 4 and the ordinary
registration-member offsets 12/32. The first layout probe incorrectly used the
installed UCRT `offsetof` macro on reference members and failed; that macro takes
the referent address and cannot establish stored reference-slot offsets. Its source,
command, diagnostic and preceding environment-preparation failures are preserved.
The corrected compile changes only the ignored evidence probe. This interface and
the affected compiler graph/application offsets are C++ source ABI observations,
not an original binary replacement or a promise about later integrated layouts.

One strict MSVC Win32 build from baseline `07fe2d8c0` passed all three existing
CTests. The actual renderer application translation unit is the only current
compiled consumer of this view and was rebuilt. Seventeen bounded source/config
and eleven tool inputs were frozen before building; all 651 actual postbuild
application dependencies match their prebuild identities. Successful build outputs
and 46 prior evidence archives remain unchanged after inspection. No new test or
runtime was added.

The frozen old application object is the actual `fae8b644` worker build, preserved
before synchronization, not a build from the later baseline. Both complete physical
COFF schemas/raw objects are retained. All 681 function symbols match between
objects: 629 are byte/relocation-identical and 52 change with the expected graph and
application layout. The focused audit includes 94 old and 94 new complete physical
sections, including catch/EH data; 81 new code sections cover 12,592 bytes and 3,701
instructions, with 571 total selected relocations. Twenty-six complete sections have
unique relocation-masked linked matches, with 328 actual PE relocation operands.
These operands are recorded without a map-resolved symbolic-target claim; other
sections have full COFF evidence only. Seven actual core-library providers retain
all 827 function symbols/code/relocation identities: four whole objects are identical,
and three differ only at timestamp bytes 4/5. The initial collector's stronger
whole-object assertion and its failure are preserved and qualified.

Evidence is in ignored `local/cc10_startup_state_cache_binding_implementation`,
with `verification.json`, complete `compiled_sections.json`, old/new application
COFF schemas, `provider_members_v02.json`, layout receipts and raw tool results.
The approved 54-row readiness archive and exact reviewed diff are nested unchanged.
This packet credits no native bytes/functions and changes no provider body or ledger.
The full B107F0 loading/Programs/NativeChildren/compiler/sampler graph, source0/W,
caller preimages, actual platform/MSG, full teardown and application/game execution
remain outside its admission and validation scope.
