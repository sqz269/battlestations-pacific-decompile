# cc15-ob5: GUI A, the native GUI manager and widget tree (design note, 2026-10-19)

Lane L5 of the cc15 object bring-up (rank 2 in `docs/CC15_WIRE_FRONTIER.md`). This is the design
note the brief asks for before any code. **Result: the bring-up stops here.** A faithful build of the
widget tree, which is what the nine rows need, depends on renderer-side objects that do not exist in
`bsp_game`. It would also give the pages a second owner beside the sprite bridge. What is missing
is listed under "Stop".

## The object

| Item | Image | Evidence |
|---|---|---|
| Accessor | 004C12B0, a double-checked singleton under the lifetime manager's section | `get_native_gui_manager_004c12b0` (src/native_platform_focus_owners.cpp), complete raw |
| Allocation | 88h, from the singleton lifetime pool | same body, `bytes = 0x88` |
| Constructor | 00AA5D70, vtable 00D5BFCC | `construct_native_gui_manager_00aa5d70`, complete raw (169 bytes) |
| Slot | [00F8BC5C]; then registered with the 01090AA0 manager through 00BD0C30 | same body |
| Built | First call at startup, 0073BAE0 Phase 7 (`GuiStartupHost::gui_manager`); 00AA5E20 then loads ten resources into it | `GuiResourceOwner::initialize_00aa5e20` (src/gui_resources.cpp) |
| Destroyed | Singleton drain: D5BFCC → 00AA6540 → 00AA6340 (provisional name) | `scalar_delete_native_gui_manager_00aa6540_fragment`; deletion binding `NativeSingletonDeletionBindings::native_gui` (src/native_singleton_destruction.cpp:110) |

What the constructor writes: +0 vtable; +0Ch a 18h tree head (00AA2920; byte +15h = 1, self-linked);
+10h..+30h zero (page vector at +14h, resource slots +24h..+2Ch); +38h a 14h list sentinel (00AA2820);
+3Ch/+40h zero; byte +48h zero (pointer state); +5Ch/+60h = 0.5f from 00CE3800 (cursor); +6Ch zero.
00AA5E20 later fills +28h (whiteGui.tga), +50h (pointer page), +7Ch/+80h (safe-zone frame boxes) and
the rest of its ten-entry list.

## What `bsp_game` has today

- `game_hosts_frontend.cpp` keeps `Impl::ManagerState`: an 88h array of dword slots that 00AA5E20
  writes. No vtable, tree, list or cursor seed, and it has no singleton registration. 004C12B0 is
  modelled as a `constructed` flag (`gui_manager_get_or_create_004c12b0`, line 477).
- Pages and widgets are projections: `GuiLayoutPage` and `GuiLayoutWidget` (include/bsp/gui_layout_loader.hpp).
  The bridge evaluates them from the Lua page scripts and draws them. A widget's +4Ch scene node is a
  counter (`next_node`). The resource slots hold bridge handles: `IDirect3DTexture9*` and `GuiLayoutPage*`.
- The singleton domain exists: `GameSingletonHost` owns the 01090AA0 cell and the deletion bindings.
  The native renderer application is constructed at startup (game_hosts.cpp:2292).
- The typed native widget runtime (`GuiWidgetOwnerRuntime`, src/gui_widget_owner.cpp) and the native
  Text application (`GameNativeTextApplication`, src/game_hosts_text.cpp) are compiled but never
  instantiated in `bsp_game`.

## The nine rows, and what each one needs

The USN04 call counts are from `cc9-physics1/local/p24r_on_u04.log` (2026-10-19 01:28) and
`tools/wire_readiness.py --frontier`.

