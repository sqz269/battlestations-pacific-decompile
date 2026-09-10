# Material entry selection and draw orchestration

`MaterialEntryDispatcher` implements the effect selector at `00B45360` and the
state/draw sequence at `00B44750` / `00B43410` around explicit required native
operations. It derives `RenderBatchMaterialDispatch`, so the recovered batch
executor can call it directly. This is a bounded orchestration fragment: the
large `00B42350` constant builder, effect clip-plane construction, dynamic model
operation, callback owner and diagnostic registry are still required inputs.
It is not a runnable complete native material pipeline or a binary replacement.

## Evidence and original ABI

Read-only Ghidra batches verified project `bsp`, saved project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language and
image base `00400000`. Fourteen complete code/data ranges match the installed
PE with SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The audit records exact range lengths, hashes, annotation preimages/proposals,
and local validation artifacts. No Ghidra or ledger writes were performed by
this packet. Descriptive application names are hypotheses.

| Address | Original ABI | Recovered role |
|---|---|---|
| `00B45360` | ECX effect, stack entry, RET4 | Effect virtual+14 selector |
| `00B44750` | ECX selected pass, stack entry/override, RET8 | Geometry, effect clip state and apply call |
| `00B43410` | ECX selected pass, six stack arguments, RET18 | States, shaders, textures, constants, callback, draw/statistics |
| `00B42350` | ECX selected pass, stack entry/override, RET8 | Required shader-constant builder |
| `00B855A0` / `00B855F0` | ECX section, RET | DWORD instance count+1C / byte indexed+58 |
| `00B48D50` / `00B48DE0` | ECX stream, RET | Vertex base+70 / index base+20 |
| `00B7AAE0` | No object/stack input, RET | DWORD global0109019C |
| `00B172C0` / `00B17320` | ECX object, RET | Pass effect owner+14 / material DWORD+104 |
| `00B16F80` | ECX diagnostic, six stack arguments, RET18 | Required effect/string/mode draw statistics |

The six apply arguments are entry, override, indexed flag, index stream,
index base and vertex base. The index-stream argument is unused in the apply
body. Its code ends at `00B43667`; the six-DWORD primitive jump table occupies
`00B43668..00B4367F` and is included in the byte identity.

## Selector behavior

1. Read the entry camera's DWORD mode+198. Mode2 with nonnull effect+138
   dispatches that special pass with null override and returns immediately.
2. Otherwise, if the root descriptor at effect+C4 has `FinalLODFadeOut` byte+16,
   visibility is below1 and mode is6, skip. This is distinct from the Lua
   `VisilityFade` option. Assembly uses COMISS/JBE; unordered visibility takes
   the regular path, as does visibility equal to or above1.
3. Capture model+170 collection before the optional effect+08/mode0 operation.
   That operation calls model virtual+48 with camera and entry flags, then
   renderer virtual+C8 with its result. Its complete owner/ABI interpretation
   is unresolved and requires the actual implementation.
4. Without a collection, reread the entry's live camera mode and visibility.
   Mode0 below1 selects effect+100; every other case selects effect+C8+4*mode.
   A null selected program skips the draw. No fixed NORMAL substitution occurs.
5. A nonnull empty collection skips. For a nonempty collection, reload its
   **front** sentinel node on every iteration; this is not indexed traversal.
   Call that override's virtual+C with the current global0109019C. If AL is
   nonzero, reread mode/visibility and select as above. Increment the unsigned
   counter and compare with the captured collection's newly read count.
   Callbacks can rotate the front or change count/mode/context. Replacing the
   model's collection during the earlier model update does not replace the
   captured collection. A malformed nonempty sentinel is an explicit host error
   in place of native `00BF6713` exception behavior.

## Geometry and apply sequence

The geometry owner resolver supplies the actual section/material/stream list,
layout and optional geometry+60 index stream. Generated instance geometry can
supply its retained mesh and instance streams in order; this module does not
guess that every source section has exactly two streams. The source section's
byte+58 indexed decision must also be supplied because it is absent from the
existing generated-section projection.

An empty stream list skips. The first stream's virtual+28 result is its byte
offset+5C (`00B48D10`); `FFFFFFFF` skips. A noninstanced section binds all streams
then resets frequencies0 and1 to1. An instanced section binds each stream and
sets frequency to tag|1 only when tag is exactly `80000000`; every other tag
uses tag|the live section instance count. This is an equality test, not a mask.

The selected pass's retained effect owner is compared with the same
global0108FBF4 projection reset by batch execution. A change stores the new owner
**before** its plane operation. If effect+13C is zero, or mode-specific distance
is nonpositive/unordered, call `00B25080` to restore pending planes. This is not
an unconditional disable. Otherwise the remaining native owner/matrix chain
constructs an extra plane and calls `00B25040`; that branch is required through
`construct_and_append_effect_plane`. Both operations must use the actual
`D3D9CameraFrameAccess` companion attached to the same renderer cache.

