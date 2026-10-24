# Mission effect texture refresh

`GameNativeMissionMaterialRefresh` composes the existing original whole-registry
`B24DD0 -> B19000 -> B1BC70/B319B0` route over the same renderer application. It
does not change `B17D90`, compensate its reference behavior, fill texture slots
directly, or choose replacement environment assets.

## Missing producer

The source-3 sampler branch in `native_material_descriptor_samplers.cpp`
(`B3B313`) records the descriptor's source-name header in the effect via
`B18FB0`, then appends `-1-effect_index` to the actual pass texture-reference
list. That establishes `NativeMaterialEffectBaseStorage::names_3c` and its
count; it does not load or publish `textures_0c`.

At draw time `B43410` maps a negative reference back to `effect_index` and calls
`B17D90`. A nonnull effect slot is borrowed. A missing slot takes the existing
fallback at `+98`, increments its real `+04` through the live increment import,
then returns the reloaded fallback. `B43410` performs no corresponding release.
Thus repeatedly executing a missing effect slot has the original unmatched
fallback credit behavior; changing that getter would alter the original body.
The system-constant gather route fills shader constants and does not populate
these effect texture arrays.

`B24DD0` scans the renderer's actual effect registry (`+1A9C`, count `+1AA0`,
`2Ch` rows, resource `+28`) and calls `B19000` for every current effect. `B19000`
resolves each nonempty recorded name through the CURRENT renderer Lua owner
`F8D434` using `B1BC70`. A nonempty Lua string override wins; otherwise the
original name is retained. It calls current renderer `+64=B319B0`, publishes the
texture slot, retains the incoming owner when identity changes, releases the
old owner, then releases the returned acquisition reference even on identity
assignment. Empty selected names leave existing slots unchanged. The existing
reconstruction retains all native child frames and original call ordering.

## Original timing and integration choice

Original `004ED910`, reached by the scene header loader `00469BF0`, publishes
`ReflectionCubeMap`, `CloudShadowMap`, terrain atlas/mask/noise/LUT/static-shadow
parameters, then calls current renderer virtual `+120` at `004EDD95`. The actual
`D5F0A8+120` target is `B24DD0`. This call is confirmed in the original export;
the existing `scene_weather_cc13.cpp` and `TerrainTextureHeaderHost` also record
the same sequence. `B24FB0` uses the route during changed-resource activation.
The indexed tactical-preview environment caller is another matching renderer
slot use; arbitrary other slot-72 candidates are not evidence of this target.

Calling the same whole-registry body after each newly loaded mission model
batch, before `BeginScene`/first submission, is an **integration inference**:
new effects then have their recorded names and pass references available, and
their environment parameters must already be published. This packet does not
claim that every original runtime model loader called `B24DD0` at that point.
It introduces no per-frame refresh. The caller must schedule initial and late
model batches and serialize loading, Lua, frames, and shutdown.

## Literal and lifetime identities

`B1BC70`'s NIL arm reads a native C string starting at loader-zero `F8D438`;
the observed data is empty and no writer is established. This is a mutable data
cell, not an immutable empty literal or an eight-byte `NativeString`. Repository
source inspection found no existing provider. The canonical
`GameNativeRendererScalarProcess` now owns its single mutable zero byte and
exposes `material_texture_nil_00f8d438()` as a borrowed `const char*`. The cell
is never reset on borrowing or renderer lifecycle. The accessor adds no writer
or claim about unobserved original mutation. The non-string arm borrows the
exact empty read-only literal `CE3A0C` through `GameNativeReadOnlyData`.

The helper constructor takes the SAME `GameNativeRendererApplication` and its
mapped data. It obtains `material_texture_refresh_context()` from that app,
preserving texture cache, canonical owners, current Lua publication, string
pool, and renderer profile identities. Every call rechecks the app's ready
gate/current slot, activates its same Lua service binding, and invokes the
existing native refresh provider. A fresh persistent outer operation is
published before entry; every reached effect has its own persistent native
refresh/name/cache frames. Failure is sticky, cannot replay, and requires
retaining helper, application, Lua services, mapped data, and native owners.
Completed operations introduce no extra texture credits and can be destroyed
after native drain. No new native owner or cleanup schedule is created.

API:

```cpp
GameNativeMissionMaterialRefresh(GameNativeRendererApplication&,
    GameNativeReadOnlyData& same_readonly_data);
void refresh_after_model_load();
bool requires_process_retention() const noexcept;
```

The primary integration must include helper retention in its existing
process-retention decision and call `refresh_after_model_load()` once after
each completed nonempty batch of appended model loads, before native frame
entry. A missing `ReflectionCubeMap` or other scene parameter remains a missing
scene/environment provider. Falling back to a name that fails real cache
loading is not evidence of a valid environment texture; no arbitrary cubemap
path is supplied here.

Original addresses/ABI are established by the existing source and exports:
`B24DD0` ECX renderer/RET; `B19000` ECX effect/RET; `B1BC70` ECX renderer Lua
owner plus fresh output/key, RET8/EAX output; `B319B0` current renderer `+64`
with name/zero, RET8/owned texture result. This helper is a new C++ composition,
not an Original vtable/FH3/SEH/register/fault ABI replacement. No arithmetic or
reference-count leaf was changed. Only source/diff review was performed here;
primary integration must establish build, actual loading, visual output, and
post-drain ownership behavior.
