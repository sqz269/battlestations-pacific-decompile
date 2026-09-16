# Raw frame-clock lifecycle

R36 reconstructs BEDE00 (145 bytes), BEDEA0 (153), BEDFB0 (156) and BEE110
(36). The caller supplies the genuine 80h allocation and a persistent borrowed
context referencing the SAME actual 01090AA0 manager cell, 01090AB0 clock cell,
and R35 D68D50 method-profile context. This creates no process owner or second
clock domain, and binds no application/frame/sound/input/renderer worker.

## Native publication and lock order

The base constructor stamps D68D20, gets the first manager, captures its raw
section +10, enters and increments +18. It publishes self, gets the manager
again, reloads current AB0, registers that pointer, and leaves the captured
first section. It does not register a cached publication or select a new lock
from the second manager.

Base destruction stamps D68D20 and follows the same first-section/current-
manager schedule. It unregisters the current AB0 even when it differs from
self, clears AB0, leaves the captured section, then stamps CE3818 on self.
Current AA0 and its manager must remain alive through this operation and the
shared drain. Guard cleanup uses existing 00411EE0; generic base cleanup uses
00412430. Get/register/unregister use existing complete raw providers.

The derived constructor calls the base before member initialization. Its
positive-zero accumulated float precedes D68D50. All timestamp words follow
the listing's store order, not a blanket memset. QPF sees original +60;
+6A..6F/+70/+78 remain untouched. Paused and fixed become zero before the
existing raw initializer, whose ignored QPC/QPF return values, local preimages,
and two current virtual update lookups remain unchanged. Initialization's
ordinary C++ failure destroys the base; caller retains allocation cleanup.

BEE110 stamps D68D50, destroys the base, then frees iff flags bit0. EAX returns
the captured address with RET4. Its original 36-byte assembly schedule is
preserved, with two calls relocated and an explicit EDX lifecycle context.

## Shared deletion and exception boundary

The current `NativeSingletonDeletionBindings` appends `frame_clock` at +108,
size112. Existing +92/+96/+100/+104 fields and all earlier admissions remain.
Only final D68D50 dispatches to the actual scalar body with the popped owner
and stable context; transient D68D20 is not added. Missing binding retains the
shared source-contract error. Context/manager/publications must outlive drain
and all retained workers. No startup owner is fabricated to populate the field.

BEDE00/BEDEA0 FH3 state0 cleans the root, state1 first cleans the captured guard
then the root. BEDFB0 state0 cleans the base after member initialization.
Ordinary source C++ cleanup mirrors these actions. Original FS exception-chain
frames, mutable stack spill aliases, hardware-fault/SEH and static CRT exception
identity are not binary-compatible claims. The BEE126 returning-free listing
gap was repaired by the parent; the prior receipt remains in the archive.

Exact byte/call/EH receipts, fresh strict build, focused genuine raw manager
registration/drain validation and immutable archive are recorded in
`reports/native_frame_clock_lifetime_r36.json` when validation closes.

## R36 component validation

Fresh strict MSVC Win32 /MD /W4 /WX /fp:strict build and all three existing
CTests pass. One ignored manifested probe constructs actual80h clocks through
the real QPC/QPF initializer, registers them in real raw14h managers, and drains
through the exact shared deletion entry. Its CRT-free observer forwards to the
real imported free and verifies root stamping, cleared AB0, released guard and
live manager at clock free. Eight source/copied-native scalar calls cover low
flags bit0 with 0/1/100h/101h; compiled scalar36 matches after just two rel32
relocations. There is no native-copy proof of the two C++ base bodies or ctor.

Two scoped Win32-import observers forward actual Enter/LeaveCriticalSection,
changing the actual publication values during entry to verify captured first
section versus current second manager/current clock reloads. A fixture-only
installed SDK invalid-parameter handler throws a real C++ exception during
registration: cleanup releases the guard and stamps root, while retaining the
published allocation and leaving derived fields untouched. This is an ordinary
source-exception check, not original FH3/SEH or init-failure execution proof.
All observers are restored; no production callback or owner binding is added.
