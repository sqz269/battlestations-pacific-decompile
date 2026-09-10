# GUI widget transform

Addresses: 00AA6740 00AA6750 00AA8240 00AA70E0 00AA7220 00AA6980 00AA7970 00AA78D0 00AA78F0 00AA7910 00AA7930 00AA7950 00AA7D00 00AA7D40 00AA7D80 00AA7DC0 00AA8710 00AA83A0 00AAAED0 00A9E110

## What the block is

The five addresses of the packet are the transform of the GUI widget base
class, the class the front-end scroll block in docs/FRONTEND_SCREEN_ANIMATION.md
drives without having identified. The block turns out to be five routines over
one base layout, plus a recursive layout pass that nothing in the earlier packet
had reached:

| Address | ABI | RET | Role |
| --- | --- | --- | --- |
| 00AA6740 | `__thiscall(widget) -> float(*)[2]` | RET | `LEA EAX,[ECX+20h]; RET`, the address of the Size pair |
| 00AA6750 | `__thiscall(widget, Vec3* out) -> out` | RET 4 | Resolved position through the parent chain |
| 00AA8240 | `__thiscall(widget, const Vec3*)` | RET 4 | The inverse, plus a recompose and a bounds refresh |
| 00AA7220 | `__thiscall(widget)` | RET | Compose the local matrix, hand it to the scene node |
| 00AA70E0 | `__thiscall(widget)` | RET | Publish the local bounding volume |
| 00AA8710 | `__thiscall(widget)` | RET | Recursive wide-screen layout pass over the child tree |

## Correction to the earlier reading

docs/FRONTEND_SCREEN_ANIMATION.md describes +18h/+1Ch as "the parent's scale
pair". It is the **Pivot**, and Scale is a different pair at +28h/+2Ch that the
parent chain never touches. Three independent pieces of evidence:

- The descriptor table at 00AAAED0 binds `widget+18h` to the literal `Pivot`
  (00d5c228), `widget+20h` to `Size` (00cff278) and `widget+28h` to `Scale`
  (00ce60ac). Each record is `LEA ECX,[EDI+off]` followed by the string the
  visitor fallback pushes: 00AAAFAD/00AAAFF3, 00AAAF54/00AAAF9F, 00AAB006/00AAB034.
- The base constructor 00AA9390 leaves +18h/+1Ch at zero (00AA93A6) and sets
  +28h/+2Ch to 1.0 (00AA93B4, the float at 00d7a24c). A scale pair defaulting to
  zero would collapse every widget; a pivot fraction defaulting to zero is the
  top-left corner.
- 00AA7220 uses the two pairs differently: +28h/+2Ch becomes the diagonal of a
  scale matrix, while +18h/+1Ch is multiplied by the widget's own size to make a
  translation. Only a normalised pivot behaves that way.

## The base layout

Built by 00AA9390, `__thiscall(this, int type_tag)`, RET 4. The object is 100h
bytes; the derived classes 00A9DF40 and 00AB5C60 start their own fields at
+100h. Offsets established here:

