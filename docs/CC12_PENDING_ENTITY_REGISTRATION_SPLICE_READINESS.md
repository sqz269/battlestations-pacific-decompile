# Pending tick-registration splice `00874C90`

Read-only packet `cc12_pending_entity_registration_splice_readiness` captures
the complete **111-byte / 31-instruction / zero-CALL** body through
`00874CFE`. Seven target-verified Ghidra/PE-image windows total **202 bytes**.
This is an intrusive tick-registration list, separate from the destroy/kill
queues at `F899A8/F899B4` and from the World entity headers.

The consumer's exact storage contract is established. **An actual-storage
Source packet is not yet ready:** current Source provides layout constants,
positional host interfaces and an assumed-empty path, but no demonstrated
common mutable registry owner or actual `00875890` registration producer.

## Actual owner identities

| Storage | Exact role |
| --- | --- |
| `E0B6D0` | Pending head sentinel, 34h-byte node. `+4` (`E0B6D4`) is head.prev; `+8` (`E0B6D8`) is head.next/first node. |
| `E0B704` | Pending tail sentinel, 34h after the head. `+4` (`E0B708`) is tail.prev/last node; `+8` (`E0B70C`) is tail.next. |
| `F876C0 + i*68h` | Group head sentinel. Group first is `+8`; group tail is `+34`; group last is the tail's previous link at `+38`. Existing wave evidence describes five such groups; this leaf does not check that range. |
| Actual registration node | Same 34h node moved between lists, with prev at `+4`, next at `+8`, and raw group-index DWORD at `+14`. The payload, profile, linked byte `+18`, and sub-list count `+24` are not changed by this leaf. |

There is **no registration count field accessed or updated here**.
`E0B6D4=0` and `E0B70C=0` are outer sentinel links, not cleared counts. The
head and tail bound a linear doubly linked list; this is not a circular
one-sentinel ring or a separately allocated 0Ch payload-node list.

Initial-image evidence is distinct from a running process. The pending head
and tail prefixes are file-backed `.data`: head.prev=0, head.next=`E0B704`,
tail.prev=`E0B6D0`, tail.next=0. The two group0 prefixes are in virtual
zero-fill, so their saved-image links are initially zero. A bounded startup
instruction at `00CD27D6` writes `[F876F8]=F876C0`, proving a later group-link
initialization boundary. Its enclosing initializer was not reconstructed or
repaired here. Zero-filled Source group storage is not a coherent empty list,
and copying original numeric pointer values into unrelated host storage does
not create the required owner.

## Exact mutation order

Let H=`E0B6D0`, T=`E0B704`, N=actual node, and G=selected group head.
The leaf reads **H.next once** initially. If it equals T, it skips movement
and still executes all four pending-sentinel reset stores.

For each N, the assembly performs these operations in order:

1. Capture `saved_next = N.next` (`874CA0`). Read `N.prev` and write
   `N.prev->next = saved_next` (`874CA3/CA6`).
2. Reload `N.next`, reload `N.prev`, then write the reloaded next node's prev
   to the reloaded prev (`874CA9..CAF`). Do not replace these with an invented
   node copy or a different unlink schedule.
3. Read N's raw DWORD `+14` after unlink, calculate the low32-bit product
   `index*68h`, then add base `F876C0` with 32-bit wrap (`874CB2..CB8`).
4. Compare the **captured** `saved_next` against T (`874CBE`). This comparison
   controls the later loop branch; the intervening MOV/LEA instructions do
   not alter its integer flags.
5. Read `G+38` and store it into N.prev (`874CC4/CC7`). Store `G+34` into
   N.next (`874CCA/CCD`). Reload `G+38`; store N into that old last node's
   next (`874CD0/CD3`); finally store N into `G+38` (`874CD6`).
6. Set the cursor to `saved_next`. Repeat unless the saved comparison reached
   T (`874CD9/CDB`). The next pointer is captured before moving N; it is not
   read back from N after the append.

