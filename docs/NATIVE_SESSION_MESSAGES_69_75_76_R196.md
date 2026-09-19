# Native session messages 69, 75 and 76 (R196)

Addresses: `00767570`, `00767600`, `008E4A60`, `008EB7E0`, `00767610`, `007676A0`; `00759E40`, `00759E80`, `0075B950`, `0075B9E0`, `00759E70`, `00759EA0`; `00759EC0`, `00759F00`, `0075BA70`, `0075BAD0`, `00759EF0`, `00759F20`. Library contracts: `007263B0`, `00725D30`. Partial parent: `00768530`.

## Result and evidence

Eighteen complete normal game bodies (1,458 bytes) and three five-slot profiles are reconstructed in `native_session_message_tags_69_75_76`. All names describe behavior and are hypotheses, not recovered symbols. The source preserves native layouts but takes explicit source-only context bindings; it is not a drop-in binary replacement.

The strict MSVC Win32 build and three existing CTests pass. The differential fixture matches **11,409 original/source pairs and 32,006,693 observation bytes**. New coverage comprises 84 constructor/destructor cases, 768 predicate cases, 18 scalar-deletion cases, 512 extended-header wire cases and 320 paired-vector cases (1,702 new cases; 9,707 inherited). All eight bit alignments are covered. Repeated message69 reads test counts 0, 1, 2, 3, 7, 16, 127, 128, 255 and 256, populated destination vectors, extra source strings, growth, embedded NULs, and pool/heap string sizes. One new source-only access-violation case checks message69 cleanup; three earlier fault cases are retained.

The collector verifies 29,021 live Ghidra/PE bytes, 52 owned CALL/tail-JMP edges and 860 fixture relocations. It adds 25 original library dependency bodies (2,991 bytes) for comparison only. Six previously absent game functions are defined. Returning-free flow is checked/repaired in four scalar/destructor bodies; no global no-return policy is changed. EH records and actions are preserved. Raw factory CALLs `00768D5E`, `00768D86`, `00768DA6` have verified instruction bytes but no stored Ghidra function ownership, so they are outside the 52-edge total.

| Type | Allocation | Constructor | Predicate | Writer | Reader | Destructor | Scalar delete | Profile |
|---|---|---|---|---|---|---|---|---|
| 69 | 38h | 767570 | 767600 | 8E4A60 | 8EB7E0 | 767610 | 7676A0 | D03768 |
| 75 | 34h | 759E40 | 759E80 | 75B950 | 75B9E0 | 759E70 | 759EA0 | D02C90 |
| 76 | 28h | 759EC0 | 759F00 | 75BA70 | 75BAD0 | 759EF0 | 759F20 | D02CA4 |

Profiles contain scalar deletion, write, read, predicate and `004499C0` always-true slots. Native methods take the receiver in ECX. Writers take one 10h cursor pointer, readers one 18h stream pointer (cursor at +4), predicates one full DWORD query, and scalar deletion one DWORD flags argument; these return with RET4. Constructors and ordinary destructors have no stack arguments. Scalar deletion returns captured receiver identity and frees only when flags bit0 is set.

## Message 69: paired WORD and owned-string vectors

The 18h base is followed by two checked-vector headers. Each is `{retained DWORD, begin, end, capacity}`: WORDs at +18 and eight-byte `{length, data}` strings at +28. Constructor `767570` inlines the base constructor: initial delivery3, fields8/C zero, D02C68, fixed type69, one capture of Game E188A8 and signed owner index +18EC (0..7 selects +18CC[index], otherwise null), then delivery1 and D03768. It clears only the six vector pointers. Opaque DWORDs +18/+28 and base padding remain unchanged. Its EH map has base/WORD cleanup actions but state remains -1 throughout the body.

Writer `8E4A60` writes mutable type8, then the WORD count as signed DWORD width8. It walks the full current WORD vector even when the count truncates or becomes negative on read. For each element it separately checks/reloads the WORD and string vectors, emits WORD12 and owned string `429AC0`. Extra strings are ignored. A shorter string vector invokes the returning invalid-parameter contract; malformed iterators are outside the comparison domain.

