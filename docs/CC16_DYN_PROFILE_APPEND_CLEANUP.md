# Dyn profile partial-construction cleanup

This packet repairs the C++ exception cleanup of the existing C50390 append
and C44000 constructor implementations. It also supplies the missing 14-byte
0040A900 vector-buffer cleanup helper. The old four-argument append,
three-argument constructor and `DynProfileScopeContext` layout remain intact
and retain their normal-path qualification. Descriptive names are hypotheses.

| Original body | Size / instructions | Original ABI |
| --- | --- | --- |
| 00C50390..00C50469 | 218 bytes / 71 | ESI parent 48h node; stack name/id; EAX reloaded appended entry; RET8 at C50467 |
| 00C44000..00C44081 | 130 bytes / 46 | EDX name; stack node/id; EAX node; RET8 at C4407F |
| 0040A900..0040A90D | 14 bytes / 7 | EAX actual vector; plain RET; no defined result contract |

The read-only wrappers verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32. Installed original PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report pins all three bodies, both handlers/actions/maps and FuncInfo
records. No Ghidra mutation, old FH registration or vendor reconstruction is
included.

## Original scopes and ownership

C50390 calls BF681B at C503A9 to allocate 48h bytes, saves that original
allocation at C503B1 and enters state0 at C503B7. Only the nonnull allocation
enters the real C44000 constructor at C503CB. State resets to -1 at C503D6,
before the parent vector can grow. Its BF55BE allocation is C503F6; the normal
old-vector BF6989 release is C5042F. Capacity is committed before allocation,
and the final result reloads the appended entry. These existing store/callback
epochs, null branches and unsigned wrapping arithmetic are unchanged.

Handler CC8383 loads FuncInfo E041FC and tail-jumps to the original FH3.
`maxState=1`, unwind map E041F4 is
`ff ff ff ff 78 83 cc 00`: state0's predecessor is -1 and action is CC8378.

```text
CC8378  8b 45 f0             mov eax,[ebp-10h]   ; captured raw allocation
CC837B  50                   push eax
CC837C  e8 2b e2 f2 ff       call BF65AC
CC8381  59                   pop ecx
CC8382  c3                   ret
```

Thus an allocation exception is outside the cleanup scope. A constructor
exception deletes the original allocation. A later parent-growth exception
does **not** delete the completed orphan or roll back the committed capacity.

C44000 zeroes child vector +4/+8/+C at C4401F..C44025, then enters state0 at
C4402B. Its existing SBO name setup, scan and real counted assignment at
C44050, followed by id/tick/call stores, remain in their original order.
Handler CC81F2 uses FuncInfo E04014; its one-entry map E0400C is
`ff ff ff ff e7 81 cc 00`.

```text
CC81E7  8b 45 04             mov eax,[ebp+4]     ; original node argument
CC81EA  83 c0 04             add eax,4
CC81ED  e9 0e 27 74 ff       jmp 0040A900

40A900  8b 00                mov eax,[eax]       ; CURRENT vector data once
40A902  85 c0                test eax,eax
40A904  74 07                jz 40A90D
40A906  50                   push eax
40A907  e8 7d c0 7e 00       call BF6989
40A90C  59                   pop ecx
40A90D  c3                   ret
```

This inner action releases current vector data only when nonnull, without
clearing data/count/capacity. It precedes the distinct outer raw-node delete.
The source makes the cleanup data load volatile so callback mutation cannot
be replaced with the constructor's earlier zero. It adds no parent rollback,
name destructor, fully constructed orphan deletion or raw-node free to A900.

The two vendor entries are distinct even though original BF6989's five-byte
JMP forwards to BF65AC, whose JMP forwards to BF9DC8. An opaque source
`memory.release(context,p)` does not establish a genuine flat BF65AC provider.
The new API therefore requires that binding explicitly; no generic release,
host global delete, null/default cleanup or vendor body is substituted.

## Additive source contracts and caller adoption

The new constructor overload takes `(node,name,id,vector_memory)`. Its vector
zeroing precedes the source `try`; a C++ exception from the real payload
performs A900 with `vector_memory.release`, then rethrows. The new append
overload takes `(parent,name,id,memory,delete_partial_00bf65ac)` and uses that
real qualified constructor. Its catch surrounds only construction and calls
the required flat `DynProfilePartialNodeDelete` on the captured allocation,
then rethrows. Both releases must be genuine matching-heap providers returning
normally. The existing real SBO implementation/allocator is reused.

Accepted C++ consumers in scope-enter, `native_dyn_collision_pass.cpp`,
`native_dyn_solver_mode1.cpp` and `native_dyn_world_step.cpp` still call the
old normal-only overload. C50310's root construction also keeps the old
constructor overload. No caller or context layout is changed here. BigC5
must separately adopt the new append and supply its genuine BF65AC binding.

The existing original caller audit has 18 C50390 sites in seven functions:
C57020 (1), C57070 (6), C5A8B0 (5), C5BB30 (3), C5C540 (1), C5C710 (1) and
C5C7A0 (1). The report preserves the exact site inventory. None receives new
failure-path qualification merely because this overload exists.

## Validation and limits

Strict MSVC Win32 `/EHsc /fp:strict /O2 /Ob2 /MD /W4 /WX` object compilation
passed for both production modules. The common Release build and both existing
checks passed. Release excludes the conditional Debug fixture below.

Standard Debug failed before reaching the fixture: unrelated Debug STL layout
assertions, `/Od` unreachable-code warnings and a Debug-layout padding warning
produced 14 error records. That failed log is preserved. Root approved a
separate optimized Debug configuration of the **existing** math target:
`/D_DEBUG /MDd /Zi /O2 /Ob2 /D_ITERATOR_DEBUG_LEVEL=0`, without incompatible
`/RTC1`, retaining `/W4 /WX /fp:strict`, `/MP2` and two MSBuild nodes. That
build and its `reconstructed_math` check passed, exercising the Debug CRT hook
block. This is optimized Debug fixture coverage, not a standard Debug pass.
Final logs, actual Debug CRT import evidence and artifact hashes are in the
report.

The single added Debug-only block in the existing math fixture runs the actual
constructor and SBO allocator. Its fixture-thread/size-filtered CRT hook
rejects the real 17-character name's 32-byte allocation and 18-byte fallback;
the first hook observes zeroed vector fields and supplies preallocated vector
storage. The rejected 32/18-byte order is asserted. Preallocated observations
verify current-data BF6989 release before
distinct original-allocation BF65AC and rethrow of the second real failure.
A second boundary in that block throws from parent-vector growth after a real
short-name constructor, checking no delete/rollback and committed capacity.
RAII restores the previous allocation hook/new handler, and the check verifies
both actual restored values. These are qualified
host CRT fixtures, not original-machine unwind execution.

Optimized append object code places allocation before state0, the real
qualified constructor inside it, and state reset before the shared append
helper. Its catch invokes the required flat delete and rethrows. A900 emits
one current-data load, nonnull branch and provider call with no stores.
The optimized `/EHsc` constructor moves its compiler EH-state write before
the nonthrowing zero stores, despite source zeroing being outside `try`.
This does not expand the C++ throwing scope, but it supplies no original
SEH/access-fault/frame-transport equivalence. Invalid storage, original FH3
transport, binary ABI compatibility, unadopted callers and game/runtime proof
remain outside the claim.

Repaired existing C50390/C44000 bodies receive zero new body credit. A900's
separate 14-byte missing C++ body is recorded for root ledger review. Vendor,
ABI and gameplay credit are zero. No game, probe, install, profile, hardware
or power action was performed.
