# Native tracer derived lifetime

Addresses: `00BAC130`, `00BAC370`, `00BAC860`, `00BAC880`, `00BAC970`,
`00BACB90`. Names are hypotheses. These are new MSVC Win32 C++ interfaces,
not original object/vtable/EH ABI replacements.

| Inclusive native range | Original ABI | Coverage |
| --- | --- | --- |
| BAC130..BAC18E | ECX actual pointer-array header; signed capacity stack; RET4 | complete |
| BAC370..BAC3BF | ECX actual pointer-array header; signed count stack; RET4 | complete |
| BAC860..BAC876 | ECX actual point-array header; RET | complete |
| BAC880..BAC896 | ECX actual pointer-array header; RET | complete |
| BAC970..BACA7D | ECX actual7ACh tracer payload; RET | complete |
| BACB90..BACBAF | ECX actual tracer; flags stack; EAX original slot; RET4 | complete |

## Canonical owner and current profiles

`NativeTracerProfileBindings` borrows the existing `NativeModelOwner` over the
actual7B0h tracer pool slot. Its node prefix, model tail, transform, scene
association and native+04 atomic are the same objects already used by node,
generated-model and render operations. It creates no native storage, count,
hierarchy, scene registry, allocator or ownership map. Binding checks the stated
extent and requires the actual0109049C pool and a live23-word D63FA0 table view.

The constructor worker calls `publish_native_tracer_profile_00bad6f0` at the
native BAD780 publication. It sets native D63FA0 and installs current-profile
adapters on the SAME scene attachment. D63FA0 reads prove slot00=BD30E0,
slot04=BACB90, slot0C=6EF860, slot18=B6F310, slot40=B6DBE0, slot50=B6ED80 and
slot54=B6EE10. Type matching uses the existing initialized model descriptor;
scene/hierarchy operations reuse their established concrete implementations.
Unknown tables or changed target entries have no successful default.

`restore_native_tracer_model_profile` restores the original host callbacks and
their `NativeModelOwner*` context BEFORE B750C0. This is necessary because the
existing model-phase publication replaces callbacks but does not reset their
context. Restoration makes no native stores: B750C0 publishes its own model
profile, and B6F440 publishes node phase. The constructor must also restore
before exceptional base cleanup. `retire_failed_native_tracer_profile` only
restores host dispatch; profile binding registers no lifetime to dispose.

Only after successful BAD6F0 construction, `NativeTracerReference` binds the
same `GeneratedModelLifetimeRuntime` and borrows actual+04 without changing it.
It replaces the ordinary `NativeModelReference` for this one owner. Runtime
duplicate-identity checks reject concurrent references over the same node.
The existing caller-owned `NativeRenderActualOwners` resolver must associate
the raw slot with this reference, just as for other concrete native owners.
The supplied retirement callback removes that association and may destroy the
host companions after destruction, physical return and runtime unbinding.

The shared B6F310 logical release and point-light views read the actual fields.
Zero-reference dispatch requires CURRENT BD30E0/BACB90, performs the full
derived destructor, and returns the same slot through concrete BABF70 and the
actual0109049C pool. It never calls ordinary model scalar deletion B75290 or
returns a tracer to the188h model pool. The retained model base remains useful
as the canonical prefix/lifetime component; it does not determine derived
physical allocation size.

## Destruction order and recovered continuation

BAC970 first publishes D63FA0. It captures CURRENT248, decrements that raw
owner's actual+04 and dispatches its current virtual0 only at zero, then clears
248 after the callback. It repeats for the then-current1BC and190. Thus a
terminal callback can replace a later field and affect the subsequent release.
The required actual-owner resolver checks that every resolved companion borrows
the exact raw+04 atomic; missing owners do not silently succeed.

The current optional254 child resolves through the SAME native node lifetime
runtime. Concrete B6DFA0 unlinks its actual hierarchy and invokes its current
virtual18;254 is cleared after that complete callback. Child owners must have
their real canonical lifetime/profile bindings already installed.

