# Library binding audit B — 2026-10-18

Scope: STL/containers, templates, compiler-generated helpers, trivial bodies,
iostdnet/iometrics telemetry, and xlive. This is an audit, not reconstruction or
runtime rewiring. CRT/math/Lua/zlib are covered by the sibling audit A.

**Decision:** keep current-toolchain STL for freshly constructed host objects.
Keep the existing compatible operations for borrowed checked-container storage.
Do not reconstruct the remaining thousands of library-tagged bodies merely
because they have original callers. Current source existence, actual host
adoption, and execution are separate facts. The sampled operation families below
do not establish that every original instantiation is needed or implemented.
The generated/template audit did locate raw ownership gaps: no exact common
body was found for `004404F0` intrusive release or `004042C0` SAP base cleanup.
Those require a concrete adopted raw caller before new work is authorized by the
library exception. Fresh ownership/compiler cleanup can satisfy new typed owners.

## Scope, counting and caller evidence

Baseline: `origin/main` at `3d581c328`; project configuration is
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The verified live count
was 76,774, matching the shared snapshot. Rebuilt local index: 76,332 internal
functions and 139,535 static call edges. The table below is an inventory, not
runtime reachability or a count of new implementation work.

| Name/tag union | Internal rows | Any indexed caller | Function ledger source under `src/` |
| --- | ---: | ---: | ---: |
| `STL_` / `std::` / `stl*` tags | 12,825 | 6,424 | 1,004 |
| `TMPL_` / `tmpl*` tags | 782 | 251 | 281 |
| `CG_` / `cg_*` / `rtti_vslot` tags | 2,852 | 518 | 567 |
| `trivial_body` tag | 1,405 | 539 | 493 |
| `TELEMETRY_` / `block_telemetry` tag | 362 | 298 | 0 |

These unions can overlap. A source ledger row can be a qualified interface,
not an installed original ABI. The named-only telemetry count is 360; two more
tagged rows explain the dispatch's 362. “Any caller” includes library-to-library
and dormant paths, so it must not be reported as “live game functions.”

Each retained sample has original game callers, directly or through the stated
container chain. `bsp.py callers` was supplemented by
`tools/image_ref_scan.py` / `tools/callsite_census.py`: E8/E9 rel32 candidates and
absolute DWORD occurrences in the installed original image. Exact example sites
are recorded below and in the companion audits. A byte occurrence alone is not
an instruction or reachable caller. The census tools' nearest preceding function
is not reliable when the candidate falls outside that function's actual body;
the telemetry audit records an unlisted `00750840` initializer and the generated
helper audit records unresolved containing functions rather than inventing owners.

## Raw-container coverage

`layout-bound` means the current STL cannot act on these raw bytes. It does not
require every old instantiation to be hand ported: preserve an existing compatible
body or translate the complete owner to a fresh typed representation.

