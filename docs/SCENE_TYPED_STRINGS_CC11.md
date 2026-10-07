# Explicit quoted scene strings (CC11)

The source parser now binds the normal explicit `S` quoted-run branch of
`008F5A00`: consume consecutive tokens whose quoted flag is set, decode each
fragment, append without separators, and retain one owning string value. The
lexer stays raw. This corrects split values, undecoded escapes, and quoted
punctuation being mistaken for property terminators.

Baseline: `c6cdf63d3ad89b14a8242a311437d19b249c7eca`. Read-only analysis used
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, verified before
live batches. Names below are descriptive hypotheses. No Ghidra, ledger, mission
policy, mode, class, Hidden, VFS, or director-switch changes accompany this work.

## Native producer and storage

| Routine | Evidence and coverage |
|---|---|
| `008F5A00` | Partial binding: explicit type-2 quoted-string arm `008F5CC2..008F5D4A`. The surrounding descriptor dispatch, other types and implicit-type forms remain partial source projections. |
| `008EFB00..008EFBBE` | Complete normal quoted-run collector read; allocator, EH and native fault behavior remain external. |
| `008EE670..008EE6D2` | Complete 99-byte decoder read; text semantics bound, static storage and register ABI external. |
| `008D8A70`, `008D9980` | Complete supporting tokenizer bodies read in this stream. Native quoted byte is `+4`; token text is raw, including backslashes. Existing `SceneLexer` remains an owning source projection. |
| `0094FF10..0094FF49`, `0094FDC0..0094FF04` | Complete supporting append/indent bodies read. The latter has a discontiguous jump from `0094FDC3` to `0094FEC8`; no separate wrapper inferred. |
| `009502E0..0095031F`, `00950240..009502DD`, `009500A0..009500EC`, `00950320..00950343` | Complete supporting normal builder bodies read: initial page, raw append, flatten, temporary cleanup. Not reconstructed here. |
| `008F3370..008F33B7`, `008F38A0..008F3930`, `00438E40` | Supporting normal existing-record/string-record/copy producers read. Only type-2 value semantics support this binding; enum fallback and record metadata remain external. |
| `008D9F20..008DA0F2` | Supporting buffer producer: two `400h` clears at `008D9F67` and `008D9F79`. Existing tokenizer reconstruction and R158 evidence independently pin those buffers. |

`008F5CE6` peeks the tokenizer; `008F5CEB` tests byte `[ESI+4]`, rather than
comparing its text to punctuation. At `008F5CF3` the caller pushes ESI (the
tokenizer); `008F5CF4` sets ECX to the bag and `008F5CF6` calls `008EFB00`.
The collector never reads incoming ECX: `008EFB52` loads its sole stack argument
from `[ESP+38h]` after its prologue. `008EFBBC RET 4` establishes stack cleanup;
`008EFBAE MOV EAX,ESI` returns the flattened allocation. No bag receiver is
invented from the caller's ECX setup.

The collector peeks at `008EFB58`, tests `+4` at `008EFB5D`, consumes at
`008EFB64`, passes the token pointer in ECX to the decoder at `008EFB6B`, and
appends the returned pointer at `008EFB75`. Its next peek at `008EFB7C` repeats
only while the quoted flag is set. Thus even quoted tokens whose entire text is
`;`, `{`, or `}` are data. The next unquoted token remains available for the
parser's terminator and following key.

The decoder receives ECX and returns `00F88C50` in EAX. Ordinary bytes copy
unchanged. Exactly `\\`, `\n`, and `\q` emit backslash, LF and double quote;
other backslash pairs emit nothing. Comparisons are case-sensitive
(`008EE695..008EE6A2`). NULL input specially clears the first scratch byte.
A solitary trailing backslash advances the pointer past the terminating NUL;
the native overread is not modelled as safe error recovery.

The collector's temporary builder starts with indentation depth zero
(`008EFB1C`, builder `+8`). Consequently `0094FDC0` inserts no indentation.
`00D162CC` contains `40 02 95 00`, selecting `00950240` for append slot zero.
That body copies bytes into `800h` payload pages. `009500A0` allocates total+1,
copies the pages and adds the final NUL. There are no separators and no inferred
aggregate `400h` limit: that bound belongs to individual tokenizer fragments.

For an existing type-2 record, `008F5D04 -> 008F3370` frees the old `+0Ch`
string and duplicates the collected value through `00438E40`. For a new string
record, `008F5D1E -> 008F38A0` creates a `38h` record, writes type 2 at `+4`
and a duplicated string at `+0Ch`, then inserts it. The collector allocation is
freed at `008F5D0A`/`008F5D24`. Record construction leaves fields including
`+28h` uninitialized; no retyping, enum metadata or native layout contract is
added to `SceneProperty`. Its vector now contains one owning source value.

The live parser listing still omits returning-free gaps. Bytes at `008F5D0F`
are `83 c4 04 e9 88 09 00 00`; bytes at `008F5D29` are
`83 c4 04 e9 6e 09 00 00`: `ADD ESP,4; JMP 008F669F`. These are read-only
receipts, not a worker flow repair. The parser terminates at `008F6784 RET 0Ch`
with an ECX bag and three stack arguments; no semantic return value is
established. Separately, the prior group-merge repair decoded the tail ending
exclusive `008F566F`, while stored `008F54F0` body metadata still ends at
`008F5658`. Neither fact supports a full parser/merge body-repair claim.