| Offset | Type | Name and evidence | Default |
| --- | --- | --- | --- |
| +00h | vtable | 00d5bbf8 and 00d5bed0 are two sibling tables; both start with 00bd30e0 and both hold 00AA7970 at +58h | |
| +04h | int | reference count, `param_1[1] = 1` at 00AA939C | 1 |
| +08h | float | authored X; only writer 00AA7D80, only reader 00AA8710 | 0 |
| +0Ch..+14h | float[3] | `Pos`, 00cf168c; the widget's own translation | (0,0,0) |
| +18h/+1Ch | float[2] | `Pivot`, 00d5c228; a fraction of the widget's own size | (0,0) |
| +20h/+24h | float[2] | `Size`, 00cff278; handed out by 00AA6740, set by virtual +58h | (0,0) |
| +28h/+2Ch | float[2] | `Scale`, 00ce60ac; the scale matrix diagonal | (1,1) |
| +48h | float | `Rotate`, 00d5c220; set by 00AA7930 | 0 |
| +4Ch | ptr | the scene node every publish goes through | 0 |
| +50h..+5Ch | float[4] | `Color`, 00ce93f8; +5Ch is the alpha virtual +4Ch writes | (1,1,1,1) |
| +60h | int | the base constructor's type tag; virtual +5Ch returns it | argument |
| +64h..+6Ch | std::list | children: stateless allocator, sentinel at +68h from 00A9B720, count at +6Ch | empty |
| +70h | ptr | parent widget; cleared by 00AA83A0 | 0 |
| +74h | bool | gate of 00AA70E0; a clear byte means no bounding volume | 0 |
| +78h | bool | `MouseHit`, 00d5c1cc | 0 |
| +84h | bool | `MouseBlock`, 00d5c1d8 | 0 |
| +A4h..+B0h | float[4] | `LowColor`, 00d5c214 | (0,0,0,1) |
| +B4h..+C0h | float[4] | `HighColor`, 00d5c208 | (1,1,1,1) |
| +C4h | float | `BlendFactor`, 00d5c1fc | 0 |
| +DCh | ptr | a layout listener; virtual +1Ch at 00AA8791 | 0 |
| +E0h | int | `WideScreenAlign`, 00d5c1ec; enumerators `Left` and `Right` | 0 |

The children list is an MSVC `std::list`: 00A9B720 allocates twelve bytes and
links the node to itself in both directions, the classic sentinel. 00AA8320 and
00AA8710 iterate it as `head = [this+68h]; node = [head]; payload = [node+8h];
node = [node]`, which is `_Next, _Prev, _Myval`. 00AA8710 bounds the loop by the
count at +6Ch rather than by the sentinel.

`Visible` is in the descriptor table but has no plain storage offset: the record
at 00AAB301 is emitted only when virtual +5Ch does not return 1, and +5Ch
(00A9E110) returns the type tag at +60h. Where the visibility flag actually
lives was not established.

### The vtable

Base table 00d5bbf8, deduced from the fact that its entry at +58h is 00AA7970,
which writes the Size pair the front-end scroll block feeds through that slot.
Slots identified:

| Slot | Target | Role |
| --- | --- | --- |
| +20h | 00AA8320 | walks the children calling each child's own +20h |
| +44h | 00AA7930 | set Rotate |
| +48h | 00AA7950 | set Scale |
| +4Ch | 00AA6980 | set alpha; writes +5Ch, then the node's material |
| +58h | 00AA7970 | set Size; writes +20h/+24h, then recomposes |
| +5Ch | 00A9E110 | return the type tag at +60h |
| +60h | 00A9CD20 | called by 00AA7170 before a bounds refresh |
| +24h | | tail-jumped at the end of the layout pass |
| +38h | on the *node*, not the widget | receives the composed matrix |

This confirms the two slots docs/FRONTEND_SCREEN_ANIMATION.md named from use.
+4Ch is the alpha setter and +58h is the size setter, and both are base-class
implementations rather than per-screen behaviour.

## 00AA6750, the resolved position

```
resolved(w):
    if w->parent == 0:                                    # 00AA67C6
        out = w->pos                                      # +0Ch..+14h
    else:
        p = w->parent
        px = p->size.x * p->pivot.x                       # 00AA675F, 00AA6766
        py = p->size.y * p->pivot.y                       # 00AA676E, 00AA6771
        r  = resolved(p)                                  # 00AA6778, direct recursion
        sx = r.x + w->pos.x                               # 00AA677D..00AA6782
        sy = r.y + w->pos.y                               # 00AA6786..00AA678C
        sz = r.z + w->pos.z                               # 00AA6790..00AA679B
        out.x = sx - px                                   # 00AA679F..00AA67A6
        out.y = sy - py                                   # 00AA67A8..00AA67B0
        out.z = sz - 0.0                                  # 00AA67B3, double at 00d7a258
    return out
```

