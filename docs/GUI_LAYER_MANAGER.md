# The GUI layer and the manager's layer list (packet `gui_layer_manager`)

Addresses: 00AA3840, 00AA38B0, 00AA38E0, 00AA38F0, 00AA3BD0, 00AC5480, 00AC6600,
00AC4C50, 00AC4D30, 00AC4450, 00AC3F80, 00AC3F90, 00AC3FA0, 00AC3FB0, 00AC57E0,
00AC51A0, 00AC4C40, 00AC4E50, 00AA4B30, 00AA3910, 00AA2F10, 00AA8BD0, 00AA87B0.

Supporting addresses read but not owned: 00AC59A0 and 00AC6040
(docs/GUI_RENDER_ORDER.md), 00AA5840, 00AA52A0, 00AA3140, 00AA6560, 00AA2490,
00AAA710, 00AAAED0 (docs/GUI_LAYOUT_LOADER.md), 00AA4F80, 00AA0F70, 00AA0E00,
00AA0E50 (docs/GAME_BLOCKING_SCREEN.md), 00AA8450, 00AA7EF0, 00AA9390, 00AA9730,
00AA7DC0, 00AA7D00, 00B6D890, 004155B0, 00419210, 004BDA70, 00AA46B0, 00AC4AD0.

Ghidra was read-only for this packet. The names below are recorded in the
ledger, not applied to the program.

## The headline: there is no separate layer container

`cGuiLayer` is not a second object hanging off a page. It **is** the page: the
type-1 case of `BSP_GuiLayout_CreateWidgetOfType` (`00AA6560`), the class
`BSP_GuiManager_LoadPage` allocates and constructs, and the element type of the
`std::vector<GuiScreen*>` at manager `+14h`. The class the layout-loader doc
calls `GuiScreen` and the class the render-order doc calls `cGuiLayer` are one
class: both allocate `124h` bytes and both stamp vtable `00D5BE38`.

So the two orderings the render-order doc separates are two views of the same
object list:

| Order | Container | Key | Field | Established by |
| --- | --- | --- | --- | --- |
| Update and hit test | `std::vector<cGuiLayer*>` at manager `+14h` | `Priority` | `+FCh` | `00AA52A0` insertion |
| Draw | `std::multimap<float, cGuiCameraStore*>` at manager `+8h` | `RenderOrder` | `+11Ch` | `00AA5070` insertion |

A layer joins the first list once, at registration, and reaches the second only
indirectly, through the camera store it acquires or shares.

## Calling conventions and RET sizes

| Address | Signature | RET | Role |
| --- | --- | --- | --- |
| 00AA3840 | `__fastcall(ECX = block)` | `RET 0` | Default constructor; returns the block in EAX |
| 00AC6600 | `__thiscall(ECX = block, const NativeString *name, int share_scene, char unused)` | `RET 0Ch` | Script-backed constructor; runs the page's Lua and calls vtable `+18h` |
| 00AA3BD0 | `__fastcall(ECX = source layer or 0)` | `RET 0` | Allocate then default- or copy-construct |
| 00AC51A0 | `__fastcall()` | tail `JMP` | `MOV ECX,0F8BF50h; JMP 00AC4E50` -- the class pool |
| 00AC4E50 | `__fastcall(ECX = pool)` | `RET 0` | Fixed-block pool acquire, guarded by a critical section at pool `+0Ch` |
| 00AC4C40 | `__fastcall(ECX = layer)` | `RET 0` | `PUSH ECX; MOV ECX,0F8BF50h; CALL 00AC47F0` -- pool release |
| 00AA38F0 | `__thiscall(ECX = layer, byte flags)` | `RET 4` | Scalar deleting destructor; frees the block when bit 0 is set |
| 00AC5480 | `__fastcall(ECX = layer)` | `RET 0` | Destructor |
| 00AC4C50 | `__thiscall(ECX = layer, visitor)` | `RET 4` | Property reader, vtable `+18h` |
| 00AC4D30 | `__thiscall(ECX = layer, visitor)` | `RET 4` | Property describer, vtable `+1Ch`; **no Ghidra function** |
| 00AC4450 | `__thiscall(ECX = layer, bool visible)` | `RET 4` | `SetVisible`, vtable `+34h` |
| 00AA38E0 | `__fastcall(ECX = layer)` | `RET 0` | `IsVisible`, vtable `+38h`; **no Ghidra function** |
| 00AA38B0 | `__thiscall(ECX = layer, type id)` | `RET 4` | `IsKindOf`, vtable `+0Ch`; **no Ghidra function** |
| 00AC3F80 | `__fastcall(ECX = layer)` | `RET 0` | Type-chain begin, vtable `+08h`; **no Ghidra function** |
| 00AC3F90 | `__fastcall(ECX = layer)` | `RET 0` | Own type id, vtable `+14h`; **no Ghidra function** |
| 00AC3FA0 | `__fastcall(ECX = layer)` | `RET 0` | `GetName`, vtable `+7Ch`; **no Ghidra function** |
| 00AC3FB0 | `__fastcall(ECX = layer)` | `RET 0` | Empty virtual, vtable `+80h`; **no Ghidra function** |
| 00AC57E0 | `__thiscall(ECX = layer, const NativeString *name)` | `RET 4` | `SetName`, vtable `+2Ch` |
| 00AA87B0 | `__thiscall(ECX = widget, float seconds)` | `RET 4` | The update, vtable `+40h`; shared with every widget |
| 00AA4B30 | `__thiscall(ECX = manager, cGuiCameraStore *store)` | `RET 4` | Remove and free a camera store |
| 00AA3910 | `__fastcall(ECX = manager)` | `RET 0` | Pointer motion, clamp, cursor state |
| 00AA2F10 | `__fastcall(ECX = manager)` | `RET 0` | Hit test over the page vector |
| 00AA8BD0 | `__thiscall(ECX = widget, const float origin[3])` | `RET 4` | Recursive widget hit test |

