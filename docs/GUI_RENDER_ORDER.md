# GUI draw order (packet `gui_render_order`)

Addresses: 00AA45A0, 00AA3E00, 00AA5070, 00AA3280, 00AA4960, 00AA2020, 00AC4C50,
00AC4450, 00AC59A0, 00AC6040, 00AA5A00, 00AA4E00, 00AA2C80, 00AA7F70.

Supporting addresses read but not owned: 004CA440 (`BSP_Game_Render`), 004C12B0
(`BSP_GuiManager_GetOrCreate`), 00AA52A0 (`BSP_GuiManager_RegisterPageByPriority`),
00AA4F80 (`BSP_GuiManager_Update`), 00AAAED0 (`BSP_GuiWidget_DescribeProperties`),
00AAA480 (the second `std::sort` instantiation), 00B6D890 (node to scene bind),
00B63F10 (look-at basis), 00B71490 (camera view set), 00B62A10 (ortho matrix),
00B6FD60 (`BSP_Camera_SetProjectionMatrix`), 004C11F0
(`BSP_RenderCommandQueue_GetSingleton`), 00B1F4D0 (the queue submit), 004BDA70
(`_Tree_iterator::operator++`), 00B0D0D0, 0041E870.

## The short version, and two corrections to the packet premise

The GUI draw order is **not** built from a sorted child snapshot, and `geOrder`
is not the key.

- **`00AA5A00` is not on the draw path.** It is an alphabetical snapshot of a
  widget's children, built for the reflection walk `00AAAED0` and nothing else.
  The comparator is `_stricmp` on each child's name, not an order number.
- **`geOrder` is never read by the game.** A byte scan of the whole image for
  the literal `geOrder` finds no match, in any casing and with or without a
  terminator. The key appears on almost every widget in the shipped interface
  scripts, so it is authoring metadata the runtime discards.

What actually orders GUI drawing is the **`RenderOrder` float on a `cGuiLayer`**.
The GUI manager keeps an MSVC `std::multimap<float, cGuiCameraStore*>` at
`manager+8h`, keyed by that float, and the per-frame draw walks it in ascending
key order. Each entry is one orthographic pass submitted to the render command
queue.

## Calling conventions and RET sizes

| Address | Convention | RET | Role |
| --- | --- | --- | --- |
| 00AA45A0 | `__thiscall(ECX = manager)` | `RET` | The GUI draw: walk the camera-store map, submit one pass per store |
| 00AA3E00 | `__thiscall(ECX = manager, scene, camera)` | `RET 8` | Camera setup and one queue submit. ECX is overwritten at 00AA3E18 and never read |
| 00AA5070 | `__thiscall(ECX = manager, camera, scene, const Key*)` | `RET 0Ch` | Allocate a 24h-byte camera store, copy the descriptor, insert into the map |
| 00AA3280 | `__thiscall(ECX = manager, const Key*)` | `RET 4` | Find an existing store with a matching descriptor, or 0 |
| 00AA4960 | `__thiscall(ECX = &map, pair<iterator,bool>* ret, const pair<float,Store*>*)` | `RET 8` | `std::_Tree::insert`, multi flavour |
| 00AA2020 | `__thiscall(ECX = camera, 6 floats)` | `RET 18h` | Widescreen-scale the ortho volume, then `BSP_Camera_SetProjectionMatrix` |
| 00AC4C50 | `__thiscall(ECX = layer, visitor)` | `RET 4` | `cGuiLayer` property read: `Priority` and `RenderOrder` |
| 00AC4450 | `__thiscall(ECX = layer, bool visible)` | `RET 4` | `cGuiLayer::SetVisible`; maintains the store's visible-layer count |
| 00AC59A0 | `__fastcall(ECX = layer)` | `RET` | Acquire or create the layer's camera store, camera, scene and lights |
| 00AC6040 | `__thiscall(ECX = new layer, const layer* src)` | `RET 4` | `cGuiLayer` copy construct; calls 00AC59A0 then bumps the visible count |
| 00AA5A00 | `__fastcall(ECX = out vector, EDX = widget)` | `RET` | Alphabetical child snapshot; returns the vector |
| 00AA4E00 | `__fastcall(ECX = first, EDX = last, int ideal, Pred)` | `RET 8` | `std::_Sort` introsort loop for the name comparator |
| 00AA2C80 | `__fastcall(ECX = first, EDX = last, Pred)` | `RET 4` | The insertion-sort half; holds the comparator body |
| 00AA7F70 | `__fastcall(ECX = first, EDX = last, Pred)` | `RET 4` | Byte-identical duplicate of 00AA2C80 for 00AAA480 |

