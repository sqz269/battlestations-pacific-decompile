# Observer endpoint notification

Packet `orch6_observer_event_producer_r` resolves the argument previously called
an event in `NativeShipAiObstacleEventAccess`. It is the original observed
endpoint pointer, delivered unchanged to each callback owner. No event record
is allocated, copied or initialized on this path. The getter invoked by the
node callback remains producer-specific; replacing every getter with identity
would be incorrect.

R adds complete notification selectors, callback-slot trampolines, conditional
notification wrappers and the two verified getter bodies. It reuses existing
`NativeObserverOwnerStorage`, `NativeObserverLifetime`, and the separately
reconstructed `dispatch_observer_edges_00695f90`. It does not edit the node,
observer lifetime, dispatch scheduler, singleton deletion or GameHosts modules.
Initial base was `0f074d19`; root dependency `bfef3a70` was merged before C++
work to obtain the reviewed raw node and actual dispatch owner.

## The producer and delivery chain

1. Scene/entity constructor00925CE0 initializes the first observer prefix:
   tableCECCC8 at00925CFF, then data/count/capacity zero at00925D0A/0D/10.
   Its separate callback-owner prefix begins at+10, initialized at00925D13..20.
   It subsequently installs primary tableD19120 at00925D44. These are distinct
   embedded endpoint bases, despite their shared10h prefix type.
2. Game-entity constructor00928630 calls that base constructor at00928651,
   then installs primary tableD192E0 at00928662. Existing unit constructors
   later install their leaf tables. This packet independently checked all21
   primary slot+4 words from the existing `unit_world_registration.cpp`
   producer map: every one points to0042B970. Full constructors are not newly
   implemented or fixture-validated here.
3. Scene removal009263C0 tests byte+5E, writes+5D/+5E/+5F=1 and+5C=0, then
   calls00925C40 at00926401 before its primary+80 tail call. The existing
   queue drain009273A0 performs the same flag stores and inlines the count
   sample before00696330 at00927510. ESI is the actual entity from queue
   node+8, preserved across the intervening calls.
4. 00925C40 captures the shared observer section, samples unsigned count+8>0
   under that section, releases it, and tail-jumps00696330 only when the saved
   predicate is true. Dispatch performs a fresh lock lookup; it does not reuse
   the sampling acquisition.
5. 00696330 loads EDX=00693550 and tail-jumps00695F90 with the same ECX first
   endpoint. The dispatcher saves that ECX, appends the endpoint's actual edge
   sequence to the existingE198E4 vector, and invokes its selected callback
   with ECX=original first and EDX=current edge+8 callback owner at006960D1.
6. 00693550 reads the callback owner's table, pushes the original first pointer,
   adjusts ECX to the callback owner, and calls table+4. For tableCF5C94 that
   slot is the already reconstructed0064B5C0. Thus its stack argument is the
   same first endpoint that009E52E0 registered at009E532D.
7. 0064B5C0 calls the delivered endpoint's own slot+4 before comparing its
   result with a fresh node+14 load. Equality clears node+14 before actual
   unregistration. It does not merely clear the field on every notification.

The other selector00696340 loads00693560, which dispatches callback-owner
slot+8. Its conditional wrapper00925C90 is called at009263A1, after
00926390's+5D/+60 stores and before its primary+7C tail call. This is a
separate slot path. In the actual nodeCF5C94 table, slot+8 is0042B120, the
verified `RET4` body; it does not execute0064B5C0 or clear node+14.

## Actual endpoint getter targets

| Producer / current table | Slot+4 target | Complete body / result |
|---|---|---|
| Scene/entity D19120, game entity D192E0 | 0042B970 | `MOV EAX,ECX; RET`, returns the actual receiver unchanged |
| All21 current unit leaf primary tables | 0042B970 | Same identity body; live words agree with installed image |
| 00A2D440 stores D23084 at00A2D464 before notification00A2D4AE | 00522E90 | `XOR EAX,EAX; RET`, returns null regardless of receiver |

