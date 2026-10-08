# Game application construction-context readiness

Read-only packet `cc12_game_application_construction_context_readiness`.
Base main `b68fdeb18`; inspected worker merge `9128fee1663bc111231e3d7fe7f7ae69b7dea641`.
**The full game context remains unready. A bounded table-storage/context owner is
Source-admissible as the next independent implementation.**

Current Source expresses the requested construction services through
`NativeGameConstructionCalls` plus `NativeGameConstructionContext`. The audit maps
all **26 effective construction methods, 19 constructor fields, 93 effective
lifetime methods and 23 lifetime fields**, including inherited methods and the
nested profile services. Every mapping identifies its actual provider, lifetime
authority and remaining requirement in the
[JSON report](../reports/cc12_game_application_construction_context_readiness.json).
“Available” below means a concrete provider in the current production Source
graph; no application execution was performed.

## First missing owner and concrete binding blocker

The first aggregate producer is still the actual application's `71A0h` allocation,
whole-allocation zeroing, actual `8h` name header and retained context frame at the
already established `0073E150..0073E1A9` caller fragment. The ordinary startup path
does not instantiate `GameNativeGameRuntime` or attach it to fixed-step simulation.
That runtime owns its Source tables and operation frames, while borrowing game
storage and the full constructor/destructor graph.

The missing global-config context is more than an omitted aggregate initializer:
`GlobalConfigContext` currently takes **`SingletonLifetimeDomain&`**, and the getter
uses the semantic manager's `system_owner()`. `GameSingletonHost` uses the actual
raw `01090AA0` manager through `SoundLifetimeAccess`. A semantic cast or another
manager would not bind those domains. The raw singleton deletion table also has
no `GlobalConfig` binding/profile branch, and no production `GlobalConfigEffects`
implementation supplies the required current-object stop/release calls.

Normal `00432650`, configuration construction/destruction, and the complete
`0087D7B0` loader already exist. Their Source bodies do not establish a production
raw owner, effect dispatcher or lifetime registration. They require a proper raw
lifetime adaptation and real retained deletion/effect bindings before the actual
game constructor can use them.

## Context/provider map

| Obligation | Current provider and authority | Remaining connection |
| --- | --- | --- |
| Raw game allocation/name/zero frame | Existing caller contract; runtime borrows allocation | Application owner and full contexts absent |
| All 26 construction service methods | Concrete existing Source defaults, including allocator, array, tracked-lock, race, Lua, parser, grid and Dyn calls | Default bodies still require the contexts below |
| Constants/profile literals | Retained `GameNativeReadOnlyData` RO bands | Bind exact references; never obtain mutable settings or callable methods from RO image words |
| Owning strings | Canonical process pool/disabled/manager cells through VFS | Game/profile/cleanup must use these same cells |
| Embedded profile | Concrete profile/mission-progress/counter/tree/string bodies | Stable context and genuine `game+650h` receiver absent |
| Settings, current online/game, SDK | Canonical settings process and actual XLive ordinal5331 adapter | Complete online/application/game producer and reached branch domain unproved |
| Unit/rank tables | Complete builders and compiled authored definitions | Stable row/count/rank storage owner and explicit frame preimage absent |
| Global config getter/owner | Existing normal body with semantic lifetime API | Actual raw lifetime/deletion/effect binding absent |
| Config Lua services | Actual `GameNativeLuaServices::bootstrap()` and `binding().files()` | Retain and activate the same binding around reached native interpreter entries |
| Config sounds | Existing cache publication/sample release, currently constructed with `crt_string_storage()` | Canonical pool binding is missing; loader's exact shared-string guard would reject the current cache |
| Config FOV | Live settings `F88980+34h` | Borrow live field; PE/BSS zero is not a runtime value |
| Game resource parsers/resource cleanup | VFS-owned `GameNativeResourceApplication` contexts and registered deletion bindings | Borrow existing exact raw manager/string domain |
| Grid construction | Renderer-owned `GameGridGraph` with actual streams/device/geometry/pools | Three actual publication cells, explicit descriptor preimages and real grid scalar dispatch |
| Dyn construction/physics cleanup | Canonical process dynamics/world/body-creation/task/convex services | Compose exact same references into both contexts; runtime checks identity |
| Runtime primary/contact tables | Complete `GameNativeGameRuntime` table and canonical Dyn contact table | Runtime still lacks its application owner |

