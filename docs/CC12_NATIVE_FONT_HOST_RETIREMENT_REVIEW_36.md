# Normal-host native font retirement review 36

Read-only review of the normal-host adoption based on `1f3bc258e116cadc3601c94af230bed5045f4bfb`.
The inspected live integration tree was `J:/PROG/battlestations-pacific-decompile-cc12_resume_integrator`.
Source references below use that base unless explicitly described as a proposed change.
This review changes no source, ledger, annotations, or original program.
The live uncommitted adoption was also reviewed below; the verdict applies to
the recorded source blob hashes, not to uninspected later edits.

## Finding and scope

For this packet, the existing sprite `GameTextHost` will borrow native font data and
the existing COM sheet from the same `FontRegistryStartup`; it does not install
`GameNativeTextApplication` or `GuiTextRuntimeFactory`. The earliest valid font
retirement point is after the menu/frontend and every sprite font borrower are
gone, while the canonical renderer is ready and before its raw singleton drain.
Keep the retired producer, registry, renderer contexts, and VFS contexts alive
through that drain; destroy the producer before the registry afterward.

The base code is not yet safe for replacing semantic resources with native ones:
the destructor deletes the frontend at `src/game_hosts.cpp:1845`, drains native
singletons at 1851, and only deletes the font host at 1879. Native font retirement
must be inserted before that drain. Normal shutdown already deletes the frontend
at 2853; insert retirement immediately afterward, before `device_->release()` at
2855. These are integration recommendations, not claims that the base implements them.

## Exact source contracts

| Source | Established contract |
| --- | --- |
| `game_native_font_application.cpp:32-74` | Producer retains references to the same registry, raw VFS bindings/strings/publications, renderer cache/grid, and actual owner registry. Domain validation reborrows ready section-layout services. |
| `game_native_font_application.cpp:94-101,139-142` | A stable operation frame is retained before native image effects. An entered production failure retains its frame and exception. |
| `game_native_font_application.cpp:157-175` | One initial load attempt after entering loading; entered failure becomes failed. No reload/replay fallback exists. |
| `game_native_font_application.cpp:190-210` | Retirement accepts fresh/ready only, validates borrowers/domain, destroys completed font owners forward, clears each exact registry binding, and latches exceptions after entering retiring. |
| `game_native_font_application.cpp:150-151,222-224` | The producer destructor terminates if retention is required. Successful ready production also requires retention until explicit retirement completes. |
| `font_registry_startup.cpp:22-29,95-111,144-155` | Descriptor owners have stable addresses and are appended before the callback. Rows borrow native resources. Retirement must clear those exact bindings before registry destruction, which otherwise terminates. |
| `native_font_resources.cpp:49-67,82-99` | Adoption transfers the sole decoded allocation and returned GFX/alpha references. Retirement clears glyph payloads, then releases/clears the owning GFX/alpha slots; an interrupted destroying phase cannot be replayed. |
| `game_native_renderer_application.cpp:629-631,871-891` | Font domain validation requires ready renderer services; raw drain changes renderer phase and destroys renderer/resource publications. Retirement after drain cannot validate its domain. |
| `game_hosts_singletons.cpp:162-170` | Shared drain destroys the canonical raw manager and then retires the native VFS runtime. A still-allocated `GameVfsHost` after this is insufficient to authorize another native font operation. |
| `game_hosts.cpp:1047-1052` | `GameDeviceHost::release()` clears overlay/capture and its borrowed device pointer. It does not release the canonical COM device; the renderer retains that through native drain and final destruction. |

## Required call order

Normal shutdown and exceptional startup destruction must converge on the same
once-only font closure before any raw singleton drain:

1. Stop frames/native callbacks/reset/recreation and remove overlay/capture
   callbacks which capture the frontend. The destructor already clears overlay
   at 1843; use equivalent callback quiescence on normal shutdown.
2. Finish summary reads while frontend/menu objects remain valid; destroy menu
   first because it borrows the frontend. Destroy frontend and null its pointer.
3. Invoke a host-level `retire_native_fonts()` wrapper once. Its completed state
   must make later shutdown/destructor closure a metadata-only no-op.
4. Release the device bridge; perform the existing diagnostic/platform and raw
   singleton teardown. Font producer and registry metadata remain allocated.
5. After drain, destroy the font producer before the descriptor registry. Then
   close scripts/VFS/renderer contexts according to existing owner order. Native
   resource destruction is now a no-op because each owner is already retired.

`destroy_singleton_lifetime_manager()` at `game_hosts.cpp:2868` is another raw
drain entry. It must invoke/check the same closure prerequisite; it must never
retire fonts while frontend/menu borrowers survive. Repeated calls after normal
shutdown may observe completed closure, but must not replay failed retirement.

The sprite borrowers are concrete: frontend `Impl` owns `text_host`, cached
`GameTextRun` values, and bridge quads (`game_hosts_frontend.cpp:196-201`). Each
run's `sheet` is borrowed (`game_hosts_text.hpp:86`), and bridge quads copy that
borrow (`game_hosts_frontend.cpp:783-793`). The frontend's `owned_textures` list
is its separate image ownership (`216-219`); do not add a font sheet to that list
or release it as though the sprite adapter owned the native font reference.

## Failure and destructor requirements

Do not use the producer's `requires_process_retention()` directly as an early
exit condition for every successful startup: ready native fonts intentionally
return true until they are retired. A host state must distinguish a complete
load awaiting safe retirement from an interrupted load/retirement. A suitable
monotonic state sequence is fresh -> loading -> ready -> retiring -> retired,
with failure latched from an attempted operation. Only the successful retired
state is an idempotent closure result.

