// cc13_sweepB5: fifth pass of the segment 15-51 leftover sweep (packet
// cc13-sweepB5, docs/CC13_SWEEPB5.md). Names are hypotheses; see the header
// for the hooks.
//
// x87 note (as cc13_sweepB..B4): float expressions follow the image's
// operation order with a float store wherever the image stores to a float
// slot; intermediate x87 values are kept in double (53-bit precision control),
// and double constants are applied in double. EH frames are noted, not
// modelled. Register-register x87 operand directions were read from the bytes
// (local/sweepB5_dis.py decodes the DC/DE forms by the Intel encoding).

#include "bsp/cc13_sweepB5.hpp"
#include "bsp/engine_seg1_cc13.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

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

// 00414260 BSP_Vector3f_ScaleTo, __thiscall(ECX = this, out, float s), RET 8:
// out = this * s per component (float stores), EAX = out. Delegates to the fixture-tested
// 24-bit model (cc13-fixture9).
float* vector3_scale_to_00414260(const float* self, float* out, float s) {
    return engine_seg1_cc13::vector3_scale_to_00414260(self, out, s);
}

// 004142A0 BSP_Vector3f_AddTo, __thiscall(ECX = this, out, v), RET 8:
// out = v + this per component (float stores), EAX = out.
float* vector3_add_to_004142a0(const float* self, float* out, const float* v) {
    out[0] = v[0] + self[0];
    out[1] = v[1] + self[1];
    out[2] = v[2] + self[2];
    return out;
}

// The inlined native-string build from a literal: {0, null}, 0041DD40(&s,
// length, 1), then when data is non-null 00BF7680(data, literal, length + 1)
// with the length the resize left.
void build_literal(Cc13SweepBNativeString& text, std::uint32_t literal, std::uint32_t length,
                   Cc13SweepBHost& host) {
    text.length = 0;
    text.data = nullptr;
    host.native_string_resize_0041dd40(&text, length, 1);
    if (text.data != nullptr) {
        host.crt_memcpy_00bf7680(text.data, host.image_global(literal), text.length + 1);
    }
}

// The inlined native-string release: when data is non-null, the data goes
// back to the sized pool with length + 1 bytes.
void release_string(Cc13SweepBNativeString& text, Cc13SweepBHost& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

// _ftol2 (00BF7420) on an ST0 value: truncation, 80000000h when out of range
// (as sweepB4's helper).
std::int32_t ftol_trunc(double value) {
    if (!(value > -2147483649.0 && value < 2147483648.0)) {
        return static_cast<std::int32_t>(0x80000000u);
    }
    return static_cast<std::int32_t>(value);
}

// The pointer stored in an image global (host.image_global gives its address).
void* image_pointer(Cc13SweepBHost& host, std::uint32_t address) {
    return *static_cast<void**>(host.image_global(address));
}

// Find the child named by a literal (built and released around 00AA7E00(row,
// &name, 1)); `use` runs on the child while the name is alive, as in the image.
template <class Use>
void with_child(void* row, std::uint32_t literal, std::uint32_t length, Cc13SweepB5Host& host, Use use) {
    Cc13SweepBNativeString name;
    build_literal(name, literal, length, host);
    use(host.widget_find_child_00aa7e00(row, &name, 1));
    release_string(name, host);
}

}  // namespace

// ---------------------------------------------------------------------------
// Segment 15
// ---------------------------------------------------------------------------

// 00528240 BSP_ControlLayoutScreen_Enter (ledger name). __thiscall(ECX =
// control-layout screen), RET; EBP frame aligned to 8, EH frame (handler
// 00C6B768, states 0..0Ch). Vtable 00CECF48 slot +18h; no direct callers.
//
// Game +61Fh clear: 00518D60(6, 0, 0, "FE.opt_title_control" (00CECFE8)),
// screen+18h = the "Main_Listbox" (00CEB334) child of +10h, the
// "ingame_Listbox" (00CECFD8) child gets vtable +34h(0). Set: +18h = the
// ingame_Listbox child, Main_Listbox +34h(0), then 004C9680. Names here are
// built with 0041E870.
// Then +18h vtable +34h(1), 00A9BEC0; template = "MainListbox_Text"
// (00CEC040, length 10h) child of +18h; for the three C strings at
// 00E08664..00E0866C: row = 00AAB4C0(template, 0), 00ABBE50(row, text, 1),
// 00A9D750(+18h, row, 0, 0). 00A9AC40(+18h, screen+8), 00A9C7C0(+18h,
// [00E08670]), vtable +60h(1), +80h(1), 00A9CF30(+18h, 1, 1).
// Grids (00528513..00528665): for i = 0, 1, j = 0..2, k = 0..2: v = i, or
// (i == 0) when byte screen+28h is set; the name is the C string
// [00E08674 + 0Ch*i + 4k] (inlined strlen + resize + memcpy); the child of
// [screen+1Ch+4j] gets 00ABAED0(&native string 00E18DC0[v][j][k] (8 bytes
// each, index k + 3j + 9v), 1).
// Command bar: "globals.back" (00CEB3A8, 0Ch), "", "", "globals.navigate"
// (00CEB394, 10h), "" built in that order; 0054B530([00E1930C], (0, e, 1),
// (B3h-as-dword -0x4D, navigate, 1), (0, "", 1), (0, "", 1), (-0x5D, back,
// 2)); released in reverse. Last: the "ingame_FrameBox" (00CECFC8, 0Fh)
// child of +14h gets vtable +34h(dword) whose low byte is game+61Fh -- the
// image stores the byte over the low byte of a stack slot holding the screen
// pointer and pushes the whole dword; the C++ builds the same merged value
// (exact on Win32; the callee's flag is a byte). No RNG draws.
void cc13_sweepB5_control_layout_screen_enter_00528240(void* screen, Cc13SweepB5Host& host) {
    auto with_assigned = [&](void* widget, std::uint32_t literal, auto use) {
        Cc13SweepBNativeString name;
        host.native_string_assign_0041e870(&name, static_cast<const char*>(host.image_global(literal)));
        use(host.widget_find_child_00aa7e00(widget, &name, 1));
        release_string(name, host);
    };
    if (at<std::uint8_t>(image_pointer(host, 0x00E188A8), 0x61F) == 0) {
        Cc13SweepBNativeString title;
        host.native_string_assign_0041e870(&title, static_cast<const char*>(host.image_global(0x00CECFE8u)));
        host.frontend_frame_select_00518d60(6, 0, 0, &title);
        release_string(title, host);
        with_assigned(at<void*>(screen, 0x10), 0x00CEB334u, [&](void* child) { at<void*>(screen, 0x18) = child; });
        with_assigned(at<void*>(screen, 0x10), 0x00CECFD8u,
                      [&](void* child) { host.widget_vcall_34_set_visible(child, 0); });
    } else {
        with_assigned(at<void*>(screen, 0x10), 0x00CECFD8u, [&](void* child) { at<void*>(screen, 0x18) = child; });
        with_assigned(at<void*>(screen, 0x10), 0x00CEB334u,
                      [&](void* child) { host.widget_vcall_34_set_visible(child, 0); });
        host.frontend_frame_select_default_004c9680();
    }

    host.widget_vcall_34_set_visible(at<void*>(screen, 0x18), 1);
    host.gui_listbox_clear_rows_00a9bec0(at<void*>(screen, 0x18));
    void* row_template = nullptr;
    with_child(at<void*>(screen, 0x18), 0x00CEC040u, 0x10, host, [&](void* child) { row_template = child; });
    for (std::uint32_t entry = 0x00E08664u; entry < 0x00E08670u; entry += 4) {
        void* row = host.widget_clone_subtree_00aab4c0(row_template, nullptr);
        host.gui_text_set_localised_cstring_00abbe50(row, *static_cast<const char**>(host.image_global(entry)), 1);
        host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x18), row, nullptr, 0);
    }
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x18), static_cast<std::uint8_t*>(screen) + 8);
    host.listbox_select_index_00a9c7c0(at<void*>(screen, 0x18), *static_cast<std::int32_t*>(host.image_global(0x00E08670u)));
    host.widget_vcall_60(at<void*>(screen, 0x18), 1);
    host.widget_vcall_80(at<void*>(screen, 0x18), 1);
    host.gui_listbox_reset_highlight_00a9cf30(at<void*>(screen, 0x18), 1, 1);

    auto* table = static_cast<std::uint8_t*>(host.image_global(0x00E18DC0u));
    for (std::int32_t i = 0; i < 2; ++i) {
        for (std::int32_t j = 0; j < 3; ++j) {
            for (std::int32_t k = 0; k < 3; ++k) {
                std::int32_t v = i;
                if (at<std::uint8_t>(screen, 0x28) != 0) {
                    v = i == 0 ? 1 : 0;
                }
                const char* text =
                    *static_cast<const char**>(host.image_global(0x00E08674u + 0xC * i + 4 * k));
                Cc13SweepBNativeString name;
                name.length = 0;
                name.data = nullptr;
                const std::uint32_t length = static_cast<std::uint32_t>(std::strlen(text));
                host.native_string_resize_0041dd40(&name, length, 1);
                if (name.data != nullptr) {
                    host.crt_memcpy_00bf7680(name.data, text, name.length + 1);
                }
                void* child = host.widget_find_child_00aa7e00(at<void*>(screen, 0x1C + 4 * j), &name, 1);
                host.gui_text_set_localised_source_00abaed0(child, table + 8 * (k + 3 * j + 9 * v), 1);
                release_string(name, host);
            }
        }
    }

    Cc13SweepBNativeString back;        // [ESP+14h]
    Cc13SweepBNativeString empty_b;     // [ESP+1Ch]
    Cc13SweepBNativeString empty_c;     // [ESP+24h]
    Cc13SweepBNativeString navigate;    // [ESP+2Ch]
    Cc13SweepBNativeString empty_e;     // [ESP+38h]
    build_literal(back, 0x00CEB3A8u, 0x0C, host);
    build_literal(empty_b, 0x00CE3A0Cu, 0, host);
    build_literal(empty_c, 0x00CE3A0Cu, 0, host);
    build_literal(navigate, 0x00CEB394u, 0x10, host);
    build_literal(empty_e, 0x00CE3A0Cu, 0, host);
    const Cc13SweepB2Host::CommandTriple triples[5] = {{0, &empty_e, 1},
                                                       {0xFFFFFFB3u, &navigate, 1},
                                                       {0, &empty_c, 1},
                                                       {0, &empty_b, 1},
                                                       {0xFFFFFFA3u, &back, 2}};
    host.command_bar_rebuild_0054b530(image_pointer(host, 0x00E1930C), triples);
    release_string(empty_e, host);
    release_string(navigate, host);
    release_string(empty_c, host);
    release_string(empty_b, host);
    release_string(back, host);

    with_child(at<void*>(screen, 0x14), 0x00CECFC8u, 0x0F, host, [&](void* child) {
        const std::uint32_t merged = (static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(screen)) &
                                      0xFFFFFF00u) |
                                     at<std::uint8_t>(image_pointer(host, 0x00E188A8), 0x61F);
        host.widget_vcall_34_set_visible(child, static_cast<std::int32_t>(merged));
    });
}

// 0053BCC0 BSP_FrameStats_FormatLines (ledger name). __thiscall(ECX = frame
// stats overlay, float rate), RET 4; EBP frame aligned to 40h, no EH frame.
// Sole caller 0053C510. The main buffer [ESP+0B0h] (210h bytes) starts empty;
// a 128-byte scratch [ESP+30h] takes the 004F9A80 pieces.
//
// Mode +8 == 1: +10h/+18h/+1Ch/+14h cleared ("" through 00ABBE50(.., 1));
// rate clamped: 0.01f (00D7A238) > rate -> 0.01f, rate > 9999.0f (00CE4C04)
// -> 9999.0f (COMISS + JBE, NaN keeps rate); mem = (float)([0109CEFC] vtable
// +4h * 2^-20 (double 00CEDB38), FILD signed). Lines through 0053BAB0 into
// the main buffer, each set on three texts with 00ABBE50(.., 1): "FPS:%.1f"
// rate (+20h/+24h/+28h); "VidM:%.1f" (uint32) device vtable +10h * 2^-20 as
// a double (+2Ch..+34h); "MEM:%.1f" mem (+38h..+40h); "APP/GAM/REN:%dms"
// _ftol(+68h/+6Ch/+70h * 1000.0 (00CE47A0)) (+44h..+4Ch, +50h..+58h,
// +5Ch..+64h).
// Other modes: +20h..+58h cleared; with the session mode game+1FE4h set,
// the counters of [game+2078h] (or +207Ch): up = the uint32 sum of +88h,
// +80h, ..., +50h, down = +84h, ..., +4Ch, then +90h and +8Ch, each
// (float)(x * 2^-10 (00CEDAE8)) as A, B, C, D; "UP:_%.1fK_(%.2fK,%.2fK)  "
// with (A + B, A, B) and "DN:_%.1fK_(%.2fK,%.2fK)  " with (C + D, C, D)
// (doubles; the sums FADDP in double), each appended with strcat_s(main,
// 210h, scratch); 0053BB40(stats); " GMsg:_%d,_%d,_%d   "
// with game+217Ch, +2178h and +2174h (mode 2) or +2170h appended. FPS:
// 0.01f > rate -> 0.01f and the low form; rate > 9999.0f -> 9999.0f, high
// form; otherwise 10.0f (00CE38B8) > rate -> low form, else high form (NaN
// takes the high form). High: " FPS:_%d/%.1fms    " (CVTTSS2SI rate,
// 1000.0 / rate); low: " FPS:_%.1f/%dms  " (rate, _ftol(1000.0 / rate));
// " MEM:_%.1fM  " ((float)(in-use * 2^-20)); main to +10h with 00ABBE50(..,
// 0); then "min: %d fps/%dms" / "max: ..." of +7Ch / +80h ((int)v, _ftol(1000.0
// / v)) printed over the main buffer to +18h / +1Ch. No RNG draws.
void cc13_sweepB5_frame_stats_format_lines_0053bcc0(void* stats, float rate, Cc13SweepB5Host& host) {
    char main_text[0x210];  // [ESP+0B0h]
    char scratch[0x80];     // [ESP+30h]
    main_text[0] = 0;
    auto text = [&](std::uint32_t literal) { return static_cast<const char*>(host.image_global(literal)); };
    const char* empty = text(0x00CE3A0Cu);
    auto set = [&](std::size_t offset, const char* value, std::int32_t flag) {
        host.gui_text_set_localised_cstring_00abbe50(at<void*>(stats, offset), value, flag);
    };
    const float low = f32_bits(0x3C23D70Au);   // 00D7A238
    const float high = f32_bits(0x461C3C00u);  // 00CE4C04
    const double mebi = f64_bits(0x3EB0000000000000ull);  // 00CEDB38, 2^-20
    const double thousand = f64_bits(0x408F400000000000ull);  // 00CE47A0
    void* tracker = image_pointer(host, 0x0109CEFCu);

    if (at<std::int32_t>(stats, 8) == 1) {
        set(0x10, empty, 1);
        set(0x18, empty, 1);
        set(0x1C, empty, 1);
        set(0x14, empty, 1);
        float clamped;  // [ESP+20h]
        if (low > rate) {
            clamped = low;
        } else if (rate > high) {
            clamped = high;
        } else {
            clamped = rate;
        }
        const float memory = static_cast<float>(host.memory_tracker_vcall_04_in_use(tracker) * mebi);  // [ESP+28h]
        auto line = [&](std::size_t first) {
            set(first, main_text, 1);
            set(first + 4, main_text, 1);
            set(first + 8, main_text, 1);
        };
        host.frame_stats_printf_0053bab0(main_text, text(0x00CEDB2Cu), static_cast<double>(clamped));
        line(0x20);
        void* device = host.renderer_get_device_00b1fef0(image_pointer(host, 0x00F8D394u));
        const std::uint32_t texture = host.device_vcall_10_available_texture_mem(device);
        host.frame_stats_printf_0053bab0(main_text, text(0x00CEDB20u), static_cast<double>(texture) * mebi);
        line(0x2C);
        host.frame_stats_printf_0053bab0(main_text, text(0x00CEDB14u), static_cast<double>(memory));
        line(0x38);
        host.frame_stats_printf_0053bab0(main_text, text(0x00CEDB08u), ftol_trunc(at<float>(stats, 0x68) * thousand));
        line(0x44);
        host.frame_stats_printf_0053bab0(main_text, text(0x00CEDAFCu), ftol_trunc(at<float>(stats, 0x6C) * thousand));
        line(0x50);
        host.frame_stats_printf_0053bab0(main_text, text(0x00CEDAF0u), ftol_trunc(at<float>(stats, 0x70) * thousand));
        line(0x5C);
        return;
    }

    for (std::size_t offset = 0x20; offset <= 0x58; offset += 4) {
        set(offset, empty, 1);
    }
    if (at<std::int32_t>(image_pointer(host, 0x00E188A8), 0x1FE4) != 0) {
        auto counters = [&]() {
            void* game = image_pointer(host, 0x00E188A8);
            void* record = at<void*>(game, 0x2078);
            return record != nullptr ? record : at<void*>(game, 0x207C);
        };
        const double kibi = f64_bits(0x3F50000000000000ull);  // 00CEDAE8, 2^-10
        auto sum = [&](std::size_t first) {
            void* record = counters();
            std::uint32_t total = 0;
            for (std::size_t k = 0; k < 8; ++k) {
                total += at<std::uint32_t>(record, first - 8 * k);
            }
            return total;
        };
        const float up = static_cast<float>(static_cast<double>(sum(0x88)) * kibi);     // [ESP+20h]
        const float down = static_cast<float>(static_cast<double>(sum(0x84)) * kibi);   // [ESP+28h]
        const float c = static_cast<float>(static_cast<double>(at<std::uint32_t>(counters(), 0x90)) * kibi);  // [ESP+24h]
        const float d = static_cast<float>(static_cast<double>(at<std::uint32_t>(counters(), 0x8C)) * kibi);  // [ESP+2Ch]
        host.format_string128_004f9a80(scratch, text(0x00CEDACCu), static_cast<double>(down) + up,
                                       static_cast<double>(up), static_cast<double>(down));
        host.crt_strcat_s_00bf97cb(main_text, 0x210, scratch);
        host.format_string128_004f9a80(scratch, text(0x00CEDAB0u), static_cast<double>(d) + c,
                                       static_cast<double>(c), static_cast<double>(d));
        host.crt_strcat_s_00bf97cb(main_text, 0x210, scratch);
        host.frame_stats_reset_accumulators_0053bb40(counters());
        void* game = image_pointer(host, 0x00E188A8);
        const std::int32_t first_count =
            at<std::int32_t>(game, 0x1FE4) == 2 ? at<std::int32_t>(game, 0x2174) : at<std::int32_t>(game, 0x2170);
        host.format_string128_004f9a80(scratch, text(0x00CEDA98u), at<std::int32_t>(game, 0x217C),
                                       at<std::int32_t>(game, 0x2178), first_count);
        host.crt_strcat_s_00bf97cb(main_text, 0x210, scratch);
    }

    float clamped;  // [ESP+20h]
    bool low_form;
    if (low > rate) {
        clamped = low;
        low_form = true;
    } else if (rate > high) {
        clamped = high;
        low_form = false;
    } else {
        clamped = rate;
        low_form = f32_bits(0x41200000u) > rate;  // 00CE38B8, 10.0f
    }
    if (low_form) {
        host.format_string128_004f9a80(scratch, text(0x00CEDA38u), static_cast<double>(clamped),
                                       ftol_trunc(thousand / clamped));
    } else {
        host.format_string128_004f9a80(scratch, text(0x00CEDA84u), ftol_trunc(clamped), thousand / clamped);
    }
    host.crt_strcat_s_00bf97cb(main_text, 0x210, scratch);
    const float memory = static_cast<float>(host.memory_tracker_vcall_04_in_use(tracker) * mebi);
    host.format_string128_004f9a80(scratch, text(0x00CEDA74u), static_cast<double>(memory));
    host.crt_strcat_s_00bf97cb(main_text, 0x210, scratch);
    set(0x10, main_text, 0);
    for (std::size_t pass = 0; pass < 2; ++pass) {
        const float value = at<float>(stats, pass == 0 ? 0x7C : 0x80);  // [ESP+20h]
        const std::int32_t ms = ftol_trunc(thousand / value);
        host.frame_stats_printf_0053bab0(main_text, text(pass == 0 ? 0x00CEDA60u : 0x00CEDA4Cu), ftol_trunc(value), ms);
        set(pass == 0 ? 0x18 : 0x1C, main_text, 1);
    }
}

