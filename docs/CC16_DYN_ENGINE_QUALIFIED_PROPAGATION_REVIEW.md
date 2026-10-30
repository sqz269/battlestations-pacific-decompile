# Dyn engine qualified propagation independent review

Qualified pass for candidate `5c320a1f5b508a59862db6d8d055c2878c0a643a` against its exact parent `02a78bd1fc85cd7e2ff06450d1aa7edeae15ae94`. No source blocker was found within the valid-input C++ exception contract. This is admission evidence for two additive library entry points; it does not establish original fault transport or Game/application adoption.

The reviewed candidate consists of `include/bsp/dyn_engine_runtime.hpp`, `src/dyn_engine_runtime.cpp`, `docs/CC16_DYN_ENGINE_QUALIFIED_PROPAGATION.md` and `reports/cc16_dyn_engine_qualified_propagation.json`. This review changes only its own document and report. Descriptive names are source hypotheses. Original normal APIs, callers, storage/context layouts and the existing 32 code/eight EH records remain unchanged.

## Native coverage and ABI

| Original entry | Full span | Instructions | Original interface |
| --- | --- | --- | --- |
| `00C55EA0` | `00C55EA0..00C55F4E`, 175 bytes | 57 | Two stack arguments: engine storage and descriptor pointer; EAX result; RET 8 |
| `00C55F50` | `00C55F50..00C55FBE`, 111 bytes | 34 | ECX descriptor pointer; EAX existing/new/null engine; plain RET |

Reads used the existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. The complete listings, handler/actions and map words were reviewed; installed PE bytes and original direct-call targets were independently checked. The installed 12,223,752-byte executable has SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The seven direct rows are C55ECF/BF681B, C55EE7/C50310, C55EFD/BF681B, C55F1A/C37740, C55F74/BF681B, C55F8E/C55EA0 and 409B17/BF6989. The existing call checker passes 7/7. Separately, PE actions CC83F1/CC83FC/CC8411 call BF65AC unconditionally on original allocation captures. Three tail sites are CC83E8/409B10, CC8408/BF6B43 and CC841D/BF6B43. Vendor bodies were not reconstructed.

The native engine FuncInfo at E042CC has three states: state0 current-vector action CC83E5; state1 raw captured-9Ch action CC83ED with predecessor0; state2 raw captured-358h action CC83F8 with predecessor0. The ensure FuncInfo E042F8 has one captured-14h action CC840D. Both retain the original FH3 route. The 14-byte 409B10 and admitted A900 operations have equal normal storage/call effects: capture current first pointer, conditionally release it and leave fields unchanged. Their register/binary interfaces and fault behavior are not thereby equivalent.

## Real provider and dependency custody

The new named qualified APIs require a non-default flat raw-allocation delete binding. The genuine existing supplier is the same initialized `GameNativeDynProcess` owner: `.dynamics().engine` supplies its actual engine/profile publication cells, Root name and memory tuple; `.profile_partial_node_delete_00bf65ac()` validates that tuple against the private default process domain before returning the real unsized scalar `::operator delete`.

That domain allocates through actual scalar `::operator new` and releases vector backing through its existing `::operator delete` memory service. The original BF65AC raw-capture route and BF6989 vector route stay distinct contracts. A merely nonnull callback does not authenticate arbitrary manual/custom heaps, foreign cells or mismatched lifetime contexts. Cleanup providers must return normally. No synthetic provider, default cleanup, new global, new owner or fallback was added.

The candidate calls the real three-argument qualified Root C50310 overload, and the real qualified C37740 task constructor. The fresh task object defines the latter target. The task source/header exactly equal admitted `237e6c51838a56e326b6fe379a3bcaf8826156e2`, whose review is `e0b04478c60c9c923e0b29f38ab5c1ea98438e07`. Root `90e4fc72fb8e4c8aec0ea87d4a62e52eb96a6773` and review `cb53aed5b73fc20b343b6cf72e271d79c5025b38` supply the existing nested contract. Their earlier link/build evidence is not a new build/runtime result for this candidate.

All 85 file/Git pins in the author report independently match. The 18 readiness source pins were checked against readiness's own `64cd1530ec223a889d8fb88fa32a0c880ab72c6c` snapshot, not misrepresented as current task equality. The two pre-change engine sources match parent02a78. Current task, supplier, hull-memory, profile-interface and existing Game construction/dynamics dependencies remain unchanged relative to that parent.

## Source scopes and observable epochs

`dyn_engine_construct_qualified_00c55ea0` rejects a missing raw provider before caller-storage access. It initializes only engine+0/+4/+8, leaving +C/+10 until the native stores. Its outer vector catch encloses the 9Ch allocation, conditional qualified Root construction, profile publication into engine+C, 358h allocation, conditional qualified task construction and final engine+10 store/return.

