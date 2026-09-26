# Canonical clock publication pointer cell, CC10

The actual `004DE4B0` getter now borrows a live `void* volatile&` publication
cell, matching the existing sampler consumers and clock shutdown context.
It converts freshly loaded pointer values to `NativeParticleClockStorage*`;
the sampler adapter forwards its original void reference directly. It no
longer reinterprets a live void-pointer scalar object as a different typed
pointer object. Equal pointer size/address was not a C++ alias/lifetime proof.

## Source and caller scope

Only `include/bsp/native_particle_clock_singleton.hpp`,
`src/native_particle_clock_singleton.cpp` and
`src/native_sampler_loader_context.cpp` change. There is one tracked direct
getter caller, the existing sampler operation-frame adapter. Existing material
sampler and system-gather contexts already borrow void publication cells.
The ignored old clock probes and sealed source copies remain immutable;
they are historical direct callers, not production consumers to migrate.

The source getter's decorated symbol changes. Consumers must compile against
the new header. Win32 references remain four bytes, but this remains a new
source ABI rather than the original no-input cdecl/EAX/RET entry. No overload,
second cell, fallback getter, placement lifetime handoff, retain, metadata,
owner/publication admission test or extra store is added. Owner payload,
untouched +18 and every native-derived access schedule are unchanged.

The captured initial volatile load/fast return, first manager read and +10
guard capture/enter/depth, second publication read, allocation/payload stores,
publication even null, second manager read, fresh publication registration,
captured-guard release and final reload retain their order. C++ catch cleanup
remains unchanged; private native FH3/SEH/exception ABI is not established.
Publication scalar-object consistency does not establish the raw allocated
owner's struct/view lifetime or its returned sampler-view interpretation.
Nonempty cache, concurrent application publication and full loader remain
outside this packet.

## Strict build and compiled artifacts

Baseline is `b23f7b1221173b78804a04dcdd162b77b55716df` plus the reviewed
three-file diff. Nine explicit source/config pins, exact baseline Git tree,
27 tool/SDK identities and preconfigure cache were recorded before build.
The fresh MSVC Win32 Release build passed `/W4 /WX /fp:strict` and all three
existing CTests. No tracked test was added. The build completed once from
2026-09-26 07:55:52.539778 UTC to 07:56:14.261217 UTC.

The old getter and adapter objects were recovered from frozen Packet B
copies and matched their exact original core-library members. The new
274-byte getter and 127-byte adapter COFF bodies are byte-identical to those
old bodies; relocation locations/kinds remain identical. Only two relocation
target names change: getter offset6 DIR32 names its newly decorated EH
handler, and adapter offset93 REL32 names the newly decorated getter.
`compiled_comparison.json` and `compiled_schedule.json` preserve both bodies,
relocations and the unchanged four publication loads/one store, two manager
reads, captured guard and registration sequence. This is object evidence,
not newly recovered native bytes or native ABI compatibility.

The reviewed standalone fixture compiled/linked once from 08:02:02.624784
UTC to 08:02:05.264920 UTC with active assertions, `/MD /EHsc /std:c++17
/O2 /Gy /W4 /WX /fp:strict /permissive-`, I386 image base10000000 and an
embedded asInvoker manifest. Its source/object/executable/map/manifest stayed
unchanged afterward. Before compile, 59 explicit inputs, 35 project headers,
three project libraries, six provider objects and 25 tool/SDK pins were
sealed. All 16 searched libraries, including dinput8/dxguid, were individually
pinned before this link. Previous Packet B's two postlink SDK identities
remain a separate historical limit; no old archive is rewritten.

The final map selects 362 genuine project members, each matching its exact
CMake object. Ten COFF windows cover4,504 bytes: nine standalone entries
match linked bytes except declared relocations; 004DE290 is object-only,
inlined into selected B1B680. The new linked getter is at10007CC0 and the
adapter at1002ADE0. Selected does not mean reached. The inherited dispatcher
section contains2,972 code bytes plus137 compiler switch-data bytes, based
on indexed COFF lookup relocations. Full section bytes are preserved and
matched, while disassembly stops before lookup data; no data-as-code repair
or stronger whole-section instruction claim is made.

## Fixture and controller preparation