`bsp.py ghidra flow` reports 0 gaps for 00AC5480, 00AC6600, 00AA3910, 00AA2F10,
00AA3840, 00AA3BD0, 00AC4450 and 00AC4C50. **00AA4B30 has one gap**: see the
teardown section.

### Listing-only routines

Seven vtable targets have no Ghidra function; each is a small leaf that the
analyser never reached because its only reference is a vtable slot. They are
reported here with their inclusive last-instruction address so the integrator can
define them before applying a name.

| Address | End (last instruction) | Body |
| --- | --- | --- |
| 00AA38B0 | 00AA38D5 | `IsKindOf`: linear scan of `[00F8BF88, 00F8BF94)` for the argument |
| 00AA38E0 | 00AA38E6 | `MOV AL,[ECX+0F5h]; RET` |
| 00AC3F80 | 00AC3F85 | `MOV EAX,[00F8BF88]; RET` |
| 00AC3F90 | 00AC3F95 | `MOV EAX,[00F8BF94]; RET` |
| 00AC3FA0 | 00AC3FA6 | `LEA EAX,[ECX+100h]; RET` |
| 00AC3FB0 | 00AC3FB0 | `RET` |
| 00AC4D30 | 00AC4D64 | The describer; see below |

## The object

`124h` bytes. Every allocation site loads the size into ECX before calling
`00AC51A0` (`00AA3BE9` in the factory, `00AA5924` in `BSP_GuiManager_LoadPage`),
and `00AC51A0` **overwrites ECX with its pool pointer on its first instruction**,
so the immediate is a dead `operator new(sizeof(cGuiLayer))` leftover. That is
what makes it good evidence: the compiler emitted the class size, and the
allocator ignores it because the pool's block size is fixed at its own setup.

The base half belongs to the widget class the constructor `00AA9390` fills. The
layer's own fields start at `+0ECh`:

| Offset | Type | Meaning | Constructor value |
| --- | --- | --- | --- |
| +0ECh | ptr | the scene the layer's widgets hang under | 0 (`00AC662D`) |
| +0F0h | ptr | the shared `cGuiCameraStore` | 0 (`00AC6634`) |
| +0F4h | byte | **1 when this layer created the store and scene** | set by `00AC59A0` |
| +0F5h | byte | visible; the flag `00AC4450` maintains | 0 (`00AC663B`) |
| +100h | size_t | name length (a `NativeString` starts here) | 0 |
| +104h | char* | name data | 0, then the constructor's `memcpy` |
| +108h | int | scene flags, the descriptor's first field | 0 |
| +10Ch | float | near plane | 0.1f (`00D7A2F0`) |
| +110h | float | far plane | 1000.0f (`00CE3804`) |
| +114h | float | scale | 1.0f (`00D7A24C`) |
| +118h | int | the **authored** `Priority` | 0 |
| +11Ch | float | `RenderOrder`, the draw-map key | +0.0f |
| +120h | byte | the load flag; see "what +120h selects" | the argument |
| +121h | byte | zeroed by `00AC6600`; no reader found | 0 |