Two things the listing settles that the pseudocode would not. First, the size
lane is the x87 load and the pivot lane the multiplier, in that order. Second,
the three sums are stored to the frame at ESP+0Ch/+10h/+14h **before** the pivot
is subtracted; the subtraction is a second pass over those slots. Folding it in
as `(r.x - px) + pos.x` is a different float result, which the regression case
in tests/math_tests.cpp pins at 2^24.

The Z lane subtracts the double 0.0 at 00d7a258, so Z accumulates down the
parent chain with no pivot term at all.

The frame is `SUB ESP,20h; PUSH ESI`, so the out pointer is at `[ESP+28h]` and
the recursive call reuses `[ESP+18h]` as the parent's out buffer.

## 00AA8240, the inverse

Same prologue on the parent, then:

```
    d.x = world.x - r.x        # 00AA8275..00AA8279
    d.y = world.y - r.y        # 00AA827D..00AA8283
    d.z = world.z - r.z        # 00AA8287..00AA828F
    w->pos.x = d.x + px        # 00AA8293..00AA82BD
    w->pos.y = d.y + py        # 00AA829F..00AA82C4
    w->pos.z = d.z + 0.0       # 00AA82AB..00AA82CB
    00AA7220(w); 00AA70E0(w)   # 00AA82CE, 00AA82D5
```

The no-parent path at 00AA82E1 copies the three floats with `MOVSS` and takes
the same two calls. The differences reach the frame before the pivot is added
back, mirroring the getter exactly, so a set followed by a get is an identity in
float arithmetic and not only in real arithmetic.

## 00AA7220, the local matrix

The 1DCh-byte frame holds four row-major 4x4 matrices and three scratch floats.
Derived values, taken from the listing because the x87 stack carries the 0.75
across three operations:

```
    A = pos.y * 0.75                    # 00AA7231..00AA724E, float 0.75 at 00e12fc4
    B = (-pivot.x) * size.x             # 00AA7273..00AA7298
    C = ((-pivot.y) * size.y) * 0.75    # 00AA72A5..00AA72E1, the 0.75 still on ST0
    angle = -0.0f - rotate              # 00AA7246, 00AA7250, -0.0f at 00d7a208
```

`FMULP ST(2)` at 00AA724E multiplies the duplicated 0.75 into the copy of pos.y
two slots down; `FMULP` at 00AA72C4 folds the surviving 0.75 into the Y pivot
term after it has already been negated and sized. The X pivot term never sees
the factor. The matrices:

| Frame slot | Matrix | Content |
| --- | --- | --- |
| ESP+98h | M1 | `T(pos.x, A, pos.z)`, identity elsewhere, rows at 00AA72C6..00AA733C |
| ESP+18h | M2 | `diag(scale.x, scale.y, 1, 1)`, 00AA7345..00AA739F |
| ESP+58h | M3 | `T(B, C, 0)`, 00AA73E4..00AA73FF |
| ESP+164h | R | `RotZ(angle)` from 00b64780, which reads only the first float at its EDX |

`BSP_Matrix_Multiply4x4` (00413920) is `__thiscall(left, destination, right)`,
RET 8, EAX = destination, row-major left*right. The two arguments pushed at
00AA740F and 00AA7417 are not consumed by 00b64780, which cleans nothing, and
are reused as the third multiply's arguments. That reuse is what fixes the chain:

```
    R1 = M3 * M2      # 00AA7442
    R2 = R1 * R       # 00AA7449
    R3 = R2 * M1      # 00AA7450
    node = w->+4Ch; node->vtable[38h](R3)   # 00AA7455..00AA745C
```

The stack balance confirms it. Seven pushes, 1Ch bytes, must be undone before
`POP EDI` at 00AA745E: three multiplies cleaning 8 each plus a `__thiscall`
virtual cleaning 4 is exactly 1Ch, and no other split of RET sizes closes.

On row vectors the effective order is pivot, then scale, then rotate, then
translate. The 0.75 on the Y lanes is 3/4, the 4:3 aspect ratio, so widget space
looks X-normalised with Y compressed; that reading comes from the value alone
and is a hypothesis.

## 00AA70E0, the bounding volume

