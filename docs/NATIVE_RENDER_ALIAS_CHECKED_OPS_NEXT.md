# Native renderer alias checked-operation boundary

The smallest complete next implementation packet is the three checked iterator
helpers and node erasure: `004BE820`, `004BECC0`, `004B9FF0`, and `004D0990`.
Their storage, mutation order, returning validation behavior, and allocator
dependencies are established. Count growth `004CE780`, range insertion
`004D26A0`, and record assignment `00B30510` remain separate: their exception
path needs the legacy small-buffer string and `std::length_error` machinery.
The name on `00408720` did not establish a concrete implementation in checkout
`5edfecc`; it has a naming record but no reconstructed-function record or matching
production entry. This discovery makes no C++ or Ghidra changes.

## Ready checked operations

The actual list owner is a 0Ch region at resource record `+8`: untouched DWORD
`+0`, sentinel pointer `+4`, and unsigned count `+8`. A checked iterator is two
DWORDs: raw owner identity `+0` and actual node pointer `+4`. Each 10h node is the
existing `NativeRenderResourceAliasNode`, with next, previous, string length,
and string data at `0/4/8/C`. Bind those actual words and owner identities;
do not copy them into a host container or reinterpret a separate wrapper's
address as the native owner token.

| Entry / complete extent | Original ABI | Required behavior |
| --- | --- | --- |
| `004BE820..004BE848`, 41 bytes | ECX left iterator address; stack right iterator address; AL inequality; `RET 4` | Validate owners, then compare current node pointers |
| `004BECC0..004BECE5`, 38 bytes | ECX iterator address; EAX same address; `RET` | Publish previous node before validating the resulting position |
| `004B9FF0..004BA017`, 40 bytes | ECX iterator address; EAX same address; `RET` | Validate current position, then reload and advance node |
| `004D0990..004D0A0A`, 123 bytes | ECX destination owner; stack output-iterator address, input owner, input node; EAX output address; `RET 0Ch` | Unlink/free node, decrement current destination count, publish next iterator |

`004BE820` captures the two iterator addresses. It reads the left owner once;
null or inequality with the right owner calls `00BF6713`. After a returning
handler it reads both current node words and compares them. It does not repeat
the owner check, initialize an iterator, or report a fixed result for invalid
owners. Only AL is the native boolean result; upper EAX bits are incidental.

`004BECC0` first checks the current owner for null. After any returning handler,
it reads the current node, captures `node->previous`, reads the current owner,
and publishes the captured previous pointer to the iterator. It then compares
that captured pointer against the captured owner's current sentinel. Equality
calls the handler **after publication**. A repair made by that second handler
is retained; the function subsequently only returns the iterator address.

`004B9FF0` checks the current owner for null, then reloads owner and node for the
sentinel check. After either returning validation call it reaches a final reload
of the iterator node, reads that node's next pointer, and publishes it. Caching
the node before the second handler changes observable repair behavior.

`004D0990` takes the input iterator by value. It captures input owner, input node,
and destination owner before validation. The null-owner handler cannot replace
that captured owner by modifying the caller's iterator; native execution still
dereferences the captured owner if the handler returns. It next compares the
captured input node against that owner's current sentinel and may call the
handler again. There is **no input-owner/destination-owner equality check**.

After validation, erasure compares the captured node against the destination's
current sentinel and captures the node's next pointer even on the sentinel
branch. If they are equal, it skips unlinking, freeing, and decrementing. On the
other branch it writes `node->previous->next = captured_next`, then reloads the
node's next and previous pointers for `node->next->previous = node->previous`.
It captures nonnull string data and length-plus-one for the existing actual
`SizedStoragePool` return, then frees the captured node through the existing
singleton lifetime free boundary. Only **after the free** does it reload and
decrement destination count with unsigned wrapping. It writes output node
before output owner and returns the output address. The returned owner is the
captured input owner, even when it differs from the destination.

Use the existing required callback contract in
`SingletonLifetimeCallbacks::invalid_parameter(context)`, already exercised by
`src/singleton_lifetime.cpp:119` and its validators. The shared manager's helper
is private, so a new operation should receive the same existing callback binding
explicitly rather than constructing a second manager or a new CRT policy.
Keep a required binding; do not supply a no-op, unconditional abort, synthetic
sentinel, or valid-input-only branch. A handler that returns without repairing
data may leave the subsequent native dereference invalid. That does not authorize
additional checks, a safe default result, or skipping the original continuation.