`+108h`..`+11Ch` is exactly the `18h`-byte camera-store descriptor
docs/GUI_RENDER_ORDER.md describes, so the descriptor is not a separate object:
it is a slice of the layer.

Two base fields matter to the layer's own code and are named here for that
reason only: `+4Ch` is the layer's scene node, and `+FCh` is the **applied**
`Priority`, the key `00AA52A0` sorts the page vector on.

`00AC3FA0` returning `ECX+100h` is the direct proof that the name lives at
`+100h`: it is vtable `+7Ch`, which is what `BSP_GuiManager_FindPageByName`
(`00AA3140`) calls on every page and what `00AC4450` calls before the visibility
hook.

## The vtable at 00D5BE38

33 slots, `+00h`..`+80h`; the floats at `00D5BEBC` and `00D5BEC0` follow
immediately, and a sibling widget vtable begins at `00D5BECC`. Comparing the two
gives the override set.

| Slot | Target | Overridden | Role |
| --- | --- | --- | --- |
| +00h | 00BD30E0 | no | ref-counted deleting-destructor thunk |
| +04h | 00AA38F0 | yes | scalar deleting destructor |
| +08h | 00AC3F80 | yes | type-chain begin |
| +0Ch | 00AA38B0 | yes | `IsKindOf` |
| +14h | 00AC3F90 | yes | own type id |
| +18h | 00AC4C50 | yes | `ReadProperties` |
| +1Ch | 00AC4D30 | yes | `DescribeProperties` |
| +2Ch | 00AC57E0 | yes | `SetName` |
| +34h | 00AC4450 | yes | `SetVisible` |
| +38h | 00AA38E0 | yes | `IsVisible` |
| +40h | 00AA87B0 | **no** | the update |
| +78h | 00AC59A0 | yes | acquire the camera store |
| +7Ch | 00AC3FA0 | yes | `GetName` |
| +80h | 00AC3FB0 | yes | empty |

The layer does **not** override the update. Slot `+40h` is the same `00AA87B0`
the sibling vtable carries, which is why the manager's per-page update and a
widget's per-child update are literally the same code.

## How a layer is created and attached

`BSP_GuiManager_LoadPage` (`00AA5840`) drives it. The steps this packet adds to
docs/GUI_LAYOUT_LOADER.md are the ones inside the constructor.

1. `00AA5862`: `00AA3140` scans the page vector by name. A hit returns the
   existing layer, so **pages are singletons by name** and a second load never
   builds a second layer.
2. `00AA5924`: `00AC51A0` takes a `124h` block from the pool at `00F8BF50`.
3. `00AC6600` writes the field block above, then builds a Lua state
   (`00B66BD0`), runs `interface/_Common.lua` and then `interface/<name>.lua`
   through `00AC5600` and `00B69D40`, takes the global table `GuiScreen`
   (`00AC6806`) and builds a property visitor over it (`004425C0`).
4. `00AC6814`: `this->vtable[+18h](visitor)`, which is `00AC4C50`.
   - `Priority` (literal `00D5CB00`, tag 1, default 0) is written to **both**
     `+FCh` and `+118h`.
   - `RenderOrder` (literal `00D5CAF4`, tag 2, default +0.0f) is written to
     `+11Ch`.
   - Then `this->vtable[+78h]()`, which is `00AC59A0`, acquires the camera store.
   - Then the node bind, the local-transform reset and
     `BSP_GuiWidget_ReadPropertiesAndChildren`, which builds the widget tree.
5. `00AC59A0` looks the descriptor up with `00AA3280` and either shares an
   existing store or creates a scene, a `GuiCam_<name>` camera, a `GuiLights` set
   and a `GuiDirectionalLight`, registers the store through `00AA5070` and sets
   `+0F4h` to 1. It applies the scene flags as `+108h | 6`.
6. `00AC59A0` tail: when `+FCh` differs from `+118h`, `+FCh` takes the authored
   value and `00AA52A0` re-registers the page. This is the only path that moves a
   page inside the vector after it is registered, and it is why the authored copy
   at `+118h` exists at all.
7. `00AA5957`: `00AA52A0` inserts the page before the first entry whose `+FCh` is
   strictly greater, so the vector stays sorted by `Priority` ascending and equal
   priorities keep insertion order.

