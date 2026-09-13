# Observer dispatch schedule

Addresses: `00695F90`, `00695BC0`, `00695D00`, `006959B0`, `00694CD0`.

This packet implements the complete normal schedule of `00695F90` over the
existing raw observer storage. It borrows `NativeObserverLifetime` and that
same lifetime's actual `00E198E4` publication cell. The callback has a new C++
function/context interface; native calls it with ECX equal to the original
first endpoint and EDX equal to the current edge's callback-owner pointer.
Event-specific virtual slot delivery belongs to the callback provider.

| Entry, inclusive end | Native ABI | Coverage / source |
| --- | --- | --- |
| `00695F90..00696112` | ECX first, EDX callback, `RET` at `00696112` | Complete normal schedule; `dispatch_observer_edges_00695f90` |
| `00695BC0..00695C48` | ECX vector, four stack arguments: output iterator, iterator owner, position, value-slot pointer; EAX output; `RET10h` at `00695C46` | Complete instance reused as `insert_one_native_singleton_slots_checked_00bd08d0` |
| `006959B0..00695B63` | ECX vector, four stack arguments: iterator owner, position, count, value-slot pointer; `RET10h` at `00695AC9`, `00695B2E`, `00695B61` | Complete instance reused as `insert_count_native_singleton_slots_00bd0700`; original EH/provider limits retained |
| `00695D00..00695DB2` | ECX vector, stack count and fill pointer; `RET8` at `00695D58` / `00695DB0` | Library boundary; private schedule size-restoration adapter only, no standalone reconstruction |
| `00694CD0..00694D29` | ECX vector, five stack arguments: output, first-owner, first-position, last-owner, last-position; EAX output; `RET14h` at `00694D27` | Library boundary; schedule's valid erase-to-end operation only; preserve existing `STL_inst_00694cd0` identity |

`00696330` and `00696340` select callbacks `00693550` / `00693560` in EDX and
tail-jump at `00696335` / `00696345`. Neither supplies a stack argument.
The other insertion caller `00695DD0` appends an incoming value-slot pointer;
`00696120` has its own snapshot/notification sequence. Resize is also called
by `006962C0` with a computed iterator distance. Their argument setups were
read; these sibling whole routines are outside this source packet.

## Storage and publication

Existing producer evidence remains authoritative. `NativeObserverOwnerStorage`
has its edge-array pointer/count/capacity at `+4/+8/+C`. Each edge stores first
at `+4`, callback owner at `+8`, and references at `+C`; the reference count is
not touched by this dispatcher. `00E198E4` points to the vector embedded at
dispatch-owner `+4`, as produced by the CRT initializer `00CCD6A0`. Within that
vector, `+0` is untouched and `+4/+8/+C` are begin/end/capacity-end. The lifetime
and dispatcher must borrow the same publication; no second vector is created.
The actual owner must remain alive until all dispatch/unregister activity ends.
Q's owner destructor leaves this alias dangling and does not delete its edges.

## Ordered schedule and mutation

1. Call `00694280`, capture the returned owner's section at `+4`, and use that
   section through cleanup. If nonnull, call real `EnterCriticalSection`, then
   increment its raw `+18` word. A null section skips both operations.
2. Read the current `00E198E4` vector and save its size. A null begin yields
   zero; otherwise the native size is `(end-begin) SAR 2`, retained as raw32.
3. Copy each first-endpoint edge pointer into the shared vector. Spare capacity
   writes the captured edge at captured end, then advances end. Growth calls
   checked insertion using the captured vector/end and an actual stack value
   slot. After each append, reload first's count and begin to derive its current
   end, reload `00E198E4`, and advance the **old** endpoint cursor by four.
   This does not repair an invalid cursor if a provider reallocates that array.
4. Capture the resulting vector size as the exclusive stop index. For each
   index from the saved size, reread the current vector's begin/size, validate,
   then reread the selected slot. Null slots are skipped. Otherwise load that
   edge's current `+8` and invoke the original callback with the original first.
   Reload `00E198E4` after each callback; do not expand the saved stop index.
5. Resize the current vector back to the saved size, filling null if growth is
   required. Then decrement the captured section's raw depth and call real
   `LeaveCriticalSection`. There is no observer retain/release or edge deletion.

The begin-field address used by indexed access is captured before validation.
A returning invalid-parameter handler causes a global-vector reload, but the
immediate slot load still uses that captured field address (`006960A5`,
`006960BB`, `006960C1`). Source raw volatile reads preserve this distinction.
No repaired bounds, substituted empty vector, or defensive callback skip is
added. Valid storage is required; arbitrary corrupt-pointer faults are unproven.

Nested calls use the same recursive section and append after the outer range.
They trim only their own range on ordinary return. Existing unregister/detach
operations null matching entries throughout the shared vector before deleting
an edge, so later callbacks can be suppressed without changing saved indices.
New registrations made during callbacks appear in a later dispatch, because
the endpoint snapshot for the current dispatch has already been copied.

## Vector reuse and library boundary

`local/observer_vector_equivalence.json` compares live Ghidra and installed PE
bytes for eight corresponding template instances. After clearing relative CALL
operands, all instructions and other bytes match; the length-error routine also
has a distinct original FH3 handler immediate. The recorded call-target pairs
establish the dependency mapping, not just equal routine sizes.

