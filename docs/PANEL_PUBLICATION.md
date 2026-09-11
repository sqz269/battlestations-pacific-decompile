# Panel row publication and character names

This packet recovers00451C90,0044F220 and005B6910 using the canonical panel
sequence, voice manager, NativeString, CRT comparator and GUI interfaces. Names
are descriptive hypotheses, not recovered symbols. Complete assembly disproves
the old `STL_inst_005b6910` name and `stl_instantiation` tag: this is a game
routine using34h voice rows and widget virtual dispatch. C++ now calls it
`hide_published_panel_row_005b6910`; the proposed Ghidra name is
`BSP_VoicePanel_HidePublishedRow`. Parent central integration replaces only this
incorrect library classification and retires its tag/bookmark under the lock.

## Character map

The panel owner's map at+4 stores NativeString keys and signed32-bit values.
The native1Ch node has left/parent/right pointers at0/4/8, string at+C, value
at+14, color+18 and sentinel byte+19.0044A730 is a case-insensitive lower-bound
walk.0044F220 checks the resulting key using existing00443D00, preserving its
stored-length-zero gates and real CRT `_stricmp` behavior.

An existing value is returned by address. On absence,0044F267..29B makes an
owned key copy,0044F29E..2A0 explicitly writes the mapped word ZERO, and0044ED30
inserts with the lower-bound hint.0044DEA0 is the actual library node link and
rebalance operation, despite its partial `STL_xlen_throw` name.0044D4B0 allocates
1Ch and0044CD80 independently copies the key into the node before copying the
mapped value.0044F286 captures temporary data in EBP; cleanup later uses that
captured pointer and the current temporary length before returning node+14.

`PanelCharacterMap` is a standard-map projection using the existing
`PanelSequenceNameLess` comparator and independently owned pooled keys.
`lookup_panel_character_0044f220` returns `int32_t&`; the loader writes through
that reference and publication reads it. This zero default is proven and is
unrelated to the palette map's uninitialized missing-value words. Keys are not
aliased to loader strings. `clear_panel_character_map` is an explicitly new
host lifecycle helper that releases each owning key; it is not assigned a
native address. NativeString does not perform implicit destructor cleanup.

The library insertion/rotation/allocator bodies above are evidence dependencies,
not additional reconstructed game routines. They are served by `std::map`,
without porting STL. Native node layout, debug-iterator validation, allocator
selection, map-size limits, exception transport and allocation/copy reentry
that mutates this map are outside this container projection. Ordinary key-copy
allocation callbacks can still change unrelated current-manager state; the
publisher preserves the subsequent native manager reload.

## Publication behavior

00451C90 looks up the current sequence entry once through actual00451920 and
retains that entry across callbacks. Each loop iteration reloads current
`[00E198C4]+A4` and compares its row count with the index using signed JGE.
The per-entry slot test is unsigned. An existing slot captures its pointer,
reads active byte, conditionally reads subtitle-enabled00F88989, then looks up
its character-name string at+C even when inactive. The established field name
`PanelSequenceSlot::palette_0c` is retained in this worker to avoid shared-file
changes; this consumer proves it is the character-map input. After lookup
temporary cleanup, it reads the mapped integer and separately reloads the
current voice manager for dispatch. The captured slot supplies text+4.

A missing slot constructs a fresh empty string and supplies enabled0,
character-1. Fresh resize(0,true) cannot allocate, so its omitted resize/copy
checks have no effects. The temporary is destroyed after row dispatch. Both
branches increment the index and repeat the current-manager count load.

The complete005B6910 body is surprising but unambiguous: it checks the row
index, calls00BF6713 on failure, reloads the current row vector, fetches that
row's widget pointer and calls virtual+34 with FALSE. RET10 consumes all four
arguments, but enabled, text pointer and character are never read. It does not
set text, apply a portrait or use the mapped integer. The reconstruction keeps
the arguments and constant hide behavior. Lookup and enabled-byte evaluation
in the caller remain real operations even though these results are unused.
A returning validation handler can repair the rows; the helper deliberately
reloads storage afterward. If it returns without making the reached row/widget
valid, native code faults and the typed projection has no recovery guarantee.

`PanelPublicationContext` binds character-map storage and existing validation
callbacks. `publish_panel_rows_00451c90(PanelSequenceContext&,
PanelPublicationContext&)` reuses all current manager, GUI and string services;
there is no generic publication callback in the reconstructed function. Parent
integration replaces the old PanelSequenceHost callback with a context getter
and a direct call. This worker does not change that shared header/source.

## ABI and evidence

| Address | Original ABI | Final instruction |
|---|---|---|
|00451C90|ECX panel owner, RET|00451E42 RET, length1|
|0044F220|ECX character map; NativeString* stack; EAX mapped pointer; RET4|0044F30B RET4, length3, inclusive end0044F30D|
|005B6910|ECX voice manager; row/enabled/text*/character stack; RET10|005B695A RET10, length3, inclusive end005B695C|

Complete assembly exports were inspected, including the mismatched decompiler
prototypes and all global reloads. Capped exports reside in ignored
`exports/bsp/functions/<address>/assembly.txt`. Ghidra flow inspection reports
no gaps for the three owned routines. No new function starts or body repairs
are required. Worker analysis was read-only; no Ghidra or ledger mutations
were performed. Root owns central name/evidence ledger integration.

## Validation and limits

Standalone `./scripts/build.ps1` passed MSVC Win32 Release and this new
worktree's configured existing test (reconstructed_math,1/1). One ignored
fixture exercises inactive-name insertion, changing the global manager during
key allocation, a returning row-validation repair, retaining the original
entry after a GUI callback changes current name, the missing-slot branch,
constant false visibility, case-insensitive mapped-value identity and complete
pooled-key cleanup. All fixture assertions passed against library SHA256
84362620E21D64E803E7E82A76404310EF00DA03D8A07B8672A89426CCDE8C28.

Fixture source: `local/panel-publication-probe.cpp`; parameterized recipe:
`./local/run-panel-publication-probe.ps1 -SourceRoot <checkout>` with optional
`-CoreLibrary <bsp_core.lib>`. It links current headers/library without copying
reconstructed source. This is not full panel configuration, binary ABI or
gameplay validation. Retained entry/slot/widget identities must remain valid;
corrupt vectors, exception/SEH parity and native CRT locale-state equivalence
remain outside the typed interfaces.
