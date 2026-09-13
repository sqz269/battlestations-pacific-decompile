# Native unit scene initialization

`initialize_native_unit_scene_00955420` reconstructs the complete normal caller
at `00955420..0095559E` (383 bytes). Native ECX is the unit, ESI preserves that
receiver, EBX is zero from `0095542F`, and the epilogue restores ESI/EBX and
uses bare `RET`. This is a new C++ interface with required external providers,
not a native ABI replacement or a complete available construction pipeline.

| Routine | Coverage | Source |
| --- | --- | --- |
| `00955420` | Complete normal caller, including both property arms and final store | `src/native_unit_scene_initialization.cpp` |

The view borrows the existing `NativeUnitObserverAlias`, `NativeUnitSceneHandleView`
and `PoseRefreshView`, plus the same unit's actual fields. It supplies no layout
cast, second unit/model/node, default matrix, owner registry or reference count.
Current `GameUnitsHost` still cannot supply this scene binding. Its boolean
`has_scene_node` and unimplemented placement path do not produce a native handle.

The chain at `00955429..48` reads current unit+360, then its +160, then that
object's +0C, and reuses V's exact +4A4 publication. `00713604` stores the selected
part set at the part instance's +160; `0087BEA4` publishes the part instance at
unit+360. The existing `unit_parts.hpp` identifies this owner. The +0C accessor
must read that actual part-set cell; this packet defines no new part-set layout.
The initial null +360 arm still dereferences +0C from zero: there is no valid
null fallback. The C8 test occurs **before** that final load and publication.

After optional canonical pose refresh, node virtual+30 receives the live three
floats at unit+FC, which are world_CC[12..14]. It does not receive a full matrix.
Current handle, world global/root, descriptor and manager loads occur at their
native sites. The primary+10 result is captured before reading descriptor+70.
Property lookup results are not cached across native repeat lookups. After the
first `MinLevel` success the caller clones the **entire current bag**, reloading
the holder, then gates the next arm again. After the ocean operation it reloads
the holder/bag for shadow lookup without repeating the kind gate.

| Call site(s) | Provider / complete required contract | Native cleanup |
| --- | --- | --- |
| `00955424` | `0087BCC0`: complete health/parts initialization, part-array and selected part-instance creation, numbering/property tail, including its failure effects. Existing hit-path arithmetic covers only a projection and is not substituted. | bare RET |
| `00955452` | `00414DB0`: existing canonical recursive pose refresh over the actual view | bare RET |
| `00955469` | Current node virtual+30 with live unit+FC. Checked table D62DE8 selects B6DAE0: ordered x87 translation stores, then current virtual+34 tail dispatch with actual node world matrix. Other dynamic targets remain required. | selected setter owns stack pointer cleanup; B6DAE0 tail passes it to +34 |
| `0095547D` | `00B6D890`: existing canonical root propagation, actual root/head/child and scene-provider operations; default adapter resolves the same owners | RET4 |
| `00955489` | Current unit primary+10. Checked CFC3D0/D0BF80 tables select 42E950, a 16-byte name-pointer getter with E1767D fallback. Required dynamic result is an opaque native word. | bare RET in checked target |
| `00955498` | `009292B0`: whole Lua self ClassID/Class operation, native strings, VehicleClass lookup, B66790 call and temporary release. Existing partial mission Lua attachment is not substituted. | RET8: class index, primary result |
| `009554A3` | `00740FE0`: allocate/initialize 14h counted registration object, append through current manager pointer array/count/capacity including growth, return actual result | bare RET |
| `009554C8`, `00955507`, `0095551E`, `0095554A`, `00955561` | `008F2260`: full bag/path lookup, including nested sub-bag traversal and actual record/null return; no replacement map or library body | RET4: key |
| `009554DA` | `008F41F0`: allocate and construct 114h bag, clone every record, insert into the native map; return complete bag | bare RET |
| `00955530`, `00955537` | `00728340` / `00475E60`: capture first child before modifying actual node+48 by AND-not / OR mask2, recurse through actual children and current sibling+3C. No exact existing source providers were found; both are required. | RET4: mask |
| `00955591` | `00747560`: complete recursive generated-model material traversal; register cShadowFactor using the actual pointer and x87-store its current value into diffuse alpha; recurse current child/sibling links | ECX=node, EDX=&unit+52C, bare RET |

All listed callees were read before these contracts. Their complete listing,
decompilation, boundary and capped incoming references are retained. Library
identities and their bodies remain externally owned. This caller adds no guard
that fabricates native unwind behavior. Provider exceptions are not suppressed;
native callee FH3/SEH equivalence remains unproved.

`MOVSS` preserves the property float payload. Other property kinds take signed
`CVTSI2SS`, with the current MXCSR rounding mode. Pseudocode's `(int)fVar3` store
is wrong: the actual store at `00955584` is `MOVSS [EDX],XMM0`. Source uses a bit
load for float payloads and an explicit SSE conversion for integer payloads.
The compiled object confirms MOVD/CVTSI2SS, captures current +360 before the
MOVSS destination store, then resolves its +160/+0C for `00747560`. That final
node can differ from cached unit+4A4. The provider sees the actual +52C pointer
while byte+62C still has its old value; only afterwards is +62C cleared.

The older `scene_entity_create.hpp` calls holder+04 `refs`. The correction in
`CRUISE_SPEED_SETTING.md` is confirmed by native producers and copy dispatch:
`00922E20` stores kind1, `00774DC0` stores kind2, and `00922DE0` copies the kind,
clones +08 only for kind1, and handles kind3 separately. The new borrowed field
is therefore `kind_04`; no refcount behavior is inferred. The shared stale header
is an integrator follow-up. Property-record +04 is a separate type discriminator,
using existing `ScenePropertyType` and +0C payload evidence.

Four focused full-caller byte/source pairs passed: null holder with dirty pose;
kind2 skip with clean pose; repeated lookups with kind1, true ocean and integer
16777217 rounded upward to float16777218; false ocean with low byte zero and
signaling-NaN float payload `7FA12345` preserved. Explicit provider boundaries
rebind current handles, globals, descriptors, holders and models. Both sides
invoke the actual canonical pose refresh and node-root unlink. Other callbacks
are contract probes, not fabricated implementations of unresolved providers.
Traces, untouched bytes, actual factor pointer, final-store timing, and current
owner arguments agree. No native allocation/Lua/material/exception body is
executed by these bridges, and no scene or gameplay validation is claimed.

The full 383-byte body, all required direct callee bodies, producer/copy evidence,
checked table slots and literal keys match live Ghidra and installed PE bytes.
Twenty-one relocation operands are verified. Read-only Ghidra has a function at
00955420; supporting 0042E950 has `no_ghidra_function`, so its complete 16 bytes
were decoded from disk and compared with live bytes without defining a function.
Exact fixture source, inputs, relocs, headers, tools, objects, libraries, manifest,
logs and hashes remain under `local/`. Strict MSVC Win32 build, both existing
CTests, eight seed comparisons and mechanical call checks are recorded in the report.
