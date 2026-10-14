# Genuine Lua service scope for GUI effect acquisition (CC12)

Packet `cc12_gui_effect_lua_service_scope`, Source base
`58f2320c8032583d52e0b6db55383b190b44d2c6`. Owned application composition at
`00B318B0`; no Original function body, Lua callback, reader or service-binding
implementation changes.

## Actual reached failure

Root's immutable executable built at `cba30590f2360eeec37db77b63b88dee3feecc7b`
(SHA256 `0312a8a9e1dbbd7c85394f932cc31dc3c8556bb4e0fd1f67f19778caf84937c8`)
ran the actual phase-7 GUI resource probe on slot 3/core 8 at
`2026-10-14T22:16:17.7197834Z..22:16:19.0639249Z`, exiting 1. Its launcher
manifest records later checkout metadata `58f2320c8`; that metadata is not the
immutable executable's build Source identity.

The numeric cache fix was reached and completed: rewritten-name VFS returned
true, its child completed at `00BDF547`, all key pooled-name ownership flags
were clear, and the original arm did not enter. The first effect then failed
with `native Lua callback invoked without active source service binding`.
Cache site was `00B3130D`; loader site `00B2ED5A` had a constructed, unregistered
creator; program site was `00B45F5E`; retained descriptor was at `00B43B4E`.
No sampler scope/procedural creator or returned effect was observed.

`NativeLuaServiceBindings::active()` is a genuine Source bridge guard shared
by DoFile and override-append callbacks. The failure does not independently
identify which of those two callbacks entered it. Both need the same service
scope around bootstrap/file execution. No fabricated callback or successful
return bypasses this guard.

## Binding and scope

The renderer's existing `DescriptorGraph` already borrows bootstrap and files
from its retained `GameNativeLuaServices`. Its canonical
`read_shader_descriptor()` activates that same owner's `binding()` for the
whole descriptor operation. The effect graph instead composes
`NativeMaterialEffectNativeChildren` over the borrowed reader and calls the
native parser directly; its acquire entry omitted that activation.

`GameNativeRendererApplication::lua_service_binding()` is a ready-only
metadata borrow of `impl_->lua_services.binding()`. It does not construct a
binding, clone bootstrap/files/VFS context, invoke a Lua service or change
publication. The GUI effect graph stores that exact reference and revalidates
getter identity, exact reader file-services identity, raw string identity and
the already-selected DoFile callback before publishing a native operation.
The real Lua/VFS/string/application owners must outlive the renderer, reader,
effect graph and every retained failed Lua frame; the getter grants no owner
credit and no permission to destroy/rebind them early.

After its one-shot acquire frame is published, the effect entry records the
borrowed binding identity and constructs the existing
`NativeLuaServiceBindings::Activation` inside the try scope. It covers the
actual cache/loader/program/descriptor work and synchronous result/logging
path. No function signature, loading flag, native reference count, callback,
Lua status, parser return, sampler guard or fallback policy changes.

The Activation implementation is unchanged: constructor saves the current
thread-local pointer and installs this binding; destructor restores the saved
pointer. Same-binding nesting, different-binding nesting and an absent outer
binding therefore restore the exact preceding selection on ordinary C++
return/unwind. Catching an exception records the existing retained operation
failure after that lexical restoration. This is Source scope behavior, not a
new test claim about Original ABI, SEH/nonlocal transfers or interrupted Lua C
frames. An interrupted state is not admitted for resumption or automatic close.
Root explicitly chose existing scope review/build and its next real callback
run over a new production observer or a test solely mirroring this wrapper.

The diagnostic-only probe follow-up prints the retained operation's borrowed
binding address. It does not dereference retired Lua state or install a new
context. Earlier `b4f259da1` compile/build/failure receipts remain unchanged.

## Close, GC and retirement coverage

* Original `00B45F5E` calls `00B43B00` for the root descriptor. Its constructor
  `00B43B25`, bootstrap(mask 1) `00B43B3B`, script `00B43B4E`, Lua field reads
  and nested DoFile/override execution are within the new acquire scope.
* Normal descriptor cleanup closes its actual Lua state at `00B44665` through
  `00B669A0`/`lua_close`. Lua close may run GC/finalizers; this synchronous close
  remains inside acquire. A successful descriptor operation retains no live
  temporary Lua state. A failed descriptor keeps its existing live state and
  children; no new catch cleanup closes, retries or repairs them.
* A texture-source script producer reached synchronously during acquisition
  also performs its normal Lua close before returning (`00C30878` Source
  observation). The acquire scope covers that synchronous child, not arbitrary
  later producers or retirement entries.
* Canonical `GameNativeRendererApplication::drain_singletons()` already
  activates the same binding over actual singleton shutdown. Current host
  shutdown routes use this method when a native renderer exists. Renderer Lua
  owner destruction calls `00B669A0` at `00B1BC1E`, before base retirement; its
  normal close/GC is covered by that existing drain activation. Metadata-only
  `after_native_drain()` does not perform Lua execution or supply a scope.
* Probe caller-reference retirement (`00B32010`) occurs before shared drain
  and after acquire's activation has ended. Its current actual effect terminal
  enters retained descriptor bindings; the reconstructed descriptor/sampler
  terminal bodies destroy strings, arrays and state-list pool slots, with no
  direct Lua close. They do not retain the successful parser's temporary Lua
  state. This does not classify every virtual child/current-profile extension
  or arbitrary future owner as Lua-free. Any separately introduced callback/GC
  retirement entry needs its own qualified synchronous binding scope; this
  packet does not broaden caller retirement or claim that acquire covers it.

On entered failure the caller must retain all actual parents/providers and use
the existing no-unwind process-exit policy. The new borrow and activation do
not authorize failed graph/creator/descriptor destruction or renderer drain.

## Evidence and acceptance boundary

The bounded Original/saved/live receipt checks 153 bytes and 13 instructions
for effect-to-reader entry, descriptor bootstrap/script/close and renderer Lua
close. Project/program identity is pinned to `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`; no fresh large-body/x87/register recovery or
Ghidra mutation was performed.

Strict Win32 compilation and the required build attempt are recorded with
exact artifacts in `reports/native_gui_effect_lua_service_scope_cc12.json`.
The default build exposes Main's already-known pending platform-TU CMake
registration. An owned ignored overlay adds only that reviewed TU for combined
Source verification; shared CMake/hosts remain untouched. No new tests or game
launch occur here. Root's combined build and actual opt-in callback/effect run
remain acceptance checks. Acquired GuiFont effects, sampler success, native
Text rendering and raw ABI/game equivalence remain unproven.
