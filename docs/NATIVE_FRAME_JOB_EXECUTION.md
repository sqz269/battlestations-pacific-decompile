# Native frame-job execution and worker protocol

Packet `orch3_native_frame_job_execution_w` implements ten complete functions in
`src/native_frame_job_execution.cpp`. The immediate consumer is point-effect
virtual zero at 8683E0: it queries frame-pool virtual +10, and defers destruction
when AL is nonzero. That target is 4BFAB0, a **signed dispatch-active > 0** test.
It does not determine whether the caller is a worker thread.

Names are hypotheses, with the existing reviewed enqueue name preserved. These
are new C++ interfaces over actual Win32 storage, not callable native vtables.

| Span (exclusive end) | Original ABI | Behavior |
| --- | --- | --- |
| `004BFAA0..004BFAA3` | RET 4; no consumed input | Empty optional virtual callback |
| `004BFAB0..004BFABD` | ECX secondary pool; AL result; RET | Signed active+18 > 0 |
| `00BE2BA0..00BE2BB0` | Win32 stdcall, stack pool; EAX 0; RET 4 | Call current virtual +0C worker |
| `00BE2EA0..00BE2F9D` | ECX secondary pool; RET | Worker registration, wait/drain/stop loop |
| `00BE2FA0..00BE301C` | ECX secondary pool; RET | Calling-thread drain and completion waits |
| `00BE3020..00BE3040` | ECX pool, stack job/argument; RET 8 | Unretained raw job enqueue |
| `00BE3110..00BE3147` | ECX pool; RET | Reset completion, signal start per worker |
| `00BE3150..00BE31D1` | ECX pool, stack low byte; RET 4 | Dispatch workers/caller under active marker |
| `00BF2CE0..00BF2CEC` | No inputs; atomic result remains EAX; RET | Increment shared 0109DBE4 |
| `00BF2CF0..00BF2CFC` | No inputs; atomic result remains EAX; RET | Decrement shared 0109DBE4 |

## Actual storage and bindings

The secondary pool occupies 138A4h bytes at the frame singleton's primary +04.
Its layout is native identity +00; thread handles +04; thread IDs +08; start-event
array +0C; completion-event array +10; signed worker count +14; dispatch-active
DWORD +18; stop DWORD +1C; remaining-job DWORD +20; and 10000 eight-byte job slots
from +24. Each slot is an actual owner pointer and raw argument DWORD. The source
reuses the existing `kFrameJobCapacity`. No replacement queue, count or pool exists.
Atomic fields retain four-byte storage: native plain accesses use relaxed
load/store, and only original Interlocked sites use atomic read-modify-write.

`NativeFrameJobExecution` borrows the existing application `RandomThreads`, its
actual event and pool tables, and a required current-virtual-zero job executor.
It does not construct another random registry or own pool/event/thread storage.
Job execution must use the exact raw owner's established binding, without a
substitute job or successful fallback. Jobs are not retained or released here.

Actual table D6821C binds event scalar BD19B0, signal BD1910, wait BD17C0 and reset
BD1960. Current event profile/entry validation precedes calls to those existing
concrete Win32 implementations. API return values are ignored as in the pool.
Tables D68650 (base) and CE7554 (frame secondary) both bind enqueue BE3020,
dispatch BE3150, worker BE2EA0, predicate 4BFAB0 and optional callback 4BFAA0.
Their different scalar entries remain separate owner-lifetime work. Thread entry
validates the current profile/+0C target before calling the reconstructed worker.
Native integer table words are never executed as host function pointers.

## Execution ordering

BE3020 performs the three observed plain count reads, uses the third as the slot
index, writes owner then argument, and increments the current count. It adds no
bounds check or producer lock. The caller must coordinate enqueue and storage
lifetime; valid native spans remain preconditions.

