# Logical streamed-dialog playback

Addresses: `00A77820`, `00A77830`, `00A77BB0`, `00A78230`, `00A783F0`,
`00A785D0`, `00A78820`, `00A865F0`.

This packet reconstructs the logical half of streamed-dialog playback over the
existing actual 5Ch objects from `A78150`/`A781C0`. Their producer remains
`sound_alternate_owner.cpp`; actual 20h tables and 14h rows remain the producer
contract in `sound_dialog_table.hpp`. No alternate vector, copied header,
cached field model or invented successful stream service is introduced.

| Routine | Coverage | Original ABI | Behavior |
| --- | --- | --- | --- |
| A77820 | Complete | No args; ST0; RET | Read current F8BBCC+218 float |
| A77830 | Complete | No args; ST0; RET | Read current F8BBCC+21C float |
| A77BB0 | Complete | ECX config1Ch; RET | Clear only references10/18 |
| A78230 | Complete | ECX dst18h, stack src; EAX dst; RET4 | Copy string0/string8/ref10/gain14 |
| A783F0 | Complete normal body and map-derived C++ cleanup | ECX logical; RET | Replace requested stream or restart existing state0/3 |
| A785D0 | Complete | ECX dst1Ch, stack src; EAX dst; RET4 | Copy base18h, then reference18 |
| A78820 | Complete for the two existing constructor profiles | ECX logical, stack dt/gain; RET8 | Advance pending timer; set stream gain; tick stream |
| A865F0 | Complete in valid row-vector domain | ECX table20h, stack name8h; EAX index; RET4 | Equal-length case-insensitive row lookup, miss0 |

All descriptive names are hypotheses. These are new C++ interfaces, with
explicit library and stream services. They are not original ABI or Windows
SEH replacements. Foreign logical vtables are outside the current constructor
domain and raise `logic_error`; no fallback gain is fabricated.

## Ownership and caller order

The active configuration begins at logical+10, pending at+30. Each contains
actual NativeStrings at0/8, a retained reference10, float gain14 and retained
table reference18. A78230 copies both strings through the existing current-
header resize/copy operations, then publishes the incoming reference before
retaining it and releasing the old reference. The gain load/store happens after
that callback. A785D0 repeats the same reference sequence at18. Identity skips
the reference changes but still executes the gain load/store.

A77BB0 releases reference10, writes zero after its callback, writes zero again,
then reloads reference18 and clears it after its own callback. It preserves
strings and gain. Both A786F0 request branches call this helper on the opposite
configuration: delayed requests clear active, immediate requests clear pending.

A783F0 clears request byte4C before releasing the previous stream. It allocates
54h, creates a retained reference argument from current logical+28, then
copy-constructs the by-value NativeString argument from current logical+10.
The A877D0 dependency consumes both arguments on every exit. A783F0 publishes
the returned stream before calling A867B0 with logical+18. It then copies that
current name into logical+8, looks up the row through the current stream+3C
table when the name is nonempty, calls A86670 with the row and logical+24 gain,
and clears active references. A default index0 is used for an empty name.

Without a replacement request, an existing stream is restarted only in state0
or3. The native diagnostic still copy-constructs and destroys a temporary
stream name even though the logging callee004254B0 is a single RET. A867B0
then receives the current logical+8 header and a reloaded stream receiver.

The caller EH map atDEAC90 has two states inDEAC80. State1 usesCB4DBB, which
tails to52E020 to destroy the retained reference argument. State0 usesCB4DB0
to free the receiver allocation throughBF65AC. The state is lowered from1 to0
before enteringA877D0, transferring both argument cleanup duties to the callee.
A failed NativeString copy has no string destructor state. Cleanup callbacks
must not throw; native Windows SEH registration and double-exception behavior
are outside this C++ interface.

## Floating point and lookup evidence

A78820 subtracts dt in x87, stores to float scratch, reloads it and writes
logical+2C before comparing zero with the rounded result. A NaN comparison
sets carry and skips expiry. An expired timer clears4D, sets4C, copies pending
to active and clears pending references. The stream test occurs after that
work. The gain virtual is followed by `FMUL logical54`, `FMUL caller gain`,
and only then a float store. Inline x87 preserves those rounding boundaries
under the current control word. The stream is reloaded separately for the
gain setter and its following tick.

Live vtable bytes establishD58E60 slot0=A77820 andD58E64 slot0=A77830. Both
tiny providers are absent as Ghidra functions at initial review. Raw disassembly
shows MOV EAX,[F8BBCC], FLD [EAX+218/21C], RET atA7782B/A7783B; both RETs
are one byte and exclusive ends areA7782C/A7783C. No worker Ghidra mutation
was made. The primary can define and annotate these after review.

A865F0 captures table vector begin8 and end=begin+countC*14h once. Every row
requires equal current NativeString lengths before calling VC2005 `__stricmp`
atBF7FBF. Two empty headers match without a string-data read. No match returns0,
which is indistinguishable from a first-row match. The source uses the current
CRT `_stricmp`; equivalence of independent CRT locale state is not claimed.

## Service and validation boundaries

`SoundDialogLogicalDependencies` requires the actual A877D0 constructor and
A867B0/A86670/A864F0/A874D0 stream bodies. Peer packets supply them and the
primary composes their contexts. The constructor dependency accepts the owned
argument objects directly and must not add argument copies or retains.
`GameplayEffectComponentLifetime` dispatches current zero-reference slot0 only
after the real Interlocked decrement reaches zero. Strings use the existing
actual-header API and allocator; storage remains live across each callback.

The machine-readable report records every body CALL with its containing
function, indirect references, external caller setup, ABI cleanup, corrections
and exact missing-function boundaries. The focused ignored probe executes
installed A78820/A865F0 bytes, with copy/clear/library/stream services substituted
at the documented boundaries. It compares timer/gain caller behavior and
lookup results; it does not independently validate the substituted copy/clear
bodies, the stream constructor EH, FMOD playback or the game application.

The final strict Release Win32 build passed with `/W4 /WX /fp:strict`, as did
both existing tests. All eight standard seeds matched disk, and the probe's
249 selected bytes independently matched live Ghidra and the installation.
The report audit checked 35 direct CALL rows without failures; indirect calls
remain supported by the explicit listing evidence. The ignored probe passed
four timer/gain comparisons (including quiet NaN and negative zero) and four
name lookups. It links with `/MD` to match `bsp_core` and embeds its manifest.
No permanent test suite is added. Details and limits are recorded in
`reports/sound_dialog_logical.json`.
