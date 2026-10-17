// cc13_sweepB10: tenth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB10, docs/CC13_SWEEPB10.md). Names are hypotheses; see the header
// for the hooks.
//
// x87 note (rule 4c): these functions run on the main thread, whose x87
// precision control is 24 bits (docs/CC13_FPU_PRECISION.md), so every x87
// arithmetic operation is modelled as rounding to float in the image's order.
// SSE arithmetic is float as written. EH frames are noted, not modelled.

#include "bsp/cc13_sweepB10.hpp"

#include <cmath>
#include <cstring>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

float f32_bits(std::uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

double f64_bits(std::uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// One x87 operation under 24-bit precision control: the exact result rounded
// to float.
float pc24(double value) {
    return static_cast<float>(value);
}

// The inlined native-string release: when data is non-null, the data goes
// back to the sized pool with length + 1 bytes.
void release_string(Cc13SweepBNativeString& text, Cc13SweepB10Host& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// The inlined temporary native string of a literal: {0, 0}, 0041DD40(length,
// 1), memcpy of length + 1 bytes when data is non-null.
void make_c_string(Cc13SweepBNativeString& text, const char* source, Cc13SweepB10Host& host) {
    text = {0, nullptr};
    host.native_string_resize_0041dd40(&text, static_cast<std::uint32_t>(std::strlen(source)), 1);
    if (text.data != nullptr) host.crt_memcpy_00bf7680(text.data, source, text.length + 1);
}

// The same temporary, used once and released (the length re-read after the
// use).
template <class Use>
void with_c_string(const char* source, Cc13SweepB10Host& host, Use use) {
    Cc13SweepBNativeString text;
    make_c_string(text, source, host);
    use(&text);
    release_string(text, host);
}

// The inlined std::list<void*>::push_back of the formation screen: head =
// [list+4h]; node = buy(head, head->prev, &value); incsize(1); head->prev =
// node; node->prev->next = node.
template <class Buy, class Inc>
void list_push_back(std::uint8_t* list, void* const* value, Buy buy, Inc inc) {
    void* const head = at<void*>(list, 0x4);
    void* const node = buy(list, head, at<void*>(head, 0x4), value);
    inc(list, 1);
    at<void*>(head, 0x4) = node;
    at<void*>(at<void*>(node, 0x4), 0x0) = node;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00538F80 BSP_HudFormationScreen_Register
// __fastcall(ECX = screen), RET; vtable 00CED6FC slot +10h. EH frame 00C6C608
// (states 0..1Eh track the name temporaries and the "mapzone" string). Image
// facts (asm 00538F80..0053A4CD):
// - 004F71D0(screen); +164h = 0.0f (XORPS), bytes +168h/+169h = 0, +170h =
//   128h, +8Ch = 0, +80h = [00D7A24C] 1.0f, bytes +95h/+94h = 0.
// - Pages through 004C12B0 / 00AA5840(name, 1, 0): GUI_formation -> +98h,
//   GUI_classicons -> +9Ch, GUI_unit -> +A0h, GUI_formation_circle -> +A4h,
//   GUI_formationOvrly -> +A8h (literals 00CED9B0..00CED968).
// - 25 widgets through 00AA7E00(page, name, 1), the page re-read for each
//   (table below, literals 00CED94C..00CED784, order as in the image);
//   +B4h (map_class_Icon) gets vtable +34h(0) right after its find.
// - Listbox +ACh: 00A9AC40(+78h); 00A9AC70(005353A0); vtable +60h(1).
// - +1Ch = [00CE3958] 2.0f; +20h = FLD1 - [00D7A258] 0.0 (x87, = 1.0f); +2Ch
//   = 2; +150h = +14Ch = 0.0f; 0093CCC0(+8h); 00B6FE10([+10h], 6).
// - x = [00424C40() + 434h]; +90h = x; quad 1 = 0093DE10(+8h; 11h, {1,1,1,1}
//   ([00D7A24C]), "guialphawrite.mshd" (00CED76C, built in the argument slot
//   by 0041E870), 2990.0f (00CED780), w, w, 5) with w = (float)(x + x)
//   (FADD ST0,ST0); stored at +178h.
// - For every node n of the singly linked list [[00E188A8]+19CCh]+34Ch (object
//   +8h, next +4h; skipped when the head is null): E = n+8h's object; factory
//   = 00B80D70(004C1400(), E+424h); inst = factory vtable +8h(6, 1.0f); root
//   = inst+0Ch; 00B6D890(root, [screen+0Ch]); for every child c of root
//   (first +34h, next +3Ch): "mapzone" temporary (00CED764); h = 00B6D800(c);
//   when 7 <= h.length, a temporary of the last 7 characters (strlen of
//   h.data + h.length - 7, 0041DD40 + memcpy) is compared: equal lengths and
//   (both empty, or _stricmp("mapzone", suffix) == 0); suffix released;
//   "mapzone" released; on a match 00B6DA70(c, 0.0f, 0). Then r = root
//   vtable +10h(6, 0); E's world matrix +CCh is rebuilt unless byte +C8h is
//   set (00414DB0 on the parent pose [E+3Ch] when non-null, then 00413920(E+
//   74h; tmp, parent+CCh) or E+74h itself; 16 floats copied to +CCh, +C8h =
//   1, +10Ch = 0); root vtable +34h(E+CCh); 0093D2D0(+8h; root, 1) and (r,
//   0); r pushed on the list +194h (00535880 / 00537E80); 00B6E680(r, root);
//   InterlockedDecrement(factory+4h), at zero factory vtable +0h(); inst
//   pushed on the list +1A0h (005357E0 / 00537B80).
// - Quad 2: (0, {1,1,1,1}, "map_texturednonormal_ocean.mshd" (00CED744, built
//   with 0041DD40 + memcpy in the slot), -500.0f (00CE3980), g, g, 12Ch) and
//   quad 3: (1, {0,0,0,0.3f} (00CE69C8), "guialphastencildiff.mshd"
//   (00CED728), 1000.0f (00CE3804), g, g, 2), g = the .data float 00E0CF40
//   read at each call; both results dropped.
// - +138h = 0; +184h = (float)(+90h * +90h) (FMUL ST0); 00536310(screen,
//   (float)(+90h + +90h)); +18Ch = 2990.0f, +190h = 1500.0f (00CED724), +188h
//   = 1400.0f (00CED560); [+174h] vtable +30h(&{0.0f, 1400.0f, 0.0f}).
// No draw from the shared RNG 00BD2F10. The doublings are exact; FMUL ST0
// rounds once to float under 24-bit precision.
// ---------------------------------------------------------------------------
void cc13_sweepB10_formation_register_00538f80(void* screen, Cc13SweepB10Host& host) {
    host.frontend_screen_register_004f71d0(screen);
    at<float>(screen, 0x164) = 0.0f;
    at<std::uint8_t>(screen, 0x168) = 0;
    at<std::uint8_t>(screen, 0x169) = 0;
    at<std::uint32_t>(screen, 0x170) = 0x128;
    at<std::uint32_t>(screen, 0x8C) = 0;
    at<float>(screen, 0x80) = f32_bits(0x3F800000u);  // 00D7A24C
    at<std::uint8_t>(screen, 0x95) = 0;
    at<std::uint8_t>(screen, 0x94) = 0;

    struct Page {
        const char* name;
        std::uint32_t slot;
    };
    static const Page kPages[5] = {{"GUI_formation", 0x98},
                                   {"GUI_classicons", 0x9C},
                                   {"GUI_unit", 0xA0},
                                   {"GUI_formation_circle", 0xA4},
                                   {"GUI_formationOvrly", 0xA8}};
    for (const Page& page : kPages) {
        with_c_string(page.name, host, [&](Cc13SweepBNativeString* name) {
            void* const manager = host.gui_manager_get_004c12b0();
            at<void*>(screen, page.slot) = host.gui_manager_load_page_00aa5840(manager, name, 1, 0);
        });
    }

    struct Widget {
        const char* name;
        std::uint32_t page;
        std::uint32_t slot;
    };
    static const Widget kWidgets[25] = {
        {"ship_plane_agressive_Icon", 0xA8, 0xD4}, {"ship_plane_defensive_Icon", 0xA8, 0xD8},
        {"form_line_Icon", 0xA8, 0xE0},            {"form_side_Icon", 0xA8, 0xE4},
        {"form_gem_Icon", 0xA8, 0xE8},             {"all_ship_leave_form_Icon", 0xA8, 0xDC},
        {"cancel_Icon", 0xA8, 0xEC},               {"repair_hl_west_Icon", 0xA8, 0xFC},
        {"repair_hl_east_Icon", 0xA8, 0x100},      {"repair_hl_north_Icon", 0xA8, 0x104},
        {"repair_hl_south_Icon", 0xA8, 0x108},     {"repair_hl_middle_Icon", 0xA8, 0x10C},
        {"repair_glass_Icon", 0xA8, 0xF0},         {"repair_frame_Icon", 0xA8, 0xF4},
        {"info_Text", 0xA8, 0xF8},                 {"unit_name_Text", 0xA0, 0xB8},
        {"FlagJP_Icon", 0xA0, 0xBC},               {"FlagUS_Icon", 0xA0, 0xC0},
        {"unit_payload_Icon", 0xA0, 0xC4},         {"unit_HP_damage_Icon", 0xA0, 0xC8},
        {"unit_HP_healthy_Icon", 0xA0, 0xCC},      {"unit_command_Icon", 0xA0, 0xD0},
        {"circle_Group", 0xA4, 0xB0},              {"map_class_Icon", 0x9C, 0xB4},
        {"Jobboldali_Listbox", 0x98, 0xAC},
    };
    for (const Widget& widget : kWidgets) {
        with_c_string(widget.name, host, [&](Cc13SweepBNativeString* name) {
            at<void*>(screen, widget.slot) = host.widget_find_child_00aa7e00(at<void*>(screen, widget.page), name, 1);
        });
        if (widget.slot == 0xB4) host.widget_vcall_34_set_visible(at<void*>(screen, 0xB4), 0);
    }

    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0xAC), static_cast<std::uint8_t*>(screen) + 0x78);
    host.gui_listbox_set_input_filter_00a9ac70(at<void*>(screen, 0xAC), 0x005353A0u);
    host.widget_vcall_60(at<void*>(screen, 0xAC), 1);

    at<float>(screen, 0x1C) = f32_bits(0x40000000u);  // 00CE3958
    at<float>(screen, 0x20) = pc24(1.0 - 0.0);         // FLD1, FSUB [00D7A258]
    at<std::int32_t>(screen, 0x2C) = 2;
    at<float>(screen, 0x150) = 0.0f;
    at<float>(screen, 0x14C) = 0.0f;
    std::uint8_t* const scene = static_cast<std::uint8_t*>(screen) + 0x8;
    host.simple_3d_scene_init_0093ccc0(scene);
    host.camera_set_clear_flags_00b6fe10(at<void*>(screen, 0x10), 6);

    const float x = at<float>(host.game_settings_get_00424c40(), 0x434);
    const float doubled = pc24(static_cast<double>(x) + x);
    at<float>(screen, 0x90) = x;
    const float white[4] = {f32_bits(0x3F800000u), f32_bits(0x3F800000u), f32_bits(0x3F800000u),
                            f32_bits(0x3F800000u)};  // 00D7A24C
    {
        Cc13SweepBNativeString effect;
        host.native_string_assign_0041e870(&effect, "guialphawrite.mshd");  // 00CED76C
        at<void*>(screen, 0x178) = host.map_scene_build_quad_0093de10(scene, 0x11, white, effect,
                                                                      f32_bits(0x453AE000u),  // 00CED780 2990
                                                                      doubled, doubled, 5);
    }

    void* node = at<void*>(at<void*>(host.global_00e188a8(), 0x19CC), 0x34C);
    if (node != nullptr) {
        do {
            std::uint8_t* const entity = at<std::uint8_t*>(node, 0x8);
            void* const factory =
                host.resource_load_default_factory_00b80d70(host.resource_manager_get_004c1400(), entity + 0x424);
            void* const instance = host.factory_vcall_08_create(factory, 6, f32_bits(0x3F800000u));
            void* const root = at<void*>(instance, 0xC);
            host.node_propagate_root_00b6d890(root, at<void*>(screen, 0xC));

            for (void* child = at<void*>(root, 0x34); child != nullptr; child = at<void*>(child, 0x3C)) {
                Cc13SweepBNativeString mapzone;
                make_c_string(mapzone, "mapzone", host);  // 00CED764
                const auto* header =
                    static_cast<const Cc13SweepBNativeString*>(host.node_get_name_header_00b6d800(child));
                bool match = false;
                if (!(mapzone.length > header->length)) {
                    const char* const tail = header->data + header->length - mapzone.length;
                    Cc13SweepBNativeString suffix = {0, nullptr};
                    host.native_string_resize_0041dd40(&suffix, static_cast<std::uint32_t>(std::strlen(tail)), 1);
                    if (suffix.data != nullptr) host.crt_memcpy_00bf7680(suffix.data, tail, suffix.length + 1);
                    if (mapzone.length == suffix.length) {
                        std::int32_t differs;
                        if (mapzone.length == 0) {
                            differs = suffix.length != 0 ? -1 : 0;
                        } else if (suffix.length == 0) {
                            differs = 1;
                        } else {
                            differs = host.crt_stricmp_00bf7fbf(mapzone.data, suffix.data);
                        }
                        match = differs == 0;
                    }
                    release_string(suffix, host);
                }
                release_string(mapzone, host);
                if (match) host.scene_node_set_visibility_factor_00b6da70(child, 0.0f, 0);
            }

            void* const r = host.node_vcall_10(root, 6, 0);
            float* world;
            if (at<std::uint8_t>(entity, 0xC8) != 0) {
                world = &at<float>(entity, 0xCC);
            } else {
                if (at<void*>(entity, 0x3C) != nullptr) host.entity_pose_refresh_world_00414db0(at<void*>(entity, 0x3C));
                float product[16];
                const float* source;
                if (at<std::uint8_t*>(entity, 0x3C) != nullptr) {
                    source = host.matrix_multiply_00413920(&at<float>(entity, 0x74), product,
                                                           &at<float>(at<std::uint8_t*>(entity, 0x3C), 0xCC));
                } else {
                    source = &at<float>(entity, 0x74);
                }
                world = &at<float>(entity, 0xCC);
                for (int i = 0; i < 16; ++i) world[i] = source[i];
                at<std::uint8_t>(entity, 0xC8) = 1;
                at<std::uint8_t>(entity, 0x10C) = 0;
            }
            host.node_vcall_34_set_matrix(root, world);
            host.map_scene_setup_ocean_materials_0093d2d0(scene, root, 1);
            host.map_scene_setup_ocean_materials_0093d2d0(scene, r, 0);

            void* const r_value = r;
            list_push_back(static_cast<std::uint8_t*>(screen) + 0x194, &r_value,
                           [&](void* l, void* next, void* prev, void* const* v) {
                               return host.formation_list_buy_node_00535880(l, next, prev, v);
                           },
                           [&](void* l, std::uint32_t n) { host.formation_list_incsize_00537e80(l, n); });
            host.node_set_parent_00b6e680(r, root);
            if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(factory, 0x4)) == 0) {
                host.refcounted_vcall_00_destroy(factory);
            }
            void* const instance_value = instance;
            list_push_back(static_cast<std::uint8_t*>(screen) + 0x1A0, &instance_value,
                           [&](void* l, void* next, void* prev, void* const* v) {
                               return host.formation_list_buy_node_005357e0(l, next, prev, v);
                           },
                           [&](void* l, std::uint32_t n) { host.formation_list_incsize_00537b80(l, n); });
            node = at<void*>(node, 0x4);
        } while (node != nullptr);
    }

    const float* const extent = static_cast<const float*>(host.image_global(0x00E0CF40));
    {
        Cc13SweepBNativeString effect;
        make_c_string(effect, "map_texturednonormal_ocean.mshd", host);  // 00CED744
        host.map_scene_build_quad_0093de10(scene, 0, white, effect, f32_bits(0xC3FA0000u),  // 00CE3980 -500
                                           *extent, *extent, 0x12C);
    }
    {
        const float shade[4] = {0.0f, 0.0f, 0.0f, f32_bits(0x3E99999Au)};  // 00CE69C8 0.3
        Cc13SweepBNativeString effect;
        make_c_string(effect, "guialphastencildiff.mshd", host);  // 00CED728
        host.map_scene_build_quad_0093de10(scene, 1, shade, effect, f32_bits(0x447A0000u),  // 00CE3804 1000
                                           *extent, *extent, 2);
    }

    at<std::uint32_t>(screen, 0x138) = 0;
    at<float>(screen, 0x184) = pc24(static_cast<double>(at<float>(screen, 0x90)) * at<float>(screen, 0x90));
    cc13_sweepB4_hud_formation_build_quad_model_00536310(
        screen, pc24(static_cast<double>(at<float>(screen, 0x90)) + at<float>(screen, 0x90)), host);
    at<float>(screen, 0x18C) = f32_bits(0x453AE000u);  // 00CED780 2990
    at<float>(screen, 0x190) = f32_bits(0x44BB8000u);  // 00CED724 1500
    at<float>(screen, 0x188) = f32_bits(0x44AF0000u);  // 00CED560 1400
    const float target[3] = {0.0f, f32_bits(0x44AF0000u), 0.0f};
    host.object_vcall_30_vec3(at<void*>(screen, 0x174), target);
}

