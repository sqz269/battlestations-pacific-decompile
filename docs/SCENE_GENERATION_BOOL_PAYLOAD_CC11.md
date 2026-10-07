# Retained Boolean values in the scene generation gate

Addresses: 0046C550 (owned); 008F2260, 008F5A00, 008F3940, 008EF1F0,
008F4F50, 008F0700 (existing supporting contracts).

Packet `cc11_scene_generation_bool_payload`, baseline
`e7ce15904665997509d01e2c3d8904429f6f19ca`. This adopts existing owning B data
at four calls in `src/scene_entity_factory.cpp`; it reconstructs no additional
gate service or original ABI. Ghidra was read-only against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

The private called reader prefers `boolean_value != 0` only when the property
has case-insensitive type B and `has_boolean`. Every other input goes through
the unchanged global `scene_property_bool`, including its first-literal raw
policy and null/empty false result. The four replacements are mode8
GenerateInGame, mode9 GenerateInGame, mode9 GenerateInEngineMovie fallback,
and the selected MultiType key after the existing area check. Headers, generic
conversion helpers, Hidden, directors, FakeAllocated, integer data, class
exemptions, geometry, mode selection, record/stock policy and defaults are
unchanged.

The admitted owning values come from one recognized explicit B true/false
literal, including ASCII case variants, closed by a semicolon. Parser storage,
type3/+0C constructors, byte cloning and byte assignment were established by
[the owning-B packet](SCENE_TYPED_BOOL_PAYLOAD_CC11.md). Presence means retained
data availability, including real false; it is not native ephemeral parser
success or a replay action. Empty/existing-context, malformed, implicit,
multitoken, declaration-conflict and enum-identity semantics remain external.

## Native receipts

The containing body is `0046C550..0046CCE0` inclusive, end exclusive
`0046CCE1` (520 listed instructions). Its stored Ghidra signature remains
`undefined(void)`. The table covers only these eleven lookup/load fragments;
it does not promote the pre-existing Source gate to a complete native binding.
Every listed Find calls `008F2260`, then reads byte `record+0C`; none checks the
lookup result or record type. Modes0..7 use the MultiType child; modes8/9 use
the parent bag in ESI. Ranges below are inclusive, with explicit exclusive ends.

| Read | Fragment | End exclusive | Find | Byte read |
| --- | --- | --- | --- | --- |
| mode0 MultiIslandCapture1v1 | 0046C9AC..0046C9BF | 0046C9C0 | 0046C9B8 | 0046C9BD MOV AL |
| mode1 MultiIslandCapture2v2 | 0046CA23..0046CA36 | 0046CA37 | 0046CA2F | 0046CA34 MOV AL |
| mode2 MultiIslandCapture3v3 | 0046CA9A..0046CAAD | 0046CAAE | 0046CAA6 | 0046CAAB MOV AL |
| mode3 MultiIslandCapture4v4 | 0046CB01..0046CB14 | 0046CB15 | 0046CB0D | 0046CB12 MOV AL |
| mode4 MultiDuel | 0046C8BE..0046C8D1 | 0046C8D2 | 0046C8CA | 0046C8CF MOV AL |
| mode5 MultiEscort | 0046C935..0046C948 | 0046C949 | 0046C941 | 0046C946 MOV AL |
| mode6 MultiSiege | 0046C7D0..0046C7E3 | 0046C7E4 | 0046C7DC | 0046C7E1 MOV AL |
| mode7 MultiCompetitive | 0046C847..0046C85A | 0046C85B | 0046C853 | 0046C858 MOV AL |
| mode8 GenerateInGame | 0046CC5B..0046CC70 | 0046CC71 | 0046CC62 | 0046CC67 CMP zero |
| mode9 GenerateInGame | 0046CB43..0046CB58 | 0046CB59 | 0046CB4A | 0046CB4F CMP zero |
| mode9 GenerateInEngineMovie | 0046CC37..0046CC45 | 0046CC46 | 0046CC3E | 0046CC43 MOV AL |

Native required-key absence can fault at these loads; Source keeps its existing
skip/false policies. Preserved cross-type raw fallback is a Source compatibility
path, with no claim that a native type0/type1/string record has Boolean meaning.
The existing Source record-created/priorbuilt control flow is preserved;
original allocation, record insertion and stock behavior are not rebound here.
`RET 5Ch` at `0046CCDE` is a terminal receipt, not proof of the new C++ ABI.

## Connected Source fixture and installed input

