# Type-6 record storage and empty-child clone Source

The already registered `src/native_scene_property_bag_storage.cpp` now contains the whole raw type-6 constructor `008EF780` and an ordinary successful-empty-child fragment of the native 71-byte arm `[008F50ED,008F5134)`. A new disjoint header exposes the two raw interfaces. The final normal Win32 build passes all three existing checks. Proposed registration is one whole Native function and one fragment; this worker adds no ledger/function credit.

## Actual construction and ownership boundary

`construct_native_scene_property_record_type6_storage_008ef780` is naked MSVC Win32 fastcall, with actual record in ECX, an explicit unused EDX formal, and actual child at entry ESP+4. Its emitted 58 bytes/16 instructions equal the entire installed body with zero relocations. It returns the same record in EAX with RET4, child in EDX and ECX=0; EBX/ESI/EDI/EBP and DF are unchanged. Final XOR flags are CF=OF=SF=0, ZF=PF=1, AF undefined; it executes no x87/SSE/MXCSR operation.

The caller supplies a real fresh 38h allocation from the canonical current malloc/free domain and the distinct mutable 114h output of the accepted ordinary empty-bag producer, with child owner+110 still zero. The constructor writes phase CE89D4, tag6, actual child+C, zero fields +18/1C/20/24/30, then actual record into child+110 BEFORE record ordinal34, byte2C=1 and final record+8. Exactly 41 record bytes change; all 15 bytes at10..17,28..2B,2D..2F are preserved. No whole-record memset, null-only guard, synthetic profile or semantic class is introduced. The caller retains the allocation/lifetime responsibility for both linked roots.

`clone_empty_child_native_scene_property_record_type6_008f50ed_fragment` borrows an actual whole-constructor-produced record with tag+4=6, actual child+C and initialized ordinal+34. That child must stay in the qualified raw61-produced empty domain: count+8=0, all 64 heads zero, DF0 and stable distinct valid source/output storage. Its source owner backlink is borrowed, not copied. The implementation allocates the real 38h record FIRST, loads actual source+C, calls the accepted ordinary empty-bag producer, passes its exact returned child to whole 58-byte constructor, rereads source+34, copies that raw DWORD through returned record+34 and returns the actual root.

The existing bag producer still performs genuine 114h allocation, whole raw61 initialization, then the actual zero-count read. Its 126-byte/41-instruction body and raw61 are unchanged. It is now marked noinline in this owned TU so the new fragment retains a direct call to that actual provider. There are no earlier same-TU callers and other TU declarations are unchanged. Source noinline concerns this ordinary C++ provider; it does not alter Native whole 154-byte 8F41F0 or claim populated cloning.

The ordinary fragment is 146 bytes/48 instructions. Its complete production transport is:

| COFF instruction offset | Actual operation |
| --- | --- |
|35h,3Ch,43h|Set real request `{object,38h,38h}`|
|4Ah|Call canonical `singleton_lifetime_allocate`|
|4Fh,52h|Push actual source+C; retain allocated root in ESI|
|5Eh|Call actual accepted `clone_empty_native_scene_property_bag_008f41f0_fragment`|
|63h|Remove the two ordinary CDECL argument DWORDs|
|66h,68h,6Ah|Unused EDX=0, ECX=actual root, PUSH actual returned child EAX|
|6Bh|Call whole 58-byte fastcall constructor, which consumes its child with RET4|
|70h,73h|Read actual source ordinal after construction and store through returned EAX+34|
|89h,91h|Actual compiler cookie check; ordinary C++ RET|

Only fresh unattached record storage is freed if child production throws. The compiler's retained 26-byte catch extent loads that exact saved allocation, calls actual canonical free, and rethrows; its 39-byte handler reaches the real C++ frame handler. These are normal Source compiler/runtime dependencies, not recovered Native CA4B6D/state2/null-allocation cleanup. The native parent arm's register/FS/stack ABI is not claimed for the ordinary function. Neither new entry was executed.

## Complete production and unchanged providers

