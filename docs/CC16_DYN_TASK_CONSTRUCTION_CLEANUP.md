# Qualified Dyn task construction

`dyn_task_manager_construct_qualified_00c37740(storage,count,memory)` adds the recovered partial-construction cleanup to the existing normal task constructor. It uses the same three actual inputs and preserves the old API and worker code. Base: published `8faf5e9e4108ae79c647adb7a6a4c11728ad6882`; no private Root dependency is required. The bounded prerequisite was identified in readiness21448 and approved separately by the primary.

This packet changes only `dyn_task_manager.hpp/.cpp` and these two evidence files. C55EA0/C55F50, Game callers, context layouts, CMake, guards, ledgers and Ghidra state are unchanged. The new entry is a typed C++ interface and remains unwired.

## Original scope and exact actions

Live read-only BSP analysis, the original PE and all saved listing addresses agree on **174 bytes/58 instructions**, C37740..C377ED. Original stack arguments are `(manager*,count DWORD)`, EAX returns manager, and RET8 consumes them. The C++ count-reference/memory-service transport is separate from that ABI.

The original zeros task+C/+10/+14 at C3775F/62/65. Handler CC806F selects FuncInfo E03DF8, magic19930522, maxState2, EHflags1, map E03DE8:

| State | Previous/action | Acquired cleanup |
| --- | --- | --- |
| 0 | -1 / CC8059 | Armed C37771 before InitializeCriticalSectionAndSpinCount at C37775. Reload original task argument, addC, tail-call408E80: capture CURRENT queue data once; BF6989-release iff nonnull; leave fields alone. |
| 1 | 0 / CC8064 | Armed C37783 after initialization returns, without testing its BOOL. Reload original task argument, add18, tail-call real C30930→DeleteCriticalSection. Then unwind state0. Remains acquired through normal return. |

Helper408E80 is14 bytes/seven instructions and reads its vector data once. C30930 is eight bytes/three instructions: PUSH EAX, call DeleteCriticalSection through IAT CE2214, RET. Both helper transfers and the original FuncInfo/map/handler are pinned in the report. No original unwind frame or numeric original function pointer is installed by this source.

## New source contract

The new named entry snapshots count on entry, then performs the same three queue zeros **before its source outer try**. That try starts before real InitializeCriticalSectionAndSpinCount. The inner try starts after the call returns, regardless of BOOL, and retains the existing semaphore, shutdown event,100 group counters/events, worker-count zero and real worker setup sequence.

On a covered C++ exception in the inner extent, the real Win32 DeleteCriticalSection operates on actual task+18 and the exception is rethrown. The outer catch then reloads actual task+C, captures its current data once, invokes the genuine `memory.release(memory.context,data)` only when nonnull, and rethrows. A pre-CS-return source failure acquires only queue cleanup. Normal return runs neither action.

Cleanup performs no queue-field write; callback effects remain. Raw358h storage belongs to its enclosing allocator/caller and is not freed here. Semaphore/event/worker handles and partially reached effects retain the original ownership gap; there is no complete task destructor, worker join, rollback or replay policy. Count0 still leaves task+0/+8 untouched and retains the normal API's destruction precondition. Actual backing, Win32 services and queue allocation/release custody must remain valid; the BF6989 release must return normally. Arbitrary/manual callbacks or foreign heaps are not authenticated by the interface.

## Emitted scope qualification

Fresh MSVC19.51 Win32 `/MD /O2 /Gy /fp:strict /EHsc /W4 /WX` output has a220-byte normal entry,26-byte current-vector helper,29-byte handler and two catch/cold records of27 and35 bytes. The C++ FuncInfo has maxState4 and two catchall ranges: inner try1..1/catch2 performs DeleteCS/rethrow; outer try0..2/catch3 performs current-queue/rethrow. Thus the inner catch's rethrow is within the outer catch's protected range.

The exact compiler behavior is qualified: state0 is physically written at+34 **before** count read+3B and queue stores+43/+4A/+52, although the source stores precede the try. InitializeCS is+59; state1 is+6A after its return and before Semaphore+6E, with no BOOL-result branch. Both CreateEvent sites are+88/+A2; the unchanged worker setup call is+C0. No state demotion precedes FS restoration+CD. For valid inputs the hoisted prelude contains no C++-throwing call.

This evidence establishes the new C++ cleanup contract. It does **not** establish original initial-store/fault epochs, Win32 extern-C/SEH behavior, hardware-fault, FH3/frame/stack identity, or secondary-cleanup transport. The original maxState2 cleanup-only map and the source maxState4 catch/rethrow map are distinct. No `/EHa`, synthetic fault bridge or original registration is used to broaden that claim.

## Compatibility and verification

Strict baseline and changed Win32 object compilation passed without warnings. All **23 pre-existing function/helper/compiler records** have identical code bytes and normalized relocation offset/type/targets, including old constructor162B, worker setup222B, batch416B and destructor102B. Only anonymous namespace hashes are normalized. The existing constructor remains normal-only; the other public interfaces retain their previous contracts.

The new normal entry has five physical call sites in the same logical OS/worker order. Its catches call real DeleteCriticalSection and the current-vector helper; the latter loads data once and conditionally invokes the supplied BF6989 service. Fresh original call verification passed **2 direct rows/0 failures** (C377D2→C37690,408E87→BF6989). Five IAT sites are checked separately against original PE import identities and live instructions; their runtime target addresses are unclaimed. The report pins five live/PE spans, six exports, baseline/current source, strict objects/listings and static audits under ignored `local/dyn_task_construction_cleanup/`.

No full build, test, probe, fixture, game or DLL call was run. The primary owns independent review, common build/link and later C55 propagation. Existing normal/runtime evidence retains its prior exact scope. Native/vendor-body, binary-ABI, activation and gameplay credit are zero.

[Implementation report](../reports/cc16_dyn_task_construction_cleanup.json).
