#pragma once
// cc13_gui1: GUI-core routines that block the most already-reconstructed callers
// (widget base of segment 77, the page loader of segment 76, the text setters of
// segment 78 and the listbox rows of segment 75). Packet cc13-gui1 (2026-10-17),
// docs/CC13_GUI1.md, section "GUI-core blockers".
//
// Every routine works on the actual native objects (raw addresses, Win32
// layout: pointers are 4 bytes) and reaches each callee through a named host
// hook invoked at the binary's call point with the binary's arguments. Each hook
// comment names the reconstruction a binder should route it to, where one
// exists. Names are hypotheses, not recovered symbols. Reconstructed and
// build-tested only; nothing is wired, fixture- or game-validated.
//
// SEH: the image's unwind states only release the NativeString temporaries when
// a callee throws. The hooks are not expected to throw; no unwind path is
// reproduced. That is the one simplification shared by every routine here.

#include <cstdint>

namespace bsp::cc13_gui1 {

// The 8-byte NativeString header (+0 length, +4 data), as on the native stack.
struct NativeStringHeader {
    std::uint32_t length;
    char* data;
};
// The 8-byte UTF-16 string header the text routines use (+0 length in code
// units, +4 data). Its pool block is length*2+2 bytes.
struct NativeWideStringHeader {
    std::uint32_t length;
    std::uint16_t* data;
};
static_assert(sizeof(void*) == 4, "cc13_gui1 reads native Win32 objects");
static_assert(sizeof(NativeStringHeader) == 8 && sizeof(NativeWideStringHeader) == 8);

// A VS2008 checked std::list iterator as the image passes it: +0 the container
// proxy (the list object's own address), +4 the node.
struct NativeListIterator {
    void* container;
    void* node;
};

// The release pair every native string temporary goes through.
struct NativePoolHooks {
    virtual ~NativePoolHooks() = default;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton, no arguments -> pool.
    virtual void* sized_storage_pool_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(pool; block, size,
    // 1), RET 0Ch (src/native_string.cpp has the concrete raw-pool path).
    virtual void pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size,
        std::uint32_t unused_one) = 0;
};

// The VS2008 STL instantiations the widget and listbox child lists use
// (vendor code: bound, not reconstructed).
struct NativeListHooks {
    virtual ~NativeListHooks() = default;
    // 00A9B790 BSP_GuiWidget_ListAllocateNode, __thiscall(list; next, prev,
    // const value*) RET 0Ch -> node.
    virtual void* list_buy_node_00a9b790(void* list, void* next, void* prev, void* const* value) = 0;
    // 00A9D480, __thiscall(list; count) RET 4: the length check and size += count.
    virtual void list_incsize_00a9d480(void* list, std::uint32_t count) = 0;
    // 00A9D6C0 STL_inst_00a9d6c0, list::insert(where, value): __thiscall(list;
    // iterator* result, iterator where (8 bytes by value), const value*) RET 10h.
    virtual void list_insert_00a9d6c0(void* list, NativeListIterator* result, NativeListIterator where,
        void* const* value) = 0;
    // 00BF6713, the CRT invalid-parameter report the checked iterators call.
    virtual void invalid_parameter_00bf6713() = 0;
};

// ---------------------------------------------------------------------------
// 00AA5840 BSP_GuiManager_LoadPage
// ---------------------------------------------------------------------------
struct GuiLoadPageHost : NativePoolHooks {
    // 00AA3140, __thiscall(manager; const NativeString* name) RET 4 -> page or 0
    // (src/gui_layout_loader.cpp GuiPageRegistry::find_00aa3140 is the logical model).
    virtual void* find_page_by_name_00aa3140(void* manager, const NativeStringHeader* name) = 0;
    // 0041E870 BSP_NativeString_Assign, __thiscall(out; const char*) RET 4, a
    // constructor body on the uninitialised stack header.
    virtual void native_string_assign_0041e870(NativeStringHeader* out, const char* text) = 0;
    // 004261A0 BSP_NativeString_Concat, __thiscall(left; out, const right*) RET 8.
    virtual void native_string_concat_004261a0(const NativeStringHeader* left, NativeStringHeader* out,
        const NativeStringHeader* right) = 0;
    // [0109CEEC] the VFS singleton; 00BDF4C0 BSP_VFS_ResolveExistingName,
    // __thiscall(vfs; NativeString*) RET 4 -> AL.
    virtual void* vfs_singleton_0109ceec() = 0;
    virtual bool vfs_resolve_existing_name_00bdf4c0(void* vfs, NativeStringHeader* path) = 0;
    // 004C1400 BSP_ResourceManager_GetSingleton; 00B80D70
    // BSP_NativeResourceManager_LoadDefaultFactory, __thiscall(manager;
    // NativeString*) RET 4 -> factory or 0.
    virtual void* resource_manager_004c1400() = 0;
    virtual void* load_default_factory_00b80d70(void* resource_manager, NativeStringHeader* path) = 0;
    // factory->vtable[+8h](0, 1.0f) -> model instance.
    virtual void* factory_instantiate_vslot8(void* factory, std::uint32_t zero, float one) = 0;
    // 00B8F450 BSP_GroupPool_AllocateStaticRawSlot (ECX = 188h) -> block;
    // 00B8F5E0 BSP_Group_Construct, __thiscall(block; const NativeString* name) RET 4.
    virtual void* group_pool_allocate_00b8f450(std::uint32_t size) = 0;
    virtual void* group_construct_00b8f5e0(void* block, const NativeStringHeader* name) = 0;
    // 00AC51A0 BSP_GuiLayer_AllocateInstance (ECX = 124h) -> block; 00AC6600
    // BSP_GuiLayer_ConstructFromScript, __thiscall(block; name, scene_root,
    // screen_flag) RET 0Ch -> page. Route to gui_layer_construct_from_script_00ac6600.
    virtual void* gui_layer_allocate_00ac51a0(std::uint32_t size) = 0;
    virtual void* gui_layer_construct_00ac6600(void* block, const NativeStringHeader* name,
        void* scene_root, std::int32_t screen_flag) = 0;
    // 00AA52A0, __thiscall(manager; page) RET 4. Route to
    // register_native_gui_page_00aa52a0 (include/bsp/native_gui_page_registry.hpp).
    virtual void register_page_by_priority_00aa52a0(void* manager, void* page) = 0;
    // [00CE221C] InterlockedIncrement.
    virtual void interlocked_increment_00ce221c(volatile long* value) = 0;
};

// ---------------------------------------------------------------------------
// 00AC6600 BSP_GuiLayer_ConstructFromScript (the page constructor)
// ---------------------------------------------------------------------------
struct GuiLayerConstructHost : NativePoolHooks {
    // 00AA9390 BSP_UIContext_Construct, __thiscall(page; 1) RET 4: base constructor.
    virtual void widget_base_construct_00aa9390(void* page, std::uint32_t type) = 0;
    virtual void native_string_resize_0041dd40(NativeStringHeader* self, std::uint32_t length,
        std::uint32_t preserve) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t size) = 0;
    // The live dwords at 00D7A2F0 (0.1f, cd cc cc 3d), 00CE3804 (1000.0f,
    // 00 00 7a 44) and 00D7A24C (1.0f, 00 00 80 3f), read at the binary's points.
    virtual std::uint32_t image_dword(std::uint32_t address) = 0;
    // 00AA6720, __thiscall(widget; node) RET 4 (bind_native_gui_widget_node_00aa6720).
    virtual void set_scene_node_00aa6720(void* widget, void* node) = 0;
    // The 4C8h Lua state owner on the stack: 00B66BD0 constructor, 00B6A020
    // open(65h) RET 4, 00B69D40 run(const NativeString* path, 0) RET 8 (result
    // unused), 00B67980 globals(out 14h object) RET 4, 00B669A0 close/destructor.
    virtual void lua_owner_construct_00b66bd0(void* owner) = 0;
    virtual void lua_owner_open_00b6a020(void* owner, std::uint32_t libraries) = 0;
    virtual void lua_owner_run_script_00b69d40(void* owner, const NativeStringHeader* path,
        std::uint32_t zero) = 0;
    virtual void lua_owner_get_globals_00b67980(void* owner, void* out_object) = 0;
    virtual void lua_owner_close_00b669a0(void* owner) = 0;
    virtual void native_string_assign_0041e870(NativeStringHeader* out, const char* text) = 0;
    // 00AC5600, __thiscall(out; NativeString by value, consumed) RET 8 -> out:
    // "interface/<name>.lua" style path of a shared script.
    virtual NativeStringHeader* compose_screen_script_path_00ac5600(NativeStringHeader* out,
        NativeStringHeader by_value_name) = 0;
    // 00AC5770, __thiscall(page; out) RET 4 -> out: the page's own script path.
    virtual NativeStringHeader* compose_layer_script_path_00ac5770(void* page, NativeStringHeader* out) = 0;
    // 004425C0, __thiscall(reader 14h; 14h Lua object by value, consumed) RET 14h.
    virtual void lua_reader_construct_004425c0(void* reader, void* by_value_object) = 0;
    // reader->vtable[+4h](0, "GuiScreen"), page->vtable[+18h](reader),
    // reader->vtable[+8h]().
    virtual void reader_vslot4(void* reader, std::uint32_t zero, const char* field) = 0;
    virtual void page_vslot18(void* page, void* reader) = 0;
    virtual void reader_vslot8(void* reader) = 0;
    // 00B6D890 BSP_Node_PropagateRootRegistration, __thiscall(node; 0) RET 4.
    virtual void node_propagate_root_00b6d890(void* node, std::uint32_t zero) = 0;
    // 00441210 BSP_LuaObjectVector_Tidy, ECX = the reader's vector at +4.
    virtual void lua_object_vector_tidy_00441210(void* vector) = 0;
};