The layer starts **invisible** (`+0F5h` = 0) and the store's visible count starts
at 0, so a freshly loaded page draws nothing until a screen calls `SetVisible`.

### What +120h selects

docs/GUI_LAYOUT_LOADER.md carries the second `LoadPage` argument through to
screen `+120h` without interpreting it. Its reader is `00AC59CB`: the reuse
branch of `00AC59A0` requires **both** a descriptor match and this byte set.

```
store = 00AA3280(&this->descriptor);        // 00AC59B4
this->store_0F0 = store;                    // 00AC59BC
if (this->byte_120 != 0 && store != 0) {    // 00AC59CB, 00AC59D2
    this->owns_store_0F4 = 0;
    this->scene_0EC = store->scene_1C;      // borrow, InterlockedIncrement
    return;
}
this->owns_store_0F4 = 1;                   // creates everything below
```

Screens pass 1 and the loading screen passes 0, so the loading screen always gets
its own scene even when its descriptor matches an existing one. Calling it a
"share the scene" flag is a hypothesis from this branch, not a recovered name.

### The layer factory

`00AA3BD0` is the type-1 case of `BSP_GuiLayout_CreateWidgetOfType`. It allocates
and then dispatches on its ECX argument: null takes the default constructor
`00AA3840`, non-null takes the copy constructor `00AC6040`. A pool that hands
back nothing makes it return 0, matching every other type case.

Because the loader's type table maps id 1 to "no suffix", a nested Lua table
whose key has no `_Suffix` becomes another layer, not a group. No shipped script
does that: `_common.lua` is the only one of the 97 files without a top-level
`GuiScreen`, and every nested key in the other 96 carries a suffix.

## SetVisible, 00AC4450

The full slot, from the listing:

```
__thiscall SetVisible(cGuiLayer *this /*ECX*/, bool visible /*[ESP+4], BL*/)  RET 4
  if (this->visible_0F5 != visible)                       // 00AC4458
      this->store_0F0->visible_layers_20 += visible ? +1 : -1;   // 00AC446A / 00AC4470
  BSP_GuiWidget_PropagateVisibility(ECX = this, 1, 1, visible, this->byte_75, 1);  // 00AC4480
  if (this->node_4C != 0)                                 // 00AC4488
      00B6D890(ECX = node, visible ? this->scene_0EC : 0);        // 00AC449B
  if (g_hook_00F8BF4C != 0 && this->visible_0F5 != visible)      // 00AC44A0, 00AC44A9
      g_hook_00F8BF4C(ECX = this->vtable[+7Ch](), DL = visible);  // 00AC44BE
  this->visible_0F5 = visible;                            // 00AC44C4
```

Three things worth stating plainly.

- The store count moves **only on an edge**, and the store pointer is
  dereferenced without a null check inside that branch. A layer whose
  `00AC59A0` never ran would fault here; in practice the constructor always runs
  it before any screen can call this.
- `PropagateVisibility` and the node bind run **unconditionally**, edge or not,
  so a redundant `SetVisible(true)` still re-walks the whole widget tree.
- The flag is written **last**, so the hook at `00F8BF4C` sees the old flag. That
  hook is installed by `009870A0` and cleared by `00989A80`; those are the only
  writers, and `00AC4450` is the only reader. It is not part of the shipped
  frame path.

The store's `+20h` count is the sole gate `BSP_GuiManager_DrawLayers` applies
(`00AA45FA`), so hiding the last visible layer of a store removes that whole
pass, camera and all.

## The property describer, 00AC4D30

Listing-only, `00AC4D30`..`00AC4D64`, `RET 4`.

```
push 0                       ; default
push [ESI+0FCh]              ; the APPLIED Priority
sub esp,8 ; [esp]=0 ; [esp+4]=00D5CB00   ; {tag 0, "Priority"}
mov ecx, edi ; call 00AC4AD0             ; the int describe helper
push edi ; mov ecx,esi ; call 00AAAED0   ; chain to the widget describer
```

**`RenderOrder` is read but never described.** The reader takes two properties
and the describer emits one, so a describe/read round trip through the editor
path silently drops `RenderOrder` and resets the pass to +0.0f. The describer
also emits the applied copy at `+FCh`, not the authored copy at `+118h`, so a
page whose priority was changed at runtime describes the new value.

## The per-frame update

`BSP_GuiManager_Update` (`00AA4F80`, docs/GAME_BLOCKING_SCREEN.md) snapshots the
page vector and, for each page, calls vtable `+40h` when

