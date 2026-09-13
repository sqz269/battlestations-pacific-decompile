# Actual texture loading and cache

`native_texture_loading_cache` reconstructs the original D5F088 registry at
renderer+1A74. It uses the actual 0Ch vector header, 2Ch resource records, 10h
alias nodes, shared actual string pool, live VFS/date services and native +04
resource counts. There is no semantic texture cache, replacement renderer
vtable, retained-object map, or synthetic successful resource provider.

The complete B319B0 wrapper captures its ECX renderer separately from the
optional guard's current renderer, copies/lowercases its native name, and calls
B30B40 with the forwarded second word, acquire-new=0 and allow-load=1. The saved
post-resize name pointer and current length govern normal release; the live
mode governs guard exit. Entry-skipped/exit-enabled guard preimages are outside
the source interface, as in the existing actual synchronization context.

B30B40 pumps the actual load-event host, normalizes a copied name and scans
every alias. A matching null row does not end the first search. Current
registry+4 resolves the name; if the names differ, a second search compares
only the first alias and appends the requested alias before reading its
resource. Only then does allow-load gate a cold load. Even a null load gets a
real record, aliases, date query and append. Nonnull resources contribute their
current size to registry+10. B30130 uses the original signed minimum capacity
64 and doubling rule, actual B2FF00 reserve, and actual B2FC60 record copy.

B31C20 initializes fallback through B31BD0 only while registry+1C is zero.
That initializer writes `error.tga` at +14, publishes +1C=1 BEFORE BDF4C0, ignores
its Boolean, recursively calls B30B40(name,0,1,1), then stores the result at +20.
There is no retry/reset of that flag after failure. Resolution uses the same
captured actual manager and mutable eight-byte header. The original diagnostic
B31CB2 targets the verified RET leaf 4254B0; it does not emit a host log.
Each resolution now owns a distinct `NativeTextureNameResolutionOperation` in
the cache acquisition: one for bootstrap, another for the ordinary lookup,
and separate children in every recursive cache frame. The required factory
creates metadata only. Its result is published before capturing the current
manager and invoking the child. The integrator's holder owns its real
`NativeVfsNameResolutionAcquired`; a shared external scratch operation is not
an admitted binding. Factory failure and callback failure are both single-use.

Reference behavior must be read literally. Record assignment/copy/destruction
does not retain or release resource+28. Fresh B319B0 returns the loader's count
without an additional increment; hot hits always increment; fallback's fresh
flag1 adds one. Read-only B32370/B32090/B316C0 teardown evidence shows the
fallback reference and record-release calls. This module adds no independent
cache reference to reconcile those schedules.

The B2C2D0 source covers actual VFS open, stream validity, direct deletion of an
invalid stream, BEF750 conversion, source release, D3DX image info and the 2D
arm. It uses the real D3DX import contracts, current memory size/base, actual
B3F2B0 pool allocation, B3F930 named owner and B23640 retained-source assignment.
The first current renderer+1D84 read gates quality reduction; the second occurs
after the actual `detail.dds` temporary is destroyed. Width/height shifts mask
the count to five bits while mip subtraction uses the complete DWORD. The
device is captured before image-info and retained across an allowed retry.

The second B2C2D0 stack word is a nullable CODE TARGET, not a texture flags mask:
at B2C636 the original executes CALL EAX with ECX=completed texture and no stack
arguments. Zero skips it. Nonzero requires an explicit concrete body binding.
The B29670 retry path similarly requires its actual current-renderer binding.
Missing bindings throw at the reached boundary with the operation retained;
they never report a successful texture.

`NativeTextureLoadOwners` registers one companion in the application's existing
`GuiNativeGeometryRegistration`, borrowing the exact atomic word at native+04.
Its real terminal calls B3F590, which releases retained memory, performs B32250
name removal, unregisters/releases the native COM/surface/name state and returns
the pool slot. Only after that terminal completes is the same canonical binding
removed and its companion retired. Registration is transactional metadata and
does not retain. The static B3F2B0 pool binding must be the same pool supplied
to the owner context. All borrowed domains outlive their native owners.
Original B31DA0/B316C0 renderer teardown is not routed through this companion
domain by this packet; its canonical terminal composition remains required
before claiming complete renderer shutdown.

