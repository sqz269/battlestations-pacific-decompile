# Native render-batch storage and lifetime

This packet reconstructs the native 18h batch, its dead-slot free list, and the
two registered singleton owners used by render-command initialization. The
implementation uses the actual `+04` atomic reference field, actual pointer
arrays, one caller-provided `SingletonLifetimeDomain`, and references to the
live `0108FE8C` pool and `0109DBBC` lock publications. It adds no command,
context, queue, job service, or alternate reference-counted batch owner.

The source and header are `src/native_render_batch_lifetime.cpp` and
`include/bsp/native_render_batch_lifetime.hpp`. These expose new C++ entrypoints;
they are not original-ABI thunks or binary replacements. The primary integrator
owns CMake registration, Ghidra annotations and shared reconstruction ledgers.

## Native storage and contracts

| Storage | Actual fields | Size |
|---|---|---:|
| Batch | vtable00, atomic signed count04, preserved mode08, borrowed-entry pointer0C/count10/capacity14 | 18h |
| Pool | vtable00, dead-slot pointer04/count08/capacity0C | 10h |
| Lock owner | vtable00, actual `TrackedCriticalSection*`04 | 8h |
| Tracked section | real Win32 `CRITICAL_SECTION`00, actual recursion counter18 | 1Ch |

All storage has no field defaults. Batch placement construction writes the
reference-base vtable, count1, batch-base vtable, then clears only the entry
pointer/count/capacity. It does not read, clear or snapshot mode08. Reusing a
dead slot runs the same constructor over the same bytes and installs D5E5AC.
The four-byte `std::atomic<int32_t>` at04 is available for the later borrowed
render-command reference binding. `recycle_zero_reference_00b55680` assumes
that native count has already reached zero; it does not decrement again.

The companion requires the actual immutable D5E5AC table with at least four
words. Recycling reads the batch's current table identity and that table's
current slot1, requiring the recovered B1C630 deleting destructor. It calls
flags0, then looks up the current pool and appends the now-dead raw address.
Unsupported/unbound profiles report an explicit binding error. There is no
fixed no-op virtual callback and no implicit lifetime action in the companion's
C++ destructor.

Valid array storage has nonnegative signed counts/capacities, count<=capacity,
and allocated readable/writable pointer spans. Requests must fit signed32 and
the native four-byte allocation product; free-list doubling must also fit.
Native minimum clamps remain 256 for entry arrays and 1 for dead-slot arrays.
Overflow, corrupt counts, invalid/null owner storage, arbitrary concurrent
mutation and access violations are outside this typed storage contract. No
additional count clamps, validation callbacks or corruption-repair behavior
are claimed. Publication and lock ownership follow the native lifecycle;
shutdown requires the owning/quiescent thread.

## Entry points and original ABI

Every address below is a complete inspected native body. Names are descriptive
hypotheses; the existing B51B50 name remains `BSP_RenderBatch_ReserveEntryPointers`.

| Address..exclusive end | Native inputs / return | Reconstructed action |
|---|---|---|
| 00B51D20..00B51D41 | ECX raw batch; EAX same; RET | Placement construction, preserve mode08 |
| 00B51B50..00B51BB2 | ECX entry descriptor; stack signed capacity; RET4 | Grow-only reserve, minimum256 |
| 00B51C60..00B51CB0 | ECX entry descriptor; stack signed count; RET4 | Reserve if needed, zero new cells, shrink count |
| 00B51D00..00B51D17 | ECX entry descriptor; RET | Count0, free pointer array |
| 00B51D50..00B51DC1 | ECX batch; RET | Batch-base phase, clear twice, free array, reference-base phase |
| 00B1C630..00B1C64E | ECX batch; stack flags; EAX original; RET4 | Destruct, scalar-free iff bit0 |
| 00B55680..00B5569E | ECX batch; RET | Current virtual+4(flags0), current pool, append dead address |
| 00B1D5B0..00B1D679 | ECX actual pool+4; EAX batch; RET | Current lock, pop LIFO or allocate18h, reconstruct |
| 00B555E0..00B5567B | ECX pool+4; stack raw slot; RET4 | Current lock, grow max1/double, append |
| 00B1C830..00B1C88F | ECX free-list descriptor; stack signed capacity; RET4 | Grow-only reserve, minimum1 |
| 00B1CE50..00B1CEA0 | ECX free-list descriptor; stack signed count; RET4 | Reserve, zero new cells, shrink count |
| 00B1D8A0..00B1D8DD | ECX free-list descriptor; RET | Scalar-free dead slots ascending, clear, array-free |
| 00B1E870..00B1E922 | no native inputs; EAX pool; RET | Double-check shared publication, allocate/register10h owner |
| 00B1E930..00B1E961 | ECX pool; stack flags; EAX original; RET4 | Clear publication first, base phase, destroy slots, optional free |
| 00B1CD90..00B1CE4D | no native inputs; EAX lock owner; RET | Double-check shared publication, allocate/construct/register8h owner |
| 00B1C9D0..00B1CA15 | ECX raw lock owner; EAX same; RET | D5E5D4 phase, create real tracked section, publish+4 |
| 00B1C3A0..00B1C3B1 | ECX lock owner; RET | Constructor unwind clears0109DBBC and writes CE3818 |
| 00B1D530..00B1D567 | ECX lock owner; stack flags; EAX original; RET4 | Destroy owned tracked section, clear publication, base, optional free |

