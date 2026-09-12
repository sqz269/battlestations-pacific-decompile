# Native particle emission state and population lock

Addresses: `00B0CA40`, `0072B740`, `0072A4F0`, `00729420`, `0072CC90`.
Evidence: `reports/native_particle_emission_state.json`; original project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
All descriptive names are hypotheses, not recovered symbols.

## Reconstructed behavior

`B0CA40` initializes the actual `6Ch` state. Original ECX is the state and its
six stack words are particle definition, unused time, emitter, position,
direction and the `108h` record; final `RET18`. The C++ entry adds borrowed EDX
bindings while retaining those six words. `B04C80` now calls this body directly.

The prefix writes emitter68, definition64, null light60 and zero40, then makes
the original sequential x87 position/current-position/direction copies. It
loads all three record24/28/2C words with MOVSS before storing the vector.
These operations retain forward overlap and floating-point spill behavior.
It reloads definition64 and captures its current vtable18 target before dispatch.

This is the **particle definition** produced by the B00CE0 family. The emitter
cone/sphere/smartarea profiles have only six slots through14 followed by strings;
interpreting their following bytes as virtual18 would dispatch invalid data.
The application must supply the real captured particle target on the same
definition, state and record. No successful placeholder initializer is supplied.

Absent current light60 returns. Otherwise the current emitter68 and captured
definition64 select model8 and radius8C. The existing actual PointLight volume
producer performs absolute or relative position writes and COMISS/JA minimum
clamping. Relative world refresh can change light60, so its current slot is
retained by reference. It uses the original position pointer after virtual18,
the same model transform and current minimum D7A238.

After volume initialization, the real shared population-lock getter returns its
8-byte owner. Its section4 is captured once; native EnterCriticalSection and the
physical DWORD18 depth increment bracket the current emitter68/model8/rootA4 and
light60 reloads and the B7B090 population path. Normal and hosted exceptional exits
release that captured section. Native linked lights skip the root list for every
nonzero backlink count. A primary review caught that forming `*roots` before
this check would be undefined for a native-safe null-root case. The caller now
checks the physical count under the same lock before forming a reference; the
existing concrete B7B090 entry then rechecks it without an intervening callback.

## Shared lock lifetime

`72B740` reads actual publication0108FF50. The fast path returns the first captured
value. The slow path gets the actual raw singleton manager01090AA0 through415350,
captures manager10, enters/increments that physical section, and rechecks the lock
publication. It allocates8, calls72A4F0, publishes the result, looks up the manager
again, reloads the publication and registers viaBD0C30. Its slow return rereads
publication after releasing the captured manager section. The implementation
uses existing native raw manager, registration, allocator and critical-section
services; no private singleton domain is introduced.

`72A4F0` storesCFDEB4 and the result ofBD1860 in owner4. Constructor cleanup invokes
729420: publication is cleared **before** storing theCE3818 base profile. The old
section word is not overwritten if construction throws before assignment.

`72CC90` setsCFDEB4, releases and clears the actual section4 via41CC80, clears
publication, installsCE3818 and frees the same owner only for flags bit0. EAX
remains the captured owner. Native free fall-through72CCBE..72CCC0 contains
`ADD ESP,4`; the final return is72CCC4 `RET4`.

CFDEB4 is a **one-entry** table containing72CC90. The next bytes spell
`barrelNum`; they are not additional virtual slots. Mixed-owner raw singleton
manager teardown still needs its real dispatch for this owner profile.

## Evidence and limits

Five complete spans are checked against both live Ghidra and installed PE bytes.
The standard Win32 build and two existing seeded tests passed before the small
null-root review correction. Final combined and native-byte validation is
recorded in the accompanying report when complete. No game execution is claimed.

The initializer borrows canonical PointLight/Model bindings, including the same
physical link arrays and transforms. Its owner resolver is required because the
scene attachment context is a retained-scene binding, not the PointLight owner.
The current point-light provider determines supported world-sphere paths.

Cleanup funclets C855C0, C85600, C85608 and CBBBC0 and dispatchers C855C8, C85613,
CBBBC8 are analyzed evidence, not extra reconstructed application functions.
Native FH3 dispatchers tail-jump the existing CRT handlerBF6B43. Hosted C++ cleanup
does not establish the original exception-register ABI, mutable EH-spill aliasing,
hardware-fault cleanup or gameplay parity.

## Follow-up packets

Recover the actual B00CE0 particle-definition family virtual18 and compose its
initialization with this state and the existing PointLight owner. Separately bind
CFDEB4 scalar deletion into actual mixed-owner singleton teardown after checking
that manager's current ownership and dispatch contract.

## AO combined integration

The combined strict Win32 build and both seeded CTests passed. Four focused
original-byte probes were linked only to the current combined production
libraries. The batch contains45 new reconstructed bodies and the directB04C80
state call,46 native signatures and8 analyzed cleanup/static bodies. All54
names/comments were saved and read back, preserving prior comments; exports
were refreshed. Five reports verify484 direct call/tail rows with no failures.
The accompanying report records per-probe coverage and exact hashes. Native
FH3, full application composition, concurrency and gameplay are unvalidated.

## Correction from docs/NATIVE_PARTICLE_TYPE_STATE_DISPATCH.md (AR)

Optional same-domain state binding directly invokes all five captured current virtual18 initializers; preserves post-call actual state/definition/emitter/light reloads. Required real resource/renderer overrides, native FH3 and gameplay remain unvalidated.

Earlier isolated dispatcher captures remain historical evidence; the optional
composition does not establish complete application wiring or gameplay.