// ---------------------------------------------------------------------------
// Segment 16
// ---------------------------------------------------------------------------

// 00547480 BSP_HudWeaponGroup_GunSightStatus (hypothesis; ledger FUN_00547480).
// __thiscall(ECX = weapon-group screen; unit, gun, mode, const float* origin,
// float aim_a, float aim_b), RET 18h, EAX = status. EH frame (handler
// 00C6CE48) around the hit record at [ESP+54h] (state 0 at 005477BB).
// Sole caller 00547A20.
//
// 005474A5..005474D6: gun+490h = 0 first; gun+5Dh set -> 0; 00729F10 false
// -> 1; screen+40h null -> 2.
// Gun kind 7 ([[gun+3F4h]+80h], 005474DC): 00729A40 false -> 5; w = (float)
// -00438B10(aim_b, unit vtable +50h); w > pi (double 00CE3D28; FCOMI + JBE,
// NaN keeps w) -> w = (float)(w - 2pi (00CE3828)); -pi (00CE3D18) >= w (FCOMIP
// + JB, NaN keeps w) -> w = (float)(w + 2pi); r = (float)0085AB50(gun, w,
// pi/4 (float 00E0B588)); r == FLT_MAX (double 00D7A278; FUCOMIP + LAHF/TEST
// 44h/JP, NaN is unequal) -> 2; e = (float)00438B10(r, gun+480h); |e| > pi/4
// -> 3, else 4 (FCOMIP + JBE: NaN -> 4).
// Mode 5 (00547601): the screen entity's world matrix row 2 (+20h) -> 00521370
// pitch/yaw; 00957BD0(unit, gun, the entity's position by value, yaw, pitch,
// screen+54h, aim); 00955830(gun, aim, &pitch, &yaw). Other modes: 004B4D80
// direction from (aim_a, aim_b); 00957740(unit, origin, direction, aim, gun);
// the muzzle speed [[[gun+3F8h]+34h]+50h] is read before 00427EB0(gun); a
// false 00955630 -> 2.
// Blocking test, only when the screen's [[+20h]+14h] unit is kind 1Ch: range =
// (float)[unit+7D0h] (CVTSI2SS). Unless screen+54h is set and 005457C0(that
// unit, [[screen+54h]+54h]) answers hostile, d = aim - the unit's position
// (float stores) and (float)(range*range) > 00427E30(d) (FCOMIP + JBE, NaN
// falls through) -> gun+490h = 1, 2. Then the segment from the gun along
// normalize(aim - gun position) scaled by range: 0042E630, then the gun
// owner's ([gun+3F0h]) vtable +20h exclude, 0098ADD0(.., record, 0); a hit
// entity of kind 5 that is the screen's unit or whose +71Ch is that unit ->
// gun+490h = 1, 004704B0, 2. Otherwise 004704B0 and on.
// Bearing: platform = *005472D0(unit+538h -> +94h, gun+38Ch); 007F60A0(
// platform, yaw, pitch) false -> 2. d = (float)00438B10(gun+484h, pitch).
// Mode 5: |d| > 3 deg (float 00D19BE0, FCOMIP + JA) -> 3; e = (float)
// 00438B10(gun+480h, yaw); |e| > 3 deg (FCOMIP + JBE) -> 3, else 4. Other
// modes: COMISS + JA against 5 deg (float 00CEDF5C) for |d| then |e| -> 3,
// else 4. NaN never takes a "> limit" arm. No RNG draws.
std::int32_t cc13_sweepB5_weapon_group_gun_sight_status_00547480(void* screen, void* unit, void* gun,
                                                                 std::int32_t mode, const float* origin,
                                                                 float aim_a, float aim_b,
                                                                 Cc13SweepB5Host& host) {
    at<std::uint8_t>(gun, 0x490) = 0;
    if (at<std::uint8_t>(gun, 0x5D) != 0) {
        return 0;
    }
    if (!host.gun_is_operational_00729f10(gun)) {
        return 1;
    }
    if (at<void*>(screen, 0x40) == nullptr) {
        return 2;
    }

    const float quarter_pi = f32_bits(0x3F490FDBu);  // 00E0B588
    if (at<std::int32_t>(at<void*>(gun, 0x3F4), 0x80) == 7) {
        if (!host.gun_any_barrel_timer_below_threshold_00729a40(gun)) {
            return 5;
        }
        const float heading = static_cast<float>(host.unit_vcall_50_heading(unit));
        float w = static_cast<float>(-host.math_subtract_wrapped_angle_00438b10(aim_b, heading));
        const double pi = f64_bits(0x400921FB60000000ull);       // 00CE3D28
        const double two_pi = f64_bits(0x401921FB60000000ull);   // 00CE3828
        const double neg_pi = f64_bits(0xC00921FB60000000ull);   // 00CE3D18
        if (w > pi) {
            w = static_cast<float>(w - two_pi);
        }
        if (neg_pi >= w) {
            w = static_cast<float>(w + two_pi);
        }
        const float r = static_cast<float>(host.turning_gun_filter_heading_0085ab50(gun, w, quarter_pi));
        if (r == f64_bits(0x47EFFFFFE0000000ull)) {  // 00D7A278
            return 2;
        }
        const float e = static_cast<float>(host.math_subtract_wrapped_angle_00438b10(r, at<float>(gun, 0x480)));
        return std::fabs(e) > quarter_pi ? 3 : 4;
    }

    float yaw = 0.0f;           // [ESP+14h]
    float pitch = 0.0f;         // [ESP+28h]
    float aim[3] = {};          // [ESP+3Ch]
    if (mode == 5) {
        void* entity = at<void*>(screen, 0x40);
        const float* matrix = host.entity_pose_world_matrix_0042d7e0(entity);
        float row_pitch = 0.0f;  // [ESP+10h]
        float row_yaw = 0.0f;    // [ESP+24h]
        host.direction_to_pitch_yaw_00521370(matrix + 8, &row_pitch, &row_yaw);
        const float* pos = host.entity_world_position_00427eb0(at<void*>(screen, 0x40));
        host.unit_gun_aim_point_00957bd0(unit, gun, pos[0], pos[1], pos[2], row_yaw, row_pitch,
                                         at<void*>(screen, 0x54), aim);
        host.device_aim_direction_to_local_pair_00955830(gun, aim, &pitch, &yaw);
    } else {
        float direction[3];  // [ESP+30h]
        host.direction_from_angles_004b4d80(direction, aim_a, aim_b);
        host.aim_resolve_ray_to_world_point_00957740(unit, origin, direction, aim, gun);
        const float speed = at<float>(at<void*>(at<void*>(gun, 0x3F8), 0x34), 0x50);
        const float* muzzle = host.entity_world_position_00427eb0(gun);
        if (!host.gun_solve_gravity_arc_00955630(unit, aim, muzzle, speed, &pitch, &yaw)) {
            return 2;
        }
    }

    void* player = at<void*>(at<void*>(screen, 0x20), 0x14);
    if (host.object_vcall_5c_is_kind(player, 0x1C)) {
        void* group = at<void*>(screen, 0x20);
        const float range = static_cast<float>(at<std::int32_t>(at<void*>(group, 0x14), 0x7D0));
        void* selected = at<void*>(screen, 0x54);
        if (selected == nullptr ||
            !cc13_sweepB_entity_is_hostile_to_party_005457c0(at<void*>(group, 0x14), at<std::int32_t>(selected, 0x54))) {
            const float* unit_pos = host.entity_world_position_00427eb0(at<void*>(group, 0x14));
            const float d[3] = {aim[0] - unit_pos[0], aim[1] - unit_pos[1], aim[2] - unit_pos[2]};
            const double range_d = range;
            const float length_sq = host.vector3_length_squared_00427e30(d);
            const float range_sq = static_cast<float>(range_d * range_d);
            if (range_sq > length_sq) {
                at<std::uint8_t>(gun, 0x490) = 1;
                return 2;
            }
        }

        alignas(4) std::uint8_t record[0x54] = {};  // [ESP+54h]
        host.hit_record_reset_00470470(record);
        const float* from = host.entity_world_position_00427eb0(gun);
        const float delta[3] = {aim[0] - from[0], aim[1] - from[1], aim[2] - from[2]};
        float unit_dir[3];  // [ESP+18h]
        float scaled[3];    // [ESP+30h]
        float to[3];        // [ESP+48h]
        const float* n = host.vector3_normalize_00419510(unit_dir, delta);
        const float* s = vector3_scale_to_00414260(n, scaled, range);
        vector3_add_to_004142a0(from, to, s);
        void* index = host.spatial_index_singleton_0042e630();
        void* exclude = host.unit_vcall_20_query_exclude(at<void*>(gun, 0x3F0));
        if (host.spatial_index_query_segment_0098add0(index, from, to, exclude, record, 0)) {
            void* hit = at<void*>(record, 0);
            if (hit != nullptr && host.object_vcall_5c_is_kind(hit, 5)) {
                void* own = at<void*>(at<void*>(screen, 0x20), 0x14);
                if (hit == own || at<void*>(hit, 0x71C) == own) {
                    at<std::uint8_t>(gun, 0x490) = 1;
                    host.hit_record_destroy_004704b0(record);
                    return 2;
                }
            }
        }
        host.hit_record_destroy_004704b0(record);
    }

    std::uint8_t* platforms = at<std::uint8_t*>(unit, 0x538) + 0x94;
    void* platform = *host.ref_ptr_array_at_grow_005472d0(platforms, at<std::int32_t>(gun, 0x38C));
    if (!host.gun_platform_angles_in_fire_window_007f60a0(platform, yaw, pitch)) {
        return 2;
    }
    const float d = static_cast<float>(host.math_subtract_wrapped_angle_00438b10(at<float>(gun, 0x484), pitch));
    const float limit = mode == 5 ? f32_bits(0x3D567750u)    // 00D19BE0, 3 degrees
                                  : f32_bits(0x3DB2B8C3u);   // 00CEDF5C, 5 degrees
    if (std::fabs(d) > limit) {
        return 3;
    }
    const float e = static_cast<float>(host.math_subtract_wrapped_angle_00438b10(at<float>(gun, 0x480), yaw));
    return std::fabs(e) > limit ? 3 : 4;
}

// ---------------------------------------------------------------------------
// Segment 19
// ---------------------------------------------------------------------------

// 0056DD80 BSP_MultiSessionBrowserScreen_RefreshFilterRows (ledger name).
// __thiscall(ECX = session browser screen), RET; EBP frame aligned to 8, EH
// frame (handler 00C6EB60, states 0..0Dh for the native strings). Callers
// 0057A910 and 0057B600; sweepB's 005789C0 path calls it through a hook.
//
// For i = 0..8, row = [screen+54h+4i] (00AA7E00(row, &name, 1) per child,
// each name built and released around its use):
// - "nike_Icon" (00CEEFF0, length 9) and "setting_2_Text" (00CEEFFC, 0Eh)
//   vtable +34h(0) (resize + memcpy builds).
// - i <= 4: flag byte screen+10h+i set -> "nike_Icon" +34h(1).
// - i == 8: "nike_Icon" of [screen+74h] +34h(the zero-extended byte +18h).
// - i = 5..7: "setting_2_Text" +34h(1); flag set -> (names built with
//   0041E870 from here on) "nike_Icon" +34h(0), "setting_2_Text" +34h(1),
//   then "setting_2_Text" of row 5/6/7 ([screen+68h/6Ch/70h]) gets
//   00ABAED0(004260B0(tmp, [screen+244h/248h/24Ch]), 1); flag clear ->
//   "setting_2_Text" 00ABBE50("globals.no" (00CED4E4), 1).
// No RNG draws.
void cc13_sweepB5_session_browser_refresh_filter_rows_0056dd80(void* screen, Cc13SweepB5Host& host) {
    const std::uint32_t nike = 0x00CEEFF0u;
    const std::uint32_t setting = 0x00CEEFFCu;
    auto with_assigned = [&](void* row, std::uint32_t literal, auto use) {
        Cc13SweepBNativeString name;
        host.native_string_assign_0041e870(&name, static_cast<const char*>(host.image_global(literal)));
        use(host.widget_find_child_00aa7e00(row, &name, 1));
        release_string(name, host);
    };
    for (std::int32_t i = 0; i < 9; ++i) {
        auto row = [&]() { return at<void*>(screen, 0x54 + 4 * i); };
        with_child(row(), nike, 9, host, [&](void* child) { host.widget_vcall_34_set_visible(child, 0); });
        with_child(row(), setting, 0x0E, host, [&](void* child) { host.widget_vcall_34_set_visible(child, 0); });
        const bool flag = at<std::uint8_t>(screen, 0x10 + i) != 0;
        if (i <= 4) {
            if (flag) {
                with_child(row(), nike, 9, host, [&](void* child) { host.widget_vcall_34_set_visible(child, 1); });
            }
            continue;
        }
        if (i == 8) {
            with_child(at<void*>(screen, 0x74), nike, 9, host, [&](void* child) {
                host.widget_vcall_34_set_visible(child, at<std::uint8_t>(screen, 0x18));
            });
            continue;
        }
        with_child(row(), setting, 0x0E, host, [&](void* child) { host.widget_vcall_34_set_visible(child, 1); });
        if (at<std::uint8_t>(screen, 0x10 + i) != 0) {
            with_assigned(row(), nike, [&](void* child) { host.widget_vcall_34_set_visible(child, 0); });
            with_assigned(row(), setting, [&](void* child) { host.widget_vcall_34_set_visible(child, 1); });
            const std::size_t value_offset = i == 5 ? 0x244 : i == 6 ? 0x248 : 0x24C;
            const std::size_t row_offset = i == 5 ? 0x68 : i == 6 ? 0x6C : 0x70;
            Cc13SweepBNativeString name;
            host.native_string_assign_0041e870(&name, static_cast<const char*>(host.image_global(setting)));
            Cc13SweepBNativeString number;
            void* text = host.native_string_from_int_004260b0(&number, at<std::int32_t>(screen, value_offset));
            void* child = host.widget_find_child_00aa7e00(at<void*>(screen, row_offset), &name, 1);
            host.gui_text_set_localised_source_00abaed0(child, text, 1);
            release_string(number, host);
            release_string(name, host);
        } else {
            with_assigned(row(), setting, [&](void* child) {
                host.gui_text_set_localised_cstring_00abbe50(
                    child, static_cast<const char*>(host.image_global(0x00CED4E4u)), 1);
            });
        }
    }
}

// ---------------------------------------------------------------------------
// Segment 20
// ---------------------------------------------------------------------------

