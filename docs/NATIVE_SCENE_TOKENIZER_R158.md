# Native scene tokenizer: stream owner and scanner (R158)

The raw settings loader `008D8190` uses the scene tokenizer constructed by
`008D9F20`. Its storage and ownership differ from the BEE-family language-file
scanner. This packet supplies seven complete normal bodies over the actual
`838h` object in `native_scene_tokenizer.hpp/.cpp`. Existing `SceneLexer` and
`OptionsTokenReader` remain semantic projections; startup has not switched to
this raw owner yet. Names are descriptive hypotheses, not recovered symbols.

| Entry | Bytes | Native contract | Source behavior |
| --- | ---: | --- | --- |
| `008D8900` | 93 | ECX owner, AL byte, RET | Cache current byte, signed cursor bound, LF counter, sticky byte EOF |
| `008D8960` | 36 | ECX owner, RET | Copy current token including NUL to previous token; clear token cache only |
| `008D8A50` | 24 | ECX owner, tail to `8D8900` | Save previous byte, clear byte cache, return next byte |
| `008D8A70` | 982 | ECX owner, EAX owner+5, RET | Complete token scan, comments, delimiters and quotes |
| `008D9C30` | 184 | ECX owner, RET | Free owned delimiters, decrement/release stream, free buffered bytes, return label to raw pool |
| `008D9F00` | 30 | ECX owner, stack flags, EAX owner, RET4 | Full destructor then `BF65AC` only for bit0 |
| `008D9F20` | 467 | ECX owner, stack stream/extra delimiters, EAX owner, RET8 | Construct label, retain stream, buffer low DWORD of size, append optional delimiters |

## Storage and sequencing

Token text occupies `+005..404`; previous text occupies `+405..804`. Flags
`+805` (token cache), `+808` (byte cache), `+809` (byte EOF) and `+80A`
(EOF captured when starting a token) are independent. `+806/+807` hold previous
and current bytes. The line counter at `+81C` starts at zero and advances when
a LF enters lookahead. `+820/+824` is the actual pooled label header;
`+828/+82C/+830/+834` hold retained stream, buffered data, size and cursor.
The constructor preserves quoted byte `+004` and padding byte `+80B`.

Whitespace is read through the current `E0C940` pointer; default delimiters
through `E0C944`. A nonnull extra-delimiter argument allocates
`strlen(default)+strlen(extra)+2` bytes and concatenates both strings. A null
argument borrows the current default pointer. Destruction compares against the
current default pointer, frees the stream only at its actual atomic zero count,
then clears `+828`. The other freed headers retain their native dangling values.

The scanner preserves comma/NUL whitespace, comments only at token boundaries,
the overlapping block terminator `/*/`, literal backslashes inside quotes, empty
quoted tokens, and the delayed token-EOF flag. The native 1024-byte buffers have
no overflow guard. Inputs that overrun them are outside this source contract.

The default stream services call the existing complete D642C0 memory-stream
methods and zero-reference destructor. This is the actual stream family used
by the options loader. Other stream profiles require an explicit source binding.
String resizing and final return use the actual raw string pool and singleton
manager. There is no mirror string/vector or shadow stream reference count.

## Evidence and repairs

The existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` was verified
for all live batches. All 1,908 bytes in 13 code/data spans matched the original
PE: 1,816 bytes of code and 92 bytes of tables, strings and pointer cells.
The report lists all 29 direct CALL/tail edges and the indirect stream/IAT sites.

Ghidra's destructor body stopped at `008D9CA8`, after a returning free. The flow
repair decoded its missing 63-byte tail; explicit function recreation extended
the saved body through `008D9CE7`. Prior name/comment values were preserved.
The scalar destructor also needed its three-byte `ADD ESP,4` at `008D9F15`
restored. Annotations are saved, read back and exports refreshed. A forced
snapshot updates the graph even though the function count did not change.

## Validation and limits

The strict MSVC Win32 build and all three existing CTests pass. One local
comparison harness executes seven copied original bodies, adapting their CALLs,
IAT entries and stream slots to the established source services. Eight corpus
pairs produce **183 paired observations and 416,044 identical bytes**. Every
observation includes the full `838h` object; its four allocation/stream pointers
are normalized to presence/default-delimiter identity. Live label, buffer and
delimiter contents plus service traces are also compared. Opaque preimages,
cache repeats, direct byte leaves, quotes/comments/NUL/high bytes, 1023-byte
tokens, high size DWORD truncation, scalar flags `100h/101h` and both retained
and zero-reference stream cleanup are covered. Actual stream/backing counters
and the raw string pool drain to zero/null.

Scalar owner deallocation is recorded by the harness rather than freeing its
fixed mapped observation buffer. Stream size/read/release use the real source
bodies; one size adapter supplies a nonzero high DWORD. The copied original's
FH3 handlers are not executed. One source allocation failure retains the native
site/state and stream reference, rejects replay, and is explicitly cleaned up
by the diagnostic caller. This does not reproduce native exception unwinding.

The source interfaces add contexts and operation records; they are not binary
ABI replacements. Arbitrary vtables, buffer overflow, malformed pointers,
asynchronous mutation, FH3/SEH and gameplay are unproven. The raw settings typed
readers, load path, application ownership and native-game admission remain open.

Evidence: `reports/native_scene_tokenizer_r158.json`; reproducible local harness
and sealed artifacts under `local/scene_tokenizer_r158` and `local/evidence-r158`.
