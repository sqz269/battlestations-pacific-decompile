# Options token reader

`OptionsTokenReader` recovers the text-reader behavior used by settings loader
`008d8190`. It reuses `SceneLexer` with the native default delimiter `;`, rather
than the scene document delimiter set. Names below are descriptive hypotheses,
not recovered symbols. The public C++ interface is not a native ABI replacement.

## Evidence and initialization

The read-only analysis/export batch verified project `bsp`, program
`/battlestationspacific.exe`, through `tools/bsp.py ghidra`. Native source is the
saved `C:/Users/sqz269/bsp.gpr`; the original game installation was unchanged.

| Address | Native contract | Reconstruction |
| --- | --- | --- |
| `008d9f20` | ECX tokenizer; stack retained stream, optional delimiter string; EAX this; RET 8, through `008da0f2` | Constructor accepts already buffered text; native ownership remains outside this interface |
| `008d8a70` | ECX tokenizer; returns pointer to token at +5; RET; cached by +805 | `peek()` delegates to the shared lexer |
| `008d8960` | ECX tokenizer; copies token to +405, clears +805; RET | `consume()` delegates to the shared lexer |
| `008d99f0` | ECX tokenizer; stack byte `ok*`; returns current token pointer; RET 4 through `008d9a74` | `read_string(bool&)` returns a copied string |
| `008d9ad0` | ECX tokenizer; stack byte `ok*`; returns signed integer in EAX; RET 4 through `008d9b33` | `read_int(bool&)` |
| `008d9a80` | ECX tokenizer; stack byte `ok*`; returns Boolean in AL; RET 4 through `008d9ac7` | `read_bool(bool&)` |
| `008d8e50` | ECX tokenizer; stack byte `ok*`; non-consuming Boolean conversion; RET 4 through `008d8ea2` | Conversion within `read_bool` |
| `008d8f70` | ECX tokenizer; skips following whitespace and updates +80A without clearing +805; RET through `008d90ed` | `SceneLexer::recover_after_failed_read()` plus reader end state |
| `008d9c30` | ECX tokenizer; destructor; RET at `008d9ce7` | Native lifetime analyzed, not implemented by this text wrapper |
| `008d46c0` | ECX pair-vector `{data,count}`; stack pointer to two int32 values; returns first matching index or -1; RET 4 through `008d4707` | Resolution-list lookup contract supplied to loader owner; not a tokenizer |

`008d9f20` initializes the current and previous 1024-byte token buffers, cache
and EOF flags, character lookahead, and line counter (native zero). It retains
the supplied stream at +828, requests its size through vtable +30, allocates a
byte buffer at +82C, and invokes stream read +24 with `(buffer, size, 0)`.
It creates the label `unknown file`. A null optional delimiter pointer uses
`[00e0c944] -> 00ce5698`, verified bytes `3b 00` (`;`). A non-null pointer appends
its string to that default. Loader `008d8190` supplies null.

The destructor frees a privately allocated delimiter list, decrements/releases
the retained stream, frees its byte buffer, and releases the native label
string. Incorrect call-return overrides on the two `_free` calls had hidden
tails at `008d9c6f` and `008d9ca9`. The integrator's saved repair is recorded in
`reports/options_token_reader_flow_repair.json`. Stored Ghidra body coverage
remained incomplete afterward. Read-only bytes from `008d9ca9` through
`008d9ce7`, independently decoded with Capstone 5.0.7, establish the label-string
release through `00419cc0` / `00bd1510`, SEH restoration, and final RET.
Do not treat the remaining short decompile as the full destructor.

## Scanner and advancement

Whitespace is exactly space, tab, CR, LF, and comma. Semicolon is a token by
itself; parentheses, braces, equals, and colon are ordinary token characters in
options text. Quoted strings lose their surrounding quotes, preserve internal
whitespace, and perform no escape processing. Unterminated quotes return the
remaining content with `quoted=true`. Empty quotes produce a real token, not
EOF. `//` and `/* ... */` comments are recognized only at a token boundary.
Unterminated comments consume to EOF.