Gated entirely on the byte at +74h (00AA70E6). When set it reaches an object
through the node at +4Ch with `00b74640(0,0)` then `00b732c0`, and calls
`00b855b0` with the address of four consecutive frame floats:

```
    [ESP+0Ch] = size.x * 0.5        # double 0.5 at 00d7a280
    [ESP+10h] = size.y * 0.5
    [ESP+14h] = 0.0                 # XORPS at 00AA7108
    [ESP+18h] = 0.5 * max(size.x, size.y)
```

The max at 00AA7132..00AA714E starts with the height in the slot and replaces it
with the width only when the width is strictly larger, so a tie keeps the
height. Three half extents followed by a radius is a box plus a sphere.

Note that 00AA7970, the Size setter behind vtable +58h, calls 00AA7220 but not
00AA70E0. A resize therefore leaves the node's bounds stale until some other
path refreshes them, which is worth checking against hit testing.

## 00AA8710, where a widget rectangle reaches the frame

This is the routine that consumes and rewrites the transform every frame, and it
is the answer to how the widget rectangle reaches the draw path: it does not,
directly. The widget is a wrapper over a scene node, and both publishes go into
that node. 00AA8710 is the only place the two publishes are driven for a whole
tree:

```
    for each child in this->children (count at +6Ch):     # 00AA8714..00AA8744
        00AA8710(child)                                   # post-order
    align = this->+E0h                                    # 00AA8746
    if align != 0 and [0109cf04]->+0Dh != 0:              # 00AA874C, 00AA8756
        if align == 1: pos.x = authored_x - K             # 00AA8761, K = 00d5c118
        elif align == 2: pos.x = authored_x + K           # 00AA8771
        else: pos.x unchanged                             # 00AA876F skips the store
    else:
        pos.x = authored_x                                # 00AA877C
    if this->+DCh: (*this->+DCh)->vtable[1Ch]()           # 00AA8782..00AA8791
    00AA7220(this); 00AA70E0(this)                        # 00AA8795, 00AA879C
    jmp this->vtable[24h]                                 # 00AA87AB
```

`K` is the double at 00d5c118, bits 3FC2222240000000h, exactly
0.14166605472564697265625. It is exactly representable as a float, so it was
almost certainly authored as one. What it is a fraction of was not recovered; it
is close to 17/120 but not equal to it at either precision.

00AA8710's only caller is 00AA4EF0, which copies a container at manager+14h and
calls it on each element, so the roots come from the GUI manager.

The scroll block's own use is consistent with this: it caches base positions
from 00AA6750 and writes them back through 00AA8240, and each write recomposes
that widget alone. The manager pass rebuilds the whole tree, so a scroll offset
survives only because it is folded into the widget's own +0Ch, which the pass
rewrites on the X lane only.

## Callers and callees

Callees of 00AA7220: 00b64780 (Z rotation), 00413920 (matrix multiply), and one
node virtual. Callees of 00AA70E0: 00b74640, 00b732c0, 00b855b0. 00AA6750 calls
only itself. 00AA8240 calls 00AA6750, 00AA7220 and 00AA70E0. 00AA6740 calls
nothing.

Callers of 00AA7220 (18): the ten field setters 00AA78D0, 00AA78F0, 00AA7910,
00AA7930, 00AA7950, 00AA7970, 00AA7D00, 00AA7D40, 00AA7D80, 00AA7DC0; then
00AA8240, 00AA8710, 00AAA710 (the property reader), 00AAB4C0, 00AB1F60,
`BSP_GUIStatefulImage_RebuildStateGeometry`, 00ACF1D0 and 00AD0D80.

Callers of 00AA70E0 (8): 00AA7170, 00AA7D00, 00AA7D40, 00AA7D80, 00AA7DC0,
00AA8240, 00AA8710, 00AAB4C0. 00AA6740 has 97 callers and 00AA6750 has 92,
almost all front-end screens.

## Uncertainties

- Which `WideScreenAlign` literal, `Left` or `Right`, is 1 and which is 2. The
  descriptor table emits both names in one record and the pairing was not read.
