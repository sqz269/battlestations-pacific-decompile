# Native cockpit helper construction

Addresses: `00B3C800`. Source implementation, 2026-09-13, worker base
`20ace1358700e43c75035b55e6b8b0a0c68ee520`, branch
`agent/orch3-cockpit-constructor-bj`. This packet adds the complete normal body
in its admitted concrete domain and an explicit C++ exception projection. It
does not add ledger credit, source registration, a build or a fixture result;
the parent integrates the disjoint persistent-block implementation and verifies
the combined source. The block contract was read at design commit
`7b2c2a539c13fe384f47b1fac29ef8197c481786`, together with BG composition and the
current BI raw-camera admission source. Names are C++ hypotheses.
The exact sibling implementation dependency is
`125e6c93e07a527fb989a48142f546c7b3bb7461`; its header/source hashes are pinned
alongside this packet's source in the report.

| Routine | Inclusive body | Original ABI | Coverage |
| --- | --- | --- | --- |
| B3C800 | B3C800..B3C952, 339 bytes, 108 instructions | ECX actual 24h helper, no stack arguments, EAX same, RET | complete normal sequence in the admitted nonnull camera/viewport domain; five-state source exception projection |

Every live query used BSP CLI's target-verifying client for
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, `x86:LE:32:default`,
base `00400000`, bridge `127.0.0.1:8089`. The full B3C800 listing, all scoped
funclets, FuncInfo/map bytes, raw-string bodies and scalar leaf bodies were
read. No Ghidra mutation or export was made. The JSON report gives precise
call sites, owners and targets for the read-only call verifier.

## Interface and native order

`construct_native_cockpit_helper_00b3c800` accepts actual writable helper storage
and extent, references to the actual current D7A2F0/CE38B8 cells, an explicit
`NativeCockpitViewportReleaseContext`, and the prepared block capability.
The release context borrows the actual callable CE2220 IAT cell and actual
two-word D5E5F8 table. Its host bindings remain fixed and live through the
attempt; table words and the callable cell are volatile current reads at the
native boundary. There is no ambient next-token field or additional registry.
This is a new host ABI, not original caller/FH3/SEH compatibility.

Entry rejects missing/misaligned/short helper storage, missing table binding,
or an invalid capability before moving the caller token or any helper store.
The block validates all four credits, installed resolver identity, raw-name
mode, shared D7A24C cell, scene/lifetime relationship and existing providers.
The moved local capability enters executing before native callbacks. The
canonical static 0108FFB0 pool and environment pool must be the same actual
owner; no allocation probe or unrelated model-base pool substitutes for it.

Starting the typed 24h helper captures and restores only the four untouched
scalar words +10/+14/+1C/+20. The atomic counter starts its normal typed lifetime
and is set through its atomic interface; its object representation is not copied.
Native stores then follow B3C823..B3C84C: CEB130, count1, state0, D61854,
zero +08/+0C/+18. D61854 contains
`[BD30E0,B3C6C0]`; D6185C is the literal `CockpitCamera` including NUL.

