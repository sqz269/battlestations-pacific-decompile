# Retained fallback resource type owner API

Baseline: `8fdcebc3d0fcf2b2f62c55988c81363cb0323de9`. This packet implements the owner API admitted by `CC12_MESH_RESOURCE_CLASSIFIER_RETAINED_COMPOSITION_FOLLOWUP`. It changes only `game_native_type_storage.hpp/.cpp` and this document/report. No Native body, Ghidra mutation, ledger or CMake registration is added.

`GameNativeTypeStorage` now owns the independent fallback guard and four descriptor words, and exposes `fallback_resource_types()` as a stable reference to a retained view. `initialize_fallback_00b86a00(existing_counter, common_root_bootstrap)` explicitly invokes the existing genuine fallback initializer. It is not called from `initialize_resource_types`, VFS startup or any other application entry in this packet. A startup call and full resource classifier/loading/retirement composition remain separate admissions.

## Storage and binding

`GameNativeFallbackResourceTypeStorage` contains a reference to actual volatile guard `0109020D` and a reference to the actual four-word volatile array with roles `0109021C/20/24/28`: own ID, Scene ID, root ID and name token. The array reference fixes its extent without copying IDs. The getter returns a const reference to the retained view; the guard and words remain live volatile cells, while the view's bindings cannot be reassigned. There is no temporary view backing or additional domain.

All new fields are appended after the prior final `group_types_` field. The compiler's complete class-layout diagnostics, from current and baseline owner compilations using the same Release Win32 settings, verify all **49** pre-existing field descriptions, offsets and order remain unchanged. Source owner size changes from **564 to 592 bytes**:

| Source owner offset | Appended field | Role |
| --- | --- | --- |
| 564 / `0x234` | `fallback_guard_0109020d_` | Independent volatile byte guard |
| 568 / `0x238` | `fallback_0109021c_[4]` | Four volatile DWORDs, own/Scene/root/name |
| 584 / `0x248` | `fallback_resource_types_` | Two retained reference bindings |

These offsets describe this MSVC Win32 C++ owner, not the original absolute-address layout or binary ABI. The constructor creates the appended storage with zero initial values and binds its view to those actual fields. The owner remains noncopyable and nonmovable. Existing surrounding Core/App consumers can require recompilation because the public owner size changes; this packet does not claim all previous objects remain identical.

The explicit method uses the established `require_common_bootstrap` check for the owner's root/node/light/directional guard and descriptor identities. It then constructs a genuine `NativeMeshResourceTypeIds` over the owner's existing mesh/Scene storage and passes the actual fallback cells, canonical three-word Scene storage, exact `00D631F4` name token, genuine provider and existing counter into `initialize_native_fallback_resource_type_00b86a00`.

For arbitrary direct callers, the counter must be the same one already borrowed by `common_root_bootstrap`. That relation remains a documented precondition: the existing storage check cannot expose or prove the bootstrap's private counter. No counter accessor, eager getter or new domain probe is introduced. The actual current VFS constructor already establishes the relation by constructing `common_types_` from `owner_services_.types()` and `type_storage_.light_types()`, then passing that same owner counter/common bootstrap to existing type initialization. It does not yet call this new fallback API.

The owner and its genuine borrowed counter/root domain must remain live through all consumers and the shared manager drain. The stack-local provider/context exist only for the synchronous initializer call; no consumer stores references to them. Exposing uninitialized storage is not active classifier admission, and this packet binds no classifier to it.

## Preserved initialization effects

The recovered fallback initializer itself is unchanged. On a well-formed call it preserves:

1. A nonzero actual guard returns with no descriptor or counter stores.
2. The cold path stores guard one, then the exact name token.
3. The genuine Scene initializer runs over the canonical owner storage.
4. Scene word0 is loaded, then stored to receiver1; Scene word1 is loaded, then stored to receiver2.
5. The genuine counter getter runs. Its next-ID is loaded and incremented before the captured old ID is stored to receiver0.

