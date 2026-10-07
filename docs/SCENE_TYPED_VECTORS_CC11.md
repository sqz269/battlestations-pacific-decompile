# Explicit V3 storage and path consumption (CC11)

Explicit `V3` assignments now retain an owning three-float payload and a
presence flag in `SceneProperty`. A present empty `V3 ;` stores three positive
zeros, while a missing property stays missing. The existing path-point consumer
copies this payload directly; diagnostic tokens are retained. This binds the
native data/storage contract, with a modern source CRT conversion provider for
nonempty input. Historical VS2005 numerical parity is **unverified**.

Baseline: `733539d5462462471094dd10c79ad0dd93c6d096`. The primary fixed the
branch, storage and provider contract before implementation. Read-only receipts
used `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, verified
before live batches. Worker Ghidra/ledger/CMake changes: none. No policy,
Hidden, class, mode, director or enum/Lua binding changes accompany this work.

## Native branch and actual consumer

| Routine | Coverage and bound contract |
|---|---|
| `008F5A00` | Partial source projection: explicit V3 branch `008F6276..008F631E`. Existing-record implicit typing and all other unbound typed arms remain partial. |
| `008F3690..008F3700` | Complete normal type-7 record producer read. New `38h` record has three dword lanes at `+0Ch/+10h/+14h`; native allocation/insert/failure/ABI remain external. |
| `008D8F40..008D8F62` | Complete scalar guard read. Peeks and calls VS2005 CRT `00BF7533` using `%f`; return means exactly one conversion. Supporting dependency, not a newly ported native provider. |
| `008D9B40..008D9BA5` | Complete scalar reader read. Same `%f`; consumes only on success and returns through ST0/RET4. Error/recovery and extended-ST0/FP-status parity are not bound. |
| `00BF7533..00BF7551` | Supporting VS2005 CRT wrapper read: `vscan_fn`, library FID matches `_sscanf`/`_sscanf_s`. Its scanner engine and numerical/error behavior are external. |
| `007B34F0..007B3727` | Existing partial path projection: fresh kind-1 holder, `PathPoints`, numeric `Point%002i`, type-6 termination and local Pos copy at `007B354A..007B3604`. Group, holder kinds 2/3, allocation and derived refresh remain outside the source binding. |
| `008EF270..008EF2AE`, `008F0700`, `008F4F60` | Supporting established vector construction/assignment/clone evidence: three owning inline lanes, rather than shared child or text storage. No new merge reconstruction. |

The parser tests type code 7 at `008F6276`, or matches explicit `V3` at
`008F6282`; an existing type-7 record may consume its matching letter at
`008F629A`. `XORPS` at `008F629F` followed by stores at `008F62A4/62AA/62B0`
initializes all three temporary lanes to positive zero. `008F62B6` calls the
first-token number guard. If it fails for the immediate `;`, the parser jumps
to `008F62EF` and performs **no scalar-reader calls**. The guard itself still
attempts `%f`; source empty vectors skip modern scalar conversion entirely.

When the first token is numeric, native parsing makes three unconditional
`008D9B40` calls at `008F62C6/62D6/62E6`, with binary32 stores at
`008F62CB/62DB/62EB`. There is not a per-component guard. Existing records
receive those three dwords at `008F62FF/6302/6305`. New records are created at
`008F6319 -> 008F3690`: ECX bag, stack key/vector pointer, `RET8`. The shared
semicolon expectation is `008F66A6 -> 008D9930`. `008F5A00` has three stack
arguments and `RET0Ch`; no semantic return value is established.

The scalar format bytes at `00CE4334` are `25 66 00` (`%f`). The guard calls
`00BF7533` at `008D8F50`; the reader calls it at `008D9B55`. On success the
reader copies current token `+5` to previous text `+405`, clears cache `+805`,
loads a stored float into ST0 and returns with `RET4`. On failure it invokes
`008D8F70`, returns zero and clears the ok byte. Those failure paths do not
establish safe recovery/rollback for this source packet.

For a fresh kind-1 path, `007B3552` looks up `PathPoints`, `007B357B` finds the
numbered child and `007B35A3` finds `Pos`. `007B35A8/35B6/35C1` load the three
record lanes; stores through `007B35D2` prepare an append at virtual `+8`
(`007B35D8`). The source `retain_path_points` now copies `vector3` directly.
Its existing source-authored raw-bag fallback still accepts exactly three raw
tokens using the unchanged `scene_scan_float`. A typed payload never uses that
fallback. Missing Pos is rejected by the source; the native dereference is
unguarded, so this is not a native graceful-error claim.

Point labels use `004F9A80`, a complete supporting `vsprintf_s` wrapper with
an `80h` destination bound. The two native caller cleanups at `007B3571` and
`007B35F1` are `ADD ESP,0Ch` (destination/format/count). `00415870` supplies
the current source count. The virtual append target/engine allocation remain
external; retaining local XYZ in a C++ vector does not reconstruct them.

## Source provider and admission

Nonempty explicit V3 parsing calls current MSVC `std::sscanf("%f")` directly
into each `float`. It does not use or rewrite shared `scene_scan_float`, whose
current source implementation manually selects a decimal prefix and then
uses `strtod` followed by a float cast. Each successful modern conversion is
retained once in the new binary32 payload; raw token text remains unchanged.

The admitted nonempty domain is a full triple of finite ordinary decimal-prefix
tokens under numeric locale `C`: optional CRT whitespace/sign, digits or a
fraction starting with a digit, and the modern `%f` conversion. Native tokenizer
fragments must be closed, NUL-free and shorter than `400h`; the existing
tokenizer buffer evidence pins `+5..404`, `+405..804` and cache `+805`.
Source checks reject embedded NUL, overlong tokens, hex prefixes, NaN/Inf,
nonfinite conversion, unsupported locale and incomplete triples with
`runtime_error`. Unterminated quotes, extra value tokens, malformed input,
overflow/underflow fidelity, rounding modes, historical CRT/FP status,
allocation, native faults and ABI are outside the admitted binding. Source
exceptions do not claim native partial publication or transactional rollback.
The original record must be absent or have the compatible type-7 descriptor;
conflicting native descriptors, implicit dispatch and structural aliases remain
outside this source parser context, which has no original record pointer.

The immediate-empty branch performs no locale query or scalar conversion:
value initialization supplies three positive-zero floats and sets presence.
Its authored `values` vector remains empty. Thus absence, diagnostic token
count and stored-vector presence are distinct facts. Owning `std::array`
copies carry the payload through existing eager group capture, redeclaration,
ordered parent overwrite and entity fill-missing/authored overwrite. No pointer
to an earlier parent's mutable payload is introduced.

Affected callers are the existing scene header/entity parsers, mission header
load, `mission_scene_probe`, library group parser, weather descriptor parser,
and traffic/group property readers (`scene_traffic_groups.cpp:264/543`). The
direct numeric V3 consumer in this packet is `retain_path_points`, used for
class `47h` in the scene contents pass. Other callers retain their existing
partial coverage; this adds no new weather, traffic, generation or VFS policy.

## Fixture and installed results

One existing scene-parser fixture now checks present-empty positive zeros,
absence, a full three-float payload and the following key, while retaining its
existing string/comma/numeric-prefix/`--` checks. The ignored manifested probe
links a fresh production parser TU and includes the actual path-consumer TU
and amended existing test TU. It also exercises the actual group add/merge
methods on one owning
capture/update scenario. Production raw scene dependencies used by the probe
were recompiled into unique local objects (`scene_unit_creators`,
`scene_entity_factory`, `scene_record_side_blocks`) against the new header,
avoiding old `SceneProperty` strides on those exercised paths. No full build
or tracked helper/test framework was added. The production parser was compiled
without a global CRT-deprecation suppression; only its direct `%f` statement
has a local MSVC warning guard.

The installed scope is all 15 library inputs plus JM06. A bounded literal
inventory, with complete line comments removed, finds 1,199 V3 assignments:
1,196 full triples and three empty declarations. Each inventoried assignment
was passed through the actual parser. The empty declarations are
`path.props:11` (`PathPoint.Pos`), line 18 (`PathPointCamera.RotRefPos`) and
line 19 (`RotRefRot`). Actual eager group capture also retains inherited
`PathPointCamera.Pos` as a typed positive-zero vector. All 22 installed groups
load in the probe's discovery driver, which calls the production property
parser and group add method; private `Impl::parse_library_file`, enum loading
and VFS discovery/open were not executed by that driver.

All 1,196 nonempty modern payloads have zero lane-bit differences against the
**prior source** decimal-prefix converter on this corpus. This is a source
regression check, not proof that those values are native-exact. The historical
VS2005 scanner and FP/error state were not differentially executed.

The actual path consumer yields one positive-zero point for a present-empty
Pos and rejects an absent Pos. Mutating diagnostic tokens leaves the typed
payload result unchanged; source-authored raw fallback remains readable. The
group scenario retains an earlier child's zero snapshot, captures later-parent
XYZ in order, preserves an authored entity value during fill-missing and
copies an authored empty-vector overwrite.

JM06 still has 96 entities in 11 classes, 14 `SubmarineGen` records, three
tutorial records, 40 retained paths and 1,163 retained points, with zero parser
errors. Its MultiType defaults, inherited torpedo-director false, Hidden and
authored engine-movie precedence pass the source probe. In particular,
`PlayerSub 01` authors engine-movie false; the probe preserves that input, rather
than applying the tutorial's inherited true to every submarine. No generation
or actual-game admission claim is inferred from these checks.

Compile and run exit 0; MSVC Win32 `/W4 /WX /fp:strict`, PE machine `14Ch`,
embedded `asInvoker` manifest. The log includes the amended existing semantic
tests' pass statement and the consumer/capture/installed/JM06 receipts above.
The report pins inputs, artifacts and exact native call sites. Full integrated
Win32/CTest validation belongs to the primary, as the public source object size
has changed. Worker did not run CMake, the game or original-native differential
tests. This is a source/fixture and installed-input/consumer result, not native
ABI compatibility or gameplay proof.

## Enum integration remains a separate boundary

Native type-4 records retain a declaration at `+28h`. Clone `008EF230` copies
that identity and the resolved integer; overwrite `008F0700` copies only the
integer. Existing enum authored parsing discards a supplied namespace after
`:` and resolves through the retained destination declaration
(`008F5E6F -> 008F3370 -> 0048E840`). A new enum record instead looks up the
named declaration in `00E1867C` (`008F5FA2 -> 0048E960`) and stores it through
`008F3A20`. First table/symbol retention remains the prior established rule.

Raw `SceneProperty` still lacks enum identity/resolved storage, and the separate
`ScenePropertyBagModel` lacks a declaration handle. Its host enum lookup methods
are declared but unused by the typed parser. A connected binding additionally
needs declaration-aware parser context before group authored parsing, stable
first-table handles, clone-versus-assign metadata rules, entity authored-update
integration and consumers of stored integers. `LUA_S` additionally imports a
live Lua table through `008F2F70`; it is absent from this bounded installed
inventory. Those storage/provider gaps are explicit blockers, not filled by a
global namespace search, fake metadata or an unused wrapper. The prior ordinary
entity flag-1/default and JM06 admission verdicts remain independent.

Primary integration: 7a7b06aaf63200e18847fb6b4630c28bd3a814e9; actual main sources independently compiled for manifested focused probe, PASS. Complete MSVC Win32 Release rebuild and all three existing CTests passed. Executable SHA256 e4c52573577519e7435eeda2f7bf1aaba70ad8fd52cb782252fdb19b386edbef. Original ABI, actual runtime binding and game validation remain unclaimed. Build receipt: J:\PROG\battlestations-pacific-decompile\local\cc11_vectors_outer_integrated_build.log.