Bind section layout, obtain optional index base (zero for null index), bind the
index stream with the first stream's signed base-vertex bit pattern, and choose
indexed drawing only for a nonnull index stream and nonzero section+58.
Then the apply body performs, in order:

1. Bind render block and sampler block; set `D3DRS_ALPHAREF` only when signed
   material+104 is nonnegative; bind the retained logical VS/PS wrappers.
2. Process texture references in source order. Pixel usage tests bit
   `(reference ordinal & 31)`; unused pixel references still advance the pixel
   sampler counter. Vertex references always bind, starting at logical sampler16.
   Negative reference indices use effect getter `00B17D90(-1-index)`; nonnegative
   indices check the material's signed16 count, with missing slots binding null.
   The existing static-only texture helper does not cover negative indices,
   so the complete evidenced loop is present here.
3. Invoke the **required actual** `00B42350` builder to patch shared VS/PS float
   blocks in place. It must cover dynamic textures, reflected material parameters,
   model/camera transforms, visibility/lighting and shadow-specific inputs used
   by the selected pass. Existing parameter/decode/shadow fragments alone do not
   establish this whole function. Clearing or manufacturing constants is not
   a valid implementation of this dependency.
4. Compute signed32 upload counts from byte VS/PS end-register minus shared
   first register. Upload only selected positive spans. After a VS upload,
   native rereads first register and PS end for the PS gate, but passes the
   **previously computed PS count**. The implementation preserves that schedule,
   including the unusual case where callbacks change the shared first register.
   It neither resizes nor clears the blocks and rejects uncovered spans.
5. Invoke the actual material+08 callback when present. Assembly has ECX=entry
   and no explicit stack arguments at CALL EAX; no calling convention is guessed
   for arbitrary raw function bits.
6. Indexed draw arguments are primitive, range+0C minimum vertex, range+10
   vertex count, range+14+index base, range+18 primitive count. Nonindexed draw
   uses range+0C+vertex base and range+18. DWORD additions wrap. Statistics are
   called even after the renderer's draw gate skips or a COM draw fails.

For nonindexed statistics, primitive types1..6 yield respectively n,2n,n+1,3n,
n+2,n+2 vertices, with DWORD wrap; unknown types yield0. Indexed statistics use
the section vertex count. `00B16F80` gates on diagnostic+681, finds an effect and
current diagnostic+684 string record (last match), creates it if absent, then
updates fourteen-mode arrays at record+04/+3C/+74/+AC/+E4 for draw count,
primitives, vertices, VS count and PS count. The registry/string allocation and
owner identity remain a required actual diagnostic operation, not a dummy map.

## Integration and limits

`MaterialEntryProgram` owns a `CompiledMaterialPass`, stable logical shader
wrappers and an aliasing shared pointer to its sampler block. Keep the program
alive while the state cache borrows its shader wrapper, including when another
program with equal COM shader pointers is bound: the cache may retain the older
wrapper identity. Successfully compiled passes have actual VS/PS objects; the
module does not implement missing logical-shader constructor states.

The effect view's fourteen programs, alternate/special programs, descriptor flag,
update flag and clip fields must come from the actual retained effect state.
Aggregate defaults are not the native effect constructor/loader. All selected
programs share the same `CompiledMaterialEffect` identity. Owner resolution is
side-effect-free. Entry/model/collection/program storage must survive callbacks,
and geometry owner bindings stay stable during each bind/draw. Instance count,
section range words, camera mode and collection state retain the documented live
reads. Malformed native negative counts, mode indices beyond13, more than four
streams and sampler bank overflow are outside the checked projection.

COM failures exposed by existing cache methods are accumulated while subsequent
native stages and selected overrides continue. The new bool reports the first
failure after that entry completes. Owner/backend failures stop further work
with earlier effects retained. This error policy, shared ownership and checked
container bounds are new C++ behavior, not original native ABI/SEH guarantees.
The primary integrator owns adding the source to CMake and real owner adapters.

## Validation

- Standalone MSVC Win32 `/std:c++17 /MD /W4 /WX /fp:strict` compilation passes.
- `scripts/build.ps1` passes the existing baseline build and reconstructed math
  test. The worker did not edit the primary-owned CMake source list; standalone
  compilation verifies the new translation unit separately.
- One ignored local differential fixture executes the 359-byte native selector
  in isolated executable memory with explicit synthetic callee adapters. Its
  21 cases and24 selected pass/override identities match this implementation:
  all14 modes, special precedence, final-LOD skip, alternate mode0, quiet NaN,
  front/count/context rereads and captured collection before model update.
  The fixture uses a synthetic COM device and constants recorder solely to
  observe the selector; this does not validate `00B42350`, clip mathematics,
  actual draw pixels, installed-resource material rendering or gameplay.
- Full byte equality and assembly inspection support the geometry/apply
  orchestration. Those complete native functions were not differentially
  executed by this selector fixture. No full pipeline, visual parity, ABI
  compatibility or game validation is claimed.