// 00593CA0 BSP_MissionScoreRecord_Assign (ledger name; the body is the
// record's member-wise copy CONSTRUCTOR: the EH state rises 0..28h as each
// member is built, handler 00C70E73). __thiscall(ECX = dst, const src*),
// RET 4, EAX = dst. Callers 00594340, 005943D0 and 007FCD90.
//
// +0/+4/+8 dwords, +0Ch/+10h/+14h floats (FLD/FSTP); member copy
// constructors (ECX = dst member, src member): 00590080 at +18h..+90h
// (every 0Ch), 00593310 +9Ch/+0A8h, 00593480 +0B4h/+0C0h, 00591D90 +0CCh,
// 0058D4B0 +0D8h..+108h (every 0Ch), 0058D540 +114h, 00590080 +120h, 0058AAC0
// +12Ch/+13Ch, 0058B870 +14Ch, 0058D5D0 +15Ch; dwords +168h..+194h; 0058D660
// +198h/+1A4h/+1B0h, 0058D6F0 +1BCh; dwords +1C8h..+1E8h; 00590080
// +1ECh/+1F8h; 0058F370 +204h..+240h (every 0Ch); bytes +24Ch/+24Dh, dword
// +250h; native strings +254h/+25Ch/+264h by the inlined copy ({0, null};
// when dst != src: 0041DD40(dst, src.length, 1), then when src.length != 0
// memcpy(dst.data, src.data, dst.length) -- no terminator byte copied);
// dwords +26Ch..+280h. No RNG draws.
void* cc13_sweepB5_mission_score_record_copy_00593ca0(void* dst, const void* src, Cc13SweepB5Host& host) {
    void* from = const_cast<void*>(src);
    auto copy_dword = [&](std::size_t offset) { at<std::uint32_t>(dst, offset) = at<std::uint32_t>(from, offset); };
    auto member = [&](std::size_t offset) { return static_cast<std::uint8_t*>(dst) + offset; };
    auto source = [&](std::size_t offset) { return static_cast<const std::uint8_t*>(src) + offset; };
    copy_dword(0);
    copy_dword(4);
    copy_dword(8);
    at<float>(dst, 0xC) = at<float>(from, 0xC);
    at<float>(dst, 0x10) = at<float>(from, 0x10);
    at<float>(dst, 0x14) = at<float>(from, 0x14);
    for (std::size_t offset = 0x18; offset <= 0x90; offset += 0xC) {
        host.tree_copy_construct_00590080(member(offset), source(offset));
    }
    host.tree_copy_construct_00593310(member(0x9C), source(0x9C));
    host.tree_copy_construct_00593310(member(0xA8), source(0xA8));
    host.tree_copy_construct_00593480(member(0xB4), source(0xB4));
    host.tree_copy_construct_00593480(member(0xC0), source(0xC0));
    host.tree_copy_construct_00591d90(member(0xCC), source(0xCC));
    for (std::size_t offset = 0xD8; offset <= 0x108; offset += 0xC) {
        host.tree_copy_construct_0058d4b0(member(offset), source(offset));
    }
    host.tree_copy_construct_0058d540(member(0x114), source(0x114));
    host.tree_copy_construct_00590080(member(0x120), source(0x120));
    host.vector_copy_construct_0058aac0(member(0x12C), source(0x12C));
    host.vector_copy_construct_0058aac0(member(0x13C), source(0x13C));
    host.member_copy_construct_0058b870(member(0x14C), source(0x14C));
    host.tree_copy_construct_0058d5d0(member(0x15C), source(0x15C));
    for (std::size_t offset = 0x168; offset <= 0x194; offset += 4) {
        copy_dword(offset);
    }
    host.tree_copy_construct_0058d660(member(0x198), source(0x198));
    host.tree_copy_construct_0058d660(member(0x1A4), source(0x1A4));
    host.tree_copy_construct_0058d660(member(0x1B0), source(0x1B0));
    host.tree_copy_construct_0058d6f0(member(0x1BC), source(0x1BC));
    for (std::size_t offset = 0x1C8; offset <= 0x1E8; offset += 4) {
        copy_dword(offset);
    }
    host.tree_copy_construct_00590080(member(0x1EC), source(0x1EC));
    host.tree_copy_construct_00590080(member(0x1F8), source(0x1F8));
    for (std::size_t offset = 0x204; offset <= 0x240; offset += 0xC) {
        host.tree_copy_construct_0058f370(member(offset), source(offset));
    }
    at<std::uint8_t>(dst, 0x24C) = at<std::uint8_t>(from, 0x24C);
    at<std::uint8_t>(dst, 0x24D) = at<std::uint8_t>(from, 0x24D);
    copy_dword(0x250);
    for (std::size_t offset : {std::size_t{0x254}, std::size_t{0x25C}, std::size_t{0x264}}) {
        auto& to = at<Cc13SweepBNativeString>(dst, offset);
        auto& in = at<Cc13SweepBNativeString>(from, offset);
        to.length = 0;
        to.data = nullptr;
        if (&to != &in) {
            host.native_string_resize_0041dd40(&to, in.length, 1);
            if (in.length != 0) {
                host.crt_memcpy_00bf7680(to.data, in.data, to.length);
            }
        }
    }
    for (std::size_t offset = 0x26C; offset <= 0x280; offset += 4) {
        copy_dword(offset);
    }
    return dst;
}

