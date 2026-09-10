# GUI layout loading

Addresses: 00AA5840 00AA7E00 00AAA710 00AA6560 00AA2490 00AA52A0 00AA3140 00AC5600 00AA5A00 00AAA480

How a named page (`FE_initial`, `FE_loading`, `_Mouse`, `_Highlight`) becomes a
widget tree. The short answer is that the pages are **Lua source**: the game
embeds a Lua interpreter, `interface/<name>.lua` is executed, and the widget
tree is built by walking the global table `GuiScreen` the script leaves behind.
The key names carry the widget class in a suffix after the last underscore.

## The chain

`00AA5840`, `__thiscall(ECX = GUI manager, const NativeString *name, int
screen_flag, char add_reference)`, `RET 0Ch`, body `00AA5840`-`00AA59A9`.

1. `00AA5862`: `00AA3140(manager, name)` scans the manager's page vector for a
   page of that name. On a hit the routine returns it, first raising the page's
   reference count at `page+4h` through `00CE221C` (`InterlockedIncrement`) when
   `add_reference` is set. That third argument is 0 at every call site read.
2. `00AA5871`-`00AA588F`: builds `"<name>" + ".mmod"` (the literal at `00D0E67C`)
   into a frame string. No directory is prepended.
3. `00AA58B8`: asks the VFS singleton at `0109CEEC` (`00BDF4C0`,
   `BSP_VFS_ResolveExistingName`) whether that name exists.
   - Yes: `004C1400` for the resource manager, `00B80D70(path)` for the model,
     then the resource's vtable `+8h` called with `(0, 1.0f)` (`FLD1` at
     `00AA58E1`). The instance is stored at **manager `+30h`** and its `+0Ch`
     becomes the object handed to the screen constructor.
   - No: `00AA58FA` allocates `188h` bytes through `00B8F450` and constructs it
     from the name with `00B8F5E0`. The load continues either way, so the
     `.mmod` is a backdrop, not the layout.
4. `00AA5924`: allocates `124h` bytes through `00AC51A0` and calls
   `00AC6600(name, model_or_placeholder, screen_flag)`, the `GuiScreen`
   constructor. `screen_flag` lands at screen `+120h`.
5. `00AA5957`: `00AA52A0(manager, screen)` registers the page.
6. Returns the screen in `EAX`.

No `interface/*.mmod` file is installed, so on this installation branch 3 always
takes the placeholder path. The model branch is reachable but unexercised by the
shipped data.

### Where the file comes from

`00AC6600` is the screen constructor and is read-only for this packet, but its
script half is the answer to "which file":

- `00AC6748`: `00AC5600` called with the string `_Common` (`00D5CB64`).
- `00AC6792`: `00AC5770`, which calls `00AC5600` again with the page's own name.
- `00AC67FF`: the literal `GuiScreen` (`00D5CB20`) is pushed into a call through
  the object at `EBP+4h`, which is the interpreter handle; `00B67980` and
  `00B6D890` in the same tail live in segment 87, whose string keywords are
  `dofile`, `dobuffer`, `userdata`, `lightuserdata`, `thread`. That is Lua.

`00AC5600` composes the path from two literals it memcpys into frame strings:
`interface/` (`00D5CB18`, 10 bytes, `00AC5684`) and `.lua` (`00CFD2C8`, 4 bytes,
`00AC5653`). So a page name resolves to **`interface/<name>.lua`**, and
`interface/_common.lua` is loaded first, every time, before the page.

`00AC5820` builds the same path with `.guiedbak` instead; it has no callers and
is the in-game editor's backup writer.

## The file format

Lua source. `interface/` holds 97 `.lua` files on the installed build; 96 of them
assign the global `GuiScreen` and one, `_common.lua`, defines the shared helper
functions (`CreateStandardMenu`, `GetMenuGoldColor`, `GetMISCols`, ...) that the
pages call. Two spellings of the page table are both in use, sometimes in the
same file:

```lua
GuiScreen = {}
    GuiScreen["Priority"] = -10000
    GuiScreen["bg_Group"] = {
        ["geOrder"] = 4,
        ["bottomFrame_Icon"] = {
            ["Pos"] = { -0.166667, 0.823611, -60.000000 },
            ["States"] = { [1] = { ["Texture"] = "fe/frame/frame_topbottom.tga" } },
        },
    }
```