None of these has a fall-through gap: `bsp.py ghidra flow` reports 0 gaps for
00AA45A0, 00AA3E00, 00AC59A0 and 00AAAED0. Every routine named here has a real
Ghidra function; there are no listing-only routines in this packet.

## `cGuiLayer` and its two ordering properties

`00AC4C50` is the layer's property reader, reached through the class vtable at
00D5BE38 slot +18h. It makes two calls to the visitor's vtable `+0Ch`, each with
three 8-byte `{tag, value}` blocks: the name block (tag 0 and a `.rdata` string),
the target block (the tag and the field address) and the default block.

| Property | Literal | Tag | Field | Default |
| --- | --- | --- | --- | --- |
| `Priority` | 00D5CB00 | 1 (int) | written to **both** layer+0FCh and layer+118h at 00AC4C94/00AC4C9A | 0 |
| `RenderOrder` | 00D5CAF4 | 2 (float) | layer+11Ch (00AC4CBB) | +0.0f |

Tag 1 is the signed int `gui_text.hpp` already recovered from `DefaultShadow`.

`Priority` at +0FCh is the key `BSP_GuiManager_RegisterPageByPriority` (00AA52A0)
sorts the page vector at manager+14h with, per docs/GUI_LAYOUT_LOADER.md. That
vector is the update and lookup order, **not** the draw order. `00AC59A0`'s tail
re-registers the page whenever the applied copy at +0FCh differs from the
authored copy at +118h (00AC5EFE..00AC5F14).

After the two property reads, 00AC4C50 calls the widget hook at vtable +78h,
binds the layer's own scene node at layer+4Ch into the layer's scene at
layer+0ECh through 00B6D890, resets the local transform through 00AA7DC0, and
descends into `BSP_GuiWidget_ReadPropertiesAndChildren` (00AAA710).

## The camera-store descriptor

`00AC6040` writes the descriptor defaults at layer+108h before 00AC59A0 runs
(00AC60F0..00AC6112), and 00AA5070 stages the same values in a fresh store
before overwriting them from the descriptor.

| Offset | Layer field | Type | Default | Matched by 00AA3280 |
| --- | --- | --- | --- | --- |
| +108h | flags | int | 0 | yes, as an integer |
| +10Ch | near plane | float | 0.1f (00D7A2F0) | yes |
| +110h | far plane | float | 1000.0f (00CE3804) | yes |
| +114h | scale | float | 1.0f (00D7A24C) | yes |
| +118h | `Priority` copy | int | 0 | **no** |
| +11Ch | `RenderOrder` | float | +0.0f | yes |

`00AA3280` walks the map and returns the first store whose descriptor matches on
those five fields, logging `cGuiManager:Re-use camera store 0x%x/%s`. Skipping
`Priority` is deliberate: two layers that differ only in page priority share one
camera, one scene and one draw pass, and are separated only inside the scene.

The layer flags reach the scene as `flags | 6` (00AC5EF6). The layer's camera is
named `GuiCam_` + the layer name at layer+100h, and its scene also gets a
`GuiLights` set and a `GuiDirectionalLight`.

## The store, and the map that orders it

The store is a 24h-byte heap block (00AA5075 `PUSH 24h`):