All 15 pre-existing bag-TU function extents remain byte-for-byte and ordered-relocation-for-ordered-relocation identical under their literal symbols, including 61-byte raw storage and 126-byte empty clone. All 138 old extents across the three selected complete provider objects remain unchanged. Other two TUs have explicit bijective mappings only for MSVC worktree-dependent unnamed-namespace/lambda salts; no code difference or unresolved target is hidden. The final bag object has 19 complete symbol extents: the old 15 plus two public functions and two compiler-generated EH extents.

Complete final bag, canonical allocator and physical-pool process objects are physically retained, and each equals exactly one complete member of the actual copied whole `bsp_core.lib`. The whole 146-byte ordinary function, all six ordered relocations, whole 58-byte constructor, complete catch/handler and all old extents are preserved. The full current application and I386 CRT DLLs are copied. Current application imports malloc/free/_callnewh and real C++ frame/throw services; complete allocator 90/free 6 remain actual current providers. No new function's application COMDAT linkage or runtime API-set resolution is inferred from archive qualification.

## Build and evidence timing

No CMake edit was needed: the existing bag TU was already registered. Initial pre-edit production objects, whole archive, all old functions, actual compiler tools and accepted readiness receipt were captured. The first clean normal build took 830.08 seconds and passed its two then-enabled checks. It emitted an inlined accepted child producer in a 196-byte main extent; those Source/input/object/archive artifacts are retained. The inlined Source producer was not a claim to have implemented Native whole8F41F0.

After the approved noinline adjustment, the standard verified seed procedure enabled the existing native differential test locally, with all 8 seed ranges matching the installed image. It wrote only ignored local evidence/header, not shared exports. The second normal build took 19.34 seconds and passed all 3 checks, but strict consumed-input review exposed three compiler localization resources absent from the earlier freeze. That build and its failed completeness check are retained.

All 285 actual consumed provider inputs were then physically frozen before the final compile, including those resources. Only the owned cpp timestamp was touched to force recompilation; its contents did not change. The final normal build took 17.35 seconds; reconstructed_math, native_math_differential and tool_tests all passed in 8.54 seconds. Every final consumed input and final Source/header matches its precompiler copy. No new tests, probes or selected type-6/child-producer entry executions occurred; only the repository's existing checks ran.

The initial missing optional Lib.read log assumption and literal comparison of worktree-salted symbols are also preserved as capture-method failures with corrected tools. No build failed. Full-function extents split at every distinct function-symbol start; EH extents and padding are retained distinctly. The complete final Source/COFF/PE/capture-tool inputs and manifest live under `local/cc12_property_type6_storage_source/`; the versioned JSON report records the seal, exact dependencies, timing and preserved alternatives.

## Remaining scope

The two interfaces create actual allocator-domain roots and raw backlinks. Phase CE89D4, child D16504 and inner D162C4 remain literal Native DATA identities, not Source vtables or permission to call Native code. The six-body cycle `008F3F30 ->004E6730 ->008F0DE0 ->008F0640(type6) ->008F59E0 ->008F5410 ->008F3F30` remains separately cohesive and unimplemented as a closed whole Source lifetime family. Its future concrete code-domain binding, full scalar flags/root free, whole 158-byte release schedule, owning-map 254 and bag double-clear must be qualified together. No tiny-scalar re-deferral is proposed.

Whole populated clone, recursive factory, wrapper owner/ordinal publication, old-value replacement, Original class/SEH/failure equivalence and gameplay remain unadmitted. This packet changes exactly the existing source file plus the new header, this document and its report. No Ghidra, ledger, packet metadata or shared build-file mutation occurred.

## Independent primary acceptance

The integrator rehashed all 2,097 retained worker artifacts, froze 294 current physical inputs before its normal build, and matched all 285 actually consumed compiler inputs. All three existing checks passed. Three whole current objects have exactly one matching member each in the current Core archive; the complete 142-function graph matches the worker graph, including ordered relocations and an explicit bijection for compiler path names. All 138 old functions, including all 15 old bag-storage functions, remain unchanged. The complete 58-byte constructor still equals the installed Original; the 146-byte ordinary fragment preserves real allocation, child production, constructor call and fresh ordinal-read order. The accepted incoming parent-header callback build registration is the one intentional worker/current recipe difference.

