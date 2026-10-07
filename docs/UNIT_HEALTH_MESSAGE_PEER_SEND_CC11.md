# Complete ordinary session peer selection for the health route

`NativeUnitHealthMessagePeerSendCalls` implements the formerly required
`00770B50` selection step in the accepted setter -> D2 constructor -> exact
flags4 route. It is an opt-in **abstract** adapter. The actual locked transport
operation `00783DC0` and all other inherited services remain required.
No concrete game host or network implementation is introduced.

The complete ordinary selector uses actual borrowed session fields. Its
protected common-header entry also accepts a real translated common message
and executable profile. The D2 override forwards the same borrowed message
base; it does not construct, clone, retain, rewrite or release a message.

## Native evidence and source contract

The full `00770B50..00770BC9` listing was reviewed. Native ABI is ECX=session,
stack target/message, RET8. These source member functions expose new C++
interfaces, not binary-compatible replacement entry points. The descriptive
name remains a hypothesis, not a recovered symbol.

| Native observation | Source behavior |
| --- | --- |
| `770B53`: session+188 | Capture the passed session's actual primary transport. |
| `770B5E`: unsigned primary+10 compared with zero | A zero count makes the local target null without reading the sentinel. Any nonzero unsigned count takes the list path. |
| `770B64..B72`: captured primary+0C, sentinel.next, conditional `BF6713`, node+8 | Capture that sentinel and first node. Preserve the reached violation service, then obtain node+8 directly as the local target. |
| `770B79..B7F`: compare actual target argument | Return when it equals the local target, including null==null on the zero-count branch. |
| `770B87/B8D`: reload session+188, call `783DC0` | A different target uses a fresh primary field observation, with the original target and message. |
| `770B97..B9E`: session+18C null gate | With initial primary null, return if the actual secondary is null. |
| `770BA4..BAF`: current message profile+0C(29), test AL | Invoke the actual executable query; a false result returns. |
| `770BB7/BBF`: reload session+18C, call `783DC0` | A true result uses a fresh secondary field observation. |

The primary list node's +8 payload is already the **local target**. It is not
the route's peer object, so there is no second +50 conversion. The selector
never rereads the current-game global, checks a session mode or substitutes
another session. Its caller already supplies the actual embedded session.

`NativeSessionPeerSendFields` borrows the actual +188/+18C cells, and
`NativeSessionPrimaryPeerFields` borrows the captured primary's +0C/+10
cells. The required binders return aliases without observing field values,
calling providers, allocating, changing ownership or changing FP controls.
All reached storage stays live for each ordinary call. A source profile's
query slot must be a valid translated entry with the declared source calling
convention. No profile table or list is fabricated by the selector.

The captured first node survives any returning iterator-violation service;
the source then reads its actual +8 just as the native listing does. The
service has no invented no-return, exception or repair policy. The fixture
covers valid lists only; invalid-storage faults and recovery are unproved.

## Required transport boundary

`send_locked_session_message_00783dc0` is pure virtual. Its **complete**
required behavior is supported by the full 22-instruction `783DC0..783DF8`
listing: capture target+4, Enter that lock, increment its +18 field, invoke
`783C80`, reload target+4, decrement that object's +18 field, then Leave the
reloaded lock. A replacement RAII lock would silently alter these observations.

The underlying serializer has actual delivery-indexed cursors, tick/prefix
writes, executable profile serialization, overflow handling and flush calls.
That work is not implemented or replaced with a success return here. Required
services must consume/serialize the caller frame synchronously or create a
separately owned representation. They must never retain or release the borrowed
setter frame. There is no default allocator, transport, stream, receive factory,
clone, critical section or queued borrowed-message pointer.

## Focused validation

One ignored local probe compiles the actual implementation translation units
with MSVC Win32 `/O2 /Gy /W4 /WX /fp:strict`, then executes them. The PE's actual
embedded RT_MANIFEST was parsed and verified as `asInvoker` (machine `014C`).
The probe completed **46 checks, zero failures**, covering seven direct cases:

- Nonempty primary list and the exact local target: no send.
- Zero count and null target: no sentinel dereference and no send.
- Zero count and remote target: use the actual primary transport.
- Null primary and null secondary: no send.
- Null primary, present secondary and the real D2 query29=false: no send.
- Present secondary and a real `0075B430` base message with type29: the actual
  equality profile returns true, and the actual type writer emits its byte.
- Unsigned count `FFFFFFFF` and the local target: take the nonempty-list path.

Every direct call passes game A's embedded session while the actual current-game
cell points to a different client-mode game B. The selected A transport and zero
mode-accessor calls verify that no global/session-mode substitution was added.
The type29 fixture establishes only common-header control flow and executable
query/writer behavior; it does not establish a concrete type29 payload/factory.

The connected case uses actual backing through game+207C, including the native
owner table, mode and embedded session transport cells. The real setter changes
health to50/max100, stores cache0.5 and byte128, constructs D2 on its borrowed
frame, traverses two actual route peer nodes, skips the primary's actual local
target, and sends once for the remote target. The required-send fixture checks
the same profile, owner, sender WORD and health payload, and synchronously calls
the actual D2 writer for the expected 29 bits. It retains no frame pointer.
This is a serialization/receipt fixture, not a native lock or network provider.

Generated `/FAs` assembly was inspected for both the generic protected entry
and the compiler-inlined D2 override. Both retain conditional primary binding,
zero-count sentinel suppression, direct node+8 comparison, the returning
iterator service, fresh primary/secondary reads, actual profile+0C(29), and
the original target/message arguments. No mode/global read was added.
The entire previously accepted implementation prefix is byte-identical.

Reproduce from the worker checkout with:

```text
cmd /c local\cc11_health_message_peer_send_check.cmd
python tools/verify_report_calls.py reports/unit_health_message_peer_send_cc11.json
```

The JSON report includes receipts and artifact SHA-256 values. Primary
integration owns metadata, Ghidra annotations, registration and the full build.
Original binary ABI, private EH, invalid-storage/fault behavior, real locking,
allocator/profile identity, networking and game integration remain unproved.

Primary integration: 5276f253521c5f6114f4b422e302ed64bb5b72b4; actual main sources independently recompiled for manifested focused probe, PASS46/0. Full MSVC Win32 Release and all three existing CTests passed. Executable SHA256 5b320d1e1ea1a1228618bfe206498ced1803ca5793511ddb20d62468890d400d. Complete locked serializer, original ABI, runtime binding and game validation remain unclaimed. Build receipt: J:\PROG\battlestations-pacific-decompile\local\cc11_peer_integrated_build.log.
