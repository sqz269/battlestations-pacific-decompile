# Explicit retained hierarchy services in the application

Baseline: `44569dc13832cee9eec3963c884f85d9aa626707`. This implements the accepted [binding readiness](CC12_MESH_HIERARCHY_APPLICATION_BINDING_READINESS.md) and its [primary review](CC12_MESH_HIERARCHY_APPLICATION_BINDING_READINESS_PRIMARY_REVIEW.md). The four C++ files add an explicit borrow of genuine hierarchy services through the existing VFS/resource owners. API names are new Source names, not recovered symbols. No startup caller is added.

`GameVfsHost::borrow_hierarchy_services()` requires completed `core_ready_` and passes through the existing interrupted-operation gate. It uses only that host's `resources_`, `native_->raw_strings()` and actual raw VFS bindings. The resource-owner overload is private to the host, checks its own operability and exact raw-string-context identity, and obtains mapped data from its original construction. An arbitrary public caller cannot supply another VFS/string/data tuple.

The resource `Impl` lazily retains a noncopyable, address-stable binding containing the actual VFS/data references, `NativeResourceRootDispatch`, stream read context, hierarchy context and service view. The read context borrows the exact raw strings, node-capable dispatcher and canonical `GameNativeShaderProcess::stream_empty_0109db64()` storage. The hierarchy context borrows the mapped bound cells. The pool is the canonical process `hierarchy_pool_0109022c()`; its existing accessor requires completed explicit startup. The returned const service view contains references to those same retained contexts and pool.

Every borrow asks `data_at` for these exact spans and checks the retained values before exposure:

| Original data | Span | Accepted retained bytes / words |
| --- | --- | --- |
| Node profile `00D68BB4` | 8 bytes | `e030bd00c09fbe00`: `00BD30E0`, `00BE9FC0` |
| Negative bound `00CE4ADC` | 4 bytes | `f90215d0`: `D01502F9` |
| Positive bound `00CE4970` | 4 bytes | `f9021550`: `501502F9` |

These are comparisons against the actual mapped cells, not copied defaults. The contexts retain references to those cells. The profile capture establishes only its two-word prefix, not a whole vtable extent. Evidence comes from the already accepted BS/BV reports and the readiness capture records; this packet performs no new Native data query.

Repeated borrowing retains the same adapter, read context, parser context and returned view. It verifies VFS/data owner identity, profile pointer, raw strings, read dispatcher, canonical empty storage, both bound references, each service-view reference and canonical pool identity. A changed domain or retargeted empty-storage pointer throws; it is not repaired or rebound. The retained `NativeResourceRootDispatch` is the existing real node terminal: `BD30E0` with `D68BB4` checks current slot4 `BE9FC0` and calls the reconstructed deleting body; stream operations forward to the actual VFS dispatcher. Unsupported current profiles remain explicit failures.

This call allocates only host metadata. It invokes no Native resource/parser getter, consumes no type ID, starts no pool, performs no I/O, creates no root/node/record/resource, parses nothing and registers no publication or cleanup. Borrowing a context does not admit later parser invocation, failure handling or retirement. Callers must keep the application, VFS, mapped data, strings and canonical pools alive through consumers and shared drain. Binding destruction does not access those borrowed owners or perform Native cleanup.

## Compile and object evidence

Before isolated compilation, the packet copied both actual Root Release projects and command logs, ten already-built full Root objects, and the four baseline owner Source files. The recorded options retain x86 `/W4 /WX /O2 /Ob2 /Oy- /EHsc /MD /fp:strict /std:c++17 /TP`; Core and App keep their distinct include sets and `/external:W0` versus `/external:W4`. Only worktree/input/output locations and evidence diagnostics are changed. The compiler reports MSVC `19.51.36244.0` from tools directory `14.51.36231`; exact tool bytes and environment are frozen. `/Bv`, `/sourceDependencies` and class-layout diagnostics do not link or execute a probe.