The existing B51B50 host implementation operates a vector-backed queue. This
packet adds its native raw-storage form so B51C60 can use actual batch fields;
it does not change or call that vector storage. Both raw reserve routines
allocate first, copy borrowed pointers, free the old array, then publish the
replacement and capacity. Batch entry destruction never releases borrowed
entry objects. It intentionally retains the dangling array pointer and capacity
after free until the next placement construction clears them. Free-list pop
retains its stale last cell. Pool destruction frees dead allocations directly,
without calling batch destructors again. Returned deleting-destructor addresses
may already designate freed storage.

Both singleton getters use the same actual manager. They capture its optional
section10, enter it, increment the actual recursion18, recheck publication,
allocate/initialize/publish, look up the manager again, and register the freshly
reloaded publication. They decrement and leave the captured section before
their final publication reload. Registration failure does not roll publication
back. Pool acquisition and append similarly capture the current lock owner's
actual section04; replacing a publication does not redirect an active guard.
The existing critical-section helper drains an owned positive recursion depth
before OS deletion and clears the owner field after freeing it.

## Assembly and unwind evidence

Assembly restores the hidden post-free continuations in B1C830 and B1D8A0;
incorrect no-return analysis must not discard the replacement publication,
remaining slot loop or final array free. None of the owned bodies uses x87/SSE
floating-point arithmetic. All size, count, pointer and flag operations are
integer operations. The native base BD30F0 body is a seven-byte vtable store
and RET (`00BD30F0..00BD30F7`), not an invented destructor stub.

The six relevant original EH maps are retained in the audit from the preceding
byte-verified command discovery:

| Body | Native cleanup states |
|---|---|
| B1D5B0 | Fresh construction state2 frees allocation; reused state1 calls actual 401130 placement delete (one-byte RET); both reach captured lock-guard state0 |
| B1E870 | State0 releases captured manager guard; no published-owner rollback |
| B1CD90 | State1 frees the in-progress8h allocation, then state0 guard |
| B1C9D0 | State0 B1C3A0 clears publication and restores simple-owner base |
| B51D50 | State1 destroys actual+0C entry array, then state0 restores reference base |
| B555E0 | State0 releases captured batch guard |

The host placement batch constructor contains only nonthrowing stores in valid
storage. Its fresh/reused native construction-failure branches are therefore
unreachable through that typed function; allocation failure still unwinds the
captured lock. The lock-owner constructor and allocation wrapper preserve the
separate native base-cleanup/free responsibilities. Batch destructor cleanup
retains the array-then-base ordering. Original C++ exception dispatch itself
was inspected statically, not executed by the differential fixture.

## Verification and boundary

Every live read used the guarded `tools/bsp.py ghidra bytes` route against
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. The 19 complete
code spans, ten data spans and eight five-byte external entry spans total 1671
bytes and match the installed binary/zero-filled globals. The native binary
SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The original fixed image has no relocation directory; 58 absolute operands
and table words were individually identified and rebased for the fixture.

One focused installed-code/host lifecycle passed 14 pool phases plus three
placement/entry-array phases, matching 288 normalized observation words. It
checks mode preimage, same count reset, borrowed-pointer growth/shrink,
destructor-retained fields, minimum capacities, doubled growth, stale pop
cells, LIFO address reuse, lock depth and actual shared-manager registration.
It also holds one real batch-lock recursion at shutdown and verifies flags1
lock-then-pool destruction, cleared publications and returned freed identities.

The fixture maps only the listed spans at their original relative addresses;
unrelated pages remain NOACCESS and unused committed bytes are INT3. Eight
explicit dependency entry bridges supply shared allocation/free, actual
manager lookup/registration and the existing tracked-section create/destroy
helpers. The native manager ABI projection borrows the same real manager's
section; it does not execute the original manager body. Native import cells
call real Win32 Enter/LeaveCriticalSection. No full PE loading, entrypoint,
arbitrary import resolution, fake entry release or immediate job service is
used. Pointer/vtable identities are normalized in comparisons; this is not
a claim that absolute allocation addresses match. B1C3A0 and native exception
dispatch are not reached by the nonthrowing lifecycle run.

The exact new source compiled in that local fixture with MSVC x86 C++17,
`/EHsc /fp:strict /O2 /Oy- /MD /W4 /WX`. Eight native math seeds matched;
the repository build passed and CTest passed both existing tests. The worker
did not edit CMake, so the fixture explicitly compiled the new source while
the repository build checked the existing library. No new tracked test suite,
Ghidra mutation or shared-ledger edit is part of this packet.

Artifacts are recorded in `reports/native_render_batch_lifetime_audit.json`.
Local evidence/check sources and logs live under `local/native_render_batch*`
and the local build helper; the executable is
`build/batch-check/native_render_batch_check.exe`. Native command/context
construction, actual command/reference binding, queue/job execution, exception
injection, allocation-failure execution, multithread stress and game rendering
remain unvalidated or outside this packet.
