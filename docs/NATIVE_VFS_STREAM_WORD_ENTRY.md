# Captured raw stream-word entries

The new dispatch-only overloads of `read_native_stream_word_slot40_00be4320`
and `read_native_stream_word_slot3c_00be4340` expose the two byte-identical
27-byte game-owned bodies through the existing `NativeAdoptedSubstreamDispatch`.
They borrow no string cells or unrelated DB64 empty-storage role. Every old
`NativeResourceStreamReadContext` overload and shared helper remains unchanged.

Each new overload seeds a private DWORD with the FULL incoming optional-count
pointer bits, then captures CURRENT raw stream vptr and slot24 through fenced
four-byte copies. It performs exactly one `source_read(target, stream, &slot,
2, optional_actual)` and returns the low WORD. It does not zero the seed,
byteswap, sign-extend, retry, check short reads or synthesize a count. Null
naturally supplies zero pointer bits; partial reads preserve untouched bytes.

`NativeVfsRuntimeBindings::stream_word_entry` accepts only already-captured
numeric BE4320 or BE4340. It forwards the SAME stream and optional count to the
corresponding overload with SAME `*this`; unknown entries reject before read.
There is no slot40/3C reread, resolver, numeric code invocation, replacement
dispatcher, fake callback or new owner. The reached CURRENT slot24 method still
passes through the existing concrete VFS `source_read` admission and providers.

Saved native AA010A passes an actual count address; AA050B/051D pass NULL. Both
domains retain their original pointer-seed behavior. This bridge neither calls
AA0020 nor establishes parser-wide stream/callback/release custody or raw16
insertion readiness. No existing caller is redirected to the new overloads.

Callers prove live raw stream/table/backing, genuine captured entry provenance,
valid optional count storage when nonnull, SAME dispatch/provider lifetime and
external serialization. Context/private-buffer/argument aliases, invalidating
callbacks and concurrent writers are unsupported. Compiler fences preserve
these raw access epochs; they add no thread synchronization, transitive raw
field guarantee or original fault timing. Existing provider failure domains
remain; no cleanup, release-on-escape, rollback or healthy receipt is invented.

The ordinary Source ABI does not preserve Original ECX/RET4, private argument
slot aliasing, unspecified EAX high bits, FH3/SEH or hardware faults. No vendor
body, loaded locale, production startup or runtime/ABI result is claimed.
Validation is two strict MSVC Win32 object compiles and literal inverse
projection of all four existing files; Root owns canonical build/integration.
`source_ready=false` for full parser/consumer activation.
