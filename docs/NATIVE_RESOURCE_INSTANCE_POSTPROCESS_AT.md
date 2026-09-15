# Resource-instance animator, skin and camera postprocessing

Addresses: 00B79BC0, 00B78750, 00B78810, 00B788D0, 00B78990, 00B922C0, 00B8A040, 00B8A120, 00B8A1C0, 00B6EE80, 00B75F00, 00B87200, 00B87260

The complete 3,613-byte B79BC0 listing establishes the next required graph dependency. It remains **analyzed, not reconstructed**. Four complete ownership/navigation helpers are implemented in `native_resource_instance_postprocess.cpp`; four typed-range specializations and their four token getters reuse the existing AS source. Those twelve complete helpers total 929 native bytes. Descriptive names are hypotheses, not recovered symbols.

## Complete helpers

| Entry | Original ABI and behavior |
| --- | --- |
| B6EE80, 65 bytes | ECX node, stacked animator, RET4. If different from current node+130, publish incoming first, increment incoming+4, decrement old+4, and call the old object's current slot zero only on zero. A destructor's mutations remain visible. Source EDX carries the borrowed lifetime service. |
| B75F00, 24 bytes | ECX node, EDX candidate ancestor, AL result, RET. Starts at node+30, excludes self, follows +30 until match/null. No cycle guard or ownership operation. The mnemonic source compiles byte-identically. |
| B87200, 19 bytes | ECX instance, EAX arithmetic pair count, RET. Null instance+24 gives zero; otherwise arithmetic shift of (+28 minus +24) by three. This is the base pair vector, not the node vector. |
| B87260, 53 bytes | ECX instance, stacked unsigned index, EAX pair.item, RET4. Validate the captured base and arithmetic count, then return its second word. If the invalid-parameter handler returns, reload current instance+24 before reading. Source EDX is unused. |

The B6EE80 source operates on actual storage and real interlocked operations. The borrowed service resolves the same current table and performs the complete destructor; it cannot substitute a no-op or shadow owner. Original numeric tables and actual host function tables require an explicit binding. No animator constructor/destructor is claimed by this setter alone.

The four 186-byte range bodies match B87CE0 exactly after normalizing direct-call relocations. All calls after the first type getter have the same targets. Each six-byte getter is `MOV EAX,[current cell]; RET`, identical to B931B0 apart from that cell. Reuse `get_native_mesh_binding_range_00b87ce0` and its current-cell reader with these cells:

| Range | Getter | Current cell | Consumer evidence |
| --- | --- | --- | --- |
| B78750 | B922C0 | 0109042C | B925D0 publishes names from 18h records; the per-node 38h animator borrows the first matching item at +34. Compact-track interpretation is provisional. |
| B78810 | B8A040 | 01090268 | B8A330 publishes track names from an item pointer array; names select slots in the shared registry for per-node 30h animators. |
| B788D0 | B8A120 | 01090278 | Selects descendant bindings used to populate a skin model's 60h records. |
| B78990 | B8A1C0 | 01090288 | Pair.node receives FOV/aspect from item+8/+C and an optional named target at node+438. |

The view captures instance, tree owner, tree node and pair count. Its vector and current backing remain live. Supplying a previously cached token or replacing the tree with a host container changes the contract.

## Complete B79BC0 control-flow analysis

Original ECX is the resource instance; no stack arguments, RET, no established return value. Native FH3 registration and states 0 through 4 surround allocations and temporary arrays. The exported pseudocode removes reachable paths and does not establish its contract; the assembly is authoritative. The three repaired post-free tails from AR are present. The branch-skipped nine-byte alignment gap at B7A167 is not executable fall-through.

