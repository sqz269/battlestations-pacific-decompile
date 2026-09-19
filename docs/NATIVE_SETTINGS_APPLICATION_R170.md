# Native settings owner and application loader

Addresses: `00CD2D60`, `00CD2D70`, `00CDEE40`, `00CDEE80`, `00CD2D80`,
`00CDEEC0`, `00CD2DA0`, `00CDEEA0`, `008D8190`, `008D7BC0`, `008D5150`,
`008D6170`, `0073D410`, `004C1E90`.

## Result

Ordinary startup now constructs the actual BCh F88980 settings storage and calls
the complete native settings loader at the represented `0073DAA5` site.
`GameNativeSettingsProcess` retains the settings block, resolution/antialias/
language vector headers, source publication cells, string contexts and CRT
callbacks through process shutdown. The previous projected loader is no longer
called by `GameStartupHost`.

`GameNativeSettingsApplication` binds the existing R167 loader to the same raw
renderer, mounted VFS, retained-memory domain and pooled strings used elsewhere
in the application. It builds the raw language catalog from installed descriptor
files, binds the actual hints owner to shared scalar deletion, copies native
resolution/AA tables, reads or writes the real CRT options file, and runs the
native capability adjustments. The renderer exposes its current publication;
the VFS exposes its existing enumeration and validation contexts.

The remaining UI/audio/locale consumers receive a read-only compatibility copy.
`settings_view_` has no independent initialization or parsing algorithm. Language
rows are copied after the raw catalog is populated. Native BCh and raw vector
storage remain authoritative for this startup path; later options-screen commit
and game/input configuration bindings remain separate work.

## Lifetime and original evidence

The verified CRT table entries CE304C/CE3050/CE3054/CE3058 register resolution
cleanup, AA cleanup, settings construction/cleanup, and language-catalog cleanup
in that order. The native table ignores registration status; the source records
all four actual `std::atexit` results without inventing a registration fallback.

Four newly reconstructed normal bodies total 111 bytes:

| Entry | Bytes | Behavior |
|---|---:|---|
| CD2D60 | 12 | Register CDEE40; preserve atexit status. |
| CD2D70 | 12 | Register CDEE80; preserve atexit status. |
| CDEE40 | 62 | If signed capacity is negative, reserve zero; positive-count loop affects EAX only; capture backing, clear count, free captured backing. |
| CDEE80 | 25 | Resize DWORD vector to zero, reload backing, free it. |

Both cleanup wrappers retain dead pointer/capacity fields after free. The source
uses the existing actual reserve/resize/allocator contracts. These C++ entrypoints
have explicit source contexts and are not original binary ABI replacements.

Ghidra had no functions at CD2D60/CD2D70. CDEE40/CDEE80 stopped at returning `_free`
calls and excluded the final POP/RET. Bounded call-flow repair and function-body
recreation restored those exact tails; prior metadata and comments were recorded.
No global no-return flag changed. The final report verifies 7,294 live/PE bytes
and 232 direct CALL/tail edges across the new bodies and existing dependencies.

The four scanner/tokenizer pointer cells initially target D15F2C/CE5698. Their
source cells borrow those verified mapped literals. Loader-zero storage for the
vectors/settings, empty buffers and online/game/hints publications is checked
against the image. Audio/video constructor constants and the renderer profile
are borrowed from the retained verified mapping.

The application and process share the R169 input publication. Its application
context retires after the shared drain, allowing later settings CRT cleanup to
take the native null-input branch without touching retired Lua/VFS services.
F8ABE8 is now a process cell shared with sound/platform services. Profile reads
resolve the real ordinal5331 from the application's currently bound XLive library
only when requested. That library binding retires after the online lifetime.
The ordinary online and actual game constructors are still unbound; their
publications remain null on this startup path, so no profile SDK read is claimed.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass.
- The unchanged R167 native/source fixture passes 12 loader pairs and 20,428
  matching observed bytes, including its retained-failure/replay case and actual
  CRT text-mode read. No additional test suite was added.
- An ordinary rebuilt application run with a private empty personal directory
  creates a 226-byte native options file, builds one installed language entry,
  copies 25 resolutions and four AA values, selects `englishauthentic`, and loads
  6,856 locale keys. Window/device creation succeeds at 2560x1440; one initial
  presentation is skipped; the loop and shared/COM teardown finish with exit0.
- A second ordinary run loads that written file. Three loop iterations produce
  two successful presentations and one skip, then exit0 with the input binding
  retired and device/API COM counts at zero.
- The earlier published R169 build also completed a baseline startup/drain run
  in this turn; its artifacts are recorded separately.

The application runs use `xlive_stub.dll` and a private options directory. They
do not establish online SDK behavior, visual parity or gameplay. A prepared
800x600/windowed input initially could not launch because a peer held the runtime
slot through the launcher's 60-second wait. The report records any later combined
run of that same input separately, with exact executable/library hashes.

## Limits and follow-up

Native private stack bytes are unknown. Startup explicitly supplies zero
preimages for the tokenizer, desktop rectangle, registry buffer/key/type,
enumeration list word, scanner output words and personal-folder buffer. Native
ignored-output/failure behavior remains in the reused bodies; zero is a source
policy, not recovered stack identity. Source interruptions retain the application
graph and terminate without guessed rollback; native FH3/SEH is not reproduced.

Follow up with the actual online/game owners, input configuration application,
options-screen changes to the raw settings owner, remaining renderer/resource
startup and gameplay validation. Successful presentation is not proof of those
remaining behaviors.