// ---------------------------------------------------------------------------
// 005B45C0 BSP_HudMapScreen_Update
// __thiscall(ECX = screen; float dt), RET 4; vtable 00CF0134 slot +20h. EH
// frame 00C72088 (state 0 = the MAPPERMANENT string). Body 005B45C0..005B5B62
// (1465 instructions; the filter switch table is 005B5B64). Image facts, in
// order (x87 reg-reg forms read with tools/x87_dc_form_scan.py; every x87
// operation is one float rounding, rule 4c):
// - 0059AB10(screen, dt). hint = 0068E5B0(004E1CA0(), tmp); empty =
//   [hint] == 0; tmp released; when empty: 00690FD0(004E1CA0(),
//   "MAPPERMANENT" (00CEDA00, built by 0041E870), 0), released.
// - u = [00E188D8]: x = u vtable +13Ch(); when [x+1B0h] != 8 and
//   !00927F10([x+1B0h]) (stdcall, ECX not read): [00E188D8] vtable +13Ch()
//   -> 0077C470(2, 0). When [00E188D8] (re-read) is non-null, != +610h and
//   byte 00E19515 is clear, that byte = 1. +610h = [00E188D8] (last read).
// - Cursor: (0.5f, 0.5f) (00CE3800) when byte game+634h is set, else GUI
//   manager +5Ch/+60h. +554h = (mx - [00E08890]) * +30h + +54Ch, +558h =
//   ([00E08894] - my) * +34h + +550h.
// - Transition (+5ACh > 0, COMISS; NaN skips): dt >= rem (FCOMI, JC taken
//   when below or unordered) snaps +5ACh = 0, +30h/+34h = +5B0h/+5B4h,
//   +54Ch/+550h = +5E8h/+5ECh, +5D4h = 1, +5B8h = +5BCh, +5D0h = 0, +5D8h =
//   (float)level (CVTSI2SS), +5DCh = +5E0h, +5C4h = +5C8h. Otherwise f = dt /
//   rem (FDIVRP, stored as float); a = a + (t - a) * f for +30h (+5B0h), +34h
//   (+5B4h), +5B8h (+5BCh), +5DCh (+5E0h), +5C4h (+5C8h); the pan +54Ch +=
//   (+5E8h - +54Ch) * f, +550h likewise; +5D0h -= f; +5D4h += f; +5D8h = e +
//   (level - e) * f (FILD); +5ACh = +5ACh - dt.
// - +5C0h = +5B8h while +5ACh > 0, else 00F89AE4[level] * [00432650()+2BCh];
//   +5CCh = +5C4h or 00E0CF50[level]; +5E4h = +5DCh or (float)0059ABC0().
// - vis = !(byte g+5h && byte g+70h), g = [[00E198C4]+60h]; +100h vtable
//   +34h(vis) (EBX: the low byte is the SETZ result, the upper bytes are
//   stale stack; passed here as 0/1). Six slots i = 0..5 at +108h: an empty
//   slot is cloned (00AAB4C0(+ECh, 0) into +11Ch for i = 5, then [+11Ch]
//   vtable +58h(00AA6740(+108h)); else 00AAB4C0(+104h, 0)); a filled slot
//   gets vtable +34h(vis) and, when vis, 00AA8240(slot, {x_off + p.x, p.y +
//   0.013888889 (double 3F8C71C720000000), p.z + e}) with x_off = (float)n *
//   size.x * 1.1f + 7/480 (double 3F8DDDDDDDDDDDDE), p = 00AA6750(+100h),
//   size = 00AA6740(+108h), n = +128h for i = 5 else i, e = -1 for i = 5
//   else -10; i != 5 also gets vtable +88h(i, 0, 1.0f).
// - alpha = 0; when vis: c = +12Ch + 1 (FADD ST0,ST2), clock [00F876A4]: c >
//   clock -> alpha = 1; else +12Ch + 2.0 (double) > clock -> alpha = 1 -
//   (clock - c); 0059D350 in all three vis cases. +120h/+124h vtable
//   +4Ch(alpha), then vtable +34h(vis).
// - Pick: m = max(|in+35F4h|, |in+35C4h|) (the second when greater, ordered),
//   in = [004BEC00()+4] read through two 004BEC00 calls. byte +528h set: 0.1
//   (double 3FB99999A0000000) > m clears it. Clear: m > 0.5f -> 005A3210.
// - Spectate: byte [[in+278Ch]+0Bh] && +520h && 00645160(+520h, DL = 1) &&
//   [00E188D8] != +520h: 00647300([[00E198C4]+40h], +520h);
//   [[[00E198C4]+CCh]+B8h] = 0.
// - Filter cycle: vis && 004C43C0(game, 120h) && !006529E0([[00E198C4]+BCh]):
//   +128h += 1, 5 -> 0; +12Ch = clock; 00ABBE50(+124h, ingame.filters_*
//   (00CF0DA4 / 0D8C / 0D74 / 0D5C / 0D44 for 0..4), 1); 0059D350.
// - Wheel zoom (in+24E8h set, in+24E0h clear): zin = 0 > in+24E4h && level <
//   3; zout = in+24E4h > 0 && level > +5FCh; the start (values, flags +40h /
//   +41h, targets), every mapped value of the map +4F4h gets byte +9h = 1
//   (checked iterator 00535110 / 00BF6713), the pan target, then
//   0059E120(+5E8h, +5B0h, +5B4h).
// - Panning unless 004C43C0(game, C4h) or (g+5h && g+70h): s = +30h * 0.4f *
//   dt; while +528h is clear the stick (in+2514h, -0.0f - in+2544h, SUBSS)
//   moves +554h / +558h by (b * s, s * -a); then the keys (in+24B4h, -0.0f -
//   in+2484h) move the pan and the cursor by (b2 * s, s * -a2). A non-zero
//   (or NaN) key delta sets BL and clears +524h; otherwise a non-zero stick
//   delta clears +524h (UCOMISS / LAHF / TEST AH,44h / JP).
// - Cursor clamp 1: u = (cx - px) / W + 0.5, v = (py - cy) / H + 0.5 (W =
//   +30h, H = +34h, px/py = +54Ch/+550h, cx/cy = +554h/+558h); lo/hi = the
//   doubles BFB99999B0000000 / 3FF1999998000000 when byte [[0109CF04]+0Dh] is
//   set, else 0.05 / 0.95, each stored as float before use; lo > u: px -=
//   (lo - u) * W, u = lo; u > hi: px += (u - hi) * W, u = hi; 0.05 > v
//   (double compare): py += (0.05 - v) * H, v = 0.05f; v > 0.95: py -= (v -
//   0.95) * H, v = 0.95f; +554h = (u - 0.5) * W + px, +558h = H * (0.5 - v)
//   + py.
// - Followed unit +524h: unless byte +5Dh or +5Eh (then +524h = 0), +54Ch =
//   vtable +C4h()[0], +550h = vtable +C4h()[2], +554h/+558h = +54Ch/+550h.
// - 0059E120(+54Ch, +30h, +34h); cursor clamp 2 (u, v recomputed and
//   clamped to the same bounds without moving the pan); +554h = px + W * (u
//   - 0.5), +558h = H * (0.5 - v) + py; BL copies the pan into them; byte
//   +5F0h copies +5F4h/+5F8h into both pairs.
// - [00E08310] == 2: BL = 004F7740() == 2; 004F7740() == 3 adds (+5F0h &&
//   g+5h && g+70h) and (006529E0([[00E198C4]+BCh]) or byte [[00E198C4]+C0h]
//   +4h); BL && !game+634h -> 00AA1FA0(004C12B0(), u, v) of the cursor.
// - +52Ch = +520h; +534h = 00927880([00E188D8]) (0 when null), cleared when
//   its +54h equals the controlled unit's; +52Ch cleared when equal to +534h;
//   005A48C0(&+52Ch, &+530h, 0), 005A48C0(&+534h, &+538h, 1); 005B2D70.
// - [00E198C4]+F0h = {+554h, 0, +558h}, byte +FCh = 1; 005B25D0(dt, 0).
// - Shader values (+40h zoom in / +41h zoom out): +1Ch = (level + 1) - t or t
//   + (level + 1), t = +5ACh / [00432650()+2B8h]; a = |(t - 0.5) * 2| (FADD
//   ST0,ST0); +28h = a * (a * a). Always: c = clamp(+1Ch - 2, 0, 1) (NaN
//   stays NaN); +20h = 1 - c; q = 1 - (+1Ch - 1) * 0.5, +24h = q, then 1 - q *
//   (q * q).
// - game+1FE4h != 0: 009052E0([game+21A0h]; [game+18ECh], dt); 0 > +60Ch ->
//   00772A40(game+1EF0h), +60Ch = 1; else +60Ch -= dt; unless 004C43C0(109h):
//   10Bh -> mode 2, 10Ah -> mode 1, 008E2EE0(ECX = mode; +554h, +558h).
// - state = 0; the controlled unit of kind 6 (vtable +5Ch) whose zone group
//   0082ADA0([u+538h], 0) contains +554h (004178F0) -> 2; +520h -> 1;
//   00AA0E60(004C12B0(), state).
// No draw from the shared RNG 00BD2F10.
// ---------------------------------------------------------------------------
void cc13_sweepB10_hud_map_update_005b45c0(void* screen, float dt, Cc13SweepB10Host& host) {
    auto F = [&](std::size_t offset) -> float& { return at<float>(screen, offset); };
    auto image_float = [&](std::uint32_t address) -> float {
        return *static_cast<const float*>(host.image_global(address));
    };
    auto game = [&]() { return static_cast<std::uint8_t*>(host.global_00e188a8()); };
    auto gui = [&]() { return static_cast<std::uint8_t*>(host.global_00e198c4()); };
    auto input = [&]() { return at<std::uint8_t*>(host.input_manager_singleton_004bec00(), 0x4); };
    auto level = [&]() { return at<std::int32_t>(screen, 0x2C); };
    auto lerp = [](float a, float t, float f) { return pc24(a + pc24(pc24(static_cast<double>(t) - a) * f)); };
    auto edge_flag = [&]() { return at<std::uint8_t>(at<void*>(host.image_global(0x0109CF04), 0x0), 0xD) != 0; };
    auto gate_closed = [&]() {
        std::uint8_t* const g = at<std::uint8_t*>(gui(), 0x60);
        return at<std::uint8_t>(g, 0x5) != 0 && at<std::uint8_t>(g, 0x70) != 0;
    };

    host.hud_map_animate_zoom_0059ab10(screen, dt);
    {
        Cc13SweepBNativeString hint = {0, nullptr};  // filled by 0068E5B0
        const auto* name = static_cast<const Cc13SweepBNativeString*>(
            host.award_tracker_copy_active_hint_0068e5b0(host.award_tracker_get_004e1ca0(), &hint));
        const bool empty = name->length == 0;
        release_string(hint, host);
        if (empty) {
            Cc13SweepBNativeString key;
            host.native_string_assign_0041e870(&key, "MAPPERMANENT");  // 00CEDA00
            host.award_tracker_trigger_hint_00690fd0(host.award_tracker_get_004e1ca0(), &key, 0);
            release_string(key, host);
        }
    }

    void* controlled = host.global_00e188d8();
    if (controlled != nullptr) {
        void* const x = host.unit_vcall_13c(controlled);
        const std::int32_t slot = at<std::int32_t>(x, 0x1B0);
        if (slot != 8 && !host.party_slot_is_ai_held_00927f10(slot)) {
            host.unit_send_role_transfer_0077c470(host.unit_vcall_13c(host.global_00e188d8()), 2, 0);
        }
        controlled = host.global_00e188d8();
        if (controlled != nullptr && at<void*>(screen, 0x610) != controlled) {
            auto& shown = *static_cast<std::uint8_t*>(host.image_global(0x00E19515));
            if (shown == 0) shown = 1;
        }
    }
    at<void*>(screen, 0x610) = controlled;

    float mouse_x;
    float mouse_y;
    if (at<std::uint8_t>(game(), 0x634) != 0) {
        mouse_x = f32_bits(0x3F000000u);  // 00CE3800
        mouse_y = f32_bits(0x3F000000u);
    } else {
        std::uint8_t* const manager = static_cast<std::uint8_t*>(host.gui_manager_get_004c12b0());
        mouse_x = at<float>(manager, 0x5C);
        mouse_y = at<float>(manager, 0x60);
    }
    {
        const float cx = pc24(pc24(pc24(static_cast<double>(mouse_x) - image_float(0x00E08890)) * F(0x30)) + F(0x54C));
        const float cy = pc24(pc24(pc24(static_cast<double>(image_float(0x00E08894)) - mouse_y) * F(0x34)) + F(0x550));
        F(0x554) = cx;
        F(0x558) = cy;
    }

    // Zoom transition.
    const float rem = F(0x5AC);
    if (rem > 0.0f) {
        if (!(dt < rem || dt != dt)) {
            F(0x5AC) = 0.0f;
            F(0x30) = F(0x5B0);
            F(0x34) = F(0x5B4);
            F(0x54C) = F(0x5E8);
            F(0x550) = F(0x5EC);
            F(0x5D4) = f32_bits(0x3F800000u);
            F(0x5B8) = F(0x5BC);
            F(0x5D0) = 0.0f;
            F(0x5D8) = static_cast<float>(level());
            F(0x5DC) = F(0x5E0);
            F(0x5C4) = F(0x5C8);
        } else {
            const float f = pc24(static_cast<double>(dt) / rem);
            F(0x30) = lerp(F(0x30), F(0x5B0), f);
            F(0x34) = lerp(F(0x34), F(0x5B4), f);
            const float dx = pc24(static_cast<double>(F(0x5E8)) - F(0x54C));
            const float dy = pc24(static_cast<double>(F(0x5EC)) - F(0x550));
            const float sx = pc24(static_cast<double>(dx) * f);
            const float sy = pc24(static_cast<double>(dy) * f);
            F(0x54C) = pc24(static_cast<double>(sx) + F(0x54C));
            F(0x550) = pc24(static_cast<double>(sy) + F(0x550));
            F(0x5B8) = lerp(F(0x5B8), F(0x5BC), f);
            F(0x5DC) = lerp(F(0x5DC), F(0x5E0), f);
            F(0x5C4) = lerp(F(0x5C4), F(0x5C8), f);
            F(0x5D0) = pc24(static_cast<double>(F(0x5D0)) - f);
            F(0x5D4) = pc24(static_cast<double>(F(0x5D4)) + f);
            const float e = F(0x5D8);
            F(0x5D8) = pc24(e + pc24(pc24(static_cast<double>(level()) - e) * f));
            F(0x5AC) = pc24(static_cast<double>(F(0x5AC)) - dt);
        }
    }
    {
        const float* const scales = static_cast<const float*>(host.image_global(0x00F89AE4));
        const float* const crossfade = static_cast<const float*>(host.image_global(0x00E0CF50));
        if (F(0x5AC) > 0.0f) {
            F(0x5C0) = F(0x5B8);
        } else {
            void* const config = host.global_config_00432650();
            F(0x5C0) = pc24(static_cast<double>(scales[level()]) * at<float>(config, 0x2BC));
        }
        const float remaining = F(0x5AC);
        F(0x5CC) = remaining > 0.0f ? F(0x5C4) : crossfade[level()];
        F(0x5E4) = remaining > 0.0f ? F(0x5DC) : static_cast<float>(host.hud_map_zoom_value_0059abc0(screen));
    }

    // Filter bar and its six slots.
    const bool vis = !gate_closed();
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x100), vis ? 1 : 0);
    for (std::int32_t i = 0; i < 6; ++i) {
        void* const slot = at<void*>(screen, 0x108 + 4 * i);
        if (slot == nullptr) {
            if (i == 5) {
                at<void*>(screen, 0x11C) = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0xEC), nullptr);
                const float* const size = host.widget_get_size_00aa6740(at<void*>(screen, 0x108));
                host.widget_vcall_58_set_pair(at<void*>(screen, 0x11C), size);
            } else {
                at<void*>(screen, 0x108 + 4 * i) =
                    host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x104), nullptr);
            }
            continue;
        }
        host.widget_vcall_34_set_visible(slot, vis ? 1 : 0);
        if (!vis) continue;
        const std::int32_t n = i == 5 ? at<std::int32_t>(screen, 0x128) : i;
        float p[3];
        host.widget_get_resolved_position_00aa6750(at<void*>(screen, 0x100), p);
        const float* const size = host.widget_get_size_00aa6740(at<void*>(screen, 0x108));
        const std::int32_t depth = i != 5 ? -10 : -1;
        const float x_off = pc24(pc24(pc24(static_cast<double>(n) * size[0]) * f64_bits(0x3FF19999A0000000ull)) +
                                 f64_bits(0x3F8DDDDDDDDDDDDEull));
        const float position[3] = {pc24(static_cast<double>(x_off) + p[0]),
                                   pc24(static_cast<double>(p[1]) + f64_bits(0x3F8C71C720000000ull)),
                                   pc24(static_cast<double>(p[2]) + static_cast<float>(depth))};
        host.widget_set_resolved_position_00aa8240(at<void*>(screen, 0x108 + 4 * i), position);
        if (i != 5) host.widget_vcall_88(at<void*>(screen, 0x108 + 4 * i), i, 0, f32_bits(0x3F800000u));
    }

    float alpha = 0.0f;
    if (vis) {
        const float t0 = F(0x12C);
        const float c = pc24(static_cast<double>(t0) + 1.0);
        const float clock = host.global_00f876a4();
        if (c > clock) {
            alpha = f32_bits(0x3F800000u);
        } else if (pc24(static_cast<double>(t0) + f64_bits(0x4000000000000000ull)) > clock) {  // 00D7A308
            alpha = pc24(1.0 - pc24(static_cast<double>(clock) - c));
        }
        host.hud_map_label_center_0059d350(screen);
    }
    host.widget_vcall_4c_set_alpha(at<void*>(screen, 0x120), alpha);
    host.widget_vcall_4c_set_alpha(at<void*>(screen, 0x124), alpha);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x120), vis ? 1 : 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x124), vis ? 1 : 0);

    // Pointer pick.
    auto stick_magnitude = [&]() {
        const float a = std::fabs(at<float>(input(), 0x35F4));
        const float b = std::fabs(at<float>(input(), 0x35C4));
        return b > a ? b : a;
    };
    if (at<std::uint8_t>(screen, 0x528) != 0) {
        const float m = stick_magnitude();
        if (f64_bits(0x3FB99999A0000000ull) > m) at<std::uint8_t>(screen, 0x528) = 0;
    } else {
        const float m = stick_magnitude();
        if (m > f32_bits(0x3F000000u)) cc13_sweepB10_hud_map_update_pointer_pick_005a3210(screen, host);
    }

    // Spectate shortcut.
    if (at<std::uint8_t>(at<void*>(input(), 0x278C), 0xB) != 0 && at<void*>(screen, 0x520) != nullptr &&
        host.unit_selectable_00645160(at<void*>(screen, 0x520), 1) &&
        host.global_00e188d8() != at<void*>(screen, 0x520)) {
        host.hud_set_spectated_unit_00647300(at<void*>(gui(), 0x40), at<void*>(screen, 0x520));
        at<std::uint32_t>(at<void*>(gui(), 0xCC), 0xB8) = 0;
    }

    // Filter cycle.
    if (vis && host.input_action_was_pressed_004c43c0(game(), 0x120) &&
        !host.overlay_busy_006529e0(at<void*>(gui(), 0xBC))) {
        at<std::int32_t>(screen, 0x128) += 1;
        if (at<std::int32_t>(screen, 0x128) == 5) at<std::int32_t>(screen, 0x128) = 0;
        F(0x12C) = host.global_00f876a4();
        static const char* const kFilters[5] = {"ingame.filters_nofilter", "ingame.filters_health",
                                                "ingame.filters_cbinfo", "ingame.filters_powerups",
                                                "ingame.filters_failures"};  // 00CF0DA4..00CF0D44
        const std::uint32_t filter = at<std::uint32_t>(screen, 0x128);
        if (filter <= 4) host.gui_text_set_localised_cstring_00abbe50(at<void*>(screen, 0x124), kFilters[filter], 1);
        host.hud_map_label_center_0059d350(screen);
    }

    // Wheel zoom start.
    {
        std::uint8_t* const wheel = input() + 0x24C0;
        if (at<std::uint8_t>(wheel, 0x28) != 0 && at<std::uint8_t>(wheel, 0x20) == 0) {
            const bool zoom_in = 0.0f > at<float>(input(), 0x24E4) && level() < 3;
            const bool zoom_out = at<float>(input(), 0x24E4) > 0.0f /* 00D7A218 */ &&
                                  level() > at<std::int32_t>(screen, 0x5FC);
            if (zoom_in || zoom_out) {
                const float* const scales = static_cast<const float*>(host.image_global(0x00F89AE4));
                const float* const crossfade = static_cast<const float*>(host.image_global(0x00E0CF50));
                const float* const extents = static_cast<const float*>(host.image_global(0x00E0CF40));
                void* config = host.global_config_00432650();
                std::int32_t lv = level();
                F(0x5B8) = pc24(static_cast<double>(scales[lv]) * at<float>(config, 0x2BC));
                F(0x5D8) = static_cast<float>(lv);
                F(0x5C4) = crossfade[lv];
                if (zoom_in) {
                    lv += 1;
                    at<std::uint8_t>(screen, 0x40) = 1;
                    at<std::uint8_t>(screen, 0x41) = 0;
                } else {
                    lv -= 1;
                    at<std::uint8_t>(screen, 0x41) = 1;
                    at<std::uint8_t>(screen, 0x40) = 0;
                }
                F(0x5B0) = F(0x30);
                at<std::int32_t>(screen, 0x2C) = lv;
                F(0x5B4) = F(0x34);
                F(0x5E8) = F(0x54C);
                F(0x5EC) = F(0x550);
                F(0x5D0) = f32_bits(0x3F800000u);
                F(0x5D4) = 0.0f;
                config = host.global_config_00432650();
                F(0x5BC) = pc24(static_cast<double>(scales[level()]) * at<float>(config, 0x2BC));
                F(0x5C8) = crossfade[level()];
                F(0x5E0) = static_cast<float>(host.hud_map_zoom_value_0059abc0(screen));
                config = host.global_config_00432650();
                F(0x5AC) = at<float>(config, 0x2B8);
                const float extent = extents[level()];
                F(0x5B0) = extent;
                F(0x5B4) = pc24(static_cast<double>(extent) * 0.75);  // 00CEC9D8

                // Every mapped value of the map +4F4h gets byte +9h = 1.
                std::uint8_t* const map = static_cast<std::uint8_t*>(screen) + 0x4F4;
                struct {
                    void* container;
                    void* node;
                } it = {map, at<void*>(at<void*>(map, 0x4), 0x0)};
                for (;;) {
                    void* const head = at<void*>(map, 0x4);
                    if (it.container == nullptr || it.container != map) host.crt_invalid_parameter_noinfo_00bf6713();
                    if (it.node == head) break;
                    if (it.container == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
                    if (it.node == at<void*>(it.container, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
                    at<std::uint8_t>(at<void*>(it.node, 0x10), 0x9) = 1;
                    host.tree_iterator_increment_00535110(&it);
                }

                const double hx = image_float(0x00E08890);
                if (zoom_out) {
                    const float n_extent = F(0x5B0);
                    const float old_extent = extents[level() + 1];  // 00E0CF44[level]
                    const float k = pc24(pc24(1.0 - pc24(static_cast<double>(old_extent) / n_extent)) * n_extent);
                    const float px = F(0x54C);
                    const float u = pc24(pc24(pc24(static_cast<double>(F(0x554)) - px) / F(0x30)) + hx);
                    F(0x5E8) = pc24(px - pc24(pc24(static_cast<double>(u) - hx) * k));
                    const float py = F(0x550);
                    const double hy = image_float(0x00E08894);
                    const float v = pc24(pc24(pc24(static_cast<double>(py) - F(0x558)) / F(0x34)) + hy);
                    F(0x5EC) = pc24(py + pc24(pc24(pc24(static_cast<double>(v) - hy) * k) * 0.75));
                } else {
                    const float older_extent = extents[level() - 1];  // 00E0CF3C[level]
                    const float n_extent = F(0x5B0);
                    const float k =
                        pc24(pc24(1.0 - pc24(static_cast<double>(n_extent) / older_extent)) * older_extent);
                    const float px = F(0x54C);
                    const float u = pc24(pc24(pc24(static_cast<double>(F(0x554)) - px) / F(0x30)) + hx);
                    F(0x5E8) = pc24(px + pc24(pc24(static_cast<double>(u) - hx) * k));
                    const float py = F(0x550);
                    const double hy = image_float(0x00E08894);
                    const float v = pc24(pc24(pc24(static_cast<double>(py) - F(0x558)) / F(0x34)) + hy);
                    F(0x5EC) = pc24(py - pc24(pc24(pc24(static_cast<double>(v) - hy) * k) * 0.75));
                }
                cc13_sweepB_clamp_point_to_world_bounds_0059e120(&F(0x5E8), at<std::uint32_t>(screen, 0x5B0),
                                                                 at<std::uint32_t>(screen, 0x5B4), host);
            }
        }
    }

    // Panning.
    bool keys_moved = false;  // BL
    if (!host.input_action_was_pressed_004c43c0(game(), 0xC4) && !gate_closed()) {
        const float s = pc24(pc24(static_cast<double>(F(0x30)) * f64_bits(0x3FD99999A0000000ull)) * dt);
        float stick_dx = 0.0f;
        float stick_dy = 0.0f;
        if (at<std::uint8_t>(screen, 0x528) == 0) {
            const float a = at<float>(input(), 0x2544);
            const float b = at<float>(input(), 0x2514);
            const float na = f32_bits(0x80000000u) - a;  // SUBSS from -0.0f (00D7A208)
            const float p = pc24(static_cast<double>(b) * s);
            const float q = pc24(static_cast<double>(s) * na);
            stick_dx = p;
            stick_dy = q;
            F(0x554) = pc24(static_cast<double>(p) + F(0x554));
            F(0x558) = pc24(static_cast<double>(q) + F(0x558));
        }
        const float a2 = at<float>(input(), 0x2484);
        const float b2 = at<float>(input(), 0x24B4);
        const float na2 = f32_bits(0x80000000u) - a2;
        const float p2 = pc24(static_cast<double>(b2) * s);
        const float q2 = pc24(static_cast<double>(s) * na2);
        F(0x54C) = pc24(static_cast<double>(p2) + F(0x54C));
        F(0x550) = pc24(static_cast<double>(F(0x550)) + q2);
        F(0x554) = pc24(static_cast<double>(p2) + F(0x554));
        F(0x558) = pc24(static_cast<double>(q2) + F(0x558));
        if (!(p2 == 0.0f) || !(q2 == 0.0f)) {
            keys_moved = true;
            at<std::uint32_t>(screen, 0x524) = 0;
        } else if (!(stick_dx == 0.0f) || !(stick_dy == 0.0f)) {
            at<std::uint32_t>(screen, 0x524) = 0;
        }
    }

    const float lo_wide = static_cast<float>(f64_bits(0xBFB99999B0000000ull));    // 00CEFFA8
    const float hi_wide = static_cast<float>(f64_bits(0x3FF1999998000000ull));    // 00CEFFB8
    const float lo_narrow = static_cast<float>(f64_bits(0x3FA99999A0000000ull));  // 00D7A270
    const float hi_narrow = static_cast<float>(f64_bits(0x3FEE666660000000ull));  // 00CEFFB0
    const double v_lo = f64_bits(0x3FA99999A0000000ull);                          // 00D7A270
    const double v_hi = f64_bits(0x3FEE666660000000ull);                          // 00CEFFB0

    // Cursor clamp 1 (moves the pan).
    {
        const double hx = image_float(0x00E08890);
        const float px = F(0x54C);
        const float W = F(0x30);
        float u = pc24(pc24(pc24(static_cast<double>(F(0x554)) - px) / W) + hx);
        const float py = F(0x550);
        const float H = F(0x34);
        const double hy = image_float(0x00E08894);
        float v = pc24(pc24(pc24(static_cast<double>(py) - F(0x558)) / H) + hy);
        const bool wide = edge_flag();
        const float lo = wide ? lo_wide : lo_narrow;
        if (lo > u) {
            F(0x54C) = pc24(px - pc24(pc24(static_cast<double>(lo) - u) * W));
            u = edge_flag() ? lo_wide : lo_narrow;
        } else {
            const float hi = wide ? hi_wide : hi_narrow;
            if (u > hi) {
                F(0x54C) = pc24(px + pc24(pc24(static_cast<double>(u) - hi) * W));
                u = edge_flag() ? hi_wide : hi_narrow;
            }
        }
        if (v_lo > v) {
            F(0x550) = pc24(static_cast<double>(py) + pc24(pc24(v_lo - v) * H));
            v = f32_bits(0x3D4CCCCDu);  // 00CEFF54
        } else if (v > v_hi) {
            F(0x550) = pc24(static_cast<double>(py) - pc24(pc24(static_cast<double>(v) - v_hi) * H));
            v = f32_bits(0x3F733333u);  // 00CEFF58
        }
        const float cx = pc24(pc24(pc24(static_cast<double>(u) - image_float(0x00E08890)) * W) + F(0x54C));
        const float cy = pc24(pc24(static_cast<double>(H) * pc24(image_float(0x00E08894) - static_cast<double>(v))) +
                              F(0x550));
        F(0x554) = cx;
        F(0x558) = cy;
    }

    // Followed unit.
    void* const followed = at<void*>(screen, 0x524);
    if (followed != nullptr) {
        if (at<std::uint8_t>(followed, 0x5D) == 0 && at<std::uint8_t>(followed, 0x5E) == 0) {
            float buffer_a[3];
            float buffer_b[3];
            const float x = host.unit_vcall_c4_position(followed, buffer_a)[0];
            const float z = host.unit_vcall_c4_position(at<void*>(screen, 0x524), buffer_b)[2];
            F(0x54C) = x;
            F(0x550) = z;
            F(0x554) = F(0x54C);
            F(0x558) = F(0x550);
        } else {
            at<std::uint32_t>(screen, 0x524) = 0;
        }
    }

    cc13_sweepB_clamp_point_to_world_bounds_0059e120(&F(0x54C), at<std::uint32_t>(screen, 0x30),
                                                     at<std::uint32_t>(screen, 0x34), host);

    // Cursor clamp 2 (the pan stays).
    {
        const float px = F(0x54C);
        const bool wide = edge_flag();
        const float W = F(0x30);
        const double hx = image_float(0x00E08890);
        const float u = pc24(pc24(pc24(static_cast<double>(F(0x554)) - px) / W) + hx);
        const float py = F(0x550);
        const float H = F(0x34);
        const double hy = image_float(0x00E08894);
        const float v = pc24(pc24(pc24(static_cast<double>(py) - F(0x558)) / H) + hy);
        const float hi = wide ? hi_wide : hi_narrow;
        const float lo = wide ? lo_wide : lo_narrow;
        float uc;
        if (lo > u) {
            uc = lo;
        } else if (u > hi) {
            uc = hi;
        } else {
            uc = u;
        }
        float vc;
        if (v_lo > v) {
            vc = f32_bits(0x3D4CCCCDu);
        } else if (v > v_hi) {
            vc = f32_bits(0x3F733333u);
        } else {
            vc = v;
        }
        F(0x554) = pc24(static_cast<double>(px) + pc24(static_cast<double>(W) * pc24(static_cast<double>(uc) - hx)));
        F(0x558) = pc24(pc24(static_cast<double>(H) * pc24(image_float(0x00E08894) - static_cast<double>(vc))) + py);
        if (keys_moved) {
            F(0x554) = F(0x54C);
            F(0x558) = F(0x550);
        }
    }
    if (at<std::uint8_t>(screen, 0x5F0) != 0) {
        F(0x554) = F(0x5F4);
        F(0x558) = F(0x5F8);
        F(0x54C) = F(0x554);
        F(0x550) = F(0x558);
    }

    if (*static_cast<const std::int32_t*>(host.image_global(0x00E08310)) == 2) {
        bool send = host.top_populated_level_004f7740() == 2;
        if (host.top_populated_level_004f7740() == 3) {
            std::uint8_t* const g = gui();
            const bool flag = at<std::uint8_t>(screen, 0x5F0) != 0 && at<std::uint8_t>(at<void*>(g, 0x60), 0x5) != 0 &&
                              at<std::uint8_t>(at<void*>(g, 0x60), 0x70) != 0;
            send = send || flag;
            bool busy = host.overlay_busy_006529e0(at<void*>(g, 0xBC));
            if (!busy) busy = at<std::uint8_t>(at<void*>(gui(), 0xC0), 0x4) != 0;
            send = send || busy;
        }
        if (send && at<std::uint8_t>(game(), 0x634) == 0) {
            const float y =
                pc24(pc24(pc24(static_cast<double>(F(0x550)) - F(0x558)) / F(0x34)) + image_float(0x00E08894));
            const float x =
                pc24(pc24(pc24(static_cast<double>(F(0x554)) - F(0x54C)) / F(0x30)) + image_float(0x00E08890));
            host.gui_manager_set_cursor_00aa1fa0(host.gui_manager_get_004c12b0(), x, y);
        }
    }

    at<void*>(screen, 0x52C) = at<void*>(screen, 0x520);
    void* const unit = host.global_00e188d8();
    void* const target = unit != nullptr ? host.controlled_target_00927880(unit) : nullptr;
    at<void*>(screen, 0x534) = target;
    if (target != nullptr && at<std::uint32_t>(target, 0x54) == at<std::uint32_t>(host.global_00e188d8(), 0x54)) {
        at<void*>(screen, 0x534) = nullptr;
    }
    if (at<void*>(screen, 0x52C) == at<void*>(screen, 0x534)) at<void*>(screen, 0x52C) = nullptr;
    cc13_sweepB_hud_map_resolve_icon_unit_005a48c0(screen, &at<void*>(screen, 0x52C), &at<void*>(screen, 0x530), false,
                                                   host);
    cc13_sweepB_hud_map_resolve_icon_unit_005a48c0(screen, &at<void*>(screen, 0x534), &at<void*>(screen, 0x538), true,
                                                   host);
    cc13_sweepB5_hud_map_load_background_005b2d70(screen, host);

    {
        std::uint8_t* const camera = gui() + 0xF0;
        at<float>(camera, 0x0) = F(0x554);
        at<float>(camera, 0x4) = 0.0f;
        at<float>(camera, 0x8) = F(0x558);
        at<std::uint8_t>(gui(), 0xFC) = 1;
    }
    cc13_sweepB5_hud_map_update_marker_models_005b25d0(screen, dt, 0, host);

    // Shader values.
    {
        bool shaped = false;
        float a = 0.0f;
        if (at<std::uint8_t>(screen, 0x40) != 0) {
            void* config = host.global_config_00432650();
            const std::int32_t next = level() + 1;
            F(0x1C) = pc24(static_cast<double>(next) - pc24(static_cast<double>(F(0x5AC)) / at<float>(config, 0x2B8)));
            config = host.global_config_00432650();
            const float t = pc24(static_cast<double>(F(0x5AC)) / at<float>(config, 0x2B8));
            const float d = pc24(static_cast<double>(t) - 0.5);  // 00D7A280
            a = std::fabs(pc24(static_cast<double>(d) + d));
            shaped = true;
        } else if (at<std::uint8_t>(screen, 0x41) != 0) {
            void* config = host.global_config_00432650();
            const float t = pc24(static_cast<double>(F(0x5AC)) / at<float>(config, 0x2B8));
            F(0x1C) = pc24(static_cast<double>(t) + (level() + 1));
            config = host.global_config_00432650();
            const float t2 = pc24(static_cast<double>(F(0x5AC)) / at<float>(config, 0x2B8));
            const float d = pc24(static_cast<double>(t2) - 0.5);
            a = std::fabs(pc24(static_cast<double>(d) + d));
            shaped = true;
        }
        if (shaped) F(0x28) = pc24(static_cast<double>(a) * pc24(static_cast<double>(a) * a));

        const float s = F(0x1C);
        const float c = pc24(static_cast<double>(s) - 2.0);  // 00D7A308
        float clamped;
        if (0.0f > c) {
            clamped = 0.0f;
        } else if (c > f32_bits(0x3F800000u)) {
            clamped = f32_bits(0x3F800000u);
        } else {
            clamped = c;
        }
        F(0x20) = pc24(1.0 - clamped);
        const float q = pc24(1.0 - pc24(pc24(static_cast<double>(s) - 1.0) * 0.5));
        F(0x24) = q;
        F(0x24) = pc24(1.0 - pc24(static_cast<double>(q) * pc24(static_cast<double>(q) * q)));
    }

    if (at<std::int32_t>(game(), 0x1FE4) != 0) {
        std::uint8_t* const g = game();
        host.player_timer_add_009052e0(at<void*>(g, 0x21A0), at<std::int32_t>(g, 0x18EC), dt);
        const float timer = F(0x60C);
        if (0.0f > timer) {
            host.session_client_send_local_score_00772a40(game() + 0x1EF0);
            F(0x60C) = f32_bits(0x3F800000u);
        } else {
            F(0x60C) = pc24(static_cast<double>(timer) - dt);
        }
        if (!host.input_action_was_pressed_004c43c0(game(), 0x109)) {
            std::int32_t mode = 0;
            if (host.input_action_was_pressed_004c43c0(game(), 0x10B)) {
                mode = 2;
            } else if (host.input_action_was_pressed_004c43c0(game(), 0x10A)) {
                mode = 1;
            }
            if (mode != 0) host.map_ping_008e2ee0(mode, F(0x554), F(0x558));
        }
    }

    std::int32_t state = 0;
    void* const own = host.global_00e188d8();
    if (own != nullptr && host.entity_vcall_5c_is_kind(own, 6)) {
        void* const group = host.nav_zone_group_0082ada0(at<void*>(host.global_00e188d8(), 0x538), 0);
        if (host.avoid_zone_first_containing_004178f0(group, &F(0x554)) != 0) state = 2;
    }
    if (at<void*>(screen, 0x520) != nullptr) state = 1;
    host.gui_manager_set_pointer_state_00aa0e60(host.gui_manager_get_004c12b0(), state);
}

// ---------------------------------------------------------------------------
// 005A3210 BSP_HudMapScreen_UpdatePointerPick
// __fastcall(ECX = screen), RET; caller 005B45C0. Image facts (asm
// 005A3210..005A3603; x87 forms from tools/x87_dc_form_scan.py):
// - Returns at once when byte g+5h and byte g+70h are set (g =
//   [[00E198C4]+60h]). 005A1310(screen, 0).
// - x = in+35F4h, y = in+35C4h, in = [004BEC00()+4] re-fetched before every
//   test; constants 0.25f (00CE3868) and -0.25f (00CF00A8), COMISS (NaN
//   fails every test): x > 0.25: y < -0.25 -> (1, 5), y > 0.25 -> (0, 4),
//   else (4, 5); x < -0.25: y < -0.25 -> (3, 7), y > 0.25 -> (2, 6), else (6,
//   7); otherwise y < -0.25 -> (1, 3), y > 0.25 -> (0, 2), else return.
// - best = 0, dist = 1e10f (00CE4970); the map +4F4h (head [+4F8h]) walked
//   from head->left with the checked iterator (00BF6713 on a bad container
//   or end dereference, 00535110 to advance). Entry e = node+10h; skipped
//   unless byte e+8h; unit u = e+4h, skipped when u == +520h or u vtable
//   +5Ch(34h) or u vtable +5Ch(2Bh). b = u vtable +C4h(buf) (the buffer is
//   read: b.x = buf[0], b.z = buf[2]). dxn = ub - uc and dyn = vb - vc with
//   ub = (b.x - px) / W + 0.5, uc = (cx - px) / W + 0.5, vb = (py - b.z) /
//   H + 0.5, vc = (py - cy) / H + 0.5 (W/H = +30h/+34h, px/py = +54Ch/
//   +550h, cx/cy = +554h/+558h, 0.5 = [00E08890]/[00E08894], each value
//   stored as float). octant = (0 > dxn) + 2 * (0 > dyn) + 4 * (|dyn| >
//   |dxn|, FCOMIP; unordered fails). When it is one of the two: d = dyn *
//   dyn + dxn * dxn; dist > d (ordered) and u != +524h -> best = e, dist = d.
// - best: p = [best+4] vtable +C4h(buf2); +554h = buf2[0], +558h = buf2[2];
//   byte +528h = 1; +524h = [best+4].
// No draw from the shared RNG 00BD2F10.
// ---------------------------------------------------------------------------
void cc13_sweepB10_hud_map_update_pointer_pick_005a3210(void* screen, Cc13SweepB10Host& host) {
    {
        std::uint8_t* const g = at<std::uint8_t*>(host.global_00e198c4(), 0x60);
        if (at<std::uint8_t>(g, 0x5) != 0 && at<std::uint8_t>(g, 0x70) != 0) return;
    }
    host.hud_map_pick_nearest_unit_005a1310(screen, 0);
    auto input = [&]() { return at<std::uint8_t*>(host.input_manager_singleton_004bec00(), 0x4); };
    const float plus = f32_bits(0x3E800000u);   // 00CE3868
    const float minus = f32_bits(0xBE800000u);  // 00CF00A8
    std::int32_t first;
    std::int32_t second;
    if (at<float>(input(), 0x35F4) > plus) {
        if (minus > at<float>(input(), 0x35C4)) {
            first = 1;
            second = 5;
        } else if (at<float>(input(), 0x35C4) > plus) {
            first = 0;
            second = 4;
        } else {
            first = 4;
            second = 5;
        }
    } else if (minus > at<float>(input(), 0x35F4)) {
        if (minus > at<float>(input(), 0x35C4)) {
            first = 3;
            second = 7;
        } else if (at<float>(input(), 0x35C4) > plus) {
            first = 2;
            second = 6;
        } else {
            first = 6;
            second = 7;
        }
    } else if (minus > at<float>(input(), 0x35C4)) {
        first = 1;
        second = 3;
    } else if (at<float>(input(), 0x35C4) > plus) {
        first = 0;
        second = 2;
    } else {
        return;
    }

    std::uint8_t* const map = static_cast<std::uint8_t*>(screen) + 0x4F4;
    void* best = nullptr;
    float dist = f32_bits(0x501502F9u);  // 00CE4970 1e10
    struct {
        void* container;
        void* node;
    } it = {map, at<void*>(at<void*>(map, 0x4), 0x0)};
    for (;;) {
        void* const head = at<void*>(map, 0x4);
        if (it.container == nullptr || it.container != map) host.crt_invalid_parameter_noinfo_00bf6713();
        if (it.node == head) break;
        if (it.container == nullptr) host.crt_invalid_parameter_noinfo_00bf6713();
        if (it.node == at<void*>(it.container, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
        void* const entry = at<void*>(it.node, 0x10);
        void* const unit = at<void*>(entry, 0x4);
        if (at<std::uint8_t>(entry, 0x8) != 0 && unit != at<void*>(screen, 0x520) &&
            !host.entity_vcall_5c_is_kind(unit, 0x34) && !host.entity_vcall_5c_is_kind(at<void*>(entry, 0x4), 0x2B)) {
            float buf[3];
            host.unit_vcall_c4_position(at<void*>(entry, 0x4), buf);
            const float px = at<float>(screen, 0x54C);
            const float W = at<float>(screen, 0x30);
            const double hx = *static_cast<const float*>(host.image_global(0x00E08890));
            const float ub = pc24(pc24(pc24(static_cast<double>(buf[0]) - px) / W) + hx);
            const float uc = pc24(hx + pc24(pc24(static_cast<double>(at<float>(screen, 0x554)) - px) / W));
            const float dxn = pc24(static_cast<double>(ub) - uc);
            const float py = at<float>(screen, 0x550);
            const float H = at<float>(screen, 0x34);
            const double hy = *static_cast<const float*>(host.image_global(0x00E08894));
            const float vb = pc24(pc24(pc24(static_cast<double>(py) - buf[2]) / H) + hy);
            const float vc = pc24(hy + pc24(pc24(static_cast<double>(py) - at<float>(screen, 0x558)) / H));
            const float dyn = pc24(static_cast<double>(vb) - vc);
            std::int32_t octant = 0;
            if (0.0f > dxn) octant = 1;
            if (0.0f > dyn) octant += 2;
            if (std::fabs(dyn) > std::fabs(dxn)) octant += 4;
            if (octant == first || octant == second) {
                const float dy2 = pc24(static_cast<double>(dyn) * dyn);
                const float dx2 = pc24(static_cast<double>(dxn) * dxn);
                const float d = pc24(static_cast<double>(dy2) + dx2);
                if (dist > d && at<void*>(entry, 0x4) != at<void*>(screen, 0x524)) {
                    best = entry;
                    dist = d;
                }
            }
        }
        host.tree_iterator_increment_00535110(&it);
    }

    if (best != nullptr) {
        float buf2[3];
        host.unit_vcall_c4_position(at<void*>(best, 0x4), buf2);
        at<float>(screen, 0x554) = buf2[0];
        at<float>(screen, 0x558) = buf2[2];
        at<std::uint8_t>(screen, 0x528) = 1;
        at<void*>(screen, 0x524) = at<void*>(best, 0x4);
    }
}

// ---------------------------------------------------------------------------
// 00617470 BSP_HudPowerupManagerScreen_UpdatePowerupBars
// __fastcall(ECX = carousel), RET; callers 00619C50, 0061BEA0. sp =
// [[00E198C4]+C0h] (re-read where the image re-reads it), bc =
// [[00E198C4]+BCh]. Image facts (asm 00617470..0061782C):
// - Returns when the vector sp+148h..+14Ch is null or empty.
// - bars[4] = 0. fetch(i): checked index (00BF6713 when the vector is null or
//   i >= size), key = [[sp+148h][i] + 4h], rec = 00617030([00F88C30] +
//   [game+18ECh] * 0Ch + 14Ch, &key), f = (float)008E4BB0(rec); a bar takes f
//   only when f > 0 (FCOMIP against FLDZ, ordered).
// - bars[+0h] <- fetch(+4h). Down: for k = +0h - 1 .. 0, i = (n + i - 1) %
//   size (unsigned DIV; n and the divisor are the size, 0 for a null vector),
//   bars[k] <- fetch(i). Up: for k = +0h + 1 .. 3, ++i; i >= size: leave
//   when the vector is null or size < 4, else i = 0 (a null vector also
//   leaves before the test); bars[k] <- fetch(i). +0h is not range-checked
//   (the image writes [ESP + k*4 + 18h] unguarded).
// - off = byte +3Fh ? (byte +3Ch ? (1 - +40h) : (+40h + 1.0)) * bc+2A8h :
//   bc+2A8h (x87, each operation rounded).
// - For j = 0..3 (widget [sp+38h+4j]): bars[j] == 0 (UCOMISS with 00D7A218,
//   JNP) -> vtable +34h(0); else vtable +34h(1), 00ABE6E0(widget, bars[j],
//   bc+2A4h - off, 0.0f, 1.0f) and, unless byte sp+20Ch, vtable +4Ch(0.5f)
//   (00CE3800). off += bc+2A8h.
// No draw from the shared RNG 00BD2F10.
// ---------------------------------------------------------------------------
// The image's DIV has no zero guard (a null vector faults there); C4724 is
// silenced instead of adding one.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4724)
#endif
void cc13_sweepB10_powerup_update_bars_00617470(void* carousel, Cc13SweepB10Host& host) {
    auto support = [&]() { return at<std::uint8_t*>(host.global_00e198c4(), 0xC0); };
    auto count = [](std::uint8_t* sp) {
        return static_cast<std::uint32_t>((at<std::uint8_t*>(sp, 0x14C) - at<std::uint8_t*>(sp, 0x148)) >> 2);
    };
    std::uint8_t* sp = support();
    if (at<std::uint8_t*>(sp, 0x148) == nullptr || count(sp) == 0) return;

    auto fetch = [&](std::uint8_t* block, std::uint32_t index) {
        if (at<std::uint8_t*>(block, 0x148) == nullptr || !(index < count(block))) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        void* key = at<void*>(at<void**>(block, 0x148)[index], 0x4);
        std::uint8_t* const map = static_cast<std::uint8_t*>(at<void*>(host.image_global(0x00F88C30), 0x0)) +
                                  at<std::int32_t>(host.global_00e188a8(), 0x18EC) * 0xC + 0x14C;
        void* const record = host.powerup_map_subscript_00617030(map, &key);
        return static_cast<float>(host.powerup_record_cooldown_fraction_008e4bb0(record));
    };

    float bars[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    std::uint32_t index = at<std::uint32_t>(carousel, 0x4);
    {
        const float f = fetch(sp, index);
        if (f > 0.0f) bars[at<std::int32_t>(carousel, 0x0)] = f;
    }
    index = at<std::uint32_t>(carousel, 0x4);
    for (std::int32_t k = at<std::int32_t>(carousel, 0x0) - 1; k >= 0; --k) {
        sp = support();
        const std::uint32_t n = at<std::uint8_t*>(sp, 0x148) != nullptr ? count(sp) : 0;
        const std::uint32_t divisor = at<std::uint8_t*>(sp, 0x148) != nullptr ? count(sp) : 0;
        index = (n + index - 1) % divisor;
        const float f = fetch(sp, index);
        if (f > 0.0f) bars[k] = f;
    }
    index = at<std::uint32_t>(carousel, 0x4);
    for (std::int32_t k = at<std::int32_t>(carousel, 0x0) + 1; k < 4; ++k) {
        sp = support();
        ++index;
        if (at<std::uint8_t*>(sp, 0x148) == nullptr) break;
        if (!(index < count(sp))) {
            if (count(sp) < 4) break;
            index = 0;
        }
        const float f = fetch(sp, index);
        if (f > 0.0f) bars[k] = f;
    }

    std::uint8_t* gui = static_cast<std::uint8_t*>(host.global_00e198c4());
    float off;
    if (at<std::uint8_t>(carousel, 0x3F) != 0) {
        const float x = at<float>(carousel, 0x40);
        if (at<std::uint8_t>(carousel, 0x3C) != 0) {
            off = pc24(pc24(1.0 - x) * at<float>(at<void*>(gui, 0xBC), 0x2A8));
        } else {
            off = pc24(pc24(static_cast<double>(x) + 1.0) * at<float>(at<void*>(gui, 0xBC), 0x2A8));  // 00D7A210
        }
    } else {
        off = at<float>(at<void*>(gui, 0xBC), 0x2A8);
    }
    for (std::uint32_t j = 0; j < 4; ++j) {
        const std::uint32_t slot = 0x38 + 4 * j;
        const float value = bars[j];
        if (value == 0.0f) {  // JNP: equal and ordered
            host.widget_vcall_34_set_visible(at<void*>(at<void*>(gui, 0xC0), slot), 0);
        } else {
            host.widget_vcall_34_set_visible(at<void*>(at<void*>(gui, 0xC0), slot), 1);
            std::uint8_t* const g = static_cast<std::uint8_t*>(host.global_00e198c4());
            const float top = pc24(static_cast<double>(at<float>(at<void*>(g, 0xBC), 0x2A4)) - off);
            host.gui_widget_set_quad_floats_00abe6e0(at<void*>(at<void*>(g, 0xC0), slot), value, top, 0.0f,
                                                     f32_bits(0x3F800000u));
            gui = static_cast<std::uint8_t*>(host.global_00e198c4());
            if (at<std::uint8_t>(at<void*>(gui, 0xC0), 0x20C) == 0) {
                host.widget_vcall_4c_set_alpha(at<void*>(at<void*>(gui, 0xC0), slot), f32_bits(0x3F000000u));
            }
        }
        gui = static_cast<std::uint8_t*>(host.global_00e198c4());
        off = pc24(static_cast<double>(at<float>(at<void*>(gui, 0xBC), 0x2A8)) + off);
    }
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

}  // namespace bsp