## Invalid-parameter and length-error paths

`00BF6713` is a 16-byte no-information wrapper: it calls `00BF66EF` with five
zero DWORDs and returns. `00BF66EF` reads encoded handler slot `0109DD64`, calls
the current decode-pointer wrapper `00C04FDE`, and tail-jumps to a nonnull
decoded handler with the original five arguments. This preserves a returning
installed handler. Without one, it calls `00C04EF3(2)` and tail-jumps to the real
`__invoke_watson` at `00BF65BB`. That default path builds status `C000000D` and
calls `TerminateProcess`; it is not evidence that every invalid-parameter call
is non-returning. `00C04EF3` currently only clears `0109EEA8` and returns.

`004CE780..004CE812` is 147 bytes, ECX list owner, stack unsigned increment,
`RET 4`, with no defined result. It captures count, computes
`uint32(1FFFFFFFh - count)`, and throws when that wrapped difference is less
than increment. Otherwise it publishes the captured count plus increment.
Do not strengthen this into `count <= max`: a pre-corrupted count above the
limit can pass because the subtraction wraps. The routine never allocates or
links a list node and leaves count unchanged on its exception branch.

That exception branch creates a **1Ch-byte legacy character string**, separate
from the renderer's eight-byte pooled strings. Its buffer/pointer union is at
`+4`, size at `+14h`, and capacity at `+18h`; capacity below 16 selects inline
storage. It initializes an empty capacity-15 temporary and calls `00408720`
with literal `00CE38F8`, exactly 16 characters: `list<T> too long`. For this
fresh destination, reserve normally requests 32 bytes; if that allocation throws,
its catch retries the exact requested length plus one, 17 bytes.

`00411700..0041175E` then constructs a 28h-byte `std::logic_error` object:
`00BF632F` initializes the 0Ch-byte `std::exception` base, the constructor writes
logic-error vtable `00D69248`, initializes a second empty 1Ch-byte string at
object `+0Ch`, and calls `00408120(source, 0, FFFFFFFFh)` to copy the temporary.
This is a second message allocation. `004CE780` finally replaces the vtable
with length-error vtable `00D69260` and calls `00BF6885(object, 00D83F98)`.

The literal temporary becomes state-0 unwind-owned only after its counted
assignment returns. FuncInfo `00D8E3D0`, unwind map `00D8E3C8`, and funclet
`00C65A80` route that cleanup to `004072D0`. The logic-error constructor's own
state-0 map `00D83F6C` and funclet `00C5E010` clean the completed exception base
through `00BF6454` if embedded-message assignment throws. The reserve helper
has its own failure cleanup; the count helper does not substitute a generic
abort or return a failure status.

ThrowInfo `00D83F98` names destructor `00411780` and a three-entry catchable-type
array `00D83FD4`: native `std::length_error` and `std::logic_error` objects are
40 bytes, and their `std::exception` base is 12 bytes. Type names at
`00E08000/00E08020/00E174F0` establish the identities. Copy functions are
`00411940` for length error, `004118D0` for logic error, and the recognized
`std::exception` copy at `00BF63A6`. `00411940` has no Ghidra function definition
in the inspected analysis. `00411780` releases the message, resets its fields,
and tail-calls the base destructor; `00411550` returns the current message data.

`00BF6885` is the existing recognized CRT `__CxxThrowException@8`; preserve that
library identity. Its template at `00D693B0` carries exception code `E06D7363`,
noncontinuable flag 1, three parameters, and magic `19930520`. It supplies the
object and ThrowInfo pointers to `RaiseException`; ThrowInfo flag 8 changes the
magic to `01994000`. Calling it with `(0,0)` is the native rethrow operation used
by the catch bodies here. The typed reconstruction may use C++ exception
mechanics at an explicit ABI boundary, but a generic host exception alone does
not prove the native message allocation, copy, cleanup, or exception ABI.

## Remaining closure and body corrections

