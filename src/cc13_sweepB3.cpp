// cc13_sweepB3: third pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB3, docs/CC13_SWEEPB3.md). Names are hypotheses; see the header
// for the hooks.
//
// x87 note (as cc13_sweepB / cc13_sweepB2): float expressions follow the
// image's operation order with a float store wherever the image stores to a
// float slot; double constants are applied in double. EH frames are noted, not
// modelled. Register-register x87 operand directions were read from the bytes
// (local/sweepB3_dis.py decodes DC/DE forms by the Intel encoding).

#include "bsp/cc13_sweepB3.hpp"

#include <cstring>
#include <initializer_list>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

std::uint8_t* bytes(void* base) {
    return static_cast<std::uint8_t*>(base);
}

// A double from its image bit pattern (exact constants).
double f64_bits(std::uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

float f32_bits(std::uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

void* image_pointer(Cc13SweepBHost& host, std::uint32_t address) {
    return *static_cast<void**>(host.image_global(address));
}

// The inlined native-string release: when data is non-null, the data goes
// back to the sized pool with length + 1 bytes.
void release_string(Cc13SweepBNativeString& text, Cc13SweepBHost& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// The inlined temporary native string of a C string: {0, 0},
// 0041DD40(length, 1), memcpy of length + 1 bytes when data is non-null, the
// use, then release_string (the length re-read after the use).
template <class Use>
void with_c_string(const char* source, std::uint32_t length, Cc13SweepBHost& host, Use use) {
    Cc13SweepBNativeString text = {0, nullptr};
    host.native_string_resize_0041dd40(&text, length, 1);
    if (text.data != nullptr) host.crt_memcpy_00bf7680(text.data, source, text.length + 1);
    use(&text);
    release_string(text, host);
}

// The inlined ref-pointer assignment: unchanged -> nothing; else store, then
// InterlockedIncrement(new+4h) when non-null, then InterlockedDecrement(old+4h)
// when non-null and, at zero, the old object's vtable +0h.
void ref_assign(void** slot, void* value, Cc13SweepBHost& host) {
    void* old = *slot;
    if (old == value) return;
    *slot = value;
    if (value != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(value, 0x4));
    if (old != nullptr && host.interlocked_decrement_00ce2220(&at<std::int32_t>(old, 0x4)) == 0) {
        host.refcounted_vcall_00_destroy(old);
    }
}

// The inlined by-value ref-pointer destructor (no store back).
void ref_release(void* value, Cc13SweepBHost& host) {
    if (value != nullptr && host.interlocked_decrement_00ce2220(&at<std::int32_t>(value, 0x4)) == 0) {
        host.refcounted_vcall_00_destroy(value);
    }
}

}  // namespace

// ===========================================================================
// Segment 18
// ===========================================================================

// 0056D660 BSP_MultiSessionBrowserScreen_FillSessionSettings
// __thiscall(ECX = session browser screen) RET; SEH frame (handler 00C6EA8E),
// aligned EBP frame. Callers 0056E360 and 00570300.
// For row i = 0..8: row = 00A9BE00(+34h, i); text = 00AA7E00(row,
// "setting_value_Text" (00CEEFDC, 12h), 1); then by the jump table at
// 0056D974:
// - i 0..4: byte +10h+i, i 8: byte +18h -> 00ABBE50(text, "globals.yes"
//   (00CED4F0) when set else "globals.no" (00CED4E4), 1);
// - i 5/6/7 with byte +15h/+16h/+17h: set -> a native string of the int
//   +244h/+248h/+24Ch (004260B0), else "globals.no" (0041E870); 00ABAED0(text,
//   string, 1); the temporary is released (the image's flag bits in its
//   local +10h only track which temporary exists);
// - any other i (unreachable): 00ABBE50(text, "ERROR!" (00CEEFD4), 1).
void cc13_sweepB3_session_browser_fill_settings_0056d660(void* screen, Cc13SweepB3Host& host) {
    const char* yes = static_cast<const char*>(host.image_global(0x00CED4F0));
    const char* no = static_cast<const char*>(host.image_global(0x00CED4E4));
    for (std::int32_t i = 0; i < 9; ++i) {
        void* row = host.gui_listbox_row_00a9be00(at<void*>(screen, 0x34), i);
        void* text = nullptr;
        with_c_string(static_cast<const char*>(host.image_global(0x00CEEFDC)), 0x12, host,
                      [&](Cc13SweepBNativeString* name) { text = host.widget_find_child_00aa7e00(row, name, 1); });
        if (i <= 4 || i == 8) {
            const std::size_t flag = i == 8 ? 0x18 : 0x10 + static_cast<std::size_t>(i);
            host.gui_text_set_localised_cstring_00abbe50(text, at<std::uint8_t>(screen, flag) != 0 ? yes : no, 1);
        } else if (i <= 7) {
            const std::size_t flag = 0x10 + static_cast<std::size_t>(i);
            const std::size_t value = 0x244 + 4 * static_cast<std::size_t>(i - 5);
            Cc13SweepBNativeString temporary = {0, nullptr};  // constructed by the callee in the image
            void* source = at<std::uint8_t>(screen, flag) != 0
                               ? host.native_string_from_int_004260b0(&temporary, at<std::int32_t>(screen, value))
                               : host.native_string_assign_0041e870(&temporary, no);
            host.gui_text_set_localised_source_00abaed0(text, source, 1);
            release_string(temporary, host);
        } else {
            host.gui_text_set_localised_cstring_00abbe50(text, static_cast<const char*>(host.image_global(0x00CEEFD4)), 1);
        }
    }
}

// ===========================================================================
// Segment 19
// ===========================================================================

// 0056D9A0 BSP_MultiSessionBrowserScreen_BuildFilterRows
// __thiscall(ECX = session browser screen) RET; SEH frame (handler 00C6EAC0),
// aligned EBP frame. Sole caller 0056F320.
// +240h = 0.0f; y = 0. For row i = 0..8 (labels from the .data table
// 00E086C4.., 9 char* entries):
// - +54h+4i = 00AAB4C0(+0F4h, +100h) (a clone); its vtable +34h(1);
// - text = 00AA7E00(row, "setting_var_Text" (00CEEE0C, 10h), 1);
// - 00AA7E00(row, "setting_2_Text" (00CEEFFC, 0Eh), 1) vtable +34h(1);
// - 00AA7E00(row, "nike_Icon" (00CEEFF0, 9), 1) vtable +34h(byte +10h+i);
// - 00ABAED0(text, label string, 1);
// - 00AA8240(row, {+104h, y + +108h, +10Ch}); y = +25Ch + y.
// Then 00683790(+12Ch, 0) and 006834A0(+12Ch, +240h - 00AA6740(+44h)[1]).
void cc13_sweepB3_session_browser_build_filter_rows_0056d9a0(void* screen, Cc13SweepB3Host& host) {
    float y = 0.0f;
    at<float>(screen, 0x240) = 0.0f;
    for (std::size_t i = 0; i < 9; ++i) {
        void*& row = at<void*>(screen, 0x54 + 4 * i);
        row = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0xF4), at<void*>(screen, 0x100));
        host.widget_vcall_34_set_visible(row, 1);
        void* text = nullptr;
        with_c_string(static_cast<const char*>(host.image_global(0x00CEEE0C)), 0x10, host,
                      [&](Cc13SweepBNativeString* name) { text = host.widget_find_child_00aa7e00(row, name, 1); });
        void* second = nullptr;
        with_c_string(static_cast<const char*>(host.image_global(0x00CEEFFC)), 0xE, host,
                      [&](Cc13SweepBNativeString* name) { second = host.widget_find_child_00aa7e00(row, name, 1); });
        host.widget_vcall_34_set_visible(second, 1);
        void* icon = nullptr;
        with_c_string(static_cast<const char*>(host.image_global(0x00CEEFF0)), 9, host,
                      [&](Cc13SweepBNativeString* name) { icon = host.widget_find_child_00aa7e00(row, name, 1); });
        host.widget_vcall_34_set_visible(icon, at<std::uint8_t>(screen, 0x10 + i));
        const char* label = *static_cast<const char**>(host.image_global(0x00E086C4 + 4 * static_cast<std::uint32_t>(i)));
        with_c_string(label, static_cast<std::uint32_t>(std::strlen(label)), host, [&](Cc13SweepBNativeString* source) {
            host.gui_text_set_localised_source_00abaed0(text, source, 1);
        });
        float position[3];
        position[0] = at<float>(screen, 0x104);
        position[1] = y + at<float>(screen, 0x108);
        position[2] = at<float>(screen, 0x10C);
        host.widget_set_resolved_position_00aa8240(row, position);
        y = at<float>(screen, 0x25C) + y;
    }
    host.gui_scroller_attach_content_00683790(bytes(screen) + 0x12C, 0);
    const float* size = host.gui_widget_get_size_00aa6740(at<void*>(screen, 0x44));
    host.gui_scroller_set_scroll_range_006834a0(bytes(screen) + 0x12C, at<float>(screen, 0x240) - size[1]);
}

// 00578580 BSP_SessionBrowserRow_UpdatePingIcons
// __thiscall(ECX = row widget, float ping) RET 4; SEH frame (handler 00C6F4D8)
// covers the temporary names. Sole caller 005D73A0.
// - Pass 1 over the five names at 00E0874C.. ("ping_1_Icon".."ping_5_Icon";
//   the table is in .data, read through image_global): threshold i is the
//   float registry(00576B10, called per entry)+A4h+4*i. FCOMI + JB: when
//   ping < threshold, or unordered, the child named i is hidden (vtable +34h
//   with 0); otherwise it is shown (+34h with 1) and counted.
// - level = count < 2 ? 2 : count < 4 ? 1 : 0 (signed compares).
// - Pass 2 over the same names: child vtable +88h(level, 0, 1.0f).
// Each child is 00AA7E00(row, name, 1) on a temporary native string whose
// length is the inlined strlen of the name.
void cc13_sweepB3_session_row_update_ping_icons_00578580(void* row, float ping, Cc13SweepB3Host& host) {
    std::int32_t shown = 0;
    for (std::uint32_t i = 0; i < 5; ++i) {
        const float threshold = at<float>(host.lobby_option_registry_00576b10(), 0xA4 + 4 * i);
        const char* name = *static_cast<const char**>(host.image_global(0x00E0874C + 4 * i));
        const bool lit = ping >= threshold;   // FCOMI + JB: below or unordered (NaN) hides
        with_c_string(name, static_cast<std::uint32_t>(std::strlen(name)), host, [&](Cc13SweepBNativeString* text) {
            void* icon = host.widget_find_child_00aa7e00(row, text, 1);
            host.widget_vcall_34_set_visible(icon, lit ? 1 : 0);
        });
        if (lit) ++shown;
    }
    std::int32_t level = 0;
    if (shown < 2) {
        level = 2;
    } else if (shown < 4) {
        level = 1;
    }
    for (std::uint32_t i = 0; i < 5; ++i) {
        const char* name = *static_cast<const char**>(host.image_global(0x00E0874C + 4 * i));
        with_c_string(name, static_cast<std::uint32_t>(std::strlen(name)), host, [&](Cc13SweepBNativeString* text) {
            void* icon = host.widget_find_child_00aa7e00(row, text, 1);
            host.widget_vcall_88(icon, level, 0, 1.0f);
        });
    }
}

