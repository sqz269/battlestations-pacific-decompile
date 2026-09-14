# Native renderer debug records

Addresses: 00B2BB90, 00B6DB10, 00B6DA30, 00B6DBC0.

`src/native_renderer_debug_records24.cpp` reconstructs the full normal B2BB90 schedule;
`src/native_renderer_debug_records24_leaves.cpp` holds its x87 vertex interior and
raw local-matrix notification closure. These are new
source interfaces over the existing actual renderer/model/geometry/string domains.
They do not establish a callable replacement ABI or game rendering parity.

| Routine | Native span, exclusive end | Coverage |
|---|---|---|
| B2BB90 debug records | B2BB90..B2C275, 1765 bytes | Complete source schedule; active native domain requirements below |
| B6DB10 local matrix | B6DB10..B6DB59, 73 bytes | Complete raw body; current virtual3C target B6DBC0 |
| B6DA30 descendants | B6DA30..B6DA62, 50 bytes | Complete raw recursion and sibling walk |
| B6DBC0 bounds notification | B6DBC0..B6DBD9, 25 bytes | Complete raw body and supported current tail loop |
| B2BF60 vertex interior | B2BF60..B2C143, 123 listed instructions | All 38 vertices; an interior, not a separate original function |

The original B2BB90 receives renderer in ECX and returns with plain RET. Its pcode
float parameter is wrong: EDI preserves renderer, EBP becomes the captured camera,
ESI holds the transient actual model, and EBX remains zero. The new interface adds
EDX context and a stacked persistent frame, returning with RET4. B6DB10 adds the
read-only profile resolver in EDX while preserving the stacked matrix argument.

## Actual storage and ordering

B29270 is the producer: ECX renderer, three stacked arguments sphere-float4,
camera and selector, RET0C. B292B3..B292FD writes six local DWORDs and B29301 calls
B25750 with renderer+1D0C. B25750 copies six words at stride18h and increments the
current count. Thus +1D0C/+1D10/+1D14 is data/count/capacity; each record contains
center XYZ, radius, selector at+10 and camera at+14. No new record owner is created.

An initially zero count returns before bindings are examined. For any other count,
a null renderer+19E4 triggers three actual raw strings (`debugshader.mshd`,
`pf43cc.mvfm`, `DebugSpheres`) and full B4C700 with section-kind3 and three zero
counts. The result is published to+19E4 before normal string destruction. After
those callbacks, the code loads the single original-one word and reloads CURRENT
+19E4, builds the diagonal matrix, verifies its actual model profile+38 and runs
the raw B6DB10 provider on the same canonical model storage.

Each live iteration snapshots the first center word, camera, camera+198 predicate,
then the remaining sphere words. A nonzero predicate skips all model work. An
active record constructs another complete model using the same strings/arguments;
the cached model is not its vertex source. The selector is reloaded from CURRENT
record storage after all model/string callbacks. Every geometry, vertex-stream and
section getter is performed again where the listing performs it.

The current logical vertex D61D6C/+10=B49980 locks 38 vertices. Selector555 produces
colorFF0000FF; other selectors produceFFFF0000. The exact x87 loops emit 13 XY,
12 YZ (indices1..12) and13 XZ vertices at stride10h. Every FILD32, FMUL64 with the
borrowed CEC730 double, float spill, FCOS/FSIN, radius multiply and final store is
retained. The second ring omits index0. There is no library trig approximation.

After a fresh stream getter and current+14=B49A80 unlock, separately fetched section
fields receive primitive-count37 at+18 and vertex-count38 at+10. Full B46A70 runs
with scene null and captured camera. The code captures the actual108FE88 cache,
calls the CURRENT CE221C import borrowed by the concrete pass context, then reads
the captured cache's CURRENT data pointer. Entry is `(increment_result-1)*28h` from
that data. Native FLDZ/FLD1 stores form the entry arguments. Full B51A20 receives
leading0, freshly captured section/geometry, actual model/camera, visibility1,
depth-override0 and flags0. A further section fetch leads through material+20,
effect+7C and first-pass+9C to the concrete current virtual08 dispatcher.

The verified base D62A80/+08=B5E5E0 is a genuine RET4. The derived
D61BE8/+08=B454D0 reaches B44750/B43410 and the concrete B42350 constant builder.
No successful host replacement is installed. Only the reached normal B6DFA0
consumes the transient model; current live count is reread after every iteration.
Negative capacity invokes actual B229D0 reserve0 before the positive count drain
and final zero store. A negative initial count still performs the cold stage,
then skips the record loop.

There is exactly one B2C209 dispatch per active record. B51A20 initializes entry
and depth and performs no pass iteration. The supported derived wrapper calls
B44750 once, which calls B43410 once atB44AF2; B43410 calls B42350 once atB4353C.
This verified multiplicity permits one embedded persistent constant-builder
child per record. Unrelated queue consumers or reentrant material callbacks do
not borrow this frame; their native runtime composition remains a separate limit.

## Current attachment notification

Existing NativeNodeBinding constructs its CameraTransform from native field
references, including+5C,+A0,+B0 and+138. Its fixed callback only checks attachment
equivalence at binding time; it cannot establish CURRENT A0/virtual3C behavior.
This packet retains the canonical actual model companion for identity/domain
checks and uses the raw fields for the operation, without copying/synchronizing a
second transform or using that fixed callback.