Receipt: `local/cc12_property_type6_storage_primary_review/receipt.json`, SHA-256 `d234fe672c72e9baec7c175ae82b7edfe144a39175265b36fad2ff72f894bbd6`. The failed review-driver iteration is retained; no Source changed during primary review. The registry adds one complete Original function at `008EF780` and one bounded fragment at `008F50ED`. This does not establish new-entry execution, an Original class/parent-clone ABI or EH domain, populated-child support, startup, or gameplay.

## Current Type6 constructor entry qualification

A fresh single-process follow-up now qualifies the bounded raw constructor entry at `008EF780`. It preserves the earlier static Source acceptance and the ordinary `008F50ED` fragment evidence. The ordinary record-clone fragment itself was not executed. The current implementation and public header are unchanged; this follow-up changes only this document and its report. Independent primary runtime admission remains pending.

One fresh Win32 process, PID **97712**, exited zero after **six** constructor calls: two raw Source calls, two fresh Native calls, and two ordinary C++ calls to the Source constructor. Each lane uses root poisons A6/59; the four raw calls physically capture the child stack DWORD, GPRs, defined arithmetic flags, DF, ES, and RET4 frame. The entire freshly loaded Native body is 58 bytes/16 instructions, with no patched operands or unresolved call. The full Source constructor and the actual 61-byte raw bag initializer equal the installed Original and were rechecked against live Ghidra after execution.

All six children are actual distinct 114h allocations returned by the current ordinary empty-bag producer, borrowing one real raw61-initialized canonical seed. All six records are actual canonical 38h allocations. The fixture records every complete root56/child276 before and after, proves all 13 live allocation extents disjoint and outside the actual thread stack, and verifies the complete seed unchanged. The constructor writes 41 root bytes, preserves the remaining 15, and changes only child+110 to the actual record. EAX/EDX/ECX, nonvolatile GPRs, ES, DF and the defined XOR flags satisfy the exact raw contract; AF is excluded. There is no dynamic x87/SSE/MXCSR state claim. The backlink-before-tail ordering is whole-body assembly evidence, not intermediate dynamic store tracing.

The current canonical allocator/free path creates and retires all storage. Each raw empty child is freed before its own record, and the seed is freed last. The fixture completes six child frees, six record frees and one seed free. These are completed-call/loop observations, not a separate event trace of every free. No phase dispatch, recursive destructor, owning-map interior free or class reconstruction is used.

The final fixture directly compiles the three complete current translation units (storage, canonical allocation, probe) and links a **fresh isolated registered Core build**: 2,132 new Core objects plus new Lua/zlib archives. All 4,624 repository Source/header/CMake inputs are frozen and bookended. The full-Code gate and a separate reader both pass **37 whole bodies / 5,941 bytes / 1,653 instructions**, 132 retained sections, 791 relocation records, all 10 weak entries and 357 CFG edges. All 37 actual loaded bodies are compared before and after the process. Duplicate std::exception COMDAT selection is resolved by exact global map name/VA and complete fresh own-section relocation/byte equality. The sole unexpanded local failure edge is the excluded GS failure path.

The Core archive resolves sibling references without stubs, partial-TU extraction or old object reuse. Other retained archive code, CRT startup and static initialization are outside the qualified Type6 helper/caller closure and receive no execution or equivalence claim. The build's duplicate `spawn_request_id_matches` warning is retained. Earlier failed direct links, the intentionally cancelled serial Core attempt, the initial missing-import-library link and tooling preflight failures are all preserved. There was no earlier Type6 target launch.

The independent binary reader decodes all 10,010 observation bytes without importing the primary gate or executing a target/provider. The malloc/free/memcpy/_callnewh bookends establish actual current SysWOW64 provider identity and 32 entry bytes, not deep allocator or Original private-CRT equivalence. Type10 failed/current and Type11 current evidence retain exact membership and all prior hashes.

Evidence: `local/type6_current01/qualification.json`, SHA-256 **6e2c06b4f10aa1510f9c8126240485cbdbbffc3fce25e8424d63213332d5f9c0**. The self-excluded `artifact_manifest.json` seals the entire family and final tracked-output snapshots. Both current code gates and the independent recorded reader pass; primary runtime admission remains pending. Populated children, the ordinary record-clone fragment, recursive cloning/lifetime, allocation failure, Original EH/class dispatch, factory ownership, startup equivalence and gameplay remain outside this result.
