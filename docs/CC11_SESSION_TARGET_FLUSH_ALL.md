# Complete native target flush-all loop

`00783490..0078350B` is a complete 123-byte, 47-instruction function. Native ECX
holds the transport, the target is one stack DWORD, and the function returns
with `RET4`. `BSP_SessionTransport_FlushTargetBuffers` is a descriptive hypothesis.
The sole current static caller is `00784B10`, at `00784C04`, after its message20
serialization path. That caller is not reconstructed by this packet.

The new `NativeUnitHealthMessageFlushAllCalls` derives from the existing
`NativeUnitHealthMessageTransportCalls` and inherits its constructor and pure
actual-storage aliases. Its public `flush_session_target_00783490` supplies the
complete loop. It adds no provider callback, profile mapping, default world,
clock, allocator, or delivery-success policy.

## Preserved schedule

1. Capture actual target `+4`, enter its real critical section, then increment
   its raw DWORD depth at `+18` modulo 32 bits.
2. Visit fresh target `D40/D44/D48` cursor cells in delivery order `0/1/2`.
   Read each captured cursor's bit word, base and current address. Compute
   `((bit & 7) != 0) - base + current` as DWORD arithmetic. Zero skips both
   the flush and the reset; this is not a signed-positive test.
3. For a nonzero count, read the current actual transport table and `+20`
   word. Invoke the existing complete ordinary host/client flush provider.
4. After return, reload the base from the **captured cursor**, then write
   current=base, bit=0, and byte[base]=0. This also happens after host
   suppression or an enqueue that cannot obtain a record.
5. Reload the actual target `+4` cell, decrement that current lock's DWORD
   depth, then call real LeaveCriticalSection. No exception cleanup is added.

The existing final `+20` implementation admits `783E00/783EA0`, and the existing
fresh `+24` dispatch admits `A41AB0/A419B0`. These invoke complete `A39120/A39160`
and the genuine `A3C1A0` owned-record enqueue. The same actual clock publication,
game and network-owner cells, actual profile words, SDK imports, and live backing
remain required. Existing unsupported-profile `logic_error` exceptions are
Source contract failures, not reproduction of native faults or private EH.

## Original-loop fixture and its boundary

The fixture at `local/cc11_target_flush_all_20261007_a` executes all 123 original
bytes. Live Ghidra and the original PE matched exactly. Only two four-byte
absolute instruction operands are relocated:

| Operand | Original IAT | Copy offset | Replacement |
| --- | --- | --- | --- |
| `007834A0` | `00CE2218` | `10` | Pointer to real EnterCriticalSection |
| `00783500` | `00CE2210` | `70` | Pointer to real LeaveCriticalSection |

All other original instructions, branches, and stack behavior are retained.
The copy changes from writable to executable/read-only storage.

Dispatch data has a separate, explicit operation relocation. The native fixture
copies ten actual `D243EC/D24448` words and replaces only `+20` with an inspected
ABI thunk. That thunk takes native ECX transport and the original three stack
arguments, leaves incoming EDX unused, and returns with `RET0C`. It forwards
directly into the **complete existing Source host/client flush implementation**
using the stable actual fixture adapter. It fabricates no result and mutates
no cursor, transport, clock or lock. The remaining `+24` words retain their
original tokens. Source-side `+20` words also retain their original tokens.

This is an independent full **original loop** comparison with shared, genuine
downstream Source providers. It is not an all-original transport stack or a
new differential proof of those previously established providers.

## Connected validation

One genuine raw clock is constructed and published through complete clock and
manager services. The real fixed-mode method is enabled for 16ms and advanced
once. Its timestamp fields are not manually seeded. Both worlds borrow the
same unchanged actual publication and method context throughout channel reset,
queue and flush calls. SDK `ntohs/htons` calls use real imports with no mutation
hooks.

Each world constructs main and alternate actual targets, owning their locks,
three separate cursors and histories. It supplies explicit borrowed `D74`
sockaddr backing, actual transport layout/profile words, two genuine unconnected
UDP sockets, a canonical owner lock, sixteen complete channel objects and three
complete queue records. Socket registration, profile rows and pool links are
explicit fixture configuration. This does not construct the full derived
transport/network owner or its send workers; no packets are sent or connected.

Eight cases passed **396 checks, zero failures**:

- Host and client mixed full/partial payloads across all three deliveries;
  the client case starts and finishes with a genuinely held recursive lock.
- Host delivery0 suppression still resets its captured cursor.
- All-empty cursors, including bit=8, retain their bits and first bytes.
- One partial-byte normal payload and one partial-byte client direct payload.
- An empty pool and exhaustion after delivery0 still reset nonempty cursors.

Checks compare cursor base/length/current/bit/first byte, all other target bytes,
both history headers and 53 samples, actual lock recursion/depth, sent and game
counters, timestamp bits, queue order/type/socket/address/payload, and complete
network-owner backing. Only named pointer fields are normalized: owner lock,
free-pool head, three record-next links, and each channel's send/receive heads
and tails. Every other byte, including floats and payloads, compares exactly.
Owned queues survive backing overwrite and complete target destruction.

The final strict Win32 build compiled 29 current repository translation units
plus the probe from an immutable 84-input source/header/fixture snapshot.
MSVC 19.51.36244 used `/O2 /W4 /WX /fp:strict` and an embedded `asInvoker`
manifest. Workspace/snapshot, three frozen support libraries, and original PE
hashes remained unchanged through the final build/run. Support libraries were
frozen from the parent's `ac58ee9db` build at observed metadata head `016d0132d`.
Main's later authorized rebuild changed its core library; the linked read-only
copies stayed unchanged, and both observations are recorded separately.

The emitted Source loop is 150 bytes. COFF records real Enter at `17`, the final
complete flush call at `61`, and real Leave at `8D`. Fresh cursor capture occurs
at `34`, fresh `+20` at `5B`, post-call base/reset at `66/68/6B/72`, and the fresh
exit-lock value at `82`. The loop contains no FP instruction. Both native ABI
thunks were inspected and forward only to the corresponding complete Source
body, with `RET0C`. There are no runtime mutation callbacks; changing a cursor,
table or lock during a real provider call is not exercised by this fixture.

## Boundary

This is complete ordinary Source behavior with a new member ABI, not native
binary/private-EH compatibility or game validation. Reached pointers, writable
cursor backing, finite fitting packet queues, profiles and locks must remain
valid. Existing downstream masked-FP/storage contracts remain in force. No
full session/transport/network-owner factory, send worker, socket exchange or
caller reconstruction is claimed. Primary integration and the full main build
remain with the integrator. Detailed evidence is in
`reports/cc11_session_target_flush_all.json`.
