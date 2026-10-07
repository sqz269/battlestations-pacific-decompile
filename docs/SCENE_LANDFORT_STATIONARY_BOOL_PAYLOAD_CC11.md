# Owning Boolean data for the LandFort Stationary read

Addresses: 004F0FB0 (owned); 008F2260, 0048E9F0, 0043B8B0, 00964790,
00748C40 (existing supporting or unresolved dependencies).

Packet `cc11_scene_landfort_stationary_bool_payload`, baseline
`50892f7c9cfbfaf3516e1df0c0fcb5577a6391e5`. The only behavior change is the
Stationary property read in `src/scene_unit_creators.cpp`. A private actually
called reader prefers `boolean_value != 0` for case-insensitive B with
`has_boolean`; otherwise it calls the unchanged global `scene_property_bool`.

The existing creator still resolves its descriptor first and stops on null,
then admits Stationary only through the same row.extra_key and properties
gates. Its false default, first-literal raw conversion, non-B/raw/missing/empty
compatibility behavior and every placement, hierarchy, name and command step
are unchanged. Only the actual LandFort row has this extra key; CommandBuilding
has none. No header, global reader, other creator or enum/default policy changes.

## Native stored-value receipt

Read-only verification used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The complete inspected listing for 004F0FB0 has90
instructions, body `004F0FB0..004F10A8` inclusive, end exclusive `004F10A9`.
The stored signature remains undefined(void); RET10h at 004F10A6 is a terminal
receipt, not a new C++/native ABI claim.

This packet binds only the stored-byte fragment `004F0FC9..004F0FDA` inclusive,
end exclusive `004F0FDB`:

| Site | Instruction/target | Evidence |
| --- | --- | --- |
| 004F0FC9 | PUSH00CEA044 | Stationary key |
| 004F0FCE | MOV ECX,ESI | Existing property bag |
| 004F0FD0 | CALL008F2260 | Property lookup |
| 004F0FD5 | CMP byte [EAX+0C],0 | Direct retained-byte read, no type/null guard |
| 004F0FD9 | JZ004F0FFC | Zero selects the ordinary branch |

The preceding `004F0FB7..004F0FC8` inclusive (end exclusive004F0FC9) calls
0048E9F0 at004F0FC0 and tests AL. Its existing normal body builds a key string,
calls0043B8B0 and returns a nonzero-result predicate; the map/provider beneath
0043B8B0 remains unbound here. The proposed older `SCENE_STATIONARY_FORT.md`
dependency is absent, so it is not treated as closed evidence.

When the stored byte is nonzero, the existing native branch finds Type at
004F0FE2, resolves through00964790 at004F0FEE and calls00748C40 at004F0FF5.
The zero branch finds Type at004F1003, resolves at004F100F and dispatches the
descriptor's vtable+28 at004F101D. These are dispatch receipts only; no new
descriptor, allocation or constructor contract is claimed. Source's existing
descriptor-first order differs from the native prefix-first order and is
deliberately preserved. Native missing/type/fault behavior is not Source's
null-stop/raw/false compatibility policy.

The accepted [owning-B evidence](SCENE_TYPED_BOOL_PAYLOAD_CC11.md) supplies the
recognized explicit true/false literal producer, native type3/+0C constructors,
byte clone/assignment and Source owning byte/presence storage. This admission
includes one recognized ASCII case variant followed by a closed semicolon;
presence retains real false. It does not cover malformed, multitoken, implicit,
empty-existing, declaration-conflict or enum identity, and does not persist a
native parser success/action flag.

## Connected Source evidence

`local/cc11_scene_landfort_stationary_bool_payload/probe.cpp` includes the real
creator TU and production scene-contents TU, exposing the actually called
private reader and actual PropertyLibrary/group services. The parser is compiled
separately; the creator also receives a strict standalone compile. The standalone
object is not linked because the included TU supplies the same creator symbols.
The public creator and its call connection are compiled, but are not executed:
no fake descriptor, allocation, class, enum or provider is supplied to force it.

The one focused fixture checks owning copies, recognized true/TRUE/false/FALSE,
lowercase B, cleared/opposing diagnostics, no-own B first-literal raw policy,
non-B metadata exclusion, missing and raw-empty compatibility. Bare B and
manually Source-authored non-B raw fields exercise Source fallback only.
In particular F true is outside the admitted parsed finite-decimal F domain;
that legacy compatibility case is constructed as Source data, with no native
F true parser claim.

Actual installed parsing finds15 library files and22 groups. LandFort at
`universe/library/landfort.props` line12 supplies Stationary=B true, inherited
through the real LandFort(Common,MultiEntity) group. Its merged owning true
survives an independent copy and diagnostic removal/reversal. USN1 contains12
actual LandFort authored Stationary=B false bags, whose ordered group/authored
merge overwrites that default and retains false after the same diagnostic edits.
Their lines are1238,2731,2752,2773,2794,2815,2836,3035,3282,3301,3320,3339.
CB2's false declaration at1316 is a separate CommandBuilding witness: the real
creator row's null extra_key excludes it. JM06 parses successfully and has no
authored Stationary witness. These are actual parser/group/reader checks and a
compiled-caller connection, not whole creator/SceneReader/emitter/game execution.

Level=E CommandBuildingLevels:Basic remains independent and unresolved as an
actual retained declaration/provider binding. The installed Basic=0 table and
the Source raw symbol map alone do not supply native E declaration/+28 identity;
this packet introduces neither integer0 nor a default or enum provider.

## Checks and qualification

Strict fresh Win32 compile, manifested link and focused fixture exit0/0/0.
Flags include `/W4 /WX /fp:strict /MD /O2 /Gy`, and the PE is machine014C with
embedded asInvoker manifest. The receipt records179 active Source/Lua inputs
(178 actual included paths plus the separate parser source),17 installed inputs
and77 immutable compiled supports, all stable pre/post.

All three libraries and74 game objects were frozen together from root's current
compatible full build at `e0dfeae01eb2a1f0b87d8ecf86b1fe1056510cf1`, after the
owning-I/B consumer rebuild. Core SHA256 is
`8ECD635AD747085DF88F20E0B5460F4D888989AFB73871CBEC08940B02DAA717`.
Every copied input has equal original-pre/original-post/pinned hashes. game_main
and freshly included scene-contents are excluded. The linked factory support
contains the prior gate adoption but the gate is not executed in this fixture.
No old pre-I consumer binary or live build input is used at link time.

The ignored recipe, manifest, immutable support, compile/link/probe logs and
native-call verification log carry exact receipts; the report hashes them.
Seven direct native rows check with0 failures, with the indirect vtable dispatch
kept separate; `git diff --check` passes. No tracked test or worker full build was
added. Root owns integrated build/CTest and independent linked verification.

The full native creator, prefix/map and descriptor providers, Stationary native
allocation/instance layout, original parser context/type/fault/alias/ownership/EH/
reentry, historical numerical/FP behavior, ABI and original-game parity remain
unclaimed. Prior source group capture/merge proof keeps its partial native
merge-tail qualification (decoded exclusive008F566F, stored body ending008F5658).

## Primary integration

Main `ac58ee9db8a9818a074aeba4e07ba52e4d29d527` passed the full Win32 build and all three existing CTests. Root independently reviewed all new code, native instructions and the entire connected fixture, rebuilt it against current main inputs, verified before/after Source, support and original PE hashes, and checked the embedded asInvoker PE32 manifest. Detailed independent counts and the executable hash are recorded in the primary_integration receipt. The scope and native ABI/game limitations above remain. No tracked tests were added.
