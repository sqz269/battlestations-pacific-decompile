# MatrixIndexedModel type predicate exposure (Source only)

The existing B91250 predicate now has one shared public Source interface:

    std::uint8_t native_matrix_indexed_model_is_type_00b91250(std::uint32_t token,
        const volatile std::uint32_t* actual_type_ids_010903c8);

It compares the actual current canonical DWORDs C8, CC, D0, D4 in order and
returns 0 or 1 immediately on the first match or after four misses. Every token
bit pattern is valid. Zero or repeated cell values remain meaningful; there is
no nonzero, distinctness, or prior-initialization condition and no new guard.
The caller supplies the genuine live cells returned by the existing canonical
accessor, retains their valid storage/lifetime, and serializes mutation/access.
Volatile is not synchronization. No cell snapshot or speculative four-load
operation replaces the ordered scalar reads.

The private matrix_is_type callback keeps its exact binding-context conversion
and require_matrix_slot(owner, 0x0c, 0x00b91250) before delegating with != 0.
The canonical producer, owner, lifetime, layout, and scene behavior are unchanged.

The complete Original B91250 body is 12 instructions/40 bytes: stack+4 token,
four current-cell comparisons, false XOR AL,AL, true MOV AL,1, and RET 4.
Only AL is semantic output. The old pseudocode/ledger fallback-pointer wording
is misleading EAX residue; metadata remains unchanged. The pointer parameter is
a qualified private C++ Source ABI projection. Original physical ABI, upper EAX,
fault/SEH behavior, and binary replacement compatibility are not established.

This exposes an existing game-owned body: zero new Native/vendor body credit.
It does not bind the derived family or establish Mission/frontend/runtime
readiness. No CMake, dependencies, or tests changed. Compiler, build, link, test,
probe, game entry, and Ghidra/ledger mutation were not authorized or run here.
