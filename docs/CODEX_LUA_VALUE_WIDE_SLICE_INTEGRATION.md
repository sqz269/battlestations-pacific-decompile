# Lua value display and raw wide search/slice integration

Three dormant Core bindings are integrated: B69130 Lua value-to-owning-string, A9EF60 wide first-of search and 770E50 wide substring. Each adds four new files and one Core registration. Existing semantic and typed records remain; this batch adds no Native/vendor-body credit and changes no consumer or startup activation.

Primary review covered all three actual HPP/CPP and complete Native listings (140/32/90 instructions), substring selected cleanup and the numeric bridges in the same single strict object. Two independent final Source reviews accepted each exact commit. Each author compiled one isolated strict Win32 COFF014C object once. Exact commits, hashes, review paths, objects and symbols are in [the integration receipt](../reports/codex_lua_value_wide_slice_integration.json).

B69130 uses genuine raw constructors and predicates. Its actual lua_tonumber return is narrowed immediately to FLOAT32; the integer bridge reloads that FLOAT32 and supplies the actual current conversion-word address to existing BF7420. The float formatter receives FLOAT32. This is static object evidence, with current Lua/CRT/FP and caller-storage qualifications. No local output rollback or temporary cleanup is invented.

A9EF60 reads set DATA before hay DATA, allows start equal to length to reach the existing raw CRT, and compares the wrapped result only for equality with CURRENT length after that call. It owns no Frame, input or cleanup.

770E50 adds negative position bits to count and sets position to zero; it does not count from the end. It captures source start before resize, uses the genuine raw WORD CRT and raw copy constructor, and distinguishes EARLY normal-footer DATA from CURRENT selected-cleanup headers. Private constructed-output authority, especially the empty path, is explicitly distinct from Native physical flags, stack/FH3 and hardware behavior. TEMP is consumed before the nonempty normal footer; selected actions consume TEMP then OUTPUT and cannot retry a consumed action or destroy an unreturned partial output.

The common retained-link build at14972bf748c04c5931cf6f0505e98354e0fb63a2 passed both existing checks in11.01s. Its map retained value/find/substring at10300be0/10300de0/10300e80 and substring C++ unwind support. After removing the temporary overlay, the canonical Release Win32 build passed both checks in10.81s; forced cache/project references and all three unbound map entries are absent. No new test, entry invocation or game execution occurred.

Peer Main1d2a1ca6150e72dddd3c594fcf70e08cec9e72f1 was joined before both builds. Of51 prior pinned paths,50 remain byte-identical; the sole prior registry change is that peer's one whole-wrapped Text registration, whose exact inverse restores the baseline. All nine raw Text qualification switches remain false.

The 770E50 historical from-end description was corrected by appended evidence while preserving its existing name and comments. The locked annotation tool journaled old values, saved bsp.gpr /battlestationspacific.exe and refreshed the affected export. No installed game code was changed.

These normalized C++ interfaces are build-tested Source bindings, not Original ABI or game validation. Caller backing, alias, current provider/error/nonlocal behavior, storage and lifetime obligations remain. Raw B68D70/B692C0, actual embedded-VM producer and zero-upvalue callback setup, resolver/full reload/raw16, GUI/Font48 and runtime gates remain separate work.
