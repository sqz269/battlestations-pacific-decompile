# Successful nonempty scene F payloads

Packet `cc11_scene_float_nonempty_payload`, baseline
`baa980857519b72c7d49ebd2d9588da03c4b877f`. Names are descriptive hypotheses.
Target: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; worker analysis
was read-only. Exact calls, inputs and artifacts are in
`reports/scene_float_nonempty_payload_cc11.json`.

Successful nonempty explicit F now retains one owning binary32 value in the
actual scene parser. The actual world-map decoder reads that value after its
existing required-property/type checks, rather than converting diagnostic text
again. Source-authored raw fallback and the existing Int/CVTSI2SS path remain.
Bare `F ;` retains its previous raw partial representation with `has_float=false`.
This packet does not bind native empty-F assignment or declaration identity.

## Native producer and important empty distinction

The primary independently reviewed the complete F branch
`008F5BE0..008F5C91`. Per-key `008F5AF4` initializes the local success byte to
zero. Existing type-1 records enter at `008F5C0C`; otherwise explicit F is matched
case-insensitively and consumed before the scalar phase. At `008F5C2E`, the
force-read byte either selects the reader directly or permits the guard at
`008F5C3A -> 008D8F40`.

Successful `008F5C55 -> 008D9B40` returns the scalar, which is stored as binary32
at `008F5C5A`. An existing record is assigned at `008F5C73` only if the local
success byte is nonzero. An absent record instead calls `008F3770` at
`008F5C8C`. A guard miss creates a positive-zero temporary at `008F5C43/46`,
but does **not** make an existing F value zero: that assignment is skipped.
Forced-read failure also leaves success zero. Thus an empty new record and an
empty write to an existing record have different contracts.

`008F3770..008F37D4` is the complete 35-instruction heap producer. It allocates
`38h` at `008F3775 -> 00BF681B`, loads the passed float at `008F3783`, writes
`+0Ch` at `008F378F` and type 1 at `008F3799`, then inserts through
`008F37B9 -> 008F33F0`. Its null-allocation arm calls the same insertion at
`008F37CC`; source allocator/failure behavior is not reconstructed.

`008EF170..008EF1A4` is a complete actual scalar-record constructor, used by
the clone at `008F4FDD`. It writes type 1 and the passed scalar to `+0Ch` by
MOVSS, initializes the observed other fields, and returns at `008EF1A2` with
RET 4; the exclusive end is `008EF1A5`. It is not an existing-record assignment
helper. Existing-record value assignment is separately supported by the type-1
arm of `008F0700`, which copies the `+0Ch` dword. These normal bodies establish
storage; this packet exposes no original constructor/heap/clone binary interface.

The scalar guard and reader use `00CE4334`, actual `%f`, with the original
VS2005 `00BF7533` scanner. The reader consumes the token only on one successful
conversion. The accepted modern source provider is described below; no new
x87 arithmetic, original rounding, ABI or scanner-engine recovery was performed.

## Source storage, consumer and ordering

`SceneProperty.float_value` and `has_float` represent owning source payload
availability. The presence flag is not the native local success byte, an
assignment action, or a claimed original record field. The parser converts an
admitted nonempty F token once and retains its exact raw token for diagnostics.
The lexer and shared `scene_scan_float`/raw decoder are unchanged. Bare F takes
the previous generic raw branch and carries no new typed-float presence.

After type validation, `world_map_bounds.cpp::decode` sets the correct source
Float/reference kind and copies the stored bits with `memcpy`. Native
`004E6C1D/004E6C41` likewise use MOVSS from stored `+0Ch`; Int values continue
through the existing `004E6C16/004E6C3A` CVTSI2SS policy. The reader is used by
actual avoid-zone rebuilding and mission-frame Lua border-zone setup. No caller,
map mode, inverted-bound policy or other runtime flag changed.

Existing owning group/child copies and successful authored overwrite carry the
value and its source presence. No pointer into an earlier parent is introduced.
Native library loading reopens an existing bag and captures ordered parents
before authored parsing; ordinary entity parsing fills group defaults before
authored writes; header parsing starts a fresh bag, then fills missing values
from SceneRootProps. Source's raw header publication does not itself implement
that latter library merge. Empty existing-record writes, implicit/conflicting
typing, duplicate raw records and broader parser context remain deferred; no
permanent empty/read-success action is propagated through resolved groups.

