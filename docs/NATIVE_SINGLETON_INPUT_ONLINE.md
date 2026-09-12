# Shared singleton drain for input and online owners

Addresses: 00BD0400, 00A3F530, 00A3F5D0, 00A3F670, 00A3FDC0, 00A909E0, 00A91150, 00A97C00, 004BEC00, 00A93DA0, 00A93DD0, 00A93E50.

The existing raw14h singleton manager can now delete recovered input and XLive
owners. `NativeSingletonDeletionBindings` supplies borrowed services for six
additional native profiles. The original BD0400 loop still pops the last slot
before reading its current profile, passes flags1, reloads live count, releases
the actual section and clears/frees vector storage. This is a finite source
dispatch table, not an arbitrary native C++ vtable or a new native routine.

| Native profile | Recovered scalar entry | Source requirement |
| --- | --- | --- |
| D24138 | A3F670 | Exact XLiveOwnerAllocation identity |
| D2413C | A3FDC0 | Exact XLiveOwnerAllocation identity |
| D5B5F4 | A909E0 | Raw backend context and actual publications |
| D5B5F8 | A91150 | Same, including raw device providers |
| D5B72C | A97C00 | Same, including GUID/class/device lifetime |
| D5B630 | A93E50 | Raw action context and concrete storage calls |

The slot-zero DWORDs were read from live analysis and agree with the workers'
installed-image evidence. Native entries receive ECX owner plus a flags DWORD,
return the captured allocation address in EAX and RET4. Services add source
arguments, and the raw manager uses an added EDX binding: no drop-in ABI claim.
Names remain hypotheses and required providers do not stand in for recovered
hardware or listener bodies.

XLiveOwnerAllocation owns one storage object and a stable typed projection; the
storage's first DWORD is the same profile read by the raw manager. It is a source
allocation boundary, not the original3F0h layout. Registration derives the raw
identity from the freshly reloaded typed publication. Teardown must finish before
its actual free callback releases this allocation; runtime/sign-in/IPC services
remain borrowed. Callers relinquish the consumed owning pointer exactly once.

Input owners are actual raw allocations, not typed device/action projections.
Their contexts must borrow the same manager/publications used at construction.
Dispatch deliberately does not compare an owner against the current publication:
the recovered destructors implement their own reload/unregister/clear order,
including replacement-publication behavior. Backend reset and zero-reference
device slot calls take no stack flags. Native teardown does not release its
DirectInput references; the explicit SDK tracker releases them only after raw
owners, devices and callbacks stop borrowing them.

NativeInputActionStorageCalls implements the action owner's two required array
calls using the concrete DWORD and30h-record providers. Its context is borrowed
through destruction, including nested34h bindings and14h modifiers. The actual
listener slot-zero provider remains required. GameSingletonHost has binding
methods for these services, but application startup does not yet create these
raw input/XLive owners or connect their canonical data to the frame projections.

Validation used the combined primary headers and archives only, with manifested
Win32 probes and no separately compiled production source. XLive exercised both
profiles through BD0400, checked two pop-before-free events, three total actual
frees, publication clearing and survival of borrowed achievement state. Backend
exercised D5B72C with nonempty fixture devices: three resets, one zero-reference
call, callback replacements and native count/refcount order. Real DirectInput
creation returned HRESULT0; two COM references survived drain and explicit host
release brought them to zero. Base/intermediate profile mappings were reviewed;
they were not separate raw-drain runtime cases.

The action fixture compared original owner/record constructors across four
preimages, deep-copied nonempty nested arrays, checked padding/full-word copies
and listener retain balance, and drained a registered D5B630 owner through the
actual manager. Both publications ended null, with one external listener
reference retained. A separate injected listener exception checked native nested
cleanup; normal listener slot-zero execution was not reached or fabricated.

The combined Win32 build, both existing CTests and eight native seed checks
passed. The six report audits checked117 direct CALL/JMP rows with zero failures;
indirect calls were qualified separately. The two-frame application regression
created its window/device, presented twice, exited0 and drained its three
existing registrations. That run did not instantiate the new input/online owners.
No permanent tests or hardware-input/gameplay validation were added. The promoted
8afc372d application attempt reached the already-running guard while another
orchestrator's executable was open; the runner ended that test after180 seconds.
The earlier two-frame pass is not final promoted application runtime proof.

Analysis corrections: four scalar/vector post-free gaps were repaired. A93D80,
C64D48 andCB6A36 cleanup tails were decoded and their stored bodies recreated
under the write lock, preserving old names/comments; each final listing has no
gaps. C64D53 andCB6833 EH handlers were defined from exact verified byte ranges.
The19 provider and19 owner signatures were saved with old values recorded, and
affected exports refreshed. These metadata repairs are not extra C++ routines.

Follow-up packets: recover actual A982B0/A98030 enumeration, A9A5A0 raw240h device
construction and A904E0 attachment; connect accepted-group and action storage to
existing frame/focus consumers without duplicate owning vectors. Online startup
still needs real mutable present parameters, current device/profile fields and
process-lifetime IPC state. Real listener profiles, canonical menu-music ownership
and the MoviePlayer host remain distinct dependencies. See the integration report
for artifact hashes, initial fixture evidence and final promotion verification.