// 005A4200 BSP_HudMapScreen_Exit (ledger name). __thiscall(ECX = HUD map
// screen), RET; EBP frame aligned to 8, EH frame (handler 00C716F2) with a
// flags word [ESP+14h] for the two conditional name strings. GUI_map vtable
// 00CF0134 slot +1Ch; no direct callers.
//
// 005A422C: 00A7D120([00F8BBD8], 0).
// Terrain (only when the root +56Ch is set): for each child of [[screen+
// 0Ch]+0Ch] (+3Ch next) named "Terrain Visibility Group" (00CF00D8; 00B6D800
// name +4h data non-null and 00BF7FBF == 0), each of its children (+34h
// first, +3Ch next): "BushGroup" (00CF00CC) and "TreeGroup" (00CF00C0) are
// built (flags 1 and 2) and compared with the inlined native-string ==
// (empty name -> equal iff the literal is empty; empty literal -> unequal;
// else 00BF7FBF == 0); the strings are released (TreeGroup first). Neither:
// the child must be a 00B74310 kind (vtable +0Ch) else visibility; then the
// material's (00B732C0(00B74640(child, 0), 0)+20h) +7Ch effect name
// (00B172D0) must be non-null and contain "wavedamp" (00CF00B4, 00BF9440)
// at offset > -1 to be left alone, otherwise visibility. Bush or Tree: only
// a child named "TreeGroup" (00BF7FBF == 0) gets visibility. Visibility =
// 00B6DA70(child, 1.0f, 0). Each matching group is then appended to the
// pointer array +570h {data, count +574h, capacity +578h} (full: 0059E5E0
// with max(2 * capacity, 1); a null slot address is not written). Then
// 00B6D890(array[i], +56Ch) for i < count (signed) and 0059FCE0(array, 0).
// 005A44FD: the selected unit [00E188D8] non-null, [[00E198C4]+58h] set,
// unit +5Eh and +5Dh clear and the local player's +19h clear ->
// 005BCA30([[00E198C4]+58h], 1).
// The model vectors +4C8h (+4CCh first, +4D0h last) and +4DCh (+4E0h,
// +4E4h): 00B6DFA0 on each element (subscript check), then the inlined
// erase(begin, end) (first > last and begin > last checks, a memmove_s of
// the zero-length tail through 00BF67A7 only when it is positive, last =
// begin). +53Ch = 0, +540h = 0.0f; 00529490 and 00529350 on
// [[00E198C4]+0C8h].
// Map +4F4h (+4F8h head): per entry the first non-zero dword of the value
// at +48h..+60h sets +53Ch = key (+0Ch) and +540h = value+74h; a non-null
// value is deleted by its vtable +0h(1); 006952A0(key, screen+64h); 00535110.
// Inlined tree clear (005356E0 per right subtree, nodes freed, head reset).
// 005A3AA0(screen); 005A09A0(screen+544h); +4F0h non-null -> 00B6DFA0 and
// cleared; game+19C4h = 0; 00772A40(game+1EF0h); 0054E610([[00E198C4]+
// 0A8h]); the six pointers +108h..+11Ch deleted by vtable +4h(1) and
// cleared. Every 00BF6713 check as written. No RNG draws.
void cc13_sweepB5_hud_map_screen_exit_005a4200(void* screen, Cc13SweepB5Host& host) {
    auto invalid_if = [&](bool bad) {
        if (bad) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    host.sound_manager_push_pop_speech_duck_00a7d120(image_pointer(host, 0x00F8BBD8), 0);

    if (at<void*>(screen, 0x56C) != nullptr) {
        auto name_of = [&](void* node) {
            return static_cast<const Cc13SweepBNativeString*>(host.node_get_name_header_00b6d800(node));
        };
        auto literal = [&](std::uint32_t address) { return static_cast<const char*>(host.image_global(address)); };
        // The inlined native-string ==: true when equal.
        auto equals = [&](void* node, const Cc13SweepBNativeString& text) {
            const Cc13SweepBNativeString* name = name_of(node);
            if (name->length == 0) {
                return text.length == 0;
            }
            if (text.length == 0) {
                return false;
            }
            return host.crt_stricmp_00bf7fbf(name->data, text.data) == 0;
        };
        for (void* node = at<void*>(at<void*>(screen, 0xC), 0xC); node != nullptr; node = at<void*>(node, 0x3C)) {
            const char* group_name = name_of(node)->data;
            if (group_name == nullptr || host.crt_stricmp_00bf7fbf(group_name, literal(0x00CF00D8u)) != 0) {
                continue;
            }
            for (void* child = at<void*>(node, 0x34); child != nullptr; child = at<void*>(child, 0x3C)) {
                Cc13SweepBNativeString bush;
                build_literal(bush, 0x00CF00CCu, 9, host);
                bool neither = false;
                if (!equals(child, bush)) {
                    Cc13SweepBNativeString tree;
                    build_literal(tree, 0x00CF00C0u, 9, host);
                    neither = !equals(child, tree);
                    release_string(tree, host);
                }
                release_string(bush, host);
                bool restore;
                if (neither) {
                    restore = true;
                    if (host.node_vcall_0c_is_kind_of(child, host.generated_model_current_type_00b74310())) {
                        void* element = host.mesh_element_00b732c0(host.generated_model_geometry_00b74640(child, 0), 0);
                        const Cc13SweepBNativeString* effect =
                            host.effect_get_name_00b172d0(at<void*>(at<void*>(element, 0x20), 0x7C));
                        if (effect->data != nullptr) {
                            const char* found = host.crt_strstr_00bf9440(effect->data, literal(0x00CF00B4u));
                            if (found != nullptr && static_cast<std::int32_t>(found - effect->data) > -1) {
                                restore = false;
                            }
                        }
                    }
                } else {
                    const char* child_name = name_of(child)->data;
                    restore = child_name != nullptr && host.crt_stricmp_00bf7fbf(child_name, literal(0x00CF00C0u)) == 0;
                }
                if (restore) {
                    host.scene_node_set_visibility_factor_00b6da70(child, 1.0f, 0);
                }
            }
            void* array = static_cast<std::uint8_t*>(screen) + 0x570;
            if (at<std::int32_t>(array, 4) == at<std::int32_t>(array, 8)) {
                std::int32_t capacity = at<std::int32_t>(array, 8) * 2;
                if (!(capacity > 1)) {
                    capacity = 1;
                }
                host.pointer_array_reserve_0059e5e0(array, capacity);
            }
            void** slot = at<void**>(array, 0) + at<std::int32_t>(array, 4);
            if (slot != nullptr) {
                *slot = node;
            }
            at<std::int32_t>(array, 4) += 1;
        }
        for (std::int32_t i = 0; i < at<std::int32_t>(screen, 0x574); ++i) {
            host.node_propagate_root_00b6d890(at<void**>(screen, 0x570)[i], at<void*>(screen, 0x56C));
        }
        host.pointer_array_resize_0059fce0(static_cast<std::uint8_t*>(screen) + 0x570, 0);
    }

    void* selected = image_pointer(host, 0x00E188D8);
    void* hud = image_pointer(host, 0x00E198C4);
    if (selected != nullptr && at<void*>(hud, 0x58) != nullptr && at<std::uint8_t>(selected, 0x5E) == 0 &&
        at<std::uint8_t>(selected, 0x5D) == 0) {
        void* game = image_pointer(host, 0x00E188A8);
        void* player = at<void*>(game, 0x18CC + 4 * at<std::int32_t>(game, 0x18EC));
        if (at<std::uint8_t>(player, 0x19) == 0) {
            host.hud_panel_set_visible3_005bca30(at<void*>(hud, 0x58), 1);
        }
    }

    auto release_models = [&](std::size_t first_offset) {
        auto first = [&]() { return at<void**>(screen, first_offset); };
        auto count = [&]() { return static_cast<std::uint32_t>(at<void**>(screen, first_offset + 4) - first()); };
        for (std::uint32_t i = 0; first() != nullptr && i < count(); ++i) {
            invalid_if(first() == nullptr || !(i < count()));
            host.node_unlink_and_release_00b6dfa0(first()[i]);
        }
        void** begin_loaded = first();
        void** end = at<void**>(screen, first_offset + 4);
        invalid_if(begin_loaded > end);
        void** begin = first();
        invalid_if(begin > at<void**>(screen, first_offset + 4));
        if (begin != end) {
            const std::int32_t tail = static_cast<std::int32_t>(at<void**>(screen, first_offset + 4) - end);
            const std::uint32_t bytes = static_cast<std::uint32_t>(tail) * 4;
            void** new_last = begin + tail;
            if (tail > 0) {
                host.crt_memmove_s_00bf67a7(begin, bytes, end, bytes);
            }
            at<void**>(screen, first_offset + 4) = new_last;
        }
    };
    release_models(0x4CC);
    release_models(0x4E0);
    at<std::int32_t>(screen, 0x53C) = 0;
    at<float>(screen, 0x540) = 0.0f;
    host.hud_counters_restore_hidden_a8_00529490(at<void*>(image_pointer(host, 0x00E198C4), 0xC8));
    host.hud_counters_restore_hidden_a6_00529350(at<void*>(image_pointer(host, 0x00E198C4), 0xC8));

    void* map = static_cast<std::uint8_t*>(screen) + 0x4F4;
    void* it[2] = {map, at<void*>(at<void*>(map, 4), 0)};  // [ESP+28h]
    for (;;) {
        void* end = at<void*>(map, 4);
        invalid_if(it[0] == nullptr || it[0] != map);
        if (it[1] == end) {
            break;
        }
        void* node = it[1];
        auto check_node = [&]() {
            invalid_if(it[0] == nullptr);
            invalid_if(node == at<void*>(it[0], 4));
        };
        bool found = false;
        for (std::size_t offset = 0x48; offset < 0x64; offset += 4) {
            check_node();
            if (at<std::int32_t>(at<void*>(node, 0x10), offset) != 0) {
                found = true;
                break;
            }
        }
        if (found) {
            invalid_if(node == at<void*>(it[0], 4));
            at<std::int32_t>(screen, 0x53C) = at<std::int32_t>(node, 0xC);
            invalid_if(node == at<void*>(it[0], 4));
            at<float>(screen, 0x540) = at<float>(at<void*>(node, 0x10), 0x74);
        }
        invalid_if(node == at<void*>(it[0], 4));
        if (void* value = at<void*>(node, 0x10)) {
            host.object_vcall_00_delete(value, 1);
        }
        invalid_if(node == at<void*>(it[0], 4));
        host.observer_unregister_pair_006952a0(at<void*>(node, 0xC), static_cast<std::uint8_t*>(screen) + 0x64);
        host.tree_iterator_increment_00535110(it);
    }
    {
        void* node = at<void*>(at<void*>(map, 4), 4);
        while (at<std::uint8_t>(node, 0x15) == 0) {
            host.tree_erase_subtree_005356e0(map, at<void*>(node, 8));
            void* left = at<void*>(node, 0);
            host.crt_operator_delete_00bf65ac(node);
            node = left;
        }
        at<void*>(at<void*>(map, 4), 4) = at<void*>(map, 4);
        at<std::uint32_t>(map, 8) = 0;
        at<void*>(at<void*>(map, 4), 0) = at<void*>(map, 4);
        at<void*>(at<void*>(map, 4), 8) = at<void*>(map, 4);
    }

    host.hud_map_delete_marker_objects_005a3aa0(screen);
    host.hud_map_release_icon_array_005a09a0(static_cast<std::uint8_t*>(screen) + 0x544);
    if (at<void*>(screen, 0x4F0) != nullptr) {
        host.node_unlink_and_release_00b6dfa0(at<void*>(screen, 0x4F0));
        at<void*>(screen, 0x4F0) = nullptr;
    }
    at<std::uint8_t>(image_pointer(host, 0x00E188A8), 0x19C4) = 0;
    host.session_client_send_local_score_00772a40(static_cast<std::uint8_t*>(image_pointer(host, 0x00E188A8)) + 0x1EF0);
    host.hud_hints_dismiss_0054e610(at<void*>(image_pointer(host, 0x00E198C4), 0xA8));
    for (std::size_t offset = 0x108; offset < 0x120; offset += 4) {
        if (void* object = at<void*>(screen, offset)) {
            host.object_vcall_04_delete(object, 1);
            at<void*>(screen, offset) = nullptr;
        }
        at<void*>(screen, offset) = nullptr;
    }
}

// 005B2D70 BSP_HudMapScreen_LoadBackground (ledger name; it is the map's full
// refresh). __thiscall(ECX = HUD map screen), RET; EBP frame aligned to 8, EH
// frame (handler 00C71F78) around the layer name. Callers 005B3470 and
// 005B45C0.
//
// "terkep_alap_hdtv:Layer2" (00CF0B98, 17h) via 00B6E750([[+8Ch]+4Ch],
// &name, 1); found: 00B6DB70 unless bit 1 of node+5Ch, then the element
// 00B732C0(00B74640(node, 0), 0)+34h = -90.0f (00CF0B70).
// +4B0h = ftol(([00F876A4] * 2) * 3.0) % 3 (IDIV remainder).
// Map +4F4h: every non-null value gets byte +8h = 0.
// Game unit list [[game+19CCh]+16Ch] (+4h next, +8h unit): alive (+5Ch set,
// +5Dh/+60h/+5Eh clear) and kind 2 -> 005B2CB0(screen, unit).
// Player list [[local player+30h]+0E0Ch] ([+8h]+4h unit): (vtable +0B4h or
// 008DDF90([game+21A4h+4*slot], unit)) and 0059D940 and +5Ch set; kind 18h
// needs a non-null +3D0h with its +5Ch set; kind 0Bh with
// [[+538h]+808h] set is skipped; class +0C4h == 1Bh needs
// [[00E198C4]+54h]+438h + 4*[[+538h]+17Ch] non-zero; then 005B2CB0.
// Map +4F4h again: entries whose value has +8h set are kept; otherwise with
// +5ACh > 0.0 (COMISS + JBE, NaN erases) a non-null value goes to 005ADDA0;
// else +598h = 1, the value is deleted (vtable +0h(1)) and cleared,
// 006952A0(key, +64h), and the entry erased (00537F20) after copying an
// advanced iterator (00535110), which the walk continues from.
// +4D8h = +4ECh = +4C4h = 0; 005A9DB0(screen, unit, 0/1/1) for the alive
// units of the local player's lists +0DDCh / +0DE8h / +0DF4h.
// Visibility: 00B6DA70(model, (float)(i < count), 0) over +4CCh (count
// +4D8h, signed), +4E0h (+4ECh, signed) and the +4B8h blocks' +0 models
// (+4C4h, unsigned); subscript checks as written. 0059F9B0.
// Tree +600h (size +608h): unless some node has byte +10h set, the inlined
// clear (0059FDB0 per right subtree, nodes freed, head reset).
// +53Ch set: (double)(+540h + 3.0) > [00F876A4] (FCOMIP + JBE) ->
// 005A2650(screen, +53Ch, 0, 0); +53Ch = 0. No RNG draws.
void cc13_sweepB5_hud_map_load_background_005b2d70(void* screen, Cc13SweepB5Host& host) {
    auto invalid_if = [&](bool bad) {
        if (bad) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    {
        Cc13SweepBNativeString name;
        build_literal(name, 0x00CF0B98u, 0x17, host);
        void* layer = host.node_find_child_by_name_00b6e750(at<void*>(at<void*>(screen, 0x8C), 0x4C), &name, 1);
        release_string(name, host);
        if (layer != nullptr) {
            if ((at<std::uint8_t>(layer, 0x5C) & 2) == 0) {
                host.transform_refresh_world_00b6db70(layer);
            }
            void* element = host.mesh_element_00b732c0(host.generated_model_geometry_00b74640(layer, 0), 0);
            at<float>(element, 0x34) = f32_bits(0xC2B40000u);  // 00CF0B70, -90.0f
        }
    }
    {
        const double zoom = *static_cast<float*>(host.image_global(0x00F876A4u));
        at<std::int32_t>(screen, 0x4B0) = ftol_trunc((zoom + zoom) * f64_bits(0x4008000000000000ull)) % 3;
    }

    void* map = static_cast<std::uint8_t*>(screen) + 0x4F4;
    {
        void* it[2] = {map, at<void*>(at<void*>(map, 4), 0)};
        for (;;) {
            void* end = at<void*>(map, 4);
            invalid_if(it[0] == nullptr || it[0] != map);
            if (it[1] == end) {
                break;
            }
            invalid_if(it[0] == nullptr);
            invalid_if(it[1] == at<void*>(it[0], 4));
            if (at<void*>(it[1], 0x10) != nullptr) {
                invalid_if(it[1] == at<void*>(it[0], 4));
                at<std::uint8_t>(at<void*>(it[1], 0x10), 8) = 0;
            }
            host.tree_iterator_increment_00535110(it);
        }
    }

    auto alive = [&](void* unit) {
        return at<std::uint8_t>(unit, 0x5C) != 0 && at<std::uint8_t>(unit, 0x5D) == 0 &&
               at<std::uint8_t>(unit, 0x60) == 0 && at<std::uint8_t>(unit, 0x5E) == 0;
    };
    for (void* n = at<void*>(at<void*>(image_pointer(host, 0x00E188A8), 0x19CC), 0x16C); n != nullptr;
         n = at<void*>(n, 4)) {
        void* unit = at<void*>(n, 8);
        if (alive(unit) && host.object_vcall_5c_is_kind(unit, 2)) {
            host.hud_map_get_or_create_layer_005b2cb0(screen, unit);
        }
    }
    auto player_list = [&](std::size_t offset) {
        void* game = image_pointer(host, 0x00E188A8);
        void* player = at<void*>(game, 0x18CC + 4 * at<std::int32_t>(game, 0x18EC));
        return at<void*>(at<void*>(player, 0x30), offset);
    };
    for (void* n = player_list(0xE0C); n != nullptr; n = at<void*>(n, 4)) {
        void* unit = at<void*>(at<void*>(n, 8), 4);
        if (!host.unit_vcall_b4(unit)) {
            void* game = image_pointer(host, 0x00E188A8);
            if (!host.objective_owner_contains_unit_008ddf90(
                    at<void*>(game, 0x21A4 + 4 * at<std::int32_t>(game, 0x18EC)), unit)) {
                continue;
            }
        }
        if (!host.hud_map_is_unit_shown_at_zoom_0059d940(screen, unit) || at<std::uint8_t>(unit, 0x5C) == 0) {
            continue;
        }
        if (host.object_vcall_5c_is_kind(unit, 0x18)) {
            void* carrier = at<void*>(unit, 0x3D0);
            if (carrier == nullptr || at<std::uint8_t>(carrier, 0x5C) == 0) {
                continue;
            }
        }
        if (host.object_vcall_5c_is_kind(unit, 0xB) && at<std::uint8_t>(at<void*>(unit, 0x538), 0x808) != 0) {
            continue;
        }
        if (at<std::int32_t>(unit, 0xC4) == 0x1B) {
            void* table = at<void*>(image_pointer(host, 0x00E198C4), 0x54);
            if (at<std::int32_t>(table, 0x438 + 4 * at<std::int32_t>(at<void*>(unit, 0x538), 0x17C)) == 0) {
                continue;
            }
        }
        host.hud_map_get_or_create_layer_005b2cb0(screen, unit);
    }

    {
        void* it[2] = {map, at<void*>(at<void*>(map, 4), 0)};  // [ESP+10h]
        for (;;) {
            void* end = at<void*>(map, 4);
            invalid_if(it[0] == nullptr || it[0] != map);
            if (it[1] == end) {
                break;
            }
            invalid_if(it[0] == nullptr);
            invalid_if(it[1] == at<void*>(it[0], 4));
            void* node = it[1];
            if (at<std::uint8_t>(at<void*>(node, 0x10), 8) != 0) {
                host.tree_iterator_increment_00535110(it);
                continue;
            }
            if (at<float>(screen, 0x5AC) > 0.0f) {
                invalid_if(node == at<void*>(it[0], 4));
                if (at<void*>(node, 0x10) != nullptr) {
                    invalid_if(node == at<void*>(it[0], 4));
                    host.hud_map_update_unit_icon_005adda0(screen, at<void*>(node, 0x10));
                }
                host.tree_iterator_increment_00535110(it);
                continue;
            }
            at<std::uint8_t>(screen, 0x598) = 1;
            invalid_if(node == at<void*>(it[0], 4));
            if (void* value = at<void*>(node, 0x10)) {
                host.object_vcall_00_delete(value, 1);
                at<void*>(node, 0x10) = nullptr;
            }
            invalid_if(node == at<void*>(it[0], 4));
            host.observer_unregister_pair_006952a0(at<void*>(node, 0xC), static_cast<std::uint8_t*>(screen) + 0x64);
            void* next[2] = {it[0], node};  // [ESP+18h]
            host.tree_iterator_increment_00535110(next);
            void* out[2];  // [ESP+20h]
            host.tree_erase_at_00537f20(map, out, it[0], node);
            it[0] = next[0];
            it[1] = next[1];
        }
    }

    at<std::int32_t>(screen, 0x4D8) = 0;
    at<std::int32_t>(screen, 0x4EC) = 0;
    at<std::int32_t>(screen, 0x4C4) = 0;
    const std::size_t overlay_lists[3] = {0xDDC, 0xDE8, 0xDF4};
    for (std::size_t list = 0; list < 3; ++list) {
        for (void* n = player_list(overlay_lists[list]); n != nullptr; n = at<void*>(n, 4)) {
            void* unit = at<void*>(at<void*>(n, 8), 4);
            if (alive(unit)) {
                host.hud_map_build_unit_order_overlays_005a9db0(screen, unit, list == 0 ? 0 : 1);
            }
        }
    }

    auto show_models = [&](std::size_t first_offset, std::size_t count_offset, bool blocks, bool is_unsigned) {
        auto first = [&]() { return at<void**>(screen, first_offset); };
        auto size = [&]() { return static_cast<std::uint32_t>(at<void**>(screen, first_offset + 4) - first()); };
        for (std::uint32_t i = 0; first() != nullptr && i < size(); ++i) {
            invalid_if(first() == nullptr || !(i < size()));
            const bool shown = is_unsigned ? i < at<std::uint32_t>(screen, count_offset)
                                           : static_cast<std::int32_t>(i) < at<std::int32_t>(screen, count_offset);
            void* model = first()[i];
            if (blocks) {
                model = at<void*>(model, 0);
            }
            host.scene_node_set_visibility_factor_00b6da70(model, static_cast<float>(shown ? 1 : 0), 0);
        }
    };
    show_models(0x4CC, 0x4D8, false, false);
    show_models(0x4E0, 0x4EC, false, false);
    show_models(0x4B8, 0x4C4, true, true);
    host.hud_map_hide_unused_arrow_widgets_0059f9b0(screen);

    if (at<std::uint32_t>(screen, 0x608) != 0) {
        void* tree = static_cast<std::uint8_t*>(screen) + 0x600;
        void* it[2] = {tree, at<void*>(at<void*>(tree, 4), 0)};  // [ESP+10h]
        bool held = false;
        for (;;) {
            void* end = at<void*>(tree, 4);
            invalid_if(it[0] == nullptr || it[0] != tree);
            if (it[1] == end) {
                break;
            }
            invalid_if(it[0] == nullptr);
            invalid_if(it[1] == at<void*>(it[0], 4));
            if (at<std::uint8_t>(it[1], 0x10) != 0) {
                held = true;
                break;
            }
            host.tree_iterator_increment_0059f160(it);
        }
        if (!held) {
            void* node = at<void*>(at<void*>(tree, 4), 4);
            while (at<std::uint8_t>(node, 0x15) == 0) {
                host.tree_erase_subtree_0059fdb0(tree, at<void*>(node, 8));
                void* left = at<void*>(node, 0);
                host.crt_operator_delete_00bf65ac(node);
                node = left;
            }
            at<void*>(at<void*>(tree, 4), 4) = at<void*>(tree, 4);
            at<std::uint32_t>(tree, 8) = 0;
            at<void*>(at<void*>(tree, 4), 0) = at<void*>(tree, 4);
            at<void*>(at<void*>(tree, 4), 8) = at<void*>(tree, 4);
        }
    }

    if (void* speaker = at<void*>(screen, 0x53C)) {
        const double until = static_cast<double>(at<float>(screen, 0x540)) + f64_bits(0x4008000000000000ull);  // 00D7A2B0
        if (until > *static_cast<float*>(host.image_global(0x00F876A4u))) {
            host.hud_map_set_speaking_unit_005a2650(screen, speaker, 0, 0);
        }
        at<void*>(screen, 0x53C) = nullptr;
    }
}

// 005B25D0 BSP_HudMapScreen_UpdateMarkerModels (ledger name). __thiscall(ECX =
// HUD map screen; float dt, char paused), RET 8. EH frame (handler 00C71F3E,
// state 0 around the operator-new 50h marker). Callers 005B45C0 and the
// 005B2D60 thunk (dt 1.0f, paused 0).
//
// Selected objective: hud = [00E198C4]+60h; when hud+5 and hud+64h are set,
// [hud+54h][hud+60h] (subscript check against +58h) else 0. 005A3AA0;
// +594h = [00F876A4]; +598h cleared when set.
// Objectives: the list 008DA180([game+21A4h+4*[game+18ECh]]) (re-fetched
// and compared with the walk's container each pass -> 00BF6713); each
// objective (node+8h) that is 008DA140 shown and 008DA130 active walks its
// entry list +20h (+24h head, checked iterator): every 008DACD0-revealed
// entry gets a 50h-byte marker (no null guard after the allocation):
// +0 = 00CEFFE8, 005A1B40(factory [+5A4h] when objective+18h == 0 else
// [+5A8h], parent +0Ch, &m+4, &m+8, &m+0Ch, &m+10h, &m+30h) (sweepB), the
// 00B179F0(m+10h, 0) colour = global-config dwords +1A8h..+1B4h, +30h =
// 1.0f, +14h = 0, +2Ch = 0, +40h = 1.0f, +1Ch = 0; then +15h = (objective
// +18h == 0), +18h = icon (005A22D0(screen, unit vtable +140h) when the
// entry's unit +0 is non-null, else 0), +20h..+28h = entry +4h..+0Ch, +2Ch =
// 0, +14h = 1, +2Ch = (objective == selected), +1Ch = the unit when it has
// no icon; 005A7A80(+57Ch, &marker).
// 005B28E1: +59Ch = selected; a = (float)(dt * 0.3925 (double 00CF0B90)), b
// = (float)(dt * 0.785 (00CF0B88)); not paused: +58Ch = (float)(+58Ch + a),
// then > 2pi (double 00CE3828; FCOMI + JBE, NaN stays) -> (float)(v - 2pi);
// +590h the same with b.
// Markers i (subscript check): +30h = 1.0f when +2Ch else 0.4f (00CE7804);
// angle = 0. Icon with unit +4h: unit +5Dh or +5Eh set -> angle 0 and the
// position is NOT refreshed (the stack copy keeps the previous marker's
// value; the image's first pass would read uninitialised stack -- here it
// starts at zero); else angle = icon+3Ch (before the step), not paused ->
// icon+3Ch = (float)(+3Ch + (+2Ch ? b : a)), > 2pi (FCOMIP + JBE) -> minus
// 2pi (FSUBR), position = unit vtable +0C4h. No icon or no unit: angle =
// +2Ch ? +590h : +58Ch, position = +1Ch's vtable +0C4h or m+20h.
// frame = 00413920(00413920(00B646E0(&angle), out, diag(s, s, s, 1)), out,
// T(x, (float)((double)i + 1300.0 (00CF0B48) + y), z)), s = +5E4h; m+8h
// vtable +34h(frame). No RNG draws.
void cc13_sweepB5_hud_map_update_marker_models_005b25d0(void* screen, float dt, std::uint8_t paused,
                                                        Cc13SweepB5Host& host) {
    auto invalid_if = [&](bool bad) {
        if (bad) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    void* selected = nullptr;  // [ESP+10h]
    {
        void* hud = at<void*>(image_pointer(host, 0x00E198C4), 0x60);
        if (at<std::uint8_t>(hud, 5) != 0 && at<std::uint8_t>(hud, 0x64) != 0) {
            void** first = at<void**>(hud, 0x54);
            const std::uint32_t index = at<std::uint32_t>(hud, 0x60);
            invalid_if(first == nullptr || !(index < static_cast<std::uint32_t>(at<void**>(hud, 0x58) - first)));
            selected = at<void**>(hud, 0x54)[index];
        }
    }
    host.hud_map_delete_marker_objects_005a3aa0(screen);
    at<float>(screen, 0x594) = *static_cast<float*>(host.image_global(0x00F876A4u));
    if (at<std::uint8_t>(screen, 0x598) != 0) {
        at<std::uint8_t>(screen, 0x598) = 0;
    }

    auto objective_list = [&]() {
        void* game = image_pointer(host, 0x00E188A8);
        return host.objective_manager_008da180(at<void*>(game, 0x21A4 + 4 * at<std::int32_t>(game, 0x18EC)));
    };
    void* list = objective_list();            // [ESP+18h]
    void* node = at<void*>(at<void*>(list, 4), 0);  // [ESP+4Ch]
    for (;;) {
        void* current = objective_list();
        invalid_if(list != current);
        if (node == at<void*>(current, 4)) {
            break;
        }
        invalid_if(node == at<void*>(list, 4));
        void* objective = at<void*>(node, 8);  // [ESP+2Ch]
        if (host.objective_is_shown_in_hud_008da140(objective) && host.objective_is_active_008da130(objective)) {
            void* entries = static_cast<std::uint8_t*>(objective) + 0x20;
            void* entry_node = at<void*>(at<void*>(objective, 0x24), 0);  // [ESP+40h]
            const bool is_selected = objective == selected;               // [ESP+17h]
            for (;;) {
                void* entries_end = at<void*>(entries, 4);  // [ESP+34h], re-read per pass
                invalid_if(entries == nullptr);
                if (entry_node == entries_end) {
                    break;
                }
                invalid_if(entries == nullptr);
                invalid_if(entry_node == at<void*>(entries, 4));
                void* entry = at<void*>(entry_node, 8);
                if (host.objective_entry_is_revealed_008dacd0(entry)) {
                    void* icon = nullptr;  // [ESP+20h]
                    if (void* unit = at<void*>(entry, 0)) {
                        icon = host.hud_map_find_unit_icon_005a22d0(screen, host.unit_vcall_140(unit));
                    }
                    auto* marker = static_cast<std::uint8_t*>(host.crt_operator_new_00bf681b(0x50));
                    if (marker != nullptr) {
                        void* factory = at<std::int32_t>(objective, 0x18) == 0 ? at<void*>(screen, 0x5A4)
                                                                                 : at<void*>(screen, 0x5A8);
                        void* config = host.global_config_00432650();
                        at<std::uint32_t>(marker, 0) = 0x00CEFFE8u;  // vtable
                        cc13_sweepB_hud_map_create_alpha_blend_model_005a1b40(
                            factory, at<void*>(screen, 0xC), &at<void*>(marker, 4), &at<void*>(marker, 8),
                            &at<void*>(marker, 0xC), &at<void*>(marker, 0x10), marker + 0x30, host);
                        std::uint32_t* colour = reinterpret_cast<std::uint32_t*>(
                            host.material_diffuse_color_00b179f0(at<void*>(marker, 0x10), 0));
                        colour[0] = at<std::uint32_t>(config, 0x1A8);
                        colour[1] = at<std::uint32_t>(config, 0x1AC);
                        colour[2] = at<std::uint32_t>(config, 0x1B0);
                        colour[3] = at<std::uint32_t>(config, 0x1B4);
                        at<float>(marker, 0x30) = 1.0f;
                        at<std::uint8_t>(marker, 0x14) = 0;
                        at<std::uint8_t>(marker, 0x2C) = 0;
                        at<float>(marker, 0x40) = 1.0f;
                        at<void*>(marker, 0x1C) = nullptr;
                    }
                    at<std::uint8_t>(marker, 0x15) = at<std::int32_t>(objective, 0x18) == 0 ? 1 : 0;
                    at<void*>(marker, 0x18) = icon;
                    at<float>(marker, 0x20) = at<float>(entry, 4);
                    at<float>(marker, 0x24) = at<float>(entry, 8);
                    at<float>(marker, 0x28) = at<float>(entry, 0xC);
                    at<std::uint8_t>(marker, 0x2C) = 0;
                    at<std::uint8_t>(marker, 0x14) = 1;
                    at<std::uint8_t>(marker, 0x2C) = is_selected ? 1 : 0;
                    void* pushed = marker;  // [ESP+1Ch]
                    if (void* unit = at<void*>(entry, 0)) {
                        if (icon == nullptr) {
                            at<void*>(marker, 0x1C) = unit;
                        }
                    }
                    host.pointer_vector_push_back_005a7a80(static_cast<std::uint8_t*>(screen) + 0x57C, &pushed);
                }
                invalid_if(entry_node == at<void*>(objective, 0x24));
                entry_node = at<void*>(entry_node, 0);
            }
        }
        invalid_if(node == at<void*>(list, 4));
        node = at<void*>(node, 0);
    }

    at<void*>(screen, 0x59C) = selected;
    const float step_a = static_cast<float>(f64_bits(0x3FD91EB860000000ull) * dt);  // 00CF0B90, 0.3925
    const float step_b = static_cast<float>(static_cast<double>(dt) * f64_bits(0x3FE91EB860000000ull));  // 00CF0B88, 0.785
    const double two_pi = f64_bits(0x401921FB60000000ull);  // 00CE3828
    if (paused == 0) {
        for (std::size_t offset : {std::size_t{0x58C}, std::size_t{0x590}}) {
            const float v = static_cast<float>(at<float>(screen, offset) + (offset == 0x58C ? step_a : step_b));
            at<float>(screen, offset) = v;
            if (v > two_pi) {
                at<float>(screen, offset) = static_cast<float>(v - two_pi);
            }
        }
    }

    // [ESP+30h], kept across markers. UNCERTAIN/DIVERGENT: the image never
    // initialises this slot, so a first marker whose icon unit has +5Dh or +5Eh
    // set reads uninitialised stack; the model starts it at 0.
    float position[3] = {};
    for (std::uint32_t i = 0;; ++i) {
        void** first = at<void**>(screen, 0x580);
        if (first == nullptr || !(i < static_cast<std::uint32_t>(at<void**>(screen, 0x584) - first))) {
            break;
        }
        invalid_if(at<void**>(screen, 0x580) == nullptr ||
                   !(i < static_cast<std::uint32_t>(at<void**>(screen, 0x584) - at<void**>(screen, 0x580))));
        auto* marker = static_cast<std::uint8_t*>(at<void**>(screen, 0x580)[i]);
        const bool lit = at<std::uint8_t>(marker, 0x2C) != 0;
        at<float>(marker, 0x30) = lit ? 1.0f : f32_bits(0x3ECCCCCDu);  // 00CE7804
        float angle = 0.0f;  // [ESP+10h]
        void* icon = at<void*>(marker, 0x18);
        void* icon_unit = icon != nullptr ? at<void*>(icon, 4) : nullptr;
        if (icon_unit != nullptr) {
            if (at<std::uint8_t>(icon_unit, 0x5D) == 0 && at<std::uint8_t>(icon_unit, 0x5E) == 0) {
                angle = at<float>(icon, 0x3C);
                if (paused == 0) {
                    at<float>(icon, 0x3C) = at<float>(icon, 0x3C) + (lit ? step_b : step_a);
                    void* spun = at<void*>(marker, 0x18);
                    if (at<float>(spun, 0x3C) > two_pi) {
                        at<float>(spun, 0x3C) = static_cast<float>(at<float>(spun, 0x3C) - two_pi);
                    }
                }
                float out[3];  // [ESP+48h]
                const float* p = host.unit_vcall_c4_position(at<void*>(at<void*>(marker, 0x18), 4), out);
                position[0] = p[0];
                position[1] = p[1];
                position[2] = p[2];
            }
        } else {
            angle = lit ? at<float>(screen, 0x590) : at<float>(screen, 0x58C);
            const float* p;
            float out[3];  // [ESP+3Ch]
            if (void* target = at<void*>(marker, 0x1C)) {
                p = host.unit_vcall_c4_position(target, out);
            } else {
                p = &at<float>(marker, 0x20);
            }
            position[0] = p[0];
            position[1] = p[1];
            position[2] = p[2];
        }
        const float scale = at<float>(screen, 0x5E4);  // [ESP+1Ch]
        float rotation[16];                           // [ESP+0D8h]
        const float* rot = host.matrix_build_rotation_y_00b646e0(rotation, &angle);
        const float height = static_cast<float>(static_cast<double>(static_cast<std::int32_t>(i)) +
                                                f64_bits(0x4094500000000000ull) + position[1]);  // 00CF0B48, 1300.0
        const float scaling[16] = {scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1};  // [ESP+54h]
        const float lift[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, position[0], height, position[2], 1};  // [ESP+94h]
        float scaled[16];  // [ESP+158h]
        float placed[16];  // [ESP+118h]
        const float* frame = host.matrix_multiply_00413920(host.matrix_multiply_00413920(rot, scaled, scaling),
                                                           placed, lift);
        host.marker_node_vcall_34_set_frame(at<void*>(marker, 8), frame);
    }
}

// 005A91E0 BSP_HudMap_BuildCaptureRadiusModel (ledger name, provisional).
// __thiscall(ECX = HUD map screen; float scale, const float* xz, float height,
// float land_radius, float capture_radius, float band), RET 18h. EH frame
// (handler 00C71B77) around the five name strings (states 0..4). Sole caller
// 005A9DB0.
//
// Pool: the vector at screen+4B4h {+0 proxy, +4 first, +8 last, +Ch end};
// empty (null first or size 0) -> 005A2340 (sweepB3) pushed with 005A7A10.
// Loop 005A9235: while first is null or the index screen+4C4h >= size
// (unsigned): the inlined front() check (null or empty -> 00BF6713), clone =
// model vtable +10h(6, 0) of [first[0]]+0, block = operator new(1Ch) with
// +0 = clone (no null check), inlined push_back (spare capacity: store at
// last, last += 4) or, when full, the iterator check (first > last ->
// 00BF6713) and 005A6650(vector, &out, {vector, last}, &block).
// Element: the subscript check (null or index >= size -> 00BF6713) then
// block = first[index]; model = block+0.
// block+4 = (float)(land^2); +8 = (float)((land + band)^2); +0Ch =
// (float)(capture^2); +10h = (float)((band + capture)^2) (x87, double
// intermediates); +14h/+18h = xz[0]/xz[1].
// Frame: scale diag(scale, 1, scale, 1) times the translation (xz[0], height,
// xz[1]) through 00413920(scale, out, translation), then model vtable +38h(out).
// Parameters, each through 00B74640(model, 0) -> 00B732C0(.., 0) -> +20h
// material with a native string built from the literal: 00B18B00
// "cCirclePosition" (00CED564, length 0Fh) -> block+14h; 00B18B20
// "cLandRadSquared" (00CF0194, 0Fh) -> +4; "cLandRadOutSquared" (00CF0180,
// 12h) -> +8; "cCapRadSquared" (00CF0170, 0Eh) -> +0Ch; "cCapRadOutSquared"
// (00CF015C, 11h) -> +10h. Then 00B6D890(model, screen+0Ch) and the index
// += 1. The matrix and xz reads happen before the registrations in the image
// (stack locals), as here. No RNG draws.
void cc13_sweepB5_hud_map_build_capture_radius_model_005a91e0(void* screen, float scale, const float* xz,
                                                              float height, float land_radius,
                                                              float capture_radius, float band,
                                                              Cc13SweepB5Host& host) {
    void* pool = static_cast<std::uint8_t*>(screen) + 0x4B4;
    auto first = [&]() { return at<void**>(pool, 4); };
    auto last = [&]() { return at<void**>(pool, 8); };
    auto size = [&]() { return static_cast<std::uint32_t>(last() - first()); };

    if (first() == nullptr || size() == 0) {
        void* block = cc13_sweepB3_hud_map_create_building_quad_model_005a2340(host);
        host.pointer_vector_push_back_005a7a10(pool, &block);
    }
    while (first() == nullptr || at<std::uint32_t>(screen, 0x4C4) >= size()) {
        if (first() == nullptr || size() == 0) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        void* clone = host.model_vcall_10_clone(at<void*>(*first(), 0), 6, 0);
        void* block = host.crt_operator_new_00bf681b(0x1C);
        at<void*>(block, 0) = clone;
        const std::uint32_t count = first() == nullptr ? 0u : size();
        if (first() != nullptr && count < static_cast<std::uint32_t>(at<void**>(pool, 0xC) - first())) {
            *last() = block;
            at<void**>(pool, 8) = last() + 1;
            continue;
        }
        void** where = last();
        if (first() > where) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        void* out_iterator[2];
        host.pointer_vector_insert_005a6650(pool, out_iterator, pool, where, &block);
    }
    const std::uint32_t index = at<std::uint32_t>(screen, 0x4C4);
    if (first() == nullptr || index >= size()) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    void* block = first()[index];

    const double land = land_radius;
    const double land_out = land + band;
    at<float>(block, 4) = static_cast<float>(land * land);
    at<float>(block, 8) = static_cast<float>(land_out * land_out);
    const double capture = capture_radius;
    const double capture_out = static_cast<double>(band) + capture;
    at<float>(block, 0xC) = static_cast<float>(capture * capture);
    at<float>(block, 0x10) = static_cast<float>(capture_out * capture_out);
    at<float>(block, 0x14) = xz[0];
    at<float>(block, 0x18) = xz[1];

    const float translation[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f, xz[0], height, xz[1], 1.0f};  // [ESP+20h]
    const float scaling[16] = {scale, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, scale, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};         // [ESP+60h]
    void* model = at<void*>(block, 0);

    struct Parameter {
        std::uint32_t literal;
        std::uint32_t length;
        std::size_t offset;
        bool pair;
    };
    const Parameter parameters[] = {
        {0x00CED564u, 0x0F, 0x14, true},   // cCirclePosition
        {0x00CF0194u, 0x0F, 0x04, false},  // cLandRadSquared
        {0x00CF0180u, 0x12, 0x08, false},  // cLandRadOutSquared
        {0x00CF0170u, 0x0E, 0x0C, false},  // cCapRadSquared
        {0x00CF015Cu, 0x11, 0x10, false},  // cCapRadOutSquared
    };
    for (const Parameter& parameter : parameters) {
        Cc13SweepBNativeString name;
        build_literal(name, parameter.literal, parameter.length, host);
        void* geometry = host.generated_model_geometry_00b74640(model, 0);
        void* material = at<void*>(host.mesh_element_00b732c0(geometry, 0), 0x20);
        void* source = static_cast<std::uint8_t*>(block) + parameter.offset;
        if (parameter.pair) {
            host.material_register_float2_parameter_00b18b00(material, &name, source);
        } else {
            host.material_register_float_parameter_00b18b20(material, &name, source);
        }
        release_string(name, host);
    }

    float frame[16];  // [ESP+A0h]
    const float* placed = host.matrix_multiply_00413920(scaling, frame, translation);
    host.model_vcall_38_set_frame(model, placed);
    host.node_propagate_root_00b6d890(model, at<void*>(screen, 0xC));
    at<std::uint32_t>(screen, 0x4C4) += 1;
}

// 005A9770 BSP_HudMapScreen_AcquireConeModel (ledger name, provisional).
// __thiscall(ECX = HUD map screen; const float* frame, float heading, float
// scale, float stretch, float range, float cone_angle, int layer), RET 1Ch.
// No EH frame. Sole caller 005A9DB0.
//
// Pool (as sweepB3's 005A8F50): checked dword vector +4DCh (+4E0h first,
// +4E4h last, +4E8h end), index +4ECh; empty -> 00506DD0 push_back of
// 005A2730(5000.0f (00CEFF70), cone_angle); while index >= size (unsigned):
// front() check, clone = vtable +10h(6, 0) of first[0], inlined push_back or
// (first > last -> 00BF6713) 00505D50 at last; subscript check; model =
// first[index].
// Both arms: R = 00413920(diag(scale, 1, scale, 1), out, 00B646E0(out,
// &(-0.0f - heading))) (SUBSS from -0.0 (00D7A208)).
// Layer 0Bh: a = (float)(range * 250000.0), c = (float)(-range * 250000.0)
// (double 00CF01A8; FCHS then FMULP), b = (float)(stretch * 6.0 + 1.0)
// (doubles 00CE6628, 00D7A210); P = (T(0, 0, c) * diag(b, 1, b, 1)) * T(0,
// 0, a); frame' = ((P * R) * frame) * T(0, 3110.0f (00CF01A4), 0).
// Otherwise: z = (float)(uint32)((layer * 5 + 5DCh) * 2) (FILD, + 2^32
// float 00CE3978 when negative); frame' = (R * frame) * T(0, z, 0).
// T(x, y, z) is the identity with row 3 = {x, y, z, 1}; every product is
// 00413920(left, out, right). Then model vtable +38h(frame'), 00B17310(
// 00B732C0(00B74640(model, 0), 0)+20h, layer), 00B6D890(model, screen+0Ch),
// index += 1. No RNG draws.
void cc13_sweepB5_hud_map_acquire_cone_model_005a9770(void* screen, const float* frame, float heading,
                                                      float scale, float stretch, float range,
                                                      float cone_angle, std::uint32_t layer,
                                                      Cc13SweepB5Host& host) {
    std::uint8_t* pool = static_cast<std::uint8_t*>(screen) + 0x4DC;
    auto first = [&]() { return at<void**>(pool, 4); };
    auto size = [&]() { return static_cast<std::uint32_t>(at<void**>(pool, 8) - first()); };
    if (first() == nullptr || size() == 0) {
        void* model = cc13_sweepB4_hud_map_create_cone_model_005a2730(f32_bits(0x459C4000u), cone_angle, host);
        host.dword_vector_push_back_00506dd0(pool, &model);
    }
    while (first() == nullptr || !(at<std::uint32_t>(screen, 0x4EC) < size())) {
        if (first() == nullptr || size() == 0) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        void* clone = host.model_vcall_10_clone(first()[0], 6, 0);
        const std::uint32_t count = first() == nullptr ? 0u : size();
        if (first() != nullptr && count < static_cast<std::uint32_t>(at<void**>(pool, 0xC) - first())) {
            *at<void**>(pool, 8) = clone;
            at<void**>(pool, 8) += 1;
            continue;
        }
        void** last = at<void**>(pool, 8);
        if (first() > last) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        void* out_iterator[2];
        host.dword_vector_insert_00505d50(pool, out_iterator, pool, last, &clone);
    }
    const std::uint32_t index = at<std::uint32_t>(screen, 0x4EC);
    if (first() == nullptr || !(index < size())) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    void* model = first()[index];

    auto translation = [](float* m, float x, float y, float z) {
        const float t[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1};
        std::memcpy(m, t, sizeof t);
    };
    const float angle = f32_bits(0x80000000u) - heading;  // [ESP+10h]
    const float scaling[16] = {scale, 0, 0, 0, 0, 1, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1};  // [ESP+14h]
    const float* placed;
    float rotation[16];
    float turned[16];
    float first_product[16];
    float second_product[16];
    float third_product[16];
    if (layer == 0xB) {
        const double r = range;
        const double scale_factor = f64_bits(0x410E848000000000ull);  // 00CF01A8, 250000.0
        const float a = static_cast<float>(r * scale_factor);
        const float b = static_cast<float>(static_cast<double>(stretch) * f64_bits(0x4018000000000000ull) +
                                           f64_bits(0x3FF0000000000000ull));  // 00CE6628 6.0, 00D7A210 1.0
        const float c = static_cast<float>(scale_factor * -r);
        float back[16];   // [ESP+14h]
        float widen[16] = {b, 0, 0, 0, 0, 1, 0, 0, 0, 0, b, 0, 0, 0, 0, 1};  // [ESP+60h]
        float out[16];    // [ESP+0A0h]
        translation(back, 0.0f, 0.0f, c);
        translation(out, 0.0f, 0.0f, a);
        float tmp[16];    // [ESP+120h]
        float cone[16];   // [ESP+0E0h]
        const float* p = host.matrix_multiply_00413920(host.matrix_multiply_00413920(back, tmp, widen), cone, out);
        const float* rot = host.matrix_build_rotation_y_00b646e0(rotation, &angle);
        const float* r_matrix = host.matrix_multiply_00413920(scaling, turned, rot);
        float lift[16];
        translation(lift, 0.0f, f32_bits(0x45426000u), 0.0f);  // 00CF01A4, 3110.0f
        const float* x1 = host.matrix_multiply_00413920(p, first_product, r_matrix);
        const float* x2 = host.matrix_multiply_00413920(x1, second_product, frame);
        placed = host.matrix_multiply_00413920(x2, third_product, lift);
    } else {
        const float* rot = host.matrix_build_rotation_y_00b646e0(rotation, &angle);
        const float* r_matrix = host.matrix_multiply_00413920(scaling, turned, rot);
        const std::uint32_t raw = (layer * 5 + 0x5DC) * 2;
        double wide = static_cast<double>(static_cast<std::int32_t>(raw));
        if (static_cast<std::int32_t>(raw) < 0) {
            wide += static_cast<double>(f32_bits(0x4F800000u));  // 00CE3978
        }
        float lift[16];
        translation(lift, 0.0f, static_cast<float>(wide), 0.0f);
        const float* x1 = host.matrix_multiply_00413920(r_matrix, first_product, frame);
        placed = host.matrix_multiply_00413920(x1, second_product, lift);
    }
    host.model_vcall_38_set_frame(model, placed);
    void* element = host.mesh_element_00b732c0(host.generated_model_geometry_00b74640(model, 0), 0);
    host.material_set_alpha_ref_00b17310(at<void*>(element, 0x20), layer);
    host.node_propagate_root_00b6d890(model, at<void*>(screen, 0xC));
    at<std::uint32_t>(screen, 0x4EC) += 1;
}

// ---------------------------------------------------------------------------
// Segment 22
// ---------------------------------------------------------------------------

// 005D18A0 BSP_MultiChatScreen_Update (ledger name, provisional).
// __thiscall(ECX = chat screen; float), RET 4. The argument slot is
// overwritten at entry with game+21ECh (the frame dt), so the passed value
// is never read. EH frame (handler 00C73CD8) around the local
// std::vector<int> (states 0/2/6/8). Vtable 00CF1998 slot +20h.
//
// Mode +40h, editor at +8h, flag +64h (open), widgets +44h/+48h/+4Ch/+60h.
// "Park": a local vector {+0 proxy (left unset by the image), +4, +8, +0Ch}
// gets 28h, 55h, 57h, 5Dh, 15h by 00442190, then 004D6480(&vector, 0) and
// the buffer is freed (00BF65AC). "Close to 3": +40h = 3, +44h/+48h/+4Ch
// vtable +34h(0/1/0), +8Dh = 1; "close to 0": +40h = 0 and the three +34h(0);
// both then +64h = 0, 00A966E0(+8h, 0, [+5Ch]) and +60h vtable +34h(0).
// With the interface manager [00E198C4]: 004C43C0(game, F9h) pressed, game
// +218Ch clear, [[mgr+0E4h]+5] clear and 005B66D0 false -> unless the device
// 004BA6D0([00F8BBF4], 0, 0) has key 0Fh down together with B8h or 38h
// (vtable +20h), 005D11F0(screen, +64h ? 3 : 4). Otherwise, open: when
// [00E198B4]+3Ch is set, or when 004D92B0(game, 4Bh) fires, a mode other than
// 3 closes to 3 (parking first from mode 4); not open (or the action did
// not fire): modes 1 and 2 close to 3.
// Without the manager: [00E198B4] set with +3Ch and +4 == 14h -> 005D11F0(1)
// unless mode 2; otherwise a non-zero mode closes to 0 (parking first from
// mode 4).
// Tail: open -> 00A96F40(+8h, dt); mode 4 -> 005D0C60(+54h, +44h), mode 3 ->
// 005D0C60(+50h, +48h). Manager present, mode 3 and +70h > 0 (unsigned):
// fade pass over the timer list +68h in step with the row list [+48h]+64h
// (started one past its begin by 00519E00): timer = (float)(timer - dt); 0 <=
// timer < 1 (COMISS + JB / JBE, NaN skips) -> the row widget's colour
// (vtable +54h) gets alpha = timer (005CF010) and vtable +50h; then a sweep
// from the row list's second node erases every timer < 0 (COMISS + JBE)
// through 005CF8A0 and detaches (00AA83A0) and deletes (vtable +4h(1)) the
// paired row. Every 00BF6713 check as written. No RNG draws.
void cc13_sweepB5_multi_chat_screen_update_005d18a0(void* screen, float unused, Cc13SweepB5Host& host) {
    static_cast<void>(unused);
    auto invalid_if = [&](bool bad) {
        if (bad) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    float dt = at<float>(image_pointer(host, 0x00E188A8), 0x21EC);  // [ESP+50h], the argument slot
    auto park = [&]() {
        alignas(4) std::uint8_t kept[0x10] = {};  // [ESP+30h]
        for (std::int32_t id : {0x28, 0x55, 0x57, 0x5D, 0x15}) {
            const std::int32_t value = id;  // [ESP+10h]
            host.dword_vector_push_back_00442190(kept, &value);
        }
        host.gui_screens_park_widgets_004d6480(kept, 0);
        if (void* first = at<void*>(kept, 4)) {
            host.crt_operator_delete_00bf65ac(first);
        }
    };
    auto close_editor = [&]() {
        const std::int32_t row = at<std::int32_t>(screen, 0x5C);
        at<std::uint8_t>(screen, 0x64) = 0;
        host.text_editor_set_active_00a966e0(static_cast<std::uint8_t*>(screen) + 8, 0, row);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x60), 0);
    };
    auto close_to_3 = [&]() {
        at<std::int32_t>(screen, 0x40) = 3;
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x44), 0);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x48), 1);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x4C), 0);
        at<std::uint8_t>(screen, 0x8D) = 1;
        close_editor();
    };
    auto close_to_0 = [&]() {
        at<std::int32_t>(screen, 0x40) = 0;
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x44), 0);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x48), 0);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x4C), 0);
        close_editor();
    };

    if (image_pointer(host, 0x00E198C4) != nullptr) {
        bool toggle = host.input_action_was_pressed_004c43c0(image_pointer(host, 0x00E188A8), 0xF9);
        void* game = image_pointer(host, 0x00E188A8);
        if (toggle) {
            void* manager = image_pointer(host, 0x00E198C4);
            toggle = at<std::uint8_t>(game, 0x218C) == 0 && at<std::uint8_t>(at<void*>(manager, 0xE4), 5) == 0 &&
                     !host.interface_transition_pending_005b66d0(manager);
        }
        if (toggle) {
            void* device = host.input_backend_device_004ba6d0(image_pointer(host, 0x00F8BBF4), 0, 0);
            const bool chord = host.input_device_vcall_20_key_down(device, 0xF) &&
                               (host.input_device_vcall_20_key_down(device, 0xB8) ||
                                host.input_device_vcall_20_key_down(device, 0x38));
            if (!chord) {
                host.multi_chat_set_mode_005d11f0(screen, at<std::uint8_t>(screen, 0x64) != 0 ? 3 : 4);
            }
        } else {
            bool close_open = false;
            if (at<std::uint8_t>(screen, 0x64) != 0) {
                close_open = at<std::uint8_t>(image_pointer(host, 0x00E198B4), 0x3C) != 0 ||
                             host.frontend_action_edge_or_repeat_004d92b0(game, 0x4B) != 0;
            }
            if (close_open) {
                const std::int32_t mode = at<std::int32_t>(screen, 0x40);
                if (mode != 3) {
                    if (mode == 4) {
                        park();
                    }
                    close_to_3();
                }
            } else {
                const std::int32_t mode = at<std::int32_t>(screen, 0x40);
                if (mode == 2 || mode == 1) {
                    close_to_3();
                }
            }
        }
    } else {
        void* chat = image_pointer(host, 0x00E198B4);
        if (chat != nullptr && at<std::uint8_t>(chat, 0x3C) != 0 && at<std::int32_t>(chat, 4) == 0x14) {
            if (at<std::int32_t>(screen, 0x40) != 2) {
                host.multi_chat_set_mode_005d11f0(screen, 1);
            }
        } else {
            const std::int32_t mode = at<std::int32_t>(screen, 0x40);
            if (mode != 0) {
                if (mode == 4) {
                    park();
                }
                close_to_0();
            }
        }
    }

    if (at<std::uint8_t>(screen, 0x64) != 0) {
        host.text_owner_dispatch_queued_events_00a96f40(static_cast<std::uint8_t*>(screen) + 8, dt);
    }
    if (at<std::int32_t>(screen, 0x40) == 4) {
        host.multi_chat_rebuild_message_rows_005d0c60(screen, at<void*>(screen, 0x54), at<void*>(screen, 0x44));
    } else if (at<std::int32_t>(screen, 0x40) == 3) {
        host.multi_chat_rebuild_message_rows_005d0c60(screen, at<void*>(screen, 0x50), at<void*>(screen, 0x48));
    }

    if (image_pointer(host, 0x00E198C4) == nullptr || at<std::int32_t>(screen, 0x40) == 4 ||
        at<std::int32_t>(screen, 0x40) != 3 || !(at<std::uint32_t>(screen, 0x70) > 0)) {
        return;
    }
    void* timers = static_cast<std::uint8_t*>(screen) + 0x68;
    void* rows[2];  // [ESP+10h] {container, node}
    rows[0] = static_cast<std::uint8_t*>(at<void*>(screen, 0x48)) + 0x64;
    rows[1] = at<void*>(at<void*>(at<void*>(screen, 0x48), 0x68), 0);
    host.list_iterator_increment_00519e00(rows);
    void* timer_it[2] = {timers, at<void*>(at<void*>(screen, 0x6C), 0)};  // [ESP+18h]
    for (;;) {
        void* node = timer_it[1];
        if (node == at<void*>(timers, 4)) {
            break;
        }
        invalid_if(node == at<void*>(timers, 4));
        at<float>(node, 8) = static_cast<float>(static_cast<double>(at<float>(node, 8)) - dt);
        invalid_if(node == at<void*>(timers, 4));
        if (!(at<float>(node, 8) < 0.0f) && !std::isnan(at<float>(node, 8))) {
            invalid_if(node == at<void*>(timers, 4));
            if (1.0f > at<float>(node, 8)) {
                void* widget = *host.list_iterator_value_00425ac0(rows);
                float colour[4];  // [ESP+20h]
                host.widget_vcall_54_colour(widget, colour);
                colour[3] = *host.list_iterator_value_005cf010(timer_it);
                host.widget_vcall_50_colour(widget, colour);
            }
        }
        invalid_if(rows[0] == nullptr);
        invalid_if(rows[1] == at<void*>(rows[0], 4));
        rows[1] = at<void*>(rows[1], 0);
        invalid_if(node == at<void*>(timers, 4));
        timer_it[1] = at<void*>(node, 0);
    }

    void* row_container = static_cast<std::uint8_t*>(at<void*>(screen, 0x48)) + 0x64;  // [ESP+10h]
    void* row_begin = at<void*>(at<void*>(at<void*>(screen, 0x48), 0x68), 0);         // [ESP+14h]
    invalid_if(row_begin == at<void*>(at<void*>(screen, 0x48), 0x68));
    void* row = at<void*>(row_begin, 0);
    void* container = timers;                                 // [ESP+18h]
    void* node = at<void*>(at<void*>(timers, 4), 0);
    for (;;) {
        void* end = at<void*>(timers, 4);  // [ESP+24h]
        invalid_if(container == nullptr || container != timers);
        if (node == end) {
            return;
        }
        invalid_if(container == nullptr);
        invalid_if(node == at<void*>(container, 4));
        if (0.0f > at<float>(node, 8)) {
            void* out[2];  // [ESP+30h]
            void** next = host.chat_list_erase_005cf8a0(timers, out, container, node);
            container = next[0];
            node = next[1];
            invalid_if(row == at<void*>(row_container, 4));
            invalid_if(row == at<void*>(row_container, 4));
            void* widget = at<void*>(row, 8);  // stored over [ESP+50h]
            row = at<void*>(row, 0);
            host.gui_widget_detach_child_00aa83a0(at<void*>(screen, 0x48), widget);
            if (widget != nullptr) {
                host.object_vcall_04_delete(widget, 1);
            }
        } else {
            invalid_if(node == at<void*>(container, 4));
            invalid_if(row == at<void*>(row_container, 4));
            node = at<void*>(node, 0);
            row = at<void*>(row, 0);
        }
    }
}