The rules the loader imposes on that table are all in `00AAA710`:

- A **string key** whose suffix names a widget class becomes a **child widget**;
  the value is that widget's own table and the walk recurses into it.
- Every other string key is a **property** of the current widget.
- **Integer keys are skipped** by the base reader (`00AAAD20` tests the variant
  tag for 0 and jumps past anything else). `States`, `[1] = {...}` and the lane
  lists of `Pos`/`Size`/`Color` are all integer-keyed, so a derived class reads
  them, not the base.

### The type table

`00AA2490`, `__fastcall(ECX = the key string)`, `RET`. It calls
`004BCB80(00CE7890 /* "_" */, 7FFFFFFFh)` for the index of the **last**
underscore and takes the substring after it, or the whole key when there is
none, then runs a chain of `__stricmp` comparisons. The size column is the ECX
immediate each `00AA6560` case target loads before its allocator call, read from
the bytes at stub `+19h`.

| Id | Suffix | Constructor stub | Instance size |
| --- | --- | --- | --- |
| 1 | (none; the page root) | `00AA3BD0` | `124h` |
| 2 | `Group` | `00AA12F0` | `0ECh` |
| 3 | `Text` | `00AA1380` | `1F4h` |
| 4 | `Line` | `00AA1530` | `104h` |
| 5 | `Progbar` | `00AA14A0` | `140h` |
| 6 | `Icon` | `00AA1410` | `138h` |
| 7 | `Scrollbar` | `00AA15C0` | `0FCh` |
| 8 | `Grid` | `00AA1650` | `124h` |
| 9 | `Model` | `00AA16E0` | `11Ch` |
| 10 | `Movie` | `00AA1770` | `130h` |
| 11 | `Listbox` | factory singleton `00F8BC10` | not a fixed block |
| 12 | `AnimIcon` | `00AA2340` | `144h` |
| 13 | `Curve` | `00AA1800` | `140h` |
| 14 | `Sound` | `00AA1890` | `108h` |
| 15 | `Button` | `00AA4440` | `0FCh` |
| 16 | `ClipBox` | `00AA1920` | `10Ch` |
| 17 | `Section` | `00AA19B0` | `12Ch` |
| 18 | `FrameBox` | `00AA1A40` | `11Ch` |

`00AA6560`, `__fastcall(ECX = type id, EDX = an object)`, is the switch that
turns an id into an instance. Every case but 11 is a two-instruction stub of the
same shape (`MOV ECX,<size>` then a call to a per-class allocate-and-construct
helper). Case 11 takes an exception path: `00A9B7E0` fetches the factory
singleton at `00F8BC10` and the instance comes from `00A9DF40` when `EDX` is
zero, or `00AA5B40(EDX)` when it is not. `00AAA710` always passes `EDX = 0`
(`XOR EDX,EDX` at `00AAAD85`), so the layout loader only ever takes the first.
Ids 1 and 11 are the only two the size column cannot describe.

The stub addresses are the case targets, not the class constructors; the
constructors are the helpers they call and were not read.

### The property descriptors

`00AAA710`, `__thiscall(ECX = widget, visitor)`, `RET 4`. For each property it
builds three 8-byte variants on the stack (`SUB ESP,8`; `MOV [ESP],tag`;
`MOV [ESP+4],pointer`) and calls the visitor's **vtable `+0Ch`** with, in stack
order, the name, the destination field and the default. The tag is the first
dword of each variant.

| Property | Tag | Widget offset | Name literal | Default |
| --- | --- | --- | --- | --- |
| `Pos` | 5 (vec3) | `+0Ch` | `00CF168C` | (0,0,0) |
| `Size` | 6 (vec2) | `+20h` | `00CFF278` | (0,0) |
| `Pivot` | 6 | `+18h` | `00D5C228` | (0,0) |
| `Scale` | 6 | `+28h` | `00CE60AC` | (1,1) |
| `Rotate` | 2 (float) | `+48h` | `00D5C220` | 0 |
| `Color` | 8 (vec4) | `+50h` | `00CE93F8` | (1,1,1,1), `00F8BCE0` |
| `LowColor` | 8 | `+A4h` | `00D5C1F4` | (0,0,0,1), `00F8BCCC` |
| `HighColor` | 8 | `+B4h` | `00D5C1EC` | (1,1,1,1) |
| `BlendFactor` | 2 | `+C4h` | `00D5C1FC` | 0 |
| `WideScreenAlign` | 0 (string) | `+E0h` | `00CE92E4` | absent |
| `Visible` | 3 (bool) | `+E4h` | `00D5C1E4` | false |
| `MouseBlock` | 3 | `+84h` | `00D5C1D8` | false |
| `MouseHit` | 3 | `+78h` | `00D5C1CC` | false |