| ID / address or family | Concrete original game caller / byte site | Class | Original layout and existing source evidence | Recommendation |
| --- | --- | --- | --- | --- |
| L1 `00450540`, `004954F0`; checked vector of pooled 8-byte strings | Game OnInit `004E3AA0` CALL `004E3CD3`; DLC reader `0050A820` CALLs `0050A867` erase, `0050A8F8` append | layout-bound | 10h header `{opaque,begin,end,capacity}`; source `native_string_vector_append.cpp`, `native_string_vector_insert_one.cpp`, `native_string_vector_erase.cpp`, resize/assignment/range lifetime bodies. Original append retains alias-safe growth and owner checks. Separate bounded source in `native_checked_string_storage.cpp` has recorded differential evidence. | Retain raw bodies and pool context. Named append/erase/insert-one/resize/assignment operations exist; this is not blanket closure of every insertion instantiation. No new body justified here. |
| L2 checked vectors of DWORD/pointer and nested headers, allocator `00553260` | Input configuration `00698A10 → 00698980 → 00698040 → 00557590 → 00553260`; input settings `006A7BE0 → 0055A9A0 → 005586B0 → 00557590` | layout-bound | 10h checked header, four-byte element stride. Existing `native_dword_vector_*` covers allocation, insert/append, copy/fill, erase, destroy and outer headers; `native_input_configuration_runtime.cpp` uses explicit raw services. Header allocation sizes and element destruction cannot be delegated to a modern overlaid vector. | Keep existing operation/provider contracts. Source allocation explicitly uses accepted current CRT, not original heap/new-handler/EH identity. No additional original heap reconstruction follows from this sample. |
| L3 legacy SBO `std::string`, `00408720`, `004089E0`, `004072D0` | Original callers include command-line `004E27E0`, main-menu update `00599DB0`, and class readers; 633 indexed callers of assign, including library throw paths | layout-bound | 1Ch object: proxy+0, buffer+4, length+14h, capacity+18h, inline threshold16. Six operations in `native_legacy_sbo_string.cpp`: counted/substring assign, grow, erase, allocate, destroy. Header asserts size/offsets. This is distinct from the game's 8-byte pooled `NativeString`. | Keep existing storage layer. Fresh `std::string` values are fine after an explicit value copy; no reinterpretation or cross-domain free. |
| L4 `std::list<hull>` head/constructor `0041BF50`, `0041E1B0` | Avoid manager `00421500` CALL `0042153E → 0041BF50` | layout-bound | Manager list at+6Ch, head+70h/count+74h; checked list header0Ch and node links+0/+4, payload+8. `native_hull_list_head.cpp`, `native_avoid_manager_lifetime.cpp`, existing clear/tidy bodies. Constructor preserves header+0. | Retain raw allocation/head/clear/tidy operations. Original ABI is supplied only where the existing source explicitly promises it. |
| L5 deque of pooled strings `0068E780`, `0068E8A0`, `0068EB90` | AwardTracker TriggerHint `00690FD0`, CALL `006910EE → 0068EB90`; tracker pending deque at+68h | layout-bound | Map+4, map-size+8, front+0Ch, count+10h; two 8-byte strings per 10h block. All three operations exist in `frontend_seg16_24_28_cc13.cpp`; allocation/copy/destruction remain explicit `Fe5Host` services. | Keep raw block geometry and FIFO order. All three named deque operations have bodies; do not infer whole award-host runtime adoption. |
| L6 scene database string→record maps and integer sets, e.g. `00466A20`, `0046B570` | Database destructor `0046EFB0 → 0046B570 → 0046A470 → 00466A20`; `seg6_cc13.cpp:922` accesses map+18h and sets+158h/+164h through raw headers | layout-bound | Head/count+4/+8, checked owner/node iterators; record-map node nil+19h. `native_scene_record_map_*` supplies extrema, lookup, rotations, allocation/link/insert, iterator, erase/range and destruction. `native_scene_integer_set_*` is separate. | Preserve source and explicit frame/provider contracts. The parent still has host-dispatched operations; existing bodies do not prove all are installed in that host. No missing sampled primitive established. |
| L7 integer→44h crew map `0087ADB0`, `0087B260`, `0087B750` | Damageable Lua reader `0087CA80`, CALL `0087D660 → 0087B750`, then stores SoldierClass at mapped+40h | layout-bound | Descriptor+60h map; node value+10h. `native_damageable_crew_tree.cpp` contains nine sampled compare/two-rotation/decrement/allocate/link/unique/hint/subscript operations. Missing-key subscript copies seventeen uninitialized DWORDs; the inspected caller writes SoldierClass at+40h. | Keep existing raw insertion family. No inspected downstream reader established dependence on the other uninitialized bytes. Do not request original allocation-order/garbage-value emulation. Whole raw Lua-reader adoption remains qualified. |
| L8 VFS mount/string/physical/pending trees, lists and pair vectors, representative `00BE0D00` | Manager teardown `00BE1F60`, CALL `00BE2151 → 00BE0D00` on manager+3Ch | layout-bound | `native_vfs_manager_lifetime.cpp` tears down raw members+30/+3C/+48/+54/+60/+6C/+7C/+94. `native_vfs_mount_tree.cpp` covers rotations/subtree/iterator/range/destructor; sibling raw providers exist for the other members. Mount nodes24h: key+10h/+14h, provider+18h, color/nil+20h/+21h. | Keep compatible teardown and shared string/CRT services. “Post-free tail” in prior evidence reads surviving owner/iterator storage; this inspection did not find a freed-node read requiring original heap behavior. |
| H1 fresh host vectors: `GameDynamicsState`/marker projections | Original dynamics count `00445DB0`, walks `004462D0`/`006DC*`; current typed counterpart reads `.pending.size()` | bind-safe | `game_dynamics_list.hpp/.cpp` and `world_entities.hpp` explicitly project values into newly constructed STL vectors. For the sampled size/ordered walk operations on valid fresh objects, current STL gives the same count/element sequence; capacity/addresses are not inputs to those operations. | Continue stock STL for these value-level operations. Original owner offsets in comments are evidence mappings, not permission to overlay a modern object. |

