# Native hardware-layout tree insertion

This packet reconstructs five complete native functions on the actual 28h tree
nodes and borrowed 108D530-style headers. The strict primary Win32 build and
existing CTests pass. Original-instruction differential review is pending;
this document does not yet claim fixture validation or saved Ghidra annotation.
Descriptive names and new C++ interfaces are reconstruction hypotheses.

| Entry | Original ABI | Scope |
| --- | --- | --- |
| `B20D30` | ECX actual iterator; RET or invalid-handler tail | Checked predecessor, including current iterator reloads after a returning handler and during parent ascent. |
| `B28370` | ECX node; stack left/parent/right/pair/color; RET14h; EAX node | Ordered left/right/parent link writes, current-count forward pair copy at +0Ch, color then zero sentinel. |
| `B29CD0` | Stack left/parent/right/pair/color; callee RET14h; EAX node | Fixed28h shared-new request and conditional complete node initialization. |
| `B2F1B0` | ECX tree; stack output/left-byte/parent/pair; RET10h; EAX output | Length limit, node allocation, current count/link publication, red-black repair and root blackening, then output node before owner. |
| `B2F540` | ECX tree; stack output/pair; RET8; EAX output | Actual pointer-key search, checked predecessor, duplicate detection and insertion; output node, inserted byte, then owner. |

The key/count/value are borrowed raw DWORDs; no declaration or layout owner is
retained or released. Unused key words, node +26h/+27h padding and result +9..Bh
padding remain untouched. All accessed storage and tree links must be valid;
the normal tree algorithms expect an initialized red-black tree with its head
sentinel. There are no added bounds, null replacement, sorting, or recovery
policies. Existing tree rotations and complete key comparison/copy routines
are reused where their instruction order matches.

The native node initializer writes links before reading the current source
pair. Overlapping source storage is therefore observable. Pair copies retain
the forward/current-count rules from the key packet. The inlined left rotation
at B2F326 rereads the replacement's left link after the first write; this order
is retained explicitly. Allocation occurs before current tree-head/count reads;
the earlier head and parent remain the arguments for initializing the new node.

`B2F1B0` checks unsigned count >= `0AAAAAA9h` before allocating. Its complete
error path assigns the 19-byte counted `map/set<T> too long\0` string, arms
temporary cleanup, constructs the existing native logic-error owner and sets
profile `D69260`. Native handler `CBD7C8` names FuncInfo `DF6050`; its one-state
unwind map at `DF6048` calls `CBD7C0` -> `4072D0`, with no catch map. ThrowInfo
`D83F98` names destructor `411780`. The C++ owning transport preserves 28h
storage, uses existing `411700` construction, `411940` length-error copy and
`411780` destruction. Its RTTI, catch type and exception ABI are host interfaces.
Assignment failure precedes temporary arming, as in the original.

The common allocator uses actual host malloc/new-handler/throw behavior. It
cannot return null successfully; the original node allocator's null branch
is preserved but is not a claimed reachable host-domain test case. Node
initialization cannot throw a C++ exception on valid storage, so no invented
constructor callback or allocation rollback is introduced.

The implementation and original ABI are in
`include/bsp/native_hardware_layout_tree_insert.hpp` and
`src/native_hardware_layout_tree_insert.cpp`. Static evidence and current
validation status are in `reports/native_hardware_layout_tree_insert_audit.json`.
The independent fixture in `NATIVE_HARDWARE_LAYOUT_TREE_INSERT_FIXTURE.md`
found no production defect and matched 8,582 behavior words, 56 output stores,
and five actual CRT throw/rethrow events against a frozen primary library.
The primary review rechecked fourteen source pins, 23 fixture pins, sixteen
library providers, and all 46 live/PE spans and loaded postimages. Five full
reconstruction entries and saved Ghidra annotations now accompany the source;
the former xlen-only name at B2F1B0 was corrected to identify the complete
linking routine. Existing comments were preserved and exports refreshed.
Tree startup, the complete hardware-layout factory, arbitrary invalid graphs,
concurrent mutation, binary replacement and game behavior remain unvalidated.
