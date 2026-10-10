# Allocation statistics retained host binding: Source packet

`GameSingletonHost` now retains the allocation-statistics constructor context and
installs it in the raw singleton deletion bindings. Profiles `00D685E0` and
`00D685F4` dispatch the popped receiver and full flags to the genuine retained
Source scalar-deletion entries `00BE2890` and `00BE2930`. A missing context reaches
the existing unsupported-binding `logic_error`. This adds no statistics owner
construction or startup registration.

The packet owns four C++ files, this document and
`reports/cc12_allocation_stats_retained_host_binding_source.json`. Its only leased
address is the already reconstructed finite dispatcher `00BD0400`. The report
contains complete Source and candidate object audit records and pins the full
local evidence. Names describe reconstructed Source behavior, not recovered
symbols or drop-in Native interfaces.

## Accepted context and preimages

Explicit Root approval of the process cell opened this packet at
`8a4be6482547641126a826866a8279e833fbda82`. Before changing C++, the worker retained
the accepted process-cell Source993 manifest and every one of its 997 pins,
170 whole selected Core objects, 11 whole selected App objects, eight actual
project/compiler/linker tlogs, 425 current Source/Git preimages, 1,478 dependency
occurrences, 69 compiler tool files and 20 actual linked libraries. The older
997-pin context-borrow preparation epoch remains separately frozen: the equal
pin count does not make it the accepted process-cell epoch.

The complete provider/header closure contains 288 roots, 963 files and 2,039
quoted-include edges. Fresh compiler dependency capture adds 714 actual files.
Working and Git preimages and compiler tool files were retained before edits.
All 80 actual primary App objects were separately retained, including the 11
selected objects. App baseline compilations use the actual App context; the
dispatcher uses the actual Core context. Neither context is inferred from the
other target.

The later primary commit `29e2a04f1d1bb814e026685374bb5e0e60ad715c` changed two
gameplay-point Source files outside this packet's 963-file closure and four
edited C++ files. Both Git images and the patch are retained. The worker remains
anchored to the accepted process-cell epoch. Root's later Source1496 checkpoint
is not claimed as a worker freeze or build result.

## Source contract and measured layouts

The retained context borrows the canonical manager-publication reference already
used by `sound_lifetime()` and the permanent process statistics cell exposed by
`GameNativeStringProcess::allocation_stats_0109cefc()`. Its `noexcept` host getter
returns that exact retained context. The new deletion binding points to it.
Normal and fallback host shutdown drain while the context and process cells are
alive. Existing observer construction does not register an owner before the new
binding is installed.

Actual MSVC Win32 class-layout dumps, captured before and after the change, give:

| Layout | Before | After |
| --- | ---: | ---: |
| `NativeSingletonDeletionBindings` size | 188 | 192 |
| New `allocation_stats` tail pointer | absent | 188 |
| `GameSingletonHost` size | 264 | 276 |
| Embedded deletion table | 60 | 60 |
| Weak-owner pointer | 248 | 252 |
| VFS pointer | 252 | 256 |
| Observer pointer | 256 | 260 |
| Pending-registry reference | 260 | 264 |
| New retained two-reference context | absent | 268 |

All 47 old binding field offsets remain unchanged. Enlarging the embedded table
necessarily shifts four later host members; the report does not claim all host
offsets are preserved. Static assertions enforce the tail offset and new binding
size. The constructor copies the exact manager-reference pointer from host+4
into context+0 and the actual process accessor's cell address into context+4.
Binding+188, at host+248, receives the context address at host+268.

`D685E0` calls `delete_native_allocation_stats_base_00be2890` and `D685F4` calls
`delete_native_allocation_stats_00be2930`. Each receives the dispatcher entry's
already-popped owner, unchanged full `uint32_t` flags, and the mandatory retained
context. Publication is not used as the receiver or compared against it. Missing
context reaches the pre-existing `logic_error`. Both lifecycle callees preserve
their existing direct `BE27F0` implementation; `BE28E0` is not substituted.

All 77 old profile case bodies remain byte-for-byte identical as Source after
removing the two explicit additions. The existing raw manager pop-before-delete,
recount, flags=1 schedule, `CE3818` generic base path and `BD0220` cleanup/rethrow
remain unchanged. No callback, virtual-dispatch substitute, repair, retry, extra
free, publication write, cache predicate, report toggle or owner activation is
introduced.

## Complete machine and consumer accounting

The focused before/after compile covers whole `game_hosts_singletons`,
`game_hosts` and `native_singleton_destruction` objects. The host constructor grows
from 710 to 761 bytes, with an explicit retained listing and instruction diff.
MSVC exchanges its `this` and log register allocation; the audit records that
change and all zero stores rather than ignoring them. The only new zero store is
the new binding at host+248 before installation. The new seven-byte host getter
is `lea eax,[ecx+268]; ret`.

