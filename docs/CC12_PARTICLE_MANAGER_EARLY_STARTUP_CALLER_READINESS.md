# Particle-manager early startup caller readiness

The Native caller contract is resolved. Recommend a future two-file Source packet
in `src/game_hosts.cpp` and `include/bsp/game_hosts.hpp`, conditional on Root
accepting the separate typed-publication/context/deletion contract and the concrete
failure-containment policy below. This review authorizes no C++ activation.

At baseline `3b0ae07e45af3283be8bd7204592fa67d3b89a85`, insert immediately before
the locale-construction comment and `locale_ = new GameLocaleHost(log_)` in
`GameStartupHost::run_initialize_phases` (line 2190), after the existing input
callback/reset marker. Native `0073E005..0073E02F` precedes locale constructor
`0073E059` and fonts/GUI `0073E13C`. The late Phase 8 marker is not the insertion site.
This does not establish parity of other omitted renderer/input/online stages.

| Native event | Established contract |
| --- | --- |
| `73E005/73E00E` | Push 34h, call BF681B with prior EH state -1. An allocation exception produces no caller-owned receiver. |
| `73E016` | Save returned EAX at steady ESP+10, independently of F8C274 and the eventual constructor return. |
| `73E01C/73E027` | Arm state 23h, then skip the constructor if the allocation is null. |
| `73E029/73E02B` | Move captured allocation to ECX and call AF0B10. Its EAX result is ignored. |
| `73E030/73E035` | Begin the following 4040h locale allocation and restore state -1 before its allocation call. |
| State 0x23 unwind | Free the captured allocation; do not read/repair publication or call another destructor. |

The enclosing entry pushes handler `C86CBA`. Its finite stub points to FuncInfo
`DB609C`: magic 0x19930522, maxState field 31h, unwind map DB60C0. Entry 0x23 at DB61D8
contains `toState=-1/action=C86C08`. The complete 14-byte cleanup is:

```asm
mov eax, [ebp-138h]
push eax
call BF65AC ; _free
pop ecx
ret
```

The prologue accounts for 148h from the entry stack anchor to steady ESP:
ESP+10 corresponds to frame-138h; the EH state at ESP+144 corresponds to frame-4.
Ghidra's existing funclet metadata ends at the call. Its separately leased
two-byte tail supplies POP/RET; no function, prototype, listing or flow was repaired.
This resolves the finite caller cleanup, not the complete private FH3 dispatcher.

Use `singleton_lifetime_allocate({SingletonAllocationKind::object, 0x34,
sizeof(NativeParticleModelManagerStorage)})`. The current Win32 storage is 34h.
The Source allocator uses `std::malloc`, `_callnewh` retry and `std::bad_alloc`;
its matching free uses `std::free`. Preserve the Native null branch even though
this allocator normally returns a pointer or throws. Leave the allocation raw:
no value initialization, memset, `new/delete`, caller smart owner or caller
publication assignment. Ignore the constructor result. On success, admit normal
registered-owner handover; on constructor failure, free only the captured pointer.

The separate owner design must provide one typed permanent F8C274 cell, the same
actual 01090AA0 cell, a live const volatile DWORD reference to verified D7A24C, and
an inline borrowed access context that survives every permitted drain. Proposed
APIs are `bind_particle_manager_domain(GameNativeReadOnlyData&)` and
`particle_manager_context()`. The latter only borrows an already-bound context.
Preparation must precede registration and reject unknown preexisting owners.
D5D7EC/AF0870 and D5D7F8/AF1080 deletion must use the popped receiver and full
flags. Those six-file owner changes and API names remain a separate proposal;
they do not implement this caller's containment policy.

The concrete Source containment proposal follows current allocation-stats policy:

1. Add `ParticleManagerPhase` and a field initialized `unattempted` to GameStartupHost;
   states are `unattempted`, `allocating`, `allocation_failed`, `constructing`,
   `failed`, `skipped` and `handed_over`. Reject a repeated caller attempt before
   allocation. Borrow the validated owner context before beginning the allocation.
2. Set `allocating` before allocation. An allocation exception sets
   `allocation_failed` and rethrows with no free and no constructor effects.
3. For a nonnull captured allocation, set `constructing` immediately before AF0B10.
   A constructor exception sets `failed` **before** freeing the captured allocation,
   then rethrows. Do not clear F8C274, unregister a substitute, run another
   destructor or reset the access context.
4. A normal constructor return becomes `handed_over`; null becomes `skipped`.
   Retire that Source phase before logging or starting locale allocation.
5. Add `exit_if_particle_manager_failed() noexcept` at the four existing guard
   clusters: destructor, application_shutdown, destroy_singleton_lifetime_manager
   and exit_process. For `constructing/failed`, log best-effort and call
   `std::_Exit(1)` before graph/context/data destruction or CRT exit callbacks.
   Allocation failure/no-constructor and successful handover keep ordinary cleanup.

These are proposed ordinary-C++ guards, not Native EH state values, Native exit
semantics, FH3/SEH or hardware-fault equivalence. They prevent ordinary Source
drain after a captured free may leave publication/registration effects dangling.
Generic singleton shutdown outside that guarded host still requires a valid
owner graph and context lifetime. Root approval of this policy is mandatory for
the proposed activation; it is not an inferred registry repair.

Portable replay passed: 174 fresh saved-program bytes across eight bounded spans,
391 whole relied Source/header files and 708 lexical quoted-include edges. Every
Source file reused exact immutable evidence; unrelated Source corpus was omitted.
The fresh 43-byte allocation interval matches prior accepted Native bytes. Other
new EH bytes are explicitly saved-program evidence, without a fresh original
executable read. Each Ghidra query used the project's verifying bsp.py client.
The separate Git check passed 399 complete blobs/current files. The include closure
is conservative lexical evidence, not actual compiler-input coverage.

Copy `portable_replay.py` and `evidence.zip` from
`local/cc12_particle_manager_early_startup_caller_readiness/` and run
`python portable_replay.py evidence.zip` with Capstone available. It reads only
the ZIP; no Git/Ghidra, compiler, executable or file-write side effects occur.
No C++ change, CMake edit, build, test, fixture, game run or SDK/audio/OS probe
occurred. The accepted FMOD stop remains the runtime boundary.

[Complete report](../reports/cc12_particle_manager_early_startup_caller_readiness.json).