```
page->vtable[+38h]()  ||  BSP_GuiScreen_HasLiveEntries(page)
```

Vtable `+38h` on a layer is `00AA38E0`, two instructions returning byte `+0F5h`,
so the first half of the gate is exactly the `SetVisible` flag. Vtable `+40h` is
`00AA87B0`, which does the same thing one level down:

1. `00AA87BE`: `widget->elapsed_80 += seconds`, before anything else and
   regardless of the widget's own visibility.
2. Walk the child list at `+68h` (count at `+6Ch`) in list order. For each child
   apply the identical gate -- child `vtable[+38h]()` or a non-null slot in the
   child's `+88h`/`+8Ch` entry array -- and call the child's `vtable[+40h]`
   with the same delta.
3. Then step the widget's own `+88h`/`+8Ch` entries through `00AD39A0`, growing
   the array through `00AA6F30` when needed.

So an invisible layer whose entry array still holds something keeps ticking, and
its whole subtree ticks with it. That array is what docs/GAME_BLOCKING_SCREEN.md
calls the live entries, and it is the reason a hidden page's animations finish.

The layer contributes nothing of its own to the update. Everything specific to a
layer happens in `SetVisible`, in the property read and in the draw walk.

## How the pointer reaches the widgets

`BSP_GuiManager_Update` calls `00AA3910` first, and only when its own flag
argument is 0 (`00AA4FA8`). Both shipped call sites pass 0.

### 00AA3910, the motion pass

1. Element 0 of the input device vector at `DAT_00F8BBF4+94h` is the cursor
   device. An empty vector skips the whole pass.
2. Device X comes from its vtable `+3Ch` and Y from `+40h`, both as **signed
   ints** converted to float.
3. First call only: bit 0 of `DAT_00F8BC6C` primes the latch at `00F8BC64` /
   `00F8BC68`, so the first frame contributes no motion.
4. `00AA3A36`: **everything below is skipped when the manager's byte at `+48h`
   is zero.** That byte is what `BSP_GuiManager_SetEnabled` writes;
   docs/GAME_BLOCKING_SCREEN.md records its readers as not found, and this is the
   reader. `SetEnabled(0)` freezes the pointer, the hit test and the cursor
   state, and leaves the last hit widget latched in `00F8BC70`.
5. The raw delta is scaled in x87 by two doubles: `00D5BEC8`
   (0.0010416667209938169, the float 1/960 promoted) then `00D7A308` (2.0), each
   step rounded back to float. The net factor is 1/480 per device unit.
6. The scaled delta is stored at manager `+64h` / `+68h` and accumulated into
   `+5Ch` / `+60h`, which the constructor seeds with 0.5f (`00CE3800`).
7. If the FE cursor icon reports hidden (its vtable `+38h`) and the delta length
   from `00419210` exceeds 0.01f, the icon is shown through its vtable `+34h`.
8. X is clamped by `004155B0`, `__fastcall(ECX = &value, EDX = &lo, [ESP] = &hi)`
   with the result in ST0. Y is clamped inline with the same narrow pair.

| Axis | Low | High | Source |
| --- | --- | --- | --- |
| X, normal | 0.009999999776482582 | 0.9900000095367432 | 00D7A238, 00CE4E0C |
| X, widescreen | -0.15666666626930237 | 1.15666663646698 | 00D5BEBC, 00D5BEC0 |
| Y, always | 0.009999999776482582 | 0.9900000095367432 | 00D7A238, 00CED5D0 as a double |

Widescreen is the byte at `[0109CF04+0Dh]`, the same flag the widget transform
uses. The widescreen X range runs outside the unit box on both sides, which is
how the cursor reaches content the wide-screen shift pushes past the 4:3 edges.

9. The clamped position is pushed into the FE cursor icon through
   `BSP_GuiWidget_SetLocalXYAndBounds`, then `00AA2F10` runs.

### 00AA2F10, the page pass

```
g_pointer_x_00F8BC74 = manager->x_5C;
g_pointer_y_00F8BC78 = manager->y_60;
g_hit_widget_00F8BC70 = 0;
g_hit_score_00F8BC7C = 1e10f;               // 00CE4970
for (page in the vector at manager+18h..+1Ch)      // ascending Priority
    if (page->vtable[+38h]() &&
        (manager->exclusive_6C == 0 || page == manager->exclusive_6C))
        00AA8BD0(ECX = page, origin = {0,0,0});
if (g_hit_widget != 0 && g_hit_widget->byte_84 == 0 && cursor->vtable[+28h]())
    g_hit_widget->vtable[+68h](0);
```

