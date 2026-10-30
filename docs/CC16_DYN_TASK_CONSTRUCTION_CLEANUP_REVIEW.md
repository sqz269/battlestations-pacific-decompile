# Dyn task construction cleanup review

Independent qualified pass of `237e6c51838a56e326b6fe379a3bcaf8826156e2`,
based on `8faf5e9e4108ae79c647adb7a6a4c11728ad6882`. No blocking discrepancy
was found. The result covers the named three-input C++ partial-construction
cleanup contract for valid actual inputs. It does not establish native fault,
SEH, original initial-store epochs, binary ABI or caller activation.

## Native acquisition and actual inputs

Read-only BSP wrappers checked the configured `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` before analysis batches. Five independently
captured live spans match the original PE, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The full `C37740..C377ED` body is 174 bytes/58 instruction starts; native
stack inputs are manager/count, EAX returns manager and `C377EB` is `RET 8`.
The new reference/service interface is distinct from that original ABI.

Original queue zeros at `C3775F/62/65` precede state 0 at `C37771` and real
InitializeCriticalSectionAndSpinCount at `C37775`. State 1 is armed at `C37783`
after the call returns, with no BOOL test. This is cleanup acquisition, not
proof that an ordinary API result denotes successful initialization.

`CC806F` selects FuncInfo `E03DF8`, maxState 2, EHflags 1 and map `E03DE8`:

| State | Predecessor | Action |
| --- | ---: | --- |
| 0 | -1 | `CC8059`: reload original task, add C, tail `408E80` |
| 1 | 0 | `CC8064`: reload original task, add 18h, tail `C30930`, then state 0 |

The seven-instruction `408E80` captures current vector data once and invokes
`BF6989` iff nonnull, with no field clear. The three-instruction `C30930`
passes actual task+18 to imported DeleteCriticalSection. Both direct calls,
five IAT sites and three tails are independently checked against the original
PE; IAT import identities are not forced runtime callee addresses.

The qualified API borrows the same actual aligned 358h storage, count word and
`AvoidZoneDynHullMemory` tuple as the old API. Layout and services are unchanged.
It uses actual Windows declarations/imports and the genuine release callback,
with no dummy provider, new context field/global or fallback free. The current
default process tuple is scalar `::operator new`/`::operator delete`; arbitrary
manual callbacks/custom heaps are not authenticated by the interface. Matching
queue allocation/release custody, stable backing/services and normal return
from release remain required. No default-domain supplier or caller is rebound.

## Source effects and emitted C++ scope

The source snapshots count and zeros queue+C/+10/+14 before its lexical outer
try. That try begins before initialization. After initialization returns,
regardless of BOOL, the inner try covers semaphore/events, the 100 group
counters/events, worker-count zero and the unchanged worker setup call.
Covered C++ failure executes actual DeleteCriticalSection(task+18), rethrows,
then the outer catch computes task+C and captures its current data once for
conditional `memory.release(context,data)`, followed by rethrow. Normal return
executes neither cleanup action.

The independently decoded source FuncInfo has maxState 4, EHflags 1 and two
standard C++ catch-all entries. The SDK type definitions and flag constants
were checked locally; no runtime handler body was reconstructed.

| Source try range | Catch state | Handler action |
| --- | ---: | --- |
| 1..1 | 2 | Actual DeleteCS then C++ rethrow |
| 0..2 | 3 | Current-queue conditional release then C++ rethrow |

The inner catch's state 2 lies within outer range 0..2. Its rethrow therefore has the
required outer queue handler under the compiler's C++ EH contract. The source
unwind map has four entries with null action pointers; these catch/rethrow
tables are not the original two-entry cleanup-only map or native frame.

The physical emission qualification is essential: state 0 at +34 precedes the
count-value read at +3B and queue stores at +43/+4A/+52. InitializeCS is +59;
state 1 is +6A after return, with no BOOL branch. Semaphore is +6E, the two
CreateEvent calls are +88/+A2, worker setup is +C0 and FS restoration is +CD.
Only state 0/state 1 are written on the normal path; there is no earlier demotion.
For valid inputs, the hoisted prelude has no C++-throwing call. This supports
the qualified C++ semantics while explicitly excluding native initial-store,
Win32 extern-C/SEH, hardware-fault and FH3/frame/stack epoch equivalence.

Cleanup does not clear queue words, free raw 358h storage, close any reached
handle, stop/join workers, invoke the full destructor or repair orphans. Reached
effects and potentially dangling queue fields remain. Count 0 still leaves
task+0/+8 untouched, including the old +0=null precondition for later normal
destruction. Secondary-cleanup transport and safe continuation/replay are
unproved. C55/raw storage custody and complete engine/Game propagation remain
separate; the actual existing caller still selects the normal-only API.

## Independent static verification

Fresh baseline and qualified MSVC Win32 objects passed
`/std:c++17 /MD /O2 /Gy /fp:strict /EHsc /W4 /WX /c /FAs`.
All 23 old function/helper/compiler records have identical code bytes and
relocation offset/type/targets after only anonymous-namespace hash normalization.
The old public constructor/worker setup/batch/destructor remain 162/222/416/102
bytes. Exactly five new records measure 220/26/29/27/35 bytes; the final inner
catch region includes six padding bytes. Fresh baseline/current records match
the retained primary emissions, including the real cleanup calls and metadata.
No `/EHa`, synthetic fault transport or original FH registration was added.

Both source variants, two dependency pins, six exports, 19 retained artifacts
and both primary objects/listings match their recorded hashes. The existing
call checker passes 2 direct rows with 0 failures and explicitly skips the five
indirect rows; those imports/instructions and the three tails are checked
separately. The independent target receipt captures both output streams rather
than relying on the primary's empty target stdout file.

Rerunnable static verification, fresh objects/listings and live receipts remain
under `local/dyn-task-construction-cleanup-review/`. `object_audit.json` SHA-256:
`3549ec69106e83801f6aae0c02a0fc17dec74bc6c1020d44b960c5304d94f362`.
The review report preserves all input and artifact records with local hashes.
Later primary common-build/link results are outside this frozen-candidate proof.

Only these two review files change. No source/CMake/shared metadata, Ghidra
mutation, new test, fixture, full build, native/object execution, game, probe,
DLL call or guard flip occurred. Native/vendor body, original ABI/fault
transport, activation and gameplay credit remain zero.
