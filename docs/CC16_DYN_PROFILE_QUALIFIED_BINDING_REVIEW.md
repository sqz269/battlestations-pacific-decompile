# Independent review of qualified Dyn profile bindings

No blocking source-admission finding was found in `1e69d099267bbe4cb24db0935bbf08ba6dfc1ec3` for the bounded default-domain supplier and two additive C++ interfaces. All six changed headers/sources and proposal `713c8776330c076604279ec18fa4baee982020e9` were read. The default Scene still constructs SAP2; no radix manager, new caller propagation or runtime activation is supplied.

Review tree base: `f1431e56da01920b5eec31747ff86078e7222a57`; candidate baseline: `605373bf732e4bd3089e8f07ddca226ab4d635a3`. Current math source/admission and its frozen evidence are unchanged and receive no broader scope here. Only this document and its report were written. No full build, tests, fixtures, probes, games, source/tool edits or Ghidra mutations were performed by this review.

## Genuine supplier and caller preconditions

The new accessor locks the existing process startup mutex, requires completed explicit startup and compares the actual stored **context, allocate and release** tuple against the existing default `process_memory`. That tuple is `{nullptr, allocate, release}`; its unchanged wrappers call global scalar `::operator new(bytes)` and global unsized scalar `::operator delete(pointer)`. An unmatched/custom tuple throws `logic_error`. No BF6989/free fallback, new global context thunk, default-looking-name inference, second process owner or factory rebinding is added.

Fresh emitted code compares the three memory fields at offsets `0/4/8`, with direct allocator/release symbol relocations, and returns a DIR32 relocation to `??3@YAXPAX@Z`. The retained Release project uses `/MD`; its verified final map resolves scalar new/delete to `MSVCRT:new_scalar.obj` and `MSVCRT:delete_scalar.obj`. This supports the actual default new/delete domain. It is not an original-image CRT identity, arbitrary module/custom-heap, Debug-domain or interchangeable-free claim.

The headers and document require the returned provider to accompany the **same actual owner's engine/profile memory and publication cell**. These remain genuine caller preconditions: the public POD context can be manually assembled, and a nonnull function pointer does not authenticate its allocation provenance. This packet does not construct the real 9Ch profile/48h nodes or supply missing caller ownership. A different `DynLiveCrt` or an opaque `memory.release` cannot qualify those nodes.

## Old paths and additive interfaces

| Interface | Reviewed emitted/source behavior |
| --- | --- |
| Old four-argument C57020 | Remains normal-only and selects four-argument C50390; its strict object is unchanged. The common compiler inlines that normal path and calls normal three-argument C44000. |
| New five-argument C57020 | Checks provider before caller profile/scope access, throws `invalid_argument` if null, and selects real five-argument C50390 on a cold cache miss. |
| Old six-argument C5A8B0 | Explicit private selector `normal_only`, provider null; no inferred qualified cleanup. |
| New seven-argument C5A8B0 | Checks provider before manager/profile/name access, builds the qualified context and selects five-argument C50390. There is no missing-provider fallback to the old path. |

Scope-enter preserves the original cache-slot reference across cold append, reloads the actual profile publication before writing that original cache, then retains node/current publication and RDTSC ordering. `DynProfileScopeContext` remains `10h` (`memory+0`, cell pointer `+0Ch`) and distinct scope storage remains `14h`.

C5's private context preserves every byte offset in the established `28h` prefix: CRT `+0`, feature `+4`, half `+8`, profile cell `+0Ch`, profile context `+10h`, names `+14h`. Only mode `+28h` and provider `+2Ch` are appended, total `30h`. The qualified bridge compares the mode, pushes the provider and calls the five-argument symbol with `ADD ESP,14h`; legacy calls the four-argument symbol with `ADD ESP,10h`. That branch executes at added x87 depth zero, outside the instruction kernel.

## Independent machine and FP checks

Fresh strict MSVC Win32 object compilation passed for all three C++ files using `/MD /std:c++17 /O2 /Gy /fp:strict /EHsc /W4 /WX /c` (compiler parallelism two). No executable was produced or run by that check.

An independent COFF section/symbol/relocation parser reproduced exact baseline/candidate equality for **14 relevant old functions plus one compiler `unique_ptr` helper**. The fresh objects match those same retained functions. Only anonymous namespace hashes were normalized; machine bytes and relocation offsets/kinds/targets were retained. The kernel, all four shims, append bridge and supplier also independently match between strict and retained common/PCH objects (seven functions).

The source kernel is text-identical to baseline. An independent CFG/x87 accounting pass reproduced:

| Property | Result |
| --- | ---: |
| Distinct original address rows / emitted instructions | 654 /704 |
| Branches /distinct targets /calls | 52 /41 /11 |
| x87 instructions /FSTP epochs | 101 /27 |
| Maximum added x87 depth | 4 |
| Added depth at every call and return | 0 |

All 704 source instructions are reachable and all 654 original address rows are retained. The original 2,246-byte retail function hash and retained final-map hash match their pinned receipts. A fresh read-only call verifier checked all 15 original body/unwind call rows with zero failures after verifying the existing BSP Ghidra program. All 36 retained proof-artifact hashes and three common-object hashes match.

The native frame/shims retain `S+10B0` context custody, footer `ESP+10AC`, buffer spill `S+14`, argument cleanup, EAX result use, stack-probe contract and four scope-leave actions. The new C++ mode branch adds no x87 arithmetic or FP-control operation. The common build's four public wrappers and `unwind_profiles` helper use balanced EBP frames where strict objects use ESP frames; their actual null guards, context stores, qualified targets and cleanup were inspected separately. No whole-object identity between these build modes is claimed.

## Admission boundary

The retained final map has no new qualified getter/scope symbol or either C5 public update symbol: these interfaces remain unwired. The candidate's retained full-build/test receipts were inspected and hashed; no build or test was rerun here.

Root C50310's longer publication/tail cleanup scope remains separate. Collision, world-step (including enter/collision dispatch) and solver propagation remain normal-only. No actual radix owner/vtable producer is supplied. Custom heaps need their genuine matching provider or a separately reviewed context-bearing design. The C++ bridge cleanup does not establish rethrow transport through the unregistered naked frame, original FH3/SEH/fault/frame identity, secondary cleanup behavior, concurrency, ambient FP exceptions or gameplay acceptance. Those remain explicit prerequisites before runtime use; static source qualification is not their closure.

No new native/vendor-body, binary ABI, activation or gameplay credit is added. Exact source/object/proof hashes, normalized comparison records and emitted-path findings are in [the review report](../reports/cc16_dyn_profile_qualified_binding_review.json).