Manager `+6Ch` is an exclusive-page filter: when it is set, only that one page is
hit tested. The constructor zeroes it and **no writer was found**, so the name
comes from the use, not from a setter.

The final call is the click delivery: the cursor device's vtable `+28h` is a
button predicate, and the hit widget's vtable `+68h` is the notification.

### 00AA8BD0, the widget pass, and the winner rule

Per widget: half extents are `size.x * scale.x` (`+18h * +20h`) and
`size.y * scale.y`; the absolute position is `pos + origin`; the child origin is
the absolute position minus the half extents in X and Y, with Z unchanged (the
`FSUB double [00D7A258]` at `00AA8C2C` subtracts +0.0).

**Children are walked first**, in list order and only when their vtable `+38h`
reports visible, then the widget itself is tested. The self-test needs the byte
at `+78h` (MouseHit) set and the byte at `+77h` clear, then passes four limit
fields at `+38h`..`+44h`, each disabled by an exact +0.0f compare, then the box
containment test against the two pointer globals.

The winner is chosen by

```
if (half_extent_x < g_hit_score_00F8BC7C) {     // 00AA8E0C, strict
    g_hit_widget_00F8BC70 = this;
    g_hit_score_00F8BC7C = half_extent_x;
}
```

**The smallest horizontal half extent wins.** Not the deepest widget, not the
nearest in Z, not the last in the list. A parent that also contains the pointer
loses to any smaller child that contains it, a small widget on a low-`Priority`
page beats a large one on a high-`Priority` page, and a tie keeps the first
candidate found because the compare is strict. Z is not read anywhere in this
routine, so the `Pos` Z that orders drawing has no effect on hit testing.

### The `_Mouse` and `_Highlight` overlays

Both are ordinary pages loaded by `BSP_GuiManager_LoadResources` (`00AA5E20`,
docs/APP_INIT_FONTS_GUI.md) and made visible immediately through the same
`SetVisible` slot; the manager then keeps direct pointers to their widgets.

| Manager field | Widget | Script |
| --- | --- | --- |
| +4Ch | the `_Mouse` page | `_mouse.lua`, `Priority` -1000 |
| +50h and +54h | `MousePtrFE_Icon` | 3 states, `common/mouse/cursor*.tga` |
| +58h | `MousePtrGUI_Icon` | the in-game cursor |
| +74h | `hl_FrameBox` | `_highlight.lua`, `Priority` -1000, `RenderOrder` 0 |
| +78h | `hlCircle_FrameBox` | the same page |

`00AA3910` drives only the icon at `+50h`; `+58h` is never touched by the pointer
pass read here. After the hit test the FE icon gets
`vtable[+88h](state, 0, 1.0f)` where `state` is 1 when something was hit and its
byte `+84h` is clear, otherwise 0. Given the three authored states this is a
cursor-shape select; which state index maps to which texture was not established.

`BSP_GuiManager_ResetScreens` (`00AA0F70`) moves the two highlight frame boxes to
(-1, -1, 0) and hides them at the top of **every** update, so a highlight is a
one-frame effect a screen must re-issue.

## Teardown

`00AA38F0` is the scalar deleting destructor: `00AC5480`, then `00AC4C40` (the
pool release) when bit 0 of the flags byte is set, returning the layer in EAX.

`00AC5480` re-stamps the vtable and branches on `+0F4h`:

```
if (this->owns_store_0F4 == 0) {                 // borrowed scene
    if (this->scene_0EC) {
        if (InterlockedDecrement(&scene->refcount_4) == 0)
            scene->vtable[0]();
        this->scene_0EC = 0;
    }
} else {                                          // owns the store
    00AA4B30(ECX = BSP_GuiManager_GetOrCreate(), this->store_0F0);
    if (this->scene_0EC) { 00B72250(...); this->scene_0EC = 0; }
}
if (this->name_data_104) pool_return(name_data, name_length + 1);
00AA9730(this);                                   // the widget destructor
```

Nothing here removes the layer from the page vector at manager `+14h`. No
un-register routine was found in this packet, and `BSP_GuiManager_LoadPage` has
no unload counterpart; the reference count at page `+4h` is raised on the
already-loaded path and nothing read here lowers it. Pages therefore look like
they live for the process.

### 00AA4B30 and its flow gap

