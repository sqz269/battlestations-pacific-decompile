# Queued resource worker dispatch

Addresses: 007175d0, 007188a0, 00b80720.

`007188a0` is a cached resource-loading wrapper with a single ECX name input.
It does **not** consume the EAX value left live by the queued worker. The
wrapper selects a concrete resource factory and dispatches the name through a
resource manager. The manager opens a VFS stream and reads/dispatches a root
object on a cache miss. This packet identifies that route without substituting
a placeholder parser or treating raw bytes as a completed game resource.

## Verified scope and ABI

Every live CLI batch verified project `bsp`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, and image base
`00400000`. The installed executable's complete ranges were matched against
Ghidra memory and decoded through their final RET. Hashes and evidence paths
are in `reports/vfs_load_worker_dispatch_audit.json`.

| Function | Matched inclusive span | Original ABI |
| --- | --- | --- |
| `007188a0` | `007188a0-007188bb` | ECX name-wrapper pointer; EAX resource pointer; no stack arguments; RET |
| `007175d0` | `007175d0-0071769d` | No explicit inputs; EAX factory singleton; RET |
| `00b80720` | `00b80720-00b80a4a` | ECX resource manager; stack name/factory pointers; EAX resource pointer; RET8 |

Factory primary vtable `00cfd850-00cfd857` and lifetime-subobject vtable pointer
`00cfd84c-00cfd84f` also match disk. Caller `00501510`, worker lifecycle
`00be0a30`, and stream functions `00bdf950`/`00be0980` belong to separate
packets. Only `007175d0` and `00b80720` were added as bounded callees here.

The pseudocode for `00b80720` has confused stack temporaries and omitted
register/stack arguments. Assembly establishes the call contracts below; its
incorrect null-looking local expressions are not used as behavioral evidence.

## Incoming EAX is not a parameter

At `007188a2`, the wrapper saves its ECX name in ESI. At `007188a4`, it calls
`007175d0` without reading incoming EAX. The **first** instruction of that
callee is `MOV EAX,FS:[0]`, replacing the incoming register value before any
possible use. The callee then obtains the singleton from global `00e19b90`.

At `007188a9`, the factory result is saved in EDI. `004c1400` returns a manager
pointer in EAX. The wrapper pushes factory then saved name, moves the manager
to ECX, and calls `00b80720`. The latter's EAX result passes directly through
the wrapper's POP/RET sequence. Therefore the worker's prior EAX value cannot
be interpreted as an implicit job pointer, parser context, or output slot for
this call.

## Factory selection

`007175d0` creates an eight-byte singleton under the singleton-lifetime
manager's critical-section protocol, including a second null check while
locked. Its primary vtable is `00cfd850`; its lifetime-registration subobject
at +4 uses `00cfd84c`. Global `00e19b90` holds the primary pointer.

Primary virtual+4 points to `0071b870`. On a miss, `00b80720` calls that slot
with the original name wrapper and stores the returned resource pointer. This
is the concrete object factory boundary. The class name and serialized format
have not been recovered in this packet. Allocation failure can leave a null
singleton; the downstream call path does not show graceful null handling.

## Manager cache and miss route

`00b80720` uses these manager fields:

| Offset | Observed role |
| --- | --- |
| +14h | Associative name/resource cache object |
| +18h | Sentinel pointer accessed by iterator-end comparison |
| +20h | Current factory pointer |
| +24h | Current/result resource pointer |

The factory is stored at +20h before lookup. `00b7e7b0` receives the original
requested name. A hit reads resource node+14h, stores manager+24h, increments
resource+4 with `InterlockedIncrement`, and returns. It performs no VFS access
or parsing. The hit path leaves manager+20h populated; only the miss-completion
path explicitly clears it.

The miss sequence is:

1. Read an opaque DWORD metric from global `0109cefc` virtual+4.
2. Call factory virtual+4(original name) and store its result at manager+24h.
3. Copy the name and call mutable VFS resolution `00bdf4c0` at `00b80825`.
   Its AL return is not checked.
4. Open the resolved copy through VFS virtual+4 with flags 2 at `00b8083c`.
5. Construct the local reader using `00bea150`, attach the stream with
   `00bf0430`, and copy the resolved name into associated reader storage.
6. Decrement the acquired stream's +4 reference count and call stream virtual+0
   if it reaches zero. Reader attachment/retention is a separate dependency.
7. Obtain the reader's root/result object through `00bea700`, then call
   `00b7f430(manager, &root)` at `00b808c5` to dispatch it.
8. Build an association containing the **original requested name** and current
   resource through `00b7f290`; insert it into manager+14h with `00b803b0`.
9. Clear manager+20h. Read the opaque metric again and store
   `first_metric - second_metric` at result resource+40h.
10. Destroy root and reader temporaries using `00be9ed0` and `00be9f10`, then
    return the resource pointer in EAX.

The metric's meaning is not established; the subtraction is not evidence of
elapsed time. Cache lookup/insertion retain the original name, while VFS
resolution mutates a separate copy used for I/O. Exact cache comparison and
canonical alias behavior require the lookup helper's contract; they must not
be copied from the separate material-effect cache implementation.

## Ownership and failure limits

The hit path explicitly returns an incremented resource reference. On the new
path, the returned resource originates at factory virtual+4, but the factory's
initial reference and the cache insertion's ownership transfer remain outside
this bounded audit. `00b80720` contains no additional explicit resource AddRef
in its miss body. An owning host implementation needs those two contracts.

The manager does not check VFS-resolution status, stream null, factory-result
null, or a parser status in this body. It later unconditionally uses the
stream's reference-count address and the result resource's +40h field. Do not
introduce a claim of graceful invalid-file handling or a guessed fallback.

The factory's singleton initialization is locked. No enclosing lock is visible
in `00b80720`, which writes shared current-factory/current-resource state. This
does not establish that arbitrary concurrent or recursive calls are safe.

## Concrete next dependencies

| Address | Required contract |
| --- | --- |
| `0071b870` | Factory virtual+4: concrete resource construction, type, initial ownership |
| `00b7f430` | Root dispatch and actual resource-field interpretation |
| `00bf0430` | Reader stream attachment and retention before caller release |
| `00bea700` | Root/result creation and serialized read format |
| `004c1400` | Resource-manager creation and global lifetime |
| `00b7e7b0` | Original-name cache comparator |
| `00b803b0` | Cache insertion ownership and duplicate handling |

The index associates `00b7f430` with `Resource`, `Hierarchy`, and `BoundingBox`
literal references. Those are useful next-packet search terms, not a recovered
file-format specification. The worker's deferred request invokes a structured
game-resource loader; merely reading the queued file into memory cannot replace
the missing factory, root-reader, and dispatch behavior.

No C++, shared metadata, Ghidra annotations, new tests, or commits were changed
for this packet. The result is an assembly/disk-backed dispatch audit; queued
resource reconstruction, build validation, and game validation remain separate.
