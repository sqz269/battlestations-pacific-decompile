# Game frame render tail (004e538e-004e54f0)

Addresses: 004de4b0, 00b19a10, 00af0450, 00af0c50, 004c1b90, 00941140, 004d8620, 004c6c70,
004e4a40 (read only), 00af0460, 00874640, 00b71530, 00b6da70, 00af6360, 00af63a0, 00af0900,
00af0990, 004c1130, 0051f520, 00b74640, 00b732c0, 00941070, 004d6790, 004d35d0, 005329c0,
00aa5840, 00aa3f70, 00a7b0a0, 00a7e490

Packet `game_frame_render_tail`. The eight leased addresses are the calls `BSP_Game_OnMove`
(004e4a40) makes after the last simulation call and before `BSP_Game_Render` (004ca440).
`docs/GAME_ON_MOVE_MAP.md` steps 21 to 24 are the anchor; this document replaces its one-line
summaries of those steps with recovered behaviour. Everything else named in the window
(00685c80, 004d80d0, 006840f0, 00776230, 004c40f0) is an external contract here.

## Frame order

Read from `local/onmove.txt` (`python tools/bsp.py disasm-raw 004e4a40 --length 2808`), because
the stored Ghidra body of 004e4a40 is eight bytes.

| Address | Call | Condition |
| --- | --- | --- |
| 004e538e | `f = [00f876a4] * (double)[00ce47a0]`, then `004de4b0()` -> `00b19a10(f)` | the simulation gate at 004e50b0 was taken |
| 004e53b4 | `004c40f0()` | the gate was not taken; the two paths join at 004e53bb |
| 004e53c3 | `00af0450([[game+5FCh]+1054h])`, ECX `00f8c274` | `game+5D4h == 0` fails; the branch runs when the state equals 0Dh |
| 004e53de | camera select: `d = [[game+19F0h]+A8h]`; `d+CCh ? d+D0h : d+C8h` | same |
| 004e540f | `00af0c50([game+19FCh], camera)`, ECX `00f8c274` | same |
| 004e5416 | `004c6c70()`, ECX game | unconditional |
| 004e542d | `00685c80(rawDelta)` | `00e198ac != 0` |
| 004e5434 | `004d80d0()` | unconditional |
| 004e5442 | `pending = false; 006840f0(&pending); 00e18cdc = 0` | unconditional |
| 004e5460 | drain loop: `00776230()` ECX `game+1EF0h`, `004c40f0()`, `006840f0(&pending)`, `00e18cdc = 0` | while `pending` |
| 004e548a | `pending = false` | unconditional |
| 004e548f | `004c1b90()` -> `00941140()` | unconditional |
| 004e549d | `004d8620()`, ECX game | unconditional |
| 004e54a2 | `004c1dd0()` -> `00be3660([0109db08])` | unconditional |
| 004e54b5 | `if (pending) skip the render block` | see below |
| 004e54bc | `00be3640([0109db14])`, `004ca440()`, `00be3660([0109db14])` | `!pending` |
| 004e54eb | `004ca1f0()` | unconditional |

**The render-block gate is dead.** The byte at `[esp+1Bh]` is the out parameter of 006840f0 and
the loop condition. 004e548a writes 0 into it on both paths that reach 004e548f (the fall-through
from the loop exit at 004e5488 and the `je` at 004e5453), and nothing between 004e548a and
004e54b5 takes its address: 004c1b90, 00941140, 004d8620 and 004c1dd0 take no pointer to it, and
00be3660 is a `__thiscall` that restores `esp`. So `jne 004e54e9` at 004e54ba is never taken and
`BSP_Game_Render` always runs on this path. The test is reproduced in the reconstruction because
it is in the binary, not because it can fire.

## 004de4b0 and 00b19a10, the particle clock

`004de4b0`: `undefined4 * __cdecl()`, RET, no arguments. Double-checked lazy singleton over
`DAT_00f8d420` under the `BSP_SingletonLifetime_GetManager()+10h` critical section, allocating
1Ch bytes through 00bf681b, storing `PTR_..._00ce7d38` at +0 and `PTR_LAB_00ce7d24` at +4 (two
vtables, so multiple inheritance or an embedded interface), zeroing +8h..+14h, then
`BSP_SingletonLifetime_Register`. Five callers; 004e4a40 is one.

`00b19a10`: `void __thiscall(this, float)`, RET 4. Stores the float at `this+18h`, then walks the
records at `this+8h` with count `this+0Ch` and stride 2Ch, calling `(*(void**)(record+28h))`'s
virtual +18h with the same float. Confirmed against the listing: `MOVSS [EDI+18h],XMM0`,
`IMUL EAX,EAX,2Ch`, `MOV ECX,[ESI+28h]` then `CALL [EDX+18h]` with the float pushed. The end
bound is recomputed from +8h and +0Ch on every iteration but the cursor advances from the old
base, so a sink that reallocates the array is not handled.