Profile reset runs before the game constructor's late `E188A8` publication. A
selected-user settings import reloads the real current manager/user/game and may
write current game `+6ACh`. Preserve that order and domain. Publishing early,
forcing an unselected manager or supplying synthetic SDK responses is not an
admissible context connection.

There is a second exact config binding gap: `SoundServices` passes
`crt_string_storage()` into `GameSoundRuntime` (`game_hosts.cpp:1357`).
`SoundSampleRuntime` retains it as `sample_cache_context().strings`. This is a
separate malloc/free storage service. `load_native_global_config_0087d7b0` checks
that cache string service is the exact `context.strings` object before opening Lua,
so a canonical VFS/string context cannot use the current sound cache unchanged.
The sound implementation exists, but this binding is unready. Establish the real
canonical binding before sound-owned strings are created and preserve its lifetime;
do not switch an existing live storage reference, substitute a cache, use CRT
strings for the config owner, or weaken the guard.

## Lifetime obligations that cannot be replaced by a generic dispatcher

The complete normal destructor is `004DCF90`; scalar deletion is `004DE270`.
The effective lifetime service has **90 concrete defaults and three abstract
entries**:

- `virtual_scalar(captured, byte_slot, flags)` for reached raw globals, menu/movie,
  grids, class values, scene payloads and embedded owners.
- `virtual_terminal(captured)` for actual current slot0 when reference counts reach0.
- Inherited `virtual_04(receiver, record)` for the current peer+4/current slot4
  callback inside `007849C0`. This third method is also required; implementing only
  the first two would leave the service abstract.

Each must dispatch an actual producer's object through its established Source
implementation or genuinely callable Source table. Grid construction, for example,
still stamps numeric `CFD484`; its existing `0070B280` Source deletion needs the
same renderer context. Calling that image vtable as a host pointer is not valid.
The physics defaults likewise require actual callable attachment tables except
for the explicitly bound concrete convex route.

| Lifetime child/context | Actual component available | Unclosed producer or authority |
| --- | --- | --- |
| Arrays | Canonical raw strings and singleton-owned observer lifetime | Same eventual root calls object and actual terminal payloads |
| Embedded state | Normal cleanup and tracked critical sections | Actual `E0AF14` flag and peer/record `virtual_04` producer |
| Profile/race | Raw pooled bridge and real nested collection/string cleanup | Stable aggregate sharing root calls and exact disabled/pool/manager cells |
| Three singleton deletions | Actual manager and complete unregister/lock schedule | Raw `E18E6C`, `E18D80`, `F89B34` owner cells and scalar payloads |
| Lua globals | Existing `B6CF90` body | Actual `10h` headers at `0108FF30/40`; current `LuaRuntimeGlobals` owns only 12 bytes for x360comp/region |
| Resources | Actual VFS resource-manager context and strict identity guards | Same root/profile lifetime services through cache clear and drain |
| Class globals | Existing registry getter and vector/tree cleanup | Actual `E187BC` publication and six `10h` headers at `E1875C..E187AC`, plus mapped payload scalar |
| Physics | Canonical engine/world/body-creation contexts and normal defaults | Same root calls, allocators/pools and reached attachment producers |
| Root publications | Required slots are explicit in Source | Actual UI/award/movie/grid cells; semantic host objects are not raw owners |

Initially null fields in the constructor do not prove an empty post-title cleanup
domain. A new empty vector, flag, singleton or sibling publication cannot serve
as its missing producer. All retained game/child operations and entered locks
must survive failure; no implicit rollback, replay or native EH claim is added.

## Smallest concrete next Source packet

Proposed packet: **`cc12_game_native_table_context_owner_source`**.