The error-construction follow-up must account for complete `00408720` counted
assignment, `00408120` substring assignment, `004072D0` destruction, reserve
`004089E0`, allocation `00408B60`, and the exception constructors/copies/destructor
above. Full string assignment additionally reaches `004087F0` self-erasure,
`00BF5695` length error, `00BF56D4` range error, actual `operator_new`, and
`memcpy_s` boundaries. Their names are not production readiness evidence.
Existing `NativeString::resize_0041dd40` is the different eight-byte pooled
representation; it cannot stand in for these 1Ch-byte objects.

The complete reserve body is `004089E0..00408B0F`, 304 bytes. Saved analysis
incorrectly treats `00408A78` as another function and ends the entry before its
first catch. Catch `00408A50` retries exact-size allocation and returns to
`00408A72`; continuation `00408A78` copies preserved data, releases the old heap
buffer, publishes the new buffer/capacity/size, terminates it, and `RET 8`.
Catch `00408AE0` releases the old heap buffer, resets the original destination
to empty inline storage, and rethrows. FuncInfo `00E049B8` describes both catches.

False `_free` no-return analysis also hides `004D09F1` stack cleanup and
`004D09F4` count decrement, `004072E2` stack cleanup before the SBO destructor's
field reset, and `00411798` stack cleanup before the exception destructor's
field reset/base destruction. The deleting destructor `004117C0..00411806`
also has post-free continuation, optional object free, and `RET 4`; its
decompiler's early returns are wrong. All bytes were inspected independently.

`004D26A0..004D27B3`, 276 bytes, takes ECX destination owner and seven stack
DWORDs: position owner/node, first owner/node, last owner/node, and one unread
word. It returns no semantic value and `RET 1Ch`. It saves the original first
iterator, validates the current range, allocates/copies a node through
`004CE6F0`, checks/increments count through `004CE780`, links the node, validates
again, then advances the source cursor. The node allocation/copy catch does
**not** cover the subsequent count check.

The real catch body at `004D2746` is absent from the main decompiler view.
FuncInfo `00D8EB30` and try map `00D8EB1C` identify an ellipsis catch at that
address. It compares original first against the advanced current first, walks
backward from the insertion position, erases through `004D0990`, advances the
saved source iterator, and repeats until those source iterators compare equal;
then it rethrows. Rollback is driven by **cursor advancement**, not simply every
allocation or link. A count failure leaves the fresh unlinked node outside this
cleanup; a handler exception after linking but before source advancement
can also leave that latest linked insertion outside the rollback count. Do not
introduce a broader RAII rollback while claiming the original behavior.

`00B30510..00B305B1`, 162 bytes, takes ECX destination record and stack source,
returns destination, and `RET 4`. It performs pooled name assignment first,
captures source sentinel and first node before clearing destination aliases,
then calls the range helper. Only after that returns does it copy the six live
payload/resource DWORDs at `+14h..+28h`, one read/store pair at a time. It has no
local EH handler and no rollback of the already-copied name. Its source/payload
reloads and actual raw-string binding still need review when that larger packet
is implemented; a `std::vector` or copied `NativeString` owner is insufficient.

## Handoff and validation

Implement the four ready checked operations with the existing real node type,
actual list storage, `SizedStoragePool`, `singleton_lifetime_free`, and a required
shared invalid-parameter callback binding. Keep node construction
`0044BCB0/004CE6F0` with its separate worker. A later packet can close the SBO and
length-error machinery before claiming full range insertion or record assignment.
Suggested checked-operation files are `include/bsp/native_render_alias_checked_ops.hpp`,
`src/native_render_alias_checked_ops.cpp`, and their matching doc/audit pair.

Every byte-read batch verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All 33 code spans and 14 data spans matched the
installed executable or its PE zero-fill; the audit records extents, SHA-256,
current names/comments, native ABIs, body corrections, and remaining dependencies.
This is read-only reconstruction discovery. No C++ changes, tests, Ghidra
mutations, native execution, ABI replacement, or gameplay validation occurred.
For the implementation packet, one focused existing-fixture extension can check
returning-handler repairs, previous-before-validation publication, and erase
capture/free/decrement order; a broad new test suite is unnecessary.