Twelve ordinary Source objects compiled successfully: the two changed translation units, eight unchanged providers and both baseline owner translation units. Every final object has complete dumpbin headers, symbols, relocations, directives and disassembly, with command/exit receipts. Resource layout diagnostics were repeated to select the nested `Impl`; the initial outer/binding layout outputs are also retained. All 576 compiler-reported Source/header files are copied, including MSVC/SDK/external dependencies.

The 22 complete objects (12 isolated plus 10 captured Root) have 9,625 sections, 30,577 physical symbol records including AUX, 16,009 relocations and 5,732 function bodies. Every body has a complete linear decode. The inventory preserves raw section bytes, code, noncode, EH, physical symbol/AUX records, relocation indices/addends and dumpbin output. The selected receipt follows 21 actual provider definitions; unresolved external boundaries and multiple COMDAT candidates remain explicit. This is an object inspection, not a link-resolution or execution certificate.

Compiler layout evidence shows:

| Owner | Before | After | Existing identity/offset result |
| --- | --- | --- | --- |
| Resource application outer | 4 bytes | 4 bytes | Same sole `impl_` member |
| Resource application `Impl` | 220 bytes | 228 bytes | All 24 existing fields retain offsets; mapped-data reference appended at 220, binding pointer at 224 |
| VFS host | 124 bytes | 124 bytes | All fields and virtual layout unchanged |
| New retained hierarchy binding | absent | 60 bytes | VFS/data/profile at 0/4/8; node dispatcher 12; reads 24; hierarchy 36; service view 48 |

All 1,080 pre-existing VFS-host function bodies preserve exact bytes and relocation edges after an explicitly recorded bijection of compiler-generated lambda names. The only added host function is the borrow. The resource owner preserves 81 of 93 baseline functions under that same accounting; its 12 changed bodies are the constructors/destructors, allocation/deletion wrappers and associated EH paths required by the appended metadata. The receipt names every changed body. Both isolated baseline owners match their captured Root bodies completely under the generated-name mapping. That mapping is anchored only by equal complete function bytes and equal-offset/type/addend relocation pairs; no code, addend, EH or data bytes are normalized, and raw lexical comparisons are also retained.

The new host borrow is 104 bytes / 38 instructions; the private resource borrow is 610 bytes / 174 instructions. Their compiled code contains the host readiness/interruption gates, resource failure/string gates, exact mapped-span requests and value comparisons, canonical process-provider calls, retained 60-byte allocation, real node-dispatch constructor and repeat-domain checks. The 44-byte resource destructor only deletes the optional 60-byte binding and the 228-byte `Impl`; it does not dereference the borrowed host or invoke Native teardown.

Captured Root objects are historical provenance, not a blanket current-provider claim. Six unchanged providers match their full isolated code and relocation edges. Captured resource-pool Source had 51 functions versus the current 78; its process getter and constructor differ. Captured VFS-application Source has 16 same-size code differences with equal named relocation edges. These differences are retained explicitly for Root's fresh normal-build composition review; the selected current provider definitions come from the freshly compiled complete Source objects.

## Evidence and remaining scope

[Machine report](../reports/cc12_mesh_hierarchy_application_binding_source.json) indexes the ignored immutable evidence under `local/cc12_mesh_hierarchy_application_binding_source/`. The final zip includes a member hash manifest, compiler/environment/tool pins, complete dependencies, current/baseline Source, captured Root configuration/objects, every object inventory, raw and generated-name comparisons, selected provider/guard receipts and retained context reports. Verification reads every member and checks exact size/SHA-256.

No CMake, ledger, Ghidra database, Native export/window, startup call, fixture, test, link, probe or game run was changed or performed. Root owns the normal full build and current application/provider composition review. The Source636 and fallback owner API remain unchanged context. This packet establishes the compiled explicit service binding only; it does not establish hierarchy parse/retirement policy, cache/classifier/mesh composition, pool-exit safety for consumers, original ABI compatibility, runtime behavior or gameplay parity.
