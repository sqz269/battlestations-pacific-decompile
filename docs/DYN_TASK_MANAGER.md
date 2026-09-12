# Dyn task manager construction and execution

`dyn_task_manager.hpp` reconstructs the complete normal paths of six native
functions. It creates actual Win32 synchronization objects and CRT worker
threads, submits borrowed task objects, waits for batch completion, and executes
the recovered shutdown/destruction sequence. The C++ interface is new, is Win32
only, and is not a replacement for the original register or SEH ABI. Names below
are descriptive hypotheses. Machine-readable call sites, bounds, hashes and
verification limits are in `reports/dyn_task_manager.json`.

| Entry and inclusive end | Native inputs and cleanup | Recovered operation |
| --- | --- | --- |
| `00C37740..00C377ED` | stack manager, worker count; `RET8` | construct 358h manager |
| `00C37690..00C3773A` | ESI manager, stack count; `RET4` | stop/join and replace workers |
| `00C37680..00C3768F` | stack manager; `RET4`, EAX=0 | CRT worker entry |
| `00C375F0..00C37676` | ESI manager; plain `RET` | wait, pop, execute, complete |
| `00C33140..00C3324B` | ESI manager, EAX task vector, stack count; `RET4` | submit and wait for batch |
| `00C40FF0..00C41069` | stack manager; `RET4` | stop and destroy manager resources |

The engine producer `00C55EA0` allocates 358h at `00C55EFD`, then loads the
descriptor's first dword at `00C55F16` only on successful allocation. It pushes
that count and the allocation before calling `00C37740` at `00C55F1A`, and stores
the result at engine+10h. This packet takes that actual count as an explicit
input; it does not derive one from processors or synthesize an engine. The
engine owner is reconstructed separately. All three scheduler call sites
(`00C577EE`, `00C5BCCA`, `00C5C0E8`) explicitly load engine+10h into **ESI**;
ECX is not the scheduler's required manager input. Their vectors are respectively
scene+CCh, world+480h, and world+468h. The engine destructor supplies its manager
as a stack argument at `00C421F2`.

| Manager offset | Producer and meaning |
| --- | --- |
| 00h | allocated array of worker thread handles; only written when workers are created or an existing nonzero group is stopped |
| 04h | worker count; constructor writes zero before adjustment |
| 08h | last `_beginthreadex` output thread ID; untouched if no worker is created |
| 0Ch / 10h / 14h | task-pointer queue allocation, current size, capacity |
| 18h..2Fh | actual 18h Win32 critical section, spin count 10000 |
| 30h..1BFh | 100 signed 32-bit completion counters |
| 1C0h..34Fh | 100 initially nonsignaled auto-reset event handles |
| 350h | initially empty semaphore, maximum count 1000 |
| 354h | initially nonsignaled manual-reset shutdown event |

The constructor preserves 00h and 08h when the requested count is zero. In
particular, fresh raw allocator storage is not implicitly zero-filled. A caller
that later invokes the destructor on a never-started manager must have supplied
a null or releasable 00h pointer; this is a caller precondition, not a recovered
constructor store. Allocation callbacks must support `release(nullptr)`.

`00C37690` signals shutdown, waits for every existing thread, resets shutdown,
frees the handle array and nulls 00h, then publishes the new count. It allocates
four bytes per worker (overflow saturates to FFFFFFFFh), calls the real
`_beginthreadex` with manager and `00C37680`, stores each returned handle and sets
priority to 2. The six CRT arguments are proven by the 18h caller cleanup at
`00C37722`. Existing queued tasks, semaphore state and completion groups are not
reset by replacement; callers must replace workers with no active submissions
or pending work when they require a clean transition.

The worker waits on `[shutdown, semaphore]` in that order. Result zero exits;
every other result follows the native dequeue path. It removes the last queued
pointer under the critical section, then invokes the borrowed object's real
vtable slot zero with ECX=task and no stack arguments. The task's writable dword
at +4 selects a completion counter. `LOCK XADD -1` followed by `DEC` detects the
transition from one to zero and signals the corresponding event. The source
uses `InterlockedDecrement`, including the native reload of task+4 before
fetching the event. Task methods, their concrete vtables and their physics
payload remain external; no dummy callback or task object is installed here.

The batch submitter holds the critical section while extending the pointer
queue, choosing the first zero counter, writing each task's +4 group and
appending pointers. Capacity grows exactly to the required size and old pointers
are copied before the old allocation is released. It releases the semaphore
before publishing the group count, still under the same critical section, then
unlocks and waits on the group's auto-reset completion event. Consequently the
single-worker execution order is LIFO. There is no zero-count fast return and
the counter scan has no bound check. Callers supply a positive signed count,
running workers, a free group below 100, a representable queue allocation, and
sufficient semaphore capacity. Simultaneous reuse of a zero group before its
previous waiter consumes the event is not repaired by this reconstruction.

The destructor calls worker-count adjustment with zero, closes only shutdown
and semaphore handles, frees the remaining handle-array pointer, deletes the
critical section, and frees a non-null queue. Neither worker replacement nor
destruction closes worker thread handles. The 100 completion-event handles are
also left open by these native paths. These are observed ownership gaps: the
source preserves them. A surrounding resource owner may retain and close these
handles separately, as the fixture does. The fixture's extra closures are not
attributed to native destruction.

Root repaired false non-returning `free` flow at `00C376C9` and `00C331A4`, and
recreated the truncated destructor body through `00C41069` under the Ghidra
write lock. The worker re-read the final live bodies. There are no remaining
call-continuation gaps in these six functions; the scheduler has an unrelated
alignment gap. The later destructor body repair and final flow report supersede
the earlier incomplete-flow diagnostic. No worker Ghidra mutations were made.

Validation: Win32 Release builds and both existing math tests pass. One ignored
real-Win32 source fixture checks zero-count untouched bytes, 102 synchronization
handles and their reset modes, priority/ID, task virtual dispatch, LIFO order,
queue growth/free/reuse, one-to-two worker replacement, overlapping groups zero
and one, and complete fixture cleanup. Its four allocations are released; it
also checks both native null frees and separately closes the three retained
worker handles and 200 retained completion events from its two manager
lifetimes. It is source execution, not original-byte differential execution,
whole-storage/CS byte comparison, stress testing or gameplay validation. The
separate engine packet covers its own original-byte zero-count constructor
comparison; that evidence is not claimed by this fixture.

Reproduce from this worktree with `cmd /c local\build_dyn_task_probe.cmd`, then
`local\dyn_task_probe.exe`. The build command embeds a manifest and compiles
the actual source directly with MSVC Win32. Exact source, command, executable,
native-span and log hashes are recorded in the report. Failure/SEH rollback,
failed Win32 calls, invalid task data, exceptions from task methods, more than
64 workers in the Win32 wait-all shutdown, active destruction, and allocation
overflow/OOM are outside the verified domain. There are no x87 expressions in
these owned bodies.
