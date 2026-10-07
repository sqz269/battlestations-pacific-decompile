# Scene director owning Boolean consumption

Packet `cc11_scene_director_bool_payload`; Source baseline `be7bd776a`.

The four actually called director reads now prefer compatible case-insensitive B
and `has_boolean`, using the independent owning byte. One private reader and
four replacements are the Source change. All other properties, the global
`scene_property_bool`, headers, switches and gunnery code are unchanged.

The existing guards preserve missing-field true defaults and publish the named
table only when any director key is present. A no-key sparse shipyard bag still
creates no scene-table row and retains true. Source reads Artillery/AA/Torpedo/DC
in its existing order; native reads Artillery/Torpedo/AA/DC. The class-8 torpedo
exception remains after the final property read, using the actual Type descriptor
kind and existing predicate. Unknown kinds retain false. Projection still occurs
before the generation rejection and Hidden check; its table can include deferred
rows. Later TorpedoEnable/CLOSEATTACK writes are unchanged.

## Native value receipt

Read-only analysis verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Names are descriptive hypotheses. Every last byte
below is inclusive, with the exclusive end shown separately.

| Containing function | Span | Coverage |
| --- | --- | --- |
| `00822C20` | Stored body `00822C20..00824B57`; end `00824B58`; 2070 listing instructions | Partial Source adoption: the four byte reads below only. All other initialization/services/class/message/controller/EH/ABI operations remain separate. Terminal `RET` at 00824B57 corroborates stored bounds, not a new formal ABI. |
| `00822C20` | Director arm `008238FD..008239A7`; end `008239A8`, 171 bytes | Four Find calls, byte +0C tests and missing/default branches. Live bytes independently read. |
| `00822C20` | Artillery `008238FD..00823923`; end `00823924` | Key 00D09944; Find 0082390B -> 008F2260; null defaults 1 at 0082391F; byte test 00823914; false temporary at 00823918. |
| `00822C20` | Torpedo `00823924..00823959`; end `0082395A` | Key 00D09934; Find 00823932 -> 008F2260; byte test 0082393B; missing/nonzero selects 1 at 00823955. Present zero retains the prior class8 test at 0082394A and zero temporary 0082394E. |
| `00822C20` | AA `0082395A..00823980`; end `00823981` | Key 00D09928; Find 00823968 -> 008F2260; null defaults 1 at 0082397C; byte test 00823971; false temporary at 00823975. |
| `00822C20` | DC `00823981..008239A7`; end `008239A8` | Key 00D0991C; Find 0082398F -> 008F2260; null defaults 1 at 008239A3; byte test 00823998; false temporary at 0082399C. |

The key strings were confirmed from live bytes 00D0991C..00D09957. These native
reads guard null but do not check record type. Source preference is bounded to
compatible owning B; non-B/raw fallback remains a Source compatibility policy,
not native untyped fault/type parity. The prior recognized type-3 producer,
constructor and byte clone/assignment contracts are in
[SCENE_TYPED_BOOL_PAYLOAD_CC11.md](SCENE_TYPED_BOOL_PAYLOAD_CC11.md). Owning
`boolean_value`/`has_boolean` are data, including false; they are not native
match-success, +2Ch or a replay action. Empty-existing/context rules are external.

The class8 indirect call is supporting prior evidence only; see
[TORPEDO_DIRECTOR_CLASS8_CC11.md](TORPEDO_DIRECTOR_CLASS8_CC11.md) and the
separately qualified JM06 Source host pair. No class provider or class/message
ABI is newly bound. The earlier message/controller evidence carries the four
bytes to 007219C0's +220h..+223h; this packet does not port the whole message.

## Installed inheritance and connected Source fixture

The 17 inputs are 15 installed libraries/22 groups and USN1/JM06. Only four
authored director declarations occur: ship.props lines 6 Artillery true,
7 Torpedo false, 8 AA true and 9 DC true. Sub(Ship), lines 199..203, inherits
them. There are zero authored mission director keys. The actual parser and
production PropertyLibrary ordered merge/copy independently confirm:

| Resolved mission bags | Each key present | Each key missing | Values before class8 override |
| --- | ---: | ---: | --- |
| USN1: 147 entity bags | 14 | 133 | Artillery/AA/DC true; Torpedo false |
| JM06: 96 entity bags | 40 | 56 | Artillery/AA/DC true; Torpedo false |

These are 54 resolved bags/216 owning values, distinct from four authored
declarations. Ship/Sub and all resolved values pass independent owning copies,
cleared diagnostics and opposite diagnostics. Recognized true/TRUE/false/FALSE,
case-insensitive B, no-own B raw FIRST behavior, Source-authored non-B metadata,
missing and bare B Source fallback also pass.

A fresh independent four-B declaration passes its cleared/copied owning values
through the real named table and existing `apply_director_stance_008624c0`
data-only bridge. The mixed values produce Artillery other-target bit 1, AA
plane bit 0, Torpedo mask 0 and DC masks 3. Existing table defaults/absence and
a later real torpedo table write also pass. These are plain Source-state calls;
they do not force class/enum/descriptor/VFS providers or execute whole
SceneReader/Gunnery. The actual Source consumer is compiled: SceneReader's
named table is read by Gunnery's existing actual ship-family query6 branch,
which copies all four fields into the stance and calls that bridge.

Three fresh strict MSVC Win32 TUs compile (parser, standalone changed consumer,
probe including production SceneContents). Compile/link/probe are 0/0/0;
PE014C with embedded asInvoker. All 179 active Source inputs (178 project/Lua
includes plus separate parser), 17 installed files and 77 support pre/post
hashes match. Current fully rebuilt same-header 7b835976e inputs were frozen
together: three libraries/74 game objects, original-pre/copy/original-post
equality. The existing bridge comes from this frozen current core; no old or
live build support is linked. Exact recipes/hashes are in the ignored artifact
manifest and JSON report. No tracked tests or worker full build are added.

Only explicit recognized one-literal closed-semicolon B and the existing admitted
group domain are native-bound. Malformed/multitoken/nonliteral/implicit B,
empty-existing/declaration conflicts, enum identity/Lua/locale/NUL/native
tokenizer/allocator/reentry/faults remain external. Source raw fallbacks do not
admit them as native behavior. The prior 008F54F0 tail decode did not extend
stored body metadata beyond 008F5658; no flow repair occurs here. No whole
initialization, class/message/controller ABI, whole SceneReader/Gunnery,
original-game/runtime proof or new provider closure is claimed. Primary main
full build/CTest and independent linked verification remain integration steps.

## Primary integration

Main C++ `bc083d99d` passed the full Win32 build and all three existing CTests; metadata head `b7b612e3176b67fd9a0ce9e976655e5fe2cec741` supplied the same Source. The independent fixture used3fresh TUs,180current Source/Lua/fixture inputs with178compiler includes,17installed files and77current supports. It reproduced243resolved bags and216owning values in54bags, plus the actual named table/plain-state gunnery mask bridge. All171fragment bytes matched disk/live and four direct Find rows passed. Whole SceneReader/Gunnery/class8 execution remains unclaimed. The PE32 asInvoker manifest was verified. Existing runtime/ABI/game qualifications remain. No tracked tests were added.