// 00AC6600, __thiscall(ECX = 124h page block; const NativeString* name, scene
// node, char share_scene) RET 0Ch -> page.
void* gui_layer_construct_from_script_00ac6600(GuiLayerConstructHost& host, void* page,
    const NativeStringHeader* name, void* scene_node, std::uint8_t share_scene);

// 00AA5840, __thiscall(ECX = manager; const NativeString* name, int
// screen_flag, char add_reference), RET 0Ch -> page.
void* gui_manager_load_page_00aa5840(GuiLoadPageHost& host, void* manager,
    const NativeStringHeader* name, std::int32_t screen_flag, std::uint8_t add_reference);

// ---------------------------------------------------------------------------
// Widget base, segments 76/77
// ---------------------------------------------------------------------------
struct GuiWidgetHooks : NativeListHooks {
    // 004C12B0 BSP_GuiManager_GetOrCreate (result unused by every routine here;
    // get_native_gui_manager_004c12b0 in src/native_platform_focus_owners.cpp).
    virtual void* gui_manager_get_or_create_004c12b0() = 0;
    // One of the 18 per-type creators 00AA6560 tail-jumps to, __fastcall(ECX =
    // source) -> widget; `creator` is its image address (00AA3BD0, 00AA12F0, ...;
    // the table is in gui_layout_create_widget_of_type_00aa6560).
    virtual void* widget_type_creator(std::uint32_t creator, void* source) = 0;
    // 00AA7220 / 00AA70E0, ECX = widget. Route to
    // recompose_native_gui_widget_transform_00aa7220 and
    // refresh_native_gui_widget_local_bounds_00aa70e0.
    virtual void recompose_local_transform_00aa7220(void* widget) = 0;
    virtual void refresh_local_bounds_00aa70e0(void* widget) = 0;
    // 00AA6750 BSP_GuiWidget_GetResolvedPosition, __thiscall(widget; float out[3])
    // RET 4 -> out (src/gui_widget.cpp).
    virtual float* get_resolved_position_00aa6750(void* widget, float* out) = 0;
    // 00AA83A0, __thiscall(parent; child) RET 4. Route to
    // detach_native_gui_widget_child_00aa83a0.
    virtual void detach_child_00aa83a0(void* parent, void* child) = 0;
    // 00B6E680, __thiscall(child node; parent node) RET 4: scene-node attach.
    virtual void scene_node_attach_00b6e680(void* child_node, void* parent_node) = 0;
    // 00B74EB0 (ECX = 184h) -> block; 00B75030 BSP_GeneratedModel_Construct,
    // __thiscall(block; const NativeString* name) RET 4 -> node (it reads the
    // length at [name] and the data at [name+4]; 00AA6640 and 00AAA710 both
    // pass a NativeString); 00AA6720, __thiscall(widget; node) RET 4 (route to
    // bind_native_gui_widget_node_00aa6720).
    virtual void* model_pool_allocate_00b74eb0(std::uint32_t size) = 0;
    virtual void* generated_model_construct_00b75030(void* block, const NativeStringHeader* name) = 0;
    virtual void set_scene_node_00aa6720(void* widget, void* node) = 0;
    // widget->vtable[+74h]().
    virtual void widget_vslot74(void* widget) = 0;
    // 00AA77B0 BSP_GuiTimedEntryArray_Resize, __thiscall(array at widget+88h;
    // count) RET 4.
    virtual void timed_entry_array_resize_00aa77b0(void* array, std::int32_t count) = 0;
    // 00BF681B operator new (cdecl, size) -> block or 0.
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    // 00AC2ED0, __thiscall(block; widget) RET 4 -> entry: the timed-entry
    // constructor for indices 0 and 1.
    virtual void* timed_entry_construct_00ac2ed0(void* block, void* widget) = 0;
};