00A2D440 is another direct00696330 caller. The inspected complete body writes
its current primary and+10 tables, samples the first-endpoint observer count,
notifies, then performs its remaining object/registry/base cleanup. Its class
name and full owning constructor are not inferred here. Its explicit table
store is enough to establish the null getter for that call. Both getter bodies
are implemented literally; the caller must resolve its actual table/target.

The source retains the historical Q `event` label only when referencing that
existing interface. R names its argument `first`. A semantic unit pointer is
not automatically an actual observer prefix. A process adaptation must retain
an explicit alias to the same canonical endpoint used during registration;
the delivered and returned identities must be that endpoint, not a fabricated
payload or a separately allocated replacement owner.

## Coverage and ABI

| Entry | Inclusive native span | Coverage / original ABI |
|---|---|---|
| 00925C40 | 00925C40..00925C83 | Complete normal conditional slot04 wrapper; ECX=first, RET or tail jump |
| 00925C90 | 00925C90..00925CD3 | Complete normal conditional slot08 wrapper; same ABI |
| 00696330 | 00696330..00696339 | Complete ten-byte slot04 selector; ECX=first, EDX overwritten with00693550, tail JMP00695F90 |
| 00696340 | 00696340..00696349 | Complete ten-byte slot08 selector; EDX=00693560 |
| 00693550 | 00693550..0069355A | Complete11-byte trampoline; ECX=first, EDX=callback owner, pushes one pointer, indirect callback consumes it, plain RET |
| 00693560 | 00693560..0069356A | Complete11-byte slot08 twin |
| 0042B970 | 0042B970..0042B972 | Complete identity getter; ECX input, EAX output, plain RET |
| 00522E90 | 00522E90..00522E92 | Complete null getter; ECX unconsumed, EAX=0, plain RET |
| 00925C45 | 00925C45..00925C71 | Partial public sampling projection only; enclosing function00925C40's prologue and dispatch/return tail are not part of this helper |

The sampling helper also represents the identical inlined block
009274DE..00927509. The native unsigned `SETA` test is retained: high-bit
counts are not treated as signed-negative. Native count/capacity validity
remains required before dispatch; this is not a new corruption guard.

Native callback EAX is unconsumed by00695F90. The source callback interface
therefore returns void. It forwards the actual first and callback identities
and the captured callback-owner table word, letting the provider dispatch the
real slot target. It never synthesizes a complete C++ method table from those
identity words.

R's C++ wrappers are not native ABI replacements. The short native wrappers
contain no FH3 registration. Exceptions in the shared dispatcher follow that
module's documented lock cleanup; neither wrappers nor providers trim its
vector or perform additional endpoint cleanup on failure. Native hardware
faults, original exception transport and arbitrary virtual targets remain
outside this new interface.

Root formally defined and saved the four previously missing11/3-byte bodies
and refreshed their exports. Their live bytes match the installed image; the
report cites `observer_event_function_definitions.json`. R made no Ghidra
mutations and preserves existing names where appropriate. No missing-function
rows remain for this packet's owned entries.

## Concrete integration boundary

`ObserverEventDeliveryContext` borrows the existing observer lifetime, actual
E198E4 publication cell and callback-slot providers. The dispatch owner must
already have been published through00CCD6A0. Registration, notification,
unregistration and destruction must use those same owners, which must outlive
all active callbacks and scans. The context introduces no owner or global.

The existing `EntityEventQueueHost::sample_has_observers_00925c40` can call
`sample_observer_endpoint_presence_00925c45` on its actual endpoint alias.
Its following `notify_observers_00696330` should call the direct slot04
selector. Calling the full conditional wrapper there would incorrectly
sample the count twice. Immediate009263C0-style removal can use the complete
conditional00925C40 wrapper. No GameHosts binding is included in R.

For an actualCF5C94 node, bind callback slot04 to the real Q node callback and
supply its existing getter access using the delivered endpoint's current
producer-backed slot04 target. Slot08 remains its distinct verified target.
Unknown endpoint/callback tables require their own implementation; identity
or no-op fallbacks are not supplied.

