# Native resource-instance mesh bone binding

Addresses: 00B87E80, 00B87CE0, 00B931B0, 00B72730, 00B90E30, 00B90630, 00B901C0, 00B90600, 00B90620, 00B76510, 00B72B20, 00B6DC20, 00427D10, 00B91220

The complete B87E80 postprocessor now operates on actual resource-instance, bone-model, mesh, node and stream storage. It binds mesh weight names to instance nodes and accumulates per-bone AABBs from vertex positions and float bone indices. Its source is [native_resource_instance_bones.cpp](../src/native_resource_instance_bones.cpp), with required borrowed services in [the header](../include/bsp/native_resource_instance_bones.hpp). Names are descriptive hypotheses, not recovered symbols. This does not yet implement the complete B891A0 graph builder or its B79BC0 postprocessor.

## Behavior and storage

B87CE0 reads the current 01090468 token and uses the existing unsigned tree lower-bound algorithm to capture a checked `{owner,node}` iterator and pair count. B87E80 reads the first word of each eight-byte `{node,item}` pair as the model, validating current vector storage while retaining the captured outer count.

The mesh's actual `+B0` weight-name **header** is captured. Its count is read for the bone-array resize, read again after resize, and re-read after every name iteration. Its data pointer is re-read for each name. Node count and backing also remain current. Equal lengths and the required current CRT case-insensitive comparison select the first matching node; empty names match directly. The borrowed pointer is stored before the type predicate. Newly grown slots are zero, while an unmatched name preserves a preexisting slot.

The predicate's table cell is captured before the current model type-token read and loaded afterward. A false predicate resets the six bounds words to the exact `+1.0e10f` and `-1.0e10f` constants. The geometry is reacquired after name binding. Stream count is captured; declaration access, semantic presence and semantic offset queries retain their native ordering. Streams with usage 2, occurrence 0 map with `(0,0,1)`.

After mapping, the current table is captured **before** stride is read; the count target is then fetched from that table and called. Bone indices use `CVTTSS2SI` on the captured mapped cursor, advancing by the captured stride. Position reads use the existing reader's current stream metadata. These can deliberately differ after a callback. Each bone's current `+4C` service supplies a six-float AABB, which 427D10 expands before six ordered stores to node `+18..+2C`. Current `+14` unmaps even when the captured vertex count is zero or negative. A missing semantic skips mapping altogether.

No owner, reference count, graph, declaration, backing array or mapping is replaced by a shadow object. Known B48CE0/B48CD0/B49980/B49A80 slots dispatch the existing concrete raw declaration/count/map/unmap functions; B6DC20 dispatches the recovered raw bounds getter. Other targets require complete actual services. Source exceptions preserve preceding publications and a reached mapping; there is no invented rollback or automatic unmap.

## Original boundaries and reuse

| Native entry | Coverage and ABI |
| --- | --- |
| B87E80 | Complete 764-byte caller; ECX instance, RET. Source EDX supplies context. |
| B87CE0 | Complete 186-byte range wrapper; ECX output, instance stacked, EAX output, RET 4. Source EDX supplies the live token cell. |
| B931B0 | Complete 6-byte current-token getter; original has no inputs. Source receives the actual cell. |
| B72730 | Complete 7-byte mesh `+B0` address getter; ECX mesh, EAX header, RET. |
| B90E30 | Complete 11-byte model `+184` adapter to the reused pointer-array resize; original ECX model, signed count stacked, RET 4. New C++ adapter interface. |
| B901C0 / B90630 | Reuse of B1C500/B1C770. Their complete 95/80-byte original bodies match after normalizing direct call relocations; allocator/free targets and the reserve correspondence are verified. No new array algorithm. |
| B90600 / B90620 | Complete 20/16-byte borrowed bone-pointer store/load at `[model+184][index]`; ECX model, stacked arguments, RET 8/4. EDX unused in source. |
| B76510 | Complete 19-byte current node-count getter; ECX instance, signed arithmetic shift of pointer difference, RET. |
| B72B20 | Complete 4-byte signed mesh stream-count getter at `+7C`; new C++ reference interface. |
| B6DC20 | Complete 42-byte six-lane x87 bounds copy; ECX node, output stacked, EAX output, RET 4. Newly defined in Ghidra without clearing or recreating neighboring functions. |
| 427D10 | Complete 191-byte six-lane x87-staged AABB expansion with conditional SSE stores; ECX box, point stacked, EAX box, RET 4. |

These thirteen complete bodies total 1,441 bytes, including the two reused array specializations. The probe's larger twenty-span reference corpus totals 1,669 bytes. The 105-byte B783E0 lookup is reused through existing generic tree mechanics inside the range wrapper; no separate new STL implementation is claimed.

The B91220 producer calls B75030, installs D63590, and initializes the distinct bone array at `+184/+188/+18C`. Its complete 45-byte body was inspected and byte-checked, but its constructor/factory ownership is not reconstructed here. This layout must not be confused with an ordinary generated model's pool index at `+184`, or the separate skin-model records of stride `60h`.

B6DC20 and 427D10 retain the exact x87 load/store and comparison schedules. The production function bytes are identical to their originals. This preserves alias ordering, NaN quieting/comparison behavior and signed-zero choices under the tested floating-point environment. It is not a replacement with `std::min`, `std::max`, or a matrix operation.

## Validation and limits

The strict Win32 build and both existing CTests pass. Eight native seeds and all 31 direct/tail call-site rows pass current Ghidra/PE checks. One focused ignored probe compares six full original-byte caller scenarios, 32 floating-point/alias variants and four source failure cases; normalized results are stable across repeated processes. It compares native owner and live allocation images, allocation/free order and preimages, predicate/name/provider events, and x87/SSE status. No repository test suite was added.

The caller scenarios cover a missing typed key, first duplicate and empty-name matching, preservation of a missing old bone, resizing, two model pairs, callback changes to names and instance-node storage, current type/predicate changes, absent semantics, nonpositive vertex counts, and derived map/count/bounds services. A masked NaN bone index also checks `CVTTSS2SI`'s integer-indefinite result and native scaled-address wrapping. The failure cases check allocation failure, a throwing predicate after pointer publication, a throwing bounds provider after mapping, and the existing reader's diagnostic boundary for native uninitialized scratch.

The reference shares the existing production position reader and canonical stream services; those helpers are not independently re-proven by this caller comparison. Its CPU mapping domain does not exercise physical D3D buffer or renderer synchronization branches. Prepared postconstruction images are grounded in verified producers and use the actual AP constructor/AQ publication helpers, but do not establish complete graph factories or populated executable admission. Malformed storage, original CRT/FH3/SEH delivery, unmasked floating traps, unsupported native uninitialized scratch and gameplay parity remain outside this validation.

The prior four AO/AP stored-body extensions still require enabled Ghidra scripts. Their immutable archives are unchanged. No worker was dispatched for this batch.