On both empty and nonempty paths, reset in this exact order:
`H.prev=0` (`874CE0`), `H.next=T` (`874CE5`), `T.next=0` (`874CEF`),
`T.prev=H` (`874CF4`). Under coherent-list preconditions this preserves pending
order within each destination group and appends after that group's existing
last node. It neither destroys, allocates nor copies a registration or payload.

## ABI, invalid-storage and concurrency boundaries

There is no argument or receiver input. The body has plain `RET`; EAX is
zero on ordinary return, without establishing a semantic return type. ESI is
saved/restored only on the nonempty path; EBX/EDI/EBP are untouched. ECX/EDX
are scratch. The final XOR establishes its arithmetic flags and the following
stores preserve them. This evidence is not a Source/native ABI replacement.

The address expression is `(base + raw_index*0x68) mod 2^32`. No signed or
unsigned range test, null check, overflow recovery, malformed-link repair,
count guard, lock, callback or allocation occurs. An empty pending list must
contain **T**, not null, in H.next. A future Source caller must qualify live
coherent sentinels, writable node/link storage, valid group selection and
non-overlapping ordinary list identities. It must not clamp an invalid index
or introduce a null-list early return as purported Native behavior.

The producer's documented append uses the lock owner returned by `875280`;
this consumer acquires no lock. The caller must establish the appropriate
producer/consumer phase and lifetime relationship. `volatile`, a private copy
or an added lock is not proof of the Native concurrency contract. The routine
is not a safe arbitrary concurrent drain: it terminates on a captured next
value and then resets the pending endpoints.

## Current Source and bounded producer blocker

`fixed_step_job_waves.hpp` records the correct offsets and five-group count,
but `FixedStepJobWaveHost::next_element(group,position,view)` is a positional
interface. `GameFixedStepHost::next_element` always returns false, and its
`flush_pending_tick_registrations_00874c90` only records the assumed-empty
case. Neither owns or resets the actual sentinel cells. The unit layout's
`construct_tick_node` is an abstract hook; its call does not provide an actual
registration producer. Searches by exact global addresses, producer address
and storage names found no current process owner for these lists.

`GameNativeReadOnlyData` explicitly maps only verified read-only data through
`E07B23` and excludes mutable globals/owners. The mutable CRT owner exposes
specific CRT cells, not this registry. Existing pending destroy/kill owners
have different storage and node identities and cannot be substituted.

The bounded producer append window connects `875890` to the **same** pending
cells: reads at `8758D3/8758E6` and the write at `8758EE` target tail.prev
`E0B708`; `8758DF` sets the node's next to `E0B704`. `8758EB` writes the
reloaded old last node's next, which is head.next only for an empty list.
The corresponding Ghidra xref to `E0B6D8` is not an unconditional direct
head write. Current metadata bounds the complete
constructor to **`00875890..0087591A`, 139 bytes / 42 instructions**, with
named internal callee `875280`. Existing evidence says it receives actual
node, payload and group and appends to the pending list. Its whole body,
lock/owner lifetime and imports were not expanded by this consumer audit.

The next bounded packet is a **read-only `00875890` producer/owner contract
audit**. Establish exact actual node initialization and publication, shared
pending sentinel identity, index provenance and the `875280` lock-owner
boundary, without sweeping all nine construction callers or expanding the
lock getter's callees. The group's bootstrap owner also needs a separately
identified and qualified initialization/lifetime contract; the one captured
startup store is insufficient. Only then can a consumer-only Source packet
borrow those same pending/group cells and real nodes. No speculative owner,
empty-list instance or Source signature is admitted here.

The [report](../reports/cc12_pending_entity_registration_splice_readiness.json)
pins all31 instructions, seven code/data windows, current Source inputs and the
bounded producer evidence. No Source/CMake/ledger/Ghidra changes, flow repair,
build, tests, probes, new Original credit or World/application admission occur.
