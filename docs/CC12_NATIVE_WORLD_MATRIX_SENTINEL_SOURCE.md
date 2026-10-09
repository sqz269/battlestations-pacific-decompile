# Native World matrix sentinel producer

The approved Source implements the complete `004C3080` raw allocation leaf and one concrete allocator adapter. The single authorized normal build and all three existing checks passed. Whole emitted code, physical symbol-index relocations, actual core members and CRT origins are sealed; independent primary merged review remains pending. **World, full-game and startup readiness remain false.**

## Behavior and ownership

The no-argument CDECL naked entry ignores incoming ECX/EDX, returns the real pointer in EAX, leaves ECX=pointer+4 on ordinary return, and uses plain RET. Both original `004CB030` and `004C8B50` call it; it is not exclusive to the World constructor.

It allocates exactly `0x6C` bytes and writes the returned pointer only at +0 and +4. Bytes +08..+6B remain unspecified allocator contents. Ownership transfers to the caller in the existing `singleton_lifetime_free` domain. No World store, zeroing, registry, class constructor or callback is added. Neither helper nor adapter is noexcept. The preserved hypothetical null-result path reaches a write to address4; this is not a graceful-null contract. The genuine fixed-size provider returns storage or throws.

The existing host CRT boundary remains explicit: original static exception/guard/atexit behavior, exception vtable/throw metadata, global new-handler identity and FH3 machinery are not reimplemented. The original allocator whole105B/33 and free5B/1 were rechecked against the immutable original PE and accepted evidence; no new live Native query or execution was needed.

## Complete emitted code

| Body | Complete extent | Relocations |
|---|---:|---|
| Public sentinel | 26 bytes / 11 instructions | One REL32 at operand [3,7) |
| Concrete request adapter | 58 bytes / 20 instructions | Cookie load, allocator call, cookie-check call |
| Existing allocator | 90 bytes / 37 instructions | All five entries retained |
| Existing matching free | 6 bytes / 1 instruction | Imported free jump |
| CRT cookie check | 14 bytes / 4 instructions | Cookie global and failure jump |
| CRT GS failure | 8 bytes / 3 instructions | None; MOV ECX,2 / INT29 / RET |

All public-helper instruction starts and 22 bytes outside CALL operand [3,7) exactly match Native. The sole relocation targets physical private-adapter symbol index15, section4, value0. Both conditional branches, including their literal unusual paths, remain unchanged.

The complete adapter creates the trivial 12-byte request `{object=3,bytes,bytes}` on its own stack, with real call-site size108, and calls `singleton_lifetime_allocate` synchronously. Its compiler-generated `/GS` machinery saves/checks the cookie and preserves EAX on success. Failure reaches the complete physical INT29 path. The full 68-byte cookie data section, including complement at+64, is retained; its initial bytes do not establish runtime state. Source and compiler policy were not changed to avoid these emitted dependencies.

The existing allocator object is byte-identical to the independently accepted design and its pre-build image. It was **reused**, not freshly compiled. Its complete27-section/9-function graph includes both bad_alloc and exception constructors/destructors, what(), scalar deleting destructors, complete vtables/RTTI/throw records and actual weak-auxiliary fallback indices. Physical symbol-table indices, section/value, relocation record/operand offsets and full code/data payloads accompany every edge; same-name guesses do not substitute for origins.

Six complete MSVCRT members are retained: delete_scalar_size, delete_scalar, std_type_info_static, secchk, gs_cookie and gs_report. Their reached graphs resolve to actual full static definitions or six complete I386 UCRT/VCRUNTIME short-import members for malloc/free/callnewh/CxxThrowException/std_exception_copy/std_exception_destroy. The whole static members also preserve unreached sibling functions; no short prefix or cold-path waiver is used. Current host imports are explicit platform endpoints, not proof of loaded DLL execution or Native CRT identity.

## Build provenance and archive

Root admitted the exact proposed header/implementation in Source commit `0892ae344a67e69c6b3d9b2591a63e24b37b8f81`, then registered only this new translation unit in `039e3f57beffe813954234fdfc192432de0b8249`. Header and implementation stayed byte-identical to the approved proposal before and after build. The old Source comment describing expected emission is retained verbatim; this report records the actual emission.

Pre-freeze: 4,781 physical pins, 2026-10-08 23:52:40.737653–23:52:54.261860 UTC. One exact `scripts/build.ps1` invocation: 23:54:37.275640–23:54:53.808872 UTC, return0. Post-capture:4,530 pins, 23:55:17.722095–23:55:31.225782 UTC. Every preimage was rehashed before compilation. Actual Source argument plus83 per-TU CL.read rows gives84 consumed inputs, all exact pre/post/current. Windows SortDefault.nls and both System32/SysWOW64 tzres.dll were frozen before compilation; the actual new-TU read record contains SortDefault.nls and System32 tzres.dll. Toolchain binaries and recipes are physically pinned, without loaded-module claims.

The normal build compiled the new sentinel and an unrelated existing game_native_game_tables TU. Existing checks passed: reconstructed_math0.21s, native_math_differential0.03s and tool_tests6.44s. No sentinel-specific fixture, new test, ad hoc probe or target execution was added. The existing duplicate lua LNK4006 warning remains recorded. Librarian output records use the actual Lib-link.read/write basenames; these were captured as post-build outputs and are not mislabeled preimages.

The complete new object is1,577 bytes, SHA-256 `c5b7a238fe5da23e20f32b49719b7a6482b7025d6b4ba095f173fcd6297eb769`. The actual final core is73,672,790 bytes, SHA-256 `c1e55cc90fb1d7e7005aa256e25a0c6a6e550d8c2ac547544f76582c3e0d553b`. Its uniquely defined sentinel member payload begins at63742544 and matches the whole object; the unique allocator/free member begins at67772102 and matches the reused32,971-byte object. Full archive scans prove the relevant public definitions once each. Private adapter binding is through its positive local index, not a global-name lookup.

The same normal application map names the real CRT providers and its complete PE import directory is retained. Sentinel, private adapter and their member are absent from that application. Current production Source scan finds only the public declaration/definition, private definition/CALL and one expectation comment. There is no actual Source World caller or helper-linked runtime result.

## Sealed evidence and remaining boundary

Evidence: `local/cc12_native_world_matrix_sentinel_source_evidence.zip`, 159,140,246 bytes, SHA-256 `ec277e89365e5ff37de73441df8b21c3d76589e7b5ea95e3f3c5b192cd09b8d6`. It contains 9,371 manifest payload files plus MANIFEST.json; every payload SHA and ZIP CRC passed. Manifest SHA-256 `d5628b1560c9b421cc8c6504dda63d02d3d3ea120db4e9067aa17456b675339d`. The companion report enumerates whole-code, actual-input, archive, indexed-provider and Native proof files. Tracked summaries remain outside the ZIP to avoid a checksum cycle.

Local verifier draft failures and their exact logs are retained. They concerned path joining, a comment included in a callsite count, historical JSON schema and duplicate retained header copies. None changed Source, compiler policy, allocator behavior or build count. The one normal build passed on its first invocation.

This closes the worker's bounded raw-sentinel implementation proof. Actual World vtable/constructor/unwind/destructor ownership, publication, original caller integration and game validation remain separate work. Primary merged-build review is still required before packet acceptance.
