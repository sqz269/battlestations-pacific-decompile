# Source1 actual owner and binding readiness, CC12

The smallest genuine candidate is the installed
`shaderfx/lights/fog_shadow_uw.shfx` descriptor: its sole sampler is
`CausticsMap`, with `TextureSource=TEXSRC_TEXANIMATOR`,
`TextureSourceName="CausticsTextureSource"`, index 3 and no sampler states.
The ordinary registered Source providers can compose this path after genuine
procedural factory setup. The first executable comparison should cover the
whole `B3B280` call, its actual loader-returned owner and binding, and a direct
observation of native `B-16`. A later Source0 consumption is a separate claim:
no such following sampler was found in this candidate's genuine composition.

This is a read-only readiness packet based on Main `e12b6755c`. It changes no
Source, registry/cache contents, prior evidence family or Ghidra state and
executes no descriptor, owner, native parent or GPU operation. The report pins
the selected Source inputs, installed files and fresh live-Ghidra/installed-PE
byte comparisons. Those pins are a static audit, not a runtime provider closure.

## Actual input and descriptor production

Installed `dx9_lua.inc:21-24` defines descriptor source selectors 0/1/2/3;
`TEXSRC_TEXANIMATOR` is 1. This field is sampler `+14`, independent of compiler
`source_mode` and `load_variants`, which remain at their existing 0/0 contract.
The physical file is a candidate input, not proof that the active VFS selected
it or that a live descriptor contains its values.

The existing full reader `B43B00` runs real Lua bootstrap/DoFile/file services,
then calls `B41830 -> B57B50`. That path allocates the actual 2Ch sampler,
copies `Name` into `+04`, writes accepted `TextureSource` into `+14`, copies
`TextureSourceName` into the actual 8-byte header at `+18`, and reads index
into `+20`. The constructor establishes pixel stage byte `+0C=0`; the absent
state tables still produce genuine initialized empty pooled headers at
`+24/+28`. `B41830` binds the actual sampler and appends its pointer through
descriptor `C4/C8`. Do not replace that production with a hand-built sampler,
reordered pointer array or a rewritten selector.

The fog file has no combiners and its interpolator tuples supply the three
required scalar ordinals. Existing written-ordinal readers can therefore
handle these physical contents without invented combiner/field preimages.
The live Lua values and resulting sampler/header/count must still be recorded.
The public application `read_shader_descriptor` requires its existing ready
renderer and activates the same Lua service binding; bypassing its ready-state
guard would not be an application proof.

The real broader selection is also concrete: installed `shaderpreload.lua`
contains `coral.mshd` and `seaweed.mshd` at lines 188 and 193, while
`terrain/coral.shfx` and `terrain/seaweed.shfx` select `fog_shadow_uw.shfx` for
`RM_UNDERWATER`. Their root sampler is `MyTexture` with default Source0.
The original compiler calls root at `B3C018`, then mode at `B3C024`.
Consequently this physical composition is Source0 first and Caustics Source1
last; it does not supply a following Source0. The cache/VFS resolution and
actual material-mode invocation remain to be observed.

## Genuine resource, binding and ownership

Use the complete ordinary table setup from
[the factory contract](NATIVE_PROCEDURAL_FACTORY_SETUP_CC12.md):
`BBCB40 -> BBC900 -> BBC5F0/BBC740 -> B1B730 -> B1B3A0`, with the same live
AA0 manager, F8D41C registry and VFS raw string domain. Keep its real table,
four factories and complete publication/constructor/registration frame tree
alive. The factory contract is frozen separately at `b208f86b3` and copied
into this packet's local receipt. The application setup/retirement wrapper is
a separate implementation packet.