// 00AA6560 BSP_GuiLayout_CreateWidgetOfType, __fastcall(ECX = type tag, EDX =
// source) RET -> widget, or 0 for a tag outside 1..18.
void* gui_layout_create_widget_of_type_00aa6560(GuiWidgetHooks& host, std::uint32_t type_tag, void* source);
// 00AD3A80 BSP_GuiTimedEntry_CreateByIndex, __fastcall(ECX = index, EDX =
// widget) RET -> entry or 0.
void* gui_timed_entry_create_by_index_00ad3a80(GuiWidgetHooks& host, std::int32_t index, void* widget);

// 00AAB4C0, __thiscall(ECX = widget; parent_clone) RET 4 -> clone.
void* gui_widget_clone_subtree_00aab4c0(GuiWidgetHooks& host, void* widget, void* parent_clone);
// 00AA8240, __thiscall(ECX = widget; const float position[3]) RET 4.
void gui_widget_set_resolved_position_00aa8240(GuiWidgetHooks& host, void* widget,
    const float* position);
// 00AA67F0, __thiscall(ECX = widget) RET -> widget+0Ch.
float* gui_widget_get_local_position_00aa67f0(void* widget) noexcept;
// 00AA6BC0, __thiscall(ECX = widget; listener, char flag) RET 8.
void gui_widget_set_listener_fields_00aa6bc0(void* widget, void* listener, std::uint8_t flag) noexcept;
// 00AAA5A0, __thiscall(ECX = parent; child, const NativeListIterator* where)
// RET 8.
void gui_widget_attach_child_00aaa5a0(GuiWidgetHooks& host, void* parent, void* child,
    const NativeListIterator* where);