1. **Compact-track group, B79BE7..B79ED6.** Capture the B78750 range. If nonempty, allocate a 20h shared registry through BF681B/B79A80, and publish each item through B925D0. Enumerate the instance's current node array, rechecking its signed count. Allocate a 38h animator per node. A current type predicate using B8F920 sets byte +30 to the inverse result; the first pair whose node matches supplies borrowed item+34. Attach through B6EE80, publish/retain the shared registry at animator+2C, release the temporary animator reference, and finally release the registry's local reference. Zero-count ranges skip this entire group.
2. **Track group, B79ED8..B7A30D.** Capture B78810. Only a nonempty range enters this block, including the later skin/transform work. Allocate another 20h registry. Resize a temporary array of 10h records to the current node count and copy node pointers into each record's first word. For every typed pair, append its item to the first matching record's +4/+8/+C pointer list, then publish its track names to the registry with B8A330. For each temporary node record, allocate a 30h animator; resize and explicitly zero its +20 track slots to registry+10 count; attach it and assign/retain registry+2C. For every item and current item-track index, look up the track name in that same registry and store the track pointer into the node's **current** animator. A missing key becomes index -1 before the raw scaled store; there is no synthesized missing-key skip.
3. **Skin or static-transform work, B7A30F..B7A6FB.** Capture B788D0 after track publication. For each temporary node, test its type using B8F920. For a positive result, collect pairs whose node's parent is this model or has this model as an ancestor. Resize the model's skin records with B91590, acquire geometry zero and its actual weight-name header, then enumerate all base instance items through B87200/B87260. A B8A120 type match must also match a collected item by identity. Search current weight names for equal length and case-insensitive content; only an empty destination record is initialized through B90C30. Arguments are the weight-name index, collected node, item+10 vector, item+1C vector, and x87-returned item+28 scalar. B90C30 uses 60h record strides at model+184, retains its first node field, reacquires current backing after release, and stores two x87-copied float3 values plus the captured scalar. B91000 finalizes the skin model. These records are distinct from AS's four-byte bone pointer array at the same model offset. If the type predicate is false and the node has an animator, copy the current local matrix, publish translation to animator+8..10, call B630F0 on the copied matrix, then copy three x87 results into animator+14..1C. The full B630F0/B91000 implementations remain dependencies.
4. **Finalize animators, B7A744..B7A78C.** Enumerate the instance's current nodes again, rechecking count, and invoke each present animator's current slot +20. This happens whether either typed animation range was empty.
5. **Camera targets, B7A78E..B7A9DC.** Capture B78990. For each pair, construct and dispose a temporary empty native string using the real sized pool. If item+10/+14 is nonempty, find the first equal-length, case-insensitive node name. On a match, release the old camera+438 target **before clearing and assigning** the new one, then retain incoming. There is no same-pointer shortcut; callback timing differs from B6EE80. No match leaves the prior target unchanged. Finally load item+8 via x87 and call B6FBB0, then load the current item+C via x87 and call B6FBD0. Recheck the captured range count for the next pair.

Allocation failures, checked-container errors, native FH3 cleanup and native malformed-storage behavior cannot be inferred from an empty-input success. No placeholder caller is added.

## Validation and remaining work

The strict Win32 build and both existing CTests pass. The unchanged original PE and live Ghidra bytes agree for fifteen spans totaling 4,734 bytes; all 127 direct/tail call rows pass the mechanical verifier. Twenty paired original-code/helper cases cover reference publication/release, same-pointer assignment, callback mutation, ancestor inclusion/exclusion, arithmetic pair counts, indexed access and returning invalid-parameter handlers that replace backing storage. One source destructor-failure case preserves prior effects. The ancestor helper is also byte-identical. The four range and four token getter equivalences are byte/relocation proofs and reuse the earlier AS fixture boundary; no additional runtime coverage is claimed for them.

Full postprocessor source still requires the registry and name publication, animator creation/finalization, native temporary-array semantics, complete skin-record finalization, B630F0 arithmetic, and the camera target sequence with its actual services. The existing B891A0 graph builder and its populated executable admission remain required. Physical mapping, original exception ABI and gameplay validation remain unproved. No worker was dispatched and no repository test suite was added.

## Retained fixture evidence

Source commit `d2fc4adc222f2d9b7172d352bc98c2bfb44fb250` is build- and fixture-tested. The immutable local manifest retains 244 inputs and 91 artifacts, including the one production object linked by the helper probe, source/header/compiler/library inputs, original reference bytes, and saved Ghidra changes. The AS manifest is retained as the prior source-reuse proof boundary. This helper batch adds no populated postprocessor runtime claim; source implementation of B79BC0 remains required.
