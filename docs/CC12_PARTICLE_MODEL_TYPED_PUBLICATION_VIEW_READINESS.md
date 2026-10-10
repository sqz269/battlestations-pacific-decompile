# Particle-model typed publication view readiness

The two model access views can borrow the retained typed `00F8C274` cell directly.
The smallest later Source change is two headers: forward-declare
`bsp::NativeParticleModelManagerStorage` in the construction header and change
both `manager_00f8c274` members to
`NativeParticleModelManagerStorage* const volatile&`. Their existing C++ uses
already convert the pointer value to the manager helpers' `void*` parameters.
No implementation, caller, publication, allocation, registration, or startup
change is required for this view adaptation. This packet grants no C++ GO.

The baseline is accepted main `a9db5b460f22fd717f90fb420046938ca892797e`,
Source3352. No C++ or build input changed, no build or test ran, and no target
executable, Native body, Ghidra mutation, SDK or operating-system probe ran.
The mandatory `brief` status check and two local `bsp.py lookup` queries did not
export or query a new Native body. Native names and addresses below are inherited
from retained Source and accepted construction/lifetime documents.

## Actual Source uses and missing initializer

The complete tracked `src/`, `include/`, and `tests/` corpus has 17 occurrences
of the two access type names, confined to their two headers and two `.cpp`
files. They are definitions, references in function signatures/private helpers,
or the lifetime companion's borrowed member. There is no Access object owner,
aggregate initializer, alias, or factory instantiation in this corpus.
The three `NativeParticleModelConstructionCallees` occurrences are its class,
destructor declaration and access member; there is no concrete implementation.
All 28 `NativeParticleModelReference` occurrences are in its declaration and
implementation, with no external construction site. These are current corpus
findings, not claims about ignored historical fixtures or external clients.

| View | Current read and required order | Receiver use |
| --- | --- | --- |
| Construction | `src/native_particle_model_construction.cpp:327`, after random draw, mask assignment and child traversal | Read the current cell once for `register_native_particle_model_00af0950(..., &node)` |
| Lifetime | `src/native_particle_model_lifetime.cpp:106`, after current variant/mesh releases and before the live-count decrement | Preserve the local captured pointer for `unregister_native_particle_model_00af0ae0(manager, &base.storage.node)` |

The existing manager helpers accept a pointer value, cast that pointee back to
the actual `NativeParticleModelManagerStorage`, and update its `models_04` weak
array. They do not retain the model, acquire its ownership or check for null.
The two view fields never assign or republish `00F8C274`.

`RegisteredModelEffectCallees::construct_model_00af74a0` is an abstract seam.
`registered_model_effect.cpp:56` calls that interface after raw-slot allocation
and returns the captured slot through the interface on construction failure.
It does not instantiate either Access view or call the concrete model constructor.
The available derived lifetime companion also needs an actual owner and access
object before it can enter the current destruction path.

Other `00F8C274` interfaces exist: the semantic `FoliageGroupManager` view in
`system_time_constants.hpp` and the raw pointer-to-cell field in
`native_system_constant_gather.hpp`. This proposal does not adapt or establish
either of them. Their distinct contracts remain separate work.

## Smallest change and lifetime contract

Only these later edits are necessary:

1. `include/bsp/native_particle_model_construction.hpp`: add the forward
   declaration inside `namespace bsp`, then replace the member's `void*`
   pointee type with `NativeParticleModelManagerStorage*`.
2. `include/bsp/native_particle_model_lifetime.hpp`: make the same member change.
   This header already includes the construction header and receives the forward
   declaration from it. No additional heavyweight include is needed.

The exact proposed member in both views is:

```cpp
NativeParticleModelManagerStorage* const volatile& manager_00f8c274;
```

`GameNativeStringProcess::particle_manager_00f8c274()` returns the existing
`NativeParticleModelManagerStorage* volatile&`. Adding `const` to that pointer
object when binding the reference borrows the same cell. It does not read the
cell, create a temporary pointer object, make the pointed-to manager const,
or extend the manager's lifetime. No reference reinterpretation, type-erased
cell, mirror, cached context value, new publication or replacement manager is
admitted. `volatile` preserves the intended cell observations; it does not add
atomicity or a concurrent lifetime guarantee.

The current constructor argument performs an ordinary object-pointer-to-`void*`
conversion after reading the typed cell. The destructor's existing per-call
`void* const manager` capture performs the same conversion at its required late
read point. Both `.cpp` files can remain unchanged. A later style-only typed
local or explicit pointee cast is unnecessary for the minimum change. Moving
the destructor read past the count decrement, retaining it in the Access object,
reloading it at the unregister call, or adding a null skip changes the contract.

A future genuine model owner can bind through the already engaged singleton
host context's `manager_00f8c274` reference, whose identity is checked against
the process cell by the dormant binder. No such initializer is added here.
The binder and permanent claim authorize one host's deletion domain; they do
not produce a manager or a model. The process cell intentionally survives host
destruction. Its pointee, model pool, companion, other access dependencies and
the Access object itself still need valid, stable lifetimes. The companion
stores `NativeParticleModelLifetimeAccess&` through terminal release and failure
retirement; a temporary Access object is insufficient.

