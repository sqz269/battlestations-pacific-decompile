# Native resource hierarchy append

| Function | Complete inclusive span | Bytes | Coverage | Original ABI |
| --- | --- | ---: | --- | --- |
| `B87AE0` | `B87AE0-B87B1A` | 59 | complete | ECX resource; one raw hierarchy-item pointer word on stack; RET4 |

The complete PE body and current read-only Ghidra bytes agree (SHA-256
`19a37acb3de69e759305a422242e3cb3e12eb8d506d701215a24b1736c62b6a5`).
The descriptive Ghidra name `BSP_Resource_AppendHierarchyItem` is a hypothesis,
already present before this packet. No Ghidra edits were made.

`B7EB90-B7EE90` produces a record with a raw hierarchy-pointer array header at
resource+1Ch. At `B7EE70-B7EE78`, it loads the resource from reader+24h into
ECX, pushes the newly produced record in EBP, and calls `B87AE0`. This is its
only direct call site. The append body reads capacity at+24h and compares count
at+20h. Only equality triggers reserve: add16 to the captured capacity with
DWORD wrap, clamp the signed result to at least16, push it, set ECX to header
resource+1Ch, and call the actual `B87350` raw hierarchy-pointer reserve. The
reserve body consumes one stack word with RET4. Its own allocation/free service
calls clean their single pushed argument through `ADD ESP,4`.

After reserve returns, append reloads current count and data from the header,
forms `data + count*4` with DWORD wrap, and stores the raw pointer word only
when the computed slot address is nonzero. It then increments the current header
count even for a zero slot address. There is no count/capacity repair, pointee
operation, AddRef, rollback, local exception handler, or pointer null guard.
Reserve exceptions propagate before the append's slot store and count increment.
The source uses the existing raw hierarchy-pointer reserve; numeric-reference
array helpers have different capacity and element contracts.

This is a complete source reconstruction of this one body with a new C++
interface. It is not a native ABI replacement. After `verify-seeds`, the strict
Win32 build passed both existing CTests (`reconstructed_math` and
`native_math_differential`). The generated project lists this source once;
source-specific compiler command/read/write records are retained under
`local/resource-hierarchy-append-bg/compiler-evidence/`. The 1,340-byte object
matches the unique member extracted from the current and copied `bsp_core.lib`
archives (SHA-256 `2e414151f68cf59d4e2c61dcf1eca952f69df69e45bd9ec4bfdd9571d7341991`).
These checks establish source integration only; no original-body fixture or
gameplay path was run.
