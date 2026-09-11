# Archive text writer
Addresses: 00BD8C80, 00BD6CC0, 00BD8CD0, 00BD6990, 00BD6AC0, 00BD6BD0, 00BD7B30, 00BD7F50, 00BD6100, 00BD49B0, 00BD48F0, 004CACA0, 00781740, 0043BDF0

`ArchiveTextWriter` supplies real archive text output behind both
`SettingsWriter` and `ProfileArchiveWriter`. It reuses `GuiLuaVariant` keys and
`SettingsValue` scalars, supports name/index/float-index sections, and exposes
the native writer's depth. The descriptive names remain hypotheses.

The native temporary is 8 bytes: vtable at0 and depth at4. The table00CE4104
contains00435640, 00BD8C80, 00BD6CC0 and00BD8CD0 in its first four slots.
Callbacks007FA670 and007FA220 construct it with depth0, emit settings/profile,
then restore00CE3784 before queuing the next callback. Writer retirement emits
nothing and does not close or flush the shared sink. The deleting destructor
bytes00435640..0043565E restore the base vtable and optionally free `this`
when the flags argument has bit0. The primary independently verified31 bytes,
defined and saved that entry, and the worker refreshed its read-only export;
see the primary's `reports/profile_persistence_definitions.json`.

The C++ writer contains references and two host interface bases. It is not an
8-byte object and is not ABI compatible. Its default destructor follows the
observed stack-writer retirement behavior; it does not model scalar-delete ABI.

| Native entry | ABI and evidence | C++ behavior |
|---|---|---|
|00BD8C80|ECX writer,8h key by value, RET8 at00BD8CBE|indent, key, `"{\n"`, increment depth|
|00BD6CC0|ECX writer, no stack args; tail JMP00BD6AC0 at00BD6CF3|decrement, indent, `"}"`, row terminator|
|00BD8CD0|ECX writer,8h key then8h value, RET10h at00BD8D0D|indent, key, scalar, row terminator|
|00BD6990|ECX depth; no stack args|one tab per depth; the initial space fill is overwritten by09h|
|00BD6AC0|CL root flag; no stack args|LF for root, comma+LF otherwise|
|00BD6BD0|owned8h native string by value, RET8|emit literal fragment; native releases temporary storage|
|00BD7B30|CL root flag,8h key by value, RET8|format root or nested key|
|00BD7F50|8h value by value, RET8|tags0..3 scalar formatting fragment|
|00BD6100|owned8h native string by value, RET8 at00BD61B8|whitespace removal and signed byte transform into FILE|
|00BD49B0|ECX manager, pointer to native string, RET4|external compressed-block buffering boundary|

Each Ghidra CLI batch verifies project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base against target.json.
Assembly resolves the pseudocode's missing ECX/CL inputs and x87 argument
promotion. Literal bytes were read through the verified Ghidra CLI.

Exact text rules:

* At depth0, key tag is ignored natively: payload is used as a C-string name,
  followed by `" = "` (00CF8284). Valid host input uses a name key.
* Nested keys start `"["` (00CF5980), append either quote/name/quote for tag0,
  decimal int for tag1, or fixed float for tag2, and end `"] = "` (00D68364).
  Name keys are copied without escaping. Float keys are not truncated to int.
* Section opening appends `"{\n"` from00CE439C. Section closing appends
  `"}"` from00CE4CD4 after decrementing depth, then the ordinary terminator.
* Root rows end LF (00CE4390); nested rows end comma+LF (00CE438C), including
  the last member of a table. Every depth level is one tab09h.
* String values first replace `\\` with `\\\\` using literals00CE7894 and
  00D6837C, then quote with backslash+quote using00CE436C and00D68378.
  The result is wrapped in quotes. CR/LF/TAB and other bytes stay literal.
  The equivalent C++ byte pass only escapes backslash and quote.
* Signed ints use004CACA0 ->004260B0 ->`sprintf("%d")`. Floats use00781740
  ->0043BDF0: FLD binary32 / FSTP binary64, then `sprintf("%f")`. In the C
  locale this is fixed notation with six decimals. Native and host use their
  active CRT locale. Bool reads the low byte and emits `true` or `false`
  from00CE4378/00CE4370.

For example, the emitted bytes for a numeric child section are:

```text
Profile = {
	["TotalScores"] = {
		[2] = {
			["Score"] = 12,
		},
	},
}
```

This is Lua table-assignment syntax based on the observed tokens, not a
generic Lua pretty-printer. Raw quotes in names or raw line breaks in values
can produce invalid Lua. Those native quirks are retained.

`ArchiveTextOutput` exposes the three shared routing inputs explicitly.
With flag0109CEE4 clear and FILE0109CEE0 nonnull, each fragment goes to
`fwrite(data,length,1,file)`. A null FILE routes to the supplied
`buffered_0109cecc` byte sink. `ArchiveStringSink` accumulates those actual
uncompressed bytes for host consumers and roundtrips. The FILE remains owned
by its caller; use binary mode for byte-exact LF output. Text-mode translation
remains the CRT's behavior, as in the native call.

Flag0109CEE4 set selects00BD6100, not memory mode. For every emitted byte it
discards space20h, CR0Dh, LF0Ah and TAB09h everywhere, even within quoted text.
Then SAR CL,4 / SHL DL,4 / OR CL,DL at00BD6175..00BD617C transforms the byte
and writes it through FILE0109CEE0. ASCII resembles a nibble swap; bytes>=80h
do not:81h becomesF8h, F1h becomesFFh. There is no fallback in this mode.

The actual null-FILE manager is not reconstructed here.00BD49B0 copies into
manager+50Ch with used count+520h. A total below10000h is appended; at or above
that threshold it fills the64KB block, calls00BD48F0, and copies the remainder.
00BD48F0 compresses, queues an allocation/length pair through00BD4860 and
resets used count. The previous Ghidra body stopped at CALL00BF6989 at00BD4974;
raw tail through RET00BD49A8 proves the queue/reset. The primary applied a
locked flow repair, but the saved Ghidra function body remained incomplete;
see its separate `reports/archive_buffer_flow_repair.json`. No compressed
save-container or scheduler implementation is implied by the memory sink.

Host safety and remaining boundaries:

* Missing sink, encoded mode without FILE, failed `fwrite`, missing settings
  services, null text, invalid tags, root numeric keys and depth under/overflow
  throw. Native code does not provide those checks and ignores write results.
  A sink exception may leave partial output; there is no rollback or success flag.
* Valid-input fragment order and shared sink routing are retained. Formatting
  is done before emission so a host validation error does not emit a key prefix.
  Native allocator/SEH side effects and allocation-failure ordering are outside scope.
* Extended00BD7F50 tags4..8 and10 are outside the
  existing SettingsValue interface. Their bodies are not claimed reconstructed.
* Current CRT `%f` is used. Old-CRT nonfinite spellings and edge rounding under
  changed floating-point environments are not proven byte-identical.
* Settings text-file persistence and keyboardSetup require real
  `ArchiveSettingsServices` callbacks. Missing services throw; there are no
  successful no-op defaults. Profile-only traversal needs no settings services.

Validation is recorded in `reports/archive_text_writer.json`. The ignored
`local/archive_text_writer_probe.cpp` is one focused fixture for exact syntax,
numeric sections, value/key escaping differences, FILE routing, lifetime,
signed encoding, and explicit missing-service failure. It can be reused by
the primary. This packet does not claim game-save loading or ABI compatibility.