// ===========================================================================
// Segment 20
// ===========================================================================

namespace {

// The inlined checked std::list walk of 00583E50: list record {+0 proxy,
// +4 head node, +8 size}, node {+0 next, +4 prev, +8 widget}. The head is
// read once for the first node and re-read for every end test. Each non-null
// widget is deleted through its vtable +4h with 1. The image's iterator
// container self-compare (CMP reg,reg; JE) can never fail and is omitted; the
// node-is-head checks before and after the delete call 00BF6713.
void delete_list_widgets(std::uint8_t* list, Cc13SweepB3Host& host) {
    void* node = at<void*>(at<void*>(list, 0x4), 0x0);
    while (node != at<void*>(list, 0x4)) {
        if (node == at<void*>(list, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
        void* widget = at<void*>(node, 0x8);
        if (widget != nullptr) host.object_vcall_04_delete(widget, 1);
        if (node == at<void*>(list, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
        node = at<void*>(node, 0x0);
    }
}

// The inlined std::list::clear: first = head->next; head->next = head;
// head->prev = head; size = 0; then every node until the head is freed with
// operator delete 00BF65AC (the head re-read for each test).
void clear_list(std::uint8_t* list, Cc13SweepB3Host& host) {
    void* node = at<void*>(at<void*>(list, 0x4), 0x0);
    at<void*>(at<void*>(list, 0x4), 0x0) = at<void*>(list, 0x4);
    at<void*>(at<void*>(list, 0x4), 0x4) = at<void*>(list, 0x4);
    at<std::uint32_t>(list, 0x8) = 0;
    while (node != at<void*>(list, 0x4)) {
        void* next = at<void*>(node, 0x0);
        host.crt_operator_delete_00bf65ac(node);
        node = next;
    }
}

}  // namespace

// 00583E50 BSP_MainMenu_ClearObjectiveRows
// __thiscall(ECX = main-menu screen) RET; no SEH frame.
// - When +2C8h is non-null: its vtable +34h(0), +2C8h = 0, then +244h and
//   +248h vtable +34h(0) (both only on this path).
// - When the byte +2C4h is set: +2B8h/+2BCh/+2C0h, when non-null, vtable
//   +4h(1); the widgets of the lists at +280h, +298h and +2A4h are deleted
//   (all three walks first), then the three lists are cleared in that order.
// - Always: byte +2C4h = 0; the list box +1B8h gets 00AA8240 with
//   {+558h + 0, +55Ch + 0, 0 + +560h} (the image adds an FLDZ to each, which
//   turns -0 into +0), then 00AA6BC0(+1B8h; screen+40h, 0).
void cc13_sweepB3_main_menu_clear_objective_rows_00583e50(void* screen, Cc13SweepB3Host& host) {
    if (at<void*>(screen, 0x2C8) != nullptr) {
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x2C8), 0);
        at<void*>(screen, 0x2C8) = nullptr;
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x244), 0);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x248), 0);
    }
    if (at<std::uint8_t>(screen, 0x2C4) != 0) {
        for (std::size_t offset : {std::size_t{0x2B8}, std::size_t{0x2BC}, std::size_t{0x2C0}}) {
            void* header = at<void*>(screen, offset);
            if (header != nullptr) host.object_vcall_04_delete(header, 1);
        }
        delete_list_widgets(bytes(screen) + 0x280, host);
        delete_list_widgets(bytes(screen) + 0x298, host);
        delete_list_widgets(bytes(screen) + 0x2A4, host);
        clear_list(bytes(screen) + 0x280, host);
        clear_list(bytes(screen) + 0x298, host);
        clear_list(bytes(screen) + 0x2A4, host);
    }
    at<std::uint8_t>(screen, 0x2C4) = 0;
    const float zero = 0.0f;
    float position[3];
    position[0] = at<float>(screen, 0x558) + zero;
    position[1] = at<float>(screen, 0x55C) + zero;
    position[2] = zero + at<float>(screen, 0x560);
    host.widget_set_resolved_position_00aa8240(at<void*>(screen, 0x1B8), position);
    host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, 0x1B8), bytes(screen) + 0x40, 0);
}

// 00585C50 BSP_MainMenuScreen_PlayTestSpeech
// __thiscall(ECX = main-menu screen, name) RET 4; SEH frame (handler
// 00C6FB7E). The single stack argument is passed on to 00A83FD0 and its slot
// is then reused as the out reference of 00A7E490 (the image releases it at
// the end, without storing it back anywhere).
// - 00A83FD0(cache *(00F8BBE8); &tmp, name); the returned reference's value is
//   ref-assigned to screen+570h; tmp is released.
// - Once (bit 1 of 00E19510): 00E1950C = 00A7ACF0(*(00F8BBD8), "GUITestSpeech")
//   on a native string built by 0041E870.
// - "Normal" (00CECD20, length 6) as a temporary string; sample =
//   *(*(00E198AC)+58h)+570h read BEFORE 00A7B0A0; type = 00A7B0A0(manager,
//   string); 00A7E490(manager; &slot, sample, *(00E1950C), type, 1); the
//   result's value is ref-assigned to *(00E198AC)+58h's +574h (re-read after
//   the call); the slot is released; the string is released.
// Note the asymmetry kept from the image: the sample is stored into ECX+570h
// but the tracked sound is made from (and stored into) the screen at
// *(00E198AC)+58h.
void cc13_sweepB3_main_menu_play_test_speech_00585c50(void* screen, void* name, Cc13SweepB3Host& host) {
    void* sample = nullptr;
    void** acquired = host.sound_sample_cache_acquire_00a83fd0(image_pointer(host, 0x00F8BBE8), &sample, name);
    ref_assign(&at<void*>(screen, 0x570), *acquired, host);
    ref_release(sample, host);

    std::uint32_t& guard = *static_cast<std::uint32_t*>(host.image_global(0x00E19510));
    if ((guard & 1) == 0) {
        guard |= 1;
        Cc13SweepBNativeString class_name = {0, nullptr};  // 0041E870 constructs it (uninitialised in the image)
        host.native_string_assign_0041e870(&class_name, static_cast<const char*>(host.image_global(0x00CEF890)));
        *static_cast<std::int32_t*>(host.image_global(0x00E1950C)) =
            host.sound_find_class_00a7acf0(image_pointer(host, 0x00F8BBD8), &class_name);
        release_string(class_name, host);
    }

    with_c_string(static_cast<const char*>(host.image_global(0x00CECD20)), 6, host,
                  [&](Cc13SweepBNativeString* type_name) {
        void* manager = image_pointer(host, 0x00F8BBD8);
        void* speech_sample = at<void*>(at<void*>(image_pointer(host, 0x00E198AC), 0x58), 0x570);
        const std::int32_t type = host.sound_find_type_00a7b0a0(manager, type_name);
        void* slot = name;  // the argument slot, overwritten by 00A7E490
        void** created = host.sound_create_tracked_00a7e490(
            manager, &slot, speech_sample, *static_cast<std::int32_t*>(host.image_global(0x00E1950C)), type, 1);
        ref_assign(&at<void*>(at<void*>(image_pointer(host, 0x00E198AC), 0x58), 0x574), *created, host);
        ref_release(slot, host);
    });
}

// 005A2340 BSP_HudMap_CreateBuildingQuadModel
// __cdecl() RET (ECX is not read); EAX = a new 1Ch-byte block (operator new
// 00BF681B) whose +0h is set to the model and nothing else; SEH frame
// (handler 00C714B9). Sole caller 005A91E0.
// - model = 00B75030(00B74EB0(), "StoredQuad") when the pool block is
//   non-null, else null (the name string is released only on that path).
// - geometry = 00B73D70(00B73B60()) when non-null, else null.
// - resource = renderer *(00F8D394) vtable +38h("pf43.mvfm"); stream =
//   renderer vtable +5Ch(4, 1, resource); resource released WITHOUT a null
//   check (InterlockedDecrement(+4h), vtable +0h at zero), as in the image.
// - 00B73BB0(geometry; 0, stream); stream released (no null check); then
//   the stream is still used: vtable +10h(0, 0, 0) gives 12 floats
//   {-550, 0, 550, 550, 0, 550, -550, 0, -550, 550, 0, -550} (00CF0048 /
//   00CF0044).
// - section = 00533FA0(); material = 00535320("map_cmd_building.mshd");
//   section +8h = 5, +10h = 4, +18h = 2; stream vtable +14h();
//   00B864C0(section, material); material released; 00B865A0(section,
//   geometry); 00B73C60(geometry, section); section released.
// - 00B75170(model; 0, geometry, -1.0f, -1.0f); geometry released.
void* cc13_sweepB3_hud_map_create_building_quad_model_005a2340(Cc13SweepB3Host& host) {
    auto drop = [&](void* object) {  // the image's unchecked ref release
        if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(object, 0x4)) == 0) {
            host.refcounted_vcall_00_destroy(object);
        }
    };
    void* holder = host.crt_operator_new_00bf681b(0x1C);
    void* model = nullptr;
    void* model_block = host.generated_model_pool_alloc_00b74eb0();
    if (model_block != nullptr) {
        Cc13SweepBNativeString name = {0, nullptr};  // constructed by 0041E870 in the image
        host.native_string_assign_0041e870(&name, static_cast<const char*>(host.image_global(0x00CF004C)));
        model = host.generated_model_construct_00b75030(model_block, &name);
        release_string(name, host);
    }
    void* geometry_block = host.mesh_object_pool_alloc_00b73b60();
    void* geometry = geometry_block != nullptr ? host.mesh_object_construct_00b73d70(geometry_block) : nullptr;
    void* resource = nullptr;
    with_c_string(static_cast<const char*>(host.image_global(0x00CED5A0)), 9, host, [&](Cc13SweepBNativeString* name) {
        resource = host.renderer_vcall_38_load(host.renderer_00f8d394(), name);
    });
    void* stream = host.renderer_vcall_5c_create_stream(host.renderer_00f8d394(), 4, 1, resource);
    drop(resource);
    host.geometry_set_vertex_stream_00b73bb0(geometry, 0, stream);
    drop(stream);
    float* vertices = host.stream_vcall_10_lock(stream, 0, 0, 0);
    const float lo = f32_bits(0xC4098000u);  // 00CF0048 -550.0f
    const float hi = f32_bits(0x44098000u);  // 00CF0044 550.0f
    const float quad[12] = {lo, 0.0f, hi, hi, 0.0f, hi, lo, 0.0f, lo, hi, 0.0f, lo};
    for (std::size_t i = 0; i < 12; ++i) vertices[i] = quad[i];
    void* section = host.draw_section_create_00533fa0();
    void* material = nullptr;
    with_c_string(static_cast<const char*>(host.image_global(0x00CF002C)), 0x15, host,
                  [&](Cc13SweepBNativeString* name) { material = host.material_create_for_effect_name_00535320(name); });
    at<std::uint32_t>(section, 0x8) = 5;
    at<std::uint32_t>(section, 0x10) = 4;
    at<std::uint32_t>(section, 0x18) = 2;
    host.stream_vcall_14_unlock(stream);
    host.draw_section_set_material_00b864c0(section, material);
    drop(material);
    host.draw_section_rebuild_layout_00b865a0(section, geometry);
    host.geometry_append_draw_section_00b73c60(geometry, section);
    drop(section);
    const float minus_one = f32_bits(0xBF800000u);  // 00D7A260
    host.generated_model_set_geometry_00b75170(model, 0, geometry, minus_one, minus_one);
    drop(geometry);
    at<void*>(holder, 0x0) = model;
    return holder;
}