Assembly `008d8dd2` through `008d8e38` only checks whitespace and delimiters
after an unquoted token begins. Thus `7//tail` and `8/*tail*/` remain whole
tokens. The earlier shared lexer incorrectly split there; this packet corrects
that verified defect. The native previous/current-character check at
`008d8c60` through `008d8cba` also recognizes the overlapping block terminator
in `/*/`, so `/*/5` yields `5`. The shared lexer now follows that behavior.

`peek()` is idempotent. Successful typed reads consume one token. Failed typed
reads retain the current cached token and call `008d8f70`: it skips scanner
whitespace immediately after that token, stops before any nonspace byte,
and copies physical EOF into native +80A. It does not skip following comments.
The wrapper tracks this extra end condition separately: `at_end()` may return
true while `peek()` still exposes the failed nonempty token. Repeated failed
reads with a later nonspace byte can therefore stall. The wrapper never adds
automatic token consumption or loop caps; dispatch and safety limits belong to
the loader.

The native failure helper is whitespace recovery, not a logging function. New
`last_error()`, `failed()`, and `failures()` observations make failed reads
reviewable without pretending the native emitted a diagnostic. A successful
typed read clears `last_error`; manual peek/consume leaves it unchanged.

## Typed values and malformed input

`008d99f0` requires nonempty token text even when `quoted=true`. On failure it
returns the still-cached token text and sets `ok=false`; it does not substitute
an arbitrary default string. At terminal recovery state it also fails on
nonempty retained text.

Both integer and Boolean conversion use `sscanf(token, "%d")`, so a decimal
prefix suffices and CRT whitespace inside quotes is allowed. The shared
`scene_scan_int` previously rejected such leading whitespace; its conversion
now uses `%d` directly. Strict Boolean conversion succeeds only for parsed
values 0 and 1. Failed integer/Boolean reads return 0/false and `ok=false`.
The apparent uninitialized scalar in `008d8e50` decompilation is saved ECX
(`PUSH ECX` at entry); for a valid native tokenizer pointer, a failed conversion
cannot accidentally qualify as 0 or 1.

| Input at current value | Operation | Result and advancement |
| --- | --- | --- |
| `12tail` | integer | 12, success, consumes token |
| `" \t-3rest"` | integer | -3, success, consumes token |
| `0x10` | integer or strict Boolean | 0 / false, success, consumes entire token |
| `01`, `1e3` | strict Boolean | true, success, consumes token |
| `2 next` | strict Boolean | false, failure; token `2` stays cached; not at EOF |
| `bad` or `bad` plus trailing whitespace | integer | 0, failure; `bad` stays cached; at EOF |
| `bad // tail` | integer | 0, failure; `bad` stays cached; not at EOF |
| `"" next` | string | empty, failure; quoted empty token stays cached; not at EOF |
| `""` | string | empty, failure; quoted empty token stays cached; at EOF |
| `"unterminated` | string | `unterminated`, success, then EOF |
| EOF | any typed read | empty / 0 / false, failure, remains at EOF |

The loader has two Boolean conventions: its early fields call `ReadInt` then
compare against zero; the strict reader is used only where `008d9a80` is
actually called. `00467cc0` consumes a matched keyword only. In particular,
the `SoundEnabled` branch at `008d85d9` jumps directly to `008d82a0` after that
match and leaves its numeric value for subsequent token dispatch. Unknown
tokens are consumed one at a time, so an unknown token must not blindly remove
the following value or keyword.

## Validation and limits

The supported equivalence domain is NUL-free text, token text no longer than
1023 bytes, and decimal conversions representable in int32. Native fixed-buffer
overflow and original-CRT numeric overflow have not been reconstructed; the
safe C++ storage and current CRT are not evidence for those native outcomes.
`SceneToken::line` keeps the existing scene reader's one-based diagnostics;
it does not expose the native zero-based prefetched-character counter.

The ignored focused fixture `local/options_token_reader_fixture.cpp` compiles
the wrapper and shared lexer under MSVC Win32 `/W4 /WX /fp:strict`. It covers
the malformed outcomes above, cache identity, comment boundaries, and unknown
token advancement. No permanent test target was added. The required full
`scripts/build.ps1` run and native seed verification are recorded in the packet
report. Passing implementation fixtures is not native scanner differential
proof, ABI compatibility, or game validation.
