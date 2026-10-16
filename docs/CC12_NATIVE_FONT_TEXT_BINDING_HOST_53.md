# CC12 native font/Text host binding 53

`GameFontHost` now exposes its existing canonical native resource collection
through one `native_fonts()` export, followed by one admitted
`bind_text_factory(GuiTextRuntimeFactory&)` attempt. It does not activate native
Text or construct a factory. The normal six-font/sprite path keeps its existing
load, resource borrowing and retirement behavior.

## Persistent export gap

The host stores `unexported`, `export_pending`, `bind_entered` or `bound` in its
existing persistent Impl, alongside the sticky `native_failed` latch. Resource
export requires completed descriptor production, the producer's ready state,
and no failure or retirement attempt. It sets `export_pending` **before** calling
the producer's irreversible `native_fonts()` and caches that exact returned
owner address. Any admitted export exception sets `native_failed`; it cannot
reset or replay.

The unresolved state remains present while the caller constructs the future
real Text application/factory. An exception between export and bind therefore
cannot leave a seemingly ordinary ready font host eligible for cleanup.

The first bind is admitted only from `export_pending`. It sets `bind_entered`
before checking exact native resource-owner identity and zero retained factory
allocations. Those accessors only observe the supplied live factory. The
producer independently repeats its existing exact-resource/empty-factory
checks and retains that factory pointer. Only its successful return advances
the host to `bound`. Every admitted bind exception, including foreign-owner or
nonempty-factory rejection, sets the sticky failure latch. There is no retry,
cancel, reset, replacement or invented empty factory.

| Request/state | Result |
| --- | --- |
| Export before complete descriptor production | Reject before producer effects; existing state unchanged. |
| First ready export | Pending before producer export; unresolved until successful bind. |
| First bind with foreign resources or retained storage | Entered, then failed; no producer binding or factory mutation; never retry. |
| First bind with exact resources and empty actual factory | Producer binds the same factory; host becomes bound. |
| Bind before export, repeated export/bind, replacement, or API after retirement | Reject during admission; otherwise healthy phase remains unchanged. |
| Retire during pending/entered state | Reject before entering the producer retirement or latching a retirement attempt; interruption remains. |
| Normal unexported or successfully bound retirement | Existing retirement body; the bound factory must be alive and empty. |

Bind the exact factory before it creates any Text borrower. No second factory
may borrow the exported collection. This API checks the font-resource/factory
association; exact registry publication-cell identity and the other canonical
Text providers remain the composition caller's obligations.

## Existing parent containment and lifetime

`native_operation_interrupted()` includes pending, entered and sticky failed
states. It continues to exclude ordinary ready fonts and a valid binding
awaiting normal retirement. The predicate reads host/producer metadata only;
it never follows a VFS or bound-factory pointer. Successful retired metadata also
remains non-interrupted.

The existing `GameStartupHost` guards were reviewed without editing them:

- Its destructor checks font interruption first, before frontend destruction,
  font retirement and shared drain (`game_hosts.cpp:1849`).
- `application_shutdown()` checks first, before shutdown effects and device
  release (`game_hosts.cpp:2827`).
- `destroy_singleton_lifetime_manager()` checks before font retirement and the
  renderer's shared raw-manager drain (`game_hosts.cpp:2934`).
- `retire_native_fonts_before_drain()` catches retirement rejection and calls
  the interruption guard, then exits even for an unknown future preflight
  failure (`game_hosts.cpp:1793`). The guard decides before diagnostics, wraps
  diagnostics, and calls `_Exit(1)` rather than unwinding the native graph.

Thus an exported-but-unbound or failed host reaches process retention before
the parent closes fonts, VFS, renderer, manager or CRT state. This does not add
an exception-recovery path or prove arbitrary native SEH behavior.

For successful composition, destroy all Text/sprite borrowers, keep the SAME
factory alive while retiring fonts, then perform the shared native drain while
retaining the font producer, registry descriptors, VFS and renderer metadata.
Future Text/effect metadata must survive the drain and close in its reviewed
owner order. Only then may the graph be destroyed. The host's member ordering
still destroys its producer before its registry. None of this packet changes
the parent's normal retirement/drain order.

## Registry publication remains unexposed

`registry()` continues to return the same borrowed member registry. No new
publication pointer cell or singleton-manager adapter is provided. The existing
`get_font_registry_007371d0` null branch allocates a heap registry, records its
private `publication_` slot, and registers deletion through
`SingletonLifetimeManager`. Registering this host's member through that path
would invent ownership and risk deletion outside the host's lifetime. The
current Text factory also has no accessor attesting the registry-cell identity.

A future separately owned binding profile may borrow an already nonnull
publication of this exact registry, with stable cell lifetime, identity checks
and no allocation/registration of the member. That requires factory/binding
transport and ownership review outside these two files. The current legacy
`SingletonLifetimeManager&` requirement is not evidence of a canonical actual
manager provider. A throwaway cell or fake manager does not close this contract.

## Evidence boundary

This is a new Source host bridge over existing font producers, not a newly
recovered Original body or entry-point/SEH/ABI replacement. No Ghidra, ledger,
`game_hosts.cpp`, `game_main.cpp`, CMake, game installation or audio/settings
changes are made. Existing producer limits for AD4C30, AD53A0 and destructive
reload remain unchanged.

Verification consists of the bounded state/call-order review, independent
primary Source review, mechanical comparison of unchanged normal-path bodies,
and the required Win32 build with existing tests. Exact inputs and results are
recorded in `reports/cc12_native_font_text_binding_host_53.json`. No dummy factory
or new runtime test was introduced. Successful real native Text-factory binding,
factory-constructor failure at runtime, native Text drawing and gameplay are
not claimed by this packet.

The Win32 Release build passed with this API compiled in the existing host
translation unit. All three existing CTests passed in 9.64 seconds. Source input
hashes were rechecked after the build; these tests do not execute the new
factory-binding/failure protocol.
