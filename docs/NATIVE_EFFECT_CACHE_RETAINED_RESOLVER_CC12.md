# Retained numeric effect cache-key resolution (CC12)

Packet `cc12_effect_cache_retained_resolver`, Source base `9cdf1e6a9`.
Owned native functions: `00B31090` and `00B2E940`. This packet composes the
already reconstructed `00BDF4C0` numeric resolver; it does not change that body,
its provider selection, or the application's VFS context.

## Reached failure and routing correction

The immutable `23fbb8a1d` GUI resource probe created and published the actual
`D5B5E8` shadow target at 2048 square, then failed on its first `GuiFont.mshd`
acquisition with `unretained effect name resolution requires an explicitly
selected callback`. Its cache operation was failed, its loader was fresh, and
no sampler scope or procedural effect creator had entered. The parent retained
the graph and exited with status 1. This is failure evidence, not effect or
rendering success. The frozen parent log is
`local/cc12_actual_native_gui_effect_startup_02.log` in the integrator worktree.

`GameNativeGuiEffectApplication` already selects the genuine numeric resolver
from its same actual VFS/string graph. The miss in `00B31090` called only the
three-argument `00B2E940` Source overload. That overload passed its stack-local
changed and original names to the callback-only two-argument loading resolver.
The latter intentionally rejects numeric contexts. The retained overload used
by the loader is not interchangeable: it requires the loader's own stable
`resolved_name` and permits only one invocation. Thus this was a **cache
retained-operation overload routing and lifetime bug**, not a missing
application callback or context-selection choice.

The new four-argument `resolve_native_effect_cache_name_00b2e940` requires a
fresh `NativeMaterialEffectCacheKeyResolutionAcquired` and its exact output
header. A numeric cache miss publishes this frame into the existing cache
operation before entering any native producer. It owns genuine pooled
original, rewritten, search, replacement and output string headers. Its
separate changed/original `NativeMaterialEffectNameResolutionCall` children
invoke the existing numeric `00BDF4C0` implementation over the same publication
cell, manager, strings, lookup, provider and logging domains. There is no new
VFS owner, registry, callback, texture wrapper or successful default.

Initial alias hits still return without resolving. The explicitly selected
legacy callback path remains available through the original Source overload.
A numeric miss requires a retained cache operation even when loading is
disabled, because key resolution itself enters real VFS producers. Native
resolver entry is now reported as `00B311E9`; the prior `00B310B8` observation
was the earlier loading-message pump site left in cache metadata.

## Original order, identity and ownership

Saved assembly, current Ghidra memory/instructions and Original PE bytes were
checked with the exact existing project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The bounded receipt covers seven spans and
selected instructions; it is not new register-ABI or whole-body recovery.

* `00B311DF` selects registry slot `+04`; `00B311E9` calls it with hidden string
  output, request and opaque context. `00B31090`'s original ABI remains ECX
  registry, four stack arguments, `RET10h`.
* `00B2EA62` replaces every case-sensitive `.mshd` occurrence with `.shfx`,
  with maximum `7fffffff`. Captured search storage is retired before captured
  replacement storage (`00B2EA7E`, `00B2EA9F`).
* `00B2EAA4` reads the current `0109CEEC` manager, `00B2EAAF` resolves the
  rewritten mutable header, and `00B2EAB7` branches on the actual false result.
* Only that false arm reloads the same current publication at `00B2EAF4`, then
  calls `00BDF4C0` at `00B2EAFF` on the separate original header. The following
  copy ignores AL and returns the current original header even when resolution
  failed. A false native result may have mutated that name.
* Output is explicitly initialized at its native production point and copied
  from the selected current header. Rewritten storage retires before original
  storage (`00B2EB64`, `00B2EB87`); `00B2E940` returns the output, `RET0Ch`.
  The cache copies that output, then retires its actual pooled storage through
  `0041DD20` before continuing its existing alias/load path.
* `00BDF4E3` normalizes its mutable input, `00BDF534` enters ordered candidates,
  and only a true result enters successful-search logging at `00BDF547`.
  `00BDF586` is `RET4`; no pure file-exists callback replaces this behavior.

No cache credit, actual effect count, native owner, loading flag, sampler word,
upper-byte knowledge guard or fallback policy changes. Search/replacement
captured release arguments remain explicit. Each resolver captures the actual
manager separately on entry; the second call does not reuse the first call's
manager snapshot or acquired VFS frame.

## Failure retention and diagnostics

Frame construction is metadata-only. Phase, step, native site, live-header
ownership, each captured call and the actual exception survive an entered
failure. The new frame cannot replay or retarget. A failed/running frame or a
frame with live pooled storage cannot be destroyed: its destructor terminates,
matching the existing retained-cache/loader contracts. Native false return is
a completed call, not a fabricated failure or success.

On success both temporary name headers retire in Original order, and cache
output retirement is acknowledged only after the caller copy and `0041DD20`
return. If any entered copy, resolver, release or output copy fails, the frame
and real pooled children remain retained. No destructor tries to replay a
release, restore a header, release a borrowed owner or continue the other arm.
The application's existing process-retention/no-unwind exit policy remains
required for a failed graph. These Source contracts do not implement Original
FH3/SEH unwind or the binary ABI.

The authorized probe diagnostic follow-up reads only retained metadata: key
phase/step/site, live-header flags, arm identity, actual manager/header address,
started/returned/failed/raw boolean result, and each actual VFS acquired
phase/active site/failure site. It does not dereference retired name buffers or
select a different context. The immutable `22ce33474` probe receipt is preserved
as historical evidence; this packet records its changed diagnostic Source
separately.

## Verification limits

Strict Win32 compilation, the required combined build and existing checks are
recorded with exact current artifact hashes in
`reports/native_effect_cache_retained_resolver_cc12.json`. The existing bounded
native/source VFS device replay checks its actual dependency semantics against
its immutable baseline; it is not cache-key routing or GUI execution proof.

This packet launches no game and claims no acquired GuiFont effect, sampler
success, draw, visible Text or world rendering. The parent must build the
registered application and rerun its genuine opt-in probe. A later actual VFS,
descriptor, compiler, sampler or cache failure must remain a retained failure
with its concrete operation site, not be converted into a successful default.
