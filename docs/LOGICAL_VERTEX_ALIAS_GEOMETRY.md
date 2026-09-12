# Logical vertex aliases through geometry and stream cloning

The additional mesh-clone route does not identify a nonnull owner at logical
stream **+4C**. Its generic memcpy writes the pointer returned by the new
stream's lock method. The factory's original logical-object pointer is retained
separately and is the value assigned to geometry. This is a bounded negative
finding; the independent +4C pointee profile and its final-zero terminal remain
unresolved.

The earlier [escaped-alias discovery](NATIVE_VERTEX_STREAM_ALIAS_ESCAPE_DISCOVERY.md)
already covers B85B80/B73BB0, section clear/layout, geometry destruction, logical
locking, and the known logical stream terminal. This extension adds the actual
**B73F50 -> B72A70 -> memcpy -> B73BB0** clone route and constructor-output
handoffs. It does not repeat the earlier adoption-store candidate search.

The [report](../reports/logical_vertex_alias_geometry.json) pins **13 fresh
live-Ghidra/installed-PE spans, 1,042 bytes**, including the complete 170-byte
B72A70 helper, complete two assigned endpoints, getter, current logical profile,
and bounded caller fragments. Every live query used python tools/bsp.py ghidra,
whose client verifies the saved bsp project, /battlestationspacific.exe,
language and image base before querying. C:/Users/sqz269/bsp.gpr exists; the
installed executable SHA-256 is
b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6.
All compared spans match. The report explicitly distinguishes complete functions
from caller fragments and data spans.

| Pointer or operation | Exact origin and destination |
| --- | --- |
| Original geometry stream | B73F6F captures source geometry in ESI. B73FFC forms its +64 cell cursor in EBX; B74007 loads the cell into ECX. |
| Clone selection | Flags bit 10h selects B72A70 at B7400F. With the bit clear, B740D7 passes the original stream directly to B73BB0 at B740DB. |
| New logical object | B72A70 calls the captured renderer table +5C at B72AA0 using source declaration, flags and count. B72AAD captures returned EAX in EDI; B72B12 returns that same EDI. Canonical D5F0A8+5C resolves to B287C0. |
| Source bytes | Source logical ESI receives virtual +10 at B72AB7. Its returned EAX becomes EBX at B72ABF. |
| Destination bytes | New logical EDI receives virtual +10 at B72AD0. Its returned EAX becomes EBP at B72AD2. |
| Generic write | B72AF5..F8 pushes count, source EBX, destination EBP, then calls memcpy BF7680. Neither data argument is formed from logical+4C or the logical base. |
| Geometry publication | B74018 copies the returned logical EAX to EBP. B7401A..1C passes EBP and stream index EDI to B73BB0. This is the logical object, not the mapping pointer held in B72A70's local EBP. |
| Temporary release | B74021..36 decrements the returned logical object's +4 and calls its current slot0 at zero. No early header-field clear is synthesized. |

B72A70's original ABI is ECX source logical stream, EAX new logical stream,
plain RET. B73F50 takes ECX source geometry and stack destination/flags, RET8.
B73BB0 takes ECX destination geometry and stack index/stream, RET8. Descriptive
names are hypotheses, not recovered source symbols.

The helper preserves several callback-sensitive distinctions. It captures the
renderer table before querying the source. It captures the source table before
the source count call and uses that captured table for the source lock. It
captures the destination table before another source count call and uses that
table for the destination lock. After both locks it calls the source declaration
getter again, reads +CC stride, and calls the source count getter again for the
copy length. After copying, it calls current source +14 and then current
destination +14. A high-level rewrite that freezes all these values at entry
would not describe the original order. The full 170-byte body has no local EH
wrapper; this finding supplies no rollback or complete clone exception contract.

The current 13-slot D61D6C profile is freshly pinned. Its +10 lock B49980 is also
re-pinned as a dependency: it invokes current physical +8, caches the returned
mapping at logical+8, and returns it. The dynamic output-offset cell is
logical+5C; the nondynamic cell is a stack argument. These explicit addresses
differ from +4C. Excluding the clone copy as a logical-header store assumes the
concrete mapping-provider contract and valid distinct native objects/mappings.
It does not prove anything about an arbitrary callback returning an overlapping
or corrupt address, or about unobserved callback side effects.

The constructor-output handoffs support the same object distinction. In
B4C8D0, the mesh constructor's return becomes EDI; current renderer +5C's stream
return becomes ESI at B4C98D and is passed unchanged to B73BB0(index1). The
caller writes logical+54=80000000h and releases the creator reference at +4.
Later, B73260(index0/1) returns the exact geometry+64 cell; B4CA43/B4CA54 pass
those values unchanged into B85B80. A separate B4A9B0 construction at AE4A83
publishes returned EAX at caller+54, tags logical+54, then reloads that same
caller field and appends it at AE4AB0. Neither caller+54 nor section+4C is the
logical owner's +4C.

Complete endpoint bytes confirm the prior result. B85B80 retains stream+4,
stores the unchanged stream at section+3C+4*count, and increments section+4C.
B73BB0 publishes the unchanged incoming pointer at geometry+64+4*index before
adjusting its temporary register to +4 for retention; it then releases the
captured old stream through current slot0 at zero. Their indexed writes target
the containing objects. Invalid indices, overflowing counts and corrupt
overlaps are not established valid ownership paths.

The logical stream's known terminal is only provenance from the earlier
discovery: current slot0 BD30E0 -> current slot4(flags1), canonically B4BF10 ->
B4B5D0 -> B62010, followed by CPU-pool return only after normal destruction.
Those destructor bodies were not re-pinned by this extension. B62010 can
separately capture logical+4C and release that pointee through its own current
slot0. Knowing the logical stream's profile therefore supplies no profile or
terminal for its retained +4C object.

No concrete nonnull +4C assignment/adoption was found, so no source-ready owner
packet is created. Further work needs an actual incoming identity and its
complete destructor/allocator/EH chain, or another concrete later consumer of
the geometry/section pointer cells. These fragments do not enumerate every
escaped alias or prove whole-program absence. Constructor zero and matching
offsets are not used as lifetime proof.

Only this document and its report change. There are no C++ changes, Ghidra
mutations, shared-ledger edits, tests, build results, native ABI-compatibility
claims or game-validation claims.
