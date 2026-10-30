// cc13_gui1: GUI-core blockers (packet cc13-gui1, 2026-10-17). Evidence, ABI and
// uncertainty per routine: the comments below and docs/CC13_GUI1.md, section
// "GUI-core blockers". Raw native objects; every callee is the named hook in
// include/bsp/cc13_gui1.hpp, invoked at the binary's call point.
#include "bsp/cc13_gui1.hpp"

#include <cstring>
#include <initializer_list>

namespace bsp::cc13_gui1 {
namespace {

template <class T>
T& at(void* base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(base) + offset);
}
template <class T>
const T& at(const void* base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const T*>(static_cast<const unsigned char*>(base) + offset);
}
// VS2008 std::list node: +0 next, +4 prev, +8 value.
void* node_next(void* node) noexcept { return at<void*>(node, 0); }
void* node_value(void* node) noexcept { return at<void*>(node, 8); }

void release_narrow(NativePoolHooks& host, const NativeStringHeader& s) {
    if (s.data != nullptr) {
        void* const pool = host.sized_storage_pool_00419cc0();
        host.pool_return_block_00bd1510(pool, s.data, s.length + 1, 1);
    }
}
void release_wide(NativePoolHooks& host, const NativeWideStringHeader& s) {
    if (s.data != nullptr) {
        void* const pool = host.sized_storage_pool_00419cc0();
        host.pool_return_block_00bd1510(pool, s.data, s.length * 2 + 2, 1);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 00AC6600 BSP_GuiLayer_ConstructFromScript. __thiscall(ECX = page; name, node,
// share_scene) RET 0Ch -> page, SEH 00CB8A93, frame 4E8h. Stack objects (S =
// ESP after the four saves): reader 14h at S+18h, page copy S+2Ch, Lua owner
// 4C8h at S+30h, path temporaries at S+10h, the by-value "_Common" string and
// the 14h globals object in outgoing-argument slots.
// ---------------------------------------------------------------------------
void* gui_layer_construct_from_script_00ac6600(GuiLayerConstructHost& host, void* page,
    const NativeStringHeader* name, void* scene_node, std::uint8_t share_scene) {
    host.widget_base_construct_00aa9390(page, 1);                       // 00AC6627
    at<std::uint32_t>(page, 0x000) = 0x00D5BE38;                        // 00AC6640, image vtable
    at<std::uint32_t>(page, 0x0EC) = 0;                                 // 00AC6646
    at<std::uint32_t>(page, 0x0F0) = 0;
    at<std::uint8_t>(page, 0x0F5) = 0;
    at<float>(page, 0x0F8) = 0.0f;                                      // 00AC6658 (XORPS)
    at<std::uint32_t>(page, 0x0FC) = 0;                                 // Priority
    auto* const page_name = &at<NativeStringHeader>(page, 0x100);
    page_name->length = 0;                                              // 00AC666D
    page_name->data = nullptr;
    if (page_name != name) {                                            // 00AC663E / 6672
        host.native_string_resize_0041dd40(page_name, name->length, 1);
        if (name->length != 0)
            host.memcpy_00bf7680(page_name->data, name->data, page_name->length);
    }
    at<std::uint32_t>(page, 0x108) = 0;                                 // 00AC66B2
    at<std::uint32_t>(page, 0x10C) = host.image_dword(0x00D7A2F0);      // 00AC6699 / 66B8
    at<std::uint32_t>(page, 0x110) = host.image_dword(0x00CE3804);      // 00AC66C0
    const std::uint32_t one = host.image_dword(0x00D7A24C);             // 00AC66D0
    at<std::uint32_t>(page, 0x114) = one;
    at<std::uint32_t>(page, 0x118) = 0;
    at<float>(page, 0x11C) = 0.0f;                                      // 00AC66E6
    at<std::uint8_t>(page, 0x120) = share_scene;                        // 00AC66EE
    at<std::uint8_t>(page, 0x121) = 0;
    at<std::uint32_t>(page, 0x20) = one;                                // 00AC6705
    at<std::uint32_t>(page, 0x24) = one;                                // 00AC670A
    host.set_scene_node_00aa6720(page, scene_node);                     // 00AC670F

    alignas(4) unsigned char lua_owner[0x4C8];                          // S+30h
    host.lua_owner_construct_00b66bd0(lua_owner);                       // 00AC6718
    host.lua_owner_open_00b6a020(lua_owner, 0x65);                      // 00AC672B

    NativeStringHeader common_name;                                     // outgoing slot
    host.native_string_assign_0041e870(&common_name, "_Common");        // 00AC673F, 00D5CB64
    NativeStringHeader path;                                            // S+10h
    NativeStringHeader* const common_path =
        host.compose_screen_script_path_00ac5600(&path, common_name);  // 00AC6748
    host.lua_owner_run_script_00b69d40(lua_owner, common_path, 0);      // 00AC675A
    if (path.data != nullptr) {                                         // 00AC675F
        void* const pool = host.sized_storage_pool_00419cc0();
        host.pool_return_block_00bd1510(pool, path.data, path.length + 1, 1);
    }
    NativeStringHeader* const page_path = host.compose_layer_script_path_00ac5770(page, &path);  // 00AC678D
    host.lua_owner_run_script_00b69d40(lua_owner, page_path, 0);        // 00AC67A0
    if (path.data != nullptr) {                                         // 00AC67A5
        void* const pool = host.sized_storage_pool_00419cc0();
        host.pool_return_block_00bd1510(pool, path.data, path.length + 1, 1);
    }

    alignas(4) unsigned char globals[0x14];                             // outgoing 14h slot
    host.lua_owner_get_globals_00b67980(lua_owner, globals);            // 00AC67DA
    alignas(4) unsigned char reader[0x14];                              // S+18h
    host.lua_reader_construct_004425c0(reader, globals);                // 00AC67E3
    host.reader_vslot4(reader, 0, "GuiScreen");                         // 00AC6804, 00D5CB24
    host.page_vslot18(page, reader);                                    // 00AC6812
    host.reader_vslot8(reader);                                         // 00AC681F
    host.node_propagate_root_00b6d890(at<void*>(page, 0x4C), 0);        // 00AC6825
    // Inline reader destructor: two base vtables around the vector tidy.
    at<std::uint32_t>(reader, 0) = 0x00CE44FC;                          // 00AC682A
    host.lua_object_vector_tidy_00441210(reader + 4);                   // 00AC683E
    at<std::uint32_t>(reader, 0) = 0x00CE374C;                          // 00AC6847
    host.lua_owner_close_00b669a0(lua_owner);                           // 00AC6857
    return page;
}

// ---------------------------------------------------------------------------
// 00AA5840 BSP_GuiManager_LoadPage. __thiscall(ECX = manager; name, screen_flag,
// add_reference), RET 0Ch, SEH 00CB71F0. Stack: ".mmod" temporary at ESP+14h,
// the concatenated path at ESP+0Ch (base after the three saves).
// ---------------------------------------------------------------------------
void* gui_manager_load_page_00aa5840(GuiLoadPageHost& host, void* manager,
    const NativeStringHeader* name, std::int32_t screen_flag, std::uint8_t add_reference) {
    void* page = host.find_page_by_name_00aa3140(manager, name);       // 00AA5862
    if (page != nullptr) {
        if (add_reference != 0)                                         // 00AA5985
            host.interlocked_increment_00ce221c(&at<volatile long>(page, 4));  // 00AA5990
        return page;
    }
    NativeStringHeader suffix;
    host.native_string_assign_0041e870(&suffix, ".mmod");              // 00AA587A, 00D0E67C
    NativeStringHeader path;
    host.native_string_concat_004261a0(name, &path, &suffix);          // 00AA588F
    release_narrow(host, suffix);                                       // 00AA5894..58B3

    // ESI is still 0 here (the failed lookup), so a VFS hit whose factory is
    // null leaves the scene root null.
    void* scene_root = nullptr;
    if (host.vfs_resolve_existing_name_00bdf4c0(host.vfs_singleton_0109ceec(), &path)) {  // 00AA58C3
        void* const resources = host.resource_manager_004c1400();      // 00AA58CC
        void* const factory = host.load_default_factory_00b80d70(resources, &path);  // 00AA58D8
        if (factory != nullptr) {
            void* const model = host.factory_instantiate_vslot8(factory, 0, 1.0f);  // 00AA58F0
            at<void*>(manager, 0x30) = model;                           // 00AA58F2
            scene_root = at<void*>(model, 0x0C);                        // 00AA58F5
        }
    } else {
        void* const block = host.group_pool_allocate_00b8f450(0x188);  // 00AA58FF
        scene_root = block != nullptr ? host.group_construct_00b8f5e0(block, name) : nullptr;  // 00AA5914
    }
    void* const layer = host.gui_layer_allocate_00ac51a0(0x124);       // 00AA5929
    page = layer != nullptr ? host.gui_layer_construct_00ac6600(layer, name, scene_root, screen_flag)  // 00AA5944
                            : nullptr;
    host.register_page_by_priority_00aa52a0(manager, page);            // 00AA5957
    release_narrow(host, path);                                         // 00AA595C..597E
    return page;
}

// ---------------------------------------------------------------------------
// Widget base
// ---------------------------------------------------------------------------

// 00AAB4C0 BSP_GuiWidget_CloneSubtree. __thiscall(ECX = widget; parent_clone)
// RET 4 -> clone. No SEH frame. The child list is the std::list at +64h (head
// pointer +68h); each child's clone is attached under this clone by the
// recursion. The +0Ch..+14h self-stores are MOVSS bit copies (no-ops kept in
// order); +08h is captured before the two refreshes and stored after them.
// No null test on the clone after 00AAB4E6: a null clone faults at +0Ch, as in
// the image.
void* gui_widget_clone_subtree_00aab4c0(GuiWidgetHooks& host, void* widget, void* parent_clone) {
    host.gui_manager_get_or_create_004c12b0();                          // 00AAB4CB, result unused
    void* const clone = gui_layout_create_widget_of_type_00aa6560(host, at<std::uint32_t>(widget, 0x60), widget);  // 00AAB4D5
    if (parent_clone != nullptr) {
        gui_widget_attach_child_00aaa5a0(host, parent_clone, clone, nullptr);  // 00AAB4F2
    } else if (clone != nullptr && at<void*>(widget, 0x70) != nullptr) {
        gui_widget_attach_child_00aaa5a0(host, at<void*>(widget, 0x70), clone, nullptr);
    }
    void* const list = static_cast<unsigned char*>(widget) + 0x64;
    void* node = node_next(at<void*>(widget, 0x68));                  // 00AAB4F7..4FA
    for (;;) {
        void* const head = at<void*>(list, 4);                          // 00AAB502, reread per pass
        if (node == head) break;                                        // 00AAB50C
        if (node == at<void*>(list, 4)) host.invalid_parameter_00bf6713();  // 00AAB510
        gui_widget_clone_subtree_00aab4c0(host, node_value(node), clone);   // 00AAB51E
        if (node == at<void*>(list, 4)) host.invalid_parameter_00bf6713();  // 00AAB523
        node = node_next(node);
    }
    std::uint32_t saved08;
    std::memcpy(&saved08, static_cast<unsigned char*>(widget) + 0x08, 4);  // 00AAB535
    std::uint32_t xyz[3];
    std::memcpy(xyz, static_cast<unsigned char*>(clone) + 0x0C, 12);  // 00AAB53A..554A
    std::memcpy(static_cast<unsigned char*>(clone) + 0x0C, xyz, 12);  // 00AAB551..555B
    host.recompose_local_transform_00aa7220(clone);                     // 00AAB560
    host.refresh_local_bounds_00aa70e0(clone);                          // 00AAB567
    std::memcpy(static_cast<unsigned char*>(clone) + 0x08, &saved08, 4);  // 00AAB573
    return clone;
}

// 00AA8240 BSP_GuiWidget_SetResolvedPosition. __thiscall(ECX = widget; const
// float position[3]) RET 4. With a parent (+70h): local = position -
// resolved(parent) + (parent+20h*parent+18h, parent+24h*parent+1Ch, 0.0),
// where the z term adds the double 0.0 at 00D7A258 (so -0.0 becomes +0.0).
// Single float ops are exact-equivalent to the x87 53-bit ones (one rounding).
// Without a parent the position is copied bitwise (MOVSS).
void gui_widget_set_resolved_position_00aa8240(GuiWidgetHooks& host, void* widget,
    const float* position) {
    void* const parent = at<void*>(widget, 0x70);
    if (parent != nullptr) {
        const float half_x = at<float>(parent, 0x20) * at<float>(parent, 0x18);  // 00AA8253..825E
        const float half_y = at<float>(parent, 0x24) * at<float>(parent, 0x1C);  // 00AA8262..8268
        float resolved_storage[3];
        const float* const resolved = host.get_resolved_position_00aa6750(parent, resolved_storage);  // 00AA826C
        const float dx = position[0] - resolved[0];                     // 00AA8275
        const float dy = position[1] - resolved[1];
        const float dz = position[2] - resolved[2];
        const float x = dx + half_x;                                    // 00AA8293
        const float y = dy + half_y;
        const float z = static_cast<float>(static_cast<double>(dz) + 0.0);  // 00AA82AF, double [00D7A258]
        at<float>(widget, 0x0C) = x;                                    // 00AA82BD
        at<float>(widget, 0x10) = y;
        at<float>(widget, 0x14) = z;
    } else {
        std::memcpy(static_cast<unsigned char*>(widget) + 0x0C, position, 12);  // 00AA82E5..82FD
    }
    host.recompose_local_transform_00aa7220(widget);                    // 00AA82CE / 00AA8302
    host.refresh_local_bounds_00aa70e0(widget);                         // 00AA82D5 / 00AA8309
}

// 00AA67F0: LEA EAX,[ECX+0Ch]; RET.
float* gui_widget_get_local_position_00aa67f0(void* widget) noexcept {
    return &at<float>(widget, 0x0C);
}

// 00AA6BC0: +0DCh = listener, byte +79h = flag; RET 8. No retain, no call.
void gui_widget_set_listener_fields_00aa6bc0(void* widget, void* listener, std::uint8_t flag) noexcept {
    at<void*>(widget, 0xDC) = listener;                                 // 00AA6BC8
    at<std::uint8_t>(widget, 0x79) = flag;                              // 00AA6BCE
}

// 00AAA5A0 BSP_GuiWidget_AttachChild. __thiscall(ECX = parent; child, const
// iterator* where) RET 8. Null child returns at once. A child with a parent is
// detached first; then push_back (node bought before head, 00A9D480 size check,
// head->prev and old-prev->next linked in that order) or list::insert at *where;
// then child+70h = parent and the scene nodes (+4Ch) are attached.
void gui_widget_attach_child_00aaa5a0(GuiWidgetHooks& host, void* parent, void* child,
    const NativeListIterator* where) {
    if (child == nullptr) return;                                       // 00AAA5AD
    if (at<void*>(child, 0x70) != nullptr)
        host.detach_child_00aa83a0(at<void*>(child, 0x70), child);      // 00AAA5B9
    void* const list = static_cast<unsigned char*>(parent) + 0x64;
    void* value = child;  // the argument slot at ESP+14h, passed by address
    if (where == nullptr) {
        void* const head = at<void*>(parent, 0x68);                     // 00AAA5C8
        void* const node = host.list_buy_node_00a9b790(list, head, at<void*>(head, 4), &value);  // 00AAA5DA
        host.list_incsize_00a9d480(list, 1);                            // 00AAA5E5
        at<void*>(head, 4) = node;                                      // 00AAA5EE
        at<void*>(at<void*>(node, 4), 0) = node;                        // 00AAA5F5
    } else {
        NativeListIterator result;
        host.list_insert_00a9d6c0(list, &result, *where, &value);       // 00AAA60E
    }
    at<void*>(child, 0x70) = parent;                                    // 00AAA613
    host.scene_node_attach_00b6e680(at<void*>(child, 0x4C), at<void*>(parent, 0x4C));  // 00AAA61D
}

// 00AA8B00 BSP_GuiTimedEntry_GetOrCreate. __thiscall(ECX = widget; int index)
// RET 4. Array at +88h (data), +8Ch (count); signed compares (JL). The slot
// address is taken from the CURRENT data pointer before 00AD3A80 runs, and the
// final read reloads the data pointer.
void* gui_timed_entry_get_or_create_00aa8b00(GuiWidgetHooks& host, void* widget, std::int32_t index) {
    return gui_timed_entry_get_or_create_00aa8b00(static_cast<GuiTimedEntryHooks&>(host), widget, index);
}

void* gui_timed_entry_get_or_create_00aa8b00(GuiTimedEntryHooks& host, void* widget, std::int32_t index) {
    void* const array = static_cast<unsigned char*>(widget) + 0x88;
    if (!(index < at<std::int32_t>(widget, 0x8C)))
        host.timed_entry_array_resize_00aa77b0(array, index + 1);       // 00AA8B20
    if (at<void**>(array, 0)[index] == nullptr) {                       // 00AA8B27
        if (!(index < at<std::int32_t>(array, 4)))
            host.timed_entry_array_resize_00aa77b0(array, index + 1);   // 00AA8B3F
        void** const slot = at<void**>(array, 0) + index;               // 00AA8B49..8B4D
        *slot = gui_timed_entry_create_by_index_00ad3a80(host, index, widget);  // 00AA8B4F..8B54
    }
    if (!(index < at<std::int32_t>(array, 4)))
        host.timed_entry_array_resize_00aa77b0(array, index + 1);       // 00AA8B62
    return at<void**>(array, 0)[index];                                 // 00AA8B67 / 00AA8B73
}

// 00AA6640 BSP_GuiWidget_CreateWithSceneNode. __fastcall(ECX = type tag, EDX =
// const NativeString* node name; ctor_arg) RET 4, SEH 00CB7368 (only guards the
// node construction). EDX is kept in EDI (00AA6657) and pushed unchanged to
// 00B75030 (00AA667E), which reads it as a NativeString; the image callers
// (among them 0053A4D0, 005A3610, 00ACA0B0) pass a stack NativeString.
// A null pool block passes a null node to 00AA6720.
void* gui_widget_create_with_scene_node_00aa6640(GuiWidgetHooks& host, std::uint32_t type_tag,
    const NativeStringHeader* node_name, void* ctor_arg) {
    void* const widget = gui_layout_create_widget_of_type_00aa6560(host, type_tag, ctor_arg);  // 00AA665D
    void* const block = host.model_pool_allocate_00b74eb0(0x184);       // 00AA6669
    void* const node = block != nullptr ? host.generated_model_construct_00b75030(block, node_name) : nullptr;  // 00AA6681
    host.set_scene_node_00aa6720(widget, node);                         // 00AA6695
    host.widget_vslot74(widget);                                        // 00AA66A1
    return widget;
}

// 00AA6560 BSP_GuiLayout_CreateWidgetOfType. __fastcall(ECX = type tag, EDX =
// source) RET. LEA EAX,[ECX-1]; CMP EAX,11h; JA -> 0 (so tag 0 and anything
// above 18 return null); else JMP [00AA65F4 + EAX*4] to a stub that moves EDX to
// ECX and tail-jumps to the creator. Table read from the image bytes.
void* gui_layout_create_widget_of_type_00aa6560(GuiWidgetHooks& host, std::uint32_t type_tag, void* source) {
    static constexpr std::uint32_t kCreators[18] = {
        0x00AA3BD0, 0x00AA12F0, 0x00AA1380, 0x00AA1530, 0x00AA14A0, 0x00AA1410,  // tags 1..6
        0x00AA15C0, 0x00AA1650, 0x00AA16E0, 0x00AA1770, 0x00AA64B0, 0x00AA2340,  // tags 7..12
        0x00AA1800, 0x00AA1890, 0x00AA4440, 0x00AA1920, 0x00AA19B0, 0x00AA1A40,  // tags 13..18
    };
    const std::uint32_t slot = type_tag - 1;                           // 00AA6560
    if (slot > 0x11) return nullptr;                                    // 00AA6566 / 00AA65F1
    return host.widget_type_creator(kCreators[slot], source);          // 00AA656C
}

// 00AD3A80 BSP_GuiTimedEntry_CreateByIndex. __fastcall(ECX = index, EDX =
// widget) RET, SEH 00CB9376 (guards the 00AC2ED0 construction only).
// Index 0 and 1: 14h bytes from operator new, constructed by 00AC2ED0(widget)
// (the image's EH state differs, 0 vs 1, nothing else). Index 2: the inline
// constructor writes +08h widget, +04h 0.0f, +00h vtable 00D5D210, +0Ch 0.0f,
// +10h 0.0f in that order. Any other index, or a null block: 0.
void* gui_timed_entry_create_by_index_00ad3a80(GuiWidgetHooks& host, std::int32_t index, void* widget) {
    return gui_timed_entry_create_by_index_00ad3a80(static_cast<GuiTimedEntryHooks&>(host), index, widget);
}

void* gui_timed_entry_create_by_index_00ad3a80(GuiTimedEntryHooks& host, std::int32_t index, void* widget) {
    if (index == 0 || index == 1) {
        void* const block = host.operator_new_00bf681b(0x14);          // 00AD3AFF / 00AD3AE7
        if (block == nullptr) return nullptr;                           // 00AD3B15
        void* entry;
        try {
            entry = host.timed_entry_construct_00ac2ed0(block, widget); // 00AD3B1A
        } catch (...) {
            // Native states0/1 -> CB936B/CB9360: free the captured block,
            // never a callback's replacement slot/header, then keep the escape.
            host.free_failed_constructor_00bf65ac(block);
            throw;
        }
        host.timed_entry_construction_returned(entry); // prepared Source custody
        return entry;
    }
    if (index == 2) {
        void* const block = host.operator_new_00bf681b(0x14);          // 00AD3AAE
        if (block == nullptr) return nullptr;                           // 00AD3AB8
        at<void*>(block, 0x08) = widget;                                // 00AD3ABD
        at<float>(block, 0x04) = 0.0f;                                  // 00AD3AC0
        at<std::uint32_t>(block, 0x00) = 0x00D5D210;                    // 00AD3AC5, image vtable
        at<float>(block, 0x0C) = 0.0f;                                  // 00AD3ACB
        at<float>(block, 0x10) = 0.0f;                                  // 00AD3AD0
        host.timed_entry_construction_returned(block); // prepared Source custody
        return block;
    }
    return nullptr;                                                     // 00AD3B2F
}

// ---------------------------------------------------------------------------
// Text
// ---------------------------------------------------------------------------
namespace {

// The shared head of 00ABAED0 and 00ABB000: compare with the cached source at
// +0F4h; when different, copy it in (resize with preserve, then memcpy of the
// NEW cached length when the source is non-empty; no copy when the source
// aliases the cache). Returns true when the source was already cached.
bool cache_text_source(GuiTextHooks& host, void* text, const NativeStringHeader* source) {
    auto* const cached = &at<NativeStringHeader>(text, 0xF4);
    if (host.native_string_equals_00435c40(cached, source)) return true;  // 00ABAEFA / 00ABB02A
    if (cached != source) {
        host.native_string_resize_0041dd40(cached, source->length, 1);
        if (source->length != 0)
            host.memcpy_00bf7680(cached->data, source->data, cached->length);
    }
    return false;
}

}  // namespace

// 00ABAED0 BSP_GuiText_SetLocalisedSource. __thiscall(ECX = text; const
// NativeString* source, char localise) RET 8, SEH 00CB8150. Unchanged source
// returns without any call (not even +50h). localise != 0: zeroed wide temp
// (ESP+0Ch) filled by 00A9FAD0 on [00F8BC4C]; else the narrow data (or the
// empty string 00F8BE4C when null) widened by 004C5E60 into ESP+14h. Then
// 00ABA8D0 with the temp, release (len*2+2), and vtable +50h(text+50h).
void gui_text_set_localised_source_00abaed0(GuiTextHooks& host, void* text,
    const NativeStringHeader* source, std::uint8_t localise) {
    if (cache_text_source(host, text, source)) return;
    if (localise != 0) {
        NativeWideStringHeader wide{0, nullptr};                        // 00ABAF36..AF3E
        host.localise_00a9fad0(host.locale_table_00f8bc4c(), &wide, source);  // 00ABAF5A
        host.update_utf16_geometry_00aba8d0(text, &wide);               // 00ABAF66
        release_wide(host, wide);                                       // 00ABAF6B..AFD2
    } else {
        const char* const narrow = source->data != nullptr ? source->data : host.empty_string_00f8be4c();
        NativeWideStringHeader wide;
        NativeWideStringHeader* const widened = host.widen_004c5e60(&wide, narrow);  // 00ABAF9A
        host.update_utf16_geometry_00aba8d0(text, widened);             // 00ABAFAA
        release_wide(host, wide);                                       // 00ABAFAF..AFD2
    }
    host.text_vslot50(text, static_cast<unsigned char*>(text) + 0x50);  // 00ABAFE3
}

// 00ABB000 BSP_GuiText_SetEllipsizedSource. __thiscall(ECX = text; const
// NativeString* source, float width, char localise) RET 0Ch, SEH 00CB8180.
// After the cache step, width == -1.0f (00D7A260, UCOMISS ordered-equal; NaN
// keeps its value) takes text+20h. The ellipsis 00AB8F00 writes a second temp
// (ESP+14h) that is released first, then the input temp (ESP+0Ch localised /
// ESP+1Ch widened).
void gui_text_set_ellipsized_source_00abb000(GuiTextHooks& host, void* text,
    const NativeStringHeader* source, float width, std::uint8_t localise) {
    if (cache_text_source(host, text, source)) return;
    if (width == -1.0f) width = at<float>(text, 0x20);                  // 00ABB05F..B077
    NativeWideStringHeader input;
    if (localise != 0) {
        input = NativeWideStringHeader{0, nullptr};                      // 00ABB088..B090
        host.localise_00a9fad0(host.locale_table_00f8bc4c(), &input, source);  // 00ABB0AC
        NativeWideStringHeader ellipsized;
        NativeWideStringHeader* const result = host.ellipsize_00ab8f00(text, &ellipsized, &input, width);  // 00ABB0C5
        host.update_utf16_geometry_00aba8d0(text, result);              // 00ABB0D2
        release_wide(host, ellipsized);                                 // 00ABB0D7..B0F7
    } else {
        const char* const narrow = source->data != nullptr ? source->data : host.empty_string_00f8be4c();
        NativeWideStringHeader* const widened = host.widen_004c5e60(&input, narrow);  // 00ABB127
        NativeWideStringHeader ellipsized;
        NativeWideStringHeader* const result = host.ellipsize_00ab8f00(text, &ellipsized, widened, width);  // 00ABB144
        host.update_utf16_geometry_00aba8d0(text, result);              // 00ABB151
        release_wide(host, ellipsized);                                 // 00ABB156..B176
    }
    release_wide(host, input);                                          // 00ABB0FC / 00ABB17B..B19E
    host.text_vslot50(text, static_cast<unsigned char*>(text) + 0x50);  // 00ABB1AF
}

// 00AB6BD0 BSP_TextContext_GetNormalizedTextHeight. Byte +0FCh set: float +178h
// / 720.0 (double 00CEF1B8); else the font at +108h: sign-extended int16 at
// font+14h / 720.0, or 0 / 720.0 for a null font. Rounded to float32 (the FSTP
// to [ESP] and reload).
float text_context_get_normalized_text_height_00ab6bd0(const void* context) noexcept {
    constexpr double k720 = 720.0;  // 00CEF1B8: 00 00 00 00 00 80 86 40
    if (at<std::uint8_t>(context, 0xFC) != 0)
        return static_cast<float>(static_cast<double>(at<float>(context, 0x178)) / k720);
    const void* const font = at<const void*>(context, 0x108);
    if (font != nullptr)
        return static_cast<float>(static_cast<double>(at<std::int16_t>(font, 0x14)) / k720);
    return static_cast<float>(0.0 / k720);
}

// 00AB6AB0 BSP_GuiText_SubmitUtf16. __thiscall(ECX = text; const wide*) RET 4:
// 00ABA8D0 then vtable +50h(text+50h). No cache compare, no copy.
void gui_text_submit_utf16_00ab6ab0(GuiTextHooks& host, void* text, const NativeWideStringHeader* source) {
    host.update_utf16_geometry_00aba8d0(text, source);                  // 00AB6AB8
    host.text_vslot50(text, static_cast<unsigned char*>(text) + 0x50);  // 00AB6AC8
}

// ---------------------------------------------------------------------------
// Listbox. Row list: std::list at +0FCh (head pointer +100h, size +104h).
// ---------------------------------------------------------------------------

// 00A9DF40, native __thiscall ECX destination, EAX destination, RET. Complete
// normal body from the 64-instruction listing; the base and sentinel allocator
// are bound to their established native bodies. Unwritten padding, list+0 and
// sentinel payloads remain untouched. Original FH3/SEH unwinding is outside the
// nonthrowing hook contract of this family; this is not a binary ABI replacement.
void* gui_listbox_construct_00a9df40(void* listbox, GuiListboxConstructHooks& host) {
    host.widget_base_construct_00aa9390(listbox, 11); // 00A9DF61
    at<std::uint32_t>(listbox, 0) = 0x00D5BBF8u;
    void* const row_list = static_cast<unsigned char*>(listbox) + 0xFC;
    void* const rows = host.allocate_sentinel_00a9b720(row_list); // 00A9DF7A
    at<void*>(listbox, 0x100) = rows;
    at<std::uint32_t>(listbox, 0x104) = 0;
    at<void*>(listbox, 0x108) = row_list;
    at<void*>(listbox, 0x10C) = rows;
    at<std::uint8_t>(listbox, 0x120) = 1;
    at<std::uint32_t>(listbox, 0x110) = 0;
    at<std::uint32_t>(listbox, 0x114) = 0;
    at<std::uint32_t>(listbox, 0x118) = 0xFFFFFFFFu;
    for (const std::uint32_t offset : {0x11Cu, 0x11Du, 0x11Eu, 0x11Fu, 0x121u})
        at<std::uint8_t>(listbox, offset) = 0;
    for (const std::uint32_t offset : {0x124u, 0x128u, 0x12Cu, 0x130u, 0x134u})
        at<std::uint32_t>(listbox, offset) = 0;
    at<std::uint8_t>(listbox, 0x149) = 1;
    at<std::uint8_t>(listbox, 0x14A) = 1;
    at<std::uint8_t>(listbox, 0x138) = 0;
    at<std::uint32_t>(listbox, 0x13C) = 0;
    at<std::uint8_t>(listbox, 0x140) = 0;
    at<std::uint8_t>(listbox, 0x141) = 0;
    at<std::uint32_t>(listbox, 0x144) = 0;
    at<std::uint8_t>(listbox, 0x148) = 0;
    void* const secondary_list = static_cast<unsigned char*>(listbox) + 0x14C;
    at<void*>(listbox, 0x150) = host.allocate_sentinel_00a9b720(secondary_list); // 00A9E036
    at<std::uint32_t>(listbox, 0x154) = 0;
    at<std::uint32_t>(listbox, 0x170) = 0;
    at<std::uint32_t>(listbox, 0x174) = 0;
    at<std::uint32_t>(listbox, 0x178) = 0;
    return listbox;
}

// 00A9D750 BSP_GuiListbox_InsertRow. __thiscall(ECX = listbox; row, const
// iterator* where, char after) RET 0Ch, no SEH.
// 1. row->vtable[+34h](1).
// 2. where == null: AttachChild(listbox, row, null), then push_back on +0FCh.
//    Otherwise copy *where (ESP+0Ch), and with `after` step it once (checked:
//    null container or end node report 00BF6713); AttachChild(listbox, row,
//    &copy) - the row-list iterator is handed to the widget child list as in the
//    image - then list::insert(copy) on +0FCh.
// 3. When the row list now has exactly one row: +108h/+10Ch = {+0FCh, first row
//    node, or the head when that row's byte +77h is set}, then vtable +80h(0).
// 4. vtable +7Ch().
void gui_listbox_insert_row_00a9d750(GuiListboxHooks& host, void* listbox, void* row,
    const NativeListIterator* where, std::uint8_t after) {
    host.widget_vslot34(row, 1);                                        // 00A9D765
    void* const rows = static_cast<unsigned char*>(listbox) + 0xFC;
    void* value = row;  // the argument slot at ESP+20h, passed by address
    if (where == nullptr) {
        host.attach_child_00aaa5a0(listbox, row, nullptr);              // 00A9D774
        void* const head = at<void*>(listbox, 0x100);                   // 00A9D779
        void* const node = host.list_buy_node_00a9b790(rows, head, at<void*>(head, 4), &value);  // 00A9D791
        host.list_incsize_00a9d480(rows, 1);                            // 00A9D79C
        at<void*>(head, 4) = node;                                      // 00A9D7A1
        at<void*>(at<void*>(node, 4), 0) = node;                        // 00A9D7A7
    } else {
        NativeListIterator position = *where;                           // 00A9D7B1..D7BA
        if (after != 0) {
            if (position.container == nullptr) host.invalid_parameter_00bf6713();  // 00A9D7C4
            if (position.node == at<void*>(position.container, 4)) host.invalid_parameter_00bf6713();  // 00A9D7D6
            position.node = node_next(position.node);                   // 00A9D7DB..D7E1
        }
        host.attach_child_00aaa5a0(listbox, row, &position);            // 00A9D7ED
        NativeListIterator result;
        host.list_insert_00a9d6c0(rows, &result, position, &value);     // 00A9D80E
    }
    if (at<std::uint32_t>(listbox, 0x104) == 1) {                       // 00A9D813
        void* const head = at<void*>(rows, 4);
        void* const first = node_next(head);
        if (first == head) host.invalid_parameter_00bf6713();           // 00A9D825
        const bool flagged = at<std::uint8_t>(node_value(first), 0x77) != 0;  // 00A9D82D
        void* const head_now = at<void*>(rows, 4);                      // 00A9D831
        at<void*>(listbox, 0x10C) = flagged ? head_now : node_next(head_now);  // 00A9D83E
        at<void*>(listbox, 0x108) = rows;                               // 00A9D84A
        host.listbox_vslot80(listbox, 0);                               // 00A9D850
    }
    host.listbox_vslot7c(listbox);                                      // 00A9D859
}

// 00A9CEB0 BSP_GuiListbox_SetField124. __thiscall(ECX = listbox; float value,
// char relayout) RET 8. +124h is stored first (MOVSS). With relayout each row is
// placed by 00A9BB70(row, 1); the end node is reread every pass. The container
// tests on ESI (= listbox+0FCh, never null) are dead in the image and omitted.
void gui_listbox_set_field124_00a9ceb0(GuiListboxHooks& host, void* listbox, float value,
    std::uint8_t relayout) {
    std::memcpy(static_cast<unsigned char*>(listbox) + 0x124, &value, 4);  // 00A9CEBE
    if (relayout == 0) return;
    void* const rows = static_cast<unsigned char*>(listbox) + 0xFC;
    void* node = node_next(at<void*>(listbox, 0x100));                  // 00A9CEC8..CED6
    while (node != at<void*>(rows, 4)) {                                // 00A9CEE2..CEF2
        if (node == at<void*>(rows, 4)) host.invalid_parameter_00bf6713();  // 00A9CEFD
        host.layout_item_00a9bb70(listbox, node_value(node), 1);        // 00A9CF0F
        if (node == at<void*>(rows, 4)) host.invalid_parameter_00bf6713();  // 00A9CF14
        node = node_next(node);
    }
}

// 00A9DA30 BSP_GuiListbox_InsertRowWithData: row+0D8h = data, then 00A9D750
// with the same ECX and (row, where, after). RET 10h.
void gui_listbox_insert_row_with_data_00a9da30(GuiListboxHooks& host, void* listbox, void* row,
    void* data, const NativeListIterator* where, std::uint8_t after) {
    at<void*>(row, 0xD8) = data;                                        // 00A9DA38
    host.insert_row_00a9d750(listbox, row, where, after);               // 00A9DA49
}

// 00A9C380 BSP_GuiListControl_SetItemVisible. __thiscall(ECX = control; int
// index, char visible) RET 8. Counts rows from the first; at the index row byte
// +77h = visible ? 0 : 1. An index past the end does nothing. End reread per pass.
void gui_list_control_set_item_visible_00a9c380(GuiListboxHooks& host, void* listbox,
    std::int32_t index, std::uint8_t visible) {
    void* const rows = static_cast<unsigned char*>(listbox) + 0xFC;
    void* node = node_next(at<void*>(listbox, 0x100));                  // 00A9C380..C390
    std::int32_t count = 0;
    while (node != at<void*>(rows, 4)) {                                // 00A9C396..C3A2
        if (count == index) {                                           // 00A9C3A4
            if (node == at<void*>(rows, 4)) host.invalid_parameter_00bf6713();  // 00A9C3C2 / 00A9C3DA
            at<std::uint8_t>(node_value(node), 0x77) = visible != 0 ? 0 : 1;  // 00A9C3D2 / 00A9C3E7
            return;
        }
        ++count;
        if (node == at<void*>(rows, 4)) host.invalid_parameter_00bf6713();  // 00A9C3AD
        node = node_next(node);
    }
}

}  // namespace bsp::cc13_gui1
