# Remaining resource descriptor startup: Source readiness

No complete initializer for compact `0109042C`, camera-resource `01090288` or
skin-model `01090344` is identifiable in the surveyed current Source. None has
backing in the actual `GameNativeTypeStorage` owner or a call in its resource-type
startup. There is **no ready C++ startup packet** for these families. Existing
live-cell consumers do not supply the missing initializer contract.

This review uses published baseline `ea35ae1c0`, including the accepted
[group descriptor and Armour review](CC12_GROUP_DESCRIPTOR_ARMOUR_SOURCE_PRIMARY_REVIEW.md).
It changes only this document and its JSON report. No original image, Native
body/data/export/handler or Ghidra state was opened. No C++, build registration,
ledger, test, build or runtime probe was changed or executed.

## What the current Source establishes

| Family | Existing Source consumer | Missing startup evidence |
| --- | --- | --- |
| Compact `0109042C` | `NativeResourcePostprocessContext` borrows the current own-ID word. `B79BC0` supplies its address to the existing `B87CE0` typed range service. The range header identifies the corresponding original `B78750/B922C0` pair. | Actual writer entrypoint, guard, complete descriptor layout/name, parent initializer chain, counter/store sequence and startup position. |
| Camera resource `01090288` | `B8AA30` consumes exactly three current words through the existing fallback predicate. `NativeCameraGroupResourceCalls` borrows the same three-cell view. Postprocessing supplies the current own-ID word to `B87CE0`; its range header names `B78990/B8A1C0`. | Actual writer entrypoint, guard, full extent including any name word, parent/counter protocol and startup position. Three predicate inputs do not establish the complete descriptor layout. |
| Skin model `01090344` | The complete `B8F920` Source leaf returns the current borrowed word. Compact-animator compatibility and pose processing pass it to the actual node's current slot-C predicate. Point-light population also borrows and tests this current cell. | Actual writer entrypoint, guard, full descriptor/name layout, parent initializer chain, counter/store sequence and startup position. The getter does not initialize anything. |

The camera predicate performs ordered equality checks against its three current
words, with no guard check, nonzero filter or initialization. `B8F920` likewise
does not synthesize a token. Descriptor getters and predicates must retain this
behavior if startup backing is later added. A zeroed stand-in, copied ID or
successful fallback would not supply the missing startup.

Current Source searches found 14 lines containing the three requested addresses,
all belonging to consumers or their declarations/comments. Searches of candidate
initializer/type-service/guard names found no matching provider. The full context
reference search found no production instance of `NativeResourcePostprocessContext`
or `NativeCameraGroupResourceCalls`, nor a concrete derived
`NativeResourcePostprocessCalls` service. The separate `game*` Source search found
no references to these graph/postprocess/camera adapter contexts. This is a bounded
Source finding; it does not identify a missing original writer or prove that an
unnamed generic routine elsewhere could never be relevant.

## Existing families that cannot be substituted

`CameraTypeBootstrap` owns the camera **object** guard `0108FF9C` and descriptor
`0108FFA0`, with complete `CD7D80/B719E0` providers. Those own/node/root/name cells
are not camera-resource `01090288`.

`NativeResourceExtraTypeIds` implements only animation `01090268` and bone
`01090278`, using guards `01090264/01090265` and `CD82F0/CD8340`. Its private
four-word initializer establishes those two existing families only. Copying that
pattern for a third family would invent its guard, name, parents and ordering.

The retained skined-mesh **resource** descriptor is `01090454`, initialized by
`CD86F0`, with five words `[own, mesh, scene, root, name]`. It cannot supply
skin-model `01090344`. The separately excluded skined-mesh family `01090370` is
also outside this packet; the Source searches do not establish a startup provider
for that address. No relationship between these distinct cells is inferred.

The newly retained group **object** descriptor `0109032C` and its `B8F590`
initializer remain separate from GroupParams resource tokens `010902E4` and
group pool `010902F4`. The camera parser pointer `010902A0` and skined-animation
parser pointer `0109043C` are singleton publications, not type descriptors.

## Actual retained owner and caller contract

`GameVfsHost` retains `GameNativeVfsApplication` and its separate resource
application. Its first-time phase invokes `native_->initialize_core()`.
The VFS implementation retains `owner_services`, nonmovable `type_storage` and
`common_types`. It constructs the latter from `owner_services.types()` and
`type_storage.light_types()`, then passes those same references into
`type_storage.initialize_resource_types()`.

