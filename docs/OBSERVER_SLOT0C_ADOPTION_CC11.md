# Observer slot0C adoption through projectile expiry

The complete ordinary event-word dispatcher is reconstructed through the
existing observer schedule. The opt-in abstract `NativeProjectileObserverTickHost`
connects it to the existing `projectile_tick_advance_sim_006e6490` source caller's
expiry/release pair. Actual callback targets, release/world services, profiles,
storage lifetimes and game binding remain required. No projectile arithmetic or
existing slot04/08 provider interface changes.

| Native scope, end exclusive | Coverage | Original protocol |
| --- | --- | --- |
| `00696120..006962B1` | Complete ordinary schedule, all143 instructions reviewed by primary | ECX first endpoint; stack event/value; `RET8` at006962AE |
| `00696350..0069635E` | Complete five-instruction forwarding wrapper | ECX first, EDX event, stack value; `RET4` at0069635B |
| `006E6490..006E65B5` | Only expiry/release caller adoption at006E6598..006E65AE; existing arithmetic/source projections unchanged | ECX tick node, stack step, original`RET4`; no whole-native-tick compatibility claim |

Live Ghidra body/prototype and instruction-context reads verify these bounds
in `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Primary independently
reviewed143+5 instructions and the caller sequence before approving this packet.
No worker definition, annotation, listing repair, export mutation or ledger edit.
No new disk/live hash or original-body differential claim for this packet.

## Shared storage and schedule

`ObserverEventWordCallbackAccess` and `ObserverEventWordDeliveryContext` are
separate types. The existing `ObserverEndpointCallbackAccess` and slot04/08
delivery aggregate remain unchanged. Each event-word context borrows the same
`NativeObserverLifetime`, actual`00E198E4` publication cell, and required actual
slot0C callback provider. No alternate list, endpoint or lifetime domain exists.

`dispatch_observer_event_words_00696120` reuses the primary-confirmed equivalent
ordinary `dispatch_observer_edges_00695f90` schedule: acquire/capture the actual
recursive section; save dispatch-vector size; append the endpoint's edge
sequence; capture the resulting index interval; skip pending nulled entries;
reload current publication after callbacks; restore saved size; release the
captured section. Existing checked insertion/resize-to-saved-size library reuse
and native-valid storage limits are documented in `OBSERVER_DISPATCH_SCHEDULE.md`.
No endpoint/edge retain or release is added. Nested dispatch uses the same
recursive section and shared interval discipline.

For each surviving edge,0069625B reads its current callback owner at edge+8;
00696262/64 read that owner's current table and slot+0C. At00696267/68/6D the
native pushes value, event, original first, then calls at0069626E. Thus the
required protocol is ECX callback owner, stack `(first,event,value)`; the callee
must consume12 bytes. This establishes the call protocol, not recovery of every
individual callback target or a claim that each body's RET0C was inspected.
Callback EAX is ignored. The source provider receives those exact identities,
captured current table and raw32 event/value through a new C++ interface.

The event is read from native private stack+40 at delivery; value is captured
in EBP before the callback loop. Source parameters are captured by value. Native
private-stack mutation/aliasing is excluded from this ordinary source domain.
Original FH3/SEH, hardware faults, malformed vector/returning validation-handler
mutations, allocator failure and exceptional-state equivalence are not claimed.
The existing C++ guard releases its captured section on a provider exception
without trimming the shared vector; this is source cleanup, not a new native
exception transport implementation.

## Connected caller and required services

The existing source caller invokes expiry and then release(code2). Its opt-in
adapter keeps only borrowed view/context/service references:

| Native observation | Source operation | Required contract |
| --- | --- | --- |
|006E6598 reads `[tick node-8]`|Final expiry reads `projectile_minus_08` once|Actual stable node pointer cell, nonnull live entity identity |
|006E659B/659D/659F supplies value0/event0 and calls00696350|PURE `observed_prefix(identity)` then notification(0,0)|Returned prefix address must equal original entity identity exactly; no field read, callback, native call, allocation or translation |
|006E65A4 independently reads SAME cell|Final release independently reloads `projectile_minus_08`|No first-identity snapshot reused across notification |
|006E65A7/65A9 supplies code2 and calls00926D90|Required complete release provider(identity,code)|Current identity/code preserved; actual flags, children, pending list, lock and virtual/world services |

Other `ProjectileTickHost` methods remain abstract. The adapter does not assign
native profiles, create projectile/observer storage, provide tick/pose/world
defaults, change the expiry gate or register itself in GameUnitsHost.
The actual node/cell and notification's first/callback prefixes must stay live
through delivery; the release identity must satisfy its own complete callee
domain. Changing a caller field in a fixture does not prove runtime reentry.

Existing `native_pending_entity_kill_00926d90` in
`native_pending_entity_producers.hpp/.cpp` can supply the release body when bound
to its SAME actual entity fields, pending owners, lock, children and current
virtual providers. This adapter does not manufacture those views/contexts or
replace that body with direct/synchronous deletion. Original00926D90 is ECX
entity, stack raw cause, RET4; source provider remains a separate C++ ABI.

`00923BD0` is a CALL inside `00923B80..00923BDB`, not a function entry. That
caller writes party/race, traverses children and delivers event6; current host
SetParty projections omit that delivery. They are not reused here. AirOps host
projections also cannot substitute for actual observer prefixes/message tails.

## Focused validation

Fresh Win32 actual `observer_event_producer.cpp` and `tick_element_overrides.cpp`
objects compiled with MSVC19.51.36244.0/toolset14.51.36231,
`/EHsc /std:c++17 /MD /O2 /Gy /DNDEBUG /W4 /WX /Iinclude`.
The one ignored fixture calls the existing tick source through the new adapter.
It uses live source prefix storage, a real prepublished recursive section and
existing register/unregister functions. A pre-existing shared-vector prefix
forces nonzero saved size. The first callback verifies A/event0/value0/current
table, unregisters the pending second pair, and changes SAME node cell A->B.
The pending callback is suppressed; release receives fresh B/code2 after saved
size and lock depth restore. Remaining registration and cleanup are checked.

Fixture table tags, tick/world observations, callback field changes and release
receipt instrumentation are SOURCE-only. They are not actual native callback
tables, a complete release adapter, native runtime reentry or gameplay evidence.
No new broad tests or nested-dispatch suite; existing nested schedule evidence
remains applicable. The executable links `/MANIFEST:EMBED` and `/OPT:REF`.
Two changed TUs are fresh objects; support libraries are pinned copies of the
previous validated fixture libraries, with hashes recorded in the report.

Generated COFF confirms a cell load before the expiry mapping/notification and
an independent cell load before release. Source code2 is supplied by the
unchanged tick caller. Native direct/indirect call receipts and fixture paths
are in `reports/observer_slot0c_adoption_cc11.json`. Full build/integration is
primary-owned; worker did no full CMake build or game run.

Queue event4/value0 remains a distinct boundary:006C0B50's complete lower-
constructor adoption, message84/session routing and holder lifetimes are still
required. Its cached task404, fresh input plane9D4 queue receiver, stored record+4
and fresh returned block80 are not changed by this packet. The previous land
parameter-copy kernel/adoption qualifications remain unchanged.

Primary integration 56d73adbdd0bf299193a14e3a0f6f505e3db329b: full MSVC Win32 build and all three existing CTests passed. The focused fixture was independently reviewed and compiled from fresh actual-main translation units, with 3 main link inputs hashed before and after linking. Win32 manifest type24/id1/asInvoker verified. Executable SHA256 c112c47034d76f9be29531829fe79ebd22bf2e6e0c452e50c4bee45cba737858. Probe local/cc11_slot0c_root.cmd; build log local/cc11_observer_start_speed_integrated_build.log. Source-only fixture, required providers, actual lifetimes, original full ABI and game validation qualifications remain.