The dispatcher section grows from 3,281 to 3,377 bytes. In both new routes the
emitted instructions load context from binding+188, branch to the same existing
failure block on null, push the context, push DWORD `[EBP+0C]` flags, push EDI
(copied from the entry ECX owner), call the respective real scalar callee, and
join the existing cleanup. All old external relocation edges remain. The other
48 nonmetadata sections, including raw drain and failure/EH helpers, are equal.
Executable-section switch-table bytes are explicitly classified separately from
instructions; their presence is not hidden by partial disassembly.

Actual App compiler read tlogs identify eight consumers of the changed headers:
`game_hosts_singletons`, `game_hosts`, `game_native_lua_services`,
`game_observer_runtime`, `game_hosts_menu`, `game_main`,
`game_native_renderer_application`, and `game_native_vfs_application`.
The first two have pre-edit baseline objects. The remaining six baseline
compiles use the unchanged source at the same working path and immutable old
headers; compiler dependencies verify those old headers were consumed. No
working-source revert was used.

Across nonconstructor host/App sections, 28 changed sections contain exactly 50
instruction changes, all the measured member displacements or host allocation /
deallocation size 264 to 276. Other section bytes and relocation edges match.
Four of the six additional consumers have equal nonmetadata objects; the menu
observer offset and `game_main` host sizes account for the other two. Complete
before/after objects remain available, including ordinary code, EH data and all
relocations.

Each before/after provider graph retains 288 whole objects: the three focused
objects, 283 complete Core archive members and two complete App providers. All
root code sections are included; explicit stats constructor/lifecycle analysis
seeds are retained dependencies, not evidence of runtime activation. The graphs
contain respectively 3,690 / 3,692 reachable sections and 10,030 / 10,038 edges.
The 2,627 frontiers in each phase are explicit real external or weak boundaries;
no unresolved BSP provider is silently stubbed.

## Normal build and qualifications

`./scripts/build.ps1` completed successfully. The existing `reconstructed_math`
and `tool_tests` checks passed (2/2); no new tests were added. This worker has no
configured native differential check and did not regenerate seed headers. The
accepted primary epoch's 3/3 result is separate historical evidence.

The normal build's complete library, executable, map, test log, project files and
tlogs are retained, together with 407 Core objects (selected/provider union) and
all 80 actual App objects. The three focused candidate objects match their
normal-build counterparts outside recorded physical metadata. The genuine stats
constructor/lifecycle/process objects retain their established nonmetadata code.

The map now selects the process stats-cell accessor and the two scalar lifecycle
symbols. `/OPT:ICF` folds the identical `BE2890` and `BE2930` implementations to
the same VA `10332eb0`; their separate genuine COFF symbols and dispatcher edges
are retained. The raw stats constructors and new host context accessor do not
appear in the linked map. Link selection is not startup execution evidence.

Primary-to-worker physical bytes are not equal: exact COFF timestamps and debug
paths differ, and recompilation also changes anonymous/lambda names and RTTI
spellings. The qualification record gives explicit bijections for 225 lambda
pairs and 252 RTTI names, without discarding machine instructions, relocation
offsets/types, graph edges or EH membership. After these recorded qualifications,
485 of 487 comparisons match, including all 407 Core objects and all eight
affected App baselines.

The two remaining comparisons, `game_hosts_lua` and `game_hosts_units`, contain
respectively two and six unreferenced byte-identical duplicate lambda helpers
without a unique declaration pairing. Both lie outside the eight actual header
consumers. Their complete objects and differences are retained, with no invented
pairing and no all-487 equality claim.

## Replay and evidence boundary

The complete physical inventory covers 1,330 object occurrences and 986 unique
physical objects totaling 71,180,166 bytes. Every physical byte has disjoint
coverage, and there are zero gaps or unclassified executable bytes. Each object
retains headers, raw sections, complete primary/AUX symbols, exact relocation
indexes, instruction listings and embedded switch data. Ordinary type32 function
symbols are separate from EH/unwind/catch symbols and compiler labels; symbol
counts are not claimed as nonoverlapping function extents.

All helpers, immutable preimages, raw/full object dumps, indexes, graph records,
compiler logs and audit records are retained under
`local/cc12_allocation_stats_retained_host_binding_source/`. The report pins them
and records evidence-parser corrections. `freeze_evidence.py` creates an immutable
`frozen/` tree, `bundle_manifest.json` and `evidence_bundle.zip`, including these
six exact repository outputs. `verify_evidence.py` replays local hashes, Git
preimages, library-member slices, complete COFF indexes, graph edges and audit
results; it then checks every bundled payload. The post-commit receipt binds the
completed replay and bundle to the exact six-file commit.

This packet establishes Source implementation, MSVC Win32 object behavior and
the stated existing worker checks. It performs no Native image query/read/run,
Ghidra mutation, GPR mutation, new Native ABI/SEH/OS check, live game validation,
statistics owner creation, startup activation or policy-level behavior change.
Primary review and integration use the integrator's current checkpoint.