The argument is not a delta. `00ce47a0` holds the double `1000.0` (bytes
`00 00 00 00 00 40 8F 40`) and `DAT_00f876a4` is the global time float whose only writer is
00874640, so `00b19a10` receives the global time in milliseconds. The independent confirmation is
`docs/SHADER_CONSTANT_DISPATCH_ANALYSIS.md` line 67: the shader system-constant prefix at
00b46cb4 builds constant c33 `Time` and takes component **y** from `004de4b0() result + 18h`,
which is exactly the field this call writes. So the particle step publishes the frame's shader
time and fans it out to every registered emitter set.

Nothing in the leased window registers records into +8h/+0Ch; that happens elsewhere (004de610,
00b33a10, 00b3b280 and 00b46a70 also call the getter). The 2Ch-byte record layout beyond +28h is
not recovered.

## 00af0450 and 00af0c50, the foliage group manager (00f8c274)

`00af0450`: `void __thiscall(this, float)`, RET 4, twelve bytes, body `MOV [ECX+2Ch],arg; RET 4`.
The matching getter at 00af0460 is `FLD [ECX+2Ch]; RET`, and 00b46cdb calls it with
`ECX = [00f8c274]` to fill component **z** of the same c33 `Time` constant
(`docs/SHADER_CONSTANT_DISPATCH_ANALYSIS.md` line 68). The value comes from
`[[game+5FCh]+1054h]`. So +2Ch is a scalar shader input owned by the foliage manager and never
read by the group walk.

`00af0c50`: `void __thiscall(this, render_camera, secondary_camera)`, RET 8. The Ghidra decompile
is unreliable here: it loses four bytes of frame depth across the indirect `CALL EAX` sites, which
is why it prints `unaff_retaddr` and `(char)param_2`. The listing was used instead.

Manager fields: +4h group-pointer array base, +8h element count, +10h a sub-object reset by
`00af0900(0)`, +1Ch the current camera, +20h a frame counter, +24h the dynamic-buffer owner,
+28h the locked buffer, +2Ch the shader scalar above.

Group fields (`*(base + i*4)`): +ACh a float compared against `[00d7a218]`, which holds 0.0;
+18Ch a description object with a byte at +70h and a byte at +7Ah; +1F8h a quad count.

Pass one (00af0c91-00af0d7a), per group:

1. Skip when `desc+70h == 1 && camera->render_mode (+198h) == 0`.
2. Skip when `00af6360(ECX = group, camera)` returns 0.
3. `lod = 00b71530(ECX = render_camera, group->vtable[48h]())`.
4. When `0051f520()+4h` is set, `[00f8d394]->vtable[C8h](bounds, render_camera, lod)`. The three
   arguments are pushed before the inner `vtable[48h]` call, which itself takes none.
5. Visible when `lod != AAAh`; otherwise, only if `desc+7Ah` is set, retry with
   `00b71530(ECX = secondary_camera, ...)` and accept when that is not AAAh.
6. `00b6da70(ECX = group, (float)visible, 0)` — the boolean is widened with `CVTSI2SS`.
7. When visible, add `group+1F8h` to a running total.

Pass two (00af0d80-00af0dbe): when the total is non-zero and +24h is set,
`00b74640(0,0)` -> `00b732c0` -> `device+3Ch`'s virtual +10h allocates `total * 4` bytes; the
pointer goes to +28h and a stack flag records that a buffer is held. That flag reuses the second
argument's home slot, which is why the second camera can only be read before 00af0d85.

Then `if ((camera+5Ch & 2) == 0) 00b6db70(camera)`, `00b6fcb0(camera)`, `00b70490(camera)`.
`camera+5Ch` is the `valid_flags` field Codex already recovered in
`include/bsp/camera_transform.hpp`.

Pass three (00af0df0-00af0e31): groups with `+ACh != 0.0` and `+1F8h == 0` are queued through
`004c1130()`'s sub-object at +4h (virtual +4h, arguments `00af0990()` result and the group), then
that sub-object's virtual +8h is called once with 1.

Pass four (00af0e60-00af0e9b): groups with `+ACh != 0.0` and `+1F8h != 0` get
`00af63a0(ECX = group, render_camera)` and their `+1F8h` is summed.

