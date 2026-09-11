# Orch5 mapped glyph and native font integration

Addresses: `00AB98F0`, `00B49980`, `00B49A80`, `00B49B60`, `00B49C70`,
`00AD4C30`, `00AD53A0`.

Reviewed commits `7ae534f3` and `1445d2a1` add actual logical stream mapping,
mapped glyph output and canonical native font-resource ownership. The primary
integrates them in the separate `agent/orch5-20260911` worktree. Three sources
are registered in the normal Win32 build: `native_logical_buffer_mapping.cpp`,
`gui_text_geometry.cpp` and `native_font_resources.cpp`.

Integrated into `main` at `606726c2`. The combined build and both existing
tests pass after merging current main. Its seven annotation applications
already match the primary's saved names/comments and recorded prior values.

Mapping uses the existing physical buffer and renderer synchronization domains.
It preserves wrapped DWORD arithmetic, callback-time reloads, output offsets,
vertex mapping publication/clear and captured guard ownership. The four known
physical profiles are the supported domain. Original native stack/SEH behavior
and an entry-disabled/exit-enabled guard transition are not emulated.

The glyph writer uses the existing verified x87 scalar kernel directly on the
actual mapped vertex storage, then writes indices in native order. Matching
optional glyphs have their UVs cleared before the explicit unfinished child
boundary at `00AB9D33`. Reporting that boundary does not itself implement or
capture the missing child-construction frame. The actual single-line/wrapped
callers and child lifetime continue in separate packets.
Native argument slots 5/6 must be preserved for that missing tail. At the
wrapped call, `00ABA70C`, `00ABA715` and `00ABA71C` compute the same address
despite intervening pushes, so all three position arguments alias one float3.
The eventual child continuation must preserve that storage and aliasing.

`NativeFontResources` keeps the existing `FontData` allocation and glyph
identities, adopts exactly two actual image references without retaining again,
and stores the raw aliases in the same glyph records. Only the embedded space
record owns those two references. Destruction releases glyph payloads, then
captured GFX and current alpha references. Stable descriptor lookup returns this
sole data owner, including its current signed height. Native renderer loading,
VFS/cache registration, reload and tree/pool ABI remain separate work.

The primary also repaired the native font destructor's false free fall-through
sites at `00AD540F` and `00AD5564`. Gap repair alone left the final 30 bytes
outside the saved function body. After archiving its documentation, the
existing definition tool restored the complete 487-byte body through
`00AD5586`. A before/after comparison preserves name, signature, calling
convention, parameters, plate and instruction comments, all 17 existing labels,
and all 11 local name/type/storage triples. The full listing has zero gaps.
`bsp.py ghidra documentation` now provides a capped read or complete archive
under `local/` for this verification. No game files were changed.

Validation: the normal MSVC Win32 build includes all three sources and passes
both existing tests. Across the three reports, 50 numeric call rows have zero
failures: 35 direct and 15 resolved indirect rows. Eight further symbolic
indirect rows remain explicit, including calls in the unimplemented child path.
The checker does not prove indirect dispatch or execution. No tests were added;
these new operations have not been executed in the game and are not native ABI
replacements. Seven reviewed names/comments were applied to the existing BSP
project, saved, and affected exports refreshed.