`__thiscall(ECX = manager, cGuiCameraStore *store)`, `RET 4`, `00AA4B30`..
`00AA4C1A`. It logs `cGuiManager:Removing camerastore 0x%x`, walks the multimap
at manager `+8h` (head at `+0Ch`) comparing node `+10h` with the argument, logs
`cGuiManager:Deleting camerastore 0x%x/%s` with the scene name, frees the store
and erases the node.

**`bsp.py ghidra flow 00aa4b30` reports one gap**: 10 bytes at
`00AA4BFB`..`00AA4C05`, hidden by a no-return annotation on `_free`
(`00BF65AC`) at `00AA4BF6`. The disk bytes are

```
00aa4bf5: push eax                 ; the store
00aa4bf6: call 0xbf65ac            ; _free
00aa4bfb: add esp, 4
00aa4bfe: mov dword ptr [esi+0x10], 0
00aa4c05: push esi ; push edi ; lea ecx,[esp+0x20] ; push ecx
00aa4c0c: mov ecx, ebx
00aa4c0e: call 0xaa46b0            ; std::_Tree::erase
```

So the decompiler's `if (store == 0) erase(); else free(store);` is an artefact:
the real order is free the store, null the node's payload, then erase the node.
Not repaired here, as instructed.

## The installed pages

Counted over the 97 `interface/*.lua` files of this installation, read-only.

| Fact | Count |
| --- | --- |
| Script files | 97 |
| Files declaring a top-level `GuiScreen` table | 96 (`_common.lua` is the prelude) |
| Files setting `GuiScreen["Priority"]` | 37 |
| Distinct `Priority` values | 25 |
| Files setting `GuiScreen["RenderOrder"]` | **2** |
| Distinct `RenderOrder` values across all pages | **2** |

The 25 distinct priorities are -100000, -10000, -1000, -200, -100, -50, -1, 0,
10, 20, 30, 60, 80, 90, 100, 141, 160, 169, 170, 193, 200, 265, 289, 302, 1000.
The 59 pages that do not set the key take the constructor's 0, which is already
in that set.

`RenderOrder` is set by exactly two files: `_debugtexts.lua` sets 4 and
`_highlight.lua` sets 0, which is the default anyway. So on this installation the
draw map holds at most **two** keys, 0.0f and 4.0f, and the entire GUI is drawn
in at most two orthographic passes with the debug text overlay last. The
remaining descriptor fields (`+108h` flags, near, far, scale) have no Lua key at
all -- `00AC4C50` never writes them -- so every page that keeps the constructor
defaults shares one store, one camera and one scene.

Other top-level keys the shipped pages carry are `Pos`, `geOrder`,
`WideScreenAlign`, `Size`, `Editable` and `ClearProps`. None of them is read by
the layer's own reader; they belong to `BSP_GuiWidget_ReadPropertiesAndChildren`.
`geOrder` remains dead, as docs/GUI_RENDER_ORDER.md established.

## Corrections and completions to other docs

- **docs/GUI_LAYOUT_LOADER.md**: the second `LoadPage` argument at screen `+120h`
  is recorded as uninterpreted. It gates the camera-store scene reuse in
  `00AC59A0`; see "what +120h selects".
- **docs/GUI_LAYOUT_LOADER.md**: "there is no separate layer container" is
  confirmed from the other side -- the page class and `cGuiLayer` are one class,
  sharing vtable `00D5BE38` and size `124h`.
- **docs/GAME_BLOCKING_SCREEN.md**: manager `+48h`, whose readers are recorded as
  not found, is read at `00AA3A36` and gates the whole pointer pass.
- **docs/GAME_BLOCKING_SCREEN.md**: `00AA3910`'s interior, recorded as not
  opened, is the motion and clamp pass documented above.
- **docs/GUI_RENDER_ORDER.md**: the layer name is a `NativeString` **whose object
  starts at `+100h`** (length at `+100h`, data at `+104h`), not a pointer at
  `+100h`. `00AC3FA0` returning `ECX+100h` is the evidence.

## Uncertainties

- Manager `+6Ch` has no writer in anything read here; "exclusive page" is named
  from its single use in `00AA2F10`.
- The four limit fields at widget `+38h`..`+44h` are transcribed comparison by
  comparison, but the four floats the vtable `+64h` call rewrites in place reach
  the comparisons through registers Ghidra lost, so which of `left`, `right`,
  `top` and `bottom` each limit is measured against is provisional. The
  reconstruction takes the box as an input rather than deriving it.
