# Particle manager raw deletion and owner readiness

Evidence-only review at accepted Source `9f7365c7641b22ec31e5bec16f0495a1c4cf0d04`.
This packet makes no C++ change and grants no startup activation. The smallest
coherent owner proposal is six files, after acceptance of the separately owned
`AF06A0`/`AF0740` raw lifetime-access overload. Existing semantic
`SingletonLifetimeDomain` access cannot serve as the actual `01090AA0` authority.

The actual process publication must be one permanent, typed
`NativeParticleModelManagerStorage* volatile` cell for `00F8C274`. A retained
`GameSingletonHost` context must borrow that exact cell, the actual `01090AA0`
manager cell, and the live verified `00D7A24C` word. The actual raw deletion map
can then admit the two recovered profiles through the existing scalar bodies.
The Native early caller schedule, allocation and failure path remain a separate
readiness packet; the current late Phase 8 marker is not an activation substitute.

## Finite profile and scalar evidence

| Current profile first DWORD | Target | Retained complete body | Effect |
| --- | --- | --- | --- |
| `00D5D7EC = 70 08 AF 00` | `00AF0870` | 30 bytes, 11 instructions | Base destructor `AF0740`, conditional captured-receiver free |
| `00D5D7F8 = 80 10 AF 00` | `00AF1080` | 30 bytes, 11 instructions | Derived destructor `AF0B90`, conditional captured-receiver free |

Only the two first DWORDs and typed data/project metadata were queried fresh.
Both scalar listings, metadata, decompilations and byte hashes were reused from
saved evidence. No fresh `AF06A0`, `AF0740`, scalar body, adjacent profile or
`D7A24C` query was made. The complete earlier particle-manager report is retained;
its 32-byte table and four-byte constant records remain attributed to that epoch.

Both scalars capture `ECX` in `ESI`, call the corresponding destructor, test bit
zero of the stack argument, and call `BF65AC` on the captured receiver only when
that bit is set. They return that captured receiver in `EAX` and use `RET 4`.
The proposed ordinary C++ map must pass the full flags word unchanged. This is
an original-ABI description, not a claim that the Source interface is a drop-in
binary entry point. Neither scalar substitutes the current publication for the
receiver it destroys, frees or returns.