The installed Release Win32 toolchain was measured using a local executable
named `libB_layout_probe.exe`, linked with `/MANIFEST:EMBED`, `/MD`,
`/D_ITERATOR_DEBUG_LEVEL=0`. Its result was:

```text
MSVC=1951 STL=145 IDL=0 pointer=4
vector<unsigned>=12 string=24 list<unsigned>=8 map<int,int>=8 deque<uint64>=20
```

Legacy checked vector/string/list/tree header sizes in the inspected original
bodies are 16/28/12/12. A matching total size, as for deque, does not establish
map-block geometry, iterator layout or ownership compatibility. Reproduce the
measurement with `sizeof` on those five standard containers in a Win32 Release
translation unit; no legacy bytes are cast to a modern object in this probe.

## Sorting, generated helpers and external services

The companion reports provide the original-body, game-callsite and replacement
comparisons:

- [Sorting and equal keys](OCT18B_LIBRARY_BINDING_B_SORT.md).
- [Compiler-generated/template/trivial helpers](OCT18B_LIBRARY_BINDING_B_CG_TMPL.md).
- [Telemetry and xlive](OCT18B_LIBRARY_BINDING_B_TELEMETRY_XLIVE.md).

| ID / family | Concrete caller and original versus substitute evidence | Primary class for this decision | Recommendation / effort |
| --- | --- | --- | --- |
| S1 session sort `00574A50` | Browser `005789C0`, CALL `00578ECB`; equal names/modes/capacity can tie. Row build chooses the last matching name at screen+26Ch. Modern `std::sort` has no portable identity-order guarantee; current MSVC matched preserved source in the bounded probe. | behaviour-sensitive | Optionally bind the already-existing `msvc9_sort` template; 15–30 minutes plus focused check, no new vendor reconstruction. No observed current-STL defect. |
| S2 rewards sort `00625D10` | `0062C2C0`, CALL `0062D43B` after builder `0062A0B0`; at most eight rows. Original insertion sort through32 preserves tied scores, as does current `stable_sort` for this use. | bind-safe | Keep bounded current call. Generic >32 `stable_sort` is not the original introsort contract. 10–20 minutes if documenting a public bound. |
| S3 GUI sort twins `00AA4E00` / `00AAA480` | `00AA5A00` CALL `00AA5AFD`, `00AAAED0` CALL `00AAB3F1`; native widget pointers/accessors. Existing `msvc9_sort` already preserves family permutation. | layout-bound | Keep current algorithm/storage/name adapter. No new reconstruction. |
| S4 defend-record sort `00A28300` | `00A28A60`, CALL `00A290F5`; threat/weight relation is not unrestricted strict weak ordering. First equal-distance anchor wins, then resources/commands change. Current implementation returns `nullopt` above32; concrete host retains construction order. | behaviour-sensitive | Preserve <=32 sequence. Before widening, recover partition/heap behavior or prove a <=32 invariant. 0.5–1 day for bounded recovery; actual >32 gameplay occurrence was not established. |
| S5 render statistics `00B148E0` | Overlay `00B151E0`, CALL `00B1527A`; case-fold-equal names retain distinct record identity, sorted in place. Current modern sort matches bounded probe; debug sink is a bare RET. | behaviour-sensitive | Low-priority output/array-order pin using existing sorter if needed; 15–30 minutes. No proven visible/gameplay divergence. |
| S6 Lua distance pairs `008C9E10` | Original producer `008CCEB0`, CALL `008CD2D7`; equal-distance objects would expose permutation in Lua array. Producer has zero indexed/rel32/absolute refs and was absent from queried Lua bindings. | unreached | Park until a concrete rebuilt registration exists; retain explicit host binding. No reconstruction now. |
| C1 raw generated ownership/cleanup, e.g. `00411F00`, `00B18D50`, `00C5FA4C` | Lock-guard/material vtables and original unwind references; bodies depend on owner fields, counts, vtable writes or original EBP. Existing guard/material source covers named cases; original EBP glue is not reusable native C++ glue. | layout-bound | Keep object-specific bodies; let current compiler emit cleanup for new source owners. A generic `delete` or CRT bind cannot dispose of borrowed bytes. Raw SAP base cleanup `004042C0` remains a coverage gap (medium effort), without an established current adopted caller requiring new work. |
| C2 intrusive ownership `004404F0`, vector resize `00504960` | Unit message CALL `0082232D`; preview CALL `0050A9F6`. Release decrements object+4, vslot0 destroys at zero, then clears slot. Resize uses raw `[data,size,capacity]`; source exists for resize, no exact shared release body located. | layout-bound | Retain/qualify actual ownership service. `shared_ptr` is not a raw substitute. A common release adapter is a small bounded task only if its current raw caller needs it; allow roughly half a day including ownership/throw validation. |
| C3 non-EH array iterators `00403560`, `00453ED0`, true no-op `00404540` | Vehicle/gunnery ctor sites `0081EF7D`/`0086514D`, action copy `004558C2`; ordinary callback loops. No-op is genuinely `RET 0Ch` through vtable. | bind-safe | Fresh native construction/copy and compiler-generated glue supply these value-level operations. A raw callback bind must retain ECX/stack contracts; no-op must pop three words. Small adapter work only. |
| C4 EH array iterators `00BF7C6E`, `00BF7CD1`, `00BF9A96` | Gameplay constructor CALL `00424BDE`; message copy `007043AA`; powerup copy `008E7A42`. Original has reverse destruction/partial-construction EH; existing raw adapters explicitly certify normal flow only. | behaviour-sensitive | Bind current compiler runtime/RAII for fresh arrays. Before extending raw throwing callbacks, qualify failure cleanup and callback ownership; medium/high effort. No exception-triggered gameplay mismatch demonstrated. |
| C5 adjustor `004134D0` | Original `ADD ECX,8; JMP 00C37450`; zero indexed/rel32/absolute references found. | unreached | Park raw adapter until a consumer exists; fresh classes get compiler-generated thunks. |
| T1 telemetry: all 362 name/tag rows | Original startup `0073D410` CALL `0073E4AC → 00757FE0 → 007524D0`, CALL `00752627 → 00A50210`; statically linked iostdnet/iometrics. Rebuilt `Cc15MetricsHost` has no concrete application implementation/owner. | unreached | No telemetry vendor reconstruction now. If enabled later, its ZString/maps/pools are layout-bound and queues/threads/network behavior sensitive; use an explicitly approved game-host policy or identify the exact compatible SDK. No compatible SDK package/version was found. |
| X1 device callback adapter, thunks `00C2F1C0`/`00C2F1C6` (5005/5006) | Original recreation CALLs `00B29972`/`00B29859`. Current adapter passes actual D3D device/presentation pointers with stdcall8/0, matching stub decoration; gated by null online owner today. | bind-safe | ABI forwarding only: keep existing adapter and SDK pointer types. This does not certify that an offline stub implements original overlay lifecycle. No reconstruction needed. |
| X2 render/message thunks `00C2F1CC`/`00C2F1D2` (5002/5030) | Original CALLs `00B2DAA2`, `00BEC1D8`, `00BECD0D`; current frame/message paths call them. Offline stub renders nothing and returns FALSE for message ownership, versus a real SDK boundary. | behaviour-sensitive | Explicit offline substitute only. Preserve ordinal/stack ABI and no-overlay scope; real SDK behavior requires a compatible bound library, not reconstructed import thunks. |
| X3 notification/sign-in/profile/storage/achievement paths | Original game callers and native adapters exist, but current `game_hosts.cpp` leaves online-manager initialization unimplemented and publication null. Stub has 31 exports versus 106 original imports; 5331 is missing on a lazy signed-in profile path. | unreached | No wholesale thunk reconstruction. Before enabling this host, review immediate-failure buffers, queue/event handles and async retirement; add 5331 only with a concrete output/size/failure contract. Small export task; medium state-machine review. |
| X4 installed game-directory xlive override | Current hash matches historical fixed-RVA patch contract; loading it as though it were stock can patch wrong rebuilt addresses. Existing run script instead requests a verified extracted redist runtime; the main-worktree private runtime was absent at inspection. | behaviour-sensitive | Preserve explicit runtime identity/selection. No binary was changed or executed. Verify the selected runtime before any later online run; no vendor body reconstruction follows. |

