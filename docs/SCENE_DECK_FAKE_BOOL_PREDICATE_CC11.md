# FakeAllocated Boolean predicate

Packet `cc11_scene_deck_fake_bool_predicate`; source baseline
`0282872d39a53125d7c40cc1d9c95f8db7b2c92f` includes approved packet metadata.
The actual `read_scene_deck_006cadd0` reader now requires B and uses the existing
case-insensitive **first**-literal `scene_property_bool`. Previously its
integer-first scan accepted I1/S"true"/F1 and its exact lowercase comparison
missed B TRUE. Only that conversion changes. The existing reader/provider,
NumSlots, first-hole behavior, stock/slot order, Type resolution, Count, Arm,
defaults and created/held-back gates remain unchanged. No owning B payload or
parser metadata is added.

Ownership: native `006CADD0`, `src/game_hosts_scene_contents.cpp`, this document
and `reports/scene_deck_fake_bool_predicate_cc11.json`. No header/API, tracked
test, CMake, shared metadata, ledger or Ghidra writes accompany this packet.

## Native consumer and supporting storage

The existing project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was checked for read-only queries. The containing
stored function body is `006CADD0..006CC5BD` **inclusive**, end **exclusive**
`006CC5BE`; this packet inspects only the mode-1 Boolean predicate/state
fragments, not the whole function or its other modes.

The key at `00CF88C8` is `FakeAllocated`. Mode 1 zeroes EBX at `006CAE13`.
Find `006CB22D → 008F2260` is followed by the null test `006CB232/234`, type-3
check `006CB236/23A` and byte+0C test `006CB23C`. The false path writes the
local false flag at `006CB246`. Predicate bytes span `006CB222..006CB249`
inclusive, ending exclusively at `006CB24A`. No integer/float/string-to-bool
conversion is performed by this native consumer.

The predicate feeds `006CB265/27B`. On true, `006CB28B` stores state 6 and
`006CB292` stores positive-zero timer. The existing launch-request test at
`006CB288` can instead load five seconds at `006CB299/2A1` and clear that byte
at `006CB2A6`. These are supporting writes; this packet adds no stock/count,
class-provider, timer-handler or whole-loader reconstruction.

| Supporting routine/arm | Inclusive bytes | Exclusive end | Coverage and evidence |
| --- | --- | --- | --- |
| `00467CC0` match/consume | `00467CC0..00467CE9` | `00467CEA` | Complete normal body read. Peek `008D8A70`, compare `00438E10`, consume `008D8960` only on match. RET4 starts at `00467CE1/CE7`. |
| `008F5A00` Boolean arm | `008F5D4F..008F5DDF` | `008F5DE0` | Partial parser coverage. Existing type 3 or explicit B; true/false match uses `00467CC0`. Existing byte+0C writes at `008F5DC2` only after recognized literal; a fresh record is constructed at `008F5DD6`. |
| `008F3940` heap producer | `008F3940..008F39A0` | `008F39A1` | Complete normal body read. Allocate 38h, type 3 at +4, inline byte at +0C, insert through `008F33F0`. RET8 starts at `008F398B/399E`. Allocation-miss/null insertion is not safe recovery evidence. |
| `008EF1F0` constructor | `008EF1F0..008EF220` | `008EF221` | Complete normal body read. Store byte+0C/type3; RET4 starts at `008EF21E`. |
| `008F4F60` Boolean clone arm | `008F4F8D..008F4FC1` | `008F4FC2` | Partial arm coverage. Source byte at `008F4F9F`, constructor `008F4FA6 → 008EF1F0`, ordinal copy at `008F4FAF`; plain RET at `008F4FC1`. |
| `008F0700` Boolean assignment arm | `008F0736..008F0745` | `008F0746` | Partial arm coverage. Destination type drives dispatch; byte copy `008F073A/073D`; RET4 starts at `008F0743`. |

`00438E10` delegates nonnull comparison to the historical CRT `_stricmp`.
The admitted ASCII Boolean literals need only case-insensitive equality, which
the existing Source provider already supplies. The inline-byte constructor,
clone and assignment evidence supports the native Boolean shape; it does not
introduce a Source payload or establish original allocation, lifetime or ABI
compatibility. All 14 reported direct-call rows mechanically verified, with
zero failures; wider dependencies remain explicit in the report.

## Active Source connection and proof

The private reader is called by existing held-back and created MotherShipGen
and AirField paths. Its `AirOpsSceneSlot::fake_allocated` reaches the existing
`air_ops_load_from_scene_006cadd0`, whose true branch sets state 6/zero timer.
The existing numeric Source class callback remains intact. Enum declaration
identity and the original `007B8A80` class provider are not supplied by this
repair or its fixture.

The ignored focused probe freshly compiles the actual scene reader/helper TU,
parser and Boolean-provider TU. It calls the real private deck reader and
existing Source air-ops loader. Compile and execution returned zero under MSVC
Win32 `/W4 /WX /fp:strict`; PE machine is 014C with embedded `asInvoker` manifest.

- All eight installed MotherShipPlanes Slot1..8 declarations in ship.props
  (lines 113/120/127/134/141/148/155/162) are B false and remain false. The
  Source reader still returns eight authored slots, NumSlots 4 and maximum 12;
  the downstream Source loader retains the existing four-slot limit/defaults.
- Standalone compatible Source fixture fields exercise B true/TRUE → state 6,
  timer 0; B false/FALSE/missing and wrong types I1/S"true"/F1 → false. Type,
  Count, Arm, deck shape and defaults are checked. It uses only the existing
  numeric Source class callback, with no fabricated enum map or actual provider.
- The existing owning raw literal copy still works. Raw diagnostic tokens are
  required; no diagnostic-removal or owning binary Boolean payload claim.
  USN1/JM06 have zero authored FakeAllocated keys, and JM06 freshly parses 96
  recursive authored entities. This is authored inventory, not scene admission.

The bounded 17-file lexical inventory contains 579 explicit B declarations:
460 false, 113 true, three TRUE and three FALSE, with no bare B. Case variation
is installed elsewhere; there is no installed FakeAllocated true/game-effect
claim. Four bare F declarations remain a separate unresolved context contract.

Eight current source/header inputs, 77 immutable support binaries, the response
file and 17 installed inputs have matching pre/post hashes. Current air-ops
source/header match the support snapshot baseline; the called Source loader is
reused, while parser/reader/Boolean provider are fresh. Artifact paths use
`local/cc11_scene_deck_fake_bool_`; complete manifests are in the report. Root
owns the integrated full build/CTests and independent review/probe.

## Admission and limits

The Boolean producer claim admits one explicit recognized true/false ASCII
literal in a closed, NUL-free ordinary compatible declaration. The consumer
type gate also rejects the fresh wrong-type shapes tested above. Implicit,
malformed/nonliteral/multiple-token, empty-existing-context and declared-type
conflict inputs are outside native equivalence. The Source helper returns false
for empty or non-true diagnostics; that is not native graceful fault/recovery
proof. Existing Boolean/empty-F updates need declaration-aware parser context,
not a permanent read-success or replay flag.

Enum +28 identity/namespace/storage/provider, Lua/VFS, native buffers/allocator/
EH/fault/reentry/ownership, full loader modes/stock accounting/class providers
and original ABI remain unproved. Prior `008F54F0` partial group-tail flow
repair stays qualified. The fixture executes the actual data reader and Source
loader, not whole SceneReaderBinding/emitter/Hidden/mode/class admission, AI,
mission runtime or original game behavior.