Both caller and worker atomically decrement remaining count to claim a slot.
A nonnegative result indexes +24 + index*8. The exact job's current virtual zero
receives the captured argument. Only after it returns are the captured slot's
owner and argument cleared. This repeats until a negative claim, followed by one
compensating increment. A single drainer therefore sees LIFO order; concurrent
workers can complete in another order. No deterministic parallel ordering is added.
The calling-thread drain then waits on each current completion event, reloading
the event array and signed worker count.

BE3110 resets each current completion event and then signals the corresponding
current start event. BE3150 first increments the shared actual 0109DBE4 counter
and writes active=1. A nonzero input byte wakes workers and calls ResumeThread on
their current handles. A zero byte instead signals completion events, allowing
the caller-only drain to pass its waits. It then drains/waits, writes active=0,
and decrements the shared counter. It has no local unwind: an execution exception
leaves active and shared counter set, and the failing slot uncleared.

BE2EA0 registers the current thread in the existing random registry before arming
its unwind state. It obtains the real Win32 thread ID and searches the current
ID array/count; the caller must be an actual listed worker. It repeatedly waits
on its current start event, resets that event, then checks stop. Without stop it
claims/executes/clears jobs, compensates its negative claim and signals its current
completion event. Stop signals completion, disables unwind, unregisters and returns.
BE2BA0 calls this through current virtual +0C and returns zero.

Handler `00CC6B28..00CC6B32` selects E012FC, map E012F4. State 0 -> -1 invokes
CC6B20, which jumps through existing 8F7D00 to BD3050 random unregistration. It
does not signal completion, restore job count or clear a failed slot. The C++
worker guard preserves that limited cleanup. Original exception ABI is untested.

## Evidence and validation

Every analysis/export batch verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. All ten complete bodies, the handler and three native
tables matched the installed executable. Their hashes and saved annotation history
are in `reports/native_frame_job_execution.json`. The optional callback, predicate,
thread entry and handler were defined and saved through the serialized tools;
`reports/native_frame_job_definitions_w.json` records those definitions.

Strict Win32 build and both existing CTests passed. No permanent tests were added.
Ignored `local/frame_job_probe_w.cpp` executes all ten copied original bodies
against their reconstructed counterparts. It checks signed predicate values,
scope-counter wrap/results, serial LIFO execution, callback-overwritten slot
clearing, direct drain, wake/reset ordering and the zero-byte caller-only path.

For both original and reconstructed bodies the probe creates one **real suspended
Win32 worker** with actual pool address/ID storage and concrete manual-reset events.
It runs a caller-only phase while suspended, then two 32-job dispatch cycles with
worker participation required by a job-owned event. Every job executes exactly
once, all slots clear, counters/active return to zero, and stop joins the actual
worker with exit code zero. Random-state registration is observed inside worker
jobs and fallback restoration is checked on that same worker after unregistration.
This uses one worker plus the caller; it is not a general stress or performance test.

Original IATs are rebound to real Interlocked/GetCurrentThreadId/ResumeThread;
0109DBE4 operands use the fixture's actual counter. Worker random calls use the
existing concrete registry. Internal dispatch calls use copied originals. Only
executed entries in original fixture pool/event/job tables are rebound to callable
original bodies or canonical event bridges; the physical event HANDLE is shared.
The C++ entry wrapper supplies the explicit existing execution binding while
preserving the actual pool argument. Instrumented job bodies check arguments,
active/scope state, registration and slot behavior; they are not reconstructed
game render jobs. Pool construction/destruction is not claimed by this fixture.

Two C++ failure cases verify dispatch's absent rollback and worker-only random
unregistration, including retained failed slot and unsignaled completion. Native
handler immediates remain unchanged, and original exception dispatch was not run.
No production test seams or success stubs were added. The actual pool allocation,
suspended-thread creation, singleton lifetime and concrete game-job execution
bindings remain the next dependencies before point-effect virtual-zero composition.
No original ABI or gameplay validation is claimed.