The ignored fixture is `local/cc11_scene_generation_bool_payload/probe.cpp`.
It uses the real parser, private production PropertyLibrary/group merge code,
and public generation gate. The Source SubmarineGen registry lookup returns8;
the exempt-class set is explicitly empty. Local and identity-parent frames are
actual bound arrays, the parent identity is null, and the pose resolver rejects
any unexpected request. Fixture mode/area arguments and stock/deferred event
observations are admitted Source interface inputs. No enum/class lookup or
geometry routine is replaced.

For modes0..7 one finite borrowed area row has the origin strictly inside.
Each selected key is tested against opposite nonselected keys, cleared and
opposing diagnostics, all eight existing slot mappings, an exact boundary and
an outside point. The real production SceneGateBinding still logs its play-area
provider as unresolved and returns a zero row; this is not production multiplayer
execution or a native play-area-table recovery. Mode8 checks generation and stock
without deferred records. Mode9 checks movie fallback, the game branch and
priorbuilt records. Independent owning copies, compatible B preference, no-own
B/non-B raw fallback, missing/empty values, and absent versus present-empty
MultiType are checked through the real gate. Deliberate non-B metadata fixtures
exercise Source fallback only.

Fresh installed parsing finds15 library inputs and22 groups, then resolves14
JM06 SubmarineGen bags with existing ordered group/authored merge. Eight ordinary
bags have game=true/movie=true; the three PlayerSub bags have game=true/movie=false;
the three tutorial TypeB bags have game=false/movie=true. All14 retain their gate
answers after clearing or reversing B diagnostics. The tutorials at JM06
lines5994..6065 author GenerateInGame=B false and present-empty MultiType.
MultiEntity at global.enums lines1772..1787 supplies movie=true and eight true
MultiType defaults. Those three bags continue to skip mode8 and use mode9's
movie fallback; each retains all eight true selected-key bytes. This fixture
does not execute the whole SceneReader, class creator, mission or game.

## Verification and limits

Three fresh actual TUs compile with MSVC Win32 `/W4 /WX /fp:strict /MD /O2 /Gy`:
parser, changed factory, and the probe including production scene-contents.
Compile, link and connected fixture exit0. The resulting PE is machine014C
with an embedded asInvoker manifest. The manifest records180 active Source/Lua
inputs (178 actual included paths plus the two separately compiled sources),
16 installed inputs and77 immutable compiled support inputs, all stable pre/post.

The three libraries were frozen before the registry build, after the completed
owning-I consumer rebuild: core SHA256
`338F924E70CA4FCB4642ED79126AB44D634B3EBB466C893CE552D65AC1F0D821`.
The74 game objects were frozen after root's registry build at
`d87f925047dfcf3c79d345e3ba5c0b62a3e1fa3a`, with unchanged SceneProperty headers;
game_main and fresh scene-contents are excluded. Every original copy has equal
source-pre/source-post/pinned hashes. This mixed support provenance is explicit
in `support_freeze.json`; no old pre-I consumer binaries are used.

`compile.cmd`, `link.cmd`, `recipe.md`, `inputs.json`, `receipt.py`, compiler/link/
probe logs, immutable support and `game_objects.rsp` preserve reproduction
receipts. The report records their hashes. Eleven direct native rows verify
with0 failures; `git diff --check` passes. No tracked test or worker full build
was added; the integrated build and CTests belong to root.

Original property registry/tokenizer context, enum/deferred Party identity,
class-exemption and play-area providers, native stock/allocator/ownership/EH/
fault/reentry, historical numerical/FP behavior, ABI and original-game parity
remain unproved. Prior group evidence still has the separately recorded partial
merge-tail decode (end exclusive008F566F, stored body ending008F5658), not a full
native merger flow repair. Source owning copy/merge proof does not remove that
qualification.

## Primary integration

Main `e0dfeae01eb2a1f0b87d8ecf86b1fe1056510cf1` passed the full Win32 build and all three existing CTests. Root independently compiled the real parser/factory and whole connected fixture containing the actual scene-contents TU, verified181current Source/Lua inputs (178compiler includes),16installed files and77current main compiled inputs before/after linking, and checked the embedded asInvoker PE32 manifest. The actual public gate and all14JM06 bags passed, including8ordinary/3PlayerSub/3tutorial policies. All11native stored-byte fragments separately matched disk/live and their CALLrows passed. Whole-body/provider/runtime/ABI/game qualifications above remain; no tracked tests were added.
