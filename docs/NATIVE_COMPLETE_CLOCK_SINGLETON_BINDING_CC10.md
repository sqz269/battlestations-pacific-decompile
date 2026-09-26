# Complete particle-clock singleton deletion binding, CC10

The reconstructed raw singleton manager now admits complete-owner profile
`00CE7D38` through the existing `004DE340` scalar-delete provider. A strict
Win32 build and all three existing CTests passed. One genuine registered,
empty-clock source composition passed; parent and child exited zero and the
closed-process receipt found no matching probe processes. This is bounded
source-interface and fixture evidence, not a native ABI or game validation.

## Production change

`NativeSingletonDeletionBindings` appends a borrowed
`NativeParticleClockShutdownContext* particle_clock` at Win32 offset 176.
Its source-interface size changes from 176 to 180 bytes; every prior field
offset remains unchanged. Consumers must compile against the new interface.
The dispatcher reads this binding only for `00CE7D38`, passes the popped
complete owner and original flags to `delete_native_particle_clock_004de340`,
and retains the existing unsupported-profile behavior for a null binding.
It admits neither secondary profile `00CE7D24` nor owner/publication equality.
It adds no allocation, retain, publication clearing or manual clock free.

The existing manager still pops before invoking deletion with flags 1, then
rereads its count. The existing clock schedule remains `004DE340 -> 00B1B680
-> 004DE290`, followed by unconditional clearing of the same `00F8D420`
publication cell and base-profile `00CE3818`. The scalar delete reads the
original flags after destruction and frees the captured complete owner only
when bit 0 is set. Its returned dead pointer is pointer bits only.

The original `00BD0400` entry takes ECX manager and returns without stack
arguments. Original `004DE340` takes ECX complete owner, one stack flags
argument, RET 4 and returns captured owner in EAX. The reconstructed C++
interfaces supply additional bindings/context and are not drop-in register
ABI replacements. Native FH3/SEH/unwind compatibility remains separate.

## One empty-clock fixture

Final source is `local/cc10_complete_clock_singleton_probe/probe.cpp`.
It calls the genuine `004DE4B0` getter with real CRT singleton allocation,
actual manager registration, tracked critical-section ownership and the
existing string-pool interface. The clock is registered exactly once; its
record count is zero. The fixture checks the initialized primary/secondary
profiles and words at +8/+C/+10/+14. The untouched +18 word is supported only
by the static native constructor/listing evidence: it is never seeded or read.

A real `NativeRenderActualOwnerRegistry`, records resize context, string
storage and returning `_invalid_parameter_noinfo` callback stay alive through
drain. The canonical release domain and real kernel32 InterlockedDecrement
entry are bound but unexecuted in this empty case. No record, sink decrement,
nonempty cache or virtual sink call receives runtime credit. The string pool
remains null and no raw-string return occurs.

The getter's typed publication interface and shutdown's void publication
interface use a strictly quiescent lifetime handoff. Aligned raw cell storage
first contains an actual `NativeParticleClockStorage*` object. After the getter
returns, the fixture captures its value, leaves the scope of every typed
reference, then placement-creates an actual `void*` at the same storage with
the same value. Placement reuse ends the previous scalar-pointer lifetime;
access to the new object uses the returned placement pointer. Four publication
bytes remain identical. No old typed reference is used afterward, and there
is no duplicate publication cell, bit clearing or extra ownership credit.
Shutdown context construction and drain follow this boundary. This follows
C++17 storage-reuse/lifetime rules, not equal pointer sizes or address identity
alone. It does not rely on a scalar pseudo-destructor ending a lifetime.
See [N4659, basic.life](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf)
and the historical scalar pseudo-destructor discussion in
[P0593R6 section 3.6](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0593r6.html).
This phase boundary is fixture-specific, not a general concurrent publication
migration or full application lifetime proof.

Only `bindings.particle_clock` is assigned. The genuine `00BD0400` drain
retires the popped complete clock through `004DE340` flags 1. Afterward the
same publication cell is null; the still-live manager's vector fields and
tracked-section slot are zero and its count is zero. No clock, vector or
section is read after free. The manager publication retains its original
pointer bits according to the raw getter policy; the fixture then frees only
the already-drained manager through the genuine singleton lifetime provider.

