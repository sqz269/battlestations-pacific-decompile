# The GUI widget's scene node and its visibility (packet `gui_widget_scene_visibility`)

Addresses: 00aab4c0, 00aa7170, 00a9cd20, 00aa8320, 00aa6a30, 00aa8530, 00aa8450,
00a9cce0, 00a9e0d0, 00a9e0b0, 00a9e070, 00aa6640, 00aa6720, 00aa6820, 00aa9520, 00aa9730,
00aaa5a0, 00ab73b0, 00a9c380.

This packet answers the four questions docs/GUI_WIDGET_TRANSFORM.md left open. The short
version: the scene node at widget+4Ch is a separate 184h-byte object the widget owns a pointer
to; the authored `Visible` property lives at widget+E4h and reaches the scene through virtual
+34h; visibility is *stored* on the node as a float factor at node+ACh and the widget tree is
walked only to fire an edge notification; and the recursive vtable +20h walk is the teardown
that releases every node under a widget.

`00AA7170` is not part of visibility at all. It is a deactivate-and-relayout pair, and virtual
+60h writes an unrelated byte at +85h.

## Calling conventions and RET sizes

| Address | Signature | RET | Role |
| --- | --- | --- | --- |
| 00AAB4C0 | `__thiscall(widget, GuiWidget* parent_clone) -> GuiWidget*` | RET 4 | Clone a widget subtree |
| 00AA7170 | `__thiscall(widget)` | RET | virtual +60h with 0, then a tail jump into 00AA70E0 |
| 00A9CD20 | `__thiscall(widget, char active)` | RET 4 | virtual +60h override: base, then re-state the items |
| 00AA6A30 | `__thiscall(widget, char active)` | RET 4 | virtual +60h base: `[this+85h] = arg` |
| 00AA8320 | `__thiscall(widget)` | RET | virtual +20h: recursive scene-node release |
| 00AA8530 | `__thiscall(widget, char visible)` | RET 4 | virtual +34h base: SetVisible |
| 00AA8450 | `__thiscall(widget, char, char, char, char, char)` | RET 14h | the visibility propagation walk |
| 00A9CCE0 | `__thiscall(widget, char visible)` | RET 4 | virtual +34h override: base, then the linked widget |
| 00A9E0D0 | `__thiscall(widget) -> bool` | RET | virtual +38h: is this widget visible |
| 00A9E0B0 | `__thiscall(widget, const float pivot[2])` | RET 4 | virtual +30h: set Pivot, then recompose |
| 00A9E070 | `__thiscall(widget, const void* class_desc) -> bool` | RET 4 | virtual +0Ch: hand-rolled kind-of test |
| 00AA6640 | `__fastcall(type_tag, const char* node_name, void* ctor_arg) -> GuiWidget*` | RET 4 | create a widget and its node |
| 00AA6720 | `__thiscall(widget, void* node)` | RET 4 | bind a node to a widget |
| 00AA6820 | `__thiscall(widget, char recurse)` | RET 4 | set widget+75h |
| 00AA9520 | `__thiscall(widget, const GuiWidget* src) -> widget` | RET 4 | base copy constructor, SEH `00cb73f1` |
| 00AA9730 | `__thiscall(widget)` | RET at 00AA99B8 | base destructor, SEH `00cb7421`; Ghidra's stored body stops at 00AA9950 on a false no-return |
| 00AAA5A0 | `__thiscall(parent, GuiWidget* child, void* position)` | RET 8 | add a child, widget side and node side |
| 00AB73B0 | `__thiscall(widget)` | RET | release the secondary node at widget+188h |
| 00A9C380 | `__thiscall(control, int index, char visible)` | RET 8 | set item `index`'s hidden byte |

Ghidra drops stack arguments for the unprototyped callees, so every signature above comes from
the listing. Three of the routines are decompiled with the wrong argument shape: 00A9CD20's
call to 00AA6A30 shows as `FUN_00aa6a30(param_2)` when ECX still holds `this` (00A9CD2A),
00AAB4C0's recursion shows as `FUN_00aab4c0(iVar2)` when ECX is the child payload and the
stack argument is the clone, and 00AAA5A0's tail shows `BSP_Node_SetParent_Provisional(parent+4Ch)`
when ECX is the *child's* node and the parent's node is the argument (00AAA616..00AAA61D).