| Site in B3C800 | Native target | Source composition and argument evidence |
| --- | --- | --- |
| B3C84F | B71930 | Actual static camera-pool allocation at its native event; no stack args. Wrapper overwrites ECX with 0108FFB0 and tails B71770. |
| B3C86C | 41E870 | Actual local 8h raw header, literal D6185C, RET4. Clears header, scans text, raw resize preserve1, copies current length+1. |
| B3C886 | B71A80 | After string success, emplace the persistent camera owner using scene credit. Forward that same header and record0 admission to raw BI construction; RET4. |
| B3C8AE | 419CC0 | Normal string return uses raw41DD20: capture data and wrapping length+1 before current getter, header untouched. |
| B3C8B5 | BD1510 | Return captured data, length+1, unused1 to returned actual pool; RET0Ch. |
| B3C8C7 | B6FBF0 | CURRENT D7A2F0 through FLD32/FSTP32; reload helper+0C between load and spill; near leaf RET4. |
| B3C8D9 | B6FC10 | CURRENT CE38B8 through FLD32/FSTP32; fresh helper+0C between load and spill; far leaf RET4. |
| B3C8E3 | B6FE10 | Fresh helper+0C, complete DWORD6; RET4. |
| B3C8F1 | B6FE20 | FLD1/FSTP32 then fresh helper+0C; RET4. |
| B3C8F8 | BF681B | Replacement raw allocation via existing admitted factory; PUSH34h, ADD ESP,4 at B3C8FD. |
| B3C90F | B1F850 | Construct replacement in actual allocation, register record1 before returning; no stack args. Save returned actual owner as native local EDI. |
| B3C923 | B71990 | Fresh helper+0C canonical camera; actual saved replacement argument, RET4. Existing publish/retain-new/release-old order, identity skip unchanged. |

After successful B71A80, admitted `NativeCameraReference` emplacement uses the
block's fixed retirement notification before helper+0C publication at B3C892.
It allocates no association storage and adds no native retain. The string-live
bit and cleanup state are consumed before the normal raw41DD20 call; a thrown
getter/return is not retried. No camera companion is emplaced before successful
local41E870. No helper companion is created inside this function.

Each of the five later camera operations resolves its freshly loaded +0C key
through the existing `find_actual_node`, checks `NativeCameraReference` with
`dynamic_cast`, checks host live phase before raw storage use, and verifies the
exact actual key/runtime. There is no cast of raw storage to a companion or
fallback to the originally allocated owner. A callback may publish another
canonical live camera, whose owner becomes the receiver at the next native
load. Host projection runs after each x87 spill, with no x87 value kept live
across RTTI or registry code. All three load/spill sequences are explicit MSVC
Win32 inline assembly; generated instruction verification is pending integration.

B3C92C calls the current borrowed CE2220 callable on the captured local
viewport's +04, never a reloaded camera+180. Only zero reads the same local
owner's current profile and current slot0. D5E5F8/BD30E0 is the supported pair;
BD30E0's current slot4 must be B1F8F0 before calling the existing concrete
implementation with flags1. Missing/changed unsupported bindings diagnose at
that boundary. Prior stores/decrements are preserved and never retried.

## Native unwind map and host settlement

FuncInfo DF74C4 has magic19930522, maxState5 and map DF74E8..DF750F.
Handler CBED14..CBED1D loads that descriptor and tails FH3 at BF6B43.

| State | Next | Complete scoped native action | Source ownership |
| --- | --- | --- | --- |
| 0 | -1 | CBECE0..CBECE7: helper at EBP-1C; CBECE3 JMP BD30F0 | Restore CEB130; end failed helper typed lifetime, no allocation free. |
| 1 | 0 | CBECE8..CBECEF: slot at EBP-18; CBECEB JMP B71350 | Return exact captured unconstructed camera slot, then helper base. |
| 2 | 1 | CBECF0..CBED08: clear saved bit0, CBED03 JMP41DD20 on EBP-14 | Consume live string, then state1 and base. |
| 3 | 0 | Same conditional string funclet | Map represented, but no native normal assignment and no source assignment to state3. |
| 4 | 0 | CBED09..CBED13: saved raw viewport, CBED0D CALL BF65AC, POP ECX | Existing factory owns exact failed allocation free, then outer base. |

The source marks factory-owned state4 before invoking the shared factory;
native code arms it after BF681B returns. This host marker adds no raw-free
obligation: allocator failure reaches only helper base; B1F850 failure first
runs its own base cleanup, then the factory frees only its captured allocation,
then helper base runs. Returned live replacement ownership is never placed in
automatic cleanup. State0 is restored before current-camera lookup/B71990.

