# Pending physical I/O lifetime and pump boundary

Addresses: `00bf3d00`, `00bf3da0`, `00bf3ed0`, `00bf4240`, `00bf46b0`, `00bf4c70`, `00bdb0b0`, `00bf41c0`, `00bf3880`, `00bf3c10`, `005092e0`, `00737a50`

The native queue can relocate request records without moving their heap I/O
allocations. Completion runs through an explicit provider pump reached from
both the application frame and a loading-thread stop wait. This supports a
bounded submission/pump interface, provided it owns pending resources until
completion and excludes recursive pumping, provider destruction and registry
mutation during callbacks. Native cancellation and complete FileStore failure
cleanup remain unresolved.

This read-only audit extends [VFS_PENDING_DISPATCH.md](VFS_PENDING_DISPATCH.md).
Eight live batches checked the existing `bsp` project and
`/battlestationspacific.exe` before reads. Twelve complete code spans and the
physical vtable/stop-wait string matched the installed PE; exact lengths, raw
hashes, ABIs and annotation proposals are in
[vfs_pending_lifetime_audit.json](../reports/vfs_pending_lifetime_audit.json).
Raw evidence is under `exports/bsp/parallel_vfs_pending_lifetime/`.

One earlier wording error was corrected with parent authorization: submission
builds its path through **provider virtual slot +1Ch**, at `00bf4410/00bf441e`.
The provider **data field +1Ch** is pending-queue capacity. These are different
uses of the same numeric offset.

## Queue relocation and removal

The queue descriptor at provider `+14h` contains data/count/capacity; records
are 56 bytes. All statements here concern valid nonnegative counts and indices.
Unchecked integer overflow, negative resize counts and allocation exceptions
are not a supported host contract.

| Routine | ABI | Established normal-path behavior |
|---|---|---|
| `00bf41c0` append | ECX queue; source record stack; RET 4 | At count==capacity, reserves max(1,2*capacity), copy-constructs at count, then increments count. |
| `00bf3da0` reserve | ECX queue; requested signed capacity stack; RET 4 | Clamps requested capacity to at least 1; grows only when larger than current capacity; allocates `capacity*56`. |
| `00bf3c10` copy construction | ECX destination; source record stack; EAX destination; RET 4 | Copies I/O pointers/scalars and independently owns both names. |
| `00bf3d00` assignment | ECX destination; source record stack; EAX destination; RET 4 | Copies I/O pointers/scalars and copies both names through the existing string resize/copy helpers. |
| `00bf4240` erase | ECX queue; index stack; RET 4 | Assigns each later record into its predecessor, destroys the final record's names, then decrements count. |
| `00bf3ed0` resize | ECX queue; requested signed count stack; RET 4 | Grows/default-initializes records or removes records from the end using name cleanup; sets count. |
| `00bf3880` record cleanup | ECX record; RET | Releases only the two names; does not release I/O allocations or handle. |

Reserve's complete raw continuation at `00bf3e70..00bf3e79` matters: after
copying every live record, it destroys old names, frees the old record block,
then stores the new data pointer and capacity. Ghidra omits this continuation
after its misleading no-return `_free` interpretation. Count is unchanged.
The handle, `OVERLAPPED*`, original allocation and aligned buffer remain the
same pointer values; neither copy nor old-record cleanup releases them.

Erase is currently missing a Ghidra function definition. Its complete span is
`00bf4240..00bf429f`, 96 bytes, exclusive end `00bf42a0`, SHA-256
`bc0c95c4ad21d9695b899b5c41da11f7691f380f10fc88735386040e4136d604`.
The decisive assignment loop is `00bf4262..00bf427d`; final name cleanup and
count decrement are `00bf4280..00bf4297`. It preserves relative order and does
not shrink capacity. Its lack of recorded xrefs does not negate the raw
completion call at `00bf47b9`, which Ghidra omits after `_free`.

Fields `+14h` and `+34h` are not copied, assigned or initialized by these
helpers. Their meaning remains unknown; this packet does not promote them
into additional ownership fields.

## Callback reentrancy and same-pump submissions

`00bf46b0` invokes the callback at `00bf476d` while the completed request is
still in the queue. Only after callback return does it release its stream,
reload the queue base, free staging/OVERLAPPED memory, close the handle and
erase the entry (`00bf4785..00bf47b9`). No in-progress flag or recursive-entry
guard is set in the inspected pump.

These are direct consequences of that order, not runtime demonstrations:

- A recursive pump can see the same completed record and invoke its callback
  again before the first invocation has cleaned it up. Subsequent cleanup can
  target stale or different records. Recursive pumping is outside a faithful
  bounded interface's supported domain.
