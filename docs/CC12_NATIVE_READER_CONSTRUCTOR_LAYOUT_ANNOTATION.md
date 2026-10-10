# Native reader constructor layout annotation

Root clarified the004425C0 name-ledger evidence and appended a correcting
Ghidra plate comment. The constructor explicitly zeroes receiver+08h/+0Ch/+10h;
the+04h proxy word is not locally initialized. The delegated vector call receives
receiver+04h, so the containing object start must not be confused with the first
zeroed pointer field. This follows the accepted complete installed-PE constructor
and caller/provider binding evidence.

The descriptive BSP_LuaReader_Construct name is retained. Both earlier comments
remain preserved; the appended text explicitly supersedes their ambiguous offset
wording. Complete old ledger/live values and the mutation log were retained before
the write. The standard annotation tool took the shared Ghidra write lock,
verified the configured bsp/program before each operation, saved the project,
and refreshed all three complete affected exports. No prototype, body, listing,
flow, Source implementation, compiler or runtime change was made.

[Annotation receipt](../reports/cc12_native_reader_constructor_layout_annotation.json).
