# Registered model effect completion, update and stop

Addresses: `00872010`, `00872740`, `00871FE0`, `00AF6BE0`, `00AF5F20`,
`00AFF570`, `00B05070`. Descriptive names below are reconstruction hypotheses.

The type-1 event now has complete completion/update/deactivation entry bodies,
the model completion decision, and the direct model/emitter stop loops. The
simulation update and application cleanup remain required real callees. These
are new C++ interfaces over the existing `RegisteredModelEffectStorage` and its
actual model slot, whose prefix is `NativeNodeStorage`. There is no additional
reference count, copied model, substitute emitter array, or successful fallback.

## Native bodies and coverage

| Routine | Inclusive body | Original ABI | Coverage |
|---|---|---|---|
| Event complete `872010` | `872010..872017` | ECX event; tail JMP AF6BE0; AL; RET | Complete entry through recovered AF6BE0 |
| Event update `872740` | `872740..872761` | ECX event; stack float delta, reference node; RET8 | Complete entry through required AF6DD0 |
| Event deactivate `871FE0` | `871FE0..871FE7` | ECX event; tail JMP AF5F20; RET | Complete entry through recovered stop chain |
| Model complete `AF6BE0` | `AF6BE0..AF6C46` | ECX model; AL boolean; RET | Complete through required 51F6B0/AFF690 |
| Model stop `AF5F20` | `AF5F20..AF5F50` | ECX model; RET | Complete through recovered AFF570 |
| Emitter stop `AFF570` | `AFF570..AFF57C` | ECX emitter; RET or tail JMP B05070 | Complete through recovered B05070 |
| State-container stop `B05070` | `B05070..B05100` | ECX container; RET | Complete through required B04F00 |

Every `bsp.py ghidra` query invokes `Client.verify()` against configured project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language x86 LE32,
and image base `00400000`. This worker made no Ghidra writes. All seven exact
live/disk byte spans matched; report hashes identify the bytes used by the fixture.
At initial inspection, `872010` and `872740` lacked function definitions. Their
last instructions are `872013: JMP AF6BE0` (5 bytes) and `87275F: RET8` (3 bytes),
respectively. The integrator owns definitions, names, refreshed exports and ledgers.

## Table, producers and actual storage

Live `D0DE18` words establish virtual08=`872010`, virtual28=`872740`, and
virtual30=`871FE0`. `8742A0` already constructs the canonical 20h event: type18=1,
model1C is the actual 2DCh particle-model slot, and prefix04 is the sole event
count. These methods do not retain, release, clear, detach or destroy the model.
Direct virtual30 does not clear event.active0C: the existing `PointEffectChildEvents`
contract captures the current virtual30, clears active0C, then invokes the body.
The application must bind the exported functions through that established interface.

`AF74A0` is a required, separately owned constructor. Its assembly initializes
model194/198/19C at `AF74DA/AF74E0/AF74E6`, sets active1A4=1 at `AF7506`, and
sets initialized1A5=0 at `AF750D`. Its later emitter loop allocates 28h emitters,
calls `AFF5F0`, appends their actual pointers to194 and increments198. `AFF5F0`
stores model08 and definition0C, retains definition04, then zeros container10
at `AFF620`. The model190 owner is constructed by `AFD2E0`, which initializes
its live count14 to zero. `AF6DD0` sets model1A5 at `AF6E34`.

`AFF690` creates a 30h container via `B053D0`/`B04E30` only if emitter10 is null;
otherwise it returns the current pointer. `B04E30` creates the actual 8h index
rows at container0C and 6Ch state backing at14; count1C starts at zero. Each row's
WORD0 identifies a state. `B04C80` selects `states14 + row.id * 6C`, calls
`B0CA40`, and increments count1C. `B0CA69` stores the incoming definition pointer
to state64; `B0CA6C` clears state60. This producer resolves the decompiler's
misleading floating-point types for these pointer fields. No second layout type
or public offset constants are introduced here.

The update's reference argument is the existing opaque reference-node argument
of `PointEffectChildEvents::update_virtual_28`; it must cover DWORD198. It cannot
be the 114h point-instance storage. A camera's canonical tail also has mode198,
but this packet does not assume every reference is a camera or add a new schema.

## Behavior and required calls

Completion first calls the existing `RegisteredModelEffectCallees::call_0051f6b0`
and then reads the returned live owner's byte04. A nonzero byte completes even
an uninitialized model. Otherwise model1A5 must be nonzero. The model198 count
and194 pointer are captured once; every nonnull emitter10 is queried through
the actual `AFF690` getter. Any nonzero container1C prevents completion. Only
after the captured span ends is current model190+14 tested for zero.