// 005D26D0 BSP_MultiErrorScreen_Enter (ledger name). __thiscall(ECX = error
// screen), RET; EBP frame aligned to 8, EH frame (handler 00C73D88, states
// 0..0Dh). Vtable 00CF1BF4 slot +18h; no direct callers.
//
// Children (00AA7E00(parent, &name, 1), names built and released): +14h =
// "Message_Text" and +18h = "MessageLower_Text" of +10h; +2Ch =
// "dialog_BG_FrameBox" of +10h; +1Ch = "Options_Listbox" of +2Ch; +20h
// "Yes_Text", +24h "No_Text", +28h "Accept_Text" of +1Ch. +2Ch vtable
// +84h([00E198C4] != 0). +50h = 1, then byte [00F88A30] clear -> +50h = 0,
// set -> 00AA0F70(004C12B0()).
// Glyph text: 00531030(a, -5Dh), 00531030(b, -5Eh), c = 004261A0(b, out,
// a); wide = 004C5E60(c.data, or 00E19536 when null); c, b, a released.
// For +20h/+24h/+28h: 00531130(text, &wide, screen+0Ch, text), 00AA6BC0(
// text, 0, 0), +1B4h = 1, +1B8h = (float)(0.0 / 960.0) (00CEC380; +0.0f),
// +1BCh = 0.011111111f (00CED318). Then 00AB80C0 and 00ABBE50("", 1) on each.
// Messages: the native-string vector at screen + 30h + 10h*[+50h] (+4h
// first, +8h last; subscript checks as written): entries 0/1/2 to
// +20h/+24h/+28h by 00ABAED0(.., 1). 00A9AC40(+1Ch, 0), vtable +60h(0),
// 00A9BF90.
// +54h == 4 and [00E08898] == 4: entries 3/4 to +20h/+24h, +20h/+24h shown,
// +28h hidden, and with +50h == 0 both +20h and +24h inserted (00A9D750);
// otherwise +20h/+24h hidden, +28h shown and, with +50h == 0, +28h
// inserted. +50h == 0 then: +1Ch vtable +60h(1), 00A9C7C0(0),
// 00A9AC70(006964B0), 00A9AC40(screen+8), vtable +40h(0.0f).
// 005D24F0; the command owner [00E1930C]: +5 set -> vtable +1Ch; +4 = +5 = 0;
// 004F83B0. The wide string is released (length * 2 + 2 bytes). No RNG.
void cc13_sweepB5_multi_error_screen_enter_005d26d0(void* screen, Cc13SweepB5Host& host) {
    auto find_into = [&](std::size_t slot, void* parent, std::uint32_t literal, std::uint32_t length) {
        with_child(parent, literal, length, host, [&](void* child) { at<void*>(screen, slot) = child; });
    };
    find_into(0x14, at<void*>(screen, 0x10), 0x00CED36Cu, 0x0C);
    find_into(0x18, at<void*>(screen, 0x10), 0x00CF1BACu, 0x11);
    find_into(0x2C, at<void*>(screen, 0x10), 0x00CED2F4u, 0x12);
    find_into(0x1C, at<void*>(screen, 0x2C), 0x00CED35Cu, 0x0F);
    find_into(0x20, at<void*>(screen, 0x1C), 0x00CED350u, 0x08);
    find_into(0x24, at<void*>(screen, 0x1C), 0x00CED348u, 0x07);
    find_into(0x28, at<void*>(screen, 0x1C), 0x00CED33Cu, 0x0B);
    host.widget_vcall_84(at<void*>(screen, 0x2C), image_pointer(host, 0x00E198C4) != nullptr ? 1 : 0);
    at<std::int32_t>(screen, 0x50) = 1;
    if (*static_cast<std::uint8_t*>(host.image_global(0x00F88A30u)) == 0) {
        at<std::int32_t>(screen, 0x50) = 0;
    } else {
        host.gui_manager_reset_screens_00aa0f70(host.gui_manager_get_004c12b0());
    }

    Cc13SweepBNativeString wide;  // [ESP+18h], {length, wchar_t* data}
    {
        Cc13SweepBNativeString a;  // [ESP+30h]
        Cc13SweepBNativeString b;  // [ESP+28h]
        Cc13SweepBNativeString c;  // [ESP+20h]
        void* first = host.native_string_construct_byte_00531030(&a, -0x5D);
        void* second = host.native_string_construct_byte_00531030(&b, -0x5E);
        const auto* joined =
            static_cast<const Cc13SweepBNativeString*>(host.native_string_concat_004261a0(second, &c, first));
        const char* data = joined->data != nullptr ? joined->data
                                                   : static_cast<const char*>(host.image_global(0x00E19536u));
        host.wide_string_construct_004c5e60(&wide, data);
        release_string(c, host);
        release_string(b, host);
        release_string(a, host);
    }
    const float origin = static_cast<float>(0.0 / f64_bits(0x408E000000000000ull));  // 00CEC380, 960.0
    for (std::size_t slot : {std::size_t{0x20}, std::size_t{0x24}, std::size_t{0x28}}) {
        void* text = at<void*>(screen, slot);
        host.text_context_set_glyph_reference_00531130(text, &wide, static_cast<std::uint8_t*>(screen) + 0xC, text);
        host.gui_widget_set_listener_00aa6bc0(at<void*>(screen, slot), nullptr, 0);
        void* target = at<void*>(screen, slot);
        at<std::uint8_t>(target, 0x1B4) = 1;
        at<float>(target, 0x1B8) = origin;
        at<float>(target, 0x1BC) = f32_bits(0x3C360B61u);  // 00CED318
    }
    for (std::size_t slot : {std::size_t{0x20}, std::size_t{0x24}, std::size_t{0x28}}) {
        host.text_context_clear_glyph_children_00ab80c0(at<void*>(screen, slot));
        host.gui_text_set_localised_cstring_00abbe50(at<void*>(screen, slot),
                                                     static_cast<const char*>(host.image_global(0x00CE3A0Cu)), 1);
    }

    auto message = [&](std::uint32_t entry) {
        auto* vector = static_cast<std::uint8_t*>(screen) + 0x30 + 0x10 * at<std::int32_t>(screen, 0x50);
        auto* first = at<std::uint8_t*>(vector, 4);
        if (first == nullptr || !(static_cast<std::uint32_t>((at<std::uint8_t*>(vector, 8) - first) >> 3) > entry)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        return at<std::uint8_t*>(vector, 4) + 8 * entry;
    };
    host.gui_text_set_localised_source_00abaed0(at<void*>(screen, 0x20), message(0), 1);
    host.gui_text_set_localised_source_00abaed0(at<void*>(screen, 0x24), message(1), 1);
    host.gui_text_set_localised_source_00abaed0(at<void*>(screen, 0x28), message(2), 1);
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x1C), nullptr);
    host.widget_vcall_60(at<void*>(screen, 0x1C), 0);
    host.gui_listbox_clear_items_00a9bf90(at<void*>(screen, 0x1C));

    if (at<std::int32_t>(screen, 0x54) == 4 && *static_cast<std::int32_t*>(host.image_global(0x00E08898u)) == 4) {
        host.gui_text_set_localised_source_00abaed0(at<void*>(screen, 0x20), message(3), 1);
        host.gui_text_set_localised_source_00abaed0(at<void*>(screen, 0x24), message(4), 1);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x20), 1);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x24), 1);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x28), 0);
        if (at<std::int32_t>(screen, 0x50) == 0) {
            host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x1C), at<void*>(screen, 0x20), nullptr, 0);
            host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x1C), at<void*>(screen, 0x24), nullptr, 0);
        }
    } else {
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x20), 0);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x24), 0);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x28), 1);
        if (at<std::int32_t>(screen, 0x50) == 0) {
            host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x1C), at<void*>(screen, 0x28), nullptr, 0);
        }
    }
    if (at<std::int32_t>(screen, 0x50) == 0) {
        host.widget_vcall_60(at<void*>(screen, 0x1C), 1);
        host.listbox_select_index_00a9c7c0(at<void*>(screen, 0x1C), 0);
        host.gui_listbox_set_input_filter_00a9ac70(at<void*>(screen, 0x1C), 0x006964B0u);
        host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x1C), static_cast<std::uint8_t*>(screen) + 8);
        host.widget_vcall_40_float(at<void*>(screen, 0x1C), 0.0f);
    }

    host.multi_error_show_message_005d24f0(screen);
    void* owner = image_pointer(host, 0x00E1930C);
    if (at<std::uint8_t>(owner, 5) != 0) {
        host.command_owner_vcall_1c(owner);
    }
    at<std::uint8_t>(owner, 4) = 0;
    at<std::uint8_t>(owner, 5) = 0;
    host.frontend_screen_commit_visibility_004f83b0(owner);
    if (wide.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), wide.data, wide.length * 2 + 2, 1);
    }
}

