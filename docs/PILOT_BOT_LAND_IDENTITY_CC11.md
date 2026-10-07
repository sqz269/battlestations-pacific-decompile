# Land task canonical retained identity, cc11

This packet corrects the conditional SOURCE interfaces for the existing land
hook, cruise and validity bodies. All three now borrow **one canonical
`const void* volatile` task+404h identity cell**. Plane+3FCh is a separate
borrowed cell. Required pure mappings derive live entity/hook/cruise views at
each observation; no translated view pointer is stored in task+404h. This is
source reconstruction, object/fixture checked, unbound to a native task arena,
profile, owner scheduler, observer lifetime or running game.

Previously hook carried a copied hook-view pointer, cruise borrowed a
cruise-view-pointer cell, and validity borrowed an entity-pointer cell. Those
three interfaces could not represent the same native retained slot without
assuming synchronization of independent caches. The source now admits one
caller-owned identity slot. Every view of one task must reference that same
slot; an installed task and an older retired task require distinct live task
storage. The API borrows storage and does not enforce or create its owner.

## Native coverage and original ABI

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, image base `00400000`. BSP client target verification ran
before the live prototype/call checks. Workers made no Ghidra writes.

| Complete native body, end exclusive | Original convention | Existing source body |
|---|---|---|
| `009B33F0..009B3434` | ECX=task, RET; saves ESI; successful exit can tail JMP | `land_task_retained_hook_009b33f0` |
| `009B3C60..009B3CE6` | ECX=task, RET; saves ESI | `land_task_update_cruise_profile_009b3c60` |
| `009B34D0..009B3552` | ECX=approach, RET; saves EBX/ESI | `land_approach_validate_site_009b34d0` |
| `009B3560..009B35DD` | ECX=task, EAX=2/1/0, RET; saves ESI | `land_task_is_command_current_009b3560` |

These C++ signatures are not original binary entry points. Names describe
observed behavior; they are not recovered original symbols. Earlier complete
body evidence remains in [the retained-hook audit](LANDING_TASK_RETAINED_HOOK_CC11.md),
[the cruise audit](PILOT_BOT_LAND_CRUISE_CC11.md) and
[the validity audit](PILOT_BOT_LAND_COMMAND_VALIDITY_CC11.md).

## Required mappings and preserved observations

`squadron_entity(identity)`, `landing_hook_view(identity)` and
`cruise_profile_view(identity)` are REQUIRED PURE source mappings. For each
admitted nonnull identity they return borrowed storage belonging to that same
live squadron. They may form reference views and resolve source object
identity. They must not read represented native field values, invoke callbacks
or native calls, allocate, publish defaults, or maintain translated-pointer
caches. Their returned references must remain valid through the corresponding
native segment. Mapping an unsupported/stale identity is outside the admitted
domain, not permission to synthesize a receiver.

Hook captures task+404h at `009B33F3` once for both the null guard and first
virtual+174h probe. If that token is nonnull it reloads at `009B340B` for the
second probe; an interned land token returns. Otherwise it captures the final
identity at `009B3422` once for the null guard and the `007EFB60` tail route.
Only that final identity is mapped to landing flag fields. Both clears still
precede plane facts and deferred BE enqueue. No current plane membership
lookup, dequeue, synchronous promotion or retired-array drain is introduced.

Cruise still observes retained plane+3FCh for the actual leader predicate.
Each actual tuning call precedes a fresh load of canonical task+404h, followed
by a pure field mapping. The first mapped view cannot substitute for the second.
Both tuning calls, blocked/reset behavior and strict ordered-negative tests
remain intact. The primary pointer COMISS and FLD/dirty-byte/FSTP kernels are
unchanged.

Validity captures the initial receiver for its null guard and first token
probe, then separately observes task+404h for the second token, descriptor
probe and admission. Mutable returned-descriptor resolution remains sequenced
before the fresh target+428h comparison. A null admission identity is passed
to the required admission provider as null without invoking a mapping; this
preserves `006C4790`'s null refusal. No later receiver null fallback is added.
Result2 still means initial null cache or first null token; a newly null second
token produces0. Block/target/queue clears, including both observed block
stores, remain volatile publications. The descriptor x87 copy kernel is
unchanged.

The pure mappings add no native observations. Required actual token,
descriptor, resolver, block-owner, owner-byte, admission, tuning and routing
services retain their existing contracts. Provider calls may replace the
canonical slot between segments within the source caller domain. Such fixture
replacement does not establish observed native reentrancy. No concurrent
mutation, structural reentry, corruption, stale mapping reuse, faults or
unwinding is modeled. Existing numeric domains still require masked exceptions,
no pending unmasked exception and an available x87 stack slot. Volatile provides
ordered source observations, not ownership, thread safety or atomicity.

## Focused validation

