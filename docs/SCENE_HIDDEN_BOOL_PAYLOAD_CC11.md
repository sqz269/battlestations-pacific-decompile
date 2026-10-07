# Hidden owning Boolean consumption

Packet: `cc11_scene_hidden_bool_payload`. Source baseline: `4036fa1a4`.

The instantiate-pass Hidden read now prefers a compatible case-insensitive B
property's already owning byte when `has_boolean` is true. It otherwise calls
the unchanged `scene_property_bool`: missing or empty diagnostics answer false,
and the first raw literal alone is compared to true without a type gate. One
private called reader and one call replacement are the entire Source change.

The host retains its existing order: class lookup, generation gate, registration
return and record metadata/path retention, gate-rejected return, then Hidden.
Its rejected counters, named spawn-pool entry, wing/deck retention and return
remain unchanged. Native checks Hidden before the generation gate. This packet
does not reconcile that ordering or claim the whole SceneReader is native exact.

## Native receipt

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, verified
before read-only inspection. Descriptive function names are hypotheses. Bounds
below distinguish inclusive last bytes from exclusive ends.

| Containing function | Span | Coverage |
| --- | --- | --- |
| `0046CF40` | Stored body `0046CF40..0046D927`; end `0046D928`, 729 listed instructions | Partial Source adoption: only Hidden byte consumption below. Entity parsing, registry, remaining body, allocation/EH/services and original ABI remain external. `RET 58h` at `0046D925` corroborates the listing tail, not a newly bound C++ ABI. |
| `0046CF40` | Pass/read `0046D39D..0046D3CA`; end `0046D3CB` | `[EDI+4]` compared to zero at D39D; D3B3 skips the read when zero. Existing Source instantiate call-site condition is retained. |
| `0046CF40` | Hidden fragment `0046D3B5..0046D3CA`; end `0046D3CB`, 22 bytes | Push key `00CE5708`, ECX=EBX, Find D3BC -> `008F2260`, compare byte `[EAX+0C]` at D3C1; D3C5 jumps to D5E4 if nonzero. Native has no null or type guard at this read. |
| `0046CF40` | Gate call `0046D426`, result test D42B and branch D42D | Supporting order receipt only: `0046C550` is called after the native Hidden read. No new gate/provider contract. |

`008F2260`'s existing no-dot lookup branch returns the map-found record or null.
Its map/native string/allocator integration is separate; this packet adds no
lookup implementation. Native untyped/no-null-check loading does not establish
Source's non-B raw or missing-false policies as native fault/type behavior.
Other Hidden references in `0046BF70` are not adopted.

The already reviewed type-3 producer and byte copy contract is in
[SCENE_TYPED_BOOL_PAYLOAD_CC11.md](SCENE_TYPED_BOOL_PAYLOAD_CC11.md): B arm
`008F5D4F..008F5DDF` (end 008F5DE0), fresh producer `008F3940`, constructor
`008EF1F0`, clone arm `008F4F8D..008F4FC1` (end 008F4FC2), and assignment arm
`008F0736..008F0745` (end 008F0746). `boolean_value`/`has_boolean` are owning Source
data, including false; they are not native parser-success, +2Ch or replay flags.

## Installed inputs and focused Source proof

The manifested Win32 probe includes actual production SceneContents, its private
called reader and compiled SceneReaderBinding. It separately compiles the actual
parser and changed consumer under MSVC `/W4 /WX /fp:strict`. Existing fixture
library dispatch invokes actual parser, PropertyLibrary add_group, ordered merge
and copy services. Whole one-file loader/VFS and whole SceneReader are not run.

The fixed 17 installed inputs are 15 library files defining 22 groups, USN1 and
JM06. Lexical inventory is 56 recognized Hidden B declarations: Common line
1759 `B FALSE`, LandConvoy line 4 `B false`, USN1 34 (15 true/19 false) and
JM06 20 true. Actual parsed authored bags confirm those counts independently.

| Actual resolved mission bags | Owning true | Owning false | Inherited false | Missing Hidden |
| --- | ---: | ---: | ---: | ---: |
| USN1: 147 entity bags | 15 | 132 | 113 | 0 |
| JM06: 96 entity bags | 20 | 76 | 76 | 0 |

USN1's true bags are 15 PlaneSquadronGen rows, including ScoutDauntless at
10735. JM06's true bags are four TBoatGen, nine DestroyerGen, six SubmarineGen
and one MotherShipGen; these are parsed class spellings, not invented native
class/enum providers. Common/LandConvoy default false, authored overwrite,
independent owning copies and all 243 resolved bags survive cleared or opposite
Hidden diagnostics. Recognized true/TRUE/false/FALSE and case-insensitive B,
legacy B without payload, first-literal raw behavior, Source-authored non-B,
missing and bare B fallback also pass. No tracked tests are added.

Compile/link/probe exit codes are all 0. The executable is PE Win32 (`014C`)
with an embedded asInvoker manifest. Its 177 actual project/Lua includes plus
separately compiled parser produce 178 active Source inputs; all 17 installed
and 77 frozen support pre/post hashes match. Current fully rebuilt post-B/I
`ac58ee9db` supports (3 libraries/74 game objects) were copied together with
original-pre/copy/original-post equality. No prior binaries or future live build
inputs are linked. The standalone consumer object is compilation evidence and
is not linked twice. Exact hashes/recipes are in the ignored artifact manifest
and the tracked JSON report. Main full build/CTest remain the primary's step.

Admission is explicit recognized single true/false B with a closed semicolon and
ordinary ASCII case spelling. Malformed/multitoken/nonliteral/implicit/empty
existing-context/declaration-conflicting B, enum identity/Lua, locale/NUL/native
tokenizer/allocator/reentry/faults are outside the native claim. Source raw
compatibility fixtures do not admit those as native behavior. Group capture
retains its prior admitted domain; the separately recorded `008F54F0` tail
decode did not extend stored function-body metadata beyond `008F5658`. No new
flow repair, metadata or Ghidra mutation occurs here. Whole SceneReader, actual
holdback side effects, emitter, original ABI and original-game/runtime proof
remain unclaimed.

## Primary integration

Main `7b835976e1268a2b7b3b7d9061cc9bc3f2ab0fbc` passed the full Win32 build and all three existing CTests. Root independently reviewed the code, native fragment and entire fixture, compiled three fresh translation units against current main, verified179current Source/Lua/fixture inputs (177actual includes),17installed files and77current compiled inputs before/after linking, and checked the PE32 asInvoker manifest. The actual parser/production group/private called reader checks passed all243resolved bags; the public caller is compiled, whole SceneReader remains unrun. All22native fragment bytes matched disk/live and both direct call rows passed. Source after-gate versus native before-gate order and all ABI/game limits above remain. No tracked tests were added.