Reader `8EB7E0` reads mutable type8, initializes one reusable temporary string, arms EH state0, and reads a signed count8. Only positive counts enter the loop. It appends each WORD first, reads into the reusable temporary string, then appends a deep copy of that string. Existing vectors are never resized or cleared. Count0 and bytes80h..FFh leave them untouched. A later string failure does not undo an appended WORD. Normal exit disarms the EH state before returning the temporary's allocation. EH handler CA42D8 / info DD6D60 / map DD6D58 invokes CA42D0 -> 41DD20 on escape; source `__finally` performs that temporary cleanup.

The WORD fast path stores at end and advances by2. The full-capacity path calls existing STL driver `7263B0`: ECX vector, stack `(output iterator, vector, captured end, WORD pointer)`, RET10h. That driver calls `725D30` with `(iterator owner, position, count=1, value pointer)`, RET10h. The latter captures the value, enforces max7FFFFFFF, and grows by1.5 with a requested-size floor. This packet binds the installed MSVC `std::vector<uint16_t, RawAllocator>` for the valid end-insertion domain; it does not port old STL bodies. Raw allocator uses the existing singleton allocation/free backend without modern large-allocation alignment prefixes. Win32 Release iterator-debug-level0 and a 12-byte vector representation are required.

String append reuses `append_checked_native_string_storage` (the existing 450540 contract). Range destruction reuses `destroy_global_config_name_range_00432050`: native ECX begin / EDX end, two unused stack owners, RET8. Both use `ActualNativeStringPoolStorage` constructed from the same publication, return gate and manager references as the raw string reader. Every allocation/return resolves the actual singleton. There is no private replacement pool.

Destructor `767610` stamps D03768 and arms state1. It destroys the captured string range, reloads/frees the backing block and clears string pointers; then frees/clears WORD pointers and stamps CE4974. Handler C88DB3 / info DB8B3C / map DB8B2C unwinds through C88DA8 -> 583D80 WORD cleanup, then C88DA0 -> 4499D0 root stamp. If string-range destruction escapes, its backing block/header remains while WORD cleanup and the root stamp complete. The focused source-only fault test confirms this state and unchanged pool contents; it does not test original FH3 exception dispatch.

## Messages 75 and 76: extended header

Both constructors initialize base fields8/C and owner14 to zero, mutable type10 to **zero**, WORD18 and bytes1A/1C to zero, delivery4 to1, and the own profile. They do not read Game and retain all other bytes. Predicates ignore the receiver and mutable type: message75 accepts full DWORD 75, 73 or 70; message76 accepts 76, 73 or 70. Their seven-byte destructors only stamp CE4974. The 31-byte scalar wrappers inline that stamp before optional free.

Both codecs begin with type8, WORD18 width12, Boolean1A, Boolean1C. Message75 then serializes Boolean20 and unsigned28 width6. It reloads Boolean20 after the value write; only false serializes unsigned24 width9, unsigned2C width1 and Boolean30. Reader follows that order and retains inactive fields. Message76 instead serializes unsigned20 width9 and unsigned24 width6. Boolean reads canonicalize to0/1; nonzero source bytes use the existing Boolean writer behavior.

## Factory table and remaining work

Verified table entries at 76A3A8 map types69,75,76 to 768D3E,768D72,768D92; types70..74 all map to 76A277. There are no constructors for those five table entries. The common/default label reads the local allocation-pointer slot at `[esp+C]` and calls its reader; the default route does not initialize that slot. This is **not evidence of a safe null return or rejection**. The predicates suggest family tags but their wider protocol meaning remains provisional. The complete 7,528-byte factory is still unbound.

Full original ABI/FH3 and CRT state, allocation failures, invalid iterator recovery, arbitrary aliases, concurrent mutation and unmasked numeric faults remain unproved. Existing `NativeStringStorage::release` and range cleanup are noexcept: C++ failure while lazily creating a pool can terminate through that interface. Source access-violation cleanup evidence does not remove this limitation. Very-large-size STL growth/exception behavior is not established by the bounded fixture.

Next useful packets: subsequent concrete factory arms, full factory composition once dependencies are ready, then packet-recorder/network binding and startup validation. This packet does not establish a running game or gameplay parity. See `reports/native_session_messages69_75_76_r196.json` for native spans, call evidence, prior Ghidra annotations and immutable tested/integrated artifacts.
