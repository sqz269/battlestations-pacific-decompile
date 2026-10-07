# Actual locked wrapper on the health message path

`NativeUnitHealthMessageLockedSendCalls` completes ordinary `00783DC0` in the
accepted setter -> D2 constructor -> exact flags4 route -> peer selector path.
It is an opt-in **abstract** adapter. The complete serializer `00783C80` and
the remaining inherited services are still required. The adapter uses actual
borrowed target/lock fields and real Win32 Enter/Leave operations.

## Evidence and exact sequence

The complete 22-instruction native body is `00783DC0..00783DF8` (exclusive end).
Its ABI is ECX=transport, stack target/message, RET8. The new member interface
is source code, not a binary-compatible replacement. The descriptive source
name is a hypothesis, not a recovered symbol.

The canonical `TrackedCriticalSection` from `random_threads.hpp` is 1Ch bytes:
the actual Win32 `CRITICAL_SECTION` starts at +0 and the tracked DWORD is +18.
The wrapper statically asserts those offsets and size. It accesses the signed
counter representation through its corresponding unsigned volatile type, so
increment/decrement preserve modulo-32-bit behavior without signed overflow.

The installed original PE was inspected directly. IAT `00CE2218` imports
`KERNEL32.dll!EnterCriticalSection`; `00CE2210` imports
`KERNEL32.dll!LeaveCriticalSection`. Its image base is `00400000`, machine014C.

| Native site | Source operation |
| --- | --- |
| `783DC3/DC7` | Preserve the actual target argument and capture its +4 lock pointer. |
| `783DCD` | Enter the captured lock through the real Win32 API. |
| `783DD7` | Increment the captured lock's actual +18 DWORD. |
| `783DDF` | Invoke required complete `783C80` with the unchanged transport, target and same borrowed message. |
| `783DE4` | Reload the actual target+4 field after the serializer returns. |
| `783DE7` | Decrement that reloaded lock's actual +18 DWORD. |
| `783DEC` | Leave that reloaded lock through the real Win32 API. |

The required target-field accessor returns an alias to the actual cell. It
performs no early value read, callback, allocation, ownership change or FP
change. There is no null guard or substitute lock. Every reached lock is live
and initialized, and every reached Leave requires valid current-thread
ownership. The source does not cache the entered lock for exit or repair an
imbalance caused by a provider changing the actual cell.

There is no RAII or cleanup-on-throw path. This is a complete **ordinary
returning** sequence; native private EH, fault behavior, invalid ownership,
concurrent unsynchronized mutation and binary ABI are outside the claim.

## Required serializer boundary

`serialize_session_message_00783c80` remains pure virtual. Its required complete
operation includes actual delivery-indexed cursors, tick/prefix writes, the
current executable profile writer, overflow copy/rewind/flush/restore, and
threshold/delivery flush effects. The implementation provides no D2-only
serializer, transport-success stub, synthetic stream, clone or receive factory.

The same captured transport and target are passed through unchanged; there is
no game/global-session lookup or new mode gate. Required services must consume
the caller frame synchronously or create a separately owned representation.
They may not retain or release the borrowed setter frame.

## Focused source fixture

One ignored probe compiles the actual implementation translation units with
MSVC Win32 `/O2 /Gy /W4 /WX /fp:strict` and `/MANIFEST:EMBED`. Its embedded
RT_MANIFEST was parsed as `asInvoker`; its actual PE imports include Win32
Enter, Leave and TryEnter. The final run passed **29 checks, zero failures**.

The unchanged-lock case uses a real initialized critical section, an actual D2
constructor/profile, and the real peer selector. Counter bits start atFFFFFFFF,
are observed as zero inside the required serializer fixture, and return to
FFFFFFFF after the wrapper. The OS recursion count is one during serialization
and zero after return. The actual D2 writer runs synchronously and the message
bytes remain unchanged.

The connected case executes the real health setter, constructor, flags4 router,
peer selector and locked wrapper. Two route peers exclude the actual local
target and send once to the remote target. Lock B is explicitly pre-owned by
the fixture thread with tracked depth1; the wrapper enters A and increments
A's depth to1. The required serializer **source fixture** checks the same
transport, target, D2 profile, borrowed owner, sender and health byte128, and
calls the actual D2 writer for the expected 29 bits. It then changes the actual
target+4 cell from A to B while both remain live.

After the wrapper returns, A remains at tracked depth1 and B is at0. One joined
Win32 observer thread cannot TryEnter A but can TryEnter B, then releases B.
This distinguishes the native fresh-field exit from a cached-lock/RAII exit.
The external fixture driver subsequently balances A and destroys both locks.
Every Enter/Leave has valid ownership; the implementation performs no repair.
No pointer to the borrowed message survives the serializer fixture call.

The controlled field replacement is source-provider behavior, not evidence
that the native serializer performs such a mutation. The synchronous writer
is a fixture for the required serializer boundary, not a partial production
implementation of `783C80`. Actual native serialization, locking integration,
network effects, allocator/profile identity and game behavior remain unproved.

Generated `/FAs` assembly confirms the real imported Enter, captured counter
increment before the required virtual call, a fresh load from the same actual
target cell afterward, reloaded counter decrement, and real imported Leave.
All previous implementation bytes are unchanged after excluding the newly
required `random_threads.hpp` include. No tracked tests were added.

Reproduce from the worker checkout:

```text
cmd /c local\cc11_health_message_locked_send_check.cmd
python tools/verify_report_calls.py reports/unit_health_message_locked_send_cc11.json
```

The JSON report records the direct-call/IAT evidence and artifact SHA-256 values.
Primary integration owns the full repository build, metadata and annotations.