| Row | Host method | USN04 calls | Reads or writes | Needs |
|---|---|---|---|---|
| 00AA6740 widget size (`LEA EAX,[ECX+20h]`) | MissionDetail::text_clip_height (already served from the projection), HudMarkers::widget_size | 1 | widget +20h/+24h | a native widget, or the projection's size |
| 00AA6BC0 listener fields | MissionDetail::finish_list_box | 1 | widget +DCh listener, byte +79h | a native widget. Only button click dispatch reads it, and that needs the input manager (no input backend: CC15_WIRE_FRONTIER.md line 46) |
| 00AB6BD0 text height | MissionDetail::briefing_text_height | 1 | GuiText context +FCh/+178h, font +14h | a native GuiText context (00AB9650) whose layout has run |
| 00AA31F0 remove and destroy page | GuiManagerResources::release_page, MoviePlayer::release_page | 2 | manager +14h vector; page virtual +20h, then deleting +04h | native 124h pages. The raw body rejects projections: "never accepts GuiLayoutPage" (include/bsp/native_gui_page_registry.hpp) |
| 00AA6720 bind scene node | GuiLayoutHost::bind_page_root_scene_node | 58 | widget +4Ch; node +138h flag word | a real node. The page root is the 18Ch cGroup (00B8F5E0) |
| 00AD4C30 font load | (no host stand-in; 00AC3910's callee) | 1 | font object | an image-shaped font in place of `GameFontHost`'s typed one |
| 008CA010 IsGUIActive | MissionLuaNative::IsGUIActive (game_hosts_lua.cpp) | 658 | 008C8C20 page-by-name over the manager, child walk, widget virtual +38h | native pages and widgets with vtables |
| 00AA7DC0 set local position (cc9) | HudMinimap/HudMarkers | 317912 | widget +0Ch..+14h, then 00AA7220/00AA70E0 | listed for cc9, not wired here |
| 0043ABA0 digit UV roll (cc9) | HudShipScreen::gauge_digit_uv | 79476 | widget vertex stream (virtual +A0h, lock) | listed for cc9, not wired here |

Only two rows touch the 88h manager itself: 00AA31F0 (its +14h vector) and 008CA010 (page lookup
through 004C12B0). Both need native pages in that vector. The other seven act on native widgets,
text contexts or fonts. **So building the manager on its own wires none of the rows.**

## Owner split (if the reduced plan below is taken)

- **The bridge is authoritative for everything drawn:** page evaluation, the projected widget tree,
  textures, glyph runs, visibility as drawn, and the pointer page.
- **The native 88h manager would be authoritative for manager state only:** the vtable, the
  tree/list sentinels, the pointer byte +48h, the cursor +5Ch/+60h and +6Ch. Its resource slots and
  page vector stay empty, because the bridge's handles are not image-shaped. The drain's 00AA6340
  then skips the +28h/+2Ch canonical releases (null slots) and runs no page virtuals (empty vector).
- One copy of each field. The bridge's `ManagerState.fields` would keep only the resource slots, and
  the state fields would move to the native block.

## Stop: what a faithful bring-up is missing

1. **Native pages (124h, 00AC6600) and the 18 widget classes.** The typed runtime exists, but
   `bsp_game` never instantiates it. Building it would put each page under two owners, which the
   lane constraints forbid: the bridge's projection draws it, and the native tree holds its state.
   The alternative is to replace the bridge's projection with the native tree, which is render
   milestone work.
2. **Per-widget scene nodes.** 00AA6640 takes a 188h slot from the model pool at 01090054
   (00B74EB0) and constructs the 184h generated model (00B75030 over 00B6F5A0). The page root is a
   cGroup (00B8F5E0). The node constructors are raw-complete and need no device, but the
   generated model's "full model layout/lifetime" is unported (ledger 00B75030). Nothing in
   `bsp_game` owns the model pool for GUI nodes or retires those nodes, so they belong to the
   renderer side.
3. **Native GuiText contexts.** 00AB6BD0 reads layout results. Producing those needs 00AB9650 plus
   00AB8400 glyph buffers, which are renderer vertex and index objects (`gui_text_buffers.cpp`, real HAL
   buffers). `GameNativeTextApplication` needs the native GUI effect application as well, and
   `bsp_game` constructs neither.
4. **Native resource loading.** 00AA5E20's texture entry goes through the renderer's
   *(00F8D394)+64h (00AA5E60, still UNIMPLEMENTED in the host). Its page entries need item 1.
5. **Input.** 00AA6BC0's listener only matters to button click dispatch, through the input manager.
   No input backend exists.
6. **An image-shaped font** for 00AD4C30, in place of the typed `GameFontHost` registry.

## Reduced plan, for the lead to approve or drop

These steps need none of the missing objects. Each would be behind `kGuiManagerCc15Bound`, with an
OFF pair and an ON pair.

- **Step 1, the object.** 004C12B0 through `get_native_gui_manager_004c12b0`. Its context would
  borrow the singleton host's 01090AA0 cell, a host-owned 00F8BC5C publication cell and the 00CE3800
  and 00D7A24C constants. The native renderer's actual owners would be borrowed only for the
  constructor's unwind. The `native_gui` deletion binding would get a page host that throws if it is
  ever reached, because the vector stays empty. The cursor, pointer-state and +6Ch accessors would
  read and write the native block. Expected ON change: none, except a new registration in the
  singleton drain at shutdown.
- **Step 2, projection-backed rows.** These are not faithful to the native widget layout. Each
  would be marked as a projection in the ledger:
  - 00AA6740 HudMarkers::widget_size from `transform.size`, as text_clip_height already does.
  - 00AB6BD0 from the bridge's `GameTextRun` height, only if its normalisation can be shown to
    equal +178h/720.
  - 008CA010 from a page-by-name lookup in the bridge registry, using the root's `visible` for
    virtual +38h. This row changes music selection (`commandhelpers.lua` 13270, `GUI_map`), so the
    ON pair would move.
  - 00AA31F0 stays a projection retire (the bridge keeps the page loaded).
  - 00AA6720, 00AA6BC0 and 00AD4C30 stay stand-ins.

Neither step makes the manager the image's widget tree. Step 1 changes no row on its own. In step 2,
only 008CA010 has behaviour a run can see.
