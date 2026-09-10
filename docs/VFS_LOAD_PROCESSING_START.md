# Loading-job preparation and worker start

Addresses: `00504790`, `00501620`, `00501510`, `00be0a30`.

State 2 prepares a FileBlock object, sets the job to state 3, then starts or
signals a persistent worker. The worker copies the current work item's name,
calls `007188a0`, and stores the returned value in that work item. The actual
processing routine and synchronization object remain necessary dependencies;
none of these four bodies is a complete parser or a completion guarantee.

This audit extends [VFS_LOADING_PUMP_LIFETIME.md](VFS_LOADING_PUMP_LIFETIME.md).
Four complete code spans, totaling 683 bytes, freshly match the installed PE.
The vtable bytes at `00d68494` also match. The empty fallback at `00e18d78` is a
live zero byte in the PE virtual zero-fill tail, so it has no original disk byte
to compare. The existing `bsp` project and `/battlestationspacific.exe` were
verified before every live export/read command. Exact spans, hashes, ABIs and
annotation proposals are in
[vfs_load_processing_start_audit.json](../reports/vfs_load_processing_start_audit.json).
Raw evidence is under `exports/bsp/parallel_vfs_processing_start/`.
No C++ or tests were added by this packet.

## Prepare the job's FileBlock: 00504790

ABI: ECX job, no stack arguments, RET. The complete span is
`00504790..0050484b` inclusive. The caller at `00509259..00509269` calls this
helper before setting state 3 and passing the same job to `00501620`.

If job `+20h` is already nonnull, the helper returns. If the name wrapper's
stored length at job `+14h` is zero, it also returns. Otherwise it allocates
28 bytes and, when the result is nonnull:

1. Calls existing substring helper `00469840` on the name wrapper at job
   `+14h`, with start 13 and requested length `storedLength - 18`.
2. Constructs `00be0a30(newObject, &temporaryName, 1)`.
3. Stores the returned object pointer into job `+20h`.
4. Destroys the temporary name after construction has copied it.

The allocation-null branch stores null in job `+20h`. Neither an empty name nor
a null allocation produces a success/failure return that the state-2 caller
tests; that caller still sets state 3 and invokes worker submission.
Do not turn FileBlock existence into an invented loading-success predicate.

The substring call is established directly by assembly at `005047df..005047ee`.
Its generic helper has prior cached evidence, not a new complete-span audit in
this packet. In particular, the caller does not validate a prefix, suffix, or
minimum 18-byte length. The natural positive-length case removes the first 13
and final 5 bytes; applying that description to arbitrary short names would
hide the native unsigned arithmetic and helper behavior.

## Construct and enter the FileBlock: 00be0a30

ABI: ECX object, name wrapper and flag as two stack arguments, EAX object,
RET 8. The complete span is `00be0a30..00be0ada` inclusive.

The constructor writes the final vtable `00d68494`, sets the dword at `+4` to 1,
zeros `+8/+0Ch/+10h`, and independently copies the name wrapper into object
`+14h/+18h`. It then makes two calls with different receivers:

| Call site | Receiver and arguments |
|---|---|
| `00be0aae -> 00bdf950` | ECX is the newly constructed FileBlock object. |
| `00be0abf -> 00be0980` | ECX is VFS global `0109ceec`; stack arguments are `&object[+14h]` and the supplied flag. |

Thus a reconstruction must not pass the FileBlock as the second call's
receiver. The second call operates on global VFS state. The independently audited
[FileBlock setup](FILE_BLOCK_SETUP.md) establishes identifier transformation and
block entry: preserve the prior manager `+79h` gate, combine it with the input
byte, optionally notify the observer, increment nesting depth, and store the
name. These helpers do not open a stream or parse a resource. Their full rules
and byte evidence belong to that separate packet.

The verified vtable has `+0 = 00bd30e0` and `+4 = 00bdebe0`. The latter is an
existing scalar-deleting-destructor candidate. The earlier retirement audit
shows job `+20h` virtual `+4(1)` before front-job destruction, then clears job
`+20h`. The destructor body and block-exit effects are not recovered here.

## Publish a job to the worker: 00501620

ABI: ECX loader, one job-pointer stack argument, RET 4. The complete span is
`00501620..0050165d` inclusive.

If loader `+1Ch` is zero, it calls:

```text
CreateThread(NULL, 0, 00501510, loader + 4, 0, NULL)
```

and stores the returned value at loader `+1Ch`. It does this before testing job
`+0Ch`. There is no subsequent handle-validity branch in this body.

Only when job `+0Ch` is nonzero does it store the supplied job at loader `+8`
and call loader `+0Ch` object's virtual `+4`. When job `+0Ch` is zero, it neither
publishes that job nor invokes that virtual, even if this call just created a
thread. It does not clear a previously published job pointer, report completion,
or update the loader state. This zero-field path must remain explicit.