The one case calls the direct genuine getter, registers the real empty clock,
and drains the actual manager through the existing CE7D38 -> 004DE340 flags1
binding. It declares one actual `void* volatile` cell initialized null, live
through the entire getter/registration/drain; the old typed-to-void fixture
handoff is removed. It retains genuine CRT manager/clock/section providers,
actual string interface and empty `NativeRenderActualOwnerRegistry`.
There is no +18 initialization/read. Empty record/cache/sink domains are
bound but their nonempty release branches and InterlockedDecrement remain
unexecuted. The sampler adapter is source/COFF-verified only; the direct case
does not execute a sampler pump, cache or application binding.

The unchanged genuine suspended-child bootstrap admits the same two20-byte
spans CE7D08/CE7D24 in one CE0000 band. The parent reports actual suspended
child PID/kernel creation time/image path, then waits for controller byteA.
The controller verifies retained process handles, mandatory kernel image/
FILETIME and exact CIM parent/command/creation; a present CIM image path must
agree, while null CIM image metadata is accepted. Cleanup addresses only
verified owned handles and exact-parent child discovery, with closure unknown
if identities/commands cannot be established. No process-name kill is used.
Controller correction after compile changes no probe/provider/linked bytes.

The first authorized controller invocation refused BEFORE launch_guard/Popen
because its `str(Path)` identity used Windows backslashes while the seal used
forward slashes. All1,029 content/size pins still matched; no fixture parent,
child, ACK or clock case occurred. The exact failed controller, old manifest,
release, actual raw tool result and zero-process audit were frozen in
`controller_preflight_refusal.zip` (nine rows/ten members). This is a
controller preflight refusal, not a failed clock runtime.

The only correction changes identity path spelling to `Path.as_posix()`;
exact dictionary/size/SHA checks remain intact. A pure preflight extracted
that exact helper by AST without importing controller main/ctypes or calling
Popen and passed all1,029 identities under the real worktree cwd. Its actual
tool result is archived. Other1,028 inputs remained unchanged. The revised
controller is separately sealed before any actual case release. Old d7 and
null-CIM controller drafts remain preserved and unexecuted in this packet.

Static helper failures are preserved separately: a substring/AST adaptation
error, inherited full-section decoder assertion, and seal-helper snapshot/
legacy filename assumptions. They occurred before any clock case and changed
no source/object/executable. No production rebuild or relink followed them.

## Runtime closure

After separate primary verification/release of the corrected manifest and
controller, ONE actual fixture case passed. Parent92428 and child15264 both
exited zero; actual child wait was0. Before reservation CE0000 was MEM_FREE;
the real ReadOnlyV1 mapper ACK and exact CASE PASS marker were observed. The
same live void cell ended null, the still-live manager's vector/count/section
fields ended zero, and only the drained manager was explicitly freed afterward.
No postfree clock/vector/section read occurred. These observed postconditions
support the reviewed source/compiled schedule; no dynamic stack trace or
per-free instrumentation was recorded.

The suspended child's mandatory kernel creation FILETIME was
134348839157185100 and matched its actual CIM command/parent/creation. CIM
image metadata was null; the retained kernel image identity remained exact.
Controller byteA was sent at2026-09-26 08:11:56.188543 UTC. Both retained
handles became signaled, final exact-parent discovery completed, no remaining
owned process existed, and no timeout, termination action or capture/cleanup
error occurred. All1,029 inputs,40 previous archives and1,188 previous frozen
files remained unchanged. `raw_actual_case_launch_tool_result.json` preserves
the actual successful exec_command return object, exit0, and output identical
to the controller's `run.json`; the call completed without a yielded session.
It is distinct from the preserved preflight refusal's raw result.

There was one controller preflight refusal with zero fixture launches and one
separately released actual fixture launch. No additional clock case, runtime
adaptation, rebuild or relink followed the actual result. The final report
keeps preflight, controller capture, source/COFF proof and reached case separate.

## Evidence boundaries

No application F8D420 producer/context binding is activated. Existing
01090AB0 frame-clock wiring is distinct. Native00736930 explicit unregister/
primary slot0 teardown and full00737F30 application shutdown remain separate
from this raw manager drain. No secondary/changed-publication case, nonempty
cache, sampler pump, source0/W, full B4/startup, original game, native register
ABI or private FH3 validation is included. Existing owner raw-view uncertainty
is retained.

The report is `reports/native_clock_publication_cell_cc10.json`; final archive
is `local/cc10_clock_publication_cell_evidence.zip`, with a member manifest and
external freeze receipt in `local/cc10_clock_publication_cell_probe`.
Prior archives/files are preserved by exact identities. The prior Packet B
missing raw tool-wrapper receipt is explicitly historical; this packet saves
the actual launch tool return separately from controller observations.