// ---------------------------------------------------------------------------
// Segment 23
// ---------------------------------------------------------------------------

// 005F3B50 BSP_OptionsScreen_BuildContentPage (ledger name, hypothesis).
// __thiscall(ECX = options screen), RET; EBP frame aligned to 8, EH frame
// (handler 00C75FB8, states 0..6 for the native strings). Called from row 6
// of 005F7C40.
//
// 005F3B72..005F3BB6: both listboxes (+84h, +88h) get 00A9AC40(0) then
// 00A9BEC0; 00E19650 = 7 before 00549620([00E1930C], 1). Title
// "FE.opt_title_dlc" (00CF3A00, length 10h) through 005189B0.
// Rows, i from 0 while i < [[00F8A304]+8] (signed, re-read per pass), entry =
// [[00F8A304]+4] + 18h*i: row = 00AAB4C0(template +9Ch, 0);
// "arrow_left_Icon" (00CEB278, 0Fh) and "arrow_right_Icon" (00CEB264, 10h)
// children get 00AA6BC0(screen+0Ch, 0); "main_Text" (00CEE4CC, 9) gets
// 00ABB000(entry (a native string), -1.0f (00D7A260), 1); the game's
// [[00E188A8]+61Fh] set -> row+77h = 1; 00A9D750(+88h, row, 0, 0); entry
// byte +12h set -> row+77h = 1 and both arrow children get vtable +34h(0).
// Then 00A9AC40(+88h, screen+8), +88h vtable +34h(1), 00A9CDC0(+84h), +88h
// vtable +60h(1), 00A9C8F0(+88h, 0), +88h floats +12Ch/+130h/+134h =
// 0.003125f (00CEE944) / 0.0027777778f (00CEE600) / 0; 005F0CD0; the help
// line "" (00CE3A0C, length 0) through 0054A0C0([00E1930C]); +254h vtable
// +34h(1) and +84h(game+61Fh != 0); +258h vtable +34h(the zero-extended
// byte game+61Fh). The +88h cursor value (the checked list iterator
// +108h/+10Ch over the list at +FCh, end = +100h -> null, else node+8h; the
// 00BF6713 checks as written) goes to screen+8 vtable +8h(value, +88h).
// Byte +251h set -> cleared, else +250h cleared; then 005F2D20. No RNG.
void cc13_sweepB5_options_screen_build_content_page_005f3b50(void* screen, Cc13SweepB5Host& host) {
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x84), nullptr);
    host.gui_listbox_clear_rows_00a9bec0(at<void*>(screen, 0x84));
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x88), nullptr);
    host.gui_listbox_clear_rows_00a9bec0(at<void*>(screen, 0x88));
    void* owner = image_pointer(host, 0x00E1930C);
    *static_cast<std::int32_t*>(host.image_global(0x00E19650)) = 7;
    host.main_menu_command_owner_set_help_variant_00549620(owner, 1);
    {
        Cc13SweepBNativeString title;
        build_literal(title, 0x00CF3A00u, 0x10, host);
        host.front_end_frame_set_title_005189b0(&title);
        release_string(title, host);
    }

    const float no_width = cc13_sweepB_f32(0xBF800000u);  // 00D7A260
    for (std::int32_t i = 0; i < at<std::int32_t>(image_pointer(host, 0x00F8A304), 8); ++i) {
        void* row = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x9C), nullptr);
        void* listener = static_cast<std::uint8_t*>(screen) + 0xC;
        with_child(row, 0x00CEB278u, 0x0F, host,
                   [&](void* child) { host.gui_widget_set_listener_00aa6bc0(child, listener, 0); });
        with_child(row, 0x00CEB264u, 0x10, host,
                   [&](void* child) { host.gui_widget_set_listener_00aa6bc0(child, listener, 0); });
        with_child(row, 0x00CEE4CCu, 0x09, host, [&](void* child) {
            void* entry = at<std::uint8_t*>(image_pointer(host, 0x00F8A304), 4) + 0x18 * i;
            host.gui_text_set_ellipsized_source_00abb000(child, entry, no_width, 1);
        });
        if (at<std::uint8_t>(image_pointer(host, 0x00E188A8), 0x61F) != 0) {
            at<std::uint8_t>(row, 0x77) = 1;
        }
        host.gui_listbox_insert_row_00a9d750(at<void*>(screen, 0x88), row, nullptr, 0);
        void* entry = at<std::uint8_t*>(image_pointer(host, 0x00F8A304), 4) + 0x18 * i;
        if (at<std::uint8_t>(entry, 0x12) != 0) {
            at<std::uint8_t>(row, 0x77) = 1;
            with_child(row, 0x00CEB278u, 0x0F, host, [&](void* child) { host.widget_vcall_34_set_visible(child, 0); });
            with_child(row, 0x00CEB264u, 0x10, host, [&](void* child) { host.widget_vcall_34_set_visible(child, 0); });
        }
    }

    void* listener = static_cast<std::uint8_t*>(screen) + 8;
    host.gui_listbox_set_listener_00a9ac40(at<void*>(screen, 0x88), listener);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x88), 1);
    host.gui_listbox_clear_00a9cdc0(at<void*>(screen, 0x84));
    host.widget_vcall_60(at<void*>(screen, 0x88), 1);
    host.listbox_select_00a9c8f0(at<void*>(screen, 0x88), 0);
    void* list = at<void*>(screen, 0x88);
    at<float>(list, 0x12C) = cc13_sweepB_f32(0x3B4CCCCDu);  // 00CEE944
    at<float>(list, 0x130) = cc13_sweepB_f32(0x3B360B61u);  // 00CEE600
    at<float>(list, 0x134) = 0.0f;
    host.options_screen_refresh_help_line_005f0cd0(screen);
    {
        Cc13SweepBNativeString help;
        build_literal(help, 0x00CE3A0Cu, 0, host);
        host.main_menu_command_bar_set_help_line_0054a0c0(image_pointer(host, 0x00E1930C), &help);
        release_string(help, host);
    }
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x254), 1);
    host.widget_vcall_84(at<void*>(screen, 0x254),
                         at<std::uint8_t>(image_pointer(host, 0x00E188A8), 0x61F) != 0 ? 1 : 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x258),
                                     at<std::uint8_t>(image_pointer(host, 0x00E188A8), 0x61F));

    void* box = at<void*>(screen, 0x88);
    void* container = at<void*>(box, 0x108);
    void* end = at<void*>(box, 0x100);
    if (container == nullptr || container != static_cast<std::uint8_t*>(box) + 0xFC) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    void* value = nullptr;
    if (at<void*>(box, 0x10C) != end) {
        if (at<void*>(box, 0x108) == nullptr) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        if (at<void*>(box, 0x10C) == at<void*>(at<void*>(box, 0x108), 4)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        value = at<void*>(at<void*>(box, 0x10C), 8);
    }
    host.listbox_listener_vcall_08_selected(listener, value, at<void*>(screen, 0x88));
    if (at<std::uint8_t>(screen, 0x251) != 0) {
        at<std::uint8_t>(screen, 0x251) = 0;
    } else {
        at<std::uint8_t>(screen, 0x250) = 0;
    }
    host.options_screen_refresh_content_rows_005f2d20(screen);
}

