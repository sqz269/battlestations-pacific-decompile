# GOAL15F complete avoidance singleton getter

The reviewed `BSP_AvoidZoneManager_GetSingleton` name remains a hypothesis.
The historical `mission_load_hosts.cpp` partial-projection record/API is
preserved. This packet adds a distinct complete concrete Source entry rather
than relabelling that earlier projection.

| Routine | Inclusive Native body | Original ABI | Coverage |
| --- | --- | --- | --- |
| `004218E0` | `004218E0-0042199C`, 189 bytes | cdecl, no arguments or incoming register inputs; EAX result; plain RET | complete fast/slow operation and recovered captured-allocation/critical-guard cleanup via current Source C++ EH |

Live analysis used the target-verifying `bsp.py` bridge: project file
`C:/Users/sqz269/bsp.gpr`, project `bsp`, program
`/battlestationspacific.exe`, x86 LE32, base `00400000`. The whole 51-instruction
listing, bounds, capped caller references, full FH3 metadata/actions and
consumed provider bodies were read. The function has no byte gaps. Typed
flow/no-return properties are not exposed by the bridge; no repair or Ghidra
write was made. Static callers are not gameplay-path validation.

## Actual Source cells and providers

`NativeAvoidSingletonContext` contains only stable borrowed addresses:

- `void* volatile* actual_01090aa0_publication_cell` for the actual raw lifetime
  manager publication.
- `Cc13AvoidZoneManagerNative** actual_00e17624_publication_cell` for the actual
  avoidance publication.
- `void* const* actual_00e188a8_publication_cell` for the actual game publication.

The address bindings/context must remain valid and fixed during the operation;
publication values may change. There is no private global, shadow cell, cached
publication snapshot, projected lifetime domain, replacement manager or
injectable allocation/registration callback. Source uses the existing actual
singleton publication/registration services, current singleton allocation/free,
real Win32 critical APIs, the existing raw guard cleanup service and accepted
complete raw avoidance manager constructor. Its constructor context borrows
the same avoidance/game cells and is live through constructor unwinding; the
manager does not retain that local context pointer.

The Source signature is
`get_native_avoid_manager_004218e0(const NativeAvoidSingletonContext&)`.
It is cdecl with an added explicit reference argument. The original no-input
callable ABI and Ghidra prototype remain separate. Source guard service also
uses its established cdecl pointer interface, while its Native entry consumes
ECX. No invisible register/context recovery is asserted.

## Complete normal schedule

1. Capture the first actual avoidance publication. A nonnull value returns
   that captured pointer, without looking up a lifetime manager, taking a lock,
   registering again or reloading the publication.
2. Otherwise call the existing lifetime getter through the actual 01090AA0
   cell. Capture the returned manager's raw section +10h exactly once. Publish
   the two-DWORD guard prefix with profile CE37FC and that section. If nonnull,
   call actual EnterCriticalSection, then increment the raw +18h depth bits.
   Guard state 0 starts only after that return and increment.
3. Recheck the CURRENT actual avoidance publication. If it is now nonnull,
   skip allocation, construction and registration, but retain the normal
   captured-section release and final publication reload.
4. If still null, allocate an actual current 78h raw block. State 1 begins only
   after allocation returns, then conditionally construct a nonnull block using
   the accepted manager constructor and exact existing 78h storage schema.
   The null branch returns a null constructor result; no fake successful owner
   or null allocation substitute is introduced.
5. End allocation cleanup state 1 BEFORE storing the actual constructor result
   into the avoidance publication. Resolve the CURRENT lifetime manager again
   through the actual 01090AA0 cell; only AFTER that return reload the CURRENT
   avoidance publication and register that value with the returned manager.
   Do not reuse the allocation pointer, constructor result or first manager for
   this registration. Actual registration retains its returning-invalid-handler,
   argument-slot, vector, allocation and no-internal-lock contracts.
6. If the original captured section is nonnull, decrement its current +18h
   depth bits, then actual LeaveCriticalSection. Slow return reloads the actual
   avoidance publication after Leave; the critical guard stays armed through
   release/final reload. It never releases a newly looked-up section instead.

Raw counter arithmetic is unsigned DWORD wrapping, preserving original ADD
bit behavior rather than adding a signed-overflow precondition. Compiler
barriers around memcpy DWORD accesses preserve fresh reads and observed store
order without adding CPU fences or C++ type-alias assumptions. Captured lock
protocol is evidence; a broader concurrency/publication safety claim is not.

All five direct Native calls are:

```text
00421906 CALL 00415350
0042193C CALL 00BF681B
00421953 CALL 00421500
00421966 CALL 00415350
00421974 CALL 00BD0C30
```

