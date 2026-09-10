# Native render queue owner: lifetime boundary and next row packet

The complete actual `34h` queue owner is not yet a closed implementation
packet. Its constructor can stop the renderer based on the **existing** word
at `+20h`, and its ordinary destructor executes queued commands before freeing
storage. Those are native behavior, including on unusual preimages; initializing
that word first or substituting a destruction-only drain would change the body.

The next ready independent packet is the complete `20`-byte row family:
`00B1DB30` reserve, `00B1E7F0` resize and `00B1F150` destruction. The newly
integrated actual-header `0041DD40` closes its string-copy dependency. This
discovery changes only this document and its [audit](../reports/native_render_queue_owner_next.json).
It adds no implementation, Ghidra annotation, ledger record or runtime claim.

## Current interfaces and actual storage

The discovery checkout is `3502565`; the string dependency is read separately
from integrated commit `719dd35`. Exact commit IDs, source hashes, original
annotations and native byte hashes are in the audit. Existing command, context
and pointer-array interfaces were checked against their current files.

| Actual queue offset | Storage and consequence |
| --- | --- |
| `00` | Native table word; construction/ordinary destruction writes `00D5E5F4`, final base cleanup writes `00CE3818`. |
| `04..13` | Two eight-byte configurations: enabled byte, three preserved padding bytes, value DWORD. |
| `14,18,1C` | Actual command-pointer header: data, signed count, signed capacity. |
| `20` | Existing control DWORD read before constructor initialization. |
| `24,28,2C` | Actual row header: data, signed count, signed capacity. Each row is `14h` bytes. |
| `30` | Actual current context pointer; no independent destructor cleanup follows queue execution. |

`NativeRenderCommandQueueStorage` in `native_render_queue_access.hpp` already
has these offsets, no automatic initialization and no destructor. Its field
helpers and actual command append/storage helpers do not implement queue
acquisition or lifetime. Actual `44h` command and `18h` context lifetime/refcount
services are present. The separate `RenderCommandQueue` host projection has
host command dispatch/deletion and is not the actual `34h` owner.

## Getter, construction and destruction

`004C11F0` is a no-argument getter returning EAX (`RET`). The fast path returns
the captured nonnull `00F8D440`. On the slow path it calls manager getter
`00415350`, captures that manager's `+10h` optional critical section, enters
it, then increments the captured section's recursion word at `+18h`. After
arming the guard it rechecks the global. If still null, `00BF681B` allocates
`34h`; a nonnull allocation enters `00B1F280`, and null becomes a null result.
It disarms raw-allocation cleanup, publishes `00F8D440`, calls `00415350`
**again**, then reloads `00F8D440` for `00BD0C30` registration, including null.
It decrements the captured section before leaving it and returns a fresh global
read after leave. Registration therefore uses the global *after* the second
getter, unlike the earlier preparation-job getter's captured argument.

Getter EH states are `0` optional-guard cleanup and `1` raw allocation cleanup
followed by the guard. Constructor failure first runs the constructor's own
member/base unwind, then the getter frees its raw allocation. Allocation
failure leaves only the guard. A registration exception retains the already
published queue; it does not destroy or unpublish it. The actions are
`00C64F00` guard via `00411EE0` and `00C64F08` raw `00BF65AC` free.

`00B1F280` is ECX actual queue, EAX original queue, `RET`. It writes the queue
table, clears the two enabled bytes/values, initializes command and row headers
to zero, then compares **untouched** `queue+20h` to `2`. It arms both member
cleanups and clears `context+30h` before taking the branch. If equal, it loads
the current renderer global `00F8D394`, its current table, and virtual slot
`+11Ch`. After that call it writes control zero, clears both configurations
again, then writes value zero/enabled one for each. Padding `05..07` and
`0D..0F` is preserved throughout. Callback changes to command/row headers or
context survive: those fields are not cleared a second time.

Constructor EH map `00DF50CC` contains base cleanup, command-header destructor
and row-header destructor, with states `0`, `1`, `2`. An exception in the
renderer call unwinds rows, commands, then base. `00B1C3C0` base cleanup first
clears current `00F8D440`, unconditionally, then writes table `00CE3818`.
Thus failed construction can clear the singleton even though the getter has
not yet published this allocation. A `this == global` guard would be invented.

