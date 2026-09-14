# Resource-instance publication and graph construction

Addresses: 00B89E90, 0071B710, 00B891A0, 007137F0, 00B896A0, 00B89150.

AQ implements the complete **82-byte B89E90** and **242-byte71B710** publication callers. Their actual instance storage is constructed by AP. It also records the graph builder's first-node root publication from assembly. This closes a dependency of model admission; it does not supply the complete graph builder or populate the executable's raw unit parts.

## Publication layout and ordering

Both original callers use ECX for the instance, stack arguments `(item,node)`, and RET8. The source entry adds an explicit context in EDX. Item slots are resolved from each captured actual profile and dispatched through required complete bindings.

| Instance field | Stored values | Producer |
| --- | --- | --- |
| +20 checked vector | `{node,item}` | B89E90, before any item dispatch |
| +30 type-token tree | unsigned item slot8 result -> checked vector of `{node,item}` | B89E90, after the first append |
| +3C checked vector | `{item,node}` when current E19BE4 token matches | 71B710 |
| +4C checked vector | `{item,node}` when current E19A98 token matches | 71B710 |
| +5C checked vector | `{item,node}` when current E19B74 token matches | 71B710 |
| +6C checked vector | `{item,node}` when current E19B64 token matches | 71B710 |
| +7C checked vector | `{item,node}` when current E19B54 token matches | 71B710 |

71B710 invokes the base publication first and tests **all five** predicates independently. It reacquires the current item profile before each token read, then resolves slotC after that read. A callback may alter later profiles or token cells. No caller retains or releases the borrowed item/node. Allocation or callback failure keeps every earlier publication; no rollback is added. The post-base null test does not make null valid for B89E90, which dereferences the captured item after its initial append.

The actual checked headers retain opaque+0 and use begin+4/end+8/capacity-end+C. Eight-byte records grow by the original 1.5x element-count rule. The source adds a non-owning two-DWORD case to existing checked-vector mechanics and uses the existing parameterized tree algorithms. It does not create a separate general STL implementation. The map has24h nodes: links0/4/8, unsigned keyC, mapped checked header10, color20 and nil21. Insertion of a missing key copies the actual freshly empty default vector, retaining proxy10 and padding22/23. B896A0 checks count against0CCCCCCBh, allocates/links/rebalances, then publishes its iterator; its former throw-only name described just one branch.

The source storage domain requires consistent initialized storage, native-valid element counts and nonoverlapping reached copies. Allocation services may fail but must not structurally mutate the containers. General original iterator/insertion ABI, private native stack aliases, malformed returning-handler continuation, native FH3 and fault timing are not supplied.

## Graph-builder evidence

CFD8CC slot8 reaches7137F0 and thenB891A0. B891A0 first calls resource slot10, which AP resolved to71AED0 for this concrete profile. It resizes the new instance's node-pointer vector at+10 from resource count+20, selects a node factory from each record's associated items, builds a name (including the unnamed `<node N>` path), creates the node and sets its local transform. Record flag58 bit0 overrides the selected factory. Parent indices are resolved against the actual node vector; a negative index on subsequent nodes uses the first constructed node.

**The graph root is the first constructed node.** B894F4 tests the node index, B894FB saves that node into the root local, and B89600..B89606 publishes the saved pointer to instance+0C. The local starts at zero, so the empty graph publishes null. The decompiler's apparent unconditional zero assignment is incorrect. Generated-model bounds use the six original x87 FLD/FSTP transfers atB8955C..B89580; these are not integer copies in a future source reconstruction.

After all nodes exist, B891A0 invokes each associated item's slot18 with the instance, source node record, constructed node and the first incoming stack word. It then calls **B79BC0** and **B87E80** for substantial animation and geometry post-processing. These complete dependencies, factory bindings and source node admission remain required. The meaning of the forwarded creation word is not inferred from the broken pseudocode. AQ does not implement B891A0 or7137F0, claim all item constructors, or prove a live class+50 writer.

## Validation and evidence limits

The isolated comparison retains **81 original reference spans /10,233 bytes**, including both callers and their storage dependency chain. Three paired sequences each publish12 items: base-only unsigned/repeated type keys; independent game type matches; and token/profile mutation during predicates. Actual CRT allocation sizes/order, initialized preimages, freed-block preimages, all live storage, tree shape/colors, pair order and dispatch observations agree. Repeats produce the same normalized result bytes. Required fixture item type services are complete for their defined borrowed objects; they do not stand in for unimplemented game resource or node destruction.

Four source cases fail the initial pair allocation, map-node allocation, mapped-vector allocation, or third predicate. The assertions check the exact retained prefix. Native FH3 handlers and native error paths are guarded and unreached; not all generic STL branches in the retained spans are exercised. Fixture teardown frees retained test allocations only after observation and is not a game destructor test.

All11 direct calls in the new callers and209 reference-provider call rows pass live ownership/callee checks. Six additional reference rethrow sites lie beyond their stored catch-function bodies and are recorded separately. Two repaired provider listings preserve their original bytes. All8 native seed checks, the strict Win32 build and both existing CTests pass. The compiled review checks pair order, base-before-predicate ordering, current table/token reads, all five independent appends, unsigned tree search, empty mapped-node fields and the shared raw-pair copy/no-destruction case.

The full game remains incomplete: graph construction/post-processing, executable model/class+50 admission, complete required external item behavior, original exception ABI and gameplay parity remain open.

## Integrated validation

Commit `72841e144d45eaf3c65ac77c34ec149a3b712cd5` passes the strict Win32 build and both existing CTests. All three paired12-publication sequences and four source failure cases pass against that library with repeat-stable normalized images. The 120-frame USN01 compatibility run passes trajectory, avoidance, generic-tick, participant, world-list and observer/pending-owner checks. The executable SHA-256 is `5600d1e04fdedf9ab548f5ffb7d402779e5de164b202ebec41c034624dfa765a`. An immutable manifest retains 463 inputs, 289 artifacts and 16 linked production objects, including compiler/header/library inputs, original reference bytes, saved Ghidra receipts and mission artifacts. Six reference catch-tail sites remain unowned. Full graph construction and post-processing, complete external item behavior, class+50 admission, native FH3 and gameplay remain open.