Tags 1, 4 and 7 are unused by the base widget. Three details are not a plain
field write:

- **WideScreenAlign** is read as a string and folded to an integer:
  `00AAAB27` compares `Left` (`00CE92E4`) and yields 1; `00AAAB48` compares
  `Right` (`00CE92DC`) through `00425850` and yields 2; anything else, an absent
  key included, leaves 0. The decompiler drops `00425850`'s stack argument and
  renders the second test as an argument-less predicate, which reads as a
  wide-screen query; the listing shows the literal being pushed.
- The authored X is cached to `+8h` and the wide-screen shift is applied to
  `+0Ch` immediately (`00AAABA6`-`00AAABD0`), using `00D5C118` and the byte at
  `0109CF04+0Dh`, the same arithmetic `00AA8710` performs later.
- **Visible** is read only when the widget's vtable `+5Ch` (the type id) is not
  1, so the page root never gets one; and a **true MouseBlock forces MouseHit to
  1 without a lookup** (`00AAAC84`), while a false one falls through to the
  MouseHit read.

After the properties, `00AA7220` (`BSP_GuiWidget_RecomposeLocalTransform`) runs.

### The child loop

The tail of `00AAA710` clears a 500-entry array of variants to `FFFFFFFFh` and
calls the visitor's **vtable `+18h`** to fill it with the current table's keys,
count returned at `[ESP+804h]`. For each entry whose tag is 0:

1. the C string is copied into a native string (`00AAAD43`, `0041DD40` + memcpy);
2. `00AA2490` maps it to a type id; a zero id skips the entry;
3. `00AA6560(id, 0)` constructs the widget;
4. `00B74EB0(184h)` allocates a scene node and `00B75030`
   (`BSP_GeneratedModel_Construct`) builds it from the same name; it is stored at
   **widget `+4Ch`** and the low two bits of node `+138h` are cleared;
5. if the child already had a parent (`+70h`) it is detached (`00AA83A0`);
6. the child is spliced into the parent's `std::list` at `+68h`, its `+70h` is
   set to the parent, and `00B6E680` (`BSP_Node_SetParent_Provisional`) attaches
   the scene nodes;
7. the child's vtable `+74h` runs, the visitor **descends** (vtable `+4h`, taking
   the key variant by value), the child's own vtable `+18h` runs, the visitor
   **ascends** (vtable `+8h`, no arguments), the child's vtable `+78h` runs.

Step 7 is the recursion: a derived class's `+18h` reaches `00AAA710` again for
the base fields and adds its own. Widget vtable `+18h` is therefore the
serialisation entry point of the whole hierarchy.

## Registration with the manager

`00AA52A0`, `__thiscall(ECX = manager, GuiScreen *page)`, `RET 4`. After
`00AA30C0(page)` it walks the `std::vector<GuiScreen*>` at manager `+14h`
(`begin +18h`, `end +1Ch`, `capacity +20h`) and inserts the page **before the
first entry whose `+FCh` is strictly greater** than the new page's. `+FCh` is the
`Priority` key: `fe_loading.lua` sets `-10000`, `gui_plane_effects.lua` sets 60.
So the single page list is kept sorted by Priority ascending and equal
priorities keep insertion order. That vector is the only page list found; there
is no separate layer container.

`00AA3140`, `__thiscall(ECX = manager, const NativeString *name)`, `RET 4`, is
the matching lookup: a linear scan of the same vector calling each page's vtable
`+7Ch` for its name and comparing with `__stricmp`.

## The named-widget search

