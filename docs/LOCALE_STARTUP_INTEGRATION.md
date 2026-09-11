# Language catalog and locale startup services

This batch connects mounted VFS resources to the language catalog, corrected
locale loading, persistent input-script startup and live Lua text resolution.
The application/GUI owners remain external consumers of these services. Source,
native adapted fixtures and installed-data checks are distinct from gameplay.

## Language discovery

`build_language_catalog_008d7bc0` follows `008d7bc0..008d818a` (no arguments,
RET). Global `00f88978` is the catalog **count**, not an independent once flag.
A nonempty table skips work; an empty result allows another attempt. The native
vector is `00f88974/78/7c` with32-byte records. Host vector allocation replaces
the native string/vector allocator and exception ABI.

`008d7c25` invokes `00886280` with directory `lockit`, extension `.lng`, flags0.
That helper normalizes a copy of the directory then calls `00bdd990`, preserving
provider order and its existing duplicate handling. Each returned path is opened
by `00bef2e0` **before** querying `004c1e90`'s owner count at+8. The scanner opens
the name through VFS mode32h at `00bef364..367` and retains the supplied name.

With a nonzero hint count, every descriptor is accepted. Otherwise the inline
comparison and five `00553c80` calls at `008d7cd8..7d68` accept case-sensitive
prefixes: `lockit/english.lng`, `french.lng`, `italian.lng`, `german.lng`,
`spanish.lng`, `englishauthentic.lng` (each with the `lockit/` prefix). These
are prefix tests, not filename equality. Rejected descriptors do not append a
record. Accepted empty descriptors append an empty record, setting the count.
Completed earlier records survive a later host error.

The four recognized keys map to `LanguageEntry` fields: `lanfile`, `lockit_id`,
`voice_dir`, `fontpath`. Comparisons use `_stricmp`; repeated keys replace values.
At `008d8055`, an unknown key jumps back to cached-token peek without acceptance,
so native code stalls. The host throws instead. Missing trailing values store
empty text at EOF. Empty quoted values may leave a cached token and stall;
that unsupported input also throws. The earlier whitespace-pair parser's silent
unknown-key skipping was not native behavior.

## Scanner evidence

`NativeTextTokens` projects the normal scanner state from the0x828-byte native
object; input bytes come from VFS. Native buffers at+1 and+401 are each1024 bytes.
Flag+801 caches a token, +802 is the previous byte, +803 the lookahead byte,
+804 its cache flag, +805 stream EOF, +806 EOF at token start, +814 separator
pointer, +818 line count, +81c filename and+824 stream. Native field/buffer ABI,
stream ownership and allocation failures are not reproduced.

| Address | Behavior | ABI |
| --- | --- | --- |
|00bee800|Copy current token to previous; clear only token cache|ECX scanner, RET|
|00bee840|One-byte stream read when uncached, actual-count EOF test, count LF|ECX scanner, AL byte, RET|
|00bee8c0|Move lookahead to previous byte, invalidate and refill|ECX scanner, RET|
|00bee8e0|Cached token peek, quotes, comments, delimiters and lookahead|ECX scanner, EAX token+1, RET|
|00beedb0|Skip scanner whitespace and publish EOF state|ECX scanner, RET|
|00bef020|Read nonempty string token, accept on success|ECX scanner, bool-output pointer stack, EAX token, RET4|

`00e15334` points to bytes at `00d15f2c`: space/TAB/CR/LF/comma. `strchr` also
matches NUL. Default separators at `00ce5698` contain only `;`; constructor
appends additional supplied separators. Comments are recognized at token start,
before separator/quote handling. Quotes preserve whitespace without escapes;
an unterminated quote returns its partial token. Comment-like bytes within a
bare token remain literal. Tokens beyond1023 bytes throw instead of overflowing.

`00bee840` pseudocode misidentifies the read-count stack variable; assembly at
`00bee869` establishes its actual zero-count test. `00bef020`'s second strlen
check rejects even a quoted empty token; its failure path retains that token.
An independent worker review and one fixture compare106 token states against
six disk/Ghidra-verified original bodies, with explicit byte-stream and CRT
adapters. That does not establish native stream/allocator ABI.

## VFS and application boundaries

`VfsLocaleRuntime` supplies required locale source operations and language
discovery. Numbered-name probes invoke existing `00bdf4c0` resolution on a copy
and discard the resolved spelling, as native does. Ordered content variants
reuse `00bdef90`; locale loads variants before the original, unlike Lua's order.
Locale reads use mode2 and descriptor reads mode32h. Provider miss returns false;
opened-but-incomplete backing throws without falling through to another mount.
Exact read counts are guarded. Native mode2 failure returns null after its
diagnostic callback (`00bdf310`), so mandatory locale reads cannot fabricate an
empty stream.

The adapter requires current mount/search/suffix state and a live hint-count
query. GUI refresh and current Lua context selection are explicit owner inputs.
The application startup can retain `InputScriptStartup`, pass its settings()
reference to profile restoration, build the catalog through `VfsLocaleRuntime`,
select the configured language on `LocaleTables`, and resolve GUI strings with
`LocaleTextResolver` plus `LocaleLuaContext` on the selected live Lua state.
The other orchestrator owns `game_hosts.*`; this batch does not overwrite those
files or claim that its currently omitted startup phases are connected.

See `INPUT_SCRIPT_STARTUP.md`, `LOCALE_FILE_LOADING.md`, `LOCALE_TEXT_LOOKUP.md`
and `reports/locale_startup_integration.json` for validation, ABI limits and
saved Ghidra annotation/export evidence. `00aa0020`'s decoded78-byte cleanup
tail remains outside its stored function body; no full-body repair is claimed.