## Where the scene node comes from

`00AA6640` is the only place a node is created for a fresh widget:

```
00aa665d  widget = 00AA6560(ECX = type_tag, EDX = stack arg)   ; the type factory
00aa6669  mem    = 00B74EB0(ECX = 184h)                        ; allocate
00aa6681  node   = 00B75030(ECX = mem, name)                   ; construct with a name string
00aa6695  widget->00AA6720(node)                               ; bind
00aa66a1  widget->vtable[74h]()                                ; post-construct hook
```

`00AA6720` is the bind, three instructions of work:

```
00aa6720  EAX = [ESP+4]
00aa6726  [ECX+4Ch] = EAX
00aa672b  if (EAX) [EAX+138h] &= FFFFFFFCh    ; clear the low two bits of the node's flag word
```

The base constructor `00AA9390` clears +4Ch (00AA9423), so a widget without that factory has no
node and every routine in this packet treats it as inert. The copy constructor `00AA9520` takes
the other path: instead of allocating, it clones the source's node through the node's own
virtual +10h and passes a per-type name from the table at `00d5c0b8` indexed by the type tag:

```
00aa96e4  ECX = src->[4Ch]
00aa96e7  EDX = src->[60h]                       ; the type tag
00aa96ec  EDX = [EDX*4 + 00d5c0b8]               ; the per-type node name
00aa96fc  node = ECX->vtable[10h](EDX)
00aa9700  this->[4Ch] = node
00aa9705  if (node) [node+138h] &= FFFFFFFCh     ; the same two bits
```

The bits are the only thing the GUI writes in node+138h. docs/CAMERA_DIRTY_PROPAGATION.md and
docs/CAMERA_ATTACHED_CALLBACK.md establish bits 4h/8h/10h/20h of that word as transform
invalidation state; bits 1h and 2h are not covered there and are not established here either.

`00AAA5A0` is the attach. Its list half splices the child into the parent's `std::list` at +64h
(or at an explicit position when the third argument is non-null) after detaching it from any old
parent through `BSP_GuiWidget_DetachChild`. Its node half is two instructions:

```
00aaa613  child->[70h] = parent
00aaa616  EDX = parent->[4Ch]
00aaa619  ECX = child->[4Ch]
00aaa61d  BSP_Node_SetParent_Provisional(ECX = child_node, EDX = parent_node)
```

Neither node pointer is tested, so a widget with no node still reparents a null. **The GUI has
no scene layer or camera of its own in this packet:** the parent chain is the only thing that
places a widget's node, and the root of that chain is whatever node the front-end screen bound.
Which node that is was not established here.

## Virtual +60h, and what 00AA7170 really does

`00AA7170` is widget vtable +78h in the `00d5bed0` table (the `00d5bbf8` table overrides it with
`00A9C050`). Nine instructions:

```
00aa7175  EDX = vtable[60h]
00aa7178  push 0
00aa717a  call EDX                 ; virtual +60h with the single argument 0
00aa717c  ECX = this
00aa717f  jmp 00AA70E0             ; tail jump into BSP_GuiWidget_RefreshLocalBounds
```

The base of virtual +60h is `00AA6A30`, which stores its byte argument at widget+85h and nothing
else. So the pair is "clear +85h, then republish the bounding volume", not a visibility change.

**Widget+85h is not visibility.** The only writer is 00AA6A30 and the only readers are in the
segment-75 control class: the per-frame update at `00A9D048` refuses to run while it is clear,
and the navigation handler at `00A9CA90` refuses a target widget whose byte is clear. `00ABC040`
calls `00AA7170` and then `00AA6A30(1)`, i.e. relayout then re-arm. "Active" is a reading from
those three uses and is marked provisional.

`00A9CD20` is the override. It calls the base with the same byte and then, when the byte is being
*cleared* and the control's byte at +11Eh is set, re-states every entry of its item list:

