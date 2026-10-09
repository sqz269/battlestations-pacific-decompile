# Production model descriptor subset Source

Packet `cc12_production_model_descriptor_subset_Source` adds the existing model
and model-base initializers to `GameNativeTypeStorage` on baseline
`1172ce41243d31a7f27313797f39b8600f1cea90`. The production VFS owner now retains
their canonical Source guard/descriptor cells and supplies the same counter and
common bootstrap used by its other represented type families. This is a built
Source composition candidate for Root integration and review; it does not
reconstruct the complete Native CRT schedule or establish absolute type IDs.

## Storage and initialization

The two additions use the actual existing interfaces:

| Native role | Retained Source backing | Borrowed interface |
| --- | --- | --- |
| Guard `01090030`, descriptor `01090034` | One byte and `volatile ModelTypeDescriptor` | `ModelTypeBootstrapStorage` |
| Guard `01090031`, descriptor `01090044` | One byte and four volatile DWORDs | `ModelBaseTypeStorage` |

The four descriptor DWORDs are own type, node type, root type and Native name
address. The model-base interface binds those four actual lvalues separately;
the patch does not invent a model-base descriptor class. The existing storage
owner is noncopyable and nonmovable. Its constructor initializes the new Source
backing to zero and binds the retained views. It invokes no type initializer,
counter getter or registry operation.

`model_types()` and `model_base_types()` return references to those retained
views. They neither copy IDs nor initialize the backing. They may expose the
actual zero or partially initialized cells before successful initialization.
Borrowers must finish before the storage owner is destroyed and must not start
type initialization after its shared singleton domain has been drained. The new
accessors have genuine Core definitions; this packet adds no model-object caller
that uses them.

`initialize_resource_types()` retains its existing common-bootstrap alias check
and every previous initializer call. Between the camera and animation calls it
constructs `ModelTypeBootstrap` and `ModelBaseTypeBootstrap` views using the
existing counter, existing common bootstrap and the retained backing, then calls
`initialize_static_00cd7e60()` and
`initialize_static_type_descriptor_00cd7eb0()` respectively. Their existing
guard, dependency and counter behavior is unchanged.

The relative order comes from the previously accepted pointer-cell evidence:

| CRT slot | Pointer cell | Target | Represented call |
| --- | --- | --- | --- |
| 905 | `00CE3558` | `00CD7D80` | Camera |
| 909 | `00CE3568` | `00CD7E60` | Model |
| 910 | `00CE356C` | `00CD7EB0` | Model-base |
| 928 | `00CE35B4` | `00CD82F0` | Animation resource |

The accepted [CRT placement audit](CC12_MODEL_DESCRIPTOR_CRT_PLACEMENT_READINESS.md)
and [storage readiness audit](CC12_PRODUCTION_MODEL_DESCRIPTOR_BOOTSTRAP_READINESS.md)
remain historical evidence. No new Native body, pointer cell, caller, handler or
Ghidra query was opened for this Source packet. Other CRT entries, intervening
pool work and lazy dependencies remain outside this represented subset.

## Production owner and failure lifetime

`GameNativeVfsApplication::Impl` already owns `NativeVfsOwnerServices`,
`GameNativeTypeStorage` and a `LightTypeBootstrap` bound to that storage and the
same `owner_services.types()` counter. Its `initialize_core()` passes those
actual objects to the type initialization sequence after setting its attempt
flag. The generic `GameNativeTypeStorage` API still requires its caller to supply
the correct counter; the existing common-storage identity check does not itself
prove counter identity. The inspected production caller supplies it correctly.

The new calls retain the existing sticky failure behavior. They do not reset
guards, retry an interrupted descriptor, roll back IDs, substitute fixed numbers
or catch a counter/dependency failure. The existing initializers write their
guard/name before child initialization and consume the counter before publishing
their own ID. A failure may therefore leave canonical partial state. The type
sequence precedes the later A0 runtime-construction try region; that existing
region does not clean up these descriptors.

The VFS host records interrupted native operations and retains the application
when required. The existing startup failure path exits before ordinary draining
when VFS initialization was interrupted. On the normal path, shared singleton
draining occurs while the retained VFS owner still exists, before renderer/VFS
destruction. This packet changes none of those lifetime rules. Source lifetime
and exception composition are not proof of Native frame-handler equivalence.

## Worker build and emitted evidence

The normal `scripts/build.ps1` Release/Win32 build completed successfully from
`2026-10-09T22:17:01.391112Z` to `22:27:31.088428Z`. Both enabled existing checks
passed: `reconstructed_math` and `tool_tests`. This fresh worker tree lacks
`local/seed_reference.hpp`, so `native_math_differential` was not enabled. No
seed export, new test, probe, Source game startup or gameplay run was performed.
Root owns integration and the subsequent seeded three-check build.

The selected input manifest contains 521 files: the accepted Source509 inputs,
the quoted project-include closure of eight explicitly selected translation
units, and the build script. All 521 were unchanged across this build. This is a
selected dependency manifest, not a claim to hash every repository build input.
The Source509 baseline uses its retained, frozen artifacts; it is not a claim
about a later mutable Root build directory.

The retained COFF receipt includes eight whole Core objects and two whole
application objects, their physical symbol-table indices, auxiliary records,
sections, relocations and indexed reachable graphs. It records 31 selected
roots and 27 unique positive Core definition rows. All 22 selected existing
provider roots preserve their code and resolved relocations against the frozen
Source509 baseline. Whole provider objects are not byte-identical: raw object
differences and anonymous-namespace symbol changes are retained without symbol
normalization in the receipt.

The changed owner object contains two changed functions (constructor and
resource initializer), two added accessor definitions and 18 unchanged
functions. Its 741-byte/147-instruction constructor contains no calls or
relocations. Old member offsets are retained; the new model guard/descriptor
occupy Source offsets `1D4`/`1D8`, the base guard/words `1E8`/`1EC..1F8`, and the
two views `1FC`/`204`. These are this build's Source host offsets, not Native
fixed-address or class-layout claims.

The 312-byte/91-instruction resource initializer has real calls to camera,
model, model-base and animation in that order at instruction offsets
`98`, `B6`, `E6` and `10A`. The same counter/common objects reach both added
providers. The two accessors each compile to a seven-byte `LEA`/`RET` leaf.
The final Source image maps 28 of the 31 selected roots. The two unused
accessors and the inlined model counter-consumption helper have no separate
application-map entry; their Core definitions are retained.

The full indexed executable graph contains 67 sections totaling 5,634 bytes and
1,756 decoded instructions, including Source exception funclets and the full
634-byte VFS initialization section with its catch and cold throw arms. Of these,
64 sections map to the Source executable and match outside their actual indexed
four-byte relocation operands; named targets and addends were checked. Three
unused sections are unmapped. Non-executable COFF/EH data is retained in the
complete receipt, without claiming a complete linked-data comparison.

## Boundary

The represented production subset now consumes both descriptor IDs through the
existing real providers and retains the actual Source backing for future users.
This closes the selected omission identified by the prior readiness packet.
It adds no Native body credit, original ABI admission, faithful whole-CRT or
startup credit, absolute numbering proof, model/vehicle construction, renderer
virtual-call mapping, resource/atlas composition or gameplay proof. Root must
integrate and review the candidate before treating it as admitted Source work.

The accompanying [JSON report](../reports/cc12_production_model_descriptor_subset_source.json)
pins the sources, accepted evidence, build artifacts and complete local emitted
receipts. Local evidence is under
`build/cc12_production_model_descriptor_subset_Source/` in the worker tree.