// 005A3610 BSP_HudMapArrowWidget_Construct
// __thiscall(ECX = 28h-byte block) RET; EAX = the block; SEH frame (handler
// 00C71588), aligned EBP frame. Sole caller 005A4A00.
// - vtable 00CEFFE4 at +0h.
// - +4h = 00AA6640(kind 4, "" (00CE3A0C, length 0), 0); its z 0.0f
//   (00AA7D40); vtable +34h(0); attached with 00AAA5A0 to
//   *(*(*(00E198C4)+54h)+94h) with 0.
// - Eight times, +8h..+24h: 00AA6640(kind 0Dh, "", 0); z 0.0f; vtable
//   +34h(0); texture = the first entry of the table at 00E08884 (a .data
//   pointer, "gui/map/arrow_ai.tga" in the image) through 00ABDD80 on a
//   temporary string; attached to the same parent.
void* cc13_sweepB3_hud_map_arrow_widget_construct_005a3610(void* widget, Cc13SweepB3Host& host) {
    at<std::uint32_t>(widget, 0x0) = 0x00CEFFE4u;  // the image's vtable address, kept as a value
    const char* empty = static_cast<const char*>(host.image_global(0x00CE3A0C));
    with_c_string(empty, 0, host, [&](Cc13SweepBNativeString* text) {
        at<void*>(widget, 0x4) = host.gui_widget_create_with_scene_node_00aa6640(4, text, 0);
    });
    host.gui_widget_set_local_z_00aa7d40(at<void*>(widget, 0x4), 0.0f);
    host.widget_vcall_34_set_visible(at<void*>(widget, 0x4), 0);
    host.gui_widget_attach_child_00aaa5a0(at<void*>(at<void*>(host.global_00e198c4(), 0x54), 0x94),
                                          at<void*>(widget, 0x4), 0);
    for (std::size_t i = 0; i < 8; ++i) {
        void*& curve = at<void*>(widget, 0x8 + 4 * i);
        with_c_string(empty, 0, host, [&](Cc13SweepBNativeString* text) {
            curve = host.gui_widget_create_with_scene_node_00aa6640(0xD, text, 0);
        });
        host.gui_widget_set_local_z_00aa7d40(curve, 0.0f);
        host.widget_vcall_34_set_visible(curve, 0);
        const char* texture = *static_cast<const char**>(host.image_global(0x00E08884));
        with_c_string(texture, static_cast<std::uint32_t>(std::strlen(texture)), host,
                      [&](Cc13SweepBNativeString* text) { host.gui_curve_set_texture_00abdd80(curve, text); });
        host.gui_widget_attach_child_00aaa5a0(at<void*>(at<void*>(host.global_00e198c4(), 0x54), 0x94), curve, 0);
    }
    return widget;
}

// 005A3830 BSP_HudMapArrowWidget_SetAiArrow
// __thiscall(ECX = arrow widget; screen, const float* from, const float* to,
// mode, style) RET 14h; SEH frame (handler 00C715A8). Sole caller 005A9DB0.
// The earlier ledger text "(order, worldPoint*, style)" under-counts: RET 14h
// and the reads at ESP+50h..60h give five stack arguments.
// - Hides +4h and the eight curves +8h..+24h (vtable +34h(0)).
// - off = (*(00E188A8))+61Fh byte != 0 ? 1000 : 0 (CVTSI2SS).
// - Map coordinates, from the bytes (DE-form operands decoded by encoding):
//   x = (p[0] - screen+54Ch) / screen+30h + *(00E08890),
//   y = (screen+550h - p[2]) / screen+34h + *(00E08894) (both .data floats,
//   0.5 in the image), for from and to; each stored as float.
// - Curve +8h: 00ABC920 clear; 00ABC970({off + x, y + 0.0 (double)},
//   width = screen+5CCh * 0.019999999552965164 (double)) for from, then to;
//   vtable +50h(mode); texture = table 00E08884[style] (0041E870 string,
//   00ABDD80, released); 00ABCA10 rebuild; vtable +34h(1).
// - Curves +0Ch..+24h: vtable +34h(0) again.
void cc13_sweepB3_hud_map_arrow_set_ai_arrow_005a3830(void* widget, void* screen, const float* from,
                                                      const float* to, std::int32_t mode, std::int32_t style,
                                                      Cc13SweepB3Host& host) {
    host.widget_vcall_34_set_visible(at<void*>(widget, 0x4), 0);
    for (std::size_t i = 0; i < 8; ++i) host.widget_vcall_34_set_visible(at<void*>(widget, 0x8 + 4 * i), 0);

    const float off = at<std::uint8_t>(host.global_00e188a8(), 0x61F) != 0 ? 1000.0f : 0.0f;
    const float origin_x = at<float>(screen, 0x54C);
    const float scale_x = at<float>(screen, 0x30);
    const float origin_y = at<float>(screen, 0x550);
    const float scale_y = at<float>(screen, 0x34);
    const float half_x = *static_cast<float*>(host.image_global(0x00E08890));
    const float half_y = *static_cast<float*>(host.image_global(0x00E08894));
    const float from_x = (from[0] - origin_x) / scale_x + half_x;
    const float from_y = (origin_y - from[2]) / scale_y + half_y;
    const float to_x = (to[0] - origin_x) / scale_x + half_x;
    const float to_y = (origin_y - to[2]) / scale_y + half_y;

    host.gui_curve_clear_points_00abc920(at<void*>(widget, 0x8));
    const double zero = f64_bits(0x0000000000000000ull);       // 00D7A258
    const double width_scale = f64_bits(0x3F947AE140000000ull);  // 00D7A2F8
    float point[2];
    point[0] = off + from_x;
    point[1] = static_cast<float>(static_cast<double>(from_y) + zero);
    float width = static_cast<float>(static_cast<double>(at<float>(screen, 0x5CC)) * width_scale);
    host.gui_curve_add_point_00abc970(at<void*>(widget, 0x8), point, width);
    point[0] = off + to_x;
    point[1] = static_cast<float>(static_cast<double>(to_y) + zero);
    width = static_cast<float>(static_cast<double>(at<float>(screen, 0x5CC)) * width_scale);
    host.gui_curve_add_point_00abc970(at<void*>(widget, 0x8), point, width);

    host.widget_vcall_50(at<void*>(widget, 0x8), mode);
    Cc13SweepBNativeString texture = {0, nullptr};  // 0041E870 constructs it (uninitialised in the image)
    host.native_string_assign_0041e870(
        &texture, *static_cast<const char**>(host.image_global(0x00E08884 + 4 * static_cast<std::uint32_t>(style))));
    host.gui_curve_set_texture_00abdd80(at<void*>(widget, 0x8), &texture);
    release_string(texture, host);
    host.gui_curve_rebuild_00abca10(at<void*>(widget, 0x8));
    host.widget_vcall_34_set_visible(at<void*>(widget, 0x8), 1);
    for (std::size_t i = 0; i < 7; ++i) host.widget_vcall_34_set_visible(at<void*>(widget, 0xC + 4 * i), 0);
}

// 005A3BF0 BSP_HudMapObject_00cf00ac_Destruct
// __thiscall(ECX = HUD map object) RET; SEH frame (handler 00C71673). Caller
// 005A4AB0 (scalar deleting destructor).
// - vtable 00CF00ACh at +0h.
// - Ref pointers +0Ch and +10h: when non-null, InterlockedDecrement(+4h) and,
//   at zero, vtable +0h; then the slot = 0 (only on that path).
// - Owned objects, each when non-null deleted through vtable +4h with 1 and
//   cleared: +18h, +1Ch, +48h..+60h (7), +64h..+6Ch (3), +20h; then +24h
//   through vtable +0h with 1 and cleared.
// - 005A1C60(object, 0).
// - Vector storage released (operator delete when non-null, the three
//   pointers zeroed) at +194h, +184h, +174h, +164h, +154h, +0ECh, +0DCh,
//   +0CCh, +0BCh, +0ACh, +9Ch, +8Ch, +7Ch, in that order.
void cc13_sweepB3_hud_map_object_destruct_005a3bf0(void* object, Cc13SweepB3Host& host) {
    at<std::uint32_t>(object, 0x0) = 0x00CF00ACu;  // the image's vtable address, kept as a value
    for (std::size_t offset : {std::size_t{0xC}, std::size_t{0x10}}) {
        void* reference = at<void*>(object, offset);
        if (reference != nullptr) {
            ref_release(reference, host);
            at<void*>(object, offset) = nullptr;
        }
    }
    auto delete_owned = [&](std::size_t offset) {
        void* owned = at<void*>(object, offset);
        if (owned != nullptr) {
            host.object_vcall_04_delete(owned, 1);
            at<void*>(object, offset) = nullptr;
        }
    };
    delete_owned(0x18);
    delete_owned(0x1C);
    for (std::size_t i = 0; i < 7; ++i) delete_owned(0x48 + 4 * i);
    for (std::size_t i = 0; i < 3; ++i) delete_owned(0x64 + 4 * i);
    delete_owned(0x20);
    if (at<void*>(object, 0x24) != nullptr) {
        host.object_vcall_00_delete(at<void*>(object, 0x24), 1);
        at<void*>(object, 0x24) = nullptr;
    }
    host.hud_map_icon_release_widgets_005a1c60(object, 0);
    for (std::size_t offset : {0x194, 0x184, 0x174, 0x164, 0x154, 0xEC, 0xDC, 0xCC, 0xBC, 0xAC, 0x9C, 0x8C, 0x7C}) {
        void* storage = at<void*>(object, offset);
        if (storage != nullptr) host.crt_operator_delete_00bf65ac(storage);
        at<void*>(object, offset) = nullptr;
        at<void*>(object, offset + 4) = nullptr;
        at<void*>(object, offset + 8) = nullptr;
    }
}