| Offset | Contents |
| --- | --- |
| +00h..+14h | the descriptor above, copied verbatim (00AA50C7..00AA50F6) |
| +18h | the camera (argument 1 of 00AA5070) |
| +1Ch | the scene (argument 2) |
| +20h | signed count of **currently visible** layers sharing this store, initialised to 0 |

`00AA5070` then builds the pair `{descriptor RenderOrder, store}` in its frame
(00AA5106..00AA5118) and calls `00AA4960`, which is the MSVC red-black tree
insert. Node layout is the standard `_Left@0, _Parent@4, _Right@8, _Myval@0Ch,
_Color@14h, _Isnil@15h`, so the key sits at node+0Ch and the store pointer at
node+10h.

The descent is two instructions of comparison:

```
00aa498c: FLD float ptr [EDX + 0xc]      ; the node's key
00aa4991: FCOMIP ST0,ST1                 ; node key against the new key
00aa4993: JBE 0x00aa499d                 ; node key <= new key -> go right
00aa4995: MOV EDX,dword ptr [EDX]        ; else go left (_Left)
```

So the tree descends left only when the new key is **strictly less** than the
node key. Equal keys always end up to the right of the ones already present,
which is `multimap` insertion, and the always-`true` bool the wrapper stores at
00AA49CB with no duplicate check is MSVC's `_Tree::insert` compiled with
`_Multi = true`. **Ties keep creation order.** Ordering is `std::less<float>`, so
a negative `RenderOrder` draws first and NaN would be unordered; no shipped
script sets one.

## The visible-layer gate

`00AC4450` is `cGuiLayer::SetVisible`, the class vtable slot +34h. When the
requested flag differs from the layer's current flag at layer+0F5h it adjusts
the store's count at +20h by one in the matching direction (00AC4465..00AC4472),
then propagates visibility down the widget tree and binds or unbinds the layer's
node from the scene with `00B6D890(scene)` or `00B6D890(0)`. `00AC6040` does the
same `+1` when a copied layer starts visible.

That count is the only enable check the draw applies. There is no scissor or
clip step anywhere in 00AA45A0 or 00AA3E00. Per-widget visibility is handled
earlier, by the node visibility factor of docs/GUI_WIDGET_SCENE_VISIBILITY.md,
and by whether a widget's node is attached to the layer's scene at all.

## The per-frame draw

`BSP_Game_Render` reaches the GUI at 004CA658: `004C12B0` then `00AA45A0`
(docs/GAME_RENDER_FRAME.md). `00AA45A0` is 62 instructions:

1. If `manager+40h` is non-null, call its vtable `+40h` with no arguments
   (00AA45A7..00AA45BB). The object at +40h was not identified.
2. Take `begin()` as `_Myhead->_Left` (`*(manager+0Ch)`), with `end()` the head
   itself, and iterate with the checked `_Tree_iterator::operator++` at 004BDA70.
3. For each entry, load the store from node+10h and skip it when
   `store+20h <= 0` (00AA45FA `CMP` / `JLE`).
4. Otherwise call `00AA3E00(ECX = manager, store->scene, store->camera)`.

The walk is flat. There is no second level: layers, screens and pages are not
re-walked here, because every widget of every visible layer is already a node in
the store's scene, and the queue draws the scene.

## The orthographic camera, and what reaches the queue

`00AA3E00` builds the camera from the back-buffer size, then submits:

1. The device singleton at 00F8D394, vtable `+80h`, fills two ints in the frame:
   width and height.
2. The eye is placed half a pixel past the centre of the unit volume:
   `eye = (0.5 + 0.5/width, 0.375 + 0.5/height, 0)`, computed in x87 extended
   precision from the doubles 0.5 (00D7A280) and 0.375 (00CF0A90) and rounded to
   float (00AA3E30..00AA3E93). This is the Direct3D 9 half-texel correction.