Tail (00af0e9d-00af0f11): when +24h is set, unlock through `device+3Ch`'s virtual +14h if the flag
is held, clear +28h, then write `sum * 4` to `device+10h` and `sum * 2` to `device+18h`. Four
vertices and two triangles per unit is a quad, which with the segment keywords `treegroup`,
`particlefloating` and `atlas` makes this the camera-facing foliage impostor batch.

`00b71530` (external) lazily refreshes the camera frustum: it sets bit 2 of `camera+2F0h`, extracts
six planes from `00b70490`'s view-projection through `00b653f0`, stores seven entries into the
plane set at `camera+2F4h` via `00b658e0`, then classifies the bounds with `00b651e0`. Those two
offsets are `CameraFrameState::frustum` (+2F4) and its projection flag word in
`include/bsp/camera_frame_state.hpp`, so the first argument of 00af0c50 is a camera in Codex's
sense, and `game+19FCh` holds the render camera.

**Uncertain**: which of the two cameras is conceptually primary. The first argument gets the
matrix refresh and the unconditional cull, so it is treated as the render camera; the second is
consulted only for groups whose description sets +7Ah. Reflection or a gameplay-versus-free-camera
split both fit and neither is proven.

## 004c1b90 and 00941140, the sound request queue (00f89b34)

`004c1b90`: `int __cdecl()`, RET. The same double-checked singleton shape as 004de4b0, 28h bytes,
constructed through `CG_array_ctor_helper_00940dd0`. Twelve callers.

`00941140`: `void __fastcall(this)`, RET. Layout: +4h..+14h five cue identifiers, +18h..+1Dh six
request bytes, +20h a forced slot whose idle value is 6, +24h a context handle.

* When `+20h != 6`, apply `+20h` directly.
* Otherwise scan slots 5 down to 0 and take the first non-zero request byte.
* Slot 1 degrades to slot 0 unless one of the two lists on `[00f8bbf4]` answers true: the first
  element of the `vector` at +70h/+74h or of the one at +B8h/+BCh, each asked through its
  virtual +2Ch. The empty-vector guard calls `00bf6713`.
* Every path then clears +18h/+1Ch and sets `+20h = 6`.

`00941070(this, slot)` returns immediately for slot 5; otherwise it builds the string `Normal`
through `BSP_NativeString_Assign`, resolves it with `00a7b0a0` (which runs `__stricmp`), and calls
`00a7e490(&result, *(this + 4 + slot*4), *(this+24h), resolvedCategory, 1)`, releasing the
refcounted result afterwards. Both 00a7b0a0 and 00a7e490 sit in segment 74 whose keywords are
`sound, stream, streaming, stereo, fmod`, so this is a sound-cue queue applied once per frame with
slot priority, not the listener binding the packet brief guessed. Slot 5 is the silent slot.

**Uncertain**: what the six slots mean and which callers set which. The twelve callers of 004c1b90
were not analysed.

## 004c6c70, the GUI visibility policy

`void __fastcall(game)`, RET. Folds screen state into two GUI manager calls. Read from the listing
because the decompiler flattens the boolean chain into mixed `|` and `==` expressions.

```
wants  = active(00e198b4) | active(00e198ac) | active(00e198b8) | (game+5D4h == 2)   // +3Ch each
if (hud = 00e198c4) {
    wants |= *(hud+D8h)+5h
    wants |= 0068a140()                                   // base screen active
    wants |= *(hud+5Ch)+5h | *(hud+54h)+5h
    wants |= (00e198b4 && *(00e198b4+54h)+64h)
    wants |= (*(hud+60h)+5h && *(hud+60h)+70h && !*(hud+54h)+5F0h)
    notCine = (game+7184h == 0)
} else notCine = 1
enabled = (wants & notCine & !(00e194b4 && *(00e194b4+5h))) | *(00425d10()+5h)
if (enabled && *(00f8abe8+3E8h)) enabled = 0
00aa0e00(00425d10-independent GUI manager 004c12b0(), enabled)
if (!hud) return
pointer = (*(hud+5Ch)+5h | *(hud+54h)+5h) & (game+7184h == 0) & !(00e194b4 && *(00e194b4+5h))
if (pointer && !*(00f8abe8+3E8h)) { 00aa0e50(gui, 1); return }   // 004c6df8 jumps past the test
if (pointer) pointer = 0
if (enabled) return
00aa0e50(gui, pointer)
```

`00f8abe8+3E8h` is the byte at decimal offset 1000 that `docs/GAME_ON_MOVE_MAP.md` step 9 already
uses as the GUI-disabled switch. `game+7184h` is the cinematic flag from
`docs/GAME_SIMULATION_GATE.md`. `BSP_GuiManager_SetEnabled` runs on every path;
`BSP_GuiManager_SetPointerVisible` does not.

