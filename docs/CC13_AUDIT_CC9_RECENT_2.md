# Audit of cc9's recent landings, part 2 (cc13-audit8)

Addresses: 00ADE820, 00ADA230, 00B6F8F0, 00B6F150, 00BA1370, 00BA0C00, 008830AA, 008FFF20, 0085A830

Reviewer: cc13-audit8 (Claude lineage cc13), at cc9's request. Read-only on cc9's files: every
finding is a verdict with instruction-address evidence, and every proposed change is a unified diff
below. Ghidra was read only (`show --asm --live`, `disasm`, `bytes`, `xrefs`, `proto`), after the
`_free` repair all-clear. Nothing here was built or run; all verdicts are static (exported/listing
evidence), not game-validated.

| # | Item | Verdict |
| --- | --- | --- |
| 1 | Wave-damp region box `00BA0C00` via `00BA1370` (`kWaveFieldLiveAmplitudeBound`, OFF) | **DEFECT** (latent while OFF): wrong translation frame; the mesh set matches |
| 2 | `kTorpedoBotReadyGateBound` (GUNNERY 161, ON) | **FAITHFUL** for the gate; three notes outside it |

## 1. Wave-damp region box: which meshes, which frame

This settles item 7 of `docs/CC13_AUDIT_CC9_RECENT.md` (audit6), which was UNCERTAIN. Audit6 took the
argument of `00BA1370` to be a single terrain `cMesh`. It is not one: it is the root of a cloned node
tree, and `00BA0C00` recurses over that tree.

### 1.1 What the node at `landscape+3CCh` is

**`00ADE820` `BSP_TerrainHeightField_AttachModel`.** It is called at `00883058` with
`(name, model instance, w, d, frame = &landscape+74h)`; the frame is pushed by `00883014 LEA ECX,[EBP+74h]`.
- **`+64h`.** `00ADE842..00ADE857`: the model instance (argument 2) is stored at `+64h`, with a reference
  count.
- **`+2Ch`.** `00ADE880..00ADE8CB`: a plain node (`BSP_Node_Construct 00B6F5A0`, base vtable `00D62C88`) is
  built, named by the string at `00CF00D8` ('Terrain Visibility Group'), and stored at `+2Ch`.
- **The frame.** `00ADE8F7..00ADE909`: `vt+38h(frame)` writes argument 5 into that node's LOCAL matrix
  (`[esp+4Ch]` is argument 5 at that depth). `00B6D890` follows.
- **The model.** `00ADE912..00ADE91C`: `BSP_Node_SetParent 00B6E680` puts the model instance's root node
  `[model+0Ch]` under the visibility group.

**`BSP_Landscape_LoadTerrain`, `0088306A..00883086`** (taken only when `[00E188A8]+19F0h` is non-zero, `00883062`).
- **Which node.** `[landscape+3D0h]` is the height field; its vtable is `00D5D350`, stored at `00ADD2C9`.
  Its slot `+40h` is `00ADA230`, `MOV EAX,[ECX+2Ch]; RET`, so the call returns the visibility group.
- **The clone.** The group's `vt+10h` (base vtable slot `00D62C98` -> `00B6F8F0` `BSP_Node_Clone`) runs with
  `(flags 6, parent 0)` (`0088307F PUSH 6`, `00883079 PUSH EBX`; EBX is 0 from `00882AEA XOR EBX,EBX`, its
  only write in the function). `00B6F8F0` allocates and constructs a node and then calls
  `BSP_Node_CopyCloneBaseState 00B6F150`.
  - `00B6F223..00B6F244`: bit `20h` of the flags is clear, so each child (`[src+34h]`, then `[child+3Ch]`) is
    cloned recursively with the new node as its parent.
  - `00B6F246..00B6F24D`: `SetParent(parent = 0)`.
  - `00B6F260..00B6F26E`: `vt+38h(src+B0h)` copies the source's local matrix.
- **The result.** The clone is stored at `+3CCh` (`00883086`), then passed through `00B6D890(0)`
  (`0088308C`) and to `00BA1370` (`008830AA`, the only caller Ghidra lists).

So `[landscape+3CCh]` is a parentless deep clone of the visibility group's subtree. It is not a
single mesh. `BSP_Transform_RefreshWorldMatrix 00B6DB70` copies local to world for a node with no
parent, so the root's world matrix is the `+74h` frame.

### 1.2 What `00BA1370` and `00BA0C00` do with it

- **The call.** `00BA1370`: `00BA1372 MOV EDI,[ESP+0Ch]`, then `PUSH EDI; PUSH EDI`, then `CALL 00BA0C00`.
  Both arguments are the clone root. The root is then appended to `field+5F4h`.