The two Native SDK imports are EnterCriticalSection through CE2218 at
0042191F and LeaveCriticalSection through CE2210 at 00421982. Source binds the
actual corresponding current Win32 imports. The first existing lifetime getter
itself has a complete actual allocation/construction/publication error
contract; its first return is not a new replacement manager.

## Recovered Native EH and current Source guards

Handler `00C5E523` loads FuncInfo `00D84968` and tail-transfers to the existing
runtime handler. The metadata has magic `19930522`, maxState 2, unwind map
`00D84958`, zero catch/IP-map/exception-spec entries and EH flags 1.

| State | Next | Action | Complete consumed operation |
| --- | --- | --- | --- |
| 0 | -1 | `00C5E510` | form guard `[EBP-14h]`, tail-transfer to the existing guard cleanup; capture guard+4, write guard profile, conditional raw depth decrement then actual Leave; preserve guard+4 |
| 1 | 0 | `00C5E518` | free captured raw allocation `[EBP-18h]` through original free, then return so state 0 cleanup follows |

Allocation failure before an allocation returns has only guard state 0.
Constructor failure first performs the constructor's own recovered cleanup,
then frees the captured raw block, then releases the captured guard. State 1
ends before publication/relookup/registration. Thus registration failure keeps
the already published allocation/current publication; it performs only guard
cleanup, without a new manager destructor, rollback, unregistration or free.
Normal decrement/Leave also remain within state 0. An exception there may
reach guard cleanup with earlier effects intact; no repair/retry is invented.
These unowned helper/action addresses are consumed evidence only.

Current Source uses nested real C++ scope guards. The allocation guard's scope
ends before publication, and the critical guard remains active through final
slow reload. Successful cleanup allows the original current C++ exception to
propagate; no catch replaces it. An unexpected exception during noexcept
cleanup uses current SDK termination policy. The actual guard provider reads
the actual eight-byte local prefix; its normal captured lock is retained
separately, as in the Native operation.

The optimized Source main body is 287 bytes. It emits actual current
`__CxxFrameHandler3` registration, Source state 1 around construction and
state 0 before publication/second lookup, and two real cleanup funclets leading
to actual free/guard services. Current compiler FuncInfo maxState is **3**:
besides the two cleanup entries it adds a `std_terminate` entry. This is an
explicit Source runtime difference, not a Native map match. Original mutable
EH-spill/caller-frame aliases, register/argument-slot identity, hardware faults,
FH3/SEH frames, cookies and private CRT/heap/exception/OS behavior are not
transplanted. No exception-path execution claim follows from emitted metadata.

## Registered ownership and dispatch boundary

Registration stores the actual reconstructed manager pointer/table in the
real raw singleton vector. Its Source scalar deleter requires an explicit
lifetime context in EDX. Original raw virtual CALL EDX supplies the target
there and therefore remains incompatible. This getter does not change that
original protocol or install automatic production wiring.

Primary's independent finite **Source** singleton-dispatch integration adds an
explicit avoidance-manager lifetime binding, admits the recovered Native
derived profile and actual Source table, and calls the genuine context-bearing
scalar. An actual Source parent drain can therefore consume the registered
entry when that explicit binding is supplied. This finite supported-profile
service is separate from arbitrary original Native virtual dispatch. Missing
binding/error policy and provider/frame/ABI limits remain explicit; there is
no fabricated successful base slot or implicit context global.

## Validation

- Strict MSVC Win32 `/std:c++17 /EHsc /MD /W4 /WX /permissive- /fp:strict /O2`
  compilation passed; `local/goal15f_avoid_singleton_compile.cmd`.
- `local/goal15f_verify_singleton_object.py` passed: Source 287-byte body, five
  actual fixed provider calls in Native order, actual SDK imports, emitted
  state transitions, both current cleanup actions and extra termination entry.
  Optional argv1 selects the registered object. No full Native189B/Source287B
  byte-equivalence comparison is asserted for this context/EH translation.
- Five direct Native call rows and the separate imported/metadata edges are
  recorded in the JSON receipt for verification. Primary and peer Source/EH
  review found no blocker.
- Primary owns the single genuine-heap normal/fast/LIFO fixture using actual
  lifetime-manager creation, raw registration, SDK locks, explicit current
  Source dispatch binding and real cleanup. Actual result belongs in the final
  receipt; the old manual ownership-handoff draft is not the final contract.
- Primary owns registration, full build/CTest/registered-object/archive checks,
  annotation save/export and integration. No production wiring, Native
  execution/differential, original caller/EH/heap ABI, whole-process leak or
  game/gameplay proof is claimed by this worker.