| Boundary | Actual behavior and required identity |
| --- | --- |
| `B3B2BA -> 4DE4B0` | Obtain the existing actual 1Ch F8D420 loader using the same manager. |
| `B3B2C5 -> B1B4D0` | Pass the original sampler `+18` header. Copy/lowercase that name, construct `Default`, call `B1A4F0` on loader `+4` with flags 1/1. |
| `B1A4F0` | Execute actual BECCD0 message/cursor work; inspect genuine aliases; resolve the copied name through current CE7D24 `+4=B19E40`. A cold miss uses current `+8=B1B810`. |
| `B1B810 -> B1B730 -> B19E90` | Forward the actual resolved name to the same registry. The incoming ECX and second stacked options are unused by the 20-byte adapter; its result is unchanged, RET8. |
| `B19E90 -> B19D60` | Compare the genuine tree key case-insensitively. Read the current node `+14` factory, its current profile, then that profile's current `+4` selector. |
| Caustics factory | Final factory profile `D644E8`, create `+4=BBC6F0`; allocate actual 34h storage, call `C30470 -> B19980`, then write resource profile `D64478`. |
| Source creator metadata | Existing `register_creator` constructs a `NativeProceduralResourceReference` over that allocation's actual `+04` and binds it into the renderer's same `NativeRenderActualOwners`. This does not retain or initialize the count. |
| Cache publication | Copy the real 2Ch record into the actual loader vector. `4D6F70` copies resource `+28` without retaining; `4D45A0` destroys names/aliases without releasing that resource. The creator credit becomes the cache-held credit. |
| Caller retain | Current CE7D24 `+0C=4DDB20` increments that same actual `+04` and returns the same owner. |
| `B3B2DF -> B44CF0 -> B44690` | Allocate a real 10h binding, copy original source-name `+18`, publish/retain the same owner at binding `+04`, then append through current pass `+5C/+6C`. |

The binding's word is the current builder `+8C`, not the sampler's index 3.
Its stored name is `CausticsTextureSource`, not the shader variable name
`CausticsMap` and not the loader's lowercase temporary. This distinction is
part of the proposed comparison.

On an isolated cold success with no intervening mutation, the existing code
predicts actual counts `1` after construction, `2` after caller retain,
`3` after binding retain and `2` after Source1 release. These are derived
expectations only: observe every actual result and never seed or rewrite a
count to fit them. A legitimate cache hit starts from its actual current
credit and adds the caller and binding retains before releasing the caller.

Resource and factory terminal profiles are separate. Resource `D64478` uses
virtual0 `BD30E0` then current `+4=BBC6D0`; ShoreWave resource `D644B4` uses
`BD30E0/BBC7F0`. Final factory `D644E8/D644F0` instead uses factory scalars
`BBC890/BBC8E0`. The existing procedural companion performs concrete resource
destruction/free, then unbinds and retires its metadata; no raw/companion access
may follow the terminal callback.

Normal retirement releases the pass binding credit through `B42250`, and the
real loader drain through `4DDA40` releases its cache credit before destroying
record names/aliases. In the isolated cold sequence these leave 1 then 0.
Keep the common owner registry, procedural companion/profile views, VFS/pool,
singleton deletion bindings and all retained operations alive through that
drain. Factory table shutdown must follow lookup quiescence and precede the
later loader/shared-manager retirement specified by the startup contract.

## Graphics and frame prerequisites

The procedural creators do not load textures, allocate a D3D resource or
initialize the remaining Caustics-specific payload. `C30470` writes only its
established base/profile/child-header/flag fields. The binding constructor
only allocates, copies a name and changes ownership. A sampler-binding proof
does not call procedural animation/update/render slots or require World to
initialize the payload used by those later slots.

The loader is not a no-op boundary: `BECCD0` runs real Peek/Translate/Dispatch
and XLive pretranslation, then the reached online/input/cursor providers.
It reads the actual captured platform and shared publications. There is no
direct D3D call in this inspected message/cursor chain; messages can still
enter real application callbacks. Use the existing live platform/input/online
domain and genuine native null guards only. Do not substitute a fake platform,
manually null globals, skip the pump or prefill MSG bytes.

