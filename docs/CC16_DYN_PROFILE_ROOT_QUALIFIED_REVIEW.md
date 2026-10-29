# Independent review of qualified Dyn Root construction

**PASS, within the new typed C++ interface's stated limits.** No blocking source or evidence mismatch was found in candidate `90e4fc72fb8e4c8aec0ea87d4a62e52eb96a6773`, frozen on parent `5ec9904f55e48f5d3cd871444b510b5cb5e1532b`. This review read both complete source files, the 48-line author document and its report. It owns only this document and the accompanying review report; it changes no source, address annotation, caller or guard.

## Source and native scope

The new three-argument Root overload requires `DynProfileRootPartialNodeDelete`, a separate flat `void (__cdecl*)(void*)` alias. The engine header does not include the scope header back into its dependency chain. No context/storage field, global, default callback or owner is added. The actual memory, publication cell, Root-name input and raw-allocation heap remain required caller inputs. A nonnull deleter does not establish that an arbitrary custom allocation tuple belongs to its heap. The context's BF6989 vector release remains a separate service.

The provider check precedes caller profile/context access. The `48h` allocation occurs before the `try`; allocator failure therefore does not enter the new Root cleanup. Its returned raw address is captured once. A nonnull allocation calls the existing real qualified C44000 overload with `context.memory`; that overload retains A900's current-vector capture and conditional BF6989 release before rethrow. Root's catch then passes only the captured raw address to the required flat deleter and rethrows. It does not reload a head, publication or constructor return as the free identity.

Live read-only analysis, the original PE and the retained export agree on all **115 bytes and 33 instruction addresses** of C50310..C50382. Native allocation at C50328 precedes captured storage at C50330 and state `0` at C50335. Root's head stores, publication at C50367 and native BF79F0 tail call at C5036D precede FS restoration at C50378, with no earlier state demotion. For original entry ESP=E, the captured allocation is E-10h and the state is E-4; the cleanup funclet's recovered frame selects that capture.

Handler CC836E selects E041D0: magic `19930522h`, maxState `1`, map E041C8. The map is `{-1, CC8363}`. Action CC8363 reads `[EBP-10h]`, pushes it, calls BF65AC, balances the argument and returns. It adds no publication/head clear, retry or rollback. The null-allocation branch still arms state `0` and publishes/clears the incoming profile normally. If a later covered failure reaches native cleanup, the capture can be null.

The source `try` has the corresponding complete payload extent: qualified C44000, all three head stores, actual profile publication, existing `std::memset(...,0,90h)` and return. Reached state stays published if a covered C++ failure causes captured-node release. Outer failure/lifetime custody must account for any resulting stale heads/publication; this packet provides no general continuation safety.

The old two-argument Root overload, both C44000 APIs and C55EA0/C55F50 remain unchanged. The enclosing engine callers continue to select the old normal-only Root API. No overload's existence promotes those callers to cleanup-qualified status.

## Independent verification

- All **21** retained source/snapshot/object/listing/audit/export/PE/span checks passed. Six relevant dependency files equal the frozen parent. The review's report records their exact hashes and Git blobs.
- Fresh MSVC 19.51 Win32 baseline and candidate objects compiled with `/std:c++17 /MD /O2 /Gy /fp:strict /EHsc /W4 /WX /c /FAs`, without warnings or executable creation.
- All **16 old function/helper records** match between the fresh baseline and candidate and their retained counterparts: exact code bytes and relocation offset/type/target, normalizing only anonymous namespace hashes. These include five public entries, nine source/template helpers and two compiler EH records.
- All three new code records match retained output: 152-byte normal entry, 29-byte handler and 51-byte catch/cold-block region. The latter contains the 18-byte captured-free/rethrow path, then the independently branched invalid-provider throw block and padding. Four C++ EH metadata records also match; their catchall range is tryLow=0/tryHigh=0/catchHigh=1.
- The emitted provider guard at +2B precedes context access at +31. Allocation +3B runs under state -1; capture +40 and state0 +43 precede qualified C44000 +55. Head stores +67/+69/+6C, publication +76 and `_memset` +7C retain state0 through FS restoration +89. Catch +98 pushes saved `[EBP-14h]`, calls the passed provider and rethrows. These are compiler-frame offsets, not the original ABI.
- Fresh original-call verification passed **4/4**: C50328→BF681B, C50347→C44000, C5036D→BF79F0 and CC8367→BF65AC. Independent PE decoding and live bytes agree with those rows.

Fresh strict object SHA256: baseline `ab652b3f0e632154b5c2e73846872db6b470b6d8929d05ef85954c96d005c148`; candidate `12a05a3faf1626e4fdaf98decb34c0109e816f12d1b42e8f29b9fdcf5d89e295`. Retained audit programs/results and target/call captures are under the review tree's ignored `local/root_qualified_review/`, pinned by the review report.

## Separate later common context

Read-only primary artifacts report a common build at `80549dde81e274e01d94733af61a776e266f0ba8`, existing checks 2/2 passing in 11.92s, and a link-only manifest-embedded PE014C. This review did not repeat that build, link or execution. Both Root source files equal candidate90e4 at80549 and at the independently observed later root checkout `82c4149e95f3d4cc6d7731802072b8a5aeba30dc`. All three new common-object code records equal the fresh strict records.

The link-only source takes genuine owner/profile/context references and binds the actual getter to the new Root overload; it neither constructs an owner nor calls Root at runtime. Its map includes the real getter, Root entry and MSVCRT unsized scalar delete. This corroborates symbol/domain routing for that separately supplied owner interface; it is not frozen90e4 dependency adoption, custom-heap authentication or an active engine caller. The report pins the actual artifacts without relabeling them as candidate builds or runtime proof.

## Admission limits

The compiler emits maxState `2` for the new C++ catch/rethrow, distinct from native maxState `1` cleanup-only metadata. Source catch coverage and `_memset` emission do not establish native BF79F0 feature selection, FH3/SEH identity, hardware-fault behavior, secondary-cleanup failure or original ESI/frame/stack ABI. No x87 arithmetic is introduced, and no ambient-FPU or runtime equivalence is claimed by these static checks.

Accept the additive interface with its required matching provider and existing qualifications. Actual Root/provider adoption, C55EA0/C55F50 enclosing scopes, interrupted-state owner handling and gameplay validation remain separate work. This review ran no full build, existing/new test, probe, fixture, game or DLL call, and made no Ghidra mutation. Native/vendor-body, activation and runtime credit are all zero.

[Machine-readable review](../reports/cc16_dyn_profile_root_qualified_review.json).
