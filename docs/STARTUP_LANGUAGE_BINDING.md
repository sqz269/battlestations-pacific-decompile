# Startup language header and message selection

`startup_already_running_message_from_native_header_008f832b` reconstructs the
message selection at `008F832B..008F83BE`, before `MessageBoxW`. It accepts the
actual eight-byte language header and calls canonical
`equal_native_string_header_00425850` for each candidate. It copies no header
or text and does not create or release a string owner. The existing
`startup_already_running_message(const char*)` remains a C-string convenience;
the misleading private `language_equals_00425850` approximation is removed.

## Original identity and lifetime

At `008F8322`, WinMain supplies `ECX=ESP+0C` to `008F7DB0`. The resolver saves
this pointer at `008F7DD6` and invokes constructor `0041E870` with `"english"`
at `008F7DE5`. Later options assignment reloads the same pointer at
`008F8086` before `0041E350`; registry assignment does so at `008F8171`.
The resolver returns that pointer in EAX at `008F81B6`. Its SEH prologue and
temporary stream/path cleanup are not adopted by this packet.

Every WinMain comparison first pushes a literal and then uses `LEA ECX,
[ESP+10]`. The push moves ESP by four bytes, so each LEA addresses the same
caller header at the original `ESP+0C`. Data is at header+4, length at header+0.
The earlier COM interface occupied the same stack region, which the existing
pseudocode aliases incorrectly. No intermediate C-string owner is returned.

| CALL to 00425850 | Candidate | Selected text/caption addresses |
| --- | --- | --- |
| `008F833E` | english `00D15B14` | `00D16A10` / `00D16A00` |
| `008F8352` | french `00CE432C` | `00D16990` / `00D1697C` |
| `008F8370` | italian `00CE4324` | `00D16920` / `00D16910` |
| `008F838E` | german `00CE4308` | `00D168C0` / `00D168B0` |
| `008F83AC` | spanish `00CE431C` | `00D16858` / `00D16A00` |

All five calls test AL. English is the initial pair and the fallback; the
first match ends comparison. Spanish changes only text and reloads the
English caption at `008F83BA`. The new helper passes the same captured header
address to each canonical call, which reads its data anew on each invocation.
It neither caches a projected data pointer nor changes the header.

`008F83BF..008F83C5` passes `(NULL,text,caption,10h)` to `MessageBoxW`.
Only after that call returns does `008F83CB` read the current header data
pointer. A nonnull pointer causes `008F83D7` to read current length, capture
`length+1` with DWORD wrap, and pass the captured data/size plus literal one
through `00419CC0` / `00BD1510`. A null pointer skips the length read and
release. The header is not cleared. Existing actual-header destruction
`0041DD20` has this release behavior, but the new selector owns neither this
release nor the surrounding message-box call.

## What the projection does and does not preserve

The five candidate literals are nonnull and nonempty. For a nonnull native
data pointer, `00425850` ignores recorded length and compares the C strings
through `_stricmp`. For null data it scans the complete candidate and returns
false. The C-string convenience therefore selects the same message for all
valid stored C strings and every recorded length, including null data; it
does not reproduce the null-data candidate-read trace. No message-selection
bug was established in the earlier projection.

The general comparator has additional behavior the former private helper
could not represent: a null candidate with nonnull native data tests recorded
length, not the first byte. That branch cannot occur at these five callsites.
See [NATIVE_STRING_COMPARE.md](NATIVE_STRING_COMPARE.md) for the complete
canonical contract and CRT boundary.

`GameStartupHost::resolve_language` still builds and returns `std::string`,
using a projected path, file read and `split_option_tokens`. The owned
`run_win_main` caller therefore continues to use the explicitly documented
C-string convenience. There is no available native owner at this callsite.
Constructing a new `NativeString` from that value would create a different
owner; it would not recover the original resolver's identity or allocation
history. No such adapter or invented successful allocation/registry result
was added. Adopting the new API in the executable requires an actual-header
resolver and owner that survives the message box, with the original cleanup
order. Full resolver I/O, tokenizer behavior, SEH and WinMain closure remain
outside this packet. The entrypoint scope in
[NATIVE_ENTRY_BOOTSTRAP.md](NATIVE_ENTRY_BOOTSTRAP.md) remains applicable.

## Evidence and checks

Read-only live queries used the configured `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, with target verification before each query. Live
function count was 63100. WinMain's selection and release have no listing
gaps. The resolver's skipped `008F8009..008F800F` bytes are
`8D A4 24 00 00 00 00` (`LEA ESP,[ESP+0]`), between an unconditional jump
and its target, not an omitted executed owner transition. Raw fragment bytes
and their hash are retained in `reports/startup_language_binding.json`.

Strict MSVC Win32 Release `/W4 /WX /fp:strict` build passed, as did both existing
CTests and all eight existing native seed checks. One ignored focused fixture
uses the existing `NativeString` constructor/destructor and verifies eight
message cases, projection agreement and unchanged header bytes. It covers
null data, empty text, unknown text, each language and mixed case, including
the Spanish caption. It is a source integration check; it does not execute
copied WinMain instructions, the original entrypoint, registry I/O or the game.
No permanent tests were added.

The fragment's original output is ESI=text, EDI=caption with fallthrough at
`008F83BF`; it has no independent function ABI or RET. The new C++ struct
return is not a drop-in binary replacement. Names are descriptive hypotheses.
No full WinMain, native resolver ownership, original CRT locale equivalence,
or game validation is claimed.
