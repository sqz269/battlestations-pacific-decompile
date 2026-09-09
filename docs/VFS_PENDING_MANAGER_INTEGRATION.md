# Pending VFS manager and FileStore integration

Addresses: `00bdda10`, `00bdc100`, `00bdc1d0`, `00bdc1e0`, `00be7cd0`, `00be78b0`, `00be7b20`, `00be7cb0`, `00be7cc0`, `00bdb0b0`, `004fc150`, `00bb9d30`, `00bb9d40`

Pending opening does **not** use the synchronous manager's normalize/alias
route. FileStore resolves one local name, but manager `00bdda10` selects mounts
using the untouched original name. Both complete names reach the provider.
The physical provider accepts actual overlapped work; FileStore and MPKG
decline this operation. FileStore records one callback per resolved key and
does not turn duplicate requests into additional notifications.

The typed implementation is in [vfs_pending.hpp](../include/bsp/vfs_pending.hpp),
[vfs_pending.cpp](../src/vfs_pending.cpp),
[file_store_requests.hpp](../include/bsp/file_store_requests.hpp) and
[file_store_requests.cpp](../src/file_store_requests.cpp). It composes the
existing `VfsMount` collection, physical queue, resolver and resident FileStore.
It is not a native manager/provider ABI replacement.

## Manager names, acceptance and pump

`00bdda10` takes ECX manager and stack `(firstName, secondName, callback, flags)`,
returns acceptance in AL and uses `RET 10h`. It copies those four arguments into
visitor `00bdc100`, traverses `00bdd0a0` with **secondName**, saves acceptance,
then destroys the visitor. There is no normalization or `00bdca80` alias call.
Visitor `00bdc1e0` sends both full copied names, callback and flags to provider
virtual `+0Ch`. The traversal suffix is unused. False continues traversal;
true stops it. Manager acceptance does not mean callback completion.

The binding uses `PhysicalDirectory::build_path_00bf3970(firstName)` exactly.
It does not strip the selected mount prefix from that first name or replace it
with the traversal suffix. The physical queue retains owned names and stable
I/O allocations, including on immediate `ReadFile` success. The earlier
[dispatch](VFS_PENDING_DISPATCH.md) and [lifetime](VFS_PENDING_LIFETIME.md)
audits describe its separate completion pump.

Fresh table bytes establish the other two startup providers:

| Provider/table | Pending open `+0Ch` | Tick `+28h` |
|---|---|---|
| FileStore `00d689e8` | `00be7cb0`: `XOR AL,AL; RET 10h` | `00be7cc0`: `RET` |
| MPKG `00d64390` | `00bb9d30`: `XOR AL,AL; RET 10h` | `00bb9d40`: `RET` |

These are actual native decline/no-op operations, not unresolved operations
given success stubs. In particular, resident FileStore entries do not make its
provider pending-open slot call back inline. RequestFile checks residency at a
different layer before provider dispatch.

`00bdb0b0` pumps the same manager `+3Ch/+40h` mount tree used by traversal. It
calls each node's provider `+28h`, in iteration order, without a name filter or
provider-identity deduplication. Shared providers can therefore be visited more
than once. `VfsPendingPumpReport` retains one report per mount visit; its
`remaining_observations` sum is explicitly not a unique outstanding-I/O count.
Completed/succeeded/failed counts describe the work those visits performed.

The manager wrapper excludes recursive pumping and mount mutation during
callbacks. Its thread-local guard detects nested pumps, not concurrent access;
the caller must serialize access. No creator-thread affinity is imposed.
Callbacks may submit additional requests, and the physical pump can visit
their completions in the same pump. Frame and loading-thread stop-wait callers
remain those established in `VFS_PENDING_LIFETIME.md`.

## FileStore request and completion order

`00be7cd0` takes ECX FileStore, stack `(originalName, userCallback)`, returns AL
and uses `RET 8`. Its established order is:

1. Copy/normalize with `00bee780`, then resolve the local name using current
   manager `00bdf4c0`. Resolution occurs before either cache lookup. Failure
   returns false, with no pending insertion or user callback.
2. Look up the **resolved** key in resident tree `+14h`. A hit returns true
   without opening a provider or invoking/registering this request's callback.
3. Look up the same key in pending tree `+20h`. A hit also returns true without
   replacing the saved callback or adding another callback.
4. Copy the resolved key and one callback pointer into the pending tree, then
   call `00bdda10(resolvedName, originalName, 00be7b20, 2)`.
5. Immediate rejection removes the new pending key through `00be79c0`, invokes
   manager error wrapper `00bd9e30(-1)`, and returns false if that handler returns.

`00be5530` and `00443d00` establish CRT case-insensitive pending-key order,
including empty-string gates and no stored-length tie-break. Pair constructors
`00be6120`/`00be62e0` own the key string and copy the single callback scalar.
There is no native callback-list fan-out. The host result enum distinguishes
`already_resident`, `already_pending`, `queued` and `rejected`; the first three
correspond to native true, but only `queued` creates new I/O.