// ---------------------------------------------------------------------------
// Segment 26
// ---------------------------------------------------------------------------

// 00621C60 BSP_RewardsScreen_RefreshPage (ledger name). __thiscall(ECX =
// rewards screen, int page), RET 4; EBP frame aligned to 8, EH frame
// (handler 00C783F0, states 0..10h for the native strings). Callers 00622E30
// (tail jump) and 0062B110.
//
// Title: the native-string vector {[00E196C0], [00E196C4]} (8-byte entries)
// subscript check, then 005189B0(&entry[page]). Page widgets +40h..+54h get
// vtable +34h(i == page).
// +258h set: 0054B530([00E1930C]) with five (0, "", 1) triples (five empty
// strings built and released).
// Otherwise c = +1Dh ? B3h : 0 and F = "globals.navigate" (00CEB394) or ""
// (inlined strlen). The second triple's command is the stack dword whose
// low byte is c and whose upper bytes are the screen pointer the slot held
// (rebuilt exactly here). Page 1 with 0061D8B0: the active xuid 00A3EB00
// ([00F8ABE8]) differs from 0061D710 -> (A2h, "globals.continue", 0), (c, F,
// 1), (A7h, "globals.playerreview", 1), (A5h, "globals.showgamercard", 2),
// (0, "", 1); equal -> the same with (0, "", 1) third. Other pages: (A2h,
// "globals.continue", 0), (c, F, 1) and three (0, "", 1). Strings released
// in reverse; F last. Commands are passed as the sign-extended pushes. No
// RNG draws.
void cc13_sweepB5_rewards_screen_refresh_page_00621c60(void* screen, std::uint32_t page, Cc13SweepB5Host& host) {
    {
        auto** first = static_cast<std::uint8_t**>(host.image_global(0x00E196C0u));
        auto** last = static_cast<std::uint8_t**>(host.image_global(0x00E196C4u));
        if (*first == nullptr || !(page < static_cast<std::uint32_t>((*last - *first) >> 3))) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
        host.front_end_frame_set_title_005189b0(*first + 8 * page);
    }
    for (std::uint32_t i = 0; i < 6; ++i) {
        host.widget_vcall_34_set_visible(at<void*>(screen, 0x40 + 4 * i), i == page ? 1 : 0);
    }
    if (at<std::uint8_t>(screen, 0x258) != 0) {
        Cc13SweepBNativeString a, b, c, d, e;  // [ESP+38h], +30h, +28h, +20h, +10h
        build_literal(a, 0x00CE3A0Cu, 0, host);
        build_literal(b, 0x00CE3A0Cu, 0, host);
        build_literal(c, 0x00CE3A0Cu, 0, host);
        build_literal(d, 0x00CE3A0Cu, 0, host);
        build_literal(e, 0x00CE3A0Cu, 0, host);
        const Cc13SweepB2Host::CommandTriple triples[5] = {{0, &e, 1}, {0, &d, 1}, {0, &c, 1}, {0, &b, 1}, {0, &a, 1}};
        host.command_bar_rebuild_0054b530(image_pointer(host, 0x00E1930C), triples);
        release_string(e, host);
        release_string(d, host);
        release_string(c, host);
        release_string(b, host);
        release_string(a, host);
        return;
    }
    const std::uint8_t navigate = at<std::uint8_t>(screen, 0x1D) != 0 ? 0xB3 : 0;
    const std::uint32_t merged =
        (static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(screen)) & 0xFFFFFF00u) | navigate;  // [ESP+10h]
    const char* text = static_cast<const char*>(
        host.image_global(at<std::uint8_t>(screen, 0x1D) != 0 ? 0x00CEB394u : 0x00CE3A0Cu));
    Cc13SweepBNativeString f;  // [ESP+18h]
    f.length = 0;
    f.data = nullptr;
    host.native_string_resize_0041dd40(&f, static_cast<std::uint32_t>(std::strlen(text)), 1);
    if (f.data != nullptr) {
        host.crt_memcpy_00bf7680(f.data, text, f.length + 1);
    }
    auto assigned = [&](Cc13SweepBNativeString& s, std::uint32_t literal) {
        host.native_string_assign_0041e870(&s, static_cast<const char*>(host.image_global(literal)));
    };
    if (page == 1 && host.rewards_selected_entry_flagged_0061d8b0(screen)) {
        const std::uint64_t active = host.xenon_active_xuid_00a3eb00(image_pointer(host, 0x00F8ABE8));
        const bool differs = active != host.rewards_selected_xuid_0061d710(screen);
        Cc13SweepBNativeString g, h, i, j;  // [ESP+40h], +48h, +50h, +58h
        assigned(g, 0x00CE3A0Cu);
        assigned(h, 0x00CEEB80u);  // globals.showgamercard
        assigned(i, differs ? 0x00CF5064u : 0x00CE3A0Cu);  // globals.playerreview or ""
        assigned(j, 0x00CEB460u);  // globals.continue
        const Cc13SweepB2Host::CommandTriple triples[5] = {{0xFFFFFFA2u, &j, 0},
                                                           {merged, &f, 1},
                                                           {differs ? 0xFFFFFFA7u : 0u, &i, 1},
                                                           {0xFFFFFFA5u, &h, 2},
                                                           {0, &g, 1}};
        host.command_bar_rebuild_0054b530(image_pointer(host, 0x00E1930C), triples);
        release_string(j, host);
        release_string(i, host);
        release_string(h, host);
        release_string(g, host);
    } else {
        Cc13SweepBNativeString k, l, m, n;  // [ESP+58h], +50h, +48h, +40h
        build_literal(k, 0x00CE3A0Cu, 0, host);
        build_literal(l, 0x00CE3A0Cu, 0, host);
        build_literal(m, 0x00CE3A0Cu, 0, host);
        build_literal(n, 0x00CEB460u, 0x10, host);
        const Cc13SweepB2Host::CommandTriple triples[5] = {
            {0xFFFFFFA2u, &n, 0}, {merged, &f, 1}, {0, &m, 1}, {0, &l, 1}, {0, &k, 1}};
        host.command_bar_rebuild_0054b530(image_pointer(host, 0x00E1930C), triples);
        release_string(n, host);
        release_string(m, host);
        release_string(l, host);
        release_string(k, host);
    }
    release_string(f, host);
}

// ---------------------------------------------------------------------------
// Segment 27
// ---------------------------------------------------------------------------

// 0065FBF0 BSP_HudSupportManagerScreen_UpdateSlotHighlights (ledger name).
// __thiscall(ECX = support-manager screen; float dt), RET 4. EH frame
// (handler 00C7B41E, states 0..6 for the "circle_HL_Icon" etc. strings).
// Sole caller 00675C40 (cc9's support-manager update).
//
// List at screen+15Ch {+0 proxy, +4 head, +8 size (+164h)}; nodes {+0 next,
// +4 prev, +8 slot widget}. Size 0 -> screen+2B0h = 0 and return.
// Walk from head->next with a checked iterator {container = the list, node}
// (each container test -> 00BF6713 as written, unreachable since the
// container is the list itself): stop at the head. For a node that is not
// begin(): prev's "circle_HL_Icon" (00CF4DC0, length 0Eh) colour via vtable
// +54h; 0.5f (00CE3800) > alpha (COMISS + JBE: NaN is not faded) -> stop on
// this node. Then the node's own circle_HL_Icon alpha; alpha != 1.0f (UCOMISS
// + LAHF/TEST 44h/JNP: NaN counts as not lit) -> its alpha a is read again
// (a third lookup), v = (float)(dt * 4.0 (double 00D7A328) + a), v > 1.0
// (FCOMIP + JBE, NaN keeps v) -> 1.0f, and vtable +4Ch(v) on the node's
// "circle_HL_Icon", "circle_s_Icon" (00CF698C, 0Dh) and "icon_s_Icon"
// (00CF6980, 0Bh) children. Next node (end check -> 00BF6713).
// After the walk: node = node->prev (begin check -> 00BF6713); its
// circle_HL_Icon alpha == 1.0f (UCOMISS + JP: NaN is unequal) -> screen+2B0h
// = 0. Every child lookup builds and releases its own native string around
// 00AA7E00(widget, &name, 1). No RNG draws.
void cc13_sweepB5_hud_support_update_slot_highlights_0065fbf0(void* screen, float dt, Cc13SweepB5Host& host) {
    if (at<std::uint32_t>(screen, 0x164) == 0) {
        at<std::uint8_t>(screen, 0x2B0) = 0;
        return;
    }
    void* list = static_cast<std::uint8_t*>(screen) + 0x15C;
    void* container = list;  // [ESP+30h], the iterator's container
    auto head = [&]() { return at<void*>(list, 4); };
    auto invalid_if = [&](bool bad) {
        if (bad) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    // The alpha (+0Ch of the vtable +54h colour) of node's "circle_HL_Icon";
    // the end check runs between the string build and the lookup.
    auto highlight_alpha = [&](void* node) {
        Cc13SweepBNativeString name;
        build_literal(name, 0x00CF4DC0u, 0x0E, host);
        invalid_if(node == at<void*>(container, 4));
        void* child = host.widget_find_child_00aa7e00(at<void*>(node, 8), &name, 1);
        float colour[4];
        const float alpha = host.widget_vcall_54_colour(child, colour)[3];
        release_string(name, host);
        return alpha;
    };

    void* node = at<void*>(head(), 0);
    for (;;) {
        invalid_if(container == nullptr || container != list);
        if (node == head()) {
            break;
        }
        invalid_if(container == nullptr || container != list);
        if (node != at<void*>(head(), 0)) {
            invalid_if(container == nullptr);
            void* prev = at<void*>(node, 4);
            invalid_if(prev == at<void*>(container, 4));
            const bool faded = 0.5f > highlight_alpha(prev);  // 00CE3800
            if (faded) {
                break;
            }
        }
        invalid_if(container == nullptr);
        const bool lit = highlight_alpha(node) == 1.0f;
        if (!lit) {
            const float a = highlight_alpha(node);
            float v = static_cast<float>(static_cast<double>(dt) * f64_bits(0x4010000000000000ull) + a);  // 00D7A328
            if (v > 1.0) {
                v = 1.0f;
            }
            const struct {
                std::uint32_t literal;
                std::uint32_t length;
            } children[] = {{0x00CF4DC0u, 0x0E}, {0x00CF698Cu, 0x0D}, {0x00CF6980u, 0x0B}};
            for (const auto& child_name : children) {
                Cc13SweepBNativeString name;
                build_literal(name, child_name.literal, child_name.length, host);
                invalid_if(node == at<void*>(container, 4));
                void* child = host.widget_find_child_00aa7e00(at<void*>(node, 8), &name, 1);
                host.widget_vcall_4c_set_alpha(child, v);
                release_string(name, host);
            }
        }
        invalid_if(node == at<void*>(container, 4));
        node = at<void*>(node, 0);
    }

    invalid_if(container == nullptr);
    node = at<void*>(node, 4);
    invalid_if(node == at<void*>(container, 4));
    if (highlight_alpha(node) == 1.0f) {
        at<std::uint8_t>(screen, 0x2B0) = 0;
    }
}

// 00660B10 BSP_HudSupportManagerScreen_BeginScrollForward (ledger name).
// __thiscall(ECX = support-manager screen), RET; EBP frame aligned to 8, EH
// frame (handler 00C7B4B0, states 0..9 for the child names). Sole caller
// 00675C40 (cc9's support-manager update).
//
// +2C0h = 1; arrows +100h/+104h vtable +34h(1). Slot arrays (4 entries
// each): A +7Ch (slot root), B +8Ch (icon_L_Icon), C +0CCh (circle_HL_Icon),
// D +9Ch (number_Text), E +108h (circle_small_01_Group), F +118h
// (circle_small_02_Group).
// +154h clear: +13Ch/+140h = A[0]/B[0]; every array shifts down (i = 0..2:
// x[i] = x[i+1], the six arrays interleaved per i); A[3] = 00AAB4C0(+138h,
// 0) with vtable +34h(1); B[3] = A[3]'s "icon_L_Icon" (00CF4DDC, 0Bh) with
// vtable +88h(3, 0, 1.0f); C[3] = "circle_HL_Icon" (00CF4DC0, 0Eh) with
// +34h(0); B[3] vtable +44h((float)(+2A8h * 5.0 (double 00D7A370) -
// +2A4h)); D[3] = B[3]'s "number_Text" (00CF4DD0, 0Bh); A[3] vtable
// +44h((float)(+2A8h * 5.0 + +2A4h)); E[3] / F[3] = A[3]'s
// "circle_small_01_Group" (00CF4DA8, 15h) / "circle_small_02_Group"
// (00CF4D90, 15h), both +34h(0); new = A[3].
// +154h set: +13Ch/+140h = A[3]/B[3]; every array shifts up (i = 3..1:
// x[i] = x[i-1]); the same build into index 0, with B[0] +44h(-0.0f -
// +2A4h) (SUBSS) and A[0] +44h(+2A4h); new = A[0].
// Both: 0065E870(screen, new, 0.0f); +2B8h = 1.0f. No RNG draws.
void cc13_sweepB5_hud_support_begin_scroll_forward_00660b10(void* screen, Cc13SweepB5Host& host) {
    at<std::uint8_t>(screen, 0x2C0) = 1;
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x100), 1);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x104), 1);
    auto find_into = [&](std::size_t slot_offset, void* parent, std::uint32_t literal, std::uint32_t length) {
        with_child(parent, literal, length, host, [&](void* child) { at<void*>(screen, slot_offset) = child; });
    };
    const bool back = at<std::uint8_t>(screen, 0x154) != 0;
    const std::size_t arrays[6] = {0x7C, 0xCC, 0x8C, 0x108, 0x118, 0x9C};  // the image's per-i copy order
    std::size_t index;
    if (!back) {
        at<void*>(screen, 0x13C) = at<void*>(screen, 0x7C);
        at<void*>(screen, 0x140) = at<void*>(screen, 0x8C);
        for (std::size_t i = 0; i < 3; ++i) {
            for (std::size_t array : arrays) {
                at<void*>(screen, array + 4 * i) = at<void*>(screen, array + 4 * (i + 1));
            }
        }
        index = 3;
    } else {
        at<void*>(screen, 0x13C) = at<void*>(screen, 0x88);
        at<void*>(screen, 0x140) = at<void*>(screen, 0x98);
        for (std::size_t i = 3; i >= 1; --i) {
            for (std::size_t array : arrays) {
                at<void*>(screen, array + 4 * i) = at<void*>(screen, array + 4 * (i - 1));
            }
        }
        index = 0;
    }
    const std::size_t a = 0x7C + 4 * index;
    const std::size_t b = 0x8C + 4 * index;
    at<void*>(screen, a) = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x138), nullptr);
    host.widget_vcall_34_set_visible(at<void*>(screen, a), 1);
    find_into(b, at<void*>(screen, a), 0x00CF4DDCu, 0x0B);
    host.widget_vcall_88(at<void*>(screen, b), 3, 0, 1.0f);
    find_into(0xCC + 4 * index, at<void*>(screen, a), 0x00CF4DC0u, 0x0E);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0xCC + 4 * index), 0);
    const double five = f64_bits(0x4014000000000000ull);  // 00D7A370
    if (!back) {
        host.widget_vcall_44_set_float(
            at<void*>(screen, b), static_cast<float>(at<float>(screen, 0x2A8) * five - at<float>(screen, 0x2A4)));
    } else {
        host.widget_vcall_44_set_float(at<void*>(screen, b), f32_bits(0x80000000u) - at<float>(screen, 0x2A4));
    }
    find_into(0x9C + 4 * index, at<void*>(screen, b), 0x00CF4DD0u, 0x0B);
    if (!back) {
        host.widget_vcall_44_set_float(
            at<void*>(screen, a), static_cast<float>(at<float>(screen, 0x2A8) * five + at<float>(screen, 0x2A4)));
    } else {
        host.widget_vcall_44_set_float(at<void*>(screen, a), at<float>(screen, 0x2A4));
    }
    find_into(0x108 + 4 * index, at<void*>(screen, a), 0x00CF4DA8u, 0x15);
    find_into(0x118 + 4 * index, at<void*>(screen, a), 0x00CF4D90u, 0x15);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x108 + 4 * index), 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0x118 + 4 * index), 0);
    host.hud_support_set_large_icon_alpha_0065e870(screen, at<void*>(screen, a), 0.0f);
    at<float>(screen, 0x2B8) = 1.0f;
}