## Provider and admission

The source uses current MSVC `std::sscanf(token, "%f", &float)` directly for one
finite ordinary decimal-prefix token under numeric locale C. Closed, NUL-free
fragments shorter than `400h` use the accepted tokenizer buffer bound. Optional
whitespace/sign, digit or digit-led fraction, and a successful finite conversion
are admitted. Source errors reject unsupported locale, NUL/overcapacity,
nonnumeric/hex prefixes, NaN/Inf and nonfinite conversion.

Unique keys in an authored raw block and absent/compatible native F descriptors
are admission conditions. Extra tokens, malformed input, implicit/conflicting
typing, duplicate-record identity, empty existing-context rules and original
fault/SEH/allocator/reentry are outside this closure. The source checks do not
claim native graceful recovery or transactionality. Historical VS2005 numerical,
FP status, error/range/underflow and extended-ST0 behavior are unverified;
modern C-locale conversion is a source provider, not the original CRT binding.

## Focused verification

One ignored manifested Win32 probe freshly compiles the parser and map-reader
TUs and includes the actual property-library production TU. Active paths never
pass enlarged SceneProperty records into stale objects: parsing, map record
access, group copies and child/overwrite operations are all freshly compiled.
The unchanged raw decoder receives token vectors/ScenePropertyValue, whose
layouts did not change. Older linked engine support objects are not invoked by
the probe. This is focused source evidence; the primary must fully rebuild all
SceneProperty consumers before linking/running its independent main probe.
The core library link input is a leased local snapshot; a shared main rebuild
changed the live library during receipt collection, so the same probe was
relinked and rerun against the pinned copy before final hashes were recorded.

`/W4 /WX /fp:strict` compilation and probe exit 0. Its PE machine is Win32 and
the actual RT_MANIFEST resource embeds `asInvoker`. The fixture checks normal
and decimal-prefix F values, one retained diagnostic token, following-key and
S/V3 preservation, actual map use despite mutated diagnostics, raw Float
fallback, required/type checks and the existing Int branch. Bare F remains raw
and the map adapter retains its previous rejection; this is preservation of a
partial source boundary, not native-empty proof. One owning library scenario
checks early-child capture, nested values, successful nonempty redeclaration,
later-parent overwrite, entity fill-missing and successful authored overwrite.
No new tracked tests or CMake changes were added.

The same 16 bounded installed inputs contain 834 F declarations: 830 nonempty
are parsed into payloads, four empty declarations stay raw. Their 830 modern
stored bits match the prior **source** decoder, with zero differences; this
does not establish native exactness. The 1199 V3 declarations still parse, all
22 installed groups load through the bounded discovery driver, and actual
scene.props/JM06 maps retain their sixteen corner vectors and border sizes
`20000/20000` and `12000/12000`. The discovery driver does not exercise private
VFS/enum loading or full scene/mission runtime. The four raw empty F fields are
camera SustainBefore/SustainAfter/Speed and plane Velocity; no defaults or
consumer binding were invented for them.

Enum/Lua declaration/provider/context/consumer architecture, original allocator,
globals, border tail, native ABI and actual-game proof remain external. The prior
`008F54F0` flow qualification remains decoded tail through exclusive `008F566F`,
stored body ending `008F5658`. Worker did not run a full build, game, native
differential, Ghidra mutation or push. Integrated build belongs to the primary.
The direct-call verifier checks 28 rows with zero failures; `git diff --check`
also passes.

Primary integration: 23587a06a8463dee5a7740e77f1cfc5f6b918317; complete main rebuild refreshed every SceneProperty consumer. Fresh actual main parser/map/library manifested probe passed with all 830 nonempty F source comparisons unchanged. MSVC Win32 Release and all three existing CTests passed. Executable SHA256 55735178cb7d901a6e35041cc76234a4a5837cf61d132f2126992c7f191c49df. Historical CRT, native empty context, original ABI, runtime binding and game validation remain unclaimed. Build receipt: J:\PROG\battlestations-pacific-decompile\local\cc11_float_integrated_build.log.
