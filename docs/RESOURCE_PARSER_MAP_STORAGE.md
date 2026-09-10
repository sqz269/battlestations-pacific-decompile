# Resource-parser map insertion, erasure, and ownership

Addresses: 00b7fd30, 00b80500, 00b7f610, 00b7f730, 00b7ca80, 00b7d590,
00b7cf80, 00b7f790, 00b7f1b0, 00b7e000, 00b7ca20, 00b7ca40, 00b7e970,
00b80a50.

The resource-manager parser map owns its key strings and tree nodes and borrows
its parser pointers. Insertion, node erasure, and full clear neither AddRef nor
release a parser. `00b80500` is range erase, including a full-clear fast path;
the manager destructor separately frees the surviving sentinel.

`reports/resource_parser_map_storage_audit.json` preserves complete original-PE
and live-Ghidra byte matches, return instructions, old names/prototypes, old
exports and plate annotations, and proposed descriptive names. Every live
query verified project `bsp`, program `/battlestationspacific.exe`, language,
and image base through `tools/bsp.py`. Names are descriptive hypotheses.

## Native representation

The parser tree starts at manager+8, with its head pointer at tree+4 and count
at tree+8. The preceding tree word is not reinterpreted here. An iterator is
two words: tree address, then node address. Normal nodes occupy 1Ch bytes:

| Node offset | Established content |
| --- | --- |
| +0 / +4 / +8 | Left / parent / right links |
| +Ch / +10h | Owned counted key length / character data |
| +14h | Raw, nonowning parser pointer |
| +18h | Color: 0 red, 1 black |
| +19h | Sentinel flag: 0 normal, 1 head |

The head's left, parent, and right links cache the minimum node, root, and
maximum node respectively. Empty-tree links all point to the head.
`00b7e000` allocates 1Ch bytes, zeros those links, sets black color and initially
clears the sentinel flag. The already audited manager constructor `00b81040`
then sets +19h to 1, links the head to itself, and initializes count zero.
The head's key and parser slots are not constructed or read by full clear.

## Key storage and unique insertion

The existing `RESOURCE_MANAGER_REGISTRATION.md` audit establishes that
`00b80a50` obtains a parser's type name through virtual +4, rejects an existing
equivalent key, otherwise constructs a copied-name/raw-parser pair and calls
`00b80290`. The latter walks the tree and rejects equivalent keys without
replacing the original parser.

Comparison first handles counted length zero. An empty key sorts before every
nonempty key. When both lengths are nonzero, native CRT `stricmp` decides the
order; there is no length tie-break. Consequently, nonempty keys with equal
C-string prefixes but different bytes after NUL are equivalent. A truly empty
key and a nonempty key starting with NUL remain distinct. Key copying preserves
the full counted bytes, not just that comparison prefix.

`00b7fd30` receives an already chosen parent and insertion side. A count at or
above 15555554h throws a length error before allocation; this does not turn
ordinary insertions into throw-only calls. It calls `00b7f610` for a 1Ch-byte
node, with left/right set to the head and color red. `00b7f1b0` sets the three
links, zeros the key pair, resizes it to the source's length using `0041dd40`,
and copies that many bytes. The separately audited resize allocates length+1
from the sized-storage pool and writes the trailing NUL. A zero-length key
keeps null storage. The constructor copies source-pair+8 directly into
node+14h and sets the normal-node sentinel flag. No parser method is invoked.

After construction succeeds, insertion increments count, links the node,
updates head minimum/maximum/root when necessary, and performs red-black
recoloring and rotations. `00b7ca80` rotates right; `00b7d590` rotates left.
One left rotation is inlined in insertion. Root color is forced black, and
the returned iterator contains the tree and newly allocated node.

## Erasure and destruction

`00b80500` checks iterator container identity. If first is the current minimum
and last is the head, it calls `00b7f730` on the root, then restores all three
head links to self and resets count to zero. The returned iterator is end.

`00b7f730` terminates on +19h. For a normal node it recursively disposes the
right subtree, saves the left child, returns nonnull key data through
`00419cc0`/`00bd1510` with size length+1 and flag 1, frees the node through
`00bf65ac`, and iterates into the saved left child. It never reads node+14h.
This proves parser ownership independently of registration's lack of AddRef.

For a partial range, `00b80500` increments the current iterator before erasing
the saved old node through `00b7f790`, repeating until the exclusive end.
`00b7cf80` finds the next node by descending to the right-subtree minimum or
ascending until it leaves a right-child chain. Incrementing a sentinel or an
unassociated iterator enters `00bf6713`'s invalid-iterator path.

Single-node erase rejects a sentinel with an out-of-range throw. It computes
the successor first. A node with two normal children is replaced structurally
by that successor, transferring links and swapping colors; it does not copy
or release the parser values. Other nodes are spliced out directly. After
updating extrema and restoring black-height through rotations/recoloring,
it returns the removed key's storage and frees the removed node. The complete
tail decrements count only when nonzero and returns the saved successor
iterator. Invalid-iterator diagnostics and native exception dispatch remain
external contracts.

The extrema helpers `00b7ca20` and `00b7ca40` follow right and left children,
respectively, until the next child is a sentinel. Assembly returns the last
normal node in EAX; their old void pseudocode omitted that return value.

The previously audited manager destructor `00b80f10` uses this full-range
clear, frees the parser-tree sentinel itself, then zeros the head/count.
The parser singletons' independent lifetime registration is unchanged.

## Original calling contracts

Stack entries below are in caller argument order. RET amounts are hexadecimal.