## Focused verification

One ignored paired fixture retains all 184 original bytes and executes the
eight R bodies plus the shared three-byte node slot08 target 0042B120. Its 12
relocations cover two lock-getter calls, four real Win32 Enter/Leave imports,
four tail jumps and two callback-address immediates. Trampolines execute their
actual indirect calls. Normal native FS state is checked after the chain.
This is routine coverage, not a claim that every byte or branch executes.

The native selector's core relocation enters an adapter that calls the same
recovered source scheduler used on the source side. Only the eight R bodies
are native/source alternatives in this fixture. The shared scheduler, Q node
callback and lifetime operations are dependencies; this probe does not
independently establish their equivalence to the original algorithms.

The final dependency uses the scheduler's private size-restoration adapter
and existing canonical count insertion. It introduces no standalone STL
resize/erase implementation. Its guarantee requires valid vector storage;
returning validation handlers that mutate that storage are outside coverage.

Both sides use the recovered dispatcher, Q node constructor/callback/lifetime,
actual edge arrays and locks, and actual14h dispatch owner plus00CCD6A0
publication. A single existing semantic fixture lifetime domain bootstraps
those real allocations and explicitly dispatches their proven deleting
destructors. This is not a second application runtime domain or independent
verification of shared observer/CRT algorithms.

Fixture endpoints are explicitly supplied10h prefixes with producer-backed
primary identity words; they are not full reconstructed units or the other
large notifier. Callback owners are actual90h nodes. Their table identities
are relocated to a callable fixture table containing the exercised04/08
targets; unused slots are not a complete vtable claim. Slot04 uses the existing
Q callback; its getter provider reads the current endpoint table and executes
the selected original R getter or its new source body. Slot08 uses the same
original0042B120 bytes on both sides.

The cases cover normal slot04 delivery, recursive conditional notification,
the null getter, the distinct slot08 path and an empty sampled endpoint.
Nested callbacks retain the outer dispatcher lock while each conditional
sample releases its own acquisition before nested dispatch. Shared cleanup
invalidates queued edge entries and each normal dispatch restores its captured
vector interval. Nodes/edges are destroyed before the actual singleton owners;
the native danglingE198E4 alias is not reused after shutdown.

Ordered callback/getter events record canonical node identities, original
first identity checks, live endpoint counts, dispatch occupancy and actual
section depth/RecursionCount. Final node owner/lifetime/flags and every83
constructor-untouched bytes are checked. This is selected-state and ordering
proof, not whole-unit/heap-byte equivalence. OS-private lock bytes, allocation
history, vector capacity tails and unrelated node fields are not compared.
No game run, arbitrary callback/EH/OOM/concurrency behavior or original whole
unit construction is claimed. Exact results, compiled dependency hashes and
the retained three Release libraries are recorded in the report/manifest.

The resumed MSVC Win32 Release build and both existing CTests passed. Five
paired scenarios produced 913 identical recorded words per side, with five
native-selector core-adapter entries, 12 getter calls and zero mismatches.
`verify_report_calls.py` checked 16 numeric call rows with zero failures; four
of those are explicitly resolved indirect calls, which the tool cannot prove
as targets. Seven additional symbolic indirect/import rows are reported as
skipped. Their targets and stack behavior are supported separately by the
retained listings, live/installed table words and exercised native bytes.

`local/event_proof_manifest_final.json` preserves the probe source, executables,
objects, original bytes, records, scripts and build logs, including the
interrupted build and the report-generator syntax failure. It retains all
nine distinct archives searched by the verbose link: the three explicitly
passed Release libraries and six MSVC/SDK libraries. Search participation
does not mean every library contributed an object. The peer schedule source
and header used for this build are copied only under ignored
`local/event_dependency`; a clean integrated build requires the separately
owned schedule packet.

The earlier dependency build and probe remain intact at
`local/event_proof_manifest.json`. The final manifest records the reviewed
peer source/header hashes and refreshed libraries, build and probe results.
