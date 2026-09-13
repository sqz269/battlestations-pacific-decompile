# Native model-base lifetime and default factory

Addresses: `00B743C0`, `00B743E0`, `00B748D0`, `00B74B60`, `00B74EC0`, `00B86720`.
Descriptive names are hypotheses. Base: `dbf67b7ac228f2de90aab0d00e58dfba047de14d`.
Read-only live queries verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, bridge8089; no analysis mutations.

The default resource factory now has a real D62D78 source provider. It constructs the actual174h node in an actual178h pool slot and exposes a separate persistent companion over actual+04. The companion registers in `GeneratedModelLifetimeRuntime`, so `find_actual_node(actual_key)` returns the same lifetime; it adds no registry, copied reference count or physical owner substitute. D62DE8 generated-model references remain a different concrete profile.

| Entry and full extent | Original ABI | Coverage |
| --- | --- | --- |
| B743C0..B743D8 (25 bytes) | ECX owner, stack actual8h name header, EAX owner, RET4 | complete |
| B743E0..B74407 (40 bytes) | ECX unused, stack token, AL bool, RET4 | complete leaf (disk listing; Ghidra has no defined function) |
| B748D0..B748DB (12 bytes) | ECX raw slot, RET | complete |
| B74B60..B74B7F (32 bytes) | ECX owner, stack flags, EAX original, RET4 | complete |
| B74EC0..B74EC9 (10 bytes) | incoming size ECX discarded, tail B6EB00, RET | complete |
| B86720..B8677E (95 bytes) | ECX unused, stack name header, EAX node/null, RET4 | complete source body and C++ failure cleanup; original FH3 identity excluded |

## Native calls and complete preparations

| Caller/site | Callee | Input setup and cleanup |
| --- | --- | --- |
| B743C0 / B743C8 | B6F5A0 | EAX=[entry ESP+4] actual name header; PUSH ESI; PUSH EAX; ESI=ECX owner. Callee RET4. Stamp D62D78 only after successful return. |
| B74B60 / B74B63 | B6F440 | PUSH ESI, ESI=ECX actual owner; no explicit arguments; callee RET. |
| B74B60 / B74B75 | B6E490 | Only low flags bit1: PUSH ESI original slot; ECX=0109008C; callee RET4. |
| B86720 / B8673B | B74EC0 | Establish FH3 frame and scratch slot; ECX=174h, no stack argument; allocator replaces ECX with actual pool. |
| B86720 / B86756 | B743C0 | Save EAX allocated slot at ESP; state[ESP+C]=0; skip on null; ECX=[ESP+14] original caller name; PUSH ECX; ECX=EAX raw slot; callee RET4. |
| B74EC0 / B74EC5 | B6EB00 | ECX=0109008C; tail JMP, unchanged return address. |
| B748D0 / B748D6 | B6E490 | PUSH ECX actual slot; ECX=0109008C; callee RET4, wrapper RET. |

Containing Ghidra bodies were checked for every defined call site. Caller queries report B86720 as the sole direct constructor/allocator caller and CC2530 as the return-wrapper caller. Scalar/type use table dispatch; no direct factory xrefs are present. Full report rows include numeric site, callee, preparation, byte seals and extents. The default factory selection evidence is in `NATIVE_COCKPIT_NODE_PRODUCER_BE.md` from the separate evidence worker; it does not prove the actual runtime asset chooses this default.

Factory handler CC2538 loads FH3 info DFB970: maxState1, unwind map DFB968 = [-1, CC2530]. CC2530 loads saved allocation `[EBP-10]` into ECX then tailcalls B748D0. Source catch returns the same allocation once and rethrows. Scalar has no local unwind; a throwing B6F440 must not return its physical slot. The source still forgets the ended scene binding after member cleanup on failure.

## Pool, type and companion contract

0109008C is the actual38h node pool, not plain-node0108FF58. CD7F20 selects0109008C and calls B6E980, then registers CE0E60 which selects the same pool and jumps B6E3D0. The existing allocator provider initializes profile D62C78, its real Win32 critical section, shared allocator-list element and slab table. Slots are178h with174h payload and trailing index. This module borrows that genuinely initialized owner; it does not implement static startup or invent a zeroed pool.

D62D78 current table checks cover terminal00=BD30E0, scalar04=B74B60, type0C=B743E0, scene/matrix34=B6E870,40=B6DBE0,50=B6ED80,54=B6EE10. Logical18=B6F310 is checked at dispatch. B743E0 compares current cells01090044,01090048,0109004C in order, short-circuiting on a match. Their initialization remains an upstream requirement. The companion stores borrowed cell references and owns `scene_attachment.context`; B6F440 installs the distinct runtime node-phase predicate after writing D62C88. The original binding context is captured at companion construction and restored immediately before the terminal B6F440 call, including its throwing path, so node-phase dispatch receives its existing context.

Raw name construction and destruction use the exact same `NativeStringRawPoolContext`, whose publication cells and current getter/providers remain live. The reference constructor rejects mismatched runtime/name contexts, wrong actual profile/table and nonpositive count. It uses actual+04, preserving its current value. No retain occurs at registration. Existing logical-release provider B6F310 handles released44, actual point-light descriptors, scenes and final release. BD30E0-style terminal dispatch checks current00/+04, invokes scalar flag1, unbinds the existing lifetime, then calls explicit companion retirement. The retirement callback may delete this reference and node binding; there is no subsequent access.

`create_native_model_base_00b86720` returns the actual node only. Its consumer must create stable canonical `NativeNodeBinding` and `NativeModelBaseReference` companions, register the scene binding once and retain them for the actual lifetime. The module deliberately supplies no guessed resource asset selection, model geometry provider or default owner registry.

## Verification and limits

Strict Win32 source TU compilation uses `/std:c++17 /EHsc /O2 /W4 /WX /fp:strict /MD`. The integrator owns central CMake registration, so this worker's full baseline build does not include the new module; the external lifecycle probe explicitly links this TU and is a worker-source control. It reuses the established plain-node lifecycle fixture for the recovered model-base scalar and original BD30E0 dispatch, actual pool return, same canonical lookup, logical release, current token changes and reentrant companion retirement. Original destructor/scalar/terminal byte arrays are sealed against the original Steam PE; callable seams use recovered source providers. Factory construction is source-side smoke coverage, not independent original factory differential proof. Ordinary C++ destructor failure checks no physical return. No original exception identity, asynchronous store observation, game reachability or gameplay result is claimed.

The final report records actual build/probe outputs and SHA256 of source, original byte evidence, recipe, executable and all three linked libraries. No committed test suite or shared test change is added. Game asset choice remains unresolved: plane+808 is a borrowed named partSet node; D62D78 is only the default, group D634F8 and resource overrides remain possible.

Verification completed: strict module and external probe compilation passed; The initial baseline had reconstructed_math 1/1. Guarded verify-seeds then matched all8 original spans and enabled the existing native test: incremental `scripts/build.ps1` passed2/2 reconstructed_math and native_math_differential. All3 linked library hashes remained unchanged. The worker-source lifecycle probe passed all existing cases, including restored original context observed at retirement and live token changes. The executable manifest was extracted and verified asInvoker. No original factory differential or real node-phase type callback reachability is inferred from these results.

The existing source failure fixture deliberately leaves a retained-owner binding unresolved and observes std::logic_error. This is a source-domain cleanup check, not raw getter bad_alloc coverage or execution of an original native exception path.