3. The target is the eye moved to `z = -9992.0f` (00D19620) and the up vector is
   `(0, 1, 0)`. `00B63F10` builds the basis, `00B71490` sets it on the camera.
4. `00AA2020` receives six floats in stack order:

   | Slot | Value | Constant |
   | --- | --- | --- |
   | +00h | -0.5f | 00CE69D0 |
   | +04h | +0.5f | 00CE3800 |
   | +08h | +0.375f | 00D5BF4C |
   | +0Ch | -0.375f | 00D5BF50 |
   | +10h | 1.0f | `FLD1` |
   | +14h | 19984.0f | 00CE3CC0 |

   Bottom is positive and top negative, so **+Y runs down the screen**, which is
   why authored `Pos` Y grows towards the bottom. The far plane is exactly twice
   the target distance. `00AA2020` scales the first two by
   `base / 00CF5750` and the second two by `base / platform+10h`, where `base` is
   00D5BD98 or 00D5BD9C depending on platform byte +0Dh (the widescreen toggle),
   then calls 00B62A10 and `BSP_Camera_SetProjectionMatrix`.
   `BSP_InterfaceView_DrawTargetOrtho` and `BSP_InterfaceView_DrawGroundOrtho`
   use the same helper.
5. A `BSP_NativeString` temporary is assigned the literal at 00CE9A38, which is
   the single character `"X"`, and 00B0D0D0 is called on the object at 00F8D39C.
6. `004C11F0` gets the render command queue singleton, and `00B1F4D0(ECX = queue,
   scene, camera, context)` submits the pass (00AA3F42..00AA3F52). The queue
   contract is Codex's docs/RENDER_COMMAND_EXECUTION.md and docs/RENDER_BATCH.md;
   nothing here inspects it.

Depth inside a pass therefore comes from each widget's own Z, the third component
of the authored `Pos` (for example -60 in `fe_loading.lua`), inside the near/far
range 1.0 to 19984.0. `RenderOrder` chooses the pass; Z chooses the depth within
it.

## What `00AA5A00` really sorts

`00AA5A00` clears the caller's `std::vector`, copies the payload dword of every
node of the widget's child list at +68h into it, and calls `00AA4E00` with
`(first, last, last - first, Pred)` where `Pred` is an empty functor written as
one byte over a stale stack slot at 00AA5AD3. That is MSVC's
`std::_Sort(_First, _Last, _Ideal, _Pred)`, with the 32-element insertion-sort
threshold at 00AA4E00, the median-of-three partition 00AA34A0, the heapsort
fallback 00AA4550 and the `_Ideal/2 + (_Ideal/2)/2` depth counter.

The predicate is in the insertion-sort half 00AA2C80 (and its byte-identical twin
00AA7F70, which serves 00AAA480):

```
piVar2 = a->vtable[0x28]()      ; a {length, char*} pair
piVar3 = b->vtable[0x28]()
if (piVar3[0] == 0)  result = (piVar2[0] != 0) ? -1 : 0
else if (piVar2[0] == 0) result = not-less
else result = _stricmp(piVar3[1], piVar2[1])
pred = result < 0
```

So the order is the widget name, case-insensitively, with an empty name first.
The empty case exists to avoid dereferencing a null character pointer, and gives
the same order `strcmp` would.

The sole caller `00AAAED0` builds the snapshot, sorts it a **second** time with
00AAA480 (a duplicate instantiation the linker did not fold), and then for each
child calls the child's vtable `+28h`, the visitor's `+4h`, the child's `+1Ch`
and the visitor's `+8h`. That is a reflection or serialisation walk. The snapshot
is a local temporary rebuilt from scratch on every call: there is no dirty flag,
no cached count and no field anywhere that holds it.

## Callers and callees