The public `native_types()` and `native_owners()` accessors already expose those
actual retained services. A later complete family initializer can use this
existing startup route and add only its genuine backing/view to the same owner.
No second counter, manager, descriptor owner or Source callback route is needed
merely to initialize another established family. The current type-storage
identity check verifies the common backing before any family writes; using the
same counter object remains the caller's existing precondition.

The shared `TypeIdCounterLifetime` borrows actual publication `0109DB7C` and its
existing lifetime domain. Its getter returns the existing owner or performs its
real allocation/publication/registration protocol. Deletion clears the counter
publication without resetting descriptor guards or IDs. New borrowers must use
that same counter and finish before retained owners are destroyed; initialization
services must not be used after the shared counter is drained.

The VFS attempt flag is committed before resource-type initialization and prevents
retry after failure. The already admitted providers retain their own early guard
and partial-field semantics. For the three missing providers, their exact guard,
store and exception behavior is still unknown and must be recovered before
implementation; an existing family's behavior is not evidence for theirs.

The existing relative order of represented CRT families remains as documented.
Group `B8F590` is an explicitly qualified Source insertion after animation/bone
and before mesh. There is no established original CRT entry or position for the
three missing families in this review. A future insertion may change subsequent
numeric IDs according to its actual counter consumption; no absolute IDs or
complete Native CRT execution are promised.

Descriptor startup would also leave consumer composition incomplete.
`NativeResourcePostprocessContext` still requires actual animation registry,
current-table/virtual-operation services and CRT bindings, and the graph context
requires its real factories, node/group companions, parenting and bone context.
`NativeCameraGroupResourceCalls` requires actual read services and complete
forwarded parser/type operations. `GameNativeResourceApplication` currently
retains the resource manager and parser singleton/deletion contexts; those
publications do not instantiate these graph/postprocess providers.

## Smallest next gate

The recommended next investigation is **only the camera-resource writer for
`01090288`**. Its current three-word predicate gives a narrow consumer anchor.

1. Identify the actual original writer and its complete ordinary body, including
   any constructor/static wrapper. The entrypoint is currently **unknown**;
   neither address adjacency nor the neighboring `CD8340` initializer establishes it.
2. Establish the real guard, complete target extent, name word if present, parent
   initializer calls, exact shared counter usage, store order and partial failure
   behavior. Check required support bodies and original ABI before reconstruction.
3. Establish its genuine caller or CRT-table placement if claiming Native order.
   Otherwise qualify any later, explicitly chosen startup position as Source
   composition. Do not infer an original CRT slot from the resource's name.
4. Only after that provider is complete and reviewed, consider one camera-resource
   backing/view plus its genuine call in the retained VFS type startup. Preserve
   live reads and exact failure semantics. Keep compact and skin-model held.

Compact and skin-model have the same missing-writer/body gate, anchored by their
own current cells and consumers above. No exact original writer address, guard,
descriptor size or Native name is fabricated for any of the three. No such
Native discovery or implementation was performed by this readiness packet.

## Evidence and limits

The receipt freezes 25 selected current Source files, six historical context
documents/reports, seven complete search outputs and a 4,174-row tracked Source
tree inventory with Git blob IDs. Every selected Source/context file matches
the baseline after normalizing checkout line endings. The inventory describes
the searched Source revision; it is not compiler dependency, emitted-code or
runtime evidence. Negative search claims remain limited to the stated patterns
and surveyed Source contracts.

Source595's normal build and three CTests, and its actual VFS-phase run ending
before device/Present with FMOD `61/78/37`, belong to the immutable primary review.
They were not rerun here and do not observe these missing descriptor cells or
execute their absent initializers. This packet adds zero reconstructed functions,
startup calls, descriptor backings, graph consumers or new execution evidence.

Primary review verified and retained all 73 report pin occurrences. The 25
selected Source files and six historical context files match their named Git
baseline; selected paths remain unchanged in the review revision. This accepts
the bounded readiness finding and the next camera-resource writer-discovery
packet under the continuing reconstruction goal. It admits no new descriptor
backing, initializer, consumer composition or Native behavior.
