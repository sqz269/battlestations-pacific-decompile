# Native camera raw name forwarding

B71A80 now accepts an actual eight-byte name header and `NativeNodeRawConstants`.
The original header address, actual 45Ch slot, existing runtime raw context and
constants reach the delivered five-argument B6F5A0. The semantic overload remains
available. This extends three existing bodies (604 + 194 + 32 = 830 bytes), with
zero new bodies or fragments and no new native ABI claim.

The immutable `NativeNodeDestructionRuntime` name domain selects construction and
cleanup. Both required mode and identity of the node/viewport D7A24C cell are
checked before prepared becomes constructing. A mode or cell mismatch preserves
the prepared prefix/tail lifetimes and their existing scene association. There
is no temporary NativeString, dummy semantic pool, duplicate runtime, context,
pool owner or publication cell.

`finish_node` selects raw B6F440 when the runtime uses the raw domain. This same
helper serves constructor unwind, ordinary destruction and scalar deletion;
NativeCameraReference already calls that scalar terminal. The actual name pool
context, its three publication/gate cells, manager/providers, environment and
canonical companions must outlive every retained reference. Counts at actual
slot+04 and the D62CF0/D62C88 profiles still address the same 45Ch allocation;
+458 remains the canonical camera pool slot identity. Binding a reference adapter
adds no reference count or copy, and its disposal contract owns later retirement.

## Load and cleanup evidence

The raw constructor repeats the established continuation literally instead of
refactoring the semantic body. Strict MSVC Win32 emitted code confirms the exact
header is pushed, checked context/constants precede the phase transition, and the
seven direct constant reads retain their order. Fov, far and one are captured
before the fov store; aspect is read after that store; scalar after the far store;
the single captured one supplies near, clear depth and both axes. Scalar1DC is
read after clear flags. FLDZ/FLD1 with FSTP32 supply viewport depth arguments.
The camera captures one renderer identity for two current dispatch calls, whereas
viewport construction reloads the renderer publication twice. The shared
continuation's alias, x87, SIMD/raw-word and current-publication boundaries remain.

The source consumes constructor state0/1/2 and destructor state1/0 exactly as
before. Failed node construction owns its cleanup; the camera ends only its tail
and association. Constructor unwind deliberately does not release published
viewport/fog. Normal raw name getter failure still consumes node/base/tail cleanup
and propagates through a direct destructor/scalar call. B71FE0 calls destruction
before testing flags bit0, so a throw prevents canonical B711E0 pool return.
Failed-name ownership remains external; an ended prefix must never be retried.
Secondary cleanup exceptions still terminate. NativeCameraReference and shared
queue callbacks retain `noexcept`, so a getter throw through that route terminates.
Native FH3/SEH, CRT exception identity and simultaneous cleanup failure are unproved.

## Verification

The final LF source/header inputs pass strict `/W4 /WX /fp:strict` Win32 compilation,
the full standard Release build, both existing CTests and eight seed comparisons.
Fresh guarded read-only Ghidra checks match all 830 original body bytes and ten
relevant call/tail operands. Existing node17/node13/camera20 caller preparations
are reused with all 50 original disk operands verified; the integrator refreshed
camera20 live xrefs. The only scalar incoming xref remains DATA D62CF4. No Ghidra
mutation, annotation, prototype, ledger or saved analysis change was performed.

The prior external camera fixture was extended once, not its 104-checkpoint
matrix. One finite current-one profile at x87 CW027F compares 13 original/source
snapshots (14,508 camera bytes per path) and nine ordered allocator/renderer/
retained events. It constructs from a nonempty actual header, checks name contents
and exactly one actual raw pool return, then directly calls scalar flags1 and
checks canonical slot return. The actual renderer+1A14 parameter regions are
bound through D3D9ViewportRendererAccess and concrete
NativeD3D9RendererParameterDispatch; a fixture observation decorator changes their
current fields/publications and delegates to those real bindings. Existing
NULLREF D3D9 device, actual viewport/pose/plane/type/pool and retained providers
are used. No fallback dimensions or successful-only production provider was added.

Complete original camera/node/41DD40 bodies execute in the captured original-byte
closure. Explicit ABI-only bridges call current shared 419CC0/BD1120/BD1510 helpers
and CRT memmove, so this is bounded parent-body differential evidence, not an
independent original string-pool implementation test. Existing successful viewport
allocation observation hooks remain for the pair; all singleton allocator hooks
are restored before the failure case, which executes the genuine current library
allocation/new-handler/throw path. The disposable probe alone enters a Windows
job with PrivateUsage+2MiB quota, provoking actual second-pool bad_alloc in the
normal raw name getter. It observes dead base/tail, no camera/name pool return,
unchanged failed name, and explicit external name/physical-slot cleanup. Prepared
wrong-mode and wrong-cell rejection is also checked. Once-only state consumption
is backed by source inspection plus final state, not a hardware store count.

All 8,712 mapped original code/boundary bytes remain identical before and after
these trajectories. The external capture at
`C:/Users/sqz269/bsp-bf-raw-camera` contains original spans, mapped before/after
bytes, paired snapshots, probe/recipe, exact physical source/header inputs,
selected current three libraries and logs. Its default `run.ps1 -Repo <root>`
compiles only external probe.cpp and links those libraries with MANIFEST:EMBED;
there is no WorkerSource mode. The JSON report seals archive and current pins.

This is reconstructed, build-tested and bounded-fixture-tested C++ behavior.
Original caller ABI, arbitrary profiles, rebound IAT target identity, foreign
point-light arrays, asynchronous/fault-time observations, native FH3 and gameplay
remain outside the established domain. The B3C800 holder/application must still
provide persistent companions and publication ownership after holder failure;
this packet does not reconstruct that raw24h holder or the complete B14A10 service.
