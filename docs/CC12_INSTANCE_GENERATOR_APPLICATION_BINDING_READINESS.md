# Instance-generator application binding readiness

Packet: `cc12_instance_generator_application_binding_readiness`

Frozen Source baseline: `0acd45594a365d0c9d452f154d1ab323c53a430f`, the
requested `0fc0be6b1` main checkpoint merged with ordinary serial-owner commit
`c194788ca156fc491f71171777c3d091777bccd6`.

## Finding and smallest next boundary

The genuine generator factory and all six context inputs already exist. The
missing application component is a stable composition of
`NativeInstanceGeneratorContext` and `NativeInstanceGeneratorOwners` over the
renderer application's existing domains. A metadata-only borrowed binding is
ready for a bounded Source proposal. Full generator activation remains held.

The smallest proposed implementation would touch the existing
`game_native_renderer_application.hpp/.cpp` and its own document/report. It
would forward-declare the real owner type, append one optional private binding
after the existing `Impl` members, and expose one ready-only borrowed owner
reference. Within that binding the context must precede its owner, so the owner
dies first. Every referenced renderer/string/layout/geometry/data provider must
remain alive until the binding and all actual native obligations are retired.
The new public borrow name and private binding type are proposals, not current
APIs and not implemented in this packet.

Borrowing would obtain only existing references and table/name views, construct
C++ metadata once, and revalidate every original identity on later permitted
borrows. The existing conservative ready/retention guard can be reused; it
already refuses a borrow while section-layout companions are live. A cached
borrowed reference must remain stable. Borrowing must not call `B451D0`,
`B85610`, a constructor, a parser, or native cleanup, and must not consume or
reset the serial. This narrow proposal does not solve the entered-operation
retirement contract described below.

Borrowing and any later entered work must preserve the application's existing
serialization contract with startup, device recreation, frames and native
drain. The serial remains an unlocked volatile DWORD; no new atomic or lock is
inferred from the metadata binding.

## Actual current interfaces and providers

The current owner API contains its constructor/destructor, `context()`,
`register_generator()` and `register_binding()`. The real entry points are
`attach_native_material_instance_generator_00b451d0()` and
`finalize_native_mesh_section_generator_00b85610()`, each with a persistent
`NativeInstanceGeneratorAcquired` supplied by the caller. Their declarations
and complete ordinary object bodies are retained. There is no application
generator binding, borrowed generator accessor, or generator-owner quiescence
query in the frozen baseline.

| Context input | Existing provider | Required identity |
| --- | --- | --- |
| `graphics` | `Impl::game_grids.graphics`, publicly reachable through `game_grid_context().graphics` | Same declaration cache, geometry registry, logical streams, physical pool, synchronization and current renderer cell |
| `layouts` | `Impl::section_layouts`, exposed by `section_layout_services()` | Same geometry, vertex context, declaration loading, hardware tree/pool and `D62AF4` profile |
| Binding serial | `game_native_renderer_scalar_process().instance_generator_binding_serial_0108fd30()` | Same ordinary Source process cell, borrowed by volatile reference |
| Four profiles | Existing `Impl::profiles.data.data_at(address,8)` | Two readable numeric words at each required profile; same retained read-only provider |
| Generic declaration name | `data_at(0x00D61C08,18)` | Existing readable span for the constructor's 17-character name plus terminator |
| Building declaration name | `data_at(0x00D61C28,42)` | Existing readable span for the constructor's 41-character name plus terminator |

The required profile pairs are `D62190` / terminal `B55CB0`, `D61BFC` /
`B450A0`, `D61C1C` / `B451A0`, and binding `D619F8` / `B417C0`. The current
owner checks first word `BD30E0`, then dispatches the current second word to
the corresponding real Source deletion body. Numeric profile entries remain
data selectors; they are not callable replacement vtables.

`GameNativeReadOnlyData::data_at()` already checks the requested span and
whether its band was admitted. The production Source span set already includes
the D6 band. This establishes the existing provider route, not that any new
span was opened or a particular live mapping succeeded in this audit. The
names and profiles must come from that same provider; no generated literal,
default table or surrogate mapping is needed or allowed.

The serial provider is solely the newly reconstructed ordinary Source accessor.
Numeric cell `0108FD30` metadata provenance remains held. This audit neither
reopens original cell bytes nor promotes the Source member to an original
fixed-address cell, Native ABI replacement or runtime-value claim.

## Domain and schedule evidence

`GameGridGraph` constructs its graphics view from the application's declaration
cache and its stream graph. Those streams use `texture_loading.geometry`,
`graph.vertex`, the matching index lifetime and physical mapping. The geometry
registration already uses `NativeRenderActualOwnerRegistry`'s actual
`bind_callback`, `unbind_callback` and `find_callback`; no new callback is
necessary. `require_gui_text_native_renderer_domain()` compares the real
renderer, owners, strings, synchronization, physical providers and declaration
type-table identities.

