# Pending VFS dispatch, ownership and completion

Addresses: `00bdc1d0`, `00bdc1e0`, `00bdb740`, `00bf43b0`, `00bf41c0`, `00bf3880`, `00bf3c10`, `00bf46b0`, `00befa40`

The inspected physical provider implements **actual Windows overlapped I/O**. Submission starts `ReadFile` and queues a request; callbacks run from its separate completion pump. Immediate `ReadFile` success is also queued, so it does not make the callback run inline. The stack visitor can be destroyed after acceptance because the physical queue has copied both names and retained the stable I/O allocations.

This read-only lane follows [VFS_PROVIDER_FLAGS.md](VFS_PROVIDER_FLAGS.md). It changed no C++, shared metadata or Ghidra state. Every live batch verified project `bsp`, program `/battlestationspacific.exe`, x86 and image base `00400000`. Exact out-of-range physical/provider helpers were individually authorized by the primary agent. Evidence is in [vfs_pending_dispatch_audit.json](../reports/vfs_pending_dispatch_audit.json), with raw exports under ignored `exports/bsp/parallel_vfs_pending/`.

## Visitor interface and exact missing bodies

Ghidra had no function definitions at `00bdc1d0` or `00bdc1e0` when this lane began. Disk/Ghidra byte comparison and complete raw disassembly establish these boundaries without creating functions:

| Start | End inclusive | End exclusive | Bytes | Behavior |
|---|---|---|---:|---|
| `00bdc1d0` | `00bdc1d3` | `00bdc1d4` | 4 | `MOV AL,[ECX+14h]; RET` |
| `00bdc1e0` | `00bdc207` | `00bdc208` | 40 | Provider dispatch, store returned AL; `RET 8` |

`00bdc1d0` takes ECX visitor and returns its acceptance byte in AL. Proposed descriptive name: `BSP_VFS_OverlappedVisitor_IsAccepted`.

`00bdc1e0` takes **ECX visitor; matched mount record and traversal suffix on stack; RET 8**. It obtains the provider from mount `+8`, calls provider virtual `+0Ch`, then writes returned AL to visitor `+14h`. The provider arguments, in source order, are:

1. Pointer to visitor's first name at `+4`.
2. Pointer to visitor's second name at `+0Ch`.
3. Callback pointer at `+1Ch`.
4. Flags at `+18h`.

The traversal-supplied suffix is **unused**. Existing `00bdda10` traverses using the second name, but this visitor passes the two complete stored names to the provider. Do not substitute the matched suffix while implementing this interface. Proposed descriptive name: `BSP_VFS_OverlappedVisitor_VisitMount`.

The complete 122-byte destructor `00bdb740` releases the second copied name (`+0Ch/+10h`), then the first (`+4/+8`), and installs base table `00d68380`. It does not cancel provider work, destroy a stream, or own the callback. This is stack-context cleanup, not request cancellation.

## Physical submission is genuinely deferred

Fresh physical table `00d69168` maps provider virtual `+0Ch` to **`00bf43b0`**, and completion virtual `+28h` to **`00bf46b0`**. The complete 759-byte submission body takes **ECX physical provider; first name, second name, callback, flags on stack; AL acceptance; RET 0x10**.

Its read-domain gate requires bit 0 clear and `(flags & 0xE) == 2` (`00bf43d9..00bf43fc`). It constructs a physical path from the first name through provider virtual slot `+1Ch` (`00bf4410/00bf441e`); provider data field `+1Ch` is queue capacity. The second name is preserved for completion, not used for this file path. Both `2` and `0x32` satisfy the gate; no separate meaning for bits `0x10`/`0x20` is established beyond the inspected path.

At `00bf442e..00bf4440`, `CreateFileA` receives `GENERIC_READ`, share-read, `OPEN_EXISTING` and attributes **`0x60000000`** (`FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING`). This differs from the synchronous physical adapter's zero-attribute open.

The routine reads the file size, allocates a staging block using the low size word plus `0x20000`, aligns the usable address to `0x10000`, rounds the requested read size to that granularity, and allocates/zeros a separate 20-byte `OVERLAPPED` structure. The handle, staging memory and `OVERLAPPED` are heap/OS resources, not addresses inside the stack visitor.

`ReadFile` at `00bf45d9` receives a nonnull `OVERLAPPED`, the aligned buffer, rounded low-DWORD count and null synchronous byte-count output. The routine accepts either immediate success or `GetLastError() == 0x3E5` / `ERROR_IO_PENDING` (`00bf45df..00bf45f3`). On either accepted result, it copies both names, appends a request record to provider `+14h`, destroys its local record's names and returns AL 1. It does **not** invoke the supplied completion callback in the submission body.

The exact error/allocation/large-file paths are not a safe host API specification. In particular, size arithmetic uses low-DWORD allocation/read counts, native allocation failures are not consistently guarded, and the decompiler's `_free` no-return interpretation omits real cleanup instructions. The next implementation should state its supported size/input domain rather than inherit undefined or unverified cases silently.