The actual changed `src/plane_squadron_host.cpp` compiled using MSVC
19.51.36244.0 x86, `/EHsc /std:c++17 /MD /O2 /DNDEBUG /Iinclude`.
The existing ignored hook probe linked the changed object with the primary
`bsp_core.lib`, `/MANIFEST:EMBED /INCREMENTAL:NO /OPT:REF`, and passed.

One linked scenario borrows the identical slot address in hook/cruise/validity.
Hook's required source token providers replace A->B->C; only C's landing flags
clear and its BE request remains deferred. Cruise starts from that C and its
second tuning provider replaces the same cell with D, writing the correct
mapped C/D fields. Validity starts from D and observes D->A->B->C through its
token/descriptor/admission calls. Pure mappings do not mutate or record field
observations. These are source fixture effects, not native runtime evidence.
Existing result2-versus0, mutable resolver, admission clear, old/new task
identity, deferred-route and numeric fixtures remain passing.

Text comparisons against the starting commit verified both primary numeric
kernel blocks unchanged. `git diff --check` passed. The live call verifier
checked nine direct/tail rows with zero failures; eight virtual rows remain
explicitly unresolved. Root owns the full Win32 build/integration. No worker
full build, game run, original ABI test or lifetime proof was performed.

## Connected next constructor boundary

The complete constructor can consume these canonical slots through REQUIRED
complete services, but the current generic `BotTaskRecord/BotTaskHost` and
Boolean `GameUnitsHost` landing fields do not provide actual retained storage.
The generic record omits task404/424/428/42C and its generic factory does not
perform the land control predicate/park selection with the actual subobjects.

| Dependency, end exclusive | Actual input/publication/unfinished boundary |
|---|---|
| `0099C6F0..0099C8A4` | ECX=task, stack owner/kind3, RET8. Actual base command construction uses owner+4 adjusted by-310h; constructs gun/control subobjects, random fields and descriptor-row values. Required complete base, not a zeroed projection. |
| `009F9CE0..009F9D78` | ECX=approach, stack plane/reference float, RET8. Copies plane/class/squadron/owner/descriptor row to approach4/8/C/10/14; clears18/1C/20; x87 speed ratio. No observer registration for retained plane or squadron. |
| `009AFE70..009AFFEB` | ECX=approach, stack plane/block, RET8. Calls common constructor; publishes block2C, block owner30, and complete006C0B50 result34. Independently reloads input plane9D4 for queue call. Tuning/random/pose/geometry arithmetic needs primary recovery before port. |
| `009B2E50..009B302B` | ECX=approach, stack plane/block, RET8. Complete lower constructor, registry, MoveTo/Follow and six land state constructors/profile stores, then eight state-name registrations. Requires actual state storage/entry contracts. |
| `009B3240..009B3307` | ECX=task, stack owner/block, EAX=task, RET8. Base first, fresh owner50 plane, full composite approach, exact profile publication; embedded plane72C virtual38 predicate; park620 or fresh-plane leader selection of4C4/500; state310 publication and state entry; bind last. |
| `009F9980..009F99A6` | ECX=approach, stack nonnull task, RET4. AFTER entry, binds18/1C/20 to actual task4/314/38C. These are aliases, not copied values. |

Existing `register_observer_pair_00694a60`/unregister and actual endpoint storage
can serve MoveTo+18h registration for its target+2Ch. Its callback
`009BDEB0..009BDECE` compares the observed object's virtual+4 result and clears
callback+14h (state+2Ch) on equality. This does not invalidate task404 or plane3FC.
Follow entry `009BED80..009BEE25` has actual tuning/reindex/leader observer work;
park entry `009B21A0..009B21BA` has stores; MoveTo entry `007B3DB0..007B3DB1`
is proved empty RET. Required entry services cannot replace these with no-ops.

Complete `006C0B50` returns an existing queue-record identity or block80 and has
queue/slot/observer/message side effects. Current `LandingQueueEntry` omits
native+4 record identity, and `landing_request_006c54c0` explicitly omits the
slot-tail transitions. That host projection is excluded as a provider. Queue
record lifetime, actual controller/world mappings, exact profile publication,
arena670 allocation/scalar destruction, constructor fault/EH cleanup and
death-time cache validity remain unbound. A conditional complete009B3240 caller
may require complete009B2E50/006C0B50 services; an actual game adapter must first
supply them. Source code producing all five identities itself additionally
needs primary numeric recovery of009F9CE0/009AFE70, then the connected lower and
outer constructor port. This packet does not manufacture that binding.

Primary integration: d314dc6e834a24e974b1a6f8d5aa3ca241aff171; actual main sources independently recompiled for the manifested focused probe, PASS. MSVC Win32 Release and all three existing CTests passed. Executable SHA256 b0b07345ff367f9e0e255a70d44fab8eed4d89f531281d55e463d2f633978cbc. Original ABI, runtime binding and game validation remain unclaimed. Build receipt: J:\PROG\battlestations-pacific-decompile\local\cc11_identity_d2_integrated_build.log.
