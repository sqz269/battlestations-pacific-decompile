# Installed ship lighting application composition

The installed `shaderfx/ship/ship.shfx` selects `lights/per_pixel_shadow.shfx` for
RM_NORMAL. Its vertex stage calls `GetAmbient(SYS.Normal)`, whose existing native
shader helper reads six `cAmbientCube` rows. The pixel stage reads
`cDirLightWorldSpaceDir`, `cDirLightDiffuseColor`, `cDirLightSpecularColor`, computes
ambient luminance, and divides ambient RGB by that luminance before multiplying the
textured diffuse color. These are real required shader inputs, not optional cosmetic
lighting. The original normal ship descriptor's other samplers remain unchanged.

Native B46A70 at B46ED1/B46EE2 explicitly skips its lighting writes when scene or
scene+1C is null. In that branch the following system registers retain the supplied
prefix preimage. Loader-zero banks are not a native light initializer; zero ambient
also permits a 0/0 in this installed combiner. The new graph supplies actual owners
that the existing gather reads instead of writing substitute shader constants.

## Actual producers and fields

`MissionShipLightingGraph` creates one actual3Ch D63168 scene resource via full
B83C50. That body creates its actual28h registry and actual98h D62F3C ambient via
B83600/B7C290, uses B825D0 for retained ambient assignment/backlink, then drops the
ambient creator credit. Existing `NativeSceneResourceReference` and the constructor's
`NativeAmbientReference` borrow the SAME raw+4 atomics and actual owner registry.

The graph initializes the canonical 01090154 directional-light pool once through
CD8080 and allocates its genuine1F0h slot through B7BAC0. The raw-header B7C6B0
constructor and `DirectionalLightOwner`/`DirectionalLightReference` retain the same
node/tail/pool storage. Light and directional type predicates use the existing
shared bootstrap; CD80A0/CD80F0 run only if their canonical guards require it. The
actual owner registry and shared node/lifetime runtime hold their normal companions.

The nonnull configuration branch of complete 004BACD0 supplies explicit diagnostic
ambient/cube RGB (1.00,1.12,1.28), diffuse (1.40,1.36,1.20), specular (0.24 each),
alpha/scales1, elevation0.6154797087 and yaw0.7853981634. It invokes genuine B7AF20,
B7AF40, six B7AF60 calls, 004B62E0, 004B4D80, B7AF90 and B7B010. All shader-consumed
ambient/mode3/cube and directional diffuse/specular/direction fields are produced by
that real implementation. These are named host diagnostic choices, not authored
USN04 weather or recovered defaults. Constructor-unwritten ambient+28 and light
specular/mode3/direction allocation preimages are preserved until the genuine
configuration route writes the relevant fields.

ON09's primary capture with the original lower diagnostic intensities remained
very dark. The installed `tex2D_sRGB` helper raises sampled RGB to power2.2, and
the normal ship combiner writes linear lit RGB without a final display conversion.
This ordinary diagnostic presents that result directly, so the source inputs now
use 4x ambient and 2x sun/specular intensity. This gain is an explicit diagnostic
choice, not original postprocessing/exposure or mission-lighting parity.

B45360's null model+170 branch still selects normal effect+C8 (or native fade+100)
for camera mode0. Its nonnull collection uses the same selection and passes the
first light as override; complete B42350 does not read that override argument.
The null per-model collection therefore does not select a different unlit shader.
No root-light propagation was added to change pass selection.

The command graph records first-frame stock D3D9 VS/PS constant readback and sRGB
write state after native queue execution, before overlay. It reports actual device
registers35/36/43/44/46/52 for fog/height/ambient/diffuse/cube/direction only when
the Get call succeeds. The gather frame's prefix is an input preimage and is not
misreported as an output snapshot. These reads change no native draw inputs.

The native ambient98h storage gains only trivial field lifetimes at +18/+28/+38
after B7C290; placement default initialization writes no byte. Its allocation is not
enlarged and no semantic SceneResource is overlaid. Gather consumes the actual raw
fields directly. Shadow+174 remains the constructor's genuine null: this graph
creates no shadow caster or alternate shadow texture. B46A70's optional shadow arm
therefore remains skipped; the normal combiner uses the existing service shadow
sampler and screen-to-texture matrix, while nonzero ambient gives defined lighting.