- **`00BA0C00(root, node)` is recursive.**
  - `00BA1331..00BA1352`: `EAX = [node+34h]` is the first child, and `ESI = [EAX+3Ch]` is the next sibling,
    loaded before the call so the walk survives the detach below.
  - Each recursive call pushes `(EBX = [EBP+8], child)` (`00BA133B`, `00BA1345`, `00BA1346`). Argument 1
    stays the clone ROOT at every depth.
- **Which nodes contribute.**
  - `00BA0C24..00BA0C3E`: a node takes part only when `vt+0Ch(00B74310())` accepts it. `00B74310` returns
    the type `[01090034]`, the geometry base that the skinned, matrix-indexed, particle and cloud type
    infos also derive from.
  - Its geometry is `[node+180h]` (`00B74650`, `00B74640`). Its sections are walked through `00B72B40` and
    `00B732C0`.
  - Every geometry node in the whole tree is visited.
- **Which vertices.** The box loop `00BA0EB0..00BA0FB6` reads raw stream positions (`004768D0`). It never
  applies a node matrix.
- **Which translation.** `00BA0FC5 MOV ESI,[EBP+8]` is argument 1, the clone root. `00BA0FD9`/`00BA1002`
  add `[ESI+120h]`/`[ESI+128h]`. These are world-matrix elements 12 and 14 (world at `+F0h`), refreshed
  through `00B6DB70` when `+5Ch` bit 2 is clear (`00BA0FC8..00BA0FD0`). That is the landscape `+74h`
  frame's x/z translation.
- **Side effect with no box effect, unmodelled in the host.** When a node has damp sections:
  - `00BA0E3A..00BA0E41` detaches the node (`SetParent(0)`);
  - `00BA0E46..00BA0E58` re-registers it under the field's render root (`field+2BCh` -> `00BA7170` ->
    `00B6D890`);
  - `00BA0E5D..00BA0E96` appends `(root, node)` to `field+614h`.

### 1.3 Comparison with the host (`src/ocean_coverage_regions.cpp`, commit `eb9fbc91b`)

- **Mesh set: same.** The host visits every hierarchy item's mesh resources. The image visits every
  geometry node of the cloned model tree, which those items produce.
- **Vertices: same.** Both use raw stream positions with no item matrix.
- **Translation: different.**
  - The host adds `world[i][12]` and `world[i][14]`, where `world[i] = item.matrix x parents x landscape_world`.
    That includes every item's local translation, rotated by its parents.
  - The image adds only the root's translation, which equals `landscape_world[12]` and `landscape_world[14]`
    because the host's scene-contents caller passes `object.world`, the `+74h` frame
    (`src/game_hosts_scene_contents.cpp`, the `008830aa` call).
  - The two agree only for a damp item whose chain of local translations is zero.
- **Refusals: the host's are extra.** The host refuses a damp section whose item has no world frame
  (`!placed[i]`). The image needs no item frame, so it keeps those sections.

**Verdict: DEFECT, latent while `kWaveFieldLiveAmplitudeBound` is OFF.** Fix it before the flip.
Audit6's conditional diff becomes unconditional, and the refusal and the unused world loop go:

```diff
--- a/src/ocean_coverage_regions.cpp
+++ b/src/ocean_coverage_regions.cpp
@@ -106,17 +106,6 @@ bool read_position(const MeshVertexStreamPayload& stream, int type, std::uint32_
     return true;
 }
 
-// Row-vector 4x4 product a * b.
-void multiply(const float a[16], const float b[16], float out[16]) {
-    float r[16];
-    for (int i = 0; i != 4; ++i)
-        for (int j = 0; j != 4; ++j) {
-            float s = 0.0f;
-            for (int k = 0; k != 4; ++k) s += a[i * 4 + k] * b[k * 4 + j];
-            r[i * 4 + j] = s;
-        }
-    std::memcpy(out, r, sizeof(r));
-}
 
 std::string base_name(const std::string& effect) {
     const std::size_t slash = effect.rfind('/');  // 00CE7898 is "/" alone
@@ -225,22 +214,8 @@ std::string ocean_coverage_add_landscape_00ba1370(OceanCoverageRegions& out,
         return "regions=0 (model parse: " + error + ")";
     }
 
-    // World frames, parents first (every item's parent index precedes it).
-    std::vector<std::array<float, 16>> world(model.hierarchy.size());
-    std::vector<bool> placed(model.hierarchy.size(), false);
     std::size_t added = 0, sections = 0, refused = 0;
     std::string notes;
-    for (std::size_t i = 0; i != model.hierarchy.size(); ++i) {
-        const HierarchyItem& item = model.hierarchy[i];
-        if (!item.matrix) continue;  // native leaves it uninitialised: no claim
-        const float* parent = landscape_world;
-        if (item.parent != 0xffffffffu) {
-            if (item.parent >= i || !placed[item.parent]) continue;
-            parent = world[item.parent].data();
-        }
-        multiply(item.matrix->data(), parent, world[i].data());
-        placed[i] = true;
-    }
     for (std::size_t i = 0; i != model.hierarchy.size(); ++i) {
         const HierarchyItem& item = model.hierarchy[i];
         for (const std::uint32_t resource : item.resources) {
@@ -251,12 +226,6 @@ std::string ocean_coverage_add_landscape_00ba1370(OceanCoverageRegions& out,
                 if (base_name(subset.effect_name).find("oceanwavedamp") == std::string::npos) continue;
                 ++sections;
                 char line[160];
-                if (!placed[i]) {
-                    ++refused;
-                    std::snprintf(line, sizeof(line), " [%s: no world frame]", item.name.c_str());
-                    notes += line;
-                    continue;
-                }
                 std::uint32_t stream_index = 0;
                 const MeshTextureRequest* texture = nullptr;
                 for (const MeshSubsetEvent& event : subset.events) {
@@ -294,9 +263,14 @@ std::string ocean_coverage_add_landscape_00ba1370(OceanCoverageRegions& out,
                     notes += " [" + item.name + ": vertex bytes short]";
                     continue;
                 }
-                // 00BA0FD9..00BA1077: + [node+120h] / [node+128h].
-                const float node_x = world[i][12];
-                const float node_z = world[i][14];
+                // 00BA0FC5..00BA1077: + [root+120h] / [root+128h]. ESI = [ebp+8] is
+                // argument 1, the clone root 00BA1370 passes at every recursion depth
+                // (00BA133B..00BA1349): a parentless clone (00B6F24D) of the visibility
+                // group whose local matrix is the Landscape +74h frame (00ADE900), so
+                // its world translation is landscape_world's. Item matrices are never
+                // applied. docs/CC13_AUDIT_CC9_RECENT_2.md item 1.
+                const float node_x = landscape_world[12];
+                const float node_z = landscape_world[14];
                 x1 = node_x + x1;
                 z1 = node_z + z1;
                 x0 = node_x + x0;
```

The diff was generated with `git diff --no-index` from an edited copy. `multiply` has no other user, so the diff
removes it. `<array>` is still used at the `red` palette. Build status of the patched copy: see
"Build check" below.

**Exposure check (optional, not needed for the verdict).** Before applying the diff, log for each damp
section the pair `(world[i][12] - landscape_world[12], world[i][14] - landscape_world[14])` in
`ocean_coverage_add_landscape_00ba1370`. Every non-zero pair is a box that the current host places
elsewhere than the image does.

**What the static read did not settle.** It assumes the host's `object.world` equals the `+74h` frame.
The host comment in `game_hosts_scene_contents.cpp` says so for a top-level Landscape, and this audit
did not re-derive it. `00ADE500` (the heightmap load), which runs between the attach and the clone, was
not read for extra geometry children. A heightmap node with an `oceanwavedamp` effect is unlikely.

## 2. `kTorpedoBotReadyGateBound` (GUNNERY 161, ON)

**Host:** `src/game_hosts_gunnery.cpp` around the `cc9_g34_torpedo_bot_ready` block (the
`image_ready` computation and `if constexpr (kTorpedoBotReadyGateBound) want_fire = image_ready;`),
`gun_bot_torpedo_ready_008fff20` and `gun_bot_heading_accepted_008fff20` in `src/gun_bot_ticks.cpp`.
**Image:** `008FFF20` `BSP_GunBot_HeadingAimTick`, body `008FFF20..0090099B`.