B6DB10 calls the existing full ordered x87 matrix-copy provider at4134F0 directly
on actual+B0. If the low flag byte&0A is nonzero, it captures actual+A0, clears
auxiliary+138 bits30 and sets+5C to zero. A nonnull captured attachment resolves
its CURRENT original profile through the existing read-only profile resolver.
TargetB6DBC0 executes the literal body; another target throws only after those
prior native writes. After notification, CURRENT first-child is reloaded before
the descendant walk. No null notification silently substitutes for a nonnull A0.

B6DBC0 clears+138 withFFFFFFC3, reloads current enclosing+A0, then dispatches that
owner's current+3C. Repeated B6DBC0 targets execute as a tail loop. Original group
D634F8/+3C and nodeD62C88/+3C both contain B6DBC0; model D62DE8's local-setter+38 is
B6DB10. Unsupported tail targets retain all previous auxiliary writes.
B6DA30 visits the current child list, only descends when a child's world-valid bit2
is set, clears its auxiliary bits30 and flags0A, tests its child before the flag
store, recursively visits it, and finally reloads current next-sibling+3C. A child
without bit2 skips its entire subtree.

## String unwind and retained effects

CBD502 loads FuncInfoDF5D40 then jumpsBF6B43. Its six-entry mapDF5D64 is:

| State | Next | Funclet | Armed owner |
|---|---:|---|---|
| 0 | -1 | CBD4C0 | cold effect string |
| 1 | 0 | CBD4CB | cold layout string |
| 2 | 1 | CBD4D6 | cold model-name string |
| 3 | -1 | CBD4E1 | record effect string |
| 4 | 3 | CBD4EC | record layout string |
| 5 | 4 | CBD4F7 | record model-name string |

All six tail to41DD20; no entry releases a generated model. The new source catches
C++ failures only to reproduce those armed string destructors. Each normal
destructor disarms itself before entry; an unwind destructor throwing terminates.
Original FH3 handler ABI, hardware faults, stack addresses and exception identity
are separate from this source schedule. Ghidra currently includes the handler
bytes in the enclosing CBD4F7 candidate; the independent byte span establishes
the actual handler entry without mutating saved analysis.

Persistent caller frames keep child acquisitions, raw scratch preimages and an
unconsumed-model diagnostic. Each record embeds a distinct constant-builder frame
and its pass frame bound to that member, plus its own gather frame initialized
from the caller-supplied prefix preimage. These rows are noncopyable; repeating a
row fails its started check. No completed pass frame or failed child is reused
for another record, and no frame allocation or reset is inserted into the native
loop. The unconsumed-model diagnostic is set immediately after B4C700,
before string cleanup can fail. There is no destructor that rolls back a model,
stream lock, cache increment, render entry, uploaded constants or pass effects.
Failure before normal model consumption retains them. The caller supplies one
fresh persistent record frame for each reached active record, including records
appended by callbacks; insufficient frames are an explicit source-domain boundary.

## Evidence and validation limits

The report carries 49 unique numeric direct/tail call rows, all mechanically
verified, plus the seven explicit indirect sites. Full bytes were compared with
the installed PE through verified live Ghidra queries. Immutable native inputs
are under `local/output/debug24_gather_native_inputs`,
`debug24_raw_notification_inputs`, `debug24_unwind_inputs`,
`debug24_producer_inputs` and `debug24_literal_inputs`; their manifest hashes are
recorded in the report. The gather dependency's previously frozen artifacts remain
unchanged. Parent integration separately corrects its captured profile-token
dispatch, without changing the public gather API.

Strict Win32 build, eight native seeds, both existing CTests and the complete
focused probe pass. Its complete parent link includes actual pass execution,
B42350 constant building, diagnostics and the corrected raw gather. The immutable
closure path/hash is recorded in the report. The probe compares whole
original/source B2 normal empty/skipped/negative-count
paths, the 38-vertex original interior over four x87 rounding modes, and raw local
matrix/descendant/bounds behavior. Its unknown-target case is a source boundary
check. Copied original blocks preserve their internal branches; only matrix/child
direct calls and the vertex step data operand are relocated where executed. The raw
hierarchy fixture uses field arrays and a read-only current profile resolver; it
does not claim that the game's hierarchy constructor graph ran. Its source local
setter does call the existing substantive x87 matrix-copy provider.

The active/cold parent is not executed. Complete linked source still requires the
real constructed renderer, model, geometry, declaration, index, effect, layout,
string and pool domain, current camera/cache/pass/constants, and cold material
compiler paths B3B513/B3B536. The generated-model dependency preserves its native
outstanding optional-index creator reference. Full game setup, original active
unwind/runtime behavior, arbitrary current virtual targets, private scratch alias
and x87 stack-fault ABI, and visual parity remain open. An executable link alone
does not establish those behaviors.


## Combined-library validation (2026-09-14)

Source a6b2884f preserves the newer integrated gather, constant builder and pass
implementations while adding this complete debug source schedule. The retained
fixture now links against the combined CMake library. Original/source zero,
skipped and negative-count parent cases, eight vertex comparisons across four
x87 rounding modes, raw matrix/notification and unsupported-target retained
effects pass. All 123 vertex interior instructions match the generated COFF
after only the documented step-address and branch-target normalization.

The batch passed strict Win32, both existing CTests, eight native seeds, 50
numeric call rows and 21 live/installed-PE spans totaling 2,848 bytes. Seven
indirect sites retain explicit contracts. Four Ghidra annotations were saved
and read back, including BSP_Renderer_DrawDebugSphereRecords at B2BB90.
The active/cold parent, native FH3 and game drawing remain unexecuted.
