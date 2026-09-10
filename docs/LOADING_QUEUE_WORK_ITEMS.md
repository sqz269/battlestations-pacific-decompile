# Loading queue work items and concrete synchronization

Addresses: 004fdba0, 00501670, 005019d0, 005051a0, 00509fd0, 0050ace0, 00bd1920.

`00501670` removes one 16-byte work record by shifting later records left,
destroying the final owned name, and decrementing count. `005051a0` destroys
the job's owned name and work-array storage. Neither routine releases the raw
context or processed-resource pointers carried by a work record.

The loader uses one initially unsignaled **manual-reset event**. Its publication,
worker wait/reset, and state-3 busy query are now tied to concrete Win32 event
methods. The recovered loader destructor drains jobs and destroys that event,
but performs no worker-stop write, wake, thread join, or thread-handle close.
A prior worker-shutdown protocol remains unresolved.

## Evidence and ABI

`reports/loading_queue_work_items_audit.json` contains full installed-PE versus
Ghidra byte matches, hashes, prior names/comments, initial reported boundaries,
flow-repair proposals, and explicit uncertainties. Each live CLI verified
project `bsp`, `/battlestationspacific.exe`, x86 language, and image base
`00400000` before reading. No shared metadata or Ghidra was changed here.

| Address | Complete matched span | Original ABI |
| --- | --- | --- |
| `00501670` | `00501670-00501715` | ECX work vector; signed index stack; RET4 |
| `005051a0` | `005051a0-00505210` | ECX job; no stack arguments; RET |
| `005019d0` | `005019d0-00501a49` | ECX work vector; signed target count stack; RET4 |
| `004fdba0` | `004fdba0-004fdbf8` | ECX loader; EAX same loader; RET |
| `00509fd0` | `00509fd0-0050a052` | ECX loader; no stack arguments; RET |
| `0050ace0` | `0050ace0-0050acfd` | ECX loader; deletion flags stack; EAX original pointer; RET4 |
| `00bd1920` | `00bd1920-00bd1951` | ECX event wrapper; AL status; no stack arguments; RET |

All seven complete spans, totaling 701 bytes, were decoded through RET after
matching. The loader destructor vtable slot and all five event slots were
matched as data. `00bd1920` initially lacked a function definition; its full
50-byte body ends before CC padding.

Three free-call continuations require primary integration review:

| Call site | Missing or truncated continuation |
| --- | --- |
| `005051f9 -> 00bf6989` | Job body initially ended at `005051fd`; stack/SEH cleanup continues through RET `00505210` |
| `0050a017 -> 00bf6989` | Loader body initially ended at `0050a01b`; event destruction/global cleanup continues through RET `0050a052` |
| `0050acf0 -> 00bf65ac` | Existing scalar-wrapper range already reaches RET4, but its initial listing omits `0050acf5 ADD ESP,4` |

The complete matched bytes, rather than the early-return pseudocode, establish
the cleanup behavior. As in the preceding packets, inspect and repair the
specific call's flow override; do not change shared `_free` declarations.

## Work-record layout and removal

The job contains a work-vector prefix at +8h:

| Job offset | Meaning |
| --- | --- |
| +8h | Record storage pointer |
| +Ch | Active record count |
| +10h | Record capacity |

Each record occupies 10h bytes:

| Record offset | Meaning |
| --- | --- |
| +0/+4 | Owned native-string length and character pointer |
| +8 | Raw callback-context DWORD |
| +Ch | Raw processed-resource/result pointer DWORD |

`00501670(vector, index)` copies every later record into the preceding slot.
It uses the existing string assignment/allocation helper for +0/+4, copies
the name bytes, and copies +8/+Ch as raw DWORDs. It then destroys the last
active record's owned string and subtracts one from count. It neither releases
nor retains the resource/context pointees and does not free array capacity.

There is no bounds guard. For a valid removal, require nonzero count and
`0 <= index < count`. An index at or above `count - 1` skips shifting but still
removes the last active record; zero count or a negative index can access
outside storage. A future host guard must be labeled as a supported-domain
restriction, not native error behavior.

The earlier state-3 path in `00509190` calls the optional job+1Ch callback with
the first record's result/context, removes index 0, and submits again if
job+Ch remains nonzero. This packet confirms that +Ch is a **work count** and
that removal advances the first work record. It does not establish that the
callback consumes or releases the result.

## Resize and job destruction

`005019d0` reserves through `005018a0` when target count exceeds capacity.
New active records are initialized to four zero DWORDs. Shrinking walks
backward: decrement count, destroy that record's name, repeat, then store
target count. Context and result pointees remain untouched in both directions.
The growth allocator and invalid/negative target behavior remain external.