// 00661240 BSP_HudSupportManagerScreen_BeginScrollBack (ledger name).
// __thiscall(ECX = support-manager screen), RET; EBP frame aligned to 8, EH
// frame (handler 00C7B510, states 0..9). Sole caller 00675C40.
//
// +2C1h = 1; arrows +0F8h/+0FCh vtable +34h(0). Slot arrays of 4: A +0ACh
// (root), B +0BCh (icon_L_Icon), C +0DCh (circle_HL_Icon).
// +154h clear: +14Ch/+150h/+148h = B[3]/C[3]/A[3]; C[0] vtable +34h(0);
// shift up (i = 3..1, order A, C, B per i); A[0] = 00AAB4C0(+138h, 0) with
// +34h(1); B[0] = its "icon_L_Icon"; B[0]'s "number_Text" +34h(0); B[0]
// vtable +88h(3, 0, 1.0f); A[0]'s "circle_small_01_Group" and
// "circle_small_02_Group" +34h(0); C[0] = A[0]'s "circle_HL_Icon" with
// +34h(1); B[0] +44h(-0.0f - +2A4h) (SUBSS); A[0] +44h(+2A4h); new = A[0].
// +154h set: +148h/+150h/+14Ch = A[0]/C[0]/B[0]; C[3] +34h(0); shift down
// (i = 0..2, order A, C, B); the same build into index 3 with B[3]
// +44h((float)(+2A8h * 5.0 - +2A4h)) and A[3] +44h((float)(+2A8h * 5.0 +
// +2A4h)) (double 00D7A370); new = A[3].
// Both: 0065E870(screen, new, 0.0f); +2BCh = 1.0f. Child names as in
// 00660B10, each built and released around its 00AA7E00. No RNG draws.
void cc13_sweepB5_hud_support_begin_scroll_back_00661240(void* screen, Cc13SweepB5Host& host) {
    at<std::uint8_t>(screen, 0x2C1) = 1;
    host.widget_vcall_34_set_visible(at<void*>(screen, 0xF8), 0);
    host.widget_vcall_34_set_visible(at<void*>(screen, 0xFC), 0);
    const bool back = at<std::uint8_t>(screen, 0x154) != 0;
    const std::size_t arrays[3] = {0xAC, 0xDC, 0xBC};  // the image's per-i copy order
    std::size_t index;
    if (!back) {
        at<void*>(screen, 0x14C) = at<void*>(screen, 0xC8);
        at<void*>(screen, 0x150) = at<void*>(screen, 0xE8);
        at<void*>(screen, 0x148) = at<void*>(screen, 0xB8);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xDC), 0);
        for (std::size_t i = 3; i >= 1; --i) {
            for (std::size_t array : arrays) {
                at<void*>(screen, array + 4 * i) = at<void*>(screen, array + 4 * (i - 1));
            }
        }
        index = 0;
    } else {
        at<void*>(screen, 0x148) = at<void*>(screen, 0xAC);
        at<void*>(screen, 0x150) = at<void*>(screen, 0xDC);
        at<void*>(screen, 0x14C) = at<void*>(screen, 0xBC);
        host.widget_vcall_34_set_visible(at<void*>(screen, 0xE8), 0);
        for (std::size_t i = 0; i < 3; ++i) {
            for (std::size_t array : arrays) {
                at<void*>(screen, array + 4 * i) = at<void*>(screen, array + 4 * (i + 1));
            }
        }
        index = 3;
    }
    const std::size_t a = 0xAC + 4 * index;
    const std::size_t b = 0xBC + 4 * index;
    const std::size_t c = 0xDC + 4 * index;
    auto hide_child = [&](void* parent, std::uint32_t literal, std::uint32_t length) {
        with_child(parent, literal, length, host, [&](void* child) { host.widget_vcall_34_set_visible(child, 0); });
    };
    at<void*>(screen, a) = host.widget_clone_subtree_00aab4c0(at<void*>(screen, 0x138), nullptr);
    host.widget_vcall_34_set_visible(at<void*>(screen, a), 1);
    with_child(at<void*>(screen, a), 0x00CF4DDCu, 0x0B, host, [&](void* child) { at<void*>(screen, b) = child; });
    hide_child(at<void*>(screen, b), 0x00CF4DD0u, 0x0B);
    host.widget_vcall_88(at<void*>(screen, b), 3, 0, 1.0f);
    hide_child(at<void*>(screen, a), 0x00CF4DA8u, 0x15);
    hide_child(at<void*>(screen, a), 0x00CF4D90u, 0x15);
    with_child(at<void*>(screen, a), 0x00CF4DC0u, 0x0E, host, [&](void* child) { at<void*>(screen, c) = child; });
    host.widget_vcall_34_set_visible(at<void*>(screen, c), 1);
    if (!back) {
        host.widget_vcall_44_set_float(at<void*>(screen, b), f32_bits(0x80000000u) - at<float>(screen, 0x2A4));
        host.widget_vcall_44_set_float(at<void*>(screen, a), at<float>(screen, 0x2A4));
    } else {
        const double five = f64_bits(0x4014000000000000ull);  // 00D7A370
        host.widget_vcall_44_set_float(
            at<void*>(screen, b), static_cast<float>(at<float>(screen, 0x2A8) * five - at<float>(screen, 0x2A4)));
        host.widget_vcall_44_set_float(
            at<void*>(screen, a), static_cast<float>(at<float>(screen, 0x2A8) * five + at<float>(screen, 0x2A4)));
    }
    host.hud_support_set_large_icon_alpha_0065e870(screen, at<void*>(screen, a), 0.0f);
    at<float>(screen, 0x2BC) = 1.0f;
}

// 00668770 BSP_HudSupportManagerScreen_RebuildUnitMarkers (hypothesis; ledger
// FUN_00668770). __thiscall(ECX = support-manager screen), RET; EBP frame
// aligned to 8, no EH frame. Callers 0066FFA0, 00674B80 (Enter) and 00675C40.
//
// 00668779..006687BB: inlined clear of the unit list +1ECh {+1F0h head,
// +1F4h size}: head links reset, size 0, every old node freed (00BF65AC).
// 006687BD..0066881D: every map +3FCh {+400h head, +404h size} value widget
// (node+10h) gets vtable +4h(1) when non-null (checked iterator, 00654650).
// 0066881F..00668863: inlined tree clear: from the root, 00659810(map,
// right) then free the node and go left until the nil byte +15h; head
// parent/left/right = head, size 0.
// 00668866..00668A58: for each [[game+19CCh]+28h] entry (+4h next, +8h unit):
// the unit's party +54h equals the local player's [game + 18CCh +
// 4*[game+18ECh]]+28h, +5Dh clear, kind 46h, 9 or 45h (vtable +5Ch, in that
// order); kind 46h needs a non-null +774h and (+778h - +774h) >> 5 (SAR)
// non-zero, otherwise the +48h count of unit+72Ch (kind 45h) or unit+1188h
// must be non-zero. Then: absent from the list +1F8h (linear walk, +8h
// value) -> push_back on the list +1ECh (00659C30(list, head, head->prev,
// &unit), 00663050(list, 1), relink); absent from the map -> 00AAB4C0 clone
// of [[[00E198C4]+0BCh]+1D8h] stored through 00667E30(map, &unit).
// 00668A5E..00668B04: map entries whose unit has +5Dh set: a copy of the
// iterator is advanced, 00662750 erases the entry (the widget is not freed),
// the iterator takes the copy, and the loop's own 00654650 then advances it
// once more, so the entry after an erased one is not tested (as the image).
// 0065A2B0. 00668B09..00668D28: per map entry, the cursor value
// [[screen+304h]+8h] (checked against +300h) picks vtable +88h(0, 0, 1.0f)
// for the current unit and (1, 0, 1.0f) otherwise, then +34h(1); a unit
// with +0C8h clear refreshes its cached world matrix (00414DB0 on the parent
// +3Ch, then 00413920(+74h, out, parent+0CCh) or +74h itself copied into
// +0CCh..+108h, +0C8h = 1, +10Ch = 0); 00658580(screen, out, +0FCh, +100h,
// +104h) and 00AA7D00(widget, out[0], out[1]). Every 00BF6713 check is
// written as the image has it. No RNG draws.
void cc13_sweepB5_hud_support_rebuild_unit_markers_00668770(void* screen, Cc13SweepB5Host& host) {
    auto invalid_if = [&](bool bad) {
        if (bad) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    std::uint8_t* base = static_cast<std::uint8_t*>(screen);

    {
        void* head = at<void*>(screen, 0x1F0);
        void* node = at<void*>(head, 0);
        at<void*>(head, 0) = head;
        at<void*>(at<void*>(screen, 0x1F0), 4) = at<void*>(screen, 0x1F0);
        at<std::uint32_t>(screen, 0x1F4) = 0;
        while (node != at<void*>(screen, 0x1F0)) {
            void* next = at<void*>(node, 0);
            host.crt_operator_delete_00bf65ac(node);
            node = next;
        }
    }

    void* map = base + 0x3FC;
    // A checked map iterator {container, node} walk; `body` returns false to stop.
    auto walk_map = [&](void** it, auto body) {
        it[0] = map;
        it[1] = at<void*>(at<void*>(screen, 0x400), 0);
        for (;;) {
            void* end = at<void*>(screen, 0x400);
            invalid_if(it[0] == nullptr || it[0] != map);
            if (it[1] == end) {
                return false;
            }
            invalid_if(it[0] == nullptr);
            invalid_if(it[1] == at<void*>(it[0], 4));
            if (!body()) {
                return true;
            }
            host.tree_iterator_increment_00654650(it);
        }
    };

    void* it[2];
    walk_map(it, [&]() {
        if (void* widget = at<void*>(it[1], 0x10)) {
            host.object_vcall_04_delete(widget, 1);
        }
        return true;
    });
    {
        void* node = at<void*>(at<void*>(map, 4), 4);
        while (at<std::uint8_t>(node, 0x15) == 0) {
            host.tree_erase_subtree_00659810(map, at<void*>(node, 8));
            void* left = at<void*>(node, 0);
            host.crt_operator_delete_00bf65ac(node);
            node = left;
        }
        at<void*>(at<void*>(map, 4), 4) = at<void*>(map, 4);
        at<std::uint32_t>(map, 8) = 0;
        at<void*>(at<void*>(map, 4), 0) = at<void*>(map, 4);
        at<void*>(at<void*>(map, 4), 8) = at<void*>(map, 4);
    }

    void* entry = at<void*>(at<void*>(image_pointer(host, 0x00E188A8), 0x19CC), 0x28);
    while (entry != nullptr) {
        void* game = image_pointer(host, 0x00E188A8);
        void* unit = at<void*>(entry, 8);  // [ESP+1Ch]
        void* player = at<void*>(game, 0x18CC + 4 * at<std::int32_t>(game, 0x18EC));
        bool take = at<std::int32_t>(unit, 0x54) == at<std::int32_t>(player, 0x28) &&
                    at<std::uint8_t>(unit, 0x5D) == 0 &&
                    (host.object_vcall_5c_is_kind(unit, 0x46) || host.object_vcall_5c_is_kind(unit, 9) ||
                     host.object_vcall_5c_is_kind(unit, 0x45));
        if (take) {
            if (host.object_vcall_5c_is_kind(unit, 0x46)) {
                const std::uintptr_t first = at<std::uintptr_t>(unit, 0x774);
                take = first != 0 &&
                       (static_cast<std::int32_t>(at<std::uintptr_t>(unit, 0x778) - first) >> 5) != 0;
            } else {
                std::uint8_t* block = host.object_vcall_5c_is_kind(unit, 0x45) ? static_cast<std::uint8_t*>(unit) + 0x72C
                                                                               : static_cast<std::uint8_t*>(unit) + 0x1188;
                take = at<std::int32_t>(block, 0x48) != 0;
            }
        }
        if (take) {
            void* listed = base + 0x1F8;
            void* node = at<void*>(at<void*>(screen, 0x1FC), 0);
            bool found = false;
            while (node != at<void*>(listed, 4)) {
                invalid_if(node == at<void*>(listed, 4));
                if (at<void*>(node, 8) == unit) {
                    found = true;
                    break;
                }
                invalid_if(node == at<void*>(listed, 4));
                node = at<void*>(node, 0);
            }
            if (!found) {
                void* list = base + 0x1EC;
                void* head = at<void*>(screen, 0x1F0);
                void* made = host.unit_list_buy_node_00659c30(list, head, at<void*>(head, 4), &unit);
                host.unit_list_incsize_00663050(list, 1);
                at<void*>(head, 4) = made;
                at<void*>(at<void*>(made, 4), 0) = made;
            }
            void* find[2];
            const bool present = walk_map(find, [&]() { return at<void*>(find[1], 0xC) != unit; });
            if (!present) {
                void* source = at<void*>(at<void*>(image_pointer(host, 0x00E198C4), 0xBC), 0x1D8);
                void* widget = host.widget_clone_subtree_00aab4c0(source, nullptr);
                *host.unit_widget_map_subscript_00667e30(map, &unit) = widget;
            }
        }
        entry = at<void*>(entry, 4);
    }

    walk_map(it, [&]() {
        if (at<std::uint8_t>(at<void*>(it[1], 0xC), 0x5D) != 0) {
            void* next[2] = {it[0], it[1]};
            host.tree_iterator_increment_00654650(next);
            void* out[2];
            host.tree_erase_at_00662750(map, out, it[0], it[1]);
            it[0] = next[0];
            it[1] = next[1];
        }
        return true;
    });

    host.hud_support_sync_cursor_iterators_0065a2b0(screen);

    walk_map(it, [&]() {
        void* node = it[1];
        auto check_node = [&]() { invalid_if(node == at<void*>(it[0], 4)); };
        invalid_if(at<void*>(screen, 0x300) == nullptr);
        invalid_if(at<void*>(screen, 0x304) == at<void*>(at<void*>(screen, 0x300), 4));
        const bool is_current = at<void*>(node, 0xC) == at<void*>(at<void*>(screen, 0x304), 8);
        check_node();
        host.widget_vcall_88(at<void*>(node, 0x10), is_current ? 0 : 1, 0, 1.0f);
        check_node();
        host.widget_vcall_34_set_visible(at<void*>(node, 0x10), 1);
        check_node();
        void* unit = at<void*>(node, 0xC);
        if (at<std::uint8_t>(unit, 0xC8) == 0) {
            if (at<void*>(unit, 0x3C) != nullptr) {
                host.entity_pose_refresh_world_00414db0(at<void*>(unit, 0x3C));
            }
            float product[16];  // [ESP+38h]
            const float* world = at<void*>(unit, 0x3C) != nullptr
                                     ? host.matrix_multiply_00413920(&at<float>(unit, 0x74), product,
                                                                     &at<float>(at<void*>(unit, 0x3C), 0xCC))
                                     : &at<float>(unit, 0x74);
            for (std::size_t i = 0; i < 16; ++i) {
                at<float>(unit, 0xCC + 4 * i) = world[i];
            }
            at<std::uint8_t>(unit, 0xC8) = 1;
            at<std::uint8_t>(unit, 0x10C) = 0;
        }
        float map_xy[2];  // [ESP+30h]
        host.hud_support_world_to_map_00658580(screen, map_xy, at<float>(unit, 0xFC), at<float>(unit, 0x100),
                                               at<float>(unit, 0x104));
        check_node();
        host.widget_set_local_xy_00aa7d00(at<void*>(node, 0x10), map_xy[0], map_xy[1]);
        return true;
    });
}

}  // namespace bsp