- The cursor icon's vtable `+88h` argument triple `(state, 0, 1.0f)` is recorded
  from the listing; the mapping from state index to authored `States` entry was
  not established.
- The tail of `00AC59A0` calls `00BF7030` and `00BF7420` and feeds the result to
  `00B8E6C0` when the base byte at `+74h` is clear. It was not followed.
- `00AC4AD0`, the int describe helper `00AC4D30` calls, was read only at its call
  site.
- Whether a page is ever unloaded is open: no un-register and no reference
  release were found.

## What remains

- The widget-side `00AA8450` (`PropagateVisibility`) takes five stack arguments
  from `00AC4450`; only the two the layer supplies are established here.
- `00AC4E50` and `00AC47F0`, the pool acquire and release, were read enough to
  establish that the size immediate at each call site is dead. The pool's own
  block size and chunk layout (`0x2544` per chunk through `00AC4070`) were not
  worked out.
- `00AC57E0` (`SetName`) has no caller in the shipped path that was found.

## Follow-up packets proposed

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `gui_widget_hit_rect` | 00AA8BD0 vtable `+64h` targets, 00AAABA0 | docs/GUI_WIDGET_HIT_RECT.md, include/bsp/gui_hit_rect.hpp | Recover the four floats vtable `+64h` rewrites and settle which edge each `+38h`..`+44h` limit bounds |
| `gui_page_lifetime` | 00AA5840 callers, page `+4h`, 00AA31F0 | docs/GUI_PAGE_LIFETIME.md | Establish whether a page is ever released, and what lowers the reference count `00AA5840` raises |
| `gui_cursor_states` | 00AA3910 `+88h` target, 00AA0E50, cursor device `+28h`/`+3Ch`/`+40h` | docs/GUI_CURSOR.md | The cursor device contract and the state index the icon takes |
| `gui_manager_exclusive_page` | writers of manager `+6Ch` | docs/GUI_MODAL_PAGE.md | Find the setter and the screens that use the exclusive-page filter |

## Reconstruction

`include/bsp/gui_layer.hpp` and `src/gui_layer.cpp`, registered in
`cmake/startup.cmake`, build into `bsp_core` with `/W4 /WX` for Win32. They
provide the `124h` layout as `bsp::GuiLayerImage` with documented offsets, the
vtable table as data, the ordering and visibility rules as pure functions, the
pointer scale and clamp, the hit-test recursion and winner rule, and the create,
update and teardown sequences over `bsp::GuiLayerHost`, one method per native
call site, in the style of `bsp::run_application_frame`. The camera-store types
come from `gui_render_order.hpp` and the script-name constants from
`gui_layout_loader.hpp`; neither is duplicated. No global is invented and no
unresolved call is stubbed.

One case was added to `tests/math_tests.cpp`, for the winner rule: a parent that
contains the pointer must lose to its smaller child, an invisible child must be
skipped, and a tie must keep the first candidate.

| Address | State |
| --- | --- |
| 00AA3840, 00AC6600, 00AA3BD0 | exported, analyzed, reconstructed, build-tested |
| 00AC5480, 00AA38F0 | exported, analyzed, reconstructed, build-tested |
| 00AC4450, 00AC4C50 | exported, analyzed, reconstructed, build-tested |
| 00AA3910, 00AA2F10, 00AA8BD0 | exported, analyzed, reconstructed, build-tested |
| 00AA87B0 | exported, analyzed, reconstructed (the gate and the accumulator), build-tested |
| 00AA4B30 | exported, analyzed (flow gap reported, not repaired), reconstructed as a host call |
| 00AC4D30, 00AA38E0, 00AA38B0, 00AC3F80, 00AC3F90, 00AC3FA0, 00AC3FB0 | analyzed from the listing (no Ghidra function), reconstructed, build-tested |
| 00AC57E0 | analyzed from the listing |
| 00AC51A0, 00AC4C40, 00AC4E50 | analyzed at the call sites |
| The 97 `interface/*.lua` files | installed-file-checked |

# Correction from GUI native scene integration

The open page-unload question below is resolved by `00AA31F0`. Its verified
assembly removes a matching page from manager+14h's vector, calls current
virtual20(), then current virtual04(1), with RET4 at00AA327C. It performs
direct deletion rather than a page-reference decrement. See
`docs/GUI_NATIVE_SCENE.md` for the disposal fragment, evidence and fixture.