`005051a0` performs only storage cleanup:

1. Free the job name at +14h/+18h using the established native string helpers.
2. Resize the work vector at job+8h to zero through `005019d0`.
3. Free its record-storage allocation.
4. Restore the stack/SEH chain and return.

It does not free the outer job, invoke its callback, release work results,
release job+20h FileBlock, or join a worker. Existing front retirement
`00506bf0` handles FileBlock cleanup and the outer job free separately; see
[VFS_LOADING_PUMP_LIFETIME.md](VFS_LOADING_PUMP_LIFETIME.md).

## Concrete event and publication protocol

Loader constructor `004fdba0` initializes the observed 20h-byte prefix:

| Loader offset | Initial value / role |
| --- | --- |
| +0 | Vtable `00ceb198` |
| +4 | Worker stop word, 0 |
| +8 | Published job pointer, 0 |
| +Ch | Manual-reset event pointer |
| +10/+14/+18 | Queue array/count/capacity, all 0 |
| +1Ch | Worker thread handle, 0 |

At `004fdbc6`, CL is explicitly set to **1** before calling existing
`BSP_Event_Create` (`00bd1970`). Its established contract is
`CreateEventA(NULL, manualReset, FALSE, NULL)`; therefore the loader's event is
manual-reset and initially unsignaled. The renderer worker's separate CL=0
auto-reset events must not be substituted here.

The fresh event-vtable read at `00d6821c` establishes:

| Slot | Target | Behavior |
| --- | --- | --- |
| +0 | `00bd19b0` | Close handle and optionally delete wrapper |
| +4 | `00bd1910` | `SetEvent` |
| +8 | `00bd17c0` | `WaitForSingleObject(handle, INFINITE)` |
| +Ch | `00bd1960` | `ResetEvent` |
| +10h | `00bd1920` | `WaitForSingleObject(handle, 0)` signal-state query |

The first four methods reuse the already reconstructed event contract in
[RENDER_WORKER.md](RENDER_WORKER.md) and `Win32Event`. The last method was
recovered here. It returns AL=1 for `WAIT_OBJECT_0`, AL=0 for timeout or
abandoned status, and calls `GetLastError` then returns AL=0 for `WAIT_FAILED`.
Unexpected numeric results preserve their low EAX byte instead of taking a
catch-all false branch.

This resolves the earlier opaque virtual calls:

1. `00501620` creates the worker if necessary, publishes a nonempty job, then
   signals the event. Its zero-work-count path still permits thread creation
   without publication/signaling.
2. `00501510` waits indefinitely on the event, checks **loader+4**, and processes
   the first work record. After storing the result it resets the event, waits
   again, and checks the worker stop word again.
3. `00509190` state 3 queries the event at virtual+10h. A signaled event means
   work is still active and returns without advancing the front.

Because the event is manual-reset, the worker wait does not consume the signal:
it remains visible to the state-3 query until the worker resets it. This is
concrete synchronization, not a no-op wake or a guessed semaphore protocol.
The prior publication and worker ordering remain as audited in
[VFS_LOAD_PROCESSING_START.md](VFS_LOAD_PROCESSING_START.md).

## Loader destruction and unresolved worker exit

The constructor vtable's destructor slot points to `0050ace0`, which calls
`00509fd0`. The full destructor:

1. Installs loader vtable `00ceb198` and calls `005092e0` to drain queued jobs.
2. Calls `004fb4b0(loader+10h, 0)` and frees the queue-array storage.
3. If event+Ch is nonnull, invokes event virtual+0 with deletion flag 1 and
   clears the event pointer.
4. Clears global `00e18d4c`, installs lifetime-base vtable `00ce3818`, restores
   the stack/SEH chain, and returns.

The scalar wrapper then frees the outer loader only if deletion flags bit 0 is
set. Neither full body writes worker stop word +4, signals worker exit, waits
on thread handle +1Ch, or closes that thread handle. Queue draining is not a
thread join. In particular, this destructor alone does not establish safe
shutdown of a persistent thread waiting on the event it destroys.

This is an explicit external lifecycle requirement, not proof that no shutdown
writer exists elsewhere. Bounded caller/neighbor metadata inspection did not
identify a further proven target. No speculative shutdown implementation was
added.

The **loader+4 worker stop word** remains distinct from the **front-job+4 stop
byte** set by `005092e0`. The existing FileStore completion callback continues
to route through the current global loader front; this audit adds no captured
origin, synthetic completion, timeout, cancellation, or flag merging.

Only this document and its audit report are new tracked artifacts. There are
no source changes, builds, tests, snapshots, commits, or game-validation claims
in this packet.