New actual-string overloads of B30510/B2F990/B2FC60/B2FF00/B31DC0 preserve the
existing semantic overloads. B32250 now has a compatible semantic constructor
and an actual-pool constructor, so temporary and record strings share one
provider. The reviewed parent surface bridge supplies that same provider to
2D surface/name operations. No pool is created by these adapters.

Caller-owned acquired frames publish the source stream, converted memory, COM
output, pool slot, completed creator, canonical registration, record publication
and caller increment at their respective boundaries. Recursive bootstrap has a
persistent nested frame. Frames cannot replay; failed frames must survive until
their actual resources are externally resolved. Destruction of an unresolved
frame terminates instead of silently losing ownership state. These host frames
do not implement native FH3 rollback. BEF750 itself has no acquired output for
its internal partial allocations; this packet preserves its input and returned
memory but does not claim recovery of that nested failure.

The acquisition also owns the wrapper, normalized, resolved and resolver-output
eight-byte headers, plus the pending record. B31C20 rejects a stack output or a
reused child before fallback/native header writes. B30B40 requires acquired
storage before any resolution, including allow-load=0; its optional interface
only admits a nonnull first-search hit. B319B0 records entry before its guard
and marks guard/copy/cache/leave failure, preventing a second native entry.
Normal and exceptional cleanup keep the original arming and release schedule.
Released buffers are marked consumed; their unchanged eight-byte headers remain
as diagnostic preimages until the child operations are destroyed. Those pointer
values must not be dereferenced or replayed as live string ownership. Headers
are declared before child members so their storage also survives child teardown.

Cube B3CED0 and volume B3CFA0 named-owner arms remain unreconstructed. Their
calls are inventoried in the report and explicitly excluded from source
coverage. D3DX image-info failure may leave native output uninitialized; that
input is also excluded, with memory retained. Create HRESULT is otherwise
handled by the observed policy: a nonnull COM output proceeds regardless of HR;
null output with the two excluded errors does not retry.

Evidence is `reports/native_texture_loading_cache.json`: 13 installed/live
matching spans, 4,944 bytes and all 163 call sites, including unreachable
BF6713 at B30BE7 after CMP EAX,EAX. The B30F3A saved-registry reload resolves the
pseudocode's bogus unaff_EBP; B319B0 RET8, B30B40 RET10 and B2C2D0 RET8 are checked
from assembly. Ghidra was read-only throughout. Native code-body annotations
remain hypotheses in the ledger until the primary performs its write pass.

Validation uses the existing Win32 build and a single ignored actual-pool
record fixture. The fixture's null row, ordered deep aliases and 64-to-65 reserve
survive source/old-array destruction, then retire the actual pool. It does not
exercise the full loader, GPU creation, Text, native ABI or game execution.
`scripts/build.ps1` passed with the source registered; CTest passed 1/1.
`verify_report_calls.py` checked 144 rows with zero failures. The remaining
19 symbolic/current-slot rows retain their observed register operands and
numeric profile or IAT identities. The ignored fixture runner is
`local/run_texture_record_probe.cmd`, linking the complete production library
with `/MANIFEST:EMBED`. The first build failed during concurrent CMake
regeneration after the source-list append; the stable rerun and final build
both passed. No compiler fix or dependency substitution was used for that race.

The acceptance correction was rebuilt with `scripts/build.ps1` (1/1 CTest) and
checked by `local/run_texture_resolution_failure_probe.cmd`. This single
extension uses the actual string pool and production call/name holders with a
deliberately throwing diagnostic provider. It verifies child publication,
retained consumed-header bytes during child destruction, separate bootstrap
ownership, and rejection of failed factory/invocation replay. It also reruns the
existing record-growth case. It is fault injection, not a successful BDF4C0,
full cache, native-FH3 or GPU execution test. The native loader diagnostic sites
are corrected to resize B2C30F and copy B2C327.