`00AA7E00`, `__thiscall(ECX = widget, const NativeString *name, int unused)`,
`RET 8`, body `00AA7E00`-`00AA7EAE`. It walks the widget's **direct children
only** (`+68h`, sentinel `+64h`), skips any child whose scene node pointer
(`+4Ch`) is null, reads the node's name through `00B6D800`, compares the length
first and then `__stricmp`, and returns the child. Zero when nothing matches.

Two corrections to existing docs follow from the listing:

- `docs/APP_INIT_FONTS_GUI.md` reads `00AA7E00` as a factory that "always takes a
  group and creates a child of it". It creates nothing; it **finds** an existing
  child. The names that doc lists as created (`MousePtrFE_Icon`,
  `MousePtrGUI_Icon`, `hl_FrameBox`, `hlCircle_FrameBox`, `safezone_43_FrameBox`,
  `safezone_169_FrameBox`) are all present as keys in the installed
  `interface/_mouse.lua` and `interface/_highlight.lua`, which is where they come
  from.
- `docs/LOADING_SCREEN_ELEMENTS.md` calls the trailing `1` "`00AA7E00`'s
  recursive flag". The argument exists in the ABI (`RET 8` pops it) but the body
  never reads it, and the walk is one level deep. Screens that reach a nested
  widget must chain lookups.

## What `00AA5A00` and `00AAA480` actually are

Both were proposed as the reader half of the descriptor table; neither is.

- `00AA5A00`, `__fastcall(out_vector, widget)`: copies the payload of each node
  of the widget's child list at `+68h` into a `std::vector`, then calls
  `00AA4E00` with the element count. It is a sorted snapshot of the children,
  called only from `00AAAED0`.
- `00AAA480` is the MSVC `std::sort` introsort loop: a median-of-three partition
  (`00AA8FB0`), an insertion sort below 32 elements (`00AA7F70`), a heapsort
  fallback (`00AAA260`) and a depth counter halved as `d/2 + (d/2)/2`. It is a
  library template instantiation, not a GUI routine, and should not be given a
  `BSP_Gui*` name.

`00AAAED0` (`BSP_GuiWidget_DescribeProperties`) stages the same thirteen
descriptors as `00AAA710` but calls `00BD5680` first and only reaches the
visitor's `+0Ch` when that returns false, and it sorts the children before
walking them. `00AAA710` is the **load** direction and `00AAAED0` the **save**
direction; `00AC5820`'s `.guiedbak` path is what saves.

## Validation against the installed game

Read-only, against `I:/SteamLibrary/steamapps/common/Battlestations Pacific`.
The reconstructed key rule and child walk were run over every page, first by a
Python reference (`local/gui_page_scan.py`, not committed) and then by the C++
implementation itself (`local/gui_page_probe.cpp`, not committed). Both produce
identical numbers.

| Measure | Value |
| --- | --- |
| `interface/*.lua` files | 97 |
| pages assigning `GuiScreen` | 96 |
| accepted by the static subset parser | 69 |
| rejected (helper calls, arithmetic, concatenation) | 27 |
| widgets built from the accepted pages | 1062 |
| deepest child nesting | 5 |

Widgets by type, from the accepted pages: Icon 555, Text 248, FrameBox 71,
Group 140, Section 19, Listbox 13, ClipBox 10, Model 4, Movie 2. Eight of the
eighteen types (Line, Progbar, Scrollbar, Grid, AnimIcon, Curve, Sound, Button)
appear in no accepted page; `AnimIcon` does appear once in a page that needs
evaluation.

The 27 rejections are the pages that call into `_common.lua` (`CreateStandardMenu`,
`CreateStandardMenu2`, `GetMISCols`), use arithmetic (`205/255`), or concatenate.
They are correctly-formed Lua that a real interpreter handles; the subset parser
is not one. `_debugtexts.lua` is a separate case: it has a stray comma and a
statement-level `["Template_Text"] = {...}`, which is malformed Lua, so the
shipped file cannot load in the game either.

The 48 distinct property keys seen across the accepted pages are, by frequency:
`geOrder`, `Pos`, `Pivot`, `Size`, `States`, `Font`, `Align`, `VerticalAlign`,
`DefaultShadow`, `DefaultText`, `Color`, `Visible`, `Multiline`, `MouseHit`,
`Rotate`, `WideScreenAlign`, `FrameSizesY`, `FrameSizesX`, `ShaderName`,
`MISColors`, `Priority`, `Texture`, then a long tail. Only nine of those are
base-widget descriptors; the rest belong to classes this packet did not read.