Update preserves FLD delta, mode comparison against3, model1C capture, and FSTP
argument ordering. Only the low mode byte is defined by native SETE DL; stale
upper EDX bits are not a second parameter. Required `AF6DD0` receives delta and
that byte, with RET8 cleanup. Its entire `AF6DD0..AF7391` simulation remains a
dependency: matrix refresh/copies, initialization and random values, x87 fixed
steps, emitter simulation, geometry and bounds have not been replaced.

Model stop captures194, clears1A4, then captures198/end once. Each emitter10 is
loaded once and skipped if null. Container stop uses signed count1C and a
definition64 byte28 filter. It calls B04F00(state,0), ignores AL, reloads count
and row backing, and swaps the last live row into the selected position. Only
WORD0 and float4 move; bytes2..3 stay in place. The selected-to-last float uses
FLD/FSTP, while last-to-selected preserves the captured DWORD via MOVSS. The
replacement row is rechecked. Required cleanup may mutate backing/count; the
implementation retains the native capture/reload boundaries and modular counters.

| Containing function | Site | Native callee | Contract / cleanup |
|---|---|---|---|
| 872010 | 872013 | AF6BE0 | Tail JMP, actual model1C; AL / RET |
| 872740 | 87275A | AF6DD0 | Required real update, float and low mode byte; callee AF738F RET8, caller87275F RET8 |
| 871FE0 | 871FE3 | AF5F20 | Tail JMP, actual model1C; RET |
| AF6BE0 | AF6BE3 | 51F6B0 | Existing required singleton getter; no stack args; read owner04 after call |
| AF6BE0 | AF6C1E | AFF690 | Required getter; actual emitter; EAX current container; AFF6F5 RET |
| AF5F20 | AF5F42 | AFF570 | Actual emitter loaded from captured span; RET |
| AFF570 | AFF577 | B05070 | Tail JMP only for nonnull captured emitter10; RET |
| B05070 | B050AA | B04F00 | Required actual6Ch state and DWORD0; B04F99/B04FAE RET4; ignore AL |

All currently reported direct callers of AFF690 were inspected: AF6C1E and
AFE8EB both use an actual emitter, the latter followed by B04C80. All B04F00
callers were checked: B050AA and B05185 push0; B05008 pushes1. Its interface
therefore preserves the full DWORD argument and AL result. B04F00's body calls
current definition64 virtual1C(state,argument), then conditionally unlinks the
actual point light and releases its node under the actual72B740 lock. It cannot
be supplied as a successful no-op. The AF6DD0 caller list has only87275A.

## Verification and integration limits

One ignored Win32 native-byte fixture (`local/model_behavior_probe.cpp`) directly
executed all seven recovered spans with relative transfers relocated. At required
boundaries, interception recorded update arguments, returned the established
nonnull emitter container, and performed deterministic cleanup-side mutations.
It did not claim to execute AF6DD0, allocating AFF690, B04F00 cleanup, or AF74A0.

The fixture compared completion gates/counts and the full stop chain, including
false cleanup results, cleanup replacing row backing, cleanup replacing the model
span/count, exact row padding, x87 signaling-NaN copy, unchanged actual event04 and
model04 counts, and unchanged direct active0C. Update mode2/3 and x87 argument
NaN bits also matched. Strict MSVC Win32 `/W4 /WX /fp:strict` compile and the
fixture passed; the executable embeds its manifest. This is bounded differential
evidence, not original ABI/SEH compatibility, a constructed particle-model test,
or gameplay/visual proof. The fixture remains ignored rather than adding tests
to the repository for unimplemented application dependencies.

The integrator must add the source to CMake, record seven routine spans, bind the
three current type-1 virtuals to existing event owners, and supply the required
callees. AF74A0 construction and the F8D2D0 pool lifetime remain separate work.
The repository build/check outcome and exact call-row verifier status are recorded
in the report; they do not replace validation of the required application path.

## AJ combined integration verification

The source is registered in bsp_core. The combined strict MSVC Win32 build and
both existing CTests passed. The report records the focused fixture replay, exact
call/tail checks, saved Ghidra name/signature preimages and comment readback.
Required external runtime bindings, original exception ABI and gameplay remain
limited as described above; this integration does not extend the fixture coverage.