The ordered volatile accesses, sticky guard and all completed stores survive a dependency exception. There is no reset, rollback, repair-on-retry, private counter, copied descriptor, duplicate Scene initialization or inferred numeric ordering. The Source owner identity check is an existing well-formed-call check and does not move a counter operation ahead of guard/name publication.

## Object and dependency evidence

Five ordinary objects were compiled: the current owner, unchanged genuine fallback initializer, mesh/Scene provider, light/root/counter provider and baseline owner. The current owner and baseline owner additionally emitted compiler class-layout diagnostics. Each compilation used the recorded normal Release Win32 `/W4 /WX /O2 /Ob2 /Oy- /EHsc /MD /fp:strict /std:c++17` settings. The generated project, full recorded owner/provider command log and selected compiler environment are frozen. `/Oy-` comes from the recorded commands; it is not misreported as a generated explicit `OmitFramePointers` property. `/Bv`, `/sourceDependencies` and the two class-layout diagnostics only collect compilation evidence. All five compiler invocations and 25 complete `dumpbin` commands returned zero; no warning/error diagnostic was found. No executable was linked or run.

Every object is retained in full, with physical symbol/AUX records, raw sections, relocations and complete function-code inventories. Totals across the five objects are **923 sections, 2,878 physical symbol records including AUX, 1,287 relocations and 538 functions**, all with complete linear instruction decoding. This is ordinary compiled Source evidence; these are not newly reconstructed Native function counts.

The new owner initializer is **121 bytes / 39 instructions**, with three exact physical-index REL32 edges:

| Relocation operand offset | Target | Resolution in captured objects |
| --- | --- | --- |
| `+0x0F` | Existing `require_common_bootstrap` | Local owner symbol index 136 |
| `+0x36` | Genuine `NativeMeshResourceTypeIds` constructor | Unique mesh provider definition, index 1535 |
| `+0x6A` | Genuine fallback initializer | Unique fallback object definition, index 22 |

There is no direct counter getter in the owner method. The retained view getter is seven bytes (`LEA` of owner `+0x248`, then `RET`). The unchanged fallback provider is 87 bytes / 33 instructions; its Scene and counter relocations at `+0x23/+0x42` resolve uniquely to mesh index1536 and light index60. The Scene provider's root and counter edges at `+0x28/+0x34` resolve to light index63 and index60. The complete local root `consume_type_id` and counter helper sections, including exception cleanup edges, remain in the inventories and selected relocation graphs.

The selected four-current-object graph still has explicit lower Source lifetime/allocation and CRT boundaries: captured lifetime section construction/destruction, manager retrieval/registration, canonical singleton allocation, exception support and security-cookie support. They were not stubbed or replaced, and this object-only packet does not claim a completed link/runtime. The baseline object is excluded from cross-object provider resolution so duplicate historical definitions cannot satisfy a new edge.

All five complete compiler dependency receipts are frozen. Their union plus selected context contains **125 project inputs** (two modified Source files and 123 unchanged current inputs), all 125 selected baseline Git blobs, two exact baseline owner compiler inputs, **236 external include files** and **seven compiler components reported by `/Bv`**. Whole Source/header/config inputs and compiler tools are saved with byte counts and SHA-256 pins. This is the stated compilation closure, not a complete OS-loader or whole-game dependency claim.

The JSON report points to the full object inventories, command outputs, layout comparison, dependency pins, offline verification and frozen evidence archive. The earlier Source632 selected-scope checkpoint remains historical context; no current whole-project Source/body/definition count is inferred from it.

## Validation boundary and next step

Offline checks verify the exact four owned tracked files, unchanged existing initialization methods and provider Sources, unchanged 49-field layout prefix, complete COFF/code coverage, genuine selected provider edges, dependency closure and frozen payload hashes. There are no new test cases, fixture runs, probes, Native queries beyond the opening brief's health contact, or runtime/gameplay claims.

The primary integrator still needs to run the normal Release Win32 build after integration, recompiling affected Core/App consumers as needed. The new API supplies actual retained Source backing and explicit initialization capability. Choosing a startup call, establishing an original CRT/numeric order, wiring the active classifier, and proving complete resource loading/retirement remain separate work.