- 00AA45A0: called only by `BSP_Game_Render` (004CA440). Calls 004BDA70 and 00AA3E00.
- 00AA3E00: called by 00AA45A0 and by `BSP_RenderMode_PrepareAndStartWorker`
  (00AA4040). Calls 0041E870, 004C11F0, 00AA2020, 00B0D0D0, 00B1F4D0, 00B63F10,
  00B71490.
- 00AA5070 and 00AA3280: called only by 00AC59A0.
- 00AC59A0: called only by 00AC6040.
- 00AA5A00: called only by 00AAAED0, which has 13 callers, all class-level
  property describers.

## Uncertainties

- The object at `manager+40h` and its vtable `+40h` are unidentified. It is
  called once per frame before the walk and takes no arguments.
- The descriptor fields at +108h..+114h are named near/far/scale from the
  defaults 0.1f, 1000.0f and 1.0f and from the flags OR at 00AC5EF6. No layer
  property writes them, so they were never observed with another value; a
  perspective GUI layer would be the case that proves the naming.
- The pass label `"X"` at 00CE9A38 is a one-character literal. It is passed to a
  `BSP_NativeString` temporary whose consumer inside 00B1F4D0 was not followed.
- `00AA2020`'s aspect divisors 00CF5750 and platform+10h were read as float
  globals but their producers were not traced.
- `std::sort` is not stable, so two children with names that compare equal have
  an unspecified relative order above the 32-element threshold. Below it the
  insertion sort preserves list order.
- The eye placement is reproduced in `double`; the native uses x87 extended
  precision. The two agree after rounding to float for every plausible
  back-buffer size, but they are not bit-identical by construction.

## What remains

- Whether any layer other than the defaults ever produces a distinct camera-store
  descriptor, which would show up as more than one entry in the map.
- Where widget Z inside a pass is finally written into the node transform, and
  whether the queue sorts within the scene as well
  (docs/RENDER_BATCH.md is the contract, not read here).
- The `cGuiLayer` vtable at 00D5BE38 beyond slots +18h and +34h.
- `00AA4040`, the other caller of 00AA3E00, which submits a GUI-style pass
  outside the manager walk.

## Follow-up packets proposed

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `gui_layer_class` | 00AC4450, 00AC59A0, 00AC6040, 00AC5F60, vtable 00D5BE38 | docs/GUI_LAYER_CLASS.md, include/bsp/gui_layer.hpp | The whole `cGuiLayer`: its 124h-byte layout, its lights and scene, the vtable slots this packet did not read, and its destruction path including the store's release. |
| `gui_manager_prehook` | 00AA45A7 call site, manager+40h, 00AA5D70 | docs/GUI_MANAGER_PREHOOK.md | What object sits at `manager+40h`, who installs it, and what its vtable +40h does each frame before the GUI draw. |
| `gui_render_mode_pass` | 00AA4040, 00AA3E00 | docs/GUI_RENDER_MODE_PASS.md | The second caller of the GUI pass submit: which scene and camera it uses and how it relates to the manager walk. |
| `gui_reflection_walk` | 00AAAED0, 00AA5A00, 00AAA480, visitor vtable +4h/+8h | docs/GUI_REFLECTION_WALK.md | The describer walk the alphabetical snapshot exists for: who the visitor is and what consumes the emitted tree. |

## State reached

| Address | State |
| --- | --- |
| 00AA45A0 | reconstructed, build-tested (`draw_gui_layers_00aa45a0`) |
| 00AA3E00 | reconstructed, build-tested (`submit_gui_pass_00aa3e00`) |
| 00AA5070 | reconstructed, build-tested (insert half only) |
| 00AA3280 | reconstructed, build-tested |
| 00AA4960 | reconstructed, build-tested (the ordering rule) |
| 00AA5A00, 00AA2C80, 00AA7F70 | reconstructed, build-tested |
| 00AA2020 | analyzed (host call; the aspect scaling is not reproduced) |
| 00AC4C50, 00AC4450, 00AC59A0, 00AC6040 | analyzed |
| 00AA4E00 | analyzed (library template) |