The destructor then lowers EH state to1, resizes actual+1A0 to0, and frees its
CURRENT backing. The incorrectly omitted BACA3C..BACA7D continuation lowers
state to0, resizes actual+194 to0 through the constructor worker's concrete
BAC310, frees its CURRENT backing, lowers state to-1, calls full canonical
B750C0, restores FS:[0] and returns. Both frees use ADD ESP,4; neither resets
the stale header pointer or capacity. There is no physical pool return until
successful scalar destruction reaches BACB90's flags&1 branch.

DFD5A8 is the FH3 descriptor, with three entries at DFD590:

| Active state | Next | Actual unwind action |
| --- | --- | --- |
| 2 | 1 | CC3D96 -> BAC880 at actual+1A0 |
| 1 | 0 | CC3D88 -> BAC860 at actual+194 |
| 0 | -1 | CC3D80 -> B750C0 on the same model prefix |

The implementation retains that cleanup order for C++ exceptions, including
not retrying a normal member whose state was already lowered, and not retrying
a failing base. A cleanup exception during another exception terminates.
MSVC FH3 metadata, SEH dispatch and asynchronous faults are not reproduced by
the new C++ interface and were not executed by this fixture.

## Array producers and leaf behavior

BAD78D..BAD7A2 initializes both0Ch headers to zero. Actual+194 owns30h point
records; its full BAC310/BAC070 implementation and record layout are in
`native_tracer_construction.hpp`. Actual+1A0 owns pointer words, with signed
count1A4/capacity1A8. BAC730 fills those cells with existing linked-point
identities; this pointer array neither retains nor destroys the points.

BAC130 clamps requested capacity to at least1, compares signed capacity,
allocates wrapped capacity*4, copies live count using reloaded backing, frees
the then-current old backing, publishes the replacement and captured capacity,
and preserves count. BAC181..BAC189 was absent from the saved flow despite its
later RET4 being defined. BAC370 performs the signed reserve test, initializes
new cells to null with the native zero-address guard, shrinks count without
touching removed cells, then publishes requested count. Both complete growth
and shrink paths are exposed. Valid actual backing and native arithmetic
preconditions remain required; no new rollback or validation is inserted.

## Verification and integration limits

Nine bounded live Ghidra spans were verified against the installed disk PE:
six complete routines, FH3 thunks, unwind map/descriptor and the current tracer
table. Every live request verified project `bsp`, program
`/battlestationspacific.exe` via `bsp.py ghidra`; no Ghidra mutation was made.
The report stores spans, lengths, hashes, call sites and original boundaries.

Strict MSVC Win32 `/W4 /WX /EHsc /fp:strict /O2` compilation passed. One focused
original-byte fixture executed BAC130/BAC370/BAC880, with verified relative
calls rebased to each other and the same existing CRT allocation/free boundary.
It passed grow/copy/null-cell, shrink, and returning-free metadata comparisons.
The fixture also compiled the constructor worker's concrete source and linked
the existing core library; every reached array operation was concrete.

The standard `scripts/build.ps1` was attempted, but the fresh worktree build
filled drive J: and failed MSB6003. Its generated build tree was removed after
checking the absolute target and absence of reparse points, recovering space.
The strict compilation, focused fixture and evidence logs remain under
`local/tracer_lifetime_*`. Parent integration must register this source and run
the combined build/existing checks; no CMake or metadata file was changed here.

Parent saved-analysis repairs remain required at BAC130 gap181..189,
BAC860 tail872..876, BAC880 tail892..896 and BAC970 tailBACA3C..BACA7D.
The report's complete numeric call rows intentionally include the newly
recovered calls so live call verification exposes unrepaired saved boundaries.
Full derived-reference teardown, populated retained-owner/child callbacks,
exception trajectories, successful complete tracer construction, renderer
composition and gameplay have not been dynamically validated by this packet.

## AL saved-analysis and combined-build integration

All four AL modules are registered in bsp_core. The standard Win32 build and
both seeded CTests passed. The report records saved name/signature preimages,
old-comment preservation, full body-range readback and the current-library
replay of bounded fixtures. Returning-free gaps and missing function definitions
are repaired and saved. Earlier worker pending notes describe their original
snapshot. Full constructor/teardown coverage, native EH compatibility and
gameplay remain bounded as documented above.
