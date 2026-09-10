# Special material shadow samplers

`material_shadow_samplers.cpp` reconstructs the bounded special-name selection,
sampler overrides and late texture-binding paths. It accepts explicit texture
and first-light inputs; it does not generate game shadow maps. Evidence is
recorded in `reports/material_shadow_samplers_audit.json`.

The installed `shaderfx/dx9_lua.inc` sets `TEXSRC_SHADOWBUFFER = 2`.
`shaderfx/common/textured.shfx` selects `per_vertex_shadow.shfx` for
`RM_NORMAL`. Its base descriptor has one pixel sampler, `MyTexture`, and the
effect descriptor `shaderfx/lights/per_vertex_shadow.shfx` has one pixel
sampler named `ShadowTexture`, with source 2 and CLAMP U/V states. Its pixel
shader samples the red channel and multiplies it into direct lighting.
The resulting native special slot is **1**; no `ShadowMap` slot is present.

## Source 2 and selection order

Descriptor helper `00b3b280` handles source 0, 1 and 3 as distinct reference
routes. Source 2 follows `00b3b303..00b3b306` directly to state handling at
`00b3b334`. It appends no texture record and does not increment builder
reference counter `+8Ch`. Its sampler states are still processed, and the
corresponding vertex `+90h` or pixel `+94h` counter advances exactly once.
This is a state-only descriptor entry. It must not be treated as material
texture Index 0. The primary integrates this extension in `material_samplers`.

Native builder `00b3b3c0` first appends descriptor samplers, then removes unused
pixel states according to compiled PS metadata. **After this pruning**, its
`00b3c0b2..00b3c349` fragment configures the two special names and applies
ShadowMap state overrides. Consequently those overrides can reintroduce
states for a slot that the pruning loop removed. The host configure helper
must run after `prune_material_sampler_states`.

`00b347e0` uses ECX descriptor, stack pointer to a counted name, EAX index or
`FFFFFFFFh`, `RET 4`. It searches descriptor `+C4h` in array order for `+C8h`
entries. Stored name length `+4h` must equal query length, then `__stricmp`
compares pointers. The first match returns the **overall descriptor ordinal**.
There is no sampler stage, dimension or TextureSource check in this lookup.

For each special name, the builder searches **only the effect descriptor**
(`builder+74h`). On a match it counts every base descriptor sampler whose
VertexSampler byte `+Ch` is zero. The stored slot is:

`base pixel sampler count + matching effect descriptor's overall ordinal`.

`ShadowMap` writes pass `+78h` at `00b3c18d`; `ShadowTexture` writes `+7Ch` at
`00b3c28a`. Native code does not convert the effect ordinal to a pixel-only
ordinal, and does not add 16 for a matching vertex-stage declaration. The
host preserves this arithmetic and rejects selected slots outside 0..19.
No source/stage restrictions absent from native code were added.

When ShadowMap is present, `00b3c2ac..00b3c349` sets these states in order:

| State | Builder byte `+AAh == 0` | Builder byte `+AAh != 0` |
|---|---|---|
| MAGFILTER (6) | POINT (1) | LINEAR (2) |
| MINFILTER (5) | POINT (1) | LINEAR (2) |
| MIPFILTER (7) | NONE (0) | NONE (0) |
| ADDRESSU (1) | CLAMP (3) | CLAMP (3) |
| ADDRESSV (2) | CLAMP (3) | CLAMP (3) |

The host receives this operational choice as `linear_shadow_map`; the
producer and broader meaning of builder byte `+AAh` remain outside the packet.
ShadowTexture receives no comparable special filter override here.

## Default and ownership

Pass constructor `00b44b10` uses ECX pass, no stack arguments, EAX this and
plain `RET`. Its bounded initialization at `00b44b35..00b44bb5` sets both
special slots to -1 and fallback pointer `+84h` to zero. It requests the
literal `white.tga` with flags 0 through renderer virtual `+64h`, proven by
table DWORD `00d5f10c` to be existing `00b319b0`. The returned pointer is
stored directly at pass `+84h`; no extra AddRef or temporary Release follows.

Deleting wrapper `00b46910` calls destructor `00b454e0`, then returns the CPU
object to its pool if stack flag bit 0 is set. Its ABI is ECX pass, stack
deletion flags, EAX this, `RET 4`. Destructor fragment
`00b4553d..00b45564` InterlockedDecrements the fallback's intrusive count at
`+4h`, invokes its destructor when zero and clears pass `+84h`. Thus the pass
owns the returned fallback reference. Native factory/file loading, full pass
construction/destruction and pool return remain separate implementations.

The typed pass retains a caller-supplied `shared_ptr<LogicalTexture>` for
the fallback. As with existing LogicalTexture APIs, the underlying COM
texture remains borrowed and must outlive that projection and cache bindings.
A controlled white texture can stand in for the explicit shadow input in a
host draw; this does not demonstrate native shadow generation.

## Late binding

Native parent `00b42350` has ECX pass, stack render-entry and override,
`RET 8`; the owned late fragment is `00b430cf..00b4315f`. It binds in this
order, with no pixel-usage-mask gate:

1. When pass `+7Ch != -1`, obtain ShadowTexture using global `00f8d39c` and
   helper `00b0d130`, then bind it at `+7Ch` through renderer virtual `+130h`.
2. When pass `+78h != -1`, take the **first light only** from the list reached
   through render-entry `+Ch`, owner `+170h`, sentinel `+1Ch`, first node,
   node `+8h`. An empty list invokes the native failure path. Getter
   `00b7aab0` returns light `+174h`, its shadow-map owner.
3. If that owner is absent, bind pass fallback `+84h`. If it exists, call its
   virtual `+8h` and bind the returned logical texture, **even when null**.
   A null returned texture does not select the fallback. Later lights are
   never searched by this fragment.

`00b0d130` is not an adjustor thunk: it loads ECX from incoming owner `+3Ch`
and tail-jumps to `00b4cb10`, which returns wrapper `+8h`. Both have no stack
arguments and return a borrowed texture in EAX. `00b7aab0` similarly has ECX
light, no stack arguments, EAX borrowed shadow owner and plain `RET`.

Renderer table DWORD `00d5f1d8` identifies virtual `+130h` as existing
`bind_texture_00b24710`. That API retains logical identity in the host cache
and maps logical slots 16..19 to D3D vertex sampler slots 257..260. The
special binder uses it rather than bypassing the established cache.

The explicit-input API distinguishes first-light absence, missing shadow
owner and an existing owner's null texture. It returns INVALIDCALL for a
required empty light list instead of invoking native failure; it reports
texture-binding HRESULTs. ShadowTexture may already have been bound before
that later light-list error, matching the native operation order. Texture
getters are supplied as stable caller-observed values, not engine globals.

## Verification boundary

All 15 captured Ghidra byte ranges, totaling 1,767 bytes, match the installed
PE, with project `bsp` and program `/battlestationspacific.exe` verified for
every live query. The report includes actual installed shader-file hashes,
exact address fragments and original ABIs. No new test suite was added;
compilation and existing real-device integration are owned by the primary.

A stale `CALL_RETURN` at `00b45522` hides the returning free-thunk cleanup
`ADD ESP,4; MOV [EDI],0` at `00b45527..00b4552f`. The complete containing
destructor bytes were verified and the old flow state was preserved for
primary repair. The fallback-release fragment itself is directly present in
the current assembly. No Ghidra mutations were made by this worker.
