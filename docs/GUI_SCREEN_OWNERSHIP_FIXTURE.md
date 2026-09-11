# Combined Screen ownership fixture

One ignored Win32 fixture passed against root integration `74e2322` plus group
bounds commit `356ce9d` (fixture checkout merge `886171f`). It exercises the real
`GuiScreenSceneRuntime` supplied services through `GuiTypeDispatchFactory` and
the same `GuiWidgetOwnerRuntime` that owns each layout tree. No production
source change or permanent test was needed.

The fixture is retained under `local/gui_screen_ownership_probe.cpp`, with
`local/build_gui_screen_ownership_probe.ps1`, its log, and the native-value
preparation script/header/evidence. The report records their hashes. Reproduce
from this worktree with:

```powershell
python local/prepare_gui_screen_ownership.py
python tools/ghidra_export.py verify-seeds
./scripts/build.ps1
./local/build_gui_screen_ownership_probe.ps1
```

## Actual ownership exercised

The two Screen pages have the exact constructor share byte `81h`, flags `10h`,
RenderOrder `2`, and differing Priority values `7` and `99`. Each has one actual
group root and one actual model child. The first acquisition finds no store and
creates the actual outer `NativeGuiSceneOwner`, weak handle, camera, replacement
viewport, fog, GuiLights resource, ambient owner and directional light. The
second finds that same canonical map record and takes the shared early return.

All node owners use the same `SceneAttachmentRuntime`,
`GeneratedModelLifetimeRuntime`, `NativeNodeDestructionRuntime` and allocator
list. The actual group, model, camera, directional and weak pools are initialized
and destroyed by their recovered owner APIs. Type IDs come from the actual
`TypeIdCounterLifetime` and type bootstrap functions. The weak mutex and type
counter use the same `SingletonLifetimeDomain` and real deleting destructors.
The directional resolver borrows the existing actual light companion through
the canonical node lifetime registry.

| Observation | Result |
|---|---|
| Owned acquisition | One store; actual scene count 1; camera and directional initial counts 1 |
| Camera setup | Actual flags `16h`, color 0, viewport count 1 and dimensions 800 by 600; actual fog present |
| Lighting/bounds | Actual directional direction `(0,0,1)`, base diffuse `(0,0,0,1)`, ambient `(0.5,0.5,0.5,1)`; actual group sphere `(0.5,0.5,0,1)` |
| Shared acquisition | Identical store and outer scene pointers; actual scene count 2; differing Priority still shares |
| Shared early return | Directional allocation, radius and bound-write counts remain 1; renderer parameter count remains 6 |
| Visibility | Two visible edges increment the same store count from 0 to 2; both supplied hook observations run |
| Shared disposal before derived release | Shared root and child bindings are null; their companions are gone; three scene roots remain: camera, light, owned root |
| Shared disposal after derived release | Scene count 1; same store and weak target remain live; camera/light companions remain |
| Owned disposal before derived release | Both GUI trees are gone; exactly camera/light roots remain; scene count 1 |
| Final owned disposal | Store removed; scene final zero retires camera/light companions; retained actual weak handle has null target and count 1 |
| Final handle release and allocator trim | All five native pools have zero live slabs; shutdown clears both actual singleton publications |

The only observer retain is `NativeWeakOwnerDomain::retain_handle`. There are
**no additional scene, camera, light, group or model retains**. Thus the fixture
cannot hide premature scene-root destruction behind protective node references.
The pre-derived wrapper checks state, then calls the original real release
service unchanged; it neither inserts nor substitutes a lifetime operation.

Disposal follows `GuiWidgetOwnerRuntime::retire_tree`, the recovered manager
composition: `00AA31F0` calls current `+20` at `00AA326A` before deleting `+04(1)`
at `00AA3276`. The former runs the pure recursive `00AA8320` logical release;
the latter reaches derived `00AC5480`, then base `00AA9730`. Shared disposal
precedes owned disposal in this one scenario. Native derived teardown does not
decrement the store's visible count: it remains 2 after shared destruction until
the owned layer removes the record. This is observed, not repaired by the probe.

## Evidence and boundaries

The preparation helper uses read-only `bsp.py ghidra bytes`, which verifies
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, for each query.
Twenty table/constant spans, 596 bytes total, match the installed PE. Complete
required profiles are captured for node, camera, model, group, directional and
base light; the outer scene captures its two required dispatch entries. The
literal radius is the actual double `2.0`. No Ghidra writes or annotation changes
occur in this packet.

This packet adds a fixture, not a new recovered function or original ABI.
Relevant existing reconstructions are `00AC59A0` (ECX layer, RET, 1467 bytes),
`00AC5480` (ECX layer, RET, 213 bytes), `00B8E6C0` (ECX actual group, stack sphere,
RET 4, 44 bytes), plus the manager disposal call sites above. Names remain
hypotheses. Their byte/old-name/comment evidence remains in
`GUI_SCREEN_SCENE_RUNTIME.md`, `GUI_NATIVE_SCENE.md`, `GUI_CAMERA_STORE_OWNER.md`,
`NATIVE_WEAK_OWNER.md`, `DIRECTIONAL_LIGHT_REFERENCE.md` and `GUI_GROUP_BOUNDS.md`.

The renderer identity is a real D3D9 NULLREF device inside a real
`D3D9StateCache`. Its supplied parameter-access boundary returns fixture-owned
800-by-600 DWORD references. It observes six native calls: two in the initial
viewport constructor, the camera's height/width pair, and two in the replacement
viewport constructor. No device drawing or presentation occurs. This does not
validate renderer startup, its actual parameter-region dispatch, or pixels.

Page construction uses the recovered plain root fragment and evaluated
`GuiTable` values; it does not run Lua or the complete GUI manager page-list
implementation. The real property prepass sets applied Priority before
acquisition, so `00AA52A0` registration is not reached. Its supplied boundary
only records calls and is asserted unused. The visibility hook is a controlled
observer, not the game's script callback. Geometry and shadow owner resolvers
reject use; this scenario has neither retained geometry nor a shadow map.
The viewport view is explicitly bound to the actual published owner but is not
consumed by a render pass.

CRT binding uses the existing recovered `legacy_crt_87except_00c27489`, host
`_errno`, fixture runtime globals (sqrt bypass 1, conversion selector 1, matherr
bypass 0), default masked x87 state and MXCSR `1F80h`. The actual radius adapter
performs the shared sqrt kernel, float spill, truncating conversion and final
float conversion. This fixture does not independently compare the original CRT
or prove unmasked exception equivalence. The separate group-bounds differential
fixture's shared-sqrt limitation still applies.

The strict MSVC Win32 `/W4 /WX` build and both existing CTests passed after seed
verification; the ignored fixture compiled `/W4 /WX` with `/MANIFEST:EMBED` and
passed. This establishes bounded host ownership composition. It is not original
whole-function differential execution, a binary replacement, a queue/render
test, failure-unwind coverage, or game validation. In particular, the opposite
teardown order (creator removes the borrowed store while a shared layer remains)
is not exercised here.
