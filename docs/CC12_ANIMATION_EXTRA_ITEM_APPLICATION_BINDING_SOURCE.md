# Animation extra-item application services

Baseline: `be101024a1d0b2b967094e76911a82c59fb4fc70`, the worker merge of current `main` preserving accepted readiness `b1273e406280f60a89891b0fe0947037b79b0d35`. This implements the finite service binding approved in [hierarchy parse/retirement readiness](CC12_MESH_HIERARCHY_PARSE_RETIREMENT_BINDING_READINESS.md). The preceding hierarchy binding is already integrated on main at `c86b0cd223dc92576ce52b519aa008f6be7ad998`.

`GameVfsHost::borrow_animation_extra_item_services()` now returns stable, noncopyable, host-owned reader/reference metadata. It requires completed core startup, passes the existing VFS interrupted-operation gate, and delegates privately to its own resource application. The application reuses `borrow_hierarchy_services()` and its existing resource-operability, raw-string, VFS, node, mapped-data and canonical-pool checks. It does not retarget that read context.

## Genuine providers and mapped identity

The retained binding contains the existing concrete `NativeAnimationChannelBodyReader`, one finite `NativeResourceExtraItemReaderCalls` implementation, `NativeAnimationDeleteCalls`, `NativeResourceExtraItemLifetimeContext` and `NativeResourceExtraItemReferences`. They all borrow the exact hierarchy read context, raw strings and node-capable dispatcher. The reference adapter forwards unhandled stream/node/reference domains to that same existing dispatcher.

| Qualified actual mapping | Bytes requested | Current words checked |
| --- | ---: | --- |
| `D62ED4` group | 8 | slot0 `BD30E0`, slot4 `B78D00` |
| `D632B0` channel | 8 | slot0 `BD30E0`, slot4 `B8AD60` |
| `D6328C` AnimationChannels | 36 | slot0 `BD30E0`, slot4 `B8A760`, slot20 `B8AD80` |
| `D632B8` Bone | 36 | slot0 `BD30E0`, slot4 `B8AF10`, slot20 `B8AF30` |

These are actual pointers from the application's original mapped-data owner. The existing 44-byte item captures include adjacent words; no complete 44-byte vtable extent is inferred. All 88 qualified bytes fall within already retained evidence. Every borrow rechecks the mapped spans/current words. Repeated borrows also check the retained hierarchy/mapped owner identities, all four table pointers, lifetime string/deletion/group identities and service-view references. No previous view is moved or rebound.

The reader terminal dispatches the supplied captured target directly:

- `B8AD80` calls `read_native_animation_channels_00b8ad80` with the exact retained reads and genuine channel-body reader; that reader reaches the existing complete `B8AAD0` body.
- `B8AF30` calls `read_native_bone_resource_item_00b8af30` with the same reads.
- Every other captured target throws `std::logic_error`. There is no fabricated remaining reader or successful fallback.

The native-semantic bodies remain unchanged. Bone copies/returns its temporary name before the seven staged reads. AnimationChannels keeps its captured group across children, including a null group. Channel publication still occurs only after a recognized `AnimationKey`; the binding adds no name/index/group prevalidation, final publication, AddRef or partial-allocation rollback. The existing item-reference terminal remains downstream of the caller's actual decrement. Group/channel deletion still uses current slot4 with flags1 and no invented nested decrement.

## Ownership and effects

One `unique_ptr<AnimationExtraItemBinding>` is appended at offset 228 in the resource `Impl`. Existing publication/context offsets remain unchanged. Its metadata is destroyed before the existing hierarchy metadata, and destruction never dereferences a borrowed singleton host or runs native cleanup. The binding contains no native item, group, channel, structured-handle or resource ownership.

The public borrow performs host metadata allocation only when first needed. It invokes no native reader/deletion, resource/parser getter, type-counter operation, pool startup, I/O, resource/root creation, cache operation or drain registration. It installs no startup call site. It can prepare the already approved hierarchy metadata when that has not yet been borrowed.