| Address | Inputs / output | Return |
| --- | --- | --- |
| `00b7fd30` | ECX tree; stack output iterator, side byte, parent, source pair; EAX output | RET10h |
| `00b80500` | ECX tree; stack output iterator, first tree/node, last tree/node; EAX output | RET14h |
| `00b7f610` | Stack left, parent, right, source pair, color; EAX allocated node | RET14h |
| `00b7f1b0` | ECX node; stack left, parent, right, source pair, color; EAX node | RET14h |
| `00b7e000` | No consumed input; EAX allocated head storage | RET |
| `00b7f730` | ECX tree carried for recursion; stack subtree node | RET4 |
| `00b7ca80` / `00b7d590` | ECX tree; stack pivot node | RET4 |
| `00b7cf80` | ECX iterator; updates its node word in place | RET |
| `00b7f790` | ECX tree; stack output iterator, input tree/node; EAX output | RETCh |
| `00b7ca20` / `00b7ca40` | ECX subtree root; EAX maximum / minimum node | RET |

## Analysis repair and host implementation

Before this audit, call sites `00b7f76c` and `00b7fa1d` had incorrect
`CALL_RETURN` overrides on calls to `free`. They hid the left-subtree loop and
single-erase count/output tail. The full PE/live bodies end at `00b7f782` and
`00b7fa5c` exclusively. The report saves those bytes and the old overrides
before the primary agent's coordinated repair. In addition, old pseudocode
marked successor-transplant blocks unreachable: assembly shows that the
iterator passed by address to `00b7cf80` changes the stack node later read at
`00b7f82e`. Assembly, rather than those removed blocks, establishes this path.

`ResourceParserMap` in `include/bsp/resource_parser_map.hpp` and
`src/resource_parser_map.cpp` implements owned names and typed, borrowed
`StructuredResourceParser*` values with a host `std::map`. It provides unique
insert returning both the stored identity and inserted flag, lookup, erase by
key, clear, and size. Duplicate lookup precedes the native count limit.
Comparison uses the native empty gates and CRT `_stricmp` without normalizing
names or rejecting embedded NUL. Erase and destruction free only host
keys/nodes. The parser interface and manager integration belong to the primary
integration work; no parser implementation is synthesized by this container.

This is a new C++ interface, not a native ABI replacement. It does not reproduce
native allocator layout, invalid-iterator diagnostic machinery, sentinel
allocation, or SEH cleanup. Allocation-failure unwinding has not been audited;
the owned-key and parser-lifetime claims concern established normal paths.
Build and integration validation are recorded by the primary agent. No new
test target, native differential run, or game validation is claimed here.

## Registered wire-payload dispatch

`StructuredResourceRegistry` in `include/bsp/structured_resource_registry.hpp`
and `src/structured_resource_registry.cpp` uses this map to select actual
Mesh, Note, and GroupParams wire decoders. Its `StructuredResourceParser`
interface exposes an owned type-name getter and a decode operation, reflecting
the roles of native parser virtual slots +4 and +8 without reproducing their
ABI. Callers own and explicitly register each concrete parser instance; the
registry borrows it. This does not claim construction of all six native
default singletons or any native item lifetime.

Registration preserves a detail visible in the complete `00b80a50` body: it
calls the type-name getter once for the probe, destroys that temporary, and
returns false on a duplicate. Only after a miss does it call the getter again
and attempt unique insertion. Its final return is true without checking the
insertion result. If a stateful getter changes the second name to an existing
key, that existing parser remains stored while registration still returns
true. The host follows this ordering and result rather than substituting a
single getter call or reporting the unique-insert result.

The refreshed 366-byte PE/live match for `00b7e970` establishes ECX manager,
one Resource-node-handle pointer on the stack, and RET4. At `00b7ea55` it loads
the selected raw parser, calls virtual +8 with the child handle, and retains
the result for append. The unknown branch creates the previously audited
eight-byte fallback and skips the payload at `00b7ea95`. Both paths append
through current resource virtual +Ch at `00b7eaa7` before child-handle release
at `00b7eab5`.

The host `dispatch_items_00b7e970` iterates the Resource container's children,
looks up their original counted tags, and invokes the selected decoder. Each
output record retains that original tag and an optional owning payload:

| Type | Concrete host parser | Established native decode | Output |
| --- | --- | --- | --- |
| Mesh | `MeshStructuredResourceParser` | `00b947a0` through `00b944e0` | `MeshResourcePayload`, including the serialized prefix DWORD |
| Note | `NoteStructuredResourceParser` | `00719000` through `00718f50` | `NoteResourcePayload.text`, truncated at the first NUL after the full counted read |
| GroupParams | `GroupParamsStructuredResourceParser` | `00b8eb50` through `00b8e580` | `GroupParamsResourcePayload.value`, followed by explicit skip/detach |
| Unregistered | None | Unknown branch in `00b7e970` | Original tag and `nullopt`, explicitly meaning unsupported payload skipped |

These payload decoders retain their existing audited behavior. Mesh and Note
do not gain an implicit unread-tail skip. GroupParams and unregistered types
explicitly skip. Records append in encounter order before the child is closed,
and the Resource container remains attached for its caller. On failure, input
already consumed and records already appended remain. Host allocation errors
inside dispatch return a diagnostic failure.

An unsupported record does not claim to construct the native fallback item.
Likewise, successfully decoded payloads are wire values, not native resource
items, GPU meshes, or renderer state. Native specialized append/classification,
singleton ownership, and renderer begin/end hooks remain separate integration
work. The original dispatch and registration name/prototype/comment/export
state is preserved alongside the earlier map evidence in the companion audit.
