# Resource graph dependency analysis and closeout

Addresses: 00B891A0, 00B79BC0, 00B87E80, 00B87CE0, 00B783E0, 00B931B0, 00B72730, 00B90E30, 00B90630, 00B901C0, 00B90600, 00B90620, 00B76510, 00427D10

This batch preserves the graph dependency analysis already underway and repairs four listing gaps. It adds no reconstructed C++ and dispatches no workers. The eight new descriptive names are hypotheses, not recovered symbols. The existing graph builder and its two postprocessors remain dependencies for populated resource-instance admission.

Twelve reviewed function spans, totaling 1,500 bytes, match the live Ghidra database and the unchanged original PE. [The report](../reports/native_graph_builder_ar.json) records their hashes, exact direct-call sites, indirect sites, constants, and remaining limits. The prior source implementations and their immutable proof archives are documented in [AO](NATIVE_UNIT_PART_DESTRUCTION_AO.md), [AP](NATIVE_MODEL_ADMISSION_AP.md), and [AQ](NATIVE_MODEL_GRAPH_AQ.md).

## Mesh bone binding and bounds: 00B87E80

The complete 764-byte body uses ECX for the actual resource instance and returns with `RET`. It first constructs a 16-byte range view through B87CE0. This captures the instance, a checked map iterator `{owner,node}`, and the mapped pair count. The outer ordinal is compared unsigned against that captured count. Each iteration validates the current owner/sentinel and current mapped vector bounds, then loads the first word of its eight-byte `{node,item}` record as the model. It does not enumerate all instance nodes as outer work items.

1. Call B74640(model, 0), then B72730 to obtain the mesh's actual `+B0` weight-name array. Capture its data and signed count; resize the model's bone-pointer array through B90E30.
2. For every weight name, search instance nodes in increasing index order. Re-read the node count through B76510 on every iteration and validate the current pointer vector. B6D800 supplies the candidate's string header. Equal lengths are required; empty names match directly, while nonempty names use the case-insensitive CRT comparison BF7FBF. The first match wins. Missing names leave the slot produced by resizing; newly grown slots are zero, while existing slots are not cleared merely by resizing to the same count.
3. B90600 stores the matching node as a borrowed pointer. Capture the candidate's virtual slot address before B74310 reads its type token; load and dispatch slot `+0C` after that getter. If the predicate returns false, reset the candidate's minima at `+18/+1C/+20` to `1.0e10f` and maxima at `+24/+28/+2C` to `-1.0e10f`. These exact float bits come from CE4970 (`501502F9`) and CE4ADC (`D01502F9`). This branch is expressed by its actual predicate; it is not an unconditional reset of every matched node.
4. Reacquire B74640(model, 0). If nonnull, capture its signed stream count from B72B20 and fetch each stream through B73260. Call current stream slot `+24` for its declaration, then B47C90(declaration, usage 2, occurrence 0). The two arguments are pushed before the virtual accessor but are consumed by B47C90. When present, reacquire the declaration and obtain the byte offset through B47C40.
5. Capture stream slot `+10` and map with `(0,0,1)`. After mapping, capture the current stride at `+0C`, then call current slot `+20` for the signed vertex count. The bone-index cursor begins at mapped base plus the declaration offset.
6. Convert each cursor float with SSE `CVTTSS2SI`, obtain the bone node through B90620, and call that node's current slot `+4C` to read its six-float AABB. Call 4768D0 to read the vertex position, expand the box through 427D10, advance the cursor by the captured stride, and copy six float words back to node `+18..+2C` in order.
7. Reacquire current stream slot `+14` and unmap. This also happens when a mapped stream has a nonpositive vertex count. Streams without the semantic are never mapped. Continue against the captured stream count, then the captured outer pair count.

The `+4C` result is an AABB, not a transform matrix. The routine has no local exception-cleanup frame: earlier publications and a reached stream lock are not automatically rolled back on failure. Invalid bone indices and unresolved bone pointers do not gain invented null acceptance. Captured pointers, callback order, current virtual tables, and the native floating-point environment all matter to a future implementation.

## Concrete helper contracts

