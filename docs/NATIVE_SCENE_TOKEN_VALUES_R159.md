# Raw scene tokenizer typed reads (R159)

`008D8190` needs typed reads over the actual `838h` tokenizer supplied by R158.
This packet implements five complete game-body schedules in
`native_scene_token_values.hpp/.cpp`. The existing `OptionsTokenReader` remains
a projection, and ordinary startup still uses the projected settings loader.
These descriptive names are hypotheses, not recovered symbols.

| Entry | Bytes | Native contract | Recovered schedule |
| --- | ---: | --- | --- |
| `008D8E50` | 83 | ECX owner, stack byte* ok, AL Boolean, RET4 | Peek token, decimal conversion, store success, retain token |
| `008D8F70` | 382 | ECX owner, RET | Skip following whitespace, preserve token cache, copy byte EOF to token EOF |
| `008D99F0` | 133 | ECX owner, stack byte* ok, EAX text, RET4 | Read nonempty string; failed reads recover then peek again |
| `008D9A80` | 72 | ECX owner, stack byte* ok, AL Boolean, RET4 | Peek Boolean, consume only when current output byte is nonzero |
| `008D9AD0` | 102 | ECX owner, stack byte* ok, EAX integer, RET4 | Convert, write success before consume; recover/false/zero on failure |

## Details needed by the settings loader

The saved conversion result starts with the incoming owner pointer bits:
both numeric entries begin with `PUSH ECX`. This preimage remains observable
to the supplied conversion service. Boolean success is exactly
`(converted == 1 && value == 0) || value == 1`; the result is `value == 1`.
The branch is retained even for a conversion service that returns failure after
writing one. Integer success is exactly `converted == 1`.

Failure does not consume the token. Recovery tests current scanner whitespace
through `E0C940`, then the immutable empty stop string at `D15F34`, and advances
only following whitespace. It does not skip comments. It copies physical EOF
`+809` into token EOF `+80A` and preserves the cached text. In particular,
embedded NUL is a stop byte for recovery even though the scanner's whitespace
test also recognizes NUL through `strchr`.

String reads reject empty quoted strings. On failure they recover, write false
through the caller's byte pointer, then peek again. This final peek matters if
that pointer aliases the token-cache flag: the output store can make it scan
the next token. Integer success writes the caller's byte before copying the
current token to the previous-token buffer. The implementation preserves these
alias-visible schedules instead of buffering outputs in a host result object.

## CRT boundary

`BF7533` receives the actual initialized local DWORD and borrowed `CE3A34`
`"%d"` format. `NativeSceneTokenValueCalls` makes that dependency explicit. Its
default calls the linked host CRT's `sscanf`; this is **not** a reconstruction
or parity claim for the original VS2005 conversion runtime. The comparison
harness routes both original game bodies and source bodies through this same
boundary. Representable decimal-prefix behavior is exercised; old-CRT numeric
overflow, locale, errno and invalid-parameter behavior remain unproven.

Read-only dependency analysis found `BF7533 -> BF74CB`, whose indirect scanning
target is `C0AD78`; the latter currently has no saved Ghidra instruction/function
at its entry. This packet does not relabel that missing CRT body as recovered.
The shared conversion boundary can later be replaced with its actual source
implementation without changing these owner layouts or read schedules.

## Evidence and verification

All new analysis batches verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Seven new spans match **776 live/PE bytes**:
772 bytes of game bodies and four bytes of format/stop strings. All 15 direct
CALL rows verify. The only listing gap is three unreachable bytes after the
string reader's jump at `008D9A0B`; it is left alone. There are no CALL gaps.
Existing names/evidence are preserved when adding annotations, saved, read
back, and refreshed in exports.

The strict MSVC Win32 build and all three existing CTests pass. One local
harness extends the sealed R158 original/source fixture. It reruns the unchanged
R158 scanner cases (8 pairs, 183 observations, 416,044 matching bytes), then
checks **10 typed-read pairs, 108 observations and 245,982 matching bytes**.
The original/source pipelines each use their corresponding scanner bodies;
both use existing actual raw string-pool and retained memory-stream services.
The reused R158 reference archive and identical whole-PE hash are recorded.

Each observation compares full `838h` storage with four pointer fields
normalized to presence/default-delimiter identity, live label/data/delimiters,
service traces, caller output and returned value. Cases include signed extrema,
decimal prefixes, quoted CRT whitespace, Boolean accept/reject, empty strings,
repeated failures, whitespace-to-EOF, comment retention, and output aliases into
the token or cache. Two controlled conversion responses expose branch/store
order. One source-only escaping conversion exception confirms that the scanned
token remains cached and the caller output remains untouched. All actual stream
backing counters and raw string-pool publication drain successfully.

No new repository test suite was added. The local fixture does not prove native
FH3/SEH, binary ABI, arbitrary stream profiles, malformed pointers, buffer
overflow, asynchronous mutation or gameplay. The complete native settings load
path, application ownership, original CRT and native-game admission remain open.

See `reports/native_scene_token_values_r159.json` and the sealed artifacts under
`local/scene_token_values_r159` / `local/evidence-r159`.
