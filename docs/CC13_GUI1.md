# cc13-gui1: GUI-core blockers

Packet cc13-gui1 (2026-10-17). Code: `src/cc13_gui1.cpp`, `include/bsp/cc13_gui1.hpp`
(namespace `bsp::cc13_gui1`). Nothing is wired.

## GUI-core blockers

Why these functions: the wiring-readiness report (`local/cc_wire_readiness.py` in the main
checkout) counts a function as reconstructed only when it has a `function` ledger record. Each
function below had only `fragment` records (logical models such as `GuiWidgetOwner`,
`GuiLayoutPage` or `GuiListboxRuntime`) or none, so every reconstructed caller was
`blocked_on_callees`. The routines here work on the actual native objects (raw addresses, Win32
layout) and reach every callee through a named host hook at the binary's call point, with the
binary's arguments. The existing fragment models are left as they are.

Status of every routine: reconstructed and build-tested. Not fixture-tested, not ABI-compatible
(new C++ signatures with a host argument), not game-validated. Names are hypotheses.

Shared simplification: the SEH frames of 00AA5840, 00AC6600, 00AA6640, 00AD3A80, 00ABAED0 and 00ABB000 only
release temporaries or the new block when a callee throws. The hooks are not expected to throw and
no unwind path is reproduced. No routine here draws from the shared RNG 00BD2F10.

