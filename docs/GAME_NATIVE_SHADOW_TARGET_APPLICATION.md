# Canonical shadow target application parent

`GameNativeShadowTargetApplication` supplies the genuine `F8BBF0` policy owner
needed by the existing native GUI effect graph. It composes the established
`A8FE30` constructor over `GameNativeRendererApplication::shadow_depth_target_context()`;
it does not allocate another material manager, copy a target or create another
publication/registry. Current frontend attachment and actual effect execution
remain primary integration work.

## Evidence and caller boundary

The native owner is `2Ch`, constructed with ECX=actual allocation and two
stacked dimensions; A8FE30 returns the owner and `RET8`. Its established Source
producer writes final `D5B5E8`, dimensions and real renderer format-check results.
It leaves native padding unwritten. Its A8A980/A8A820 base path already publishes
F8BBF0 and registers the actual owner in the canonical AA0 manager.

Existing [APP_INIT_RENDERER.md](APP_INIT_RENDERER.md) and primary review of the
saved original `73DCCB..73DD04` qualify the application caller: read F8899D once
into DL, `NEG DL; SBB EDX,EDX; AND 800h; ADD 800h`, retain the extent in ESI before
BF681B(2Ch), capture allocation, skip construction on null, otherwise push the
saved extent twice and call A8FE30. Zero means 2048; every nonzero byte means
4096. F8899D is the canonical `GameNativeSettingsProcess::settings().data()+1Dh`
cell. The projected `GameSettingsBlock` is not read.

This packet implements a new typed Source application API. It does not recover
the entire 73D410 body, its register entry, native state16/FH3 cleanup or binary
ABI. The caller address remains separately leased; no caller annotation or
ledger mutation is made here. Constructor/provider/retirement Source is reused.

## Production and retained failure

Create and retain the parent metadata first, after real settings initialization
and load and renderer/device startup. Its C++ constructor only borrows and
authenticates the existing renderer context and canonical empty F8BBF0 cell.
Then call `construct()` once on that retained object.

The method captures the actual settings byte once and persists both dimensions
before entering `singleton_lifetime_allocate` with native and host sizes 2Ch.
Returned allocation is saved immediately. The established allocation boundary
retries CRT allocation/new-handler calls and throws on exhaustion; the original
null-result guard is nevertheless preserved. No target is substituted on null.

Before A8FE30 entry the parent records the allocation, stage and context
operation index. The native context attaches its retained operation before
native work. The same constructor performs the actual capability/+F8 checks and
AA0 publication/registration. On return, the parent verifies the actual returned
allocation, canonical publication, final profile and dimensions. It does not
call A8FF30, choose an enable byte or create shadow texture/surface resources.
The genuine policy+0Eh is already produced by A8FE30 for the effect consumer.

Any entered exception stores the exact `exception_ptr`, prior stage, actual
allocation and native-operation index. Existing native frames preserve their
section/publication/child state. A failed parent cannot be replayed or destroyed;
the caller must retain the entire heap graph, renderer, settings and services
under the established process-retention exit policy. `constructor_operation()`
exposes the actual native stage and exception state when that record was attached.
There is no rollback, early free, repaired profile or success fallback.

## Borrowing and retirement

`target()` borrows only after completion and rechecks that F8BBF0 still equals
the same returned pointer. It never rereads a freed target. A null result remains
null. The effect graph already borrows this same canonical publication cell.

Retain the parent through the original singleton drain. The renderer already
binds that manager's deleting-owner dispatch to this same shadow context; final
D5B5E8 uses A900C0, then A8FFF0/A8A9D0/A8A8C0 and the existing shared free path.
The parent does not unregister, clear the cell, decrement or delete the owner.
After native drain, `after_native_renderer_drain()` requires a null canonical
publication and the existing renderer closure checks before marking metadata
closed. Destruction is legal only before production or after closure.

## Verification and attachment

The owned TU strict-compiles for MSVC Win32 C++17 with `/W4 /WX /EHsc
/permissive- /O2 /MD /fp:strict`. Logs and object evidence are recorded separately
from the frozen sampler receipts under `local/cc12_native_shadow_target_application`.
No new tests, native executable, platform substitute or runtime fixture is added.
The registered-tree `scripts/build.ps1` build passes the existing two suites
(12.09 seconds). This new TU still requires primary registration and combined
linkage; its separate strict compile does not establish an activated producer.

Primary must register this TU and retain a parent at the real phase7 startup
point before native GUI effect acquisition. The focused actual-resource probe
can inspect the returned target's final profile/dimensions/policy and acquire
original `GuiFont.mshd` using the existing sampler/effect graph, preserving any
actual descriptor/compiler/sampler failure. The sampler complete-word knowledge
guard remains unchanged. Native Text/GUI draw, world rendering, gameplay and
original exception/register ABI are unproved.