Physical completion calls adapter `00be7b20(stream, firstName, secondName)`,
with three callee-cleaned stack arguments. The adapter calls singleton getter
`004fc150`, reads its factory's store at `+8`, and calls:

`00be78b0(firstName, secondName, stream)`

The completion method takes ECX store and uses `RET 0Ch`. It normalizes a local
copy of firstName for the pending lookup, reads the callback at node `+14h`,
**erases the pending record**, inserts the stream via `00be7760`, then invokes
the saved **user callback with `(firstName, secondName)` only** at `00be7942`.
The user callback has two callee-cleaned arguments and receives no stream.
Those arguments preserve the completion inputs, not the local normalized copy.
The host copies their strings before calling user code.

An intervening resident insertion still wins through AddFile's duplicate rule;
native completion ignores the insertion result and calls the saved callback.
Thus user code observes the resident store after insertion, and a same-key
request from that callback returns resident without another notification.

## Factory lifetime and failures

The native adapter retains no originating FileStore pointer. Getter `004fc150`
reads current singleton `0109db68` and can lazily construct/register the factory
if it is absent. Completion then reads that factory's `+8` directly; it does not
call the lazy store getter. Destroying/replacing the singleton or store while
requests exist can therefore redirect completion or leave it without the
expected pending entry. Native general replacement/teardown policy is unproven.

`FileStoreRequests` uses an explicit **stable factory contract**: construct one
controller for the shared store supplied by `VfsProviderFactories`, and keep
that factory/store identity unchanged until all accepted physical queues drain.
The controller owns no VFS context or queue. Accepted callbacks retain its
shared request state and resident store as a host lifetime safeguard. They do
not implement arbitrary native singleton replacement. Caller callback captures
must also respect that drain-before-destruction contract.

A terminal failed physical read is removed from the physical queue without
callback. Consequently its FileStore pending key remains and later requests
return `already_pending`. The implementation preserves this visible state;
it has no invented failure callback, retry or clearing operation. Pump reports
carry actual physical errors. The pending-map count cannot prove that OS I/O
is still active, and zero physical requests cannot prove every FileStore key
completed successfully.

The complete raw pending-node erase `00be6a20` releases key storage and the node,
then decrements the tree count after `_free`; Ghidra's body stops before that
tail. Complete destructor `00be7bf0` clears resident streams, destroys pending
keys/nodes and frees tree sentinels. Pending subtree helper `00be66c0` frees
strings/nodes without calling the stored callback. These are bookkeeping
destructors, not a demonstrated I/O cancellation or failure-notification path.

Host `begin_pending_shutdown_fragment` stops provider submissions, then callers
must explicitly pump until every provider is drained before destroying owners.
It does not clear FileStore keys or wait/cancel. Provider errors do not prevent
later pump visits. User callback exceptions propagate after the physical queue
records the exception and releases that terminal request. FileStore's earlier
pending removal/resident insertion is not rolled back. Allocation exceptions
follow that same visible mutation order; supported provider submission
exceptions occur before I/O acceptance and remove the newly inserted key.

## Evidence and validation boundary

Eight fresh analysis batches verified project `bsp`, program
`/battlestationspacific.exe`, x86 LE and base `00400000`. Complete code spans,
table bytes and RequestFile diagnostic strings matched installed PE SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The singleton slot is in the PE's virtual zero-filled data tail, not raw file
bytes; its saved Ghidra zero value is not a runtime observation.

Exact spans, hashes, ABIs, proposed annotations and raw-tail boundaries are in
[vfs_pending_manager_integration_audit.json](../reports/vfs_pending_manager_integration_audit.json).
Ignored evidence is under `exports/bsp/parallel_vfs_pending_manager/`.
No Ghidra mutation or shared ledger change was made by this packet.

MSVC Win32 `/Zs /W4 /WX /fp:strict` accepted the manager/controller and probe sources.
The existing installed-file physical probe now also builds real factory/root
and FileStore mounts around the same `inputs.lua` byte oracle. It checks deferred
completion, actual FileStore decline and physical submission, pending/resident
deduplication, first callback only, original/resolved names, alias bypass,
per-mount pump visits, cache bytes and copied stream survival after owner
destruction. The earlier two-read append and callback-error-alias checks remain.
Timeout or an exception with possibly active I/O explicitly fails the process;
the fixture does not destroy an undrained queue. The primary integrator then
reported a successful coordinated Win32 build, both existing CTests and the
full installed probe. The mounted FileStore portion reported one callback,
zero duplicate callbacks, one cache decline, one physical submission, two
pumps with two no-op/two physical visits, one completion and zero failures.
This is installed-file host validation. Routing by the second name with a
distinguishing nonempty prefix, repeated-provider pump visits and terminal-I/O
pending-key stranding remain source-reviewed boundaries, not additional tested
scenarios. The empty-root mounts do not independently distinguish which name
selects a mount.

Native differential execution, cancellation, failed-I/O pending-map recovery,
arbitrary provider callbacks, singleton replacement, concurrent use, unmount
policy and original-game loading behavior remain outside the validated boundary.
