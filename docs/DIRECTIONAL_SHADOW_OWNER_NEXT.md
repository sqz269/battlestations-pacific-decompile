# Directional-light shadow owner: next implementation boundary

Discovery only, based on `c314dfc`. The concrete object retained by light `+174h`
is the `508h` allocation constructed by `00A8FD30 -> 00A8FA30 -> 00A8E2E0`.
Its final dispatch table is `00D5B5D8`. This establishes a real implementation
target; it does **not** make the existing borrowed `SystemShadowMapOwner` view
a complete constructor or an owning native replacement.

`reports/directional_shadow_owner_next.json` records native preimages, SHA-256
hashes checked against the installed executable and current Ghidra bytes, source
hashes, native ABI, returning-free tails, and dependency classifications.
No Ghidra, names ledger, reconstruction ledger, source, build or test changes
were made. Names below are descriptive hypotheses, not recovered class symbols.

## Concrete profiles and ownership

| Object | Evidence and native layout |
| --- | --- |
| Per-light shadow owner | Factory `00A8FD30`, `508h` ordinary `operator new` allocation; `+04` intrusive count starts at 1. Base construction installs `00D5B574`; final table `00D5B5D8` contains virtual `+00=00BD30E0`, `+04=00A8FCD0`, `+08=00A8FCF0`, `+0C=00A8FD10`. |
| Global shadow target | Separate `2Ch` object published through `00F8BBF0`; `00A8FE30` constructor, table `00D5B5E8`, direct deleting destructor at virtual `+00=00A900C0`. `+04/+08` are extents, **not** a reference count. `+10/+18` own color/depth texture wrappers; `+14/+1C` own their level-zero surface wrappers. |
| D3D9 2D texture | Renderer table `00D5F0A8 +88h` resolves to factory `00B2A070`; constructor `00B3F7B0` installs `00D61948`. `+04` is an intrusive count; `+10` owns the COM texture; `+28/+2C` are mutable reported dimensions; `+40/+44/+48` hold cached surface records; `+4C` is an optional retained source. |
| D3D9 surface | `00B3F630`, `34h` wrapper, table `00D619A0`, virtual `+04=00B3F5B0`; constructor and native pool lifetime remain incomplete. |
| Per-shadow target group | Five `40h` allocations constructed at `00B1FBB0`, table `00D5E600`, count `+04=1`; retained color wrappers at `+08..14`, depth wrapper at `+18`, additional cached COM references at `+28..38`. |
| Per-shadow viewport | Five `34h` allocations constructed at `00B1F850`, table `00D5E5F8`, count `+04=1`. Four attach to real camera objects; the fifth is held at shadow `+504`. |

The base shadow constructor stores the passed light at shadow `+0C` without a
retain and copies light `+A4` into shadow `+34`. The destructor releases neither.
Treat these as borrowed fields; the meaning of the copied `+A4` word is not
established here. Do not create a shadow-to-light ownership cycle.

## Light publication and complete destruction

`00B7BDF0` is `thiscall(light, replacement)`, `RET 4`. It captures old `+174`,
skips on identical pointers, publishes the replacement, increments replacement
`+04` if nonnull, then decrements the captured old owner and invokes its current
virtual `+00` on zero. Publication is observable before either reference-count
operation; replacing this with release-before-publish changes reentry behavior.

World setup `004DF3C0..004DF3F3` passes game `+19F8` in ECX to `00A8FD30`,
passes its return to `00B7BDF0`, then drops the creator reference. The caller's
`XOR DL,DL` is unused by the factory and constructor. `00A8FD30` has no stack
arguments, returns EAX, and ends with plain `RET`; its constructor has ECX=this,
one stack light pointer, EAX=this, `RET 4`. Other direct factory callers are
`00504130`, `005AB190` and `0067F080`. A further `00B7BDF0` call in `00BA1400`
copies an existing shadow owner; it is not another factory.

Light destructor `00B7C5B0` captures its nonnull `+174` at `00B7C644`, decrements
shadow `+04`, invokes shadow virtual `+00` on zero, then writes light `+174=0`.
The complete light/base-node destructor is owned by the separate light packet.

Shadow virtual `+00=00BD30E0` calls its current virtual `+04` with flag 1.
Both `00A8FCD0` and base-table `00A8E160` call `00A8DEC0`; if flag bit 0 is set,
they call `00BF65AC`, execute the real returning tail, and return this in EAX
with `RET 4`. The tails include `ADD ESP,4; MOV EAX,ESI; POP ESI; RET 4`;
the pseudocode's `extraout_EAX` after free is not the ABI.

`00A8DEC0` first installs base table `00D5B574`, then performs this order:

1. Release/clear viewport `+504`, then viewports `+10,+14,+18,+1C`.
2. For cameras `+20,+24,+28,+2C`, call `00B71990(camera, null)` first; then, if
   the field is nonnull, call `00B6DFA0` and clear that camera field.
3. Release/clear fallback texture `+384`.
4. Release/clear target groups `+4F0,+4F4,+4F8,+4FC`, then `+500`.
5. Call `00BD30F0`, which installs reference-base table `00CEB130`; return.

Each retained-field release captures the pointer, decrements `+04`, invokes its
current virtual `+00` on zero, and clears the field after that callback. The
camera viewport calls are unconditional; simply skipping null cameras does not
reproduce the native destructor's preconditions. The active render-camera field
`+30` is not released by this destructor. Normal `00A8F3B0` temporarily retains
that camera and releases/clears `+30` at `00A8F9CF..00A8F9EF`; destruction during
an active update is outside this discovered ordinary-lifetime contract.

The factory's one-state EH map frees the raw allocation via `00CB6580`.
Derived constructor EH state 0 invokes `00A8DEC0` through `00CB6550`; states 1
and 2 free the in-progress target-group allocation before returning to state 0.
All three free funclets include their actual `POP ECX; RET` continuation.
The base constructor has a distinct 19-state map, including strings, camera
allocations and viewport allocations. Its state-0 action is only `00BD30F0`.
Do not substitute a blanket complete-shadow-destructor rollback for that map.

## Actual texture dispatch and dimensions

The two short shadow virtuals are currently missing function definitions in
Ghidra. Their complete installed and Ghidra byte preimages establish:

| Virtual | Native behavior, ECX=shadow, EAX=borrowed texture, no retain, plain RET |
| --- | --- |
| `+08`, `00A8FCF0` | Capture global `00F8BBF0`; if both target bytes `+0C` and `+0D` are nonzero, tailcall `00A8FDB0` to return target `+18`; otherwise return shadow `+384`. |
| `+0C`, `00A8FD10` | Same flag decision; true tailcalls `00A8FD90` to return target `+10`; false returns shadow `+384`. |

There is no null-global guard, no texture creation in these getters, and no
retain. Each invocation reloads the global target and checks its flags. A
`SystemShadowMapOwner` adapter must use the same actual mutable target and the
same actual fallback texture. Its existing rendering fragment calls virtual
`+08` twice, so those calls must remain separate.

The fallback is loaded as literal `white.tga` at `00A8E3C6..00A8E3DA` through
renderer virtual `+64`, with flags zero, and the returned reference is adopted
at shadow `+384`. This is the native fallback resource, not authorization to
invent a replacement image or assume arbitrary dimensions. The successful 2D
file-load route uses existing named constructor `00B3F930` and the same
`00D61948` table; other loader outcomes remain the loader's responsibility.

For the concrete `00D61948` profile, virtual `+3C=00B3CE50` returns the DWORD at
texture `+28`, and virtual `+40=00B3CE60` returns `+2C`. Both are four-byte
`MOV EAX,[ECX+offset]; RET` getters, also undefined as Ghidra functions today.
`00B3F7B0` initially fills these fields from `GetLevelDesc(0)`, but they are
**mutable reported metadata**, not a guarantee of current physical COM size.

Global target virtual `+08=00A8FDD0` always stores its requested enable byte at
`+0C`. When `+0D` is nonzero, it calls `00B3CEB0` for both texture wrappers:
enabled writes target `+04/+08`; disabled writes `8,8`. `00B3CEB0` directly
overwrites texture `+28/+2C` and makes no COM call. A fresh `GetLevelDesc` query,
or an immutable dimension snapshot, would produce different behavior.

Global target virtual `+04=00A8FF30` creates textures only once (`+20` guard),
stores enable at `+0C`, initially clears `+0D`, and uses extents `8,8` when
disabled. With pixel-shader capability at least `200h` and nonzero target
`+24`, renderer virtual `+88` creates color `(width,height,1,+24,10h)` then
depth `(width,height,1,+28,100h)` textures. It obtains each level-zero surface
through texture virtual `+30` with `(0,0)`, stores the four owners, sets `+0D=1`,
and eventually marks `+20=1`. Capability rejection also marks the attempt.
The constructor's format selection is evidenced separately by `00A8FE30`:
color `17h`, depth DF16 or D16 after supported-format probes.

`00A8FFF0` releases the global target's color surface, color texture, depth
surface, depth texture in that order before its base unpublish path. The global
target's destructor is not shadow virtual `+00` and is not an intrusive release.

## Constructor dependencies and reusable code