Latch the host attempt before invoking load or retirement. Producer preflight
checks occur before its internal try/phase transition (`157-160`, `192-198`);
an exception there can leave its internal phase fresh or ready. A host must not
interpret that as permission to repeat an attempted operation after failure.
Record the failure and retain all providers before any frontend deletion,
partial-owner destruction, shared drain, or automatic member teardown.

The established game-host policy is log, close, then `std::_Exit(1)` for an
incomplete native graph (`game_hosts.cpp:1760-1766` and analogous guards). Apply
that policy to interrupted font operations, including retirement preconditions
and domain validation. A noexcept destructor helper must catch failures and
take that retention exit rather than unwind into producer/registry destructors.
Do not clear native associations manually, roll back the successfully loaded
prefix, free a failed frame, or invoke retirement again after a failure.

A later unrelated C++ startup failure, such as fingerprint/frontend setup after
native load completed, does not alone make that completed font operation failed.
If every borrower can retire and the same renderer/VFS domain remains ready,
the fallback destructor may perform the one normal font retirement before drain.
Keep host allocation assigned to its owning startup pointer before entering load
so startup failure cannot lose the producer/registry retention root.

In `GameFontHost::Impl`, declare the producer after the registry (reverse member
destruction), or explicitly reset it in a destructor after completed retirement
while the registry is still alive. Never rely on the native resource owner's
destructor to clean up a ready or failed producer after renderer drain.

## Future native Text integration

The current frontend creates only `GameTextHost` (`game_hosts_frontend.cpp:696-700`).
The additional `GameNativeTextApplication` definitions in `game_hosts_text.cpp`
are not an installed normal-frontend factory at this base. Therefore this packet
must not export `native_fonts()` or bind a throwaway factory merely to authorize
sprite retirement.

When a native Text factory is actually installed, the required order changes:
retire all native Text/material borrowers, release completed flags0 storage,
verify the SAME bound factory's retained allocation count is zero, call font
retirement while that factory is still alive, and only then destroy the factory.
Producer `bind_text_factory()` checks identity before construction (`183-188`),
and retirement dereferences it (`194-196`). Factory destruction with retained
allocations terminates (`gui_text_runtime_factory.cpp:137-138`); completed flags0
storage still remains counted until `release_completed_storage()` (`407-440`).
Deleting that future frontend/factory before font retirement would be invalid.

## Integration checks and evidence boundary

The inspected live adoption satisfies this sprite-only lifetime contract at
source level, with no remaining blocking lifetime finding in these four files:

| File | Inspected Git blob hash |
| --- | --- |
| `include/bsp/game_hosts_fonts.hpp` | `778dd15a8d63489fd599dfca87ebe2243ac0587f` |
| `src/game_hosts_fonts.cpp` | `5445846b9d98fd3911a1dfd407a704f78ac00fe1` |
| `include/bsp/game_hosts.hpp` | `b2c385f63616e482b4c6e284a022bfb162939e7a` |
| `src/game_hosts.cpp` | `1c4b7f81718a45ea1a87fbd483b2bc3e1505f52e` |

The live host rejects loading after failure/retirement, catches preflight and
post-production exceptions into `native_failed`, and checks complete sole
producer/registry associations. Retirement latches its attempt before callee
preflight, verifies cleared rows, and marks success for metadata-only repeats.
Its interrupted predicate has the required truth table:

| Native/host condition | Interrupted guard | Authorized closure |
| --- | --- | --- |
| Fresh, never entered native load | false | One empty retirement while renderer ready |
| Successful ready load, not retired | false | One retirement after sprite borrowers close |
| Successful retired producer | false | Metadata-only repeat; no renderer validation or native replay |
| Load/preflight/post-production failure | true | Retain graph and no-unwind process exit |
| Retirement attempted but unfinished, or any latched failure | true | Retain graph and no-unwind process exit |

`FontRegistryStartup fonts` precedes `GameNativeFontApplication native` in the
host fields, ensuring producer destruction first. The startup guard runs before
teardown in normal shutdown, exceptional destructor, and explicit manager drain.
Normal/fallback closure runs after frontend destruction and before raw drain;
the direct manager-drain helper rejects surviving frontend ownership. Normal
shutdown now clears the overlay callback before deleting the frontend. Callback
and frame serialization remains the documented host precondition; this review
does not establish concurrent callback safety.

Source checks needed when accepting the adoption:

- `GameFontHost` owns one registry and one producer; native rows use that same
  registry and no semantic `FontResources`/second glyph map is populated.
- Sprite font data/sheet lookup accepts the actual live native association,
  preserves data/glyph identities, and borrows the validated actual COM sheet
  without casting a logical native texture pointer to a COM interface.
- All normal/fallback/raw-drain entries close fonts before renderer/VFS drain,
  after sprite borrowers, with no second native retirement after success/failure.
- A failed/partial load or retirement exits before automatic destruction/drain;
  later unrelated startup failure can still retire a fully loaded font set once.
- Final `GameFontHost` member order destroys producer before registry; retired
  registry rows have both `resources == nullptr` and `native_resources == nullptr`.
- Run the required Win32 build and relevant existing checks after source changes.
  A normal-host run and font rendering/capture observation are separate evidence.

Performed here: bounded current-source inspection and call-order review only.
No build, test, gameplay run, screenshot, native ABI probe, or failure injection
was performed by this review. No new tests are proposed for the document change.
The parent integration's build/runtime results must be recorded independently.

This source composition preserves successful ownership/resource identities;
it does not recover AD4C30's original per-field DAT stream timing, native
string/tree/sentinel allocation, SEH/unwind, or AD51D0 destructive reload.
AD53A0 remains typed payload/GFX/alpha retirement, excluding its native tail.
The normal sprite draw remains a bridge rather than native Text/material render
submission. Original ABI and game validation are not established by this review.