```
00a9cd2c  00AA6A30(ECX = this, arg)
00a9cd31  if (arg) return
00a9cd35  if (![this+11Eh]) return
          for (node = *[this+100h]; node != [this+100h]; node = *node)
00a9cd8f      00A9BA90(ECX = this, &list, node, node != [this+10Ch] ? 3 : 1)
```

`[this+10Ch]` is the selected entry: it gets state 1 and everything else gets state 3. The list
object spans +FCh..+10Ch and is iterated the same way as the widget child list.

## Where the visibility and enable flags live

Three separate bytes, and none of them is what the earlier reading guessed.

| Offset | Name and evidence | Default |
| --- | --- | --- |
| +75h | The recurse flag SetVisible hands to `00B6DA70` (00AA8581, 00AA8598). Set by `00AA6820`, a three-instruction setter | 0 (00AA9466) |
| +76h | Cleared by the constructor (00AA9469), forced to 1 by the copy constructor (00AA9602): a "this widget is a clone" marker. No reader found | 0 |
| +77h | Hidden. `00A9C380` stores 0 for a visible item and 1 for a hidden one (00A9C3D2/00A9C3E7), so the sense is inverted from the property name | 0 (00AA946C) |
| +85h | Active; see above | 0 |
| +E4h | The `Visible` property itself | uninitialised |

`Visible` is the byte at **widget+E4h**. The descriptor writer `00AAAED0` emits the record with
the name literal at `00d5c1e4`, type 3 (bool) and default 1, and only when virtual +5Ch does not
return 1:

```
00aab2d3  if (this->vtable[5Ch]() == 1) skip
00aab2da  AL = [EDI+E4h]                       ; the current value
00aab2fa  default = { type 3, value 1 }
00aab306  if (current == default) skip         ; 00BD5680, the variant compare
00aab322  emit(name = 00d5c1e4, current)
```

The reader `00AAA710` (owned by the GUI layout loader packet) parses into the same byte and then
pushes it straight through the virtual:

```
00aaac09  EBP = EDI + E4h                      ; the storage the record points at
00aaac16  [EAX+4] = 00d5c1e4                   ; "Visible"
00aaac28  this->vtable[0Ch](record)            ; parse
00aaac2a  EAX = [EBP]                          ; the parsed byte
00aaac33  EDI->vtable[34h](EAX)                ; SetVisible
```

So +E4h is authored storage, and virtual +34h is what actually changes anything. Neither
constructor writes +E4h: `00AA9390` initialises +0Ch..+E0h and stops, and `00AA9520` copies
+0Ch..+E0h and stops. A widget whose layout omits `Visible` therefore calls `SetVisible` with
whatever the allocator left behind. That is either a defect or evidence that a derived
constructor writes it; no derived constructor was read here.

## The visibility mechanism

**Visibility is stored on the node, not on the widget.** `00A9E0D0`, widget vtable +38h, is the
whole predicate:

```
00a9e0d0  EAX = [ECX+4Ch]; if (!EAX) return false
00a9e0d7  XMM0 = [EAX+ACh]
00a9e0df  if (XMM0 <= [00d7a218]) return false     ; the float 0.0
00a9e0e8  return true
```

`node+ACh` is written by `00B6DA70 BSP_SceneNode_SetVisibilityFactor`,
`__thiscall(node, float, bool)`, RET 8: it stores the float and, when the third argument is set,
walks the node's own child list (`+34h` first child, `+3Ch` next sibling) storing the *same*
value into every descendant. It is an overwrite, not a multiply, and with the flag clear it
touches exactly one node.

`00AA8530` is SetVisible:

```
00aa8536  ok = 1; for (p = this->[70h]; p && ok; p = p->[70h]) ok &= p->vtable[38h]()
00aa8571  00AA8450(ECX = this, ok, ok, visible, this->[75h], 1)
00aa8576  node = this->[4Ch]
00aa857d  if (node) 00B6DA70(ECX = node, visible ? 1.0 : 0.0, this->[75h])
```

The order is load-bearing: the widget walk runs first, so every virtual +38h it dispatches still
observes the *old* factor. The ancestor loop stops at the first ancestor that answers false
(the `JZ 00AA8559` at 00AA8545).

`00AA8450` is the propagation walk, five byte arguments:

