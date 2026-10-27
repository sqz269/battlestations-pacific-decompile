# CC13 cgretag: CG_vector_deleting_dtor / CG_array_ctor_helper re-tag

Packet cc13-cgretag (2026-10-24). The library inventory tagged every function that *called* the eh
vector destructor iterator (00BF7C6E) as `CG_vector_deleting_dtor`, and every caller of a vector
constructor iterator as `CG_array_ctor_helper`. A deleting destructor tests its flag argument (bit 2
for vector, bit 1 for scalar); none of the 38 + 15 functions still so named in Ghidra did, except
006FAAA0 and 006FAB30 (TEST BL,2 on the flag, then 00BF7C6E over the [this-4] count).

## Classification (67 targets: 38 vector, 15 array, 14 CG_static_init over 20 instructions)

| class | count | action |
|---|---|---|
| real destructor | 29 | renamed `<Class>_Destruct_<addr>` or `Unknown_*_<addr>` |
| constructor (provisional) | 9 | renamed |
| std::list::sort (0049F6A0, 005DF1A0, 007765A0) | 3 | renamed `STL_List_Sort_<addr>`, tag `stl_shape` |
| other real body (Lua/stream readers) | 3 | renamed |
| genuine helper | 9 | keep CG_ name |
| genuine static initializer | 14 | unchanged |

Four of the renamed set already carried cc13-mistag names (007D03A0, 0083B120, 0086C240, 0099BE30).
Class names come from a named constructor that stores the same offset-0 vtable, or from host hook
names; seven unnamed ctor/dtor pairs are named by that vtable (`Unknown_Vt<vtable>_Construct/_Destruct`).
All names are hypotheses.

## std::list::sort

All three bodies test size > 1 on [this+8], build a local array of 26 (0x1A) 12-byte lists through
the eh vector constructor iterator, run the merge loop bounded by 0x19 (25 bins) and destroy the array
through 00BF7C6E: the VC9 `_Binlist` implementation of `list::sort`. Library code; it stays bound.

## Tags

The 74 cg_vector_deleting_dtor and 58 cg_array_ctor_helper tag rows that fail
`build_tag_ledger.shape_ok` were deleted in place (the two passing vector rows stay). The three
iterator-only implicit constructors (004CCB00, 005FD650, 009DBDF0) fail the array shape's
20-instruction limit and lost their tag rows, but keep their CG_ names, so the board still excludes them.

004D9830 (tagged cg_scalar_deleting_dtor) passes the scalar shape test falsely: its TEST ...,1 / ,2
operate on a local EH-cleanup byte, while its argument byte is stored to [this+61Eh]. It is not a
deleting destructor (provisional).
