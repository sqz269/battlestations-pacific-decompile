# Native network-console send queues and adaptive profiles (R175)

Addresses: 00a3a890, 00a3a8f0, 00a3aef0, 00a3b150, 00a3b210, 00a3bc60,
00a3c1a0, 00a3c5e0, 008d2f50

## Implemented scope

Seven complete normal bodies,2,428 original bytes, supply the remaining native
queue/timing dependencies of the send worker. Names are descriptive hypotheses.
The new source interfaces carry explicit process cells, constants and clock stack
preimages; the selector additionally preserves the native register/stack ABI.

| Entry | Original ABI | Behavior |
| --- | --- | --- |
| A3A890 (74B) | ECX owner, stack index/max/min/drop/weight, RET14 | Write a20-byte latency profile at owner+8+index*14h. Preserve x87 addition/multiply and the delayed mean store between other field writes. |
| A3A8F0 (444B) | ECX channel, stack removed/stale/now/interval, RET10/EAX node | Search three send lists. Keep now/interval on x87 throughout scans, honor unsigned sequence window and stale/closing/stream2 conditions, rotate reliable records to the tail or remove them, then stamp the selected record. |
| A3AEF0 (497B) | ECX channel, stack level-out, RET4/ST0 | Publish current level494 first. With sent70 zero, return the original default constant without sampling. Otherwise inspect min(sent,max(4,sent-ack60)) entries, cycling backwards over32 slots, and average the largest up-to-four values using the native x87/SSE comparison/spill sequence. |
| A3B150 (178B) | ECX channel, stack two pool-slot addresses, RET8 | Prepend whole send lists0..2 to the82Ch pool, then receive lists0..2 to the820h pool; clear each consumed head/tail, then run full R172 channel reset. No allocation or element free. |
| A3B210 (520B) | ECX channel, stack header/type/record, RET0C/AL | Update last socket14, estimate latency, move profile level up/down, sample time, then build a sequenced packet and timing slot or apply the unsequenced drop accumulator and append to stream2. Upper EAX is not a Boolean contract. |
| A3BC60 (182B) | ECX owner, stack sockaddr16, RET4 | Under captured tracked148, return/reset the first active channel matching only IPv4. Unlike A3BB30, a miss does not call the SDK conversion helper. |
| A3C1A0 (533B) | ECX owner, stack socket/address/type/data/length/direct, RET18/AL | Require a registered socket and available82Ch record. Populate metadata, then enqueue a direct index or normal channel packet. Once a record is taken, missing channel/disabled direct queue/profile drop returns it to the pool and still returns true. |

## Producer and arithmetic evidence

At8D31ED, `BSP_MultiGlobals_LoadLobbyTables` supplies Lua `LatencyLevel` index-1,
`Latency_Max`, `Latency_Min`, `Packet_Drop` and `Sync_Send_Weight` to A3A890.
That producer establishes16 actual20-byte records at owner+8, ending before the
tracked section at148. No profile defaults or fabricated table are installed by
this packet. Native indexing remains unchecked; callers must supply valid types,
profile rows and sequenced stream0..1, with complete finite lists and bounded data.

The timing estimator starts all four ranked values at zero and retains the native
unordered branches, so negative or unordered samples are not silently sorted into
the result. It does not cap the iteration count at32; larger outstanding counts
cycle through the ring. One/two/three/four-value returns retain their distinct
float spills and arithmetic. Packet serialization preserves the current F8ABDC
loads for profile selection, accumulation and the later server/drop decision.

The unsequenced path writes the accumulated float before comparing it with one,
subtracts the original double constant when ordered and large enough, and drops
only when server byte150 is nonzero and packet length exceeds32. A drop leaves
the partially written header/metadata intact for pool reuse. Normal payloads use
the8-byte header; direct packets use a3-byte header and a signed82Ch pool-index
calculation. No size clamp, fresh allocation or whole-record clear was added.

The selector's emitted444-byte body is **byte-for-byte identical** to the original.
Its explicit fastcall signature has an ignored EDX argument so ECX, the four stack
arguments and RET10 retain their original placement. Other functions use new
source signatures. Source tracked guards provide C++ cleanup on binding errors;
original private FH3/SEH behavior is not claimed.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. One local original-code
fixture compares76 cases and46,347,700 bytes: eight profile and32 estimator x87
precision/rounding/sample combinations, the zero-sample fast path,14 selector
cases, three complete pool-return cases, ten packet-queue cases and eight owner
enqueue cases. The checks include masked FP status/stack balance, raw list pointers
before normalization, sequence wrap, profile movement, intentional drops, real
tracked sections, nested lookup lock replacement, first-match removal and direct
pool index7. One source-only missing-clock case checks partial reset and guard release.

The fixture executes all2,428 new bytes and344 previously verified reset/lookup
bytes. It uses the actual fixed-clock provider, real Win32 sections and CRT memcpy,
plus a recording NTOHS boundary on lookup misses. No permanent test suite was added.
Evidence covers4,232 live/PE-matched bytes and13 direct calls. Seven functions and
the send-worker/Lua-producer dependency fragments are named/commented under the
Ghidra lock, with prior values, saved readback and refreshed exports preserved.
Immutable tested/combined archives and the report pin source, objects and observations.

## Remaining work

The send worker's queue/timing callees now have complete source bodies. Full worker
composition still needs actual socket operations and packet-recording dependencies
787AA0/787B00 ->787850, followed by the thread wrappers and full derived constructor.
The Lua producer itself remains a dependency fragment here. No ordinary startup,
network peer/exchange, concurrent-worker or gameplay claim follows from these
component comparisons. Native faults, unmasked FP traps, original allocator identity
and full exception/ABI equivalence outside the exact selector remain unproved.
