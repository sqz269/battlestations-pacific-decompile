# Raw property iterator initializer readiness (CC12)

Addresses: 00480690

The complete `00480690` iterator initializer is 65 bytes, 22 instructions and
zero calls, with no stored listing gaps. Its whole body is established, but
it is **not ready for independently genuine-input current raw Source
registration**: the actual populated raw bag-map producer remains absent.
An empty-map initializer or a semantic container does not close this helper's
whole input domain. No Source/native execution, new tests, shared metadata,
Ghidra mutation or other native-body expansion was performed.

## Body and ABI

Owned range is `00480690..004806D0`, exclusive end `004806D1`. Complete
live pseudocode/listing/bytes matched the installed PE, SHA-256
`b9167301cb1875e13c9fd0c9075479b79b62210969d8a67cd5f3108c2aeaff2c`.
All live queries used `bsp.py`, which verifies the `bsp` project and
`/battlestationspacific.exe`, x86 language and image base before querying;
configuration pins `C:/Users/sqz269/bsp.gpr` and the original installed PE.
No instruction-flow repair is needed.

ECX is a writable actual three-DWORD (`0Ch`) iterator. Its `+0` is an actual
embedded map pointer. There are no stack arguments and the return is plain
RET. ECX stays unchanged. EDI is preserved on every path; EBX/ESI/EBP are
untouched and ESP is balanced. EDX is overwritten with the map pointer and,
on the nonempty path, the selected head. On the count-zero path EAX remains
its incoming machine value; on the nonzero path EAX is the selected bucket
index. No declared C++ value result follows from that path-dependent EAX.
Integer flags are unspecified. There are no REP/string, x87 or SSE
instructions, and no DF requirement or mutation beyond normal caller ABI.

The initializer reads count at map `+4`, and head DWORDs at
map `+8 + index*4`. It does not read the map profile or dereference a node.
The actual bag's embedded map has `108h` bytes, with 64 heads `0..63`.
The whole native contract is:

| Branch | Writes | Native behavior |
| --- | --- | --- |
| count equals zero | iterator `+4 = 0` | preserves iterator `+8` and incoming EAX; reads no heads |
| count is any nonzero DWORD | iterator `+8 = 0`, then selected index; iterator `+4 = selected head` | examines head 0, then increments before probing later heads; reloads selected head before publication |

Native tests only zero versus nonzero. Negative bit patterns are not rejected.
For a stable coherent populated actual map, it selects the first nonnull
head among `0..63`. It preserves iterator `+0` and never changes any map or
node bytes. The caller supplies a distinct stack iterator and actual root
storage; arbitrary iterator/map overlap is outside this admitted caller path.

A precise malformed-state limit must survive any eventual Source body.
The loop compares the current index with `40h` **before incrementing**.
If count is nonzero but all 64 heads are null, `004806B6` probes head 64
at map `+108`; `004806C1` then reloads that same DWORD, leaving index 64
and publishing it as the current node. In an actual `114h` bag this is the
adjacent bag `+10C` ordinal, not a 65th real bucket. Do not clamp at 63,
substitute null, repair corrupted input implicitly, or claim malformed safety.

## Actual native caller and producer admission

The previous sealed complete clone body `008F41F0..008F4289` establishes
actual bag layout and the exact caller memory provenance. Its entry ECX is
retained in ESI at `008F41FA`; `008F423C` advances that actual source bag
by four bytes to its embedded map. `008F423F` gives ECX the actual local
iterator at `[ESP+8]`; `008F4243` writes the actual map pointer into iterator
`+0`; `008F4247` calls `00480690` with no pushed argument. The caller reads
iterator `+4` at `008F424C`, then reads the resulting genuine node's key
`+4` and record `+8` at `008F4254` / `008F4260`.

The native clone's new root is an actual `114h` heap allocation, with
`D16504` at root `+0` and `D162C4` at embedded map `+0`. Count, all 64 heads,
ordinal and owner are initialized in that complete body. These are native
producer/layout facts, not proof that current Source constructs populated
actual raw maps. All ten live xrefs are saved; only this already verified
clone caller's argument layout is admitted here. Other callers' bodies and
`0047E480` were not expanded.

## Fresh current Source boundary

The Source search was refreshed in both the named worktree and live main.
At the receipt timestamp live main was `2b2b2aad0fc287d5a378cdeb0d9192f7aedb5723`.
Its “Register raw property bag storage constructor” commit changes only
`config/parallel_work.json`: this is registration metadata, with no actual
`008F41A0` Source implementation found yet. The external constructor work
stream remains owned there and was not duplicated.

`ScenePropertyBagModel` and `ScenePropertyMergeHost.iterator_open` are
semantic/abstract surfaces, not the actual raw header and node producer.
The existing `initialize_native_property_tree_library_storage_008f5670`
constructs an empty different `10Ch` root with `D1650C` / `D162C8` profiles.
It does not establish a populated actual `D162C4` record-map producer.
Allocator or genuine pool storage alone also does not publish this map's
count, heads, keys, record pointers and owning nodes.

The immediate empty actual-bag producer is the separately owned whole
constructor `008F41A0`; its integration must be rechecked before relying on
it. Even after that initializer exists, count-zero input alone cannot close
this whole iterator. Actual raw insertion `008F33F0 -> 008F28F0` must establish
genuine current nonempty map/head/node publication. Those bodies remain
named and unexpanded in this packet. No forged nodes, copied semantic bag,
null-only closure, enum-map substitution, opaque provider callback, lifetime,
profile/class/EH identity, factory, world, binary replacement ABI or game
claim is introduced.

## Outcome and verification

There is no ready implementation packet from this audit. The exact blocker
is current Source production of genuine populated actual `D162C4` maps:
raw `008F33F0` insertion and its underlying `008F28F0` publication contract,
following real `008F41A0` raw storage initialization. A separately leased
bounded audit of that producer is the next evidence needed; this packet
does not expand it or the advance routine.

The report supplies the exact `008F4247 -> 00480690` caller row and live
containing body. The existing call verifier checked that row with zero failures; the owned
routine itself has zero calls. No C++ changed, so no build or new test was
run. Complete static native evidence is separate from Source input-provider
readiness, runtime behavior, ABI compatibility and game validation.

Ignored raw evidence is under
`J:/PROG/battlestations-pacific-decompile-cc12_property_payload/local/cc12_property_iterator_initializer/`:
complete owned pseudocode, assembly and bytes; PE equality/flow receipts;
all xrefs; exact caller instruction/body range; copied prior sealed clone
listing/byte receipt; fresh Source search and file-hash receipts; and the
artifact manifest pinned by the tracked report.

Primary review independently read all22 live instructions, rechecked the whole
65-byte installed body and all24 sealed artifact hashes. Main09d170c37 now
contains the raw61-byte008F41A0 storage constructor, after this audit snapshot.
Populated map production still needs008F33F0/008F28F0 and their dependencies;
the original snapshot is retained and no Source/native runtime claim is added.