`00B1F330` is ECX queue, `RET`, and has a real continuation beyond Ghidra's
current false no-return boundary at the first `00BF6989`. It writes the queue
table, arms state 2, calls **`00B1EBE0` queue execution**, changes state to 1,
resizes the current row header to zero, reloads row data and array-frees it.
It changes state to 0, resizes the current command header to zero using
`00B1CC80`, reloads command data and array-frees it. Finally it clears
`00F8D440`, writes base table `00CE3818`, restores SEH and returns. The complete
native span is `00B1F330..00B1F3BA` exclusive, 138 bytes; the listing alone
ends at `00B1F379`. There is no separate release of `queue+30h`.

Its EH map `00DF5108` parallels constructor member/base cleanup. An exception
from execution runs row, command-header and base cleanup without retrying
execution or destroying command objects independently. An exception after
state changes to 1 does not re-enter row destruction. `00B1F6B0` calls this
ordinary destructor, frees the original pointer with `00BF65AC` only when
flags bit 0 is set, returns the original pointer in EAX, and uses `RET 4`.
The primary table's first DWORD at `00D5E5F4` is exactly `00B1F6B0`; adjacent
tables are not additional queue virtual slots.

## Why execution and renderer stop remain dependencies

`00B1EBE0` reads live signed queue count and command cells. For each command it
compares the command's context at `+28h` with current queue context. When
different it publishes the new pointer before retaining its actual `+4h`
counter and releasing the old pointer through its current terminal table.
It reloads the command cell and calls command virtual `+0` (current command
profile resolves to unimplemented execution `00B1D950`). Afterward it reloads
current queue context, releases that captured nonnull context, then writes
null **after** the terminal callback. It next reloads control; zero reloads
the command cell and, if nonnull, calls ordinary destruction `00B1DDD0` and
frees that command. The loop and
final control decision reload actual queue fields. Only final control zero
resizes the command header to zero. Nonzero control retains commands; an empty
queue leaves a pre-existing nonnull context alone. Existing context-reference
and command-lifetime bodies close these lifetime operations but not execution.

The renderer profile rooted at `00D5F0A8` has `00B28A90` in slot `+11Ch`
(`00D5F1C4`). The full stop body captures the renderer, optionally calls
`00B33BF0` on `renderer+1970h`, disables both synchronization globals through
`00B33AA0(false)`, calls `00B26920` on the captured renderer and finally
`Sleep(100)`. It does not clear the worker pointer or join a thread.

`00B33BF0` clears worker byte `+4h`, reloads its acknowledgement owner at `+10h`
and tailcalls that owner's current table `+8h`. The verified event profile
`00D6821C` resolves that slot to `00BD17C0`: wait on actual handle `+4h` with
`INFINITE`. The existing `Win32Event` host class is not that polymorphic native
event owner, and the concrete worker constructor `00B33DA0` installs separate
wake/ack owners, a suspended thread, priority and resume behavior. The observed
stop acknowledgment is not thread termination. No worker/thread owner is
introduced in this packet.

`00B26920` calls the renderer's **current** table each iteration: `+130h` for
20 null sampler bindings, `+134h` for four null vertex streams, then `+138h`
for null index stream/base zero. The observed slots resolve to `00B24710`,
`00B24840`, `00B24B00`. It then invokes `00B241C0` on actual cache subobject
`renderer+34h`, reloads default color wrapper `+197Ch` for `00B23D80(slot 0)`,
and performs device virtual `+9Ch` with null (depth-stencil surface), with
the native optional-guard checks and device reload. `00B241C0` releases raw
intrusive cached references and clears validity/storage fields. Existing
`D3D9StateCache::invalidate()` and semantic shared-pointer binding helpers do
not supply that actual cache owner/reset; partial native renderer parameters
also do not provide a full renderer. This exact missing primitive plus real
worker/event ownership prevents treating the constructor branch as closed.

## Ready packet: complete actual 20-byte row family

Use a trivial raw row with unsigned length at `+00`, character pointer at `+04`
and three float words at `+08`, `+0C`, `+10`. The header is exactly three native
DWORDs: data, signed count, signed capacity. Operate on the queue's existing
`+24h` header address, or an explicitly identical standalone raw header;
do not create a shadow vector or auto-constructed `NativeString` array.