| Address | Function | Original ABI | What the code does |
|---|---|---|---|
| 00AA5840 | `gui_manager_load_page_00aa5840` | thiscall(manager; name, screen_flag, add_reference) RET 0Ch | Calls 00AA3140. If the page exists, InterlockedIncrement(page+4) through [00CE221C] when add_reference is set, and returns it. Otherwise builds `<name>.mmod` (0041E870 and 004261A0, releasing the suffix temp first), asks the VFS singleton [0109CEEC] whether the path exists. On a hit 00AA5840 calls 004C1400, 00B80D70 and factory vtable+8h(0, 1.0f). The model goes to manager+30h and the scene root is model+0Ch. On a miss it calls 00B8F450(188h) and 00B8F5E0(name). Then 00AC51A0(124h), 00AC6600(name, root, screen_flag), 00AA52A0(page) (also called with a null page), releases the path and returns the page. |
| 00AC6600 | `gui_layer_construct_from_script_00ac6600` | thiscall(page; name, node, char share_scene) RET 0Ch | The page constructor. Calls 00AA9390(page, 1), then writes vtable 00D5BE38 and zeroes +0ECh..+0FCh. Copies the name to +100h. Sets +10Ch/+110h/+114h from the live dwords 00D7A2F0 (0.1f), 00CE3804 (1000.0f) and 00D7A24C (1.0f); the 1.0f also goes to +20h/+24h. Stores byte +120h = share_scene and calls 00AA6720(node). On a 4C8h Lua owner on the stack it calls 00B66BD0 and 00B6A020(65h), runs "_Common" (00AC5600, then 00B69D40) and the page script (00AC5770, then 00B69D40), releasing each path. It takes the globals with 00B67980, builds a reader with 004425C0, and calls reader vtable+4h(0, "GuiScreen"), page vtable+18h(reader) and reader vtable+8h. Then it calls 00B6D890(page+4Ch, 0), runs the inline reader destructor (vtables 00CE44FC and 00CE374C around the vector tidy), and 00AC6600 then calls 00B669A0 and returns the page. |
| 00AAB4C0 | `gui_widget_clone_subtree_00aab4c0` | thiscall(widget; parent_clone) RET 4 | Calls 004C12B0 (result unused), then 00AA6560(type +60h, widget). Attaches the clone under parent_clone, or else under widget+70h when both the clone and that parent are non-null. Recurses over the +64h children, rereading the end node on each pass and keeping the checked-iterator 00BF6713 reports. Copies clone+0Ch..+14h onto themselves, calls 00AA7220 and 00AA70E0, and stores the saved +08h last. |
| 00AA8240 | `gui_widget_set_resolved_position_00aa8240` | thiscall(widget; const float[3]) RET 4 | With a parent: pos - 00AA6750(parent) + (p20*p18, p24*p1C, double 0.0 at 00D7A258) goes to +0Ch..+14h. Without one: bitwise copy. Then 00AA7220 and 00AA70E0. |
| 00AA67F0 | `gui_widget_get_local_position_00aa67f0` | thiscall RET | Returns widget+0Ch. |
| 00AA6BC0 | `gui_widget_set_listener_fields_00aa6bc0` | thiscall(widget; listener, char) RET 8 | +0DCh = listener, then byte +79h. |
| 00AAA5A0 | `gui_widget_attach_child_00aaa5a0` | thiscall(parent; child, where*) RET 8 | Returns at once for a null child. Detaches the child from its old parent (00AA83A0). Then either push_back (00A9B790, 00A9D480(1), then the two link stores) or 00A9D6C0 insert at *where. Sets child+70h = parent and calls 00B6E680(child+4Ch; parent+4Ch). |
| 00AA8B00 | `gui_timed_entry_get_or_create_00aa8b00` | thiscall(widget; index) RET 4 | Three signed count checks, each resizing through 00AA77B0(index+1). The slot address is captured before 00AD3A80 runs, and the data pointer is reloaded for the final read. |
| 00AA6640 | `gui_widget_create_with_scene_node_00aa6640` | fastcall(tag, node_name; ctor_arg) RET 4 | Calls 00AA6560, 00B74EB0(184h) and 00B75030(block, name) (the node is 0 for a null block), then 00AA6720(widget, node) and vtable+74h(). |
| 00AA6560 | `gui_layout_create_widget_of_type_00aa6560` | fastcall(tag, source) RET | Returns 0 when tag-1 > 11h. Otherwise one of 18 creators, read from the jump table at 00AA65F4: 3BD0, 12F0, 1380, 1530, 14A0, 1410, 15C0, 1650, 16E0, 1770, 64B0, 2340, 1800, 1890, 4440, 1920, 19B0, 1A40 (00AAxxxx, tags 1..18). |
| 00AD3A80 | `gui_timed_entry_create_by_index_00ad3a80` | fastcall(index, widget) RET | Indices 0 and 1: operator new(14h), then 00AC2ED0(widget). Index 2: inline constructor that writes +08h widget, +04h 0, vtable 00D5D210, +0Ch 0, +10h 0. Anything else returns 0. |
| 00ABAED0 | `gui_text_set_localised_source_00abaed0` | thiscall(text; source, char localise) RET 8 | Compares with the cache at +0F4h (00435C40) and returns with no call when equal. Otherwise resizes and memcpys the source in. Then either 00A9FAD0 on [00F8BC4C] into a zeroed wide temp, or 004C5E60 on the data (00F8BE4C when null). Then 00ABA8D0, releases the temp (len*2+2) and calls vtable+50h(text+50h). |
| 00ABB000 | `gui_text_set_ellipsized_source_00abb000` | thiscall(text; source, float width, char localise) RET 0Ch | Same cache step. A width equal to -1.0f (ordered, so NaN is kept) becomes text+20h. Calls 00AB8F00(&out, &in, width), then 00ABA8D0(result), releases out and then in, and calls vtable+50h. |
| 00AB6BD0 | `text_context_get_normalized_text_height_00ab6bd0` | thiscall RET, ST0 | Byte +0FCh ? float +178h / 720 : (font +108h ? int16 font+14h : 0) / 720, rounded to float. |
| 00AB6AB0 | `gui_text_submit_utf16_00ab6ab0` | thiscall(text; wide*) RET 4 | Calls 00ABA8D0, then vtable+50h(text+50h). |
| 00A9D750 | `gui_listbox_insert_row_00a9d750` | thiscall(listbox; row, where*, char after) RET 0Ch | Calls row vtable+34h(1). With no position: 00AAA5A0(row, 0) and push_back on +0FCh. With one: copies the iterator, steps it once when `after` is set, then 00AAA5A0(row, &copy) and 00A9D6C0. When the list size is 1, sets +108h/+10Ch to the first row (or the head when its byte +77h is set) and calls vtable+80h(0). Always ends with vtable+7Ch(). |
| 00A9CEB0 | `gui_listbox_set_field124_00a9ceb0` | thiscall(listbox; float, char) RET 8 | Stores +124h. With the flag, calls 00A9BB70(row, 1) for each row. |
| 00A9DA30 | `gui_listbox_insert_row_with_data_00a9da30` | thiscall(listbox; row, data, where, after) RET 10h | Sets row+0D8h = data, then calls 00A9D750. |
| 00A9C380 | `gui_list_control_set_item_visible_00a9c380` | thiscall(listbox; index, char visible) RET 8 | At the index-th row, byte +77h = visible ? 0 : 1. |

Uncertainties:
- 00A9D750 hands the row-list iterator (container +0FCh) to 00AAA5A0, which inserts into the widget
  child list at +64h. That is what the image does. Whether the two lists are kept parallel, so that
  the node is valid in both, was not established.
- 00A9CEB0's dead `TEST ESI,ESI` container checks are omitted. ESI is listbox+0FCh, which is never
  null.
- The vtable value written by 00AD3A80 is the image address 00D5D210. A binder for native objects
  must map it.

Skipped:
- 00A9D9D0 `BSP_PointerList_AssignRange`: std::list<T*>::assign(first, last). It clears the list
  (each node is freed by _free) and then 00A9D9D0 calls 00A9D560 InsertRange. That is an STL
  instantiation, out of scope by the 2026-10-16 ruling. It still blocks 00A9DA60, 00A9E130 and
  00A9E230.
- 00ABA8D0 `BSP_TextContext_UpdateUtf16Geometry`, called by the three text setters. Skipped as too
  large for this packet: about 228 instructions, plus its own unreconstructed callees 00AB9FD0 and
  00ABA270. Several fragments exist (`src/gui_text_runtime_content.cpp`, `src/gui_text_content.cpp`).
  It is now the one callee that blocks 00ABAED0, 00ABB000 and 00AB6AB0 from wiring.
