# Native render-batch reference companion

`NativeRenderBatchReference` now binds a canonical host companion to one acquired
actual 18h batch and its existing +04 atomic. Its terminal operation can propagate
an allocation failure from the real batch-pool return path. It is deliberately
separate from `RenderCommandReference`, whose terminal method is `noexcept`.
Command destruction must use the new `NativeRenderBatchReferences` domain for
its two batches; scene/context references keep their existing owner domain.
This refines the batch-binding recommendation in `NATIVE_RENDER_COMMAND_OWNER_NEXT.md`.

The packet adds only a host companion and zero-count dispatch helper. It does
not reconstruct another native body, replace existing `00B55680`, add another
array/pool/lock, or change Ghidra, ledgers, CMake or native layouts.

## API and ownership

`NativeRenderBatchReference(storage, batches, table, disposal)` borrows actual
`storage.references_04`. It neither initializes nor retains that count. The table
argument must be the **same current `D5E5AC` view** used by the supplied
`NativeRenderBatchLifetime`. Constructor and terminal dispatch both validate
the actual table identity, virtual0 `B55680`, and deleting +4 `B1C630`. The
existing recycle method validates only its deleting slot because it already
represents the selected native virtual0 body.

`release_native_render_batch_reference(references, storage)` decrements actual
+04 first. Only a transition to zero calls `resolve_actual_batch(raw)`. Lookup
is a pure canonical association and must not retain, release, allocate or run
lifetime behavior. The helper verifies both storage identity and the borrowed
atomic before calling the companion's throwing `release_zero_00b55680`.
Missing bindings and unsupported current profiles are errors, with no fallback.

The companion, batch domain and lookup association survive until terminal release.
Caller-owned companion capacity must be prepared before command initialization;
binding an acquired batch must be nonthrowing association work in the valid
domain. Reacquiring the same physical pool slot creates a new companion lifetime.
The companion destructor does not perform native cleanup; destroying a still
bound companion violates its explicit lifetime contract.

The saved, nonthrowing retirement callback receives one of two outcomes:

| Outcome | Native state on callback entry |
| --- | --- |
| `returned_to_pool` | Batch destruction and append to the actual current free list completed. |
| `unreturned_dead_slot` | Batch destruction completed, but lazy pool/lock lookup or free-list allocation threw before this slot was appended. |

Retirement removes only the host association and may dispose the companion.
It must not allocate, throw, free/append the native slot, modify native fields,
or repair the pool. The implementation performs no companion or native access
after that callback. Caller synchronization must prevent slot reuse from
observing a stale association. This packet adds no synchronization mechanism.

## Why the terminal must throw

The current native order is exact:

1. `B55680` loads current table+4 and calls it with flags0 at `B5568A`.
2. `B1C630 → B51D50` destroys borrowed entry storage and finishes in base table
   `CEB130`; it does not scalar-free the batch for flags0.
3. `B5568D` obtains the current pool through `B1E870`.
4. `B55697` calls `B555E0`, which obtains the current lock owner, enters its
   captured section, and may reserve free-list storage through `B1C830` at
   `B5563E`.
5. Only after reserve succeeds does `B55653` publish the dead-slot cell and
   `B55655` increment its count.

The append's state0 cleanup is `CC0310 → 411EE0`: unwind the captured lock
guard. Its handler is `CC0318`, FuncInfo `DF8E50`, unwind map `DF8E48`.
It does not return/free the batch or undo another singleton's publication.
`B55680` has no own EH frame.

In the existing valid nonnegative header domain, batch entry destruction and
ordinary free cannot throw. A later lazy-pool/lock or append allocation failure
therefore leaves actual +04 at zero, table `CEB130`, entry count zero, and the
native dangling entry pointer/capacity unchanged. The physical batch is still
allocated but absent from the pool's free list. The companion catches that
exception, observes the completed `CEB130` phase, marks itself retired, reports
`unreturned_dead_slot`, then rethrows the original exception. It neither retries
nor supplies additional native cleanup.

If a dependency rejects its profile before reaching the completed base phase,
the companion preserves its association and propagates the exception without
falsely reporting a dead slot. Invalid/corrupt headers, concurrent profile
mutation and exceptions after successful append are outside the supported
native domain. On success the implementation does not inspect native storage
after pool return.

For `B1DDD0`, allowing the pool exception to propagate is essential: its EH
cleans ordered array, indexed array and diagnostic only. Later batches/groups,
scene and context remain untouched. A `noexcept` batch terminal would terminate
before those command cleanup actions could run.

## Evidence and verification

The guarded repository CLI verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 language and image base before live reads.
Ten bounded spans, **860 bytes**, match installed executable SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report records each span/hash and the unchanged dependency sources.

| Original dependency span, exclusive end | Bytes | Original ABI |
| --- | ---: | --- |
| `B55680..B5569E` | 30 | ECX=batch at count0; RET |
| `B555E0..B5567B` | 155 | ECX=actual pool+4; stack dead slot; RET4 |
| `B1C830..B1C88F` | 95 | ECX=free-list header; signed capacity; RET4 |
| `B1C630..B1C64E` | 30 | ECX=batch; flags; EAX=original address; RET4 |
| `B51D50..B51DC1` | 113 | ECX=batch; RET |
| `B1E870..B1E922` | 178 | No native inputs; EAX=current pool; RET |
| `B1CD90..B1CE4D` | 189 | No native inputs; EAX=current lock owner; RET |

The new source compiled with MSVC x86 `/std:c++17 /EHsc /fp:strict /O2 /MD /W4
/WX`. The required repository build passed, including both existing CTest
checks after all eight native seed spans were verified. CMake registration is
left to the primary integrator; the new source was compiled explicitly.

One focused local check compiles the new component together with the real
`NativeRenderBatchLifetime`, `singleton_lifetime_allocate/free`, tracked locks
and canonical singleton domain. Debug CRT's allocation hook rejects exactly
one `malloc(4)` request during growth of an empty native free list; the existing
allocator's new-handler path then throws `std::bad_alloc`. No fake pool return
or replacement terminal callback is used.

The check passed: nonzero release performs no lookup; zero release resolves
once; entry storage is freed once; the original exception propagates; the pool
header remains zero; the captured lock depth returns to zero; actual dead batch
bytes remain; and retirement deletes the canonical companion exactly once with
`unreturned_dead_slot`. After evidence capture the fixture explicitly frees its
orphan; that cleanup is outside both the reconstructed call and retirement.
The same focused sequence rejects a changed virtual0 before destruction, then
restores the caller's profile and verifies successful real pool return and
companion self-deletion. Domain shutdown cleans the actual registered owners.

The allocation-failure check executes reconstructed code with an actual CRT
allocation rejection. It does not execute original native exception dispatch,
the whole command owner, game logic or rendering. No additional original-function
or binary-compatible ABI claim is made.
