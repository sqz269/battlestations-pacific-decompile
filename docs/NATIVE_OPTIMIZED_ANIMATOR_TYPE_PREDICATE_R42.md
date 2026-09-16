# Optimized animator type predicate

`00B782D0..00B782F7` searches the current three DWORD type cells at
`010900FC`, `01090100`, and `01090104`. It compares each cell with the stacked
type ID, returns AL=1 at the first match or AL=0 after the third mismatch,
and pops the four-byte argument. The incoming ECX owner is unused.

The raw provider now lives beside the existing type-ID getter in
`native_material_constant_metadata.hpp/.cpp`. Its body is adopted from
`a6b2884f2:src/native_material_constant_build_leaves.cpp`. The source interface
borrows the same contiguous cells in EDX and calculates their end address.
It reads each cell when reached; it does not cache IDs, inspect the owner,
allocate, or invoke a fallback. Only the byte result is specified. The added
EDX input and changed upper EAX bits make this a source bridge, not a drop-in
native ABI replacement.

This closes one leaf needed by the pending raw material constant builder.
It neither adopts that builder nor activates the renderer. Existing metadata
providers remain separate and are reused by the later composition.

Exact byte, build, and focused differential evidence is recorded in
`reports/native_optimized_animator_type_predicate_r42.json`. No full parent,
FH3/SEH, renderer, or gameplay validation is implied.
