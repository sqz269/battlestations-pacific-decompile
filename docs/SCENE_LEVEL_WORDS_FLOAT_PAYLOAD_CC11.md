# Retained Level/LevelUpSeconds DWORDs — CC11

The existing scene level loop now selects its existing field references and
calls the unchanged `retain_scene_raw_word`. Successful explicit `F` payloads
are copied as DWORD bytes independently of diagnostics. The `which=0/1` loop,
key order, selectors, creation gate, defaults, independent success/no-reset
policy, and CaptureRange/Value calls remain unchanged.

Baseline: `1aa42b780`, packet `cc11_scene_level_words_float_payload`. Owned
address: `006F2780`. Owned tracked files are this document,
[the report](../reports/scene_level_words_float_payload_cc11.json), and
`src/game_hosts_scene_contents.cpp`. Ghidra was read-only, targeting
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The descriptive native
name `BSP_CommandBuilding_InitializeFromProperties` remains a hypothesis.

## Native and source data contract

The complete 110-instruction normal body `006F2780..006F2935` was read in this
stream and independently reviewed by primary. Its terminal `RET` is at
`006F2935` (exclusive end `006F2936`). ECX is retained in ESI at `006F2781`;
the holder is object+`C0`, and the property bag is holder+`8`.

| Key | Literal / Find | Present-record DWORD | Native null key | Unit store |
| --- | --- | --- | --- | --- |
| Level | `00CFAE70` / `006F27A8 → 008F2260` | `006F27B1 [record+0C]` | `006F27B6 XOR EAX,EAX` (0) | `006F27BE → +770` |
| LevelUpSeconds | `00CFAE60` / `006F27CC → 008F2260` | `006F27D5 [record+0C]` | `006F27DA MOV EAX,Ah` (10) | `006F27DF → +76C` |

Neither native path tests the record type or numerically converts a float to
an integer. Fresh F7.25 supplies `40E80000`; F-0 supplies `80000000`. The source
reader uses `memcpy`, preserving that representation rather than integer 7/0.
All **14 direct call rows reverified, 0 failed**. Base/static-body `007482B0`,
object+`758` setup `008ED720`, the other property reads, and effective-mode
`004BCA50` remain supporting dependencies outside this adoption. It is not a
complete native initializer or an original-ABI binding.

The previous level loop already had the common reader's exact raw policy:
missing or empty diagnostics skip; case-sensitive `I` first scans an integer;
failed I and every other raw type try the same float scanner, then copy its
bytes; only successful reads assign the field and its presence flag. The new
three-line body selects those same bool/int32 references and calls the reader.
Its definition matches the baseline exactly (hash in the report). Failure
keeps existing state; the two fields remain independent.

Native null keys write 0/10 on the unit. SOURCE skipped-record no-reset is a
distinct preserved policy; fresh record and unit-sink defaults are still 0/10.
Bare `F;` remains unavailable/raw. No empty-F replay flag, enum metadata,
default, public interface, layout, or unrelated caller is added.

## Existing active consumers

`GameUnitsHost` copies the two words at `src/game_hosts_units.cpp:14947`.
The Level and seconds getters at lines 33400/33407 return int32 with existing
class/index fallbacks 0/10. `GameShipAiHost::Impl::build_capture_buildings` seeds
`b.level_770`/`b.level_up_seconds_76c` at `src/game_hosts_ship_ai.cpp:12238`.
`kCommandBuildingLevelBound` is true at line 755; the existing owned-building
tick compares its timer against the float cast of seconds, checks Level >= 3,
and calls the existing level handler (lines 12365/12367). Its 0..3 handler
admission, timer, armour/health effects, flags, and math are unchanged and were
not executed or re-recovered by this packet.

## Installed witnesses and focused source proof

The ignored probe includes the actual production helper/library TU and a fresh
parser. It selects the existing record field references; the production loop
is compiled, but the whole `SceneReaderBinding` loop/emitter is not invoked.
MSVC Win32 `/W4 /WX /fp:strict` compilation and probe execution both returned
**0**. PE `014C` and embedded `asInvoker` were verified. All 77 immutable support
inputs (three libraries, 74 objects), their response file, and 17 installed
files matched pre/post link/probe. Active parser/library/helper paths are
fresh; enlarged records are not passed into cached support objects. Primary
owns the full main build/CTests and independent integration probe.

- Installed 15 library inputs yield 22 SOURCE groups. `commandbuilding.props`
  line 4 gives seconds I10; USN1 `usn_1_marshall.scn` CB2, with `Common`,
  `LandFort`, `CommandBuilding` groups, gives I600 at line 1305. Both match the
  old SOURCE loop.
- Level is `E CommandBuildingLevels : Basic` (`commandbuilding.props:3`).
  `global.enums:1696` declares Basic0, Medium1, Advanced2, Expert3, XLevel4.
  The current SOURCE raw Basic scan skips and leaves default 0 with presence
  false. The fixture preserves that behavior; numeric default equality is not
  enum resolution/declaration/storage proof. The prior anchored installed
  `.props`/`.scn` search found no `Level = I` witness. No installed Level-I/F
  or resolved-native-level claim is made.
- Separate fresh F declarations have no inherited E/I record conflict. They
  preserve `40E80000`/`80000000` after diagnostic removal and owning group
  capture/redeclaration. One focused scenario also checks prefix/signed zero,
  synthetic I-first/failed-I/raw F/non-I/case policy, malformed/missing/
  raw-empty no-reset, independent selectors, defaults 0/10, and CaptureRange/
  Value sentinels.
- Fresh JM06 parsing gives 96 authored entities and zero authored Level or
  LevelUpSeconds bags. This is input inventory, not class/mode/game admission.

Artifacts are `local/cc11_scene_level_words_float_probe.cpp`, `.cmd`, `.obj`,
`.exe`, `local/cc11_scene_level_words_float_scene.obj`, and compile/probe logs;
their hashes are in the report. There is no level-tick or whole-game run.

The [nonempty F parser contract](SCENE_FLOAT_NONEMPTY_PAYLOAD_CC11.md) uses modern
MSVC `std::sscanf("%f")` directly into float once for finite ordinary C-locale
decimal-prefix tokens in closed NUL-free input below `400h`, with compatible
unique-key declarations/owning ordinary groups. Historical VS2005 CRT numerical/
error/FP-status/extended-ST0 parity remains unverified. Native enum storage and
namespace/provider integration, existing-record typing/empty-F context,
duplicate/implicit conflicts, forward/self/cyclic/aliased groups, Lua/VFS,
allocator/EH/fault/reentry/global/lifetime interfaces, native admission, original
ABI, and game behavior remain separate boundaries. Native group-merge tail
repair remains qualified by the existing partial-flow receipt; owning-copy
fixture results do not establish whole native group-merge flow or ABI parity.