`00B1DB30` takes ECX header and signed requested capacity, `RET 4`. It clamps
request to at least one and grows only. Allocate `requested * 20` raw bytes
through the existing `00BF55BE` allocation boundary. For each row, using the
live old count, compute a replacement slot. If its computed address is nonnull,
capture the current old row pointer, zero only destination length/data and
skip string copy if source and destination addresses compare equal. Otherwise
call `resize_native_string_header_0041dd40(destination, storage, source_length,
true)`. After that call reread captured source length; if nonzero copy using
the **current destination length**, current source data and destination data.
Then execute three separate ordered `FLD m32`/`FSTP m32` pairs, offsets
`08`, `0C`, `10`. Preserve x87 status/quieting behavior; raw DWORD copying or
SSE bit moves are not equivalent for signaling NaNs. For normal input these
are float copies, but the evidence includes the actual instructions.

After copying, destroy old strings **forward**, reloading current old header
data/count on each iteration. Capture each nonnull data pointer, read its
length plus one, then perform `00419CC0(data,size,1)` and `00BD1510`; do not
clear the old row words. Array-free the then-current old header data with
`00BF6989`; only after free returns publish replacement data and capacity.
Header count is not assigned by reserve. The omitted listing continuation
`00B1DC49..00B1DC53` contains these publication stores and was checked as bytes.

Reserve's single EH action `00CBCA80` computes the constructed-end pointer
and passes it plus the current destination to `00401130`. The **entire
verified helper is byte `C3` (`RET`)**. An exception while copying therefore
does not free the replacement allocation or previously constructed strings.
Do not add RAII rollback or destructors. Any effects already made by allocation
or string callbacks remain observable. This is a native quirk, not an inferred
missing cleanup implementation.

`00B1E7F0` takes ECX header and signed count, `RET 4`. If requested count exceeds
capacity, reserve first, then reload old count. Growth clears only the first
eight bytes of each computed nonnull new row, reloading header data for each
slot; float preimages survive. Header count is not incremented during growth.
Shrink repeatedly decrements actual count **before** computing/capturing the
current last row, releases a nonnull string using length plus one, then rereads
count for the next comparison. A callback can change count/data. Freed words
are not cleared. One final store sets requested count.

`00B1F150` takes ECX header, `RET`: resize zero, reload current data, array-free
it, and return. Count is zero but data and capacity remain dangling/unchanged.
The full body is 23 bytes including `00B1F162..00B1F167`, omitted after the
currently misannotated no-return free call. Do not reset the header after free.

The actual string service was revalidated at integrated `719dd35`:
`resize_native_string_header_0041dd40(void*, NativeStringStorage&, uint32_t,
bool)` operates on the supplied header with callback-sensitive reads. Its
existing explicit `NativeStringStorage` boundary replaces native pool-getter
side effects, has a `noexcept` release and omits a native zero-byte memcpy.
Those limitations must remain explicit in the row implementation. Shared
`singleton_lifetime_allocate/free` already provides the raw allocation/free
host boundary; request matching native/host 20-byte storage, without a new
queue ownership policy. Use valid nonnegative header count/capacity and suitable
allocated spans; preserve native arithmetic/branches without inventing corrupt
header repair. x87 exceptions/status and reserve failure are concrete risks
for one focused original-byte differential check.

Recommended ownership: addresses `00B1DB30`, `00B1E7F0`, `00B1F150`; new files
`include/bsp/native_render_queue_rows.hpp`, `src/native_render_queue_rows.cpp`,
`docs/NATIVE_RENDER_QUEUE_ROWS.md`, `reports/native_render_queue_rows_audit.json`.
Primary integration owns CMake/ledgers/Ghidra changes. Full row entries can be
implemented independently of queue getter, scheduler, worker, renderer and
command execution. As a separate small packet, `00B1C3C0` base cleanup and
`00B1D590` command-header destructor are also behaviorally closed using current
`00B1CC80`; they do not replace the selected row-family packet or close queue
ordinary destruction.

## Evidence limits

The audit verifies 33 live native spans, 2,219 bytes, against the installed
executable. Every live query passes the repository's project/program guard.
It includes constructor/destructor/getter EH actions/maps, row unwind, table
slots, omitted free-call continuations and the empty helper. Source hashes
pin 22 files across the discovery and string-integration revisions. Native
bytes and stored prototypes are evidence; many stored prototypes are still
`undefined ...(void)` and do not express the assembly-derived ABI above.

This is source/assembly discovery with installed-byte equality. No source was
reconstructed, no build or fixture was run for this report, and no game,
renderer, concurrency or drop-in ABI behavior was validated. The prior string
packet's build/fixture results remain that packet's evidence, not new queue
validation. Integrators must preserve captured old names/comments when later
annotating the ready entries and repair the demonstrated no-return listing
gaps only under the repository's Ghidra write lock.