All model unregisters and reentrant releases must complete while the observed
manager and its weak array remain valid. Manager destruction clears/frees weak
array storage without destroying its models. A retained cell cannot make a live
model safe after that drain. A replacement host cannot adopt the old graph;
the permanent process claim remains in force. Preserve current-cell observation
after preceding callbacks and preserve the captured receiver during unregister.

## Before evidence and required after evidence

The packet retains the complete Source3352 construction and lifetime COFF
objects and decodes both complete relevant function sections. These are Source
objects, not Original instruction windows. The retained Root report supplies
their provenance and reported Core-member hashes; this packet does not reopen
the full Core archive or reproduce the previous build.

| Existing Source3352 function | Complete section | Existing cell read | Call relocation |
| --- | --- | --- | --- |
| `construct_native_particle_model_00af74a0` | 1,868 bytes, 563 instructions | `+060B: mov ecx,[eax+34h]`, `+060E: mov ecx,[ecx]` | call `+0611`, AF0950 helper relocation at `+0612` |
| `destroy_native_particle_model_00af6c50` | 600 bytes, 192 instructions | `+015D: mov eax,[edx+4]`, `+0160: mov esi,[eax]`; live-count store follows at `+016B` | call `+0173`, AF0AE0 helper relocation at `+0174` |

The constructor still passes the actual node; the destructor passes the captured
manager and actual model node. The parser handles `.bss` sections with declared
size and zero raw-data pointer as having no file payload. Both objects' complete
sections, symbols and relocation records are retained.

The observed Access member offsets are `34h` and `04h`. From the current Win32
field declarations, expected total sizes are `48h` and `14h`, alignment four;
the constructor's following live-count reference is expected at `38h` and the
lifetime one at `08h`. Those total sizes are Source layout expectations, not
measured compiler type records: these saved objects have no `.debug$T` record.
There is no post-change layout or code proof in a read-only packet.

A later authorized two-header patch must start from the latest accepted startup
Source epoch and fresh leases; Source3352 is this review's historical baseline.
It must capture exact before files and the
actual compiler read/write/command/items cohort, then run the normal Win32
build and existing checks. The lexical include graph identifies seven candidate
translation units: array resize, cone definition, model construction, model
lifetime, model update, record children and record update. Actual compiler logs
must determine the final cohort. Retain whole before/after objects and affected
Core members; compare complete nondebug function bytes/relocations and validate
the publication loads, call arguments and count-store order above. If claiming
exact size/alignment equality, retain compiler layout evidence as well. Rebuild
all consumers of the changed class definitions; identical names do not waive
that C++ definition requirement. No new executable fixture is needed for the
view-only proposal, and this packet does not authorize one.

## Activation remains unestablished

Model execution still needs the genuine initialized `F8D2D0` pool (`2E0h` slot,
`2DCh` payload and its separate trailing pool ID), prepared `NativeModelOwner`,
stable node/scene binding, current `D5DA50` profile and `F8D308` type words,
canonical live-count and random state, valid variants and emitter definitions,
resources `F8C280`, shadow `F8D39C`, renderer `F8D394` with its actual virtual5C,
mesh/material/section owners and pools, layout/parameter services, and reached
emitter/variant/mesh zero-reference terminals. Bind and failure-retirement hooks
must use the same actual-owner domain. The caller must preserve the raw-slot
return and partial-construction unwind contracts. Startup activation of the
manager alone establishes none of those model contracts. The ordinary model's
`188h` pool and destructor remain inappropriate for a particle model.

The 186-file recursive read-set lists every external include occurrence.
External prerequisites comprise MSVC C++ library/runtime headers and Windows/
DirectX headers (`Windows.h`, `windows.h`, `d3d9.h`), with their exact spellings
in the report. No SDK headers, compiler binaries or transformed third-party
headers were frozen, and no full compiler-preimage claim is made. Portable
analysis needs Python and Capstone as recorded in `before_code.json`.

## Retention and validation

`reports/cc12_particle_model_typed_publication_view_readiness.json` records the
contract, source anchors, complete query results, artifact hashes and exclusions.
The single packet directory is
`local/cc12_particle_model_typed_publication_view_readiness/`.
It contains complete Git/current archives for 4,214 tracked Source/test files,
11 additional whole inputs, 30 exact excerpts, 306 project include edges,
the whole Source3352 snapshot/report and two saved COFF objects. The Source3352
overlap is 3,348 files; all remaining inputs are explicitly tied to the baseline
Git/current bytes. Six newline-different epoch files are separately retained.
Referenced older documents/reports are whole, without recursively importing
their historical proof trees or revalidating their runtime claims.

`python portable_replay.py evidence.zip` reads only its ZIP after dependency
loading and rejects other file opens, writes, subprocesses, network and dynamic
library loads. It passed both in place and from a physically copied directory,
checking all Source pins, queries, excerpts, includes and 755 decoded instructions.
The separate `verify_git.py` check passed 4,225 baseline blobs and current files.
These establish replayable readiness evidence, not C++ implementation, activation,
Original ABI, live execution, rendering or gameplay parity.