| Observer helper | Existing source instance | Operation |
| --- | --- | --- |
| `00695BC0` | `00BD08D0` | Checked single insertion |
| `006959B0` | `00BD0700` | Count insertion, including all capacity/in-place arms |
| `00693830` | `00BCFEB0` | Raw pointer-slot allocation |
| `00695230` | `00BD0500` | Copy with current CRT `memmove_s` |
| `00695730` | `00BD0560` | Count fill, rereading the value slot |
| `006946B0` | `00BD0160` | Assign range, rereading the value slot |
| `006946D0` | `00BD0180` | Copy backward with current CRT `memmove_s` |
| `00695940` | `00BD0590` | Existing native-layout source length-error owner |

No allocator, CRT, or vector-library body is added. The private dispatcher
adapter consumes only restoration to the saved size. It preserves the native
unsigned comparisons, signed-shift counts, captured end and validation order.
In the native-valid vector domain, shrink's last iterator equals the captured
current end, so there are no elements to copy. The adapter publishes the new
end without destroying or zeroing erased slots and retains capacity. Growth
uses the existing canonical count insertion with a real null fill slot.
There is no standalone native resize/erase entry and no reconstructed library
coverage claimed for `00695D00` or `00694CD0`. The discarded output iterator is
unobserved by this scheduler. Malformed vectors and returning invalid-parameter
handlers that mutate the vector are outside the adapter's guarantee.

The fixed SDK `_invalid_parameter_noinfo`, `memmove_s`, and existing malloc/free
providers own their current CRT handlers, errno, heap, and exceptions. Original
VS2005 static CRT internals, register side effects inside those services, and
native exception payload transport are not independently reproduced here.

## Cleanup and Ghidra metadata limits

The native FH3 descriptor at `00DAB804` has one unwind state whose action is
`00C7EB10`: `LEA ECX,[EBP-1Ch]; JMP00411EE0`. It destroys only the captured lock
guard. Source C++ exceptions likewise release that section without resizing the
vector. Original FH3/SEH, asynchronous faults, and mutable unwind-stack aliases
are not part of the new C++ ABI. The original handler thunk at `00C7EB18` was
initially missing from Ghidra. The root now defined and saved its exact ten-byte
body `00C7EB18..00C7EB21`; the last JMP starts at `00C7EB1D`. Current read-only
CLI checks confirm that body and the descriptor/unwind-map words. This metadata
repair does not extend source EH coverage.

The root repaired decoding of `00695AB1..00695AB3` (`83 C4 04`) after the free
call at `00695AAC`. The saved listing now has no decoded call gap, but Ghidra
still reports no containing function at `00695AB1`; `00695AB4` belongs to
`006959B0..00695B63`. Full verified PE coverage is not complete body membership.
Worker access was read-only and all exports used the project-verifying BSP CLI.

## Focused verification

The ignored fixture builds with `local/build_observer_schedule_probe.cmd` and
`/MANIFEST:EMBED`; prepare with `python local/prepare_observer_schedule_probe.py`
and run `local/observer_schedule_probe.exe` from the worktree root. It uses ten
original spans (1,485 bytes), relocated relative transfers/data operands, real
Win32 critical sections, and actual current CRT memory services. Both sides
share the existing source raw manager, dispatch owner, edge registration,
unregister, and destruction providers. Callback endpoints belong explicitly to
the fixture, not invented game event/vtable implementations.

The paired scenario passes with all 88 canonical words equal and five original
vector-allocation calls. It covers a pre-existing prefix, allocation growth, nested
dispatch, deletion and nonfinal unregister of pending edges, registration during
callbacks, capacity reuse, and an empty endpoint. Its last phase shortens the
valid vector in the final callback, exercising null-filled growth back to the
saved size. The report records its outcome
and canonical callback/owner/size/capacity/depth words. A separate source-only
C++ exception observation checks the declared cleanup boundary; no original
FH3 exception is executed. The source exception leaves size four and releases
the actual section to depth zero. Allocation/validation failure arms are unexecuted.
Inactive vector storage, allocator metadata, OS-internal critical-section bytes,
absolute pointer values, freed storage and library internals are not compared.
Fixture teardown deletes every source-created edge, frees endpoint arrays and
the dispatch owner, removes manager registrations, releases both real critical
sections, and frees manager/native image storage. This is not mixed-profile
shutdown, runtime host binding, or gameplay proof. No tracked test was added.

`./scripts/build.ps1` passes the Win32 Release build and both existing CTests.
The report embeds the original-byte spans, direct call-site rows, normalized
template evidence, canonical records, and an artifact manifest including the
exact linked SDK/CRT libraries, core objects, source/probe hashes and logs.
The first probe build's narrowing-conversion errors and a later fixture-only
teardown-counter failure are retained. Known `00CF7E64` edges call the real
deleting entry directly, so the fallback virtual-service counter must remain
zero; all endpoint counts are zero after detach. That correction changed only
the ignored fixture, and native/source records already matched on the failed run.
An earlier draft's standalone library copies were removed at root review; its
source, linked library/object, probe and reports remain under ignored
`local/observer_schedule_prior_library_draft/`. Those historical proof files
must not be confused with the final narrow adapter's evidence.
