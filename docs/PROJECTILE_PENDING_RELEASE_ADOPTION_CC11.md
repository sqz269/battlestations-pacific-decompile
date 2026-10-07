# Projectile expiry adopts the pending-kill producer

Packet `cc11_projectile_pending_release_adoption`, base `bd64c7aed`.
Addresses: `006E6490`, `00926D90`. The new concrete
`NativeProjectilePendingEntityServices` connects the existing projectile tick's
required release service to the already complete borrowed-storage pending-kill
body. This is conditional SOURCE adoption with required actual contexts; it is
not an original binary replacement or a game host binding.

| Native scope, end exclusive | Coverage in this packet | Original ABI |
|---|---|---|
| `006E6490..006E65B5` | Only the existing expiry/release service connection; other tick/math behavior unchanged | ECX tick node, stack float step, `RET4` at `006E65B2` |
| `00926D90..00926E7E` | Complete ordinary producer reused unchanged from `native_pending_entity_producers.cpp`, 79 instructions/238 bytes | ECX actual entity, stack raw DWORD cause, `RET4` at `00926E7B`; no defined return |

The native caller loads node `[-8]` at `006E6598`, notifies event0/value0 through
`00696350` at `006E659F`, then independently reloads the SAME cell at
`006E65A4` and calls `00926D90` with literal2 at `006E65A9`. The existing
`NativeProjectileObserverTickHost` preserves these separate reads. Neither its
source nor the observer/tick providers are changed here. A notification may
change which live entity the release receives; the new service does not cache
the expiry identity or translate it into a semantic unit.

## Required borrowed context

The final service stores only `NativePendingEntityOwners&` and
`NativePendingEntityProducerAccess&`. `observed_prefix(identity)` returns the
identical-address `NativeObserverOwnerStorage` alias, with no field read,
lookup, allocation or callback. The admitted pointer already denotes live,
mutable actual entity storage whose observer prefix is at offset0. This cast
does not create prefix storage or establish its lifetime.

`release_projectile_00926d90(identity,code)` directly calls
`native_pending_entity_kill_00926d90(owners,const_cast<void*>(identity),
static_cast<uint32_t>(code),access)`. Win32 `int` and DWORD sizes are asserted;
the conversion preserves cause bits modulo2^32. There is no guard, callback
substitute, additional allocation, exception interception or synchronous free.

The caller must bind these SAME actual contexts:

| Context | Required contract |
|---|---|
| Adjacent `00F899A8/00F899B4` owners | Actual initialized live raw headers and finite valid sentinel rings, not private queues or shadow counts |
| `resolve_entity` | Pure mapping of the SAME identity to live parent `3C`, sibling `44`, child `48`, flags `5F/60` and cause `70` lvalues; no sidecars or copies |
| `lock_owner_009248d0` | Complete actual getter, valid owner and captured section lifetime; native null section is allowed, null owner is invalid |
| Current entity slot `+70` | Required actual target receiving literal1 after killed flag/cause publication; no universal default |
| Children and source field changes | Actual identities with fields alive through each recursive call and fresh sibling reload |
| Queue node/count operations | Existing proved canonical `00924B10/008665F0` allocation and `009267F0/008675E0` count bodies; retain their failure behavior |

The reused kill body acquires its captured lock even when the killed flag is
already set. It reads destroyed `+60` before writing killed `+5F`, conditionally
writes the cause and invokes current virtual `+70(1)`. Cause7 becomes stored2
and receives its separate post-callback store, while the ORIGINAL cause reaches
direct recursive child calls. Fresh siblings are read after those calls.
Children enqueue before their parent. Allocation precedes count growth and
captured-head/current-previous link publication. These operations remain in
the existing body; the service introduces no alternate schedule.

Actual global initialization/CRT registration, drain, current virtual profiles,
pending getter, entity/world/observer lifetimes and arena bindings remain
required and unbound. Invalid/null/freed fields, malformed rings, hardware
faults and native private FH3/SEH are not supplied. Existing source exceptions
retain the producer's captured-section cleanup and lack of flag/list rollback.
The source service has a new C++ ABI and no native vtable installation.

## Focused source verification

One ignored fixture adapts the existing connected tick/observer case. A, B and
one child have actual-shaped storage with prefix0 and static assertions for
all producer offsets. Real observer registration and unregister suppress a
pending callback; event0/value0 on A rebinds the SAME node cell to B. Release
then runs the real pending-kill body on B/cause2.

The fixture's REQUIRED SOURCE virtual70 observation checks B/literal1/current
table tag and published killed flag/cause, then calls the existing real
`native_pending_entity_destroy_00926c80`. An explicit SOURCE child `+78`
observation declines its destroy-recursion edge. The child starts destroyed;
kill still visits and enqueues it without changing its existing cause. Real
canonical allocation/count/link bodies produce destroy `[B]` and kill
`[child,B]`. The fixture checks flags, rings, preserved owner words, observer
interval restoration and both real recursive-section depths. Canonical queue
cleanup frees nodes/sentinels and leaves payloads alive.

This proves the connected SOURCE path. Its virtual/lock-getter/profile/tick
observations and source CRT allocation are instrumentation, not recovered
actual game providers, runtime reentry or native-global registration proof.
Existing producer native/source differential coverage is recorded in
[NATIVE_PENDING_ENTITY_PRODUCERS.md](NATIVE_PENDING_ENTITY_PRODUCERS.md) and
[pending_entity_library_reuse.json](../reports/pending_entity_library_reuse.json);
this packet does not rerun or extend those native cases.

Fresh adapter, pending-producer, tick and event TUs compile with MSVC Win32
`/W4 /WX`. The manifested probe passes, support archives retain pinned hashes
before/after, and compiled alias/release/tick loads match the stated schedule.
The detailed receipts and native direct/indirect call checks are in
[projectile_pending_release_adoption_cc11.json](../reports/projectile_pending_release_adoption_cc11.json).
Root owns CMake registration, full build and independent validation. Landing
queue/lower/composite/holder audits remain separate unfinished work.

Primary integration bf95ff29f053dd139c37f52690fba901c5c9de91: full MSVC Win32 and all three existing CTests passed. The reviewed focused fixture passed an independent fresh actual-main-TU manifested compile/run, with all 3 main link-input hashes stable before/after linking. PE014C and resource24/id1/asInvoker verified. Native direct CALL rows verified with zero failures. This remains conditional SOURCE adoption with the provider, lifetime, historical CRT/FP, native fault, original ABI and game limits above. Executable SHA256 34b5050b7831097e592c17d5d821da13577da000c713cc54c54298b1d8d33ccc. Probe local/cc11_pending_root.cmd; build log local/cc11_capture_pending_integrated_build.log.