The retained PE is 12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
It was copied from the prior packet's frozen artifact, not reread from the game
installation. PE section/file placement is replayed for exactly the four spans
above. Typed metadata verifies `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, with modification number 49 unchanged. No Ghidra
annotation, prototype, listing, flow, function-body or project mutation occurred.

## Current Source and smallest proposal

The six current files below contain neither the `F8C274` authority nor the two
particle deletion admissions. This negative claim is restricted to those complete
files, not every Source file. The current access type still borrows a semantic
`SingletonLifetimeDomain`; the independently owned raw-access change is required.

| Proposed file pair | Proposed responsibility |
| --- | --- |
| `game_native_string_process.hpp/.cpp` | Append the typed `F8C274` cell and a nonallocating accessor to the intentionally retained process owner. Forward-declare the raw storage type. |
| `game_hosts_singletons.hpp/.cpp` | Retain one inline optional `NativeParticleModelManagerAccess`; explicitly bind it once from the canonical process cells and verified readonly data, and expose only its already-bound context. |
| `native_singleton_destruction.hpp/.cpp` | Append one borrowed context pointer and admit `D5D7EC -> AF0870`, `D5D7F8 -> AF1080`; preserve the existing missing-binding/unknown-profile failure. |

The file basenames above are under `include/bsp/` and `src/`, respectively.
The context uses the raw `SoundLifetimeAccess` overload after that dependency
lands. Its preparation validates the required data spans and first profile slots,
then emplaces borrowed references only. It must not call a manager getter,
allocate the raw receiver, publish/register an owner, allocate the context on the
heap, or read and cache the `D7A24C` value. The existing derived constructor's
`const auto one = access.one_00d7a24c` remains the value-observation point.

Binding occurs before any possible `AF06A0` registration. Repeated binding is
admissible only for the same typed process cell, same actual manager-cell domain,
same live constant reference and same installed context pointer. The existing
`uses_actual_storage()` and `borrows_same_domain(raw_cell)` expose the manager
domain identity; current pointer equality or two null values do not. Reject
retargeting or adoption of an unknown preexisting owner. A null publication alone
does not prove the raw manager registry has no particle owner, so initial binding
also requires a quiescent, unattempted constructor phase or equivalent established
ownership history. Do not reset or rebind while owners or callbacks remain.

Do not reinterpret a `void* volatile&` as the typed cell, keep a mirror publication,
or substitute semantic `FoliageGroupManager`. The model construction/lifetime
contexts currently request `void* const volatile&`; their later consumers require
a separately reviewed safe view of the same authority. They are outside this
six-file owner proposal.

The new deletion-context pointer would append after `allocation_stats` at the
current 192-byte structure tail. Offset 192 and size 196 are proposed Win32 values
to assert during implementation, not measurements from a build in this packet.
Growing this embedded structure shifts later `GameSingletonHost` members; no
unchanged host-layout claim is made.

## Drain, allocator and owner lifetime

The raw manager drain pops the stored receiver before reading its current profile,
passes flags 1, invokes the selected deletion route and recounts afterward. Pass
that receiver and all supplied flags to the admitted scalar without a null-skip
or comparison against current `F8C274`. The scalar destroys the captured receiver;
the base destructor separately unregisters the **current** publication using its
distinct manager observations and clears the publication after unregister returns.
Do not merge those observations or redirect unregister to the captured receiver.

The derived cleanup captures `+30` into a local for section release, cleans the
second array at `+10`, then the first at `+04`, then the base. These are weak raw
pointers: no pointee deletion/reference changes. Preserve the stale published
`+30` word and array backing pointers/capacities established by the existing bodies.

`singleton_lifetime_allocate` uses Source `std::malloc` with `_callnewh` retry/throw;
`singleton_lifetime_free` uses Source `std::free`. A future raw 34h caller allocation
must use that same Source CRT allocation domain and remain compatible with this
scalar free. Stack storage, semantic allocation, ordinary `new/delete`, and memory
from the Original process CRT are not admitted by this review.

`GameSingletonHost` explicitly deletes copying and declares a destructor, so it
has no implicitly generated move operations. Preserve that stable-address policy
for the inline optional context; explicitly deleted moves would be a possible
implementation clarification. The destructor calls `shutdown()` in its body,
before destruction of the context and other members. Normal shutdown drains the
actual manager before freeing it and clearing its canonical publication. Current
startup teardown also drains before resetting input/sound and deleting the host.
The canonical process/data owners intentionally outlive this host and CRT teardown.

If a host is destroyed or replaced while its manager registry still contains a
raw particle owner, the old host must complete that drain while its bound context,
canonical cells, verified `D7A24C` reference, receiver storage, arrays, critical
section and allocation domain remain alive. Context reset must not precede drain;
a replacement host cannot adopt an unknown surviving graph just because the
publication cell has process lifetime. Reentrant registrations/callbacks must
complete within that lifetime, without a conflicting concurrent mutation or a
callback after host retirement.

If deletion throws, the existing raw drain has already popped the receiver;
its catch clears manager storage with `BD0220` and rethrows. This does not establish
receiver release, publication clear or successful ownership handoff. An exception
escaping the host's implicitly nonthrowing destructor terminates. This proposal
adds no skip, repair, adoption, `_exit`, retry or recovery policy. Partial
construction profiles, constructor failure/free behavior and caller unwind must
be admitted by the separate early-caller review before activation.

## Reproducible evidence and remaining gate

`reports/cc12_particle_manager_raw_deletion_owner_readiness.json` embeds the frozen
manifest, exact claims, capture receipts and checks. Ignored local evidence lives
under `local/cc12_particle_manager_raw_deletion_owner_readiness/`. It retains 476
complete Git/working Source files reached from 13 selected roots, 13 extra complete
Source/Git inputs, eight scoped whole-file searches and 37 exact excerpts. The
885 resolved project include edges are replayed; unresolved system/SDK includes
are explicitly listed. The configured Lua archive is pinned but this closure
uses no Lua headers. No installed SDK or transformed build-header preimage is
claimed. The path/blob discovery index covers 4,206 files; file contents are
retained only for the selected 476, with no global corpus-negative claim.

Run `python local/cc12_particle_manager_raw_deletion_owner_readiness/replay.py`
for the artifact-only check. Its audit guard permits only retained packet reads
after interpreter imports and rejects writes, checkout reads, Git, subprocesses,
network and dynamic loading. A physically copied `portable_replay/` passes with
identical output. The separate `verify_git.py --check-working` matched 489 Git
blobs and 489 working files against the fixed accepted epoch.

The six-file binding proposal is ready for integrator review after the raw-access
dependency. No C++/CMake/startup implementation, build, test, new fixture, SDK/OS
change, Native/Lua execution, live game validation or ABI compatibility is claimed.
Activation requires the separately reviewed early Native caller schedule and its
allocation/failure/partial-owner/lifetime contract; moving or filling the late
Phase 8 marker alone cannot satisfy that gate.
