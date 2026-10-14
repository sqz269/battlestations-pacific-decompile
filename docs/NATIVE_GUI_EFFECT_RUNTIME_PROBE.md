# Retained actual GUI effect resource probe

`GameNativeGuiEffectProbe` is an opt-in resource probe after genuine renderer,
device, VFS, Lua/script, settings and input startup. It composes the accepted
canonical shadow target, sampler parent and GUI effect producer. It does not
construct Text, publish materials into the frontend, replace a native provider
or render a frame. No live run is claimed by this Source packet.

## Caller API and ordering

The constructor takes the actual `GameHostLog`, `GameNativeRendererApplication`,
`GameVfsHost`, `GameSingletonHost`, `GameNativeReadOnlyData`,
`GameNativeSettingsProcess`, `GameInputRuntime` and `NativePlatformLoadPublication`
borrowing the caller's existing platform/focus publication cell. Its constructor
creates heap metadata and borrows the existing
raw VFS services; it performs no native production. The caller must retain that
heap object before calling `run()` and retain all the supplied application
services throughout entered operations and shared drain.

Core commit `bf7e06b26` supports the renderer's existing const-qualified
`NativePlatformWindowFocusStorage` publication without another cell or receiver.
Primary passes `renderer.loading_platform_focus_publication()` when registered.
That actual partial storage has HWND+30h and focus+41h. The reached BECB20 path
reads +41h; this is a scoped load-pump receiver, not whole-platform storage or
binary ABI. The probe does not cast projected `Win32PlatformState` or copy the
focus owner. The core wrapper reloads the same borrowed cell once at load entry.

`run()` first retains a canonical `GameNativeShadowTargetApplication` and calls
its one-shot constructor. That captures actual raw settings+1Dh, allocates the
genuine 2Ch owner and uses the existing renderer shadow context for A8FE30's
publication, AA0 registration and real format probes. It then constructs the
sampler parent with an **empty** `NativePlatformLoadMessagePreimage` and no root
entry/release evidence. Actual `PeekMessageA` produces its MSG output. The
root/mode shared-word knowledge gate remains unchanged; no stack word is read
or initialized by this probe.

The retained effect graph receives that parent's exact bindings. Two stable
actual 8h `NativeString` headers in the retained probe receive `GuiFont.mshd`
and `GuiFontBilinear.mshd` through A41E870 using the same raw VFS string storage
as the renderer effect registry. B318B0 executes the genuine cache, VFS, loader,
descriptor and compiler path. There is no alternate provider, success default,
copied owner map or wrapper texture/material/effect.

`run()` returns a const observation containing each exact native caller credit.
Those credits remain owned by the probe slots. The caller may inspect or borrow
the raw pointers; it must not separately publish, retain or release them. The
probe checks the canonical `NativeMaterialEffectReference`, its alias of actual
atomic+04, supported current profile and positive observed count. No count is
rewritten or incremented. Alias results still occupy separate owning slots.

`fallback_call_seen` records an actual recursive loader cache call. A returned
native fallback is reported as that observed resource, not certified as the
requested GuiFont shader content. The helper does not print a forced PASS.
Native null, unavailable/unsafe Source branches and actual C++ failures remain
meaningful results; null is preserved in the retained producer observation and
rejected for the probe's required resource inspection.

## Caller retirement and shared native drain

After a complete run, call `retire_caller_references()` **before** shared native
drain. It releases the second slot and then the first, exactly once per returned
credit, using the existing B32010 actual+04 decrement and current-profile
canonical terminal path. It captures the real count immediately before release
and records release entry before invoking it. Afterward, only an existing
canonical companion permits another raw read. A live companion must show one
removed credit; terminal disappearance must follow a pre-release count of one.
No freed effect is dereferenced. Slots retain opaque identity observations after
their owning pointers are cleared.

After both credits return, the method destroys the two input-name allocations
through native A41DD20, in reverse order, before the original string-pool drain.
The 8h header addresses remain stable metadata through graph closure; their
stale destroyed contents are never read. Entered failure retains any remaining
credits, names and native operation frames without attempting rollback.

The caller next performs the **existing** original singleton/renderer drain
while the complete probe and all supplied providers remain alive. It then calls
`after_native_renderer_drain()`, which closes the effect, sampler and shadow
metadata in that order using their existing closure checks. This method performs
no cache clear, reference decrement, singleton removal or native destruction.
Only a never-run or successfully closed probe can be destroyed.

## Failure observations and process retention

Each method stores the precise stage and `exception_ptr`, emits available
observations and rethrows. The caller must preserve the entered heap probe and
application graph under the existing no-unwind process-retention exit policy.
It must not reset a `unique_ptr`, leave a stack probe to unwind, retry acquisition
or attempt success cleanup on a failed graph.

`emit_failure_observations()` is nonthrowing. It records the shadow operation's
native site/state/publication/section, each input header and returned slot,
sampler scope/procedural creator counts, effect/cache/loader phases and native
sites, raw creator/companion/owner-record identities and every recursive
fallback cache frame. Attached program operations expose descriptor/Lua and
compiler prefix/tail state, native sites, pass registration, root/mode sampler
sites and knowledge flags. It never reads an unknown private sampler word or
unproduced compiler scratch fields. A logging failure emits a stderr retention
notice; it does not destroy the graph or supply invented observations.

## Verification and integration boundary

The new TU strict-compiles with MSVC Win32 C++17 `/W4 /WX /EHsc /permissive-
/O2 /MD /fp:strict`. Worker-local Source/link receipts are isolated under
`local/cc12_native_gui_effect_probe`; earlier immutable sampler/shadow receipts
are preserved. There is no new test framework or live executable launch.

Combined `bsp_game` linkage passes through a local CMake overlay adding only the
pending sampler, shadow and probe TUs. A forced-retained link anchor keeps the
constructor/destructor and addresses of every probe method reachable, so unused
method elimination cannot hide missing dependencies. That anchor is never
called; it is link evidence only. A temporary unqualified outer-namespace type
in that local anchor was corrected and its failed log preserved separately.
The default build configuration was restored with `scripts/build.ps1`, which
passes the existing two suites (10.53 seconds). Shared CMake is unchanged.

Primary owns permanent CMake registration, the opt-in phase7 harness hook,
actual caller references and serialized runtime launch. The new helper supplies
the complete retained probe API; a compiled/linkable helper alone does not prove
the live original resources load or retire. Actual GuiFont compiler success,
Text/GUI/world rendering, gameplay and native register/FH3 ABI remain unproved.