A full B3B280 comparison also needs an actually constructed 88h pass. The
registered `initialize_native_compiler_pass_00b44b10` creates its canonical
state owners and performs the actual renderer `D5F0A8 +64 -> B319B0`
`white.tga` acquisition, transferring that result to pass `+84`. This is
separate from the procedural resource cache. Use a genuine completed pass and
builder in the ready renderer domain; a zeroed pass or texture-cache owner
substituted for the procedural owner is not sufficient. The root's recent
device-creation failures therefore still matter to the existing application
route, even though raw registry/factory creation can be checked independently
without a GPU call.

The retained parent must include the descriptor/sampler/state-list lifetimes,
actual builder/pass, the full singleton/default-load/cache/factory operation
chain, the binding operation, canonical creator metadata and the release
observation. A production per-compiler call scope supplies the invocation
identity; the exported nonexecuting domain view does not. Root and mode share
their actual physical-word knowledge only within that one invocation.

## Smallest comparison and admission boundary

After genuine setup and owner/pass preparation, enter the complete native
`B3B280` with ECX actual builder and stacked actual pass/parsed fog descriptor,
and compare the ordinary Source operation over corresponding actual domains.
The mode call of a real coral/seaweed underwater compiler invocation is the
natural larger integration boundary. It additionally requires the real
compiler/cache bracket and its earlier root Source0 entry-word proof.

For the narrower Source1-only call, initial W can remain unknown: no Source0
consumption precedes binding. The native three-argument CALL at `B3B2DF`
writes `B-16=00B3B2E4`; the qualified current nonzero decrement preserves it.
Directly observe owner/count/result and W at `B3B2F5` or `B3B32D` without a
stack-writing shim. Preserve original call placement/return addresses in the
native comparison and qualify the current imported decrement before/after.
Do not equate the Source C++ stack with native W. Record actual invocation,
sampler, loader, registry node/factory, returned owner, cache record, binding,
pass byte80, builder counters and the Source observed-release frame.

For the genuine fog contents, successful nonnull binding increments builder
`+8C` and pixel counter `+94` once, leaves vertex counter `+90` unchanged,
adds one binding and no texture-reference row, and executes no sampler-state
setter. Verify those actual deltas. An independent comparison run must obtain
its owners through the same genuine setup/load operations; do not decrement
one obligation twice or reset counts/caches to manufacture equality.

The five physical `.shfx` files containing `TEXSRC_TEXANIMATOR` are pinned.
Fog has only Source1; the two per-vertex underwater files have Source1 then
Source2 with states; both Shore files have Source3 entries followed by three
Source1 entries with states. Shore's actual `dummy`/`shore_map` combiners add
no Source0. State calls establish their own later `B-16=00B3B36E`, so they
would not demonstrate preservation of the release word. Do not splice a
following Source0 into any of these lists. The first fixture can prove the
whole Source1 parent and direct word while leaving downstream use untested.

Keep failures explicit. A cache/creator-registration failure can retain real
creator/cache/name obligations; binding failure leaves the caller's actual
owner obligation in the parent frame. B44CF0 frees its raw binding only on
constructor failure, while the constructor cleans its own name. Completed
bindings/cache entries are not rolled back. A failed observed zero dispatch
has already decremented once; retain its phase and never retry that release.
Resolve actual residuals before acknowledging diagnostics, or retain the
failed process/domain as existing policy requires.

No actual selected descriptor, loader owner, binding, native B-16 observation,
Original/Source runtime provider closure, zero-terminal execution, whole caller
ABI/FH3/SEH, full material compilation, GPU output or gameplay was admitted by
this audit. The next work is a concrete genuine setup/application fixture,
not another Source reconstruction of the already available factory/loader/
binding bodies. See the [report](../reports/native_sampler_source1_owner_binding_readiness_cc12.json)
for byte/file receipts and the remaining executable prerequisites.