Base `00A8E2E0` creates four real `00B71A80` camera objects, five viewports,
loads the fallback texture, sets camera projection/mode/visibility inputs and
attaches the four viewport references. It obtains viewport defaults from the
real renderer. Shadow direction is initialized to `(0,1,0)`, limit words at
`+390..39C` to zero, and scalar `+08` to raw `46EA6000` from `00CE77FC`.
World setup may replace `+08` with authored scene `+A84` afterward. These are
proven individual writes, not justification to zero-initialize the whole owner.

Derived `00A8FA30` lays out four half-extent viewports, computes matrices at
`+244,+284,+2C4,+304` through `00A8AAA0`, then creates the five target groups.
All groups retain the global target's actual color/depth **surface** wrappers.
The color surface is captured once before the group loop; depth is reloaded
for each group. Texture wrappers returned by shadow virtuals are distinct from
these surface wrappers.

Neither constructor writes shader matrices `+144,+184,+1C4,+204` consumed by
`SystemShadowMapOwner`. `00A8F3B0` starts by copying identity into those four
and the four matrices at `+44..104`, even before its enable/capability gate.
Its active branch performs additional camera/view/cascade work through
`00A8EA20`. A constructed object is not automatically a render-ready shadow
owner, and substituting identity in the constructor hides that boundary.

| Existing code | Reuse boundary |
| --- | --- |
| `SystemShadowMapOwner` / `SystemShadowTexture` | Borrowed rendering projections. Add real dispatch and shared mutable dimension bindings; do not add duplicate matrix or dimension storage. No owning constructor/refcount/destructor exists here. |
| `D3D9RetainedTexture2D` | Reuses real file bytes, COM recreation and source-before-COM cleanup for the fallback load. Omits native registry, cached surface ownership and intrusive pool lifetime. Its immutable options are insufficient for the shadow toggle's mutable reported dimensions. |
| `D3D9ResetTexture2D` and cached-level helpers | Useful actual COM/metadata/reset building blocks. Cached surface wrappers are borrowed; they do not implement native `00B3FD80` retained cached-surface return or full texture destruction. |
| `D3D9SurfaceBinding` helpers | Provide real surface COM initialization/release, not complete native surface wrapper construction or its pool deletion. |
| `D3D9FrameTargets` | Shares real surface-wrapper ownership for device binding, but does not include native group cached COM references or its complete destructor. |
| Camera/frame projections | Existing field and renderer access are reusable; native camera allocation, full `00B71A80` construction, viewport retention and shadow update remain separate work. |

The texture's own zero-count path is `00BD30E0 -> 00B3F590 -> 00B3F2E0`.
The deleting wrapper returns storage to pool `0108DB38` via `00B3D8D0`, not
ordinary free. `00B3F2E0` releases source, notifies renderer/registry, releases
texture COM, releases cached surface wrappers, frees their array, then calls
base `00B33F50`. Its missing listing tail after `00BF6989` is included in the
report. Likewise target-group `00B1FC00` releases retained wrappers and its
cached COM references before array cleanup and the actual returning base tail.
Neither lifetime is fully represented by a plain borrowed `LogicalTexture`.

## Smallest useful implementation packets

The **ready** packet is native texture dispatch and reported-dimension access:
define/annotate the four missing functions `00A8FCF0`, `00A8FD10`, `00B3CE50`,
`00B3CE60`, plus the two global texture getters if still undefined; reconstruct
their exact branches using actual bound owner fields. Include `00B3CEB0` and
`00A8FDD0` only when the same mutable metadata can be bound end to end. Preserve
separate width/height dispatch calls and borrowed return lifetime. This adds
usable behavior without pretending to create or own a native shadow.

A separate light integration packet can reconstruct `00B7BDF0` once its actual
reference-counted shadow owner is available. It must use that owner's real
virtual-zero cleanup; a callback that silently does nothing is not sufficient.

A **complete owning shadow factory is not yet ready**. Its named-but-incomplete
dependencies include `00B71A80` cameras and `00B71990` viewport retention,
`00B1F850` viewport lifecycle, `00B1FBB0/00B1FC00` target groups, global-target
construction/create/toggle/destruction, renderer texture factory `00B2A070`,
unnamed texture constructor `00B3F7B0`, cached surfaces `00B3FD80`, texture and
surface pools, and the exact constructor EH maps. Render-ready creation also
requires `00A8F3B0 -> 00A8EA20`. Existing D3D9 helpers can be reused within those
packets; they do not discharge the ownership or update dependencies themselves.

This discovery is native-byte and source grounded. It is not reconstructed,
build-tested, fixture-tested, ABI-compatible, or game-validated implementation.