// 005A8F50 BSP_HudMapScreen_AcquireLineModel
// __thiscall(ECX = HUD map screen; const float* frame, float scale, layer)
// RET 0Ch; returns nothing (EAX is whatever 00B6D890 left). No SEH frame.
// Pool: checked dword vector at +4C8h (+4CCh first, +4D0h last, +4D4h end),
// next free index +4D8h.
// - Empty pool: push_back (00506DD0) of 005A2C30(5000.0f).
// - While the pool holds no more than the index (unsigned compare): a clone
//   of the first model (vtable +10h(6, 0)) is appended, by the inlined fast
//   path when there is capacity, else 00505D50 at end (first > last calls
//   00BF6713 first). An empty pool here calls 00BF6713.
// - model = pool[index] (index past the end calls 00BF6713).
// - frame' = (diag(scale, 1, scale, 1) * frame) * T, T = identity with
//   row 3 = {0, z, 0, 1}, z = (float)(uint32)(layer * 10 + 3000) (FILD plus
//   2^32 when negative); both products through 00413920.
// - model vtable +38h(frame'); 00B17310(00B732C0(00B74640(model, 0), 0)+20h,
//   layer); 00B6D890(model, screen+0Ch); index += 1.
void cc13_sweepB3_hud_map_acquire_line_model_005a8f50(void* screen, const float* frame, float scale,
                                                      std::uint32_t layer, Cc13SweepB3Host& host) {
    std::uint8_t* pool = bytes(screen) + 0x4C8;
    auto first = [&]() { return at<std::uint8_t*>(pool, 0x4); };
    auto count = [&]() {
        return static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(pool, 0x8) - first()) >> 2);
    };
    if (first() == nullptr || count() == 0) {
        void* model = host.hud_map_create_simple_color_model_005a2c30(f32_bits(0x459C4000u));  // 00CEFF70 5000.0f
        host.dword_vector_push_back_00506dd0(pool, &model);
    }
    while (first() == nullptr || !(at<std::uint32_t>(screen, 0x4D8) < count())) {
        if (first() == nullptr || count() == 0) host.crt_invalid_parameter_noinfo_00bf6713();
        void* clone = host.model_vcall_10_clone(at<void*>(first(), 0x0), 6, 0);
        const std::uint32_t size = first() == nullptr ? 0 : count();
        if (first() != nullptr &&
            size < static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(pool, 0xC) - first()) >> 2)) {
            at<void*>(at<std::uint8_t*>(pool, 0x8), 0x0) = clone;
            at<std::uint8_t*>(pool, 0x8) += 4;
        } else {
            std::uint8_t* last = at<std::uint8_t*>(pool, 0x8);
            if (first() > last) host.crt_invalid_parameter_noinfo_00bf6713();
            void* out_iterator[2];
            host.dword_vector_insert_00505d50(pool, out_iterator, pool, last, &clone);
        }
    }
    const std::uint32_t index = at<std::uint32_t>(screen, 0x4D8);
    if (first() == nullptr || !(index < count())) host.crt_invalid_parameter_noinfo_00bf6713();
    void* model = at<void*>(first(), 4 * index);

    const float one = f32_bits(0x3F800000u);  // 00D7A24C
    const float scaling[16] = {scale, 0, 0, 0, 0, one, 0, 0, 0, 0, scale, 0, 0, 0, 0, one};
    float z;
    {
        const std::uint32_t raw = (layer * 5 + 0x5DC) * 2;
        double wide = static_cast<double>(static_cast<std::int32_t>(raw));
        if (static_cast<std::int32_t>(raw) < 0) wide += static_cast<double>(f32_bits(0x4F800000u));  // 00CE3978
        z = static_cast<float>(wide);
    }
    const float lift[16] = {one, 0, 0, 0, 0, one, 0, 0, 0, 0, one, 0, 0, z, 0, one};
    float scaled[16];
    float placed[16];
    const float* left = host.matrix_multiply_00413920(scaling, scaled, frame);
    const float* result = host.matrix_multiply_00413920(left, placed, lift);
    host.model_vcall_38_set_frame(model, result);
    void* element = host.mesh_element_00b732c0(host.generated_model_geometry_00b74640(model, 0), 0);
    host.material_set_alpha_ref_00b17310(at<void*>(element, 0x20), layer);
    host.node_propagate_root_00b6d890(model, at<void*>(screen, 0xC));
    at<std::uint32_t>(screen, 0x4D8) += 1;
}

// 005C0BD0 BSP_HudMinimapScreen_HideUnusedMarkers
// __thiscall(ECX = HUD minimap screen) RET; no SEH frame. Callers 005C0E10
// (Exit) and 005C0F20 (Update).
// Five checked pointer vectors (+4h first, +8h last) with their used-count
// cursors: +104h/+154h, +114h/+158h, +124h/+15Ch, +134h/+160h, +144h/+164h.
// For each: i runs from cursor + 1 while first is non-null and i < count
// (unsigned; first and last re-read each step); a non-null slot is deleted
// through its vtable +0h with 1 and cleared; then 005C0390(vector, cursor + 1,
// 0) (the cursor re-read). The image's repeated bounds check before the slot
// read (00BF6713) is kept although the loop condition already holds.
void cc13_sweepB3_hud_minimap_hide_unused_markers_005c0bd0(void* screen, Cc13SweepB3Host& host) {
    static constexpr std::size_t kVectors[5][2] = {
        {0x104, 0x154}, {0x114, 0x158}, {0x124, 0x15C}, {0x134, 0x160}, {0x144, 0x164}};
    for (const auto& pair : kVectors) {
        std::uint8_t* vector = bytes(screen) + pair[0];
        auto count = [&]() {
            return static_cast<std::uint32_t>(
                static_cast<std::int32_t>(at<std::uint8_t*>(vector, 0x8) - at<std::uint8_t*>(vector, 0x4)) >> 2);
        };
        std::uint32_t i = at<std::uint32_t>(screen, pair[1]);
        for (;;) {
            i += 1;
            if (at<std::uint8_t*>(vector, 0x4) == nullptr || !(i < count())) break;
            if (at<std::uint8_t*>(vector, 0x4) == nullptr || !(i < count())) host.crt_invalid_parameter_noinfo_00bf6713();
            void*& slot = at<void*>(at<std::uint8_t*>(vector, 0x4), 4 * static_cast<std::size_t>(i));
            if (slot != nullptr) {
                host.object_vcall_00_delete(slot, 1);
                slot = nullptr;
            }
        }
        host.dword_vector_resize_005c0390(vector, at<std::uint32_t>(screen, pair[1]) + 1, 0);
    }
}

// ===========================================================================
// Segment 23
// ===========================================================================

namespace {

// 005E0DB0's inlined pass over a checked pointer vector (+0h first, +4h
// last): each non-null element is deleted through its vtable +4h with 1 and
// cleared (bounds re-checked against 00BF6713 as in the image); then the
// inlined erase(begin(), end()): first > last calls 00BF6713 (twice, as the
// image checks the iterator pair and then the range); when first != the end
// iterator, the tail (none, since the range is the whole vector) is moved with
// memmove_s 00BF67A7 when its count is positive and last = first + tail.
void lobby_delete_and_erase(std::uint8_t* vector, Cc13SweepB3Host& host) {
    auto count = [&]() {
        return static_cast<std::uint32_t>(
            static_cast<std::int32_t>(at<std::uint8_t*>(vector, 0x4) - at<std::uint8_t*>(vector, 0x0)) >> 2);
    };
    for (std::uint32_t i = 0;; ++i) {
        if (at<std::uint8_t*>(vector, 0x0) == nullptr || !(i < count())) break;
        if (at<std::uint8_t*>(vector, 0x0) == nullptr || !(i < count())) host.crt_invalid_parameter_noinfo_00bf6713();
        void*& slot = at<void*>(at<std::uint8_t*>(vector, 0x0), 4 * static_cast<std::size_t>(i));
        if (slot != nullptr) {
            host.object_vcall_04_delete(slot, 1);
            slot = nullptr;
        }
    }
    std::uint8_t* end = at<std::uint8_t*>(vector, 0x4);
    if (at<std::uint8_t*>(vector, 0x0) > end) host.crt_invalid_parameter_noinfo_00bf6713();
    std::uint8_t* begin = at<std::uint8_t*>(vector, 0x0);
    if (begin > at<std::uint8_t*>(vector, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
    if (begin != end) {
        const std::int32_t tail = static_cast<std::int32_t>(at<std::uint8_t*>(vector, 0x4) - end) >> 2;
        const std::uint32_t tail_bytes = static_cast<std::uint32_t>(tail) * 4;
        if (tail > 0) host.crt_memmove_s_00bf67a7(begin, tail_bytes, end, tail_bytes);
        at<std::uint8_t*>(vector, 0x4) = begin + tail_bytes;
    }
}

// The inlined vector storage release: operator delete 00BF65AC of first when
// non-null, then first/last/end = 0.
void release_vector_storage(std::uint8_t* vector, Cc13SweepB3Host& host) {
    if (at<void*>(vector, 0x0) != nullptr) host.crt_operator_delete_00bf65ac(at<void*>(vector, 0x0));
    at<void*>(vector, 0x0) = nullptr;
    at<void*>(vector, 0x4) = nullptr;
    at<void*>(vector, 0x8) = nullptr;
}

}  // namespace

// 005E0DB0 BSP_MultiGameLobbyScreen_Destruct
// __thiscall(ECX = lobby screen) RET; SEH frame (handler 00C74CB7). Caller
// 005E11D0 (scalar deleting destructor).
// - vtables 00CF23E4 / 00CF23D0 / 00CF23B0 at +0h/+8h/+0Ch.
// - 005CFB80(*(*(00E198B4)+54h)) clears the chat logs.
// - Lists +134h and +0B4h: 00AA83F0, then their scrollers +158h / +0CCh get
//   006834A0(0.0f) and 00683790(0).
// - Vectors +1F8h and +244h: lobby_delete_and_erase.
// - Pages +14h and +18h: 00AA31F0(004C12B0(), page) each.
// - Vector +2E4h of 24h-byte entries: when non-null, 005DACA0 on each entry
//   from first to last, then operator delete; +2E4h..+2ECh = 0.
// - Storage of +244h, +234h, +1F8h released (in that order).
// - vtable 00CEB0FC at +8h; 004F71A0 base destructor.
void cc13_sweepB3_multi_game_lobby_destruct_005e0db0(void* screen, Cc13SweepB3Host& host) {
    at<std::uint32_t>(screen, 0x0) = 0x00CF23E4u;  // image vtable addresses, kept as values
    at<std::uint32_t>(screen, 0x8) = 0x00CF23D0u;
    at<std::uint32_t>(screen, 0xC) = 0x00CF23B0u;
    host.multi_chat_clear_logs_005cfb80(at<void*>(image_pointer(host, 0x00E198B4), 0x54));
    host.gui_list_clear_00aa83f0(at<void*>(screen, 0x134));
    host.gui_scroller_set_scroll_range_006834a0(bytes(screen) + 0x158, 0.0f);
    host.gui_scroller_attach_content_00683790(bytes(screen) + 0x158, 0);
    host.gui_list_clear_00aa83f0(at<void*>(screen, 0xB4));
    host.gui_scroller_set_scroll_range_006834a0(bytes(screen) + 0xCC, 0.0f);
    host.gui_scroller_attach_content_00683790(bytes(screen) + 0xCC, 0);
    lobby_delete_and_erase(bytes(screen) + 0x1F8, host);
    lobby_delete_and_erase(bytes(screen) + 0x244, host);
    host.gui_manager_remove_page_00aa31f0(host.gui_manager_get_004c12b0(), at<void*>(screen, 0x14));
    host.gui_manager_remove_page_00aa31f0(host.gui_manager_get_004c12b0(), at<void*>(screen, 0x18));
    std::uint8_t* entries = at<std::uint8_t*>(screen, 0x2E4);
    if (entries != nullptr) {
        std::uint8_t* last = at<std::uint8_t*>(screen, 0x2E8);
        for (std::uint8_t* entry = entries; entry != last; entry += 0x24) host.lobby_map_entry_destruct_005daca0(entry);
        host.crt_operator_delete_00bf65ac(at<void*>(screen, 0x2E4));
    }
    at<void*>(screen, 0x2E4) = nullptr;
    at<void*>(screen, 0x2E8) = nullptr;
    at<void*>(screen, 0x2EC) = nullptr;
    release_vector_storage(bytes(screen) + 0x244, host);
    release_vector_storage(bytes(screen) + 0x234, host);
    release_vector_storage(bytes(screen) + 0x1F8, host);
    at<std::uint32_t>(screen, 0x8) = 0x00CEB0FCu;
    host.frontend_screen_destruct_004f71a0(screen);
}

namespace {

// The option map 005E3290 reads for each (key, value): the local player's
// record map when *(00E198AC) and its +5Ch are non-null and the record's byte
// +424h is set, else registry(00576B10)+28h. The record is
// vector(*(00E198AC)+5Ch+24h)[005D7070(*(00E19594))], 434h bytes each; a null
// vector or an index at or past the (signed) element count calls 00BF6713.
void* lobby_option_map(Cc13SweepB3Host& host) {
    void* frontend = image_pointer(host, 0x00E198AC);
    if (frontend != nullptr && at<void*>(frontend, 0x5C) != nullptr) {
        std::uint8_t* players = static_cast<std::uint8_t*>(at<void*>(frontend, 0x5C)) + 0x24;
        const std::uint32_t index =
            host.lobby_player_index_005d7070(*static_cast<std::uint32_t*>(host.image_global(0x00E19594)));
        std::uint8_t* first = at<std::uint8_t*>(players, 0x4);
        if (first == nullptr ||
            !(index < static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(players, 0x8) - first) /
                                                 0x434))) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        std::uint8_t* record = at<std::uint8_t*>(players, 0x4) + index * 0x434;
        if (at<std::uint8_t>(record, 0x424) != 0) return record + 0x428;
    }
    return static_cast<std::uint8_t*>(host.lobby_option_registry_00576b10()) + 0x28;
}

}  // namespace