The current application constructs `section_layouts` from that same geometry,
vertex context, declaration loading and hardware context. The concrete layout
service checks the canonical registry, renderer publication, declaration pool,
profile/type table and strings. Its descriptor path requires the genuine
`NativeLogicalVertexReference`, and its layout path calls the real `B2F710`
factory, reusing the same canonical companion for a cache hit.

The existing generator base constructor directly calls the actual `B317E0`
declaration loader, then builds a two-entry layout key in **section stream
first, instance declaration second** order. It must not be replaced with the
convenient GUI declaration wrapper: that wrapper throws on a null declaration,
whereas this genuine constructor preserves its own null-result schedule. No
`GeneratedInstanceGeometry` or other projection object supplies these owners.

The complete already-retained `B451D0` body and the current Source agree on:

1. Empty or unrecognized effect descriptor selection leaves the existing
   section binding untouched. Building comparison comes first; generic
   comparison re-reads effect `+C4`, then its string header `+28`.
2. Allocate `1Ch`, invoke the chosen complete generic/building constructor, and
   stop binding work if it returns null. Constructor unwind frees the raw
   allocation only; it does not add completed-resource rollback.
3. Allocate the `10h` binding. A nonnull result receives its profile, count,
   empty generator field and the old serial at `+8`, followed by the unlocked
   modulo-32-bit increment. A null binding skips those writes but still reaches
   the genuine setter with null; no successful null-allocation alternative is
   invented.
4. `B41780` assigns/retains the generator; consume the generator's creator
   reference; `B417E0` publishes the binding at section `+5C`; then consume the
   binding's creator reference.

The Source adds canonical companion registration after generator construction
and after binding serial consumption. Registration adds no native retain and
no rollback. Its C++ allocations and transactional bind can fail, so those
host metadata boundaries remain visible in the acquired frame. The serial is
not restored after a later failure. Fourteen ordered instruction/Source matches,
the complete branch bodies and all relevant ordinary provider bodies are
retained; no new original body or callee bytes were queried.

`B85610` checks section `+20` material and material `+7C` effect before calling
the full attachment. The existing full subset reader publishes/retains the
section in the mesh **before** calling `B85610`, passes the actual mesh `+60`
value unchanged, and releases the section creator afterward. A binding borrow
must not activate or rearrange that parser schedule.

## Lifetime gaps that remain before activation

`NativeInstanceGeneratorOwners` owns a private list of companions and terminates
if that list survives its destructor. `register_owner()` can keep an unbound
companion when canonical registration throws. No public query observes this
list. A narrowly scoped const observation over the whole list would be a new
API requiring its own authorized Source change; it cannot be called as though
it already existed.

The canonical registry's real `size()` and `empty()` APIs are insufficient:
they include unrelated owner families and omit generator companions retained
before bind. Layout `has_live_companions()` is also not a generator-owner
query. The application's current `requires_process_retention()` checks its
phase, shaders, shadow, pending layout acquisition and live layout companions;
it does not observe generator-owner entries or generator acquired frames.

Even a future generator-list query would not cover every interruption. A raw
creator, temporary name, declaration or completed generator can survive a
failure before a companion is inserted. Conversely, a frame reaching
`complete` can leave a section-owned binding and generator alive. A later
activation contract therefore must retain both the complete acquired frame and
its provider graph, account for pre-registration failures, and retire actual
payloads/companions before shared renderer drain. It must not infer retirement
from null bookkeeping pointers, a completed phase, or an empty global registry.
No callback, destructor rollback, replay, no-op cleanup or default payload is
introduced to hide these obligations.

The metadata-only binding can be reviewed independently of these future
entered-operation controls. Connecting full mesh/subset/material/compiler/
sampler/parser execution remains outside that binding and outside this audit.

## Frozen evidence and verification boundary

The [report](../reports/cc12_instance_generator_application_binding_readiness.json)
pins the review package under
`local/cc12_instance_generator_application_binding_readiness/`.
It retains 33 Source translation-unit roots, their 490-file repository include
closure, 501 full Source/context files, 928 resolved include edges and no
unresolved quoted repository include. Every full file is compared with its
Git blob. All 4,209 Source census hashes from the preceding ordinary build
match the current frozen baseline. The old Source746 checkpoint is preserved
with all 750 retained pins.

Each of the 33 roots has its complete existing ordinary COFF object: 5,426
fully decoded functions, 28,299 physical primary/AUX symbol records and 15,466
relocations, with no undecoded function tails. All 95 explicit linker inputs,
the full core archive, normal build configuration and map are retained.
Nineteen actual API bodies are matched to the Source contract. The only
original assembly/pseudocode copied is the same four previously frozen
`B451D0` / `B85610` files. This is complete selected Source/object evidence,
not a claim that every Native dynamic edge is resolved or executable.

`review_evidence.py` verifies every manifest/ZIP payload, full Git content,
all 33 COFF inventories, selected archive members and the complete contract
audit. It does not compile, run a test/executable, access Ghidra, read the
original image or mutate Source. This packet changes only this document and
its report; it adds no C++, CMake entries, tests, callbacks or wiring. Existing
build artifacts are evidence from the preceding Source packet, not new
runtime, startup, activation, ABI or gameplay validation.
