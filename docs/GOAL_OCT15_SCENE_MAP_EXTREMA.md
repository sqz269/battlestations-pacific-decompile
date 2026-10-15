# Actual scene-map extremal node leaves

Addresses: `00466A00`, `00466A20`.

The new ordinary C++ in `src/native_scene_record_map_extrema.cpp` implements
both complete functional leaves directly over actual node storage. Neither
native body has a callee or provider dependency, and the current index/source
search had no earlier reconstruction. There are no callbacks, node copies,
owner conversions, allocator substitutes or STL replacements.

| Preserved native library name | Source interpretation | Executable coverage | Full byte envelope |
| --- | --- | --- | --- |
| `STL_Tree_Max_00466a00` | maximum through right+8 | complete: `00466A00..00466A0C`, `00466A10..00466A1B`; 25 bytes | 28 bytes; excludes unreachable alignment `00466A0D..00466A0F` from functional coverage |
| `STL_Tree_Min_00466a20` | minimum through left+0 | complete: `00466A20..00466A3A`; 27 bytes | 27 bytes, includes reachable six-byte neutral `LEA EBX,[EBX]` at `00466A2A..00466A2F` |

The totals are 52 executable native bytes in 55 bytes of envelopes. Of the
52, six are the reachable min alignment instruction with no semantic effect;
46 bytes perform functional operations. The max gap's original `8D 49 00`
encoding is retained as envelope evidence but is skipped by the native branch
at `00466A0B`, and by the initial child-sentinel branch. It is not reconstructed
execution. No Ghidra listing repair or binary alignment identity is claimed.

The existing correct library names remain unchanged. The descriptive Source
interpretations are body/layout/caller-derived hypotheses, not recovered
original symbols. The APIs are:

```cpp
void* __fastcall maximum_native_scene_record_map_node_00466a00(void* actual_node);
void* __fastcall minimum_native_scene_record_map_node_00466a20(void* actual_node);
```

Native ECX is the initial node, EDX is scratch, there are no stacked arguments,
and plain `RET` returns the current node in EAX. Min's neutral LEA preserves
EBX. The current `void` pseudocode fails to expose this result, but both callers
consume EAX. This packet documents that mismatch and supplies the actual Source
result; it does not change a live Ghidra prototype or claim recovered ABI types.

The established node constructor/insertion and lookup evidence from the same
continuing stream supplies actual left+0, right+8 and sentinel+19 fields, as
documented in `docs/SCENE_RECORD_MAP.md`. These leaves only read those fields.
There is no new node/owner structure or mapped-type inference.

The exact traversal matters:

1. Capture the initial node value. Read its selected child first (right+8 for
   max, left+0 for min), then read that child's sentinel byte+19h.
2. If that byte is nonzero, return the original node immediately.
3. Otherwise advance the current node to the captured child, read CURRENT
   selected child, read its sentinel, and repeat while zero.
4. Return the final current node, not the terminal sentinel child.

No initial-node sentinel flag is read. A sentinel input is not automatically
returned: its selected child still governs descent. The Source therefore does
not narrow to positive/non-sentinel inputs even though current callers guard
theirs. Volatile raw DWORD and byte reads retain the exact external order.
There are no writes, length/key checks, cycle guards, head access or sentinel
normalization. Zero pointers receive no added guard; every taken access needs
valid backing. Cyclic child chains are not converted into an error or a result.

All live caller sites are in the containing `0046A470` body. The min call at
`0046A550` sets ECX=EDI and uses EAX for the following head-left store at
`0046A555`; the max call at `0046A572` sets ECX=EDI and stores EAX into
head-right at `0046A577`. Neither passes stack arguments. Their preceding
sentinel checks select whether to call these leaves; that caller precondition
does not add an initial-node check to the whole leaf bodies. The full native
listings, exact argument/result contexts and containing boundaries were read.

The companion report retains original bytes, envelope/executable distinctions,
hashes, both exact CALL rows, old names/comments and validation state. Every
live client verifies project `bsp`, `/battlestationspacific.exe`, x86 language
and image base against configured `C:/Users/sqz269/bsp.gpr`. Evidence annotation
uses the write lock, preserves comments and native names, records prior values,
saves the project and refreshes both exports. No live prototype repair occurs.

Compilation and existing checks do not establish direct original-byte
execution. No new routine tests are added. Original binary ABI, private-frame
and instruction-fault identity, application adoption and game behavior remain
unvalidated; nominal mapped-type uncertainty is unaffected.