## Callers and callees

- `00AA5840`: 67 callers, every screen's register or enter slot among them
  (`0067CA80` for `FE_initial`, `0057C560` for `FE_loading`,
  `BSP_GuiManager_LoadResources` `00AA5E20` for `_Mouse` and `_Highlight`).
  Callees `00AA3140`, `0041E870`, `004261A0`, `00BDF4C0`, `004C1400`, `00B80D70`,
  `00B8F450`, `00B8F5E0`, `00AC51A0`, `00AC6600`, `00AA52A0`, `00419CC0`,
  `00BD1510`.
- `00AA7E00`: 167 callers, all screen code. Callees `00B6D800`, `__stricmp`.
- `00AAA710`: 13 callers, one per derived widget class (`00A9E400`, `00AABD70`,
  `00AB3310`, `00ABB630`, `00ABE1D0`, `00AC0280`, `00AC2DE0`, `00AC4C50`,
  `00AC85A0`, `00ACCE20`, `00ACDDB0`, `00ACE650`, and one more). Callees include
  `00AA2490`, `00AA6560`, `00B74EB0`, `00B75030`, `00B6E680`, `00AA83A0`,
  `00AA7220`, `00425850`.
- `00AA2490`: one caller, `00AAA710`. Callees `004BCB80`, `__stricmp`.
- `00AA6560`: callers `00AA6640`, `00AAA710`, `00AAB4C0`.
- `00AC5600`: callers `00AC5770`, `00AC6600`.

## Uncertainties

- The **second argument of `00AA5840`** (1 from most screens, 0 from the loading
  screen) reaches screen `+120h` and nothing read here consumes it. It is
  carried through the reconstruction, not interpreted.
- The **`.mmod` branch is unexercised** on this installation: no
  `interface/*.mmod` exists and the name is looked up with no directory. Whether
  a mounted package supplies one was not checked, and what manager `+30h` does
  with the instance was not followed.
- **`00AC6600` was read, not reconstructed.** The interpreter handle at `EBP+4h`,
  the exact Lua API used and what happens when a script raises an error are all
  unrecovered; the pseudocode for that function is heavily corrupted
  (`unaff_EBX`, `unaff_EBP`) and only its listing is trustworthy.
- The **variant type tags** 5, 6, 8, 2, 3, 0 are named from the field widths and
  the defaults, not from a table of tag meanings. Tags 1, 4 and 7 were not seen.
- The **key order** the visitor's `+18h` produces is a Lua table's, which has no
  defined order. The reconstruction preserves the script's textual order, which
  is a choice, not a recovered fact, and it affects sibling order in the tree.
- Six shipped keys spell the property `visible` in lower case. A Lua table
  lookup is case-sensitive, so those are probably dead; this was inferred from
  the language, not from reading the visitor's lookup.
- `00AA30C0`, called first by `00AA52A0`, was not read; "retain" is a guess from
  position.

## What remains

- The visitor class itself: which object implements vtable `+4h`, `+8h`, `+0Ch`
  and `+18h`, and how a Lua value of the wrong kind is coerced or rejected.
- The per-class property readers, which is where `States`, `Texture`, `Font`,
  `Align`, `geOrder` and `FrameSizesX/Y` are consumed. `geOrder` appears on
  almost every widget and is not a base descriptor.
- Widget vtable `+74h` and `+78h`, the hooks either side of the descend.
- The Listbox factory at `00F8BC10` and `00AA5B40`'s non-zero path.

