# Native enum symbol declaration, CC12

`declare_native_enum_symbol_008f2e40` supplies the complete ordinary wrapper over
accepted raw string, symbol lookup, and symbol insertion Source. Its explicit
context borrows genuine current-CRT CEnum bindings. The new TU remains
unregistered for integration review.

## Original contract

The sealed read-only evidence verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 little-endian 32-bit image base `00400000`, at
modification 52. Complete live bytes match the installed executable. The
returning non-thunk body is one 296-byte range `[008F2E40,008F2F68)`, SHA-256
`38c37a1c272bde559079351d9af289e79fb4141030ef1fc5b52cf2539248024b`.
There are no listing gaps requiring repair. The cached `undefined ...(void)`
prototype is not the ABI authority.

Assembly establishes ECX as the genuine outer CEnum owner, entry stack+4 as
the nonnull symbol C string, entry stack+8 as the signed 32-bit incoming value,
and RET8 on both ordinary exits. No stable semantic EAX result is established;
both observed callers discard it. The Source interface returns `void` and adds
an explicit context. It does not reproduce the Original calling convention.

## Complete ordinary schedule

The wrapper constructs a genuine eight-byte `NativeString` query through
`construct_native_string_header_0041e870`, then calls whole
`find_native_enum_symbol_node_0048d480` on `owner+4`. It captures whether the
returned node is nonnull before destroying the current query header through
the concrete raw-pool `destroy_native_string_header_0041dd20`. That provider
reloads the actual data and length, resolves `00419CC0`, and returns the buffer
through `00BD1510`; it leaves the header unchanged.

For a duplicate, the wrapper returns after that cleanup. It preserves the
first case-insensitive matching key and mapped word, count, and both metadata
words. Original `008F2ECA` calls `004254B0` with a format pointer, `owner+11C`,
incoming text, and value. The complete callee is the single byte `C3` (`RET`),
verified against live and disk bytes, SHA-256
`ae3f4619b0413d70d3004b9131c3752153074e45725be13b9a148978895e359e`.
It never dereferences those pointers or emits diagnostics. The Source duplicate
branch therefore needs no diagnostic implementation, callback, or stub.

For a new symbol, the Original reads the current `owner+10C` only after the
first query has been returned. Signed `JGE` selects the minimum of that word
and the incoming value. It stores this candidate to `+10C`. Signed `JLE` then
selects between the same updated candidate and the incoming value for `+110`.
Thus `+110` becomes the incoming value. The old `+110` is never read: a
maximum-of-old-`+110` or reverse-map interpretation would change behavior.
The Source retains the signed comparisons and two ordered physical stores.

Only after those stores does it reconstruct the same eight-byte header from
the incoming C string, then call whole `insert_native_enum_symbol_word_008f2850`
on `owner+4`. It preserves that provider's second lookup, real node allocation,
owning key copy, and native publication/count schedule. The mapped word receives
all incoming bits; signedness is used only for the wrapper's metadata decisions.
After insertion, the wrapper destroys the current second query. It introduces
no metadata rollback or extra reread after insertion.

## Borrowed domain and provider roles

The caller supplies a genuine live current-CRT `0x19C` CEnum owner, a real
initialized symbol pool corresponding to `00E17578`, the canonical
`NativeStringRawPoolContext`, and distinct `00E186ED` enum/hash fallback and
`00E17654` query fallback bindings. `00E175E8` is the separate table pool;
`00E175B0` and `00E177E4` belong to property services. None is substituted here.
Canonical enum process startup remains a separate provider and is not created
by this wrapper or its context.

Admission inherits the direct providers' successful, stable, disjoint,
coherent, nonoverflowing ordinary storage domain: finite valid chains, matching
NUL-terminated ASCII keys, nonnull incoming text, stable C-locale CRT, actual
raw string ownership, and the correct initialized pool. Empty strings and both
duplicate and new declarations retain their established schedules. Context
bindings and owners remain live throughout each call. The wrapper supplies no
invented globals, default storage, model lexer/graph, virtual phase dispatcher,
property-pool bridge, or allocation/cleanup guard.

## Static validation and evidence

Strict MSVC Win32 compilation uses
`/O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict /std:c++17`.
The first attempt caught an assembler-reserved parameter name; its exact
failed command/result, compiler log, and Source snapshot are retained.
After renaming that parameter, original chunk `58e816` returned exit 0 and
`strict_exit=0`; the compiler log contains only the TU filename.

All emitted code and relocations were inspected: wrapper COMDAT `0xD7`,
`NativeString` default constructor `0x10`, and three helpers `0x0B/0x11/0x17`.
The wrapper's six REL32 relocations name only the accepted construct, lookup,
destroy, and insert providers. The sole additional DIR32NB relocation is
debug metadata to the emitted constructor, not a runtime dispatch.
The code preserves lookup-result capture across cleanup, the duplicate early
return, signed JGE/JLE decisions, physical `+10C` then `+110` stores before
reconstruction/insertion, and second current-header cleanup. Its compiler
moves the second pure comparison after the first store; this does not change
ordinary values or memory effects in the admitted disjoint/stable domain.

The report records the required own-worktree build's actual launch and natural
terminal receipts: session `23849`, initial chunk `7ad0a8`, terminal chunk
`e0b72b`, exit 0. Both existing CTest suites passed. The build emitted LNK4006
for duplicate `spawn_request_id_matches` definitions in existing Lua-spawn
objects. The strict standalone object
is the compilation evidence for this unregistered new TU. No new test, probe,
or target runtime was added or started.

Frozen read-only evidence remains under `local/WholeEnumSymbolDeclarationSource01/`;
all new Source, object, direct-input copies, original paired tool receipts, and
build evidence are appended beneath its `Source02/` subdirectory. Source inputs
were unchanged from the sealed inspection when the worker fast-forwarded to
Main `c0892f83b`. No shared header, CMake, ledger, Ghidra, or application wiring
was edited. These are ordinary Source/static build results; Original ABI,
FS/EH/unwind, private historical CRT/locale, class provenance, fault/reentry,
runtime, game equivalence, and startup/application integration remain unproved.