- Where the `Visible` flag is stored. Virtual +5Ch returns the type tag, so the
  gate at 00AAB2E9 is a type test, not a visibility read.
- What `K` at 00d5c118 is a fraction of, and whether the 0.75 is genuinely the
  4:3 aspect ratio or a coincidence.
- The class name. Neither 00d5bbf8 nor 00d5bed0 has an MSVC complete-object
  locator in the dword before it, so RTTI gives nothing.
- The byte at +0Dh of 0109cf04 is read as "wide screen is on" from this one use.
- 00AAB4C0, the remaining caller of both publishes, goes through
  `BSP_GuiManager_GetOrCreate` and 00AA6560 with the type tag; it was not read
  beyond its first twenty instructions.

## What remains

- The scene node class behind +4Ch and its virtual +38h, which is where the
  composed matrix actually becomes a draw. That is render territory.
- 00AAA710, the property reader, which is the counterpart of 00AAAED0 and the
  only place a widget's parent is attached alongside `BSP_Node_SetParent_Provisional`.
- Whether the missing bounds refresh in 00AA7970 is a real defect in hit testing.

## State reached

| Address | State |
| --- | --- |
| 00AA6740 | reconstructed, build-tested |
| 00AA6750 | reconstructed, build-tested, fixture-tested |
| 00AA8240 | reconstructed, build-tested, fixture-tested |
| 00AA7220 | reconstructed to the matrix inputs, build-tested, fixture-tested; the multiply chain is described, not ported |
| 00AA70E0 | reconstructed, build-tested |
| 00AA8710 | reconstructed, build-tested |
| 00AA6980, 00AA7970, 00AA78D0..00AA7DC0, 00AA83A0, 00AAAED0, 00A9E110 | analyzed |

Nothing here is ABI-compatible or game-validated. `bsp::GuiWidgetTransform` is a
projection with a `std::vector` where the original has an intrusive list.

## Listing-only routines

00A9E110 has no Ghidra function: the body is `MOV EAX,[ECX+60h]; RET` at
00A9E110..00A9E113 followed by int3 padding. It must be defined before
`BSP_GuiWidget_GetTypeId` can be applied.

## Follow-up packets proposed

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `gui_widget_property_reader` | 00AAA710, 00AAA480, 00AA5A00, 00AA6560 | docs/GUI_WIDGET_PROPERTY_READER.md, include/bsp/gui_widget_properties.hpp | The reader half of the descriptor table at 00AAAED0: how a parsed layout value reaches a field, and where a widget's parent is bound alongside `BSP_Node_SetParent_Provisional`. |
| `gui_widget_node_binding` | 00AA6560, 00AAB4C0, 00AA7170, 00A9CD20 | docs/GUI_WIDGET_NODE_BINDING.md | How the scene node at widget+4Ch is created and attached, and what virtual +60h does before a bounds refresh. |
| `gui_widget_visibility` | 00AA8320, 00A9CD20, 00AA6A30 | docs/GUI_WIDGET_VISIBILITY.md | The visibility and enable flags the descriptor table names but does not locate, and the recursive +20h walk. |

## Corrections from docs/GUI_WIDGET_SCENE_VISIBILITY.md

- The scene node at widget+4Ch is a separate 0x184-byte object created by `00aa6640` for a fresh widget and cloned by the copy constructor `00aa9520`; attach is `BSP_Node_SetParent_Provisional` with the child's node in ECX.
- Virtual +60h is not visibility: its base writes the byte at widget+85h that one control class reads, so `00aa7170` clears that byte and republishes the bounds.
- The authored `Visible` property is the byte at widget+E4h; effective visibility is the float at node+ACh, written by `BSP_GuiWidget_SetVisible` after the ancestor walk and the propagation walk, with widget+75h deciding whether a descendant takes the requested value. Draw skips on the node factor; hit tests skip on virtual +38h plus the +77h and +78h bytes.
- The vtable +20h walk is teardown (children first, kind-of test, unlink, release), and `00aab4c0` is a subtree clone because `00aa6560` is a type-tag factory switch.
