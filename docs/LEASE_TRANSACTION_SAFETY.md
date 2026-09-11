# Lease registry transaction safety

Packet `orch2_lease_transactions_t` repairs the lease writer in
`tools/coordination.py`. This is repository tooling, with no game function or ABI
claim. The integrator reported a malformed 811-byte suffix after simultaneous
writers used the same `leases.tmp`; its separate repair preserved 756 complete
records. This packet does not repair or stress the live registry.

The former `claim` and `release` both loaded `leases.jsonl` outside any lock, then
wrote a shared temporary filename. Distinct temporary files alone would still
lose changes when two writers replace the registry from stale snapshots.

## Transaction contract

Both APIs now acquire `lease_transaction()` before reading the registry and keep
it through conflict checking, coverage merging, mutation, and publication.
`save_leases(rows)` rejects calls outside that transaction. Any future manual
writer must use this order:

```python
with coordination.lease_transaction():
    rows = coordination.load_leases()
    # Check and modify this snapshot while still holding the transaction.
    coordination.save_leases(rows)
```

The lease schema, active/expired filtering, owner identity, address/range/file
overlap checks, extension unions and original claimed timestamp, and retained
release records are unchanged. The repository writer search found only `claim`
and `release` calling `save_leases`; both are covered. Read-only listing can read
an atomically published snapshot without taking the transaction lock.

`leases.lock` is a persistent, separate file. On Windows, `CreateFileW` opens it
with read/write access, zero sharing, `OPEN_ALWAYS`, and non-inheritable security
attributes. The open handle owns exclusivity; closing it or terminating its
process releases it. No timestamp, PID guess, expiry, rename, unlink, or timeout
can reclaim a live handle. The Windows sharing and handle contracts are defined
by [Microsoft's CreateFileW documentation](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew).
The POSIX fallback uses `flock`; it was not exercised in this Windows validation.
Same-thread nested calls reuse the transaction. A different registry inside an
active transaction is rejected. The existing `ghidra.lock` protocol is separate
and was not changed.

Acquisition retries for up to 30 seconds and then raises `TimeoutError` without
altering the lock file. Registry reads, atomic replacement, and cleanup retry
Windows sharing/access errors for up to five seconds, with 20 ms intervals.
This includes Win32 errors 5/32/33 and CRT `EACCES`/`EPERM`, since the focused
fixture initially exposed `PermissionError(errno=13)` without `winerror` during
a concurrent read. Persistent permission errors still fail. Invalid JSON is
reported; it is never suppressed, truncated, or repaired by these APIs.

Publication uses a unique, exclusively created `.leases-<pid>-<random>.tmp` in
the registry directory, writes the complete JSON Lines snapshot, flushes and
`fsync`s it, closes it, then calls `os.replace`. A failure only attempts to remove
that writer's temporary file. Abrupt termination can leave an unused unique
temporary file; it is never treated as registry content. This protocol does not
claim recovery from disk failure or full power-loss durability.

## Validation and deployment

One focused regression scenario in `tests/test_coordination_transactions.py`
launches ten simultaneous writers and a continuous reader against a fresh
`TemporaryDirectory` supplied as `BSP_COORDINATION_DIR` to every process. It
checks 100 distinct leases, every address/range/file extension, 50 releases,
exactly one successful contested-file claimant, and complete JSON snapshots.
The same scenario checks a live-holder timeout, killed-holder recovery, nested
API use, unlocked-save rejection, a real Windows deny-sharing handle followed
by successful replacement retry, failed-serialization preservation, and refusal
to overwrite an existing malformed suffix. Normal completion leaves no
temporary snapshot files. The fixture passed on Windows 11 / Python 3.13.11;
the four existing exporter unit tests also passed. No C++ source changed, so no
native build, CTest, Ghidra, or game validation was performed for this packet.

The JSON format remains readable by older worktrees, but **older writers do not
honor `leases.lock`**. An older claim/release can still overwrite a newer snapshot
or collide with another old writer's `leases.tmp`. Before relying on the shared
registry under concurrency, deploy this complete `tools/coordination.py` change
to every active writer checkout and restart any process that already imported
the old module. Drain old operations before resuming concurrent writes. This
worker only changes its own worktree; integration and rollout remain with the
integrator. Do not delete the persistent lock file as a recovery action.