The body acquires no loader/job references, copies no job data, and performs no
join, handle close, or timeout. A nonzero stored handle suppresses further
thread creation without any liveness query. Those are observed control-flow
facts, not a complete native error or thread-lifetime policy.

## Worker entry and processing boundary: 00501510

ABI: thread entry with one stack argument, callee cleanup RET 4, returns EAX 1
on normal exit. Its complete span is `00501510..00501615` inclusive. The argument
points into the original loader, rather than a newly owned thread-state object:

| Worker argument field | Loader field | Observed role |
|---|---|---|
| `+0` | `+4` | Stop word checked after the synchronization call. |
| `+4` | `+8` | Published current job pointer. |
| `+8` | `+0Ch` | Object providing virtual `+4/+8/+0Ch/+10h` operations across worker and caller. |

The worker first invokes that object's virtual `+8` at `0050153c`. After it
returns, a nonzero stop word exits with result 1. Otherwise each iteration:

1. Obtains `work = publishedJob[+8]`, then a character pointer at `work +4`.
   A null character pointer uses the observed empty fallback byte.
2. Computes C-string length by scanning to NUL, allocates a local string through
   `0041dd40`, and copies `length + 1` bytes when its local buffer is nonnull.
   It does not use a source stored-length field for this copy.
3. Reloads the published job and captures its work pointer in ESI. At the call
   to `007188a0` (`005015b2`), ECX points to the owned local string wrapper;
   EAX also holds `&publishedJob[+8]`, but the follow-up assembly audit proves it
   is a dead intermediate: `007175d0` overwrites incoming EAX before reading it.
   `007188a0` takes the ECX name only; see `VFS_LOAD_WORKER_DISPATCH.md`.
4. Stores returned EAX into the captured work record's `+0Ch` at `005015b7`.
   It then destroys the local name if still allocated.
5. Invokes synchronization virtual `+0Ch`, then virtual `+8`, then checks the
   stop word again. It loops only if that word remains zero.

Ghidra's pseudocode removes the local copy and cleanup blocks as unreachable.
The full assembly contains the allocation-dependent branches at `0050158d` and
`005015c8`, the `memcpy` at `00501599`, and cleanup at `005015d5/005015dc`.
The effects of the string-allocation helper must be preserved; the worker does
not intentionally process an uninitialized empty temporary.

The synchronization method bodies are not known here. Their observed order
supports a publication/processing handshake, but assigning exact event,
semaphore, lock, reset, or acquire/release semantics would be premature. In
particular, thread creation precedes publication of the first job: replacing
the initial virtual `+8` with a no-op could expose an unpublished job.

## Ownership and connection back to state 3

The worker stop word at **loader `+4`** differs from the stop byte at **front
job `+4`** set by `005092e0`. The audited loading stop routine waits for job
count zero and does not write this worker stop word, join the worker, or close
its handle. The normal worker can persist between jobs; its actual shutdown
writer and enclosing loader destructor remain separate dependencies.

The caller must preserve the loader containing the thread argument, its
synchronization object, the published job, and the work record through all
worker accesses. No retain/release establishes those lifetimes in the inspected
start or worker body. The worker reloads the job between name copying and
processing, so caller serialization/publication discipline also matters.
The result write uses the work pointer captured before the processing call;
possible work-pointer replacement by that callee needs its own contract.

The earlier state-3 code queries synchronization virtual `+10h`. On its normal
ready path it optionally calls job `+1Ch` with `(work[+0Ch], work[+8])`, invokes
`00501670(&job[+8], 0)`, and tests job `+0Ch` to requeue or retire. This suggests
a sequence of work records whose leading fields are a name wrapper, user value,
and processing result. Interpreting job `+0Ch` as the sequence count and
`00501670` as removing its first item is provisional until that helper is
recovered. This packet calls it a nonzero work field rather than inventing a
container implementation.

## Smallest real next implementation

The recovered boundary can support an owned-name processing adapter once
`007188a0`'s actual inputs, result type, and ownership are established. A real
adapter must execute that processing contract and preserve the result delivery
and work-record removal sequence; a callback returning a fabricated success
value would not move the reconstruction forward.

Full worker start additionally requires the synchronization object's concrete
virtual targets, creation/initial state and shutdown/join contract. Implementing
only `CreateThread` with placeholder synchronization would violate the observed
publication order. A synchronous host policy could be explicit, but would not
be a reconstruction of these native worker functions.

The next narrowly bounded dependencies are `007188a0` and `00501670` for actual
processing and work-record advancement, the concrete synchronization vtable,
and the loader's worker shutdown writer. The independent `00bdf950/00be0980`
audit establishes FileBlock entry; its destructor's exit semantics are still
needed before a standalone owning FileBlock implementation. The known deleting
destructor `00bdebe0` calls `00bdcb30`, whose body remains outside these packets.