## What survives visitor destruction

The provider's queue descriptor is data/count/capacity at `+14h/+18h/+1Ch`; entries have stride `0x38`. `00bf41c0` appends one record, growing capacity through `00bf3da0` when full and copying via `00bf3c10`. Its ABI is **ECX queue descriptor; source record on stack; RET 4**.

The complete 225-byte record copy constructor `00bf3c10` copies handle, `OVERLAPPED` pointer, staging pointers, operation kind, logical size and callback values, and separately allocates/copies both strings. The complete 119-byte cleanup `00bf3880` releases **only those two strings**. It does not free the I/O allocations or close the handle. Thus destroying the submission temporary and then the visitor leaves the queued request's owned name copies and I/O resources intact on the normal accepted path.

| Request offset | Established meaning |
|---|---|
| `+0` | Open file handle |
| `+4` | Heap `OVERLAPPED` pointer |
| `+8` | Operation kind, 1 on this submission route |
| `+0C` | Original staging allocation |
| `+10` | Aligned address used by `ReadFile` |
| `+18/+1C` | Original file size, low/high words |
| `+20/+24` | Owned first name |
| `+28/+2C` | Owned second name |
| `+30` | Callback pointer |

Fields `+14` and `+34`, container growth/removal internals, shutdown cancellation, allocation exceptions and callback reentrancy were not established. A safe typed reconstruction needs stable I/O allocations and separately owned names; it must not retain pointers to the stack visitor or temporary record.

## Completion pump and callback contract

The complete 299-byte `00bf46b0` takes **ECX physical provider; no stack arguments; RET**. Diagnostic strings identify it as `cPhysicalDirectoryX86::Tick`.

It scans queued entries. An `OVERLAPPED.Internal` value of **`0x103`** leaves that request pending; this status value is distinct from Win32 `ERROR_IO_PENDING` (`0x3E5`). For another status it calls `GetOverlappedResult(handle, overlapped, &bytesTransferred, FALSE)` (`00bf46d0..00bf46ef`). There is no blocking wait in this call.

On success with operation kind 1, it constructs a separate memory stream and calls the saved callback at `00bf476d` with:

`callback(stream, &request.firstName, &request.secondName)`

The callback uses three stack arguments with callee cleanup; neither a `this` pointer nor another hidden user-data field is established. On the FileStore route these names are the resolved first name and original second name from `00be7cd0`. Adapter `00be7b20`, already audited, forwards them into FileStore completion `00be78b0`, which inserts the stream before invoking the caller's callback. Both name arguments are borrowed during the call; callers retaining them need their own copies.

The full 165-byte helper **`00befa40`** takes **ECX source-byte pointer; size-low and size-high on stack; EAX new memory stream; RET 8**. Assembly shows it uses the low size word only, allocates new backing through `008d43c0`, copies the bytes, wraps that backing through `00bef6d0` and releases its temporary backing reference. The high size word is not consumed. This independent copy is why a retained callback stream can outlive release of the asynchronous staging buffer.

After the callback, the pump releases its temporary stream reference. The raw tail omitted by Ghidra's current `_free` no-return interpretation then frees the original staging allocation and `OVERLAPPED`, closes the handle and removes the queue entry through `00bf4240` (`00bf4785..00bf47c2`). Removal is followed by rechecking the same index; pending entries alone advance the index.

Two material boundaries remain:

- `GetOverlappedResult`'s transferred-byte count is **not** compared with the original logical file size before copying that original low-DWORD size. Short reads or changing files can expose an uninitialized tail; no completeness guarantee is inferred from native acceptance or callback invocation.
- A failed completed I/O logs an error and performs physical-resource/queue cleanup **without invoking the callback**. Whether another path clears the corresponding FileStore pending entry is unverified. Do not invent a native failure callback or claim complete pending-state cleanup.

## Smallest next interface

The bounded next interface is a **submission operation plus an explicit completion pump**, with the two-name/callback/flags contract above, owned queued names and stable I/O allocations. Its acceptance result means “queued” on this physical path. It must preserve deferred callback timing even when the initial `ReadFile` succeeds immediately. Successful callbacks receive a separately backed stream; the provider releases its temporary reference after callback return.

The synchronous startup-preload integration is a separate path and does not supply this queue/pump contract. This audit does not establish a host async implementation, the outer tick scheduling caller, other providers' virtual `+0Ch` behavior, cancellation or complete failure semantics. Queue-growth/removal and callback reentrancy need focused validation before implementation.

Nine complete native code spans and selected table/string bytes matched saved Ghidra and current PE after target checks. Raw assembly supplied missing visitor bodies and the completion tail; no `_free` or other library symbol was renamed. There were no C++ builds, fixtures or original-game runtime tests in this read-only lane.