The profile inner catch owns the captured original 9Ch allocation, covers only the qualified Root call, frees that capture unconditionally on a C++ exception and rethrows. It ends before engine+C publication and before the 358h allocator callback. Consequently a later task allocation failure does not free the completed profile/Root.

The task inner catch owns the captured original 358h allocation. Only after allocation and a nonnull test does the function read the volatile descriptor DWORD exactly once and call the real qualified task constructor. The null path skips both operations and stores null at +10. On failure the acquired task child actions precede original-358h raw deletion; rethrow then reaches the outer engine-vector catch.

The outer catch passes actual engine storage to A900, which captures CURRENT engine first/data once and conditionally releases that current pointer. It neither frees a cached initial vector pointer nor clears fields. It then rethrows. An enclosing qualified ensure frees its separate captured original14h allocation last. No parent publication clear, orphan deletion, handle close/join, callback undo or worker rollback was introduced.

`dyn_engine_ensure_qualified_00c55f50` first captures the volatile actual publication and returns nonnull before function-body provider validation, descriptor reads or allocation. Caller-side evaluation of supplier arguments still precedes entry. On the cold path provider validation precedes the 14h allocation; that allocation is outside its captured catch. After acquisition, conditional qualified construction, publication of the returned pointer including null, and return stay in that scope. It does not recheck callback replacement/publication before its final store.

## Compiler EH and physical qualifications

Fresh MSVC Win32 objects were compiled for exact parent02a78 engine code, candidate5c320 engine code and the unchanged admitted task source with `/std:c++17 /MD /O2 /Gy /fp:strict /EHsc /W4 /WX /c /FAs`. All three compile successfully without warnings. No `/EHa` or fault-conversion helper was used.

| New source function | Emitted public size | MSVC maxState | Catch regions |
| --- | --- | --- | --- |
| Qualified constructor | 210 bytes | 6 | profile try1..1/catch2; task try3..3/catch4; outer try0..4/catch5 |
| Qualified ensure | 125 bytes | 2 | try0..0/catch1 |

The sibling raw catches rethrow into the outer current-vector catch. Actual catch operands preserve original9Ch/358h/14h captures. No task destruction, CloseHandle, wait, signal or profile rollback appears in these new catches. Fresh baseline32/current40 code records and baseline8/current18 EH records equal the author's strict objects in all bytes and relocation offsets/types/targets, normalizing only the anonymous C++ namespace hash. The existing 32 code and eight EH records also remain equal between parent and candidate.

The optimized source physically installs state0 at constructor+40 before the initial stores +47/+4D/+54. This differs from native acquisition after those stores. Profile+C is stored at +7D before physical demotion at +8A; native demotes before its profile store. These intervals contain no C++-throwing operation on valid inputs; they do not prove corresponding hardware-fault coverage. Task allocation is +91, state3 +95, capture +9C, null test/branch +9F/+A1, sole volatile descriptor load +A7 and real qualified task call +B1. There is no descriptor preload across allocation.

Ensure's actual publication read +31 and nonnull branch +35 precede provider check +37, allocator +43, capture +48/state0 +4B and conditional constructor +5E. Publication +69 precedes handler restoration +6E. These facts establish the compiled C++ scope, not the original machine frame, fault epochs or EH metadata identity.

## Admission boundary

Qualified pass is restricted to normal valid storage/provider inputs and covered C++ exceptions with normal-return cleanup. Hardware faults, native SEH/FH3/private frame/register/stack transport, Win32 extern-C exceptions, secondary cleanup exceptions and original binary ABI remain unproved. The state hoist/demotion differences are explicit; no source/fault equivalence is inferred.

Existing Root std::memset versus native BF79F0 limits, task count0/BOOL behavior, partial worker/handle custody and no-join limitations remain. Completed profile/Root objects can remain orphaned; reached publications can remain stale or dangling after failure. The code supplies no safe retry/continuation license or automatic cleanup of those effects.

Old normal-only APIs and real callers retain their prior limitations. In particular `native_game_construction.cpp` still calls old `dyn_engine_ensure_00c55f50`; availability of these qualified APIs is not adoption. Whole Game/application owner attachment and shutdown/borrower lifetime closure remain separate work. No guard, CMake, ledger, Ghidra state or frozen peer was changed. No test, executable, allocation execution, common build/link, probe, DLL call or game was run for this review. No new native-body, vendor, ABI or activation credit is assigned.

The machine-readable review pins the candidate and retained/fresh static artifacts. Parent admission/common checks remain the integrator's responsibility.
