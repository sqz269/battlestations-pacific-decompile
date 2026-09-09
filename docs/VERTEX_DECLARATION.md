# Vertex declaration records

`vertex_declaration.cpp` reconstructs seven declaration routines. The object has
a packed-offset cursor at `+8h`, a flat array at `+ch`, fifteen per-usage arrays at
`+18h`, and stride at `+cch`. Each record contains five DWORDs: offset, type, method,
usage and an index/sentinel. This is not the packed D3DVERTEXELEMENT9 structure.
The new interface uses vectors and bounds checks rather than the original allocator.

Append `00b48330` accepts type, usage and offset with RET Ch. Offset -1 uses the
packed cursor. The record initializes method to zero and the final DWORD to -1,
then is copied into both the flat list and the corresponding usage list. Cursor
advances by type size even for an explicit offset. Stride recomputation `00b47d20`
sums all element sizes, independent of offsets. The 17-entry size table at
`00d61cc0` is retained exactly. The later purpose/assignment of the final sentinel
remains unconfirmed.

Semantic existence/index routines `00b47c90`/`00b47ce0` scan the flat list for the
requested occurrence of a usage. Offset/type/size getters `00b47c40`/`00b47c20`/
`00b47c60` directly index the per-usage list. The existence routine's truth value
is AL; the new interface returns bool and does not preserve incidental high EAX
bits. Invalid type, usage and getter occurrence inputs abort under the new API's
bounds preconditions; the native routines have unchecked accesses.

The logical vertex-stream base constructor `00b61e20` reads declaration stride,
assigns an increasing stream ID, initializes tag `40000000h` and caches selected
semantic metadata (position, normal, texture coordinate and four-byte color).
It is annotated but not ported; full declaration/stream construction and shader
declaration translation remain outstanding.

The existing diagnostic probe builds FLOAT4/POSITIONT and D3DCOLOR/COLOR elements,
checks stride 20, color offset 16, size 4 and flat index 1, then uses that declaration
in the reconstructed logical stream. Non-indexed and indexed pixel readbacks both
pass. This covers packed append and those lookups. Explicit-offset behavior is
assembly-grounded but has no runtime differential check. No new test cases were added.

The hardware conversion is now recovered separately in `HARDWARE_VERTEX_LAYOUT.md`;
the probe uses CreateVertexDeclaration/SetVertexDeclaration instead of SetFVF.
Shader input linkage, native declaration allocator/ABI and gameplay remain unverified.
Next, connect the native logical stream constructor and shared-buffer allocation path.