## Follow-up packets proposed

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `gui_script_visitor` | 00AC6600, 00B67980, 00B6D890, 00441210 | docs/GUI_SCRIPT_VISITOR.md, include/bsp/gui_script_visitor.hpp | The Lua reader behind vtable +4h/+8h/+0Ch/+18h: how a table key becomes a typed value, what the 500-entry key array is, and what an erroring script leaves behind. |
| `gui_icon_properties` | 00AB2B70, 00AB3310, 00ACCE20 | docs/GUI_ICON_PROPERTIES.md | The Icon class (id 6, 138h bytes): `States`, `Texture`, `UV_LURB`, `ShaderName` and how a state index selects a texture. |
| `gui_text_properties` | 00AC2090, 00AC2DE0 | docs/GUI_TEXT_PROPERTIES.md | The Text class (id 3, 1F4h bytes): `Font`, `Align`, `VerticalAlign`, `Multiline`, `DefaultText`, `DefaultShadow`, and the tie to the font layout packet. |
| `gui_render_order` | 00AA5A00, 00AA4E00, 00AAB4C0 | docs/GUI_RENDER_ORDER.md | `geOrder`/`RenderOrder`: where the sorted child snapshot is consumed and how it reaches the draw queue. |

## State reached

| Address | State |
| --- | --- |
| 00AA5840 | analyzed, reconstructed, build-tested, installed-file-checked |
| 00AA2490 | analyzed, reconstructed, build-tested, installed-file-checked |
| 00AAA710 | analyzed, reconstructed (both halves), build-tested, installed-file-checked |
| 00AA7E00 | analyzed, reconstructed, build-tested |
| 00AA6560 | analyzed (the id-to-stub table and its sizes), reconstructed as data |
| 00AA52A0, 00AA3140 | analyzed, reconstructed, build-tested |
| 00AC5600 | analyzed, reconstructed (path composition only) |
| 00AA5A00, 00AAA480 | analyzed; identified as a child snapshot and as std::sort |
| 00AC6600 | exported and read; not reconstructed |

Nothing here is ABI-compatible or game-validated. "Installed-file-checked" means
the recovered rules were run over the shipped `interface/*.lua` and produced a
consistent tree, not that the game was observed doing so.

## Corrections from docs/GUI_TEXT_WIDGET.md and docs/GUI_ICON_WIDGET.md

- `00ac2090` / `00ac2de0` are not the Text class: they are the describe/read pair of a Curve class whose only property is an integer-keyed `Points` array. The Text class (id 3) allocates at `00ab79e0` (tagged `cg_static_dtor_stub` in the inventory, wrongly: it allocates), constructs at `00ab9650` with vtable `00d5c6c8`; virtual +18h is the reader `00abb630` and +1Ch the describer `00ab7b60`. It is the object the font packets call the text context. The describer's `ShadowPos` compare at `00ab8028` is dead, so a `Front` widget always describes as `Behind`.
- `00acce20` is the Model widget's property reader (type id 9: `ModelName`, `ModelTextureOverride`, `ScaleVector`, `RotationEuler`), not part of the Icon class.

## Corrections from docs/GUI_LUA_READER.md

- The `GuiScreen` literal is at `00d5cb24`, not `00d5cb20`; `00b6d890` is a scene-node reparent, not a Lua call; the 500-entry array is 500 dwords (250 eight-byte key pairs with the count at +7D0h, caching live Lua string pointers, no bounds check).
- Every page construction creates a private Lua 5.1.1 state with base, table, string and math only (mask 65h through the `{name, opener}` table at `00d62bb8`), registers no C functions, loads with `luaL_loadbuffer` without testing the result and runs the chunk with `lua_call`, so a malformed page reaches the panic handler `00b669c0` and Lua exits the process; a missing script file is silent.

## Corrections from docs/GUI_RENDER_ORDER.md

- The sorted child snapshot `00aa5a00` is not on the draw path: its comparator (`00aa2c80`, twin `00aa7f70`) orders children alphabetically by name through widget vtable +28h with `_stricmp`, empty names first, rebuilt from scratch on every call, and its only consumer is the reflection walk `00aaaed0`.
- `geOrder` is never read by the runtime (no such literal in the image); it is authoring metadata. Drawing is ordered by the float `RenderOrder` on a `cGuiLayer` (read by `00ac4c50` into layer+11Ch), keying a `std::multimap<float, cGuiCameraStore*>` at manager+8h; equal keys keep layer creation order, layers differing only in `Priority` share a pass, and the visible-layer count at store+20h is the only enable check. `BSP_Game_Render` reaches `00aa45a0`, which walks the map ascending and, per store, builds a half-pixel-offset look-at with an orthographic volume where +Y runs down the screen and submits one pass to the render command queue.
