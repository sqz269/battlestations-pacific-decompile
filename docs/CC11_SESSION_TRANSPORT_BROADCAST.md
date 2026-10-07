# Session transport broadcast

Packet `cc11_health_transport_broadcast` reconstructs the whole
`[00784830,0078489F)` body: **111 bytes, 43 instructions**. Original ECX is the
transport, one stack argument is the borrowed message, and the body returns
with `RET 4`. Live Ghidra and disk bytes match, including the incoming CALL
at `0076C529` in `0076C500`. The descriptive broadcast name is a hypothesis.

The new member is
`NativeUnitHealthMessageSerializedCalls::broadcast_session_message_00784830`.
It uses the existing pure `bind_peer_send_primary` and
`target_critical_section_04` bindings, then directly calls the unchanged,
complete, private final `serialize_session_message_00783c80`. It adds no
transport profile, serializer override, queue callback or clock service.

## Complete traversal

The native body captures transport `+0C` and its first node, then rereads the
sentinel at each loop end test. It includes the first node; an empty list returns
without touching a target. Transport `+10` count is never read. Before accessing
node `+08` target it compares the node with another fresh sentinel read, calling
`00BF6713` if equal and retaining the return continuation. The initial native
owner comparison is literally `CMP EBP,EBP`, so its first CRT call is unreachable.
Source omits that proved dead comparison while preserving both live guards.

For each node, it captures the target pointer and target `+04` lock, calls real
`EnterCriticalSection`, increments raw depth `+18` modulo 32 bits, and calls the
complete serializer. It then reloads target `+04`, decrements that current
lock's depth modulo 32 bits and calls `LeaveCriticalSection`. A fresh sentinel
validation follows before reading node `+00` to advance. Later nodes may alias
an earlier target; each node still serializes once. No count shortcut or local
target exclusion is added. There is no RAII cleanup on exceptions, matching the
ordinary native sequence.

The message remains synchronously borrowed: serialization may consume it or
retain an owned representation, and must not retain/release the message itself.
The reached finite list, targets, actual locks and provider backing must stay
live. Current SDK `_invalid_parameter_noinfo` is a fixed may-return service;
historical `00BF6713` globals, Watson, private fault and EH behavior remain open.

## Connected fixture and its boundary

The ignored fixture is `local/cc11_transport_broadcast_20261007_a/probe.cpp`.
It compares the complete copied **original outer loop** against the new Source
loop, both connected to the existing complete Source serializer and providers.
This is not an all-original serializer graph or a native class replacement.

Only six four-byte original code operands are relocated: three rel32 CRT calls
at offsets `19`, `27`, `5F`; real OS IAT addresses at `34` and `55`; and the
serializer CALL at `47`. The other **87 bytes are unchanged**. Real Enter/Leave
addresses occupy fixed IAT cells on the fixture page, which becomes executable
readonly after setup. No transport profile is rewritten. Actual readonly PE
host `D243EC` and clock `D68D50` words feed the existing Source admission rules.

The serializer relocation reaches a fixed 23-byte ABI bridge. It passes native
ECX transport and stack target/message to the live concrete Source adapter.
The fixture exposes the inherited protected declaration with C++ `using`;
the production final serializer remains private. COFF proves the generated
Source vftable's `+40` slot is that same complete final serializer. This is a
compiler-generated Source class table, not an invented native transport table.
The bridge adds no serializer override, partial implementation, recording hook
or list mutation. The new Source member itself calls the complete serializer
directly.

Each world genuinely constructs three `D74` target bases, their real OS locks,
nine allocated cursor objects and six 50-/3-sample histories. Slots 0/1/2 have
mask 7; complete target destruction returns it to zero. All three `464`-byte
payload slices per target are **inline** in target storage; separate snapshots
cover the allocated cursor objects. Borrowed sockaddr bytes follow each target
base at `+D74`; no derived target/transport constructor is claimed.

The existing complete fixed-clock provider uses actual clock storage with its
fixed flag and native profile tokens. The existing network-owner backing has
real SDK locks/socket/conversions, 16 genuinely constructed channel members and
three genuinely constructed queue records. Its full owner/transport lifetime is
not reconstructed here. There is no send/connect or recording SDK hook.

Five cases cover empty list/count 77, one peer/count zero, three peers/count one
with nonempty buffers, a later node aliasing the first target, and three peers
whose real D2 writes reach the reliable flush threshold. The last case reaches
the complete current clock, host `+20`/`+24`, owned enqueue and post-return cursor
reset. All three owned payloads survive a later overwrite of borrowed inline
storage. Exact cursor positions prove visit counts; payload oracles and full
target snapshots cover serialization/reset changes. Histories, all 53 samples
per target, sentinel/nodes and the borrowed message remain exact; real lock
depths balance. Owned queue order, metadata, counters and sequences agree.

Final result: **698 checks, zero failures, five original-loop cases**.
The native-loop comparison establishes the new loop and its composition with
the existing providers; it does not independently re-prove every provider's
original machine code, reentrant lock replacement, failure paths or networking.

## Build and evidence

Strict MSVC Win32 `19.51.36244.0` compiled **24 repository translation units plus
the probe: 25 compilation units total**. The immutable final snapshot contains
69 inputs: those 25 units and 44 headers. It uses `/O2 /Gy /W4 /WX /fp:strict
/MD /EHsc`, an embedded `asInvoker` manifest and `/OPT:REF`. Three readonly
support libraries were frozen from the successful main build at
`b02e3154636139d964b1e32f0f91b710dd71985e` with pre/copy/post hash equality,
before allowing another main rebuild. Final input, library and native-image
hash checks pass. An initial compile only needed its lock-definition include
corrected to `random_threads.hpp`; its failed snapshot is retained separately.

Full Source COFF review covers the 163-byte/63-instruction broadcast, its
six-byte fixed CRT IAT bridge, the 23-byte serializer ABI bridge and pure
22-/10-byte field binders. It proves first-node inclusion, unread count,
fresh sentinel tests, direct complete serializer call and fresh lock reload.
Source ECX is the adapter, two stack arguments are transport/message, and it
returns `RET 8`: a new Source interface, not the original ECX transport ABI.

Five direct static rows pass verification: incoming `0076C529`, the three CRT
sites (one unreachable), and serializer call `00784876`. Enter/Leave IAT sites
are separately recorded as actual SDK imports. Exact native/Source/bridge bytes,
relocations, hashes and build inputs are in the JSON report and ignored receipts.
The primary owns shared build registration, annotations and the full main build.
The nonadvancing array/message-deletion contract in `0076C500`, native class ABI,
network delivery and game execution remain open.
