# Compatible deadline map storage layer

This source adapter reuses reviewed insertion mechanics from the existing
hardware-layout tree implementation. It does not reconstruct or rename another
STL instantiation. The only existing source change is
`src/native_hardware_layout_tree_insert.cpp`; its public APIs remain unchanged.
The new internal `detail/native_tree_insert_storage.hpp` parameterizes layout,
allocation, rotations, count limit and validation without changing the logic.

The extraction covers predecessor traversal, node linking, red-black repair
and the existing inlined left rotation. Hardware entry points B20D30 and B2F1B0
still use color/nil24/25,28h allocation, the original comparator outside this
layer, count limit0AAAAAA9 and the original returning callback/error provider.
The deadline adapter selects color/nil14/15,18h scalar nodes, count limit1FFFFFFE
and the existing raw18h rotation functions. Neither specialization owns a
second header, map, node projection or payload vector.

Whole native bodies independently support this reuse:4B7520 and B20D30 are
137 bytes/48 instructions with differences only in sentinel displacements and
direct transfer operands.4CF010 and B2F1B0 are492 bytes/167 instructions with
differences only in color/nil offsets, unsigned count limit, direct transfers
and original FH3 handler address. Every byte in each span matched both disk and
live analysis. This is evidence for source policy specialization, not a new
native ABI/FH3 claim. Existing library names remain unchanged.

The four source storage interfaces are deliberately narrower than subscript:

| API | Contract |
| --- | --- |
| `allocate_input_deadline_map_node` |Allocate18h through the existing malloc/new-handler domain; write left/right/parent, current pair key/value DWORDs, color and nil0. Pair is two words, not a hardware-layout key. Padding16/17 remains untouched. |
| `decrement_input_deadline_map_iterator` |Reuse the predecessor algorithm with nil15. A returning initial invalid handler is followed by a fresh node read; end selects the current maximum. No flags, refcounts or payload ownership. |
| `equal_input_deadline_map_iterators` |Capture the first owner, validate owner equality through the real CRT handler, then reload both nodes for comparison. A returning handler may change those nodes. |
| `link_input_deadline_map_node` |Consume the caller-selected valid parent/side and actual pair. Check native unsigned count limit before allocation, increment current count after it, link and rebalance, make root black, publish output NODE before OWNER. |

The canonical header remains12h: opaque word0, head4, count8. Deadline nodes
remain18h: links0/4/8, signed scalar key+C, float bits+10, color14, nil15 and
padding16/17. The adapter copies mapped bits without a floating conversion.
The caller owns the actual header and chooses the insertion site; this layer
adds no comparator, duplicate policy, range clamp or successful missing-key
provider. Source inputs are evaluated arguments, not the original native stack
slots; native FH3, mutable register/spill aliases and binary ABI are excluded.

The source-owned length error uses the existing public
`NativeHardwareLayoutTreeLengthError` transport with actual28h D69260 storage.
The19-byte `map/set<T> too long` payload is assigned before arming temporary
string destruction. Then the existing native logic-error constructor, copy
and destructor contracts supply ownership. Reaching the limit leaves the tree
and output unmodified. No fake exception or handler is installed in production.
The real `_invalid_parameter_noinfo` service may return or throw.

Validation used the strict Win32 build and both existing CTests; all8 native
seeds matched. The existing hardware insertion fixture was copied unchanged
from the dedicated fixture worktree into ignored local storage and linked
against this worker archive, once before extraction and again afterward.
Both runs matched8582 original/host behavior words,56 output stores each, and
five actual CRT throw events each. Its existing cases also cover allocation
failure, exception ownership, mutable/returning handlers and output ordering.
The fixture's original workspace and its stored evidence were not changed.

One additional ignored manifested deadline fixture linked six18h nodes using
fixture-selected vacant sites. It checked signed ordering, parent links,
red-black properties, all six raw mapped payloads, output aliasing header words,
two actual returning CRT handler calls, the owning length error at1FFFFFFE,
and cleanup through existing86FDE0. The test restores the deliberately aliased
header before cleanup. It does not supply a production insertion search or
claim native differential coverage of this new adapter; only the hardware
fixture executes original instructions. No SDK, game, cursor or application
execution was performed.

Ignored runners accept a `-CoreWorktree` argument and link only that tree's
headers/archive: `local/run_hardware_insert_storage_regression.ps1` and
`local/run_native_input_deadline_map_storage_probe.ps1`. The report records
archive, source, input-fixture and result hashes. CALL rows describe the native
contracts and unchanged hardware entry points; no new original-address ledger
records are added for recognized library mechanics.

The next bounded layer must provide4D6900 subscript and4D3CD0 checked hinted
insertion over these interfaces, including signed lower-bound key capture,
duplicate handling and checked iterator ownership.4D1F40 unique insertion is
also still required by the fallback. The current layer exposes no get-or-insert
stub. It is ready for those source contracts to consume without changing
canonical nodes, while the startup worker retains its required
`call_004d6900(actual_header, key_pointer)` boundary.

The final CALL audit checked22 direct/tail rows with0 failures. Existing native
boundaries remain B20D30..B20DB8 (one-byte RET) and B2F1B0..B2F39B (RET10h at
B2F399, three bytes). There are no missing starts or excluded-tail calls in
this packet. Native library addresses cited for the new storage contracts are
read-only evidence and receive no new ledger records or ABI claims.
