# Registered particle-model event

Addresses: `008742A0`, `00874430`, `0086B1C0`, `008745F0`.

The type1 event owns a20h allocation and registers its raw address in the live
effect manager's auxiliary array. It selects a Particle definition variant,
constructs its2DCh particle model, attaches that model to the supplied point
instance's node, copies the point's cached matrix into model+298, then registers.
The event destructor unregisters and restores base tables. It never destroys,
releases, stops, clears, or detaches the model. Descriptive names are hypotheses.

The C++ bodies are complete through explicitly required application callees;
successful model construction/application execution remains unvalidated. There
is no substitute model constructor or pool, guessed success result, duplicate
node registry, or independent event reference count. Root integration supplies
the canonical `live_effect_event_registry.hpp` implementation and build entry.

| Routine | Inclusive body | Original ABI | Coverage |
| --- | --- | --- | --- |
| constructor | 008742A0..0087442E,399 bytes | ECX event; stack definition,point; EAX event; RET8 | complete ordinary body and states0/1 through required callees |
| destructor | 00874430..0087449A,107 bytes | ECX event; RET | complete including base unwind |
| factory | 0086B1C0..0086B230,113 bytes | ECX definition; stack point; EAX event/null; RET4 | complete through required current gate getter/model constructor |
| scalar destructor | 008745F0..0087460D,30 bytes | ECX event; stack flags; EAX original address; RET4 | complete; bit0 frees20h event only |

The native SEH/vtable ABI and invalid memory fault behavior are not reproduced.
Empty variant counts and null completed models are rejected by the new C++
interface. It requires a bound D5DA50 current table whose+38 is B6DB10. Other
current-table implementations require their real dispatch before use.

## Storage producers and argument provenance

`NativeRegisteredEffectPrefixStorage` supplies the existing common1Ch prefix.
The constructor writes primary tables CEB130 then D0DE18, ref04=1, secondary
tables D0C8C0 then D0DE14, active0C=1, point10, definition14, and type18=1.
Only the final model1C publication follows model construction. Bytes0D..0F retain
the allocator preimage; none of the borrowed fields is retained.

The first argument is the actual34h Particle component produced by the existing
`construct_effect_particle_0086bc80`, whose final table is D0D5B4 and whose
28/2C/30 array descriptor is initially zero. Reader871D00 resolves `Particle`
names and appends raw variant owners into28, incrementing2C. These are distinct
from the point instance's unrelated fields28/2C; no alternative point layout is
declared here. The point is the existing `PointEffectInstanceStorage`, including
its canonical node110 binding and cached90 matrix.

Full8742A0 assembly was inspected. At8742BB ESP is incomingESP-94, so9C loads
the second stack argument into EAX. At8742CC ESP is incomingESP-A0, soA4 loads
the first into ESI. At8743CB ESP is incomingESP-A4, soAC reloads the second into
ESI. EBP is the event throughout; EDI captures definition count before random;
EBX remains1. Random runs with ECX=0, then unsigned DIV EDI selects the remainder.
The variant array pointer is reloaded after random; the selected pointer is
captured before allocation. These timings are preserved.

After model virtual38, the model1C field is reloaded for parenting. It is reloaded
again after parenting for the custom298 matrix destination. The cached90 source
is first snapshotted as16 DWORDs, then copied by canonical x87 `004134F0`.

## Required and concrete contracts

| Site | Native dependency | Contract/evidence | Implementation |
| --- | --- | --- | --- |
| 874313 | BD2FC0 | primary stream ECX0, no stack words | same `RandomThreads` |
| 874327 | AF6B60 -> AF69E0 | raw slot from F8D2D0; thunk overwrites incomingECX=2DC | required actual pool |
| 87433E | AF74A0 | ECX rawslot, stack selected variant, RET4; returns same slot | required actual particle-model constructor |
| 8743C9 | current model virtual38 | D5DA50+38=B6DB10; identity local matrix, RET4 | canonical transform over existing scene binding |
| 8743DC | B6E680 | current model child, point.node110 parent, RET4 | canonical native parenting runtime |
| 874400 | 4134F0 | model298 destination, captured point90 source, RET4 | canonical x87 matrix copy |
| 874406,874463 | 4D1100 | live F8765C getter from same singleton domain | canonical getter |
| 87440D | 866A10 | current manager and raw event, RET4 | canonical registry under F87654 lock |
| 87446A | 866B00 | same raw event/manager/domain/lock, RET4 | canonical unregister |
| 874486 | BD30F0 | restore primaryCEB130 only | exact base store after base phase |
| 86B1D9 | 51F6B0 | getter of current8h F8C27C owner; factory reads byte04 afterwards | required real getter |
| 86B1E6 | BF681B | one20h size; ADD ESP,4 at86B1EB | canonical allocation boundary |
| 86B206 | 8742A0 | captured ECX definition plus stack point, RET8 | complete constructor above |
| 8745F3 | 874430 | ECX preserved in ESI, no stack arguments | complete destructor above |
| 874600 | BF65AC | event address, bit0 only; ADD ESP,4 at874605 | canonical free boundary |