// 005E3290 BSP_MultiGameLobbyScreen_Construct
// __thiscall(ECX = 300h-byte block) RET; EAX = the block; SEH frame (handler
// 00C74F9B). Callers 00688E40, 00689540.
// - 004F7180 base constructor; base vtables 00CEB0FC/00CEB110 at +8h/+0Ch, then
//   00CF23E4 at +0h, 00CF23D0 at +8h, 00CF23B0 at +0Ch.
// - 00683610 scrollers at +30h, +0CCh, +158h; dwords +1F8h..+200h,
//   +234h..+23Ch, +244h..+24Ch, +2E4h..+2ECh and +1Ch = 0; bytes +270h, +271h = 0.
// - 00E19594 = *(*(00E188A8)+60Ch).
// - For key 0..13 (key and value live in stack slots passed by address):
//   key 8: value counts up from 0 while value < count (the record from
//     00575E80(registry+10h, key), +8h, re-fetched each step, unsigned); the
//     first value whose flag 005C49C0(005C6850(option map, key), value) is 1
//     goes to 00E1957C, and the walk stops when it equals registry+60h;
//   other keys: value counts down from count - 1 while >= 0 (signed); a flag
//     of 1 stores value to 00E1955C[key] and the walk stops when it equals
//     registry+40h+4*key.
// - Finally, when *(00F8A2FC) is non-null, its byte +75h = 0.
void* cc13_sweepB3_multi_game_lobby_construct_005e3290(void* screen, Cc13SweepB3Host& host) {
    host.frontend_screen_construct_004f7180(screen);
    at<std::uint32_t>(screen, 0x8) = 0x00CEB0FCu;  // image vtable addresses, kept as values
    at<std::uint32_t>(screen, 0xC) = 0x00CEB110u;
    at<std::uint32_t>(screen, 0x0) = 0x00CF23E4u;
    at<std::uint32_t>(screen, 0x8) = 0x00CF23D0u;
    at<std::uint32_t>(screen, 0xC) = 0x00CF23B0u;
    host.gui_scroller_construct_00683610(bytes(screen) + 0x30);
    host.gui_scroller_construct_00683610(bytes(screen) + 0xCC);
    host.gui_scroller_construct_00683610(bytes(screen) + 0x158);
    for (std::size_t offset : {0x1F8, 0x1FC, 0x200, 0x234, 0x238, 0x23C, 0x244, 0x248, 0x24C}) {
        at<std::uint32_t>(screen, offset) = 0;
    }
    at<std::uint8_t>(screen, 0x270) = 0;
    at<std::uint8_t>(screen, 0x271) = 0;
    at<std::uint32_t>(screen, 0x2E4) = 0;
    at<std::uint32_t>(screen, 0x2E8) = 0;
    at<std::uint32_t>(screen, 0x2EC) = 0;
    at<std::uint32_t>(screen, 0x1C) = 0;
    *static_cast<std::uint32_t*>(host.image_global(0x00E19594)) = at<std::uint32_t>(host.global_00e188a8(), 0x60C);

    auto registry = [&]() { return static_cast<std::uint8_t*>(host.lobby_option_registry_00576b10()); };
    for (std::int32_t key = 0; key < 14; ++key) {
        std::uint32_t value_slot = 0;
        if (key == 8) {
            std::uint32_t value = 0;
            value_slot = 0;
            if (at<std::uint32_t>(host.lobby_map_subscript_00575e80(registry() + 0x10, &key), 0x8) == 0) continue;
            for (;;) {
                void* inner = host.lobby_tree_subscript_005c6850(lobby_option_map(host), &key);
                const std::uint8_t* flag = host.lobby_flag_subscript_005c49c0(inner, &value_slot);
                if (*flag == 1) {
                    *static_cast<std::uint32_t*>(host.image_global(0x00E1957C)) = value;
                    if (value == at<std::uint32_t>(registry(), 0x60)) break;
                }
                value += 1;
                value_slot = value;
                if (!(value < at<std::uint32_t>(host.lobby_map_subscript_00575e80(registry() + 0x10, &key), 0x8))) {
                    break;
                }
            }
        } else {
            std::int32_t value =
                at<std::int32_t>(host.lobby_map_subscript_00575e80(registry() + 0x10, &key), 0x8) - 1;
            value_slot = static_cast<std::uint32_t>(value);
            while (value >= 0) {
                void* inner = host.lobby_tree_subscript_005c6850(lobby_option_map(host), &key);
                const std::uint8_t* flag = host.lobby_flag_subscript_005c49c0(inner, &value_slot);
                if (*flag == 1) {
                    static_cast<std::int32_t*>(host.image_global(0x00E1955C))[key] = value;
                    if (value == at<std::int32_t>(registry(), 0x40 + 4 * static_cast<std::size_t>(key))) break;
                }
                value -= 1;
                value_slot = static_cast<std::uint32_t>(value);
            }
        }
    }
    void* online = image_pointer(host, 0x00F8A2FC);
    if (online != nullptr) at<std::uint8_t>(online, 0x75) = 0;
    return screen;
}

// 005EA030 BSP_MultiMainMenuScreen_LayoutAchievementLocks
// __thiscall(ECX = multi main menu screen) RET; SEH frame (handler 00C754A0),
// aligned EBP frame. Callers 005EA700 and 005EA720.
// - base = 00AA67F0(+0DCh) copied; lock = 00AA67F0(00AA7E00(+0DCh,
//   "achievement_lock_Icon", 1)) copied (00CF2904, length 15h).
// - Walks the checked list at *(+0D8h)+68h (head; count +6Ch): for each
//   node whose widget (node+8h) has byte +76h set, with n = the placed count:
//   position {base.x + (float)(n % 3 * 0.084375001490116119),
//   base.y + (float)(n / 3 * 0.10277800261974335), base.z} (00AA7DC0), and
//   its "achievement_lock_Icon" child placed at lock. The node-is-head
//   checks call 00BF6713. The count is re-read through +0D8h each step.
// - +10h = rows = (n + 2) / 3; scroller +6Ch: 00683790(0), 006834A0((float)
//   ((rows - 3) * 0.10277800261974335)); then 005E9BE0(screen).
void cc13_sweepB3_multi_main_layout_achievement_locks_005ea030(void* screen, Cc13SweepB3Host& host) {
    const char* lock_name = static_cast<const char*>(host.image_global(0x00CF2904));
    const double column_step = f64_bits(0x3FB59999A0000000ull);  // 00CF29A0
    const double row_step = f64_bits(0x3FBA4FA8C0000000ull);     // 00CF2998

    const float* base_source = host.gui_widget_get_local_position_00aa67f0(at<void*>(screen, 0xDC));
    const float base[3] = {base_source[0], base_source[1], base_source[2]};
    std::int32_t placed = 0;
    float lock[3];
    with_c_string(lock_name, 0x15, host, [&](Cc13SweepBNativeString* text) {
        void* lock_template = host.widget_find_child_00aa7e00(at<void*>(screen, 0xDC), text, 1);
        const float* source = host.gui_widget_get_local_position_00aa67f0(lock_template);
        lock[0] = source[0];
        lock[1] = source[1];
        lock[2] = source[2];
    });

    void* list = at<void*>(screen, 0xD8);
    void* node = at<void*>(at<void*>(list, 0x68), 0x0);
    std::uint32_t index = 0;
    if (at<std::uint32_t>(list, 0x6C) != 0) {
        do {
            if (node == at<void*>(list, 0x68)) host.crt_invalid_parameter_noinfo_00bf6713();
            if (at<std::uint8_t>(at<void*>(node, 0x8), 0x76) != 0) {
                const std::int32_t row = placed / 3;
                const std::int32_t column = placed - row * 3;
                const float dx = static_cast<float>(static_cast<double>(column) * column_step);
                const float dy = static_cast<float>(static_cast<double>(row) * row_step);
                float position[3];
                position[2] = base[2];
                position[0] = base[0] + dx;
                position[1] = base[1] + dy;
                if (node == at<void*>(list, 0x68)) host.crt_invalid_parameter_noinfo_00bf6713();
                host.gui_widget_set_local_position_00aa7dc0(at<void*>(node, 0x8), position);
                with_c_string(lock_name, 0x15, host, [&](Cc13SweepBNativeString* text) {
                    if (node == at<void*>(list, 0x68)) host.crt_invalid_parameter_noinfo_00bf6713();
                    void* icon = host.widget_find_child_00aa7e00(at<void*>(node, 0x8), text, 1);
                    host.gui_widget_set_local_position_00aa7dc0(icon, lock);
                });
                placed += 1;
            }
            index += 1;
            if (node == at<void*>(list, 0x68)) host.crt_invalid_parameter_noinfo_00bf6713();
            node = at<void*>(node, 0x0);
        } while (index < at<std::uint32_t>(at<void*>(screen, 0xD8), 0x6C));
    }
    const std::int32_t rows = (placed + 2) / 3;
    at<std::int32_t>(screen, 0x10) = rows;
    host.gui_scroller_attach_content_00683790(bytes(screen) + 0x6C, 0);
    host.gui_scroller_set_scroll_range_006834a0(bytes(screen) + 0x6C,
                                                static_cast<float>(static_cast<double>(rows - 3) * row_step));
    host.multi_main_update_session_list_005e9be0(screen);
}