**Grouped counts:** 25 decision rows across the two tables: **bind-safe 4,
layout-bound 11, behaviour-sensitive 6, unreached 4**. Companion tables use finer
groupings and overlapping ABI/layout qualifiers; do not add their row totals to
these counts. In particular, T1 and X3 use current rebuilt reachability as the
primary class, while their dormant contracts still have documented risks.

### Ranked behaviour-sensitive actions

1. **AI defend >32 / nonstandard relation (S4):** state-bearing gap already
   explicit in current source. Establish input bound first; if it can be exceeded,
   the remaining comparison sequence needs a compatible body (0.5–1 day). A
   generic weight comparator is insufficient.
2. **External XLive identity (X4):** preserve the explicit verified runtime path;
   the installed fixed-RVA override is not stock. This is selection/verification
   work, not new reconstruction. No current unsafe runtime execution is claimed.
3. **Reached offline XLive render/message semantics (X2):** intentional restricted
   substitution, not overlay/message equivalence. Keep the offline scope; review
   a concrete SDK bind before enabling online behavior (medium integration work).
4. **Raw EH array failure paths (C4):** qualify cleanup before admitting throwing
   callbacks. Fresh native arrays use current compiler/runtime cleanup; reconstruct
   no compiler-runtime body merely to duplicate that service (medium/high review).