## 004d8620, the front-end screen update

`void __fastcall(game)`, RET, SEH scope with handler 00c664c8.

Fast path, taken when `00e188ae == 0` and `00425d10()+4h == 0` and `[00e19698]+4h == 0`:
`BSP_GuiManager_Update(BSP_GuiManager_GetOrCreate(), *(float*)(game+21ECh), 0)` and return. The
argument pair matches the `00aa4f80(delta, 0)` calls the front-end states make, so `game+21ECh` is
the front-end delta.

Otherwise the routine builds a small list at `[esp+..]` and pushes handles into it with
`004d6790(list, &handle)`:

* `005329c0(ECX = 00425d10(), out)` when the menu command screen's +4h is set;
* `[00e19698]+8h` when that screen's +4h is set;
* `[00e18d48]+1Ch` unconditionally;
* when `00e198c4 == 0 && 00e198ac != 0`, the page `00aa5840(gui, &name, 1, 0)` where `name` is the
  string `FE_main` at 00ce7a70 assigned through 0041e870 and released through the sized pool.

It then calls `004d35d0(temp, &list)` with the delta at `temp+10h` (the callee cleans 18h bytes:
a 14h-byte by-value temporary plus the pointer), `00aa3f70(BSP_GuiManager_GetOrCreate())`, and
frees the list buffer through `_free`. `004d35d0` has no Ghidra prototype beyond
`undefined FUN_004d35d0(void)`.

**Uncertain**: the element type of the list and what 004d35d0 does with it. The name
`BSP_Frontend_UpdateActiveScreens` describes the collection step, which is what the evidence
supports; `docs/GAME_ON_MOVE_MAP.md` suggested `BSP_Frontend_UpdateRoot` before the body was read.

## The render block

`004c1dd0` returns the profiler singleton; `00be3640` begins and `00be3660` ends a counter. The
tail closes the counter identified by `[0109db08]` (the game counter opened by the profiler label
`GGame::OnMove::game` in step 18), opens `[0109db14]` around `BSP_Game_Render` (004ca440,
`__thiscall(game)`, no arguments), closes it, and calls `BSP_Game_FinishRenderFrame` (004ca1f0).
Nothing is passed to `BSP_Game_Render`: the render queue reached it through the globals opened in
steps 9 and 10 (`BSP_RenderCommandQueue_GetSingleton`, `00f8d394`), which this packet leaves
read-only. `docs/RENDER_COMMAND_EXECUTION.md` and `docs/RENDER_BATCH.md` own that side.

## Ghidra functions

All eight leased addresses have Ghidra function bodies; none needed a raw listing to be defined.
The orchestrator can apply names directly. 004e4a40 itself still has an eight-byte stored body and
must be read with `disasm-raw`.

## Reconstruction state

| Address | Name recorded | State |
| --- | --- | --- |
| 004de4b0 | BSP_ParticleClock_GetSingleton | analysed; singleton shape reconstructed as a struct only |
| 00b19a10 | BSP_ParticleClock_SetFrameTime | reconstructed, build-tested |
| 00af0450 | BSP_FoliageGroups_SetShaderTime | reconstructed, build-tested |
| 00af0c50 | BSP_FoliageGroups_BuildVisibleSet | reconstructed, build-tested |
| 004c1b90 | BSP_SoundRequestQueue_GetSingleton | analysed |
| 00941140 | BSP_SoundRequestQueue_ApplyPending | reconstructed, build-tested |
| 004d8620 | BSP_Frontend_UpdateActiveScreens | analysed; only the call order is modelled |
| 004c6c70 | BSP_Game_ApplyGuiVisibility | reconstructed, build-tested |
| 004e538e-004e54f0 | `bsp::run_render_tail` | reconstructed, build-tested |

Nothing here is fixture-tested, ABI-compatible or game-validated. `include/bsp/render_tail.hpp`
and `src/render_tail.cpp` expose new C++ interfaces over an injected host; they are not drop-in
replacements and no global or type was invented to make them link.

## What remains

* The 2Ch-byte particle-clock record and whatever registers records into +8h/+0Ch.
* The foliage group class: +18Ch description, the `vtable[48h]` bounds accessor, 00af6360,
  00af63a0, 00af0990 and the 004c1130 build queue.
* Which camera the second argument of 00af0c50 is.
* The six sound slots and the twelve callers that set them.
* `004d35d0`, `004d6790` and the list they share, and `00aa5840`'s page lookup.
* The external calls in the window: 00685c80, 004d80d0, 006840f0, 00776230, 004c40f0.