// ===========================================================================
// Segment 25
// ===========================================================================

// 006113A0 BSP_HudPowerupCarousel_AnimateScroll
// __thiscall(ECX = carousel, float dt) RET 4; no SEH frame. Sole caller
// 00619C50. Sibling of 00611770 on the carousel; screen =
// *(*(00E198C4)+0C0h), support = *(*(00E198C4)+0BCh), both re-read at each
// use; p = support+2A4h, s = support+2A8h, v = carousel+40h (re-read).
// - byte +3Fh clear -> nothing.
// - +40h = (float)(+40h - dt * 4.0); 0 when 0 > it.
// - byte +3Ch clear: screen+21Ch vtable +44h(p - s * v); screen+220h
//   +44h(s * v - p); 0065E870(support; screen+21Ch, v); acc = (float)((v +
//   1.0) * s).
//   byte +3Ch set (the image keeps the 4.0 from the decay on the x87 stack and
//   reuses it): +21Ch +44h(p - (4 * s + (1 - v) * s)); +220h +44h((1 - v) * s
//   + 4.0 * s - p); 0065E870(support; screen+21Ch, v); acc = (1 - v) * s.
// - Four rows i: screen+28h+4i +44h(p - acc); screen+48h+4i +44h(acc - p);
//   acc = s + acc.
// - 0065E870(support; screen+34h when byte +3Ch is clear, else screen+28h,
//   1 - v).
// - When v == 0.0f (ordered): screen+21Ch, when non-null, vtable +4h(1) (not
//   cleared); byte +3Fh = 0; support+100h and support+104h vtable +34h(1)
//   (the last as a tail jump).
void cc13_sweepB3_hud_powerup_carousel_animate_006113a0(void* carousel, float dt, Cc13SweepB3Host& host) {
    if (at<std::uint8_t>(carousel, 0x3F) == 0) return;
    const double four = f64_bits(0x4010000000000000ull);  // 00D7A328
    const float decayed = static_cast<float>(static_cast<double>(at<float>(carousel, 0x40)) - static_cast<double>(dt) * four);
    at<float>(carousel, 0x40) = decayed;
    if (0.0f > decayed) at<float>(carousel, 0x40) = 0.0f;

    auto screen = [&]() { return at<void*>(host.global_00e198c4(), 0xC0); };
    auto support = [&]() { return at<void*>(host.global_00e198c4(), 0xBC); };
    auto v = [&]() { return at<float>(carousel, 0x40); };
    const bool wrapped = at<std::uint8_t>(carousel, 0x3C) != 0;
    float acc;
    if (!wrapped) {
        host.widget_vcall_44_set_float(at<void*>(screen(), 0x21C),
                                       at<float>(support(), 0x2A4) - at<float>(support(), 0x2A8) * v());
        host.widget_vcall_44_set_float(at<void*>(screen(), 0x220),
                                       at<float>(support(), 0x2A8) * v() - at<float>(support(), 0x2A4));
        host.hud_support_set_large_icon_alpha_0065e870(support(), at<void*>(screen(), 0x21C), v());
        acc = static_cast<float>((static_cast<double>(v()) + f64_bits(0x3FF0000000000000ull)) *  // 00D7A210
                                 static_cast<double>(at<float>(support(), 0x2A8)));
    } else {
        float s = at<float>(support(), 0x2A8);
        host.widget_vcall_44_set_float(at<void*>(screen(), 0x21C),
                                       at<float>(support(), 0x2A4) - (static_cast<float>(four) * s + (1.0f - v()) * s));
        s = at<float>(support(), 0x2A8);
        host.widget_vcall_44_set_float(
            at<void*>(screen(), 0x220),
            static_cast<float>(static_cast<double>((1.0f - v()) * s) + static_cast<double>(s) * four) -
                at<float>(support(), 0x2A4));
        host.hud_support_set_large_icon_alpha_0065e870(support(), at<void*>(screen(), 0x21C), v());
        acc = (1.0f - v()) * at<float>(support(), 0x2A8);
    }
    for (std::size_t i = 0; i < 4; ++i) {
        host.widget_vcall_44_set_float(at<void*>(screen(), 0x28 + 4 * i), at<float>(support(), 0x2A4) - acc);
        host.widget_vcall_44_set_float(at<void*>(screen(), 0x48 + 4 * i), acc - at<float>(support(), 0x2A4));
        acc = at<float>(support(), 0x2A8) + acc;
    }
    void* other = at<void*>(screen(), wrapped ? 0x28 : 0x34);
    host.hud_support_set_large_icon_alpha_0065e870(support(), other, 1.0f - v());
    if (v() == f32_bits(0x00000000u)) {  // 00D7A218
        void* slider = at<void*>(screen(), 0x21C);
        if (slider != nullptr) host.object_vcall_04_delete(slider, 1);
        at<std::uint8_t>(carousel, 0x3F) = 0;
        host.widget_vcall_34_set_visible(at<void*>(support(), 0x100), 1);
        host.widget_vcall_34_set_visible(at<void*>(support(), 0x104), 1);
    }
}

// 00611770 BSP_HudPowerupManagerScreen_TickSlide
// __thiscall(ECX = powerup-manager screen, float dt) RET 4; no SEH frame. Sole
// caller 0061BEA0. The image stores intermediates into its own argument slot
// and the PUSH ECX slot; those are not observable and are kept as locals.
// - v = +1FCh; v == 0 (UCOMISS, ordered) -> nothing.
// - +1FCh = (float)(v - dt * 4.0); then 0 when 0 > it (FCOMI, ordered).
//   Let c be the new +1FCh (re-read at each use), support =
//   *(*(00E198C4)+0BCh) (re-read at each use), p = support+2A4h, s =
//   support+2A8h, w = +210h (read once before the branch).
// - byte +1F8h set: w vtable +44h(s * c + p); +214h vtable +44h(-p - s * c);
//   0065E870(support; w, c); acc = (float)((c + 1.0) * s).
//   byte +1F8h clear: the same with (5.0 - c) in place of c for the two
//   +44h calls (0065E870 still gets c); acc = (1 - c) * s.
// - Four rows i: +58h+4i vtable +44h(p + acc); +68h+4i vtable +44h(-p - acc);
//   acc = s + acc.
// - 0065E870(support; +64h when byte +1F8h is set, else +58h; 1 - c).
// - When +1FCh == 0.0f (00D7A218; ordered): +210h, when non-null, vtable
//   +4h(1) (deleted; the pointer is NOT cleared); byte +1F9h = 0; +0A0h and
//   +0A4h vtable +34h(1) (the last as a tail jump).
void cc13_sweepB3_hud_powerup_tick_slide_00611770(void* screen, float dt, Cc13SweepB3Host& host) {
    const float v = at<float>(screen, 0x1FC);
    if (v == 0.0f) return;
    const float decayed = static_cast<float>(static_cast<double>(v) - static_cast<double>(dt) * f64_bits(0x4010000000000000ull));  // 00D7A328
    at<float>(screen, 0x1FC) = decayed;
    if (0.0f > decayed) at<float>(screen, 0x1FC) = 0.0f;

    auto support = [&]() { return at<void*>(host.global_00e198c4(), 0xBC); };
    auto c = [&]() { return at<float>(screen, 0x1FC); };
    void* slider = at<void*>(screen, 0x210);
    const bool open = at<std::uint8_t>(screen, 0x1F8) != 0;
    float acc;
    if (open) {
        host.widget_vcall_44_set_float(slider, at<float>(support(), 0x2A8) * c() + at<float>(support(), 0x2A4));
        host.widget_vcall_44_set_float(at<void*>(screen, 0x214),
                                       -at<float>(support(), 0x2A4) - at<float>(support(), 0x2A8) * c());
        host.hud_support_set_large_icon_alpha_0065e870(support(), at<void*>(screen, 0x210), c());
        acc = static_cast<float>((static_cast<double>(c()) + f64_bits(0x3FF0000000000000ull)) *  // 00D7A210
                                 static_cast<double>(at<float>(support(), 0x2A8)));
    } else {
        const double five = f64_bits(0x4014000000000000ull);  // 00D7A370
        host.widget_vcall_44_set_float(
            slider, static_cast<float>((five - static_cast<double>(c())) * static_cast<double>(at<float>(support(), 0x2A8)) +
                                       static_cast<double>(at<float>(support(), 0x2A4))));
        host.widget_vcall_44_set_float(
            at<void*>(screen, 0x214),
            static_cast<float>(-static_cast<double>(at<float>(support(), 0x2A4)) -
                               (five - static_cast<double>(c())) * static_cast<double>(at<float>(support(), 0x2A8))));
        host.hud_support_set_large_icon_alpha_0065e870(support(), at<void*>(screen, 0x210), c());
        acc = (1.0f - c()) * at<float>(support(), 0x2A8);
    }
    for (std::size_t i = 0; i < 4; ++i) {
        host.widget_vcall_44_set_float(at<void*>(screen, 0x58 + 4 * i), at<float>(support(), 0x2A4) + acc);
        host.widget_vcall_44_set_float(at<void*>(screen, 0x68 + 4 * i), -at<float>(support(), 0x2A4) - acc);
        acc = at<float>(support(), 0x2A8) + acc;
    }
    void* other = at<void*>(screen, open ? 0x64 : 0x58);
    host.hud_support_set_large_icon_alpha_0065e870(support(), other, 1.0f - c());
    if (c() == f32_bits(0x00000000u)) {  // 00D7A218
        if (at<void*>(screen, 0x210) != nullptr) host.object_vcall_04_delete(at<void*>(screen, 0x210), 1);
        at<std::uint8_t>(screen, 0x1F9) = 0;
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xA0), 1);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xA4), 1);
    }
}

