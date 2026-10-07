# Complete ordinary landing approach cleanup (cc11)

`009B2C80..009B2D62` is a complete **226-byte, 52-instruction** ordinary
destructor with zero live listing gaps. Its whole original PE body matches the
current live bytes, SHA256
`b816aff23cc930c8b1d3fa44a92510b0d8ab930722328abe39e916b594bdf733`.
Native ECX is the approach receiver; it preserves ESI/EDI and returns with RET.
The new C++ API reconstructs its ordinary cleanup order, not the image ABI or
its private FH/SEH behavior. No x87 arithmetic or new numeric policy is added.

## Actual members and order

Borrow one stable nonnull approach with at least `26Ch` contiguous live backing.
This is the minimum reached by the cleanup fields, not a full constructor,
class size, arena or self-deletion contract. The pure binder checks addresses
and extent only; it does not read represented values or invoke providers.

| Approach member | Exact ordinary operation |
|---|---|
| `254,228,200,1E0,1C0,1A0` | Complete shared `007B45F0`, in this order |
| Follow root `108`, callback `120`, FIRST cell `134` | Complete `006CDD70`, then shared `007B45F0` |
| MoveTo root `CC`, callback `E4`, vector `D8` | Complete `007B65E0` |
| Registry `B8` | Complete ordinary `004116D0` |
| Root profile `0` | Raw `D1FDB8` stamp after every provider |

Each shared view aliases profile+0 and the actual vector+0Ch. The vector uses
the already completed **24-byte `CF5C94` element** contract; the unrelated 90h
node API is not used. The Follow FIRST identity is its actual component+14h,
not a semantic plane or an invented endpoint-offset translation. All ten
natural calls invoke the existing concrete complete Source bodies directly;
there are no whole-body, head/tail or cleanup-phase providers.

The mandatory actual `NativeObserverLifetime` retains its manager/publication,
recursive-lock, dispatch and deletion-provider requirements. Require coherent
live storage, valid FIRST endpoints, known admitted element profiles and
ordinary successful same-CRT allocations/free. Callback-bearing fields are
read by their existing providers at the original points. Aliases must remain
stable; structural reentry, concurrency, invalidation, failure, overflow and
fault paths are excluded. The native registry null-placement condition at
`009B2D38..46` is outside this nonnull ordinary domain.

Retain final provider profiles and fields. Shared data/capacity words remain
dangling numeric identities after array release; never dereference or reuse
their retired allocations. Registry headers are cleared by `004116D0`, while
its proxy and borrowed name/state pointees remain untouched. No approach,
embedded state, FIRST endpoint or borrowed parameter object is self-freed.
All image profile words, including final `D1FDB8`, remain uncallable in Source.

## Connected consumer and remaining boundary

The actual landing task destructor `009B3F50` captures task+3F8h and calls this
body at `009B3FBF`, after its leader/plane+9D4h dequeue branch and before base
`007B4030`. `009B3220` also calls it at `009B3223`; that scalar's free
continuation is a separate unresolved listing gap, not reconstructed here.
Private unwind callers establish reachability only.

This packet supplies the complete ordinary approach-cleanup dependency. It
does not bind the whole task destructor, dequeue, owner retirement schedule,
task+404 retained lifetime, fresh plane+9D4 queue lifetime, base/death cleanup,
arena, remaining state/composite constructors, callable image class profiles,
original ABI, private EH or gameplay. Existing semantic GameUnitsHost
projections are unchanged. No pointer lifetime is inferred from the profiles
or from a successful fixture.

## Focused validation

One ignored connected case executes Source and the **whole original 226-byte
body** against separate actual-shaped `26Ch` storage. It uses genuine registry
construction/Add, MoveTo construction/registration, eight existing vector
operations, real observer registration/deletion/pending suppression and an
already-published actual OS recursive section. Remaining state setup is
explicit borrowed Source instrumentation; their constructors, Follow tuning
entry and runtime manager lookup are not called or claimed. A retained Follow
pair forces its complete ordinary/base cleanup.

The copied original changes only ten four-byte natural CALL operands, routed
to complete genuine Source bridges on the exact received member addresses.
All other 186 bytes remain identical. Its original normal frame/FS prefix and
return continuation remain intact; private unwind execution is excluded.
Bridge counters establish that copied-caller order within this Source domain;
they do not prove original native provider/class ABI or runtime callbacks.

Only inside this executable, a restored CRT-import trace observes live fields
before always forwarding the captured genuine CRT free. It checks ordered
pair/array/registry release, decrement-before-element cleanup, provider-final
fields/padding and profile publication. No retired allocation is dereferenced;
subsequent comparisons read only the still-live approach headers/bytes and
saved numeric identities. There is no reentry or injected production provider.
Both paths restore the pending span and the actual recursive lock depth.

Nine fresh strict MSVC Win32 TUs, including the fixture, passed **287 assertions
in this one connected Source/original-copy case**. `/W4 /WX /permissive-`,
`/O2 /MD /std:c++17` and the PE32 embedded resource-ID1 `asInvoker` manifest
are verified. All 58 input hashes remain unchanged pre/post; all 38 actual
compiler project includes are pinned. Three support libraries were frozen
with source-before/copy/source-after equality from the parent-confirmed current
b7 build; only those immutable copies were used afterward.

Source COFF shows the six-iteration shared loop at `18h`, Follow observer at
`2Ch`, Follow shared at `36h`, MoveTo at `40h`, registry at `48h`, and final
raw profile store at `56h`. These are Source emission offsets, not native ABI
or instruction-encoding equivalence. The unique ignored recipe and manifest
are `local/cc11_land_approach_lifetime_probe_build.ps1` and
`local/cc11_land_approach_lifetime_probe_manifest.json`; precise receipts are
in [the report](../reports/landing_approach_lifetime_cc11.json).

All ten current native direct call rows passed, with zero failures.

No tracked tests, shared metadata, Ghidra mutations, CMake changes or full build
were performed. Root owns independent review, registration, integration and
the full Win32 build.

Primary integration at `e240b8c81d6d43464ae5eb6a6e9db8b7c56cd956` passed the complete MSVC Win32 build and all three existing CTests. The independent current-library fixture freshly compiled 9 actual TUs and passed 287 checks, with 53 Source/header/fixture pins and 38 actual compiler includes. All Source, recipe, installed inputs, three current support libraries and the original PE remained unchanged. New Source COFF, exact native byte agreement, manifest, logs and receipts are recorded in the report.
Current cleanup COFF is94 bytes/39 instructions, calling all concrete providers before the final profile store at56h. The fixture checks the whole226-byte original with only ten natural CALL operands changed and186 bytes untouched, with real providers and before-free-only observation. No remaining constructor/task/world/native-class-ABI/game claim is added.

Profile identity remains qualified: the fixture deliberately uses a borrowed raw D1FF94 preimage and does not invoke the native inner constructor. Current producer evidence at9B2E50 instead publishes D1FF88 atroot and D1FF84 atregistry+B8. This distinction does not change the ordinary cleanup assertions or establish class ABI.