Actual parser/reader use remains separately admitted. A future caller must own the real item/handle/resource graph, retain interrupted frames and latch failures through the owning VFS policy, prohibit replay, and retire successful payloads before singleton drain and process pool cleanup. Neither this metadata destructor nor the manager's borrowed cache supplies that ownership. `B7F100/B7EB90`, `B7F430`, `B80720` and whole resource-container retirement remain outside this packet. Renderer mesh services must still use the actual renderer-owned geometry domain.

## Current Source evidence

Before compilation, the worker copied the actual integrator's two projects, both complete compiler command logs and seven ordinary objects from `J:/PROG/battlestations-pacific-decompile-cc12_resume_integrator`, at captured head `f1066a4807e76a346395b60702caa7db0d38ee4d`. The referenced normal build ran successfully from `2026-10-10T05:55:00.488196Z` to `05:55:16.970769Z`; it is baseline context, not acceptance of this candidate.

Nine isolated MSVC Win32 compilations passed using those current per-target Release commands: the two changed owners, two baseline owner copies, and five unchanged genuine reader/lifetime providers. `/W4 /WX /O2 /Ob2 /Oy- /EHsc /MD /fp:strict /std:c++17` and target-specific include/external settings were retained; only source/include/output destinations and version/dependency/layout diagnostics differ. Full compiler commands/output, all 45 dumpbin runs, 457 compiler-reported whole Source/header files and compiler-tool snapshots are retained. No link, build script, new test, probe or runtime operation was run by this worker.

| Comparison | Result |
| --- | --- |
| Resource `Impl` layout | 228 → 232 bytes; all 26 existing field offsets preserved; one appended metadata pointer |
| `GameVfsHost` layout | 124 bytes; all fields and vtable positions unchanged |
| Current integrator objects → fresh baseline owners | Resource 130/130 and VFS 1,081/1,081 complete functions have identical bytes and relocation edges under explicitly anchored compiler-generated name mapping |
| Baseline → changed VFS owner | All 1,081 previous functions unchanged; one 104-byte borrow added |
| Baseline → changed resource owner | 119/130 previous functions unchanged; 11 constructor/destructor/`unique_ptr`/unwind functions change for the appended metadata; 47 new functions include the binding and emitted C++ support |
| Existing `Impl` constructor | Same ordered relocation-bound call sequence; instruction comparison adds the offset-228 null store, advances its metadata EH state and adjusts branch displacements |
| Five unchanged providers | All complete functions and relocation edges match captured integrator objects: 20 channel-body, 19 AnimationChannels, 35 group/channel lifetime, 7 Bone, 38 item-lifetime functions |

The 16 complete object inventories preserve every physical primary/AUX symbol, raw code/noncode section, relocation and EH payload: 20,037 symbol records, 9,852 relocations and 3,919 completely decoded functions. Twenty-nine selected genuine functions and eleven complete binding/owner bodies have independent full receipts. The finite reader is a 100-byte internal-linkage COFF definition; the private application borrow is 630 bytes, and the compiled qualification helper contains all actual table addresses and expected current-slot values.

The new binding destructor is exactly a three-byte `RET 0`. The resource owner destructor contains only three sized C++ metadata deletes, for 100-byte extra-item metadata, 60-byte hierarchy metadata and the 232-byte implementation. Direct provider edges resolve by physical symbol index or unique positive definitions in the enumerated current Source objects. Compiler-generated anonymous/lambda spellings are mapped only through recorded equal-code relocation anchors; raw names, indices, addends and complete bytes remain available. Separate noncode EH payloads are retained, not silently normalized into the function-byte comparison.

[Machine report](../reports/cc12_animation_extra_item_application_binding_source.json) records all files, compiler/dependency/COFF receipts, profile evidence and limitations. The immutable ignored bundle includes full contents and a member manifest. Root retains the normal build/link, primary review and integration gate. This packet adds no Native reconstruction, original callable ABI/FH3/SEH, fixture, startup, runtime or gameplay credit.