The original file is read-only data input, never executed. The fixture uses
the genuine `GameNativeDataBootstrapChild` suspended-child reservation and
one-use `ReadOnlyV1` handoff for exactly two 20-byte spans, `00CE7D08` and
`00CE7D24`, in the single `00CE0000` band. Before reservation the parent
observed MEM_FREE. A real mapping ACK precedes the child case. The earlier
unsuspended source was reviewed, superseded before compilation and preserved
separately in `unexecuted_unsuspended_revision.zip`; it has no runtime credit.

## Build, artifact and run evidence

Baseline is `d3eec1a8554a154f1cd12d15d19e5abda45179da` plus the reviewed
two-file production diff. Eleven explicit source/build pins and this exact
Git tree were sealed before the fresh Release Win32 build. The build log
records `/W4 /WX /fp:strict` and three passing CTests: reconstructed_math,
native_math_differential and tool_tests. Toolchain identities were captured
during build and before standalone compilation, not individually hashed
before the production build.

The standalone source has active assertions and compiles with `/MD /EHsc
/O2 /Gy /W4 /WX /fp:strict /permissive-`, with an embedded asInvoker manifest.
Before compile/link, 53 explicit project inputs, the project header closure,
three exact libraries and selected provider objects were copied and pinned.
The executable is I386, image base `10000000`. No rebuild or relink followed.

The link selected 362 genuine project archive members; each matches its exact
CMake object. These are selected, not all reached. `compiled_evidence.json`
stores the member/object/source identities and nine COFF bodies, 4,377 bytes.
Eight standalone entries match linked bytes except declared four-byte COFF
relocations. `004DE290` is object-only: `/OPT:REF` discarded its standalone
entry after its schedule was inlined into selected `00B1B680`. The inlined
schedule includes the empty-record loop skip, two resize-zero calls, current
array free and publication base cleanup. No standalone linked-004DE290 claim
is made. Dispatcher instructions load context at +B0 for CE7D38 and pass
original flags/owner to the existing scalar delete; manager instructions pop
before dispatch and scalar-delete instructions destroy before flags/free.

An initial inspection helper assumed all nine bodies had standalone map
entries and stopped on 004DE290. That helper and its diagnosed expectation
error are preserved. This was a static inspection error, not a compile,
link or fixture failure, and did not change source/object/executable.

Two implicit SDK inputs, `dinput8.lib` and `dxguid.lib` from SDK
10.0.26100.0, were discovered in the successful verbose link receipt. Their
individual hashes and copied payloads were sealed only after linking, before
the run. The earlier seal identified their SDK root/version but did not hash
these two files individually. This explicit prelink coverage limit remains;
the already-successful object/executable were not regenerated. All 16
searched libraries and 294 actual fixture header files have exact identities.

The final preRun seal contains 1,022 identities. The one launch ran from
2026-09-26 07:36:10.389582 UTC to 07:36:11.401146 UTC: parent PID 54320,
child PID 48136, both exit 0, child wait result 0, no timeout. The source
reported its bound actual manager/clock, cell and count, then CASE PASS.
All 1,022 inputs, 36 previous archives and 50 prior frozen files remained
unchanged. The CIM process audit found zero matching processes. Observed
postconditions support the source/compiled schedule above; the fixture did
not record a dynamic call stack or instrument every free instruction.

## Boundaries and archive

No changed-publication case, secondary admission, nonempty cache, sink
decrement, other singleton profile, full startup, native private FH3 or game
execution was tested. Bound canonical domains and compiled branches outside
the empty-clock path remain unexecuted. Existing clock providers are unchanged.

The report is `reports/native_complete_clock_singleton_binding_cc10.json`.
The immutable archive is
`local/cc10_complete_clock_singleton_binding_evidence.zip`, with per-member
identities in the probe directory's `manifest.json` and an external
`freeze_receipt.json`. It includes exact source/provider/header/library/
object/executable/map inputs, command/log/run/closure receipts, the superseded
unexecuted source and inspection error, and the original bounded design
archive containing eight native windows (610 code + 44 data bytes). Prior
archives/files are preserved by identity rather than re-frozen or overwritten.