// 00614320 BSP_HudPowerupManagerScreen_BindSupportWidgets
// __thiscall(ECX = powerup-manager screen) RET; SEH frame (handler
// 00C77A50), aligned EBP frame. Sole caller 0061B850.
// support = *(*(00E198C4)+0BCh), re-read for every copy. For slot j = 0..3:
// - screen+28h+4j = support+7Ch+4j, +58h+4j = support+0ACh+4j,
//   +68h+4j = support+0BCh+4j, +48h+4j = support+8Ch+4j,
//   +78h+4j = support+0CCh+4j, +88h+4j = support+0DCh+4j (in that order).
// - For k = 0..3: name = 0043C130("circle_full_", 004260B0(k)) then
//   004261A0(name, "_Section") (temporaries released in the image's order:
//   the prefixed string, the number, the "_Section" string). In container
//   screen+28h+4j: k == j -> screen+38h+4j = 00AA7E00(..., name, 1), its
//   vtable +34h(0), then +4Ch(0.5f); else 00AA7E00(...)->vtable +34h(0). In
//   container screen+58h+4j: 00AA7E00(..., name, 1)->vtable +34h(0).
// Finally screen+0A0h = support+0F8h and screen+0A4h = support+0FCh.
void cc13_sweepB3_hud_powerup_bind_support_widgets_00614320(void* screen, Cc13SweepB3Host& host) {
    auto support = [&]() { return at<void*>(host.global_00e198c4(), 0xBC); };
    for (std::int32_t j = 0; j < 4; ++j) {
        const std::size_t s = 4 * static_cast<std::size_t>(j);
        at<void*>(screen, 0x28 + s) = at<void*>(support(), 0x7C + s);
        at<void*>(screen, 0x58 + s) = at<void*>(support(), 0xAC + s);
        at<void*>(screen, 0x68 + s) = at<void*>(support(), 0xBC + s);
        at<void*>(screen, 0x48 + s) = at<void*>(support(), 0x8C + s);
        at<void*>(screen, 0x78 + s) = at<void*>(support(), 0xCC + s);
        at<void*>(screen, 0x88 + s) = at<void*>(support(), 0xDC + s);
        for (std::int32_t k = 0; k < 4; ++k) {
            Cc13SweepBNativeString suffix = {0, nullptr};
            host.native_string_resize_0041dd40(&suffix, 8, 1);
            if (suffix.data != nullptr) {
                host.crt_memcpy_00bf7680(suffix.data, host.image_global(0x00CF4C5C), suffix.length + 1);
            }
            // Constructed by the callees (uninitialised stack slots in the image).
            Cc13SweepBNativeString number = {0, nullptr};
            Cc13SweepBNativeString prefixed = {0, nullptr};
            Cc13SweepBNativeString name = {0, nullptr};
            void* number_out = host.native_string_from_int_004260b0(&number, k);
            void* prefixed_out = host.native_string_prefixed_0043c130(
                &prefixed, static_cast<const char*>(host.image_global(0x00CF4C4C)), number_out);
            host.native_string_concat_004261a0(prefixed_out, &name, &suffix);
            release_string(prefixed, host);
            release_string(number, host);
            release_string(suffix, host);
            if (k == j) {
                at<void*>(screen, 0x38 + s) = host.widget_find_child_00aa7e00(at<void*>(screen, 0x28 + s), &name, 1);
                host.widget_vcall_34_set_visible(at<void*>(screen, 0x38 + s), 0);
                host.widget_vcall_4c_set_float(at<void*>(screen, 0x38 + s), f32_bits(0x3F000000u));  // 00CE3800
            } else {
                host.widget_vcall_34_set_visible(
                    host.widget_find_child_00aa7e00(at<void*>(screen, 0x28 + s), &name, 1), 0);
            }
            host.widget_vcall_34_set_visible(host.widget_find_child_00aa7e00(at<void*>(screen, 0x58 + s), &name, 1),
                                             0);
            release_string(name, host);
        }
    }
    at<void*>(screen, 0xA0) = at<void*>(support(), 0xF8);
    at<void*>(screen, 0xA4) = at<void*>(support(), 0xFC);
}

// ===========================================================================
// Segment 26
// ===========================================================================

// 00650450 BSP_HudSubmarineScreen_Update
// __thiscall(ECX = HUD submarine screen, one unused stack dword) RET 4; no
// SEH frame. Vtable slot +20h of 00CF61D4; no direct callers.
// Unit = *(00E188D8) (re-read after the kind test): null or vtable +5Ch(8)
// false -> nothing.
// - Surface ring +18h: when 0.2 (double 00CE3D10) > unit+127Ch (ordered):
//   vtable +38h false -> +34h(1); then 006501A0(screen). Otherwise vtable +38h
//   true -> +4Ch(0.0f), +34h(0), byte +1Ch = 0, float +20h = 0.
// - Needle +0Ch: vtable +44h(-((3 - unit+1268h) * K - *(00E08CB0) * pi / 180))
//   with K = 0.22689280907313028, pi = 3.1415927410125732 (doubles).
// - Depth: d0 = unit+1200h, d3 = unit+120Ch read BEFORE the pose refresh
//   (00414DB0 when byte unit+0C8h is 0); y = unit+100h after it;
//   d = d3 > y ? d3 : (y > d0 ? d0 : y). i = 1; while !(d >= depth[clamp(i)])
//   and ++i <= 3; t = (d - depth[clamp(i - 1)]) / (depth[clamp(i)] -
//   depth[clamp(i - 1)]) stored as float; needle +10h: vtable
//   +44h(-((3.0 - (t + (i - 1))) * K - *(00E08CB0) * pi / 180)).
//   depth[] is unit+1200h.., clamp to 0..3.
// - Gauge +14h: once (bit 1 of 00E19828) 00E19824 = 1.4660766f (00CF4870);
//   c = clamp(unit+127Ch, 0, 1) (COMISS: NaN passes through);
//   vtable +44h(0.5 * s - s * c), s = 00E19824.
void cc13_sweepB3_hud_submarine_update_00650450(void* screen, float unused, Cc13SweepB3Host& host) {
    (void)unused;
    if (host.global_00e188d8() == nullptr) return;
    if (!host.entity_vcall_5c_is_kind(host.global_00e188d8(), 8)) return;
    void* unit = host.global_00e188d8();
    const double k_angle = f64_bits(0x3FCD0AD2D5555555ull);  // 00CF6210
    const double pi = f64_bits(0x400921FB60000000ull);       // 00CE3D28
    const double degrees = f64_bits(0x4066800000000000ull);  // 00CE3D20

    const float surfaced = at<float>(unit, 0x127C);
    const std::int32_t stage = at<std::int32_t>(unit, 0x1268);
    if (f64_bits(0x3FC99999A0000000ull) > static_cast<double>(surfaced)) {  // 00CE3D10
        if (!host.widget_vcall_38_is_visible(at<void*>(screen, 0x18))) {
            host.widget_vcall_34_set_visible(at<void*>(screen, 0x18), 1);
        }
        host.hud_submarine_pulse_alpha_006501a0(screen);
    } else if (host.widget_vcall_38_is_visible(at<void*>(screen, 0x18))) {
        host.widget_vcall_4c_set_float(at<void*>(screen, 0x18), 0.0f);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x18), 0);
        at<std::uint8_t>(screen, 0x1C) = 0;
        at<float>(screen, 0x20) = 0.0f;
    }

    auto bias = [&]() {
        return static_cast<double>(*static_cast<float*>(host.image_global(0x00E08CB0))) * pi / degrees;
    };
    host.widget_vcall_44_set_float(
        at<void*>(screen, 0xC), static_cast<float>(-(static_cast<double>(3 - stage) * k_angle - bias())));

    const float top = at<float>(unit, 0x1200);
    const float bottom = at<float>(unit, 0x120C);
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    const float height = at<float>(unit, 0x100);
    float depth;
    if (bottom > height) {
        depth = bottom;
    } else if (height > top) {
        depth = top;
    } else {
        depth = height;
    }
    auto clamp3 = [](std::int32_t i) { return i < 0 ? 0 : (i > 3 ? 3 : i); };
    auto band = [&](std::int32_t i) { return at<float>(unit, 0x1200 + 4 * static_cast<std::size_t>(clamp3(i))); };
    std::int32_t i = 1;
    for (;;) {
        if (depth >= band(i)) break;
        i += 1;
        if (i > 3) break;
    }
    const std::int32_t lower = i - 1;
    const float lower_depth = band(lower);
    const float t = (depth - lower_depth) / (band(i) - lower_depth);
    const double turns = f64_bits(0x4008000000000000ull) - (static_cast<double>(t) + static_cast<double>(lower));
    host.widget_vcall_44_set_float(at<void*>(screen, 0x10), static_cast<float>(-(turns * k_angle - bias())));

    std::uint32_t& guard = *static_cast<std::uint32_t*>(host.image_global(0x00E19828));
    float& sweep = *static_cast<float*>(host.image_global(0x00E19824));
    if ((guard & 1) == 0) {
        sweep = f32_bits(0x3FBBA866u);  // 00CF4870
        guard |= 1;
    }
    const float level = at<float>(unit, 0x127C);
    float clamped;
    if (0.0f > level) {
        clamped = 0.0f;
    } else if (level > 1.0f) {
        clamped = 1.0f;
    } else {
        clamped = level;
    }
    const double s = static_cast<double>(sweep);
    host.widget_vcall_44_set_float(
        at<void*>(screen, 0x14),
        static_cast<float>(f64_bits(0x3FE0000000000000ull) * s - s * static_cast<double>(clamped)));  // 00D7A280
}

// ===========================================================================
// Segment 28
// ===========================================================================

namespace {

// A std::list record on the stack: {+0 proxy, +4 head node, +8 size}; the
// callees (005AAAF0, 0084D560) construct all three.
struct Cc13SweepB3ListRecord {
    void* proxy;
    void* head;
    std::uint32_t size;
};

// The inlined node loop of std::list::clear / ~list: from first until the
// head (re-read each step) every node goes to operator delete 00BF65AC, then
// the head itself.
void free_list_nodes(void* first, void* const& head, Cc13SweepB3Host& host) {
    void* node = first;
    while (node != head) {
        void* next = at<void*>(node, 0x0);
        host.crt_operator_delete_00bf65ac(node);
        node = next;
    }
    host.crt_operator_delete_00bf65ac(head);
}

// Unlink every node from the head (head->next = head->prev = head, size 0)
// and return the old first node.
void* unlink_list(Cc13SweepB3ListRecord& list) {
    void* first = at<void*>(list.head, 0x0);
    at<void*>(list.head, 0x0) = list.head;
    at<void*>(list.head, 0x4) = list.head;
    list.size = 0;
    return first;
}

}  // namespace