| Argument | Seed from 00AA8530 | Meaning |
| --- | --- | --- |
| 1 | ancestors visible | the parent chain's effective visibility *before* |
| 2 | ancestors visible | the parent chain's effective visibility *after* |
| 3 | the requested value | what SetVisible was called with |
| 4 | widget+75h | carried unchanged the whole way down |
| 5 | 1 | take argument 3 rather than this widget's own virtual +38h |

Per widget:

```
00aa8456  if (!this->[4Ch]) return                       ; and the children are not visited
00aa8460  before = arg1 && this->vtable[38h]()
00aa8479  after  = arg2 ? (arg5 ? arg3 : this->vtable[38h]()) : false
00aa84a4  if (before != after) this->vtable[3Ch](after)  ; base 00A9E100 is `RET 4`, a no-op
00aa84f4  for each child in list order:
00aa8512      child->00AA8450(before, after, arg3, arg4, arg4 ? arg5 : 0)
```

The child's fifth argument is computed branchlessly as `MOV DL,BL; NEG DL; SBB DL,DL; AND
EDX,arg5` with BL holding argument 4. That single term is the whole propagation rule, and it
mirrors `00B6DA70`'s third argument exactly:

- **recurse set.** Every descendant takes the requested value, matching the node walk that
  stamps the same factor onto the whole node subtree.
- **recurse clear.** Only the widget the call started at takes the requested value; every
  descendant is asked its own virtual +38h, matching a node walk that touched one node. A
  descendant hidden on its own node stays hidden when an ancestor is shown.

+75h defaults to 0, so the default behaviour is the second one.

`00A9CCE0` is the override in the `00d5bbf8` table: base first, then, when the control's id at
+118h is not -1, `BSP_GuiManager_GetOrCreate()`, a lookup through `00AA0F50(id)`, and the same
virtual +34h on whatever comes back. One widget can therefore slave another's visibility.

## How a hidden widget is skipped

- **Draw.** `00B748E0`, the node's queue-submission entry, opens with
  `MOVSS XMM0,[ESI+ACh]; COMISS XMM0,[00d7a218]; JBE end`. A zero factor produces no draw. It is
  a per-node test, which is why hiding a subtree needs the recurse flag; the widget-side walk
  only fires the +3Ch hook.
- **Hit test.** `00AA8BD0` walks the children first, recursing into a child only when that
  child's virtual +38h answers true (00AA8C5F), and then tests its own rectangle only when
  `[+78h]` (MouseHit) is set and `[+77h]` (hidden) is clear (00AA8C8D, 00AA8C97). So a hit test
  is stopped by the node factor on the way down and by the two bytes at the leaf.
- **Update.** `00A9D030`, widget vtable +40h, requires `[+77h]` clear, `[+85h]` set, virtual
  +38h true and a manager byte at `BSP_GuiManager_GetOrCreate()+70h` clear before it does
  anything (00A9D03D..00A9D06D).

## The recursive vtable +20h walk

`00AA8320` is the same slot in both vtables and it is the scene half of destruction:

```
00aa8325  for each child in list order: child->vtable[20h]()
00aa8362  desc = 00AB6A30()                     ; MOV EAX,[00f8be28]; RET
00aa8372  if (this->vtable[0Ch](desc)) 00AB73B0(this)
00aa8380  if (this->[4Ch]) { BSP_Node_UnlinkAndRelease(node); this->[4Ch] = 0 }
```

`00A9E070`, the base of vtable +0Ch, scans the two-entry table `00f8bc88..00f8bc90` for the
descriptor it is handed: a hand-rolled kind-of test, not RTTI. `00AB73B0` releases a second node
the matching class keeps at +188h. Because +4Ch is cleared, a second call is a no-op.

The two callers are `00AA9730`, the base destructor, which calls it before it destroys the
children and the parent link, and `00ACC6A0`. The destructor's later block at 00AA97AC..00AA97D2
re-tests `[ESI+4Ch]` and would call `00B6E680`, `00B6D890` and `00B6D940` on it; that block is
unreachable, because 00AA8320 has already cleared the field. It reads like an inlined
base-class body the compiler could not prove dead.

## 00AAB4C0 is a clone, not a bind

`00AA6560` is a 40-plus-arm switch on the type tag that dispatches to a per-type constructor, so
`00AAB4C0(this, parent_clone)` duplicates a whole widget subtree:

```
00aab4d0  clone = 00AA6560(ECX = this->[60h], EDX = this)
00aab4da  if (parent_clone || (clone && this->[70h]))
00aab4f2      00AAA5A0(ECX = parent_clone ? parent_clone : this->parent, clone, 0)
00aab4f7  for each child: child->00AAB4C0(clone)
00aab531  saved = this->[8h]
00aab551  clone->pos = clone->pos                ; a self-assignment, three loads and three stores
00aab560  00AA7220(clone)                        ; recompose
00aab567  00AA70E0(clone)                        ; refresh bounds
00aab573  clone->[8h] = saved                    ; the authored X the copy constructor skips
```

The ECX for the attach is the parent clone on the first arm and `this->[70h]` on the second
(00AAB4E8), so a top-level clone is attached beside its original. The copy constructor starts at
+0Ch, which is why +8h is copied by hand at the end. This makes `gui_widget_node_binding` and
`gui_widget_visibility` from docs/GUI_WIDGET_TRANSFORM.md both answered, but by a different
route than proposed: 00AAB4C0 is the duplicate path, not the creation path.

## How the front-end screen sets reach these flags

They do not reach them directly. `004F7620` writes the *screen* byte `+4h` (requested) and the
pump `004F8830` reconciles it with `+5h` (applied) through each screen's own virtuals, as
docs/FRONTEND_SCREEN_SETS.md and docs/GAME_FRONTEND_STATES.md establish. Nothing in that path
touches a widget. The connection is one level lower: a screen's enter or leave virtual either
builds and destroys its widget tree (creation through `00AA6640`, destruction through
`00AA9730`, which runs the +20h walk) or calls virtual +34h on the widgets it keeps. **Which
screens take which of the two routes was not established here** and needs a screen-side packet.
The one confirmed widget-side entry point from a screen is `00ACDEF0`, the only direct caller of
`00AA8530`, and `00AC4450`, the only outside caller of `00AA8450`.

## Callers and callees

| Address | Callers | Callees |
| --- | --- | --- |
| 00AAB4C0 | itself, plus the layout paths named in docs/GUI_WIDGET_TRANSFORM.md | 004C12B0, 00AA6560, 00AAA5A0, itself, 00AA7220, 00AA70E0 |
| 00AA7170 | 00A9C050, 00AAE770, 00AB10F0, 00ABC040, 00AC1BB0, 00AC78E0, 00AC8890, 00ACEB50 | virtual +60h, 00AA70E0 |
| 00A9CD20 | vtable 00d5bbf8 +60h | 00AA6A30, 00A9BA90 |
| 00AA6A30 | 00A9CD20, 00ABC040, vtable 00d5bed0 +60h | none |
| 00AA8320 | 00AA9730, 00ACC6A0, both vtables +20h | 00AB6A30, virtual +0Ch, 00AB73B0, 00B6DFA0 |
| 00AA8530 | 00A9CCE0, 00ACDEF0, vtable 00d5bed0 +34h | virtual +38h, 00AA8450, 00B6DA70 |
| 00AA8450 | 00AA8530, 00AC4450, itself | virtual +38h, virtual +3Ch, itself |
| 00AAA5A0 | 00AAB4C0 and the layout loader | 00AA83A0, 00A9B790, 00A9D480, 00A9D6C0, 00B6E680 |
| 00AA6720 | 00AA6640, 00AB98F0, 00AC6600 | none |

## Uncertainties

- **+E4h is never initialised.** Both constructors stop at +E0h. Either a derived constructor
  writes it or the first `SetVisible` after a layout load reads uninitialised memory.
- **"Active" for +85h** is a reading from three uses in one class. Nothing outside the
  segment-75 control reads the byte, so it may be that class's own state living in a base-class
  hole rather than a base-class concept.
- **Node+138h bits 1h and 2h.** Both bind sites clear them and nothing in the GUI sets them.
  Their meaning belongs to the scene packets.
- **The GUI's root node.** Established only as "whatever the parent chain ends at". No scene
  layer, camera or render-target association was recovered.
- **The node's virtual +10h** is used as a clone-with-name; the node class and its vtable are
  not identified, so that reading rests on the argument being a name string from a per-type table.
- **The self-assignment in 00AAB4C0** (00AAB53A..00AAB55B) is reproduced as a no-op. It is most
  likely an inlined `SetPos(GetPos())` the compiler folded, but a volatile or aliasing reading
  cannot be excluded from the listing alone.
- `00AA9730` keeps its placeholder ledger name. An earlier packet recorded that Ghidra's stored
  body ends at 00AA9950 on a false no-return and the real RET is at 00AA99B8, so only the head of
  the destructor was read here. The existing 00AA9390 record also names the base constructor
  `BSP_UIContext_Construct` while every routine in this packet is named `BSP_GuiWidget_*`; the two
  refer to the same class and the inconsistency is not resolved.
- `00AA9520`'s vtable stores are `00ceb130` then `00d5c130`, neither of which is one of the two
  leaf tables, so the base class has its own table and the leaf constructor overwrites it. The
  class hierarchy above the two leaf tables was not mapped.

## What remains

- The screen-side half: which front-end screens call virtual +34h and which rebuild their tree.
- `00A9BA90`, the per-item state call in the +60h override, and what states 1 and 3 mean.
- `00A9D030` and `00A9CA60` beyond their entry gates; both are large and were read only far
  enough to establish what +77h and +85h do.
- The node side of `00AA6980` (virtual +4Ch), which reaches a material colour alpha at +0Ch
  through `00B74650`, `00B74640`, `00B72B40`, `00B732C0` and `00B179F0`. Alpha and the visibility
  factor are two different fields; nothing here combines them.

## Proposed follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `gui_widget_update_tick` | 00A9D030, 00AA87B0, 00A9CA60, 00A9BA90 | docs/GUI_WIDGET_UPDATE_TICK.md, include/bsp/gui_widget_update.hpp | The segment-75 control's per-frame update and navigation handler behind the +77h/+85h gates, and the item state call the +60h override drives. |
| `gui_widget_hit_test` | 00AA8BD0, virtual +64h, 00AA8710 | docs/GUI_WIDGET_HIT_TEST.md, include/bsp/gui_widget_hit_test.hpp | The rectangle walk: the accumulated origin, the +64h rect virtual and the two byte gates, as a pure predicate over the tree. |
| `gui_widget_alpha_material` | 00AA6980, 00B74650, 00B74640, 00B72B40, 00B732C0, 00B179F0 | docs/GUI_WIDGET_ALPHA_MATERIAL.md | How widget+5Ch reaches a material colour alpha through the node's model, and how it relates to the visibility factor at node+ACh. |
| `gui_screen_widget_lifetime` | 00ACDEF0, 00AC4450, 00ACC6A0, 00AA9730 | docs/GUI_SCREEN_WIDGET_LIFETIME.md | The screen-side entry points: which screens call SetVisible and which destroy their tree, closing the gap between the screen-set pump and these flags. |

## State reached

| Address | State |
| --- | --- |
| 00AAB4C0 | reconstructed, build-tested (`clone_subtree`) |
| 00AA7170 | reconstructed, build-tested (`deactivate_and_refresh_bounds`) |
| 00A9CD20 | analyzed |
| 00AA6A30 | analyzed (a host call, `set_active`) |
| 00AA8320 | reconstructed, build-tested (`release_scene_nodes`) |
| 00AA8530 | reconstructed, build-tested (`set_widget_visible`) |
| 00AA8450 | reconstructed, build-tested and fixture-tested (`propagate_visibility`) |
| 00A9CCE0 | analyzed |
| 00A9E0D0 | reconstructed, build-tested (`widget_is_visible`) |
| 00A9E0B0, 00A9E070 | analyzed |
| 00AA6640, 00AA6720, 00AA6820 | analyzed |
| 00AA9520, 00AA9730 | analyzed |
| 00AAA5A0 | reconstructed, build-tested (node half only, `attach_child`) |
| 00AB73B0, 00A9C380 | analyzed |
