# Scene-record map iterator increment

Addresses: `00467980`, `004681F0`, `004688D0`.

The complete raw iterator increment and both wrappers operate on actual eight-byte
`{owner,node}` storage. The implementation uses the existing concrete current CRT
invalid-parameter service. It adds no host callback, alternate tree owner, layout
overlay, null fallback, or success-only admission. Existing library-shaped names
are retained as descriptive hypotheses.

| Routine | Inclusive body | Bytes / instructions | Native ABI |
| --- | --- | --- | --- |
| `00467980` | `00467980..004679E2` | 99 / 38 | ECX actual iterator; no stack arguments; plain RET; no unified semantic EAX result |
| `004681F0` | `004681F0..004681FB` | 12 / 6 | ECX iterator; plain RET; EAX captured iterator |
| `004688D0` | `004688D0..004688DB` | 12 / 6 | ECX iterator; plain RET; EAX captured iterator |

This covers all 123 bytes and 50 instructions. Incoming EDX is not consumed.
ESI preserves the original iterator address. Source retains the observed ECX/RET
shape through MSVC Win32 `__fastcall`; it does not promise native flags, ambient
register values, return/fault addresses, private frames, or byte identity.

## Current storage and access schedule

Iterator owner is DWORD `+0`, node is DWORD `+4`. A node has left `+0`, parent
`+4`, right `+8`, and a BYTE nil flag at `+19h`. Raw DWORD address arithmetic
wraps at 32 bits. Every access reached requires valid storage. Node links need
not be distinct; Source preserves alias-sensitive accesses with volatile loads
and stores. There is no new nonnull-owner or nonnil-node precondition.

1. Read owner once. If zero, report at `00467988`; if the service returns,
   load the CURRENT iterator node at `0046798D`. Do not recheck owner.
2. If that captured node is nil, the POP-ESI/JMP at `00467997` reports and
   returns. A returning handler causes no later iterator store or link access.
3. Otherwise read captured node.right, then its nil byte. If nonnil, descend
   left: read candidate.left, check the child's nil byte, advance candidate and
   repeat. Publish only the final candidate at `004679BA`.
4. If the right child is nil, read captured node.parent. For each nonnil parent,
   reread CURRENT iterator.node at `004679C8`, then read parent.right and compare.
   On equality publish parent at `004679D0` BEFORE reading that parent's parent
   at `004679D5`. Repeat the nil check. On inequality or nil, publish the selected
   parent unconditionally at `004679DE`.

The final ascent store remains present even when it repeats an intermediate
store. Ordinary C++ node snapshots could erase the current-cell distinction when
the iterator and node storage overlap; this implementation retains those reads
and writes. Concurrent unsynchronized mutation and invalid raw storage are not
admitted by the C++ interface.

The core has no consistent native EAX result: right-subtree return leaves the
nil left child in EAX; ascent leaves the selected parent; the report tail leaves
the current service's ambient result. Source therefore exposes `void`. Each
wrapper captures the original iterator address in ESI, invokes the core at
`004681F3` or `004688D3`, then explicitly returns that captured address, including
when a returning handler has changed iterator contents.

## Concrete invalid-parameter policy and callers

Both reports bind existing `invoke_native_invalid_parameter_00bf6713()`. Its
complete Source entry uses the actual current SDK `_invalid_parameter_noinfo`
declaration, without `noexcept` or `noreturn`. The native 16-byte service has
XOR EAX, five zero pushes, a child transfer at `00BF671A`, ADD ESP,14h, and RET.
The Source's five zero words are unused padding for the real zero-argument SDK
API. No five-argument provider or callback is invented. Current handler return,
throw, termination, and side effects remain current CRT policy. A thrown C++
exception propagates; these routines add no cleanup or exception conversion.
Original encoded handler selection, debugger hook, Watson delivery, original
CRT globals and SEH transport are outside this qualified existing boundary.

Live references to the core are exactly six: both wrappers, and parent routines
`0046A470`, `0046B570`, `0046B680`, `0046BD10`. The four parent transfers are
at `0046A4EE`, `0046B609`, `0046B6F1`, `0046BD6A`. Their ECX inputs are stack
iterator addresses; all discard the ambient core return. The latter two reload
owner/node from the actual iterator after return. The wrappers have no live
references. Caller bodies and the existing provider remain outside this packet.

## Validation

Initial Source draft: full listing, live boundaries/bytes, six native caller
sites and concrete provider Source reviewed. Required strict MSVC Win32 build,
existing CTests, focused object review, report transfer verification, locked
annotation/save/export, and final metadata are pending. No new tests are added.
Packet-specific native differential and game validation have not run.