5. **Session tie order (S1):** cheap pin to an existing historical template if exact
   identity permutation is required (15–30 minutes). Modern/preserved source probe
   agreed in 640 cases; an existing mismatch has not been demonstrated.
6. **Render diagnostic identity order (S5):** same optional pin, lower priority;
   the native print sink is disabled and no gameplay effect was shown.

The sorting probe used four actual source element classes, repeated keys and
lengths 0/1/8/31/32/33/40/41/64/129. Preserved `msvc9_sort` versus installed
`std::sort` had 0 identity-order differences in 640 cases. `stable_sort` diverged
in 80/160 cases per class, first at 33 elements; that does **not** invalidate the
concrete <=8 rewards caller. This is a source-family probe, not execution of the
original sort bytes or proof for every input/comparator/exception path.

No independently established original `partial_sort` or `stable_sort`
instantiation was found in the bounded sample. The three-caller merge-like
`00499E70` remains unresolved; its `STL_List_Sort` discovery tag is insufficient
to identify its exact element/comparator contract. It is outside the 25 decision
rows and is not silently certified bind-safe. No current `std::partial_sort`
call was found. Original qsort-to-stable-sort use belongs to audit A.

The `rtti_vslot` solver `00403720` and stale `cg_static_dtor_stub` GUI pool
allocator `00AAC260` are concrete semantic bodies, not generic compiler runtime.
Their existing source must be retained; their discovery tags are not permission
to replace them with a stock RTTI or global-destruction helper.

## Verification and limits

- `./scripts/build.ps1`: Win32 Release build passed; both enabled existing tests
  passed (`reconstructed_math`, `tool_tests`). No C++ host file changed.
- Read-only original byte scans and project/program-verified Ghidra queries;
  seven raw-layout classification notes appended through `bsp.py ledger add-name
  --append-evidence`, preserving names and previous evidence.
- `ghidra_annotate.py --apply` used the shared write lock, saved prior comments
  in `local/ghidra-annotations-20261019T043244Z.json`, and saved the project.
  All seven affected exports were refreshed through `bsp.py ghidra export`.
- No game runs, network service calls or deployment. Historical differential
  records were inspected rather than rerun; this audit adds no original ABI,
  native exception-runtime, full-host-adoption or gameplay proof.