// 0068BC80 BSP_InGameInterface_BuildTacticalOverlayList (ledger name kept;
// the body only answers whether the overlay has anything to show)
// __thiscall(ECX = in-game interface, not read) RET; AL; SEH frame (handler
// 00C7DDE8). Sole caller 0068C0B0. game = *(00E188A8), lists = game+19CCh
// (re-read before each later walk), team = (game+18CCh+4*game+18ECh)+28h.
// Lists are {+4h next, +8h value} chains ending in null. True as soon as:
// 1. support sites (lists+16Ch): a site with +78Ch != 0 gets its list +784h
//    copied (005AAAF0); for each copied node, 0084D560(node value; &classes,
//    team, 1) builds a list; a non-empty one frees both temporary lists and
//    returns true; otherwise both are freed as the walk goes (00BF6713 when
//    the iterator is the head before advancing);
// 2. lists+358h: a unit with +54h == team whose interface at +72Ch (null when
//    the unit is null, which the image would dereference) answers non-zero
//    from vtable +0h;
// 3. lists+88h: the same with the interface at +1188h;
// 4. lists+364h: +54h == team and a non-empty vector of 20h-byte elements at
//    +774h/+778h;
// 5. lists+3B8h: +344h == 0 and +54h == team and, for team 0, +330h != 0 or,
//    for team 1, +324h != 0.
// Otherwise false.
bool cc13_sweepB3_ingame_tactical_overlay_check_0068bc80(void* iface, Cc13SweepB3Host& host) {
    (void)iface;
    void* game = host.global_00e188a8();
    void* side = at<void*>(game, 0x18CC + 4 * static_cast<std::size_t>(at<std::uint32_t>(game, 0x18EC)));
    void* lists = at<void*>(game, 0x19CC);
    const std::int32_t team = at<std::int32_t>(side, 0x28);
    for (void* node = at<void*>(lists, 0x16C); node != nullptr; node = at<void*>(node, 0x4)) {
        void* site = at<void*>(node, 0x8);
        if (at<std::uint32_t>(site, 0x78C) == 0) continue;
        Cc13SweepB3ListRecord copy;
        host.list_copy_005aaaf0(&copy, bytes(site) + 0x784);
        void* entry = at<void*>(copy.head, 0x0);
        while (entry != copy.head) {
            Cc13SweepB3ListRecord classes;
            host.support_site_build_supply_class_list_0084d560(at<void*>(entry, 0x8), &classes, team, 1);
            const bool found = classes.size != 0;
            void* first = unlink_list(classes);
            if (found) {
                free_list_nodes(first, classes.head, host);
                classes.head = nullptr;
                free_list_nodes(unlink_list(copy), copy.head, host);
                return true;
            }
            free_list_nodes(first, classes.head, host);
            classes.head = nullptr;
            if (entry == copy.head) host.crt_invalid_parameter_noinfo_00bf6713();
            entry = at<void*>(entry, 0x0);
        }
        free_list_nodes(unlink_list(copy), copy.head, host);
        copy.head = nullptr;
    }
    lists = at<void*>(host.global_00e188a8(), 0x19CC);
    for (void* node = at<void*>(lists, 0x358); node != nullptr; node = at<void*>(node, 0x4)) {
        std::uint8_t* unit = at<std::uint8_t*>(node, 0x8);
        if (team != at<std::int32_t>(unit, 0x54)) continue;
        if (host.interface_vcall_00_query(unit != nullptr ? unit + 0x72C : nullptr) != 0) return true;
    }
    lists = at<void*>(host.global_00e188a8(), 0x19CC);
    for (void* node = at<void*>(lists, 0x88); node != nullptr; node = at<void*>(node, 0x4)) {
        std::uint8_t* unit = at<std::uint8_t*>(node, 0x8);
        if (team != at<std::int32_t>(unit, 0x54)) continue;
        if (host.interface_vcall_00_query(unit != nullptr ? unit + 0x1188 : nullptr) != 0) return true;
    }
    lists = at<void*>(host.global_00e188a8(), 0x19CC);
    for (void* node = at<void*>(lists, 0x364); node != nullptr; node = at<void*>(node, 0x4)) {
        void* value = at<void*>(node, 0x8);
        if (team != at<std::int32_t>(value, 0x54)) continue;
        std::uint8_t* first = at<std::uint8_t*>(value, 0x774);
        if (first != nullptr && (static_cast<std::int32_t>(at<std::uint8_t*>(value, 0x778) - first) >> 5) != 0) {
            return true;
        }
    }
    for (void* node = at<void*>(lists, 0x3B8); node != nullptr; node = at<void*>(node, 0x4)) {
        void* value = at<void*>(node, 0x8);
        if (at<std::uint32_t>(value, 0x344) != 0 || at<std::int32_t>(value, 0x54) != team) continue;
        if (team == 0) {
            if (at<std::uint32_t>(value, 0x330) != 0) return true;
        } else if (team == 1) {
            if (at<std::uint32_t>(value, 0x324) != 0) return true;
        }
    }
    return false;
}

// ===========================================================================
// Segment 46
// ===========================================================================

// 007A7200 BSP_MovCam_SetPathWalk
// __thiscall(ECX = movie camera; path, start, end, float a, flag, float c,
// const native string* name, d) RET 20h; no SEH frame. Sole caller 008B47E0
// (the luaMW_MovCam_PathWalk binding). The image reuses the start/end
// argument slots for the step and the loop index; kept as locals here.
// - 0079C830 clears the path records (the sweepB reconstruction is called).
// - count = |end - start| + 1 (CDQ/XOR/SUB), step = end >= start ? 1 : -1,
//   point runs from start by step; for index 0..count-1:
//   slot = index + (flag's low byte == 0 ? 2 : 0); the pointer vector
//   camera+478h is resized (0079EB10(slot + 1, 0)) when its size is below
//   slot + 1; a 4Ch-byte record from operator new 00BF681B (null kept: the
//   image then stores through it) is initialised (vtable 00D043D8, dwords
//   +4h/+8h/+0Ch/+14h/+24h/+48h = 0, byte +10h = 0, floats +18h..+20h,
//   +28h..+38h, +44h = 0, +3Ch and +40h = 0.33333334f) and stored at slot
//   (00BF6713 when out of range);
//   entity = 007ACA30(path) (called afresh each time): +14h = entity+3Ch;
//   +18h..+20h = 007AF740(path; local, point); when entity vtable +5Ch(4Ah)
//   is false +48h = 0; otherwise with owner = entity+1E4h and
//   rec(point) = 007AD070(owner, point) per read: +3Ch/+40h/+44h from
//   rec+3Ch/+40h/+44h, kind = rec+20h: 0 -> +48h = 0; 1 -> +48h = 1, +24h =
//   rec+24h; 2 or 3 -> +48h = kind, +28h..+30h = rec+28h..+30h; 4 -> +48h =
//   4, +34h/+38h = rec+34h/+38h; 5 -> +48h = 5, +34h/+38h from 0078FC60
//   (sweepB's reconstruction); other kinds leave the record as built.
// - Then 007A5A00(camera; a, flag, c, name, d).
void cc13_sweepB3_movcam_set_path_walk_007a7200(void* camera, void* path, std::int32_t start, std::int32_t end,
                                                float a, std::uint32_t flag, float c,
                                                const Cc13SweepBNativeString* name, std::int32_t d,
                                                Cc13SweepB3Host& host) {
    cc13_sweepB_movcam_clear_path_records_0079c830(camera, host);
    const std::int32_t diff = static_cast<std::int32_t>(static_cast<std::uint32_t>(end) - static_cast<std::uint32_t>(start));
    const std::int32_t sign = diff >> 31;
    const std::int32_t count = static_cast<std::int32_t>(static_cast<std::uint32_t>((diff ^ sign) - sign) + 1u);
    const std::int32_t step = end >= start ? 1 : -1;
    std::int32_t point = start;
    for (std::int32_t index = 0; index < count; ++index) {
        std::uint32_t slot = static_cast<std::uint32_t>(index);
        if ((flag & 0xFF) == 0) slot += 2;
        std::uint8_t* records = bytes(camera) + 0x478;
        const std::uint32_t size =
            at<std::uint8_t*>(records, 0x4) == nullptr
                ? 0
                : static_cast<std::uint32_t>(
                      static_cast<std::int32_t>(at<std::uint8_t*>(records, 0x8) - at<std::uint8_t*>(records, 0x4)) >> 2);
        if (size < slot + 1) host.pointer_vector_resize_0079eb10(records, slot + 1, nullptr);
        std::uint8_t* record = static_cast<std::uint8_t*>(host.crt_operator_new_00bf681b(0x4C));
        if (record != nullptr) {
            at<std::uint32_t>(record, 0x4) = 0;
            at<std::uint32_t>(record, 0x8) = 0;
            at<std::uint32_t>(record, 0xC) = 0;
            at<std::uint32_t>(record, 0x14) = 0;
            at<std::uint8_t>(record, 0x10) = 0;
            at<std::uint32_t>(record, 0x0) = 0x00D043D8u;  // the image's vtable address, kept as a value
            for (std::size_t offset : {0x18, 0x1C, 0x20}) at<float>(record, offset) = 0.0f;
            at<std::uint32_t>(record, 0x24) = 0;
            for (std::size_t offset : {0x28, 0x2C, 0x30, 0x34, 0x38}) at<float>(record, offset) = 0.0f;
            at<std::uint32_t>(record, 0x48) = 0;
            at<float>(record, 0x40) = f32_bits(0x3EAAAAABu);  // 00D04678
            at<float>(record, 0x3C) = f32_bits(0x3EAAAAABu);
            at<float>(record, 0x44) = 0.0f;
        }
        std::uint8_t* first = at<std::uint8_t*>(records, 0x4);
        if (first == nullptr ||
            !(slot < static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint8_t*>(records, 0x8) - first) >> 2))) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        at<void*>(at<std::uint8_t*>(records, 0x4), 4 * static_cast<std::size_t>(slot)) = record;

        at<std::uint32_t>(record, 0x14) = at<std::uint32_t>(host.path_object_get_entity_007aca30(path), 0x3C);
        float world_buffer[3];
        const float* world = host.scene_path_point_to_world_007af740(path, world_buffer, point);
        at<float>(record, 0x18) = world[0];
        at<float>(record, 0x1C) = world[1];
        at<float>(record, 0x20) = world[2];
        if (!host.entity_vcall_5c_is_kind(host.path_object_get_entity_007aca30(path), 0x4A)) {
            at<std::uint32_t>(record, 0x48) = 0;
        } else {
            void* owner = static_cast<std::uint8_t*>(host.path_object_get_entity_007aca30(path)) + 0x1E4;
            auto point_record = [&]() { return host.movcam_path_point_007ad070(owner, point); };
            at<float>(record, 0x3C) = at<float>(point_record(), 0x3C);
            at<float>(record, 0x40) = at<float>(point_record(), 0x40);
            at<float>(record, 0x44) = at<float>(point_record(), 0x44);
            const std::uint32_t kind = at<std::uint32_t>(point_record(), 0x20);
            if (kind == 0) {
                at<std::uint32_t>(record, 0x48) = 0;
            } else if (kind == 1) {
                at<std::uint32_t>(record, 0x48) = kind;
                at<std::uint32_t>(record, 0x24) = at<std::uint32_t>(point_record(), 0x24);
            } else if (kind == 2 || kind == 3) {
                at<std::uint32_t>(record, 0x48) = kind;
                void* source = point_record();
                at<float>(record, 0x28) = at<float>(source, 0x28);
                at<float>(record, 0x2C) = at<float>(source, 0x2C);
                at<float>(record, 0x30) = at<float>(source, 0x30);
            } else if (kind == 4) {
                at<std::uint32_t>(record, 0x48) = kind;
                void* source = point_record();
                at<float>(record, 0x34) = at<float>(source, 0x34);
                at<float>(record, 0x38) = at<float>(source, 0x38);
            } else if (kind == 5) {
                at<std::uint32_t>(record, 0x48) = kind;
                float pair_buffer[2];
                const float* pair = cc13_sweepB_movcam_path_point_pair_0078fc60(owner, pair_buffer, point, host);
                at<float>(record, 0x34) = pair[0];
                at<float>(record, 0x38) = pair[1];
            }
        }
        point += step;
    }
    host.movcam_begin_path_walk_007a5a00(camera, a, static_cast<std::uint8_t>(flag), c, name, d);
}

}  // namespace bsp