AF6B60 is misleadingly labelled `CG_static_dtor_stub` in current Ghidra. Its
two-instruction body sets ECX=F8D2D0 and jumps to AF69E0. That locked allocator
uses2E0h slot stride and a2DCh trailing pool ID. AF74A0 is a large derived model
constructor: it calls the188h generated-model constructor first, then creates
mesh/material/stream/emitter state and retains its selected variant at18C.
Reconstructing that base alone does not implement AF74A0. The required callee
must construct actual model storage, establish its real node/scene bindings,
and expose the same typed matrix storage at298 before returning.

The saved scalar-destructor listing omits874605 after the incorrectly
nonreturning `_free` call. Disk and live bytes both contain `83 C4 04` there,
followed by the preserved-address return at874608. The C++ implementation
preserves that continuation; no worker Ghidra flow mutation was made.

The only direct8742A0 caller is86B206. The factory appears in Particle table
D0D5B4 at+18 (D0D5CC). The destructor's only direct caller is scalar8745F3.
All source-grounded callee bodies listed above were inspected; the large
AF74A0 dependency was read as a contract boundary, not reconstructed here.

## Failure and ownership boundaries

Native EH descriptorDC842C has state0 -> C96360 ->858150 and state1 ->
C96368 ->AF62F0, then state0. AF62F0 pushes rawECX, selects F8D2D0, and
calls AF60B0 to return the slot using its trailing2DC ID. Therefore only an
AF74A0 constructor exception returns that raw slot; AF74A0 must already have
unwound its constructed members. Successful-model work does not arm model
destruction. Exceptions in later local/parent/registry work unwind the event
base only. Destructor EH state0C96380 also dispatches858150. Base858150 writes
D0C88C/D0C888, then BD30F0 writes CEB130 without changing ref04.

Type1 current virtual30 is871FE0, which loads model1C and tails AF5F20.
Raw AF5F20 clears model1A4 and calls AFF570 over the captured194/198 emitter
array. This is a separate stop action, not model disposal. These two function
starts are absent from saved Ghidra; no mutation was made. Evidence ranges are
871FE0..871FE7 (8 bytes) and AF5F20..AF5F50 (49 bytes). Actual particle/node
ownership, emitter lifecycle, scene resource completion, and eventual model
disposal still require the real application bindings. The event destructor
provides no general substitute for those paths.

The live event registry's other constructor stream is type4. Serial point
advance can skip these registered classes because their updates have a
separate manager path; registration does not make their model lifetimes owned
by the registry. This packet does not claim that path is game-validated.

## Verification

The existing bsp.gpr program `/battlestationspacific.exe` was verified with the
CLI before the read/export batch, count63371 matching the snapshot. Assembly,
RET cleanup, current tables, producers, native EH descriptors, and call sites
were checked read-only. No prototype, flow, function, comment, or project-save
mutation was performed by this worker. All four complete body byte spans match
between disk and live Ghidra; report hashes pin them. Eight canonical seeds
also match. The call verifier passes15 direct rows and explicitly skips the
one indirect row; that current table slot was inspected separately.

MSVC Win32 `/std:c++20 /MD /EHsc /W4 /WX` compilation passed. The ignored
`local/model_probe_ai.cpp` linked the real root `bsp_core.lib`, with embedded
manifest. It executes107 original destructor bytes with only direct call
boundaries redirected to the same live registry/base bodies, and compares all
20h output bytes against the reconstruction. Both remove the first event by
last-cell replacement and preserve model1C/ref04/padding; lock recursion ends0.
The30-byte original scalar destructor also matches flags0/2 without ending
the C++ storage lifetime or freeing its allocation. The probe also executes
the113-byte original factory's suppressed branch and checks
the C++ result with byte04=80. Finally, an explicitly failing model-constructor
dependency verifies raw-slot return before base unwind, no model publication,
preserved preimage tail, and no registry insertion. The failure injection does
not pretend to implement any successful particle-model allocation/construction.

Probe result: PASS. No complete model construction, full application execution,
rendering, original ABI substitution, or gameplay was tested. The root owns the
full build/CTest/CMake and final report-call verification integration.
