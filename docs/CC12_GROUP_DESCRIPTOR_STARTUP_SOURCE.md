# Retained production group descriptor startup

`GameNativeTypeStorage` now owns the existing group guard `010902E1` and exact
`NativeGroupTypeDescriptor` for `0109032C`. The real VFS resource-type startup
calls the complete existing `NativeGroupTypes::initialize_00b8f590()` provider
with those same cells, the existing VFS counter and its common node/root
bootstrap. The new `group_types()` accessor borrows the live cells without
initializing them or copying IDs.

The baseline is `8b0c2966f`, including the immutable
[Source585 pool review](CC12_NODE_GROUP_POOL_SOURCE_PRIMARY_REVIEW.md).
Only the type-storage header and implementation change. The existing VFS caller,
group provider, shared type providers, CMake registration and ledgers are unchanged.
There was no new Native body/data/handler read, original-image opening or Ghidra
operation. This is a descriptor startup prerequisite, not a group object or graph
consumer admission.

## Storage, provider and real caller

The appended backing is one volatile guard byte and the existing four-DWORD
descriptor: own ID, inherited node ID, inherited root ID and Native name address.
`NativeGroupTypeStorage` retains references to that backing. It is distinct from
the retained group pool `010902F4`; neither a pool slot nor a type-ID snapshot is
substituted for the descriptor. Existing fields and views remain before the new
members. Copy and move remain disabled.

The constructor creates cold Source backing and binds the view only. In the
current Win32 object, the new guard and descriptor are at host offsets `218h`
and `21Ch`, and the view's two pointers are at `22Ch` and `230h`. These are compiled
Source offsets, not original executable addresses or an ABI mapping. The emitted
accessor is a pointer-return leaf; it makes no initializer or counter call.

The complete unchanged provider in `src/native_group_owner.cpp` performs:

1. Read the actual group guard; a nonzero guard returns without changing the target.
2. Set the guard first, then write the existing Native name address `00D634F0`.
3. Initialize the same shared node descriptor through `00B6F110`, then copy its
   live node/root IDs into the actual group target.
4. Obtain the same `TypeIdCounterLifetime` owner through `006FAC20`, advance its
   current counter and write the group's own ID last.

The current Source provider disassembly retains that order: guard store at `+16h`,
name store at `+19h`, node initializer call at `+27h`, counter getter at `+39h`,
counter update at `+44h`, and group own-ID store at `+47h`. These are offsets in
the emitted Source function only; no Native instruction comparison was performed.
The existing type getter/predicate continue to read current own/node/root words
without a guard test, zero filter or automatic lazy initialization.

`GameVfsHost` retains `GameNativeVfsApplication` and its first-time initialization
calls `native_->initialize_core()`. The actual descriptor caller remains
`GameNativeVfsApplication::Impl::initialize_core()`.
Its constructor creates `common_types(owner_services.types(), type_storage.light_types())`;
startup passes those same `owner_services.types()` and `common_types` into
`type_storage.initialize_resource_types()`. No VFS edit or new initializer route
was needed. The existing storage-identity check still runs before any family
initialization. Generic callers must likewise supply the same counter instance;
that is the existing contract, not a newly invented counter or runtime check.

The resource sequence preserves every earlier family call and inserts the group
provider after animation/bone initialization and before the three mesh calls.
The emitted caller passes its original counter/common references and the new
borrowed guard/descriptor pointers to the existing group service. This position
is explicit Source composition and is qualified in the header and implementation.
No original group CRT-table wrapper or placement is fabricated. A clear group
guard adds one counter consumption and can shift not-yet-initialized families'
numeric IDs; this packet promises neither fixed IDs nor full Native CRT order.

## Partial state and lifetime

The existing guard-first ordering is preserved. A failure after the guard store
leaves the guard and any fields already written intact. There is no reset,
rollback, catch-and-retry, repair or fallback initializer. The new accessor
exposes that same partial state rather than masking it behind a success flag.
The actual VFS caller marks its overall attempt before entering resource-type
startup and refuses a second attempt. Source-only direct callers retain the
provider's existing guard behavior.

The VFS application retains this nonmovable type-storage owner, its common
bootstrap and owner services. Borrowers must finish before that owner is destroyed
and must not use initialization services after the shared counter is drained.
The counter's existing deletion protocol clears its publication without resetting
descriptor guards or IDs. The new descriptor adds no independent cleanup or
lifetime manager. A future `NativeGroupTypes` service for actual objects must
borrow these same cells and the same owner services; that consumer composition is
not supplied here.

## Compiler and Source verification

The normal MSVC Win32 Release build passed both configured worker CTests:
`reconstructed_math` and `tool_tests`. No local seed-reference header is present,
so no Native differential test was configured. Root's earlier three-test result
belongs to Source585. No new test or runtime probe was added.

Complete MSVC `CL.read` and `CL.command` records were captured for all three
Core/App translation units tracking the changed header (type storage, VFS
application and renderer application), plus seven existing type-provider units.
All **686 recorded input files** were pinned and copied: 412 project files and
274 external files, with zero missing inputs. Six complete read/command/write
tracking logs, the compiler configuration and driver identity are retained.
This is the complete recorded dependency set for those ten units, including SDK
headers; it is not the entire application, a full preprocessing/environment
closure or a fresh compilation claim for unchanged incremental provider objects.

Ten current COFF objects have complete physical symbol/auxiliary, section and
relocation inventories. Twenty-three required positive code definitions were
verified. Five required code-reference groups connect the VFS caller to resource
startup, startup to the real group constructor/initializer, and the group provider
to the real node initializer/counter getter. Complete object dumps and the three
relevant Source disassemblies are pinned. No malformed relocation index was found.
These checks do not establish all transitive runtime behavior or Native ABI.

The game executable was built but not run. Source585's recorded plain-node/group
atexit-zero results and subsequent FMOD failure remain unchanged historical
evidence. Production group/model objects, graph dispatch, terminal object
lifetimes and compact `0109042C`, camera-resource `01090288` and skin-model
`01090344` families remain separate incomplete composition. No successful startup,
Present, gameplay, original/source differential or complete Native CRT admission
is claimed.