// 00AA8B00, __thiscall(ECX = widget; int index) RET 4 -> entry.
void* gui_timed_entry_get_or_create_00aa8b00(GuiWidgetHooks& host, void* widget, std::int32_t index);
// 00AA6640, __fastcall(ECX = type tag, EDX = const NativeString* node_name;
// ctor_arg) RET 4 -> widget.
void* gui_widget_create_with_scene_node_00aa6640(GuiWidgetHooks& host, std::uint32_t type_tag,
    const NativeStringHeader* node_name, void* ctor_arg);

// ---------------------------------------------------------------------------
// Text, segment 78
// ---------------------------------------------------------------------------
struct GuiTextHooks : NativePoolHooks {
    // 00435C40, __thiscall(string; const other*) RET 4 -> AL equal.
    virtual bool native_string_equals_00435c40(const NativeStringHeader* self,
        const NativeStringHeader* other) = 0;
    // 0041DD40 BSP_NativeString_Resize, __thiscall(string; length, preserve)
    // RET 8 (resize_native_string_header_0041dd40, include/bsp/native_string.hpp).
    virtual void native_string_resize_0041dd40(NativeStringHeader* self, std::uint32_t length,
        std::uint32_t preserve) = 0;
    // 00BF7680 memcpy (CRT).
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t size) = 0;
    // [00F8BC4C] the locale table; 00A9FAD0, __thiscall(table; out wide, const
    // NativeString* key) RET 8: the localised lookup.
    virtual void* locale_table_00f8bc4c() = 0;
    virtual void localise_00a9fad0(void* table, NativeWideStringHeader* out,
        const NativeStringHeader* key) = 0;
    // 004C5E60, __thiscall(out wide; const char*) RET 4 -> out: widen.
    virtual NativeWideStringHeader* widen_004c5e60(NativeWideStringHeader* out, const char* text) = 0;
    // 00F8BE4C, the image's empty narrow string.
    virtual const char* empty_string_00f8be4c() = 0;
    // 00AB8F00, __thiscall(text; out wide, const in wide*, float width) RET 0Ch
    // -> out: ellipsis.
    virtual NativeWideStringHeader* ellipsize_00ab8f00(void* text, NativeWideStringHeader* out,
        const NativeWideStringHeader* in, float width) = 0;
    // 00ABA8D0 BSP_TextContext_UpdateUtf16Geometry, __thiscall(text; const wide*) RET 4.
    virtual void update_utf16_geometry_00aba8d0(void* text, const NativeWideStringHeader* source) = 0;
    // text->vtable[+50h](text+50h).
    virtual void text_vslot50(void* text, void* text_plus_50) = 0;
};