- An append during a callback can reallocate the record array. Stable heap I/O
  resources survive, and native cleanup reloads the queue base afterward.
  However, the callback received pointers to the old record's name wrappers;
  reserve frees those wrappers and their old name allocations. Copy the names
  before any action that can append; do not retain/use the borrowed arguments
  after such an action.
- After erasing a completed request, the pump rechecks the same index. A
  pending request alone advances it. The loop compares against the current
  count at `00bf47ca`; it does not freeze an initial count. Requests appended
  by a callback may therefore be visited, and if complete, dispatched in the
  same outer pump invocation. This is still deferred relative to submission.
- Provider destruction or non-append queue mutation during callback invalidates
  assumptions used by cleanup. No provider retain/release protects this call
  in the inspected pump.

The prior completion contract still applies: callback arguments are
`(independently backed stream, &firstName, &secondName)`, and the provider
releases its temporary stream reference after return. A failed completed I/O
is removed without callback. Actual transferred bytes are not validated
against the original size before native memory copying.

## Outer pump callers

`00bdb0b0` is a complete 108-byte routine, ECX VFS manager, no stack arguments,
RET. It walks the manager's provider tree using descriptor `+3Ch` and sentinel
`+40h`. For each node, provider is node `+18h`; it invokes provider virtual
`+28h` at `00bdb0ff`, then advances the existing iterator at `00bdb105`.
The verified physical table `00d69168` maps that slot to `00bf46b0`.

No snapshot of providers, temporary provider reference or lock appears around
that call. Registry mutation during callbacks is therefore outside the bounded
contract; the iterator's current node is still needed after the provider
returns. The shared iterator helper and arbitrary provider implementations
were not expanded into separate correctness claims.

Two direct callers are established by fresh references and complete raw bodies:

- `BSP_Application_RunFrame` (`00737a50`) loads VFS global `0109ceec` at
  `00737b79` and calls `00bdb0b0` at `00737b7f`, once on its ordinary frame path.
- `005092e0`, identified by byte-checked
  `GIAchievements::StopLoadingThread()`, also loads `0109ceec` and calls the
  manager pump at `0050932a`. While object `+14h` is nonzero, it sleeps 100 ms,
  pumps providers, calls `00509190`, and repeats. Its stop flag and surrounding
  virtual calls belong to that caller's loading-thread lifecycle; they do not
  establish per-request physical-I/O cancellation.

Callbacks run synchronously on whichever thread invokes the pump. This audit
does not establish thread affinity or prove these two direct callers are every
possible indirect route. A main-frame-only scheduling claim would be too narrow.

## Teardown boundary

The complete physical destructor `00bf4c70..00bf4d2d` includes a continuation
missing from Ghidra pseudocode. Its explicit pending-queue teardown calls
`00bf3ed0(queue,0)` at `00bf4cfe`, then frees the record array at `00bf4d06`.
Resize-to-zero releases names only. This sequence does not drain the pump,
close pending handles or free pending staging/OVERLAPPED allocations.

No pending-I/O cancellation operation is established by the inspected queue
teardown. If reached with live requests and no external drain/cancellation,
the sequence loses their bookkeeping without releasing those I/O resources.
Whether higher-level lifetime rules prevent that situation is unresolved.
The destructor's unrelated tree/base cleanup helpers, unwinding cleanup and
outer shutdown/unmount policy were not expanded; they are not a proven
replacement cancellation mechanism.

## Smallest supported host contract

Keep the earlier submission plus explicit-pump boundary, with these ownership
and scheduling requirements:

1. A serialized owner holds each accepted request's handle, stable staging
   allocation, stable `OVERLAPPED`, both owned names and callback through
   completion. Moving a queue entry must not duplicate resource destruction.
2. Submission never calls completion inline. Pumping examines the live queue
   in order; success creates independent memory backing before callback and
   terminal cleanup removes the completed entry afterward.
3. Exclude recursive pumping, concurrent queue access, provider destruction and
   provider-registry mutation during callbacks. Allow callback submission only
   with names copied before enqueue and no dependence on the borrowed wrappers
   afterward. A host API that supplies stable name copies can make that lifetime
   explicit; it is a safe interface choice, not a recovered native guarantee.
4. Require the queue to be drained before ordinary provider destruction.
   Cancellation needs a separate contract proving OS completion/resource
   lifetime and FileStore pending-state cleanup; clearing this native vector
   is not cancellation.

These restrictions permit a coherent bounded implementation without claiming
native races, allocation failures, reentrant callbacks or shutdown failures
are safe. No C++, tests, shared metadata or Ghidra mutations were added here.
