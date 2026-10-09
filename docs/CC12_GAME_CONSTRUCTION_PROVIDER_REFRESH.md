# CC12: game construction provider refresh

At `e23cd31b86ae9bf89cced29fe6a3a64524a32cdc`, the full ordinary application
context for `GameNativeGameRuntime` is still unready. The old GlobalConfig
raw-lifetime API, canonical Sound-string and live-FOV interface blockers are
resolved. The previously proposed game table owner also exists. Actual game
allocation/publication, several retained contexts and the complete cleanup
payload domain remain unconnected.

This is a current Source refresh of
[the earlier readiness audit](CC12_GAME_APPLICATION_CONSTRUCTION_CONTEXT_READINESS.md).
Its stale claims are not edited. The companion report maps all 19 construction
fields and all 23 lifetime fields, plus the nested GlobalConfig/profile
requirements. Availability of a provider below does not mean the application
already constructs its consumer.

## What is resolved

| Earlier blocker | Current Source evidence | Remaining obligation |
| --- | --- | --- |
| GlobalConfig semantic singleton API | `GlobalConfigContext` now owns a `SoundLifetimeAccess` view; `00432650` captures that domain's lock, reloads its manager and registers the actual owner. | Compose retained publication/context/effects and add genuine raw deletion dispatch. |
| Sound built with independent CRT strings | `SoundServices` passes `app.vfs_->borrow_raw_services().strings` to `GameSoundRuntime`; samples/cache retain that exact input. | Borrow the same object for GlobalConfig and keep it alive through drain. The loader's identity guard remains. |
| No live FOV reference | `GameNativeSettingsApplication::fov_divisor_00f889b4()` returns the actual process-owned float at settings `+34h`. | Borrow it in the retained loader context; no cached value or PE cell. |
| Missing unit/rank table owner | `GameNativeGameTables` owns real 23x16-byte unit rows, count and 12x97-DWORD rank storage. | Instantiate it in the future owner, explicitly supply `unit_frame_word_preimage`, and borrow once before the original builders run. |

The table owner deliberately invokes neither builder. Its count-zero and
single-borrow guard does not authorize replaying raw contexts or adding a
second 23-row append. The earlier recommendation to implement that owner is
therefore obsolete; application use is still absent.

## Actual root and context frontier

Production census finds `GameNativeGameRuntime` only in its implementation,
header and the fixed-step attachment/consumer. It finds no application
construction of that runtime, `NativeGameConstructionContext`,
`NativeGameLifetimeContext`, or `GameNativeGameTables`, and no caller of
`attach_native_game`. With no native game attached, the fixed-step branch
records the native physics site and returns.

The actual `71A0h` allocation, caller-wide zeroing, actual eight-byte name
header and retained caller frame for the established `0073E150..0073E1A9`
contract are still absent. The runtime owns its method tables and operation
frames; it borrows the allocation and composed contexts. It cannot substitute
for that caller producer. The existing constructor accesses the embedded
profile at actual `game+650h`, writes the three grid publications after their
respective constructions, and publishes `E188A8` late, before Dyn creation.
Its partially failed graph and publications must remain retained.

| Construction obligations | Current authority | Still missing |
| --- | --- | --- |
| Game publication | `GameNativeSettingsProcess::game_00e188a8()` is the canonical cell. | Actual game object/caller; preserve late publication. |
| Three grid cells and movie cell | Explicit constructor and destructor reference fields exist. | Actual shared application publication owners; semantic movie objects are not raw owners. |
| Strings and numeric/literal inputs | Canonical VFS string object and retained verified `GameNativeReadOnlyData`. | Bind exact references without sibling strings, mutable PE cells or numeric callable tables. |
| Embedded profile | Concrete `NativePlayerProfileCalls` plus canonical settings/online/game/SDK references and RO literals. | Stable construction context owner; eventual receiver remains `game+650h`. |
| Construction calls/embedded/array constants | Existing concrete normal Source defaults and explicit RO inputs. | Retain their aggregate instances with the future game frame. |
| Tables | `GameNativeGameTables` provider exists. | Application instance and explicit native-frame preimage. |
| GlobalConfig | Raw lifetime interface, normal owner/getter/loader and the five loader-field providers exist. | Actual `F878E4` publication/context owner, complete effects and deletion bindings. |
| Resource parsers | VFS's `raw_game_resource_parsers_context()`. | Borrow exact retained manager/string domain. |
| Grids | Renderer `game_grid_context()` borrows actual streams, geometry, declaration cache and pools. | Three explicit descriptor preimages, canonical publication cells and real scalar dispatch. |
| Dynamics and game tables | Canonical `GameNativeDynProcess` and runtime primary/contact tables. | Same construction/physics-lifetime contexts and the application runtime owner. |