| Entry | Assembly-established behavior |
| --- | --- |
| B87CE0 | ECX output view, stacked instance, EAX output, RET 4. Read the current B931B0 token, query instance tree `+30` through B783E0, capture `{owner,node}`, validate its relationship to the current head, and capture `(mapped_end-mapped_begin) >> 3`, or zero for end/null data. |
| B783E0 | Exact lookup by unsigned key using a lower-bound tree search; output is the checked `{owner,node}` iterator. This is a library helper, not a new game reconstruction. |
| B931B0 | Return the current DWORD at 01090468. The database's initial zero does not establish the initialized runtime token or its registration path. |
| B72730 | ECX actual mesh, EAX `mesh+B0`, RET. This agrees with `NativeMeshStorage::weight_names_b0`. |
| B90E30 | Add `184h` to ECX and tail-jump to B90630. The stacked count and RET 4 belong to the target. |
| B90630 | Signed resize of a `{data,count,capacity}` pointer array. Reserve if required; zero newly grown cells; decrement the current count while shrinking; publish the requested count. No retain/release operations. |
| B901C0 | Clamp requested capacity to at least one. Allocate `capacity*4` through BF55BE, copy current pointer cells in order with the native destination-null test, free old data through BF6989, then publish new data and capacity. The nine-byte post-free gap contained this publication. |
| B90600 | ECX model, stacked index and node, RET 8. Store the borrowed node at `[model+184][index]` with stride four. |
| B90620 | Existing named getter: ECX model, stacked index, RET 4. Read the same stride-four bone-pointer array without bounds validation. |
| B76510 | ECX instance, RET. Return zero for null `+14`; otherwise arithmetic-shift the current `(+18)-(+14)` byte difference by two. |
| 427D10 | ECX six-float box, stacked point pointer, EAX original box, RET 4. Process min x/y/z and then max x/y/z, with an x87-staged source read and comparison for every lane, followed by conditional SSE copies. |

427D10 is a complete 191-byte body. It re-reads the source for each min/max lane and uses the incoming stack argument word as float scratch after capturing the pointer in EDX. Its `FCOMIP`/`JBE` decisions preserve unordered-comparison behavior. NaNs, signed zeros, denormals, exceptions and alias timing must be checked before replacing it with ordinary C++ comparisons or `min`/`max`.

Existing source provides the actual mesh/name storage and the position reader `read_native_vertex_position_004768d0`. The latter preserves format-specific behavior and reads current stream metadata without adding map/unmap calls. The raw pointer-array reserve/resize routines at B1C500/B1C770 are reuse candidates; this batch does not establish full specialization equivalence. An ordinary model's pool index at `+184` must not be overlaid with the distinct bone-bearing model's pointer array.

## Graph and factory boundary

AQ already established that B891A0 publishes the first constructed node to instance `+0C`; only an empty graph publishes null. The 24 bytes at D63210 confirm three destructor/create pairs: `{B87180,B866C0}`, `{B871B0,B86720}`, and `{B871E0,B86780}`. The graph's default selection uses D63218, while record flag `+58` bit 0 selects D63220. An associated item's factory can replace the default. The canonical model-base factory B86720 already exists in source; the other factory contracts and the complete caller still need recovery and integration.

B79BC0 is a separate 3,613-byte postprocessor. Its three known post-free listing gaps were repaired, but its full semantic contract was not recovered in this batch. B891A0, B79BC0 and B87E80 are not complete C++ implementations. Full dispatch bindings, nonempty graph admission, the class `+50` producer, original exception ABI and required gameplay validation remain open.

## Restart and cleanup evidence

The restarted bridge opened the correct saved project and program. The normal locked flow-repair tool decoded four call-site gaps totaling 43 bytes: three in B79BC0 and one in B901C0. Readback found no remaining call gaps in those functions. Branch-skipped alignment bytes were left alone. Prior call-site values and responses are preserved in [the flow receipt](../reports/native_graph_builder_flow_ar.json), and affected exports were refreshed after saving.

A read-only request to the supported inline-script endpoint still returned `Script execution disabled`, requiring `GHIDRA_MCP_ALLOW_SCRIPTS=1`. No setting was changed and no script guard was bypassed. Consequently, these earlier stored-body extensions remain pending:

| Function | Current stored end | Previously verified physical end |
| --- | --- | --- |
| 00712C80 | 00712D7C | 00712F18 |
| 007112E0 | 00711357 | 0071136B |
| 00711120 | 00711131 | 00711136 |
| 00B89DB0 | 00B89E32 | 00B89E8E |

The earlier AO/AP/AQ proof archives remain tied to their original inputs and are unchanged. This batch supplies static byte/listing evidence and saved analysis; it does not add a native differential or gameplay validation claim. No new worker packets were dispatched.