| Check | Image | Host | Result |
| --- | --- | --- | --- |
| Heading source | `00900380 CALL 0085AB50(raw, [00E0B588] = pi/4)`, then `0090038D FST [ESI+60h]` | `want_horz` is replaced by `gun_snap_heading_to_fire_window_007f6190(arcs, raw, kQuarterPi)`. No later writer for category 7: the swing error is category 1, the flak error 5/6, and artillery is 2/3/4/6/9 | match |
| FLT_MAX abort | `00900392 FLD double [00D7A278]` (`47EFFFFFE0000000` = FLT_MAX), `FUCOMIP`, then `LAHF; TEST AH,44h; JP 009003CE`. Equal falls through to `vt+1E8h(0)` and `RET`; unequal or NaN continues | A failed snap sets `have_target = false`, so `image_ready` is false and the request is cleared. The helper's `!= kGunBotFloatMax` is true for NaN as well | match |
| `0085ABA0` answer | `009003DD` call; `009003E2` reloads ECX and never tests AL | The ON path does not use `accepted` | match |
| Angle test | `009003F8 00438B10(bot+60h, [gun+480h])`, `AND 7FFFFFFFh`, `COMISS [00CE3984]` (`3C8EFA35` = 1 degree), `JBE 00900974` (abort when `|d| >= 1 deg`, or unordered) | `fabs(wrapped_angle_subtract_00438b10(want_horz, gun.angles.horz)) < kGunBotOneDegree`, with the same argument order. NaN gives false | match |
| Window test | `00900422..00900433 vt+1D0h(1)` = `0085A830`: `gun+490h` refuses, then `007F60A0` at the current `+480h`/`+484h` (`0085A86C..0085A887`), then the tail `00729A80(1)` | `may_fire_here = gun_fire_allowed_007f60a0(arcs, horz, vert)`. The `+490h` and `00729A80(1)` reload conjuncts are left to the fire pass's `gun_can_fire_turning_0085a830(..., true)` | match for the window. Note (a) |
| Inhibit | `00900847..00900888`: `[gun+3F0h]+634h`, `SHR 2; TEST DL,1` (**bit 2**), tested **after** the jitter draws | `!inhibited`, with `inhibited` the constant `false` | no effect now. Note (b) |
| RNG | No draw in `009003A0..00900433`. The first draws are `00900830 00BD2F10` with `ECX = 1` (`00900825`) and `0090083E 00BD2FC0` with `ECX = 1` (`00900835`), after the friendly-crossing walk | The gate draws nothing | match. Note (c) |
| Scope | The TorpedoBot tick | `torpedo_gun && category == 7 && !player_seat` | as GUNNERY 161.3 states |
| OFF path | | `image_ready` comes from pure functions, and only the `torpedo_ready_*` counters and one `done()` record change. `want_fire` keeps the generic value | equivalent |

**Verdict: FAITHFUL.** No diff for the gate.

**Notes, outside the gate (no change asked):**
- **(a) The reload conjuncts are checked later in the host.** The image's `vt+1D0h(1)` also refuses on
  `gun+490h` and on `00729A80(1)`, the reload and cooldown gate. During a reload the image clears the
  request every tick (`00900974 vt+1E8h(0)`). The host keeps the request high and lets the fire pass
  (`can_fire`, `gun_can_fire_turning_0085a830(..., true)`) refuse.
  - The generic OFF test already worked this way, so the substitution predates this switch.
  - It matters only if the request latch (`0072D2C0`) treats a request held high differently from one that
    is cleared and set again, for example in the timing of its `fire_stagger` draw.
- **(b) The inhibit bit for this bot is bit 2, and it is tested after the draws.**
  - The AA gunner tests bit 0 of `unit+634h`; the torpedo bot tests bit 2 (`00900882 SHR EDX,2`).
  - The torpedo bot tests it after the two stream-1 draws. An inhibited launch therefore still consumes two
    draws.
  - This matters only if the host's `inhibited` is ever wired.
- **(c) The AngleErr jitter belongs to `kTorpedoGyroHeadingBound`, not to this gate.**
  - The image draws `00BD2F10(lo, hi)` and then `00BD2FC0 & 1` on stream 1. Bit set gives +1 and bit clear
    gives -1 (`00900843..0090084F`).
  - The host draws twice through `draw(Draw::torpedo_gyro, ...)`, the single stream-1 LCG by default, and
    takes the sign from a float `< 0.5`. The distribution is the same, but a different bit of the draw sets
    the sign.
  - The host draws only when a shot is created. The image draws on every attempt that passes the friendly
    walk, including attempts that `00900888` (inhibit) or `00417940` (`009008DF`, unmodelled) then abort.

## Coverage

| Routine | Coverage |
| --- | --- |
| `00BA1370` | complete (`00BA1370..00BA13B8`) |
| `00BA0C00` | partial. Read in full: the node walk, the type gate, the detach and re-register, the box loop, the translation and the recursion. Taken from audit6 and not re-read: the section filter `00BA0C95..00BA0E20` and the mask and texture tail `00BA109D..00BA1317` |
| `00ADE820` | partial: `00ADE820..00ADE989`. `00ADE500` was not read |
| `00B6F150` | the clone path for flags 6 with parent 0 only |
| `008FFF20` | `00900380..00900433`, `00900800..0090099B`. The friendly walk `0090043D..009007F6` was not re-read (it is GUNNERY 125/161 territory) |
| `0085A830` | complete (`0085A830..0085A89F`, disk bytes) |

## Build check

The item 1 diff was applied to this worktree's copy of `src/ocean_coverage_regions.cpp`, built with
`./scripts/build.ps1` (exit 0, no warnings in the filtered output, 2 of 2 tests passed), and then reverted
(`git checkout`). Nothing of cc9's is committed. The patched behaviour was not run: it is build-tested
only, and it stays OFF behind `kWaveFieldLiveAmplitudeBound`. `tools/verify_report_calls.py` reads a
`reports/*.json`. This audit has no report file, so the verifier was not run on it.