// 00ABAED0, __thiscall(ECX = text; const NativeString* source, char localise) RET 8.
void gui_text_set_localised_source_00abaed0(GuiTextHooks& host, void* text,
    const NativeStringHeader* source, std::uint8_t localise);
// 00ABB000, __thiscall(ECX = text; const NativeString* source, float width,
// char localise) RET 0Ch.
void gui_text_set_ellipsized_source_00abb000(GuiTextHooks& host, void* text,
    const NativeStringHeader* source, float width, std::uint8_t localise);
// 00AB6BD0, __thiscall(ECX = text context) RET, result in ST0 after a float32
// store and reload.
float text_context_get_normalized_text_height_00ab6bd0(const void* context) noexcept;
// 00AB6AB0, __thiscall(ECX = text; const wide*) RET 4.
void gui_text_submit_utf16_00ab6ab0(GuiTextHooks& host, void* text, const NativeWideStringHeader* source);

// ---------------------------------------------------------------------------
// Listbox, segment 75
// ---------------------------------------------------------------------------
struct GuiListboxConstructHooks {
    virtual ~GuiListboxConstructHooks() = default;
    // Existing complete normal-path body: construct_native_gui_widget_base_00aa9390.
    // Native __thiscall(ECX = listbox; type) RET 4.
    virtual void widget_base_construct_00aa9390(void* listbox, std::uint32_t type) = 0;
    // Existing allocator: allocate_native_gui_widget_list_sentinel_00a9b720.
    // Native ECX = list subobject (unused); RET -> self-linked 0Ch-byte head.
    virtual void* allocate_sentinel_00a9b720(void* list) = 0;
};
// 00A9DF40: ECX = destination, EAX = destination, RET. Complete normal path;
// preserves all unwritten padding. The shared hook/nonthrowing SEH boundary
// described at the top of this header applies.
void* gui_listbox_construct_00a9df40(void* listbox, GuiListboxConstructHooks&);

struct GuiListboxHooks : NativeListHooks {
    // 00AAA5A0 BSP_GuiWidget_AttachChild (gui_widget_attach_child_00aaa5a0 above).
    virtual void attach_child_00aaa5a0(void* parent, void* child, const NativeListIterator* where) = 0;
    // 00A9BB70 BSP_GuiListbox_LayoutItem, __thiscall(listbox; row, char) RET 8.
    virtual void layout_item_00a9bb70(void* listbox, void* row, std::uint8_t flag) = 0;
    // 00A9D750 (gui_listbox_insert_row_00a9d750 below).
    virtual void insert_row_00a9d750(void* listbox, void* row, const NativeListIterator* where,
        std::uint8_t after) = 0;
    // row->vtable[+34h](char), listbox->vtable[+80h](int), listbox->vtable[+7Ch]().
    virtual void widget_vslot34(void* widget, std::uint8_t value) = 0;
    virtual void listbox_vslot80(void* listbox, std::int32_t value) = 0;
    virtual void listbox_vslot7c(void* listbox) = 0;
};

// 00A9D750, __thiscall(ECX = listbox; row, const NativeListIterator* where,
// char after) RET 0Ch.
void gui_listbox_insert_row_00a9d750(GuiListboxHooks& host, void* listbox, void* row,
    const NativeListIterator* where, std::uint8_t after);
// 00A9CEB0, __thiscall(ECX = listbox; float value, char relayout) RET 8.
void gui_listbox_set_field124_00a9ceb0(GuiListboxHooks& host, void* listbox, float value,
    std::uint8_t relayout);
// 00A9DA30, __thiscall(ECX = listbox; row, data, where, char after) RET 10h.
void gui_listbox_insert_row_with_data_00a9da30(GuiListboxHooks& host, void* listbox, void* row,
    void* data, const NativeListIterator* where, std::uint8_t after);
// 00A9C380, __thiscall(ECX = listbox; int index, char visible) RET 8.
void gui_list_control_set_item_visible_00a9c380(GuiListboxHooks& host, void* listbox,
    std::int32_t index, std::uint8_t visible);

}  // namespace bsp::cc13_gui1