The settings process exposes actual publication **references**, not a completed
online/game owner. Its SDK service is bound to `SoundServices::xlive`; future
selected-user imports must retain that real library and reload the actual
manager/user/game in the existing order. A context provider must not initialize
those cells, suppress that branch, fabricate SDK results or publish early.

## Lifetime obligations remain genuine blockers

`NativeGameLifetimeCalls` still has three effective pure-virtual obligations:
`virtual_scalar`, `virtual_terminal`, and inherited `virtual_04`. The last
dispatches a captured record through the current peer+4 receiver/current slot4.
No production derived root call service supplies this full payload domain.
Numeric original profiles are not callable Source tables, and the presence of
initially null fields does not justify empty teardown implementations.

| Lifetime fields/domain | Available now | Unclosed actual producer/binding |
| --- | --- | --- |
| Disabled/string/manager cells | Canonical process AA4/AA8/AA0 and VFS raw contexts. | Retained root/profile/array contexts must share these exact cells. |
| Seven UI/award publications, movie and three grids | Parent destructor field contracts. | Actual application cells and reached scalar receivers. |
| Arrays | Singleton-owned `GameObserverRuntime::lifetime()` and raw strings. | Same root calls object and real terminal payload dispatch. |
| Embedded state | Complete cleanup/lock routines. | Actual `E0AF14` cell and peer/record slot4 target domain. |
| Profile/race | `NativeGameProfileLifetimeContext` and concrete cleanup bodies. | Stable aggregate borrowing the same eventual root call service, not independent callbacks. |
| Three singleton deletions | Actual raw manager and complete unregister/lock schedule. | `E18E6C`, `E18D80`, `F89B34` owner cells and genuine scalar payloads. |
| Lua globals | Existing cleanup body. | Actual 10h headers at `108FF30/40`; the retained Lua globals object only supplies x360comp/region. |
| Resource cleanup | VFS actual `NativeResourceManagerContext`. | Existing guards require the same root/profile calls and AA0/AA8/AA4 cells. |
| Class cleanup | Existing `CE6C68` deletion branch and `game_classes` binding field. | No application assignment of that binding; actual `E187BC`, six 10h vectors and mapped payload dispatch remain absent. |
| Physics | Canonical engine/world/body-creation/task/convex contexts. | Same root calls and real callable attachment tables beyond the concrete convex route. |

The GlobalConfig deletion gap is separate: no production `GlobalConfigContext`
or `GlobalConfigEffects` provider, and no `CE3D98` branch, exists in the current
raw singleton dispatcher. Existing sample final release supplies the first
array's known sample domain; it does not establish arbitrary current stop/zero
targets for the second array. The integrated
[second-array caller audit](CC12_GLOBAL_CONFIG_SECOND_ARRAY_DIRECT_CALLERS.md)
still leaves its nonnull producer unattributed. This refresh does not repeat
that worker's address analysis or claim its indirect targets are harmless.

## Smallest independent next implementation

Propose **`cc12_game_native_player_profile_context`**: a retained,
construction-only provider in
`include/bsp/game_native_player_profile_context.hpp` and
`src/game_native_player_profile_context.cpp`.

It can own the already concrete `NativePlayerProfileCalls` and one
`NativePlayerProfileContext`, borrowing the exact VFS `.strings` object,
`&settings_process.settings()`, the same publication/SDK references from
`settings_process.profile_context()`, and verified RO literals at
`CE3A0C`, `CEF794`, `CEF15C`. Disable copy/move so retained context/call
addresses stay stable. Preparing or borrowing it performs no profile
allocation, construction, reset, SDK import, table build, publication or cleanup.

This closes one actual context-provider gap without requiring GlobalConfig,
World, root lifetime dispatch or the `71A0h` caller. The actual embedded receiver
and all constructor calls remain at their original parent sites. The provider,
VFS strings, RO data and live SDK binding must outlive every borrowed/failed
operation. The separate destructor profile context must use the eventual
shared `NativeGameLifetimeCalls` object; adding a second cleanup service would
violate existing identity guards. This proposal earns no original-function
credit and does not admit application game construction by itself.

## Evidence boundary

Current source files were inspected in bounded slices and by exact symbol
census. Of the earlier audit's 64 distinct provider-evidence files, 58 still
match byte-for-byte; six changed (`game_hosts`, GlobalConfig header/source,
singleton deletion header/source and `game_main`). Both complete game context
headers/bodies and the profile header/body match their earlier pins. New
providers and changed relevant bindings were checked directly. The report
records current hashes and all field mappings.

Only this document and its report changed. No Source, old documentation,
CMake, configuration, ledger, Ghidra state, build, test, probe or game process
was changed/run. No original-address lease was required for this Source-only
refresh; Root's `009035D0` and peer work remain untouched. Interface readiness
and retained historical evidence are not runtime, ABI or gameplay validation.