B83D50 registers the actual sun in the actual scene-resource light registry using
the genuine directional type predicate. That registry key is borrowed: the graph's
single native creator credit keeps the light alive. The first native list node must
contain that exact sun before publication. Gather then produces:

| Gather prefix byte offset (original stack offset) | Actual source |
| --- | --- |
| 2A0 (ESP+2B4) | existing CFAD80 exponent constant |
| 2B0..2BF (ESP+2C4) | ambient owner+18 (normal camera mode0) |
| 2C0..2CF (ESP+2D4) | first directional light+184 |
| 2D0..2DF (ESP+2E4) | first directional light+194 |
| 2E0..33F (ESP+2F4) | six ambient owner+38 float4 faces |
| 340..34B (ESP+354) | first directional light+1E0 direction |

The typed register registry maps these to cAmbientColor, cDirLightDiffuseColor,
cDirLightSpecularColor, cAmbientCube and cDirLightWorldSpaceDir. The native prefix
scratch base is ESP+14, so original listing stack offsets are 14h bytes larger.

## Source global and runtime bindings

The supported installed PE's mapped image was checked: E18AE0..E18B1B are
loader-zero .data. One intentionally retained source process owner supplies these
unused configuration fallback guard/color cells; no other application owner was
found. They are not read by the admitted nonnull branch. Direction F8758C has an
existing source owner, `kWorldUp00f8758c`, reflecting native CD2280 initialization;
the graph uses an immutable bit copy of that constant for the unread fallback.

Scale-ambient F88A0C aliases the loaded settings owner's +8C, so its loader image
does not describe its live value. The parent supplies the exact startup capture
`arguments.word00` as a uint8_t constructor input. The graph retains an immutable
read snapshot of that configured byte, with settings/device reset excluded while
the graph is live, and logs it. This is not a second writable settings owner. The
nonnull branch reads this captured byte and the borrowed real read-only D7A24C one.

Registry storage uses the application's existing raw_allocate/raw_free CRT source
bindings and existing invoke_native_invalid_parameter_00BF6713. The vendor STL
throw site B82DD0 is a qualified stock `std::length_error("vector<T> too long")`
binding. Its native category/message/nonreturn behavior is preserved, while native
RTTI/FH3/private exception object ABI is not claimed. It earns no reconstructed
game-body credit. Valid one-light registry construction does not reach length or
invalid-iterator failures. All source call frames are initialized, persistent host
scratch; they do not claim original private stack identity.

## Integration and retirement

Include `bsp/game_native_mission_lighting.hpp` at TU top level and the `.inc` after
the renderer/scene graph types, before mission_ship.inc. Construct metadata with
`MissionShipLightingGraph(log, graph, cameras, owners, raw_strings,
static_cast<uint8_t>(arguments.word00))`. Call
`create_and_bind(scene->raw())` AFTER attaching every model to the native-null outer
scene. This invokes genuine B723F0 to retain the new resource in outer+1C. B723F0
does not propagate roots; static models keep their genuinely null+170 and B45360's
unlit pass-selection route, while B46A70 reads the real global lighting owner.

After the last completed command/frame, call `retire_before_native_drain()` BEFORE
the existing scene detaches its model roots. Genuine B723F0(null) releases the
outer's resource credit and restores the scene's admitted null-lighting state.
B83EC0 removes the borrowed sun registry key; releasing the resource creator then
runs B83410/B82ED0 and the ambient terminal. The now-unregistered sun's one creator
credit retires through B6DFA0/B7C820 and its same pool. Keep all companions until
`after_native_drain()` marks closure. Interrupted construction/retirement requires
process retention and no implicit cleanup/replay.

## Evidence and validation

The live analysis batch reported `/battlestationspacific.exe` in the configured
`C:/Users/sqz269/bsp.gpr` project. Existing B83C50/B7C290/B7C6B0 exports, B46A70 raw
listing schedule and complete 004BACD0 source/export were reviewed. This packet adds
application composition; no existing Ghidra names/comments need replacement.
`git diff --check` is the local static check. Primary integration owns Win32 build,
existing checks, live terminal observations, screenshots and teardown verification.
Original binary caller/register/FH3 ABI and full mission weather/shadow parity remain
separate from this explicit source diagnostic.