When BI rejects while an emplaced camera owner is still prepared, the caller
abandons only that preparation before raw-slot return. Dead callee-unwound
owners remain in the block. Early camera failure cancels the unused viewport
record; late camera failure can leave its first viewport live even though the
camera typed lifetime ended and raw slot is returned. Never recover that orphan
through ended camera fields or automatically release it.

`camera_completed` is an explicit HOST-only fact. If reference emplacement
unexpectedly diagnoses a competing/invalid binding after B71A80 returns, the
source cleans the still-live local string and helper base but preserves the
completed camera and raw slot. This is outside the valid admitted equivalence
domain, not an original native throw or a state3 assignment. It may pin the
block indefinitely pending external diagnostic recovery.

After helper publication, any string/setter/replacement/dispatch failure has
only the evidenced base obligation. B71990 or final local decrement/dispatch
failure adds no automatic replacement decrement. Nested throwing C++ cleanup
terminates rather than inventing unrestricted FH3/SEH behavior. Local capability
destruction then cancels only unused credits and marks the block settled;
it does not reset companions, forget records, return slots or release survivors.

## Verification and limits

The worker read the complete 339-byte body and scoped EH map/funclets, raw
41E870/41DD20 bodies, all four scalar leaves, B71930/B71350, B71990, B1F850,
BD30E0/BD30F0/B1F8F0 and BF681B normal listing. Direct/tail call rows are
checked with `tools/verify_report_calls.py`; indirect rows were manually checked
at their actual containing call sites. Source/dependency hashes and verifier
results are in the report. No native callee stubs or dependency body credit
were added. No CMake, ledger, Ghidra or other source file is owned here.

There are no worker builds, tests, fixture executions or generated-x87 claims.
The sibling block header/source are a coordinated dependency absent from this
base. Parent integration must build both packets together, inspect generated
FLD32/FSTP32 and FLD1/FSTP32, and evaluate the smallest actual-constructor
comparison. Prior BI success/source failures exercised B71A80, not B3C800.

The nonnull raw allocator, actual pool, raw-string, real renderer publications,
CRT and canonical table/profile domains remain requirements. Native null
branches do not constitute successful fallback: missing canonical camera or
null viewport is explicitly unsupported in source. Existing B71990 and other
callee internal reference operations retain their concrete Win32/profile domain;
the new final-release context does not extend them to arbitrary patched IATs
or vtables. Unknown/noncamera current +0C remains a domain error.

The caller independently owns the actual 24h allocation and stable helper
companion storage, binds helper owner/reference only after successful return,
and publishes only after those bindings succeed. Blocks remain persistent
through every actual native survivor and retire/reset only after explicit host
quiescence. Independently prepared nested blocks are supported; destructive
callback liveness, semantic view/cache borrow completion, concurrent mutation,
outer B14A10/render-service construction and game execution remain unproved.


## BJ combined integration validation

Exact source `dd86c2152a814a352ee0e2f3c3b503fc678b0008` passed the strict MSVC Win32 build, both existing CTests, eight seed spans and the current-library actual constructor fixture. One full mapped-original/source B3C800 constructor-through-helper-terminal pair at x87CW027F: 12 live checkpoints, 13392 camera bytes and 432 helper snapshot bytes per path, 12 ordered events, 14736 identical normalized snapshot bytes per path and 36 identical normalized helper bytes before free. The 9492 mapped code bytes remain unchanged after both paths and the failure observation. One source-only renderer-call2 failure verifies unregistered first-viewport cancellation/free, both local/camera raw-string returns, exact primed camera slot/pool return, helper base/count1/zero camera representation, absence of helper companions, settled block and one explicit host-quiescent reset before caller-owned helper free. No original FH3 exception path is executed. Independent source and fixture reviews found no issues within those domains. See `reports/native_cockpit_constructor_bj_validation.json` for exact source/library pins and sealed evidence. Original replacement ABI, unrestricted native FH3/SEH, arbitrary native write-trace parity and gameplay remain unvalidated.