## Source admission and fixture

`parse_property_body` uses existing `SceneToken.quoted` and decodes each run
before concatenation. It does not decode the joined result again. The one
existing scene-parser fixture was extended with adjacent path fragments,
quoted `;/{/}`, recognized and unrecognized case-sensitive escapes, and a
following integer property. It checks one retained string per `S` assignment
and that the following key survives. Existing comma, numeric-prefix and `--`
checks in that fixture remain intact.

This binding admits well-formed, closed quoted fragments with at most 1023
bytes, no embedded NUL and no solitary trailing backslash, under the source
parser's existing finite input/token budget. `008D9F53..008D9F79` clears
`400h` bytes at `+5` and `+405`; the cache at `+805` confirms the second buffer
boundary. See `NATIVE_SCENE_TOKENIZER_R158.md` storage/overflow qualification and
`native_scene_tokenizer.hpp`'s `400h` admission. The three byte risks explicitly
throw a source `runtime_error`. Unterminated quotes and other malformed input
are outside the binding, rather than a newly claimed native recovery mode.

Native scratch capacity beyond the producer's admitted token size, aliasing,
reentry, thread safety, allocator/EH/fault behavior, partial publication and raw
record ownership remain external. Unquoted `S`, implicit descriptor-driven
strings, enum/reference/child/numeric typing and existing metadata/redeclaration
semantics are not extended by this packet. Source exceptions do not establish
native rollback or graceful failure. This is not a binary-compatible replacement.

## Installed input effect and mission consumer

The bounded input inventory comprises `global.enums`, all 14 installed library
`.props` files, and installed JM06. A literal quoted-`S` assignment inventory
found 191 records, zero adjacent runs and one escaped assignment. The actual
production parser parsed each inventoried assignment: 190 values retain their
bytes and one changes. This inventory is not a full VFS scan or a claim about
every installed mission/string.

JM06 line 548 authors `StageScript = S "COTP-IJN\\PRCPIJN\\JM06";`.
The new value is `COTP-IJN\PRCPIJN\JM06`, with one backslash per separator.
The actual mission host first looks up `GameStageScript`, falls back to
`StageScript`, and takes `values.front()` into script slot 8
(`game_hosts_mission.cpp:1229..1235`). Its enabled stage-script policy then feeds
that value to the actual `mission_script_path` composer
(`game_hosts_mission.cpp:1718..1725`, `mission_scene_load.cpp:97..100`). No policy
flag changed.

The probe links that production composer and gets the exact path
`Scripts/missions/COTP-IJN\PRCPIJN\JM06.lua`. Opening it under the installed
root through Windows `ifstream` reads the 80,310-byte installed Lua file. This
checks spelling and the direct filesystem input; the complete mission host,
VFS/archive provider, Lua execution and gameplay were not run. The decoded
StageScript bytes are intentionally changed, so an all-strings-unchanged claim
would be false. JM06 still parses as 96 entities in 11 classes, including 14
`SubmarineGen` records, with zero source parse errors.

## Verification and remaining proof

`local/cc11_scene_string_probe.cpp` includes the actual production parser TU
and the amended existing math-test TU, then performs the installed input/path
checks above. The manifested MSVC Win32 probe compiled with `/W4 /WX
/fp:strict /EHsc /MD /O2`; it linked the existing main `bsp_core`/Lua/zlib
libraries and required Windows libraries. An initial ad hoc link omitted
`advapi32`/`shell32`; the final explicit library list resolves those existing
dependencies without stubs. The final compile and run both exited 0. The PE is
machine `14Ch` with an embedded `asInvoker` manifest. Its log states
`Reconstructed math semantic tests passed (not binary equivalence)` and the
191/190+1 and JM06 receipts above.

`reports/scene_typed_strings_cc11.json` pins asset, source, probe and log hashes,
the exact compile command and native call-site rows. `git diff --check` and
`verify_report_calls.py` are required before commit. No full CMake build or
game run was started; the primary owns integrated Win32 build/CTest validation.
The result is source-, focused fixture- and installed path-tested. It supplies
original normal text-rule evidence, not original-game differential execution,
native allocator/fault fidelity or ABI compatibility.

Primary review/integration: 3f07b3b71709ebe6afcda21be1ece878da5a871a; MSVC Win32 Release and all three existing CTests passed. Actual parser/consumer TU PASS: one existing scene fixture, 16 installed inputs and 191 quoted S values; 190 unchanged, JM06 script path decodes to accessible installed 80310B Lua file; 96 entities/11 classes/14 submarine instances/0 errors retained. Executable SHA256 086aa79f815f76f700754e3f6cac0f98506332a1879367e49fb6670bcad7fa23. Native ABI and game validation remain unclaimed. Build receipt: J:\PROG\battlestations-pacific-decompile\local\cc11_strings_validity_integrated_build.log.