Add a stable canonical table owner and a `NativeGameTablesContext` view over its
`23 x 16` byte unit rows, shared count and `12 x 97` DWORD rank output. Borrow
`native_unit_conversion_definitions()` and `native_gunnery_preference_words()`;
their Source literal/data identities are already established. The owner must
outlive every table consumer and prevent accidental sibling table domains.

Require `unit_frame_word_preimage` explicitly from the future retained caller.
Its upper24 bits are opaque original stack input, so the owner must not choose0
or another undocumented native default. This explicit Source input does not
qualify natural Native caller-stack identity.

Preparing the provider must not invoke either builder. Keep the original
`008D9150` and `00727BD0` constructor sites. Initial unit append needs fresh count0;
never reset a used count or allow a second23-row append into23-row storage.
Preserve the rank builder's per-row clear/order and authored row0 ID97 cross-row
write. No new arithmetic or ABI recovery is needed.

Suggested files are `include/bsp/game_native_game_tables.hpp`,
`src/game_native_game_tables.cpp`, its CMake registration and focused packet
documentation. This provider can be implemented independently without game/profile
allocation, `E188A8` publication, startup/menu wiring, config adaptation or a
partial lifetime dispatcher. It closes one concrete dependency; full application
context admission remains open afterward. Use the required Win32 build and
relevant existing checks, retaining new artifacts with their actual build date.

The next hard binding frontier remains the config raw lifetime/effect domain and
its missing canonical sound-string binding.
Any fresh register/x87/EH recovery needed later must be split into a bounded Astra
packet after identifying the actual producer/address; none was performed here.

## Retained evidence and limits

The local capture retains **789 complete current provider/support files**, including
the bounded providers' local include dependencies and Source method-body inventory.
It reuses the six whole September `R105/R106/R107/R109/R124/R155` archives, rehashing
their exact bytes and retaining the prior **1,706-member** size/hash/CRC proof.
Their original timestamps and controlled fixture boundaries remain explicit.
No archived recipe was replayed, and current files are not relabelled as historical
compiler inputs. The prior complete 4,037-file Source capture was rehashed against
current Source: 4,033 files are unchanged and the four canonical fallback changes
are retained with current bytes. This reuses physical evidence without reopening
the full bootstrap analysis.

A verified read-only Ghidra query records the existing `004DDB90` extent. The
retained complete Original PE was rehashed; the entire `1,751` byte constructor
and previously established `90` byte `0073E150..0073E1A9` caller fragment are pinned.
That fragment is not a whole application initializer reconstruction. No new ABI,
x87, EH, build, probe, game or title execution was performed.

Artifact root: `local/cc12_game_application_construction_context_readiness_20261008a/`.
The final `whole_artifact_manifest.json` seals current captures, reused whole
archives and the two tracked deliverables. The first inventory attempt missed a
local C++ `Calls::` alias; its driver is preserved and the corrected extractor
resolves the exact alias before checking all method obligations.

Editorial attribution: the prior pending profile-readiness JSON contained
`Normal4DCFE0` in one prose row. The existing header/Source entry is `004DCF90`;
the parent was notified. This packet modifies only its two new documentation files.

Primary review accepted the readiness findings after independently rehashing
all 4,861 retained artifacts and six complete historical archives, matching the
whole constructor and caller-fragment bytes to the current installed PE, and
checking all 139 effective method obligations against current declarations.
All 136 concrete default bodies match current Source; the three inherited
abstract lifetime methods remain explicit. Six captured provider/support files
now differ through subsequent registry, documentation and staged CMake changes;
none of this snapshot is asserted to be current compiler-input evidence.

Primary receipt: `local/cc12_game_application_construction_context_primary_review/receipt.json`,
SHA256 `3ff516531257480102b1a827643dbe9706ed903c3f8352d14986b240ee0233c6`.
The stable table-owner Source packet is authorized within the bounds above.
The full application, raw configuration and sound-string bindings remain unready;
this review adds no Source/build/execution or reconstructed-function credit.
